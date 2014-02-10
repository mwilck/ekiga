
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
 *                         conversation.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2014 by Julien Puydt
 *   copyright            : (c) 2014 by Julien Puydt
 *   description          : declaration of the interface of a conversation
 *
 */

#ifndef __CONVERSATION_H__
#define __CONVERSATION_H__

#include "heap.h"

namespace Ekiga {

  struct Message {
    /* when? */
    const std::string time;
    /* who? */
    const std::string name;
    /* For the payload the idea is:
     * payload["bare"] for a bare text message for example
     * paylord["xhtml"] for a XHTML-enhanced message
     *
     * A gui trying to show a message would so something like:
     * iter = payload.find('xhtml')
     * if (iter != msg.payload.end())
     *     display_xhtml (iter->second);
     *
     * iter = msg.payload.find('bare')
     * if (iter != msg.payload.end())
     *     display_bare (iter->second);
     *
     * <if code comes here that's a problem>
     */
    const std::map<const std::string, const std::string> payload;
  };

  class Conversation: public LiveObject
  {
  public:

    /* This contains the list of people we discuss with */
    virtual HeapPtr get_heap () const = 0;

    /*FIXME: perhaps a std::map<const std::string, const std::string>
     * would be better?
     */
    virtual const std::string get_title () const = 0;
    virtual const std::string get_topic () const = 0;

    /* A view created after a conversation started needs to be able to list
     * the existing messages
     */
    virtual void visit_messages (boost::function1<bool, const Message&>) const = 0;

    /* This couple of methods makes it possible to count how many
     * messages haven't been read by the user: the conversation
     * increments its unread counter everytime a message is received,
     * and everytime a view is seen (has the focus, not just exists),
     * it resets it to zero.
     */
    virtual int get_unred_messages_number () const = 0;
    virtual void reset_unread_messages_number () const = 0;

    /* views which already exist need to know only about new messages */
    boost::signals2::signal<void(const Message&)> message_received;
  };

  typedef boost::shared_ptr<Conversation> ConversationPtr;
};

#endif
