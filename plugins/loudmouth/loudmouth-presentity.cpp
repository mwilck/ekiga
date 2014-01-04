
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
 *                         loudmouth-presentity.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of a loudmouth presentity
 *
 */

#include "loudmouth-presentity.h"

#include <stdlib.h>
#include <glib/gi18n.h>

#include <boost/algorithm/string/predicate.hpp>

#include "form-request-simple.h"

#include "loudmouth-helpers.h"

LM::Presentity::Presentity (LmConnection* connection_,
			    LmMessageNode* item_):
  has_chat (false), connection(connection_), item(item_)
{
  lm_connection_ref (connection);
  lm_message_node_ref (item);
}

LM::Presentity::~Presentity ()
{
  lm_message_node_unref (item);
  item = 0;

  lm_connection_unref (connection);
  connection = 0;
}

const std::string
LM::Presentity::get_name () const
{
  const gchar* result = lm_message_node_get_attribute (item, "name");

  if (result == NULL) {

    result = lm_message_node_get_attribute (item, "jid");
  }

  return result;
}

bool
LM::Presentity::has_uri (const std::string uri) const
{
  std::string full_jid = get_jid ();
  std::string base_jid = full_jid.substr (0, full_jid.find ('/'));
  return boost::starts_with (uri, base_jid);
}

const std::string
LM::Presentity::get_presence () const
{
  std::string result = "offline";

  if ( !infos.empty ()) {

    infos_type::const_iterator iter = infos.begin ();
    ResourceInfo best = iter->second;
    ++iter;
    while (iter != infos.end ()) {

      if (iter->second.priority > best.priority) {

	best = iter->second;
      }
      ++iter;
    }
    if (best.presence == "") {
      result = "available";
    }
    else if (best.presence == "xa") {
      result = "away";
    }
    else if (best.presence == "dnd") {
      result = "busy";
    }
    else {
      result = best.presence;
    }
  }

  return result;
}

const std::string
LM::Presentity::get_status () const
{
  std::string result = "";

  if ( !infos.empty ()) {

    infos_type::const_iterator iter = infos.begin ();
    ResourceInfo best = iter->second;
    ++iter;
    while (iter != infos.end ()) {

      if (iter->second.priority > best.priority) {

	best = iter->second;
      }
      ++iter;
    }
    result = best.status;
  }

  return result;
}

const std::set<std::string>
LM::Presentity::get_groups () const
{
  std::set<std::string> result;

  for (LmMessageNode* node = item->children; node != NULL; node = node->next) {

    if (g_strcmp0 (node->name, "group") == 0) {

      if (node->value) {

	result.insert (node->value);
      }
    }
  }

  return result;
}

bool
LM::Presentity::populate_menu (Ekiga::MenuBuilder& builder)
{
  const gchar* subscription = lm_message_node_get_attribute (item, "subscription");
  const gchar* ask = lm_message_node_get_attribute (item, "ask");

  if ( !has_chat) {

    builder.add_action ("im-message-new", _("Start chat"), boost::ref (chat_requested));
  } else {

    builder.add_action ("im-message-new", _("Continue chat"), boost::ref (chat_requested));
  }
  builder.add_separator ();

  builder.add_action ("edit", _("_Edit"),
		      boost::bind (&LM::Presentity::edit_presentity, this));

  if (g_strcmp0 (subscription, "none") == 0) {

    builder.add_action ("ask", _("Ask him/her to see his/her status"), boost::bind (&LM::Presentity::ask_to, this));
  }
  if (g_strcmp0 (subscription, "from") == 0) {

    builder.add_action ("stop", _("Forbid him/her to see my status"), boost::bind (&LM::Presentity::revoke_from, this));
    if (ask == NULL)
      builder.add_action ("ask", _("Ask him/her to see his/her status"), boost::bind (&LM::Presentity::ask_to, this));
    else
      builder.add_ghost ("ask", _("Ask him/her to see his/her status (pending)"));
  }
  if (g_strcmp0 (subscription, "to") == 0) {

    builder.add_action ("stop", _("Stop getting his/her status"), boost::bind (&LM::Presentity::stop_to, this));
  }
  if (g_strcmp0 (subscription, "both") == 0) {

    builder.add_action ("stop", _("Forbid him/her to see my status"), boost::bind (&LM::Presentity::revoke_from, this));
    builder.add_action ("stop", _("Stop getting his/her status"), boost::bind (&LM::Presentity::stop_to, this));
  }

  builder.add_action ("remove", _("_Remove"),
		      boost::bind (&LM::Presentity::remove_presentity, this));
  return true;
}

const std::string
LM::Presentity::get_jid () const
{
  return lm_message_node_get_attribute (item, "jid");
}

LmConnection*
LM::Presentity::get_connection () const
{
  return connection;
}

void
LM::Presentity::update (LmMessageNode* item_)
{
  lm_message_node_unref (item);
  item = item_;
  lm_message_node_ref (item);
  updated ();
}

void
LM::Presentity::push_presence (const std::string resource,
			       LmMessageNode* presence)
{
  if (resource.empty ())
    return;

  ResourceInfo info;

  LmMessageNode* priority = lm_message_node_find_child (presence, "priority");
  if (priority != NULL) {

    info.priority = atoi (lm_message_node_get_value (priority));

  } else {

    info.priority = 50;
  }

  LmMessageNode* status = lm_message_node_find_child (presence, "status");
  if (status != NULL) {

    const gchar* status_str = lm_message_node_get_value (status);
    if (status_str != NULL)
      info.status = status_str;
  }

  LmMessageNode* away = lm_message_node_find_child (presence, "show");
  if (away != NULL) {

    info.presence = lm_message_node_get_value (away);
  } else {

    info.presence = "available";
  }

  const gchar* oftype = lm_message_node_get_attribute (presence, "type");
  if (oftype != NULL) {

    if (oftype == std::string ("unavailable")) {

      info.presence = "unavailable";
    }
  }

  infos[resource] = info;

  if (info.presence == "unavailable") {

    infos.erase (resource);
  }

  updated ();
}

void
LM::Presentity::edit_presentity ()
{
  boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&LM::Presentity::edit_presentity_form_submitted, this, _1, _2)));

  request->title (_("Edit roster element"));
  request->instructions (_("Please fill in this form to change an existing "
			   "element of the remote roster"));
  request->text ("name", _("Name:"), get_name (), std::string ());

  request->editable_set ("groups", _("Choose groups:"),
			 get_groups (), get_groups ());

  questions (request);
}

void
LM::Presentity::edit_presentity_form_submitted (bool submitted,
						Ekiga::Form& result)
{
  if (!submitted)
    return;

  const std::string name = result.text ("name");
  const std::set<std::string> groups = result.editable_set ("groups");
  LmMessage* message = lm_message_new_with_sub_type (NULL, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_SET);
  LmMessageNode* query = lm_message_node_add_child (lm_message_get_node (message), "query", NULL);
  lm_message_node_set_attribute (query, "xmlns", "jabber:iq:roster");
  LmMessageNode* node = lm_message_node_add_child (query, "item", NULL);

  {
    gchar* escaped = g_markup_escape_text (name.c_str (), -1);
    lm_message_node_set_attributes (node,
				    "jid", get_jid ().c_str (),
				    "name", escaped,
				    NULL);
    g_free (escaped);
  }

  for (std::set<std::string>::const_iterator iter = groups.begin (); iter != groups.end (); ++iter) {

    gchar* escaped = g_markup_escape_text (iter->c_str (), -1);
    lm_message_node_add_child (node, "group", escaped);
    g_free (escaped);
  }

  lm_connection_send_with_reply (connection, message,
				 build_message_handler (boost::bind(&LM::Presentity::handle_edit_reply, this, _1, _2)), NULL);
  lm_message_unref (message);
}

LmHandlerResult
LM::Presentity::handle_edit_reply (LmConnection* /*connection*/,
				   LmMessage* message)
{
  if (lm_message_get_sub_type (message) == LM_MESSAGE_SUB_TYPE_ERROR) {

    std::cout << "Don't know how to handle : " << lm_message_node_to_string (lm_message_get_node (message)) << std::endl;
  }

  return LM_HANDLER_RESULT_REMOVE_MESSAGE;
}

void
LM::Presentity::remove_presentity ()
{
  LmMessage* message = lm_message_new_with_sub_type (NULL, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_SET);
  LmMessageNode* query = lm_message_node_add_child (lm_message_get_node (message), "query", NULL);
  lm_message_node_set_attribute (query, "xmlns", "jabber:iq:roster");
  LmMessageNode* node = lm_message_node_add_child (query, "item", NULL);

  lm_message_node_set_attributes (node,
				  "jid", get_jid ().c_str (),
				  "subscription", "remove",
				  NULL);

  lm_connection_send_with_reply (connection, message, get_ignore_answer_handler (), NULL);
  lm_message_unref (message);
}

void
LM::Presentity::revoke_from ()
{
  LmMessage* message = lm_message_new (NULL, LM_MESSAGE_TYPE_PRESENCE);
  lm_message_node_set_attributes (lm_message_get_node (message),
				  "to", get_jid ().c_str (),
				  "type", "unsubscribed",
				  NULL);
  lm_connection_send_with_reply (connection, message, get_ignore_answer_handler (), NULL);
  lm_message_unref (message);
}

void LM::Presentity::ask_to ()
{
  LmMessage* message = lm_message_new (NULL, LM_MESSAGE_TYPE_PRESENCE);
  lm_message_node_set_attributes (lm_message_get_node (message),
				  "to", get_jid ().c_str (),
				  "type", "subscribe",
				  NULL);
  lm_connection_send_with_reply (connection, message, get_ignore_answer_handler (), NULL);
  lm_message_unref (message);
}

void
LM::Presentity::stop_to ()
{
  LmMessage* message = lm_message_new (NULL, LM_MESSAGE_TYPE_PRESENCE);
  lm_message_node_set_attributes (lm_message_get_node (message),
				  "to", get_jid ().c_str (),
				  "type", "unsubscribe",
				  NULL);
  lm_connection_send_with_reply (connection, message, get_ignore_answer_handler (), NULL);
  lm_message_unref (message);
}
