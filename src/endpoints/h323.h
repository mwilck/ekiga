
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
 *                         h323endpoint.h  -  description
 *                         ------------------------------
 *   begin                : Wed 24 Nov 2004
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the H.323 Endpoint class.
 *
 */


#ifndef _H323_ENDPOINT_H_
#define _H323_ENDPOINT_H_


#include "config.h"

#include "common.h"

#include "manager.h"


/* Minimal H.323 endpoint implementation */
class GMH323Endpoint : 
    public H323EndPoint,
    public Ekiga::CallProtocolManager,
    public Ekiga::PresentityDecorator,
    public Ekiga::ContactDecorator
{
  PCLASSINFO(GMH323Endpoint, H323EndPoint);

 public:

  GMH323Endpoint (GMManager &ep, Ekiga::ServiceCore & core, unsigned listen_port);


  /* ContactDecorator and PresentityDecorator */
  bool populate_menu (Ekiga::Contact &contact,
                      Ekiga::MenuBuilder &builder);

  bool populate_menu (const std::string uri,
                      Ekiga::MenuBuilder & builder);

  bool menu_builder_add_actions (const std::string & fullname,
                                 std::map<std::string, std::string> & uris,
                                 Ekiga::MenuBuilder & builder);


  /* CallProtocolManager */
  bool dial (const std::string & uri); 

  const std::string & get_protocol_name () const;

  void set_dtmf_mode (unsigned mode);
  unsigned get_dtmf_mode () const;

  bool set_listen_port (unsigned port);
  const Ekiga::CallProtocolManager::Interface & get_listen_interface () const;


  /* H.323 CallProtocolManager */
  void set_forward_uri (const std::string & uri);
  const std::string & get_forward_uri () const;


  /* OPAL methods */
  void Register (const PString & aor,
                 const PString & authUserName,
                 const PString & password,
                 const PString & gatekeeperID,
                 unsigned int expires,
                 bool unregister);

  bool UseGatekeeper (const PString & address = PString::Empty (),
		      const PString & domain = PString::Empty (),
		      const PString & iface = PString::Empty ());
  
  bool RemoveGatekeeper (const PString & address);
  
  bool IsRegisteredWithGatekeeper (const PString & address);
  
  bool OnIncomingConnection (OpalConnection &connection,
                             unsigned options,
                             OpalConnection::StringOptions *str_options);


 private:
  void on_dial (std::string uri);

  GMManager & endpoint;
  Ekiga::ServiceCore & core;
  Ekiga::Runtime & runtime;

  PMutex gk_name_mutex;
  PString gk_name;

  Ekiga::CallProtocolManager::Interface interface;

  std::string protocol_name;
  std::string uri_prefix;
  std::string forward_uri;

  unsigned listen_port;
};

#endif
