
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
 *                         endpoint.cpp  -  description
 *                         ----------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains the Endpoint class.
 *
 */


#include "../config.h"

#include "endpoint.h"
#include "connection.h"
#include "gatekeeper.h"
#include "urlhandler.h"
#include "ils.h"
#include "lid.h"
#include "gnomemeeting.h"
#include "sound_handling.h"
#include "tray.h"
#include "misc.h"
#include "chat_window.h"
#include "log_window.h"
#include "pref_window.h"
#include "main_window.h"
#include "calls_history_window.h"
#include "stats_drawing_area.h"

#include "dialog.h"
#include "gm_conf.h"
#include "gm_events.h"

#include <h261codec.h>

#include <ptclib/http.h>
#include <ptclib/html.h>
#include <ptclib/pstun.h>


#define new PNEW


/* The class */
GMH323EndPoint::GMH323EndPoint ()
{
  /* Initialise the endpoint paramaters */
  video_grabber = NULL;
  SetCallingState (GMH323EndPoint::Standby);
  
#ifdef HAS_IXJ
  lid = NULL;
#endif
  ils_client = NULL;
  listener = NULL;
  gk = NULL;
  sc = NULL;

  /* Use IPv6 address family by default if available. */
#ifdef P_HAS_IPV6
  if (PIPSocket::IsIpAddressFamilyV6Supported())
    PIPSocket::SetDefaultIpAddressFamilyV6();
#endif
  
  audio_tester = NULL;

  audio_reception_popup = NULL;
  audio_transmission_popup = NULL;
  
  ILSTimer.SetNotifier (PCREATE_NOTIFIER (OnILSTimeout));
  ils_registered = false;

  RTPTimer.SetNotifier (PCREATE_NOTIFIER (OnRTPTimeout));
  GatewayIPTimer.SetNotifier (PCREATE_NOTIFIER (OnGatewayIPTimeout));
  GatewayIPTimer.RunContinuous (PTimeInterval (5));

  signallingChannelCallTimeout = PTimeInterval (0, 0, 3);
    
  NoIncomingMediaTimer.SetNotifier (PCREATE_NOTIFIER (OnNoIncomingMediaTimeout));

  missed_calls = 0;

  dispatcher = gm_events_dispatcher_new ();

  last_audio_octets_received = 0;
  last_video_octets_received = 0;
  last_audio_octets_transmitted = 0;
  last_video_octets_transmitted = 0;

  NoAnswerTimer.SetNotifier (PCREATE_NOTIFIER (OnNoAnswerTimeout));
  CallPendingTimer.SetNotifier (PCREATE_NOTIFIER (OnCallPending));
  OutgoingCallTimer.SetNotifier (PCREATE_NOTIFIER (OnOutgoingCall));
}


GMH323EndPoint::~GMH323EndPoint ()
{
  if (listener)
    RemoveListener (listener);

  /* Delete any GMH323Gatekeeper thread, the real gatekeeper
   * will be delete by openh323 
   */
  if (gk)
    delete (gk);

  /* Delete any GMStunClient thread. 
   */
  if (sc)
    delete (sc);
  
  PWaitAndSignal m(ils_access_mutex);
  /* Delete any ILS client which could be running */
  if (ils_client)
    delete (ils_client);

  if (dispatcher)
    g_object_unref (dispatcher);

  /* Create a new one to unregister */
  if (ils_registered) {
    
    ils_client = new GMILSClient ();
    ils_client->Unregister ();
    delete (ils_client);
  }
  
  /* Remove any running audio tester, if any */
  if (audio_tester)
    delete (audio_tester);
}


H323Connection *
GMH323EndPoint::MakeCallLocked (const PString & call_addr,
				PString & call_token)
{
  PWaitAndSignal m(lca_access_mutex);
  H323Connection *con = NULL;
  
  called_address = call_addr;
  
  con = H323EndPoint::MakeCallLocked (call_addr, call_token);

#ifdef HAS_IXJ
  GMLid *lid = NULL;
  lid = GetLid ();
#endif

  if (con) {
    
    OutgoingCallTimer.RunContinuous (PTimeInterval (5));

#ifdef HAS_IXJ
    if (lid)
      lid->UpdateState (GMH323EndPoint::Calling); // Calling
#endif
  }
  else {
    
    OutgoingCallTimer.Stop ();

#ifdef HAX_IXJ
    if (lid)
      lid->UpdateStatus (GMH323EndPoint::Standby); // Busy
#endif

    sound_event_mutex.Wait ();
    GMSoundEvent ("busy_tone_sound");
    sound_event_mutex.Signal ();
  }

#ifdef HAS_IXJ
  if (lid)
    lid->Unlock ();
#endif
  
  return con;
}


void GMH323EndPoint::UpdateDevices ()
{
  BOOL use_lid = FALSE;

  BOOL preview = FALSE;

  PString dev;
  gchar *audio_input = NULL;


  /* Get the config settings */
  gnomemeeting_threads_enter ();
  preview = gm_conf_get_bool (VIDEO_DEVICES_KEY "enable_preview");
  audio_input = gm_conf_get_string (AUDIO_DEVICES_KEY "input_device");
  gnomemeeting_threads_leave ();
  
  gnomemeeting_sound_daemons_suspend ();

  /* Do not change these values during calls */
  if (GetCallingState () == GMH323EndPoint::Standby) {

#ifdef HAS_IXJ
    GtkWidget *prefs_window = NULL;
    GMLid *l = GetLid ();
    OpalMediaFormat::List capa;

    prefs_window = GnomeMeeting::Process ()->GetPrefsWindow (); 
    if (l) {

      use_lid = TRUE;
      l->Unlock ();
    }
    else
      use_lid = FALSE;

    dev = audio_input;

    /* Quicknet hardware */
    if (dev.Find ("/dev/phone") != P_MAX_INDEX) {

      /* Use the quicknet card if needed */
      if (!use_lid) {
	
	CreateLid (audio_input);
	
	capa = GetAvailableAudioCapabilities ();

	/* We added the LID : update the codecs list 
	 * and the capabilities */
	gnomemeeting_threads_enter ();
	gm_prefs_window_update_audio_codecs_list (prefs_window, capa);
	gnomemeeting_threads_leave ();

	AddAllCapabilities ();
      }
    }
    else if (use_lid) {
      
      RemoveLid ();
      
      capa = GetAvailableAudioCapabilities ();

      /* We removed the LID : update the codecs list 
       * and the capabilities */
      gnomemeeting_threads_enter ();
      gm_prefs_window_update_audio_codecs_list (prefs_window, capa);
      gnomemeeting_threads_leave ();

      AddAllCapabilities ();
    }
    
#endif
    
    /* Video preview */
    if (preview) 
      CreateVideoGrabber (TRUE, TRUE);
    else
      RemoveVideoGrabber ();
  }

  gnomemeeting_sound_daemons_resume ();

  g_free (audio_input);
}


H323Capabilities 
GMH323EndPoint::RemoveCapability (PString name)
{
  capabilities.Remove (name);
  return capabilities;
}


void 
GMH323EndPoint::RemoveAllCapabilities ()
{
  if (capabilities.GetSize ())
    capabilities.RemoveAll ();
}


void 
GMH323EndPoint::AddAllCapabilities ()
{
  RemoveAllCapabilities ();
  

  AddAudioCapabilities ();
  AddVideoCapabilities ();
  AddUserInputCapabilities ();
}


void 
GMH323EndPoint::SetCallingState (GMH323EndPoint::CallingState i)
{
  PWaitAndSignal m(cs_access_mutex);
  
  calling_state = i;
  /* FIXME: holding a mutex!? */
  if (dispatcher)
    g_signal_emit_by_name (dispatcher, "endpoint-state-changed", i);
}


GMH323EndPoint::CallingState
GMH323EndPoint::GetCallingState (void)
{
  PWaitAndSignal m(cs_access_mutex);

  return calling_state;
}


H323Connection * 
GMH323EndPoint::SetupTransfer (const PString & token,
			       const PString & call_identity,
			       const PString & remote_party,
			       PString & new_token,
			       void *)
{
  H323Connection *con = NULL;

  con = 
    H323EndPoint::SetupTransfer (token,
				 call_identity,
				 remote_party,
				 new_token);

  SetTransferCallToken (new_token);
  
  return con;
}


void 
GMH323EndPoint::AddVideoCapabilities ()
{
  int video_size = 0;

  video_size = gm_conf_get_int (VIDEO_DEVICES_KEY "size");

  /* Add video capabilities */
  if (video_size == 1) {

    if (autoStartTransmitVideo && !autoStartReceiveVideo) {
      
      /* CIF Capability in first position */
      AddCapability (new H323_H261Capability (0, 4, FALSE, FALSE, 6217));
      AddCapability (new H323_H261Capability (2, 0, FALSE, FALSE, 6217));
    }
    else {

      /* CIF Capability in first position */
      SetCapability (0, 1, new H323_H261Capability (0, 4, FALSE, FALSE, 6217));
      SetCapability (0, 1, new H323_H261Capability (2, 0, FALSE, FALSE, 6217));
    }
  }
  else {

    if (autoStartTransmitVideo && !autoStartReceiveVideo) {
      
      AddCapability (new H323_H261Capability (2, 0, FALSE, FALSE, 6217)); 
      AddCapability (new H323_H261Capability (0, 4, FALSE, FALSE, 6217));
    }
    else {

      SetCapability (0, 1, new H323_H261Capability (2, 0, FALSE, FALSE, 6217)); 
      SetCapability (0, 1, new H323_H261Capability (0, 4, FALSE, FALSE, 6217));
    }
  }
}


void
GMH323EndPoint::AddUserInputCapabilities ()
{
  int cap = 0;

  cap = gm_conf_get_int (H323_ADVANCED_KEY "dtmf_sending");
    
  if (cap == 3)
    capabilities.SetCapability (0, P_MAX_INDEX, new H323_UserInputCapability(H323_UserInputCapability::SignalToneH245));
  else if (cap == 2)
    capabilities.SetCapability(0, P_MAX_INDEX, new H323_UserInputCapability(H323_UserInputCapability::SignalToneRFC2833));
  else if (cap == 4) {
      
    PINDEX num = capabilities.SetCapability(0, P_MAX_INDEX, new H323_UserInputCapability(H323_UserInputCapability::HookFlashH245));
    capabilities.SetCapability(0, num+1, new H323_UserInputCapability(H323_UserInputCapability::BasicString));
      
  } else if (cap != 1)
    AddAllUserInputCapabilities(0, P_MAX_INDEX);
}


OpalMediaFormat::List
GMH323EndPoint::GetAvailableAudioCapabilities ()
{
#ifdef HAS_IXJ
  GMLid *l = NULL;
#endif
  
  PString s;
  
  OpalMediaFormat::List full_list;
  OpalMediaFormat::List lid_list;
  
  full_list = H323PluginCodecManager::GetMediaFormats ();

#ifdef HAS_IXJ
  l = GetLid ();

  if (l) {

    lid_list = l->GetAvailableAudioCapabilities ();
    l->Unlock ();
  
    for (int i = 0 ; i < lid_list.GetSize () ; i++) {

      if (full_list.GetValuesIndex (lid_list [i]) == P_MAX_INDEX)
	full_list.Append (new OpalMediaFormat (lid_list [i]));
    }
  }
#endif
  
  return full_list;
}


void 
GMH323EndPoint::AddAudioCapabilities ()
{
  gchar **couple = NULL;
  GSList *codecs_data = NULL;
  
  
  /* Read config settings */ 
  codecs_data = gm_conf_get_string_list (AUDIO_CODECS_KEY "list");
  

  /* Let's go */
  while (codecs_data) {
    
    couple = g_strsplit ((gchar *) codecs_data->data, "=", 0);

    if (couple && couple [0] && couple [1] && !strcmp (couple [1], "1")) 
      H323EndPoint::AddAllCapabilities (0, 0, PString (couple [0]) + ("*"));
    
    g_strfreev (couple);
    codecs_data = codecs_data->next;
  }

  g_slist_free (codecs_data);

#ifdef HAS_IXJ
    GMLid *l = NULL;
    if ((l = GetLid ())) {
/* FIXME */
      if (l->IsOpen ()) {

	H323_LIDCapability::AddAllCapabilities (*l, capabilities, 0, 0);
      }

      l->Unlock ();
    }
#endif
}


PString
GMH323EndPoint::GetCurrentIP ()
{
  PIPSocket::InterfaceTable interfaces;
  PIPSocket::Address ip_addr;


  if (!PIPSocket::GetInterfaceTable (interfaces))
    PIPSocket::GetHostAddress (ip_addr);
  else {

    for (int i = 0; i < interfaces.GetSize(); i++) {

      ip_addr = interfaces [i].GetAddress();

      if (ip_addr != 0  && 
	  ip_addr != PIPSocket::Address()) /* Ignore 127.0.0.1 */
	
	return ip_addr.AsString ();
    }
  }

  return PString ();
}


void 
GMH323EndPoint::TranslateTCPAddress(PIPSocket::Address &local_address, 
				    const PIPSocket::Address &remote_address)
{
  PIPSocket::Address addr;
  BOOL ip_translation = FALSE;
  gchar *ip = NULL;

  gnomemeeting_threads_enter ();
  ip_translation = gm_conf_get_bool (NAT_KEY "enable_ip_translation");
  gnomemeeting_threads_leave ();

  if (ip_translation) {

    /* Ignore Ip translation for local networks and for IPv6 */
    if ( !IsLocalAddress (remote_address)
#ifdef P_HAS_IPV6
	 && (remote_address.GetVersion () != 6 || remote_address.IsV4Mapped ())
#endif
	 ) {

      gnomemeeting_threads_enter ();
      ip = gm_conf_get_string (NAT_KEY "public_ip");
      gnomemeeting_threads_leave ();

      if (ip) {

	addr = PIPSocket::Address (ip);

	if (addr != PIPSocket::Address ("0.0.0.0"))
	  local_address = addr;
      }

      g_free (ip);
    }
  }
}


BOOL 
GMH323EndPoint::StartListener ()
{
  int listen_port = 1720;

  gnomemeeting_threads_enter ();
  listen_port = gm_conf_get_int (PORTS_KEY "listen_port");
  gnomemeeting_threads_leave ();

  
  /* Start the listener thread for incoming calls */
  listener =
    new H323ListenerTCP (*this, PIPSocket::GetDefaultIpAny (), listen_port);
   

  /* unsuccesfull */
  if (!H323EndPoint::StartListener (listener)) {

    delete listener;
    listener = NULL;

    return FALSE;
  }
   
  return TRUE;
}


void 
GMH323EndPoint::StartAudioTester (gchar *audio_manager,
				  gchar *audio_player,
				  gchar *audio_recorder)
{
  PWaitAndSignal m(at_access_mutex);
  
  if (audio_tester)     
    delete (audio_tester);

  audio_tester =
    new GMAudioTester (audio_manager, audio_player, audio_recorder);
}


void 
GMH323EndPoint::StopAudioTester ()
{
  PWaitAndSignal m(at_access_mutex);

  if (audio_tester) {
   
    delete (audio_tester);
    audio_tester = NULL;
  }
}


GMVideoGrabber *
GMH323EndPoint::CreateVideoGrabber (BOOL start_grabbing,
				    BOOL synchronous)
{
  PWaitAndSignal m(vg_access_mutex);

  if (video_grabber)
    delete (video_grabber);

  video_grabber = new GMVideoGrabber (start_grabbing, synchronous);

  return video_grabber;
}


void
GMH323EndPoint::RemoveVideoGrabber ()
{
  PWaitAndSignal m(vg_access_mutex);

  if (video_grabber) {

    delete (video_grabber);
  }      
  video_grabber = NULL;
}


GMVideoGrabber *
GMH323EndPoint::GetVideoGrabber ()
{
  PWaitAndSignal m(vg_access_mutex);

  if (video_grabber)
    video_grabber->Lock ();
  
  return video_grabber;
}


H323Gatekeeper *
GMH323EndPoint::CreateGatekeeper(H323Transport * transport)
{
  return new H323GatekeeperWithNAT (*this, transport);
}


H323Connection *
GMH323EndPoint::CreateConnection (unsigned call_reference)
{
  return new GMH323Connection (*this, call_reference);
}


void
GMH323EndPoint::ILSRegister (void)
{
  /* Force the Update */
  ILSTimer.RunContinuous (PTimeInterval (5));
}


void 
GMH323EndPoint::SetCurrentCallToken (PString s)
{
  PWaitAndSignal m(ct_access_mutex);

  current_call_token = s;
}


PString 
GMH323EndPoint::GetCurrentCallToken ()
{
  PWaitAndSignal m(ct_access_mutex);

  return current_call_token;
}


H323Gatekeeper *
GMH323EndPoint::GetGatekeeper ()
{
  return gatekeeper;
}


void 
GMH323EndPoint::GatekeeperRegister ()
{
  int timeout = 0;
  int registering_method = 0;
  
  gnomemeeting_threads_enter ();
  registering_method = gm_conf_get_int (H323_GATEKEEPER_KEY "registering_method");
  timeout = gm_conf_get_int (H323_GATEKEEPER_KEY "registration_timeout");
  gnomemeeting_threads_leave ();
  
  if (gk)
    delete (gk);
  
  registrationTimeToLive = 
    PTimeInterval (0, PMAX (120, PMIN (3600, timeout * 60)));
  gk = new GMH323Gatekeeper ();
}


void 
GMH323EndPoint::SetSTUNServer ()
{
  if (sc)
    delete (sc);
  
  /* Be a client for the specified STUN Server */
  sc = new GMStunClient (TRUE);
}


BOOL 
GMH323EndPoint::OnIncomingCall (H323Connection & connection, 
                                const H323SignalPDU &, H323SignalPDU &)
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
  BOOL use_gateway = FALSE;
  
#ifdef HAS_IXJ
  GMLid *l = NULL;
#endif    
  
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
  use_gateway = gm_conf_get_bool (H323_GATEWAY_KEY "use_gateway");
  gateway_conf = gm_conf_get_string (H323_GATEWAY_KEY "host");
  gnomemeeting_threads_leave ();


  if (forward_host_conf)
    forward_host = PString (GMURL (forward_host_conf).GetValidURL ());
  else
    forward_host = PString ("");
    
  gateway = PString (gateway_conf);


  /* Remote Name and application */
  GetRemoteConnectionInfo (connection, utf8_name, utf8_app, utf8_url);


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
    do_forward = TRUE;
  }
  else if (icm == DO_NOT_DISTURB) {

    msg =
      g_strdup_printf (_("Rejecting call from %s (Do Not Disturb)"),
		       (const char *) utf8_name);
    
    do_reject = TRUE;
  }
  /* if we are already in a call: forward or reject */
  else if (GetCallingState () != GMH323EndPoint::Standby) {

    /* if we have enabled forward when busy, do the forward */
    if (!forward_host.IsEmpty() && busy_forward) {

      msg = 
	g_strdup_printf (_("Forwarding call from %s to %s (Busy)"),
			 (const char *) utf8_name, 
			 (const char *) forward_host);

      do_forward = TRUE;
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

      connection.ClearCall (H323Connection::EndedByLocalBusy); 
      return FALSE;
    }
    else if (do_forward) {

      gnomemeeting_threads_enter ();
      gm_main_window_flash_message (main_window, _("Call forwarded"));
      gnomemeeting_threads_leave ();

      if (use_gateway && !gateway.IsEmpty ())
        forward_host = forward_host + "@" + gateway;

      return !connection.ForwardCall (forward_host);
    }
    else if (do_answer) {

      gnomemeeting_threads_enter ();
      gm_main_window_flash_message (main_window,
				    _("Call automatically answered"));
      gnomemeeting_threads_leave ();

      return TRUE;
    }
  }
   

  /* If we are here, the call doesn't need to be rejected, forwarded
     or automatically answered */
  gnomemeeting_threads_enter ();
  gm_tray_update_calling_state (tray, GMH323EndPoint::Called);
  gm_main_window_update_calling_state (main_window, GMH323EndPoint::Called);
  gnomemeeting_threads_leave ();


  /* Update the LID state */
#ifdef HAS_IXJ
  l = GetLid ();
  
  if (l) {

    l->UpdateState (GMH323EndPoint::Called);
    l->Unlock ();
  }
#endif

    
  /* The timers */
  NoAnswerTimer.SetInterval (0, PMAX (no_answer_timeout, 10));
  CallPendingTimer.RunContinuous (PTimeInterval (5));

  
  /* If no forward or reject, update the internal state */
  SetCurrentCallToken (connection.GetCallToken ());
  SetCallingState (GMH323EndPoint::Called);
  

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

  return TRUE;
}


BOOL
GMH323EndPoint::OnConnectionForwarded (H323Connection &,
				       const PString &forward_party,
				       const H323SignalPDU &)
{
  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;
  
  gchar *msg = NULL;
  PString call_token;

  call_token = GetCurrentCallToken ();
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();

  
  if (MakeCall (forward_party, call_token)) {

    gnomemeeting_threads_enter ();
    msg = g_strdup_printf (_("Forwarding call to %s"),
			   (const char*) forward_party);
    gm_main_window_flash_message (main_window, msg);
    gm_history_window_insert (history_window, msg);
    gnomemeeting_threads_leave ();
    g_free (msg);

    return TRUE;
  }
  else {

    msg = g_strdup_printf (_("Error while forwarding call to %s"),
			   (const char*) forward_party);
    gnomemeeting_threads_enter ();
    gnomemeeting_warning_dialog (GTK_WINDOW (main_window), msg, _("There was an error when forwarding the call to the given host."));
    gnomemeeting_threads_leave ();

    g_free (msg);

    return FALSE;
  }

  return FALSE;
}


void 
GMH323EndPoint::OnConnectionEstablished (H323Connection & connection, 
                                         const PString & token)
{
  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;
  GtkWidget *chat_window = NULL;
  GtkWidget *tray = NULL;

  gchar *utf8_url = NULL;
  gchar *utf8_app = NULL;
  gchar *utf8_name = NULL;
  gchar *utf8_local_name = NULL;
  BOOL reg = FALSE;
  BOOL forward_on_busy = FALSE;
  BOOL stay_on_top = FALSE;
  IncomingCallMode icm = AVAILABLE;
  
#ifdef HAS_IXJ
  GMLid *l = NULL;
#endif
  

  /* Get the widgets */
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  tray = GnomeMeeting::Process ()->GetTray ();
  
  
  /* Start refreshing the stats */
  RTPTimer.RunContinuous (PTimeInterval (0, 1));

  
  /* Remote Name and application */
  GetRemoteConnectionInfo (connection, utf8_name, utf8_app, utf8_url);
  utf8_local_name = g_strdup ((const char *) GetLocalUserName ());

  
  /* Get the config settings */
  gnomemeeting_threads_enter ();
  reg = gm_conf_get_bool (LDAP_KEY "enable_registering");
  icm = 
    (IncomingCallMode) gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode");
  forward_on_busy = gm_conf_get_bool (CALL_FORWARDING_KEY "forward_on_busy");
  stay_on_top = gm_conf_get_bool (VIDEO_DISPLAY_KEY "stay_on_top");
  gnomemeeting_threads_leave ();
  

  /* Stop the Timers */
  NoAnswerTimer.Stop ();
  CallPendingTimer.Stop ();
  OutgoingCallTimer.Stop ();


#ifdef HAS_IXJ
  l = GetLid ();
  
  if (l) {

    l->UpdateState (GMH323EndPoint::Connected);
    l->Unlock ();
  }
#endif


  /* Update internal state */
  SetCurrentCallToken (token);
  SetCallingState (GMH323EndPoint::Connected);


  /* Update the GUI */
  gnomemeeting_threads_enter ();
  
  if (connection.HadAnsweredCall ())
    gm_main_window_set_call_url (main_window, GMURL ().GetDefaultURL ());

  gm_main_window_flash_message (main_window, _("Connected"));
  gm_history_window_insert (history_window,
			    _("Connected with %s using %s"), 
			    utf8_name, utf8_app);
  gnomemeeting_text_chat_call_start_notification (chat_window);

  gm_main_window_set_remote_user_name (main_window, utf8_name);
  gm_main_window_set_stay_on_top (main_window, stay_on_top);

  gm_main_window_update_calling_state (main_window, GMH323EndPoint::Connected);
  gm_tray_update_calling_state (tray, GMH323EndPoint::Connected);
  gm_tray_update (tray, GMH323EndPoint::Connected, icm, forward_on_busy);
  gnomemeeting_threads_leave ();
  if (dispatcher)
    g_signal_emit_by_name (dispatcher, "call-begin", (const gchar *)token);

  
  /* Update ILS if needed */
  if (reg)
    ILSRegister ();

  g_free (utf8_name);
  g_free (utf8_local_name);
  g_free (utf8_app);
  g_free (utf8_url);
}


void
GMH323EndPoint::GetRemoteConnectionInfo (H323Connection & connection,
					 gchar * & utf8_name,
					 gchar * & utf8_app,
					 gchar * & utf8_url)
{
  const H323Transport *transport = NULL;
  H323TransportAddress address;

  PINDEX idx;
  
  PString remote_ip;
  PString remote_name;
  PString remote_app;
  PString remote_alias;


  /* Get information about the remote user */
  remote_name = connection.GetRemotePartyName ();
  idx = remote_name.Find ("(");
  if (idx != P_MAX_INDEX) {
    
    remote_alias = remote_name.Mid (idx + 1);
    remote_alias =
      remote_alias.Mid (0, (remote_alias.Find (",") != P_MAX_INDEX) ?
			remote_alias.Find (",") : remote_alias.Find (")"));
    remote_name = remote_name.Left (idx);
  }
  idx = remote_name.Find ("[");
  if (idx != P_MAX_INDEX)
    remote_name = remote_name.Left (idx);
  
  remote_app = connection.GetRemoteApplication ();
  idx = remote_app.Find ("(");
  if (idx != P_MAX_INDEX)
    remote_app = remote_app.Left (idx);
  idx = remote_app.Find ("[");
  if (idx != P_MAX_INDEX)
    remote_app = remote_app.Left (idx);
  
  if (IsRegisteredWithGatekeeper ()) {

    if (!connection.GetRemotePartyNumber ().IsEmpty ())
      remote_ip = connection.GetRemotePartyNumber ();
    else if (!remote_alias.IsEmpty ()) 
      remote_ip = remote_alias;
  }

  if (remote_ip.IsEmpty ()) {

    /* Get the remote IP to display in the calls history */
    transport = connection.GetSignallingChannel ();
    if (transport) 
      remote_ip = transport->GetRemoteAddress ().GetHostName ();
  }

  remote_ip = GMURL ().GetDefaultURL () + remote_ip;
  utf8_app = gnomemeeting_get_utf8 (remote_app.Trim ());
  utf8_name = gnomemeeting_get_utf8 (remote_name.Trim ());
  utf8_url = gnomemeeting_get_utf8 (remote_ip);
}


void 
GMH323EndPoint::OnConnectionCleared (H323Connection & connection, 
                                     const PString & clearedCallToken)
{
  GtkWidget *calls_history_window = NULL;
  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;
  GtkWidget *chat_window = NULL;
  GtkWidget *tray = NULL;
  
  gchar *msg_reason = NULL;
  
  gchar *utf8_url = NULL;
  gchar *utf8_name = NULL;
  gchar *utf8_app = NULL;

  PTimeInterval t;

  BOOL reg = FALSE;
  BOOL not_current = FALSE;
  BOOL forward_on_busy = FALSE;

  IncomingCallMode icm = AVAILABLE;

#ifdef HAS_IXJ
  GMLid *l = NULL;
#endif

  calls_history_window = GnomeMeeting::Process ()->GetCallsHistoryWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  tray = GnomeMeeting::Process ()->GetTray ();


  if (connection.GetConnectionStartTime ().IsValid ())
    t = PTime () - connection.GetConnectionStartTime();


  /* Get config settings */
  gnomemeeting_threads_enter ();
  reg = gm_conf_get_bool (LDAP_KEY "enable_registering");
  icm = (IncomingCallMode)gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode");
  forward_on_busy = gm_conf_get_bool (CALL_FORWARDING_KEY "forward_on_busy");
  gnomemeeting_threads_leave ();


  /* If we are called because the current incoming call has ended and 
     not another call, ok, else do nothing */
  if (GetCurrentCallToken () == clearedCallToken) {

    if (!GetTransferCallToken ().IsEmpty ()) {

      SetCurrentCallToken (GetTransferCallToken ());
      SetTransferCallToken (PString ());
    }
    else {

      SetCurrentCallToken (PString ());
      SetTransferCallToken (PString ());
    }
  }
  else
    not_current = TRUE;



  switch (connection.GetCallEndReason ()) {

  case H323Connection::EndedByLocalUser :
    msg_reason = g_strdup (_("Local user cleared the call"));
    break;
  case H323Connection::EndedByRemoteUser :
    msg_reason = g_strdup (_("Remote user cleared the call"));
    break;
  case H323Connection::EndedByRefusal :
    msg_reason = g_strdup (_("Remote user did not accept the call"));
    break;
  case H323Connection::EndedByNoAccept :
    msg_reason = g_strdup (_("Local user did not accept the call"));
    break;
  case H323Connection::EndedByAnswerDenied :
    msg_reason = g_strdup (_("Local user did not accept the call"));
    break;
  case H323Connection::EndedByGatekeeper :
    msg_reason = g_strdup (_("The Gatekeeper cleared the call"));
    break;
  case H323Connection::EndedByNoAnswer :
    msg_reason = g_strdup (_("Call not answered in the required time"));
    break;
  case H323Connection::EndedByCallerAbort :
    msg_reason = g_strdup (_("Remote user has stopped calling"));
    break;
  case H323Connection::EndedByTransportFail :
    msg_reason = g_strdup (_("Abnormal call termination"));
    break;
  case H323Connection::EndedByConnectFail :
    msg_reason = g_strdup (_("Could not connect to remote host"));
    break;
  case H323Connection::EndedByNoBandwidth :
    msg_reason = g_strdup (_("Insufficient bandwidth"));
    break;
  case H323Connection::EndedByCapabilityExchange :
    msg_reason = g_strdup (_("No common codec"));
    break;
  case H323Connection::EndedByCallForwarded :
    msg_reason = g_strdup (_("Call forwarded"));
    break;
  case H323Connection::EndedBySecurityDenial :
    msg_reason = g_strdup (_("Security check failed"));
    break;
  case H323Connection::EndedByLocalBusy :
    msg_reason = g_strdup (_("Local user is busy"));
    break;
  case H323Connection::EndedByRemoteBusy :
    msg_reason = g_strdup (_("Remote user is busy"));
    break;
  case H323Connection::EndedByRemoteCongestion :
    msg_reason = g_strdup (_("Congested link to remote party"));
    break;
  case H323Connection::EndedByLocalCongestion :
    msg_reason = g_strdup (_("Congested link to remote party"));
    break;
  case H323Connection::EndedByUnreachable :
    msg_reason = g_strdup (_("Remote user is unreachable"));
    break;
  case H323Connection::EndedByNoEndPoint :
    msg_reason = g_strdup (_("Remote user is unreachable"));
    break;
  case H323Connection::EndedByHostOffline :
    msg_reason = g_strdup (_("Remote host is offline"));
    break;
  case H323Connection::EndedByTemporaryFailure :
    msg_reason = g_strdup (_("Temporary failure"));
    break;
  case H323Connection::EndedByNoUser :
    msg_reason = g_strdup (_("User not found"));
    break;

  default :
    msg_reason = g_strdup (_("Call completed"));
  }

  /* Update the calls history */
  GetRemoteConnectionInfo (connection, utf8_name, utf8_app, utf8_url);

  gnomemeeting_threads_enter ();
  if (t.GetSeconds () == 0 
      && connection.HadAnsweredCall ()
      && connection.GetCallEndReason () != H323Connection::EndedByLocalUser) {

    gm_calls_history_add_call (MISSED_CALL, 
			       utf8_name,
			       utf8_url,
			       "0",
			       msg_reason,
			       utf8_app);
    mc_access_mutex.Wait ();
    missed_calls++;
    mc_access_mutex.Signal ();

    /* Translators, pay attention to the singular/plural distinction */
    gm_main_window_push_info_message (main_window, 
				      ngettext (_("Missed %d call"),
						_("Missed %d calls"),
						missed_calls), 
				      missed_calls);
  }
  else
    if (connection.HadAnsweredCall ())
      gm_calls_history_add_call (RECEIVED_CALL, utf8_name,
				 utf8_url,
				 t.AsString (0),
				 msg_reason,
				 utf8_app);
    else
      gm_calls_history_add_call (PLACED_CALL, 
				 utf8_name,
				 GetLastCallAddress (),
				 t.AsString (0),
				 msg_reason,
				 utf8_app);


  gm_history_window_insert (history_window, msg_reason);
  gnomemeeting_text_chat_call_stop_notification (chat_window);
  gm_main_window_flash_message (main_window, msg_reason);
  gnomemeeting_threads_leave ();
  if (dispatcher)
    g_signal_emit_by_name (dispatcher, "call-end",
			   (const gchar *)clearedCallToken);

  g_free (utf8_app);
  g_free (utf8_name);
  g_free (utf8_url);  
  g_free (msg_reason);

  if (not_current) 
    return;


  /* Stop the Timers */
  NoAnswerTimer.Stop ();
  CallPendingTimer.Stop (); 
  OutgoingCallTimer.Stop ();
  NoIncomingMediaTimer.Stop ();
  

  /* Play busy tone if it is not a missed call */
  sound_event_mutex.Wait ();
  if (!(t.GetSeconds () == 0 && connection.HadAnsweredCall ()))
    GMSoundEvent ("busy_tone_sound");
  sound_event_mutex.Signal ();

  
  /* Update the main window */
  gnomemeeting_threads_enter ();
  gm_main_window_set_remote_user_name (main_window, "");
  gm_main_window_clear_stats (main_window);
  gnomemeeting_threads_leave ();
  

  /* Play Busy Tone */
#ifdef HAS_IXJ
  l = GetLid ();
  
  if (l) {

    l->UpdateState (GMH323EndPoint::Standby);
    l->Unlock ();
  }
#endif

  /* we reset the no-data detection */
  RTPTimer.Stop ();
  last_audio_octets_received = 0;
  last_video_octets_received = 0;
  last_audio_octets_transmitted = 0;
  last_video_octets_transmitted = 0;

  /* We update the stats part */  
  gnomemeeting_threads_enter ();

  audio_reception_popup = NULL;
  audio_transmission_popup = NULL;

  
  /* We update the GUI */
  gm_main_window_set_stay_on_top (main_window, FALSE);
  gm_main_window_update_calling_state (main_window, GMH323EndPoint::Standby);
  gm_tray_update_calling_state (tray, GMH323EndPoint::Standby);
  gm_tray_update (tray, GMH323EndPoint::Standby, icm, forward_on_busy);
  gnomemeeting_threads_leave ();
  
  gnomemeeting_sound_daemons_resume ();

  
  /* Update internal state */
  SetCallingState (GMH323EndPoint::Standby);


  /* No need to do all that if we are simply receiving an incoming call
     that was rejected because of DND */
  if (GetCallingState () != GMH323EndPoint::Called
      && GetCallingState () != GMH323EndPoint::Calling) {

    /* Update ILS if needed */
    if (reg)
      ILSRegister ();
  }


  /* Try to update the devices use if some settings were changed 
     during the call */
  UpdateDevices ();
}


void 
GMH323EndPoint::SavePicture (void)
{ 
  PTime ts = PTime ();
  
  GtkWidget *main_window = NULL;
  GdkPixbuf *pic = NULL;
  
  gchar *prefix = NULL;
  gchar *dirname = NULL;
  gchar *filename = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  prefix = gm_conf_get_string (GENERAL_KEY "save_prefix");
  
  dirname = (gchar *) g_get_home_dir ();
  pic = gm_main_window_get_current_picture (main_window);

  if (pic && prefix && dirname) {
    
    filename = g_strdup_printf ("%s/%s%.2d_%.2d_%.2d-%.2d%.2d%.2d.png",
				dirname, prefix,
				ts.GetYear(), ts.GetMonth(), ts.GetDay(),
				ts.GetHour(), ts.GetMinute(), ts.GetSecond());
	
    gdk_pixbuf_save (pic, filename, "png", NULL, NULL);
    g_free (prefix);
    g_free (filename);
  }
}


void
GMH323EndPoint::SetUserNameAndAlias ()
{
  gchar *firstname = NULL;
  gchar *lastname = NULL;
  gchar *local_name = NULL;
  gchar *alias = NULL;
  BOOL gk_alias_as_first = FALSE;

  
  /* Set the local User name */
  gnomemeeting_threads_enter ();
  firstname = gm_conf_get_string (PERSONAL_DATA_KEY "firstname");
  lastname = gm_conf_get_string (PERSONAL_DATA_KEY "lastname");
  alias = gm_conf_get_string (H323_GATEKEEPER_KEY "alias");  
  gk_alias_as_first =
    gm_conf_get_bool (H323_GATEKEEPER_KEY "register_alias_as_primary");  
  gnomemeeting_threads_leave ();

  
  if (firstname && lastname && strcmp (firstname, ""))  { 

    if (strcmp (lastname, ""))
      local_name = g_strconcat (firstname, " ", lastname, NULL);
    else
      local_name = g_strdup (firstname);

    SetLocalUserName (local_name);
  }

  if (alias && strcmp (alias, "")) {

    if (!gk_alias_as_first)
      AddAliasName (alias);
    else {

      SetLocalUserName (alias);

      if (local_name && strcmp (local_name, ""))
	AddAliasName (local_name);
    }
  }

  
  g_free (alias);
  g_free (firstname);
  g_free (lastname);
  g_free (local_name);
}


void
GMH323EndPoint::SetPorts ()
{
  gchar *rtp_port_range = NULL;
  gchar *udp_port_range = NULL;
  gchar *tcp_port_range = NULL;

  gchar **rtp_couple = NULL;
  gchar **udp_couple = NULL;
  gchar **tcp_couple = NULL;

  rtp_port_range = gm_conf_get_string (PORTS_KEY "rtp_port_range");
  udp_port_range = gm_conf_get_string (PORTS_KEY "udp_port_range");
  tcp_port_range = gm_conf_get_string (PORTS_KEY "tcp_port_range");

  if (rtp_port_range)
    rtp_couple = g_strsplit (rtp_port_range, ":", 0);
  if (udp_port_range)
    udp_couple = g_strsplit (udp_port_range, ":", 0);
  if (tcp_port_range)
    tcp_couple = g_strsplit (tcp_port_range, ":", 0);
  
  if (tcp_couple && tcp_couple [0] && tcp_couple [1]) {

    SetTCPPorts (atoi (tcp_couple [0]), atoi (tcp_couple [1]));
    PTRACE (1, "Set TCP port range to " << atoi (tcp_couple [0])
	    << atoi (tcp_couple [1]));
  }

  if (rtp_couple && rtp_couple [0] && rtp_couple [1]) {

    SetRtpIpPorts (atoi (rtp_couple [0]), atoi (rtp_couple [1]));
    PTRACE (1, "Set RTP port range to " << atoi (rtp_couple [0])
	    << atoi (rtp_couple [1]));
  }

  if (udp_couple && udp_couple [0] && udp_couple [1]) {

    SetUDPPorts (atoi (udp_couple [0]), atoi (udp_couple [1]));
    PTRACE (1, "Set UDP port range to " << atoi (udp_couple [0])
	    << atoi (udp_couple [1]));
  }

  g_free (tcp_port_range);
  g_free (udp_port_range);
  g_free (rtp_port_range);
  g_strfreev (tcp_couple);
  g_strfreev (udp_couple);
  g_strfreev (rtp_couple);
}


void
GMH323EndPoint::Init ()
{
  GtkWidget *main_window = NULL;
  
  gchar *stun_server = NULL;
  
  stun_server = gm_conf_get_string (NAT_KEY "stun_server");


  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  
  /* Update the internal state */
  autoStartTransmitVideo =
    gm_conf_get_bool (VIDEO_CODECS_KEY "enable_video_transmission");
  autoStartReceiveVideo =
    gm_conf_get_bool (VIDEO_CODECS_KEY "enable_video_reception");

  disableH245Tunneling = !gm_conf_get_bool (H323_ADVANCED_KEY "enable_h245_tunneling");
  disableFastStart = !gm_conf_get_bool (H323_ADVANCED_KEY "enable_fast_start");
  disableH245inSetup = !gm_conf_get_bool (H323_ADVANCED_KEY "enable_early_h245");
    
  /* Setup ports */
  SetPorts ();
  
  /* Update general devices configuration */
  UpdateDevices ();
  
  /* Set the User Name and Alias */  
  SetUserNameAndAlias ();

  /* Add capabilities */
  AddAllCapabilities ();

  /* Register to gatekeeper */
  if (gm_conf_get_int (H323_GATEKEEPER_KEY "registering_method"))
    GatekeeperRegister ();

  /* The LDAP part, if needed */
  if (gm_conf_get_bool (LDAP_KEY "enable_registering"))
    ILSRegister ();
  
  
  if (!StartListener ()) 
    gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Error while starting the listener"), _("You will not be able to receive incoming calls. Please check that no other program is already running on the port used by GnomeMeeting."));

  
  if (gm_conf_get_bool (NAT_KEY "enable_stun_support")) 
    SetSTUNServer ();


  g_free (stun_server);
}


void
GMH323EndPoint::OnNoIncomingMediaTimeout (PTimer &,
					  INT)
{
  if (gm_conf_get_bool (CALL_OPTIONS_KEY "clear_inactive_calls"))
    ClearAllCalls (H323Connection::EndedByTransportFail, FALSE);
}


void 
GMH323EndPoint::OnILSTimeout (PTimer &,
			      INT)
{
  PWaitAndSignal m(ils_access_mutex);

  gboolean reg = false;

  gnomemeeting_threads_enter ();
  reg = gm_conf_get_bool (LDAP_KEY "enable_registering");
  gnomemeeting_threads_leave ();


  if (!ils_client) {
    
    ils_client = new GMILSClient ();

    if (reg) {

      if (!ils_registered) {

	ils_client->Register ();
	ils_registered = true;
      }
      else {

	if (ILSTimer.GetResetTime ().GetMinutes () == 20) {

	  ils_client->Unregister ();
	  ils_client->Register ();
	  ils_registered = true;
	}
	else {

	  ils_client->Modify ();
	}
      }
    }
    else if (ils_registered) {

      ils_client->Unregister ();
      ils_registered = false;
    }

    delete (ils_client);
    ils_client = NULL;
  }  


  ILSTimer.RunContinuous (PTimeInterval (0, 0, 20));
}


BOOL 
GMH323EndPoint::OpenAudioChannel (H323Connection & connection,
				  BOOL is_encoding,
				  unsigned bufferSize,
				  H323AudioCodec & codec)
{
  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;
  
  PSoundChannel *sound_channel = NULL;
  
  PString plugin;
  PString device;
  
  unsigned int vol = 0;

  BOOL sd = FALSE;
  BOOL no_error = TRUE;

  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();

  
  PWaitAndSignal m(audio_channel_mutex);

  /* Wait that the primary call has terminated (in case of transfer)
     before opening the channels for the second call */
  TransferCallWait ();


  /* Stop the OnNoAnswerTimers */
  NoAnswerTimer.Stop ();
  CallPendingTimer.Stop ();
  OutgoingCallTimer.Stop ();
  
  
  gnomemeeting_threads_enter ();
  sd = gm_conf_get_bool (AUDIO_CODECS_KEY "enable_silence_detection");
  gnomemeeting_threads_leave ();

  if (is_encoding) 
    codec.SetSilenceDetectionMode (!sd ?
				   H323AudioCodec::NoSilenceDetection :
				   H323AudioCodec::AdaptiveSilenceDetection);
  

  /* Suspend the daemons */
  gnomemeeting_sound_daemons_suspend ();

#ifdef HAS_IXJ
  GMLid *l = NULL;

  /* If we are using a hardware LID, connect the audio stream to the LID */
  if ((l = GetLid ()) && l->IsOpen()) {

    lid->StopTone (0);
      
    if (!codec.AttachChannel (new OpalLineChannel (*l, OpalIxJDevice::POTSLine, codec))) {

      l->Unlock ();

      return FALSE;
    }
    else
      if (l)
	l->Unlock ();

    gnomemeeting_threads_enter ();
    gm_history_window_insert (history_window,
			      _("Attaching lid hardware to codec"));
    gnomemeeting_threads_leave ();
  }
  else
#endif
    {   
      gnomemeeting_threads_enter ();
      plugin = gm_conf_get_string (AUDIO_DEVICES_KEY "plugin");
      if (is_encoding)
	device = gm_conf_get_string (AUDIO_DEVICES_KEY "input_device");
      else
	device = gm_conf_get_string (AUDIO_DEVICES_KEY "output_device");
      gnomemeeting_threads_leave ();

      if (device.Find (_("No device found")) == P_MAX_INDEX) {

	sound_event_mutex.Wait ();
	sound_channel = 
	  PSoundChannel::CreateOpenedChannel (plugin,
					      device,
					      is_encoding ?
					      PSoundChannel::Recorder
					      : PSoundChannel::Player, 
					      1, 8000, 16);
	sound_event_mutex.Signal ();

       
	if (sound_channel) {

	  /* Translators : the full sentence is "Opening %s for playing with
	     plugin %s" or "Opening %s for recording with plugin" */

	  gnomemeeting_threads_enter ();
	  gm_history_window_insert (history_window, is_encoding ?
				    _("Opened %s for recording with plugin %s")
				    : _("Opened %s for playing with plugin %s"),
				    (const char *) device, 
				    (const char *) plugin);
	  gnomemeeting_threads_leave ();

	  /* Control the channel and attach it to the codec, do not
	     auto delete it */
	  sound_channel->SetBuffers (bufferSize, soundChannelBuffers);
	  no_error = codec.AttachChannel (sound_channel);

	  /* Update the volume sliders */
	  GetDeviceVolume (sound_channel, is_encoding, vol);
	  gnomemeeting_threads_enter ();
	  gm_main_window_set_volume_sliders_values (main_window, 
						    is_encoding?-1:(int) vol,
						    !is_encoding?-1:(int) vol);
	  gnomemeeting_threads_leave ();
	}
	else
	  no_error = FALSE; /* No PSoundChannel */
      }
      else {

	return FALSE; /* Device was _("No device found"), ignore, no popup */
      }
    }

  
  if (!no_error) {

    gnomemeeting_threads_enter ();

    if (is_encoding) {

      if (!audio_transmission_popup)
	audio_transmission_popup =
	  gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Could not open audio channel for audio transmission"), _("An error occured while trying to record from the soundcard for the audio transmission. Please check that your soundcard is not busy and that your driver supports full-duplex.\nThe audio transmission has been disabled."));
    }
    else
      if (!audio_reception_popup)
	audio_reception_popup =
	  gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Could not open audio channel for audio reception"), _("An error occured while trying to play audio to the soundcard for the audio reception. Please check that your soundcard is not busy and that your driver supports full-duplex.\nThe audio reception has been disabled."));
    gnomemeeting_threads_leave ();
  }
    
  return no_error;
}


BOOL
GMH323EndPoint::SetDeviceVolume (PSoundChannel *sound_channel,
                                 BOOL is_encoding,
                                 unsigned int vol)
{
  return DeviceVolume (sound_channel, is_encoding, TRUE, vol);
}


BOOL
GMH323EndPoint::GetDeviceVolume (PSoundChannel *sound_channel,
                                 BOOL is_encoding,
                                 unsigned int &vol)
{
  return DeviceVolume (sound_channel, is_encoding, FALSE, vol);
}


BOOL 
GMH323EndPoint::StartLogicalChannel (const PString & capability_name,
				     unsigned id, 
				     BOOL from_remote)
{
  H323Connection *con = NULL;
  H323Channel *channel = NULL;
  H323Capability *capability = NULL;
  H323Capabilities capabilities;
  BOOL no_error = FALSE;

  PString mode, current_capa;

  con = FindConnectionWithLock (GetCurrentCallToken ());

  if (con) {

    channel = con->FindChannel (id, from_remote);
    capabilities = con->GetLocalCapabilities ();
    capability = capabilities.FindCapability (capability_name);

      
    if (!from_remote) {

      if (channel ||
	  !capability ||
	  !con->OpenLogicalChannel (*capability,
				    capability->GetDefaultSessionID(),
				    H323Channel::IsTransmitter)) {
        no_error = FALSE;
      }
    }

    con->Unlock ();
  }

    
  return no_error;
}


BOOL 
GMH323EndPoint::StopLogicalChannel (unsigned id, 
				    BOOL from_remote)
{
  H323Connection *con = NULL;
  H323Channel *channel = NULL;
  BOOL no_error = TRUE;

  con = FindConnectionWithLock (GetCurrentCallToken ());

  if (con) {

    channel =
      con->FindChannel (id, from_remote);

    if (channel) {
      
      con->CloseLogicalChannelNumber (channel->GetNumber ());
    }
    else 
      no_error = FALSE;

    con->Unlock ();
  }

  return no_error;
}


void 
GMH323EndPoint::OnRTPTimeout (PTimer &, INT)
{
  GtkWidget *main_window = NULL;
  
  int rtt = 0;
  int jitter_buffer_size = 0;
  RTP_Session *audio_session = NULL;
  RTP_Session *video_session = NULL;

  H323Connection *con = NULL;

  gchar *msg = NULL;

  int received_packets = 0;
  int lost_packets = 0;
  int late_packets = 0;
  float lost_packets_per = 0.0;
  float late_packets_per = 0.0;

  int new_audio_octets_received = 0;
  int new_video_octets_received = 0;
  int new_audio_octets_transmitted = 0;
  int new_video_octets_transmitted = 0;
  float received_audio_speed = 0;
  float received_video_speed = 0;
  float transmitted_audio_speed = 0;
  float transmitted_video_speed = 0;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  con = FindConnectionWithLock (GetCurrentCallToken ());

  if (!con)
    return;

  if (con->GetConnectionStartTime () == PTime (0)) {

    con->Unlock ();
    return;
  }

  PTimeInterval t =
    PTime () - con->GetConnectionStartTime();

  audio_session = 
    con->GetSession(RTP_Session::DefaultAudioSessionID);	  
  video_session = 
    con->GetSession(RTP_Session::DefaultVideoSessionID);

	  
  if (audio_session) {
    new_audio_octets_received = audio_session->GetOctetsReceived();
    new_audio_octets_transmitted = audio_session->GetOctetsSent();
  } else {
    new_audio_octets_received = last_audio_octets_received;
    new_audio_octets_transmitted = last_audio_octets_transmitted;
  }
  received_audio_speed = (float) (new_audio_octets_received
				  - last_audio_octets_received)/ 1024;
  transmitted_audio_speed = (float) (new_audio_octets_transmitted
				     - last_audio_octets_transmitted)/ 1024;


  if (video_session) {
    new_video_octets_received = video_session->GetOctetsReceived();
    new_video_octets_transmitted = video_session->GetOctetsSent();
  } else {
    new_video_octets_received = last_video_octets_received;
    new_video_octets_transmitted = last_video_octets_transmitted;
  }
  received_video_speed = (float) (new_video_octets_received
				  - last_video_octets_received)/ 1024;
  transmitted_video_speed = (float) (new_video_octets_transmitted
				     - last_video_octets_transmitted)/ 1024;

  
  /* If we didn't receive any audio and video data this time,
     then we start the timer */
  if (new_audio_octets_received == last_audio_octets_received
      && new_video_octets_received == last_video_octets_received) {
    
    if (!NoIncomingMediaTimer.IsRunning ()) 
      NoIncomingMediaTimer.SetInterval (0, 30);
  }
  else
    NoIncomingMediaTimer.Stop ();
  
  last_audio_octets_received = new_audio_octets_received;
  last_video_octets_received = new_video_octets_received;
  last_audio_octets_transmitted = new_audio_octets_transmitted;
  last_video_octets_transmitted = new_video_octets_transmitted;

  msg = g_strdup_printf 
    (_("%.2ld:%.2ld:%.2ld  A:%.2f/%.2f   V:%.2f/%.2f"), 
     (long) t.GetHours (), (long) (t.GetMinutes () % 60), 
     (long) (t.GetSeconds () % 60),
     transmitted_audio_speed, received_audio_speed,
     transmitted_video_speed, received_video_speed);
	

  if (audio_session) {

    received_packets = audio_session->GetPacketsReceived ();
    lost_packets = audio_session->GetPacketsLost ();
    late_packets = audio_session->GetPacketsTooLate ();
	
    jitter_buffer_size = audio_session->GetJitterBufferSize ();
  }
	
  if (video_session) {

    received_packets =
      received_packets + video_session->GetPacketsReceived ();
    lost_packets = lost_packets+video_session->GetPacketsLost ();
    late_packets = late_packets+video_session->GetPacketsTooLate ();
  }

  if (received_packets > 0) {

    lost_packets_per = ((float) lost_packets * 100.0
			/ (float) received_packets);
    late_packets_per = ((float) late_packets * 100.0
			/ (float) received_packets);
  }

  rtt = con->GetRoundTripDelay().GetMilliSeconds();
      
  con->Unlock ();


  gdk_threads_enter ();
  gm_main_window_flash_message (main_window, msg);

  if (gm_conf_get_int (USER_INTERFACE_KEY "main_window/control_panel_section") 
      == 0)
    gm_main_window_update_stats (main_window,
			     lost_packets_per,
			     late_packets_per,
			     (int) rtt,
			     (int) (jitter_buffer_size / 8),
			     new_video_octets_received,
			     new_video_octets_transmitted,
			     new_audio_octets_received,
			     new_audio_octets_transmitted);
      

  gdk_threads_leave ();


  g_free (msg);
}


PString
GMH323EndPoint::CheckTCPPorts ()
{
  PHTTPClient web_client ("GnomeMeeting");
  PString html;
  PString url;

  
  url = PString("http://seconix.com/firewall/index.php?min_tcp_port=")
    + PString (GetTCPPortBase ()) + PString ("&max_tcp_port=")
    + PString (GetTCPPortMax ());

  web_client.GetTextDocument (url, html);

  return html;
}


void 
GMH323EndPoint::OnGatewayIPTimeout (PTimer &,
				    INT)
{
  PHTTPClient web_client ("GnomeMeeting");
  PString html, ip_address;
  gboolean ip_checking = false;

  gdk_threads_enter ();
  ip_checking = gm_conf_get_bool (NAT_KEY "enable_ip_checking");
  gdk_threads_leave ();

  if (ip_checking) {

    gchar *ip_detector = gm_conf_get_string (NAT_KEY "public_ip_detector");
    if (ip_detector != NULL && web_client.GetTextDocument (ip_detector, html)) {

      if (!html.IsEmpty ()) {

	PRegularExpression regex ("[0-9]*[.][0-9]*[.][0-9]*[.][0-9]*");
	PINDEX pos, len;

	if (html.FindRegEx (regex, pos, len)) 
	  ip_address = html.Mid (pos,len);

      }
    }
    if (ip_detector != NULL)
      g_free (ip_detector);
    if (!ip_address.IsEmpty ()) {

      gdk_threads_enter ();
      gm_conf_set_string (NAT_KEY "public_ip",
			(gchar *) (const char *) ip_address);
      gdk_threads_leave ();
    }
  }

  GatewayIPTimer.RunContinuous (PTimeInterval (0, 0, 15));
}


void
GMH323EndPoint::OnNoAnswerTimeout (PTimer &,
				   INT) 
{
  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;

  
  H323Connection *connection = NULL;
  
  gchar *forward_host_conf = NULL;
  
  PString forward_host;

  gboolean no_answer_forward = FALSE;
  
  
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  main_window = GnomeMeeting::Process ()->GetMainWindow ();


  gdk_threads_enter ();
  
  /* Forwarding on no answer */
  no_answer_forward = 
    gm_conf_get_bool (CALL_FORWARDING_KEY "forward_on_no_answer");
  forward_host_conf = 
    gm_conf_get_string (CALL_FORWARDING_KEY "forward_host");

  if (forward_host_conf)
    forward_host = PString (forward_host_conf);
  else
    forward_host = PString ("");

  gdk_threads_leave ();


  /* If forward host specified and forward requested */
  if (!forward_host.IsEmpty () && no_answer_forward) {

    connection = FindConnectionWithLock (GetCurrentCallToken ());

    if (connection) {
      
      connection->ForwardCall (PString ((const char *) forward_host));

      gdk_threads_enter ();
      gm_history_window_insert (history_window,
			       _("Forwarding Call to %s (No Answer)"), 
			       (const char *) forward_host);

      gm_main_window_flash_message (main_window, _("Call forwarded"));
      gdk_threads_leave ();

      connection->Unlock ();
    }
  }
  else {

    if (GetCallingState () == GMH323EndPoint::Called) 
      ClearAllCalls (H323Connection::EndedByNoAnswer, FALSE);
  }
  g_free (forward_host_conf);
}


void
GMH323EndPoint::OnCallPending (PTimer &,
			       INT) 
{
  GtkWidget *tray = NULL;
  
  BOOL is_ringing = FALSE;

  tray = GnomeMeeting::Process ()->GetTray ();


  gdk_threads_enter ();
  gm_tray_ring (tray);
  is_ringing = gm_tray_is_ringing (tray);
  gdk_threads_leave ();

  
  if (is_ringing) {

    sound_event_mutex.Wait ();
    GMSoundEvent ("incoming_call_sound");
    sound_event_mutex.Signal ();
  }

  if (CallPendingTimer.IsRunning ())
    CallPendingTimer.RunContinuous (PTimeInterval (0, 1));
}


void
GMH323EndPoint::OnOutgoingCall (PTimer &,
				INT) 
{
  sound_event_mutex.Wait ();
  GMSoundEvent ("ring_tone_sound");
  sound_event_mutex.Signal ();

  if (OutgoingCallTimer.IsRunning ())
    OutgoingCallTimer.RunContinuous (PTimeInterval (0, 3));
}


BOOL 
GMH323EndPoint::DeviceVolume (PSoundChannel *sound_channel,
                              BOOL is_encoding,
                              BOOL set, 
			      unsigned int &vol) 
{
  H323Connection *con = NULL;

#ifdef HAS_IXJ
  GMLid *l = NULL;
#endif

  BOOL err = TRUE;
  PString call_token;

  call_token = GetCurrentCallToken ();

  con = FindConnectionWithLock (call_token);

  if (con) {

#ifdef HAS_IXJ
    if (set) {

      l = GetLid ();
      if (l) {

        l->SetVolume (is_encoding, vol);
        l->Unlock ();

        con->Unlock ();

        return TRUE; /* Ignore the sound channel if a LID is used. Hopefully,
                        OpenH323 will be fixed soon */
      }
    }
#endif


    /* TRUE = from_remote = playing */
    if (sound_channel) {

      if (set) {

        err = 
          sound_channel->SetVolume (vol)
          && err;
      }
      else {

        err = 
          sound_channel->GetVolume (vol)
          && err;
      }
    }

    con->Unlock ();
  }

  return err;
}


BOOL 
GMH323EndPoint::OpenVideoChannel (H323Connection & connection,
                                  BOOL is_encoding, 
                                  H323VideoCodec & codec)
{
  int vq = 0;
  int bf = 0;
  int tr_fps = 0;
  int bitrate = 2;

  PVideoChannel *channel = NULL;
  GDKVideoOutputDevice *display_device = NULL;
  
  GMVideoGrabber *vg = NULL;

  BOOL result = FALSE;

  PWaitAndSignal m(video_channel_mutex);
  
  /* Wait that the primary call has terminated (in case of transfer)
     before opening the channels for the second call */
  TransferCallWait ();


  /* Stop the OnNoAnswerTimers */
  NoAnswerTimer.Stop ();
  CallPendingTimer.Stop ();

  /* Transmitting */
  if (is_encoding && autoStartTransmitVideo) {

    gnomemeeting_threads_enter ();
    vq = gm_conf_get_int (VIDEO_CODECS_KEY "transmitted_video_quality");
    bf = gm_conf_get_int (VIDEO_CODECS_KEY "transmitted_background_blocks");
    bitrate = gm_conf_get_int (VIDEO_CODECS_KEY "maximum_video_bandwidth");
    tr_fps = gm_conf_get_int (VIDEO_CODECS_KEY "transmitted_fps");
    gnomemeeting_threads_leave ();


    /* Will update the codec settings */
    vq = 25 - (int) ((double) vq / 100 * 24);

    
    /* The maximum quality corresponds to the lowest quality indice, 1
       and the lowest quality corresponds to 24 */
    codec.SetTxQualityLevel (vq);
    codec.SetBackgroundFill (bf);   
    codec.SetMaxBitRate (bitrate * 8 * 1024);
    codec.SetTargetFrameTimeMs (0);
    codec.SetVideoMode (H323VideoCodec::AdaptivePacketDelay |
			codec.GetVideoMode());

    /* Needed to be able to stop start the channel on-the-fly. When
     * the channel has been closed, the rawdata channel has been closed
     * too but not deleted. We delete it now.
     */
    vg = GetVideoGrabber ();
    if (!vg || !vg->IsChannelOpen ()) {

      CreateVideoGrabber (FALSE, TRUE);
      vg = GetVideoGrabber ();
    }
      
    if (vg) {
    
      vg->StopGrabbing ();
      channel = vg->GetVideoChannel ();
      vg->Unlock ();
    }
    else
      return FALSE;


    if (channel)
      result = codec.AttachChannel (channel, FALSE);
   
    return result;
  }
  else if (!is_encoding && autoStartReceiveVideo) {

    channel = new PVideoChannel;
    display_device = new GDKVideoOutputDevice (is_encoding);
    display_device->SetColourFormatConverter ("YUV420P");      
    channel->AttachVideoPlayer (display_device);

    vg = GetVideoGrabber ();
    if (vg) {
      
      if (!autoStartTransmitVideo) 
        vg->StopGrabbing ();
      
      vg->Unlock ();
    }
      

    if (channel)
      result = codec.AttachChannel (channel);

    return result;
  }


  return FALSE;
}


#ifdef HAS_IXJ
GMLid *
GMH323EndPoint::GetLid (void)
{
  PWaitAndSignal m(lid_access_mutex);
    
  if (lid) 
    lid->Lock ();

  return lid;
}


GMLid *
GMH323EndPoint::CreateLid (PString lid_device)
{
  PWaitAndSignal m(lid_access_mutex);
  
  if (lid)
    delete (lid);

  lid = new GMLid (lid_device);

  return lid;
}


void
GMH323EndPoint::RemoveLid (void)
{
  PWaitAndSignal m(lid_access_mutex);
  
  if (lid)     
    delete (lid);

  lid = NULL;
}
#endif


void
GMH323EndPoint::SendTextMessage (PString callToken,
				 PString message)
{
  H323Connection *connection = NULL;

  connection = FindConnectionWithLock (callToken);

  if (connection) {

    connection->SendUserInput ("MSG"+message);
    connection->Unlock ();
  }

}


BOOL
GMH323EndPoint::IsCallOnHold (PString callToken)
{
  H323Connection *connection = NULL;
  BOOL result = FALSE;

  connection = FindConnectionWithLock (callToken);

  if (connection) {
    
    result = connection->IsCallOnHold ();
    connection->Unlock ();
  }

  return result;
}


BOOL
GMH323EndPoint::SetCallOnHold (PString callToken,
			       gboolean state)
{
  H323Connection *connection = NULL;
  BOOL result = FALSE;
  
  connection = FindConnectionWithLock (callToken);

  if (connection) {
    
    if (state)
      connection->HoldCall (TRUE);
    else
      connection->RetrieveCall ();

    if (connection->IsCallOnHold () == state)
      result = TRUE;
    
    connection->Unlock ();
  }

  return result;
}


void
GMH323EndPoint::SendDTMF (PString call_token,
			  PString dtmf)
{
  H323Connection *connection = NULL;
  
  connection = FindConnectionWithLock (call_token);

  if (connection) {
    
    connection->SendUserInput (dtmf);
    connection->Unlock ();
  }
}


gboolean
GMH323EndPoint::IsCallWithAudio (PString callToken)
{
  H323Connection *connection = NULL;
  H323Channel *channel = NULL;
  gboolean result = FALSE;


  connection = FindConnectionWithLock(callToken);

  if (connection) {
    channel = connection->FindChannel (RTP_Session::DefaultAudioSessionID,
				       FALSE);
    if (channel)
      result = TRUE;
    else
      result = FALSE;
    connection->Unlock ();
  }

  return result;
}


gboolean
GMH323EndPoint::IsCallWithVideo (PString callToken)
{
  H323Connection *connection = NULL;
  H323Channel *channel = NULL;
  gboolean result = FALSE;


  connection = FindConnectionWithLock(callToken);

  if (connection) {
    channel = connection->FindChannel (RTP_Session::DefaultVideoSessionID,
				       FALSE);
    if (channel)
      result = TRUE;
    else
      result = FALSE;
    connection->Unlock ();
  }

  return result;
}


gboolean
GMH323EndPoint::IsCallAudioPaused (PString callToken)
{
  H323Connection *connection = NULL;
  H323Channel *channel = NULL;
  gboolean result = FALSE;


  connection = FindConnectionWithLock(callToken);

  if (connection) {
    channel = connection->FindChannel (RTP_Session::DefaultAudioSessionID,
				       FALSE);
    if (channel)
      result = channel->IsPaused ();
    else
      result = FALSE;
    connection->Unlock ();
  }

  return result;
}


gboolean
GMH323EndPoint::IsCallVideoPaused (PString callToken)
{
  H323Connection *connection = NULL;
  H323Channel *channel = NULL;
  gboolean result = FALSE;


  connection = FindConnectionWithLock(callToken);

  if (connection) {
    channel = connection->FindChannel (RTP_Session::DefaultVideoSessionID,
				       FALSE);
    if (channel)
      result = channel->IsPaused ();
    else
      result = FALSE;
    connection->Unlock ();
  }

  return result;
}


BOOL
GMH323EndPoint::SetCallAudioPause (PString callToken, 
				   BOOL state)
{
  BOOL result = FALSE;
  
  H323Connection *connection = NULL;
  H323Channel *channel = NULL;


  connection = FindConnectionWithLock (callToken);

  if (connection) {
  
    channel = connection->FindChannel (RTP_Session::DefaultAudioSessionID,
				       FALSE);
    if (channel) {
      
      channel->SetPause (state);
      result = TRUE;
    }
    
    connection->Unlock ();
  }

  return result;
}


BOOL
GMH323EndPoint::SetCallVideoPause (PString callToken, 
				   BOOL state)
{
  BOOL result = FALSE;

  H323Connection *connection = NULL;
  H323Channel *channel = NULL;


  connection = FindConnectionWithLock (callToken);

  if (connection) {
    
    channel = connection->FindChannel (RTP_Session::DefaultVideoSessionID,
				       FALSE);
    if (channel) {
      
      channel->SetPause (state);
      result = TRUE;
    }
    connection->Unlock ();
  }

  return result;
}


void
GMH323EndPoint::ResetMissedCallsNumber ()
{
  PWaitAndSignal m(mc_access_mutex);

  missed_calls = 0;
}


int
GMH323EndPoint::GetMissedCallsNumber ()
{
  PWaitAndSignal m(mc_access_mutex);

  return missed_calls;
}

void
GMH323EndPoint::AddObserver (GObject *observer)
{
  g_return_if_fail (observer != NULL);

  gm_events_dispatcher_add_observer (dispatcher, observer);
}

PString
GMH323EndPoint::GetLastCallAddress ()
{
  PWaitAndSignal m(lca_access_mutex);

  return called_address;
}


void 
GMH323EndPoint::SetTransferCallToken (PString s)
{
  tct_access_mutex.Wait ();
  transfer_call_token = s;
  tct_access_mutex.Signal ();
}


PString 
GMH323EndPoint::GetTransferCallToken ()
{
  PString c;

  tct_access_mutex.Wait ();
  c = transfer_call_token;
  tct_access_mutex.Signal ();

  return c;
}


void
GMH323EndPoint::TransferCallWait ()
{

  while (!GetTransferCallToken ().IsEmpty ())
    PThread::Current ()->Sleep (100);
}
          
