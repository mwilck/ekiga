
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
                                     Opal::EndPoint& _endpoint,
                                     Opal::H323::EndPoint& _h323_endpoint)
        : Opal::CallManager (_core, _endpoint),
          h323_endpoint (_h323_endpoint), protocol_name ("h323")
{
  /* Setup things */
  Ekiga::SettingsCallback setup_cb = boost::bind (&Opal::H323::CallManager::setup, this, _1);
  h323_settings = Ekiga::SettingsPtr (new Ekiga::Settings (H323_SCHEMA, setup_cb));
  call_forwarding_settings = Ekiga::SettingsPtr (new Ekiga::Settings (CALL_FORWARDING_SCHEMA, setup_cb));
  video_codecs_settings = Ekiga::SettingsPtr (new Ekiga::Settings (VIDEO_CODECS_SCHEMA));
}


Opal::H323::CallManager::~CallManager ()
{
#if DEBUG
  std::cout << "Opal::H323::CallManager: Destructor invoked" << std::endl;
#endif
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


bool Opal::H323::CallManager::dial (const std::string & uri)
{
  if (!is_supported_uri (uri))
    return false;

  return h323_endpoint.SetUpCall (uri);
}


bool
Opal::H323::CallManager::is_supported_uri (const std::string & uri)
{
  return (!uri.empty () && uri.find ("h323:") == 0);
}


const std::string & Opal::H323::CallManager::get_protocol_name () const
{
  return protocol_name;
}


const Ekiga::CallManager::InterfaceList Opal::H323::CallManager::get_interfaces () const
{
  Ekiga::CallManager::InterfaceList ilist;

  OpalListenerList listeners = h323_endpoint.GetListeners ();
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


bool
Opal::H323::CallManager::set_listen_port (unsigned port)
{
  return h323_endpoint.StartListener (port);
}


void
Opal::H323::CallManager::set_dtmf_mode (unsigned mode)
{
  switch (mode)
    {
    case 0:
      h323_endpoint.SetSendUserInputMode (OpalConnection::SendUserInputAsString);
      PTRACE (4, "Opal::H323::CallManager\tSet DTMF Mode to String");
      break;
    case 1:
      h323_endpoint.SetSendUserInputMode (OpalConnection::SendUserInputAsTone);
      PTRACE (4, "Opal::H323::CallManager\tSet DTMF Mode to Tone");
      break;
    case 3:
      h323_endpoint.SetSendUserInputMode (OpalConnection::SendUserInputAsQ931);
      PTRACE (4, "Opal::H323::CallManager\tSet DTMF Mode to Q931");
      break;
    default:
      h323_endpoint.SetSendUserInputMode (OpalConnection::SendUserInputAsInlineRFC2833);
      PTRACE (4, "Opal::H323::CallManager\tSet DTMF Mode to RFC2833");
      break;
    }
}


unsigned
Opal::H323::CallManager::get_dtmf_mode () const
{
  if (h323_endpoint.GetSendUserInputMode () == OpalConnection::SendUserInputAsString)
    return 0;

  if (h323_endpoint.GetSendUserInputMode () == OpalConnection::SendUserInputAsTone)
    return 1;

  if (h323_endpoint.GetSendUserInputMode () == OpalConnection::SendUserInputAsInlineRFC2833)
    return 2;

  if (h323_endpoint.GetSendUserInputMode () == OpalConnection::SendUserInputAsQ931)
    return 2;

  g_return_val_if_reached (1);
}


void Opal::H323::CallManager::setup (const std::string & setting)
{
  if (setting.empty () || setting == "listen-port")
    set_listen_port (h323_settings->get_int ("listen-port"));

  if (setting.empty () || setting == "maximum-video-tx-bitrate") {

    // maximum_video_tx_bitrate is the max video bitrate specified by the user
    // add to it 10% (approx.) accounting for audio,
    // and multiply it by 10 as needed by SetInitialBandwidth
    int maximum_video_tx_bitrate = video_codecs_settings->get_int ("maximum-video-tx-bitrate");
    h323_endpoint.SetInitialBandwidth (OpalBandwidth::Tx, maximum_video_tx_bitrate > 0 ? maximum_video_tx_bitrate * 11 : 100000);
    PTRACE (4, "Opal::H323::EndPoint\tSet maximum/initial tx bandwidth to " << maximum_video_tx_bitrate * 11);
  }

  if (setting.empty () || setting == "enable-h245-tunneling") {

    h323_endpoint.DisableH245Tunneling (!h323_settings->get_bool ("enable-h245-tunneling"));
    PTRACE (4, "Opal::H323::EndPoint\tH.245 Tunneling: " << h323_settings->get_bool ("enable-h245-tunneling"));
  }

  if (setting.empty () || setting == "enable-early-h245") {

    h323_endpoint.DisableH245inSetup (!h323_settings->get_bool ("enable-early-h245"));
    PTRACE (4, "Opal::H323::EndPoint\tEarly H.245: " << h323_settings->get_bool ("enable-early-h245"));
  }

  if (setting.empty () || setting == "enable-fast-connect") {

    h323_endpoint.DisableFastStart (!h323_settings->get_bool ("enable-fast-connect"));
    PTRACE (4, "Opal::H323::EndPoint\tFast Connect: " << h323_settings->get_bool ("enable-fast-connect"));
  }

  if (setting.empty () || setting == "dtmf-mode") {

    set_dtmf_mode (h323_settings->get_enum ("dtmf-mode"));
  }


  /* Setup the various forwarding targets.
   * The no answer delay is defined in the opal-call-manager (our parent).
   */
  if (setting.empty () || setting == "forward-on-no-anwer" || setting == "forward-host")
    h323_endpoint.SetNoAnswerForwardTarget (call_forwarding_settings->get_bool ("forward-on-no-answer") ? h323_settings->get_string ("forward-host") : "");

  if (setting.empty () || setting == "forward-on-busy" || setting == "forward-host")
    h323_endpoint.SetBusyForwardTarget (call_forwarding_settings->get_bool ("forward-on-busy") ? h323_settings->get_string ("forward-host") : "");

  if (setting.empty () || setting == "always-forward" || setting == "forward-host")
    h323_endpoint.SetUnconditionalForwardTarget (call_forwarding_settings->get_bool ("always-forward") ? h323_settings->get_string ("forward-host") : "");

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

    h323_endpoint.SetDefaultH239Control(h323_settings->get_bool ("enable-h239"));
    PTRACE (4, "Opal::H323::EndPoint\tH.239 Control: " << h323_settings->get_bool ("enable-h239"));
  }

  // We do not call the parent setup () method as it is also handled
  // by our Opal::Sip::CallManager
}
