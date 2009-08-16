
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
 * Ekiga is licensed under the GPL license and as a special exc_managertion,
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


#include <algorithm>
#include <sstream>

#include <glib/gi18n.h>

#include "config.h"

#include "sip-endpoint.h"

#include "opal-bank.h"
#include "opal-call.h"

#include "account-core.h"
#include "chat-core.h"
#include "personal-details.h"
#include "opal-account.h"

namespace Opal {

  namespace Sip {

    class subscriber : public PThread
    {
      PCLASSINFO(subscriber, PThread);

    public:
      subscriber (const Opal::Account & _account,
                  Opal::Sip::EndPoint & _manager,
		  bool _registering)
        : PThread (1000, AutoDeleteThread),
	  account (_account),
	  manager (_manager),
	  registering (_registering)
      {
        this->Resume ();
      };

      void Main ()
      {
	if (registering) {

	  manager.Register (account.get_username (),
			    account.get_host (),
			    account.get_authentication_username (),
			    account.get_password (),
			    account.is_enabled (),
			    account.is_limited (),
			    account.get_timeout ());
	} else {

	  manager.Unregister (account.get_aor ());
	}
      };

    private:
      const Opal::Account & account;
      Opal::Sip::EndPoint & manager;
      bool registering;
    };
  };
};



/* The class */
Opal::Sip::EndPoint::EndPoint (Opal::CallManager & _manager,
                               Ekiga::ServiceCore & _core,
                               unsigned _listen_port)
    :   SIPEndPoint (_manager),
	manager (_manager),
	core (_core)
{
  gmref_ptr<Ekiga::ChatCore> chat_core = core.get ("chat-core");
  gmref_ptr<Opal::Bank> bank = core.get ("opal-account-store");

  auto_answer_call = false;
  protocol_name = "sip";
  uri_prefix = "sip:";
  listen_port = (_listen_port > 0 ? _listen_port : 5060);

  dialect = gmref_ptr<SIP::Dialect>(new SIP::Dialect (core, sigc::mem_fun (this, &Opal::Sip::EndPoint::send_message)));
  chat_core->add_dialect (dialect);

  bank->account_added.connect (sigc::mem_fun (this, &Opal::Sip::EndPoint::on_bank_updated));
  bank->account_removed.connect (sigc::mem_fun (this, &Opal::Sip::EndPoint::on_bank_updated));
  bank->account_updated.connect (sigc::mem_fun (this, &Opal::Sip::EndPoint::on_bank_updated));

  /* Timeouts */
  SetAckTimeout (PTimeInterval (0, 32));
  SetPduCleanUpTimeout (PTimeInterval (0, 1));
  SetInviteTimeout (PTimeInterval (0, 60));
  SetNonInviteTimeout (PTimeInterval (0, 6));
  SetRetryTimeouts (500, 4000);
  SetMaxRetries (8);

  /* Start listener */
  set_listen_port (listen_port);

  /* Update the User Agent */
  SetUserAgent ("Ekiga/" PACKAGE_VERSION);

  /* Ready to take calls */
  manager.AddRouteEntry("sip:.* = pc:*");
  manager.AddRouteEntry("pc:.* = sip:<da>");

  /* NAT Binding */
  SetNATBindingRefreshMethod (SIPEndPoint::EmptyRequest);
}


Opal::Sip::EndPoint::~EndPoint ()
{
}


bool
Opal::Sip::EndPoint::populate_menu (Ekiga::ContactPtr contact,
				    const std::string uri,
				    Ekiga::MenuBuilder &builder)
{
  return menu_builder_add_actions (contact->get_name (), uri, builder);
}


bool
Opal::Sip::EndPoint::populate_menu (Ekiga::PresentityPtr presentity,
				    const std::string uri,
				    Ekiga::MenuBuilder& builder)
{
  return menu_builder_add_actions (presentity->get_name (), uri, builder);
}


bool
Opal::Sip::EndPoint::menu_builder_add_actions (const std::string& fullname,
					       const std::string& uri,
					       Ekiga::MenuBuilder & builder)
{
  bool populated = false;

  gmref_ptr<Opal::Bank> bank = core.get ("opal-account-store");

  std::list<std::string> uris;
  std::list<std::string> accounts;

  if (!(uri.find ("sip:") == 0 || uri.find (":") == string::npos))
    return false;

  if (uri.find ("@") == string::npos) {

    for (Opal::Bank::iterator it = bank->begin ();
	 it != bank->end ();
	 it++) {

      if ((*it)->get_protocol_name () == "SIP" && (*it)->is_active ()) {

	std::stringstream uristr;
	std::string str = uri;

	for (unsigned i = 0 ; i < str.length() ; i++) {

	  if (str [i] == ' ' || str [i] == '-') {
	    str.erase (i,1);
	    i--;
	  }
	}

	if (str.find ("sip:") == string::npos)
	  uristr << "sip:" << str;
	else
	  uristr << str;

	uristr << "@" << (*it)->get_host ();

	uris.push_back (uristr.str ());
	accounts.push_back ((*it)->get_name ());
      }
    }
  } else {
    uris.push_back (uri);
    accounts.push_back ("");
  }

  std::list<std::string>::iterator ita = accounts.begin ();
  for (std::list<std::string>::iterator it = uris.begin ();
       it != uris.end ();
       it++) {

    std::stringstream call_action;
    std::stringstream transfer_action;
    if (!(*ita).empty ()) {
      call_action << _("Call") << " [" << (*ita) << "]";
      transfer_action << _("Transfer") << " [" << (*ita) << "]";
    }
    else {
      call_action << _("Call");
      transfer_action << _("Transfer");
    }

    if (0 == GetConnectionCount ())
      builder.add_action ("call", call_action.str (),
                          sigc::bind (sigc::mem_fun (this, &Opal::Sip::EndPoint::on_dial), (*it)));
    else
      builder.add_action ("transfer", transfer_action.str (),
                          sigc::bind (sigc::mem_fun (this, &Opal::Sip::EndPoint::on_transfer), (*it)));

    ita++;
  }

  ita = accounts.begin ();
  for (std::list<std::string>::iterator it = uris.begin ();
       it != uris.end ();
       it++) {

    std::stringstream msg_action;
    if (!(*ita).empty ())
      msg_action << _("Message") << " [" << (*ita) << "]";
    else
      msg_action << _("Message");

    builder.add_action ("message", msg_action.str (),
                        sigc::bind (sigc::mem_fun (this, &Opal::Sip::EndPoint::on_message), (*it), fullname));

    ita++;
  }

  populated = true;

  return populated;
}


void
Opal::Sip::EndPoint::fetch (const std::string _uri)
{
  PWaitAndSignal mut(listsMutex);
  std::string::size_type loc = _uri.find ("@", 0);
  std::string domain;

  if (loc != string::npos)
    domain = _uri.substr (loc+1);

  // It is not in the list of uris for which a subscribe is active
  if (std::find (subscribed_uris.begin (), subscribed_uris.end (), _uri) == subscribed_uris.end ()) {

    // The account is active
    if (std::find (active_domains.begin (), active_domains.end (), domain) != active_domains.end ()) {

      Subscribe (SIPSubscribe::Presence, 300, PString (_uri.c_str ()));
      Subscribe (SIPSubscribe::Dialog, 300, PString (_uri.c_str ()));
      subscribed_uris.push_back (_uri);
    }
    else {

      to_subscribe_uris.push_back (_uri);
    }
  }
}


void
Opal::Sip::EndPoint::unfetch (const std::string uri)
{
  PWaitAndSignal mut(listsMutex);
  if (std::find (subscribed_uris.begin (), subscribed_uris.end (), uri) != subscribed_uris.end ()) {

    Subscribe (SIPSubscribe::Presence, 0, PString (uri.c_str ()));
    Subscribe (SIPSubscribe::Dialog, 0, PString (uri.c_str ()));
    subscribed_uris.remove (uri);
  }
}


void
Opal::Sip::EndPoint::publish (const Ekiga::PersonalDetails & details)
{
  std::map<std::string, PString> publishing;
  std::string hostname = (const char *) PIPSocket::GetHostName ();
  std::string presence = ((Ekiga::PersonalDetails &) (details)).get_presence ();
  std::string status = ((Ekiga::PersonalDetails &) (details)).get_status ();

  {
    PWaitAndSignal mut(listsMutex);
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
      data += presence.c_str ();
      if (!status.empty ()) {
	data += " - ";
	data += status.c_str ();
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

      publishing[to]=data;
    }
  }

  for (std::map<std::string, PString>::const_iterator iter = publishing.begin ();
       iter != publishing.end ();
       ++iter) {

    Publish (iter->first, iter->second, 500); // TODO: allow to change the 500
  }
}


bool
Opal::Sip::EndPoint::send_message (const std::string & _uri,
				   const std::string & _message)
{
  if (!_uri.empty () && (_uri.find ("sip:") == 0 || _uri.find (':') == string::npos) && !_message.empty ()) {

    PURL fromAddress;
    PString conversationId;
    Message (PURL(_uri), "text/plain;charset=UTF-8", _message, fromAddress, conversationId);
    return true;
  }

  return false;
}


bool
Opal::Sip::EndPoint::dial (const std::string & uri)
{
  std::stringstream ustr;

  if (uri.find ("sip:") == 0 || uri.find (":") == string::npos) {

    if (uri.find (":") == string::npos)
      ustr << "sip:" << uri;
    else
      ustr << uri;

    PString token;
    manager.SetUpCall("pc:*", ustr.str(), token, (void*) ustr.str().c_str());

    return true;
  }

  return false;
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

    // SIP Info
  case 0:
    SetSendUserInputMode (OpalConnection::SendUserInputAsTone);
    break;

    // RFC2833
  case 1:
    SetSendUserInputMode (OpalConnection::SendUserInputAsProtocolDefault);
    break;
  default:
    break;
  }
}


unsigned
Opal::Sip::EndPoint::get_dtmf_mode () const
{
  // SIP Info
  if (GetSendUserInputMode () == OpalConnection::SendUserInputAsTone)
    return 0;

  // RFC2833
  if (GetSendUserInputMode () == OpalConnection::SendUserInputAsInlineRFC2833)
    return 1;

  return 1;
}


bool
Opal::Sip::EndPoint::set_listen_port (unsigned port)
{
  unsigned udp_min, udp_max;

  listen_iface.protocol = "udp";
  listen_iface.voip_protocol = "sip";
  listen_iface.id = "*";

  manager.get_udp_ports (udp_min, udp_max);

  if (port > 0) {

    std::stringstream str;
    RemoveListener (NULL);

    str << "udp$*:" << port;
    if (!StartListeners (PStringArray (str.str ()))) {

      port = udp_min;
      str << "udp$*:" << port;
      while (port <= udp_max) {

        if (StartListeners (PStringArray (str.str ()))) {

          listen_iface.port = port;
          return true;
        }

        port++;
      }
    }
    else {
      listen_iface.port = port;
      return true;
    }
  }

  return false;
}


const Ekiga::CallProtocolManager::Interface&
Opal::Sip::EndPoint::get_listen_interface () const
{
  return listen_iface;
}



void
Opal::Sip::EndPoint::set_forward_uri (const std::string & uri)
{
  forward_uri = uri;
}


const std::string&
Opal::Sip::EndPoint::get_forward_uri () const
{
  return forward_uri;
}


void
Opal::Sip::EndPoint::set_outbound_proxy (const std::string & uri)
{
  outbound_proxy = uri;
  SetProxy (SIPURL (outbound_proxy));
}


const std::string&
Opal::Sip::EndPoint::get_outbound_proxy () const
{
  return outbound_proxy;
}


void
Opal::Sip::EndPoint::set_nat_binding_delay (unsigned delay)
{
  if (delay > 0)
    SetNATBindingTimeout (PTimeInterval (0, delay));
}


unsigned
Opal::Sip::EndPoint::get_nat_binding_delay ()
{
  return GetNATBindingTimeout ().GetSeconds ();
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


bool
Opal::Sip::EndPoint::subscribe (const Opal::Account & account)
{
  if (account.get_protocol_name () != "SIP")
    return false;

  new subscriber (account, *this, true);
  return true;
}


bool
Opal::Sip::EndPoint::unsubscribe (const Opal::Account & account)
{
  if (account.get_protocol_name () != "SIP")
    return false;

  new subscriber (account, *this, false);
  return true;
}


void
Opal::Sip::EndPoint::Register (const std::string username,
			       const std::string host_,
			       const std::string auth_username,
			       const std::string password,
			       bool is_enabled,
			       bool is_limited,
			       unsigned timeout)
{
  PWaitAndSignal mut(listsMutex);
  PString _aor;
  std::stringstream aor;
  std::string host(host_);
  std::string::size_type loc = host.find (":", 0);
  if (loc != std::string::npos)
    host = host.substr (0, loc);

  if (username.find ("@") == std::string::npos)
    aor << username << "@" << host;
  else
    aor << username;

  SIPRegister::Params params;
  params.m_addressOfRecord = aor.str ();
  params.m_registrarAddress = host;
  if (is_limited)
    params.m_contactAddress = "%LIMITED";
  params.m_authID = auth_username;
  params.m_password = password;
  params.m_expire = is_enabled ? timeout : 0;
  params.m_minRetryTime = 0;
  params.m_maxRetryTime = 0;

  // Update the list of active domains
  std::string domain = Opal::Sip::EndPoint::get_aor_domain (aor.str ());
  bool found = (std::find (active_domains.begin (), active_domains.end (), domain) != active_domains.end ());
  if (is_enabled && !found)
    active_domains.push_back (domain);
  else if (!is_enabled && found)
    active_domains.remove (domain);

  // Register the given aor to the give registrar
  if (!SIPEndPoint::Register (params, _aor))
    OnRegistrationFailed (aor.str (), SIP_PDU::MaxStatusCode, is_enabled);
}


void
Opal::Sip::EndPoint::OnRegistered (const PString & _aor,
				   bool was_registering)
{
  PWaitAndSignal mut(listsMutex);
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

    server = get_aor_domain (aor);
    if (server.empty ())
      return;

    if (was_registering) {
      for (std::list<std::string>::iterator iter = to_subscribe_uris.begin ();
           iter != to_subscribe_uris.end () ; ) {

        found = (*iter).find (server, 0);
        if (found != string::npos) {

          Subscribe (SIPSubscribe::Presence, 300, PString ((*iter).c_str ()));
          Subscribe (SIPSubscribe::Dialog, 300, PString ((*iter).c_str ()));
          subscribed_uris.push_back (*iter);
          iter = to_subscribe_uris.erase (iter);
        }
        else
          ++iter;
      }
    }
    else {
      for (std::list<std::string>::iterator iter = subscribed_uris.begin ();
           iter != subscribed_uris.end () ; ) {

        found = (*iter).find (server, 0);
        if (found != string::npos) {

          Unsubscribe (SIPSubscribe::Presence, PString ((*iter).c_str ()));
          Unsubscribe (SIPSubscribe::Dialog, PString ((*iter).c_str ()));
          to_subscribe_uris.push_back (*iter);
          iter = subscribed_uris.erase (iter);
        }
        else
          iter++;
      }
    }
  }

  /* Subscribe for MWI */
  if (!IsSubscribed (SIPSubscribe::MessageSummary, aor))
    Subscribe (SIPSubscribe::MessageSummary, 3600, aor);

  /* Signal */
  Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this,
							  &Opal::Sip::EndPoint::registration_event_in_main),
					   strm.str (),
					   was_registering ? Account::Registered : Account::Unregistered,
					   std::string ()));
}


void
Opal::Sip::EndPoint::OnRegistrationFailed (const PString & _aor,
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
    info = _("Forbidden, please check that username and password are correct");
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
    info = _("Proxy authentication required");
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

  case SIP_PDU::Local_Timeout:
    info = _("Remote party host is offline");
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

  /* opal adds a RequestTerminated, and this should not be shown to user,
   * as a sip code has already been scheduled to be shown
   */
  if (r != SIP_PDU::Failure_RequestTerminated) {
    /* Signal */
    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this,
							    &Opal::Sip::EndPoint::registration_event_in_main),
					     strm.str (),
					     wasRegistering ?Account::RegistrationFailed : Account::UnregistrationFailed,
					     info));
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
  Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &Opal::Sip::EndPoint::mwi_received_in_main), party, info));
}


bool
Opal::Sip::EndPoint::OnIncomingConnection (OpalConnection &connection,
					   unsigned options,
					   OpalConnection::StringOptions * stroptions)
{
  PTRACE (3, "Opal::Sip::EndPoint\tIncoming connection");

  if (!SIPEndPoint::OnIncomingConnection (connection, options, stroptions))
    return false;

  if (!forward_uri.empty () && manager.get_unconditional_forward ())
    connection.ForwardCall (forward_uri);
  else if (manager.GetCallCount () > 1) {

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
      else if (auto_answer_call || manager.get_auto_answer ()) {
        auto_answer_call = false;
        PTRACE (3, "Opal::Sip::EndPoint\tAuto-Answering incoming connection");
        call->answer ();
      }
      else // Pending
        call->set_reject_delay (manager.get_reject_delay ());
    }
  }

  return true;
}


PBoolean 
Opal::Sip::EndPoint::OnReceivedINVITE (OpalTransport& transport, 
                                       SIP_PDU* pdu)
{
  if (pdu == NULL) 
    return SIPEndPoint::OnReceivedINVITE (transport, pdu);

  PString str;
  int appearance;

  pdu->GetMIME ().GetAlertInfo (str, appearance);
  static const char ringanswer[] = "Ring Answer";
  PINDEX pos = str.Find (ringanswer);

  if (pos != P_MAX_INDEX) {
    PTRACE (3, "Opal::Sip::EndPoint\tRing Answer in AlertInfo header, will Auto-Answer incoming connection");
    auto_answer_call = true;
  }

  return SIPEndPoint::OnReceivedINVITE (transport, pdu);
}


bool
Opal::Sip::EndPoint::OnReceivedMESSAGE (OpalTransport & transport,
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
    uri.Sanitise (SIPURL::RequestURI);
    std::string display_name = (const char *) uri.GetDisplayName ();
    std::string message_uri = (const char *) uri.AsString ();
    std::string _message = (const char *) pdu.GetEntityBody ();

    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &Opal::Sip::EndPoint::push_message_in_main), message_uri, display_name, _message));
  }

  return SIPEndPoint::OnReceivedMESSAGE (transport, pdu);
}


void
Opal::Sip::EndPoint::OnMessageFailed (const SIPURL & messageUrl,
				      SIP_PDU::StatusCodes /*reason*/)
{
  SIPURL to = messageUrl;
  to.Sanitise (SIPURL::ToURI);
  std::string uri = (const char *) to.AsString ();
  std::string display_name = (const char *) to.GetDisplayName ();

  Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &Opal::Sip::EndPoint::push_notice_in_main),
					   uri, display_name,
					   _("Could not send message")));
}


SIPURL
Opal::Sip::EndPoint::GetRegisteredPartyName (const SIPURL & host,
					     const OpalTransport & transport)
{
  PString local_address;
  PIPSocket::Address address;
  WORD port;
  PString url;
  SIPURL registration_address;
  PWaitAndSignal mut(defaultAORMutex);

  /* If we are registered to an account corresponding to host, use it.
   */
  PSafePtr<SIPHandler> info = activeSIPHandlers.FindSIPHandlerByDomain(host.GetHostName (), SIP_PDU::Method_REGISTER, PSafeReadOnly);
  if (info != NULL) {

    return SIPURL ("\"" + GetDefaultDisplayName () + "\" <" + info->GetAddressOfRecord ().AsString () + ">");
  }
  else {

    /* If we are not registered to host,
     * then use the default account as outgoing identity.
     * If we are exchanging messages with a peer on our network,
     * then do not use the default account as outgoing identity.
     */
    if (host.GetHostAddress ().GetIpAndPort (address, port) && !manager.IsLocalAddress (address)) {

      if ( !default_aor.empty ())
        return SIPURL ("\"" + GetDefaultDisplayName () + "\" <" + PString(default_aor) + ">");
    }
  }

  /* As a last resort, ie not registered to host, no default account or
   * dialog with a local peer, then use the local address
   */
  return GetDefaultRegisteredPartyName (transport);
}


void
Opal::Sip::EndPoint::OnPresenceInfoReceived (const PString & user,
                                             const PString & basic,
                                             const PString & note)
{
  PINDEX j;
  PCaselessString b = basic;
  PCaselessString s = note;

  std::string presence = "unknown";
  std::string status;

  if (!basic.IsEmpty ()) {

    if (b.Find ("Open") != P_MAX_INDEX)
      presence = "online";
    else
      presence = "offline";
  }

  if (!note.IsEmpty ()) {

    if (s.Find ("Away") != P_MAX_INDEX)
      presence = "away";
    else if (s.Find ("On the phone") != P_MAX_INDEX)
      presence = "inacall";
    else if (s.Find ("Ringing") != P_MAX_INDEX)
      presence = "ringing";
    else if (s.Find ("dnd") != P_MAX_INDEX
             || s.Find ("Do Not Disturb") != P_MAX_INDEX)
      presence = "dnd";

    else if (s.Find ("Free For Chat") != P_MAX_INDEX)
      presence = "freeforchat";

    if ((j = s.Find (" - ")) != P_MAX_INDEX)
      status = (const char *) note.Mid (j + 3);
  }

  SIPURL sip_uri = SIPURL (user);
  sip_uri.Sanitise (SIPURL::ExternalURI);
  std::string _uri = sip_uri.AsString ();
  std::string old_presence = presence_infos[_uri].presence;
  std::string old_status = presence_infos[_uri].status;

  // If first notification, and no information, then we are offline
  if (presence == "unknown" && old_presence.empty ())
    presence = "offline";

  // If presence change, then signal it to the various components
  // If presence is unknown (notification with empty body), and it is not the
  // first notification, and we can conclude it is a ping back from the server
  // to indicate the presence status did not change, hence we do nothing.
  if (presence != "unknown" && (old_presence != presence || old_status != status)) {
    presence_infos[_uri].presence = presence;
    presence_infos[_uri].status = status;
    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &Opal::Sip::EndPoint::presence_status_in_main), _uri, presence_infos[_uri].presence, presence_infos[_uri].status));
  }
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
  default:
  case SIPDialogNotification::Trying:
  case SIPDialogNotification::Terminated:
    break;
  }

  dialog_infos[uri].presence = presence;
  dialog_infos[uri].status = status;

  if (_status)
    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &Opal::Sip::EndPoint::presence_status_in_main), uri, dialog_infos[uri].presence, dialog_infos[uri].status));
  else
    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &Opal::Sip::EndPoint::presence_status_in_main), uri, presence_infos[uri].presence, presence_infos[uri].status));
}


void Opal::Sip::EndPoint::on_dial (std::string uri)
{
  manager.dial (uri);
}


void Opal::Sip::EndPoint::on_message (std::string uri,
                                      std::string name)
{
  dialect->start_chat_with (uri, name);
}


void Opal::Sip::EndPoint::on_transfer (std::string uri)
{
  /* FIXME : we don't handle several calls here */
  for (PSafePtr<OpalConnection> connection(connectionsActive, PSafeReference); connection != NULL; ++connection)
    if (!PIsDescendant(&(*connection), OpalPCSSConnection))
      connection->TransferConnection (uri);
}

void
Opal::Sip::EndPoint::registration_event_in_main (const std::string aor,
						 Opal::Account::RegistrationState state,
						 const std::string msg)
{
  gmref_ptr<Opal::Bank> bank = core.get ("opal-account-store");
  AccountPtr account = bank->find_account (aor);

  if (account) {

    account->handle_registration_event (state, msg);
  }
}


void
Opal::Sip::EndPoint::presence_status_in_main (std::string uri,
					      std::string presence,
					      std::string status)
{
  presence_received.emit (uri, presence);
  status_received.emit (uri, status);
}

void
Opal::Sip::EndPoint::push_message_in_main (const std::string uri,
					   const std::string name,
					   const std::string msg)
{
  dialect->push_message (uri, name, msg);
}

void
Opal::Sip::EndPoint::push_notice_in_main (const std::string uri,
					  const std::string name,
					  const std::string msg)
{
  dialect->push_notice (uri, name, msg);
}

void
Opal::Sip::EndPoint::mwi_received_in_main (const std::string aor,
					   const std::string info)
{
  gmref_ptr<Opal::Bank> bank = core.get ("opal-account-store");
  AccountPtr account = bank->find_account (aor);

  if (account) {

    account->handle_message_waiting_information (info);
  }
}

void
Opal::Sip::EndPoint::on_bank_updated (Ekiga::ContactPtr /*contact*/)
{
  { // first we flush the existing value
    PWaitAndSignal mut(defaultAORMutex);
    default_aor = "";
  }

  { // and now we compute it again
    gmref_ptr<Opal::Bank> bank = core.get ("opal-account-store");
    bank->visit_accounts (sigc::mem_fun (this, &Opal::Sip::EndPoint::search_for_default_account));
  }
}

bool
Opal::Sip::EndPoint::search_for_default_account (Opal::AccountPtr account)
{
  PWaitAndSignal mut(defaultAORMutex);
  bool result = true;

  /* here is how result is computed here : first, remember it means to go on
   * the search ; then we want the ekiga.net accounts to have some priority over
   * others, so by default we want to go on. But if we find an account which is both
   * suitable (active) and ekiga.net, then we want to stop.
   */

  if (account->is_active ()) {

    default_aor = account->get_aor ();
    if (account->get_type () == Opal::Account::Ekiga) {

      result = false;
    }
  }

  return result;
}
