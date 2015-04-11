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
 *                         h323endpoint.cpp  -  description
 *                         --------------------------------
 *   begin                : Wed 24 Nov 2004
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the H.323 Endpoint class.
 *
 */


#include <glib/gi18n.h>

#include "h323-endpoint.h"

namespace Opal {

  namespace H323 {

    class GatekeeperHandler : public PThread
    {
      PCLASSINFO (GatekeeperHandler, PThread);

  public:

      GatekeeperHandler (Opal::Account & _account,
                          Opal::H323::EndPoint& _ep,
                          bool _registering)
        : PThread (1000, AutoDeleteThread),
        account (_account),
        ep (_ep),
        registering (_registering)
      {
        this->Resume ();
      }

      void Main ()
      {
        std::string info;

        if (!registering && ep.IsRegisteredWithGatekeeper (account.get_host ())) {
          ep.RemoveGatekeeper (account.get_host ());
          account.handle_registration_event (Account::Unregistered, std::string ());
          return;
        }
        else if (registering && !ep.IsRegisteredWithGatekeeper (account.get_host ())) {

          ep.RemoveGatekeeper (0);

          if (!account.get_username ().empty ())
            ep.SetLocalUserName (account.get_username ());

          ep.SetGatekeeperPassword (account.get_password (), account.get_username ());
          ep.SetGatekeeperTimeToLive (account.get_timeout () * 1000);
          bool result = ep.UseGatekeeper (account.get_host ());

          // There was an error (missing parameter or registration failed)
          // or the user chose to not register
          if (!result) {

            // Registering failed
            if (ep.GetGatekeeper () != NULL) {

              switch (ep.GetGatekeeper()->GetRegistrationFailReason ()) {

              case H323Gatekeeper::DuplicateAlias :
                // Translators : The alias we are registering already exists : failure
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
              case H323Gatekeeper::TryingAlternate:
              case H323Gatekeeper::NumRegistrationFailReasons:
              case H323Gatekeeper::GatekeeperRejectReasonMask:
              case H323Gatekeeper::RegistrationRejectReasonMask:
              case H323Gatekeeper::UnregistrationRejectReasonMask:
              default:
                info = _("Failed");
                break;
              }
            }
            else
              info = _("Failed");

            // Signal
            account.handle_registration_event (Account::RegistrationFailed, info);
          }
          else {

            account.handle_registration_event (Account::Registered, std::string ());
          }
        }
      }

  private:
      Opal::Account & account;
      Opal::H323::EndPoint& ep;
      bool registering;
    };
  };
};


/* The class */
Opal::H323::EndPoint::EndPoint (Opal::EndPoint & _endpoint,
                                const Ekiga::ServiceCore& _core): H323EndPoint (_endpoint),
                                                                  core (_core)
{
  /* Ready to take calls */
  GetManager ().AddRouteEntry("h323:.* = pc:*");
  GetManager ().AddRouteEntry("pc:.* = h323:<da>");
}


Opal::H323::EndPoint::~EndPoint ()
{
}


bool
Opal::H323::EndPoint::SetUpCall (const std::string&  uri)
{
  PString token;
  GetManager ().SetUpCall("pc:*", uri, token, (void*) uri.c_str());

  return true;
}


bool
Opal::H323::EndPoint::message (G_GNUC_UNUSED const Ekiga::ContactPtr & contact,
                               G_GNUC_UNUSED const std::string & uri)
{
  return false; /* Not reimplemented yet */
}


bool
Opal::H323::EndPoint::StartListener (unsigned port)
{
  if (port > 0) {

    RemoveListener (NULL);

    std::stringstream str;
    str << "tcp$*:" << port;
    if (StartListeners (PStringArray (str.str ()))) {
      PTRACE (4, "Opal::H323::EndPoint\tSet listen port to " << port);
      return true;
    }
  }

  return false;
}


void
Opal::H323::EndPoint::EnableAccount (Account& account)
{
  new GatekeeperHandler (account, *this, true);
}


void
Opal::H323::EndPoint::DisableAccount (Account& account)
{
  new GatekeeperHandler (account, *this, false);
}


void
Opal::H323::EndPoint::SetNoAnswerForwardTarget (const PString & _party)
{
  noAnswerForwardParty = _party;
}


void
Opal::H323::EndPoint::SetUnconditionalForwardTarget (const PString & _party)
{
  unconditionalForwardParty = _party;
}


void
Opal::H323::EndPoint::SetBusyForwardTarget (const PString & _party)
{
  busyForwardParty = _party;
}


bool
Opal::H323::EndPoint::UseGatekeeper (const PString & address,
                                     const PString & domain,
                                     const PString & iface)
{
  bool result = false;

  if (!IsRegisteredWithGatekeeper (address)) {
    result = H323EndPoint::UseGatekeeper (address, domain, iface);

    if (result) {
      PWaitAndSignal m(gk_name_mutex);
      gk_name = address;
    }
  }

  return result;
}


bool
Opal::H323::EndPoint::RemoveGatekeeper (const PString & address)
{
  if (IsRegisteredWithGatekeeper (address))
    return H323EndPoint::RemoveGatekeeper (0);

  return FALSE;
}


bool
Opal::H323::EndPoint::IsRegisteredWithGatekeeper (const PString & address)
{
  PWaitAndSignal m(gk_name_mutex);

  return ((gk_name *= address) && H323EndPoint::IsRegisteredWithGatekeeper ());
}


bool
Opal::H323::EndPoint::OnIncomingConnection (OpalConnection & connection,
					    G_GNUC_UNUSED unsigned options,
					    G_GNUC_UNUSED OpalConnection::StringOptions *stroptions)
{
  PTRACE (3, "Opal::H323::EndPoint\tIncoming connection");

  if (!H323EndPoint::OnIncomingConnection (connection, options, stroptions))
    return false;

  /* Unconditional call forward? */
  if (!unconditionalForwardParty.IsEmpty ()) {
    PTRACE (3, "Opal::H323::EndPoint\tIncoming connection forwarded to " << busyForwardParty << " (Unconditional)");
    connection.ForwardCall (unconditionalForwardParty);
    return false;
  }

  /* Busy call forward? */
  for (PSafePtr<OpalConnection> conn(connectionsActive, PSafeReference); conn != NULL; ++conn) {
    if (conn->GetCall().GetToken() != connection.GetCall().GetToken() && !conn->IsReleased ()) {
      if (!busyForwardParty.IsEmpty ()) {
        PTRACE (3, "Opal::H323::EndPoint\tIncoming connection forwarded to " << busyForwardParty << " (busy)");
        connection.ForwardCall (busyForwardParty);
      }
      else {
        PTRACE (3, "Opal::H323::EndPoint\tIncoming connection rejected (busy)");
        connection.ClearCall (OpalConnection::EndedByLocalBusy);
      }
      return false;
    }
  }

  /* No Answer Call Forward or Reject */
  Opal::Call *call = dynamic_cast<Opal::Call *> (&connection.GetCall ());
  if (call)
    call->set_forward_target (noAnswerForwardParty);

  return true;
}
