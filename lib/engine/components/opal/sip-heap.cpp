
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2014 Damien Sandras <dsandras@seconix.com>

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
 *                         sip-heap.cpp -  description
 *                         ------------------------------------------
 *   begin                : written in 2014 by Julien Puydt
 *   copyright            : (c) 2014 by Julien Puydt
 *   description          : implementation of an Ekiga::Heap implementation for a SIP::Conversation
 *
 */

#include "sip-heap.h"
#include "uri-presentity.h"


const std::string
SIP::Heap::get_name () const
{
  return ""; // FIXME?
}

const std::string
SIP::Heap::get_name (const std::string uri) const
{
  bool found = false;
  std::string result = uri; // sensible default

  for (const_iterator iter = begin ();
       iter != end () and not found;
       ++iter) {

    boost::shared_ptr<Ekiga::URIPresentity> pres
      = boost::dynamic_pointer_cast<Ekiga::URIPresentity> (*iter);
    if (pres and pres->get_uri () == uri) {

      found = true;
      result = pres->get_name ();
    }
  }
  return result;
}
