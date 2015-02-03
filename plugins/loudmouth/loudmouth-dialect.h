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
 *                         loudmouth-dialect.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : declaration of the loudmouth dialect
 *
 */

#ifndef __LOUDMOUTH_DIALECT_H__
#define __LOUDMOUTH_DIALECT_H__

#include "dialect-impl.h"

#include "loudmouth-handler.h"
#include "loudmouth-conversation.h"
#include "loudmouth-presentity.h"

#include "services.h"

namespace LM
{
  class Dialect:
    public Ekiga::DialectImpl<Conversation>,
    public LM::Handler
  {
  public:

    Dialect (Ekiga::ServiceCore& core_);

    ~Dialect ();

    bool populate_menu (Ekiga::MenuBuilder& builder);

    /* specific */

    void push_message (PresentityPtr,
		       const Ekiga::Message::payload_type payload);

    void open_chat (PresentityPtr presentity);

    /* LM::Handler implementation */
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

    Ekiga::ServiceCore& core;

    void group_chat_action ();
    void on_open_group_chat_submitted (bool submitted,
				       Ekiga::Form& result);
  };

  typedef boost::shared_ptr<Dialect> DialectPtr;
};

#endif
