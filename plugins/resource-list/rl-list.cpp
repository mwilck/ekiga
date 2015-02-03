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
 *                         rl-list.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : resource-list list class
 *
 */

#include "config.h"

#include <glib.h>
#include <glib/gi18n-lib.h>

#include "rl-list.h"

class RL::ListImpl
{
public: // no need to make anything private

  ListImpl (Ekiga::ServiceCore& core_,
	    boost::shared_ptr<XCAP::Path> path_,
	    int pos,
	    const std::string group_,
	    xmlNodePtr node_);

  ~ListImpl ();

  bool has_name (const std::string name) const;

  void push_presence (const std::string uri_,
		      const std::string presence);

  void push_status (const std::string uri_,
		    const std::string status);

  std::string compute_path () const;

  void flush ();

  void refresh ();

  void on_xcap_answer (bool error,
		       std::string value);

  void parse ();

  /* data for itself */

  Ekiga::ServiceCore& core;

  boost::shared_ptr<XCAP::Path> path;
  int position;

  std::string group;

  boost::shared_ptr<xmlDoc> doc;
  xmlNodePtr node;

  xmlNodePtr name_node;
  std::string position_name;
  std::string display_name;

  /* make the world know what we have */
  bool visit_presentities (boost::function1<bool, Ekiga::Presentity&> visitor) const;

  void publish () const;

  boost::signals2::signal<void(boost::shared_ptr<Entry>)> entry_added;
  boost::signals2::signal<void(boost::shared_ptr<Entry>)> entry_updated;
  boost::signals2::signal<void(boost::shared_ptr<Entry>)> entry_removed;


  /* data for its children */
  typedef enum { LIST, ENTRY } ChildType;

  std::list<ChildType> ordering;
  std::list<boost::shared_ptr<List> > lists;
  std::list<std::pair<boost::shared_ptr<Entry>, std::list<boost::signals2::connection> > > entries;
};


/* implementation of the List class */

RL::List::List (Ekiga::ServiceCore& core_,
		boost::shared_ptr<XCAP::Path> path_,
		int pos,
		const std::string group_,
		xmlNodePtr node_)
{
  impl = new ListImpl (core_, path_, pos, group_, node_);
  impl->entry_added.connect (entry_added);
  impl->entry_updated.connect (entry_updated);
  impl->entry_removed.connect (entry_removed);
}

RL::List::~List ()
{
  delete impl;
}

void
RL::List::flush ()
{
  return impl->flush ();
}

bool
RL::List::has_name (const std::string name) const
{
  return impl->has_name (name);
}

void
RL::List::push_presence (const std::string uri_,
			 const std::string presence)
{
  impl->push_presence (uri_, presence);
}

void
RL::List::push_status (const std::string uri_,
		       const std::string status)
{
  impl->push_status (uri_, status);
}

bool
RL::List::visit_presentities (boost::function1<bool, Ekiga::Presentity&> visitor) const
{
  return impl->visit_presentities (visitor);
}

void
RL::List::publish () const
{
  impl->publish ();
}

/* implementation of the ListImpl class */

RL::ListImpl::ListImpl (Ekiga::ServiceCore& core_,
			boost::shared_ptr<XCAP::Path> path_,
			int pos,
			const std::string group_,
			xmlNodePtr node_):
  core(core_), position(pos), group(group_), doc(), node(node_)
{
  {
    gchar* raw = NULL;

    if ( !group_.empty ()) {

      /* Translators: #%d - ordinal number */
      raw = g_strdup_printf (_("%s / List #%d"),
			     group_.c_str (), position);
    } else {

      /* Translators: #%d - ordinal number */
      raw = g_strdup_printf (_("List #%d"), position);
    }
    position_name = raw;
    g_free (raw);
  }

  display_name = position_name; // will be set to better when we get the data

  if (node != NULL) {

    xmlChar* str = xmlGetProp (node, BAD_CAST "name");

    if (str != NULL) {

      path = path_->build_child_with_attribute ("list", "name",
						(const char*)str);
      xmlFree (str);
    } else {

      path = path_->build_child_with_position ("list", position);
    }
    parse ();
  } else {

    path = path_;
    refresh ();
  }
}

RL::ListImpl::~ListImpl ()
{
}

bool
RL::ListImpl::has_name (const std::string name) const
{
  return (name == position_name) || (name == display_name);
}

void
RL::ListImpl::flush ()
{
  ordering.clear ();

  for (std::list<boost::shared_ptr<List> >::iterator iter = lists.begin ();
       iter != lists.end ();
       ++iter)
    (*iter)->flush ();
  lists.clear ();

  for (std::list<std::pair<boost::shared_ptr<Entry>, std::list<boost::signals2::connection> > >::iterator iter = entries.begin ();
       iter != entries.end ();
       ++iter) {

    iter->first->removed ();
    for (std::list<boost::signals2::connection>::iterator conn_iter
	   = iter->second.begin ();
	 conn_iter != iter->second.end ();
	 ++conn_iter)
      conn_iter->disconnect ();
  }
  entries.clear ();

  doc.reset ();
  node = NULL;
  name_node = NULL;
}

void
RL::ListImpl::refresh ()
{
  flush ();

  boost::shared_ptr<XCAP::Core> xcap = core.get<XCAP::Core> ("xcap-core");
  xcap->read (path, boost::bind (&RL::ListImpl::on_xcap_answer, this, _1, _2));
}

void
RL::ListImpl::on_xcap_answer (bool error,
			      std::string value)
{
  if (error) {

    // FIXME: how to properly tell the user?

  } else {

    doc = boost::shared_ptr<xmlDoc> (xmlRecoverMemory (value.c_str (), value.length ()), xmlFreeDoc);
    if ( !doc)
      doc = boost::shared_ptr<xmlDoc> (xmlNewDoc (BAD_CAST "1.0"), xmlFreeDoc);
    node = xmlDocGetRootElement (doc.get ());
    if (node == NULL
	|| node->name == NULL
	|| !xmlStrEqual (BAD_CAST "list", node->name)) {

      // FIXME : how to properly tell the user?
    } else {

      parse ();
    }
  }
}

void
RL::ListImpl::parse ()
{
  int list_pos = 1;
  int entry_pos = 1;

  for (xmlNodePtr child = node->children; child != NULL; child = child->next) {

    if (child->type == XML_ELEMENT_NODE
	&& child->name != NULL
	&& xmlStrEqual (BAD_CAST "display-name", child->name)) {

      xmlChar* str = xmlNodeGetContent (child);
      name_node = child;
      if (str != NULL) {

	if ( !group.empty ())
	  display_name = group + " / " + (const char*) str;
	else
	  display_name = (const char*) str;
	xmlFree (str);
      }
      break;
    }

  }

  for (xmlNodePtr child = node->children; child != NULL; child = child->next) {

    if (child->type == XML_ELEMENT_NODE
	&& child->name != NULL
	&& xmlStrEqual (BAD_CAST "list", child->name)) {

      boost::shared_ptr<List> list = boost::shared_ptr<List> (new List (core, path,
							list_pos, display_name,
							child));
      list->entry_added.connect (entry_added);
      list->entry_updated.connect (entry_updated);
      list->entry_removed.connect (entry_removed);
      lists.push_back (list);
      ordering.push_back (LIST);
      list_pos++;
      list->publish ();
      continue;
    }

    if (child->type == XML_ELEMENT_NODE
	&& child->name != NULL
	&& xmlStrEqual (BAD_CAST "entry", child->name)) {

      boost::shared_ptr<Entry> entry = boost::shared_ptr<Entry> (new Entry (core, path,
							    entry_pos,
							    display_name,
							    doc, child));
      std::list<boost::signals2::connection> conns;
      conns.push_back (entry->updated.connect (boost::bind (boost::ref (entry_updated), entry)));
      conns.push_back (entry->removed.connect (boost::bind (boost::ref (entry_removed), entry)));
      entries.push_back (std::pair<boost::shared_ptr<Entry>, std::list<boost::signals2::connection> > (entry, conns));
      ordering.push_back (ENTRY);
      entry_pos++;
      entry_added (entry);
      continue;
    }
  }
}

void
RL::ListImpl::push_presence (const std::string uri_,
			     const std::string presence)
{
  for (std::list<boost::shared_ptr<List> >::const_iterator iter = lists.begin ();
       iter != lists.end ();
       ++iter)
    (*iter)->push_presence (uri_, presence);

  for (std::list<std::pair<boost::shared_ptr<Entry>, std::list<boost::signals2::connection> > >::const_iterator iter = entries.begin ();
       iter != entries.end ();
       ++iter) {

    if (iter->first->get_uri () == uri_)
      iter->first->set_presence (presence);
  }
}

void
RL::ListImpl::push_status (const std::string uri_,
			   const std::string status)
{
  for (std::list<boost::shared_ptr<List> >::const_iterator iter = lists.begin ();
       iter != lists.end ();
       ++iter)
    (*iter)->push_status (uri_, status);

  for (std::list<std::pair<boost::shared_ptr<Entry>, std::list<boost::signals2::connection> > >::const_iterator iter = entries.begin ();
       iter != entries.end ();
       ++iter) {

    if (iter->first->get_uri () == uri_)
      iter->first->set_status (status);
  }
}

bool
RL::ListImpl::visit_presentities (boost::function1<bool, Ekiga::Presentity&> visitor) const
{
  bool go_on = true;

  for (std::list<boost::shared_ptr<List> >::const_iterator iter = lists.begin ();
       go_on && iter != lists.end ();
       ++iter)
    go_on = (*iter)->visit_presentities (visitor);

  for (std::list<std::pair<boost::shared_ptr<Entry>, std::list<boost::signals2::connection> > >::const_iterator iter = entries.begin ();
       go_on && iter != entries.end ();
       ++iter) {

    go_on = visitor (*(iter->first));
  }

  return go_on;
}

void
RL::ListImpl::publish () const
{
  for (std::list<boost::shared_ptr<List> >::const_iterator iter = lists.begin ();
       iter != lists.end ();
       ++iter)
    (*iter)->publish ();

  for (std::list<std::pair<boost::shared_ptr<Entry>, std::list<boost::signals2::connection> > >::const_iterator iter = entries.begin ();
       iter != entries.end ();
       ++iter) {

    entry_added (iter->first);
  }
}
