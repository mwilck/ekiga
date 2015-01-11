
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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


#include <glib/gi18n.h>
#include "config.h"
#include "sip-endpoint.h"
#include "chat-core.h"


/* The class */
Opal::Sip::EndPoint::EndPoint (Opal::CallManager & _manager,
                               const Ekiga::ServiceCore& _core): SIPEndPoint (_manager),
                                                                 manager (_manager),
                                                                 core (_core)
{
  boost::shared_ptr<Ekiga::ChatCore> chat_core = core.get<Ekiga::ChatCore> ("chat-core");
  boost::shared_ptr<Ekiga::PresenceCore> presence_core = core.get<Ekiga::PresenceCore> ("presence-core");

  protocol_name = "sip";
  uri_prefix = "sip:";

  dialect = boost::shared_ptr<SIP::Dialect>(new SIP::Dialect (presence_core, boost::bind (&Opal::Sip::EndPoint::send_message, this, _1, _2)));
  chat_core->add_dialect (dialect);

  /* Timeouts */
  SetAckTimeout (PTimeInterval (0, 32));
  SetPduCleanUpTimeout (PTimeInterval (0, 1));
  SetInviteTimeout (PTimeInterval (0, 60));
  SetNonInviteTimeout (PTimeInterval (0, 6));
  SetRetryTimeouts (500, 4000);
  SetMaxRetries (8);

  /* Update the User Agent */
  SetUserAgent ("Ekiga/" PACKAGE_VERSION);

  /* Ready to take calls */
  manager.AddRouteEntry("sip:.* = pc:*");
  manager.AddRouteEntry("pc:.* = sip:<da>");

  /* NAT Binding */
  PTimeInterval timeout;
  KeepAliveType type;
  GetKeepAlive (timeout, type);
  SetKeepAlive(timeout, KeepAliveByOPTION);

  settings = boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (SIP_SCHEMA));
  settings->changed.connect (boost::bind (&EndPoint::setup, this, _1));
}


Opal::Sip::EndPoint::~EndPoint ()
{
}

void
Opal::Sip::EndPoint::setup (std::string setting)
{
  if (setting.empty () || setting == "listen-port")  {
    set_listen_port (settings->get_int ("listen-port"));
  }
  if (setting.empty () || setting == "binding-timeout")  {
    set_nat_binding_delay (settings->get_int ("binding-timeout"));
  }
  if (setting.empty () || setting == "outbound-proxy-host")  {
    set_outbound_proxy (settings->get_string ("outbound-proxy-host"));
  }
  if (setting.empty () || setting == "dtmf-mode")  {
    set_dtmf_mode (settings->get_enum ("dtmf-mode"));
  }
  if (setting.empty () || setting == "forward-host")  {
    set_forward_uri (settings->get_string ("forward-host"));
  }
}


bool
Opal::Sip::EndPoint::send_message (const std::string & _uri,
				   const Ekiga::Message::payload_type payload)
{
  // FIXME: here we should check which kind of payload we have
  Ekiga::Message::payload_type::const_iterator iter = payload.find("text/plain");
  if (!_uri.empty () && (_uri.find ("sip:") == 0 || _uri.find (':') == string::npos) && iter != payload.end ()) {

    OpalIM im;
    im.m_to = PURL (_uri);
    im.m_bodies.SetAt (PMIMEInfo::TextPlain(), iter->second);
    Message (im);

    return true;
  }

  return false;
}


bool
Opal::Sip::EndPoint::dial (const std::string & uri)
{
  std::stringstream ustr;

  if (!is_supported_uri (uri))
    return false;

  if (uri.find (":") == string::npos)
    ustr << "sip:" << uri;
  else
    ustr << uri;

  PString token;
  manager.SetUpCall("pc:*", ustr.str(), token, (void*) ustr.str().c_str());

  return true;
}


bool
Opal::Sip::EndPoint::transfer (const std::string & uri,
                               bool attended)
{
  /* This is not handled yet */
  if (attended)
    return false;

  if (GetConnectionCount () == 0 || !is_supported_uri (uri))
      return false; /* No active SIP connection to transfer, or
                     * transfer request to unsupported uri
                     */

  /* We don't handle several calls here */
  for (PSafePtr<OpalConnection> connection(connectionsActive, PSafeReference);
       connection != NULL;
       ++connection) {
    if (!PIsDescendant(&(*connection), OpalPCSSConnection)) {
      connection->TransferConnection (uri);
      return true; /* We could handle the transfer */
    }
  }

  return false;
}


bool
Opal::Sip::EndPoint::message (const Ekiga::ContactPtr & contact,
                              const std::string & uri)
{
  if (!is_supported_uri (uri))
    return false;

  dialect->start_chat_with (uri, contact->get_name ());
  return true;
}


bool
Opal::Sip::EndPoint::is_supported_uri (const std::string & uri)
{
  return (!uri.empty () && (uri.find ("sip:") == 0 || uri.find (':') == string::npos));
}


const std::string&
Opal::Sip::EndPoint::get_protocol_name () const
{
  return protocol_name;
}


void
Opal::Sip::EndPoint::set_dtmf_mode (unsigned mode)
{
  switch (mode) {

  case 0:  // RFC2833
    PTRACE (4, "Opal::Sip::EndPoint\tSet DTMF Mode to RFC2833");
    SetSendUserInputMode (OpalConnection::SendUserInputAsInlineRFC2833);
    break;
  case 1:  // SIP Info
  default:
    PTRACE (4, "Opal::Sip::EndPoint\tSet DTMF Mode to SIP INFO");
    SetSendUserInputMode (OpalConnection::SendUserInputAsTone);
    break;
  }
}


unsigned
Opal::Sip::EndPoint::get_dtmf_mode () const
{
  // RFC2833
  if (GetSendUserInputMode () == OpalConnection::SendUserInputAsInlineRFC2833)
    return 0;

  // SIP Info
  if (GetSendUserInputMode () == OpalConnection::SendUserInputAsTone)
    return 1;

  g_return_val_if_reached (1);
  return 1;
}


bool
Opal::Sip::EndPoint::set_listen_port (unsigned port)
{
  unsigned udp_min, udp_max;
  unsigned tcp_min, tcp_max;

  manager.get_udp_ports (udp_min, udp_max);
  manager.get_tcp_ports (tcp_min, tcp_max);

  const std::string protocols[] = { "udp", "tcp", "" };
  const unsigned ports[][2] = { { udp_min, udp_max }, { tcp_min, tcp_max } };

  if (port > 0) {

    interfaces.clear ();
    RemoveListener (NULL);
    for (int i = 0 ; !protocols[i].empty () ; i++) {

      std::stringstream str;
      str << protocols[i] << "$*:" << port;
      listen_iface.protocol = protocols[i];
      listen_iface.voip_protocol = "sip";
      listen_iface.id = "*";
      if (!StartListeners (PStringArray (str.str ()))) {

        port = ports[i][0];
        while (port <= ports[i][1]) {
          str << protocols[i] << "$*:" << port;

          if (StartListeners (PStringArray (str.str ()))) {

            PTRACE (4, "Opal::Sip::EndPoint\tSet listen port to " << port << " (" << protocols[i] << ")");
            listen_iface.port = port;
            interfaces.push_back (listen_iface);
            break;
          }

          port++;
        }
      }
      else {
        listen_iface.port = port;
        interfaces.push_back (listen_iface);
        PTRACE (4, "Opal::Sip::EndPoint\tSet listen port to " << port << " (" << protocols[i] << ")");
      }
    }
  }

  return false;
}


const Ekiga::CallProtocolManager::InterfaceList &
Opal::Sip::EndPoint::get_interfaces () const
{
  return interfaces;
}


void
Opal::Sip::EndPoint::set_forward_uri (const std::string & uri)
{
  if (!uri.empty ()) {
    forward_uri = uri;
    PTRACE (4, "Opal::Sip::EndPoint\tSet Forward URI to " << uri);
  }
}


const std::string&
Opal::Sip::EndPoint::get_forward_uri () const
{
  return forward_uri;
}


void
Opal::Sip::EndPoint::set_outbound_proxy (const std::string & uri)
{
  if (!uri.empty ()) {
    outbound_proxy = uri;
    PTRACE (4, "Opal::Sip::EndPoint\tSet outbound proxy to " << uri);
    SetProxy (SIPURL (outbound_proxy));
  }
}


const std::string&
Opal::Sip::EndPoint::get_outbound_proxy () const
{
  return outbound_proxy;
}


void
Opal::Sip::EndPoint::set_nat_binding_delay (unsigned delay)
{
  if (delay > 0) {
    PTRACE (4, "Opal::Sip::EndPoint\tNat binding delay set to " << delay);
    PTimeInterval timeout;
    KeepAliveType type;
    GetKeepAlive (timeout, type);
    SetKeepAlive (PTimeInterval (0, delay), type);
  }
}


unsigned
Opal::Sip::EndPoint::get_nat_binding_delay ()
{
  PTimeInterval timeout;
  KeepAliveType type;
  GetKeepAlive (timeout, type);
  return timeout.GetSeconds ();
}


std::string
Opal::Sip::EndPoint::get_aor_domain (const std::string & aor)
{
  std::string domain;
  std::string::size_type loc = aor.find ("@", 0);

  if (loc != string::npos)
    domain = aor.substr (loc+1);

  return domain;
}


void
Opal::Sip::EndPoint::enable_account (Account & account)
{
  PString _aor;

  SIPRegister::Params params;
  params.m_addressOfRecord = "sip:" + account.get_username () + "@" + account.get_host () + ";transport=tcp";
  params.m_compatibility = SIPRegister::e_RFC5626;
  params.m_authID = account.get_authentication_username ();
  params.m_password = account.get_password ();
  params.m_expire = account.is_enabled () ? account.get_timeout () : 0;
  params.m_minRetryTime = PMaxTimeInterval;  // use default value
  params.m_maxRetryTime = PMaxTimeInterval;  // use default value

  // Register the given aor to the given registrar
  if (!SIPEndPoint::Register (params, _aor)) {
    params.m_addressOfRecord = "sip:" + account.get_username () + "@" + account.get_host ();
    if (!SIPEndPoint::Register (params, _aor))
      account.handle_registration_event (Account::RegistrationFailed,
                                         _("Transport error"));
  }
}


void
Opal::Sip::EndPoint::disable_account (Account & account)
{
  PString aor = "sip:" + account.get_username () + "@" + account.get_host ();
  if (!IsRegistered (aor))
    aor += ";transport=tcp";

  if (!IsRegistered (aor))
    return;

  Unregister (aor);
}


void
Opal::Sip::EndPoint::OnRegistrationStatus (const RegistrationStatus & status)
{
  std::string info;

  boost::shared_ptr<Opal::Bank> bank = core.get<Opal::Bank> ("opal-account-store");
  Opal::AccountPtr account = bank->find_account (status.m_addressofRecord);
  if (!account)
    return;

  if (status.m_reason == SIP_PDU::Information_Trying)
    return;

  SIPEndPoint::OnRegistrationStatus (status);

  /* Successful registration or unregistration */
  if (status.m_reason == SIP_PDU::Successful_OK) {
    account->handle_registration_event (status.m_wasRegistering?Account::Registered:Account::Unregistered,
                                        std::string ());
  }
  /* Registration or unregistration failure */
  else {
    SIPURL m_addressOfRecord = SIPURL (status.m_addressofRecord);
    if (m_addressOfRecord.GetTransportProto () == "TCP") {
      SIPRegister::Params params;
      PString _aor;
      m_addressOfRecord.SetParamVar ("transport", "udp");
      params.m_addressOfRecord = m_addressOfRecord;
      params.m_compatibility = SIPRegister::e_RFC5626;
      params.m_authID = status.m_handler->GetAuthID ();
      params.m_password = status.m_handler->GetPassword ();
      params.m_expire = status.m_handler->GetExpire ();
      params.m_minRetryTime = PMaxTimeInterval;  // use default value
      params.m_maxRetryTime = PMaxTimeInterval;  // use default value
      SIPEndPoint::Register (params, _aor);
      return;
    }

    /* all these codes are defined in opal, file include/sip/sippdu.h */
    switch (status.m_reason) {
    case SIP_PDU::IllegalStatusCode:
      info = _("Illegal status code");
      break;

    case SIP_PDU::Local_TransportError:
      info = _("Transport error");
      break;

    case SIP_PDU::Local_BadTransportAddress:
      info = _("Invalid address");
      break;

    case SIP_PDU::Local_Timeout:
      /* Translators: Host of the remote party is offline, this should
       * appear when the remote host does not reply in an acceptable time */
      info = _("Remote party host is offline");
      break;

    case SIP_PDU::Information_Trying:
    case SIP_PDU::Information_Ringing:
    case SIP_PDU::Information_CallForwarded:
    case SIP_PDU::Information_Queued:
    case SIP_PDU::Information_Session_Progress:
    case SIP_PDU::Successful_OK:
    case SIP_PDU::Successful_Accepted:
      break;

    case SIP_PDU::Redirection_MultipleChoices:
      /* Translators: the following strings are answers from the SIP server
       * when the packet it receives has an error, see
       * http://www.ietf.org/rfc/rfc3261.txt, chapter 21 for more information */
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

    case SIP_PDU::Failure_BadRequest:
      info = _("Bad request");
      break;

    case SIP_PDU::Failure_UnAuthorised:
      info = _("Unauthorized");
      break;

    case SIP_PDU::Failure_PaymentRequired:
      info = _("Payment required");
      break;

    case SIP_PDU::Failure_Forbidden:
      info = _("Forbidden, please check that username and password are correct");
      break;

    case SIP_PDU::Failure_NotFound:
      info = _("Not found");
      break;

    case SIP_PDU::Failure_MethodNotAllowed:
      info = _("Method not allowed");
      break;

    case SIP_PDU::Failure_NotAcceptable:
      info = _("Not acceptable");
      break;

    case SIP_PDU::Failure_ProxyAuthenticationRequired:
      info = _("Proxy authentication required");
      break;

    case SIP_PDU::Failure_RequestTimeout:
      info = _("Timeout");
      break;

    case SIP_PDU::Failure_Conflict:
      info = _("Conflict");
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
      /* Translators:  The extension we are trying to register does not exist.
       * Here extension is a specific "phone number", see
       * http://en.wikipedia.org/wiki/Extension_(telephone)
       * for more information */
      info = _("Bad extension");
      break;

    case SIP_PDU::Failure_ExtensionRequired:
      info = _("Extension required");
      break;

    case SIP_PDU::Failure_IntervalTooBrief:
      info = _("Interval too brief");
      break;

    case SIP_PDU::Failure_TemporarilyUnavailable:
      info = _("Temporarily unavailable");
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
      info = _("SIP version not supported");
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

    case SIP_PDU::Local_NotAuthenticated:
      info = _("Invalid certificates");
      break;

    case SIP_PDU::Failure_TransactionDoesNotExist:
    case SIP_PDU::Failure_Gone:
    case SIP_PDU::MaxStatusCode:
    case SIP_PDU::Local_NoCompatibleListener:
    case SIP_PDU::Local_CannotMapScheme:
    case SIP_PDU::Local_KeepAlive:
    case SIP_PDU::Local_TransportLost:
    case SIP_PDU::Failure_UnresolvableDestination:
    default:
      info = _("Failed");
    }

    /* Opal adds a RequestTerminated, and this should not be shown to user,
     * as a sip code has already been scheduled to be shown
     */
    if (status.m_reason != SIP_PDU::Failure_RequestTerminated)
      account->handle_registration_event (status.m_wasRegistering?Account::RegistrationFailed:Account::UnregistrationFailed,
                                          info);
  }
}


void
Opal::Sip::EndPoint::OnMWIReceived (const PString & party,
				    OpalManager::MessageWaitingType /*type*/,
				    const PString & info)
{
  std::string mwi = info;
  std::transform (mwi.begin(), mwi.end(), mwi.begin(), ::tolower);
  if (mwi == "no")
    mwi = "0/0";

  /* Signal */
  Ekiga::Runtime::run_in_main (boost::bind (boost::ref(mwi_event), party, mwi));
}


bool
Opal::Sip::EndPoint::OnIncomingConnection (OpalConnection &connection,
					   unsigned options,
					   OpalConnection::StringOptions * stroptions)
{
  bool busy = false;
  PTRACE (3, "Opal::Sip::EndPoint\tIncoming connection");

  if (!SIPEndPoint::OnIncomingConnection (connection, options, stroptions))
    return false;

  for (PSafePtr<OpalConnection> conn(connectionsActive, PSafeReference); conn != NULL; ++conn) {
    if (conn->GetCall().GetToken() != connection.GetCall().GetToken() && !conn->IsReleased ())
      busy = true;
  }

  if (!forward_uri.empty () && manager.get_unconditional_forward ())
    connection.ForwardCall (forward_uri);
  else if (busy) {

    if (!forward_uri.empty () && manager.get_forward_on_busy ())
      connection.ForwardCall (forward_uri);
    else {
      connection.ClearCall (OpalConnection::EndedByLocalBusy);
    }
  }
  else {

    Opal::Call *call = dynamic_cast<Opal::Call *> (&connection.GetCall ());
    if (call) {

      if (!forward_uri.empty () && manager.get_forward_on_no_answer ())
        call->set_no_answer_forward (manager.get_reject_delay (), forward_uri);
      else // Pending
        call->set_reject_delay (manager.get_reject_delay ());
    }
  }

  return true;
}


bool
Opal::Sip::EndPoint::OnReceivedMESSAGE (SIP_PDU & pdu)
{
  if (pdu.GetMIME().GetContentType(false) != "text/plain")
    return false; // Ignore what we do not handle.

  PString from = pdu.GetMIME().GetFrom();
  PINDEX j = from.Find (';');
  if (j != P_MAX_INDEX)
    from = from.Left(j); // Remove all parameters
  j = from.Find ('<');
  if (j != P_MAX_INDEX && from.Find ('>') == P_MAX_INDEX)
    from += '>';

  SIPURL uri = from;
  uri.Sanitise (SIPURL::RequestURI);
  std::string display_name = (const char *) uri.GetDisplayName ();
  std::string message_uri = (const char *) uri.AsString ();
  std::string _message = (const char *) pdu.GetEntityBody ();
  Ekiga::Message::payload_type payload;
  // FIXME: we push as 'text/plain' without really knowing
  payload.insert (std::make_pair ("text/plain", _message));
  GTimeVal current;
  g_get_current_time (&current);
  gchar* time = g_time_val_to_iso8601 (&current);
  Ekiga::Message msg = {time, display_name, payload };
  g_free (time);

  Ekiga::Runtime::run_in_main (boost::bind (&Opal::Sip::EndPoint::push_message_in_main, this, message_uri, msg));

  return SIPEndPoint::OnReceivedMESSAGE (pdu);
}


void
Opal::Sip::EndPoint::OnMESSAGECompleted (const SIPMessage::Params & params,
                                         SIP_PDU::StatusCodes reason)
{
  PTRACE (4, "IM sending completed, reason: " << reason);

  // after TemporarilyUnavailable, RequestTimeout appears too, hence do not process it too
  if (reason == SIP_PDU::Successful_OK || reason == SIP_PDU::Failure_RequestTimeout)
    return;

  SIPURL to = params.m_remoteAddress;
  to.Sanitise (SIPURL::ToURI);
  std::string uri = (const char*) to.AsString ();
  std::string display_name = (const char*) to.GetDisplayName ();

  std::string reason_shown = _("Could not send message: ");
  if (reason == SIP_PDU::Failure_TemporarilyUnavailable)
    reason_shown += _("user offline");
  else
    reason_shown += SIP_PDU::GetStatusCodeDescription (reason).operator std::string ();  // too many to translate them with _()...
  Ekiga::Message::payload_type payload;
  // FIXME: we push as 'text/plain' without really knowing...
  payload.insert (std::make_pair ("text/plain", reason_shown));
  GTimeVal current;
  g_get_current_time (&current);
  gchar* time = g_time_val_to_iso8601 (&current);
  Ekiga::Message msg = {time, "" /* it's a notice */, payload };
  g_free (time);

  Ekiga::Runtime::run_in_main (boost::bind (&Opal::Sip::EndPoint::push_message_in_main, this, uri, msg));
}


SIPURL
Opal::Sip::EndPoint::GetRegisteredPartyName (const SIPURL & aor,
					     const OpalTransport & transport)
{
  PWaitAndSignal m(aorMutex);
  std::string local_aor = accounts[(const char*) aor.GetHostName ()];

  if (!local_aor.empty ())
    return local_aor.c_str ();

  // as a last resort, use the local address
  return GetDefaultLocalURL (transport);
}


void
Opal::Sip::EndPoint::OnDialogInfoReceived (const SIPDialogNotification & info)
{
  gchar* _status = NULL;
  std::string status;
  std::string presence;
  std::string uri = (const char *) info.m_entity;
  PString remote_uri = info.m_remote.m_identity;
  PString remote_display_name = info.m_remote.m_display.IsEmpty () ? remote_uri : info.m_remote.m_display;
  if (uri.find ("sip:") == string::npos)
    uri = "sip:" + uri;

  switch (info.m_state) {
  case SIPDialogNotification::Proceeding:
  case SIPDialogNotification::Early:
    if (!remote_display_name.IsEmpty ())
      _status = g_strdup_printf (_("Incoming call from %s"), (const char *) remote_display_name);
    else
      _status = g_strdup_printf (_("Incoming call"));
    status = _status;
    presence = "ringing";
    break;
  case SIPDialogNotification::Confirmed:
    if (!remote_display_name.IsEmpty ())
      _status = g_strdup_printf (_("In a call with %s"), (const char *) remote_display_name);
    else
      _status = g_strdup_printf (_("In a call"));
    presence = "inacall";
    status = _status;
    break;
  case SIPDialogNotification::EndStates:  // bookkeeping code
    break;
  case SIPDialogNotification::Trying:
  case SIPDialogNotification::Terminated:
  default:
    break;
  }
}


void
Opal::Sip::EndPoint::push_message_in_main (const std::string uri,
					   const Ekiga::Message msg)
{
  dialect->push_message (uri, msg);
}

void
Opal::Sip::EndPoint::update_aor_map (std::map<std::string, std::string> _accounts)
{
  PWaitAndSignal m(aorMutex);
  accounts = _accounts;
}
