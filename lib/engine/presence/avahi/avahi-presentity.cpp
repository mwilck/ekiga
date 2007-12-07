
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

#include "avahi-presentity.h"

Avahi::Presentity::Presentity (Ekiga::PresenceCore &_core,
			       std::string _name,
			       std::string _url):
  core(_core), name(_name), url (_url)
{
  online = true;
}

Avahi::Presentity::~Presentity ()
{
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

const std::string
Avahi::Presentity::get_presence () const
{
  if (online)
    return "presence-available";
  else
    return "presence-busy";
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
  return std::set<std::string>();
}

const std::string
Avahi::Presentity::get_uri () const
{
  return url;
}

bool
Avahi::Presentity::populate_menu (Ekiga::MenuBuilder &builder)
{
  return core.populate_presentity_menu (url, builder);
}

void
Avahi::Presentity::set_online (bool val)
{
  online = val;
}

void
Avahi::Presentity::set_status (const std::string _status)
{
  status = _status;
}
