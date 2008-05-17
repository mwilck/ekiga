
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
#include "ekiga.h"
#include "pcss.h"

#include "misc.h"

#include "gmconf.h"
#include "gmdialog.h"

#include <opal/transcoders.h>


#define new PNEW


/* The class */
GMH323Endpoint::GMH323Endpoint (GMManager & ep, Ekiga::ServiceCore & _core)
: H323EndPoint (ep), 
  endpoint (ep),
  core (_core),
  runtime (*(dynamic_cast<Ekiga::Runtime *> (core.get ("runtime"))))
{
  udp_min = 5000;
  udp_max = 5100; 
  tcp_min = 30000;
  tcp_max = 30010; 
  listen_port = 1720;

  SetInitialBandwidth (40000);

  uri_prefix = "h323:";
  protocol_name = "h323";

  start_listening ();
}

const std::string & GMH323Endpoint::get_protocol_name () const
{
  return protocol_name;
}


const Ekiga::CallManager::Interface & GMH323Endpoint::get_interface () const
{
  return interface;
}


bool GMH323Endpoint::populate_menu (Ekiga::Contact &contact,
                                   Ekiga::MenuBuilder &builder)
{
  std::string name = contact.get_name ();
  std::map<std::string, std::string> uris = contact.get_uris ();

  return menu_builder_add_actions (name, uris, builder);
}


bool GMH323Endpoint::populate_menu (const std::string uri,
                                   Ekiga::MenuBuilder & builder)
{
  std::map<std::string, std::string> uris; 
  uris [""] = uri;

  return menu_builder_add_actions ("", uris, builder);
}


bool GMH323Endpoint::menu_builder_add_actions (const std::string & /*fullname*/,
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

    builder.add_action ("call", action, sigc::bind (sigc::mem_fun (this, &GMH323Endpoint::on_dial), iter->second));

    populated = true;
  }

  return populated;
}

void
GMH323Endpoint::SetUserInputMode ()
{
  int mode = 0;

  gnomemeeting_threads_enter ();
  mode = gm_conf_get_int (H323_KEY "dtmf_mode");
  gnomemeeting_threads_leave ();

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


void
GMH323Endpoint::Register (const PString & aor,
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
    /*
    runtime.run_in_main (sigc::bind (registration_event.make_slot (), 
                                     aor,
                                     Ekiga::CallCore::Processing,
                                     std::string ()));
*/ //TODO
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

      /* Signal */
      /*
      runtime.run_in_main (sigc::bind (registration_event.make_slot (), 
                                       aor, 
                                       Ekiga::CallCore::RegistrationFailed,
                                       info));
                                       */
    }
    else {

      /* Signal */
      /*
      runtime.run_in_main (sigc::bind (registration_event.make_slot (), 
                                       aor,
                                       Ekiga::CallCore::Registered,
                                       std::string ()));
                                       */
    }
  }
  else if (unregister && IsRegisteredWithGatekeeper (host)) {

    H323EndPoint::RemoveGatekeeper (0);
    RemoveAliasName (authUserName);

    /* Signal */
    /*
    runtime.run_in_main (sigc::bind (registration_event.make_slot (), 
                                     aor,
                                     Ekiga::CallCore::Unregistered,
                                     std::string ()));
                                     */
  }
}


bool 
GMH323Endpoint::UseGatekeeper (const PString & address,
			       const PString & domain,
			       const PString & iface)
{
  bool result = 
    H323EndPoint::UseGatekeeper (address, domain, iface);

  PWaitAndSignal m(gk_name_mutex);
  
  gk_name = address;

  return result;
}
  

bool 
GMH323Endpoint::RemoveGatekeeper (const PString & address)
{
  if (IsRegisteredWithGatekeeper (address))
    return H323EndPoint::RemoveGatekeeper (0);

  return FALSE;
}
  
  
bool 
GMH323Endpoint::IsRegisteredWithGatekeeper (const PString & address)
{
  PWaitAndSignal m(gk_name_mutex);
  
  return ((gk_name *= address) && H323EndPoint::IsRegisteredWithGatekeeper ());
}


H323Connection *GMH323Endpoint::CreateConnection (OpalCall & _call,
                                                  const PString & token,
                                                  void *userData,
                                                  OpalTransport & transport,
                                                  const PString & alias,
                                                  const H323TransportAddress & address,
                                                  H323SignalPDU *setupPDU,
                                                  unsigned options,
                                                  OpalConnection::StringOptions *stringOptions)
{
  /* FIXME
  Ekiga::Call *call = dynamic_cast<Ekiga::Call *> (&_call);
  Ekiga::CallCore *call_core = dynamic_cast<Ekiga::CallCore *> (core.get ("call-core"));
  if (call_core)
    call_core->add_call (call, this);
    */

  return H323EndPoint::CreateConnection (_call, token, userData, transport, alias, address, setupPDU, options, stringOptions);
}


bool 
GMH323Endpoint::OnIncomingConnection (OpalConnection & /*connection*/,
                                      G_GNUC_UNUSED unsigned options,
                                      G_GNUC_UNUSED OpalConnection::StringOptions *str_options)
{
  PTRACE (3, "GMH323Endpoint\tIncoming connection");

  /*
  if (!forward_uri.empty () && unconditional_forward)
    connection.ForwardCall (forward_uri);
  else if (endpoint.GetCallsNumber () >= 1) { 

    if (!forward_uri.empty () && forward_on_busy)
      connection.ForwardCall (forward_uri);
    else 
      connection.ClearCall (OpalConnection::EndedByLocalBusy);
  }
  else
    return H323EndPoint::OnIncomingConnection (connection, options, stroptions);
    */ //TODO

  return false;
}


void 
GMH323Endpoint::OnRegistrationConfirm ()
{
  H323EndPoint::OnRegistrationConfirm ();
}

  
void 
GMH323Endpoint::OnRegistrationReject ()
{
  PWaitAndSignal m(gk_name_mutex);

  gk_name = PString::Empty ();

  H323EndPoint::OnRegistrationReject ();
}


void 
GMH323Endpoint::OnEstablished (OpalConnection &connection)
{
  PTRACE (3, "GMSIPEndpoint\t H.323 connection established");
  H323EndPoint::OnEstablished (connection);
}


void 
GMH323Endpoint::OnReleased (OpalConnection &connection)
{
  PTRACE (3, "GMSIPEndpoint\t H.323 connection released");
  H323EndPoint::OnReleased (connection);
}


bool GMH323Endpoint::start_listening ()
{
  std::stringstream str;
  RemoveListener (NULL);

  interface.publish = false;
  interface.voip_protocol = protocol_name;
  interface.protocol = "tcp";
  interface.interface = "*";

  str << "tcp$*:" << listen_port;
  if (StartListeners (PStringArray (str.str ().c_str ()))) {
    interface.port = listen_port;
    return true;
  }

  return false;
}


bool GMH323Endpoint::set_tcp_ports (const unsigned min, const unsigned max) 
{
  if (min > 0 && max > 0 && min < max) {

    tcp_min = min;
    tcp_max = max;
    endpoint.SetTCPPorts (tcp_min, tcp_max);

    return true;
  }

  return false;
}


bool GMH323Endpoint::set_udp_ports (const unsigned min, const unsigned max) 
{
  if (min > 0 && max > 0 && min + 12 < max) {

    udp_min = min;
    udp_max = max;
    endpoint.SetRtpIpPorts (udp_min, udp_max);
    endpoint.SetUDPPorts (udp_min, udp_max);

    return true;
  }

  return false;
}


bool GMH323Endpoint::set_listen_port (const unsigned listen)
{
  if (listen > 0) {

    listen_port = listen;
    return start_listening ();
  }

  return false;
}


void GMH323Endpoint::on_dial (std::string uri)
{
  endpoint.dial (uri);
}
