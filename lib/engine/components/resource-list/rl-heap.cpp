
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

#include <glib.h>
#include <iostream>

#include "robust-xml.h"

#include "rl-heap.h"

RL::Heap::Heap (Ekiga::ServiceCore& core_,
		xmlNodePtr node_):
  core(core_),
  node(node_), name(NULL),
  root(NULL), user(NULL),
  username(NULL), password(NULL),
  doc(NULL)
{
  for (xmlNodePtr child = node->children; child != NULL; child = child->next) {

    if (child->type == XML_ELEMENT_NODE
	&& child->name != NULL) {

      if (xmlStrEqual (BAD_CAST ("name"), child->name)) {

	name = child;
	continue;
      }
      if (xmlStrEqual (BAD_CAST ("root"), child->name)) {

	root = child;
	continue;
      }
      if (xmlStrEqual (BAD_CAST ("user"), child->name)) {

	user = child;
	continue;
      }
      if (xmlStrEqual (BAD_CAST ("username"), child->name)) {

	username = child;
	continue;
      }
      if (xmlStrEqual (BAD_CAST ("password"), child->name)) {

	password = child;
	continue;
      }
    }
  }

  if (name == NULL)
    name = xmlNewChild (node, NULL, BAD_CAST "name",
			BAD_CAST robust_xmlEscape(node->doc,
						  _("Unnamed")).c_str ());
  if (root == NULL)
    root = xmlNewChild (node, NULL, BAD_CAST "root", BAD_CAST "");
  if (user == NULL)
    user = xmlNewChild (node, NULL, BAD_CAST "user", BAD_CAST "");
  if (username == NULL)
    username = xmlNewChild (node, NULL, BAD_CAST "username", BAD_CAST "");
  if (password == NULL)
    password = xmlNewChild (node, NULL, BAD_CAST "password", BAD_CAST "");

  refresh ();
}

RL::Heap::Heap (Ekiga::ServiceCore& core_,
		const std::string name_,
		const std::string root_,
		const std::string user_,
		const std::string username_,
		const std::string password_):
  core(core_),
  node(NULL), name(NULL),
  root(NULL), user(NULL),
  username(NULL), password(NULL),
  doc(NULL)
{
  node = xmlNewNode (NULL, BAD_CAST "entry");
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
  root = xmlNewChild (node, NULL,
		      BAD_CAST "root",
		      BAD_CAST robust_xmlEscape (node->doc,
						root_).c_str ());
  user = xmlNewChild (node, NULL,
		      BAD_CAST "user",
		      BAD_CAST robust_xmlEscape (node->doc,
						 user_).c_str ());
  username = xmlNewChild (node, NULL,
			  BAD_CAST "username",
			  BAD_CAST robust_xmlEscape (node->doc,
						     username_).c_str ());
  password = xmlNewChild (node, NULL,
			  BAD_CAST "password",
			  BAD_CAST robust_xmlEscape (node->doc,
						     password_).c_str ());
  refresh ();
}

RL::Heap::~Heap ()
{
  if (doc != NULL)
    xmlFreeDoc (doc);
}

const std::string
RL::Heap::get_name () const
{
  std::string result;
  xmlChar* str = xmlNodeGetContent (name);
  if (str != NULL)
    result = (const char*)str;
  else
    result = _("Unnamed");

  xmlFree (str);

  return result;
}

void
RL::Heap::visit_presentities (sigc::slot<bool, Ekiga::Presentity&> visitor)
{
  bool go_on = true;

  for (std::list<gmref_ptr<List> >::iterator iter = lists.begin ();
       go_on && iter != lists.end ();
       ++iter)
    go_on = (*iter)->visit_presentities (visitor);
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
RL::Heap::refresh ()
{
  gmref_ptr<XCAP::Core> xcap = core.get ("xcap-core");
  std::string root_str;
  std::string username_str;
  std::string password_str;
  std::string user_str;

  {
    xmlChar* str = xmlNodeGetContent (root);
    if (str != NULL)
      root_str = (const char*)str;
  }
  {
    xmlChar* str = xmlNodeGetContent (user);
    if (str != NULL)
      user_str = (const char*)str;
  }
  {
    xmlChar* str = xmlNodeGetContent (username);
    if (str != NULL)
      username_str = (const char*)str;
  }
  {
    xmlChar* str = xmlNodeGetContent (password);
    if (str != NULL)
      password_str = (const char*)str;
  }
  gmref_ptr<XCAP::Path> path(new XCAP::Path (root_str, "resource-lists",
					     user_str));
  path->set_credentials (username_str, password_str);
  path = path->build_child ("resource-lists");

  for (std::list<gmref_ptr<List> >::iterator iter = lists.begin ();
       iter != lists.end ();
       ++iter)
    (*iter)->flush ();
  lists.clear ();

  if (doc)
    xmlFreeDoc (doc);
  doc = NULL;

  xcap->read (path, sigc::mem_fun (this, &RL::Heap::on_document_received));
}

void
RL::Heap::on_document_received (XCAP::Core::ResultType result,
				std::string value)
{
  switch (result) {

  case XCAP::Core::SUCCESS:

    parse_doc (value);
    break;
  case XCAP::Core::ERROR:

    std::cout << "XCAP error: " << value << std::endl;
    // FIXME: do something
    break;
  default:
    // shouldn't happen
    break;
  }
}

void
RL::Heap::parse_doc (std::string raw)
{
  doc = xmlRecoverMemory (raw.c_str (), raw.length ());

  xmlNodePtr doc_root = xmlDocGetRootElement (doc);

  if (doc_root == NULL) {

    // FIXME: warn the user somehow?
    xmlFreeDoc (doc);
    doc = NULL;
  } else {

    int pos = 1;
    std::string root_str;
    std::string user_str;
    std::string username_str;
    std::string password_str;

    {
      xmlChar* str = xmlNodeGetContent (root);
      if (str != NULL)
	root_str = (const char*)str;
    }
    {
      xmlChar* str = xmlNodeGetContent (user);
      if (str != NULL)
	user_str = (const char*)str;
    }
    {
      xmlChar* str = xmlNodeGetContent (username);
      if (str != NULL)
	username_str = (const char*)str;
    }
    {
      xmlChar* str = xmlNodeGetContent (password);
      if (str != NULL)
	password_str = (const char*)str;
    }

    gmref_ptr<XCAP::Path> path(new XCAP::Path (root_str, "resource-lists",
					       user_str));
    path->set_credentials (username_str, password_str);
    path = path->build_child ("resource-lists");

    for (xmlNodePtr child = root->children; child != NULL; child = child->next)
      if (child->type == XML_ELEMENT_NODE
	  && child->name != NULL
	  && xmlStrEqual (BAD_CAST ("list"), child->name)) {

	gmref_ptr<List> list(new List (core, path, pos, "", child));
	list->entry_added.connect (sigc::mem_fun (this, &RL::Heap::on_entry_added));
	list->entry_updated.connect (sigc::mem_fun (this, &RL::Heap::on_entry_updated));
	list->entry_removed.connect (sigc::mem_fun (this, &RL::Heap::on_entry_removed));
	list->publish ();
	lists.push_back (list);
	pos++;
	continue;
      }
  }
}

void
RL::Heap::push_presence (const std::string uri_,
			 const std::string presence)
{
  for (std::list<gmref_ptr<List> >::iterator iter = lists.begin ();
       iter != lists.end ();
       ++iter)
    (*iter)->push_presence (uri_, presence);
}

void
RL::Heap::push_status (const std::string uri_,
		       const std::string status)
{
  for (std::list<gmref_ptr<List> >::iterator iter = lists.begin ();
       iter != lists.end ();
       ++iter)
    (*iter)->push_status (uri_, status);
}

void
RL::Heap::on_entry_added (gmref_ptr<Entry> entry)
{
  presentity_added.emit (*entry);
}

void
RL::Heap::on_entry_updated (gmref_ptr<Entry> entry)
{
  presentity_updated.emit (*entry);
}

void
RL::Heap::on_entry_removed (gmref_ptr<Entry> entry)
{
  presentity_removed.emit (*entry);
}