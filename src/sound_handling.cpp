
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


extern GtkWidget *gm;


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
GMAudioTester::GMAudioTester (GMH323EndPoint *e, GtkWindow *w)
  :PThread (1000, AutoDeleteThread)
{
  ep = e;
  stop = FALSE;
  window = w;
  
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
  void *buffer = malloc (8 * 1024);
  BOOL displayed = FALSE;

  memset (buffer, 0, sizeof (buffer));

  /* We try to open the 2 selected devices */
  if (!player->Open (ep->GetSoundChannelPlayDevice (), PSoundChannel::Player,
		     1, 8000, 16)) {

    gdk_threads_enter ();
    gnomemeeting_error_dialog (GTK_WINDOW (window), _("Impossible to open the selected audio device (%s) for playing. Please check your audio setup."), (const char *) ep->GetSoundChannelPlayDevice ());
    gdk_threads_leave ();

    stop = TRUE;
  }
  else {
    
    gdk_threads_enter ();
    gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_play), 
			      GetPlayerVolume ());
    gdk_threads_leave ();
  }

  if (!recorder->Open (ep->GetSoundChannelRecordDevice (), 
		       PSoundChannel::Recorder,
		       1, 8000, 16)) {

    gdk_threads_enter ();
    gnomemeeting_error_dialog (GTK_WINDOW (window), _("Impossible to open the selected audio device (%s) for recording. Please check your audio setup."), (const char *) ep->GetSoundChannelRecordDevice ());
    gdk_threads_leave ();

    stop = TRUE;
  }
  else {
    
    gdk_threads_enter ();
    gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_rec), 
			      GetRecorderVolume ());
    gdk_threads_leave ();
  }



  quit_mutex.Wait ();

  while (!stop) {
 
    
    if (!recorder->Read (buffer, 8 * 1024)) {
      
      gdk_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (window), _("The selected audio device (%s) was successfully opened but it is impossible to read data from this device. Please check your audio setup."), (const char*) ep->GetSoundChannelRecordDevice ());
      gdk_threads_leave ();

      stop = TRUE;
    }
    else if (!player->Write (buffer, 8 * 1024)) {

      gdk_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (window), _("The selected audio device (%s) was successfully opened but it is impossible to write data to this device. Please check your audio setup."), (const char*) ep->GetSoundChannelPlayDevice ());
      gdk_threads_leave ();
      
      stop = TRUE;
    }
    else {

      if (!displayed) {

	gdk_threads_enter ();
	gnomemeeting_message_dialog (GTK_WINDOW (window), _("GnomeMeeting is now recording from %s and playing back to %s. Please speak in your microphone, you should hear yourself back into the speakers. Please make sure that what you hear is not the electronic feedback but your real recorded voice. If you don't hear yourself speaking, please fix your audio setup before using GnomeMeeting or others won't hear you. You can use the sliders in the control panel to adjust the volume of the speakers and of the microphone."), (const char*) ep->GetSoundChannelRecordDevice (), (const char*) ep->GetSoundChannelPlayDevice ());

	gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_settings_frame), TRUE);
	gdk_threads_leave ();
      }

      displayed = TRUE;
    }

    memset (buffer, 0, 8 * 1024);
  }
  

  gdk_threads_enter ();
  gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_settings_frame), FALSE);
  gdk_threads_leave ();  

  quit_mutex.Signal ();
}


int GMAudioTester::GetPlayerVolume ()
{
  unsigned int vol = 0;

  if (player)
    player->GetVolume (vol);

  return vol;
}


BOOL GMAudioTester::SetPlayerVolume (int vol)
{
  if (player) {

    player->SetVolume (vol);
    return TRUE;
  }

  return FALSE;
}


int GMAudioTester::GetRecorderVolume ()
{
  unsigned int vol = 0;

  if (recorder)
    recorder->GetVolume (vol);

  return vol;
}


BOOL GMAudioTester::SetRecorderVolume (int vol)
{
  if (recorder) {

    recorder->SetVolume (vol);
    return TRUE;
  }
  
  return FALSE;
}


void GMAudioTester::Stop ()
{
  stop = 1;
}
