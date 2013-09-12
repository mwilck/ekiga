
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras

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
 *                         loudmouth-chat-simple.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of a loudmouth simple chat
 *
 */

#include "loudmouth-chat-simple.h"

#include "personal-details.h"

LM::SimpleChat::SimpleChat (Ekiga::ServiceCore& core_,
			    PresentityPtr presentity_):
  core(core_), presentity(presentity_)
{
  presentity->has_chat = true;
}

LM::SimpleChat::~SimpleChat ()
{
  presentity->has_chat = false;
}

const std::string
LM::SimpleChat::get_title () const
{
  return presentity->get_name ();
}

void
LM::SimpleChat::connect (boost::shared_ptr<Ekiga::ChatObserver> observer)
{
  observers.push_back (observer);
}

void
LM::SimpleChat::disconnect (boost::shared_ptr<Ekiga::ChatObserver> observer)
{
  observers.remove (observer);
  if (observers.empty ())
    removed ();
}

bool
LM::SimpleChat::send_message (const std::string msg)
{
  bool result = false;

  if (lm_connection_is_authenticated (presentity->get_connection ())) {

    result = true;
    boost::shared_ptr<Ekiga::PersonalDetails> details = core.get<Ekiga::PersonalDetails> ("personal-details");
    const std::string my_name = details->get_display_name ();
    LmMessage* message = lm_message_new (NULL, LM_MESSAGE_TYPE_MESSAGE);
    lm_message_node_set_attributes (lm_message_get_node (message),
				    "to", presentity->get_jid ().c_str (),
				    "type", "chat",
				    NULL);
    lm_message_node_add_child (lm_message_get_node (message), "body", msg.c_str ());
    lm_connection_send (presentity->get_connection (), message, NULL);
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
LM::SimpleChat::got_message (const std::string msg)
{
  for (std::list<boost::shared_ptr<Ekiga::ChatObserver> >::iterator iter = observers.begin ();
       iter != observers.end ();
       ++iter) {

    (*iter)->message (presentity->get_name (), msg);
  }
}

bool
LM::SimpleChat::populate_menu (Ekiga::MenuBuilder& /*builder*/)
{
  return false; // FIXME;
}

Ekiga::PresentityPtr
LM::SimpleChat::get_presentity () const
{
  return presentity;
}
