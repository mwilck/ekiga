
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2014 Damien Sandras <dsandras@seconix.com>
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
 *                         live-object-menu.h  -  description
 *                         ----------------------------------
 *   begin                : written in 2014 by Damien Sandras
 *   copyright            : (c) 2014 by Damien Sandras
 *   description          : A live object menu definition
 *
 */

#ifndef __LIVE_OBJECT_MENU_H__
#define __LIVE_OBJECT_MENU_H__

#include "action.h"
#include "data-action.h"
#include "null-deleter.h"

#include <gtk/gtk.h>

namespace Ekiga {

  /* ActorMenu
   *
   * We will have one ActorMenu per UI object displaying a Actor.
   *
   * Each Actor object exposes Action objects. Those actions are actions
   * specific to the Actor. They can be triggered, for example, by the
   * various ActorMenu associated with the different views of the
   * Actor.
   *
   * However, even though we can have several ActorMenu for the same
   * Actor (one per UI view), we need to register the associated actions
   * only once.
   */
  class ActorMenu
  {
  public:

    ActorMenu (Actor & obj);
    virtual ~ActorMenu ();


    /** Activate the action of the given name. The first enabled action
     *  (if any) is activated when no action name is given.
     * @param name is the action name, or can be left empty.
     */
    virtual void activate (const std::string & name = "");


    /** Return a pointer to the GMenuModel corresponding to the ActorMenu (if any).
     * @return a pointer to the GMenuModel, NULL if none (no action, or all actions
     *         are disabled).
     */
    GMenuModel *get ();


    /** Return the number of actions of the ActorMenu. The counter is only
     *  updated after get () has been called.
     */
    unsigned size ();

  protected:
    /** Sync GIO actions with Actions */
    virtual void sync_gio_actions ();

    /** Add GIO action with given name */
    virtual void add_gio_action (const std::string & name);

    /** Add GIO action corresponding to the given Action */
    virtual void add_gio_action (ActionPtr a);

    /** Remove GIO action with the given name */
    virtual void remove_gio_action (const std::string & name);

    /** Return the XML representation of the enabled Actions */
    virtual const std::string as_xml (const std::string & id = "");

    /** Remove the XML representation of the full menu with
     * enabled Actions.
     */
    virtual const std::string build ();

    Actor & obj;

  private:
    unsigned n;
    GtkBuilder *builder;
  };

  template < class T >
  class DataActorMenu : public ActorMenu
  {
  public:

    DataActorMenu (DataActor< T > & _obj) : ActorMenu (_obj) {};

    /** Set the (data, string) tuple usable by the DataActorMenu.
     *  Available actions will depend on the data being set.
     * @param the Data part of the tuple.
     * @param the s part of the tuple.
     */
    void set_data (T _t = T (),
                   const std::string & _s = "")
    {
      boost::shared_ptr< DataActor< T > > a = boost::dynamic_pointer_cast< DataActor< T > > (boost::shared_ptr< Actor > (&obj, null_deleter2 ()));
      if (a) a->set_data (_t, _s);
      sync_gio_actions ();
    };
  };
  typedef boost::shared_ptr<ActorMenu> ActorMenuPtr;
}
#endif
