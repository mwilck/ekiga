
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
 *                         data-actor.h  -  description
 *                         -------------------------------
 *   begin                : written in March 2014 by Damien Sandras
 *   copyright            : (c) 2014 by Damien Sandras
 *   description          : An engine contact actor.
 *
 */

#ifndef __CONTACT_ACTOR_H__
#define __CONTACT_ACTOR_H__

#include <string>

#include "data-action.h"
#include "actor.h"

namespace Ekiga {

  /**
   * @defgroup actions DataActor
   * @{
   */


  /* An actor is an object able to execute Actions.
   *
   * Actor can register actions through the add_action method.
   * acting.
   */
  template < class T >
  class DataActor : public Actor
  {

  public:
    typedef boost::shared_ptr< DataAction< T > > DataActionPtr;

    /** Register an action on the given DataActor.
     *
     * Actions that are not "added" using this method will not be usable
     * from menus.
     *
     * @param A DataAction.
     */
    void add_action (DataActionPtr action)
    {
      Actor::add_action (action);
      action->set_data (t, s);
    }



    /** Set the (Data, s) tuple on which the DataActions should be run.
     * They must stay valid until the DataAction is activated.
     * Actions are enabled/disabled following the parameters validity.
     * @param the Data part of the tuple.
     * @param the s part of the tuple.
     */
    void set_data (T _t = T (),
                   const std::string & _s = "")
    {
      ActionMap::iterator it;
      t = _t;
      s = _s;

      for (it = actions.begin(); it != actions.end(); ++it) {
        boost::shared_ptr < DataAction < T > > a = boost::dynamic_pointer_cast< DataAction < T > > (it->second);
        if (a)
          a->set_data (t, s);
      }
    }

  private:

    T t;
    std::string s;
  };

  /**
   * @}
   */
}

#endif

