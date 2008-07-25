
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

#include <algorithm>
#include <sstream>

#include "sip.h"

#include "opal-call.h"

#include "presence-core.h"
#include "account-core.h"
#include "chat-core.h"
#include "personal-details.h"
#include "opal-account.h"

namespace Opal {

  namespace Sip {

    class dialer : public PThread
    {
      PCLASSINFO(dialer, PThread);

    public:

      dialer (const std::string & uri, Opal::CallManager & ep) 
        : PThread (1000, AutoDeleteThread), 
        dial_uri (uri),
        endpoint (ep) 
      {
        this->Resume ();
      };

      void Main () 
        {
          PString token;
          endpoint.SetUpCall ("pc:*", dial_uri, token);
        };

    private:
      const std::string dial_uri;
      Opal::CallManager & endpoint;
    };


    class subscriber : public PThread
    {
      PCLASSINFO(subscriber, PThread);

    public:
      subscriber (const Opal::Account & _account,
                  Opal::Sip::CallProtocolManager & ep) 
        : PThread (1000, AutoDeleteThread),
        account (_account),
        endpoint (ep) 
      {
        this->Resume ();
      };

      void Main () 
        {
          endpoint.Register (account);
        };

    private:
      const Opal::Account & account;
      Opal::Sip::CallProtocolManager & endpoint;
    };
  };
};


using namespace Opal::Sip;


/* The class */
CallProtocolManager::CallProtocolManager (Opal::CallManager & ep, 
                                          Ekiga::ServiceCore & _core, 
                                          unsigned _listen_port)
                    : SIPEndPoint (ep), 
                      Ekiga::PresencePublisher (_core), 
                      endpoint (ep), 
                      core (_core),
                      presence_core (*(dynamic_cast<Ekiga::PresenceCore *> (core.get ("presence-core")))),
                      runtime (*(dynamic_cast<Ekiga::Runtime *> (core.get ("runtime")))),
                      account_core (*(dynamic_cast<Ekiga::AccountCore *> (core.get ("account-core"))))
{
  Ekiga::ChatCore* chat_core;

  protocol_name = "sip";
  uri_prefix = "sip:";
  listen_port = _listen_port;

  chat_core = dynamic_cast<Ekiga::ChatCore *> (core.get ("chat-core"));
  dialect = new SIP::Dialect (core, sigc::mem_fun (this, &CallProtocolManager::send_message));
  chat_core->add_dialect (*dialect);

  /* Timeouts */
  SetAckTimeout (PTimeInterval (0, 32));
  SetPduCleanUpTimeout (PTimeInterval (0, 1));
  SetInviteTimeout (PTimeInterval (0, 6));
  SetNonInviteTimeout (PTimeInterval (0, 6));
  SetRetryTimeouts (500, 4000);
  SetMaxRetries (8);

  /* Start listener */
  set_listen_port (listen_port);

  /* Update the User Agent */
  SetUserAgent ("Ekiga/" PACKAGE_VERSION);

  /* Ready to take calls */
  endpoint.AddRouteEntry("sip:.* = pc:<db>");
  endpoint.AddRouteEntry("pc:.* = sip:<da>");

  /* NAT Binding */
  SetNATBindingRefreshMethod (SIPEndPoint::EmptyRequest);

  Ekiga::PersonalDetails *details = dynamic_cast<Ekiga::PersonalDetails *> (_core.get ("personal-details"));
  if (details)
    publish (*details);
}

CallProtocolManager::~CallProtocolManager ()
{
  delete dialect;
}


bool CallProtocolManager::populate_menu (Ekiga::Contact &contact,
                                         Ekiga::MenuBuilder &builder)
{
  std::string name = contact.get_name ();
  std::map<std::string, std::string> uris = contact.get_uris ();

  return menu_builder_add_actions (name, uris, builder);
}


bool CallProtocolManager::populate_menu (Ekiga::Presentity& presentity,
					 const std::string uri,
                                         Ekiga::MenuBuilder & builder)
{
  std::map<std::string, std::string> uris; 
  uris [""] = uri;

  return menu_builder_add_actions (presentity.get_name (), uris, builder);
}


bool CallProtocolManager::menu_builder_add_actions (const std::string& fullname,
                                                    std::map<std::string,std::string> & uris,
                                                    Ekiga::MenuBuilder & builder)
{
  bool populated = false;

  /* Add actions of type "call" for all uris */
  for (std::map<std::string, std::string>::const_iterator iter = uris.begin ();
       iter != uris.end ();
       iter++) {

    std::string call_action = _("Call");
    std::string forward_action = _("Forward");
    std::string msg_action = _("Message");

    if (!iter->first.empty ()) {

      call_action = call_action + " [" + iter->first + "]";
      forward_action = forward_action + " [" + iter->first + "]";
      msg_action = msg_action + " [" + iter->first + "]";
    }

    if (0 == GetConnectionCount ()) {

      builder.add_action ("call", call_action,
			  sigc::bind (sigc::mem_fun (this, &CallProtocolManager::on_dial), iter->second));
    } else {

      builder.add_action ("forward", forward_action,
			  sigc::bind (sigc::mem_fun (this, &CallProtocolManager::on_forward), iter->second));
    }

    builder.add_action ("message", msg_action,
			sigc::bind (sigc::mem_fun (this, &CallProtocolManager::on_message), iter->second, fullname));

    populated = true;
  }

  return populated;
}


void CallProtocolManager::fetch (const std::string _uri)
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


void CallProtocolManager::unfetch (const std::string uri)
{
  if (IsSubscribed (SIPSubscribe::Presence, uri.c_str ())) {

    SIPSubscribe::SubscribeType type = SIPSubscribe::Presence;
    Subscribe (type, 0, PString (uri.c_str ()));

    subscribed_uris.remove (uri);
  }
}


void CallProtocolManager::publish (const Ekiga::PersonalDetails & details)
{
  std::string hostname = (const char *) PIPSocket::GetHostName ();
  std::string short_status = ((Ekiga::PersonalDetails &) (details)).get_short_status ();
  std::string long_status = ((Ekiga::PersonalDetails &) (details)).get_long_status ();

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
    data += short_status.c_str ();
    if (!long_status.empty ()) {
      data += " - ";
      data += long_status.c_str ();
    }
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
    Publish (to.c_str (), data, 120); // TODO: allow to change the 500 
  }
}


bool CallProtocolManager::send_message (const std::string & _uri, 
                                        const std::string & _message)
{
  if (!_uri.empty () && (_uri.find ("sip:") == 0 || _uri.find (':') == string::npos) && !_message.empty ()) {

    SIPEndPoint::Message (_uri, _message);

    return true;
  }

  return false;
}


bool CallProtocolManager::dial (const std::string & uri)
{
  std::stringstream ustr;

  if (uri.find ("sip:") == 0 || uri.find (":") == string::npos) {

    if (uri.find (":") == string::npos)
      ustr << "sip:" << uri;
    else
      ustr << uri;

    new dialer (ustr.str (), endpoint);

    return true;
  }

  return false;
}


const std::string & CallProtocolManager::get_protocol_name () const
{
  return protocol_name;
}


void CallProtocolManager::set_dtmf_mode (unsigned mode)
{
  switch (mode) {

    // SIP Info
  case 0:
    SetSendUserInputMode (OpalConnection::SendUserInputAsTone);
    break;

    // RFC2833
  case 1:
    SetSendUserInputMode (OpalConnection::SendUserInputAsInlineRFC2833);
    break;
  default:
    break;
  }
}


unsigned CallProtocolManager::get_dtmf_mode () const
{
  // SIP Info
  if (GetSendUserInputMode () == OpalConnection::SendUserInputAsTone)
    return 0;

  // RFC2833
  if (GetSendUserInputMode () == OpalConnection::SendUserInputAsInlineRFC2833)
    return 1;

  return 1;
}


bool CallProtocolManager::set_listen_port (unsigned port)
{
  unsigned udp_min, udp_max;

  interface.protocol = "udp";
  interface.interface = "*";

  endpoint.get_udp_ports (udp_min, udp_max);

  if (port > 0 && port >= udp_min && port <= udp_max) {

    std::stringstream str;
    RemoveListener (NULL);

    str << "udp$*:" << port;
    if (!StartListeners (PStringArray (str.str ()))) {

      port = udp_min;
      str << "udp$*:" << port;
      while (port <= udp_max) {

        if (StartListeners (PStringArray (str.str ()))) {

          interface.port = port;
          return true;
        }

        port++;
      }
    }
    else
      interface.port = port;
  }

  return false;
}


const Ekiga::CallProtocolManager::Interface & CallProtocolManager::get_listen_interface () const
{
  return interface;
}



void CallProtocolManager::set_forward_uri (const std::string & uri)
{
  forward_uri = uri;
}


const std::string & CallProtocolManager::get_forward_uri () const
{
  return forward_uri;
}


void CallProtocolManager::set_outbound_proxy (const std::string & uri)
{
  outbound_proxy = uri;
  SetProxy (SIPURL (outbound_proxy));
}


const std::string & CallProtocolManager::get_outbound_proxy () const
{
  return outbound_proxy;
}


void CallProtocolManager::set_nat_binding_delay (unsigned delay)
{
  SetNATBindingTimeout (PTimeInterval (0, delay));
}


unsigned CallProtocolManager::get_nat_binding_delay ()
{
  return GetNATBindingTimeout ().GetSeconds ();
}


bool CallProtocolManager::subscribe (const Opal::Account & account)
{
  if (account.get_protocol_name () != "SIP")
    return false;

  new subscriber (account, *this);
  return true;
}


bool CallProtocolManager::unsubscribe (const Opal::Account & account)
{
  if (account.get_protocol_name () != "SIP")
    return false;

  new subscriber (account, *this);
  return true;
}


void CallProtocolManager::ShutDown ()
{
  listeners.RemoveAll ();

  for (PSafePtr<SIPTransaction> transaction(transactions, PSafeReference);      transaction != NULL; ++transaction)
    transaction->WaitForCompletion();

  while (activeSIPHandlers.GetSize() > 0) {
    PSafePtr<SIPHandler> handler = activeSIPHandlers;
    activeSIPHandlers.Remove(handler);
  }

  SIPEndPoint::ShutDown ();
}


void CallProtocolManager::Register (const Opal::Account & account)
{
  std::stringstream aor;
  
  aor << account.get_username () << "@" << account.get_host ();
  if (!SIPEndPoint::Register (account.get_host (),
                              account.get_username (),
                              account.get_authentication_username (),
                              account.get_password (),
                              PString::Empty (), 
                              (account.is_enabled () ? account.get_timeout () : 0)))
    OnRegistrationFailed (aor.str (), SIP_PDU::MaxStatusCode, account.is_enabled ());
}


void CallProtocolManager::OnRegistered (const PString & _aor,
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

  /* Signal */
  Ekiga::Account *account = account_core.find_account (strm.str ());
  if (account)
    runtime.run_in_main (sigc::bind (registration_event.make_slot (), account,
                                     was_registering ? Ekiga::AccountCore::Registered : Ekiga::AccountCore::Unregistered,
                                     std::string ()));
}


void CallProtocolManager::OnRegistrationFailed (const PString & _aor,
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
    info = _("Unauthorized");
    break;

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
    info = _("Not acceptable");
    break;

  case SIP_PDU::IllegalStatusCode:
    info = _("Illegal status code");
    break;

  case SIP_PDU::Redirection_MultipleChoices:
    info = _("Multiple choices");
    break;

  case SIP_PDU::Redirection_MovedPermanently:
    info = _("Moved permanently");
    break;

  case SIP_PDU::Redirection_MovedTemporarily:
    info = _("Moved temporarily");
    break;

  case SIP_PDU::Redirection_UseProxy:
    info = _("Use proxy");
    break;

  case SIP_PDU::Redirection_AlternativeService:
    info = _("Alternative service");
    break;

  case SIP_PDU::Failure_NotFound:
    info = _("Not found");
    break;

  case SIP_PDU::Failure_MethodNotAllowed:
    info = _("Method not allowed");
    break;

  case SIP_PDU::Failure_ProxyAuthenticationRequired:
    info = _("Proxy auth. required");
    break;

  case SIP_PDU::Failure_LengthRequired:
    info = _("Length required");
    break;

  case SIP_PDU::Failure_RequestEntityTooLarge:
    info = _("Request entity too big");
    break;

  case SIP_PDU::Failure_RequestURITooLong:
    info = _("Request URI too long");
    break;

  case SIP_PDU::Failure_UnsupportedMediaType:
    info = _("Unsupported media type");
    break;

  case SIP_PDU::Failure_UnsupportedURIScheme:
    info = _("Unsupported URI scheme");
    break;

  case SIP_PDU::Failure_BadExtension:
    info = _("Bad extension");
    break;

  case SIP_PDU::Failure_ExtensionRequired:
    info = _("Extension required");
    break;

  case SIP_PDU::Failure_IntervalTooBrief:
    info = _("Interval too brief");
    break;

  case SIP_PDU::Failure_LoopDetected:
    info = _("Loop detected");
    break;

  case SIP_PDU::Failure_TooManyHops:
    info = _("Too many hops");
    break;

  case SIP_PDU::Failure_AddressIncomplete:
    info = _("Address incomplete");
    break;

  case SIP_PDU::Failure_Ambiguous:
    info = _("Ambiguous");
    break;

  case SIP_PDU::Failure_BusyHere:
    info = _("Busy Here");
    break;

  case SIP_PDU::Failure_RequestTerminated:
    info = _("Request terminated");
    break;

  case SIP_PDU::Failure_NotAcceptableHere:
    info = _("Not acceptable here");
    break;

  case SIP_PDU::Failure_BadEvent:
    info = _("Bad event");
    break;

  case SIP_PDU::Failure_RequestPending:
    info = _("Request pending");
    break;

  case SIP_PDU::Failure_Undecipherable:
    info = _("Undecipherable");
    break;

  case SIP_PDU::Failure_InternalServerError:
    info = _("Internal server error");
    break;

  case SIP_PDU::Failure_NotImplemented:
    info = _("Not implemented");
    break;

  case SIP_PDU::Failure_BadGateway:
    info = _("Bad gateway");
    break;

  case SIP_PDU::Failure_ServiceUnavailable:
    info = _("Service unavailable");
    break;

  case SIP_PDU::Failure_ServerTimeout:
    info = _("Server timeout");
    break;

  case SIP_PDU::Failure_SIPVersionNotSupported:
    info = _("SIP version not supp.");
    break;

  case SIP_PDU::Failure_MessageTooLarge:
    info = _("Message too large");
    break;

  case SIP_PDU::GlobalFailure_BusyEverywhere:
    info = _("Busy everywhere");
    break;

  case SIP_PDU::GlobalFailure_Decline:
    info = _("Decline");
    break;

  case SIP_PDU::GlobalFailure_DoesNotExistAnywhere:
    info = _("Does not exist anymore");
    break;

  case SIP_PDU::GlobalFailure_NotAcceptable:
    info = _("Globally not acceptable");
    break;

  case SIP_PDU::Local_TransportError:
  case SIP_PDU::Local_BadTransportAddress:
    info = _("Transport error");
    break;
  
  case SIP_PDU::Failure_TransactionDoesNotExist:
  case SIP_PDU::Failure_Gone:
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

  /* Signal the SIP Endpoint */
  SIPEndPoint::OnRegistrationFailed (strm.str ().c_str (), r, wasRegistering);

  /* Signal */
  Ekiga::Account *account = account_core.find_account (strm.str ());
  if (account)
    runtime.run_in_main (sigc::bind (registration_event.make_slot (), account,
                                     wasRegistering ? Ekiga::AccountCore::RegistrationFailed : Ekiga::AccountCore::UnregistrationFailed,
                                     info));
}


bool CallProtocolManager::OnIncomingConnection (OpalConnection &connection,
                                                unsigned options,
                                                OpalConnection::StringOptions * stroptions)
{
  PTRACE (3, "CallProtocolManager\tIncoming connection");

  if (!forward_uri.empty () && endpoint.get_unconditional_forward ())
    connection.ForwardCall (forward_uri);
  else if (endpoint.GetCallsNumber () > 1) { 

    if (!forward_uri.empty () && endpoint.get_forward_on_busy ())
      connection.ForwardCall (forward_uri);
    else {
      connection.ClearCall (OpalConnection::EndedByLocalBusy);
    }
  }
  else {

    Opal::Call *call = dynamic_cast<Opal::Call *> (&connection.GetCall ());
    if (call) {

      if (!forward_uri.empty () && endpoint.get_forward_on_no_answer ()) 
        call->set_no_answer_forward (endpoint.get_reject_delay (), forward_uri);
      else
        call->set_reject_delay (endpoint.get_reject_delay ());
    }

    return SIPEndPoint::OnIncomingConnection (connection, options, stroptions);
  }

  return false;
}


void CallProtocolManager::OnReceivedMESSAGE (G_GNUC_UNUSED OpalTransport & transport,
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
  if (!last || *last != pdu.GetMIME ().GetFrom ()) {

    val = new PString (pdu.GetMIME ().GetFrom ());
    msgData.SetAt (SIPURL (from).AsString (), val);

    SIPURL uri = from;
    uri.Sanitise (SIPURL::FromURI);
    std::string display_name = (const char *) uri.GetDisplayName ();
    std::string message_uri = (const char *) uri.AsString ();
    std::string _message = (const char *) pdu.GetEntityBody ();

    dialect->push_message (message_uri, display_name, _message);
  }
}


void CallProtocolManager::OnMessageFailed (const SIPURL & messageUrl,
                                           SIP_PDU::StatusCodes /*reason*/)
{
  SIPURL to = messageUrl;
  to.Sanitise (SIPURL::ToURI);
  std::string uri = (const char *) to.AsString ();
  std::string display_name = (const char *) to.GetDisplayName ();
  
  dialect->push_notice (uri, display_name, _("Could not send message"));
}


SIPURL CallProtocolManager::GetRegisteredPartyName (const SIPURL & host)
{
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

      Ekiga::Account *account = account_core.find_account ("ekiga.net");

      if (account)
        return SIPURL ("\"" + GetDefaultDisplayName () + "\" <" + account->get_aor () + ">");
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
CallProtocolManager::OnPresenceInfoReceived (const PString & user,
                                             const PString & basic,
                                             const PString & note)
{
  PINDEX j;
  PCaselessString b = basic;
  PCaselessString s = note;

  std::string status;
  std::string presence = "presence-unknown";

  if (b.Find ("Closed") != P_MAX_INDEX)
    presence = "presence-offline";
  else
    presence = "presence-online";

  if (s.Find ("Away") != P_MAX_INDEX)
    presence = "presence-away";
  else if (s.Find ("On the phone") != P_MAX_INDEX
           || s.Find ("Ringing") != P_MAX_INDEX) 
    presence = "presence-inacall";
  else if (s.Find ("dnd") != P_MAX_INDEX
           || s.Find ("Do Not Disturb") != P_MAX_INDEX) 
    presence = "presence-dnd";

  else if (s.Find ("Free For Chat") != P_MAX_INDEX) 
    presence = "presence-freeforchat";

  if ((j = s.Find (" - ")) != P_MAX_INDEX)
    status = (const char *) note.Mid (j + 3);

  SIPURL sip_uri = SIPURL (user);
  sip_uri.Sanitise (SIPURL::ExternalURI);
  std::string _uri = sip_uri.AsString ();

  /**
   * TODO
   * Wouldn't it be convenient to emit the signal and have the presence core listen to it ?
   */
  runtime.run_in_main (sigc::bind (presence_core.presence_received.make_slot (), _uri, presence));
  runtime.run_in_main (sigc::bind (presence_core.status_received.make_slot (), _uri, status));
}


void CallProtocolManager::on_dial (std::string uri)
{
  endpoint.dial (uri);
}

void CallProtocolManager::on_message (std::string uri,
				      std::string name)
{
  dialect->start_chat_with (uri, name);
}

void CallProtocolManager::on_forward (std::string uri)
{
  PStringList connections = GetAllConnections ();
  /* FIXME : we don't handle several connections here */
  PSafePtr<OpalConnection> connection = GetConnectionWithLock (connections[0]);

  connection->ForwardCall (uri);
}
