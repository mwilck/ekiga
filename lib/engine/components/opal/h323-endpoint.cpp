
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

    class subscriber : public PThread
    {
      PCLASSINFO(subscriber, PThread);

  public:

      subscriber (const Opal::Account & _account,
                  Opal::H323::EndPoint& _manager,
                  bool _registering,
                  const PSafePtr<OpalPresentity> & _presentity)
        : PThread (1000, AutoDeleteThread),
        account (_account),
        manager (_manager),
        registering (_registering),
        presentity (_presentity)
      {
        this->Resume ();
      };

      void Main ()
        {
          if (registering) {
            if (presentity && !presentity->IsOpen ())
              presentity->Open ();
            manager.Register (account);
          } else {
            manager.Unregister (account);

            if (presentity && presentity->IsOpen ())
              presentity->Close ();

          }
        };

  private:
      const Opal::Account & account;
      Opal::H323::EndPoint& manager;
      bool registering;
      const PSafePtr<OpalPresentity> & presentity;
    };
  };
};


/* The class */
Opal::H323::EndPoint::EndPoint (Opal::CallManager & _manager):
    H323EndPoint (_manager),
    manager (_manager)
{
  protocol_name = "h323";
  uri_prefix = "h323:";
  /* Ready to take calls */
  manager.AddRouteEntry("h323:.* = pc:*");
  manager.AddRouteEntry("pc:.* = h323:<da>");

  settings = boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (H323_SCHEMA));
  settings->changed.connect (boost::bind (&EndPoint::setup, this, _1));

  video_codecs_settings = boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (VIDEO_CODECS_SCHEMA));
  video_codecs_settings->changed.connect (boost::bind (&EndPoint::setup, this, _1));
}

Opal::H323::EndPoint::~EndPoint ()
{
}


bool
Opal::H323::EndPoint::dial (const std::string&  uri)
{
  if (!is_supported_uri (uri))
    return false;

  PString token;
  manager.SetUpCall("pc:*", uri, token, (void*) uri.c_str());

  return true;
}


bool
Opal::H323::EndPoint::transfer (const std::string & uri,
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
Opal::H323::EndPoint::message (G_GNUC_UNUSED const Ekiga::ContactPtr & contact,
                               G_GNUC_UNUSED const std::string & uri)
{
  return false; /* Not reimplemented yet */
}


bool
Opal::H323::EndPoint::is_supported_uri (const std::string & uri)
{
  return (!uri.empty () && uri.find ("h323:") == 0);
}


const std::string&
Opal::H323::EndPoint::get_protocol_name () const
{
  return protocol_name;
}


void
Opal::H323::EndPoint::set_dtmf_mode (unsigned mode)
{
  switch (mode)
    {
    case 0:
      SetSendUserInputMode (OpalConnection::SendUserInputAsString);
      PTRACE (4, "Opal::H323::EndPoint\tSet DTMF Mode to String");
      break;
    case 1:
      SetSendUserInputMode (OpalConnection::SendUserInputAsTone);
      PTRACE (4, "Opal::H323::EndPoint\tSet DTMF Mode to Tone");
      break;
    case 3:
      SetSendUserInputMode (OpalConnection::SendUserInputAsQ931);
      PTRACE (4, "Opal::H323::EndPoint\tSet DTMF Mode to Q931");
      break;
    default:
      SetSendUserInputMode (OpalConnection::SendUserInputAsInlineRFC2833);
      PTRACE (4, "Opal::H323::EndPoint\tSet DTMF Mode to RFC2833");
      break;
    }
}


unsigned
Opal::H323::EndPoint::get_dtmf_mode () const
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


bool
Opal::H323::EndPoint::set_listen_port (unsigned port)
{
  listen_iface.protocol = "tcp";
  listen_iface.voip_protocol = "h323";
  listen_iface.id = "*";

  port = (port > 0 ? port : 1720);

  std::stringstream str;
  RemoveListener (NULL);

  str << "tcp$*:" << port;
  if (StartListeners (PStringArray (str.str ()))) {

    listen_iface.port = port;
    PTRACE (4, "Opal::H323::EndPoint\tSet listen port to " << port);
    return true;
  }

  return false;
}

void
Opal::H323::EndPoint::set_initial_bandwidth (unsigned bitrate)
{
  SetInitialBandwidth (OpalBandwidth::Tx, bitrate > 0 ? bitrate : 100000);
  PTRACE (4, "Opal::H323::EndPoint\tSet maximum/initial tx bandwidth to " << bitrate);
}


const Ekiga::CallProtocolManager::Interface&
Opal::H323::EndPoint::get_listen_interface () const
{
  return listen_iface;
}


void
Opal::H323::EndPoint::set_forward_uri (const std::string& uri)
{
  if (!uri.empty ())
    forward_uri = uri;
  PTRACE (4, "Opal::H323::EndPoint\tSet Forward URI to " << uri);
}


const std::string&
Opal::H323::EndPoint::get_forward_uri () const
{
  return forward_uri;
}


bool
Opal::H323::EndPoint::subscribe (const Opal::Account & account,
                                 const PSafePtr<OpalPresentity> & presentity)
{
  if (account.get_protocol_name () != "H323")
    return false;

  new subscriber (account, *this, true, presentity);

  return true;
}


bool
Opal::H323::EndPoint::unsubscribe (const Opal::Account & account,
                                   const PSafePtr<OpalPresentity> & presentity)
{
  if (account.get_protocol_name () != "H323")
    return false;

  new subscriber (account, *this, false, presentity);

  return true;
}


void
Opal::H323::EndPoint::Register (const Opal::Account& account)
{
  std::string info;

  if (account.is_enabled () && !IsRegisteredWithGatekeeper (account.get_host ())) {

    H323EndPoint::RemoveGatekeeper (0);

    if (!account.get_username ().empty ()) {
      SetLocalUserName (account.get_username ());
      AddAliasName (manager.GetDefaultDisplayName ());
    }

    SetGatekeeperPassword (account.get_password (), account.get_username ());
    SetGatekeeperTimeToLive (account.get_timeout () * 1000);
    bool result = UseGatekeeper (account.get_host ());

    // There was an error (missing parameter or registration failed)
    // or the user chose to not register
    if (!result) {

      // Registering failed
      if (GetGatekeeper () != NULL) {

        switch (GetGatekeeper()->GetRegistrationFailReason ()) {

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
      Ekiga::Runtime::run_in_main (boost::bind (&Opal::H323::EndPoint::registration_event_in_main, this, boost::cref (account), Account::RegistrationFailed, info));
    }
    else {

      Ekiga::Runtime::run_in_main (boost::bind (&Opal::H323::EndPoint::registration_event_in_main, this, boost::cref (account), Account::Registered, std::string ()));
    }
  }
}


void
Opal::H323::EndPoint::Unregister (const Opal::Account& account)
{
  RemoveGatekeeper (account.get_host ());
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
  bool busy = false;

  PTRACE (3, "Opal::H323::EndPoint\tIncoming connection");

  if (!H323EndPoint::OnIncomingConnection (connection, options, stroptions))
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
      else
        call->set_reject_delay (manager.get_reject_delay ());
    }

    return H323EndPoint::OnIncomingConnection (connection, options, stroptions);
  }

  return false;
}


void
Opal::H323::EndPoint::registration_event_in_main (const Opal::Account& account,
						  Opal::Account::RegistrationState state,
						  const std::string msg)
{
  account.handle_registration_event (state, msg);
}


void
Opal::H323::EndPoint::setup (const std::string setting)
{
  if (setting.empty () || setting == "listen-port") {

    set_listen_port (settings->get_int ("listen-port"));
  }
  if (setting.empty () || setting == "maximum-video-tx-bitrate") {

    int maximum_video_tx_bitrate = video_codecs_settings->get_int ("maximum-video-tx-bitrate");
    // maximum_video_tx_bitrate is the max video bitrate specified by the user
    // add to it 10% (approx.) accounting for audio,
    // and multiply it by 10 as needed by SetInitialBandwidth
    set_initial_bandwidth (maximum_video_tx_bitrate * 11);
  }
  if (setting.empty () || setting == "enable-h245-tunneling") {

    DisableH245Tunneling (!settings->get_bool ("enable-h245-tunneling"));
    PTRACE (4, "Opal::H323::EndPoint\tH.245 Tunneling: " << settings->get_bool ("enable-h245-tunneling"));
  }
  if (setting.empty () || setting == "enable-early-h245") {

    DisableH245inSetup (!settings->get_bool ("enable-early-h245"));
    PTRACE (4, "Opal::H323::EndPoint\tEarly H.245: " << settings->get_bool ("enable-early-h245"));
  }
  if (setting.empty () || setting == "enable-fast-connect") {

    DisableFastStart (!settings->get_bool ("enable-fast-connect"));
    PTRACE (4, "Opal::H323::EndPoint\tFast Connect: " << settings->get_bool ("enable-fast-connect"));
  }
  if (setting.empty () || setting == "dtmf-mode") {

    set_dtmf_mode (settings->get_enum ("dtmf-mode"));
  }
  if (setting.empty () || setting == "forward-host") {

    set_forward_uri (settings->get_string ("forward-host"));
  }
  if (setting.empty () || setting == "video-role") {

    CallManager::VideoOptions options;
    manager.get_video_options (options);
    options.extended_video_roles = settings->get_enum ("video-role");
    manager.set_video_options (options);
  }
  if (setting.empty () || setting == "enable-h239") {
    SetDefaultH239Control(settings->get_bool ("enable-h239"));
    PTRACE (4, "Opal::H323::EndPoint\tH.239 Control: " << settings->get_bool ("enable-h239"));
  }
}
