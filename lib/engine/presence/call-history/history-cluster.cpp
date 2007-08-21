
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
 *                         history-cluster.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of the cluster for the call history
 *
 */

#include <iostream>

#include "history-cluster.h"

History::Cluster::Cluster (Ekiga::ServiceCore &_core): core(_core)
{
  presence_core
    = dynamic_cast<Ekiga::PresenceCore*>(core.get ("presence-core"));

  heap = new Heap (core);

  presence_core->presence_received.connect (sigc::mem_fun (this, &History::Cluster::on_presence_received));

  add_heap (*heap);
}

History::Cluster::~Cluster ()
{
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

bool
History::Cluster::is_supported_uri (const std::string uri) const
{
  return presence_core->is_supported_uri (uri);
}

const std::set<std::string>
History::Cluster::existing_groups () const
{
  return heap->existing_groups ();
}

bool
History::Cluster::populate_menu (Ekiga::MenuBuilder &)
{
  /* nothing
   * unless we want the "clear history" action in the addressbook window menu
   */
  return false;
}

void
History::Cluster::on_presence_received (std::string uri,
				       std::string presence)
{
  for (History::Heap::iterator iter = heap->begin ();
       iter != heap->end ();
       iter++)
    if (uri == iter->get_uri ())
      iter->set_presence (presence);
}
