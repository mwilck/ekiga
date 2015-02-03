/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2014 Damien Sandras <dsandras@seconix.com>

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
 *                         loudmouth-conversation.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2014 by Julien Puydt
 *   copyright            : (c) 2014 by Julien Puydt
 *   description          : implementation of a loudmouth conversation
 *
 */

#include "loudmouth-conversation.h"

void
LM::Conversation::visit_messages (boost::function1<bool, const Ekiga::Message&> visitor) const
{
  for (std::list<Ekiga::Message>::const_iterator iter = messages.begin ();
       iter != messages.end ();
       ++iter) {

    if (!visitor (*iter))
      break;
  }
}

bool
LM::Conversation::send_message (const Ekiga::Message::payload_type& /*payload*/)
{
  return true; // FIXME: to implement
}

void
LM::Conversation::got_message (const Ekiga::Message::payload_type& /*payload*/)
{
  // FIXME: to implement
}

void
LM::Conversation::reset_unread_messages_count ()
{
  unreads = 0;
  updated ();
}

bool
LM::Conversation::populate_menu (Ekiga::MenuBuilder& /*builder*/)
{
  return false; // FIXME: to implement
}
