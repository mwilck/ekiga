
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras

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
 *                         sip-presentity.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of a presentity for SIP chats
 *
 */

#include "sip-presentity.h"

SIP::Presentity::Presentity (Ekiga::ServiceCore &_core,
			     std::string name_,
			     std::string uri_)
  : core(_core), name(name_), uri(uri_), presence("presence-unknown")
{
  presence_core = dynamic_cast<Ekiga::PresenceCore*>(core.get ("presence-core"));

  presence_core->fetch_presence (uri);
}

SIP::Presentity::~Presentity ()
{
  presence_core->unfetch_presence (uri);
}

const std::string
SIP::Presentity::get_name () const
{
  return name;
}

const std::string
SIP::Presentity::get_presence () const
{
  return presence;
}

const std::string
SIP::Presentity::get_status () const
{
  return status;
}

const std::string
SIP::Presentity::get_avatar () const
{
  return avatar;
}

const std::set<std::string>
SIP::Presentity::get_groups () const
{
  return groups;
}

const std::string
SIP::Presentity::get_uri () const
{
  return uri;
}

void
SIP::Presentity::set_presence (const std::string _presence)
{
  presence = _presence;
  updated.emit ();
}

void
SIP::Presentity::set_status (const std::string _status)
{
  status = _status;
  updated.emit ();
}

bool
SIP::Presentity::populate_menu (Ekiga::MenuBuilder &builder)
{
  bool result = false;

  result = presence_core->populate_presentity_menu (uri, builder);

  return result;
}
