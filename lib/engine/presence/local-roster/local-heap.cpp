
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
 *                         local-heap.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of the heap of the local roster
 *
 */

#include <iostream>
#include <set>

#include "config.h"

#include "gmconf.h"
#include "form-request-simple.h"

#include "local-heap.h"

#define KEY "/apps/" PACKAGE_NAME "/contacts/roster"

/*
 * Helpers
 */
static const std::list<std::string>
split_on_commas (const std::string str)
{
  std::list<std::string> result;
  std::string::size_type len = str.length ();
  std::string::size_type start = str.find_first_not_of (',', 0);
  std::string::size_type stop = str.find_first_of(',', start);

  while (std::string::npos != start
         || std::string::npos != stop) {

    result.push_back (str.substr (start, stop - start));
    start = str.find_first_not_of (',', stop);
    stop = str.find_first_of(',', start);
  }
  if (start < len)
    result.push_back (str.substr (start, len - start));

  return result;
}


/*
 * Public API
 */
Local::Heap::Heap (Ekiga::ServiceCore &_core): core (_core), doc (NULL)
{
  xmlNodePtr root;

  presence_core = dynamic_cast<Ekiga::PresenceCore*>(core.get ("presence-core"));

  const gchar *c_raw = gm_conf_get_string (KEY);

  // Build the XML document representing the contacts list from the configuration
  if (c_raw != NULL) {

    const std::string raw = c_raw;
    doc = xmlRecoverMemory (raw.c_str (), raw.length ());

    root = xmlDocGetRootElement (doc);
    if (root == NULL) {
      root = xmlNewDocNode (doc, NULL, BAD_CAST "list", NULL);
      xmlDocSetRootElement (doc, root);
    }

    if (root->children) {

      for (xmlNodePtr child = root->children; child != NULL; child = child->next) {
        if (child->type == XML_ELEMENT_NODE
            && child->name != NULL
            && xmlStrEqual (BAD_CAST ("entry"), child->name))
          add (child);
      }
    }
  }
  // Or create a new XML document
  else {

    doc = xmlNewDoc (BAD_CAST "1.0");
    root = xmlNewDocNode (doc, NULL, BAD_CAST "list", NULL);
  }
}


Local::Heap::~Heap ()
{
  if (doc != NULL)
    xmlFreeDoc (doc);
}


const std::string
Local::Heap::get_name () const
{
  return _("Local Roster");
}


bool
Local::Heap::populate_menu (Ekiga::MenuBuilder &builder)
{
  Ekiga::UI *ui = dynamic_cast<Ekiga::UI*>(core.get ("ui"));
  bool populated = false;

  if (ui != NULL) {

    builder.add_action ("new",
                        _("New contact"),
                        sigc::bind (sigc::mem_fun (this, &Local::Heap::build_new_presentity_form), "", ""));
    populated = true;
  }

  return populated;
}


bool
Local::Heap::has_presentity_with_uri (const std::string uri) const
{
  bool result = false;

  for (const_iterator iter = begin ();
       iter != end () && result != true;
       iter++)
    result = (iter->get_uri () == uri);

  return result;
}


const std::list<std::string>
Local::Heap::existing_groups () const
{
  std::list<std::string> result;
  std::set<std::string> ensemble; // FIXME: awful, but the api is broken

  for (const_iterator iter = begin ();
       iter != end ();
       iter++) {

    std::list<std::string> groups = iter->get_groups ();
    ensemble.insert (groups.begin (), groups.end ());
  }

  result.insert (result.begin (), ensemble.begin (), ensemble.end ());

  return result;
}


void
Local::Heap::build_new_presentity_form (const std::string name,
					const std::string uri)
{
  Ekiga::UI *ui = dynamic_cast<Ekiga::UI*>(core.get ("ui"));

  if (ui != NULL && !has_presentity_with_uri (uri)) {

    Ekiga::FormRequestSimple request;
    std::list<std::string> groups = existing_groups ();
    std::map<std::string, std::string> choices;

    request.title (_("Add to local roster"));
    request.instructions ("Please fill in this form to add a new contact "
			  "to ekiga's internal roster");
    request.text ("name", _("Name:"), name);
    if (presence_core->is_supported_uri (uri)) {

      request.hidden ("good-uri", "yes");
      request.hidden ("uri", uri);
    }
    else {

      request.hidden ("good-uri", "no");
      request.text ("uri", "Address", uri);
    }

    for (std::list<std::string>::const_iterator iter = groups.begin ();
	 iter != groups.end ();
	 iter++)
      choices[*iter] = *iter;

    request.multiple_list ("old_groups",
			   _("Put contact in groups:"),
			   std::list<std::string>(), choices, true);

    request.submitted.connect (sigc::mem_fun (this, &Local::Heap::new_presentity_form_submitted));

    ui->run_form_request (request);
  }
}


/*
 * Private API
 */
void
Local::Heap::add (xmlNodePtr node)
{
  Presentity *presentity = NULL;

  presentity = new Presentity (core, node);

  common_add (*presentity);
}


void
Local::Heap::add (const std::string name,
		  const std::string uri,
		  const std::list<std::string> groups)
{
  Presentity *presentity = NULL;
  xmlNodePtr root = NULL;

  root = xmlDocGetRootElement (doc);
  presentity = new Presentity (core, name, uri, groups);

  xmlAddChild (root, presentity->get_node ());

  common_add (*presentity);
}


void
Local::Heap::common_add (Presentity &presentity)
{
  // Add the presentity to the Ekiga::Heap
  add_presentity (presentity);

  // Fetch presence
  presence_core->fetch_presence (presentity.get_uri ());

  // Connect the GmConf::Presentity private signals.
  presentity.save_me.connect (sigc::mem_fun (this, &Local::Heap::save));
  presentity.remove_me.connect (sigc::bind (sigc::mem_fun (this, &Local::Heap::remove), &presentity));
}


void
Local::Heap::remove (Presentity *presentity)
{
  presence_core->unfetch_presence (presentity->get_uri ());

  remove_presentity (*presentity);

  save ();
}


void
Local::Heap::save () const
{
  xmlChar *buffer = NULL;
  int size = 0;

  xmlDocDumpMemory (doc, &buffer, &size);

  gm_conf_set_string (KEY, (const char *)buffer);

  xmlFree (buffer);
}


void
Local::Heap::new_presentity_form_submitted (Ekiga::Form &result)
{
  try {

    const std::string name = result.text ("name");
    const std::string good_uri = result.hidden ("good-uri");
    std::string uri;
    const std::list<std::string> old_groups = result.multiple_list ("old_groups");
    //const std::list<std::string> new_groups =  split_on_commas (result.text ("new_groups"));
    std::set<std::string> groups_set;

    groups_set.insert (old_groups.begin (), old_groups.end ());
    //groups_set.insert (new_groups.begin (), new_groups.end ());

    if (good_uri == "yes")
      uri = result.hidden ("uri");
    else
      uri = result.text ("uri");

    if (presence_core->is_supported_uri (uri)
	&& !has_presentity_with_uri (uri)) {

      add (name, uri, std::list<std::string>(groups_set.begin (),
					     groups_set.end ()));
      save ();
    } else {

      Ekiga::UI *ui = dynamic_cast<Ekiga::UI*>(core.get ("ui"));
      Ekiga::FormRequestSimple request;

      result.visit (request);
      if (!presence_core->is_supported_uri (uri))
	request.error ("You supplied an unsupported address");
      else
	request.error ("You already have a contact with this address!");
      request.submitted.connect (sigc::mem_fun (this, &Local::Heap::new_presentity_form_submitted));
      ui->run_form_request (request);
    }
  } catch (Ekiga::Form::not_found) {

#ifdef __GNUC__
    std::cerr << "Invalid form submitted to "
	      << __PRETTY_FUNCTION__ << std::endl;
#endif
  }
}
