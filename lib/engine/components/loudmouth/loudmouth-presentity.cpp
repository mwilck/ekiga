
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

#include <iostream>
#include <string.h>
#include <glib/gi18n.h>

#include "form-request-simple.h"

#include "loudmouth-presentity.h"

LM::Presentity::Presentity (LmConnection* connection_,
			    LmMessageNode* item_):
  connection(connection_), item(item_)
{
  lm_connection_ref (connection);
  lm_message_node_ref (item);
}

LM::Presentity::~Presentity ()
{
  std::cout << __PRETTY_FUNCTION__ << std::endl;

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

const std::string
LM::Presentity::get_presence () const
{
  return "FIXME";
}

const std::string
LM::Presentity::get_status () const
{
  return "FIXME";
}

const std::string
LM::Presentity::get_avatar () const
{
  return "FIXME";
}

const std::set<std::string>
LM::Presentity::get_groups () const
{
  std::set<std::string> result;

  for (LmMessageNode* node = item->children; node != NULL; node = node->next) {

    if (strcmp (node->name, "group") == 0) {

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
  builder.add_action ("edit", _("_Edit"),
		      sigc::mem_fun (this, &LM::Presentity::edit_presentity));
  builder.add_action ("remove", _("_Remove"),
		      sigc::mem_fun (this, &LM::Presentity::remove_presentity));
  return true;
}

const std::string
LM::Presentity::get_jid () const
{
  return lm_message_node_get_attribute (item, "jid");
}

void
LM::Presentity::update (LmMessageNode* item_)
{
  lm_message_node_unref (item);
  item = item_;
  lm_message_node_ref (item);
  updated.emit ();
}

void
LM::Presentity::edit_presentity ()
{
  Ekiga::FormRequestSimple request(sigc::mem_fun (this, &LM::Presentity::edit_presentity_form_submitted));

  request.title (_("Edit roster element"));
  request.instructions (_("Please fill in this form to change an existing "
			  "element of the remote roster"));
  request.text ("name", _("Name:"), get_name ());

  request.editable_set ("groups", _("Choose groups:"),
			get_groups (), get_groups ());

  if (!questions.handle_request (&request)) {

    // FIXME: better error reporting
#ifdef __GNUC__
    std::cout << "Unhandled form request in "
	      << __PRETTY_FUNCTION__ << std::endl;
#endif
  }
}

void
LM::Presentity::edit_presentity_form_submitted (bool submitted,
						Ekiga::Form& result)
{
  if (!submitted)
    return;

  try {

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

    lm_connection_send (connection, message, NULL);
    lm_message_unref (message);

  } catch (Ekiga::Form::not_found) {
#ifdef __GNUC__
    std::cerr << "Invalid form submitted to "
	      << __PRETTY_FUNCTION__ << std::endl;
#endif
  }
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

  lm_connection_send (connection, message, NULL);
  lm_message_unref (message);
}
