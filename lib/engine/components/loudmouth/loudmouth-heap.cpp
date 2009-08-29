
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
 *                         loudmouth-heap.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of a loudmouth heap
 *
 */

#include <string.h>
#include <glib/gi18n.h>

#include "form-request-simple.h"

#include "loudmouth-heap.h"

static LmHandlerResult
iq_handler_c (LmMessageHandler* /*handler*/,
	      LmConnection* /*connection*/,
	      LmMessage* message,
	      LM::Heap* heap)
{
  return heap->iq_handler (message);
}

static LmHandlerResult
presence_handler_c (LmMessageHandler* /*handler*/,
		    LmConnection* /*connection*/,
		    LmMessage* message,
		    LM::Heap* heap)
{
  return heap->presence_handler (message);
}

static LmHandlerResult
message_handler_c (LmMessageHandler* /*handler*/,
		   LmConnection* /*connection*/,
		   LmMessage* message,
		   LM::Heap* heap)
{
  return heap->message_handler (message);
}

LM::Heap::Heap (boost::shared_ptr<Ekiga::PersonalDetails> details_,
		DialectPtr dialect_,
		LmConnection* connection_):
  details(details_), dialect(dialect_), connection(connection_)
{
  details->updated.connect (boost::bind (&LM::Heap::on_personal_details_updated, this));

  lm_connection_ref (connection);

  iq_lm_handler = lm_message_handler_new ((LmHandleMessageFunction)iq_handler_c, this, NULL);
  lm_connection_register_message_handler (connection, iq_lm_handler, LM_MESSAGE_TYPE_IQ, LM_HANDLER_PRIORITY_NORMAL);

  presence_lm_handler = lm_message_handler_new ((LmHandleMessageFunction)presence_handler_c, this, NULL);
  lm_connection_register_message_handler (connection, presence_lm_handler, LM_MESSAGE_TYPE_PRESENCE, LM_HANDLER_PRIORITY_NORMAL);

  message_lm_handler = lm_message_handler_new ((LmHandleMessageFunction)message_handler_c, this, NULL);
  lm_connection_register_message_handler (connection, message_lm_handler, LM_MESSAGE_TYPE_MESSAGE, LM_HANDLER_PRIORITY_NORMAL);

  { // populate the roster
    LmMessage* roster_request = lm_message_new_with_sub_type (NULL, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_GET);
    LmMessageNode* node = lm_message_node_add_child (lm_message_get_node (roster_request), "query", NULL);
    lm_message_node_set_attributes (node, "xmlns", "jabber:iq:roster", NULL);
    lm_connection_send (connection, roster_request, NULL);
    lm_message_unref (roster_request);
  }
  { // initial presence push
    LmMessage* presence_push = lm_message_new (NULL, LM_MESSAGE_TYPE_PRESENCE);
    lm_connection_send (connection, presence_push, NULL);
    lm_message_unref (presence_push);
  }

  on_personal_details_updated (); // fake, but if we start as dnd, we want it known
}

LM::Heap::~Heap ()
{
  lm_connection_unregister_message_handler (connection, iq_lm_handler, LM_MESSAGE_TYPE_IQ);
  lm_message_handler_unref (iq_lm_handler);
  iq_lm_handler = 0;

  lm_connection_unregister_message_handler (connection, presence_lm_handler, LM_MESSAGE_TYPE_PRESENCE);
  lm_message_handler_unref (presence_lm_handler);
  presence_lm_handler = 0;

  lm_connection_unregister_message_handler (connection, message_lm_handler, LM_MESSAGE_TYPE_MESSAGE);
  lm_message_handler_unref (message_lm_handler);
  message_lm_handler = 0;

  lm_connection_unref (connection);
  connection = 0;
}

const std::string
LM::Heap::get_name () const
{
  return name;
}

void
LM::Heap::set_name (const std::string name_)
{
  name = name_;
  updated ();
}

bool
LM::Heap::populate_menu (Ekiga::MenuBuilder& builder)
{
  builder.add_action ("new", _("New _Contact"), boost::bind (&LM::Heap::add_item, this));
  return true;
}

bool
LM::Heap::populate_menu_for_group (const std::string /*group*/,
				   Ekiga::MenuBuilder& /*builder*/)
{
  return false;
}


void
LM::Heap::disconnected ()
{
  removed ();
}

LmHandlerResult
LM::Heap::iq_handler (LmMessage* message)
{
  if (lm_message_get_sub_type (message) == LM_MESSAGE_SUB_TYPE_SET
      || lm_message_get_sub_type (message) == LM_MESSAGE_SUB_TYPE_RESULT) {

    LmMessageNode* node = lm_message_node_get_child (lm_message_get_node (message), "query");
    if (node != NULL) {

      const gchar* xmlns = lm_message_node_get_attribute (node, "xmlns");
      if (xmlns != NULL && strcmp (xmlns, "jabber:iq:roster") == 0) {

	parse_roster (node);
      }
    }
  }

  return LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS;
}


LmHandlerResult
LM::Heap::presence_handler (LmMessage* message)
{
  const gchar* from_c = lm_message_node_get_attribute (lm_message_get_node (message), "from");
  const gchar* type_attr = lm_message_node_get_attribute (lm_message_get_node (message), "type");
  std::string base_jid;
  std::string resource;

  if (from_c != 0) {

    std::string from (from_c);
    std::string::size_type index = from.find ('/');
    base_jid = std::string (from, 0, index);
    resource = std::string (from, index + 1, std::string::npos);
  }

  PresentityPtr item = find_item (base_jid);

  if (type_attr != NULL && strcmp (type_attr, "subscribe") == 0) {

    boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&LM::Heap::subscribe_from_form_submitted, this)));
    LmMessageNode* status = lm_message_node_find_child (lm_message_get_node (message), "status");
    gchar* instructions = NULL;
    std::string item_name;

    if (item) {

      item_name = item->get_name ();
    } else {

      item_name = base_jid;
    }

    request->title (_("Authorization to see your presence"));

    if (status != NULL && lm_message_node_get_value (status) != NULL) {

      instructions = g_strdup_printf (_("%s asks the permission to see your presence, saying: \"%s\"."),
				      item_name.c_str (), lm_message_node_get_value (status));
    } else {

      instructions = g_strdup_printf (_("%s asks the permission to see your presence."),
				      item_name.c_str ());
    }
    request->instructions (instructions);
    g_free (instructions);

    std::map<std::string, std::string> choices;
    choices["grant"] = _("grant him/her the permission to see your presence");
    choices["refuse"] = _("refuse him/her the permission to see your presence");
    choices["later"] = _("decide later (also close or cancel this dialog)");
    request->single_choice ("answer", _("Your answer is: "), "grant", choices);

    request->hidden ("jid", base_jid);

    questions (request);
  } else {

    if (item) {

      item->push_presence (resource, lm_message_get_node (message));
    }
  }

  return LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS;
}

LmHandlerResult
LM::Heap::message_handler (LmMessage* message)
{
  const gchar* from_c = lm_message_node_get_attribute (lm_message_get_node (message), "from");
  const gchar* type_attr = lm_message_node_get_attribute (lm_message_get_node (message), "type");
  std::string base_jid;

  if (from_c != 0) {

    std::string from (from_c);
    std::string::size_type index = from.find ('/');
    base_jid = std::string (from, 0, index);
  }

  PresentityPtr item = find_item (base_jid);

  if (type_attr == NULL
      || (type_attr != NULL && strcmp (type_attr, "normal") == 0)
      || (type_attr != NULL && strcmp (type_attr, "chat") == 0)) {

    LmMessageNode* body = lm_message_node_find_child (lm_message_get_node (message), "body");
    if (lm_message_node_get_value (body) != NULL) {

      dialect->push_message (item, lm_message_node_get_value (body));
    }
  }

  return LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS;
}

void
LM::Heap::parse_roster (LmMessageNode* query)
{
  for (LmMessageNode* node = query->children; node != NULL; node = node->next) {

    if (strcmp (node->name, "item") != 0) {

      continue;
    }

    const gchar* jid = lm_message_node_get_attribute (node, "jid");
    bool found = false;
    for (iterator iter = begin (); !found && iter != end (); ++iter) {

      if ((*iter)->get_jid () == jid) {

	found = true;
	const gchar* subscription = lm_message_node_get_attribute (node, "subscription");
	if (subscription != NULL && strcmp (subscription, "remove") == 0) {

	  (*iter)->removed ();
	} else {

	  (*iter)->update (node);
	}
      }
    }
    if ( !found) {

      PresentityPtr presentity(new Presentity (connection, node));
      presentity->chat_requested.connect (boost::bind (boost::bind (&LM::Heap::on_chat_requested, this), presentity));
      add_presentity (presentity);
    }
  }
}

void
LM::Heap::add_item ()
{
  boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&LM::Heap::add_item_form_submitted, this)));

  request->title (_("Add a roster element"));
  request->instructions (_("Please fill in this form to add a new"
			   "element to the remote roster"));
  request->text ("jid", _("Identifier:"), _("identifier@server"));

  questions (request);
}

void
LM::Heap::add_item_form_submitted (bool submitted,
				   Ekiga::Form& result)
{
  if ( !submitted)
    return;

  const std::string jid = result.text ("jid");
  if ( !jid.empty ()) {

    LmMessage* subscribe = lm_message_new (NULL, LM_MESSAGE_TYPE_PRESENCE);
    lm_message_node_set_attributes (lm_message_get_node (subscribe),
				    "to", jid.c_str (),
				    "type", "subscribe",
				    NULL);
    lm_connection_send (connection, subscribe, NULL);
    lm_message_unref (subscribe);
  }
}

void
LM::Heap::subscribe_from_form_submitted (bool submitted,
					 Ekiga::Form& result)
{
  if ( !submitted)
    return;

  const std::string jid = result.hidden ("jid");
  const std::string answer = result.single_choice ("answer");

  if (answer == "grant") {

    LmMessage* message = lm_message_new (NULL, LM_MESSAGE_TYPE_PRESENCE);
    lm_message_node_set_attributes (lm_message_get_node (message),
				    "to", jid.c_str (),
				    "type", "subscribed",
				    NULL);
    lm_connection_send (connection, message, NULL);
    lm_message_unref (message);
    LmMessage* subscribe = lm_message_new (NULL, LM_MESSAGE_TYPE_PRESENCE);
    lm_message_node_set_attributes (lm_message_get_node (subscribe),
				    "to", jid.c_str (),
				    "type", "subscribe",
				    NULL);
    lm_connection_send (connection, subscribe, NULL);
    lm_message_unref (subscribe);
  } else if (answer == "refuse") {

    LmMessage* message = lm_message_new (NULL, LM_MESSAGE_TYPE_PRESENCE);
    lm_message_node_set_attributes (lm_message_get_node (message),
				    "to", jid.c_str (),
				    "type", "unsubscribed",
				    NULL);
    lm_connection_send (connection, message, NULL);
    lm_message_unref (message);
  }
}

LM::PresentityPtr
LM::Heap::find_item (const std::string jid)
{
  PresentityPtr result;

  for (iterator iter = begin (); iter != end (); ++iter) {

    if ((*iter)->get_jid () == jid) {

      result = *iter;
      break;
    }
  }

  return result;
}

void
LM::Heap::on_personal_details_updated ()
{
  LmMessage* message = lm_message_new (NULL, LM_MESSAGE_TYPE_PRESENCE);

  lm_message_node_add_child (lm_message_get_node (message), "show", details->get_presence ().c_str ());
  lm_message_node_add_child (lm_message_get_node (message), "status", details->get_status ().c_str ());

  lm_connection_send (connection, message, NULL);
  lm_message_unref (message);
}

void
LM::Heap::on_chat_requested (PresentityPtr presentity)
{
  dialect->open_chat (presentity);
}
