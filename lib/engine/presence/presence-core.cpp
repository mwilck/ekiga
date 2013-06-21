
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
 *                         presence-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *                          (c) 2008 by Damien Sandras
 *   description          : implementation of the main
 *                          presentity managing object
 *
 */

#include "presence-core.h"
#include "personal-details.h"


Ekiga::PresenceCore::PresenceCore ( boost::shared_ptr<Ekiga::PersonalDetails> _details): details(_details)
{
  conns.add (details->updated.connect (boost::bind (boost::bind (&Ekiga::PresenceCore::publish, this, _1), details)));
}

void
Ekiga::PresenceCore::add_cluster (ClusterPtr cluster)
{
  clusters.insert (cluster);
  cluster_added (cluster);
  conns.add (cluster->updated.connect (boost::ref (updated)));
  conns.add (cluster->heap_added.connect (boost::bind (&Ekiga::PresenceCore::on_heap_added, this, _1, cluster)));
  conns.add (cluster->heap_updated.connect (boost::bind (&Ekiga::PresenceCore::on_heap_updated, this, _1, cluster)));
  conns.add (cluster->heap_removed.connect (boost::bind (&Ekiga::PresenceCore::on_heap_removed, this, _1, cluster)));
  conns.add (cluster->presentity_added.connect (boost::bind (&Ekiga::PresenceCore::on_presentity_added, this, _1, _2, cluster)));
  conns.add (cluster->presentity_updated.connect (boost::bind (&Ekiga::PresenceCore::on_presentity_updated, this, _1, _2, cluster)));
  conns.add (cluster->presentity_removed.connect (boost::bind (&Ekiga::PresenceCore::on_presentity_removed, this, _1, _2, cluster)));
  cluster->questions.connect (boost::ref (questions));

  updated ();
}

void
Ekiga::PresenceCore::visit_clusters (boost::function1<bool, ClusterPtr > visitor) const
{
  bool go_on = true;
  for (std::set<ClusterPtr >::const_iterator iter = clusters.begin ();
       iter != clusters.end () && go_on;
       iter++)
    go_on = visitor (*iter);
}

bool
Ekiga::PresenceCore::populate_menu (MenuBuilder &builder)
{
  bool populated = false;

  for (std::set<ClusterPtr >::iterator iter = clusters.begin ();
       iter != clusters.end ();
       ++iter)
    if ((*iter)->populate_menu (builder))
      populated = true;

  return populated;
}

void Ekiga::PresenceCore::on_heap_added (HeapPtr heap,
					 ClusterPtr cluster)
{
  heap_added (cluster, heap);
}

void
Ekiga::PresenceCore::on_heap_updated (HeapPtr heap,
				      ClusterPtr cluster)
{
  heap_updated (cluster, heap);
}

void
Ekiga::PresenceCore::on_heap_removed (HeapPtr heap, ClusterPtr cluster)
{
  heap_removed (cluster, heap);
}

void
Ekiga::PresenceCore::on_presentity_added (HeapPtr heap,
					  PresentityPtr presentity,
					  ClusterPtr cluster)
{
  presentity_added (cluster, heap, presentity);
}

void
Ekiga::PresenceCore::on_presentity_updated (HeapPtr heap,
					    PresentityPtr presentity,
					    ClusterPtr cluster)
{
  presentity_updated (cluster, heap, presentity);
}

void
Ekiga::PresenceCore::on_presentity_removed (HeapPtr heap,
					    PresentityPtr presentity,
					    ClusterPtr cluster)
{
  presentity_removed (cluster, heap, presentity);
}

void
Ekiga::PresenceCore::add_presentity_decorator (boost::shared_ptr<PresentityDecorator> decorator)
{
  presentity_decorators.push_back (decorator);
}

bool
Ekiga::PresenceCore::populate_presentity_menu (PresentityPtr presentity,
					       const std::string uri,
					       MenuBuilder &builder)
{
  bool populated = false;

  for (std::list<boost::shared_ptr<PresentityDecorator> >::const_iterator iter
	 = presentity_decorators.begin ();
       iter != presentity_decorators.end ();
       ++iter) {

    populated = (*iter)->populate_menu (presentity, uri, builder) || populated;
  }

  return populated;
}

void
Ekiga::PresenceCore::add_presence_fetcher (boost::shared_ptr<PresenceFetcher> fetcher)
{
  presence_fetchers.push_back (fetcher);
  conns.add (fetcher->presence_received.connect (boost::bind (&Ekiga::PresenceCore::on_presence_received, this, _1, _2)));
  conns.add (fetcher->status_received.connect (boost::bind (&Ekiga::PresenceCore::on_status_received, this, _1, _2)));
  for (std::map<std::string, uri_info>::const_iterator iter
	 = uri_infos.begin ();
       iter != uri_infos.end ();
       ++iter)
    fetcher->fetch (iter->first);
}

void
Ekiga::PresenceCore::fetch_presence (const std::string uri)
{
  uri_infos[uri].count++;

  if (uri_infos[uri].count == 1) {

    for (std::list<boost::shared_ptr<PresenceFetcher> >::iterator iter
	   = presence_fetchers.begin ();
	 iter != presence_fetchers.end ();
	 ++iter)
      (*iter)->fetch (uri);
  }

  presence_received (uri, uri_infos[uri].presence);
  status_received (uri, uri_infos[uri].status);
}

void Ekiga::PresenceCore::unfetch_presence (const std::string uri)
{
  uri_infos[uri].count--;

  if (uri_infos[uri].count <= 0) {

    uri_infos.erase (uri_infos.find (uri));

    for (std::list<boost::shared_ptr<PresenceFetcher> >::iterator iter
	   = presence_fetchers.begin ();
	 iter != presence_fetchers.end ();
	 ++iter)
      (*iter)->unfetch (uri);
  }
}

void
Ekiga::PresenceCore::on_presence_received (const std::string uri,
					   const std::string presence)
{
  uri_infos[uri].presence = presence;
  presence_received (uri, presence);
}

void
Ekiga::PresenceCore::on_status_received (const std::string uri,
					 const std::string status)
{
  uri_infos[uri].status = status;
  status_received (uri, status);
}

void
Ekiga::PresenceCore::add_presence_publisher (boost::shared_ptr<PresencePublisher> publisher)
{
  presence_publishers.push_back (publisher);
}

void Ekiga::PresenceCore::publish (boost::shared_ptr<PersonalDetails> details) 
{
  for (std::list<boost::shared_ptr<PresencePublisher> >::iterator iter
	 = presence_publishers.begin ();
       iter != presence_publishers.end ();
       ++iter)
    (*iter)->publish (*details);
}

bool
Ekiga::PresenceCore::is_supported_uri (const std::string uri) const
{
  bool result = false;

  for (std::list<boost::function1<bool, std::string> >::const_iterator iter
	 = uri_testers.begin ();
       iter != uri_testers.end () && result == false;
       iter++)
    result = (*iter) (uri);

  return result;
}

void
Ekiga::PresenceCore::add_supported_uri (boost::function1<bool,std::string> tester)
{
  uri_testers.push_back (tester);
}
