
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


#include "config.h"

#include "sip.h"
#include "pcss.h"
#include "ekiga.h"
#include "urlhandler.h"

#include "main.h"
#include "preferences.h"
#include "statusicon.h"
#include "misc.h"
#ifdef HAVE_DBUS
#include "dbus.h"
#endif

#include <gmconf.h>
#include <gmdialog.h>

#include <ptlib/ethsock.h>
#include <opal/transcoders.h>
#include <sip/handlers.h>

#include "presence-core.h"

#define new PNEW



/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with the SIP Outbound Proxy changes.
 * BEHAVIOR     :  It updates the endpoint.
 * PRE          :  data is a pointer to the GMSIPEndPoint.
 */
static void outbound_proxy_changed_nt (G_GNUC_UNUSED gpointer id,
                                       GmConfEntry *entry,
                                       gpointer data);


/* DESCRIPTION  :  This callback is called to update capabilities when the
 *                 DTMF mode is changed.
 * BEHAVIOR     :  Updates them.
 * PRE          :  data is a pointer to the GMSIPEndPoint.
 */
static void dtmf_mode_changed_nt (G_GNUC_UNUSED gpointer id,
                                  GmConfEntry *entry,
                                  gpointer data);


/* DESCRIPTION  :  This callback is called to update capabilities when the
 *                 NAT binding timeout is changed.
 * BEHAVIOR     :  Update it.
 * PRE          :  data is a pointer to the GMSIPEndPoint.
 */
static void dtmf_mode_changed_nt (G_GNUC_UNUSED gpointer id,
                                  GmConfEntry *entry,
                                  gpointer data);


static void
outbound_proxy_changed_nt (G_GNUC_UNUSED gpointer id,
                           GmConfEntry *entry,
                           gpointer data)
{
  gchar *outbound_proxy_host = NULL;

  g_return_if_fail (data != NULL);

  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING) {

    gdk_threads_enter ();
    outbound_proxy_host = gm_conf_get_string (SIP_KEY "outbound_proxy_host");
    gdk_threads_leave ();

    ((GMSIPEndpoint *) data)->SetProxy (outbound_proxy_host); 
    g_free (outbound_proxy_host);
  }
}


static void
dtmf_mode_changed_nt (G_GNUC_UNUSED gpointer id,
                      GmConfEntry *entry,
                      G_GNUC_UNUSED gpointer data)
{
  unsigned int mode = 0;

  g_return_if_fail (data != NULL);

  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    mode = gm_conf_entry_get_int (entry);
    ((GMSIPEndpoint *) data)->SetUserInputMode (mode);
  }
}


static void
nat_binding_timeout_changed_nt (G_GNUC_UNUSED gpointer id,
                                GmConfEntry *entry,
                                G_GNUC_UNUSED gpointer data)
{
  unsigned int binding_timeout = 0;

  g_return_if_fail (data != NULL);

  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    binding_timeout = gm_conf_entry_get_int (entry);
    ((GMSIPEndpoint *) data)->SetNATBindingTimeout (PTimeInterval (0, binding_timeout));
  }
}

/* The class */
GMSIPEndpoint::GMSIPEndpoint (GMManager & ep, Ekiga::ServiceCore & _core)
: SIPEndPoint (ep), endpoint (ep), core (_core)
{
  NoAnswerTimer.SetNotifier (PCREATE_NOTIFIER (OnNoAnswerTimeout));
  Init ();
}


GMSIPEndpoint::~GMSIPEndpoint ()
{
}


void
GMSIPEndpoint::fetch (const std::string _uri)
{
  std::string::size_type loc = _uri.find ("@", 0);
  std::string domain;

  if (loc != string::npos) 
    domain = _uri.substr (loc+1);

  if (std::find (uris.begin (), uris.end (), _uri) == uris.end ())
    uris.push_back (_uri);

  if (std::find (domains.begin (), domains.end (), domain) != domains.end ()
      && !IsSubscribed (SIPSubscribe::Presence, _uri.c_str ())) {

    SIPSubscribe::SubscribeType type = SIPSubscribe::Presence;
    Subscribe (type, 1800, PString (_uri.c_str ()));
  }
}


void
GMSIPEndpoint::unfetch (const std::string uri)
{
  if (IsSubscribed (SIPSubscribe::Presence, uri.c_str ())) {

    SIPSubscribe::SubscribeType type = SIPSubscribe::Presence;
    Subscribe (type, 0, PString (uri.c_str ()));

    uris.remove (uri);
  }
}


void 
GMSIPEndpoint::Init ()
{
  gchar *outbound_proxy_host = NULL;
  int binding_timeout = 60;
  unsigned int mode = 0;

  /* Read configuration */
  gnomemeeting_threads_enter ();
  outbound_proxy_host = gm_conf_get_string (SIP_KEY "outbound_proxy_host");
  binding_timeout = gm_conf_get_int (NAT_KEY "binding_timeout");
  mode = gm_conf_get_int (SIP_KEY "dtmf_mode");
  gnomemeeting_threads_leave ();

  /* Timeouts */
  SetAckTimeout (PTimeInterval (0, 32));
  SetPduCleanUpTimeout (PTimeInterval (0, 1));
  SetInviteTimeout (PTimeInterval (0, 6));
  SetNonInviteTimeout (PTimeInterval (0, 6));
  SetNATBindingTimeout (PTimeInterval (0, binding_timeout));
  SetRetryTimeouts (500, 4000);
  SetMaxRetries (8);

  /* Input mode */
  SetUserInputMode (mode);

  /* Update the User Agent */
  SetUserAgent ("Ekiga/" PACKAGE_VERSION);

  /* Initialise internal parameters */
  if (outbound_proxy_host && !PString (outbound_proxy_host).IsEmpty ())
    SetProxy (outbound_proxy_host);
  SetNATBindingRefreshMethod (SIPEndPoint::EmptyRequest);

  /* Notifiers */
  gm_conf_notifier_add (SIP_KEY "outbound_proxy_host",
                        outbound_proxy_changed_nt, (gpointer) this);
  gm_conf_notifier_add (SIP_KEY "dtmf_mode",
                        dtmf_mode_changed_nt, (gpointer) this);
  gm_conf_notifier_add (NAT_KEY "binding_timeout",
                        nat_binding_timeout_changed_nt, (gpointer) this);

  g_free (outbound_proxy_host);
}


bool 
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
GMSIPEndpoint::SetUserInputMode (unsigned int mode)
{
  switch (mode) 
    {
    case 0:
      SetSendUserInputMode (OpalConnection::SendUserInputAsTone);
      break;
    case 1:
      SetSendUserInputMode (OpalConnection::SendUserInputAsInlineRFC2833);
      break;
    default:
      break;
    }
}


void
GMSIPEndpoint::PublishPresence (const PString & to,
                                guint state)
{
  PString status;
  PString note;
  PString body;

  switch (state) {

  case CONTACT_ONLINE:
    status = "Online";
    note = "open";
    break;

  case CONTACT_OFFLINE:
  case CONTACT_INVISIBLE:
    status = "Offline";
    note = "closed";
    break;

  case CONTACT_DND:
    status = "Do Not Disturb";
    note = "open";
    break;

  case CONTACT_AWAY:
    status = "Away";
    note = "open";
    break;

  case CONTACT_FREEFORCHAT:
    status = "Free For Chat";
    note = "open";
    break;

  default:
    break;
  }

  body = SIPPublishHandler::BuildBody (to, note, status);
  Publish (to, body, 500); // FIXME
}


void 
GMSIPEndpoint::Register (const PString & aor,
                         const PString & authUserName,
                         const PString & password,
                         unsigned int expires,
                         bool unregister)
{
  bool result = false;

  /* Account is enabled, and we are not registered */
  if (!unregister && !IsRegistered (aor)) {

    /* Signal the OpalManager */
    endpoint.OnRegistering (aor, true);

    /* Trigger registering */
    result = SIPEndPoint::Register (PString::Empty (), aor, authUserName, password, PString::Empty (), expires);

    if (!result) 
      endpoint.OnRegistrationFailed (aor, true, _("Failed"));
  }
  else if (unregister && IsRegistered (aor)) {

    SIPEndPoint::Unregister (aor);
  }
}


void
GMSIPEndpoint::OnRegistered (const PString & _aor,
                             bool was_registering)
{
  std::string aor = (const char *) _aor;
  std::string::size_type found;
  std::string::size_type loc = aor.find ("@", 0);
  std::string server;

  guint status = CONTACT_ONLINE;

  /* Signal the OpalManager */
  endpoint.OnRegistered (aor, was_registering);

  /* Signal the SIPEndpoint */
  SIPEndPoint::OnRegistered (aor, was_registering);

  if (loc != string::npos) {

    server = aor.substr (loc+1);

    if (server.empty ())
      return;

    if (was_registering && std::find (domains.begin (), domains.end (), server) == domains.end ()) 
      domains.push_back (server);

    if (!was_registering && std::find (domains.begin (), domains.end (), server) != domains.end ()) 
      domains.remove (server);

    for (std::list<std::string>::const_iterator iter = uris.begin (); 
         iter != uris.end () ; 
         iter++) {

      found = (*iter).find (server, 0);
      if (found != string::npos
          && ((was_registering && !IsSubscribed (SIPSubscribe::Presence, (*iter).c_str ()))
              || (!was_registering && IsSubscribed (SIPSubscribe::Presence, (*iter).c_str ())))) {

        SIPSubscribe::SubscribeType type = SIPSubscribe::Presence;
        Subscribe (type, was_registering ? 500 : 0, PString ((*iter).c_str ()));
        if (!was_registering)
          uris.remove (*iter);
      }
    }
  }

  /* Publish current state */
  if (was_registering)
    status = gm_conf_get_int (PERSONAL_DATA_KEY "status");
  else
    status = CONTACT_OFFLINE;
  PublishPresence (aor, status);

  /* Subscribe for MWI */
  if (!IsSubscribed (SIPSubscribe::MessageSummary, aor)) { 
    SIPSubscribe::SubscribeType t = SIPSubscribe::MessageSummary;
    Subscribe (t, 3600, aor);
  }
}


void
GMSIPEndpoint::OnRegistrationFailed (const PString & aor,
                                     SIP_PDU::StatusCodes r,
                                     bool wasRegistering)
{
  std::string info;

  switch (r) {

  case SIP_PDU::Failure_BadRequest:
    info = _("Bad request");
    break;

  case SIP_PDU::Failure_PaymentRequired:
    info = _("Payment required");
    break;

  case SIP_PDU::Failure_UnAuthorised:
  case SIP_PDU::Failure_Forbidden:
    info = _("Forbidden");
    break;

  case SIP_PDU::Failure_RequestTimeout:
    info = _("Timeout");
    break;

  case SIP_PDU::Failure_Conflict:
    info = _("Conflict");
    break;

  case SIP_PDU::Failure_TemporarilyUnavailable:
    info = _("Temporarily unavailable");
    break;

  case SIP_PDU::Failure_NotAcceptable:
    info = _("Not Acceptable");
    break;

  case SIP_PDU::IllegalStatusCode:
  case SIP_PDU::Redirection_MultipleChoices:
  case SIP_PDU::Redirection_MovedPermanently:
  case SIP_PDU::Redirection_MovedTemporarily:
  case SIP_PDU::Redirection_UseProxy:
  case SIP_PDU::Redirection_AlternativeService:
  case SIP_PDU::Failure_NotFound:
  case SIP_PDU::Failure_MethodNotAllowed:
  case SIP_PDU::Failure_ProxyAuthenticationRequired:
  case SIP_PDU::Failure_Gone:
  case SIP_PDU::Failure_LengthRequired:
  case SIP_PDU::Failure_RequestEntityTooLarge:
  case SIP_PDU::Failure_RequestURITooLong:
  case SIP_PDU::Failure_UnsupportedMediaType:
  case SIP_PDU::Failure_UnsupportedURIScheme:
  case SIP_PDU::Failure_BadExtension:
  case SIP_PDU::Failure_ExtensionRequired:
  case SIP_PDU::Failure_IntervalTooBrief:
  case SIP_PDU::Failure_TransactionDoesNotExist:
  case SIP_PDU::Failure_LoopDetected:
  case SIP_PDU::Failure_TooManyHops:
  case SIP_PDU::Failure_AddressIncomplete:
  case SIP_PDU::Failure_Ambiguous:
  case SIP_PDU::Failure_BusyHere:
  case SIP_PDU::Failure_RequestTerminated:
  case SIP_PDU::Failure_NotAcceptableHere:
  case SIP_PDU::Failure_BadEvent:
  case SIP_PDU::Failure_RequestPending:
  case SIP_PDU::Failure_Undecipherable:
  case SIP_PDU::Failure_InternalServerError:
  case SIP_PDU::Failure_NotImplemented:
  case SIP_PDU::Failure_BadGateway:
  case SIP_PDU::Failure_ServiceUnavailable:
  case SIP_PDU::Failure_ServerTimeout:
  case SIP_PDU::Failure_SIPVersionNotSupported:
  case SIP_PDU::Failure_MessageTooLarge:
  case SIP_PDU::GlobalFailure_BusyEverywhere:
  case SIP_PDU::GlobalFailure_Decline:
  case SIP_PDU::GlobalFailure_DoesNotExistAnywhere:
  case SIP_PDU::GlobalFailure_NotAcceptable:
  case SIP_PDU::MaxStatusCode:
  default:
    info = _("Failed");

  case SIP_PDU::Information_Trying:
  case SIP_PDU::Information_Ringing:
  case SIP_PDU::Information_CallForwarded:
  case SIP_PDU::Information_Queued:
  case SIP_PDU::Information_Session_Progress:
  case SIP_PDU::Successful_OK:
  case SIP_PDU::Successful_Accepted:
    break;
  }

  /* Signal the OpalManager */
  endpoint.OnRegistrationFailed (aor, wasRegistering, info);

  /* Signal the SIP Endpoint */
  SIPEndPoint::OnRegistrationFailed (aor, r, wasRegistering);
}


bool 
GMSIPEndpoint::OnIncomingConnection (OpalConnection &connection,
                                     G_GNUC_UNUSED unsigned options,
                                     G_GNUC_UNUSED OpalConnection::StringOptions * stroptions)
{
  PSafePtr<OpalConnection> con = NULL;
  PSafePtr<OpalCall> call = NULL;

  gchar *forward_host = NULL;

  guint status = CONTACT_ONLINE;
  gboolean busy_forward = FALSE;
  gboolean always_forward = FALSE;
  int no_answer_timeout = FALSE;

  bool res = FALSE;

  unsigned reason = 0;

  PTRACE (3, "GMSIPEndpoint\tIncoming connection");

  gnomemeeting_threads_enter ();
  forward_host = gm_conf_get_string (SIP_KEY "forward_host"); 
  busy_forward = gm_conf_get_bool (CALL_FORWARDING_KEY "forward_on_busy");
  always_forward = gm_conf_get_bool (CALL_FORWARDING_KEY "always_forward");
  status = gm_conf_get_int (PERSONAL_DATA_KEY "status");
  no_answer_timeout =
    gm_conf_get_int (CALL_OPTIONS_KEY "no_answer_timeout");
  gnomemeeting_threads_leave ();

  call = endpoint.FindCallWithLock (endpoint.GetCurrentCallToken());
  if (call)
    con = endpoint.GetConnection (call, TRUE);
  if ((con && con->GetIdentifier () == connection.GetIdentifier())) {
    return TRUE;
  }

  if (status == CONTACT_DND)
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
  else if (status == CONTACT_FREEFORCHAT)
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
GMSIPEndpoint::OnMWIReceived (const PString & to,
                              G_GNUC_UNUSED SIPSubscribe::MWIType type,
                              const PString & msgs)
{
  GtkWidget *accounts_window = NULL;

  endpoint.OnMWIReceived (to, msgs);

  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();

  // FIXME when migrating the accounts window
  gnomemeeting_threads_enter ();
  gm_accounts_window_update_account_state (accounts_window,
                                           FALSE,
                                           to,
                                           NULL,
                                           (const char *) msgs);
  gnomemeeting_threads_leave ();
}


void 
GMSIPEndpoint::OnReceivedMESSAGE (G_GNUC_UNUSED OpalTransport & transport,
                                  SIP_PDU & pdu)
{
  PString *last = NULL;
  PString *val = NULL;

  PString from = pdu.GetMIME().GetFrom();   
  PINDEX j = from.Find (';');
  if (j != P_MAX_INDEX)
    from = from.Left(j); // Remove all parameters
  j = from.Find ('<');
  if (j != P_MAX_INDEX && from.Find ('>') == P_MAX_INDEX)
    from += '>';

  PWaitAndSignal m(msgDataMutex);
  last = msgData.GetAt (SIPURL (from).AsString ());
  if (!last || *last != pdu.GetMIME ().GetCallID ()) {

    val = new PString (pdu.GetMIME ().GetCallID ());
    msgData.SetAt (SIPURL (from).AsString (), val);
    endpoint.OnMessageReceived(from, pdu.GetEntityBody());
  }
}


void 
GMSIPEndpoint::OnMessageFailed (const SIPURL & messageUrl,
                                SIP_PDU::StatusCodes reason)
{
  endpoint.OnMessageFailed (messageUrl, reason);
}


void
GMSIPEndpoint::Message (const PString & to,
                        const PString & body)
{
  SIPEndPoint::Message (to, body);
  endpoint.OnMessageSent (to, body);
}


SIPURL
GMSIPEndpoint::GetRegisteredPartyName (const SIPURL & host)
{
  GmAccount *account = NULL;

  PString local_address;
  PIPSocket::Address address;
  WORD port;
  PString url;
  SIPURL registration_address;

  PSafePtr<SIPHandler> info = activeSIPHandlers.FindSIPHandlerByDomain(host.GetHostName (), SIP_PDU::Method_REGISTER, PSafeReadOnly);

  if (info != NULL)
    registration_address = info->GetTargetAddress();

  // If we are not exchanging messages with a local party, use the default account
  // otherwise, use a direct call address in the from field
  if (host.GetHostAddress ().GetIpAndPort (address, port) && !manager.IsLocalAddress (address)) {

    account = gnomemeeting_get_default_account ("SIP");
    if (account && account->enabled) {

      if (info == NULL || registration_address.GetHostName () == account->host) {

        if (PString(account->username).Find("@") == P_MAX_INDEX)
          url = PString (account->username) + "@" + PString (account->host);
        else
          url = PString (account->username);

        return url;
      }
    }
  }


  // Not found (or local party)
  local_address = GetListeners()[0].GetLocalAddress();

  PINDEX j = local_address.Find ('$');
  if (j != P_MAX_INDEX)
    local_address = local_address.Mid (j+1);
  SIPURL myself = 
    SIPURL ("\"" + GetDefaultDisplayName () + "\" <" + PString ("sip:") + GetDefaultLocalPartyName() + "@" + local_address + ";transport=udp>");

  return myself;
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
GMSIPEndpoint::OnPresenceInfoReceived (const PString & user,
                                       const PString & basic,
                                       const PString & note)
{
  PCaselessString b = basic;
  PCaselessString s = note;

  std::string status = "presence-unknown";
  std::string presence;

  SIPURL sip_uri = SIPURL (user);
  sip_uri.AdjustForRequestURI ();
  std::string _uri = sip_uri.AsString ();

  if (b.Find ("Closed") != P_MAX_INDEX) {
    presence = "presence-offline";
    status = _("Offline");
  }
  else {
    presence = "presence-online";
    status = _("Online");
  }

  if (s.Find ("Away") != P_MAX_INDEX) {
    presence = "presence-away";
    status = _("Away");
  }
  else if (s.Find ("On the phone") != P_MAX_INDEX
           || s.Find ("Ringing") != P_MAX_INDEX
           || s.Find ("Do Not Disturb") != P_MAX_INDEX) {
    presence = "presence-dnd";
    status = _("Do Not Disturb");
  }
  else if (s.Find ("Free For Chat") != P_MAX_INDEX) {
    presence = "presence-freeforchat";
    status = _("Free For Chat");
  }

  Ekiga::PresenceCore *presence_core =
    dynamic_cast<Ekiga::PresenceCore *>(core.get ("presence-core"));
  Ekiga::Runtime *runtime = 
    dynamic_cast<Ekiga::Runtime *> (core.get ("runtime")); 

  if (runtime) {
   
    /**
     * TODO
     * Wouldn't it be convenient to emit the signal and have the presence core listen to it ?
     */
    runtime->run_in_main (sigc::bind (presence_core->presence_received.make_slot (), _uri, presence));
    runtime->run_in_main (sigc::bind (presence_core->status_received.make_slot (), _uri, status));
  }
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


