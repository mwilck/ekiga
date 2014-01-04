
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
 *                         loudmouth-dialect.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of the loudmouth dialect
 *
 */

#include <glib/gi18n.h>

#include "form-request-simple.h"

#include "loudmouth-dialect.h"

LM::Dialect::Dialect (Ekiga::ServiceCore& core_):
  core(core_)
{
}

LM::Dialect::~Dialect ()
{
}

void
LM::Dialect::push_message (PresentityPtr presentity,
			   const std::string msg)
{
  bool found = false;

  for (simple_iterator iter = simple_begin ();
       iter != simple_end ();
       ++iter) {

    if (presentity == (*iter)->get_presentity ()) {

      (*iter)->got_message (msg);
      found = true;
      break;
    }
  }

  if ( !found) {

    SimpleChatPtr chat(new SimpleChat (core, presentity));

    add_simple_chat (chat);
    chat->got_message (msg);
  }
}

struct open_chat_helper
{

  open_chat_helper (Ekiga::PresentityPtr presentity_):
    presentity(presentity_)
  { }

  bool operator() (Ekiga::SimpleChatPtr chat_) const
  {
    LM::SimpleChatPtr chat = boost::dynamic_pointer_cast<LM::SimpleChat> (chat_);
    bool go_on = true;

    if (chat->get_presentity () == presentity) {

      chat->user_requested ();      
      go_on = false;
    }

    return go_on;
  }

  Ekiga::PresentityPtr presentity;
};

void
LM::Dialect::open_chat (PresentityPtr presentity)
{
  if ( !presentity->has_chat) {

    LM::SimpleChatPtr chat(new SimpleChat (core, presentity));
    add_simple_chat (chat);
    chat->user_requested ();
  } else {

    open_chat_helper helper(presentity);
    visit_simple_chats (boost::ref (helper));
  }
}

bool
LM::Dialect::populate_menu (Ekiga::MenuBuilder& builder)
{
  return false; // FIXME: this is here until the feature is ready
  builder.add_action ("group_chat", _("Join a discussion group"),
		      boost::bind (&LM::Dialect::group_chat_action, this));

  return true;
}

void
LM::Dialect::group_chat_action ()
{
  boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&LM::Dialect::on_open_group_chat_submitted, this, _1, _2)));

  request->title (_("Open a group chat room"));

  request->instructions (_("Please provide a room name"));

  request->text ("name", _("Room name"), "", _("The name of the room you want to enter"));

  request->text ("pseudo", _("Pseudonym"), g_get_user_name (), _("The pseudonym you'll have in the room"));

  questions (request);
}

void
LM::Dialect::on_open_group_chat_submitted (bool submitted,
					   Ekiga::Form& result)
{
  if ( !submitted)
    return;

  std::string name = result.text ("name");
  std::string pseudo = result.text ("pseudo");

  std::cout << "Should enter the room '" << name << "' with pseudonym '" << pseudo << "'" << std::endl;
}

void
LM::Dialect::handle_up (LmConnection* /*connection*/,
			const std::string /*name*/)
{
  /* nothing to do afaict */
}

void
LM::Dialect::handle_down (LmConnection* /*connection*/)
{
  // FIXME: here we should find all dead c(h)ats
}

LmHandlerResult
LM::Dialect::handle_iq (LmConnection* /*connection*/,
			LmMessage* /*message*/)
{
  return LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS; // FIXME: implement properly
}

LmHandlerResult
LM::Dialect::handle_message (LmConnection* /*connection*/,
			     LmMessage* /*message*/)
{
  return LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS; // FIXME: implement properly
}

LmHandlerResult
LM::Dialect::handle_presence (LmConnection* /*connection*/,
			      LmMessage* /*message*/)
{
  return LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS; // FIXME: implement properly
}
