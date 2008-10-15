
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
 *                         rl-heap.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : resource-list heap declaration
 *
 */

#ifndef __RL_HEAP_H__
#define __RL_HEAP_H__

#include "gmref.h"

#include "heap.h"
#include "xcap-core.h"

#include <libxml/tree.h>

#include "rl-list.h"

namespace RL {

  class Heap: public Ekiga::Heap
  {
  public:

    Heap (Ekiga::ServiceCore& core_,
	  xmlNodePtr node);

    /* name: the name of the Heap in the GUI
     * root: the XCAP root address
     * user: the user as XCAP user
     * username: the username on the HTTP server
     * password: the password on the HTTP server
     *
     * Don't complain to me(Snark) it's complex : read RFC4825 and cry with me
     *
     */
    Heap (Ekiga::ServiceCore& core_,
	  const std::string name_,
	  const std::string root_,
	  const std::string user_,
	  const std::string username_,
	  const std::string password_);

    ~Heap ();

    const std::string get_name () const;

    void visit_presentities (sigc::slot<bool, Ekiga::Presentity&> visitor);

    bool populate_menu (Ekiga::MenuBuilder& builder);

    bool populate_menu_for_group (std::string group,
				  Ekiga::MenuBuilder& builder);

    xmlNodePtr get_node () const;

    void push_presence (const std::string uri,
			const std::string presence);

    void push_status (const std::string uri,
		      const std::string status);

    sigc::signal<void> trigger_saving;

  private:

    Ekiga::ServiceCore& core;

    xmlNodePtr node;
    xmlNodePtr name;
    xmlNodePtr root;
    xmlNodePtr user;
    xmlNodePtr username;
    xmlNodePtr password;

    xmlDocPtr doc;

    std::list<gmref_ptr<List> > lists;

    void refresh ();

    void on_document_received (XCAP::Core::ResultType result,
			       std::string doc);

    void parse_doc (std::string doc);

    void on_entry_added (gmref_ptr<Entry> entry);
    void on_entry_updated (gmref_ptr<Entry> entry);
    void on_entry_removed (gmref_ptr<Entry> entry);
  };
};

#endif
