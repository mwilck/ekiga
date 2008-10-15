
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras
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
 *                         rl-entry.cpp -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : resource-list entry class
 *
 */


#include <libxml/debugXML.h>

#include "config.h"

#include "rl-entry.h"

#include "presence-core.h"

RL::Entry::Entry (Ekiga::ServiceCore& core_,
		  gmref_ptr<XCAP::Path> path_,
		  int pos,
		  const std::string group,
		  xmlNodePtr node_):
  core(core_), position(pos), doc(NULL), node(node_), name_node(NULL),
  presence("unknown"), status("")
{
  groups.insert (group);

  if (node != NULL) {

    xmlChar* str = xmlGetProp (node, BAD_CAST "name");

    if (str != NULL) {

      path = path_->build_child_with_attribute ("entry", "name",
						(const char*)str);
      xmlFree (str);
    } else {

      path = path_->build_child_with_position ("entry", position);
    }
    parse ();
  } else {

    path = path_;
  }
}

RL::Entry::~Entry ()
{
  if (doc != NULL)
    xmlFreeDoc (doc);
}

bool
RL::Entry::is_positional () const
{
  return get_uri ().empty ();
}

const std::string
RL::Entry::get_uri () const
{
  std::string result;
  xmlChar* str = xmlGetProp (node, (const xmlChar*) "uri");

  if (str != NULL) {

    result = ((const char*)str);
    xmlFree (str);
  }

  return result;
}


void
RL::Entry::set_presence (const std::string presence_)
{
  presence = presence_;
  updated.emit ();
}

void
RL::Entry::set_status (const std::string status_)
{
  status = status_;
  updated.emit ();
}

const std::string
RL::Entry::get_name () const
{
  std::string result;
  xmlChar* str = xmlNodeGetContent (name_node);

  if (str != NULL) {

    result = ((const char*)str);
    xmlFree (str);
  }

  return result;
}

bool
RL::Entry::populate_menu (Ekiga::MenuBuilder& builder)
{
  bool populated = false;
  gmref_ptr<Ekiga::PresenceCore> presence_core = core.get ("presence-core");
  std::string uri(get_uri ());

  builder.add_action ("refresh", _("_Refresh"),
		      sigc::mem_fun (this, &RL::Entry::refresh));

  if ( !uri.empty ())
    populated =
      presence_core->populate_presentity_menu (*this, uri, builder)
      || populated;

  return populated;
}

void
RL::Entry::refresh ()
{
  /* FIXME: here we should fetch the document, then parse it */
  std::cout << "Refreshing on " << path << std::endl;
}

void
RL::Entry::parse ()
{
  for (xmlNodePtr child = node->children; child != NULL; child = child->next) {


    if (child->type == XML_ELEMENT_NODE
	&& child->name != NULL
	&& xmlStrEqual (BAD_CAST "display-name", child->name)) {

      name_node = child;
    }
  }
}
