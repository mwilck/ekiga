
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras

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
 *                         history-heap.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of the heap of the call history
 *
 */

#include <iostream>
#include <set>

#include "config.h"

#include "gmconf.h"

#include "history-heap.h"

#define KEY "/apps/" PACKAGE_NAME "/contacts/call_history"

History::Heap::Heap (Ekiga::ServiceCore &_core) :
  core(_core), doc(NULL)
{
  xmlNodePtr root;

  presence_core
    = dynamic_cast<Ekiga::PresenceCore*>(core.get ("presence-core"));

  const gchar *c_raw = gm_conf_get_string (KEY);

  if (c_raw != NULL) {

    const std::string raw = c_raw;

    doc = xmlRecoverMemory (raw.c_str (), raw.length ());

    root = xmlDocGetRootElement (doc);
    
    if (root == NULL) {

      root = xmlNewNode (NULL, BAD_CAST "list");
      xmlDocSetRootElement (doc, root);
    }

    for (xmlNodePtr child = root->children;
	 child != NULL;
	 child = child->next)
      if (child->type == XML_ELEMENT_NODE
	  && child->name != NULL
	  && xmlStrEqual (BAD_CAST ("entry"), child->name))
        add (child);
  } else
    doc = xmlNewDoc (BAD_CAST "1.0");
}

History::Heap::~Heap ()
{
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
  if (doc != NULL)
    xmlFreeDoc (doc);
}

const std::string
History::Heap::get_name () const
{
  return "Call history";
}

void
History::Heap::add (xmlNodePtr node)
{
  Presentity *presentity = NULL;

  presentity = new Presentity (core, node);
  common_add (*presentity);
}

void
History::Heap::add (const std::string name,
		    const std::string uri,
		    const std::string status,
		    call_type c_t)
{
  Presentity *presentity = NULL;
  xmlNodePtr root = NULL;

  root = xmlDocGetRootElement (doc);
  presentity = new Presentity (core, name, uri, status, c_t);

  xmlAddChild (root, presentity->get_node ());

  common_add (*presentity);
}

void
History::Heap::common_add (Presentity &presentity)
{
  add_presentity (presentity);
  presence_core->fetch_presence (presentity.get_uri ());
}

bool
History::Heap::populate_menu (Ekiga::MenuBuilder &builder)
{
  builder.add_action ("clear",
		      "Clear", sigc::mem_fun (this,
					      &History::Heap::clear));
  return true;
}

bool
History::Heap::has_presentity_with_uri (const std::string uri) const
{
  bool result = false;

  for (const_iterator iter = begin ();
       iter != end () && result != true;
       iter++)
    result = (iter->get_uri () == uri);

  return result;
}

const std::set<std::string>
History::Heap::existing_groups () const
{
  // here it's more logical to lie
  return std::set<std::string> ();
}

void
History::Heap::save () const
{
  xmlChar *buffer = NULL;
  int size = 0;

  xmlDocDumpMemory (doc, &buffer, &size);

  gm_conf_set_string (KEY, (const char *)buffer);

  xmlFree (buffer);
}

void
History::Heap::clear ()
{

  while (begin () != end ()) {

    presence_core->unfetch_presence (begin ()->get_uri ());
    remove_presentity (*begin ());
  }
  save ();
}
