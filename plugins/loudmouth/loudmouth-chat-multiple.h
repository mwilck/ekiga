
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2011 Damien Sandras

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
 *                         loudmouth-chat-multiple.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2011 by Julien Puydt
 *   copyright            : (c) 2011 by Julien Puydt
 *   description          : declaration of a loudmouth multiple chat
 *
 */

#ifndef __LOUDMOUTH_CHAT_MULTIPLE_H__
#define __LOUDMOUTH_CHAT_MULTIPLE_H__

#include "services.h"
#include "chat-multiple.h"

#include <loudmouth/loudmouth.h>

namespace LM
{
  class MultipleChat:
    public Ekiga::MultipleChat
  {
  public:

    MultipleChat (Ekiga::ServiceCore& core_,
		  LmConnection* connection_);

    ~MultipleChat ();

    const std::string get_title () const;

    void connect (boost::shared_ptr<Ekiga::ChatObserver> observer);

    void disconnect (boost::shared_ptr<Ekiga::ChatObserver> observer);

    bool send_message (const std::string msg);

    bool populate_menu (Ekiga::MenuBuilder& builder);

    Ekiga::HeapPtr get_heap () const;

    /* specific api */

    void got_message (const std::string who,
		      const std::string msg);

  private:

    Ekiga::ServiceCore& core;
    LmConnection* connection;
    std::list<boost::shared_ptr<Ekiga::ChatObserver> > observers;
    Ekiga::HeapPtr heap; // FIXME: it needs a loudmouth-heap-chat!
    std::string my_name;
  };

  typedef boost::shared_ptr<MultipleChat> MultipleChatPtr;

};

#endif
