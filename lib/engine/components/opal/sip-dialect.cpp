
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Implementation of the SIP dialect
 *
 */

#include "sip-dialect.h"
#include "presence-core.h"
#include "personal-details.h"

SIP::Dialect::Dialect (Ekiga::ServiceCore& core,
		       boost::function2<bool, std::string, std::string> sender_):
  presence_core(core.get<Ekiga::PresenceCore> ("presence-core")),
  personal_details(core.get<Ekiga::PersonalDetails> ("personal-details")),
  sender(sender_)
{
}

SIP::Dialect::~Dialect ()
{
}

void
SIP::Dialect::push_message (const std::string uri,
			    const std::string name,
			    const std::string msg)
{
  SimpleChatPtr chat;

  chat = open_chat_with (uri, name, false);

  if (chat)
    chat->receive_message (msg);
}

void
SIP::Dialect::push_notice (const std::string uri,
			   const std::string name,
			   const std::string msg)
{
  SimpleChatPtr chat;

  chat = open_chat_with (uri, name, false);

  chat->receive_notice (msg);
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

boost::shared_ptr<SIP::SimpleChat>
SIP::Dialect::open_chat_with (std::string uri,
			      std::string name,
			      bool user_request)
{
  SimpleChatPtr result;

  for (simple_iterator iter = simple_begin ();
       iter != simple_end ();
       ++iter)
    if ((*iter)->get_uri () == uri)
      result = *iter;

  if ( !result) {

    boost::shared_ptr<Ekiga::PresenceCore> pcore = presence_core.lock ();
    boost::shared_ptr<Ekiga::PersonalDetails> details = personal_details.lock ();
    if (pcore && details) {

      result = SimpleChatPtr (new SimpleChat (pcore, details, name, uri,
					      boost::bind(sender, uri, _1)));
      add_simple_chat (result);
    }
  }

  if (user_request && result)
    result->user_requested ();

  return result;
}
