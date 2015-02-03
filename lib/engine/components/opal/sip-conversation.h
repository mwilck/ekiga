/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2014 Damien Sandras <dsandras@seconix.com>

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
 *                         sip-conversation.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2014 by Julien Puydt
 *   copyright            : (c) 2014 by Julien Puydt
 *   description          : declaration of a SIP conversation
 *
 */

#ifndef __SIP_CONVERSATION_H__
#define __SIP_CONVERSATION_H__

#include "conversation.h"
#include "presence-core.h"

#include "sip-heap.h"

namespace SIP {

  class Conversation: public Ekiga::Conversation
  {
  public:

    Conversation (boost::shared_ptr<Ekiga::PresenceCore> _core,
		  const std::string _uri,
		  const std::string _name,
		  boost::function1<bool, const Ekiga::Message::payload_type&> _sender);

    // generic Ekiga::Conversation api:

    Ekiga::HeapPtr get_heap () const
    { return boost::dynamic_pointer_cast<Ekiga::Heap>(heap); }

    const std::string get_title () const
    { return title; }

    const std::string get_status () const
    { return status; }

    void visit_messages (boost::function1<bool, const Ekiga::Message&> visitor) const;
    bool send_message (const Ekiga::Message::payload_type& payload);

    int get_unread_messages_count () const
    { return unreads; }

    void reset_unread_messages_count ();

    bool populate_menu (Ekiga::MenuBuilder& /*builder*/)
    { return false; /* FIXME */ }

    // protocol-specific api
    const std::string get_uri () const
    { return uri; }

    void receive_message (const Ekiga::Message& message);

  private:

    boost::weak_ptr<Ekiga::PresenceCore> presence_core;
    std::string uri;
    std::string title;
    std::string status;
    boost::function1<bool, Ekiga::Message::payload_type> sender;
    boost::shared_ptr<Heap> heap;
    int unreads;
    std::list<Ekiga::Message> messages;
  };

  typedef typename boost::shared_ptr<Conversation> ConversationPtr;
};

#endif
