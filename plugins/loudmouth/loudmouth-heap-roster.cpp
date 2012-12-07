
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

#include "loudmouth-helpers.h"

#include "loudmouth-heap-roster.h"

LM::HeapRoster::HeapRoster (boost::shared_ptr<Ekiga::PersonalDetails> details_,
			    DialectPtr dialect_):
  details(details_), dialect(dialect_)
{
  details->updated.connect (boost::bind (&LM::HeapRoster::on_personal_details_updated, this));
}

LM::HeapRoster::~HeapRoster ()
{
}

const std::string
LM::HeapRoster::get_name () const
{
  return name;
}

LmConnection*
LM::HeapRoster::get_connection () const
{
  return connection;
}

bool
LM::HeapRoster::populate_menu (Ekiga::MenuBuilder& builder)
{
  builder.add_action ("new", _("A_dd Contact"), boost::bind (&LM::HeapRoster::add_item, this));
  dialect->populate_menu (builder);
  return true;
}

bool
LM::HeapRoster::populate_menu_for_group (const std::string /*group*/,
					 Ekiga::MenuBuilder& /*builder*/)
{
  return false;
}

void
LM::HeapRoster::handle_up (LmConnection* connection_,
			   const std::string name_)
{
  connection = connection_;
  name = name_;

  { // populate the roster
    LmMessage* roster_request = lm_message_new_with_sub_type (NULL, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_GET);
    LmMessageNode* node = lm_message_node_add_child (lm_message_get_node (roster_request), "query", NULL);
    lm_message_node_set_attributes (node, "xmlns", "jabber:iq:roster", NULL);
    lm_connection_send_with_reply (connection, roster_request,
				   build_message_handler (boost::bind (&LM::HeapRoster::handle_initial_roster_reply, this, _1, _2)), NULL);
    lm_message_unref (roster_request);
  }
  { // initial presence push
    LmMessage* presence_push = lm_message_new (NULL, LM_MESSAGE_TYPE_PRESENCE);
    lm_connection_send (connection, presence_push, NULL);
    lm_message_unref (presence_push);
  }

  on_personal_details_updated (); // fake, but if we start as dnd, we want it known
  updated ();
}

void
LM::HeapRoster::handle_down (LmConnection* /*connection*/)
{
  removed ();
}

LmHandlerResult
LM::HeapRoster::handle_iq (LmConnection* /*connection*/,
			   LmMessage* message)
{
  LmHandlerResult result = LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS;
  if (lm_message_get_sub_type (message) == LM_MESSAGE_SUB_TYPE_SET) {

    LmMessageNode* node = lm_message_node_get_child (lm_message_get_node (message), "query");
    if (node != NULL) {

      const gchar* xmlns = lm_message_node_get_attribute (node, "xmlns");
      if (xmlns != NULL && strcmp (xmlns, "jabber:iq:roster") == 0) {

	parse_roster (node);
	result = LM_HANDLER_RESULT_REMOVE_MESSAGE;
      }
    }
  }

  return result;
}

LmHandlerResult
LM::HeapRoster::handle_presence (LmConnection* /*connection*/,
				 LmMessage* message)
{
  LmHandlerResult result = LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS;
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

    result = LM_HANDLER_RESULT_REMOVE_MESSAGE;
    boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&LM::HeapRoster::subscribe_from_form_submitted, this, _1, _2)));
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

     result = LM_HANDLER_RESULT_REMOVE_MESSAGE;
     item->push_presence (resource, lm_message_get_node (message));
    }
  }

  return result;
}

LmHandlerResult
LM::HeapRoster::handle_message (LmConnection* /*connection*/,
				LmMessage* message)
{
  LmHandlerResult result = LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS;
  LmMessageNode* node = lm_message_get_node (message);
  const gchar* from_c = lm_message_node_get_attribute (node, "from");
  const gchar* type_attr = lm_message_node_get_attribute (node, "type");
  std::string base_jid;

  if (from_c != 0) {

    std::string from (from_c);
    std::string::size_type index = from.find ('/');
    base_jid = std::string (from, 0, index);
  }

  PresentityPtr item = find_item (base_jid);

  if (item && (type_attr == NULL
	       || (type_attr != NULL && strcmp (type_attr, "normal") == 0)
	       || (type_attr != NULL && strcmp (type_attr, "chat") == 0))) {

    // let's imagine it's a basic chat message
    LmMessageNode* body = lm_message_node_find_child (node, "body");
    if (body && lm_message_node_get_value (body) != NULL) {

      result = LM_HANDLER_RESULT_REMOVE_MESSAGE;
      dialect->push_message (item, lm_message_node_get_value (body));
    }
    // it could also be an avatar or a pubsub event or...
  }

  return result;
}

LmHandlerResult
LM::HeapRoster::handle_initial_roster_reply (LmConnection* /*connection*/,
					     LmMessage* message)
{
  LmHandlerResult result = LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS;
  if (lm_message_get_sub_type (message) == LM_MESSAGE_SUB_TYPE_RESULT) {

    LmMessageNode* node = lm_message_node_get_child (lm_message_get_node (message), "query");
    if (node != NULL) {

      const gchar* xmlns = lm_message_node_get_attribute (node, "xmlns");
      if (xmlns != NULL && strcmp (xmlns, "jabber:iq:roster") == 0) {

	parse_roster (node);
	result = LM_HANDLER_RESULT_REMOVE_MESSAGE;
      }
    }
  }

  return result;
}

void
LM::HeapRoster::parse_roster (LmMessageNode* query)
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
      presentity->chat_requested.connect (boost::bind (&LM::HeapRoster::on_chat_requested, this, presentity));
      add_presentity (presentity);
      const gchar* subscription = lm_message_node_get_attribute (node, "subscription");
      if (subscription != NULL && strcmp (subscription, "none") == 0) {

	const gchar* ask = lm_message_node_get_attribute (node, "ask");
	if (ask == NULL || (ask != NULL && strcmp (ask, "subscribe") != 0)) {

	  std::set<std::string>::iterator iter = items_added_by_me.find (presentity->get_jid ());
	  if (iter != items_added_by_me.end ()) {

	    /* if we're here then this is a new contact, we are not subscribed to it,
	     * and we did not ask to be subscribed to it and we added it recently:
	     * let's ask for subscription!
	     *
	     * Beware that the first three actions could have been done from another client,
	     * so that last condition is important so we don't start doing things
	     * in the back of the user!
	     */
	    items_added_by_me.erase (iter);
	    LmMessage* subscribe = lm_message_new (NULL, LM_MESSAGE_TYPE_PRESENCE);
	    lm_message_node_set_attributes (lm_message_get_node (subscribe),
					    "to", presentity->get_jid ().c_str (),
					    "type", "subscribe",
					    NULL);
	    lm_connection_send (connection, subscribe, NULL);
	    lm_message_unref (subscribe);
	  }
	}
      }
    }
  }
}

void
LM::HeapRoster::add_item ()
{
  boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&LM::HeapRoster::add_item_form_submitted, this, _1, _2)));

  request->title (_("Add a roster element"));
  request->instructions (_("Please fill in this form to add a new"
			   " element to the remote roster"));
  request->text ("jid", _("Identifier:"), _("identifier@server"), std::string ());
  request->text ("name", _("Name:"), "", _("Name of the contact, as shown in your roster"));
  request->editable_set ("groups",
			 _("Put contact in groups:"),
			 std::set<std::string>(), existing_groups ());

  questions (request);
}

void
LM::HeapRoster::add_item_form_submitted (bool submitted,
					 Ekiga::Form& result)
{
  if ( !submitted)
    return;

  const std::string jid = result.text ("jid");
  const std::string contact_name = result.text ("name");
  const std::set<std::string> groups = result.editable_set ("groups");

  if ( !jid.empty ()) {

    LmMessage* message = lm_message_new_with_sub_type (NULL, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_SET);
    LmMessageNode* query = lm_message_node_add_child (lm_message_get_node (message), "query", NULL);
    lm_message_node_set_attribute (query, "xmlns", "jabber:iq:roster");
    LmMessageNode* node = lm_message_node_add_child (query, "item", NULL);
    lm_message_node_set_attributes (node,
				    "jid", jid.c_str (),
				    NULL);
    if ( !contact_name.empty ()) {

      gchar* escaped = g_markup_escape_text (contact_name.c_str (), -1);
      lm_message_node_set_attributes (node,
				      "name", escaped,
				      NULL);
    }

    for (std::set<std::string>::const_iterator iter = groups.begin (); iter != groups.end (); ++iter) {

      gchar* escaped = g_markup_escape_text (iter->c_str (), -1);
      lm_message_node_add_child (node, "group", escaped);
      g_free (escaped);
    }

    items_added_by_me.insert (jid);
    lm_connection_send (connection, message, NULL);
    lm_message_unref (message);
  }
}

void
LM::HeapRoster::subscribe_from_form_submitted (bool submitted,
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
LM::HeapRoster::find_item (const std::string jid)
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
LM::HeapRoster::on_personal_details_updated ()
{
  LmMessage* message = lm_message_new (NULL, LM_MESSAGE_TYPE_PRESENCE);

  lm_message_node_add_child (lm_message_get_node (message), "show", details->get_presence ().c_str ());
  lm_message_node_add_child (lm_message_get_node (message), "status", details->get_status ().c_str ());

  lm_connection_send (connection, message, NULL);
  lm_message_unref (message);
}

void
LM::HeapRoster::on_chat_requested (PresentityPtr presentity)
{
  dialect->open_chat (presentity);
}

struct lm_existing_groups_helper
{
  std::set<std::string> groups;

  bool operator() (Ekiga::PresentityPtr presentity)
  {
    const std::set<std::string> presentity_groups = presentity->get_groups ();

    groups.insert (presentity_groups.begin (),
		   presentity_groups.end ());

    return true;
  }
};

const std::set<std::string>
LM::HeapRoster::existing_groups () const
{
  std::set<std::string> result;

  {
    lm_existing_groups_helper helper;
    visit_presentities (boost::ref (helper));
    result = helper.groups;
  }
  result.insert (_("Family"));
  result.insert (_("Friend"));
  /* Translator: http://www.ietf.org/rfc/rfc4480.txt proposes several
     relationships between you and your contact; associate means
     someone who is at the same "level" than you.
  */
  result.insert (_("Associate"));
  /* Translator: http://www.ietf.org/rfc/rfc4480.txt proposes several
     relationships between you and your contact; assistant means
     someone who is at a lower "level" than you.
  */
  result.insert (_("Assistant"));
  /* Translator: http://www.ietf.org/rfc/rfc4480.txt proposes several
     relationships between you and your contact; supervisor means
     someone who is at a higher "level" than you.
  */
  result.insert (_("Supervisor"));
  /* Translator: http://www.ietf.org/rfc/rfc4480.txt proposes several
     relationships between you and your contact; self means yourself.
  */
  result.insert (_("Self"));

  return result;
}
