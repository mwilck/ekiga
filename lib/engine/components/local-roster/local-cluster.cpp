
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
 *                         local-cluster.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of the cluster for the local roster
 *
 */

#include "local-cluster.h"
#include "action.h"

#include <glib/gi18n.h>

Local::Cluster::Cluster (boost::shared_ptr<Ekiga::PresenceCore> pcore):
  presence_core (pcore)
{
}

Local::Cluster::~Cluster ()
{
}

bool
Local::Cluster::is_supported_uri (const std::string & uri) const
{
  boost::shared_ptr<Ekiga::PresenceCore> pcore = presence_core.lock ();

  if (pcore)
    return pcore->is_supported_uri (uri);
  else
    return false;
}

const std::set<std::string>
Local::Cluster::existing_groups () const
{
  return heap->existing_groups ();
}

void
Local::Cluster::set_heap (HeapPtr _heap)
{
  heap = _heap;
  add_heap (heap);
  boost::shared_ptr<Ekiga::PresenceCore> pcore = presence_core.lock ();
  if (pcore) {

    pcore->presence_received.connect (boost::bind (&Local::Cluster::on_presence_received, this, _1, _2));
    pcore->status_received.connect (boost::bind (&Local::Cluster::on_status_received, this, _1, _2));
  }
}

void
Local::Cluster::on_presence_received (std::string uri,
				      std::string presence)
{
  heap->push_presence (uri, presence);
}

void Local::Cluster::on_status_received (std::string uri,
					 std::string status)
{
  heap->push_status (uri, status);
}

void Local::Cluster::register_actions (boost::shared_ptr<Ekiga::ContactCore> contact_core)
{
  Ekiga::TesterList testers;
  testers.push_back (boost::bind (&Local::Cluster::is_supported_uri, this, _2));
  testers.push_back (boost::bind (&Local::Heap::has_no_presentity_with_uri, heap, _2));
  Ekiga::ActionPtr add (new Ekiga::ContactAction ("local-cluster-add", _("Add to Contact List"),
                                                  boost::bind (static_cast<void (Local::Heap::*)(const Ekiga::ContactPtr&, const std::string&)>(&Local::Heap::new_presentity), heap, _1, _2),
                                                  testers));
  contact_core->add_action (add);
}

void Local::Cluster::register_actions ()
{
  /* Add Actor actions */
  add_action (Ekiga::ActionPtr (new Ekiga::Action ("local-cluster-new", _("New Contact"),
                                                   boost::bind (static_cast<void (Local::Heap::*)(const std::string&, const std::string&)>(&Local::Heap::new_presentity), heap, "", ""))));
}
