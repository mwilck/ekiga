
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

#include <sstream>

#include "sip.h"

#include "presence-core.h"
#include "personal-details.h"


/* The class */
GMSIPEndpoint::GMSIPEndpoint (GMManager & ep, Ekiga::ServiceCore & _core)
: SIPEndPoint (ep), 
  Ekiga::PresencePublisher (_core), 
  endpoint (ep), 
  core (_core),
  runtime (*(dynamic_cast<Ekiga::Runtime *> (core.get ("runtime"))))
{
  uri_prefix = "sip:";

  /* Timeouts */
  SetAckTimeout (PTimeInterval (0, 32));
  SetPduCleanUpTimeout (PTimeInterval (0, 1));
  SetInviteTimeout (PTimeInterval (0, 6));
  SetNonInviteTimeout (PTimeInterval (0, 6));
  SetRetryTimeouts (500, 4000);
  SetMaxRetries (8);

  /* Update the User Agent */
  SetUserAgent ("Ekiga/" PACKAGE_VERSION);

  Ekiga::PersonalDetails *details = dynamic_cast<Ekiga::PersonalDetails *> (_core.get ("personal-details"));
  if (details)
    publish (*details);
}


GMSIPEndpoint::~GMSIPEndpoint ()
{
}


bool GMSIPEndpoint::dial (const std::string uri)
{
  return endpoint.dial (uri);
}


bool GMSIPEndpoint::send_message (const std::string uri, const std::string message)
{
  if (!uri.empty () && !message.empty ()) {
    Message (uri.c_str (), message.c_str ());

    return true;
  }

  return false;
}


Ekiga::CodecList GMSIPEndpoint::get_codecs ()
{
  return endpoint.get_codecs ();
}


void GMSIPEndpoint::set_codecs (Ekiga::CodecList & _codecs)
{
  endpoint.set_codecs (_codecs);
}

bool GMSIPEndpoint::populate_menu (Ekiga::Contact &contact,
                                   Ekiga::MenuBuilder &builder)
{
  std::string name = contact.get_name ();
  std::map<std::string, std::string> uris = contact.get_uris ();

  return menu_builder_add_actions (name, uris, builder);
}


bool GMSIPEndpoint::populate_menu (const std::string uri,
                                   Ekiga::MenuBuilder & builder)
{
  std::map<std::string, std::string> uris; 
  uris [""] = uri;

  return menu_builder_add_actions ("", uris, builder);
}


bool GMSIPEndpoint::menu_builder_add_actions (const std::string & fullname,
                                              std::map<std::string,std::string> & uris,
                                              Ekiga::MenuBuilder & builder)
{
  bool populated = false;

  /* Add actions of type "call" for all uris */
  for (std::map<std::string, std::string>::const_iterator iter = uris.begin ();
       iter != uris.end ();
       iter++) {

    std::string action = _("Call");

    if (!iter->first.empty ())
      action = action + " [" + iter->first + "]";

    builder.add_action ("call", action, sigc::bind (sigc::mem_fun (this, &GMSIPEndpoint::on_dial), iter->second));

    populated = true;
  }

  /* Add actions of type "message" for all uris */
  for (std::map<std::string, std::string>::const_iterator iter = uris.begin ();
       iter != uris.end ();
       iter++) {

    std::string action = _("Message");

    if (!iter->first.empty ())
      action = action + " [" + iter->first + "]";

    builder.add_action ("message", action, sigc::bind (sigc::mem_fun (this, &GMSIPEndpoint::on_message), fullname, iter->second));

    populated = true;
  }

  return populated;
}



void
GMSIPEndpoint::fetch (const std::string _uri)
{
  std::string::size_type loc = _uri.find ("@", 0);
  std::string domain;

  if (loc != string::npos) 
    domain = _uri.substr (loc+1);

  if (std::find (subscribed_uris.begin (), subscribed_uris.end (), _uri) == subscribed_uris.end ())
    subscribed_uris.push_back (_uri);

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

    subscribed_uris.remove (uri);
  }
}


void 
GMSIPEndpoint::publish (const Ekiga::PersonalDetails & details)
{
  std::string hostname = (const char *) PIPSocket::GetHostName ();
  // TODO: move this code outside of this class and allow a 
  // more complete document
  std::string status = ((Ekiga::PersonalDetails &) (details)).get_short_status ();
  for (std::list<std::string>::iterator it = aors.begin ();
       it != aors.end ();
       it++) {
    std::string to = it->substr (4);
    PString data;
    data += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n";

    data += "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"pres:";
    data += to;
    data += "\">\r\n";

    data += "<tuple id=\"";
    data += to; 
    data += "_on_";
    data += hostname;
    data += "\">\r\n";

    data += "<note>";
    data += status.c_str ();
    data += "</note>\r\n";

    data += "<status>\r\n";
    data += "<basic>";
    data += "open";
    data += "</basic>\r\n";
    data += "</status>\r\n";

    data += "<contact priority=\"1\">sip:";
    data += to;
    data += "</contact>\r\n";

    data += "</tuple>\r\n";
    data += "</presence>\r\n";
    Publish (to.c_str (), data, 500); // TODO: allow to change the 500 
  }
}


void 
GMSIPEndpoint::set_outbound_proxy (const std::string & uri)
{
  SIPEndPoint::SetProxy (uri.c_str ());
}


void 
GMSIPEndpoint::set_dtmf_mode (unsigned int mode)
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
GMSIPEndpoint::set_nat_binding_delay (unsigned int delay)
{
  SIPEndPoint::SetNATBindingTimeout (PTimeInterval (0, delay));
}

void 
GMSIPEndpoint::set_forward_host (const std::string & uri)
{
  forward_uri = uri;
}

void 
GMSIPEndpoint::set_forward_on_busy (bool enabled)
{
  forward_on_busy = enabled;
}

void 
GMSIPEndpoint::set_unconditional_forward (bool enabled)
{
  unconditional_forward = enabled;
}

void 
GMSIPEndpoint::set_forward_on_no_answer (bool enabled)
{
  forward_on_no_answer = enabled;
}

void 
GMSIPEndpoint::set_no_answer_timeout (const unsigned timeout)
{
  no_answer_timeout = timeout;
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
GMSIPEndpoint::Register (const PString & _aor,
                         const PString & authUserName,
                         const PString & password,
                         unsigned int expires,
                         bool unregister)
{
  std::string aor = (const char *) _aor;
  std::stringstream strm;
  bool result = false;

  /* Account is enabled, and we are not registered */
  if (!unregister && !IsRegistered (aor)) {

    if (aor.find (uri_prefix) == std::string::npos) 
      strm << uri_prefix << aor;
    else
      strm << aor;

    /* Signal the OpalManager */
    endpoint.OnRegistering (strm.str (), true); // TODO we could directly emit the signal from here

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
  std::stringstream strm;

  if (aor.find (uri_prefix) == std::string::npos) 
    strm << uri_prefix << aor;
  else
    strm << aor;

  std::list<std::string>::iterator it = find (aors.begin (), aors.end (), aor);

  if (was_registering) {
   
    if (it == aors.end ())
      aors.push_back (strm.str ());
  }
  else {

    if (it != aors.end ())
      aors.remove (strm.str ());
  }

  /* Signal the OpalManager */
  endpoint.OnRegistered (strm.str (), was_registering); // TODO we could directly emit the signal from here

  if (loc != string::npos) {

    server = aor.substr (loc+1);

    if (server.empty ())
      return;

    if (was_registering && std::find (domains.begin (), domains.end (), server) == domains.end ()) 
      domains.push_back (server);

    if (!was_registering && std::find (domains.begin (), domains.end (), server) != domains.end ()) 
      domains.remove (server);

    for (std::list<std::string>::const_iterator iter = subscribed_uris.begin (); 
         iter != subscribed_uris.end () ; 
         iter++) {

      found = (*iter).find (server, 0);
      if (found != string::npos
          && ((was_registering && !IsSubscribed (SIPSubscribe::Presence, (*iter).c_str ()))
              || (!was_registering && IsSubscribed (SIPSubscribe::Presence, (*iter).c_str ())))) {

        SIPSubscribe::SubscribeType type = SIPSubscribe::Presence;
        Subscribe (type, was_registering ? 500 : 0, PString ((*iter).c_str ()));
        if (!was_registering)
          subscribed_uris.remove (*iter);
      }
    }
  }

  /* Subscribe for MWI */
  if (!IsSubscribed (SIPSubscribe::MessageSummary, aor)) { 
    SIPSubscribe::SubscribeType t = SIPSubscribe::MessageSummary;
    Subscribe (t, 3600, aor);
  }
}


bool GMSIPEndpoint::MakeConnection (OpalCall & _call,
                                    const PString &party,
                                    void *userData,
                                    unsigned int options,
                                    OpalConnection::StringOptions *stringOptions)
{
  Ekiga::Call *call = dynamic_cast<Ekiga::Call *> (&_call);
  runtime.run_in_main (sigc::bind (new_call, call));

  return SIPEndPoint::MakeConnection (_call, party, userData, options, stringOptions);
}


void
GMSIPEndpoint::OnRegistrationFailed (const PString & _aor,
                                     SIP_PDU::StatusCodes r,
                                     bool wasRegistering)
{
  std::stringstream strm;
  std::string info;
  std::string aor = (const char *) _aor;

  if (aor.find (uri_prefix) == std::string::npos) 
    strm << uri_prefix << aor;
  else
    strm << aor;

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
  endpoint.OnRegistrationFailed (strm.str ().c_str (), wasRegistering, info);

  /* Signal the SIP Endpoint */
  SIPEndPoint::OnRegistrationFailed (strm.str ().c_str (), r, wasRegistering);
}


bool 
GMSIPEndpoint::OnIncomingConnection (OpalConnection &connection,
                                     G_GNUC_UNUSED unsigned options,
                                     G_GNUC_UNUSED OpalConnection::StringOptions * stroptions)
{
  PSafePtr<OpalConnection> con = NULL;
  PSafePtr<OpalCall> call = NULL;

  unsigned reason = 0;

  PTRACE (3, "GMSIPEndpoint\tIncoming connection");

  call = endpoint.FindCallWithLock (endpoint.GetCurrentCallToken());
  if (call)
    con = endpoint.GetConnection (call, TRUE);
  if ((con && con->GetIdentifier () == connection.GetIdentifier())) {
    return TRUE;
  }

  if (!forward_uri.empty () && unconditional_forward)
    reason = 2; // Forward
  else if (endpoint.GetCallingState () != GMManager::Standby) { // TODO : to remove

    if (!forward_uri.empty () && forward_on_busy)
      reason = 2; // Forward
    else
      reason = 1; // Reject
  }
  else
    reason = 0; // Ask the user

  return endpoint.OnIncomingConnection (connection, reason, forward_uri.c_str ());
}


void 
GMSIPEndpoint::OnMWIReceived (const PString & to,
                              G_GNUC_UNUSED SIPSubscribe::MWIType type,
                              const PString & msgs)
{
  endpoint.OnMWIReceived (to, msgs);
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

    SIPURL uri = from;
    std::string display_name = (const char *) uri.GetDisplayName ();
    uri.AdjustForRequestURI ();
    std::string message_uri = (const char *) uri.AsString ();
    std::string message = (const char *) pdu.GetEntityBody ();

    // FIXME should be a signal
    //audiooutput_core.play_event("new_message_sound");

    runtime.run_in_main (sigc::bind (im_received.make_slot (), display_name, message_uri, message));
  }
}


void 
GMSIPEndpoint::OnMessageFailed (const SIPURL & messageUrl,
                                SIP_PDU::StatusCodes /*reason*/)
{
  SIPURL to = messageUrl;
  to.AdjustForRequestURI ();
  std::string uri = (const char *) to.AsString ();
  runtime.run_in_main (sigc::bind (im_failed.make_slot (), uri, 
                                   _("Could not send message")));
}


void
GMSIPEndpoint::Message (const PString & _to,
                        const PString & body)
{
  SIPEndPoint::Message (_to, body);

  SIPURL to = _to;
  to.AdjustForRequestURI ();
  std::string uri = (const char *) to.AsString ();
  std::string message = (const char *) body;
  runtime.run_in_main (sigc::bind (im_sent.make_slot (), uri, message));
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

  /* If we are registered to an account corresponding to host, use it.
   */
  PSafePtr<SIPHandler> info = activeSIPHandlers.FindSIPHandlerByDomain(host.GetHostName (), SIP_PDU::Method_REGISTER, PSafeReadOnly);
  if (info != NULL) {

    return SIPURL ("\"" + GetDefaultDisplayName () + "\" <" + info->GetTargetAddress ().AsString () + ">");
  }
  else {

    /* If we are not registered to host, 
     * then use the default account as outgoing identity.
     * If we are exchanging messages with a peer on our network,
     * then do not use the default account as outgoing identity.
     */
    if (host.GetHostAddress ().GetIpAndPort (address, port) && !manager.IsLocalAddress (address)) {

      account = gnomemeeting_get_default_account ("SIP");
      if (account && account->enabled) {

        if (PString(account->username).Find("@") == P_MAX_INDEX)
          url = PString (account->username) + "@" + PString (account->host);
        else
          url = PString (account->username);

        return SIPURL ("\"" + GetDefaultDisplayName () + "\" <" + url + ">");
      }
    }
  }

  /* As a last resort, ie not registered to host, no default account or
   * dialog with a local peer, then use the local address 
   */
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
  PTRACE (3, "GMSIPEndpoint\t SIP connection established");
  SIPEndPoint::OnEstablished (connection);
}


void 
GMSIPEndpoint::OnReleased (OpalConnection &connection)
{
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

  std::string status;
  std::string presence = "presence-unknown";

  SIPURL sip_uri = SIPURL (user);
  sip_uri.AdjustForRequestURI ();
  std::string _uri = sip_uri.AsString ();

  if (b.Find ("Closed") != P_MAX_INDEX) {
    presence = "presence-offline";
  }
  else {
    presence = "presence-online";
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

  /**
   * TODO
   * Wouldn't it be convenient to emit the signal and have the presence core listen to it ?
   */
  runtime.run_in_main (sigc::bind (presence_core->presence_received.make_slot (), _uri, presence));
  runtime.run_in_main (sigc::bind (presence_core->status_received.make_slot (), _uri, status));
}


void GMSIPEndpoint::on_dial (std::string uri)
{
  endpoint.dial (uri);
}


void GMSIPEndpoint::on_message (std::string name,
                                std::string uri)
{
  runtime.run_in_main (sigc::bind (new_chat.make_slot (), name, uri));
}
