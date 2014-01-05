
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
  /* We should never get an iq request from the server, but only
   * answers to what we asked
   */
  return LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS;
}

LmHandlerResult
LM::Dialect::handle_message (LmConnection* /*connection*/,
			     LmMessage* message)
{
  LmHandlerResult result = LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS;

  if (lm_message_get_sub_type (message) == LM_MESSAGE_SUB_TYPE_GROUPCHAT) {

    // FIXME: here we should find the multiple chat which is supposed to receive it, and push it through
  }

  return result;
}

LmHandlerResult
LM::Dialect::handle_presence (LmConnection* /*connection*/,
			      LmMessage* message)
{
  LmHandlerResult result = LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS;

  LmMessageNode* x = lm_message_node_get_child (lm_message_get_node (message), "x");
  if (x) {

    const gchar* xmlns = lm_message_node_get_attribute (x, "xmlns");
    if (xmlns != NULL && std::string(xmlns).find ("http://jabber.org/protocol/muc") == 0) {

      bool found_100 = false;
      bool found_110 = false;
      bool found_210 = false;
      for (LmMessageNode* child = lm_message_get_node (message)->children;
	   child != NULL;
	   child = child->next) {

	if (g_strcmp0 (child->name, "status") == 0) {

	  const gchar* code = lm_message_node_get_attribute (child, "code");
	  if (code != NULL) {

	    if (g_strcmp0 (code, "100") == 0)
	      found_100 = true;
	    if (g_strcmp0 (code, "110") == 0)
	      found_110 = true;
	    if (g_strcmp0 (code, "210") == 0)
	      found_210 = true;
	  }
	}
      }

      /* FIXME: to implement according to those ideas:
       *
       * - if we found a code 110, that means we managed to enter a
       * multiple chat
       *
       * - if we get a code 201, then the room was created on our
       * - behalf and we should configure it
       *
       * - if we found a code 210, then we managed to enter a multiple
       * chat, but the server had to assign us another nick (because
       * of a collision for example)
       *
       * - if we found a code 100, then we are supposed to tell the
       *  user that he entered a non-anynymous room and the full jid
       *  is known to everyone
       *
       * - we should check that the presence comes from a
       * member of one of the existing multiple chats and handle push
       * it to the correct multiple chat
       *
       * - it could also be an error, in which case the message is of
       * subtype 'error', and contains a child 'error', which could
       * have an attribute 'type' set to 'auth', with a child either
       * 'not-authorized' if we lack a (correct) password or
       * 'registration-required' if the room is members-only
       *
       * In any non-erroneous case, the message may contain (in the x
       * node) an 'item' child with 'affiliation', 'role' and
       * (optionally) 'jid' attribute.
       *
       *
       * Perhaps the best organisation is:
       *
       * 0. the dialect should check for the erroneous cases and
       * handle them
       *
       * 1. for normal presence packets, the dialect should just check
       * for the 110 code ;
       *
       * 2. if there's one, create a new multiple chat ;
       *
       * 3. if there's none, find the corresponding multiple chat ;
       *
       * 4. in any case, send the presence message for full treatment
       * to the right multiple chat ; that is let the multiple chat
       * manage the various codes.
       *
       * Notice that the code as currently written already does too
       * much according to this idea.
       */
    }
  }

  return result;
}
