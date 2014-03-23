
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
 *                         contact-actor.h  -  description
 *                         -------------------------------
 *   begin                : written in March 2014 by Damien Sandras
 *   copyright            : (c) 2014 by Damien Sandras
 *   description          : An engine contact actor.
 *
 */

#ifndef __CONTACT_ACTOR_H__
#define __CONTACT_ACTOR_H__

#include <string>

#include "contact-action.h"
#include "actor.h"

namespace Ekiga {

  /**
   * @defgroup actions ContactActor
   * @{
   */


  /* An actor is an object able to execute Actions.
   *
   * Actor can register actions through the add_action method.
   * acting.
   */
  class ContactActor : public Actor
  {
    friend class ContactActorMenu;

  public:

    /** Register an action on the given ContactActor.
     *
     * Actions that are not "added" using this method will not be usable
     * from menus.
     *
     * @param A ContactAction.
     */
    void add_action (ActionPtr action);


    /** Set the (Contact, uri) tuple on which the ContactActions should be run.
     * They must stay valid until the ContactAction is activated.
     * Actions are enabled/disabled following the parameters validity.
     * @param the contact part of the tuple.
     * @param the uri part of the tuple.
     */
    void set_data (ContactPtr _contact = ContactPtr (),
                   const std::string & _uri = "");


  private:

    ContactPtr contact;
    std::string uri;
  };

  /**
   * @}
   */
}

#endif

