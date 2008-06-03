
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
 *                         h323endpoint.cpp  -  description
 *                         --------------------------------
 *   begin                : Wed 24 Nov 2004
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the H.323 Endpoint class.
 *
 */


#include "config.h"

#include "h323.h"

#include "opal-call.h"


class dialer : public PThread
{
  PCLASSINFO(dialer, PThread);

public:

  dialer (const std::string & uri, GMManager & ep) 
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
  GMManager & endpoint;
};


using namespace Opal::H323;


/* The class */
CallProtocolManager::CallProtocolManager (GMManager & ep, Ekiga::ServiceCore & _core, unsigned _listen_port)
                    : H323EndPoint (ep), 
                      endpoint (ep),
                      core (_core),
                      runtime (*(dynamic_cast<Ekiga::Runtime *> (core.get ("runtime"))))
{
  protocol_name = "h323";
  uri_prefix = "h323:";
  listen_port = _listen_port;

  /* Initial requested bandwidth */
  SetInitialBandwidth (40000);

  /* Start listener */
  set_listen_port (listen_port);

  /* Ready to take calls */
  endpoint.AddRouteEntry("h323:.* = pc:<db>");
  endpoint.AddRouteEntry("pc:.* = h323:<da>");
}


bool CallProtocolManager::populate_menu (Ekiga::Contact &contact,
                                         Ekiga::MenuBuilder &builder)
{
  std::string name = contact.get_name ();
  std::map<std::string, std::string> uris = contact.get_uris ();

  return menu_builder_add_actions (name, uris, builder);
}


bool CallProtocolManager::populate_menu (const std::string uri,
                                         Ekiga::MenuBuilder & builder)
{
  std::map<std::string, std::string> uris; 
  uris [""] = uri;

  return menu_builder_add_actions ("", uris, builder);
}


bool CallProtocolManager::menu_builder_add_actions (const std::string & /*fullname*/,
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

    builder.add_action ("call", action, sigc::bind (sigc::mem_fun (this, &CallProtocolManager::on_dial), iter->second));

    populated = true;
  }

  return populated;
}


bool CallProtocolManager::dial (const std::string & uri)
{
  PString token;
  std::stringstream ustr;

  if (uri.find ("h323:") == 0) {

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
  switch (mode) 
    {
    case 0:
      SetSendUserInputMode (OpalConnection::SendUserInputAsString);
      break;
    case 1:
      SetSendUserInputMode (OpalConnection::SendUserInputAsTone);
      break;
    case 2:
      SetSendUserInputMode (OpalConnection::SendUserInputAsInlineRFC2833);
      break;
    case 3:
      SetSendUserInputMode (OpalConnection::SendUserInputAsQ931);
      break;
    default:
      break;
    }
}


unsigned CallProtocolManager::get_dtmf_mode () const
{
  if (GetSendUserInputMode () == OpalConnection::SendUserInputAsString)
    return 0;

  if (GetSendUserInputMode () == OpalConnection::SendUserInputAsTone)
    return 1;

  if (GetSendUserInputMode () == OpalConnection::SendUserInputAsInlineRFC2833)
    return 2;

  if (GetSendUserInputMode () == OpalConnection::SendUserInputAsQ931)
    return 2;

  return 1;
}


bool CallProtocolManager::set_listen_port (unsigned port)
{
  interface.protocol = "tcp";
  interface.interface = "*";

  if (port > 0) {

    std::stringstream str;
    RemoveListener (NULL);

    str << "tcp$*:" << port;
    if (StartListeners (PStringArray (str.str ()))) {

      interface.port = port;
      return true;
    }
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


void CallProtocolManager::Register (const PString & aor,
                                    const PString & authUserName,
                                    const PString & password,
                                    const PString & gatekeeperID,
                                    unsigned int expires,
                                    bool unregister)
{
  PString info;
  PString host;
  PINDEX i = 0;

  bool result = false;

  i = aor.Find ("@");
  if (i == P_MAX_INDEX)
    return;

  host = aor.Mid (i+1);

  if (!unregister && !IsRegisteredWithGatekeeper (host)) {

    H323EndPoint::RemoveGatekeeper (0);

    /* Signal */
    runtime.run_in_main (sigc::bind (endpoint.registration_event.make_slot (), 
                                     aor,
                                     Ekiga::CallCore::Processing,
                                     std::string ()));

    if (!authUserName.IsEmpty ()) {
      SetLocalUserName (authUserName);
      AddAliasName (endpoint.GetDefaultDisplayName ());
    }

    SetGatekeeperPassword (password);
    SetGatekeeperTimeToLive (expires * 1000);
    result = UseGatekeeper (host, gatekeeperID);

    /* There was an error (missing parameter or registration failed)
       or the user chose to not register */
    if (!result) {

      /* Registering failed */
      if (gatekeeper) {

        switch (gatekeeper->GetRegistrationFailReason()) {

        case H323Gatekeeper::DuplicateAlias :
          info = _("Duplicate alias");
          break;
        case H323Gatekeeper::SecurityDenied :
          info = _("Bad username/password");
          break;
        case H323Gatekeeper::TransportError :
          info = _("Transport error");
          break;
        case H323Gatekeeper::RegistrationSuccessful:
          break;
        case H323Gatekeeper::UnregisteredLocally:
        case H323Gatekeeper::UnregisteredByGatekeeper:
        case H323Gatekeeper::GatekeeperLostRegistration:
        case H323Gatekeeper::InvalidListener:
        case H323Gatekeeper::NumRegistrationFailReasons:
        case H323Gatekeeper::RegistrationRejectReasonMask:
        default :
          info = _("Failed");
          break;
        }
      }
      else
        info = _("Failed");

      runtime.run_in_main (sigc::bind (endpoint.registration_event.make_slot (), 
                                       aor, 
                                       Ekiga::CallCore::RegistrationFailed,
                                       info));
    }
    else {

      /* Signal */
      runtime.run_in_main (sigc::bind (endpoint.registration_event.make_slot (), 
                                       aor,
                                       Ekiga::CallCore::Registered,
                                       std::string ()));
    }
  }
  else if (unregister && IsRegisteredWithGatekeeper (host)) {

    H323EndPoint::RemoveGatekeeper (0);
    RemoveAliasName (authUserName);

    /* Signal */
    runtime.run_in_main (sigc::bind (endpoint.registration_event.make_slot (), 
                                     aor,
                                     Ekiga::CallCore::Unregistered,
                                     std::string ()));
  }
}


bool CallProtocolManager::UseGatekeeper (const PString & address,
                                         const PString & domain,
                                         const PString & iface)
{
  bool result = 
    H323EndPoint::UseGatekeeper (address, domain, iface);

  PWaitAndSignal m(gk_name_mutex);

  gk_name = address;

  return result;
}


bool CallProtocolManager::RemoveGatekeeper (const PString & address)
{
  if (IsRegisteredWithGatekeeper (address))
    return H323EndPoint::RemoveGatekeeper (0);

  return FALSE;
}


bool CallProtocolManager::IsRegisteredWithGatekeeper (const PString & address)
{
  PWaitAndSignal m(gk_name_mutex);

  return ((gk_name *= address) && H323EndPoint::IsRegisteredWithGatekeeper ());
}


bool CallProtocolManager::OnIncomingConnection (OpalConnection & connection,
                                                G_GNUC_UNUSED unsigned options,
                                                G_GNUC_UNUSED OpalConnection::StringOptions *stroptions)
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

    return H323EndPoint::OnIncomingConnection (connection, options, stroptions);
  }

  return false;
}


void CallProtocolManager::on_dial (std::string uri)
{
  endpoint.dial (uri);
}
