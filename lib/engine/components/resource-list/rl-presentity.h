
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
 *                         rl-presentity.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : resource-list presentity interface
 *
 */

#ifndef __RL_PRESENTITY_H__
#define __RL_PRESENTITY_H__

#include "services.h"
#include "presentity.h"

namespace RL {

  class Presentity: public Ekiga::Presentity
  {
  public:

    Presentity (Ekiga::ServiceCore& core_,
		std::string name_,
		std::string uri_);

    ~Presentity ();

    void add_group (std::string group);

    const std::string get_name () const
    { return name; }

    const std::string get_presence () const
    { return presence; }

    const std::string get_status () const
    { return status; }

    const std::string get_avatar () const
    { return ""; }

    const std::set<std::string> get_groups () const
    { return groups; }

    bool populate_menu (Ekiga::MenuBuilder& builder);

    const std::string get_uri () const
    { return uri; }

    void set_presence (const std::string presence_);

    void set_status (const std::string status_);

  private:
    Ekiga::ServiceCore& core;

    std::string name;
    std::string uri;
    std::string presence;
    std::string status;
    std::set<std::string> groups;
  };
};

#endif
