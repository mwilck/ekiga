
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
 *                         lid.cpp  -  description
 *                         -----------------------
 *   begin                : Sun Dec 1 2002
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains LID functions.
 *
 */


#include "../config.h"

#include "lid.h"
#include "gnomemeeting.h"
#include "urlhandler.h"
#include "misc.h"
#include "sound_handling.h"
#include "main_window.h"
#include "callbacks.h"

#include "dialog.h"
#include "gconf_widgets_extensions.h"


static gboolean transfer_callback_cb (gpointer data);


/* Declarations */
extern GtkWidget *gm;


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Hack to be able to call the transfer_call_cb in as a g_idle
 *                 from a thread.
 * PRE          :  /
 */
static gboolean
transfer_callback_cb (gpointer data)
{
  gdk_threads_enter ();
  transfer_call_cb (NULL, NULL);
  gdk_threads_leave ();

  return false;
}


#ifdef HAS_IXJ
GMLid::GMLid ()
  :PThread (1000, NoAutoDeleteThread)
{
  stop = 0;
  lid = NULL;

  /* Open the device */
  Open ();
  this->Resume ();
  thread_sync_point.Wait ();
} 


GMLid::~GMLid ()
{
  stop = 1;
  PWaitAndSignal m(quit_mutex);

  Close ();
}


void GMLid::Open ()
{  
  gchar *lid_device = NULL;
  gchar *lid_country = NULL;
  int lid_aec = 0;
  
  GmWindow *gw = NULL;
  GConfClient *client = NULL;

  PWaitAndSignal m(device_access_mutex);
  
  gnomemeeting_threads_enter ();
  client = gconf_client_get_default ();
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  gnomemeeting_threads_leave ();

  if (!lid) {

    gnomemeeting_threads_enter ();
    lid_device = gconf_get_string (AUDIO_DEVICES_KEY "input_device");
    lid_country = gconf_get_string (AUDIO_DEVICES_KEY "lid_country_code");
    lid_aec = gconf_get_int (AUDIO_DEVICES_KEY "lid_echo_cancellation_level");
       gnomemeeting_threads_leave ();

    if (lid_device == NULL)
      lid_device = g_strdup ("/dev/phone0");

    lid = new OpalIxJDevice;
    if (lid->Open (lid_device)) {
      
      gchar *msg = NULL;
      msg = g_strdup_printf (_("Using Quicknet device %s"), 
			     (const char *) lid->GetName ());
      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (gw->history_text_view, msg);
      gnomemeeting_threads_leave ();
      g_free (msg);
      
      lid->SetLineToLineDirect(0, 1, FALSE);
      
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

      lid->StopTone (0);
      lid->SetLineToLineDirect (0, 1, FALSE);

    }
    else {
      
      gnomemeeting_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Error while opening the Quicknet device."), _("Please check that your driver is correctly installed and that the device is working correctly."));
      gnomemeeting_threads_leave ();
    }
  }
}


void GMLid::Stop ()
{
  stop = 1;
}


void GMLid::Close ()
{
  PWaitAndSignal m(device_access_mutex);
  
  if (lid)
    lid->Close ();
}


void GMLid::Main ()
{
  BOOL OffHook, lastOffHook;
  BOOL do_not_connect = TRUE;

  GMH323EndPoint *endpoint = NULL;
  GmWindow *gw = NULL;

  char old_c = 0;
  
  const char *url = NULL;
  PTime now, last_key_press;

  int calling_state = 0;
  unsigned int vol = 0;

  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();
  cout << "ici" << endl <<flush;
  /* Check the initial hook status. */
  OffHook = lastOffHook = lid->IsLineOffHook (OpalIxJDevice::POTSLine);

  gnomemeeting_threads_enter ();
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  int lid_odt = gconf_get_int (AUDIO_DEVICES_KEY "lid_output_device_type");
  
  /* Update the mixers if the lid is used */
  lid->GetPlayVolume (0, vol);
  GTK_ADJUSTMENT (gw->adj_play)->value = (int) (vol);
  lid->GetRecordVolume (0, vol);
  GTK_ADJUSTMENT (gw->adj_rec)->value = (int) (vol);
  gtk_widget_queue_draw (GTK_WIDGET (gw->audio_settings_frame));
  gnomemeeting_threads_leave ();


  while (lid && lid->IsOpen() && !stop) {

    endpoint = GnomeMeeting::Process ()->Endpoint ();
    calling_state = endpoint->GetCallingState ();

    OffHook = (lid->IsLineOffHook (0));
    now = PTime ();

    char c = lid->ReadDTMF (0);
    if (c) {

      gnomemeeting_threads_enter ();
      gnomemeeting_dialpad_event (c);
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

      if (calling_state == 3)  /* 3 = incoming call */
	GnomeMeeting::Process ()->Connect ();



      if (calling_state == 0) { /* not connected */

	if (lid_odt == 0) { // POTS

	  lid->PlayTone (0, OpalLineInterfaceDevice::DialTone);
	  lid->EnableAudio (0, TRUE);
	}
	else
	  lid->EnableAudio (0, FALSE);

	gnomemeeting_threads_enter ();
	url = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry)); 
	gnomemeeting_threads_leave ();
	
	if (url && !GMURL (url).IsEmpty ())
	  GnomeMeeting::Process ()->Connect ();
      }
    }

    
    /* if phone is on hook */
    if ((OffHook == FALSE) && (lastOffHook == TRUE)) {

      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (gw->history_text_view, _("Phone is on hook"));
      gnomemeeting_statusbar_flash (gw->statusbar, _("Phone is on hook"));

      lid->RingLine (0, 0);
      
      /* Remove the current called number */
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry), 
			  GMURL ().GetDefaultURL ());
      gnomemeeting_threads_leave ();

      if (calling_state == 2 || calling_state == 1) {

	GnomeMeeting::Process ()->Disconnect ();
      }
    }


    if (OffHook == TRUE) {

      PTimeInterval t = now - last_key_press;
      
      if ((t.GetSeconds () > 5 && !do_not_connect) || c == '#') {

	if (calling_state == 0)
	  GnomeMeeting::Process ()->Connect ();
	do_not_connect = TRUE;
      }

      if (old_c == '*' && c == '1' && calling_state == 2) {

	gnomemeeting_threads_enter ();
	hold_call_cb (NULL, NULL);
	gnomemeeting_threads_leave ();
      }


      if (old_c == '*' && c == '2' && calling_state == 2) {

	gnomemeeting_threads_enter ();
	g_idle_add (transfer_callback_cb, NULL);
	gnomemeeting_threads_leave ();
      }
    }


    lastOffHook = OffHook;
    if (c)
      old_c = c;
    
    /* We must poll to read the hook state */
    PThread::Sleep (50);
  }
}


void
GMLid::UpdateState (GMH323EndPoint::CallingState i)
{
  int lid_odt = 0;
  
  lid_odt = gconf_get_int (AUDIO_DEVICES_KEY "lid_output_device_type");
  
  if (lid && lid->IsOpen ()) {
    
    switch (i) {

    case GMH323EndPoint::Calling:
      lid->RingLine (0, 0);
      lid->StopTone (0);
      lid->PlayTone (0, OpalLineInterfaceDevice::RingTone);
      
      break;

    case GMH323EndPoint::Called: 

      if (lid_odt == 0)
	lid->RingLine (OpalIxJDevice::POTSLine, 0x33);
      else
	lid->RingLine (0, 0);
      break;

    case GMH323EndPoint::Standby: /* Busy */
      lid->RingLine (0, 0);
      lid->StopTone (0);
      lid->PlayTone (0, OpalLineInterfaceDevice::BusyTone);
      if (lid_odt == 1) 
	PThread::Current ()->Sleep (2800);

      lid->StopTone (0);
      
      break;

    case GMH323EndPoint::Connected:

      lid->RingLine (0, 0);
      lid->StopTone (0);
      lid->SetRemoveDTMF (0, TRUE);      
    }

    if (lid_odt == 0) // POTS
      lid->EnableAudio (0, TRUE);
    else 
      lid->EnableAudio (0, FALSE);
  }
}


void
GMLid::SetAEC (unsigned int l, OpalLineInterfaceDevice::AECLevels level)
{
  if (lid && lid->IsOpen ())
    lid->SetAEC (l, level);
}


void
GMLid::SetCountryCodeName (const PString & d)
{
  if (lid && lid->IsOpen ())
    lid->SetCountryCodeName (d);
}


void
GMLid::SetVolume (int x, int y)
{
  if (lid && lid->IsOpen ()) {

    lid->SetPlayVolume (0, x);
    lid->SetRecordVolume (0, y);
  }
}


OpalLineInterfaceDevice *
GMLid::GetLidDevice ()
{
  return lid;
}


BOOL
GMLid::areSoftwareCodecsSupported ()
{
  if (lid && lid->IsOpen ())
    return (lid->GetMediaFormats ().GetValuesIndex (OpalMediaFormat(OPAL_PCM16)) 
	    != P_MAX_INDEX);
  else
    return TRUE;
}


void
GMLid::Lock ()
{
  device_access_mutex.Wait ();
}


void
GMLid::Unlock ()
{
  device_access_mutex.Signal ();
}
#endif

