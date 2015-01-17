
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
 *                         sipendpoint.h  -  description
 *                         -----------------------------
 *   begin                : Wed 24 Nov 2004
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the SIP Endpoint class.
 *
 */


#ifndef _SIP_ENDPOINT_H_
#define _SIP_ENDPOINT_H_

#include <opal/opal.h>

#include "presence-core.h"
#include "call-manager.h"
#include "call-protocol-manager.h"
#include "opal-bank.h"
#include "sip-dialect.h"
#include "call-core.h"
#include "services.h"

#include "opal-call-manager.h"

#include "ekiga-settings.h"

namespace Opal {

  namespace Sip {

    class EndPoint : public SIPEndPoint,
		     public Ekiga::CallProtocolManager
    {
      PCLASSINFO(EndPoint, SIPEndPoint);

    public:

      typedef std::list<std::string> domain_list;
      typedef std::list<std::string>::iterator domain_list_iterator;

      EndPoint (CallManager& ep,
		const Ekiga::ServiceCore& core);

      ~EndPoint ();

      /* Set up endpoint: all options or a specific setting */
      void setup (std::string setting = "");


      /* Chat subsystem */
      bool send_message (const std::string & uri,
                         const Ekiga::Message::payload_type payload);


      /* CallProtocolManager */
      bool dial (const std::string & uri);

      bool is_supported_uri (const std::string & uri);


      const std::string & get_protocol_name () const;

      void set_dtmf_mode (unsigned mode);
      unsigned get_dtmf_mode () const;

      bool set_listen_port (unsigned port);

      const Ekiga::CallProtocolManager::InterfaceList & get_interfaces () const;


      /* SIP EndPoint */
      void set_nat_binding_delay (unsigned delay);
      unsigned get_nat_binding_delay ();

      void set_outbound_proxy (const std::string & uri);
      const std::string & get_outbound_proxy () const;

      void set_forward_uri (const std::string & uri);
      const std::string & get_forward_uri () const;

      // a message waiting information was received
      // the parameters are the aor and the info
      boost::signals2::signal<void(std::string, std::string)> mwi_event;

      /* Helpers */
      static std::string get_aor_domain (const std::string & aor);

      void update_aor_map (std::map<std::string, std::string> _accounts);

      /* Enable / Disable accounts. The account given as argument
       * will be updated to reflect the current account state once
       * the operation has been successful.
       */
      void enable_account (Account & account);
      void disable_account (Account & account);

      /* OPAL Methods */
      void OnRegistrationStatus (const RegistrationStatus & status);

      void OnMWIReceived (const PString & party,
                          OpalManager::MessageWaitingType type,
                          const PString & info);

      bool OnIncomingConnection (OpalConnection &connection,
                                 unsigned options,
                                 OpalConnection::StringOptions * stroptions);

      void OnDialogInfoReceived (const SIPDialogNotification & info);

      bool OnReceivedMESSAGE (SIP_PDU & pdu);

      void OnMESSAGECompleted (const SIPMessage::Params & params,
                               SIP_PDU::StatusCodes reason);


      /* Callbacks */
    private:
      void push_message_in_main (const std::string uri,
				 const Ekiga::Message msg);

      PMutex aorMutex;
      std::map<std::string, std::string> accounts;

      // this object is really managed by opal,
      // so the way it is handled here is correct
      CallManager & manager;

      std::map<std::string, PString> publications;

      Ekiga::CallProtocolManager::Interface listen_iface;

      std::string protocol_name;
      std::string uri_prefix;
      std::string forward_uri;
      std::string outbound_proxy;

      boost::shared_ptr<SIP::Dialect> dialect;

      boost::shared_ptr<Ekiga::Settings> settings;
      const Ekiga::ServiceCore & core;
      Ekiga::CallProtocolManager::InterfaceList interfaces;
    };
  };
};
#endif
