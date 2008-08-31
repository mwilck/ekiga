
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras
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
 *                         avahi-presentity.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of an avahi presentity
 *
 */

#include "config.h"

#include "avahi-presentity.h"

Avahi::Presentity::Presentity (Ekiga::PresenceCore &_core,
			       std::string _name,
			       std::string _url):
  core(_core), name(_name), presence("unknown"), url (_url)
{
  groups.insert (_("Neighbours"));
}

Avahi::Presentity::~Presentity ()
{
}

const std::string
Avahi::Presentity::get_presence () const
{
  return presence;
}

const std::string
Avahi::Presentity::get_status () const
{
  return status;
}

const std::string
Avahi::Presentity::get_name () const
{
  return name;
}

const std::string
Avahi::Presentity::get_avatar () const
{
  return "";
}

const std::set<std::string>
Avahi::Presentity::get_groups () const
{
  return groups;
}

const std::string
Avahi::Presentity::get_uri () const
{
  return url;
}

bool
Avahi::Presentity::populate_menu (Ekiga::MenuBuilder &builder)
{
  return core.populate_presentity_menu (*this, url, builder);
}

void
Avahi::Presentity::set_presence (const std::string _presence)
{
  presence = _presence;
}

void
Avahi::Presentity::set_status (const std::string _status)
{
  status = _status;
}
