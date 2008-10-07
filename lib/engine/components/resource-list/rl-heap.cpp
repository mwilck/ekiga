
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
 *                         rl-heap.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : resource-list heap implementation
 *
 */

#include "config.h"

#include <iostream>

#include "robust-xml.h"

#include "rl-heap.h"

RL::Heap::Heap (Ekiga::ServiceCore& core_,
		xmlNodePtr node_):
  core(core_), node(node_), uri(NULL),
  username(NULL), password(NULL), name(NULL)
{
  for (xmlNodePtr child = node->children; child != NULL; child = child->next) {

    if (child->type == XML_ELEMENT_NODE
	&& child->name != NULL) {

      if (xmlStrEqual (BAD_CAST ("uri"), child->name))
	uri = child;
      if (xmlStrEqual (BAD_CAST ("username"), child->name))
	username = child;
      if (xmlStrEqual (BAD_CAST ("password"), child->name))
	password = child;
      if (xmlStrEqual (BAD_CAST ("name"), child->name))
	name = child;
    }
  }

  if (uri == NULL)
    uri = xmlNewChild (node, NULL, BAD_CAST "uri", BAD_CAST "");
  if (name == NULL)
    name = xmlNewChild (node, NULL, BAD_CAST "name",
			BAD_CAST robust_xmlEscape(node->doc,
						  _("Unnamed")).c_str ());
  if (username == NULL)
    username = xmlNewChild (node, NULL, BAD_CAST "username", BAD_CAST "");
  if (password == NULL)
    password = xmlNewChild (node, NULL, BAD_CAST "password", BAD_CAST "");

  update ();
}

RL::Heap::Heap (Ekiga::ServiceCore& core_,
		const std::string name_,
		const std::string uri_,
		const std::string username_,
		const std::string password_):
  core(core_), node(NULL), uri(NULL), username(NULL), password(NULL), name(NULL)
{
  node = xmlNewNode (NULL, BAD_CAST "entry");
  uri = xmlNewChild (node, NULL,
		     BAD_CAST "uri",
		     BAD_CAST robust_xmlEscape (node->doc,
						uri_).c_str ());
  username = xmlNewChild (node, NULL,
			  BAD_CAST "username",
			  BAD_CAST robust_xmlEscape (node->doc,
						     username_).c_str ());
  password = xmlNewChild (node, NULL,
			  BAD_CAST "password",
			  BAD_CAST robust_xmlEscape (node->doc,
						     password_).c_str ());
  if ( !name_.empty ())
    name = xmlNewChild (node, NULL,
			BAD_CAST "name",
			BAD_CAST robust_xmlEscape (node->doc,
						   name_).c_str ());
  else
    name = xmlNewChild (node, NULL,
			BAD_CAST "name",
			BAD_CAST robust_xmlEscape (node->doc,
						   _("Unnamed")).c_str ());
  update ();
}

RL::Heap::~Heap ()
{
}

const std::string
RL::Heap::get_name () const
{
  std::string result;
  xmlChar* str = xmlNodeGetContent (name);
  if (str != NULL)
    result = (const gchar*)str;
  else
    result = _("Unnamed");

  xmlFree (str);

  return result;
}

const std::string
RL::Heap::get_uri () const
{
  std::string result;
  xmlChar* str = xmlNodeGetContent (uri);
  if (str != NULL)
    result = (const gchar*)str;
  else
    result = "";

  xmlFree (str);

  return result;
}


bool
RL::Heap::populate_menu (Ekiga::MenuBuilder& /*builder*/)
{
  return false; // FIXME
}

bool
RL::Heap::populate_menu_for_group (std::string /*group*/,
				   Ekiga::MenuBuilder& /*builder*/)
{
  return false; // FIXME
}

xmlNodePtr
RL::Heap::get_node () const
{
  return node;
}

void
RL::Heap::update ()
{
  XCAP::Core* xcap
    = dynamic_cast<XCAP::Core*>(core.get ("xcap-core"));
  xcap->read (get_uri (),
	      sigc::mem_fun (this, &RL::Heap::on_document_received));
}

void
RL::Heap::on_document_received (XCAP::Core::ResultType result,
				std::string doc)
{
  switch (result) {

  case XCAP::Core::SUCCESS:

    parse_doc (doc);
    break;
  case XCAP::Core::ERROR:

    std::cout << "Error: " << doc << std::endl;
    break;
  default:
    // shouldn't happen
    break;
  }
}

void
RL::Heap::parse_doc (std::string raw)
{
  xmlDocPtr doc = xmlRecoverMemory (raw.c_str (), raw.length ());
  xmlNodePtr root = xmlDocGetRootElement (doc);

  if (root == NULL) {

    // FIXME: warn the user somehow?
  } else {

    for (xmlNodePtr child = root->children; child != NULL; child = child->next)
      if (child->type == XML_ELEMENT_NODE
	  && child->name != NULL
	  && xmlStrEqual (BAD_CAST ("list"), child->name)) {

	parse_list (child, NULL);
      }
  }
  xmlFreeDoc (doc);
}

void
RL::Heap::parse_list (xmlNodePtr list,
		      const gchar* base_name)
{
  gchar* display_name = NULL;
  gchar* unnamed = NULL;

  if (base_name == NULL)
    unnamed = g_strdup (_("Unnamed"));
  else
    unnamed = g_strdup_printf ("%s / %s",
			       base_name, _("Unnamed"));

  for (xmlNodePtr child = list->children;
       child != NULL;
       child = child->next) {

    if (child->type == XML_ELEMENT_NODE
	&& child->name != NULL) {

      if (xmlStrEqual (BAD_CAST ("display-name"), child->name)) {

	xmlChar* xml_str = xmlNodeGetContent (child);
	if (xml_str != NULL && display_name == NULL) {

	  if (base_name == NULL)
	    display_name = g_strdup ((const gchar*)xml_str);
	  else
	    display_name = g_strdup_printf ("%s / %s",
					    base_name, (const gchar*)xml_str);
	}
	xmlFree (xml_str);
      }

      if (xmlStrEqual (BAD_CAST ("list"), child->name)) {

	if (display_name != NULL)
	  parse_list (child, display_name);
	else
	  parse_list (child, unnamed);
      }
      if (xmlStrEqual (BAD_CAST ("entry"), child->name)) {

	if (display_name != NULL)
	  parse_entry (child, display_name);
	else
	  parse_entry (child, unnamed);
      }
    }
  }

  g_free (unnamed);
  g_free (display_name);
}

void
RL::Heap::parse_entry (xmlNodePtr entry,
		       const gchar* group_name)
{
  gchar* entry_uri = NULL;
  std::string display_name = _("Unnamed");
  Presentity* presentity = NULL;

  {
    xmlChar* str = xmlGetProp (entry, BAD_CAST "uri");
    if (str != NULL)
      entry_uri = g_strdup ((const gchar*)str);
  }

  for (xmlNodePtr child = entry->children; child != NULL; child = child->next)
    if (child->type == XML_ELEMENT_NODE
	&& child->name != NULL
	&& xmlStrEqual (BAD_CAST ("display-name"), child->name)) {

      xmlChar* xml_str = xmlNodeGetContent (child);
      if (xml_str != NULL)
	display_name = (const gchar*)xml_str;
      xmlFree (xml_str);
    }

  if (entry_uri != NULL) {

    presentity = new Presentity (core, display_name, entry_uri);
    if (group_name != NULL)
      presentity->add_group (group_name);
    add_presentity (*presentity);
    g_free (entry_uri);
  }
}

void
RL::Heap::push_presence (const std::string uri_,
			 const std::string presence)
{
  for (iterator iter = begin ();
       iter != end ();
       ++iter)
    if (iter->get_uri () == uri_)
      iter->set_presence (presence);
}

void
RL::Heap::push_status (const std::string uri_,
		       const std::string status)
{
  for (iterator iter = begin ();
       iter != end ();
       ++iter)
    if (iter->get_uri () == uri_)
      iter->set_status (status);
}
