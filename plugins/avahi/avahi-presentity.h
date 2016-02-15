/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>

 * This program is free software; you can  redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ekiga is licensed under the GPL license and as a special exception, you
 * have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OPAL, OpenH323 and PWLIB
 * programs, as long as you do follow the requirements of the GNU GPL for all
 * the rest of the software thus combined.
 */


/*
 *                         avahi-presentity.h  -  description
 *                         ----------------------------------
 *   begin                : written in 2015 by Damien Sandras
 *   copyright            : (c) 2015 by Damien Sandras
 *   description          : declaration of Avahi::Presentity
 *
 */



#ifndef __AVAHI_PRESENTITY_H__
#define __AVAHI_PRESENTITY_H__

#include "dynamic-object.h"
#include "presentity.h"

#include "presence-core.h"

namespace Avahi
{
  class Presentity:
      public Ekiga::Presentity,
      public Ekiga::DynamicObject<Presentity>
  {

public:
    static boost::shared_ptr<Presentity> create (boost::shared_ptr<Ekiga::PresenceCore> presence_core,
                                                 std::string name,
                                                 std::string uri,
                                                 std::list<std::string> groups);

    ~Presentity ();

    /**
     * Getters for the presentity
     */
    const std::string get_name () const;

    const std::string get_presence () const;

    const std::string get_note () const;

    const std::list<std::string> get_groups () const;

    bool has_uri (const std::string uri_) const;

    const std::string get_uri () const;

  private:
    Ekiga::scoped_connections connections;

    std::string name;
    std::string uri;
    std::string presence;
    std::list<std::string> groups;
    std::string note;

    void on_presence_received (std::string uri_,
                               std::string presence_);

    void on_note_received (std::string uri_,
                           std::string note_);

private:
    Presentity (boost::shared_ptr<Ekiga::PresenceCore> presence_core,
                std::string name,
                std::string uri,
                std::list<std::string> groups);
  };
};

#endif
