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
Avahi::Presentity::create (boost::shared_ptr<Ekiga::PresenceCore> _presence_core,
                           std::string _name,
                           std::string _uri,
                           std::list<std::string> _groups)
{
  return boost::shared_ptr<Avahi::Presentity> (new Avahi::Presentity (_presence_core, _name, _uri, _groups));
}


Avahi::Presentity::Presentity (boost::shared_ptr<Ekiga::PresenceCore> _presence_core,
                               std::string _name,
                               std::string _uri,
                               std::list<std::string> _groups) : URIPresentity (_presence_core, _name, _uri, _groups)
{
}


Avahi::Presentity::~Presentity ()
{
}
