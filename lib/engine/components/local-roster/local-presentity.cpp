
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>

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
 *                         local-presentity.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of a presentity for the local roster
 *
 */

#include <algorithm>
#include <set>
#include <glib/gi18n.h>

#include "form-request-simple.h"
#include "local-cluster.h"
#include "robust-xml.h"
#include "local-presentity.h"


// remove leading and trailing spaces and tabs (useful for copy/paste)
// also, if no protocol specified, add leading "sip:"
std::string
canonize_uri (std::string uri)
{
  const size_t begin_str = uri.find_first_not_of (" \t");
  if (begin_str == std::string::npos)  // there is no content
    return "";

  const size_t end_str = uri.find_last_not_of (" \t");
  const size_t range = end_str - begin_str + 1;
  uri = uri.substr (begin_str, range);
  const size_t pos = uri.find (":");
  if (pos == std::string::npos)
    uri = uri.insert (0, "sip:");
  return uri;
}


/* at one point we will return a smart pointer on this... and if we don't use
 * a false smart pointer, we will crash : the reference count isn't embedded!
 */
struct null_deleter
{
    void operator()(void const *) const
    {
    }
};


/*
 * Public API
 */
Local::Presentity::Presentity (Ekiga::ServiceCore &_core,
			       boost::shared_ptr<xmlDoc> _doc,
			       xmlNodePtr _node) :
  core(_core), doc(_doc), node(_node), presence("unknown")
{
}


Local::Presentity::Presentity (Ekiga::ServiceCore &_core,
			       boost::shared_ptr<xmlDoc> _doc,
			       const std::string name,
			       const std::string uri,
			       const std::set<std::string> groups) :
  core(_core), doc(_doc), presence("unknown")
{
  node = xmlNewNode (NULL, BAD_CAST "entry");
  xmlSetProp (node, BAD_CAST "uri", BAD_CAST uri.c_str ());
  xmlSetProp (node, BAD_CAST "preferred", BAD_CAST "false");
  xmlNewChild (node, NULL,
	       BAD_CAST "name",
	       BAD_CAST robust_xmlEscape (node->doc,
					  name).c_str ());
  for (std::set<std::string>::const_iterator iter = groups.begin ();
       iter != groups.end ();
       iter++)
    xmlNewChild (node, NULL,
		 BAD_CAST "group",
		 BAD_CAST robust_xmlEscape (node->doc,
					    *iter).c_str ());
}


Local::Presentity::~Presentity ()
{
}


const std::string
Local::Presentity::get_name () const
{
  std::string name;
  xmlChar* xml_str = NULL;

  for (xmlNodePtr child = node->children ;
       child != NULL ;
       child = child->next) {

    if (child->type == XML_ELEMENT_NODE
        && child->name != NULL) {

      if (xmlStrEqual (BAD_CAST ("name"), child->name)) {

	xml_str = xmlNodeGetContent (child);
	if (xml_str != NULL) {

	  name = (const char*)xml_str;
	  xmlFree (xml_str);
	} else {

	  name = _("Unnamed");
	}
      }
    }
  }

  return name;
}


const std::string
Local::Presentity::get_presence () const
{
  return presence;
}


const std::string
Local::Presentity::get_status () const
{
  return status;
}


const std::set<std::string>
Local::Presentity::get_groups () const
{
  std::set<std::string> groups;

  for (xmlNodePtr child = node->children ;
       child != NULL ;
       child = child->next) {

    if (child->type == XML_ELEMENT_NODE
        && child->name != NULL) {

      if (xmlStrEqual (BAD_CAST ("group"), child->name)) {

	xmlChar* xml_str = xmlNodeGetContent (child);
	if (xml_str != NULL) {

	  groups.insert ((const char*) xml_str);
	  xmlFree (xml_str);
	}
      }
    }
  }

  return groups;
}


const std::string
Local::Presentity::get_uri () const
{
  std::string uri;
  xmlChar* xml_str = NULL;

  xml_str = xmlGetProp (node, BAD_CAST "uri");
  if (xml_str != NULL) {

    uri = (const char*)xml_str;
    xmlFree (xml_str);
  }

  return uri;
}

bool
Local::Presentity::has_uri (const std::string uri) const
{
  return uri == get_uri ();
}

void
Local::Presentity::set_presence (const std::string _presence)
{
  presence = _presence;
  updated ();
}

void
Local::Presentity::set_status (const std::string _status)
{
  status = _status;
  updated ();
}


bool
Local::Presentity::populate_menu (Ekiga::MenuBuilder &builder)
{
  bool populated = false;
  boost::shared_ptr<Ekiga::PresenceCore> presence_core = core.get<Ekiga::PresenceCore> ("presence-core");

  populated
    = presence_core->populate_presentity_menu (PresentityPtr(this, null_deleter ()),
					       get_uri (), builder);

  if (populated)
    builder.add_separator ();

  builder.add_action ("edit", _("_Edit"),
		      boost::bind (&Local::Presentity::edit_presentity, this));
  builder.add_action ("remove", _("_Remove"),
		      boost::bind (&Local::Presentity::remove, this));

  return true;
}


xmlNodePtr
Local::Presentity::get_node () const
{
  return node;
}


void
Local::Presentity::edit_presentity ()
{
  ClusterPtr cluster = core.get<Local::Cluster> ("local-cluster");
  boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&Local::Presentity::edit_presentity_form_submitted, this, _1, _2)));

  std::string name = get_name ();
  std::string uri = get_uri ();
  std::set<std::string> groups = get_groups ();
  std::set<std::string> all_groups = cluster->existing_groups ();

  request->title (_("Edit roster element"));
  request->instructions (_("Please fill in this form to change an existing "
			   "element of ekiga's internal roster"));
  request->text ("name", _("Name:"), name, _("Name of the contact, as shown in your roster"));
  request->text ("uri", _("Address:"), uri, _("Address, e.g. sip:xyz@ekiga.net; if you do not precise the host part, e.g. sip:xyz, then you can choose it by right-clicking on the contact in roster"));
  request->boolean ("preferred", _("Is a preferred contact"), is_preferred ());

  request->editable_set ("groups", _("Choose groups:"),
			 groups, all_groups);

  questions (request);
}


void
Local::Presentity::edit_presentity_form_submitted (bool submitted,
						   Ekiga::Form &result)
{
  if (!submitted)
    return;

  const std::string new_name = result.text ("name");
  const std::set<std::string> groups = get_groups ();
  const std::set<std::string> new_groups = result.editable_set ("groups");
  std::string new_uri = result.text ("uri");
  const std::string uri = get_uri ();
  bool preferred = result.boolean ("preferred");
  std::set<xmlNodePtr> nodes_to_remove;

  new_uri = canonize_uri (new_uri);

  for (xmlNodePtr child = node->children ;
       child != NULL ;
       child = child->next) {

    if (child->type == XML_ELEMENT_NODE
        && child->name != NULL) {

      if (xmlStrEqual (BAD_CAST ("name"), child->name)) {

	robust_xmlNodeSetContent (node, &child, "name", new_name);
      }
    }
  }

  if (uri != new_uri) {

    boost::shared_ptr<Ekiga::PresenceCore> presence_core = core.get<Ekiga::PresenceCore> ("presence-core");
    presence_core->unfetch_presence (uri);
    presence = "unknown";
    presence_core->fetch_presence (new_uri);
    xmlSetProp (node, (const xmlChar*)"uri", (const xmlChar*)new_uri.c_str ());
  }

  // the first loop looks at groups we were in : are we still in ?
  for (xmlNodePtr child = node->children ;
       child != NULL ;
       child = child->next) {

    if (child->type == XML_ELEMENT_NODE
        && child->name != NULL) {

      if (xmlStrEqual (BAD_CAST ("group"), child->name)) {

	xmlChar* xml_str = xmlNodeGetContent (child);

	if (xml_str != NULL) {

	  if (new_groups.find ((const char*) xml_str) == new_groups.end ()) {

	    nodes_to_remove.insert (child); // don't free what we loop on!
	  }
	  xmlFree (xml_str);
	}
      }
    }
  }

  // ok, now we can clean up!
  for (std::set<xmlNodePtr>::iterator iter = nodes_to_remove.begin ();
       iter != nodes_to_remove.end ();
       ++iter) {

    xmlUnlinkNode (*iter);
    xmlFreeNode (*iter);
  }

  // the second loop looking for groups we weren't in but are now
  for (std::set<std::string>::const_iterator iter = new_groups.begin ();
       iter != new_groups.end ();
       iter++) {

    if (std::find (groups.begin (), groups.end (), *iter) == groups.end ()) {
      xmlNewChild (node, NULL,
		   BAD_CAST "group",
		   BAD_CAST robust_xmlEscape (node->doc, *iter).c_str ());
    }
  }

  if (preferred) {

    xmlSetProp (node, BAD_CAST "preferred", BAD_CAST "true");
  } else {

    xmlSetProp (node, BAD_CAST "preferred", BAD_CAST "false");
  }

  updated ();
  trigger_saving ();
}


void
Local::Presentity::rename_group (const std::string old_name,
				 const std::string new_name)
{
  bool old_name_present = false;
  bool already_in_new_name = false;
  std::set<xmlNodePtr> nodes_to_remove;

  /* remove the old name's node
   * and check if we aren't already in the new name's group
   */
  for (xmlNodePtr child = node->children ;
       child != NULL ;
       child = child->next) {

    if (child->type == XML_ELEMENT_NODE
        && child->name != NULL) {

      if (xmlStrEqual (BAD_CAST ("group"), child->name)) {

	xmlChar* xml_str = xmlNodeGetContent (child);

	if (xml_str != NULL) {

	  if (!xmlStrcasecmp ((const xmlChar*)old_name.c_str (), xml_str)) {
	    nodes_to_remove.insert (child); // don't free what we loop on!
            old_name_present = true;
	  }

	  if (!xmlStrcasecmp ((const xmlChar*)new_name.c_str (), xml_str)) {
	    already_in_new_name = true;
	  }

	  xmlFree (xml_str);
	}
      }
    }
  }

  // ok, now we can clean up!
  for (std::set<xmlNodePtr>::iterator iter = nodes_to_remove.begin ();
       iter != nodes_to_remove.end ();
       ++iter) {

    xmlUnlinkNode (*iter);
    xmlFreeNode (*iter);
  }

  if (old_name_present && !already_in_new_name) {

    xmlNewChild (node, NULL,
		 BAD_CAST "group",
		 BAD_CAST robust_xmlEscape (node->doc,
					    new_name).c_str ());

  }

  updated ();
  trigger_saving ();
}


void
Local::Presentity::remove ()
{
  boost::shared_ptr<Ekiga::PresenceCore> presence_core = core.get<Ekiga::PresenceCore> ("presence-core");
  presence_core->unfetch_presence (get_uri ());

  xmlUnlinkNode (node);
  xmlFreeNode (node);

  trigger_saving ();
  removed ();
}

bool
Local::Presentity::is_preferred () const
{
  bool preferred = false;
  xmlChar* xml_str = xmlGetProp (node, (const xmlChar*)"preferred");

  if (xml_str != NULL) {

    if (xmlStrEqual (xml_str, BAD_CAST "true")) {

      preferred = true;
    } else {

      preferred = false;
    }
    xmlFree (xml_str);
  }

  return preferred;
}
