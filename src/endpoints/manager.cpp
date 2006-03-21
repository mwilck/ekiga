
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


#include "../../config.h"

#include "manager.h"
#include "pcss.h"
#include "h323.h"
#include "sip.h"

#include "accounts.h"
#include "urlhandler.h"
#if 0
#include "ils.h"
#endif

#include "ekiga.h"
#include "audio.h"
#include "statusicon.h"
#include "misc.h"
#include "chat.h"
#include "history.h"
#include "preferences.h"
#include "main.h"
#include "callshistory.h"

#include "gmstatsdrawingarea.h"

#ifdef HAS_DBUS
#include "dbus.h"
#endif

#include "gmdialog.h"
#include "gmconf.h"

#include <ptclib/http.h>
#include <ptclib/html.h>
#include <ptclib/pstun.h>


#define new PNEW

/* The class */
GMManager::GMManager ()
{
  /* Initialise the endpoint paramaters */
  video_grabber = NULL;
  SetCallingState (GMManager::Standby);
  
#ifdef HAS_AVAHI
  zcp = NULL;
#endif

#if 0
  ils_client = NULL;
#endif

  gk = NULL;
  sc = NULL;

  PIPSocket::SetDefaultIpAddressFamilyV4();
  
  audio_tester = NULL;

  audio_reception_popup = NULL;
  audio_transmission_popup = NULL;
  
  manager = NULL;

#if 0
  ILSTimer.SetNotifier (PCREATE_NOTIFIER (OnILSTimeout));
  ils_registered = false;
#endif

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
    g_free (device_name);
  }
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
  OpalMediaFormatList list;
  OpalMediaFormatList full_list;
  OpalMediaFormatList media_formats;

  media_formats = pcssEP->GetMediaFormats ();
  list += OpalTranscoder::GetPossibleFormats (media_formats);

  const char *allowed_codecs []= 
    {
      "G.711-ALaw-64k",
      "G.711-uLaw-64k",
      "iLBC-13k3",
      "LPC-10",
      "GSM-06.10",
      "MS-GSM",
      "SpeexNarrow-8k",
      "SpeexWide-20.6k",
      "G.726-16k",
      "G.726-32k",
      NULL,
    };

  for (int i = 0 ; i < list.GetSize () ; i++) {

    if (list [i].GetDefaultSessionID () == 1) {
      
      for (int j = 0 ; allowed_codecs [j] != NULL ; j++)
	if (!strcmp (allowed_codecs [j], list [i]))
	  full_list += list [i];
    }
  }
  
  return full_list;
}


void
GMManager::SetAllMediaFormats ()
{
  SetAudioMediaFormats ();
  SetVideoMediaFormats ();
  SetUserInputMode ();
}


void 
GMManager::SetAudioMediaFormats ()
{
  OpalMediaFormatList media_formats;
  PStringArray order, mask;
  
  GSList *codecs_data = NULL;
  
  gchar **couple = NULL;
  
  /* Get all the media formats */
  media_formats = pcssEP->GetMediaFormats ();
  media_formats += OpalTranscoder::GetPossibleFormats (media_formats);
  
  /* Read the codecs in the config to add them with the correct order */ 
  codecs_data = gm_conf_get_string_list (AUDIO_CODECS_KEY "list");
  while (codecs_data) {
    
    couple = g_strsplit ((gchar *) codecs_data->data, "=", 0);

    if (couple && couple [0] && couple [1]) 
      if (!strcmp (couple [1], "1")) 
	order += couple [0];
    
    g_strfreev (couple);
    codecs_data = g_slist_next (codecs_data);
  }
  g_slist_free (codecs_data);
  
  /* Build the mask with all other codecs that were not added */
  media_formats.Remove (order);
  for (int i = 0 ; i < media_formats.GetSize () ; i++)
    if (media_formats [i].GetDefaultSessionID () == 1)
      mask += media_formats [i];
  
  /* Update the order and mask */
  SetMediaFormatMask (mask);
  SetMediaFormatOrder (order);
}


void 
GMManager::SetVideoMediaFormats ()
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

  if (video_grabber) {

    delete (video_grabber);
  }      
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
#if 0
  BOOL ilsreg = FALSE; 
#endif
  
#ifdef HAS_AVAHI
  PWaitAndSignal m(zcp_access_mutex);
  if (zcp)  
    zcp->Publish ();
#endif
#if 0
  gnomemeeting_threads_enter ();
  ilsreg = gm_conf_get_bool (LDAP_KEY "enable_registering");
  gnomemeeting_threads_leave ();
  ILSTimer.RunContinuous (PTimeInterval (5));
#endif
}


void
GMManager::Register (GmAccount *account)
{
  PWaitAndSignal m(manager_access_mutex);

  if (manager)
    delete manager;
  manager = new GMAccountsEndpoint (account, *this);
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
  
  sc = NULL;
}


BOOL
GMManager::OnForwarded (OpalConnection &,
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
					   NULL, utf8_url, NULL, NULL, NULL);
  if (contacts) {
    
    contact = GM_CONTACT (contacts->data);
    g_free (utf8_name);
    utf8_name = g_strdup (contact->fullname);
    g_slist_foreach (contacts, (GFunc) gmcontact_delete, NULL);
    g_slist_free (contacts);
  }
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
				  int reason,
				  PString extra)
{
  BOOL res = FALSE;

  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;
  GtkWidget *history_window = NULL;
  GtkWidget *statusicon = NULL;
#ifdef HAS_DBUS
  GObject *dbus_component = NULL;
#endif

  gchar *msg = NULL;
  gchar *short_reason = NULL;
  gchar *long_reason = NULL;

  gchar *utf8_name = NULL;
  gchar *utf8_app = NULL;
  gchar *utf8_url = NULL;

  GetRemoteConnectionInfo (connection, utf8_name, utf8_app, utf8_url);

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  statusicon = GnomeMeeting::Process ()->GetStatusicon ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
#ifdef HAS_DBUS
  dbus_component = GnomeMeeting::Process ()->GetDbusComponent ();
#endif

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

    SetCallingState (GMManager::Called);
    SetCurrentCallToken (connection.GetCall ().GetToken ());

    /* Update the UI */
    gnomemeeting_threads_enter ();
    gm_statusicon_update_menu (statusicon, GMManager::Called);
    gm_main_window_update_calling_state (main_window, GMManager::Called);
    gm_chat_window_update_calling_state (chat_window, 
					 NULL, 
					 NULL, 
					 GMManager::Called);

    gm_main_window_incoming_call_dialog_show (main_window,
					      utf8_name, 
					      utf8_app, 
					      utf8_url);
#ifdef HAS_DBUS
    gnomemeeting_dbus_component_set_call_state (dbus_component,
						GetCurrentCallToken (),
						GMManager::Called);
    gnomemeeting_dbus_component_set_call_info (dbus_component,
					       GetCurrentCallToken (),
					       utf8_name,
					       utf8_app,
					       utf8_url,
					       NULL);
#endif
    gnomemeeting_threads_leave ();
  }

  g_free (utf8_app);
  g_free (utf8_name);
  g_free (utf8_url);

  return res;
}


void 
GMManager::OnEstablishedCall (OpalCall &call)
{
  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;
  GtkWidget *statusicon = NULL;
#ifdef HAS_DBUS
  GObject *dbus_component = NULL;
#endif

  BOOL stay_on_top = FALSE;
  BOOL forward_on_busy = FALSE;

  IncomingCallMode icm = AVAILABLE;
  
  /* Get the widgets */
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  statusicon = GnomeMeeting::Process ()->GetStatusicon ();
#ifdef HAS_DBUS
  dbus_component = GnomeMeeting::Process ()->GetDbusComponent ();
#endif

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
  SetCallingState (GMManager::Connected);
  SetCurrentCallToken (call.GetToken ());
 
  /* Update the GUI */
  gnomemeeting_threads_enter ();
  if (called_address.IsEmpty ()) 
    gm_main_window_set_call_url (main_window, GMURL ().GetDefaultURL ());
  gm_main_window_set_stay_on_top (main_window, stay_on_top);
  gm_statusicon_update_full (statusicon, GMManager::Connected,
			     icm, forward_on_busy);
#ifdef HAS_DBUS
  gnomemeeting_dbus_component_set_call_state (dbus_component,
					      GetCurrentCallToken (),
					      GMManager::Connected);
#endif
  gnomemeeting_threads_leave ();
}


void 
GMManager::OnEstablished (OpalConnection &connection)
{
  RTP_Session *audio_session = NULL;
  RTP_Session *video_session = NULL;

  gchar *utf8_url = NULL;
  gchar *utf8_app = NULL;
  gchar *utf8_name = NULL;
  gchar *utf8_protocol_prefix = NULL;
  gchar *msg = NULL;

  GtkWidget *history_window = NULL;
  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;
#ifdef HAS_DBUS
  GObject *dbus_component = NULL;
#endif

  /* Get the widgets */
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
#ifdef HAS_DBUS
  dbus_component = GnomeMeeting::Process ()->GetDbusComponent ();
#endif

  /* Do nothing for the PCSS connection */
  if (PIsDescendant(&connection, OpalPCSSConnection)) {
    
    PTRACE (3, "GMManager\t Will establish the connection");
    OpalManager::OnEstablished (connection);
    return;
  }
  
  /* Update internal state */
  GetRemoteConnectionInfo (connection, utf8_name, utf8_app, utf8_url);
  utf8_protocol_prefix = gnomemeeting_get_utf8 (connection.GetEndPoint ().GetPrefixName ().Trim ());
  gnomemeeting_threads_enter ();
  if (GetCallingState () != GMManager::Connected)
    gm_history_window_insert (history_window, _("Connected with %s using %s"), 
			      utf8_name, utf8_app);
  msg = g_strdup_printf (_("Connected with %s"), utf8_name);
  gm_main_window_set_status (main_window, msg);
  gm_main_window_flash_message (main_window, msg);
  gm_chat_window_push_info_message (chat_window, NULL, msg);
  gm_main_window_update_calling_state (main_window, GMManager::Connected);
  gm_chat_window_update_calling_state (chat_window, 
				       utf8_name,
				       utf8_url, 
				       GMManager::Connected);
#ifdef HAS_DBUS
  gnomemeeting_dbus_component_set_call_info (dbus_component,
					     connection.GetCall ().GetToken (),
					     utf8_name, utf8_app, utf8_url,
					     utf8_protocol_prefix);
#endif
  gnomemeeting_threads_leave ();
  
  /* Asterisk sometimes forgets to send an INVITE, HACK */
  audio_session = 
    connection.GetSession (OpalMediaFormat::DefaultAudioSessionID);
  video_session = 
    connection.GetSession (OpalMediaFormat::DefaultVideoSessionID);
  if (audio_session)
    audio_session->SetIgnoreOtherSources (TRUE);
  if (video_session)
    video_session->SetIgnoreOtherSources (TRUE);
  
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

  PTRACE (3, "GMManager\t Will establish the connection");
  OpalManager::OnEstablished (connection);
}


void 
GMManager::OnClearedCall (OpalCall & call)
{
  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;
  GtkWidget *statusicon = NULL;
#ifdef HAS_DBUS
  GObject *dbus_component = NULL;
#endif
  
  BOOL reg = FALSE;
  BOOL forward_on_busy = FALSE;
  IncomingCallMode icm = AVAILABLE;
  ViewMode m = SOFTPHONE;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  statusicon = GnomeMeeting::Process ()->GetStatusicon ();
#ifdef HAS_DBUS
  dbus_component = GnomeMeeting::Process ()->GetDbusComponent ();
#endif
  
  if (GetCurrentCallToken() != PString::Empty() 
      && GetCurrentCallToken () != call.GetToken())
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
  
  /* we reset the no-data detection */
  RTPTimer.Stop ();
  stats.Reset ();

  /* Play busy tone */
  pcssEP->PlaySoundEvent ("busy_tone_sound"); 

  /* Update the various parts of the GUI */
  gnomemeeting_threads_enter ();
  gm_main_window_set_stay_on_top (main_window, FALSE);
  gm_main_window_update_calling_state (main_window, GMManager::Standby);
  gm_chat_window_update_calling_state (chat_window, 
				       NULL, 
				       NULL, 
				       GMManager::Standby);
  gm_statusicon_update_full (statusicon, GMManager::Standby,
			     icm, forward_on_busy);
  gm_main_window_set_status (main_window, _("Standby"));
  gm_main_window_set_account_info (main_window, 
				   GetRegisteredAccounts ()); 
  gm_main_window_clear_stats (main_window);
  gm_main_window_update_logo (main_window);
  gm_main_window_clear_signal_levels (main_window);
  gm_main_window_set_view_mode (main_window, m);
#ifdef HAS_DBUS
  gnomemeeting_dbus_component_set_call_state (dbus_component,
					      GetCurrentCallToken (),
					      GMManager::Standby);
#endif
  gnomemeeting_threads_leave ();

  /* Update internal state */
  SetCallingState (GMManager::Standby);
  SetCurrentCallToken ("");

  /* Try to update the devices use if some settings were changed 
     during the call */
  UpdateDevices ();
}


void
GMManager::OnReleased (OpalConnection & connection)
{ 
  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;
  GtkWidget *history_window = NULL;
  
  gchar *msg_reason = NULL;
  
  gchar *utf8_url = NULL;
  gchar *utf8_name = NULL;
  gchar *utf8_app = NULL;

  PTimeInterval t;

  /* Get the widgets */
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  
  /* Do nothing for the PCSS connection */
  if (PIsDescendant(&connection, OpalPCSSConnection)) {
    
    PTRACE (3, "GMManager\t Will release the connection");
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
  gm_main_window_push_message (main_window, 
			       GetMissedCallsNumber (), 
			       GetMWI ());
  gm_main_window_flash_message (main_window, msg_reason);
  gm_chat_window_push_info_message (chat_window, NULL, "");
  gnomemeeting_threads_leave ();

  g_free (utf8_app);
  g_free (utf8_name);
  g_free (utf8_url);  
  g_free (msg_reason);

  /* Reinitialize codecs */
  re_audio_codec = tr_audio_codec = re_video_codec = tr_video_codec = "";

  PTRACE (3, "GMManager\t Will release the connection");
  OpalManager::OnReleased (connection);
}


void 
GMManager::OnHold (OpalConnection & connection)
{
  GtkWidget *main_window = NULL;
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  gnomemeeting_threads_enter ();
  gm_main_window_set_call_hold (main_window,
				connection.IsConnectionOnHold ());
  gnomemeeting_threads_leave ();
}


void
GMManager::OnUserInputString (OpalConnection & connection,
			       const PString & value)
{
  GtkWidget *chat_window = NULL;
  GtkWidget *statusicon = NULL;

  gchar *name = NULL;
  gchar *url = NULL;
  gchar *app = NULL;
	
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  statusicon = GnomeMeeting::Process ()->GetStatusicon ();
  
  GetRemoteConnectionInfo (connection, name, app, url);

  if (value.Find ("MSG") != P_MAX_INDEX) {

    gnomemeeting_threads_enter ();
    gm_text_chat_window_insert (chat_window, url, name, 
				(const char *) value.Mid (3), 1);  
    if (!gnomemeeting_window_is_visible (chat_window))
      gm_statusicon_signal_message (statusicon, TRUE);
    gnomemeeting_threads_leave ();
  }
}


void 
GMManager::SavePicture (void)
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
    PTRACE (1, "Set TCP port range to " << atoi (tcp_couple [0]) << ":"
	    << atoi (tcp_couple [1]));
  }

  if (rtp_couple && rtp_couple [0] && rtp_couple [1]) {

    SetRtpIpPorts (atoi (rtp_couple [0]), atoi (rtp_couple [1]));
    PTRACE (1, "Set RTP port range to " << atoi (rtp_couple [0]) << ":"
	    << atoi (rtp_couple [1]));
  }

  if (udp_couple && udp_couple [0] && udp_couple [1]) {

    SetUDPPorts (atoi (udp_couple [0]), atoi (udp_couple [1]));
    PTRACE (1, "Set UDP port range to " << atoi (udp_couple [0]) << ":"
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
GMManager::Init ()
{
  GtkWidget *main_window = NULL;
  GtkWidget *prefs_window = NULL;

  OpalEchoCanceler::Params ec;
  OpalSilenceDetector::Params sd;
  OpalMediaFormatList list;
  
  int min_jitter = 20;
  int max_jitter = 500;
  int nat_method = 0;
  
  gboolean enable_sd = TRUE;  
  gboolean enable_ec = TRUE;  

  gchar *ip = NULL;
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();

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

  /* Start Listeners */
  StartListeners ();
  
  /* Create a Zeroconf client */
#ifdef HAS_AVAHI
  CreateZeroconfClient ();
#endif

  /* Update publishers */
  UpdatePublishers ();

  /* Update the codecs list */
  //FIXME Move to UpdateGUI in GnomeMeeting
  list = GetAvailableAudioMediaFormats ();
  gm_prefs_window_update_audio_codecs_list (prefs_window, list);
  
  /* Set the media formats */
  SetAllMediaFormats ();
  
  /* Register the various accounts */
  Register ();

  g_free (ip);
}


void
GMManager::StartListeners ()
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
      gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Error while starting the listener for the H.323 protocol"), _("You will not be able to receive incoming H.323 calls. Please check that no other program is already running on the port used by Ekiga."));
      gnomemeeting_threads_leave ();
    }
  }

  if (sipEP) {
    
    gnomemeeting_threads_enter ();
    port = gm_conf_get_int (SIP_KEY "listen_port");
    gnomemeeting_threads_leave ();
    
    if (!sipEP->StartListener (iface, port)) {
      
      gnomemeeting_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Error while starting the listener for the SIP protocol"), _("You will not be able to receive incoming SIP calls. Please check that no other program is already running on the port used by Ekiga."));
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

#if 0
void 
GMManager::OnILSTimeout (PTimer &,
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
#endif

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
  GtkWidget *main_window = NULL;
  
  if (!OpalManager::OnOpenMediaStream (connection, stream))
    return FALSE;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  gnomemeeting_threads_enter ();
  gm_main_window_update_calling_state (main_window, GMManager::Connected);
  gnomemeeting_threads_leave ();

  if (pcssEP->GetMediaFormats ().FindFormat(stream.GetMediaFormat()) == P_MAX_INDEX)
    OnMediaStream (stream, FALSE);

  return TRUE;
}


BOOL 
GMManager::OnMediaStream (OpalMediaStream & stream,
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
  codec_name = stream.GetMediaFormat ().GetEncodingName ();

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
GMManager::UpdateRTPStats (PTime start_time,
			    RTP_Session *audio_session,
			    RTP_Session *video_session)
{
  PTimeInterval t;
  PTime now;
  
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

      stats.a_re_bandwidth = (re_bytes - stats.re_a_bytes) 
	/ (1024.0 * elapsed_seconds);
      stats.a_tr_bandwidth = (tr_bytes - stats.tr_a_bytes) 
	/ (1024.0 * elapsed_seconds);

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
GMManager::OnRTPTimeout (PTimer &, 
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
  gm_main_window_push_info_message (main_window, msg);
  gm_main_window_update_stats (main_window,
			       lost_packets_per,
			       late_packets_per,
			       out_of_order_packets_per,
			       (int) (stats.jitter_buffer_size),
			       stats.v_re_bandwidth,
			       stats.v_tr_bandwidth,
			       stats.a_re_bandwidth,
			       stats.a_tr_bandwidth);
  gdk_threads_leave ();


  g_free (msg);
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
  if (ip_detector != NULL)
    g_free (ip_detector);
  if (!ip_address.IsEmpty () && ip_checking) {

    gdk_threads_enter ();
    gm_conf_set_string (NAT_KEY "public_ip",
			(gchar *) (const char *) ip_address);
    gdk_threads_leave ();
  }

  GatewayIPTimer.RunContinuous (PTimeInterval (0, 0, 15));
}


void
GMManager::OnOutgoingCall (PTimer &,
			    INT) 
{
  pcssEP->PlaySoundEvent ("ring_tone_sound");

  if (OutgoingCallTimer.IsRunning ())
    OutgoingCallTimer.RunContinuous (PTimeInterval (0, 3));
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


BOOL
GMManager::SendTextMessage (PString url,
			     PString message)
{
  PSafePtr <OpalCall> call = NULL;
  PSafePtr <OpalConnection> connection = NULL;

  PString call_token;

  call_token = GetCurrentCallToken ();

  /* We need specific code as the system is different for H.323
   * and SIP.
   */
  if (GMURL (url).GetType() == "h323") {

    call = FindCallWithLock (call_token);

    if (call != NULL) {

      connection = GetConnection (call, TRUE);

      if (connection != NULL) {

	connection->SendUserInputString ("MSG"+message);
	return TRUE;
      }
    }
  }
  else if (GMURL (url).GetType () == "sip") {

    return sipEP->SendMessage (url, message);
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
      connection->SendUserInputTone(dtmf [0], 0);
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
GMManager::AddMWI (const PString & host,
		    const PString & user,
		    const PString & value)
{
  PString key;
  PString * val = NULL;
  
  PWaitAndSignal m(mwi_access_mutex);

  key = host + "-" + user;
  val = new PString (value);

  mwiData.SetAt (key, val);
}


PString 
GMManager::GetMWI (const PString & host,
		    const PString & user)
{
  PString key;
  PString *value = NULL;
  
  PWaitAndSignal m(mwi_access_mutex);

  key = host + "-" + user;
  
  value = mwiData.GetAt (key);

  if (value)
    return *value;

  return "";
}


PString 
GMManager::GetMWI ()
{
  PINDEX i = 0;
  PINDEX j = 0;
  int total = 0;
  
  PString key;
  PString val;
  PString *value = NULL;
  
  PWaitAndSignal m(mwi_access_mutex);

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
  
  return total;
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


void
GMManager::ResetMissedCallsNumber ()
{
  PWaitAndSignal m(mc_access_mutex);

  missed_calls = 0;
}


int
GMManager::GetMissedCallsNumber ()
{
  PWaitAndSignal m(mc_access_mutex);

  return missed_calls;
}


PString
GMManager::GetLastCallAddress ()
{
  PWaitAndSignal m(lca_access_mutex);

  return called_address;
}

