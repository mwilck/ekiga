
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
GMH323Endpoint::GMH323Endpoint (GMManager & ep)
	: H323EndPoint (ep), endpoint (ep)
{
  udp_min = 5000;
  udp_max = 5100; 
  tcp_min = 30000;
  tcp_max = 30010; 
  listen_port = 5060;

  SetInitialBandwidth (40000);
}


GMH323Endpoint::~GMH323Endpoint ()
{
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

    /* Signal the OpalManager */
    endpoint.OnRegistering (aor, true);

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

      /* Signal the OpalManager */
      endpoint.OnRegistrationFailed (aor, true, info);
    }
    else {
      /* Signal the OpalManager */
      endpoint.OnRegistered (aor, true);
    }
  }
  else if (unregister && IsRegisteredWithGatekeeper (host)) {

    H323EndPoint::RemoveGatekeeper (0);
    RemoveAliasName (authUserName);

    /* Signal the OpalManager */
    endpoint.OnRegistered (aor, false);
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

  str << "tcp$*:" << listen_port;
  if (StartListeners (PStringArray (str.str ().c_str ()))) 
    return true;

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
    std::cout << "set udp " << udp_min << " : " << udp_max << std::endl << std::flush;
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


