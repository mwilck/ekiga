
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
 *                         rlist-cluster.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : resource-list cluster implementation
 *
 */

#include "config.h"
#include <glib/gi18n.h>

#include "rl-cluster.h"

#include "form-request-simple.h"
#include "presence-core.h"

#include <iostream>

#define RL_KEY "resource-lists"

RL::Cluster::Cluster (Ekiga::ServiceCore& core_): core(core_), doc()
{
  boost::shared_ptr<Ekiga::PresenceCore> presence_core = core.get<Ekiga::PresenceCore> ("presence-core");

  presence_core->presence_received.connect (boost::bind (&RL::Cluster::on_presence_received, this, _1, _2));
  presence_core->status_received.connect (boost::bind (&RL::Cluster::on_status_received, this, _1, _2));
  contacts_settings = boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (CONTACTS_SCHEMA));
  std::string raw = contacts_settings->get_string (RL_KEY);

  if (!raw.empty ()) {

    doc = boost::shared_ptr<xmlDoc> (xmlRecoverMemory (raw.c_str (), raw.length ()), xmlFreeDoc);
    if ( !doc)
      doc = boost::shared_ptr<xmlDoc> (xmlNewDoc (BAD_CAST "1.0"), xmlFreeDoc);

    xmlNodePtr root = xmlDocGetRootElement (doc.get ());
    if (root == NULL) {

      root = xmlNewDocNode (doc.get (), NULL, BAD_CAST "list", NULL);
      xmlDocSetRootElement (doc.get (), root);
    } else {

      for (xmlNodePtr child = root->children;
	   child != NULL;
	   child = child->next)
	if (child->type == XML_ELEMENT_NODE
	    && child->name != NULL
	    && xmlStrEqual (BAD_CAST "entry", child->name))
	  add (child);
    }

  } else {

    doc = boost::shared_ptr<xmlDoc> (xmlNewDoc (BAD_CAST "1.0"), xmlFreeDoc);
    xmlNodePtr root = xmlNewDocNode (doc.get (), NULL, BAD_CAST "list", NULL);
    xmlDocSetRootElement (doc.get (), root);
    add ("https://xcap.sipthor.net/xcap-root", "alice", "123", "alice@example.com", "XCAP Test", false); // FIXME: remove
  }
}

RL::Cluster::~Cluster ()
{
}

bool
RL::Cluster::populate_menu (Ekiga::MenuBuilder& builder)
{
  builder.add_action ("add", _("Add resource list"),
		      boost::bind (&RL::Cluster::new_heap, this,
				   "", "", "", "", "", false));
  return true;
}

void
RL::Cluster::add (xmlNodePtr node)
{
  HeapPtr heap (new Heap (core, doc, node));

  common_add (heap);
}

void
RL::Cluster::add (const std::string uri,
		  const std::string username,
		  const std::string password,
		  const std::string user,
		  const std::string name,
		  bool writable)
{
  HeapPtr heap (new Heap (core, doc, name, uri, user, username, password, writable));
  xmlNodePtr root = xmlDocGetRootElement (doc.get ());

  xmlAddChild (root, heap->get_node ());

  save ();
  common_add (heap);
}

void
RL::Cluster::common_add (HeapPtr heap)
{
  add_heap (heap);

  // FIXME: here we should ask for presence for the heap...

  heap->trigger_saving.connect (boost::bind (&RL::Cluster::save, this));
}

void
RL::Cluster::save () const
{
  xmlChar* buffer = NULL;
  int bsize = 0;

  xmlDocDumpMemory (doc.get (), &buffer, &bsize);

  contacts_settings->set_string (RL_KEY, (const char *)buffer);

  xmlFree (buffer);
}

void
RL::Cluster::new_heap (const std::string name,
		       const std::string uri,
		       const std::string username,
		       const std::string password,
		       const std::string user,
		       bool writable)
{
  boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&RL::Cluster::on_new_heap_form_submitted, this, _1, _2, _3)));

  request->title (_("Add new resource-list"));
  request->instructions (_("Please fill in this form to add a new "
			   "contact list to ekiga's remote roster"));
  request->text ("name", _("Name:"), name, std::string ());
  request->text ("uri", _("Address:"), uri, std::string ());
  request->boolean ("writable", _("Writable:"), writable);
  request->text ("username", _("Username:"), username, std::string ());
  request->text ("password", _("Password:"), password, std::string (), Ekiga::FormVisitor::PASSWORD);
  request->text ("user", _("User:"), user, std::string ());

  questions (request);
}

bool
RL::Cluster::on_new_heap_form_submitted (bool submitted,
					 Ekiga::Form &result,
                                         std::string &/*error*/)
{
  if (!submitted)
    return false;

  const std::string name = result.text ("name");
  const std::string uri = result.text ("uri");
  const std::string username = result.text ("username");
  const std::string password = result.text ("password");
  const std::string user = result.text ("user");
  bool writable = result.boolean ("writable");

  add (name, uri, username, password, user, writable);

  return true;
}


void
RL::Cluster::on_presence_received (std::string uri,
				   std::string presence)
{
  for (iterator iter = begin ();
       iter != end ();
       ++iter) {

    (*iter)->push_presence (uri, presence);
  }
}

void
RL::Cluster::on_status_received (std::string uri,
				 std::string status)
{
  for (iterator iter = begin ();
       iter != end ();
       ++iter) {

    (*iter)->push_status (uri, status);
  }
}
