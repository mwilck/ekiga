
/*  sound_handling.cpp
 *
 *  GnomeMeeting -- A Video-Conferencing application
 *  Copyright (C) 2000-2002 Damien Sandras
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
/*
 *                         sound_handling.cpp  -  description
 *                         ----------------------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains sound handling functions.
 *   email                : dsandras@seconix.com
 *
 */


#include "sound_handling.h"
#include "common.h"
#include "endpoint.h"
#include "misc.h"
#include "tray.h"
#include "dialog.h"

#include <ptlib.h>

#ifdef __FreeBSD__
#include <sys/types.h>
#include <signal.h>
#endif

#ifdef HAS_IXJ
#include <ixjlid.h>
#endif

#ifndef DISABLE_GNOME
#include <gnome.h>
#endif

static void dialog_response_cb (GtkWidget *, gint, gpointer);

extern GtkWidget *gm;


/* Callbacks */
void dialog_response_cb (GtkWidget *w, gint, gpointer data)
{
  GmDruidWindow *dw = NULL;

  dw = gnomemeeting_get_druid_window (gm);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dw->audio_test_button),
				false);

  gtk_widget_destroy (w);
}


/* The functions */
void 
gnomemeeting_sound_daemons_suspend (void)
{
  int esd_client = 0;
  
  /* Put ESD into standby mode */
  esd_client = esd_open_sound (NULL);

  if (esd_standby (esd_client) != 1) 
    PTRACE (0, "Failed to resume ESD");
      
  esd_close (esd_client);
}


void 
gnomemeeting_sound_daemons_resume (void)
{
  int esd_client = 0;

  /* Put ESD into normal mode */
  esd_client = esd_open_sound (NULL);

  if (esd_resume (esd_client) != 1) 
    PTRACE (0, "Failed to resume ESD");

  esd_close (esd_client);
}


void 
gnomemeeting_mixers_mic_select (void)
{
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
}


int gnomemeeting_get_mixer_volume (char *mixer, int source)
{
  int vol = 0;
  int mixerfd = -1;
  
  if (!mixer)
    return 0;

  mixerfd = open (mixer, O_RDWR);

  if (mixerfd == -1)
    return 0;

  if (source == SOURCE_AUDIO)
    ioctl (mixerfd, MIXER_READ (SOUND_MIXER_READ_VOLUME), &vol);

  if (source == SOURCE_MIC)
    ioctl (mixerfd, MIXER_READ (SOUND_MIXER_MIC), &vol);

  close (mixerfd);
  return vol;
}


void gnomemeeting_set_mixer_volume (char *mixer, int source, int vol)
{
  int mixerfd = -1;
  
  if (!mixer)
    return;

  mixerfd = open (mixer, O_RDWR);

  if (mixerfd == -1)
    return;

  if (source == SOURCE_AUDIO)
    ioctl (mixerfd, MIXER_WRITE (SOUND_MIXER_VOLUME), &vol);

  if (source == SOURCE_MIC)
    ioctl (mixerfd, MIXER_WRITE (SOUND_MIXER_MIC), &vol);
  
  close (mixerfd);
}


PStringArray gnomemeeting_get_audio_player_devices ()
{
  PStringArray devices;
  PStringArray d;
  int cpt = 0;
  
  devices = PSoundChannel::GetDeviceNames (PSoundChannel::Player);

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
  
  devices = PSoundChannel::GetDeviceNames (PSoundChannel::Recorder);

  while (cpt < devices.GetSize ()) {

    if (strcmp (devices [cpt], "loopback"))
      d.AppendString (devices [cpt]);

    cpt++;
  }
  
  return d;
}


PStringArray gnomemeeting_get_mixers ()
{
  int mixerfd = -1;
  int cpt = -1;
  int i = 0;
  
  PStringArray mixers;

  PString mixer_name;
  PString mixer;

  while (i < 2) {

    if (i == 0)
      mixer_name = PString ("/dev/sound/mixer");
    else
      mixer_name = PString ("/dev/mixer");
    
    mixer = mixer_name;

    for (cpt = -1 ; cpt < 10 ; cpt++) {

      if (cpt != -1)
	mixer = mixer_name + PString (cpt);

      mixerfd = open (mixer, O_RDWR);

      if (!(mixerfd == -1)) {

	mixers += mixer;

	close (mixerfd);
      }
    }

    i++;
  }

  return mixers;
}


gint 
gnomemeeting_sound_play_ringtone (GtkWidget *widget)
{
  /* First we check the current displayed image in the systray.
     We can't call gnomemeeting_threads_enter as idles and timers
     are executed in the main thread */
#ifndef DISABLE_GNOME
  gdk_threads_enter ();
  gboolean is_ringing = gnomemeeting_tray_is_ringing (G_OBJECT (widget));
  gdk_threads_leave ();

  /* If the systray icon contains the ringing pic */
  if (is_ringing) {

    gnome_triggers_do ("", NULL, "gnomemeeting", 
		       "incoming_call", NULL);
  }
#endif

  return TRUE;
}


/* The Audio tester class */
GMAudioTester::GMAudioTester (GMH323EndPoint *e)
  :PThread (1000, AutoDeleteThread)
{
  ep = e;
  stop = FALSE;
  
  gnomemeeting_sound_daemons_suspend ();
  gnomemeeting_threads_enter ();
  gw = gnomemeeting_get_main_window (gm);
  gnomemeeting_threads_leave ();

  player = new PSoundChannel;
  recorder = new PSoundChannel;

  this->Resume ();
}


GMAudioTester::~GMAudioTester ()
{
  stop = 1;
  quit_mutex.Wait ();

  player->Close ();
  recorder->Close ();
  
  delete (player);
  delete (recorder);

  gnomemeeting_sound_daemons_resume ();
  quit_mutex.Signal ();
}


void GMAudioTester::Main ()
{
  GConfClient *client = NULL;

  GtkWidget *dialog = NULL;
  GtkWidget *label = NULL;

  gchar *msg = NULL;
  char *buffer_play = (char *) malloc (8 * 1024);
  char *buffer_record = (char *) malloc (8 * 1024);
  char *buffer_ring = (char *) malloc (8 * 5 * 1024);
  char *mixer = NULL;
  
  int buffer_play_pos = 0;
  int buffer_rec_pos = 0;
  int clock = 0;

  bool displayed = false;
  bool label_displayed = false;

  PTime now;
  
  client = gconf_client_get_default ();
  
  memset (buffer_ring, 0, sizeof (buffer_ring));
  memset (buffer_play, 0, sizeof (buffer_play));
  memset (buffer_record, 0, sizeof (buffer_record));

  mixer =
    gconf_client_get_string (client, DEVICES_KEY "audio_recorder_mixer", NULL);
  gnomemeeting_set_mixer_volume (mixer, SOURCE_MIC, 100);
  g_free (mixer);
  
  /* We try to open the 2 selected devices */
  if (!player->Open (ep->GetSoundChannelPlayDevice (), PSoundChannel::Player,
		     1, 8000, 16)) {

    gdk_threads_enter ();
    gnomemeeting_error_dialog (GTK_WINDOW (gw->druid_window), _("Impossible to open the selected audio device (%s) for playing. Please check your audio setup."), (const char *) ep->GetSoundChannelPlayDevice ());
    gdk_threads_leave ();

    stop = TRUE;
  }


  if (!recorder->Open (ep->GetSoundChannelRecordDevice (), 
		       PSoundChannel::Recorder,
		       1, 8000, 16)) {

    gdk_threads_enter ();
    gnomemeeting_error_dialog (GTK_WINDOW (gw->druid_window), _("Impossible to open the selected audio device (%s) for recording. Please check your audio setup."), (const char *) ep->GetSoundChannelRecordDevice ());
    gdk_threads_leave ();

    stop = TRUE;
  }

  quit_mutex.Wait ();

  while (!stop) {
 
    
    if (!recorder->Read ((void *) buffer_record, 8 * 1024)) {
      
      gdk_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (gw->druid_window), _("The selected audio device (%s) was successfully opened but it is impossible to read data from this device. Please check your audio setup."), (const char*) ep->GetSoundChannelRecordDevice ());
      gdk_threads_leave ();

      stop = TRUE;
    }
    else if (clock > 5
	     && !player->Write ((void *) buffer_play, 8 * 1024)) {

      gdk_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (gw->druid_window), _("The selected audio device (%s) was successfully opened but it is impossible to write data to this device. Please check your audio setup."), (const char*) ep->GetSoundChannelPlayDevice ());
      gdk_threads_leave ();

      stop = TRUE;
    }
    else {

      if (!displayed) {

	gdk_threads_enter ();

	dialog =
	  gtk_dialog_new_with_buttons ("Audio test running...",
				       GTK_WINDOW (gw->druid_window),
				       (enum GtkDialogFlags) (GTK_DIALOG_MODAL
				       | GTK_DIALOG_DESTROY_WITH_PARENT),
				       GTK_STOCK_OK,
				       GTK_RESPONSE_ACCEPT,
				       NULL);
	msg = g_strdup_printf (_("GnomeMeeting is now recording from %s and playing back to %s. Please say \"1 2 3\" in your microphone, you should hear yourself back into the speakers in 5 seconds.\n\nRecording... Please talk."), (const char*) ep->GetSoundChannelRecordDevice (), (const char*) ep->GetSoundChannelPlayDevice ());
	label = gtk_label_new (msg);
	gtk_label_set_line_wrap (GTK_LABEL (label), true);
	g_free (msg);
	
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), label,
			    FALSE, FALSE, 0);
	
	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (dialog_response_cb), NULL);
	gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
					   GTK_RESPONSE_ACCEPT, false);
	gtk_window_set_transient_for (GTK_WINDOW (dialog),
				      GTK_WINDOW (gw->druid_window));
	gtk_widget_show_all (dialog);
	gdk_threads_leave ();
      }

      displayed = TRUE;
    }

    if (clock >= 5) {
      
      buffer_play_pos += 8 * 1024;

      if (label && !label_displayed && clock) {

	gnomemeeting_threads_enter ();
	gtk_label_set_text (GTK_LABEL (label), _("GnomeMeeting is now playing what it is recording with a 5 seconds delay. If you don't hear yourself with the delay, you will have to fix your audio setup and probably install a full-duplex driver before calling other GnomeMeeting users.\n\nRecording and playing... Please talk."));
	gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
					   GTK_RESPONSE_ACCEPT, true);
	gnomemeeting_threads_leave ();

	label_displayed = true;
      }
    }

    if (buffer_play_pos >= 8 * 5 * 1024)
      buffer_play_pos = 0;
    if (buffer_rec_pos >= 8 * 5 * 1024)
      buffer_rec_pos = 0;

    memcpy (&buffer_ring [buffer_rec_pos], buffer_record, 8 * 1024);
    memcpy (buffer_play, &buffer_ring [buffer_play_pos], 8 * 1024);
    buffer_rec_pos += 8 * 1024;

    clock = (PTime () - now).GetSeconds ();
  }
  

  free (buffer_ring);
  free (buffer_record);
  free (buffer_play);
  
  quit_mutex.Signal ();
}


void GMAudioTester::Stop ()
{
  stop = 1;
}
