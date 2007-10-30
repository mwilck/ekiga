
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
 *                         endpoint.cpp  -  description
 *                         ----------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the Endpoint class.
 *
 */


#include "config.h"

#include "manager.h"
#include "pcss.h"
#include "h323.h"
#include "sip.h"

#include "accounts.h"
#include "urlhandler.h"

#include "ekiga.h"
#include "audio.h"
#include "misc.h"
#include "preferences.h"
#include "main.h"

#include "gmdialog.h"
#include "gmconf.h"

#include <opal/transcoders.h>
#include <ptclib/http.h>
#include <ptclib/html.h>
#include <ptclib/pstun.h>

#define new PNEW


extern "C" {
  unsigned char linear2ulaw(int pcm_val);
  int ulaw2linear(unsigned char u_val);
};


/* The class */
GMManager::GMManager ()
{
  /* Initialise the endpoint paramaters */
  video_grabber = NULL;
  SetCallingState (GMManager::Standby);
  
#ifdef HAS_AVAHI
  zcp = NULL;
#endif

  gk = NULL;
  sc = NULL;

  PIPSocket::SetDefaultIpAddressFamilyV4();
  
  audio_tester = NULL;

  audio_reception_popup = NULL;
  audio_transmission_popup = NULL;
  
  manager = NULL;

  RTPTimer.SetNotifier (PCREATE_NOTIFIER (OnRTPTimeout));
  AvgSignalTimer.SetNotifier (PCREATE_NOTIFIER (OnAvgSignalTimeout));
  GatewayIPTimer.SetNotifier (PCREATE_NOTIFIER (OnGatewayIPTimeout));
  GatewayIPTimer.RunContinuous (PTimeInterval (5));

  IPChangedTimer.SetNotifier (PCREATE_NOTIFIER (OnIPChanged));
  IPChangedTimer.RunContinuous (120000);
  NoIncomingMediaTimer.SetNotifier (PCREATE_NOTIFIER (OnNoIncomingMediaTimeout));

  h323EP = NULL;
  sipEP = NULL;
  pcssEP = NULL;

  PVideoDevice::OpenArgs video = GetVideoOutputDevice();
  video.deviceName = "EKIGAOUT";
  SetVideoOutputDevice (video);
  
  video = GetVideoOutputDevice();
  video.deviceName = "EKIGAIN";
  SetVideoPreviewDevice (video);
  
  video = GetVideoInputDevice();
  video.deviceName = "Moving logo";
  SetVideoInputDevice (video);
}


GMManager::~GMManager ()
{
  Exit ();
}


void
GMManager::Exit ()
{
  ClearAllCalls (OpalConnection::EndedByLocalUser, TRUE);

  StopAudioTester ();

#ifdef HAS_AVAHI
  RemoveZeroconfClient ();
#endif

  RemoveAccountsEndpoint ();

  RemoveVideoGrabber ();

  RemoveSTUNClient ();
}


BOOL
GMManager::SetUpCall (const PString & call_addr,
		       PString & call_token)
{
  BOOL result = FALSE;
  
  result = OpalManager::SetUpCall ("pc:*", call_addr, call_token, NULL);

  if (!result) 
    pcssEP->PlaySoundEvent ("busy_tone_sound");
  
  return result;
}


BOOL
GMManager::AcceptCurrentIncomingCall ()
{
  if (pcssEP) {

    pcssEP->AcceptCurrentIncomingCall ();
    return TRUE;
  }

  return FALSE;
}


void GMManager::UpdateDevices ()
{
  BOOL preview = FALSE;
  gchar *device_name = NULL;

  /* Get the config settings */
  gnomemeeting_threads_enter ();
  preview = gm_conf_get_bool (VIDEO_DEVICES_KEY "enable_preview");
  device_name = gm_conf_get_string (VIDEO_DEVICES_KEY "input_device");
  gnomemeeting_threads_leave ();
  
  /* Do not change these values during calls */
  if (GetCallingState () == GMManager::Standby) {

    /* Video preview */
    if (preview) 
      CreateVideoGrabber (TRUE, TRUE);
    else 
      RemoveVideoGrabber ();
      
    /* Update the video input device */
    PVideoDevice::OpenArgs video = GetVideoInputDevice();
    video.deviceName = device_name;
    SetVideoInputDevice (video);
  }

  g_free (device_name);
}


void 
GMManager::SetCallingState (CallingState i)
{
  PWaitAndSignal m(cs_access_mutex);
  
  calling_state = i;
}


GMManager::CallingState
GMManager::GetCallingState ()
{
  PWaitAndSignal m(cs_access_mutex);

  return calling_state;
}


OpalMediaFormatList
GMManager::GetAvailableAudioMediaFormats ()
{
  OpalMediaFormatList full_list;
  OpalMediaFormatList sip_list;
  OpalMediaFormatList h323_list;
 
  sip_list = sipEP->GetAvailableAudioMediaFormats ();
  h323_list = h323EP->GetAvailableAudioMediaFormats ();

  /* Merge common codecs */
  for (PINDEX i = 0 ; i < sip_list.GetSize () ; i++) {

    for (PINDEX j = 0 ; j < h323_list.GetSize () ; j++) {
      
      if (sip_list [i].GetPayloadType () == h323_list [j].GetPayloadType ()
          && sip_list [i].GetBandwidth () == h323_list [j].GetBandwidth ()) {

        full_list += sip_list [i];
        full_list += h323_list [j];
      }
    }
  }

  return full_list;
}


OpalMediaFormatList
GMManager::GetAvailableVideoMediaFormats ()
{
  OpalMediaFormatList full_list;
  OpalMediaFormatList sip_list;
  OpalMediaFormatList h323_list;
 
  sip_list = sipEP->GetAvailableVideoMediaFormats ();
  h323_list = h323EP->GetAvailableVideoMediaFormats ();

  /* Merge common codecs */
  for (PINDEX i = 0 ; i < sip_list.GetSize () ; i++) {
    full_list += sip_list [i];
  }
  for (PINDEX j = 0 ; j < h323_list.GetSize () ; j++) {
    full_list += h323_list [j];
  }

  return full_list;
}


void 
GMManager::SetAudioMediaFormats (PStringArray *order)
{
  PStringArray initial_order;
  PStringArray initial_mask;

  OpalMediaFormatList media_formats;
  OpalMediaFormatList media_formats_list;

  PStringArray mask;

  if (order == NULL) 
    return;

  OpalMediaFormat::GetAllRegisteredMediaFormats(media_formats_list);

  media_formats_list.Remove (*order);

  // Configure Audio Codec Order
  for (int i = 0 ; i < media_formats_list.GetSize () ; i++)
    if (media_formats_list [i].GetDefaultSessionID () == OpalMediaFormat::DefaultAudioSessionID) {

      if (media_formats_list [i].IsTransportable())
        mask += media_formats_list [i];
      else
        *order += media_formats_list [i];
    }

  initial_order = GetMediaFormatOrder ();
  initial_mask = GetMediaFormatMask ();

  // Skip Video MediaFormats
  for (int i = 0 ; i < initial_order.GetSize () ; i++)
    if (OpalMediaFormat (initial_order [i]).GetDefaultSessionID () == OpalMediaFormat::DefaultVideoSessionID)
      *order += initial_order [i];

  for (int i = 0 ; i < initial_mask.GetSize () ; i++)
    if (OpalMediaFormat (initial_mask [i]).GetDefaultSessionID () == OpalMediaFormat::DefaultVideoSessionID)
      mask += initial_mask [i];
  
  SetMediaFormatMask (mask);
  SetMediaFormatOrder (*order);
}


void 
GMManager::SetVideoMediaFormats (PStringArray *order)
{
  int size = 0;
  int frame_rate = 0;
  int max_rx_bitrate = 2;
  int max_tx_bitrate = 2;

  PStringArray initial_order;
  PStringArray initial_mask;

  OpalMediaFormatList media_formats_list;

  PStringArray mask;
  PStringArray last_priority;

  if (order == NULL) 
    return;

  gnomemeeting_threads_enter ();
  frame_rate = gm_conf_get_int (VIDEO_CODECS_KEY "frame_rate");
  max_rx_bitrate = gm_conf_get_int (VIDEO_CODECS_KEY "maximum_video_rx_bitrate");
  max_tx_bitrate = gm_conf_get_int (VIDEO_CODECS_KEY "maximum_video_tx_bitrate");
  size = gm_conf_get_int (VIDEO_DEVICES_KEY "size");
  gnomemeeting_threads_leave ();

  if (frame_rate <= 0)
    frame_rate = 15;

  OpalMediaFormat::GetAllRegisteredMediaFormats(media_formats_list);

  //Configure all MediaOptions of all Video MediaFormats
  for (int i = 0 ; i < media_formats_list.GetSize () ; i++) {
    OpalMediaFormat media_format = media_formats_list[i];
    if (media_format.GetDefaultSessionID () == OpalMediaFormat::DefaultVideoSessionID) {

      media_format.SetOptionInteger (OpalVideoFormat::FrameWidthOption (), 
                                 video_sizes[size].width);  
      media_format.SetOptionInteger (OpalVideoFormat::FrameHeightOption (), 
                                 video_sizes[size].height);  
      media_format.SetOptionBoolean (OpalVideoFormat::DynamicVideoQualityOption (), 
                                 TRUE);  
      media_format.SetOptionBoolean (OpalVideoFormat::AdaptivePacketDelayOption (), 
                                 TRUE);
      media_format.SetOptionInteger (OpalVideoFormat::FrameTimeOption (),
                                 (int)(90000 / frame_rate));
      media_format.SetOptionInteger (OpalVideoFormat::MaxBitRateOption (), 
                                 max_rx_bitrate * 1024);
      media_format.SetOptionInteger (OpalVideoFormat::TargetBitRateOption (), 
                                 max_tx_bitrate * 1024);
      media_format.AddOption(new OpalMediaOptionUnsigned(OpalVideoFormat::MaxFrameSizeOption(), 
                                 true, OpalMediaOption::NoMerge, 1400));

      OpalMediaFormat::SetRegisteredMediaFormat(media_format);
    }
  }

  initial_order = GetMediaFormatOrder ();
  initial_mask = GetMediaFormatMask ();

  // Skip Audio MediaFormats
  for (int i = 0 ; i < initial_order.GetSize () ; i++)
    if (OpalMediaFormat (initial_order [i]).GetDefaultSessionID () == OpalMediaFormat::DefaultAudioSessionID)
      *order += initial_order [i];

  for (int i = 0 ; i < initial_mask.GetSize () ; i++)
    if (OpalMediaFormat (initial_mask [i]).GetDefaultSessionID () == OpalMediaFormat::DefaultAudioSessionID)
      mask += initial_mask [i];

  media_formats_list.Remove (*order);

  // Configure Video Codec Order
  for (int i = 0 ; i < media_formats_list.GetSize () ; i++) {

    if (media_formats_list [i].GetDefaultSessionID () == OpalMediaFormat::DefaultVideoSessionID) {

      if (media_formats_list [i].IsTransportable())
        mask += media_formats_list [i];
      else 
        *order += media_formats_list [i];
    }
  }

  SetMediaFormatMask (mask);
  SetMediaFormatOrder (*order);
}


void
GMManager::SetUserInputMode ()
{
  h323EP->SetUserInputMode ();
  sipEP->SetUserInputMode ();
}


GMH323Endpoint *
GMManager::GetH323Endpoint ()
{
  return h323EP;
}


GMSIPEndpoint *
GMManager::GetSIPEndpoint ()
{
  return sipEP;
}


GMPCSSEndpoint *
GMManager::GetPCSSEndpoint ()
{
  return pcssEP;
}


PString
GMManager::GetCurrentAddress (PString protocol)
{
  OpalEndPoint *ep = NULL;

  PIPSocket::Address ip(PIPSocket::GetDefaultIpAny());
  WORD port = 0;

  ep = FindEndPoint (protocol.IsEmpty () ? "sip" : protocol);
  
  if (!ep)
    return PString ();
  
  if (!ep->GetListeners ().IsEmpty ())
    ep->GetListeners()[0].GetLocalAddress().GetIpAndPort (ip, port);
      
  return ip.AsString () + ":" + PString (port);
}


PString
GMManager::GetURL (PString protocol)
{
  GmAccount *account = NULL;
  PString url;
  gchar *account_url = NULL;

  if (protocol.IsEmpty ())
    return PString ();

  account = gnomemeeting_get_default_account ((gchar *)(const char *) protocol);

  if (account) {

    if (account->enabled)
      account_url = g_strdup_printf ("%s:%s@%s", (const char *) protocol, 
				     account->username, account->host);
    gm_account_delete (account);
  }

  if (!account_url)
    account_url = g_strdup_printf ("%s:%s", (const char *) protocol,
				   (const char *) GetCurrentAddress (protocol));

  url = account_url;
  g_free (account_url);

  return url;
}


void 
GMManager::StartAudioTester (gchar *audio_manager,
				  gchar *audio_player,
				  gchar *audio_recorder)
{
  PWaitAndSignal m(at_access_mutex);
  
  if (audio_tester)     
    delete (audio_tester);

  audio_tester =
    new GMAudioTester (audio_manager, audio_player, audio_recorder, *this);
}


void 
GMManager::StopAudioTester ()
{
  PWaitAndSignal m(at_access_mutex);

  if (audio_tester) {
   
    delete (audio_tester);
    audio_tester = NULL;
  }
}


GMVideoGrabber *
GMManager::CreateVideoGrabber (BOOL start_grabbing,
                               BOOL synchronous)
{
  PWaitAndSignal m(vg_access_mutex);

  if (video_grabber)
    delete (video_grabber);

  video_grabber = new GMVideoGrabber (start_grabbing, synchronous, *this);

  return video_grabber;
}


void
GMManager::RemoveVideoGrabber ()
{
  PWaitAndSignal m(vg_access_mutex);

  PTRACE(4, "GMManager\t Deleting grabber device");  //FIXME: There seems to be a problem in win32 since 2.0.x here
  if (video_grabber) {

    delete (video_grabber);
  }      
  PTRACE(4, "GMManager\t Sucessfully deleted grabber device");  //FIXME: There seems to be a problem in win32 since 2.0.x here
  video_grabber = NULL;
}


GMVideoGrabber *
GMManager::GetVideoGrabber ()
{
  PWaitAndSignal m(vg_access_mutex);

  if (video_grabber)
    video_grabber->Lock ();
  
  return video_grabber;
}


void
GMManager::UpdatePublishers (void)
{
#ifdef HAS_AVAHI
  PWaitAndSignal m(zcp_access_mutex);
  if (zcp)  
    zcp->Publish ();
#endif
}


void
GMManager::Register (GmAccount *account)
{
  PWaitAndSignal m(manager_access_mutex);

  if (manager == NULL)
    manager = new GMAccountsEndpoint (*this);

  if (account != NULL)
    manager->RegisterAccount (account);
}


void 
GMManager::PublishPresence (guint status)
{
  PWaitAndSignal m(manager_access_mutex);

  if (manager == NULL)
    manager = new GMAccountsEndpoint (*this);

  manager->PublishPresence (status);
}


void
GMManager::RemoveAccountsEndpoint ()
{
  PWaitAndSignal m(manager_access_mutex);

  if (manager)
    delete manager;
  manager = NULL;
}


void 
GMManager::SetCurrentCallToken (PString s)
{
  PWaitAndSignal m(ct_access_mutex);

  current_call_token = s;
}


PString 
GMManager::GetCurrentCallToken ()
{
  PWaitAndSignal m(ct_access_mutex);

  return current_call_token;
}


#ifdef HAS_AVAHI
void 
GMManager::CreateZeroconfClient ()
{
  PWaitAndSignal m(zcp_access_mutex);

  if (zcp)
    delete zcp;

  zcp = new GMZeroconfPublisher (*this);
}


void 
GMManager::RemoveZeroconfClient ()
{
  PWaitAndSignal m(zcp_access_mutex);

  if (zcp) {

    delete zcp;
    zcp = NULL;
  }
}
#endif

			     
void 
GMManager::CreateSTUNClient (BOOL display_progress,
			     BOOL display_config_dialog,
			     BOOL wait,
			     GtkWidget *parent)
{
  PWaitAndSignal m(sc_mutex);

  if (sc) 
    delete (sc);

  SetSTUNServer (PString ());
  
  /* Be a client for the specified STUN Server */
  sc = new GMStunClient (display_progress, 
			 display_config_dialog, 
			 wait,
			 parent,
			 *this);
}


void 
GMManager::RemoveSTUNClient ()
{
  PWaitAndSignal m(sc_mutex);

  if (sc) 
    delete (sc);
  
  SetSTUNServer (PString ());
  
  sc = NULL;

  ResetListeners ();
}


BOOL
GMManager::OnForwarded (OpalConnection &connection,
                        const PString & forward_party)
{
  /* Emit the signal */
  Ekiga::Runtime *runtime = GnomeMeeting::Process ()->GetRuntime (); // FIXME
  Ekiga::CallInfo info (connection, Ekiga::CallInfo::Forwarded);
  runtime->run_in_main (sigc::bind (call_event.make_slot (), info));

  return TRUE;
}



PSafePtr<OpalConnection> GMManager::GetConnection (PSafePtr<OpalCall> call, 
						    BOOL is_remote)
{
  PSafePtr<OpalConnection> connection = NULL;

  if (call == NULL)
    return connection;

  connection = call->GetConnection (is_remote ? 1:0);
  /* is_remote => SIP or H.323 connection
   * !is_remote => PCSS Connection
   */
  if (!connection
      || (is_remote && PIsDescendant(&(*connection), OpalPCSSConnection))
      || (!is_remote && !PIsDescendant(&(*connection), OpalPCSSConnection))) 
    connection = call->GetConnection (is_remote ? 0:1);

  return connection;
}


void GMManager::GetRemoteConnectionInfo (OpalConnection & connection,
					  gchar * & utf8_name,
					  gchar * & utf8_app,
					  gchar * & utf8_url)
{
  PINDEX idx;
  
  PString remote_url;
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
  idx = remote_name.Find ("@");
  if (idx != P_MAX_INDEX)
    remote_name = remote_name.Left (idx);
  
  /* The remote application */
  remote_app = connection.GetRemoteApplication (); 
  idx = remote_app.Find ("(");
  if (idx != P_MAX_INDEX)
    remote_app = remote_app.Left (idx);
  idx = remote_app.Find ("[");
  if (idx != P_MAX_INDEX)
    remote_app = remote_app.Left (idx);
  
  /* The remote url */
  remote_url = connection.GetRemotePartyCallbackURL ();

  /* UTF-8 Conversion */
  utf8_app = gnomemeeting_get_utf8 (remote_app.Trim ());
  utf8_name = gnomemeeting_get_utf8 (remote_name.Trim ());
  utf8_url = gnomemeeting_get_utf8 (remote_url);
}
  

void 
GMManager::GetCurrentConnectionInfo (gchar *&name, 
				      gchar *&url)
{
  PString call_token;

  PSafePtr <OpalCall> call = NULL;
  PSafePtr <OpalConnection> connection = NULL;
  
  gchar *app = NULL;

  call_token = GetCurrentCallToken ();
  call = FindCallWithLock (call_token);

  if (call != NULL) {

    connection = GetConnection (call, TRUE);

    if (connection != NULL) 
      GetRemoteConnectionInfo (*connection, name, app, url);
  }
}


BOOL
GMManager::OnIncomingConnection (OpalConnection &connection,
                                 unsigned reason,
                                 PString extra)
{
  BOOL res = FALSE;

  /* Act on the connection */
  switch (reason) {

  case 1:
    connection.ClearCall (OpalConnection::EndedByLocalBusy);
    res = FALSE;
    break;
    
  case 2:
    connection.ForwardCall (extra);
    res = FALSE;
    break;
    
  case 4:
    res = TRUE;
  default:

  case 0:
    res = OpalManager::OnIncomingConnection (connection, 0, NULL);
    break;
  }
  
  /* Update the current state if action is 0 or 4.
   * Show popup if action is 1 (show popup)
   */
  if (reason == 0 || reason == 4) {
    
    SetCallingState (GMManager::Called);
    SetCurrentCallToken (connection.GetCall ().GetToken ());
  }

  if (reason == 0) {

    /* Emit the signal */
    Ekiga::Runtime *runtime = GnomeMeeting::Process ()->GetRuntime (); // FIXME
    Ekiga::CallInfo info (connection, Ekiga::CallInfo::Incoming);
    runtime->run_in_main (sigc::bind (call_event.make_slot (), info));
  }

  return res;
}


void 
GMManager::OnEstablishedCall (OpalCall &call)
{
  /* Update the timers */
  RTPTimer.RunContinuous (PTimeInterval (1000));
  AvgSignalTimer.RunContinuous (PTimeInterval (50));

  /* Update internal state */
  SetCallingState (GMManager::Connected);
  SetCurrentCallToken (call.GetToken ());
}


void 
GMManager::OnEstablished (OpalConnection &connection)
{
  RTP_Session *audio_session = NULL;
  RTP_Session *video_session = NULL;

  /* Do nothing for the PCSS connection */
  if (PIsDescendant(&connection, OpalPCSSConnection)) {
    
    PTRACE (3, "GMManager\t Will establish the connection");
    OpalManager::OnEstablished (connection);
    return;
  }
  
  /* Asterisk sometimes forgets to send an INVITE, HACK */
  audio_session = connection.GetSession (OpalMediaFormat::DefaultAudioSessionID);
  video_session = connection.GetSession (OpalMediaFormat::DefaultVideoSessionID);
  if (audio_session) {
    audio_session->SetIgnoreOtherSources (TRUE);
    audio_session->SetIgnorePayloadTypeChanges (TRUE);
  }
  
  if (video_session) {
    video_session->SetIgnoreOtherSources (TRUE);
    video_session->SetIgnorePayloadTypeChanges (TRUE);
  }
  
  PTRACE (3, "GMManager\t Will establish the connection");
  OpalManager::OnEstablished (connection);

  /* Emit the signal */
  Ekiga::Runtime *runtime = GnomeMeeting::Process ()->GetRuntime (); // FIXME
  Ekiga::CallInfo info (connection, Ekiga::CallInfo::Established);
  runtime->run_in_main (sigc::bind (call_event.make_slot (), info));
}


void 
GMManager::OnClearedCall (OpalCall & call)
{
  if (GetCurrentCallToken() != PString::Empty() 
      && GetCurrentCallToken () != call.GetToken())
    return;
  
  /* Stop the Timers */
  NoIncomingMediaTimer.Stop ();
  
  /* we reset the no-data detection */
  RTPTimer.Stop ();
  AvgSignalTimer.Stop ();
  stats.Reset ();

  /* Play busy tone if we were connected */
  if (GetCallingState () == GMManager::Connected)
    pcssEP->PlaySoundEvent ("busy_tone_sound"); 

  /* Update internal state */
  SetCallingState (GMManager::Standby);
  SetCurrentCallToken ("");

  /* Reinitialize codecs */
  re_audio_codec = tr_audio_codec = re_video_codec = tr_video_codec = "";

  /* Try to update the devices use if some settings were changed 
     during the call */
  UpdateDevices ();
}


void
GMManager::OnReleased (OpalConnection & connection)
{ 
  PTimeInterval t;
  Ekiga::Runtime *runtime = GnomeMeeting::Process ()->GetRuntime (); // FIXME

  PTRACE (3, "GMManager\t Will release the connection");
  OpalManager::OnReleased (connection);

  /* Do nothing for the PCSS connection */
  if (PIsDescendant(&connection, OpalPCSSConnection)) {
    return;
  }

  /* Start time */
  if (connection.GetConnectionStartTime ().IsValid ())
    t = PTime () - connection.GetConnectionStartTime();
  
  if (t.GetSeconds () == 0 
      && !connection.IsOriginating ()
      && connection.GetCallEndReason ()!=OpalConnection::EndedByAnswerDenied) {

    Ekiga::CallInfo info (connection, Ekiga::CallInfo::Missed);
    runtime->run_in_main (sigc::bind (call_event.make_slot (), info));
  }

  /* The currently active call was cleared */
  if (GetCurrentCallToken() != PString::Empty() 
      && GetCurrentCallToken () != connection.GetCall().GetToken())
    return;

  /* Emit the signal */
  Ekiga::CallInfo info (connection, Ekiga::CallInfo::Cleared);
  runtime->run_in_main (sigc::bind (call_event.make_slot (), info));
}


void 
GMManager::OnHold (OpalConnection & connection)
{
  Ekiga::Runtime *runtime = GnomeMeeting::Process ()->GetRuntime (); // FIXME
  Ekiga::CallInfo info (connection, Ekiga::CallInfo::Held);
  runtime->run_in_main (sigc::bind (call_event.make_slot (), info));
}


void 
GMManager::OnMessageReceived (const SIPURL & _from,
                              const PString & _body)
{
  SIPURL from = _from;
  std::string display_name = (const char *) from.GetDisplayName ();
  from.AdjustForRequestURI ();
  std::string uri = (const char *) from.AsString ();
  std::string message = (const char *) _body;

  GMManager *ep = NULL;
  GMPCSSEndpoint *pcssEP = NULL;

  ep = GnomeMeeting::Process ()->GetManager ();
  pcssEP = ep->GetPCSSEndpoint ();
  pcssEP->PlaySoundEvent ("new_message_sound"); // FIXME use signals here too

  Ekiga::Runtime *runtime = GnomeMeeting::Process ()->GetRuntime (); // FIXME
  runtime->run_in_main (sigc::bind (im_received.make_slot (), display_name, uri, message));
}


void 
GMManager::OnMessageFailed (const SIPURL & _to,
                            SIP_PDU::StatusCodes reason)
{
  SIPURL to = _to;
  to.AdjustForRequestURI ();
  std::string uri = (const char *) to.AsString ();
  Ekiga::Runtime *runtime = GnomeMeeting::Process ()->GetRuntime (); // FIXME
  runtime->run_in_main (sigc::bind (im_failed.make_slot (), uri, 
                                    _("Could not send message")));
}


void 
GMManager::OnMessageSent (const PString & _to,
                          const PString & body)
{
  SIPURL to = _to;
  to.AdjustForRequestURI ();
  std::string uri = (const char *) to.AsString ();
  std::string message = (const char *) body;
  Ekiga::Runtime *runtime = GnomeMeeting::Process ()->GetRuntime (); // FIXME
  runtime->run_in_main (sigc::bind (im_sent.make_slot (), uri, message));
}


void
GMManager::SetUserNameAndAlias ()
{
  gchar *firstname = NULL;
  gchar *lastname = NULL;
  gchar *local_name = NULL;
  
  
  gnomemeeting_threads_enter ();
  firstname = gm_conf_get_string (PERSONAL_DATA_KEY "firstname");
  lastname = gm_conf_get_string (PERSONAL_DATA_KEY "lastname");
  gnomemeeting_threads_leave ();

  
  local_name = gnomemeeting_create_fullname (firstname, lastname);
  if (local_name)
    SetDefaultDisplayName (local_name);

  
  /* Update the H.323 endpoint user name and alias */
  h323EP->SetUserNameAndAlias ();

  
  /* Update the SIP endpoint user name and alias */
  sipEP->SetUserNameAndAlias ();


  g_free (local_name);
  g_free (firstname);
  g_free (lastname);
}


void
GMManager::SetPorts ()
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
    PTRACE (1, "Set TCP port range to " << setfill (':') 
            << atoi (tcp_couple [0]) << ":"
	    << atoi (tcp_couple [1])) << setfill (' ');
  }

  if (rtp_couple && rtp_couple [0] && rtp_couple [1]) {

    SetRtpIpPorts (atoi (rtp_couple [0]), atoi (rtp_couple [1]));
    PTRACE (1, "Set RTP port range to " << setfill (':')
            << atoi (rtp_couple [0]) << ":"
	    << atoi (rtp_couple [1])) << setfill (' ');
  }

  if (udp_couple && udp_couple [0] && udp_couple [1]) {

    SetUDPPorts (atoi (udp_couple [0]), atoi (udp_couple [1]));
    PTRACE (1, "Set UDP port range to " << setfill (':')
            << atoi (udp_couple [0]) << ":"
	    << atoi (udp_couple [1])) << setfill (' ');
  }

  g_free (tcp_port_range);
  g_free (udp_port_range);
  g_free (rtp_port_range);
  g_strfreev (tcp_couple);
  g_strfreev (udp_couple);
  g_strfreev (rtp_couple);
}


void
GMManager::Init ()
{
  OpalEchoCanceler::Params ec;
  OpalSilenceDetector::Params sd;
  OpalMediaFormatList list;
  
  int min_jitter = 20;
  int max_jitter = 500;
  int nat_method = 0;
  
  gboolean enable_sd = TRUE;  
  gboolean enable_ec = TRUE;  

  gchar *ip = NULL;
  
  /* GmConf cache */
  gnomemeeting_threads_enter ();
  min_jitter = gm_conf_get_int (AUDIO_CODECS_KEY "minimum_jitter_buffer");
  max_jitter = gm_conf_get_int (AUDIO_CODECS_KEY "maximum_jitter_buffer");
  enable_sd = gm_conf_get_bool (AUDIO_CODECS_KEY "enable_silence_detection");
  enable_ec = gm_conf_get_bool (AUDIO_CODECS_KEY "enable_echo_cancelation");
  autoStartTransmitVideo = 
    autoStartReceiveVideo = 
    gm_conf_get_bool (VIDEO_CODECS_KEY "enable_video");
  nat_method = gm_conf_get_int (NAT_KEY "method");
  ip = gm_conf_get_string (NAT_KEY "public_ip");
  gnomemeeting_threads_leave ();

  /* Setup ports */
  SetPorts ();

  /* Set Up IP translation */
  if (nat_method == 2 && ip)
    SetTranslationAddress (PString (ip));
  else
    SetTranslationAddress (PString ("0.0.0.0"));
  
  /* H.323 Endpoint */
  h323EP = new GMH323Endpoint (*this);
  h323EP->Init ();
  AddRouteEntry("pc:.*             = h323:<da>");
	
  /* SIP Endpoint */
  sipEP = new GMSIPEndpoint (*this);
  sipEP->Init ();
  AddRouteEntry("pc:.*             = sip:<da>");
  
  /* PC Sound System Endpoint */
  pcssEP = new GMPCSSEndpoint (*this);
  AddRouteEntry("h323:.* = pc:<da>");
  AddRouteEntry("sip:.* = pc:<da>");
  
  /* Jitter buffer */
  SetAudioJitterDelay (PMAX (min_jitter, 20), PMIN (max_jitter, 1000));
  
  /* Silence Detection */
  sd = GetSilenceDetectParams ();
  if (enable_sd)
    sd.m_mode = OpalSilenceDetector::AdaptiveSilenceDetection;
  else
    sd.m_mode = OpalSilenceDetector::NoSilenceDetection;
  SetSilenceDetectParams (sd);
  
  /* Echo Cancelation */
  ec = GetEchoCancelParams ();
  if (enable_ec)
    ec.m_mode = OpalEchoCanceler::Cancelation;
  else
    ec.m_mode = OpalEchoCanceler::NoCancelation;
  SetEchoCancelParams (ec);
  
  /* Update general devices configuration */
  UpdateDevices ();
  
  /* Set the User Name and Alias */  
  SetUserNameAndAlias ();

  /* Create a Zeroconf client */
#ifdef HAS_AVAHI
  CreateZeroconfClient ();
#endif

  /* Update publishers */
  UpdatePublishers ();

  /* Set initial codecs */
  SetMediaFormatOrder (PStringArray ());
  SetMediaFormatMask (PStringArray ());

  /* Reset the listeners */
  ResetListeners ();

  /* Register the various accounts */
  Register ();

  g_free (ip);
  
  stats.Reset();
}


void
GMManager::ResetListeners ()
{
  GtkWidget *main_window = NULL;
  GtkWidget *druid = NULL;

  GtkWidget *dialog = NULL;

  gchar *iface = NULL;
  gchar *ports = NULL;
  gchar **couple = NULL;

  WORD port = 0;
  WORD min_port = 5060;
  WORD max_port = 5080;

  BOOL success = FALSE;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  druid = GnomeMeeting::Process ()->GetDruidWindow ();

  gnomemeeting_threads_enter ();
  iface = gm_conf_get_string (PROTOCOLS_KEY "interface");
  gnomemeeting_threads_leave ();
  
  /* Create the H.323 and SIP listeners */
  if (h323EP) {
  
    gnomemeeting_threads_enter ();
    port = gm_conf_get_int (H323_KEY "listen_port");
    gnomemeeting_threads_leave ();
    
    h323EP->RemoveListener (NULL);
    if (!h323EP->StartListener (iface, port)) {

      gnomemeeting_threads_enter ();
      dialog = gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Error while starting the listener for the H.323 protocol"), _("You will not be able to receive incoming H.323 calls. Please check that no other program is already running on the port used by Ekiga."));
      if (gtk_window_is_active (GTK_WINDOW (druid)))
	gtk_widget_set_parent (dialog, druid);
      gnomemeeting_threads_leave ();
    }
  }

  if (sipEP) {
    
    gnomemeeting_threads_enter ();
    port = gm_conf_get_int (SIP_KEY "listen_port");
    ports = gm_conf_get_string (PORTS_KEY "udp_port_range");
    if (ports)
      couple = g_strsplit (ports, ":", 2);
    if (couple && couple [0]) {
      min_port = atoi (couple [0]);
    }
    if (couple && couple [1]) {
      max_port = atoi (couple [1]);
    }
    gnomemeeting_threads_leave ();
    
    sipEP->RemoveListener (NULL);
    if (!sipEP->StartListener (iface, port)) {
      
      port = min_port;
      while (port <= max_port && !success) {
       
        success = sipEP->StartListener (iface, port);
        port++;
      }

      if (!success) {

        gnomemeeting_threads_enter ();
        dialog = gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Error while starting the listener for the SIP protocol"), _("You will not be able to receive incoming SIP calls. Please check that no other program is already running on the ports used by Ekiga."));
        if (gtk_window_is_active (GTK_WINDOW (druid)))
          gtk_widget_set_parent (dialog, druid);
        gnomemeeting_threads_leave ();
      }
    }

    g_strfreev (couple);
    g_free (ports);
  }

  g_free (iface);
}


void
GMManager::OnIPChanged (PTimer &,
			INT)
{
  PIPSocket::InterfaceTable ifaces;
  OpalTransportAddress current_address;
  PIPSocket::Address current_ip;
  
  PString ip;
  PINDEX i = 0;
  
  BOOL found_ip = FALSE;
  
  gchar *iface = NULL;

  gnomemeeting_threads_enter ();
  iface = gm_conf_get_string (PROTOCOLS_KEY "interface");
  gnomemeeting_threads_leave ();

  /* Detect the valid interfaces */
  PIPSocket::GetInterfaceTable (ifaces);
  if (ifaces.GetSize () <= 0) {
  
    g_free (iface);
    return;
  }
  
  /* Is there a listener? */
  if (sipEP->GetListeners ().GetSize () > 0) 
    current_address = sipEP->GetListeners ()[0].GetLocalAddress ();
  else if (h323EP->GetListeners ().GetSize () > 0) 
    current_address = h323EP->GetListeners ()[0].GetLocalAddress ();
  else {

    g_free (iface);
    return;
  }
    
  /* Are we listening on the correct interface? */
  if (current_address.GetIpAddress (current_ip)) {

    while (i < ifaces.GetSize () && !found_ip) {

      ip = ifaces [i].GetAddress ().AsString ();

      if (ip == current_ip.AsString ())
	found_ip = TRUE;

      i++;
    }
  }

  if (!found_ip) {

    PTRACE (4, "Ekiga\tIP Address changed, updating listeners.");

    if (ifaces.GetSize () > 1) {
      
      /* This will update the UI and the listeners through a GConf
       * Notifier.
       */
      gnomemeeting_threads_enter ();
      GnomeMeeting::Process ()->DetectInterfaces ();
      gnomemeeting_threads_leave ();
    }
  }

  g_free (iface);
}


void
GMManager::OnNoIncomingMediaTimeout (PTimer &,
				      INT)
{
  if (gm_conf_get_bool (CALL_OPTIONS_KEY "clear_inactive_calls"))
    ClearAllCalls (H323Connection::EndedByTransportFail, FALSE);
}

BOOL
GMManager::SetDeviceVolume (PSoundChannel *sound_channel,
			     BOOL is_encoding,
			     unsigned int vol)
{
  return DeviceVolume (sound_channel, is_encoding, TRUE, vol);
}


BOOL
GMManager::GetDeviceVolume (PSoundChannel *sound_channel,
                                 BOOL is_encoding,
                                 unsigned int &vol)
{
  return DeviceVolume (sound_channel, is_encoding, FALSE, vol);
}


void
GMManager::OnClosedMediaStream (const OpalMediaStream & stream)
{
  OpalManager::OnClosedMediaStream (stream);

  if (pcssEP->GetMediaFormats ().FindFormat(stream.GetMediaFormat()) == P_MAX_INDEX)
    OnMediaStream ((OpalMediaStream &) stream, TRUE);
}


BOOL 
GMManager::OnOpenMediaStream (OpalConnection & connection,
			       OpalMediaStream & stream)
{
  if (!OpalManager::OnOpenMediaStream (connection, stream))
    return FALSE;

  if (pcssEP->GetMediaFormats ().FindFormat(stream.GetMediaFormat()) == P_MAX_INDEX)
    OnMediaStream (stream, FALSE);

  return TRUE;
}


BOOL 
GMManager::OnMediaStream (OpalMediaStream & stream,
			   BOOL is_closing)
{
  PString codec_name;
  BOOL is_encoding = FALSE;
  BOOL is_video = FALSE;
 
  is_video = (stream.GetSessionID () == OpalMediaFormat::DefaultVideoSessionID);
  is_encoding = !stream.IsSource (); // If the codec is from a source media
  				     // stream, the sink will be PCM or YUV
				     // and we are receiving.
  codec_name = stream.GetMediaFormat ().GetEncodingName ();
  codec_name = codec_name.ToUpper ();

  Ekiga::Runtime *runtime = GnomeMeeting::Process ()->GetRuntime (); // FIXME
  runtime->run_in_main (sigc::bind (media_stream_event.make_slot (), 
                                    std::string ((const char *) codec_name),
                                    is_video,
                                    is_encoding,
                                    is_closing));
    
  return TRUE;
}


void 
GMManager::UpdateRTPStats (PTime start_time,
                           RTP_Session *audio_session,
                           RTP_Session *video_session)
{
  PTimeInterval t;
  PTime now;
  
  OpalVideoMediaStream *stream = NULL;
  PString call_token;
  PSafePtr <OpalCall> call = NULL;
  PSafePtr <OpalConnection> connection = NULL;
  PVideoOutputDevice* device = NULL;

  int elapsed_seconds = 0;
  int re_bytes = 0;
  int tr_bytes = 0;
  int buffer_size = 0;
  int time_units = 8;

  t = now - stats.last_tick;
  elapsed_seconds = t.GetSeconds ();

  if (elapsed_seconds > 1) { /* To get more precision */

    if (audio_session) {

      re_bytes = audio_session->GetOctetsReceived ();
      tr_bytes = audio_session->GetOctetsSent ();

      stats.a_re_bandwidth = PMAX ((re_bytes - stats.re_a_bytes) / (1024.0 * elapsed_seconds), 0);
      stats.a_tr_bandwidth = PMAX ((tr_bytes - stats.tr_a_bytes) / (1024.0 * elapsed_seconds), 0);

      buffer_size = audio_session->GetJitterBufferSize ();
      time_units = audio_session->GetJitterTimeUnits ();
      
      stats.jitter_buffer_size = buffer_size / PMAX (time_units, 8);

      stats.re_a_bytes = re_bytes;
      stats.tr_a_bytes = tr_bytes;

      stats.total_packets += audio_session->GetPacketsReceived ();
      stats.lost_packets += audio_session->GetPacketsLost ();
      stats.late_packets += audio_session->GetPacketsTooLate ();
      stats.out_of_order_packets += audio_session->GetPacketsOutOfOrder (); 
    }

    if (video_session) {
      
      re_bytes = video_session->GetOctetsReceived ();
      tr_bytes = video_session->GetOctetsSent ();

      stats.v_re_bandwidth = PMAX ((re_bytes - stats.re_v_bytes) / (1024.0 * elapsed_seconds), 0);
      stats.v_tr_bandwidth = PMAX ((tr_bytes - stats.tr_v_bytes) / (1024.0 * elapsed_seconds), 0);

      stats.re_v_bytes = re_bytes;
      stats.tr_v_bytes = tr_bytes;
      
      stats.total_packets += video_session->GetPacketsReceived ();
      stats.lost_packets += video_session->GetPacketsLost ();
      stats.late_packets += video_session->GetPacketsTooLate ();
      stats.out_of_order_packets += video_session->GetPacketsOutOfOrder ();
    }

    stats.v_re_fps = 0;
    stats.re_width = 0;
    stats.re_height =  0;
    stats.v_tr_fps = 0;
    stats.tr_width = 0;
    stats.tr_height =  0;

    call_token = GetCurrentCallToken ();
    call = FindCallWithLock (call_token);

    if (call != NULL) {

      connection = GetConnection (call, FALSE);
      if (connection != NULL) {

        stream = (OpalVideoMediaStream *)connection->GetMediaStream (OpalMediaFormat::DefaultVideoSessionID, FALSE);
        if (stream != NULL) {

          // Get and calculate statistics for the Output Device
          device = stream->GetVideoOutputDevice ();
          if (device) {

            stats.v_re_fps = (int)((device->GetNumberOfFrames() - stats.v_re_frames) / elapsed_seconds);
            stats.v_re_frames = device->GetNumberOfFrames();
            if (stats.v_re_frames == 0) {

              stats.re_width = 0; 
              stats.re_height = 0;
            }
            else
              device->GetFrameSize(stats.re_width, stats.re_height);
          } 
        }

        stream = (OpalVideoMediaStream *)connection->GetMediaStream (OpalMediaFormat::DefaultVideoSessionID, TRUE);
        if (stream != NULL) {

          // Get and calculate statistics for the Preview Device
          device = stream->GetVideoOutputDevice ();
          if (device) {
            stats.v_tr_fps = (int)((device->GetNumberOfFrames() - stats.v_tr_frames) / elapsed_seconds);
            stats.v_tr_frames = device->GetNumberOfFrames();
            if (stats.v_tr_frames == 0) {
              stats.tr_width = 0; 
              stats.tr_height = 0;
            }
            else
              device->GetFrameSize(stats.tr_width, stats.tr_height);
          }
        }
      }
    }

    stats.last_tick = now;
    stats.start_time = start_time;
  }
}


void 
GMManager::OnAvgSignalTimeout (PTimer &,
                               INT)
{
  PSafePtr <OpalCall> call = NULL;
  PSafePtr <OpalConnection> connection = NULL;

  OpalRawMediaStream *audio_stream = NULL;
  Ekiga::Runtime *runtime = GnomeMeeting::Process ()->GetRuntime (); // FIXME

  float output = 0;
  float input = 0;
  
  call = FindCallWithLock (GetCurrentCallToken ());
  if (call != NULL) {

    connection = GetConnection (call, FALSE);

    if (connection != NULL) {

      audio_stream = 
        (OpalRawMediaStream *) 
        connection->GetMediaStream (OpalMediaFormat::DefaultAudioSessionID, 
                                    FALSE);
      if (audio_stream)
        output = (linear2ulaw (audio_stream->GetAverageSignalLevel ()) ^ 0xff) / 100.0;

      audio_stream = 
        (OpalRawMediaStream *) 
        connection->GetMediaStream (OpalMediaFormat::DefaultAudioSessionID, 
                                    TRUE);
      if (audio_stream)
        input = (linear2ulaw (audio_stream->GetAverageSignalLevel ()) ^ 0xff) / 100.0;
    } 
    runtime->run_in_main (sigc::bind (audio_signal_event.make_slot (), input, output));
  }
}


void 
GMManager::OnRTPTimeout (PTimer &, 
                         INT)
{
  float lost_packets_per = 0;
  float late_packets_per = 0;
  float out_of_order_packets_per = 0;

  PString remote_address;
  
  PSafePtr <OpalCall> call = NULL;
  PSafePtr <OpalConnection> connection = NULL;
  
  RTP_Session *audio_session = NULL;
  RTP_Session *video_session = NULL;
 
  /* If we didn't receive any audio and video data this time,
    then we start the timer */
  if (stats.a_re_bandwidth == 0 && stats.v_re_bandwidth == 0) {

    if (!NoIncomingMediaTimer.IsRunning ()) 
      NoIncomingMediaTimer.SetInterval (0, 30);
  }
  else
    NoIncomingMediaTimer.Stop ();

  /* Update the audio and video sessions statistics */
  {
    call = FindCallWithLock (GetCurrentCallToken ());
    if (call != NULL) {

      connection = GetConnection (call, TRUE);

      if (connection != NULL) {

        audio_session = 
          connection->GetSession (OpalMediaFormat::DefaultAudioSessionID);
        video_session = 
          connection->GetSession (OpalMediaFormat::DefaultVideoSessionID);

        UpdateRTPStats (connection->GetConnectionStartTime (),
                        audio_session,
                        video_session);
      }
    }
  }

  if (stats.total_packets > 0) {

    lost_packets_per = ((float) stats.lost_packets * 100.0
			/ (float) stats.total_packets);
    late_packets_per = ((float) stats.late_packets * 100.0
			/ (float) stats.total_packets);
    out_of_order_packets_per = ((float) stats.out_of_order_packets * 100.0
				/ (float) stats.total_packets);
    stats.lost_packets_per = PMIN (100, PMAX (0, lost_packets_per));
    stats.late_packets_per = PMIN (100, PMAX (0, late_packets_per));
    stats.out_of_order_packets_per = PMIN (100, PMAX (0, out_of_order_packets_per));
  }

  Ekiga::Runtime *runtime = GnomeMeeting::Process ()->GetRuntime (); // FIXME
  runtime->run_in_main (sigc::bind (call_stats_event.make_slot (), stats));
}


void 
GMManager::OnGatewayIPTimeout (PTimer &,
				    INT)
{
  PHTTPClient web_client ("Ekiga");
  PString html, ip_address;
  gboolean ip_checking = false;

  web_client.SetReadTimeout (PTimeInterval (0, 5));
  gdk_threads_enter ();
  ip_checking = gm_conf_get_bool (NAT_KEY "enable_ip_checking");
  gchar *ip_detector = gm_conf_get_string (NAT_KEY "public_ip_detector");
  gdk_threads_leave ();

  if (ip_detector != NULL
      && ip_checking
      && web_client.GetTextDocument (ip_detector, html)) {

    if (!html.IsEmpty ()) {

      PRegularExpression regex ("[0-9]*[.][0-9]*[.][0-9]*[.][0-9]*");
      PINDEX pos, len;

      if (html.FindRegEx (regex, pos, len)) 
	ip_address = html.Mid (pos,len);

    }
  }
  g_free (ip_detector);
  if (!ip_address.IsEmpty () && ip_checking) {

    gdk_threads_enter ();
    gm_conf_set_string (NAT_KEY "public_ip",
			(gchar *) (const char *) ip_address);
    gdk_threads_leave ();
  }

  GatewayIPTimer.RunContinuous (PTimeInterval (0, 0, 15));
}


BOOL 
GMManager::DeviceVolume (PSoundChannel *sound_channel,
			  BOOL is_encoding,
			  BOOL set, 
			  unsigned int &vol) 
{
  BOOL err = TRUE;

  if (sound_channel && GetCallingState () == GMManager::Connected) {

    
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

  return err;
}


BOOL
GMManager::CreateVideoInputDevice (const OpalConnection &con,
				    const OpalMediaFormat &format,
				    PVideoInputDevice * & device,
				    BOOL & auto_delete)
{
  GMVideoGrabber *vg = NULL;
  auto_delete = FALSE;

  vg = GetVideoGrabber ();
  if (!vg) {

    CreateVideoGrabber (FALSE, TRUE);
    vg = GetVideoGrabber ();
  }

  vg->StopGrabbing ();
  device = vg->GetInputDevice ();
  vg->Unlock ();
  
  return (device != NULL);
}


BOOL 
GMManager::CreateVideoOutputDevice(const OpalConnection & connection,
				    const OpalMediaFormat & format,
				    BOOL preview,
				    PVideoOutputDevice * & device,
				    BOOL & auto_delete)
{
  const PVideoDevice::OpenArgs & args = videoOutputDevice;

  /* Display of the input video, the display already
   * has the same size as the grabber 
   */
  if (preview) {

    GMVideoGrabber *vg = NULL;
    auto_delete = FALSE;

    vg = GetVideoGrabber ();

    if (vg) {

      device = vg->GetOutputDevice ();
      vg->Unlock ();
    }
    return (device != NULL);
  }
  else { /* Display of the video we are receiving */
    
    auto_delete = TRUE;
    device = PVideoOutputDevice::CreateDeviceByName (args.deviceName);
    
    if (device != NULL) {
      
      videoOutputDevice.width = 
	format.GetOptionInteger(OpalVideoFormat::FrameWidthOption (), 176);
      videoOutputDevice.height = 
	format.GetOptionInteger(OpalVideoFormat::FrameHeightOption (), 144);

      if (device->OpenFull (args, FALSE))
	return TRUE;

      delete device;
    }
  }

  return FALSE;
}


BOOL
GMManager::IsCallOnHold (PString callToken)
{
  PSafePtr<OpalCall> call = FindCallWithLock (callToken);
  OpalConnection *connection = NULL;
  
  if (call != NULL) {

    connection = GetConnection (call, TRUE);

    if (connection != NULL) {

      return connection->IsConnectionOnHold ();
    }
  }	
  
  return FALSE;
}


BOOL
GMManager::SetCallOnHold (PString callToken,
			   gboolean state)
{
  PSafePtr<OpalCall> call = FindCallWithLock (callToken);
  OpalConnection *connection = NULL;

  if (call != NULL) {

    connection = GetConnection (call, TRUE);

    if (connection != NULL) {

      if (state) 
	connection->HoldConnection ();
      else 
	connection->RetrieveConnection ();
    }
  }	
  
  return TRUE;
}


void
GMManager::SendDTMF (PString callToken,
		      PString dtmf)
{
  PSafePtr <OpalCall> call = NULL;
  PSafePtr <OpalConnection> connection = NULL;

  call = FindCallWithLock (callToken);

  if (call != NULL) {
    
    connection = GetConnection (call, TRUE);

    if (connection != NULL) 
      connection->SendUserInputTone(dtmf [0], 180);
  }
}


gboolean
GMManager::IsCallWithAudio (PString callToken)
{
  PSafePtr <OpalCall> call = NULL;
  PSafePtr <OpalConnection> connection = NULL;

  OpalMediaStream *stream = NULL;

  call = FindCallWithLock (callToken);

  if (call != NULL) {
    
    connection = GetConnection (call, TRUE);

    if (connection != NULL) {

      stream = 
	connection->GetMediaStream (OpalMediaFormat::DefaultAudioSessionID, 
				    FALSE);
      if (stream)
	return TRUE;
    }
  }
  
  return FALSE;
}


gboolean
GMManager::IsCallWithVideo (PString callToken)
{
  PSafePtr <OpalCall> call = NULL;
  PSafePtr <OpalConnection> connection = NULL;

  OpalMediaStream *stream = NULL;

  call = FindCallWithLock (callToken);

  if (call != NULL) {
    
    connection = GetConnection (call, TRUE);

    if (connection != NULL) {

      stream = 
	connection->GetMediaStream (OpalMediaFormat::DefaultVideoSessionID, 
				    FALSE);
      if (stream)
	return TRUE;
    }
  }
  
  return FALSE;
}


gboolean
GMManager::IsCallAudioPaused (PString callToken)
{
  OpalMediaStream *stream = NULL;
  PSafePtr<OpalCall> call = FindCallWithLock (callToken);
  PSafePtr<OpalConnection> connection = NULL;

  if (call != NULL) {

    connection = GetConnection (call, TRUE);

    if (connection != NULL) {

      stream = 
	connection->GetMediaStream (OpalMediaFormat::DefaultAudioSessionID, 
				    FALSE); // Sink media stream of the
      					    // SIP connection

      if (stream != NULL) {

	return stream->IsPaused();
      }
    }
  }	
  
  return FALSE;
}


gboolean
GMManager::IsCallVideoPaused (PString callToken)
{
  OpalMediaStream *stream = NULL;
  PSafePtr<OpalCall> call = FindCallWithLock (callToken);
  PSafePtr<OpalConnection> connection = NULL;
  
  if (call != NULL) {

    connection = GetConnection (call, TRUE);

    if (connection != NULL) {

      stream = 
	connection->GetMediaStream (OpalMediaFormat::DefaultVideoSessionID, 
				    FALSE); // Sink media stream of the
      					    // SIP connection

      if (stream != NULL) {

	return stream->IsPaused();
      }
    }
  }	
  
  return FALSE;
}


BOOL
GMManager::SetCallAudioPause (PString callToken, 
			       BOOL state)
{
  OpalMediaStream *stream = NULL;
  PSafePtr<OpalCall> call = FindCallWithLock (callToken);
  PSafePtr<OpalConnection> connection = NULL;

  if (call != NULL) {

    connection = GetConnection (call, TRUE);

    if (connection != NULL) {

      stream = 
	connection->GetMediaStream (OpalMediaFormat::DefaultAudioSessionID, 
				    FALSE); // Sink media stream of the
      					    // SIP connection

      if (stream != NULL) {

	stream->SetPaused (state);
	return TRUE;
      }
    }
  }

  return FALSE;
}


BOOL
GMManager::SetCallVideoPause (PString callToken, 
			       BOOL state)
{
  OpalMediaStream *stream = NULL;
  PSafePtr<OpalCall> call = FindCallWithLock (callToken);
  PSafePtr<OpalConnection> connection = NULL;

  if (call != NULL) {

    connection = GetConnection (call, TRUE);

    if (connection != NULL) {

      stream = 
	connection->GetMediaStream (OpalMediaFormat::DefaultVideoSessionID, 
				    FALSE); // Sink media stream of the
      					    // SIP connection

      if (stream != NULL) {

	stream->SetPaused (state);
	return TRUE;
      }
    }
  }

  return FALSE;
}


void
GMManager::OnMWIReceived (const PString & account,
                          const PString & mwi)
{
  Ekiga::Runtime *runtime = GnomeMeeting::Process ()->GetRuntime (); // FIXME
  PINDEX i = 0;
  PINDEX j = 0;
  int total = 0;

  PString key;
  PString val;
  PString *value = NULL;
  
  PWaitAndSignal m(mwi_access_mutex);

  /* Add MWI information for given account */
  value = mwiData.GetAt (key);

  /* Something changed for that account */
  if (value && *value != mwi) {
    mwiData.SetAt (account, new PString (mwi));

    /* Compute the total number of new voice mails */
    while (i < mwiData.GetSize ()) {

      value = NULL;
      key = mwiData.GetKeyAt (i);
      value = mwiData.GetAt (key);
      if (value) {

        val = *value;
        j = val.Find ("/");
        if (j != P_MAX_INDEX)
          val = value->Left (j);
        else 
          val = *value;

        total += val.AsInteger ();
      }
      i++;
    }

    runtime->run_in_main (sigc::bind (mwi_event.make_slot (), 
                                      (const char *) account, 
                                      (const char *) mwi,
                                      total));

    /* Sound event if new voice mail */
    pcssEP->PlaySoundEvent ("new_voicemail_sound");
  }
}


int
GMManager::GetRegisteredAccounts ()
{
  int number = 0;
  
  number = sipEP->GetRegisteredAccounts ();
  if (h323EP->H323EndPoint::IsRegisteredWithGatekeeper ())
    number++;

  return number;
}
