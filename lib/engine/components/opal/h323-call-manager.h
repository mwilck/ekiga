
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2015 Damien Sandras <dsandras@seconix.com>
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
 *                         h323-call-manager.h  -  description
 *                         ----------------------------------
 *   begin                : Sun Mar 15 2014
 *   authors              : Damien Sandras
 *   description          : This file contains the engine H.323 CallManager.
 *
 */


#ifndef __H323_CALL_MANAGER_H_
#define __H323_CALL_MANAGER_H_

#include "opal-call-manager.h"
#include "opal-endpoint.h"
#include "h323-endpoint.h"

#include "ekiga-settings.h"

namespace Opal {

  namespace H323 {

    /* This is one engine H.323 CallManager implementation.
     * It uses the Opal::CallManager class to implement a
     * more specialized H.323 engine CallManager.
     */
    class CallManager :
        public Opal::CallManager,
        public Ekiga::URIActionProvider
    {
  public:
      CallManager (Ekiga::ServiceCore& core,
                   Opal::EndPoint& endpoint,
                   Opal::H323::EndPoint& h323_endpoint);
      ~CallManager ();

      /* URIActionProvider Methods */
      void pull_actions (Ekiga::Actor & actor,
                         const std::string & name,
                         const std::string & uri);

      /* CallManager methods we implement */
      bool dial (const std::string & uri);

      bool is_supported_uri (const std::string & uri);

      const std::string & get_protocol_name () const;

      bool set_listen_port (unsigned port);

      const Ekiga::CallManager::InterfaceList get_interfaces () const;

      void set_dtmf_mode (unsigned mode);

      unsigned get_dtmf_mode () const;

  private:
      void setup (const std::string & setting = "");

      Ekiga::SettingsPtr h323_settings;
      Ekiga::SettingsPtr call_forwarding_settings;
      Ekiga::SettingsPtr video_codecs_settings;
      Opal::H323::EndPoint& h323_endpoint;
      std::string protocol_name;
    };
  };
};
#endif
