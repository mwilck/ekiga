
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
 *                         avahi_publish.h  -  description
 *                         ------------------------------------
 *   begin                : Sun Aug 21 2005
 *   copyright            : (C) 2005 by Sebastien Estienne 
 *   description          : This file contains the Avahi zeroconf publisher. 
 *
 */


#ifndef _AVAHI_PUBLISHER_H_
#define _AVAHI_PUBLISHER_H_

#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/alternative.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>
#include <avahi-glib/glib-watch.h>

#include "presence-core.h"
#include "services.h"

#include "call-manager.h"

class Ekiga::PersonalDetails;

namespace Avahi
{
  class PresencePublisher 
    : public Ekiga::PresencePublisher,
      public Ekiga::Service
  {
public:
    PresencePublisher (Ekiga::ServiceCore & core);
    ~PresencePublisher ();

    
    /*** Service API ***/
    const std::string get_name () const
      { return "avahi-presence-publisher"; }

    const std::string get_description () const
      { return "\tObject bringing in Avahi presence publishing"; }

    
    /*** PresencePublisher API ***/
    void publish (const Ekiga::PersonalDetails & details);


    /*** Avahi::PresencePublisher API ***/
    bool connect (); 
    void disconnect ();

    void client_callback (AvahiClient *client, 
                          AvahiClientState state);

    void entry_group_callback (AvahiEntryGroup *group, 
                               AvahiEntryGroupState state);

private:
    Ekiga::ServiceCore & core;
    AvahiClient *client;
    AvahiEntryGroup *group;

    char *name;                    /* Srv Record */
    uint16_t port;                 /* port number of Srv Record */
    AvahiStringList *text_record;  /* H323 Txt Record */

    AvahiGLibPoll *glib_poll;
    const AvahiPoll *poll_api;

    std::map<std::string, Ekiga::CallManager::Interface> to_publish;
  };
};
#endif
