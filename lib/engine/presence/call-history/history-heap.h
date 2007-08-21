
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
 *                         history-heap.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of the heap for the call history
 *
 */

#ifndef __HISTORY_HEAP_H__
#define __HISTORY_HEAP_H__

#include <libxml/tree.h>

#include "ui.h"
#include "heap-impl.h"
#include "history-presentity.h"

namespace History
{
  class Heap
    : public Ekiga::HeapImpl<Presentity, Ekiga::delete_presentity_management <Presentity> >
  {
  public:

    /* generic api */
    
    Heap (Ekiga::ServiceCore &_core);

    ~Heap ();

    const std::string get_name () const;

    bool populate_menu (Ekiga::MenuBuilder &);

    bool has_presentity_with_uri (const std::string uri) const;

    const std::set<std::string> existing_groups () const;

    /* more specific api */

    void add (const std::string name,
	      const std::string uri,
	      const std::string status,
	      call_type c_t);

    void clear ();

  private:

    void parse_entry (xmlNodePtr entry);

    void save () const;

    void add (xmlNodePtr node);

    void common_add (Presentity &presentity);

    Ekiga::ServiceCore &core;
    Ekiga::PresenceCore *presence_core;
    xmlDocPtr doc;
  };
};

#endif
