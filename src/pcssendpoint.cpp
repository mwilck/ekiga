
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
  GtkWidget *tray = NULL;
  
  int no_answer_timeout = 45;

  IncomingCallMode icm = AVAILABLE;
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  tray = GnomeMeeting::Process ()->GetTray ();


  /* Check the config keys */
  gnomemeeting_threads_enter ();
  icm = (IncomingCallMode) gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode");
  no_answer_timeout = gm_conf_get_int (CALL_OPTIONS_KEY "no_answer_timeout");
  gnomemeeting_threads_leave ();


  /* The token identifying the current call */
  incomingConnectionToken = connection.GetToken ();
  endpoint.SetCurrentCallToken (connection.GetCall ().GetToken ());


  /* Auto-Answer this call */
  if (icm == AUTO_ANSWER) {
   
    endpoint.OnIncomingConnection ((OpalConnection &) connection, 
				   4, PString ());
    return;
  }


  /* If we are here, the call doesn't need to be rejected, forwarded
     or automatically answered */
  gnomemeeting_threads_enter ();
  if (tray)
    gm_tray_update_calling_state (tray, GMEndPoint::Called);
  gm_main_window_update_calling_state (main_window, GMEndPoint::Called);
  gnomemeeting_threads_leave ();


  /* The timers */
  NoAnswerTimer.SetInterval (0, PMAX (no_answer_timeout, 10));
  CallPendingTimer.RunContinuous (PTimeInterval (5));


  /* If no forward or reject, update the internal state */
  endpoint.SetCallingState (GMEndPoint::Called);
  endpoint.SetCurrentCallToken (connection.GetCall ().GetToken ());
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
						is_source?-1:(int) play_vol,
						is_source?(int)record_vol:-1);
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

  if (tray) {

    gdk_threads_enter ();
    gm_tray_ring (tray);
    is_ringing = gm_tray_is_ringing (tray);
    gdk_threads_leave ();
  }

  
  if (is_ringing) 
    PlaySoundEvent ("incoming_call_sound");
  

  if (CallPendingTimer.IsRunning ())
    CallPendingTimer.RunContinuous (PTimeInterval (0, 1));
}


