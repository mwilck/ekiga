
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
 *                         actor.h  -  description
 *                         -----------------------
 *   begin                : written in February 2014 by Damien Sandras
 *   copyright            : (c) 2014 by Damien Sandras
 *   description          : An engine actor.
 *
 */

#ifndef __ACTOR_H__
#define __ACTOR_H__

#include "action.h"

namespace Ekiga {

  /**
   * @defgroup actions Actor
   * @{
   */


  /* An actor is an object able to execute Actions.
   *
   * Actor can register actions through the add_action method.
   * acting.
   */
  class Actor
  {
    friend class ActorMenu;
    friend class ContactActorMenu;

  public:

    /** Register an action on the given Actor.
     *
     * Actions that are not "added" using this method will not be usable
     * from menus.
     *
     * @param An Action.
     */
    void add_action (ActionPtr action);


  protected:

    /** This method must be called by each Actor to register Actions.
     */
    virtual void register_actions ();

    ActionMap actions;
  };

  /**
   * @}
   */
}

#endif

