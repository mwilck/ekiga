
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
 *                         loudmouth-heap-roster.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008-2011 by Julien Puydt
 *   copyright            : (c) 2008-2011 by Julien Puydt
 *   description          : declaration of a loudmouth heap for the roster
 *
 */

#ifndef __LOUDMOUTH_HEAP_ROSTER_H__
#define __LOUDMOUTH_HEAP_ROSTER_H__

#include "heap-impl.h"
#include "personal-details.h"
#include "loudmouth-dialect.h"
#include "loudmouth-handler.h"

namespace LM
{
  class HeapRoster:
    public Ekiga::HeapImpl<Presentity>,
    public LM::Handler
  {
  public:

    HeapRoster (boost::shared_ptr<Ekiga::PersonalDetails> details_,
		DialectPtr dialect_);

    ~HeapRoster ();

    const std::string get_name () const;

    bool populate_menu (Ekiga::MenuBuilder& builder);

    bool populate_menu_for_group (const std::string group,
				  Ekiga::MenuBuilder& builder);

    LmConnection* get_connection () const;

    /* public to be accessed by C callbacks */

    LmHandlerResult iq_handler (LmMessage* message);

    LmHandlerResult presence_handler (LmMessage* message);

    LmHandlerResult message_handler (LmMessage* message);

    // implementation of the LM::Handler abstract class :
    void handle_up (LmConnection* connection,
		    const std::string name);
    void handle_down (LmConnection* connection);
    LmHandlerResult handle_iq (LmConnection* connection,
			       LmMessage* message);
    LmHandlerResult handle_message (LmConnection* connection,
				    LmMessage* message);
    LmHandlerResult handle_presence (LmConnection* connection,
				     LmMessage* message);

  private:

    boost::signals2::connection details_connection;

    boost::shared_ptr<Ekiga::PersonalDetails> details;

    DialectPtr dialect;

    std::string name;

    LmConnection* connection;

    LmHandlerResult handle_initial_roster_reply (LmConnection* connection,
						 LmMessage* message);
    void parse_roster (LmMessageNode* query);

    void add_item ();

    void add_item_form_submitted (bool submitted,
				  Ekiga::Form& result);

    void subscribe_from_form_submitted (bool submitted,
					Ekiga::Form& result);

    PresentityPtr find_item (const std::string jid);

    void on_personal_details_updated ();

    void on_chat_requested (PresentityPtr presentity);

    LmHandlerResult iq_handler_roster (LmMessage* message);
    LmHandlerResult iq_handler_muc (LmMessage* message);

    LmHandlerResult presence_handler_roster (LmMessage* message);
    LmHandlerResult presence_handler_muc (LmMessage* message);

    LmHandlerResult message_handler_roster (LmMessage* message);
    LmHandlerResult message_handler_muc (LmMessage* message);

    const std::set<std::string> existing_groups () const;

    /* when adding an item, we first ask to add it to the roster,
     * then get notified that it was really added,
     * and then we could ask to subscribe to it,
     * *but* we don't want to do that if that was done from another client
     * so when we ask to add it, we note it in that set, so when we get
     * notified it was added, we can know we did that and act accordingly.
     */
    std::set<std::string> items_added_by_me;
  };

  typedef boost::shared_ptr<HeapRoster> HeapRosterPtr;

};

#endif
