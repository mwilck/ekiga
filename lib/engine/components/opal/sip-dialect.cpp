
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
 *                         sip-dialect.cpp  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2014 by Julien Puydt
 *   description          : Implementation of the SIP dialect
 *
 */

#include "sip-dialect.h"
#include "presence-core.h"
#include "personal-details.h"

SIP::Dialect::Dialect (boost::shared_ptr<Ekiga::PresenceCore> core_,
		       boost::function2<bool, std::string, Ekiga::Message::payload_type> sender_):
  presence_core(core_),
  sender(sender_)
{
}

SIP::Dialect::~Dialect ()
{
}

void
SIP::Dialect::push_message (const std::string uri,
			    const Ekiga::Message& msg)
{
  ConversationPtr conversation;

  conversation = open_chat_with (uri, msg.name, false);

  if (conversation)
    conversation->receive_message (msg);
}

bool
SIP::Dialect::populate_menu (Ekiga::MenuBuilder& /*builder*/)
{
  return false;
}

void
SIP::Dialect::start_chat_with (std::string uri,
			       std::string name)
{
  (void)open_chat_with (uri, name, true);
}

SIP::ConversationPtr
SIP::Dialect::open_chat_with (std::string uri,
			      std::string name,
			      bool user_request)
{
  ConversationPtr result;
  std::string display_name = name;
  boost::shared_ptr<Ekiga::PresenceCore> pcore = presence_core.lock ();

  for (iterator iter = begin ();
       iter != end ();
       ++iter)
    if ((*iter)->get_uri () == uri)
      result = *iter;

  if ( !result && pcore) {

    // FIXME: here find a better display_name
    result = ConversationPtr (new Conversation(pcore,
					       uri,
					       display_name,
					       boost::bind(sender, uri, _1)));
    add_conversation (result);
  }

  if (user_request && result)
    result->user_requested ();

  return result;
}
