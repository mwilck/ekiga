
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
extern GnomeMeeting *MyApp;


/* The GTK callbacks */
void dialog_response_cb (GtkWidget *w, gint, gpointer data)
{
  GmDruidWindow *dw = NULL;

  dw = MyApp->GetDruidWindow ();

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dw->audio_test_button),
				false);

  gtk_widget_destroy (w);
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
  device = dev;
  stop = FALSE;
}


GMAudioRP::~GMAudioRP ()
{
  stop = TRUE;

  PWaitAndSignal m(quit_mutex);
}


void GMAudioRP::Main ()
{
  GConfClient *client = NULL;
  PSoundChannel *channel = NULL;
  GmWindow *gw = NULL;
  gchar *manager = NULL;
  char *buffer = NULL;
  int buffer_pos = 0;

  PString device_name;

  PWaitAndSignal m(quit_mutex);

  gw = MyApp->GetMainWindow ();
  client = gconf_client_get_default ();

#ifndef TRY_PLUGINS
  channel = new PSoundChannel;
#endif
  
  /* Build the real device name for the plugins system */
  gdk_threads_enter ();

  manager =
    gconf_client_get_string (client, DEVICES_KEY "audio_manager", 0);
  if (manager)
    device_name = PString (manager) + " " + device;
  g_free (manager);

  gdk_threads_leave ();

  buffer = (char *) malloc (640);
  memset (buffer, '0', sizeof (buffer));


  /* We try to open the 2 selected devices */
#ifndef TRY_PLUGINS
  if (!channel->Open (device, 
		      is_encoding ? PSoundChannel::Recorder 
		      : PSoundChannel::Player,
		      1, 8000, 16)) {
#else
    
  channel = 
    PDeviceManager::GetOpenedSoundDevice (device_name,
					  is_encoding ? PDeviceManager::Input 
					  : PDeviceManager::Output,
					  1, 8000, 16);
  if (!channel) {
#endif
    gdk_threads_enter ();  
    if (is_encoding)
      gnomemeeting_error_dialog (GTK_WINDOW (gw->druid_window), _("Failed to open the device"), _("Impossible to open the selected audio device (%s) for recording. Please check your audio setup, the permissions and that the device is not busy."), (const char *) device);
    else
      gnomemeeting_error_dialog (GTK_WINDOW (gw->druid_window), _("Failed to open the device"), _("Impossible to open the selected audio device (%s) for playing. Please check your audio setup, the permissions and that the device is not busy."), (const char *) device);
    gdk_threads_leave ();  
  }
  else {

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

	  tester->buffer_ring_access_mutex.Wait ();
	  memcpy (&tester->buffer_ring [buffer_pos], buffer,  640); 
	  tester->buffer_ring_access_mutex.Signal ();

	  buffer_pos += 640;
	}
      }
      else {

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

      if (buffer_pos >= 80000)
	buffer_pos = 0;	
    }
  }


  if (channel) {

    channel->Close ();
    delete (channel);
  }

  free (buffer);
}


/* The Audio tester class */
GMAudioTester::GMAudioTester (GMH323EndPoint *e)
  :PThread (1000, NoAutoDeleteThread)
{
#ifndef DISABLE_GNOME
  ep = e;
  stop = FALSE;
  
  gnomemeeting_sound_daemons_suspend ();
  gnomemeeting_threads_enter ();
  gw = MyApp->GetMainWindow ();
  gnomemeeting_threads_leave ();


#endif

  this->Resume ();
}


GMAudioTester::~GMAudioTester ()
{
#ifndef DISABLE_GNOME
  stop = 1;
  quit_mutex.Wait ();

  gnomemeeting_sound_daemons_resume ();
  quit_mutex.Signal ();
#endif
}


void GMAudioTester::Main ()
{
#ifndef DISABLE_GNOME
  GMAudioRP *player = NULL;
  GMAudioRP *recorder = NULL;

  PWaitAndSignal m(quit_mutex);

  buffer_ring = (char *) malloc (8000 /*Hz*/ * 5 /*s*/ * 2 /*16bits*/);

  memset (buffer_ring, '0', sizeof (buffer_ring));

  player = new GMAudioRP (this, ep->GetSoundChannelPlayDevice (), FALSE);
  recorder = new GMAudioRP (this, ep->GetSoundChannelRecordDevice (), TRUE);

  PTime now;

  recorder->Resume ();


  while (!stop) {

    if ((PTime () - now).GetSeconds () > 3)
      player->Resume ();

    PThread::Current ()->Sleep (100);
  }
  
  delete (player);
  delete (recorder);

  free (buffer_ring);
#endif
}
