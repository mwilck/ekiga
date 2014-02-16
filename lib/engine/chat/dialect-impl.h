
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
 *                         dialect.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008-2014 by Julien Puydt
 *   description          : basic implementation of a real chat backend
 *
 */

#ifndef __DIALECT_IMPL_H__
#define __DIALECT_IMPL_H__

#include "dialect.h"

#include "reflister.h"

namespace Ekiga
{
  template<typename ConversationType = Conversation>
  class DialectImpl:
    public Dialect,
    public RefLister<ConversationType>,
    public boost::signals2::trackable
  {
  public:

    /** The constructor.
     */
    DialectImpl ();

    /** Aggregates the number of unread messages in all conversations
     * within the dialect
     */
    int get_unread_messages_count () const;

    /** Triggers a callback for all conversations of the Dialect
     * @param: The callback (the return value means "go on" and allows
     * stopping the visit)
     */
    void visit_conversations (boost::function1<bool, ConversationPtr> visitor) const;

  protected:

    using RefLister<ConversationType>::add_connection;

    /* More STL-like ways to access the chats within this Ekiga::DialectImpl
     */
    typedef typename RefLister<ConversationType>::iterator iterator;
    typedef typename RefLister<ConversationType>::const_iterator const_iterator;

    iterator begin ();
    iterator end ();
    
    const_iterator begin () const;
    const_iterator end () const;

    /** Adds a Conversation to the Ekiga::Dialect.
     * @param The Conversation to be added
     * @return: The Ekiga::Dialect 'conversation_added' signal is emitted.
     */
    void add_conversation (boost::shared_ptr<ConversationType> conversation);

    /** Removes a Conversation from the Ekiga::Dialect.
     * @param The Conversation to be removed.
     */
    void remove_conversation (boost::shared_ptr<ConversationType> conversation);

  };
};

template<typename ConversationType>
Ekiga::DialectImpl<ConversationType>::DialectImpl ()
{
  RefLister<ConversationType>::object_added.connect (boost::ref (conversation_added));
  RefLister<ConversationType>::object_removed.connect (boost::ref (conversation_removed));
  RefLister<ConversationType>::object_updated.connect (boost::ref (conversation_updated));
}

template<typename ConversationType>
int
Ekiga::DialectImpl<ConversationType>::get_unread_messages_count () const
{
  int count = 0;
  for (const_iterator iter = begin (); iter != end (); ++iter)
    count += (*iter)->get_unread_messages_count ();

  return count;
}

template<typename ConversationType>
void
Ekiga::DialectImpl<ConversationType>::visit_conversations (boost::function1<bool, ConversationPtr> visitor) const
{
  RefLister<ConversationType>::visit_objects (visitor);
}

template<typename ConversationType>
typename Ekiga::DialectImpl<ConversationType>::iterator
Ekiga::DialectImpl<ConversationType>::begin ()
{
  return RefLister<ConversationType>::begin ();
}

template<typename ConversationType>
typename Ekiga::DialectImpl<ConversationType>::iterator
Ekiga::DialectImpl<ConversationType>::end ()
{
  return RefLister<ConversationType>::end ();
}

template<typename ConversationType>
typename Ekiga::DialectImpl<ConversationType>::const_iterator
Ekiga::DialectImpl<ConversationType>::begin () const
{
  return RefLister<ConversationType>::begin ();
}

template<typename ConversationType>
typename Ekiga::DialectImpl<ConversationType>::const_iterator
Ekiga::DialectImpl<ConversationType>::end () const
{
  return RefLister<ConversationType>::end ();
}

template<typename ConversationType>
void
Ekiga::DialectImpl<ConversationType>::add_conversation (boost::shared_ptr<ConversationType> conversation)
{
  conversation->questions.connect (boost::ref (questions));

  this->add_object (conversation);
}

template<typename ConversationType>
void
Ekiga::DialectImpl<ConversationType>::remove_conversation (boost::shared_ptr<ConversationType> conversation)
{
  this->remove_object (conversation);
}

#endif
