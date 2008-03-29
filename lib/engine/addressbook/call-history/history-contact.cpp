
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
 *                         history-contact.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of a call history entry
 *
 */

#include <iostream>

#include "history-contact.h"

History::Contact::Contact (Ekiga::ServiceCore &_core,
			   xmlNodePtr _node):
  core(_core), node(_node)
{
  xmlChar *xml_str;

  contact_core
    = dynamic_cast<Ekiga::ContactCore*>(core.get ("contact-core"));

  xml_str = xmlGetProp (node, (const xmlChar *)"type");
  if (xml_str != NULL)
    groups.insert ((const char *)xml_str);
  xmlFree (xml_str);

  xml_str = xmlGetProp (node, (const xmlChar *)"uri");
  if (xml_str != NULL)
    uri = (const char *)xml_str;
  xmlFree (xml_str);

  for (xmlNodePtr child = node->children ;
       child != NULL ;
       child = child->next) {

    if (child->type == XML_ELEMENT_NODE
        && child->name != NULL) {

      if (xmlStrEqual (BAD_CAST ("name"), child->name)) {

        xml_str = xmlNodeGetContent (child);
	if (xml_str != NULL)
	  name = (const char *)xml_str;
        xmlFree (xml_str);
      }

      if (xmlStrEqual (BAD_CAST ("status"), child->name)) {

        xml_str = xmlNodeGetContent (child);
	if (xml_str != NULL)
	  status = (const char *)xml_str;
        xmlFree (xml_str);
      }
    }
  }
}

History::Contact::Contact (Ekiga::ServiceCore &_core,
			   const std::string _name,
			   const std::string _uri,
			   const std::string _status,
			   call_type c_t):
  core(_core), name(_name), uri(_uri), status(_status)
{
  contact_core
    = dynamic_cast<Ekiga::ContactCore*>(core.get ("contact-core"));

  node = xmlNewNode (NULL, BAD_CAST "entry");

  xmlSetProp (node, BAD_CAST "uri", BAD_CAST uri.c_str ());
  xmlNewChild (node, NULL,
	       BAD_CAST "name", BAD_CAST name.c_str ());
  xmlNewChild (node, NULL,
	       BAD_CAST "status", BAD_CAST status.c_str ());

  switch (c_t) {

  case RECEIVED:

    xmlSetProp (node, BAD_CAST "type", BAD_CAST "Received");
    groups.insert ("Received");
    break;
  case PLACED:

    groups.insert ("Placed");
    break;
  case MISSED:

    groups.insert ("Missed");
    break;

  default:

    break;
  }
}

History::Contact::~Contact ()
{
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

const std::string
History::Contact::get_name () const
{
  return name;
}

const std::set<std::string>
History::Contact::get_groups () const
{
  return groups;
}

bool
History::Contact::populate_menu (Ekiga::MenuBuilder &builder)
{
  return contact_core->populate_contact_menu (*this, builder);
}

xmlNodePtr
History::Contact::get_node ()
{
  return node;
}

const std::map<std::string,std::string>
History::Contact::get_uris () const
{
  std::map<std::string,std::string> result;

  result[""] = uri;

  return result;
}

bool
History::Contact::is_found (std::string /*test*/) const
{
  /* FIXME */
  return true;
}
