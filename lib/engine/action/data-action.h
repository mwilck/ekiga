
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
 *                         data-action.h  -  description
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

#include <list>

namespace Ekiga {


  /**
   * @defgroup contacts Address Book
   * @{
   */

  /* A DataAction is an action which is available or not depending on
   * the data it should act on.
   *
   * The main difference between an Action and a DataAction is the fact
   * that a DataAction is executed for a given (DataPtr, string) tuple
   * iff the (DataPtr, s) tuple is valid for the given action.
   *
   * Usually, DataPtr will be a boost::shared_ptr< Data >.
   */
  template < class T >
  class DataAction : public Action
  {

  public:
    typedef boost::function2< void, T, const std::string & > Callback;
    typedef boost::function2< bool, T, const std::string & > Tester;
    typedef std::list< Tester > TesterList;


    /** Create an Action given a name, a description, a callback and
     * a validity tester.
     * @param the Action name (please read 'CONVENTION' in action.h).
     * @param the Action description. Can be used as description in menus
     *        implementing Actions.
     * @param the callback to executed when the DataAction is activated by
     *        the user (from a menu or from the code itself) for the
     *        given data.
     * @param the tester checking if the DataAction can be executed for
     *        the given tuple.
     */
    DataAction (const std::string & _name,
                const std::string & _description,
                Callback _callback,
                Tester _tester);


    /** Create an Action given a name, a description, a callback and
     * a validity tester.
     * @param the Action name (please read 'CONVENTION' in action.h).
     * @param the Action description. Can be used as description in menus
     *        implementing Actions.
     * @param the callback to executed when the DataAction is activated by
     *        the user (from a menu or from the code itself) for the
     *        given data.
     * @param a list of testers to check if the DataAction can be executed for
     *        the given tuple.
     */
    DataAction (const std::string & _name,
                const std::string & _description,
                Callback _callback,
                const TesterList & _testers);


    /** Set the (DataPtr, string) tuple on which the DataAction should be run.
     * They must stay valid until the DataAction is activated.
     * The Action is enabled/disabled following the parameters validity.
     * @param the contact part of the tuple.
     * @param the s part of the tuple.
     */
    void set_data (T _t = T (),
                   const std::string & _s = "");


    /** Checks if the DataAction can be run on the (Data, string) tuple given
     * as argument.
     * @param the contact part of the tuple.
     * @param the s part of the tuple.
     * @return true of the action can be run, false otherwise.
     */
    bool can_run_with_data (T _t,
                            const std::string & _s);


  private:

    void on_activated ();

    Callback callback;
    TesterList testers;

    T t;
    std::string s;
  };


  /**
   * @}
   */
};


template <  class T  >
Ekiga::DataAction< T >::DataAction (const std::string & _name,
                                    const std::string & _description,
                                    Callback _callback,
                                    Tester _tester) :
    Action (_name, _description)
{
  callback = _callback;
  testers.push_back (_tester);

  /* DataAction< T > objects should be disabled until data is set */
  set_data ();
}


template < class T >
Ekiga::DataAction< T >::DataAction (const std::string & _name,
                                    const std::string & _description,
                                    Callback _callback,
                                    const TesterList & _testers) :
    Action (_name, _description)
{
  callback = _callback;
  testers = _testers;

  /* DataAction< T > objects should be disabled until data is set */
  set_data ();
}


template < class T >
void
Ekiga::DataAction< T >::set_data (T _t,
                                  const std::string & _s)
{
  if (can_run_with_data (t, s)) {
    t = _t;
    s = _s;
    enable ();
  }
  else {
    t = T ();
    s = "";
    disable ();
  }
}


template < class T >
void
Ekiga::DataAction< T >::on_activated ()
{
  if (is_enabled ())
    callback (t, s);
}


template < class T >
bool
Ekiga::DataAction< T >::can_run_with_data (T _t,
                                           const std::string & _s)
{
  if (!testers.empty ()) {
    for (typename TesterList::const_iterator it = testers.begin ();
         it != testers.end ();
         ++it) {
      if (!(*it) (_t, _s))
        return false;
    }
    return true;
  }
  return false;
}
#endif
