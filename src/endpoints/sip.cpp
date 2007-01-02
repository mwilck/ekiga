
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
 *                         sipendpoint.cpp  -  description
 *                         --------------------------------
 *   begin                : Wed 8 Dec 2004
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the SIP Endpoint class.
 *
 */


#include "../../config.h"

#include "sip.h"
#include "pcss.h"
#include "ekiga.h"

#include "main.h"
#include "chat.h"
#include "preferences.h"
#include "history.h"
#include "statusicon.h"
#include "misc.h"
#ifdef HAS_DBUS
#include "dbus.h"
#endif

#include "gmconf.h"
#include "gmdialog.h"

#include <ptlib/ethsock.h>

#define new PNEW


/* The class */
GMSIPEndpoint::GMSIPEndpoint (GMManager & ep)
: SIPEndPoint (ep), endpoint (ep)
{
  NoAnswerTimer.SetNotifier (PCREATE_NOTIFIER (OnNoAnswerTimeout));
}


GMSIPEndpoint::~GMSIPEndpoint ()
{
}


void 
GMSIPEndpoint::Init ()
{
  GtkWidget *main_window = NULL;

  gchar *outbound_proxy_host = NULL;
  int binding_timeout = 60;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  gnomemeeting_threads_enter ();
  outbound_proxy_host = gm_conf_get_string (SIP_KEY "outbound_proxy_host");
  binding_timeout = gm_conf_get_int (NAT_KEY "binding_timeout");
  gnomemeeting_threads_leave ();


  /* Timeouts */
  SetPduCleanUpTimeout (PTimeInterval (0, 1));
  SetInviteTimeout (PTimeInterval (0, 6));
  SetNonInviteTimeout (PTimeInterval (0, 6));
  SetNATBindingTimeout (PTimeInterval (0, binding_timeout));
  SetRetryTimeouts (500, 4000);
  SetMaxRetries (8);


  /* Update the User Agent */
  SetUserAgent ("Ekiga/" PACKAGE_VERSION);
  

  /* Initialise internal parameters */
  if (outbound_proxy_host && !PString (outbound_proxy_host).IsEmpty ())
    SetProxy (outbound_proxy_host);
  SetNATBindingRefreshMethod (SIPEndPoint::EmptyRequest);


  g_free (outbound_proxy_host);
}


BOOL 
GMSIPEndpoint::StartListener (PString iface, 
			      WORD port)
{
  PString iface_noip;
  PString ip;
  PIPSocket::InterfaceTable ifaces;
  PINDEX i = 0;
  PINDEX pos = 0;
  
  gboolean ok = FALSE;
  gboolean found = FALSE;

  gchar *listen_to = NULL;

  RemoveListener (NULL);

  /* Detect the valid interfaces */
  PIPSocket::GetInterfaceTable (ifaces);

  while (i < ifaces.GetSize ()) {
    
    ip = " [" + ifaces [i].GetAddress ().AsString () + "]";
    
    if (ifaces [i].GetName () + ip == iface) {
      listen_to = 
	g_strdup_printf ("udp$%s:%d", 
			 (const char *) ifaces [i].GetAddress().AsString(),
			 port);
      found = TRUE;
    }
      
    i++;
  }

  i = 0;
  pos = iface.Find("[");
  if (pos != P_MAX_INDEX)
    iface_noip = iface.Left (pos).Trim ();
  while (i < ifaces.GetSize() && !found) {

    if (ifaces [i].GetName () == iface_noip) {
      listen_to = 
	g_strdup_printf ("udp$%s:%d", 
			 (const char *) ifaces [i].GetAddress().AsString(),
			 port);
      found = TRUE;
    }
    
    i++;
  }

  /* Start the listener thread for incoming calls */
  if (!listen_to)
    return FALSE;

  ok = StartListeners (PStringArray (listen_to));
  g_free (listen_to);

  return ok;
}


void
GMSIPEndpoint::SetUserNameAndAlias ()
{
  PString default_local_name;

  default_local_name = endpoint.GetDefaultDisplayName ();

  if (!default_local_name.IsEmpty ()) {

    SetDefaultDisplayName (default_local_name);
  }
}


void 
GMSIPEndpoint::SetUserInputMode ()
{
  // Do nothing, only RFC2833 is supported.
}


void
GMSIPEndpoint::OnRegistered (const PString & domain,
			     const PString & username,
			     BOOL wasRegistering)
{
  GtkWidget *accounts_window = NULL;
  GtkWidget *history_window = NULL;
  GtkWidget *main_window = NULL;
#ifdef HAS_DBUS
  GObject   *dbus_component = NULL;
#endif

  gchar *msg = NULL;

  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
#ifdef HAS_DBUS
  dbus_component = GnomeMeeting::Process ()->GetDbusComponent ();
#endif

  gnomemeeting_threads_enter ();
  /* Registering is ok */
  if (wasRegistering) {

    msg = g_strdup_printf (_("Registered to %s"), 
			   (const char *) domain);
    gm_accounts_window_update_account_state (accounts_window, 
					     FALSE,
					     (const char *) domain, 
					     (const char *) username, 
					     _("Registered"),
					     NULL);
  }
  else {

    msg = g_strdup_printf (_("Unregistered from %s"),
			   (const char *) domain); 
    gm_accounts_window_update_account_state (accounts_window, 
					     FALSE,
					     (const char *) domain, 
					     (const char *) username, 
					     _("Unregistered"),
					     NULL);
  }

#ifdef HAS_DBUS
  gnomemeeting_dbus_component_account_registration (dbus_component,
						    username, domain,
						    wasRegistering);
#endif

  gm_history_window_insert (history_window, msg);
  gm_main_window_flash_message (main_window, msg);
  if (endpoint.GetCallingState() == GMManager::Standby)
    gm_main_window_set_account_info (main_window, 
				     endpoint.GetRegisteredAccounts());
  gnomemeeting_threads_leave ();

  /* Signal the SIPEndpoint */
  SIPEndPoint::OnRegistered (domain, username, wasRegistering);

  g_free (msg);
}


void
GMSIPEndpoint::OnRegistrationFailed (const PString & host,
				     const PString & user,
				     SIP_PDU::StatusCodes r,
				     BOOL wasRegistering)
{
  GtkWidget *accounts_window = NULL;
  GtkWidget *history_window = NULL;
  GtkWidget *main_window = NULL;

  gchar *msg_reason = NULL;
  gchar *msg = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();

  gnomemeeting_threads_enter ();
  /* Registering is ok */
  switch (r) {

  case SIP_PDU::Failure_BadRequest:
    msg_reason = g_strdup (_("Bad request"));
    break;

  case SIP_PDU::Failure_PaymentRequired:
    msg_reason = g_strdup (_("Payment required"));
    break;

  case SIP_PDU::Failure_UnAuthorised:
  case SIP_PDU::Failure_Forbidden:
    msg_reason = g_strdup (_("Forbidden"));
    break;

  case SIP_PDU::Failure_RequestTimeout:
    msg_reason = g_strdup (_("Timeout"));
    break;

  case SIP_PDU::Failure_Conflict:
    msg_reason = g_strdup (_("Conflict"));
    break;

  case SIP_PDU::Failure_TemporarilyUnavailable:
    msg_reason = g_strdup (_("Temporarily unavailable"));
    break;
    
  case SIP_PDU::Failure_NotAcceptable:
    msg_reason = g_strdup (_("Not Acceptable"));
    break;

  default:
    msg_reason = g_strdup (_("Registration failed"));
  }

  if (wasRegistering) {

    msg = g_strdup_printf (_("Registration failed: %s"), 
			   msg_reason);

    gm_accounts_window_update_account_state (accounts_window, 
					     FALSE,
					     (const char *) host, 
					     (const char *) user, 
					     _("Registration failed"),
					     NULL);
  }
  else {

    msg = g_strdup_printf (_("Unregistration failed: %s"), 
			   msg_reason);

    gm_accounts_window_update_account_state (accounts_window, 
					     FALSE,
					     (const char *) host, 
					     (const char *) user, 
					     _("Unregistration failed"),
					     NULL);
  }

  gm_history_window_insert (history_window, msg);
  gm_main_window_push_message (main_window, msg);
  gnomemeeting_threads_leave ();

  /* Signal the SIP Endpoint */
  SIPEndPoint::OnRegistrationFailed (host, user, r, wasRegistering);


  g_free (msg);
}


BOOL 
GMSIPEndpoint::OnIncomingConnection (OpalConnection &connection)
{
  PSafePtr<OpalConnection> con = NULL;
  PSafePtr<OpalCall> call = NULL;

  gchar *forward_host = NULL;

  IncomingCallMode icm;
  gboolean busy_forward = FALSE;
  gboolean always_forward = FALSE;
  int no_answer_timeout = FALSE;

  BOOL res = FALSE;

  int reason = 0;

  PTRACE (3, "GMSIPEndpoint\tIncoming connection");

  gnomemeeting_threads_enter ();
  forward_host = gm_conf_get_string (SIP_KEY "forward_host");
  busy_forward = gm_conf_get_bool (CALL_FORWARDING_KEY "forward_on_busy");
  always_forward = gm_conf_get_bool (CALL_FORWARDING_KEY "always_forward");
  icm =
    (IncomingCallMode) gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode");
  no_answer_timeout =
    gm_conf_get_int (CALL_OPTIONS_KEY "no_answer_timeout");
  gnomemeeting_threads_leave ();

  call = endpoint.FindCallWithLock (endpoint.GetCurrentCallToken());
  if (call)
    con = endpoint.GetConnection (call, TRUE);
  if ((con && con->GetIdentifier () == connection.GetIdentifier())) {
    return TRUE;
  }
  
  if (icm == DO_NOT_DISTURB)
    reason = 1;
  else if (forward_host && always_forward)
    reason = 2; // Forward
  /* We are in a call */
  else if (endpoint.GetCallingState () != GMManager::Standby) {

    if (forward_host && busy_forward)
      reason = 2; // Forward
    else
      reason = 1; // Reject
  }
  else if (icm == AUTO_ANSWER)
    reason = 4; // Auto Answer
  else
    reason = 0; // Ask the user

  if (reason == 0)
    NoAnswerTimer.SetInterval (0, PMIN (no_answer_timeout, 60));

  res = endpoint.OnIncomingConnection (connection, reason, forward_host);

  g_free (forward_host);

  return res;
}


void 
GMSIPEndpoint::OnMWIReceived (const PString & remoteAddress,
			      const PString & user,
			      SIPMWISubscribe::MWIType type,
			      const PString & msgs)
{
  GMManager *ep = NULL;
  GMPCSSEndpoint *pcssEP = NULL;
  
  GtkWidget *main_window = NULL;
  GtkWidget *accounts_window = NULL;

  int total = 0;
  
  if (endpoint.GetMWI (remoteAddress, user) != msgs) {

    total = endpoint.GetMWI ().AsInteger ();

    /* Update UI */
    endpoint.AddMWI (remoteAddress, user, msgs);

    main_window = GnomeMeeting::Process ()->GetMainWindow ();
    accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();

    gnomemeeting_threads_enter ();
    gm_main_window_push_message (main_window, 
				 endpoint.GetMissedCallsNumber (), 
				 endpoint.GetMWI ());
    gm_accounts_window_update_account_state (accounts_window,
					     FALSE,
					     remoteAddress,
					     user,
					     NULL,
					     (const char *) msgs);
    gnomemeeting_threads_leave ();

    /* Sound event if new voice mail */
    if (endpoint.GetMWI ().AsInteger () > total) {

      ep = GnomeMeeting::Process ()->GetManager ();
      pcssEP = ep->GetPCSSEndpoint ();
      pcssEP->PlaySoundEvent ("new_voicemail_sound");
    }
  }
}


void 
GMSIPEndpoint::OnReceivedMESSAGE (OpalTransport & transport,
				  SIP_PDU & pdu)
{
  PString *last = NULL;
  PString *val = NULL;
  
  PString from = pdu.GetMIME().GetFrom();   
  PINDEX j = from.Find (';');
  if (j != P_MAX_INDEX)
    from = from.Left(j); // Remove all parameters

  last = msgData.GetAt (SIPURL (from).AsString ());
  if (!last || *last != pdu.GetMIME ().GetCallID ()) {

    val = new PString (pdu.GetMIME ().GetCallID ());
    msgData.SetAt (SIPURL (from).AsString (), val);
    OnMessageReceived(from, pdu.GetEntityBody());
  }
}


void 
GMSIPEndpoint::OnMessageReceived (const SIPURL & from,
				  const PString & body)
{
  GMManager *ep = NULL;
  GMPCSSEndpoint *pcssEP = NULL;

  GtkWidget *chat_window = NULL;
  GtkWidget *statusicon = NULL;

  gboolean chat_window_visible = FALSE;
  
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  statusicon = GnomeMeeting::Process ()->GetStatusicon ();

  SIPEndPoint::OnMessageReceived (from, body);

  gnomemeeting_threads_enter ();
  gm_text_chat_window_insert (chat_window, from.AsString (), 
			      from.GetDisplayName (), (const char *) body, 1);  
  chat_window_visible = gnomemeeting_window_is_visible (chat_window);
  gnomemeeting_threads_leave ();

  if (!chat_window_visible) {
   
    gnomemeeting_threads_enter ();
    gm_statusicon_signal_message (statusicon, TRUE);
    gnomemeeting_threads_leave ();

    ep = GnomeMeeting::Process ()->GetManager ();
    pcssEP = ep->GetPCSSEndpoint ();
    pcssEP->PlaySoundEvent ("new_message_sound");
  }
}


void 
GMSIPEndpoint::OnMessageFailed (const SIPURL & messageUrl,
				SIP_PDU::StatusCodes reason)
{
  GtkWidget *chat_window = NULL;
  gchar *msg = NULL;

  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  
  switch (reason) {

  case SIP_PDU::Failure_NotFound:
    msg = g_strdup (_("Error: User not found"));
    break;

  case SIP_PDU::Failure_TemporarilyUnavailable:
    msg = g_strdup (_("Error: User offline"));
    break;

  case SIP_PDU::Failure_UnAuthorised:
  case SIP_PDU::Failure_Forbidden:
    msg = g_strdup (_("Error: Forbidden"));
    break;

  case SIP_PDU::Failure_RequestTimeout:
    msg = g_strdup (_("Error: Timeout"));
    break;

  default:
    msg = g_strdup (_("Error: Failed to transmit message"));
  }

  gnomemeeting_threads_enter ();
  gm_text_chat_window_insert (chat_window, messageUrl.AsString (), 
			      NULL, msg, 2);
  gnomemeeting_threads_leave ();

  g_free (msg);
}
      

int
GMSIPEndpoint::GetRegisteredAccounts ()
{
  return SIPEndPoint::GetRegistrationsCount ();
}


SIPURL
GMSIPEndpoint::GetRegisteredPartyName (const PString & host)
{
  GmAccount *account = NULL;

  PString url;
  SIPURL registration_address;

  PSafePtr<SIPInfo> info = activeSIPInfo.FindSIPInfoByDomain(host, SIP_PDU::Method_REGISTER, PSafeReadOnly);

  if (info != NULL)
    registration_address = info->GetRegistrationAddress();

  account = gnomemeeting_get_default_account ("SIP");
  if (account) {

    if (info == NULL || registration_address.GetHostName () == account->host) {

      if (PString(account->username).Find("@") == P_MAX_INDEX)
        url = PString (account->username) + "@" + PString (account->host);
      else
        url = PString (account->username);

      return url;
    }
  }
  if (info != NULL) 
    return registration_address;
  else 
    return SIPEndPoint::GetDefaultRegisteredPartyName (); 
}


void 
GMSIPEndpoint::OnEstablished (OpalConnection &connection)
{
  NoAnswerTimer.Stop ();

  PTRACE (3, "GMSIPEndpoint\t SIP connection established");
  SIPEndPoint::OnEstablished (connection);
}


void 
GMSIPEndpoint::OnReleased (OpalConnection &connection)
{
  NoAnswerTimer.Stop ();

  PTRACE (3, "GMSIPEndpoint\t SIP connection released");
  SIPEndPoint::OnReleased (connection);
}


void
GMSIPEndpoint::OnNoAnswerTimeout (PTimer &,
                                  INT) 
{
  gchar *forward_host = NULL;
  gboolean forward_on_no_answer = FALSE;
  
  if (endpoint.GetCallingState () == GMManager::Called) {
   
    gnomemeeting_threads_enter ();
    forward_host = gm_conf_get_string (SIP_KEY "forward_host");
    forward_on_no_answer = 
      gm_conf_get_bool (CALL_FORWARDING_KEY "forward_on_no_answer");
    gnomemeeting_threads_leave ();

    if (forward_host && forward_on_no_answer) {
      
      PSafePtr<OpalCall> call = 
        endpoint.FindCallWithLock (endpoint.GetCurrentCallToken ());
      PSafePtr<OpalConnection> con = 
        endpoint.GetConnection (call, TRUE);
    
      con->ForwardCall (forward_host);
    }
    else
      ClearAllCalls (OpalConnection::EndedByNoAnswer, FALSE);

    g_free (forward_host);
  }
}


