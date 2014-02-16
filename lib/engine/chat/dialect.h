
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>

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
 *                         dialect.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of the interface of a chat
 *                          implementation backend
 *
 */

#ifndef __DIALECT_H__
#define __DIALECT_H__

#include "conversation.h"

namespace Ekiga
{

  class Dialect: public virtual LiveObject
  {
  public:

    /** The destructor.
     */
    virtual ~Dialect () {}

    /** Aggregates the number of unread messages in all conversations
     * within the dialect
     */
    virtual int get_unread_messages_count () const = 0;

    /** Triggers a callback for all conversations of the Dialect
     * @param: The callback (the return value means "go on" and allows
     * stopping the visit)
     */
    virtual void visit_conversations (boost::function1<bool, ConversationPtr> visitor) const = 0;

    /**
     * This signal is emitted when an Ekiga::Conversation has been added
     * to the dialect.
     */
    boost::signals2::signal<void(ConversationPtr)> conversation_added;

    /**
     * This signal is emitted when an Ekiga::Conversation has been removed
     * from the dialect.
     */
    boost::signals2::signal<void(ConversationPtr)> conversation_removed;

    /**
     * This signal is emitted when an Ekiga::Conversation has been updated
     * in the dialect.
     */
    boost::signals2::signal<void(ConversationPtr)> conversation_updated;
  };

  typedef boost::shared_ptr<Dialect> DialectPtr;

};

#endif
