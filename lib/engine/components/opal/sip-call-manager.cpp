
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
 *                         sip-call-manager.cpp  -  description
 *                         ------------------------------------
 *   begin                : Sun Mar 15 2014
 *   authors              : Damien Sandras
 *   description          : This file contains the engine SIP CallManager.
 *
 */


#include "config.h"

#include <glib/gi18n.h>

#include "sip-call-manager.h"
#include "sip-endpoint.h"


/* The engine class */
Opal::Sip::CallManager::CallManager (Ekiga::ServiceCore& _core,
                               Opal::EndPoint& _endpoint) : Opal::CallManager (_core, _endpoint), protocol_name ("sip")
{
  /* Setup things */
  Ekiga::SettingsCallback setup_cb = boost::bind (&Opal::Sip::CallManager::setup, this, _1);
  sip_settings = Ekiga::SettingsPtr (new Ekiga::Settings (SIP_SCHEMA, setup_cb));

  setup ();
  std::cout << "hey: Created Opal::Sip::CallManager" << std::endl;
}


Opal::Sip::CallManager::~CallManager ()
{
  std::cout << "hey: Destroyed Opal::Sip::CallManager" << std::endl;
}


/* URIActionProvider Methods */
void Opal::Sip::CallManager::pull_actions (Ekiga::Actor & actor,
                                     G_GNUC_UNUSED const std::string & name,
                                     const std::string & uri)
{
  if (is_supported_uri (uri)) {
    add_action (actor, Ekiga::ActionPtr (new Ekiga::Action ("call", _("Call"), boost::bind (&Opal::Sip::CallManager::dial, this, uri))));
  }
}


const std::string & Opal::Sip::CallManager::get_protocol_name () const
{
  return protocol_name;
}


const Ekiga::CallManager::InterfaceList Opal::Sip::CallManager::get_interfaces () const
{
  Ekiga::CallManager::InterfaceList ilist;

  boost::shared_ptr<Opal::Sip::EndPoint> sip_endpoint = endpoint.get_sip_endpoint ();
  if (!sip_endpoint)
    return ilist;

  OpalListenerList listeners = sip_endpoint->GetListeners ();
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


void Opal::Sip::CallManager::setup (const std::string & setting)
{
  std::cout << "In Opal::Sip::CallManager::setup " << std::endl;
  boost::shared_ptr<Opal::Sip::EndPoint> sip_endpoint = endpoint.get_sip_endpoint ();
  if (sip_endpoint) {
    if (setting.empty () || setting == "listen-port")  {
      sip_endpoint->set_listen_port (sip_settings->get_int ("listen-port"));
    }
    if (setting.empty () || setting == "binding-timeout")  {
      sip_endpoint->set_nat_binding_delay (sip_settings->get_int ("binding-timeout"));
    }
    if (setting.empty () || setting == "outbound-proxy-host")  {
      sip_endpoint->set_outbound_proxy (sip_settings->get_string ("outbound-proxy-host"));
    }

    if (setting.empty () || setting == "dtmf-mode")  {
      sip_endpoint->set_dtmf_mode (sip_settings->get_enum ("dtmf-mode"));
    }

    if (setting.empty () || setting == "forward-host")  {
      sip_endpoint->set_forward_uri (sip_settings->get_string ("forward-host"));
    }
  }

  Opal::CallManager::setup (setting);
}
