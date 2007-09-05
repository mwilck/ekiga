
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
 *                         local-presentity.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of a presentity for the local roster
 *
 */

#include <iostream>
#include <set>

#include "config.h"

#include "ui.h"
#include "form-request-simple.h"
#include "local-cluster.h"

#include "local-presentity.h"

/*
 * Helpers
 */
static const std::set<std::string>
split_on_commas (const std::string str)
{
  std::set<std::string> result;
  std::string::size_type len = str.length ();
  std::string::size_type start = str.find_first_not_of (',', 0);
  std::string::size_type stop = str.find_first_of(',', start);

  while (std::string::npos != start
         || std::string::npos != stop) {

    result.insert (str.substr (start, stop - start));
    start = str.find_first_not_of (',', stop);
    stop = str.find_first_of(',', start);
  }

  if (start < len)
    result.insert (str.substr (start, len - start));

  return result;
}


/*
 * Public API
 */
Local::Presentity::Presentity (Ekiga::ServiceCore &_core,
			       xmlNodePtr _node) :
  core(_core), node(_node), presence("presence-unknown")
{
  xmlChar *xml_str = NULL;

  presence_core = dynamic_cast<Ekiga::PresenceCore*>(core.get ("presence-core"));

  xml_str = xmlGetProp (node, (const xmlChar *)"uri");
  uri = (const char *)xml_str;
  xmlFree (xml_str);

  for (xmlNodePtr child = node->children ;
       child != NULL ;
       child = child->next) {

    if (child->type == XML_ELEMENT_NODE
        && child->name != NULL) {

      if (xmlStrEqual (BAD_CAST ("name"), child->name)) {

        xml_str = xmlNodeGetContent (child);
        name = (const char *)xml_str;
	name_node = child;
        xmlFree (xml_str);
      }

      if (xmlStrEqual (BAD_CAST ("group"), child->name)) {

        xml_str = xmlNodeGetContent (child);
	group_nodes[(const char *)xml_str] = child;
        xmlFree (xml_str);
      }
    }
  }

  for (std::map<std::string, xmlNodePtr>::const_iterator iter
	 = group_nodes.begin ();
       iter != group_nodes.end ();
       iter++)
    groups.insert (iter->first);
}


Local::Presentity::Presentity (Ekiga::ServiceCore &_core,
			       const std::string _name,
			       const std::string _uri,
			       const std::set<std::string> _groups) :
  core(_core), name(_name), uri(_uri),
  presence("presence-unknown"), groups(_groups)
{
  presence_core = dynamic_cast<Ekiga::PresenceCore*>(core.get ("presence-core"));

  node = xmlNewNode (NULL, BAD_CAST "entry");
  xmlSetProp (node, BAD_CAST "uri", BAD_CAST uri.c_str ());
  name_node = xmlNewChild (node, NULL,
			   BAD_CAST "name", BAD_CAST name.c_str ());
  for (std::set<std::string>::const_iterator iter = groups.begin ();
       iter != groups.end ();
       iter++)
    group_nodes[*iter] = xmlNewChild (node, NULL,
				      BAD_CAST "group",
				      BAD_CAST iter->c_str ());
}


Local::Presentity::~Presentity ()
{
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}


const std::string
Local::Presentity::get_name () const
{
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


const std::string
Local::Presentity::get_avatar () const
{
  return avatar;
}


const std::set<std::string>
Local::Presentity::get_groups () const
{
  return groups;
}


const std::string
Local::Presentity::get_uri () const
{
  return uri;
}


void
Local::Presentity::set_presence (const std::string _presence)
{
  presence = _presence;
  updated.emit ();
}

void
Local::Presentity::set_status (const std::string _status)
{
  status = _status;
  updated.emit ();
}


bool
Local::Presentity::populate_menu (Ekiga::MenuBuilder &builder)
{
  Ekiga::UI *ui = dynamic_cast<Ekiga::UI*>(core.get ("ui"));
  bool populated = false;

  populated = presence_core->populate_presentity_menu (uri, builder);

  if (ui != NULL) {

    if (populated)
      builder.add_separator ();
    builder.add_action ("edit", _("_Edit"),
			sigc::mem_fun (this, &Local::Presentity::edit_presentity));
    builder.add_action ("remove", _("_Remove"),
			sigc::mem_fun (this, &Local::Presentity::remove));
    populated = true;
  }

  return populated;
}


xmlNodePtr
Local::Presentity::get_node () const
{
  return node;
}


void
Local::Presentity::edit_presentity ()
{
  Ekiga::UI *ui = dynamic_cast<Ekiga::UI*>(core.get ("ui"));
  Cluster *cluster = dynamic_cast<Cluster*>(core.get ("local-cluster"));
  Ekiga::FormRequestSimple request;
  std::set<std::string> all_groups = cluster->existing_groups ();

  request.title (_("Edit roster element"));
  request.instructions (_("Please fill in this form to change an existing "
			  "element of ekiga's internal roster"));
  request.text ("name", _("Name:"), name);

  request.editable_set ("groups", _("Choose groups:"),
			groups, all_groups);

  request.submitted.connect (sigc::mem_fun (this, &Local::Presentity::edit_presentity_form_submitted));
  ui->run_form_request (request);
}


void
Local::Presentity::edit_presentity_form_submitted (Ekiga::Form &result)
{
  try {

    /* we first fetch all data before making any change, so if there's
     * a problem, we don't do anything */
    const std::string new_name = result.text ("name");
    const std::set<std::string> new_groups = result.editable_set ("groups");
    std::map<std::string, xmlNodePtr> future_group_nodes;

    name = new_name;
    xmlNodeSetContent (name_node,
		       xmlEncodeSpecialChars(name_node->doc,
					     BAD_CAST name.c_str ()));

    // the first loop looks at groups we were in : are we still in ?
    for (std::map<std::string, xmlNodePtr>::const_iterator iter
	   = group_nodes.begin ();
	 iter != group_nodes.end () ;
	 iter++) {

      if (new_groups.find (iter->first) == new_groups.end ()) {

	xmlUnlinkNode (iter->second);
	xmlFreeNode (iter->second);
      } else
	future_group_nodes[iter->first] = iter->second;
    }

    // the second loop looking for groups we weren't in but are now
    for (std::set<std::string>::const_iterator iter = new_groups.begin ();
	 iter != new_groups.end ();
	 iter++) {

      if (std::find (groups.begin (), groups.end (), *iter) == groups.end ())
	future_group_nodes[*iter] = xmlNewChild (node, NULL,
						 BAD_CAST "group",
						 BAD_CAST iter->c_str ());
    }

    // ok, now we know our groups
    group_nodes = future_group_nodes;
    groups = new_groups;

    updated.emit ();
    trigger_saving.emit ();
  } catch (Ekiga::Form::not_found) {
#ifdef __GNUC__
    std::cerr << "Invalid form submitted to "
	      << __PRETTY_FUNCTION__ << std::endl;
#endif
  }
}


void
Local::Presentity::remove ()
{
  presence_core->unfetch_presence (uri);

  xmlUnlinkNode (node);
  xmlFreeNode (node);

  removed.emit ();
  trigger_saving.emit ();
}
