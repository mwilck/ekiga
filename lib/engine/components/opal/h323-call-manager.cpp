
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
 *                         h323-call-manager.cpp  -  description
 *                         -------------------------------------
 *   begin                : Sun Mar 15 2014
 *   authors              : Damien Sandras
 *   description          : This file contains the engine H.323 CallManager.
 *
 */


#include "config.h"

#include <glib/gi18n.h>

#include "h323-call-manager.h"
#include "h323-endpoint.h"


/* The engine class */
Opal::H323::CallManager::CallManager (Ekiga::ServiceCore& _core,
                                      Opal::EndPoint& _endpoint) : Opal::CallManager (_core, _endpoint), protocol_name ("h323")
{
  /* Setup things */
  Ekiga::SettingsCallback setup_cb = boost::bind (&Opal::H323::CallManager::setup, this, _1);
  h323_settings = Ekiga::SettingsPtr (new Ekiga::Settings (H323_SCHEMA, setup_cb));
  video_codecs_settings = Ekiga::SettingsPtr (new Ekiga::Settings (VIDEO_CODECS_SCHEMA));

  setup ();
  std::cout << "hey: Created Opal::H323::CallManager" << std::endl;
}


Opal::H323::CallManager::~CallManager ()
{
  std::cout << "hey: Destroyed Opal::H323::CallManager" << std::endl;
}


/* URIActionProvider Methods */
void Opal::H323::CallManager::pull_actions (Ekiga::Actor & actor,
                                            G_GNUC_UNUSED const std::string & name,
                                            const std::string & uri)
{
  if (is_supported_uri (uri)) {
    add_action (actor, Ekiga::ActionPtr (new Ekiga::Action ("call", _("Call"), boost::bind (&Opal::H323::CallManager::dial, this, uri))));
  }
}


const std::string & Opal::H323::CallManager::get_protocol_name () const
{
  return protocol_name;
}


const Ekiga::CallManager::InterfaceList Opal::H323::CallManager::get_interfaces () const
{
  Ekiga::CallManager::InterfaceList ilist;

  boost::shared_ptr<Opal::H323::EndPoint> h323_endpoint = endpoint.get_h323_endpoint ();
  if (!h323_endpoint)
    return ilist;

  OpalListenerList listeners = h323_endpoint->GetListeners ();
  for (int i = 0 ; i < listeners.GetSize () ; i++) {
    Ekiga::CallManager::Interface iface;
    PIPSocket::Address address;
    WORD port;
    PString proto_prefix = listeners[i].GetLocalAddress ().GetProtoPrefix ();
    listeners[i].GetLocalAddress ().GetIpAndPort (address, port);

    iface.voip_protocol = get_protocol_name ();
    iface.id = "*";
    iface.protocol = (const char*) proto_prefix.Left (proto_prefix.GetLength () - 1); // Strip final $ delimiter
    iface.port = (unsigned int) port;

    ilist.push_back (iface);
  }

  return ilist;
}


void Opal::H323::CallManager::setup (const std::string & setting)
{
  std::cout << "In Opal::H323::EndPoint::setup " << std::endl;
  boost::shared_ptr<Opal::H323::EndPoint> h323_endpoint = endpoint.get_h323_endpoint ();
  if (h323_endpoint) {

    if (setting.empty () || setting == "listen-port") {

      h323_endpoint->set_listen_port (h323_settings->get_int ("listen-port"));
    }
    if (setting.empty () || setting == "maximum-video-tx-bitrate") {

      int maximum_video_tx_bitrate = video_codecs_settings->get_int ("maximum-video-tx-bitrate");
      // maximum_video_tx_bitrate is the max video bitrate specified by the user
      // add to it 10% (approx.) accounting for audio,
      // and multiply it by 10 as needed by SetInitialBandwidth
      h323_endpoint->set_initial_bandwidth (maximum_video_tx_bitrate * 11);
    }
    if (setting.empty () || setting == "enable-h245-tunneling") {

      h323_endpoint->DisableH245Tunneling (!h323_settings->get_bool ("enable-h245-tunneling"));
      PTRACE (4, "Opal::H323::EndPoint\tH.245 Tunneling: " << h323_settings->get_bool ("enable-h245-tunneling"));
    }
    if (setting.empty () || setting == "enable-early-h245") {

      h323_endpoint->DisableH245inSetup (!h323_settings->get_bool ("enable-early-h245"));
      PTRACE (4, "Opal::H323::EndPoint\tEarly H.245: " << h323_settings->get_bool ("enable-early-h245"));
    }
    if (setting.empty () || setting == "enable-fast-connect") {

      h323_endpoint->DisableFastStart (!h323_settings->get_bool ("enable-fast-connect"));
      PTRACE (4, "Opal::H323::EndPoint\tFast Connect: " << h323_settings->get_bool ("enable-fast-connect"));
    }
    if (setting.empty () || setting == "dtmf-mode") {

      h323_endpoint->set_dtmf_mode (h323_settings->get_enum ("dtmf-mode"));
    }
    if (setting.empty () || setting == "forward-host") {

      h323_endpoint->set_forward_uri (h323_settings->get_string ("forward-host"));
    }
    if (setting.empty () || setting == "video-role") {

      /*
         CallManager::VideoOptions options;
         endpoint.get_video_options (options);
         options.extended_video_roles = h323_settings->get_enum ("video-role");
         endpoint.set_video_options (options);
       */

      std::cout << "FIXME" << std::endl;
    }
    if (setting.empty () || setting == "enable-h239") {
      h323_endpoint->SetDefaultH239Control(h323_settings->get_bool ("enable-h239"));
      PTRACE (4, "Opal::H323::EndPoint\tH.239 Control: " << h323_settings->get_bool ("enable-h239"));
    }

  }

  // We do not call the parent setup () method as it is also handled
  // by our Opal::Sip::CallManager
}
