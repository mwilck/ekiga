 
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2002 Damien Sandras
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
 */

/*
 *                         lid.cpp  -  description
 *                         ----------------------------
 *   begin                : Sun Dec 1 2002
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains LID functions.
 *   email                : dsandras@seconix.com
 *
 */

#include "../config.h"

#include "lid.h"
#include "endpoint.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "common.h"
#include "main_window.h"
#include "dialog.h"
#include "sound_handling.h"

#ifndef DISABLE_GNOME
#include <gnome.h>
#endif

#define new PNEW


/* Declarations */
extern GtkWidget *gm;
extern GnomeMeeting *MyApp;


#ifdef HAS_IXJ
GMLid::GMLid ()
  :PThread (1000, NoAutoDeleteThread)
{
  stop = 0;
  lid = NULL;

  /* Open the device */
  Open ();
  this->Resume ();
} 


GMLid::~GMLid ()
{
  stop = 1;
  Close ();
  quit_mutex.Wait ();
  quit_mutex.Signal ();
}


void GMLid::Open ()
{  
  gchar *lid_device = NULL;
  gchar *lid_country = NULL;
  int lid_aec = 0;

  GmWindow *gw = NULL;
  GConfClient *client = gconf_client_get_default ();
  
  gnomemeeting_threads_enter ();
  gw = gnomemeeting_get_main_window (gm);
  gnomemeeting_threads_leave ();

  if (!lid) {

    lid_device =  
      gconf_client_get_string (client, DEVICES_KEY "lid_device", NULL);
    lid_country =
      gconf_client_get_string (client, DEVICES_KEY "lid_country", NULL);
    lid_aec =
      gconf_client_get_int (client, DEVICES_KEY "lid_aec", NULL);
    
    if (lid_device == NULL)
      lid_device = g_strdup ("/dev/phone0");
    
    lid = new OpalIxJDevice;
    if (lid->Open ("/dev/phone0")) {
      
      gchar *msg = NULL;
      msg = g_strdup_printf (_("Using Quicknet device %s"), 
			     (const char *) lid->GetName ());
      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (gw->history_text_view, msg);
      gnomemeeting_threads_leave ();
      g_free (msg);
      
      lid->SetLineToLineDirect(0, 1, FALSE);
      lid->EnableAudio(0, TRUE); 
      
      if (lid_country)
	lid->SetCountryCodeName(lid_country);
      
      switch (lid_aec) {
	
      case 0:
	lid->SetAEC (0, OpalLineInterfaceDevice::AECOff);
	break;

      case 1:
	lid->SetAEC (0, OpalLineInterfaceDevice::AECLow);
	break;
	
      case 2:
	lid->SetAEC (0, OpalLineInterfaceDevice::AECMedium);
	break;
	
      case 3:
	lid->SetAEC (0, OpalLineInterfaceDevice::AECHigh);
	break;
	
      case 5:
	lid->SetAEC (0, OpalLineInterfaceDevice::AECAGC);
	break;
      }
      
    }
    else {
      
      gconf_client_set_bool (client, DEVICES_KEY "lid", 0, 0);
      gnomemeeting_threads_enter ();
      gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (gm), gw->speaker_phone_button, _("Error while opening the Quicknet device. Disabling Quicknet device."));
      gnomemeeting_threads_leave ();
    }
  }
}


OpalLineInterfaceDevice *GMLid::GetLidDevice ()
{
  return lid;
}


void GMLid::Stop ()
{
  stop = 1;
}


void GMLid::Close ()
{
  GConfClient *client = NULL;
  GmWindow *gw = NULL;
  gchar *mixer = NULL;
  int vol = 0;
  
  if (lid)
    lid->Close ();

  client = gconf_client_get_default ();

  /* Restore the normal mixers settings */
  gnomemeeting_threads_enter ();
  gw = gnomemeeting_get_main_window (gm);

  mixer =
    gconf_client_get_string (client, DEVICES_KEY "audio_player_mixer", NULL);
  vol = gnomemeeting_get_mixer_volume (mixer, SOURCE_AUDIO);
  g_free (mixer);
  GTK_ADJUSTMENT (gw->adj_play)->value = (int) (vol & 255);
  
  mixer =
    gconf_client_get_string (client, DEVICES_KEY "audio_recorder_mixer", NULL);
  vol = gnomemeeting_get_mixer_volume (mixer, SOURCE_MIC);
  g_free (mixer);
  GTK_ADJUSTMENT (gw->adj_rec)->value = (int) (vol & 255);
  gtk_widget_queue_draw (GTK_WIDGET (gw->audio_settings_frame));

  gnomemeeting_threads_leave ();
}


void GMLid::Main ()
{
  BOOL OffHook, lastOffHook;
  BOOL do_not_connect = TRUE;

  GMH323EndPoint *endpoint = NULL;
  GmWindow *gw = NULL;

  PTime now, last_key_press;

  int calling_state = 0;
  unsigned int vol = 0;

  quit_mutex.Wait ();

  /* Check the initial hook status. */
  OffHook = lastOffHook = lid->IsLineOffHook (OpalIxJDevice::POTSLine);

  gnomemeeting_threads_enter ();
  gw = gnomemeeting_get_main_window (gm);

  /* Update the mixers if the lid is used */
  lid->GetPlayVolume (0, vol);
  GTK_ADJUSTMENT (gw->adj_play)->value = (int) (vol);
  lid->GetRecordVolume (0, vol);
  GTK_ADJUSTMENT (gw->adj_rec)->value = (int) (vol);
  gtk_widget_queue_draw (GTK_WIDGET (gw->audio_settings_frame));
  gnomemeeting_threads_leave ();

  /* OffHook can take a few cycles to settle, so on the first pass */
  /* assume we are off-hook and play a dial tone. */
  if (lid)
    lid->PlayTone (0, OpalLineInterfaceDevice::DialTone);

  while (lid != NULL && lid->IsOpen() && !stop)
  {
    endpoint = MyApp->Endpoint ();

    calling_state = endpoint->GetCallingState ();

    OffHook = (lid->IsLineOffHook (0));
    now = PTime ();

    char c = lid->ReadDTMF (0);
    if (c) {

      gnomemeeting_threads_enter ();
      gnomemeeting_dialpad_event (PString (c));
      gnomemeeting_threads_leave ();

      last_key_press = PTime ();
      do_not_connect = FALSE;
    }


    /* If there is a state change */
    if ((OffHook == TRUE) && (lastOffHook == FALSE)) {

      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (gw->history_text_view, _("Phone is off hook"));
      gnomemeeting_statusbar_flash (gw->statusbar, _("Phone is off hook"));
      gnomemeeting_threads_leave ();

      if (calling_state == 3) { /* 3 = incoming call */

	lid->StopTone (0);
	
	MyApp->Connect ();
      }


      if (calling_state == 0) { /* not connected */

        lid->PlayTone (0, OpalLineInterfaceDevice::DialTone);
      }
    }

    
    /* if phone is on hook */
    if ((OffHook == FALSE) && (lastOffHook == TRUE)) {

      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (gw->history_text_view, _("Phone is on hook"));
      gnomemeeting_statusbar_flash (gw->statusbar, _("Phone is on hook"));

      /* Remove the current called number */
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry), 
			  "callto://");
      gnomemeeting_threads_leave ();

      if (calling_state == 2 || calling_state == 1) {

	MyApp->Disconnect ();
      }
    }


    if (OffHook == TRUE) {

      PTimeInterval t = now - last_key_press;
      
      if (t.GetSeconds () > 5 && !do_not_connect) {

	if (calling_state == 0)
	  MyApp->Connect ();
	do_not_connect = TRUE;
      }
    }


    lastOffHook = OffHook;

    /* We must poll to read the hook state */
    PThread::Sleep(200);
  }

  quit_mutex.Signal ();
}
#endif

