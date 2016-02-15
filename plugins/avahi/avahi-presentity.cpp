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
 *                         avahi-presentity.cpp  -  description
 *                         ------------------------------------
 *   begin                : written in 2015 by Damien Sandras
 *   copyright            : (c) 2015 Damien Sandras
 *   description          : implementation of the avahi presentity
 *
 */

#include "avahi-presentity.h"


boost::shared_ptr<Avahi::Presentity>
Avahi::Presentity::create (boost::shared_ptr<Ekiga::PresenceCore> presence_core,
                           std::string _name,
                           std::string _uri,
                           std::list<std::string> _groups)
{
  return boost::shared_ptr<Avahi::Presentity> (new Avahi::Presentity (presence_core, _name, _uri, _groups));
}

Avahi::Presentity::Presentity (boost::shared_ptr<Ekiga::PresenceCore> presence_core,
                               std::string _name,
                               std::string _uri,
                               std::list<std::string> _groups)
        : name(_name), uri(_uri), presence("unknown"), groups(_groups)
{
  boost::signals2::connection conn;

  conn = presence_core->presence_received.connect (boost::bind (&Avahi::Presentity::on_presence_received, this, _1, _2));
  connections.add (conn);

  conn = presence_core->note_received.connect (boost::bind (&Avahi::Presentity::on_note_received, this, _1, _2));
  connections.add (conn);

  presence_core->pull_actions (*this, _name, _uri);
}

Avahi::Presentity::~Presentity ()
{
}


const std::string
Avahi::Presentity::get_name () const
{
  return name;
}

const std::string
Avahi::Presentity::get_presence () const
{
  return presence;
}

const std::string
Avahi::Presentity::get_note () const
{
  return note;
}

const std::list<std::string>
Avahi::Presentity::get_groups () const
{
  return groups;
}

const std::string
Avahi::Presentity::get_uri () const
{
  return uri;
}

bool
Avahi::Presentity::has_uri (const std::string uri_) const
{
  return uri == uri_;
}

void
Avahi::Presentity::on_presence_received (std::string uri_,
                                         std::string presence_)
{
  if (uri == uri_) {

    presence = presence_;
    updated (this->shared_from_this ());
  }
}

void
Avahi::Presentity::on_note_received (std::string uri_,
                                     std::string note_)
{
  if (uri == uri_) {

    note = note_;
    updated (this->shared_from_this ());
  }
}
