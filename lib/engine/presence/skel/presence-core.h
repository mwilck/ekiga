
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
 *                          (c) 2008 by Damien Sandras
 *   description          : declaration of the main
 *                          presentity managing object
 *
 */

#ifndef __PRESENCE_CORE_H__
#define __PRESENCE_CORE_H__

#include "services.h"
#include "cluster.h"

/* The presence core has several goals :
 * - one of them is of course to list presentities, and know what happens to
 * them ;
 * - another one is that we may want to store presentities somewhere as dead
 * data, but still be able to gain presence information and actions on them.
 *
 * This is obtained by using three types of helpers :
 * - the abstract class PresentityDecorator, which allows to enable actions on
 * presentities based on uris ;
 * - the abstract class PresenceFetcher, through which it is possible to gain
 * presence information : they allow the PresenceCore to declare some presence
 * information is needed about an uri, or now unneeded ;
 * - finally, a simple callback-based api allows to add detecters for supported
 * uris : this allows for example a Presentity to know if it should declare
 * an uri as "foo@bar" or as "prtcl:foo@bar". FIXME : couldn't a chain of
 * responsibility be used there instead of a special registering magic?
 */


namespace Ekiga
{
  class PresentityDecorator
  {
  public:

    /** The destructor.
     */
    virtual ~PresentityDecorator () {}

    /** Completes the menu for actions available on an uri
     * @param The uri for which actions could be made available.
     * @param A MenuBuilder object to populate.
     */
    virtual bool populate_menu (const std::string /*uri*/,
				MenuBuilder &/*builder*/) = 0;
  };

  class PresenceFetcher
  {
  public:

    /** The destructor.
     */
    virtual ~PresenceFetcher () {}

    /** Triggers presence fetching for the given uri
     * (notice: the PresenceFetcher should count how many times it was
     * requested presence for an uri, in case several presentities share it)
     * @param The uri for which to fetch presence information.
     */
    virtual void fetch (const std::string /*uri*/) = 0;

    /** Stops presence fetching for the given uri
     * (notice that if some other presentity asked for presence information
     * on the same uri, the fetching should go on until the last of them is
     * gone)
     * @param The uri for which to stop fetching presence information.
     */
    virtual void unfetch (const std::string /*uri*/) = 0;

    /** Those signals are emitted whenever this presence fetcher gets
     * presence information about an uri it was required to handle.
     * The information is given as a pair of strings (uri, data).
     */
    sigc::signal<void, std::string, std::string> presence_received;
    sigc::signal<void, std::string, std::string> status_received;
  };

  class PresencePublisher
  {
  public:

    virtual ~PresencePublisher () {}

    virtual void publish (const std::string & /*presence*/,
                          const std::string & /*extended_status*/) = 0;
  };
};

namespace Ekiga
{

  class PresenceCore:
    public Service
  {
  public:

    /** The constructor.
     */
    PresenceCore () {}

    /** The destructor.
     */
    ~PresenceCore ();

    /*** Service Implementation ***/
  public:
    /** Returns the name of the service.
     * @return The service name.
     */
    const std::string get_name () const
    { return "presence-core"; }

    /** Returns the description of the service.
     * @return The service description.
     */
    const std::string get_description () const
    { return "\tPresence managing object"; }

    /*** API to list presentities ***/
  public:

    /** Adds a cluster to the PresenceCore service.
     * @param The cluster to be added.
     */
    void add_cluster (Cluster &cluster);

    /** Triggers a callback for all Ekiga::Cluster clusters of the
     * PresenceCore service.
     * @param The callback.
     */
    void visit_clusters (sigc::slot<void, Cluster &> visitor);

    /** This signal is emitted when an Ekiga::Cluster has been added
     * to the PresenceCore Service.
     */
    sigc::signal<void, Cluster &> cluster_added;

    /** Those signals are forwarding the heap_added, heap_updated
     * and heap_removed from the given Cluster.
     *
     */
    sigc::signal<void, Cluster &, Heap &> heap_added;
    sigc::signal<void, Cluster &, Heap &> heap_updated;
    sigc::signal<void, Cluster &, Heap &> heap_removed;

    /** Those signals are forwarding the presentity_added, presentity_updated
     * and presentity_removed from the given Heap of the given Cluster.
     */
    sigc::signal<void, Cluster &, Heap &, Presentity &> presentity_added;
    sigc::signal<void, Cluster &, Heap &, Presentity &> presentity_updated;
    sigc::signal<void, Cluster &, Heap &, Presentity &> presentity_removed;

  private:

    std::set<Cluster *> clusters;
    void on_heap_added (Heap &heap, Cluster *cluster);
    void on_heap_updated (Heap &heap, Cluster *cluster);
    void on_heap_removed (Heap &heap, Cluster *cluster);
    void on_presentity_added (Heap &heap,
			      Presentity &presentity,
			      Cluster *cluster);
    void on_presentity_updated (Heap &heap,
				Presentity &presentity,
				Cluster *cluster);
    void on_presentity_removed (Heap &heap,
				Presentity &presentity,
				Cluster *cluster);

    /*** API to act on presentities ***/
  public:

    /** Adds a decorator to the pool of presentity decorators.
     * @param The presentity decorator.
     */
    void add_presentity_decorator (PresentityDecorator &decorator);

    /** Populates a menu with the actions available on a given uri.
     * @param The uri for which the decoration is needed.
     * @param The builder to populate.
     */
    bool populate_presentity_menu (const std::string uri,
				   MenuBuilder &builder);

  private:

    std::set<PresentityDecorator *> presentity_decorators;

    /*** API to help presentities get presence ***/
  public:

    /** Adds a fetcher to the pool of presentce fetchers.
     * @param The presence fetcher.
     */
    void add_presence_fetcher (PresenceFetcher &fetcher);

    /** Tells the PresenceCore that someone is interested in presence
     * information for the given uri.
     * @param: The uri for which presence is requested.
     */
    void fetch_presence (const std::string uri);

    /** Tells the PresenceCore that someone becomes uninterested in presence
     * information for the given uri.
     * @param: The uri for which presence isn't requested anymore.
     */
    void unfetch_presence (const std::string uri);

    /** Those signals are emitted whenever information has been received
     * about an uri ; the information is a pair of strings (uri, information).
     */
    sigc::signal<void, std::string, std::string> presence_received;
    sigc::signal<void, std::string, std::string> status_received;

  private:

    std::set<PresenceFetcher *> presence_fetchers;

<<<<<<< HEAD:lib/engine/presence/skel/presence-core.h
    /* help publishing presence */
  public:

    void add_presence_publisher (PresencePublisher &publisher);

    void publish (const std::string & status, 
                  const std::string & extended_status);

  private:

    std::set<PresencePublisher *> presence_publishers;

    /* help decide whether an uri is supported by runtime */
=======
    /*** API to control which uri are supported by runtime ***/
>>>>>>> Commented the presence stack:lib/engine/presence/skel/presence-core.h
  public:

    /** Decides whether an uri is supported by the PresenceCore
     * @param The uri to test for support
     * @return True if the uri is supported
     */
    bool is_supported_uri (const std::string uri) const;

    /** Adds an uri tester to the PresenceCore
     * @param The tester
     */
    void add_supported_uri (sigc::slot<bool,std::string> tester);

  private:

    std::set<sigc::slot<bool, std::string> > uri_testers;

    /*** Misc ***/
  public:

    /** Create the menu of the actions available in the PresenceCore.
     * @param A MenuBuilder object to populate.
     */
    bool populate_menu (MenuBuilder &builder);

    /** This chain allows the PresenceCore to present forms to the user
     */
    ChainOfResponsibility<FormRequest*> questions;

  };

};

#endif
