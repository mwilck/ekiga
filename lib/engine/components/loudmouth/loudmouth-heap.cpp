
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

#include <iostream>
#include <string.h>

#include "loudmouth-heap.h"

LmHandlerResult
iq_handler_c (LmMessageHandler* /*handler*/,
		      LmConnection* /*connection*/,
		      LmMessage* message,
		      LM::Heap* heap)
{
  return heap->iq_handler (message);
}

LM::Heap::Heap (LmConnection* connection_): connection(connection_)
{
  lm_connection_ref (connection);

  iq_lm_handler = lm_message_handler_new ((LmHandleMessageFunction)iq_handler_c, this, NULL);
  lm_connection_register_message_handler (connection, iq_lm_handler, LM_MESSAGE_TYPE_IQ, LM_HANDLER_PRIORITY_NORMAL);

  { // populate the roster
    LmMessage* roster_request = lm_message_new_with_sub_type (NULL, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_GET);
    LmMessageNode* node = lm_message_node_add_child (lm_message_get_node (roster_request), "query", NULL);
    lm_message_node_set_attributes (node, "xmlns", "jabber:iq:roster", NULL);
    lm_connection_send (connection, roster_request, NULL);
    lm_message_unref (roster_request);
  }
}

LM::Heap::~Heap ()
{
  lm_connection_unregister_message_handler (connection, iq_lm_handler, LM_MESSAGE_TYPE_IQ);

  lm_message_handler_unref (iq_lm_handler);
  iq_lm_handler = 0;

  lm_connection_unref (connection);
  connection = 0;

  std::cout << __PRETTY_FUNCTION__ << std::endl;
}

const std::string
LM::Heap::get_name () const
{
  return lm_connection_get_jid (connection);
}

bool
LM::Heap::populate_menu (Ekiga::MenuBuilder& /*builder*/)
{
  return false;
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
  removed.emit ();
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

	  (*iter)->removed.emit ();
	} else {

	  (*iter)->update (node);
	}
      }
    }
    if ( !found) {

      gmref_ptr<Presentity> presentity(new Presentity (connection, node));
      add_presentity (presentity);
    }
  }
}
