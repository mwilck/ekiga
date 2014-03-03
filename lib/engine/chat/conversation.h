
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
     * payload["text/plain"] for a bare text message for example
     * paylord["text/xhtml"] for a XHTML-enhanced message
     *
     * A gui trying to show a message would so something like:
     * iter = payload.find('text/xhtml')
     * if (iter != msg.payload.end())
     *     display_xhtml (iter->second);
     *
     * iter = msg.payload.find('text/plain')
     * if (iter != msg.payload.end())
     *     display_bare (iter->second);
     *
     * <if code comes here that's a problem>
     */
    typedef typename std::map<std::string, std::string> payload_type;
    const payload_type payload;
  };

  class Conversation: public LiveObject
  {
  public:

    /* This contains the list of people we discuss with */
    virtual HeapPtr get_heap () const = 0;

    /* The title of the conversation
     * (which can be as simple as the name of the remote contact/room)
     */
    virtual const std::string get_title () const = 0;

    /* The conversation status
     * (can for example contain typing notifications)
     */
    virtual const std::string get_status () const = 0;

    /* A view created after a conversation started needs to be able to list
     * the existing messages
     */
    virtual void visit_messages (boost::function1<bool, const Message&>) const = 0;

    /* Send a message through this conversation
     * @param: the message to send
     * @return: whether we could send
     */
    virtual bool send_message (const Message::payload_type&) = 0;

    /* This couple of methods makes it possible to count how many
     * messages haven't been read by the user: the conversation
     * increments its unread counter everytime a message is received,
     * and everytime a view is seen (has the focus, not just exists),
     * it resets it to zero.
     */
    virtual int get_unread_messages_count () const = 0;
    virtual void reset_unread_messages_count () = 0;

    /* Those are properties accessors, which can be used to access
     * things like a 'topic', for example, which make little sense in
     * a generic api but could still be handy in a gui.
     *
     * Beware that if you check the result directly (in an if, or passing it
     * through a function, the obtained value will be wether or not the value
     * is available, and not the value itself!
     *
     * To be more specific, you're supposed to:
     * val = foo.get_bool_property ("bar");
     * if (val) {
     *   <do something with *val> (notice *val not val!)
     * }
     */
    virtual boost::optional<bool> get_bool_property (const std::string) const
    { return boost::optional<bool> (); }

    virtual boost::optional<int> get_int_property (const std::string) const
    { return boost::optional<int> (); }

    virtual boost::optional<std::string> get_string_property (const std::string) const
    { return boost::optional<std::string> (); }


    /* views which already exist need to know only about new messages */
    boost::signals2::signal<void(const Message&)> message_received;

    /* This signal is emitted when the user requested to see this */
    boost::signals2::signal<void(void)> user_requested;
  };

  typedef boost::shared_ptr<Conversation> ConversationPtr;
};

#endif
