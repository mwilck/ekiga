
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
 *                         rl-presentity.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : resource-list presentity implementation
 *
 */

#include "rl-presentity.h"

#include "presence-core.h"

RL::Presentity::Presentity (Ekiga::ServiceCore& core_,
			    std::string name_,
			    std::string uri_):
  core(core_), name(name_), uri(uri_), presence("unknown")
{
}

RL::Presentity::~Presentity ()
{
}

void
RL::Presentity::add_group (std::string group)
{
  groups.insert (group);
}

bool
RL::Presentity::populate_menu (Ekiga::MenuBuilder &builder)
{
  bool populated = false;
  Ekiga::PresenceCore* presence_core
    = dynamic_cast<Ekiga::PresenceCore*>(core.get ("presence-core"));

  populated = presence_core->populate_presentity_menu (*this, uri, builder);


  return populated;
}

void
RL::Presentity::set_presence (const std::string presence_)
{
  presence = presence_;
  updated.emit ();
}

void
RL::Presentity::set_status (const std::string status_)
{
  status = status_;
  updated.emit ();
}
