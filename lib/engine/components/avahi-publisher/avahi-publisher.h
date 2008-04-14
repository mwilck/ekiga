
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras
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
 *                         avahi-publisher.h  -  description
 *                         ---------------------------------
 *   begin                : Mon 14 Apr 2008
 *   copyright            : (C) 2000-2008 by Damien Sandras
 *   description          : This file contains the avahi publisher.
 *
 */



#ifndef _AVAHI_PUBLISHER_H_
#define _AVAHI_PUBLISHER_H_

#include "presence-core.h"
#include "services.h"

class Ekiga::PersonalDetails;

namespace Avahi
{
  class PresencePublisher : public Ekiga::PresencePublisher,
                            public Ekiga::Service
  {
    public:
      PresencePublisher (Ekiga::ServiceCore & core);

      /*** Service API ***/
      const std::string get_name () const
        { return "avahi-presence-publisher"; }

      const std::string get_description () const
        { return "\tObject bringing in Avahi presence publishing"; }

      /*** PresencePublisher API ***/
      void publish (const Ekiga::PersonalDetails & details);

    private:
      Ekiga::ServiceCore & core;
  };
};
#endif
