
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

void
LM::Presentity::update (LmMessageNode* item_)
{
  lm_message_node_unref (item);
  item = item_;
  lm_message_node_ref (item);
  updated.emit ();
}

const std::string
LM::Presentity::get_jid () const
{
  return lm_message_node_get_attribute (item, "jid");
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
LM::Presentity::populate_menu (Ekiga::MenuBuilder& /*builder*/)
{
  return false; // FIXME
}
