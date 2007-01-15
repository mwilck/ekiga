
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
 *                         pcssendpoint.cpp  -  description
 *                         --------------------------------
 *   begin                : Sun Oct 24 2004
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the PCSS Endpoint class.
 *
 */


#include "../../config.h"

#include "common.h"

#include <opal/patch.h>

#include "gmconf.h"
#include "gmdialog.h"

#include "pcss.h"
#include "manager.h"
#include "ekiga.h"
#include "urlhandler.h"

#include "misc.h"
#include "audio.h"
#include "main.h"
#include "callshistory.h"
#include "history.h"
#include "statusicon.h"


#define new PNEW

GMPCSSEndpoint::GMPCSSEndpoint (GMManager & ep) 
: OpalPCSSEndPoint (ep), endpoint (ep)
{
  CallPendingTimer.SetNotifier (PCREATE_NOTIFIER (OnCallPending));
  OutgoingCallTimer.SetNotifier (PCREATE_NOTIFIER (OnOutgoingCall));

  SetSoundChannelBufferDepth (3);
}


GMPCSSEndpoint::~GMPCSSEndpoint () 
{
}


BOOL 
GMPCSSEndpoint::MakeConnection (OpalCall & call, 
                                const PString & party,  
                                void * userData)
{
  return OpalPCSSEndPoint::MakeConnection (call, party, userData);
}


void GMPCSSEndpoint::AcceptCurrentIncomingCall ()
{
  if (!incomingConnectionToken.IsEmpty ()) {
    
    AcceptIncomingConnection (incomingConnectionToken);
    incomingConnectionToken = PString ();
  }
}


void GMPCSSEndpoint::OnShowIncoming (const OpalPCSSConnection & connection)
{
  IncomingCallMode icm = AUTO_ANSWER;
  GtkWidget *statusicon = NULL;
  guint interval = 2000;

  if (endpoint.GetCallingState() != GMManager::Called)
    return;

  statusicon = GnomeMeeting::Process ()->GetStatusicon ();

  /* Check the config keys */
  gnomemeeting_threads_enter ();
  icm =
    (IncomingCallMode) gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode");
  gnomemeeting_threads_leave ();

  /* The token identifying the current call */
  incomingConnectionToken = connection.GetToken ();

  /* If it is an auto-answer, answer now */
  if (icm == AUTO_ANSWER) {
  
    AcceptCurrentIncomingCall ();
    return;
  }

  /* The timers */
  CallPendingTimer.RunContinuous (interval);

  gnomemeeting_threads_enter ();
  gm_statusicon_ring (statusicon, interval);
  gnomemeeting_threads_leave ();
}


BOOL GMPCSSEndpoint::OnShowOutgoing (const OpalPCSSConnection & connection)
{
  if (endpoint.GetCallingState () == GMManager::Calling)
    OutgoingCallTimer.RunContinuous (PTimeInterval (5));

  return TRUE;
}


void 
GMPCSSEndpoint::PlaySoundEvent (PString ev)
{
  PWaitAndSignal m(sound_event_mutex);

  GMSoundEvent c (ev);
}


PSoundChannel * 
GMPCSSEndpoint::CreateSoundChannel (const OpalPCSSConnection & connection,
				    const OpalMediaFormat & format,
				    BOOL is_source)
{
  PTRACE(3, "Ekiga\tCreating Sound Channel");
  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;
  GtkWidget *statusicon = NULL;

  PSoundChannel *sound_channel = NULL;

  gchar *plugin = NULL;
  gchar *device = NULL;

  unsigned int play_vol = 0;
  unsigned int record_vol = 0;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  statusicon = GnomeMeeting::Process ()->GetStatusicon ();

  /* Stop the Timers */
  CallPendingTimer.Stop ();
  OutgoingCallTimer.Stop ();

  /* Update the GUI */
  gnomemeeting_threads_enter ();
  gm_statusicon_stop_ringing (statusicon);
  gnomemeeting_threads_leave ();

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

  if (PString (device).Find (_("No device found")) == P_MAX_INDEX) {

    PWaitAndSignal m(sound_event_mutex);

    sound_channel = 
      PSoundChannel::CreateOpenedChannel (plugin,
					  device,
					  is_source ?
					  PSoundChannel::Recorder
					  : PSoundChannel::Player, 
					  1, format.GetClockRate (), 16);
    if (sound_channel) {

      
      /* Update the volume sliders */
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

      g_free (plugin);
      g_free (device);

      return sound_channel;
    }


    gnomemeeting_threads_enter ();
    if (is_source) 
      gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Could not open audio channel for audio transmission"), _("An error occured while trying to record from the soundcard for the audio transmission. Please check that your soundcard is not busy and that your driver supports full-duplex.\nThe audio transmission has been disabled."));
    else
      gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Could not open audio channel for audio reception"), _("An error occured while trying to play audio to the soundcard for the audio reception. Please check that your soundcard is not busy and that your driver supports full-duplex.\nThe audio reception has been disabled."));
    gnomemeeting_threads_leave ();
  }  

  g_free (plugin);
  g_free (device);

  return NULL;
}
  

void 
GMPCSSEndpoint::OnEstablished (OpalConnection &connection)
{
  GtkWidget *statusicon = NULL;

  statusicon = GnomeMeeting::Process ()->GetStatusicon ();

  CallPendingTimer.Stop ();
  OutgoingCallTimer.Stop ();

  gnomemeeting_threads_enter ();
  gm_statusicon_stop_ringing (statusicon);
  gnomemeeting_threads_leave ();

  PTRACE (3, "GMPCSSEndpoint\t PCSS connection established");
  OpalPCSSEndPoint::OnEstablished (connection);
}


void 
GMPCSSEndpoint::OnReleased (OpalConnection &connection)
{
  PTimeInterval t;
  GtkWidget *statusicon = NULL;

  statusicon = GnomeMeeting::Process ()->GetStatusicon ();

  CallPendingTimer.Stop ();
  OutgoingCallTimer.Stop ();

  gnomemeeting_threads_enter ();
  gm_statusicon_stop_ringing (statusicon);
  gnomemeeting_threads_leave ();

  gnomemeeting_sound_daemons_resume ();

  PTRACE (3, "GMPCSSEndpoint\t PCSS connection released");
  OpalPCSSEndPoint::OnReleased (connection);
}


PString 
GMPCSSEndpoint::OnGetDestination (const OpalPCSSConnection &connection)
{
  return PString ();
}


void 
GMPCSSEndpoint::GetDeviceVolume (unsigned int &play_vol,
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


BOOL
GMPCSSEndpoint::SetDeviceVolume (unsigned int play_vol,
				 unsigned int record_vol)
{
  BOOL success1 = FALSE;
  BOOL success2 = FALSE;

  PSafePtr<OpalCall> call = NULL;
  PSafePtr<OpalConnection> connection = NULL;
  OpalAudioMediaStream *stream = NULL;
  PSoundChannel *channel = NULL;

  g_return_val_if_fail (play_vol >= 0 && play_vol <= 100, FALSE);
  g_return_val_if_fail (record_vol >= 0 && record_vol <= 100, FALSE);
  
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
	success1 = TRUE;
      }
      
      stream = (OpalAudioMediaStream *) 
	connection->GetMediaStream (OpalMediaFormat::DefaultAudioSessionID,
				    TRUE);

      if (stream) {

	channel = (PSoundChannel *) stream->GetChannel ();
	channel->SetVolume (record_vol);
	success2 = TRUE;
      }
    }
  }

  return (success1 && success2);
}


void
GMPCSSEndpoint::OnOutgoingCall (PTimer &,
                                INT) 
{
  PlaySoundEvent ("ring_tone_sound");

  if (OutgoingCallTimer.IsRunning ())
    OutgoingCallTimer.RunContinuous (PTimeInterval (0, 3));
}


void
GMPCSSEndpoint::OnCallPending (PTimer &,
			       INT) 
{
  PlaySoundEvent ("incoming_call_sound");
}


