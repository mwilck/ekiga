
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
 *                         action.h  -  description
 *                         ------------------------
 *   begin                : written in February 2014 by Damien Sandras
 *   copyright            : (c) 2014 by Damien Sandras
 *   description          : An engine action.
 *
 */

#ifndef __ACTION_H__
#define __ACTION_H__

#include <boost/signals2.hpp>
#include <boost/function.hpp>
#include <boost/smart_ptr.hpp>
#include <map>

namespace Ekiga {

  /**
   * @defgroup actions Action
   * @{
   */


  /* An action is a way for Actor objects to expose functionnality to the UI
   * or to other elements.
   *
   * Actions are usually exposed through the object API.
   *
   * Global actions are exposed through the parent class API. However, derived
   * objects might allow processing more actions than the parent object.
   *
   * The Action object interface allows such derived objects to expose their
   * own specific actions to be globally in such a way that they are usable
   * through the user interface without requiring dynamic casts in the UI code
   * to be able to use the full derived object API.
   *
   * CONVENTION:
   * -----------
   *
   * Usually, specific actions implemented by some specialized derived objects
   * will be defined in the derived object itself. However, generic actions -
   * which are supposed to be implemented by all components - will be defined
   * in the parent class. For example, the "dial" action will be defined
   * in the CallCore itself, but the "make coffee" action will be defined in
   * the CoffeeMachine object implementation.
   *
   * Generic actions are supposed to be named globally: e.g. "call".
   * Specific actions are supposed to be named according the object name:
   * e.g. "coffee-machine-do-coffee".
   */
  class Action
  {
  public:

    /** Create an Action given a name and a description.
     * @param the Action name (please read 'CONVENTION').
     * @param the Action description. Can be used as description in menus
     *        implementing Actions.
     */
    Action (const std::string & _name,
            const std::string & _description);


    /** Create an Action given a name, a description and a callback.
     * @param the Action name (please read 'CONVENTION').
     * @param the Action description. Can be used as description in menus
     *        implementing Actions.
     * @param the callback to executed when the Action is activated by
     *        the user (from a menu or from the code itself).
     */
    Action (const std::string & _name,
            const std::string & _description,
            boost::function0<void> _callback);


    /** Return the Action name.
     * @return the Action name (please read 'CONVENTION').
     */
    const std::string & get_name ();


    /** Return the Action description.
     * @return the Action description.
     */
    const std::string & get_description ();


    /** Activate the Action.
     * This will emit the "activated" signal and trigger the callback
     * execution.
     */
    void activate ();


    /** Enable the Action.
     * This will enable the action. Only enabled actions are usable
     * and appear in menus.
     */
    void enable ();


    /** Disable the Action.
     * This will disable the action. Only enabled actions are usable
     * and appear in menus.
     */
    void disable ();


    /** Return the Action state.
     * @return true if the Action is enabled, false otherwise.
     */
    bool is_enabled ();


  protected:

    std::string name;
    std::string description;
    boost::function0<void> callback;


  private:

    /** Internal callback executed when the Action activated signal is emitted.
     * It basically calls the callback.
     */
    virtual void on_activated ();


    /** This signal is emitted when the Action is activated. This triggers
     * the signal execution.
     */
    boost::signals2::signal<void(void)> activated;

    bool enabled;
  };

  typedef boost::shared_ptr<Action> ActionPtr;
  typedef std::map< std::string, ActionPtr > ActionMap;

  /**
   * @}
   */
}

#endif
