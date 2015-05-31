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
 *                         uri-presentity.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of a presentity around a simple URI
 *
 */

#include "uri-presentity.h"

/* at one point we will return a smart pointer on this... and if we don't use
 * a false smart pointer, we will crash : the reference count isn't embedded!
 */
struct null_deleter
{
    void operator()(void const *) const
    {
    }
};

Ekiga::URIPresentity::URIPresentity (boost::shared_ptr<Ekiga::PresenceCore> pcore,
				     std::string name_,
				     std::string uri_,
				     std::list<std::string> groups_):
  presence_core(pcore), name(name_), uri(uri_), presence("unknown"), groups(groups_)
{
  boost::signals2::connection conn;

  conn = pcore->presence_received.connect (boost::bind (&Ekiga::URIPresentity::on_presence_received, this, _1, _2));
  connections.add (conn);

  conn = pcore->status_received.connect (boost::bind (&Ekiga::URIPresentity::on_status_received, this, _1, _2));
  connections.add (conn);

  pcore->fetch_presence (uri);
}

Ekiga::URIPresentity::~URIPresentity ()
{
  boost::shared_ptr<Ekiga::PresenceCore> pcore = presence_core.lock ();
  if (pcore)
    pcore->unfetch_presence (uri);
}

const std::string
Ekiga::URIPresentity::get_name () const
{
  return name;
}

const std::string
Ekiga::URIPresentity::get_presence () const
{
  return presence;
}

const std::string
Ekiga::URIPresentity::get_status () const
{
  return status;
}

const std::list<std::string>
Ekiga::URIPresentity::get_groups () const
{
  return groups;
}

const std::string
Ekiga::URIPresentity::get_uri () const
{
  return uri;
}

bool
Ekiga::URIPresentity::has_uri (const std::string uri_) const
{
  return uri == uri_;
}

void
Ekiga::URIPresentity::on_presence_received (std::string uri_,
					    std::string presence_)
{
  if (uri == uri_) {

    presence = presence_;
    //updated ();
    std::cout << "FIXME: Updated" << std::endl;
  }
}

void
Ekiga::URIPresentity::on_status_received (std::string uri_,
					  std::string status_)
{
  if (uri == uri_) {

    status = status_;
    //updated ();
    std::cout << "FIXME: Updated" << std::endl;
  }
}
