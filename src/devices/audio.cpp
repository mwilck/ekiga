
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         sound_handling.cpp  -  description
 *                         ----------------------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains sound handling functions.
 *
 */

#include "../../config.h"

#include "common.h"
#include "ekiga.h"
#include "audio.h"
#include "manager.h"
#include "misc.h"
#include "druid.h"

#include "gmlevelmeter.h"
#include "gmdialog.h"
#include "gmconf.h"


#ifdef __linux__
#include <linux/soundcard.h>
#endif

#ifdef __FreeBSD__
#if (__FreeBSD__ >= 5)
#include <sys/soundcard.h>
#else
#include <machine/soundcard.h>
#endif
#endif

#include <ptclib/pwavfile.h>
#include <cmath>

#ifdef WIN32
#include "winpaths.h"
#endif

static void dialog_response_cb (GtkWidget *, gint, gpointer);


/* The GTK callbacks */
static void dialog_response_cb (GtkWidget *w, 
				gint, 
				gpointer data)
{
  g_return_if_fail (data);


  gm_druid_window_set_test_buttons_sensitivity (GTK_WIDGET (data), FALSE);

  gtk_widget_hide (w);
}


/* The functions */
void 
gnomemeeting_sound_daemons_suspend (void)
{
#ifndef DISABLE_GNOME
  int esd_client = 0;
  
  /* Put ESD into standby mode */
  esd_client = esd_open_sound (NULL);

  if (esd_client >= 0) {
    
    esd_standby (esd_client);
    esd_close (esd_client);
  }
  PThread::Current ()->Sleep (300); // FIXME dirty workaround for ESD bug
#endif
}


void 
gnomemeeting_sound_daemons_resume (void)
{
#ifndef DISABLE_GNOME
  int esd_client = 0;

  /* Put ESD into normal mode */
  esd_client = esd_open_sound (NULL);

  if (esd_client >= 0) {
    
    esd_resume (esd_client);
    esd_close (esd_client);
  }
#endif
}


GMSoundEvent::GMSoundEvent (PString ev)
{
  event = ev;

  gnomemeeting_sound_daemons_suspend ();
  Main ();
  gnomemeeting_sound_daemons_resume ();
}


void GMSoundEvent::Main ()
{
  gchar *sound_file = NULL;
  gchar *device = NULL;
  gchar *plugin = NULL;
  gchar *filename = NULL;

  PSound sound;
  PSoundChannel *channel = NULL;

  PBYTEArray buffer;  

  PString psound_file;
  PString enable_event_conf_key;
  PString event_conf_key;
  PWAVFile *wav = NULL;
  
  plugin = gm_conf_get_string (AUDIO_DEVICES_KEY "plugin");
  if (event == "busy_tone_sound" || event == "ring_tone_sound")
    device = gm_conf_get_string (AUDIO_DEVICES_KEY "output_device");
  else
    device = gm_conf_get_string (SOUND_EVENTS_KEY "output_device");

  enable_event_conf_key = PString (SOUND_EVENTS_KEY) + "enable_" + event;
  if (event.Find ("/") == P_MAX_INDEX) {
    
    event_conf_key = PString (SOUND_EVENTS_KEY) + event;
    sound_file = gm_conf_get_string ((gchar *) (const char *) event_conf_key);
  }
  
  if (!sound_file ||
      gm_conf_get_bool ((gchar *) (const char *) enable_event_conf_key)) {

    if (!sound_file)    
      sound_file = g_strdup ((const char *) event);

    /* first assume the entry is a full path to a file */
    wav = new PWAVFile (sound_file, PFile::ReadOnly);
   
    if (!wav->IsValid ()) {
      /* it isn't a full path to a file : add our default path */

      delete wav;
      wav = NULL;

      filename = g_build_filename (DATA_DIR, "sounds", PACKAGE_NAME,
				   sound_file, NULL);
      wav = new PWAVFile (filename, PFile::ReadOnly);
      g_free (filename);

    }
 
    if (wav->IsValid ()) {
      
      channel =
	PSoundChannel::CreateOpenedChannel (plugin, device,
					    PSoundChannel::Player, 
					    wav->GetChannels (),
					    wav->GetSampleRate (),
					    wav->GetSampleSize ());
    

      if (channel) {

	channel->SetBuffers (640, 2);
	
	buffer.SetSize (wav->GetDataLength ());
	memset (buffer.GetPointer (), '0', buffer.GetSize ());
	wav->Read (buffer.GetPointer (), wav->GetDataLength ());
      
	sound = buffer;
	channel->PlaySound (sound);
	channel->Close ();
    
	delete (channel);
      }

    }
  }

  delete wav;
  g_free (sound_file);
  g_free (device);
  g_free (plugin);
}


GMAudioRP::GMAudioRP (BOOL enc,
		      GMAudioTester &t)
  :PThread (1000, NoAutoDeleteThread), tester (t)
{
  is_encoding = enc;
  device_name = enc ? tester.audio_recorder:tester.audio_player;
  driver_name = tester.audio_manager;
  stop = FALSE;

  this->Resume ();
  thread_sync_point.Wait ();
}


GMAudioRP::~GMAudioRP ()
{
  stop = TRUE;

  PWaitAndSignal m(quit_mutex);
}


void GMAudioRP::Main ()
{
  GtkWidget *druid_window = NULL;
  
  PSoundChannel *channel = NULL;
  
  gchar *msg = NULL;
  char *buffer = NULL;
  
  BOOL label_displayed = FALSE;

  int buffer_pos = 0;
  static int nbr_opened_channels = 0;

  gfloat val = 0.0;
  
  PTime now;

  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();

  druid_window = GnomeMeeting::Process ()->GetDruidWindow ();
    
  buffer = (char *) malloc (640);
  memset (buffer, '0', sizeof (buffer));
  
  /* We try to open the selected device */
  if (driver_name != "Quicknet") {
    channel = 
      PSoundChannel::CreateOpenedChannel (driver_name,
					  device_name,
					  is_encoding ?
					  PSoundChannel::Recorder
					  : PSoundChannel::Player,
					  1, 16000, 16);
  }
  
  if (!is_encoding)
    msg = g_strdup_printf ("<b>%s</b>", _("Opening device for playing"));
  else
    msg = g_strdup_printf ("<b>%s</b>", _("Opening device for recording"));

  gdk_threads_enter ();
  gtk_label_set_markup (GTK_LABEL (tester.test_label), msg);
  gdk_threads_leave ();
  g_free (msg);

  if (!channel) {

    gdk_threads_enter ();  
    if (is_encoding)
      gnomemeeting_error_dialog (GTK_WINDOW (druid_window), _("Failed to open the device"), _("Impossible to open the selected audio device (%s) for recording. Please check your audio setup, the permissions and that the device is not busy."), (const char *) device_name);
    else
      gnomemeeting_error_dialog (GTK_WINDOW (druid_window), _("Failed to open the device"), _("Impossible to open the selected audio device (%s) for playing. Please check your audio setup, the permissions and that the device is not busy."), (const char *) device_name);
    gdk_threads_leave ();  
  }
  else {
    
    nbr_opened_channels++;

#ifndef WIN32
    channel->SetBuffers (640, 2);
#else
    channel->SetBuffers (640, 3);
#endif
    
    while (!stop) {
      
      if (is_encoding) {

	if (!channel->Read ((void *) buffer, 640)) {
      
	  gdk_threads_enter ();  
	  gnomemeeting_error_dialog (GTK_WINDOW (druid_window), _("Cannot use the audio device"), _("The selected audio device (%s) was successfully opened but it is impossible to read data from this device. Please check your audio setup."), (const char*) device_name);
	  gdk_threads_leave ();  

	  stop = TRUE;
	}
	else {
	  
	  if (!label_displayed) {

	    msg =  g_strdup_printf ("<b>%s</b>", _("Recording your voice"));

	    gdk_threads_enter ();
	    gtk_label_set_markup (GTK_LABEL (tester.test_label), msg);
	    if (nbr_opened_channels == 2)
	      gnomemeeting_threads_dialog_show (GTK_WIDGET (tester.test_dialog));
	    gdk_threads_leave ();
	    g_free (msg);

	    label_displayed = TRUE;
	  }


	  /* We update the VUMeter only 3 times for each sample
	     of size 640, that will permit to spare some CPU cycles */
	  for (int i = 0 ; i < 480 ; i = i + 160) {
	    
	    val = GetAverageSignalLevel ((const short *) (buffer + i), 160); 

	    gdk_threads_enter ();
	    gtk_levelmeter_set_level (GTK_LEVELMETER (tester.level_meter), val);
	    gdk_threads_leave ();
	  }

	  
	  tester.buffer_ring_access_mutex.Wait ();
	  memcpy (&tester.buffer_ring [buffer_pos], buffer,  640); 
	  tester.buffer_ring_access_mutex.Signal ();

	  buffer_pos += 640;
	}
      }
      else {
	
	if ((PTime () - now).GetSeconds () > 3) {
	
	  if (!label_displayed) {

	    msg = g_strdup_printf ("<b>%s</b>", 
				   _("Recording and playing back"));

	    gdk_threads_enter ();
	    gtk_label_set_markup (GTK_LABEL (tester.test_label), msg);
	    if (nbr_opened_channels == 2)
	      gnomemeeting_threads_dialog_show (GTK_WIDGET (tester.test_dialog));
	    gdk_threads_leave ();
	    g_free (msg);

	    label_displayed = TRUE;
	  }

	  tester.buffer_ring_access_mutex.Wait ();
	  memcpy (buffer, &tester.buffer_ring [buffer_pos], 640); 
	  tester.buffer_ring_access_mutex.Signal ();
	
	  buffer_pos += 640;

	  if (!channel->Write ((void *) buffer, 640)) {
      
	    gdk_threads_enter ();  
	    gnomemeeting_error_dialog (GTK_WINDOW (druid_window), _("Cannot use the audio device"), _("The selected audio device (%s) was successfully opened but it is impossible to write data to this device. Please check your audio setup."), (const char*) device_name);
	    gdk_threads_leave ();  

	    stop = TRUE;
	  }
	}
	else
	  PThread::Current ()->Sleep (100);
      }

      if (buffer_pos >= 80000)
	buffer_pos = 0;	
    }
  }

  if (channel) {

    channel->Close ();
    delete (channel);
  }
  
  if (nbr_opened_channels > 0) nbr_opened_channels--;

  free (buffer);
}


gfloat
GMAudioRP::GetAverageSignalLevel (const short *buffer, int size)
{
  int sum = 0;
  int csize = 0;
  
  while (csize < size) {

    if (*buffer < 0)
      sum -= *buffer++;
    else
      sum += *buffer++;

    csize++;
  }
	  
  return log10 (9.0*sum/size/32767+1)*1.0;
}


/* The Audio tester class */
  GMAudioTester::GMAudioTester (gchar *m,
				gchar *p,
				gchar *r,
				GMManager & endpoint)
  :PThread (1000, NoAutoDeleteThread), ep (endpoint)
{
  stop = FALSE;

  test_dialog = NULL;
  test_label = NULL;
  
  if (m)
    audio_manager = PString (m);
  if (p)
    audio_player = PString (p);
  if (r)
    audio_recorder = PString (r);
  
  this->Resume ();
  thread_sync_point.Wait ();
}


GMAudioTester::~GMAudioTester ()
{
  stop = 1;
  PWaitAndSignal m(quit_mutex);
}


void GMAudioTester::Main ()
{
  GtkWidget *druid_window = NULL;
  
  GMAudioRP *player = NULL;
  GMAudioRP *recorder = NULL;

  gchar *msg = NULL;

  druid_window = GnomeMeeting::Process ()->GetDruidWindow ();


  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();

  if (audio_manager.IsEmpty ()
      || audio_recorder.IsEmpty ()
      || audio_player.IsEmpty ()
      || audio_recorder == PString (_("No device found"))
      || audio_player == PString (_("No device found")))
    return;

  gnomemeeting_sound_daemons_suspend ();
  
  buffer_ring = (char *) malloc (8000 /*Hz*/ * 5 /*s*/ * 2 /*16bits*/);

  memset (buffer_ring, '0', sizeof (buffer_ring));

  gdk_threads_enter ();
  test_dialog =
    gtk_dialog_new_with_buttons ("Audio test running",
				 GTK_WINDOW (druid_window),
				 (GtkDialogFlags) (GTK_DIALOG_MODAL),
				 GTK_STOCK_OK,
				 GTK_RESPONSE_ACCEPT,
				 NULL);
  msg = 
    g_strdup_printf (_("Ekiga is now recording from %s and playing back to %s. Please say \"1, 2, 3, Ekiga rocks!\" in your microphone. You should hear yourself back into the speakers with a four-second delay."), (const char *) audio_recorder, (const char *) audio_player);
  test_label = gtk_label_new (msg);
  gtk_label_set_line_wrap (GTK_LABEL (test_label), true);
  g_free (msg);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (test_dialog)->vbox), test_label,
		      FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (test_dialog)->vbox), 
		      gtk_hseparator_new (), FALSE, FALSE, 2);

  test_label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (test_dialog)->vbox), 
		      test_label, FALSE, FALSE, 2);

  level_meter = gtk_levelmeter_new ();
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (test_dialog)->vbox), 
		      level_meter, FALSE, FALSE, 2);
    
  g_signal_connect (G_OBJECT (test_dialog), "delete-event",
		    G_CALLBACK (gtk_widget_hide_on_delete), NULL);
  g_signal_connect (G_OBJECT (test_dialog), "response",
		    G_CALLBACK (dialog_response_cb), 
		    druid_window);

  gtk_window_set_transient_for (GTK_WINDOW (test_dialog),
				GTK_WINDOW (druid_window));
  gtk_widget_show_all (GTK_DIALOG (test_dialog)->vbox);
  gdk_threads_leave ();

  recorder = new GMAudioRP (TRUE, *this);
  player = new GMAudioRP (FALSE, *this);

  
  while (!stop && !player->IsTerminated () && !recorder->IsTerminated ()) {

    PThread::Current ()->Sleep (100);
  }

  delete (player);
  delete (recorder);

  gdk_threads_enter ();
  if (test_dialog)
    gnomemeeting_threads_widget_destroy (test_dialog);
  gdk_threads_leave ();

  gnomemeeting_sound_daemons_resume ();
  
  free (buffer_ring);
}
