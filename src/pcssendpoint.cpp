
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
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
 *                         pcssendpoint.cpp  -  description
 *                         --------------------------------
 *   begin                : Sun Oct 24 2004
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains the PCSS EndPoint class.
 *
 */


#include "../config.h"
#include "common.h"

#include <lib/gm_conf.h>
#include <lib/dialog.h>

#include "pcssendpoint.h"
#include "endpoint.h"
#include "gnomemeeting.h"
#include "urlhandler.h"

#include "misc.h"
#include "sound_handling.h"
#include "main_window.h"
#include "calls_history_window.h"
#include "log_window.h"
#include "tray.h"


#define new PNEW


GMPCSSEndPoint::GMPCSSEndPoint (GMEndPoint & ep) 
	: OpalPCSSEndPoint (ep), endpoint (ep)
{
  NoAnswerTimer.SetNotifier (PCREATE_NOTIFIER (OnNoAnswerTimeout));
  CallPendingTimer.SetNotifier (PCREATE_NOTIFIER (OnCallPending));
}


void GMPCSSEndPoint::AcceptCurrentIncomingCall ()
{
  if (!incomingConnectionToken.IsEmpty ()) {
    
    AcceptIncomingConnection (incomingConnectionToken);
    incomingConnectionToken = PString ();
  }
}


void GMPCSSEndPoint::OnShowIncoming (const OpalPCSSConnection & connection)
{
  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;
  GtkWidget *tray = NULL;
  
  char *msg = NULL;
  
  PString gateway;
  PString forward_host;

  int no_answer_timeout = 45;
  gchar *utf8_name = NULL;
  gchar *utf8_app = NULL;
  gchar *utf8_url = NULL;
  
  gchar *forward_host_conf = NULL;
  gchar *gateway_conf = NULL;

  IncomingCallMode icm = AVAILABLE;
  
  BOOL busy_forward = FALSE;
  BOOL show_popup = FALSE;
  BOOL do_forward = FALSE;
  BOOL do_reject = FALSE;
  BOOL do_answer = FALSE;
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  tray = GnomeMeeting::Process ()->GetTray ();


  /* Check the config keys */
  gnomemeeting_threads_enter ();
  forward_host_conf = gm_conf_get_string (CALL_FORWARDING_KEY "forward_host");
  busy_forward = gm_conf_get_bool (CALL_FORWARDING_KEY "forward_on_busy");
  icm =
    (IncomingCallMode) gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode");
  show_popup = gm_conf_get_bool (USER_INTERFACE_KEY "show_popup");
  no_answer_timeout = gm_conf_get_int (CALL_OPTIONS_KEY "no_answer_timeout");
  gateway_conf = gm_conf_get_string (H323_KEY "default_gateway");
  gnomemeeting_threads_leave ();


  /* The token identifying the current call */
  incomingConnectionToken = connection.GetToken ();
  endpoint.SetCurrentCallToken (connection.GetCall ().GetToken ());


  if (forward_host_conf)
    forward_host = PString (GMURL (forward_host_conf).GetValidURL ());
  else
    forward_host = PString ("");
    
  gateway = PString (gateway_conf);

  
  /* Get remote Name and application */
  endpoint.GetRemoteConnectionInfo ((OpalConnection &) connection,
				    utf8_name, 
				    utf8_app,
				    utf8_url); 

  
  /* Update the log and status bar */
  msg = g_strdup_printf (_("Call from %s"), (const char *) utf8_name);
  gnomemeeting_threads_enter ();
  gm_main_window_flash_message (main_window, msg);
  gm_history_window_insert (history_window, msg);
  gnomemeeting_threads_leave ();
  g_free (msg);


  /* Check what action to take */
  if (!GMURL(forward_host).IsEmpty() && icm == FORWARD) {

    msg = 
      g_strdup_printf (_("Forwarding call from %s to %s (Forward all calls)"),
		       (const char *) utf8_name, (const char *) forward_host);
    //FIXME
    //do_forward = TRUE;
  }
  else if (icm == DO_NOT_DISTURB) {

    msg =
      g_strdup_printf (_("Rejecting call from %s (Do Not Disturb)"),
		       (const char *) utf8_name);
    
    do_reject = TRUE;
  }
  /* if we are already in a call: forward or reject */
  else if (endpoint.GetCallingState () != GMEndPoint::Standby) {

    /* if we have enabled forward when busy, do the forward */
    if (!forward_host.IsEmpty() && busy_forward) {

      msg = 
	g_strdup_printf (_("Forwarding call from %s to %s (Busy)"),
			 (const char *) utf8_name, 
			 (const char *) forward_host);

      //FIXME
     // do_forward = TRUE;
    } 
    else {

      /* there is no forwarding, so reject the call */
      msg = g_strdup_printf (_("Rejecting call from %s (Busy)"),
			     (const char *) utf8_name);
     
      do_reject = TRUE;
    }
  }
  else if (icm == AUTO_ANSWER) {

    msg =
      g_strdup_printf (_("Accepting call from %s (Auto Answer)"),
		       (const char *) utf8_name);
    
    do_answer = TRUE;
  }


  /* Take that action */
  if (do_reject || do_forward || do_answer) {

    /* Add the full message in the log */
    gnomemeeting_threads_enter ();
    gm_history_window_insert (history_window, msg);
    gnomemeeting_threads_leave ();

    /* Free things, we will return */
    g_free (gateway_conf);
    g_free (forward_host_conf);
    g_free (utf8_name);
    g_free (utf8_app);
    g_free (msg);

    if (do_reject) {
      
      gnomemeeting_threads_enter ();
      gm_main_window_flash_message (main_window, _("Call rejected"));
      gnomemeeting_threads_leave ();

      /* Rejects the current call */
      ClearCall (connection.GetCall ().GetToken (), 
		 H323Connection::EndedByLocalBusy); 
    }
    else if (do_forward) {

      gnomemeeting_threads_enter ();
      gm_main_window_flash_message (main_window, _("Call forwarded"));
      gnomemeeting_threads_leave ();

      if (!gateway.IsEmpty ()
	  && forward_host.Find (gateway) == P_MAX_INDEX) 	
        forward_host = forward_host + "@" + gateway;

      //FIXME connection.ForwardCall (forward_host);
    }
    else if (do_answer) {

      gnomemeeting_threads_enter ();
      gm_main_window_flash_message (main_window,
				    _("Call automatically answered"));
      gnomemeeting_threads_leave ();

      AcceptIncomingConnection (connection.GetToken ());
    }

    return;
  }
   

  /* If we are here, the call doesn't need to be rejected, forwarded
     or automatically answered */
  gnomemeeting_threads_enter ();
  gm_tray_update_calling_state (tray, GMEndPoint::Called);
  gm_main_window_update_calling_state (main_window, GMEndPoint::Called);
  gnomemeeting_threads_leave ();

    
  /* The timers */
  NoAnswerTimer.SetInterval (0, PMAX (no_answer_timeout, 10));
  CallPendingTimer.RunContinuous (PTimeInterval (5));

  
  /* If no forward or reject, update the internal state */
  endpoint.SetCallingState (GMEndPoint::Called);


  /* Incoming Call Popup, if needed */
  if (show_popup) {
    
    gnomemeeting_threads_enter ();
    gm_main_window_incoming_call_dialog_show (main_window,
					      utf8_name, 
					      utf8_app, 
					      utf8_url);
    gnomemeeting_threads_leave ();
  }
  

  g_free (gateway_conf);
  g_free (forward_host_conf);
  g_free (utf8_name);
  g_free (utf8_app);
  g_free (utf8_url);
}


BOOL GMPCSSEndPoint::OnShowOutgoing (const OpalPCSSConnection & connection)
{
  return TRUE;
}


void 
GMPCSSEndPoint::PlaySoundEvent (PString ev)
{
  PWaitAndSignal m(sound_event_mutex);

  GMSoundEvent c (ev);
}


PSoundChannel * 
GMPCSSEndPoint::CreateSoundChannel (const OpalPCSSConnection & connection,
				    BOOL is_source)
{
  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;

  PSoundChannel *sound_channel = NULL;

  PString plugin;
  PString device;

  unsigned int play_vol = 0;
  unsigned int record_vol = 0;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();


  /* Stop the OnNoAnswerTimers */
  NoAnswerTimer.Stop ();
  CallPendingTimer.Stop ();


  /* Suspend the daemons */
  gnomemeeting_sound_daemons_suspend ();


  /* Open the channel */
  gnomemeeting_threads_enter ();
  plugin = gm_conf_get_string (AUDIO_DEVICES_KEY "plugin");
  if (is_source)
    device = gm_conf_get_string (AUDIO_DEVICES_KEY "input_device");
  else
    device = gm_conf_get_string (AUDIO_DEVICES_KEY "output_device");
  gnomemeeting_threads_leave ();

  if (device.Find (_("No device found")) == P_MAX_INDEX) {

    PWaitAndSignal m(sound_event_mutex);

    sound_channel = 
      PSoundChannel::CreateOpenedChannel (plugin,
					  device,
					  is_source ?
					  PSoundChannel::Recorder
					  : PSoundChannel::Player, 
					  1, 8000, 16);
    if (sound_channel) {

      
      /* Update the volume sliders */
      //FIXME popups
      if (is_source) 
	sound_channel->GetVolume (record_vol);
      else 
	sound_channel->GetVolume (play_vol);

      
      /* Translators : the full sentence is "Opening %s for playing with
	 plugin %s" or "Opening %s for recording with plugin" */
      gnomemeeting_threads_enter ();
      gm_history_window_insert (history_window, is_source ?
				_("Opened %s for recording with plugin %s")
				: _("Opened %s for playing with plugin %s"),
				(const char *) device, 
				(const char *) plugin);
      
      gm_main_window_set_volume_sliders_values (main_window, 
						play_vol,
						record_vol);
      gnomemeeting_threads_leave ();

      return sound_channel;
    }


    gnomemeeting_threads_enter ();
    if (is_source) 
      gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Could not open audio channel for audio transmission"), _("An error occured while trying to record from the soundcard for the audio transmission. Please check that your soundcard is not busy and that your driver supports full-duplex.\nThe audio transmission has been disabled."));
    else
      gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Could not open audio channel for audio reception"), _("An error occured while trying to play audio to the soundcard for the audio reception. Please check that your soundcard is not busy and that your driver supports full-duplex.\nThe audio reception has been disabled."));
    gnomemeeting_threads_leave ();
  }  


  return NULL;
}
  

void 
GMPCSSEndPoint::OnEstablished (OpalConnection &connection)
{
  NoAnswerTimer.Stop ();
  CallPendingTimer.Stop ();

  PTRACE (3, "GMPCSSEndPoint\t PCSS connection established");
  OpalPCSSEndPoint::OnEstablished (connection);
}


void 
GMPCSSEndPoint::OnReleased (OpalConnection &connection)
{
  PTimeInterval t;

  NoAnswerTimer.Stop ();
  CallPendingTimer.Stop ();

  
  /* Start time */
  if (connection.GetConnectionStartTime ().IsValid ())
    t = PTime () - connection.GetConnectionStartTime();

  
  /* Play busy tone if it is not a missed call */
  cout << "FIXME" << endl << flush;
  PlaySoundEvent ("busy_tone_sound"); 

  
  PTRACE (3, "GMPCSSEndPoint\t PCSS connection released");
  OpalPCSSEndPoint::OnReleased (connection);
}


PString 
GMPCSSEndPoint::OnGetDestination (const OpalPCSSConnection &)
{

  return PString ();
}


void 
GMPCSSEndPoint::GetDeviceVolume (unsigned int &play_vol,
				 unsigned int &record_vol)
{
  PSafePtr<OpalCall> call = NULL;
  PSafePtr<OpalConnection> connection = NULL;
  OpalAudioMediaStream *stream = NULL;
  PSoundChannel *channel = NULL;

  call = endpoint.FindCallWithLock (endpoint.GetCurrentCallToken ());
  
  if (call != NULL) {

    connection = endpoint.GetConnection (call, FALSE);

    if (connection) {

      stream = (OpalAudioMediaStream *) 
	connection->GetMediaStream (OpalMediaFormat::DefaultAudioSessionID,
				    FALSE);

      if (stream) {

	channel = (PSoundChannel *) stream->GetChannel ();
	channel->GetVolume (play_vol);
      }

      
      stream = (OpalAudioMediaStream *) 
	connection->GetMediaStream (OpalMediaFormat::DefaultAudioSessionID,
				    TRUE);

      if (stream) {

	channel = (PSoundChannel *) stream->GetChannel ();
	channel->GetVolume (record_vol);
      }
    }
  }
}


void
GMPCSSEndPoint::SetDeviceVolume (unsigned int play_vol,
				 unsigned int record_vol)
{
  PSafePtr<OpalCall> call = NULL;
  PSafePtr<OpalConnection> connection = NULL;
  OpalAudioMediaStream *stream = NULL;
  PSoundChannel *channel = NULL;

  g_return_if_fail (play_vol >= 0 && play_vol <= 100);
  g_return_if_fail (record_vol >= 0 && record_vol <= 100);
  
  call = endpoint.FindCallWithLock (endpoint.GetCurrentCallToken ());
  
  if (call != NULL) {

    connection = endpoint.GetConnection (call, FALSE);

    if (connection) {

      stream = (OpalAudioMediaStream *) 
	connection->GetMediaStream (OpalMediaFormat::DefaultAudioSessionID,
				    FALSE);

      if (stream) {

	channel = (PSoundChannel *) stream->GetChannel ();
	channel->SetVolume (play_vol);
      }

      
      stream = (OpalAudioMediaStream *) 
	connection->GetMediaStream (OpalMediaFormat::DefaultAudioSessionID,
				    TRUE);

      if (stream) {

	channel = (PSoundChannel *) stream->GetChannel ();
	channel->SetVolume (record_vol);
      }
    }
  }
}


void
GMPCSSEndPoint::OnNoAnswerTimeout (PTimer &,
				   INT) 
{
  if (endpoint.GetCallingState () == GMEndPoint::Called) 
    ClearAllCalls (H323Connection::EndedByNoAnswer, FALSE);
}


void
GMPCSSEndPoint::OnCallPending (PTimer &,
			       INT) 
{
  GtkWidget *tray = NULL;
  
  BOOL is_ringing = FALSE;

  tray = GnomeMeeting::Process ()->GetTray ();


  gdk_threads_enter ();
  gm_tray_ring (tray);
  is_ringing = gm_tray_is_ringing (tray);
  gdk_threads_leave ();

  
  if (is_ringing) 
    PlaySoundEvent ("incoming_call_sound");
  

  if (CallPendingTimer.IsRunning ())
    CallPendingTimer.RunContinuous (PTimeInterval (0, 1));
}


