
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
#include "pcssendpoint.h"
#include "h323endpoint.h"
#include "sipendpoint.h"

#include "accounts.h"
#include "urlhandler.h"
#include "ils.h"
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
#include "lid.h"
#include "dbus_component.h"

#include "dialog.h"
#include "gm_conf.h"

#include <ptclib/http.h>
#include <ptclib/html.h>
#include <ptclib/pstun.h>


#define new PNEW


/* The class */
GMEndPoint::GMEndPoint ()
{
  /* Initialise the endpoint paramaters */
  video_grabber = NULL;
  SetCallingState (GMEndPoint::Standby);
  
#ifdef HAS_IXJ
  lid = NULL;
#endif
#if defined(HAS_HOWL) || defined(HAS_AVAHI)
  zcp = NULL;
#endif
  ils_client = NULL;

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
  
  manager = NULL;

  ILSTimer.SetNotifier (PCREATE_NOTIFIER (OnILSTimeout));
  ils_registered = false;

  RTPTimer.SetNotifier (PCREATE_NOTIFIER (OnRTPTimeout));
  GatewayIPTimer.SetNotifier (PCREATE_NOTIFIER (OnGatewayIPTimeout));
  GatewayIPTimer.RunContinuous (PTimeInterval (5));

  NoIncomingMediaTimer.SetNotifier (PCREATE_NOTIFIER (OnNoIncomingMediaTimeout));

  missed_calls = 0;

  OutgoingCallTimer.SetNotifier (PCREATE_NOTIFIER (OnOutgoingCall));

  h323EP = NULL;
  sipEP = NULL;
  pcssEP = NULL;

  PVideoDevice::OpenArgs video = GetVideoOutputDevice();
  video.deviceName = "GDKOUT";
  SetVideoOutputDevice (video);
  
  video = GetVideoOutputDevice();
  video.deviceName = "GDKIN";
  SetVideoPreviewDevice (video);
  
  video = GetVideoInputDevice();
  video.deviceName = "MovingLogo";
  SetVideoInputDevice (video);
}


GMEndPoint::~GMEndPoint ()
{
  /* Delete any GMStunClient thread. 
   */
  if (sc)
    delete (sc);
  
  /* Delete any ILS client which could be running */
  PWaitAndSignal m(ils_access_mutex);
  if (ils_client)
    delete (ils_client);

  /* Create a new one to unregister */
  if (ils_registered) {
    
    ils_client = new GMILSClient ();
    ils_client->Unregister ();
    delete (ils_client);
  }
  
  /* Remove any running audio tester, if any */
  if (audio_tester)
    delete (audio_tester);

  /* Stop the zeroconf publishing thread */
#if defined(HAS_HOWL) || defined(HAS_AVAHI)
  zcp_access_mutex.Wait ();
  if (zcp)
    delete (zcp);
  zcp_access_mutex.Signal ();
#endif

  manager_access_mutex.Wait ();
  if (manager)
    delete (manager);
  manager_access_mutex.Signal ();
}


BOOL
GMEndPoint::SetUpCall (const PString & call_addr,
		       PString & call_token)
{
  BOOL result = FALSE;
  
  lca_access_mutex.Wait();
  called_address = call_addr;
  lca_access_mutex.Signal();
  
  OutgoingCallTimer.RunContinuous (PTimeInterval (5));
  result = OpalManager::SetUpCall ("pc:*", call_addr, call_token, NULL);

  if (!result) {

    OutgoingCallTimer.Stop ();
    pcssEP->PlaySoundEvent ("busy_tone_sound");
  }
  
  return result;
}


BOOL
GMEndPoint::AcceptCurrentIncomingCall ()
{
  if (pcssEP) {

    pcssEP->AcceptCurrentIncomingCall ();
    return TRUE;
  }

  return FALSE;
}


void GMEndPoint::UpdateDevices ()
{
  BOOL preview = FALSE;
  gchar *device_name = NULL;

  /* Get the config settings */
  gnomemeeting_threads_enter ();
  preview = gm_conf_get_bool (VIDEO_DEVICES_KEY "enable_preview");
  device_name = gm_conf_get_string (VIDEO_DEVICES_KEY "input_device");
  gnomemeeting_threads_leave ();
  
  gnomemeeting_sound_daemons_suspend ();

  /* Do not change these values during calls */
  if (GetCallingState () == GMEndPoint::Standby) {

    /* Video preview */
    if (preview) 
      CreateVideoGrabber (TRUE, TRUE);
    else 
      RemoveVideoGrabber ();
      
    /* Update the video input device */
    PVideoDevice::OpenArgs video = GetVideoInputDevice();
    video.deviceName = device_name;
    SetVideoInputDevice (video);
    g_free (device_name);
  }
}


void 
GMEndPoint::SetCallingState (GMEndPoint::CallingState i)
{
  PWaitAndSignal m(cs_access_mutex);
  
  calling_state = i;
}


GMEndPoint::CallingState
GMEndPoint::GetCallingState (void)
{
  PWaitAndSignal m(cs_access_mutex);

  return calling_state;
}

/*
H323Connection * 
GMEndPoint::SetupTransfer (const PString & token,
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
GMEndPoint::AddVideoCapabilities ()
{
  int video_size = 0;

  video_size = gm_conf_get_int (VIDEO_DEVICES_KEY "size");
*/
  /* Add video capabilities */
  /*if (video_size == 1) {

    if (autoStartTransmitVideo && !autoStartReceiveVideo) {
      
    */  /* CIF Capability in first position */
     /* AddCapability (new H323_H261Capability (0, 1, FALSE, FALSE, 6217));
      AddCapability (new H323_H261Capability (1, 0, FALSE, FALSE, 6217));
    }
    else {

      *//* CIF Capability in first position */
     /* SetCapability (0, 1, new H323_H261Capability (0, 1, FALSE, FALSE, 6217));
      SetCapability (0, 1, new H323_H261Capability (1, 0, FALSE, FALSE, 6217));
    }
  }
  else {

    if (autoStartTransmitVideo && !autoStartReceiveVideo) {
      
      AddCapability (new H323_H261Capability (1, 0, FALSE, FALSE, 6217)); 
      AddCapability (new H323_H261Capability (0, 1, FALSE, FALSE, 6217));
    }
    else {

      SetCapability (0, 1, new H323_H261Capability (1, 0, FALSE, FALSE, 6217)); 
      SetCapability (0, 1, new H323_H261Capability (0, 1, FALSE, FALSE, 6217));
    }
  }
}
*/


OpalMediaFormatList
GMEndPoint::GetAvailableAudioMediaFormats ()
{
  OpalMediaFormatList list;
  OpalMediaFormatList full_list;
  OpalMediaFormatList media_formats;

  media_formats = pcssEP->GetMediaFormats ();
  list += OpalTranscoder::GetPossibleFormats (media_formats);

  for (int i = 0 ; i < list.GetSize () ; i++) {

    if (list [i].GetDefaultSessionID () == 1)
      full_list += list [i];
  }
  
  return full_list;
}


void
GMEndPoint::SetAllMediaFormats ()
{
  SetAudioMediaFormats ();
  SetVideoMediaFormats ();
  SetUserInputMode ();
}


void 
GMEndPoint::SetAudioMediaFormats ()
{
  PStringArray mask, order;
  GSList *codecs_data = NULL;
  
  gchar **couple = NULL;
  
  /* Read config settings */ 
  codecs_data = gm_conf_get_string_list (AUDIO_CODECS_KEY "list");
  

  /* Let's go */
  while (codecs_data) {
    
    couple = g_strsplit ((gchar *) codecs_data->data, "=", 0);

    if (couple && couple [0] && couple [1]) {
     
      if (!strcmp (couple [1], "1")) 
	order += couple [0];
      else
	mask += couple [0];
    }
    
    g_strfreev (couple);
    codecs_data = g_slist_next (codecs_data);
  }

  g_slist_free (codecs_data);

  SetMediaFormatMask (mask);
  SetMediaFormatOrder (order);
}


void 
GMEndPoint::SetVideoMediaFormats ()
{
  int size = 0;
  int vq = 0;
  int bitrate = 2;
  
  PStringArray order = GetMediaFormatOrder ();
  
  gnomemeeting_threads_enter ();
  vq = gm_conf_get_int (VIDEO_CODECS_KEY "transmitted_video_quality");
  bitrate = gm_conf_get_int (VIDEO_CODECS_KEY "maximum_video_bandwidth");
  size = gm_conf_get_int (VIDEO_DEVICES_KEY "size");
  gnomemeeting_threads_leave ();

  /* Will update the codec settings */
  vq = 25 - (int) ((double) vq / 100 * 24);

  OpalMediaFormat qcifmediaFormat (OPAL_H261_QCIF);
  qcifmediaFormat.SetOptionInteger (OpalVideoFormat::EncodingQualityOption, vq);
  qcifmediaFormat.SetOptionBoolean (OpalVideoFormat::DynamicVideoQualityOption, TRUE);
  qcifmediaFormat.SetOptionBoolean (OpalVideoFormat::AdaptivePacketDelayOption, TRUE);
  qcifmediaFormat.SetOptionInteger (OpalVideoFormat::TargetBitRateOption, bitrate * 8 * 1024);
  
  OpalMediaFormat cifmediaFormat (OPAL_H261_CIF);
  cifmediaFormat.SetOptionInteger (OpalVideoFormat::EncodingQualityOption, vq);
  cifmediaFormat.SetOptionBoolean (OpalVideoFormat::DynamicVideoQualityOption, TRUE);
  cifmediaFormat.SetOptionBoolean (OpalVideoFormat::AdaptivePacketDelayOption, TRUE);
  cifmediaFormat.SetOptionInteger (OpalVideoFormat::TargetBitRateOption, bitrate * 8 * 1024);
  
  OpalMediaFormat::SetRegisteredMediaFormat (qcifmediaFormat);
  OpalMediaFormat::SetRegisteredMediaFormat (cifmediaFormat);

  if (size == 0) {

    order += "H.261(QCIF)";
    order += "H.261(CIF)";
  }
  else {

    order += "H.261(CIF)";
    order += "H.261(QCIF)";
  }
  SetMediaFormatOrder (order);
}


void
GMEndPoint::SetUserInputMode ()
{
  h323EP->SetUserInputMode ();
  sipEP->SetUserInputMode ();
}


GMH323EndPoint *
GMEndPoint::GetH323EndPoint ()
{
  return h323EP;
}


GMSIPEndPoint *
GMEndPoint::GetSIPEndPoint ()
{
  return sipEP;
}


GMPCSSEndPoint *
GMEndPoint::GetPCSSEndPoint ()
{
  return pcssEP;
}


PString
GMEndPoint::GetCurrentIP (PString protocol)
{
  OpalEndPoint *ep = NULL;

  PIPSocket::Address ip(PIPSocket::GetDefaultIpAny());
  WORD port = 0;

  if (protocol.IsEmpty ())
    return PString ();
      
  ep = FindEndPoint (protocol);
  
  if (!ep)
    return PString ();
  
  if (!ep->GetListeners ().IsEmpty ())
    ep->GetListeners()[0].GetLocalAddress().GetIpAndPort (ip, port);
      
  return ip.AsString () + ":" + PString (port);
}


PString
GMEndPoint::GetURL (PString protocol)
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
				     account->login, account->host);
    gm_account_delete (account);
  }

  if (!account_url)
    account_url = g_strdup_printf ("%s:%s", (const char *) protocol,
				   (const char *) GetCurrentIP (protocol));

  url = account_url;
  g_free (account_url);

  return url;
}


BOOL
GMEndPoint::TranslateIPAddress(PIPSocket::Address &local_address, 
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

    return TRUE;
  }

  return FALSE;
}


void 
GMEndPoint::StartAudioTester (gchar *audio_manager,
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
GMEndPoint::StopAudioTester ()
{
  PWaitAndSignal m(at_access_mutex);

  if (audio_tester) {
   
    delete (audio_tester);
    audio_tester = NULL;
  }
}


GMVideoGrabber *
GMEndPoint::CreateVideoGrabber (BOOL start_grabbing,
				    BOOL synchronous)
{
  PWaitAndSignal m(vg_access_mutex);

  if (video_grabber)
    delete (video_grabber);

  video_grabber = new GMVideoGrabber (start_grabbing, synchronous);

  return video_grabber;
}


void
GMEndPoint::RemoveVideoGrabber ()
{
  PWaitAndSignal m(vg_access_mutex);

  if (video_grabber) {

    delete (video_grabber);
  }      
  video_grabber = NULL;
}


GMVideoGrabber *
GMEndPoint::GetVideoGrabber ()
{
  PWaitAndSignal m(vg_access_mutex);

  if (video_grabber)
    video_grabber->Lock ();
  
  return video_grabber;
}


void
GMEndPoint::UpdatePublishers (void)
{
  BOOL ilsreg = FALSE; 

#if defined(HAS_HOWL) || defined(HAS_AVAHI)
  PWaitAndSignal m(zcp_access_mutex);
  if (zcp)  
    zcp->Publish ();
#endif

  gnomemeeting_threads_enter ();
  ilsreg = gm_conf_get_bool (LDAP_KEY "enable_registering");
  gnomemeeting_threads_leave ();
  ILSTimer.RunContinuous (PTimeInterval (5));
}


void
GMEndPoint::Register (GmAccount *account)
{
  PWaitAndSignal m(manager_access_mutex);

  if (manager)
    delete manager;
  manager = new GMAccountsManager (account);
}


void 
GMEndPoint::SetCurrentCallToken (PString s)
{
  PWaitAndSignal m(ct_access_mutex);

  current_call_token = s;
}


PString 
GMEndPoint::GetCurrentCallToken ()
{
  PWaitAndSignal m(ct_access_mutex);

  return current_call_token;
}


void 
GMEndPoint::SetSTUNServer ()
{
  if (sc)
    delete (sc);
  
  /* Be a client for the specified STUN Server */
  sc = new GMStunClient (TRUE);
}


BOOL
GMEndPoint::OnForwarded (OpalConnection &,
			 const PString & forward_party)
{
  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;
  GtkWidget *history_window = NULL;

  gchar *msg = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();

  gnomemeeting_threads_enter ();
  msg = g_strdup_printf (_("Forwarding call to %s"),
			 (const char*) forward_party);
  gm_main_window_flash_message (main_window, msg);
  gm_history_window_insert (history_window, msg);
  gnomemeeting_threads_leave ();
  g_free (msg);

  return TRUE;
}



PSafePtr<OpalConnection> GMEndPoint::GetConnection (PSafePtr<OpalCall> call, 
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


void GMEndPoint::GetRemoteConnectionInfo (OpalConnection & connection,
					  gchar * & utf8_name,
					  gchar * & utf8_app,
					  gchar * & utf8_url)
{
  PINDEX idx;
  
  PString remote_url;
  PString remote_name;
  PString remote_app;
  PString remote_alias;

  GmContact *contact = NULL;
  GSList *contacts = NULL;
  

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
  
  utf8_app = gnomemeeting_get_utf8 (remote_app.Trim ());
  utf8_name = gnomemeeting_get_utf8 (remote_name.Trim ());
  utf8_url = gnomemeeting_get_utf8 (remote_url);

  /* Check if that URL appears in the address book */
  int nbr = 0;
  contacts = 
    gnomemeeting_addressbook_get_contacts (NULL, nbr, FALSE, 
					   NULL, utf8_url, NULL, NULL);
  if (contacts) {
    
    contact = GM_CONTACT (contacts->data);
    g_free (utf8_name);
    utf8_name = g_strdup (contact->fullname);
    g_slist_foreach (contacts, (GFunc) gm_contact_delete, NULL);
    g_slist_free (contacts);
  }
}


BOOL
GMEndPoint::OnIncomingConnection (OpalConnection &connection,
				  int reason,
				  PString extra)
{
  BOOL res = FALSE;

  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;
  GtkWidget *history_window = NULL;
  GtkWidget *tray = NULL;
  
  gchar *msg = NULL;
  gchar *short_reason = NULL;
  gchar *long_reason = NULL;

  gchar *utf8_name = NULL;
  gchar *utf8_app = NULL;
  gchar *utf8_url = NULL;

  GetRemoteConnectionInfo (connection, utf8_name, utf8_app, utf8_url);

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  tray = GnomeMeeting::Process ()->GetTray ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();

  /* Update the log and status bar */
  msg = g_strdup_printf (_("Call from %s"), (const char *) utf8_name);
  gnomemeeting_threads_enter ();
  gm_main_window_flash_message (main_window, msg);
  gm_chat_window_push_info_message (chat_window, NULL, msg);
  gm_history_window_insert (history_window, msg);
  gnomemeeting_threads_leave ();
  g_free (msg);

  /* Act on the connection */
  switch (reason) {

  case 1:
    connection.ClearCall (OpalConnection::EndedByLocalBusy);
    res = FALSE;
    short_reason = g_strdup (_("Rejecting incoming call"));
    long_reason = 
      g_strdup_printf (_("Rejecting incoming call from %s"), utf8_name);
    break;
    
  case 2:
    connection.ForwardCall (extra);
    res = FALSE;
    short_reason = g_strdup (_("Forwarding incoming call"));
    long_reason = 
      g_strdup_printf (_("Forwarding incoming call from %s to %s"), 
		       utf8_name, (const char *) extra);
    break;
    
  case 4:
    res = TRUE;
    short_reason = g_strdup (_("Auto-Answering incoming call"));
    long_reason = g_strdup_printf (_("Auto-Answering incoming call from %s"),
				   (const char *) utf8_name);
  default:
  case 0:
    res = OpalManager::OnIncomingConnection (connection);
    break;
  }
  
  /* Display the action message */
  gnomemeeting_threads_enter ();
  if (short_reason) 
    gm_main_window_flash_message (main_window, short_reason);
  if (long_reason)
    gm_history_window_insert (history_window, long_reason);
  gnomemeeting_threads_leave ();
  
  /* Update the current state and show popup if action is 1 */
  if (reason == 0) {

    SetCallingState (GMEndPoint::Called);
    SetCurrentCallToken (connection.GetCall ().GetToken ());

    /* Update the UI */
    gnomemeeting_threads_enter ();
    gm_tray_update_calling_state (tray, GMEndPoint::Called);
    gm_main_window_update_calling_state (main_window, GMEndPoint::Called);
    gm_chat_window_update_calling_state (chat_window, 
					 NULL, 
					 NULL, 
					 GMEndPoint::Called);

    gm_main_window_incoming_call_dialog_show (main_window,
					      utf8_name, 
					      utf8_app, 
					      utf8_url);
    gnomemeeting_threads_leave ();
  }

  g_free (utf8_app);
  g_free (utf8_name);
  g_free (utf8_url);

  return res;
}


void 
GMEndPoint::OnEstablishedCall (OpalCall &call)
{
  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;
  GtkWidget *tray = NULL;

  BOOL stay_on_top = FALSE;
  BOOL forward_on_busy = FALSE;

  IncomingCallMode icm = AVAILABLE;
  
  /* Get the widgets */
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  tray = GnomeMeeting::Process ()->GetTray ();

  /* Get the config settings */
  gnomemeeting_threads_enter ();
  stay_on_top = gm_conf_get_bool (VIDEO_DISPLAY_KEY "stay_on_top");
  forward_on_busy = gm_conf_get_bool (CALL_FORWARDING_KEY "forward_on_busy");
  gnomemeeting_threads_leave ();
  
  /* Start refreshing the stats */
  RTPTimer.RunContinuous (PTimeInterval (0, 1));

  /* Stop the Timers */
  OutgoingCallTimer.Stop ();

  /* Update internal state */
  SetCallingState (GMEndPoint::Connected);
  SetCurrentCallToken (call.GetToken ());
  
  /* Update the GUI */
  gnomemeeting_threads_enter ();
  if (called_address.IsEmpty ()) 
    gm_main_window_set_call_url (main_window, GMURL ().GetDefaultURL ());
  gm_main_window_update_calling_state (main_window, GMEndPoint::Connected);
  gm_tray_update_calling_state (tray, GMEndPoint::Connected);
  gm_main_window_set_stay_on_top (main_window, stay_on_top);
  gm_main_window_update_calling_state (main_window, GMEndPoint::Connected);
  gm_tray_update_calling_state (tray, GMEndPoint::Connected);
  gm_tray_update (tray, GMEndPoint::Connected, icm, forward_on_busy);
  gnomemeeting_threads_leave ();

  /* Update the various information publishers */
  UpdatePublishers ();
}


void 
GMEndPoint::OnEstablished (OpalConnection &connection)
{
  gchar *utf8_url = NULL;
  gchar *utf8_app = NULL;
  gchar *utf8_name = NULL;
  gchar *utf8_protocol_prefix = NULL;
  gchar *msg = NULL;

  GtkWidget *history_window = NULL;
  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;
  GObject *dbus_component = NULL;

  /* Get the widgets */
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  dbus_component = GnomeMeeting::Process ()->GetDbusComponent ();

  /* Do nothing for the PCSS connection */
  if (PIsDescendant(&connection, OpalPCSSConnection)) {
    
    PTRACE (3, "GMEndPoint\t Will establish the connection");
    OpalManager::OnEstablished (connection);
    return;
  }
  
  /* Update internal state */
  GetRemoteConnectionInfo (connection, utf8_name, utf8_app, utf8_url);
  utf8_protocol_prefix = gnomemeeting_get_utf8 (connection.GetEndPoint ().GetPrefixName ().Trim ());
  gnomemeeting_threads_enter ();
  if (GetCallingState () != GMEndPoint::Connected)
    gm_history_window_insert (history_window, _("Connected with %s using %s"), 
			      utf8_name, utf8_app);
  msg = g_strdup_printf (_("Connected with %s"), utf8_name);
  gm_main_window_set_status (main_window, msg);
  gm_main_window_flash_message (main_window, msg);
  gm_chat_window_push_info_message (chat_window, NULL, msg);
  gm_chat_window_update_calling_state (chat_window, 
				       utf8_name,
				       utf8_url, 
				       GMEndPoint::Connected);
  gnomemeeting_dbus_component_set_call_info (dbus_component,
					     connection.GetToken (), utf8_name,
					     utf8_app, utf8_url,
					     utf8_protocol_prefix);
  gnomemeeting_threads_leave ();
  
  if (!connection.IsOriginating ()) {
    
    // FIXME
    PWaitAndSignal m(lca_access_mutex);

    called_address = PString ();
  }

  g_free (utf8_name);
  g_free (utf8_app);
  g_free (utf8_url);
  g_free (utf8_protocol_prefix);
  g_free (msg);

  PTRACE (3, "GMEndPoint\t Will establish the connection");
  OpalManager::OnEstablished (connection);
}


void 
GMEndPoint::OnClearedCall (OpalCall & call)
{
  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;
  GtkWidget *tray = NULL;
  
  BOOL reg = FALSE;
  BOOL forward_on_busy = FALSE;
  IncomingCallMode icm = AVAILABLE;
  ViewMode m = SOFTPHONE;
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  tray = GnomeMeeting::Process ()->GetTray ();
  
  if (GetCurrentCallToken() != call.GetToken())
    return;
  
  /* Get the config settings */
  gnomemeeting_threads_enter ();
  icm = (IncomingCallMode)
    gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode");
  forward_on_busy = gm_conf_get_bool (CALL_FORWARDING_KEY "forward_on_busy");
  reg = gm_conf_get_bool (LDAP_KEY "enable_registering");
  m = (ViewMode) gm_conf_get_int (USER_INTERFACE_KEY "main_window/view_mode");
  gnomemeeting_threads_leave ();
  
  /* Stop the Timers */
  OutgoingCallTimer.Stop ();
  NoIncomingMediaTimer.Stop ();
  
  gnomemeeting_sound_daemons_resume ();
  
  /* No need to do all that if we are simply receiving an incoming call
     that was rejected because of DND */
  if (GetCallingState () != GMEndPoint::Called
      && GetCallingState () != GMEndPoint::Calling) 
    UpdatePublishers();

  /* Update internal state */
  SetCallingState (GMEndPoint::Standby);
  SetCurrentCallToken ("");

  /* Play busy tone */
  pcssEP->PlaySoundEvent ("busy_tone_sound"); 

  /* Try to update the devices use if some settings were changed 
     during the call */
  UpdateDevices ();

  /* Update the various parts of the GUI */
  gnomemeeting_threads_enter ();
  gm_main_window_set_stay_on_top (main_window, FALSE);
  gm_main_window_update_calling_state (main_window, GMEndPoint::Standby);
  gm_tray_update_calling_state (tray, GMEndPoint::Standby);
  gm_chat_window_update_calling_state (chat_window, 
				       NULL, 
				       NULL, 
				       GMEndPoint::Standby);
  gm_tray_update (tray, GMEndPoint::Standby, icm, forward_on_busy);
  gm_main_window_set_status (main_window, _("Standby"));
  gm_main_window_set_account_info (main_window, 
				   GetRegisteredAccounts ()); 
  gm_main_window_clear_stats (main_window);
  gm_main_window_update_logo (main_window);
  gm_main_window_set_view_mode (main_window, m);
  gnomemeeting_threads_leave ();
  
}


void
GMEndPoint::OnReleased (OpalConnection & connection)
{ 
  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;
  GtkWidget *history_window = NULL;
  
  gchar *msg_reason = NULL;
  
  gchar *utf8_url = NULL;
  gchar *utf8_name = NULL;
  gchar *utf8_app = NULL;

  gchar *info = NULL;

  PTimeInterval t;

  /* Get the widgets */
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();

  /* we reset the no-data detection */
  RTPTimer.Stop ();
  stats.Reset ();
  
  /* Do nothing for the PCSS connection */
  if (PIsDescendant(&connection, OpalPCSSConnection) || GetCallingState () == GMEndPoint::Standby) {
    
    PTRACE (3, "GMEndPoint\t Will release the connection");
    OpalManager::OnReleased (connection);
    return;
  }
  
  /* Start time */
  if (connection.GetConnectionStartTime ().IsValid ())
    t = PTime () - connection.GetConnectionStartTime();
  
  switch (connection.GetCallEndReason ()) {

  case OpalConnection::EndedByLocalUser :
    msg_reason = g_strdup (_("Local user cleared the call"));
    break;
  case OpalConnection::EndedByNoAccept :
    msg_reason = g_strdup (_("Local user rejected the call"));
    break;
  case OpalConnection::EndedByAnswerDenied :
    msg_reason = g_strdup (_("Local user rejected the call"));
    break;
  case OpalConnection::EndedByRemoteUser :
    msg_reason = g_strdup (_("Remote user cleared the call"));
    break;
  case OpalConnection::EndedByRefusal :
    msg_reason = g_strdup (_("Remote user rejected the call"));
    break;
  case OpalConnection::EndedByNoAnswer :
    msg_reason = g_strdup (_("Call not answered in the required time"));
    break;
  case OpalConnection::EndedByCallerAbort :
    msg_reason = g_strdup (_("Remote user has stopped calling"));
    break;
  case OpalConnection::EndedByTransportFail :
    msg_reason = g_strdup (_("Abnormal call termination"));
    break;
  case OpalConnection::EndedByConnectFail :
    msg_reason = g_strdup (_("Could not connect to remote host"));
    break;
  case OpalConnection::EndedByGatekeeper :
    msg_reason = g_strdup (_("The Gatekeeper cleared the call"));
    break;
  case OpalConnection::EndedByNoUser :
    msg_reason = g_strdup (_("User not found"));
    break;
  case OpalConnection::EndedByNoBandwidth :
    msg_reason = g_strdup (_("Insufficient bandwidth"));
    break;
  case OpalConnection::EndedByCapabilityExchange :
    msg_reason = g_strdup (_("No common codec"));
    break;
  case OpalConnection::EndedByCallForwarded :
    msg_reason = g_strdup (_("Call forwarded"));
    break;
  case OpalConnection::EndedBySecurityDenial :
    msg_reason = g_strdup (_("Security check failed"));
    break;
  case OpalConnection::EndedByLocalBusy :
    msg_reason = g_strdup (_("Local user is busy"));
    break;
  case OpalConnection::EndedByLocalCongestion :
    msg_reason = g_strdup (_("Congested link to remote party"));
    break;
  case OpalConnection::EndedByRemoteBusy :
    msg_reason = g_strdup (_("Remote user is busy"));
    break;
  case OpalConnection::EndedByRemoteCongestion :
    msg_reason = g_strdup (_("Congested link to remote party"));
    break;
  case OpalConnection::EndedByUnreachable :
    msg_reason = g_strdup (_("Remote user is unreachable"));
    break;
  case OpalConnection::EndedByNoEndPoint :
    msg_reason = g_strdup (_("Remote user is unreachable"));
    break;
  case OpalConnection::EndedByHostOffline :
    msg_reason = g_strdup (_("Remote host is offline"));
    break;
  case OpalConnection::EndedByTemporaryFailure :
    msg_reason = g_strdup (_("Remote user is offline"));
    break;

  default :
    msg_reason = g_strdup (_("Call completed"));
  }

  
  /* Update the calls history */
  GetRemoteConnectionInfo (connection, utf8_name, utf8_app, utf8_url);
  
  gnomemeeting_threads_enter ();
  if (t.GetSeconds () == 0 
      && !connection.IsOriginating ()
      && connection.GetCallEndReason ()!=OpalConnection::EndedByAnswerDenied) {

    gm_calls_history_add_call (MISSED_CALL, 
			       utf8_name,
			       utf8_url,
			       "0",
			       msg_reason,
			       utf8_app);
    mc_access_mutex.Wait ();
    missed_calls++;
    mc_access_mutex.Signal ();

    info = g_strdup_printf (_("Missed calls: %d - Voice Mails: %s"),
			    GetMissedCallsNumber (),
			    (const char *) GetMWI ());
    gm_main_window_push_info_message (main_window, info);
    g_free (info);
  }
  else
    if (!connection.IsOriginating ())
      gm_calls_history_add_call (RECEIVED_CALL, 
				 utf8_name,
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
  gm_main_window_flash_message (main_window, msg_reason);
  gm_chat_window_push_info_message (chat_window, NULL, "");
  gnomemeeting_threads_leave ();

  g_free (utf8_app);
  g_free (utf8_name);
  g_free (utf8_url);  
  g_free (msg_reason);

  PTRACE (3, "GMEndPoint\t Will release the connection");
  OpalManager::OnReleased (connection);
}


void 
GMEndPoint::OnHold (OpalConnection & connection)
{
  GtkWidget *main_window = NULL;
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  gnomemeeting_threads_enter ();
  gm_main_window_set_call_hold (main_window,
				connection.IsConnectionOnHold ());
  gnomemeeting_threads_leave ();
}


void
GMEndPoint::OnUserInputString (OpalConnection & connection,
			       const PString & value)
{
  GtkWidget *chat_window = NULL;
  
  PString val;
  PString remote = connection.GetRemotePartyName ();
  PINDEX bracket;

  
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  
  
  /* The remote party name has to be converted to UTF-8, but not
     the text */
  gchar *utf8_remote = NULL;

  /* The MCU sends MSG[remote] value as message, 
     check if we are not using the MCU */
  bracket = value.Find("[");

  if ((bracket != P_MAX_INDEX) && (bracket == 3)) {
    
    remote = value.Mid (bracket + 1, value.Find ("] ") - 4);
    bracket = value.Find ("] ");
    val = value.Mid (bracket + 1);
  }
  else {

    if (value.Find ("MSG") != P_MAX_INDEX)
      val = value.Mid (3);
    else
      return;
  }

  /* If the remote name can be converted, use the conversion,
     else (Netmeeting), suppose it is ISO-8859-1 */  
  remote = gnomemeeting_pstring_cut (remote);
  if (g_utf8_validate ((gchar *) (const unsigned char*) remote, -1, NULL))
    utf8_remote = g_strdup ((char *) (const unsigned char *) (remote));
  else
    utf8_remote = gnomemeeting_from_iso88591_to_utf8 (remote);

  gnomemeeting_threads_enter ();
  //if (utf8_remote && strcmp (utf8_remote, "")) 
    //gnomemeeting_text_chat_insert (chat_window, utf8_remote, val, 1);
  
  if (!GTK_WIDGET_VISIBLE (chat_window))
    gm_conf_set_bool (USER_INTERFACE_KEY "main_window/show_chat_window", true);

  g_free (utf8_remote);
  gnomemeeting_threads_leave ();
}


void 
GMEndPoint::SavePicture (void)
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
GMEndPoint::SetUserNameAndAlias ()
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
GMEndPoint::SetPorts ()
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
GMEndPoint::Init ()
{
  GtkWidget *main_window = NULL;
  GtkWidget *prefs_window = NULL;

  OpalSilenceDetector::Params sd;
  OpalMediaFormatList list;
  
  int min_jitter = 20;
  int max_jitter = 500;
  
  gboolean enable_sd = TRUE;  
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();

  /* GmConf cache */
  gnomemeeting_threads_enter ();
  min_jitter = gm_conf_get_int (AUDIO_CODECS_KEY "minimum_jitter_buffer");
  max_jitter = gm_conf_get_int (AUDIO_CODECS_KEY "maximum_jitter_buffer");
  enable_sd = gm_conf_get_bool (AUDIO_CODECS_KEY "enable_silence_detection");
  gnomemeeting_threads_leave ();
  
  /* Update the internal state */
  autoStartTransmitVideo = 
    gm_conf_get_bool (VIDEO_CODECS_KEY "enable_video_transmission");
  autoStartReceiveVideo = 
    gm_conf_get_bool (VIDEO_CODECS_KEY "enable_video_reception");

  /* Setup ports */
  SetPorts ();
  
  /* H.323 EndPoint */
  h323EP = new GMH323EndPoint (*this);
  h323EP->Init ();
  AddRouteEntry("pc:.*             = h323:<da>");
	
  /* SIP EndPoint */
  sipEP = new GMSIPEndPoint (*this);
  sipEP->Init ();
  AddRouteEntry("pc:.*             = sip:<da>");
  
  /* PC Sound System EndPoint */
  pcssEP = new GMPCSSEndPoint (*this);
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
  
  /* Update general devices configuration */
  UpdateDevices ();
  
  /* Set the User Name and Alias */  
  SetUserNameAndAlias ();

  /* Start Listeners */
  StartListeners ();
  
  /* FIXME: not clean */
#if defined(HAS_HOWL) || defined(HAS_AVAHI)
  zcp_access_mutex.Wait ();
  zcp = new GMZeroconfPublisher ();
  zcp_access_mutex.Signal ();
#endif

  UpdatePublishers ();

  /* Update the codecs list */
  //FIXME Move to UpdateGUI in GnomeMeeting
  list = GetAvailableAudioMediaFormats ();
  gm_prefs_window_update_audio_codecs_list (prefs_window, list);
  
  /* Set the media formats */
  SetAllMediaFormats ();
  
  /* Register the various accounts */
  Register ();
}


void
GMEndPoint::StartListeners ()
{
  GtkWidget *main_window = NULL;

  gchar *iface = NULL;
  WORD port = 0;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  
  gnomemeeting_threads_enter ();
  iface = gm_conf_get_string (PROTOCOLS_KEY "interface");
  gnomemeeting_threads_leave ();
  

  if (h323EP) {
  
    gnomemeeting_threads_enter ();
    port = gm_conf_get_int (H323_KEY "listen_port");
    gnomemeeting_threads_leave ();
    
    if (!h323EP->StartListener (iface, port)) {

      gnomemeeting_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Error while starting the listener for the H.323 protocol"), _("You will not be able to receive incoming H.323 calls. Please check that no other program is already running on the port used by GnomeMeeting."));
      gnomemeeting_threads_leave ();
    }
  }

  if (sipEP) {
    
    gnomemeeting_threads_enter ();
    port = gm_conf_get_int (SIP_KEY "listen_port");
    gnomemeeting_threads_leave ();
    
    if (!sipEP->StartListener (iface, port)) {
      
      gnomemeeting_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Error while starting the listener for the SIP protocol"), _("You will not be able to receive incoming SIP calls. Please check that no other program is already running on the port used by GnomeMeeting."));
      gnomemeeting_threads_leave ();
    }
  }

  g_free (iface);
}


void
GMEndPoint::OnNoIncomingMediaTimeout (PTimer &,
				      INT)
{
  if (gm_conf_get_bool (CALL_OPTIONS_KEY "clear_inactive_calls"))
    ClearAllCalls (H323Connection::EndedByTransportFail, FALSE);
}


void 
GMEndPoint::OnILSTimeout (PTimer &,
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
GMEndPoint::SetDeviceVolume (PSoundChannel *sound_channel,
			     BOOL is_encoding,
			     unsigned int vol)
{
  return DeviceVolume (sound_channel, is_encoding, TRUE, vol);
}


BOOL
GMEndPoint::GetDeviceVolume (PSoundChannel *sound_channel,
                                 BOOL is_encoding,
                                 unsigned int &vol)
{
  return DeviceVolume (sound_channel, is_encoding, FALSE, vol);
}


void
GMEndPoint::OnClosedMediaStream (const OpalMediaStream & stream)
{
  OpalManager::OnClosedMediaStream (stream);

  if (pcssEP->GetMediaFormats ().FindFormat(stream.GetMediaFormat()) == P_MAX_INDEX)
    OnMediaStream ((OpalMediaStream &) stream, TRUE);
}


BOOL 
GMEndPoint::OnOpenMediaStream (OpalConnection & connection,
			       OpalMediaStream & stream)
{
  if (!OpalManager::OnOpenMediaStream (connection, stream))
    return FALSE;

  if (pcssEP->GetMediaFormats ().FindFormat(stream.GetMediaFormat()) == P_MAX_INDEX)
    OnMediaStream (stream, FALSE);

  return TRUE;
}


BOOL 
GMEndPoint::OnMediaStream (OpalMediaStream & stream,
			   BOOL is_closing)
{
  GtkWidget *history_window = NULL;
  GtkWidget *main_window = NULL;
  
  PString codec_name;
  BOOL is_encoding = FALSE;
  BOOL is_video = FALSE;
  BOOL preview = FALSE;
  
  gchar *msg = NULL;


  /* Get the data */
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  
  is_video = (stream.GetSessionID () == OpalMediaFormat::DefaultVideoSessionID);
  is_encoding = !stream.IsSource (); // If the codec is from a source media
  				     // stream, the sink will be PCM or YUV
				     // and we are receiving.
  codec_name = PString (stream.GetMediaFormat ());

  gnomemeeting_threads_enter ();
  preview = gm_conf_get_bool (VIDEO_DEVICES_KEY "enable_preview");
  gnomemeeting_threads_leave ();

  if (is_video) {
    
    is_closing ?
      (is_encoding ? is_transmitting_video = FALSE:is_receiving_video = FALSE)
      :(is_encoding ? is_transmitting_video = TRUE:is_receiving_video = TRUE);
  }
  else {
    
    is_closing ?
      (is_encoding ? is_transmitting_audio = FALSE:is_receiving_audio = FALSE)
      :(is_encoding ? is_transmitting_audio = TRUE:is_receiving_audio = TRUE);
  }

  /* Do not optimize, easier for translators */
  if (is_encoding) {
    if (!is_closing) {
      
      if (!is_video)
	tr_audio_codec = codec_name;
      else
	tr_video_codec = codec_name;
      
      msg = 
	g_strdup_printf (_("Opened codec %s for transmission"),
			 (const char *) codec_name);
    }
    else {
      
      if (!is_video)
	tr_audio_codec = "";
      else
	tr_video_codec = "";

      msg = 
	g_strdup_printf (_("Closed codec %s which was opened for transmission"),
			 (const char *) codec_name);
    }
  }
  else {
    
    if (!is_closing) {
     
      if (!is_video)
	re_audio_codec = codec_name;
      else
	re_video_codec = codec_name;

      msg = 
	g_strdup_printf (_("Opened codec %s for reception"),
			 (const char *) codec_name);
    }
    else {
      
      if (!is_video)
	re_audio_codec = "";
      else
	re_video_codec = "";

      msg = 
	g_strdup_printf (_("Closed codec %s which was opened for reception"),
			 (const char *) codec_name);
    }
  }

  /* Update the GUI and menus wrt opened channels */
  gnomemeeting_threads_enter ();
  gm_history_window_insert (history_window, msg);
  gm_main_window_update_sensitivity (main_window, is_video, is_video?is_receiving_video:is_receiving_audio, is_video?is_transmitting_video:is_transmitting_audio);
  gm_main_window_set_channel_pause (main_window, FALSE, is_video);
  gm_main_window_set_call_info (main_window, 
				tr_audio_codec, re_audio_codec,
				tr_video_codec, re_video_codec);
  gnomemeeting_threads_leave ();
  
  g_free (msg);
    
  return TRUE;
}


void 
GMEndPoint::UpdateRTPStats (PTime start_time,
			    RTP_Session *audio_session,
			    RTP_Session *video_session)
{
  PTimeInterval t;
  PTime now;
  
  int elapsed_seconds = 0;
  int re_bytes = 0;
  int tr_bytes = 0;


  t = now - stats.last_tick;
  elapsed_seconds = t.GetSeconds ();

  if (elapsed_seconds > 1) { /* To get more precision */

    if (audio_session) {

      re_bytes = audio_session->GetOctetsReceived ();
      tr_bytes = audio_session->GetOctetsSent ();

      stats.a_re_bandwidth = (re_bytes - stats.re_a_bytes) 
	/ (1024.0 * elapsed_seconds);
      stats.a_tr_bandwidth = (tr_bytes - stats.tr_a_bytes) 
	/ (1024.0 * elapsed_seconds);

      stats.jitter_buffer_size = audio_session->GetJitterBufferSize ();

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

      stats.v_re_bandwidth = (re_bytes - stats.re_v_bytes) 
	/ (1024.0 * elapsed_seconds);
      stats.v_tr_bandwidth = (tr_bytes - stats.tr_v_bytes) 
	/ (1024.0 * elapsed_seconds);

      stats.re_v_bytes = re_bytes;
      stats.tr_v_bytes = tr_bytes;
      
      stats.total_packets += video_session->GetPacketsReceived ();
      stats.lost_packets += video_session->GetPacketsLost ();
      stats.late_packets += video_session->GetPacketsTooLate ();
      stats.out_of_order_packets += video_session->GetPacketsOutOfOrder ();
    }
    
    stats.last_tick = now;
    stats.start_time = start_time;
  }
}


void 
GMEndPoint::OnRTPTimeout (PTimer &, 
			  INT)
{
  GtkWidget *main_window = NULL;
  
  gchar *msg = NULL;
	
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

  PTimeInterval t;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  /* Update the audio and video sessions statistics */
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

  if (stats.start_time.IsValid ())
    t = PTime () - stats.start_time;

  msg = g_strdup_printf 
    (_("%.2ld:%.2ld:%.2ld  A:%.2f/%.2f   V:%.2f/%.2f"), 
     (long) t.GetHours (), (long) (t.GetMinutes () % 60), 
     (long) (t.GetSeconds () % 60),
     stats.a_tr_bandwidth, stats.a_re_bandwidth, 
     stats.v_tr_bandwidth, stats.v_re_bandwidth);

  
  if (stats.total_packets > 0) {

    lost_packets_per = ((float) stats.lost_packets * 100.0
			/ (float) stats.total_packets);
    late_packets_per = ((float) stats.late_packets * 100.0
			/ (float) stats.total_packets);
    out_of_order_packets_per = ((float) stats.out_of_order_packets * 100.0
				/ (float) stats.total_packets);
    lost_packets_per = PMIN (100, PMAX (0, lost_packets_per));
    late_packets_per = PMIN (100, PMAX (0, late_packets_per));
    out_of_order_packets_per = PMIN (100, PMAX (0, out_of_order_packets_per));
  }


  gdk_threads_enter ();
  gm_main_window_flash_message (main_window, msg);
  gm_main_window_update_stats (main_window,
			       lost_packets_per,
			       late_packets_per,
			       out_of_order_packets_per,
			       (int) (stats.jitter_buffer_size / 8),
			       stats.v_re_bandwidth,
			       stats.v_tr_bandwidth,
			       stats.a_re_bandwidth,
			       stats.a_tr_bandwidth);
  gdk_threads_leave ();


  g_free (msg);
}


PString
GMEndPoint::CheckTCPPorts ()
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
GMEndPoint::OnGatewayIPTimeout (PTimer &,
				    INT)
{
  PHTTPClient web_client ("GnomeMeeting");
  PString html, ip_address;
  gboolean ip_checking = false;

  web_client.SetReadTimeout (PTimeInterval (0, 5));
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
GMEndPoint::OnOutgoingCall (PTimer &,
			    INT) 
{
  pcssEP->PlaySoundEvent ("ring_tone_sound");

  if (OutgoingCallTimer.IsRunning ())
    OutgoingCallTimer.RunContinuous (PTimeInterval (0, 3));
}


BOOL 
GMEndPoint::DeviceVolume (PSoundChannel *sound_channel,
			  BOOL is_encoding,
			  BOOL set, 
			  unsigned int &vol) 
{
  BOOL err = TRUE;

  if (sound_channel && GetCallingState () == GMEndPoint::Connected) {

    
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
GMEndPoint::CreateVideoInputDevice (const OpalConnection &con,
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
GMEndPoint::CreateVideoOutputDevice(const OpalConnection & connection,
				    const OpalMediaFormat & format,
				    BOOL preview,
				    PVideoOutputDevice * & device,
				    BOOL & auto_delete)
{
  const PVideoDevice::OpenArgs & args = videoOutputDevice;

  /* Display of the input video, the display already
   * has the same size has the grabber 
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
	format.GetOptionInteger(OpalVideoFormat::FrameWidthOption, 176);
      videoOutputDevice.height = 
	format.GetOptionInteger(OpalVideoFormat::FrameHeightOption, 144);

      if (device->OpenFull (args, FALSE))
	return TRUE;

      delete device;
    }
  }

  return FALSE;
}


#ifdef HAS_IXJ
GMLid *
GMEndPoint::GetLid (void)
{
  PWaitAndSignal m(lid_access_mutex);
    
  if (lid) 
    lid->Lock ();

  return lid;
}


GMLid *
GMEndPoint::CreateLid (PString lid_device)
{
  PWaitAndSignal m(lid_access_mutex);
  
  if (lid)
    delete (lid);

  lid = new GMLid (lid_device);

  return lid;
}


void
GMEndPoint::RemoveLid (void)
{
  PWaitAndSignal m(lid_access_mutex);
  
  if (lid)     
    delete (lid);

  lid = NULL;
}
#endif


void
GMEndPoint::SendTextMessage (PString callToken,
			     PString message)
{
  PSafePtr <OpalCall> call = NULL;
  PSafePtr <OpalConnection> connection = NULL;

  call = FindCallWithLock (callToken);

  if (call != NULL) {
    
    connection = GetConnection (call, TRUE);

    if (connection != NULL) {
      
      connection->SendUserInputString ("MSG"+message);
    }
  }
  else {

    sipEP->SendMessage (callToken, message);
  }
}


BOOL
GMEndPoint::IsCallOnHold (PString callToken)
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
GMEndPoint::SetCallOnHold (PString callToken,
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
GMEndPoint::SendDTMF (PString callToken,
		      PString dtmf)
{
  PSafePtr <OpalCall> call = NULL;
  PSafePtr <OpalConnection> connection = NULL;

  call = FindCallWithLock (callToken);

  if (call != NULL) {
    
    connection = GetConnection (call, TRUE);

    if (connection != NULL) 
      connection->SendUserInputTone(dtmf [0], 50);
  }
}


gboolean
GMEndPoint::IsCallWithAudio (PString callToken)
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
GMEndPoint::IsCallWithVideo (PString callToken)
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
GMEndPoint::IsCallAudioPaused (PString callToken)
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
GMEndPoint::IsCallVideoPaused (PString callToken)
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
GMEndPoint::SetCallAudioPause (PString callToken, 
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
GMEndPoint::SetCallVideoPause (PString callToken, 
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
GMEndPoint::AddMWI (PString,
		    PString,
		    PString value)
{
  PWaitAndSignal m(mwi_access_mutex);

  mwi = value;
}


PString 
GMEndPoint::GetMWI ()
{
  PWaitAndSignal m(mwi_access_mutex);

  if (mwi.IsEmpty ())
    mwi = "0/0";

  return mwi;
}


int
GMEndPoint::GetRegisteredAccounts ()
{
  int number = 0;
  
  number = sipEP->GetRegisteredAccounts ();
  if (h323EP->IsRegisteredWithGatekeeper ())
    number++;

  return number;
}


void
GMEndPoint::ResetMissedCallsNumber ()
{
  PWaitAndSignal m(mc_access_mutex);

  missed_calls = 0;
}


int
GMEndPoint::GetMissedCallsNumber ()
{
  PWaitAndSignal m(mc_access_mutex);

  return missed_calls;
}


PString
GMEndPoint::GetLastCallAddress ()
{
  PWaitAndSignal m(lca_access_mutex);

  return called_address;
}


void 
GMEndPoint::SetTransferCallToken (PString s)
{
  tct_access_mutex.Wait ();
  transfer_call_token = s;
  tct_access_mutex.Signal ();
}


PString 
GMEndPoint::GetTransferCallToken ()
{
  PString c;

  tct_access_mutex.Wait ();
  c = transfer_call_token;
  tct_access_mutex.Signal ();

  return c;
}


void
GMEndPoint::TransferCallWait ()
{

  while (!GetTransferCallToken ().IsEmpty ())
    PThread::Current ()->Sleep (100);
}
          
