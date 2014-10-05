
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
 *                         rl-presentity.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of a presentity in a resource-list
 *
 */

#include <algorithm>
#include <set>

#include <glib/gi18n.h>

#include "form-request-simple.h"
#include "robust-xml.h"
#include "xcap-core.h"

#include "rl-presentity.h"

/* at one point we will return a smart pointer on this... and if we don't use
 * a false smart pointer, we will crash : the reference count isn't embedded!
 */
struct null_deleter
{
    void operator()(void const *) const
    {
    }
};


RL::Presentity::Presentity (Ekiga::ServiceCore &services_,
			    boost::shared_ptr<XCAP::Path> path_,
			    boost::shared_ptr<xmlDoc> doc_,
			    xmlNodePtr node_,
			    bool writable_) :
  services(services_), doc(doc_), node(node_), writable(writable_),
  name_node(NULL), presence("unknown"), status("")
{
  boost::shared_ptr<Ekiga::PresenceCore> presence_core(services.get<Ekiga::PresenceCore> ("presence-core"));
  xmlChar *xml_str = NULL;
  xmlNsPtr ns = xmlSearchNsByHref (doc.get (), node,
                                   BAD_CAST "http://www.ekiga.org");

  if (ns == NULL) {

    // FIXME: we should handle the case, even if it shouldn't happen
  }

  xml_str = xmlGetProp (node, BAD_CAST "uri");
  if (xml_str != NULL) {

    uri = (const char *)xml_str;
    path = path_->build_child_with_attribute ("entry", "uri", uri);
    xmlFree (xml_str);
  } else {

    // FIXME: we should handle the case, even if it shouldn't happen

  }

  for (xmlNodePtr child = node->children ;
       child != NULL ;
       child = child->next) {

    if (child->type == XML_ELEMENT_NODE
        && child->name != NULL) {

      if (xmlStrEqual (BAD_CAST ("display-name"), child->name)) {

        name_node = child;
        continue;
      }
      else if (xmlStrEqual (BAD_CAST ("group"), child->name)
               && child->ns == ns) {

        xml_str = xmlNodeGetContent (child);
        if (xml_str != NULL)
          group_nodes[(const char *)xml_str] = child;
        else
          group_nodes[""] = child;
        xmlFree (xml_str);
        continue;
      }
    }
  }

  for (std::map<std::string, xmlNodePtr>::const_iterator iter
         = group_nodes.begin ();
       iter != group_nodes.end ();
       iter++)
    groups.insert (iter->first);

  presence_core->fetch_presence (uri);
}

RL::Presentity::~Presentity ()
{
}


const std::string
RL::Presentity::get_name () const
{
  std::string result;

  if (name_node != NULL) {

    xmlChar* str = xmlNodeGetContent (name_node);
    if (str != NULL) {

      result = ((const char*)str);
      xmlFree (str);
    }
  } else {

    result = _("Unnamed");
  }

  return result;
}


const std::string
RL::Presentity::get_presence () const
{
  return presence;
}


const std::string
RL::Presentity::get_status () const
{
  return status;
}


const std::set<std::string>
RL::Presentity::get_groups () const
{
  return groups;
}


const std::string
RL::Presentity::get_uri () const
{
  return uri;
}


void
RL::Presentity::set_presence (const std::string _presence)
{
  presence = _presence;
  updated ();
}

void
RL::Presentity::set_status (const std::string _status)
{
  status = _status;
  updated ();
}


bool
RL::Presentity::has_uri (const std::string _uri) const
{
  return _uri == get_uri ();
}

bool
RL::Presentity::populate_menu (Ekiga::MenuBuilder &builder)
{
  bool populated = false;
  boost::shared_ptr<Ekiga::PresenceCore> presence_core(services.get<Ekiga::PresenceCore> ("presence-core"));

  populated = presence_core->populate_presentity_menu (PresentityPtr (this, null_deleter ()), uri, builder);

  if (writable) {

    if (populated)
      builder.add_separator ();

    builder.add_action ("edit", _("_Edit"),
			boost::bind (&RL::Presentity::edit_presentity, this));
    builder.add_action ("remove", _("_Remove"),
			boost::bind (&RL::Presentity::remove, this));
  }

  return true;
}


void
RL::Presentity::edit_presentity ()
{
  boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&RL::Presentity::edit_presentity_form_submitted, this, _1, _2, _3)));

  // FIXME: we should be able to know all groups in the heap
  std::set<std::string> all_groups = groups;

  request->title (_("Edit remote contact"));
  request->instructions (_("Please fill in this form to change an existing "
                           "contact on a remote server"));
  request->text ("name", _("Name:"), get_name (), std::string ());
  request->text ("uri", _("Address:"), uri, std::string ());

  request->editable_set ("groups", _("Choose groups:"),
                         groups, all_groups);

  questions (request);
}


bool
RL::Presentity::edit_presentity_form_submitted (bool submitted,
						Ekiga::Form &result,
                                                std::string &/*error*/)
{
  if (!submitted)
    return false;

  const std::string new_name = result.text ("name");
  const std::string new_uri = result.text ("uri");
  const std::set<std::string> new_groups = result.editable_set ("groups");
  std::map<std::string, xmlNodePtr> future_group_nodes;
  xmlNsPtr ns = xmlSearchNsByHref (node->doc, node,
                                   BAD_CAST "http://www.ekiga.org");
  bool reload = false;

  robust_xmlNodeSetContent (node, &name_node, "name", new_name);

  if (uri != new_uri) {

    xmlSetProp (node, (const xmlChar*)"uri", (const xmlChar*)uri.c_str ());
    boost::shared_ptr<Ekiga::PresenceCore> presence_core(services.get<Ekiga::PresenceCore> ("presence-core"));
    presence_core->unfetch_presence (uri);
    reload = true;
  }

  for (std::map<std::string, xmlNodePtr>::const_iterator iter
         = group_nodes.begin ();
       iter != group_nodes.end () ;
       iter++) {

    if (new_groups.find (iter->first) == new_groups.end ()) {

      xmlUnlinkNode (iter->second);
      xmlFreeNode (iter->second);
    }
    else {
      future_group_nodes[iter->first] = iter->second;
    }
  }

  for (std::set<std::string>::const_iterator iter = new_groups.begin ();
       iter != new_groups.end ();
       iter++) {

    if (std::find (groups.begin (), groups.end (), *iter) == groups.end ())
      future_group_nodes[*iter] = xmlNewChild (node, ns,
					       BAD_CAST "group",
					       BAD_CAST robust_xmlEscape (node->doc, *iter).c_str ());
  }

  group_nodes = future_group_nodes;
  groups = new_groups;

  save (reload);

  return true;
}

void
RL::Presentity::save (bool reload)
{
  xmlBufferPtr buffer = xmlBufferCreate ();
  int result = xmlNodeDump (buffer, node->doc, node, 0, 0);

  if (result >= 0) {

    boost::shared_ptr<XCAP::Core> xcap = services.get<XCAP::Core> ("xcap-core");
    xcap->write (path, "application/xcap-el+xml",
                 (const char*)xmlBufferContent (buffer),
                 boost::bind (&RL::Presentity::save_result, this, _1, reload));
  }

  xmlBufferFree (buffer);

}

void
RL::Presentity::remove ()
{
  xmlUnlinkNode (node);
  xmlFreeNode (node);
  boost::shared_ptr<Ekiga::PresenceCore> presence_core = services.get<Ekiga::PresenceCore> ("presence-core");

  presence_core->unfetch_presence (uri);

  boost::shared_ptr<XCAP::Core> xcap = services.get<XCAP::Core> ("xcap-core");
  xcap->erase (path,
               boost::bind (&RL::Presentity::erase_result, this, _1));
}

void
RL::Presentity::save_result (std::string error,
			     bool reload)
{
  if ( !error.empty ()) {

    // FIXME: do better
    std::cout << "XCAP error: " << error << std::endl;
    trigger_reload ();
  } else {

    if (reload)
      trigger_reload ();
    else
      updated ();
  }
}

void
RL::Presentity::erase_result (std::string error)
{
  if ( !error.empty ()) {

    // FIXME: do better
    std::cout << "XCAP error: " << error << std::endl;
  }

  trigger_reload ();
}
