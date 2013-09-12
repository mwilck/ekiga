
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2011 Damien Sandras

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
 *                         loudmouth-chat-multiple.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2011 by Julien Puydt
 *   copyright            : (c) 2011 by Julien Puydt
 *   description          : implementation of a loudmouth multiple chat
 *
 */

#include "loudmouth-chat-multiple.h"

LM::MultipleChat::MultipleChat (Ekiga::ServiceCore& core_,
				LmConnection* connection_):
  core(core_), connection(connection_)
{
}

LM::MultipleChat::~MultipleChat ()
{
}

const std::string
LM::MultipleChat::get_title () const
{
  return "FIXME";
}

void
LM::MultipleChat::connect (boost::shared_ptr<Ekiga::ChatObserver> observer)
{
  observers.push_back (observer);
}

void
LM::MultipleChat::disconnect (boost::shared_ptr<Ekiga::ChatObserver> observer)
{
  observers.remove (observer);
  if (observers.empty ())
    removed ();
}

bool
LM::MultipleChat::send_message (const std::string msg)
{
  bool result = false;

  if (lm_connection_is_authenticated (connection)) {

    result = true;
    LmMessage* message = lm_message_new (NULL, LM_MESSAGE_TYPE_MESSAGE);
    // FIXME: here we should set the destination correctly
    lm_message_node_add_child (lm_message_get_node (message), "body", msg.c_str ());
    lm_connection_send (connection, message, NULL);
    lm_message_unref (message);
    for (std::list<boost::shared_ptr<Ekiga::ChatObserver> >::iterator iter = observers.begin ();
	 iter != observers.end ();
	 ++iter) {

      (*iter)->message (my_name, msg);
    }
  }

  return result;
}

void
LM::MultipleChat::got_message (const std::string who,
			       const std::string msg)
{
  for (std::list<boost::shared_ptr<Ekiga::ChatObserver> >::iterator iter = observers.begin ();
       iter != observers.end ();
       ++iter) {

    (*iter)->message (who, msg);
  }
}

bool
LM::MultipleChat::populate_menu (Ekiga::MenuBuilder& /*builder*/)
{
  return false; // FIXME;
}

Ekiga::HeapPtr
LM::MultipleChat::get_heap () const
{
  return heap;
}
