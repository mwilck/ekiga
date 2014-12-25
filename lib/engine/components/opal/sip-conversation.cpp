
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
 *                         sip-conversation.cpp -  description
 *                         ------------------------------------------
 *   begin                : written in 2014 by Julien Puydt
 *   copyright            : (c) 2014 by Julien Puydt
 *   description          : implementation of a SIP conversation
 *
 */

#include "sip-conversation.h"
#include "uri-presentity.h"

SIP::Conversation::Conversation (boost::shared_ptr<Ekiga::PresenceCore> _core,
				 const std::string _uri,
				 const std::string _name,
				 boost::function1<bool, const Ekiga::Message::payload_type&> _sender):
  presence_core(_core), uri(_uri), title(_name), sender(_sender), unreads(0)
{
  // FIXME: this api isn't good: we obviously don't handle correctly Conversation with several people!
  boost::shared_ptr<Ekiga::URIPresentity> presentity =
    boost::shared_ptr<Ekiga::URIPresentity> (new Ekiga::URIPresentity (_core,
                                                                       title,
                                                                       uri,
                                                                       std::list<std::string>
                                                                       ()));
  heap  = boost::shared_ptr<Heap> (new Heap);
  heap->add_presentity (boost::dynamic_pointer_cast<Ekiga::Presentity> (presentity));
}

void
SIP::Conversation::visit_messages (boost::function1<bool, const Ekiga::Message&> visitor) const
{
  for (std::list<Ekiga::Message>::const_iterator iter = messages.begin();
       iter != messages.end ();
       ++iter) {

    if (!visitor(*iter))
      break;
  }
}

bool
SIP::Conversation::send_message (const Ekiga::Message::payload_type& payload)
{
  return sender (payload);
}

void
SIP::Conversation::reset_unread_messages_count ()
{
  unreads = 0;
  updated ();
}

void
SIP::Conversation::receive_message (const Ekiga::Message& message)
{
  // if the endpoint doesn't put a real name, we'll try to find one
  Ekiga::Message msg = { message.time,
			 heap->get_name (message.name),
			 message.payload };
  messages.push_back (msg);
  message_received (msg);
  unreads++;
  updated();
}
