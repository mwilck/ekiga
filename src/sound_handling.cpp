
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
#include "gnomemeeting.h"
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
extern GnomeMeeting *MyApp;


/* The functions */

int 
gnomemeeting_volume_set (char *mixer, int source, int *volume)
{
  int res, mixerfd;

#ifdef HAS_IXJ
  OpalLineInterfaceDevice *lid = NULL;

  if (!strcmp (mixer, "/dev/phone0")) 
  {
    unsigned vol = 0;

    if ((MyApp) && (MyApp->Endpoint ()))
	lid = MyApp->Endpoint ()->GetLidDevice ();

    if (source == SOURCE_AUDIO) 
      if (lid)
	lid->SetPlayVolume (0, vol);

    if (source == SOURCE_MIC)
      if (lid)
	lid->SetRecordVolume (0, vol);

    *volume = (int) vol;
  }
  else {
#endif

    mixerfd = open (mixer, O_RDWR); 
      
    if (mixerfd == -1)
      return -1;

    if (source == SOURCE_AUDIO)
      res = ioctl (mixerfd, MIXER_WRITE (SOUND_MIXER_VOLUME), volume);

    if (source == SOURCE_MIC)
      res = ioctl (mixerfd, MIXER_WRITE (SOUND_MIXER_MIC), volume);

    close (mixerfd);

#ifdef HAS_IXJ
  }
#endif

  return 0;
}


int 
gnomemeeting_volume_get (char *mixer, int source, int *volume)
{
  int mixerfd = -1, res = -1;

#ifdef HAS_IXJ
  OpalLineInterfaceDevice *lid = NULL;

  if (!strcmp (mixer, "/dev/phone0")) {

    unsigned vol;
    if ((MyApp) && (MyApp->Endpoint ()))
      lid = MyApp->Endpoint ()->GetLidDevice ();

    if (source == SOURCE_AUDIO) 
      if (lid)
	lid->GetPlayVolume (0, vol);

    if (source == SOURCE_MIC)
      if (lid)
	lid->GetRecordVolume (0, vol);

    *volume = (int) vol;
  }
  else {
#endif

    if (mixer)
      mixerfd = open (mixer, O_RDWR);
      
    if (mixerfd == -1)
      return -1;

    if (source == SOURCE_AUDIO)
      res = ioctl (mixerfd, MIXER_READ (SOUND_MIXER_READ_VOLUME), volume);

    if (source == SOURCE_MIC)
      res = ioctl (mixerfd, MIXER_READ (SOUND_MIXER_MIC), volume);

    close (mixerfd);

#ifdef HAS_IXJ
  }
#endif  

  return TRUE;
}


int 
gnomemeeting_set_recording_source (char *mixer, int source)
{
  int mixerfd = -1;
  int rcsrc;
  
  if (mixer)
    mixerfd = open (mixer, O_RDWR);
      
  if (mixerfd == -1)
      return -1;

  if (ioctl (mixerfd, SOUND_MIXER_READ_RECSRC, &rcsrc))
    rcsrc = 0;

  if (source == SOURCE_AUDIO) {
    rcsrc |= SOUND_MASK_MIC;
    ioctl (mixerfd, SOUND_MIXER_WRITE_RECSRC, &rcsrc);
  }

  close (mixerfd);

  return 0;
}


int 
gnomemeeting_get_mixer_name (char *mixer, char **name)
{

#ifdef __FreeBSD__
  strcpy (*name,"/dev/mixer");
#else

  int mixerfd, res;
  mixer_info info;

  mixerfd = open (mixer, O_RDWR);
      
  if (mixerfd == -1)
      return -1;
  
  res = ioctl(mixerfd, SOUND_MIXER_INFO, &info);
  *name = g_strdup (info.name);


  close (mixerfd);
#endif
  return 0;
}


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

  if (!recorder->Open (ep->GetSoundChannelRecordDevice (), 
		       PSoundChannel::Recorder,
		       1, 8000, 16)) {

    gdk_threads_enter ();
    gnomemeeting_error_dialog (GTK_WINDOW (window), _("Impossible to open the selected audio device (%s) for recording. Please check your audio setup."), (const char *) ep->GetSoundChannelRecordDevice ());
    gdk_threads_leave ();

    stop = TRUE;
  }


  quit_mutex.Wait ();

  while (!stop) {
 
    
    if (!recorder->Read (buffer, 8 * 1024)) {
      
      gdk_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (window), _("Impossible to read data from the selected audio device (%s). Please check your audio setup."), (const char*) ep->GetSoundChannelRecordDevice ());
      gdk_threads_leave ();

      stop = TRUE;
    }
    else if (!player->Write (buffer, 8 * 1024)) {

      gdk_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (window), _("Impossible to write data to the selected audio device (%s). Please check your audio setup."), (const char*) ep->GetSoundChannelPlayDevice ());
      gdk_threads_leave ();
      
      stop = TRUE;
    }
    else {

      if (!displayed) {

	gdk_threads_enter ();
	gnomemeeting_message_dialog (GTK_WINDOW (window), _("GnomeMeeting is now recording from %s and playing back to %s. Please speak in your microphone. You should hear yourself back into the speakers. Please make sure that what you hear is not the electronic feedback but your real recorded voice. If you don't hear yourself speaking, please fix your audio setup before using GnomeMeeting or others won't hear you. Most probably that your driver is not able to do full-duplex."), (const char*) ep->GetSoundChannelRecordDevice (), (const char*) ep->GetSoundChannelPlayDevice ());

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


BOOL GMAudioTester::SetPlayerVolume (int vol)
{
  if (player) {

    player->SetVolume (vol);
    return TRUE;
  }

  return FALSE;
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
