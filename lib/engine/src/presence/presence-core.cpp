
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
 *                         presence-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of the main
 *                          presentity managing object
 *
 */

#include <iostream>

#include "presence-core.h"

Ekiga::PresenceCore::~PresenceCore ()
{
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

void
Ekiga::PresenceCore::add_cluster (Cluster &cluster)
{
  clusters.push_front (&cluster);
  cluster_added.emit (cluster);
}

void
Ekiga::PresenceCore::visit_clusters (sigc::slot<void, Cluster &> visitor)
{
  for (std::list<Cluster *>::iterator iter = clusters.begin ();
       iter != clusters.end ();
       iter++)
    visitor (*(*iter));
}

bool
Ekiga::PresenceCore::populate_menu (MenuBuilder &/*builder*/)
{
  // FIXME: to implement
}

void
Ekiga::PresenceCore::add_presentity_decorator (PresentityDecorator &decorator)
{
  presentity_decorators.push_front (&decorator);
}

bool
Ekiga::PresenceCore::populate_presentity_menu (const std::string uri,
					       MenuBuilder &builder)
{
  bool populated = false;

  for (std::list<PresentityDecorator *>::const_iterator iter
	 = presentity_decorators.begin ();
       iter != presentity_decorators.end ();
       iter++) {

    if ((*iter)->populate_menu (uri, builder))
      populated = true;
  }
}

void
Ekiga::PresenceCore::add_presence_fetcher (PresenceFetcher &fetcher)
{
  presence_fetchers.push_front (&fetcher);
  fetcher.presence_received.connect (presence_received.make_slot ());
  fetcher.status_received.connect (status_received.make_slot ());
}

void
Ekiga::PresenceCore::fetch_presence (const std::string uri)
{
  for (std::list<PresenceFetcher *>::iterator iter
	 = presence_fetchers.begin ();
       iter != presence_fetchers.end ();
       iter++)
    (*iter)->fetch (uri);
}

void Ekiga::PresenceCore::unfetch_presence (const std::string uri)
{
  for (std::list<PresenceFetcher *>::iterator iter
	 = presence_fetchers.begin ();
       iter != presence_fetchers.end ();
       iter++)
    (*iter)->unfetch (uri);
}

bool
Ekiga::PresenceCore::is_supported_uri (const std::string uri) const
{
  bool result = false;

  for (std::list<sigc::slot<bool, std::string> >::const_iterator iter
	 = uri_testers.begin ();
       iter != uri_testers.end () && result == false;
       iter++)
    result = (*iter) (uri);

  return result;
}

void
Ekiga::PresenceCore::add_supported_uri (sigc::slot<bool,std::string> tester)
{
  uri_testers.push_front (tester);
}
