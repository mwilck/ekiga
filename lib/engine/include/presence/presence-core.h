
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
 *                         presence-core.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of the main
 *                          presentity managing object
 *
 */

#ifndef __PRESENCE_CORE_H__
#define __PRESENCE_CORE_H__

#include "services.h"
#include "cluster.h"

/* declaration of a few helper classes */
namespace Ekiga
{
  class PresentityDecorator
  {
  public:

    virtual ~PresentityDecorator () {}

    virtual bool populate_menu (const std::string /*uri*/,
                                MenuBuilder &/*builder*/) = 0;
  };

  class PresenceFetcher
  {
  public:

    virtual ~PresenceFetcher () {}

    virtual void fetch (const std::string /*uri*/) = 0;

    virtual void unfetch (const std::string /*uri*/) = 0;

    sigc::signal<void, std::string, std::string> presence_received;
    sigc::signal<void, std::string, std::string> status_received;
  };
};

namespace Ekiga
{

  class PresenceCore:
    public Service
  {
    /* object basics */
  public:

    PresenceCore () {}

    ~PresenceCore ();

    /* service implementation */
  public:
    const std::string get_name () const
    { return "presence-core"; }

    const std::string get_description () const
    { return "\tPresence managing object"; }

    /* api to list presentities */
  public:

    void add_cluster (Cluster &cluster);

    void visit_clusters (sigc::slot<void, Cluster &> visitor);

    sigc::signal<void, Cluster &> cluster_added;

  private:

    std::list<Cluster *> clusters;

    /* act on presentities */
  public:

    void add_presentity_decorator (PresentityDecorator &decorator);

    bool populate_presentity_menu (const std::string uri,
				   MenuBuilder &builder);

  private:

    std::list<PresentityDecorator *> presentity_decorators;

    /* help presentities get presence */
  public:

    void add_presence_fetcher (PresenceFetcher &fetcher);

    void fetch_presence (const std::string uri);

    void unfetch_presence (const std::string uri);

    sigc::signal<void, std::string, std::string> presence_received;
    sigc::signal<void, std::string, std::string> status_received;

  private:

    std::list<PresenceFetcher *> presence_fetchers;

    /* help decide whether an uri is supported by runtime */
  public:

    bool is_supported_uri (const std::string uri) const;

    void add_supported_uri (sigc::slot<bool,std::string> tester);

  private:

    std::list<sigc::slot<bool, std::string> > uri_testers;

    /* unsorted */
  public:

    bool populate_menu (MenuBuilder &builder);

  };

};

#endif
