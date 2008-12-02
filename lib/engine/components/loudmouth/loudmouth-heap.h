
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras

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
 *                         loudmouth-heap.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : declaration of a loudmouth heap
 *
 */

#ifndef __LOUDMOUTH_HEAP_H__
#define __LOUDMOUTH_HEAP_H__

#include "heap-impl.h"
#include "personal-details.h"
#include "loudmouth-presentity.h"

namespace LM
{
  class Heap:
    public Ekiga::HeapImpl<Presentity>,
    public sigc::trackable
  {
  public:

    Heap (gmref_ptr<Ekiga::PersonalDetails> details_,
	  LmConnection* connection_);

    ~Heap ();

    const std::string get_name () const;

    bool populate_menu (Ekiga::MenuBuilder& builder);

    bool populate_menu_for_group (const std::string group,
				  Ekiga::MenuBuilder& builder);

    void disconnected ();

    /* public to be accessed by C callbacks */

    LmHandlerResult iq_handler (LmMessage* message);

    LmHandlerResult presence_handler (LmMessage* message);

  private:

    gmref_ptr<Ekiga::PersonalDetails> details;

    LmConnection* connection;

    LmMessageHandler* iq_lm_handler;

    LmMessageHandler* presence_lm_handler;

    void parse_roster (LmMessageNode* query);

    void add_item ();

    void add_item_form_submitted (bool submitted,
				  Ekiga::Form& result);

    void subscribe_from_form_submitted (bool submitted,
					Ekiga::Form& result);

    gmref_ptr<Presentity> find_item (const std::string jid);

    void on_personal_details_updated ();
  };
};

#endif
