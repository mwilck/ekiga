
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras

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
 *                         gmconf-cluster.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of the cluster for the gmconf roster
 *
 */

#include <iostream>

#include "gmconf-cluster.h"

GMConf::Cluster::Cluster (Ekiga::ServiceCore &_core): core(_core)
{
  presence_core
    = dynamic_cast<Ekiga::PresenceCore*>(core.get ("presence-core"));

  heap = new Heap (core);

  presence_core->presence_received.connect (sigc::mem_fun (this, &GMConf::Cluster::on_presence_received));
  presence_core->status_received.connect (sigc::mem_fun (this, &GMConf::Cluster::on_status_received));

  add_heap (*heap);
}

GMConf::Cluster::~Cluster ()
{
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

bool
GMConf::Cluster::is_supported_uri (const std::string uri) const
{
  return presence_core->is_supported_uri (uri);
}

const std::list<std::string>
GMConf::Cluster::existing_groups () const
{
  return heap->existing_groups ();
}

void
GMConf::Cluster::populate_menu (Ekiga::MenuBuilder &)
{
  // FIXME to implement
}

void
GMConf::Cluster::on_presence_received (std::string uri,
				       std::string presence)
{
  for (GMConf::Heap::iterator iter = heap->begin ();
       iter != heap->end ();
       iter++)
    if (uri == iter->get_uri ())
      iter->set_presence (presence);
}

void GMConf::Cluster::on_status_received (std::string uri,
					  std::string status)
{
  for (GMConf::Heap::iterator iter = heap->begin ();
       iter != heap->end ();
       iter++)
    if (uri == iter->get_uri ())
      iter->set_status (status);
}
