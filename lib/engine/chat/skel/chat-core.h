
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras

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
 *                         chat-core.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras 
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : declaration of the interface of a chat core.
 *                          A chat core manages ChatManagers.
 *
 */

#ifndef __CHAT_CORE_H__
#define __CHAT_CORE_H__

#include "services.h"

#include <sigc++/sigc++.h>
#include <set>
#include <map>


namespace Ekiga
{

/**
 * @defgroup chats Chats and protocols
 * @{
 */

  class ChatManager;

  class ChatCore
    : public Service
    {

  public:
      typedef std::set<ChatManager *>::iterator iterator;
      typedef std::set<ChatManager *>::const_iterator const_iterator;

      /** The constructor
       */
      ChatCore () {}

      /** The destructor
       */
      ~ChatCore () {}


      /*** Service Implementation ***/

      /** Returns the name of the service.
       * @return The service name.
       */
      const std::string get_name () const
        { return "chat-core"; }


      /** Returns the description of the service.
       * @return The service description.
       */
      const std::string get_description () const
        { return "\tChat Core managing ChatManager objects"; }


      /** Adds a ChatManager to the ChatCore service.
       * @param The manager to be added.
       */
      void add_manager (ChatManager &manager);

      /** Return iterator to beginning
       * @return iterator to beginning
       */
      iterator begin ();
      const_iterator begin () const;

      /** Return iterator to end
       * @return iterator to end 
       */
      iterator end ();
      const_iterator end () const;

      /** This signal is emitted when a Ekiga::ChatManager has been
       * added to the ChatCore Service.
       */
      sigc::signal<void, ChatManager &> manager_added;


      /*** Instant Messaging ***/ 

      /** See chat-manager.h for API **/
      bool send_message (const std::string & uri, 
                         const std::string & message);

      sigc::signal<void, const ChatManager &, const std::string &, const std::string &> im_failed;
      sigc::signal<void, const ChatManager &, const std::string &, const std::string &, const std::string &> im_received;
      sigc::signal<void, const ChatManager &, const std::string &, const std::string &> im_sent;
      sigc::signal<void, const ChatManager &, const std::string &, const std::string &> new_chat;


  private:
      void on_im_failed (const std::string &, const std::string &, ChatManager *manager);
      void on_im_sent (const std::string &, const std::string &, ChatManager *manager);
      void on_im_received (const std::string &, const std::string &, const std::string &, ChatManager *manager);
      void on_new_chat (const std::string &, const std::string &, ChatManager *manager);

      std::set<ChatManager *> managers;
    };

/**
 * @}
 */

};


#endif
