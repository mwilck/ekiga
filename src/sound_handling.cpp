
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2003 Damien Sandras
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         sound_handling.cpp  -  description
 *                         ----------------------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains sound handling functions.
 *
 */

#include "../config.h"

#include "common.h"
#include "gnomemeeting.h"
#include "sound_handling.h"
#include "endpoint.h"
#include "misc.h"
#include "tray.h"
#include "dialog.h"


#ifdef HAS_IXJ
#include <ixjlid.h>
#endif

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

static void dialog_response_cb (GtkWidget *, gint, gpointer);
  
extern GtkWidget *gm;


/* The GTK callbacks */
void dialog_response_cb (GtkWidget *w, gint, gpointer data)
{
  GmDruidWindow *dw = NULL;

  dw = GnomeMeeting::Process ()->GetDruidWindow ();

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dw->audio_test_button),
				false);

  gtk_widget_hide (w);
}


/* The functions */
void 
gnomemeeting_sound_daemons_suspend (void)
{
#ifndef WIN32
#if defined(HAS_ESD)
  return;
#else
  int esd_client = 0;
  
  /* Put ESD into standby mode */
  esd_client = esd_open_sound (NULL);

  esd_standby (esd_client);
      
  esd_close (esd_client);
#endif
#endif
}


void 
gnomemeeting_sound_daemons_resume (void)
{
#ifndef WIN32
#if defined(HAS_ESD)
  return;
#else
  int esd_client = 0;

  /* Put ESD into normal mode */
  esd_client = esd_open_sound (NULL);

  esd_resume (esd_client);

  esd_close (esd_client);
#endif
#endif
}


void 
gnomemeeting_mixers_mic_select (void)
{
#ifndef WIN32
#ifndef P_MACOSX
  int rcsrc = 0;
  int mixerfd = -1;                                                            
  int cpt = -1;
  PString mixer_name = PString ("/dev/mixer");
  PString mixer = mixer_name;

  for (cpt = -1 ; cpt < 10 ; cpt++) {

    if (cpt != -1)
      mixer = mixer_name + PString (cpt);

    mixerfd = open (mixer, O_RDWR);

    if (!(mixerfd == -1)) {
      
      if (ioctl (mixerfd, SOUND_MIXER_READ_RECSRC, &rcsrc) == -1)
        rcsrc = 0;
    
      rcsrc = SOUND_MASK_MIC;                         
      ioctl (mixerfd, SOUND_MIXER_WRITE_RECSRC, &rcsrc);
    
      close (mixerfd);
    }
  }
#else
  return;
#endif
#endif
}


PStringArray gnomemeeting_get_audio_player_devices ()
{
  PStringArray devices;
  PStringArray d;
  int cpt = 0;

#ifndef TRY_PLUGINS  
  devices = PSoundChannel::GetDeviceNames (PSoundChannel::Player);
#else
  //  devices = PDeviceManager::GetSoundDeviceNames (PDeviceManager::Output);
#endif

#ifdef HAS_IXJ
  devices += OpalIxJDevice::GetDeviceNames ();
#endif
  
  while (cpt < devices.GetSize ()) {

    if (strcmp (devices [cpt], "loopback"))
      d.AppendString (devices [cpt]);

    cpt++;
  }

  return d;
}


PStringArray gnomemeeting_get_audio_recorder_devices ()
{
  PStringArray devices;
  PStringArray d;
  int cpt = 0;
  
#ifndef TRY_PLUGINS
  devices = PSoundChannel::GetDeviceNames (PSoundChannel::Recorder);
#else
  //  devices = PDeviceManager::GetSoundDeviceNames (PDeviceManager::Input);
#endif

#ifdef HAS_IXJ
  devices += OpalIxJDevice::GetDeviceNames ();
#endif

  while (cpt < devices.GetSize ()) {

    if (strcmp (devices [cpt], "loopback"))
      d.AppendString (devices [cpt]);

    cpt++;
  }
  
  return d;
}


gint 
gnomemeeting_sound_play_ringtone (GtkWidget *widget)
{
  /* First we check the current displayed image in the systray.
     We can't call gnomemeeting_threads_enter as idles and timers
     are executed in the main thread */
#ifndef DISABLE_GNOME
  gdk_threads_enter ();
  gboolean is_ringing = gnomemeeting_tray_is_ringing (widget);
  gdk_threads_leave ();

  /* If the systray icon contains the ringing pic */
  if (is_ringing) {

    gnome_triggers_do ("", NULL, "gnomemeeting", 
		       "incoming_call", NULL);
  }
#endif

  return TRUE;
}


GMAudioRP::GMAudioRP (GMAudioTester *t, PString dev, BOOL enc)
  :PThread (1000, NoAutoDeleteThread)
{
  is_encoding = enc;
  tester = t;
  device_name = dev;
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
  PSoundChannel *channel = NULL;
  GmWindow *gw = NULL;
  
  gchar *msg = NULL;
  char *buffer = NULL;

  BOOL label_displayed = FALSE;
  int buffer_pos = 0;
  static int nbr_opened_channels = 0;
  
  PTime now;

  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();
  
  gw = GnomeMeeting::Process ()->GetMainWindow ();

  buffer = (char *) malloc (640);
  memset (buffer, '0', sizeof (buffer));

  /* We try to open the selected devices */
  channel = 
    PDeviceManager::GetOpenedSoundDevice (device_name,
					  is_encoding ? PDeviceManager::Input 
					  : PDeviceManager::Output,
					  1, 8000, 16);

  if (!is_encoding)
    msg = g_strdup_printf ("<b>%s</b>", _("Opening device for playing"));
  else
    msg = g_strdup_printf ("<b>%s</b>", _("Opening device for recording"));

  gdk_threads_enter ();
  gtk_label_set_markup (GTK_LABEL (tester->test_label), msg);
  gdk_threads_leave ();
  g_free (msg);

  if (!channel) {

    gdk_threads_enter ();  
    if (is_encoding)
      gnomemeeting_error_dialog (GTK_WINDOW (gw->druid_window), _("Failed to open the device"), _("Impossible to open the selected audio device (%s) for recording. Please check your audio setup, the permissions and that the device is not busy."), (const char *) device);
    else
      gnomemeeting_error_dialog (GTK_WINDOW (gw->druid_window), _("Failed to open the device"), _("Impossible to open the selected audio device (%s) for playing. Please check your audio setup, the permissions and that the device is not busy."), (const char *) device);
    gdk_threads_leave ();  
  }
  else {

    nbr_opened_channels++;
    
    channel->SetBuffers (640, 2);

    while (!stop) {

      if (is_encoding) {

	if (!channel->Read ((void *) buffer, 640)) {
      
	  gdk_threads_enter ();  
	  gnomemeeting_error_dialog (GTK_WINDOW (gw->druid_window), _("Cannot use the audio device"), _("The selected audio device (%s) was successfully opened but it is impossible to read data from this device. Please check your audio setup."), (const char*) device);
	  gdk_threads_leave ();  

	  stop = TRUE;
	}
	else {

	  if (!label_displayed) {

	    msg =  g_strdup_printf ("<b>%s</b>", _("Recording your voice"));

	    gdk_threads_enter ();
	    gtk_label_set_markup (GTK_LABEL (tester->test_label), msg);
	    if (nbr_opened_channels == 2)
	      gtk_widget_show_all (GTK_WIDGET (tester->test_dialog));
	    gdk_threads_leave ();
	    g_free (msg);

	    label_displayed = TRUE;
	  }

	  tester->buffer_ring_access_mutex.Wait ();
	  memcpy (&tester->buffer_ring [buffer_pos], buffer,  640); 
	  tester->buffer_ring_access_mutex.Signal ();

	  buffer_pos += 640;
	}
      }
      else {

	if ((PTime () - now).GetSeconds () > 3) {

	  if (!label_displayed) {

	    msg = g_strdup_printf ("<b>%s</b>", 
				   _("Recording and playing back"));

	    gdk_threads_enter ();
	    gtk_label_set_markup (GTK_LABEL (tester->test_label), msg);
	    if (nbr_opened_channels == 2)
	      gtk_widget_show_all (GTK_WIDGET (tester->test_dialog));
	    gdk_threads_leave ();
	    g_free (msg);

	    label_displayed = TRUE;
	  }

	  tester->buffer_ring_access_mutex.Wait ();
	  memcpy (buffer, &tester->buffer_ring [buffer_pos], 640); 
	  tester->buffer_ring_access_mutex.Signal ();
	
	  buffer_pos += 640;

	  if (!channel->Write ((void *) buffer, 640)) {
      
	    gdk_threads_enter ();  
	    gnomemeeting_error_dialog (GTK_WINDOW (gw->druid_window), _("Cannot use the audio device"), _("The selected audio device (%s) was successfully opened but it is impossible to write data to this device. Please check your audio setup."), (const char*) device);
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

  nbr_opened_channels = PMAX (nbr_opened_channels--, 0);

  free (buffer);
}


/* The Audio tester class */
  GMAudioTester::GMAudioTester (gchar *m,
				gchar *p,
				gchar *r)
  :PThread (1000, NoAutoDeleteThread)
{
#ifndef DISABLE_GNOME
  stop = FALSE;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  test_dialog = NULL;
  test_label = NULL;
  
  if (m)
    audio_manager = PString (m);
  if (p)
    audio_player = PString (p);
  if (r)
    audio_recorder = PString (r);
#endif
  
  this->Resume ();
  thread_sync_point.Wait ();
}


GMAudioTester::~GMAudioTester ()
{
#ifndef DISABLE_GNOME
  stop = 1;
  PWaitAndSignal m(quit_mutex);

  gnomemeeting_threads_enter ();
  if (test_dialog)
    gtk_widget_destroy (test_dialog);
  gnomemeeting_threads_leave ();
#endif
}


void GMAudioTester::Main ()
{
#ifndef DISABLE_GNOME
  GMAudioRP *player = NULL;
  GMAudioRP *recorder = NULL;
  GmDruidWindow *dw = NULL;

  PString device_name;

  gchar *msg = NULL;

  dw = GnomeMeeting::Process ()->GetDruidWindow ();

  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();

  if (audio_manager.IsEmpty ()
      || audio_recorder.IsEmpty ()
      || audio_player.IsEmpty ()
      || audio_recorder == PString (_("No device found"))
      || audio_player == PString (_("No device found")))
    return;
  
  buffer_ring = (char *) malloc (8000 /*Hz*/ * 5 /*s*/ * 2 /*16bits*/);

  memset (buffer_ring, '0', sizeof (buffer_ring));

  gdk_threads_enter ();
  test_dialog =
    gtk_dialog_new_with_buttons ("Audio test running",
				 GTK_WINDOW (gw->druid_window),
				 (enum GtkDialogFlags) (GTK_DIALOG_MODAL),
				 GTK_STOCK_OK,
				 GTK_RESPONSE_ACCEPT,
				 NULL);
  msg = 
    g_strdup_printf (_("GnomeMeeting is now recording from %s and playing back to %s. Please say \"1 2 3\" in your microphone, you should hear yourself back into the speakers with a 4 seconds delay."), (const char *) audio_recorder, (const char *) audio_player);
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

  g_signal_connect (G_OBJECT (test_dialog), "delete-event",
		    G_CALLBACK (gtk_widget_hide_on_delete), NULL);
  g_signal_connect (G_OBJECT (test_dialog), "response",
		    G_CALLBACK (dialog_response_cb), NULL);

  gtk_window_set_transient_for (GTK_WINDOW (test_dialog),
				GTK_WINDOW (gw->druid_window));
  gdk_threads_leave ();

  device_name = audio_manager + " " + audio_recorder;
  recorder = new GMAudioRP (this, device_name, TRUE);

  device_name = audio_manager + " " + audio_player;
  player = new GMAudioRP (this, device_name, FALSE);

  
  while (!stop && !player->IsTerminated () && !recorder->IsTerminated ()) {

    PThread::Current ()->Sleep (100);
  }

  delete (player);
  delete (recorder);

  gdk_threads_enter ();
  GTK_TOGGLE_BUTTON (dw->audio_test_button)->active = FALSE;
  gtk_widget_queue_draw (GTK_WIDGET (dw->audio_test_button));
  gdk_threads_leave ();

  free (buffer_ring);
#endif
}
