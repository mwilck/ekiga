
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
 *                         contact-action.h  -  description
 *                         --------------------------------
 *   begin                : written in February 2014 by Damien Sandras
 *   copyright            : (c) 2014 by Damien Sandras
 *   description          : An engine action.
 *
 */

#ifndef __CONTACT_ACTION_H__
#define __CONTACT_ACTION_H__

#include "contact.h"
#include "action.h"

namespace Ekiga {

  /**
   * @defgroup contacts Address Book
   * @{
   */

  /* A ContactAction is an action related to a Contact supporting URIs.
   *
   * The main difference between an Action and a ContactAction is the fact
   * that a ContactAction is executed for a given (Contact, uri) tuple
   * iff the (Contact, uri) tuple is valid for the given action.
   */
  class ContactAction : public Action
  {

  public:
    /** Create an Action given a name, a description, a callback and
     * a validity tester.
     * @param the Action name (please read 'CONVENTION' in action.h).
     * @param the Action description. Can be used as description in menus
     *        implementing Actions.
     * @param the callback to executed when the ContactAction is activated by
     *        the user (from a menu or from the code itself) for the
     *        given Contact and uri.
     * @param the tester checking if the ContactAction can be executed for
     *        the given tuple.
     */
    ContactAction (const std::string & _name,
                   const std::string & _description,
                   boost::function2<void, ContactPtr, std::string> _callback,
                   boost::function2<bool, ContactPtr, std::string> _tester);


    /** Set the (Contact, uri) tuple on which the ContactAction should be run.
     * They must stay valid until the ContactAction is activated.
     * The Action is enabled/disabled following the parameters validity.
     * @param the contact part of the tuple.
     * @param the uri part of the tuple.
     */
    void set_data (ContactPtr _contact = ContactPtr (),
                   const std::string & _uri = "");


    /** Checks if the ContactAction can be run on the (Contact, uri) tuple given
     * as argument.
     * @param the contact part of the tuple.
     * @param the uri part of the tuple.
     * @return true of the action can be run, false otherwise.
     */
    bool can_run_with_data (ContactPtr _contact,
                            const std::string & _uri);


  private:

    void on_activated ();

    boost::function2<void, ContactPtr, std::string> callback;
    boost::function2<bool, ContactPtr, std::string> tester;
    ContactPtr contact;
    std::string uri;
  };

  typedef boost::shared_ptr<ContactAction> ContactActionPtr;

  /**
   * @}
   */
};
#endif
