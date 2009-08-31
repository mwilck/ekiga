
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
 *                         loudmouth-chat-simple.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : declaration of a loudmouth simple chat
 *
 */

#ifndef __LOUDMOUTH_CHAT_SIMPLE_H__
#define __LOUDMOUTH_CHAT_SIMPLE_H__

#include "services.h"
#include "chat-simple.h"

#include "loudmouth-presentity.h"

namespace LM
{
  class SimpleChat:
    public Ekiga::SimpleChat
  {
  public:

    SimpleChat (Ekiga::ServiceCore& core_,
		PresentityPtr presentity_);

    ~SimpleChat ();

    const std::string get_title () const;

    void connect (boost::shared_ptr<Ekiga::ChatObserver> observer);

    void disconnect (boost::shared_ptr<Ekiga::ChatObserver> observer);

    bool send_message (const std::string msg);

    bool populate_menu (Ekiga::MenuBuilder& builder);

    Ekiga::PresentityPtr get_presentity () const;

    /* specific api */

    void got_message (const std::string msg);

  private:

    Ekiga::ServiceCore& core;
    PresentityPtr presentity;
    std::list<boost::shared_ptr<Ekiga::ChatObserver> > observers;
  };

  typedef boost::shared_ptr<SimpleChat> SimpleChatPtr;

};

#endif
