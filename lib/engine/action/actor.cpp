
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
 *                         actor.cpp  -  description
 *                         -------------------------
 *   begin                : written in February 2014 by Damien Sandras
 *   copyright            : (c) 2014 by Damien Sandras
 *   description          : An engine actor.
 *
 */

#include "actor.h"

using namespace Ekiga;


void
Actor::add_action (ActionPtr action)
{
  remove_action (action->get_name ()); // Remove any other action with the same name.

  actions.push_back (action);

  conns.add (action->enabled.connect (boost::bind (boost::ref (action_enabled), action->get_name ())));
  conns.add (action->disabled.connect (boost::bind (boost::ref (action_disabled), action->get_name ())));

  action_added (action->get_name ());
}


void
Actor::add_action (const ActionStore & _actions)
{
  for (ActionStore::const_iterator it = _actions.begin (); it != _actions.end () ; ++it)
    add_action (*it);
}


bool
Actor::remove_action (const std::string & name)
{
  for (ActionStore::iterator it = actions.begin (); it != actions.end () ; ++it) {
    if ((*it)->get_name () == name) {
      action_removed (name);
      actions.erase (it);
      return true;
    }
  }
  return false;
}


bool
Actor::enable_action (const std::string & name)
{
  for (ActionStore::iterator it = actions.begin (); it != actions.end () ; ++it) {
    if ((*it)->get_name () == name) {
      (*it)->enable ();
      return true;
    }
  }
  return false;
}


bool
Actor::disable_action (const std::string & name)
{
  for (ActionStore::iterator it = actions.begin (); it != actions.end () ; ++it) {
    if ((*it)->get_name () == name) {
      (*it)->disable ();
      return true;
    }
  }
  return false;
}


void
Actor::remove_actions ()
{
  for (ActionStore::iterator it = actions.begin (); it != actions.end () ; ++it) {
    action_removed ((*it)->get_name ());
  }
  actions.clear ();
}


Actor::const_iterator
Actor::begin () const
{
  return actions.begin ();
}


Actor::const_iterator
Actor::end () const
{
  return actions.end ();
}


Actor::iterator
Actor::begin ()
{
  return actions.begin ();
}


Actor::iterator
Actor::end ()
{
  return actions.end ();
}
