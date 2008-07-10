
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
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of the main chat managing object
 *
 */

#ifndef __CHAT_CORE_H__
#define __CHAT_CORE_H__

#include "services.h"
#include "dialect.h"

// FIXME: that one is for Damien's temporary code
#include "chat-manager.h"

/* FIXME: probably it should have a decorator system, so we can for example
 * hook a logger
 */

namespace Ekiga
{
  /**
   * @defgroup chats Chats and protocols
   * @{
   */

  /*  Core object for text chat support.
   *
   * Notice that you give dialects to this object as references, so they won't
   * be freed here : it's up to you to free them somehow.
   */
  class ChatCore: public Service
  {
  public:

    /** The constructor.
     */
    ChatCore () {}

    /** The destructor.
     */
    ~ChatCore ();

    /*** service implementation ***/
  public:

    /** Returns the name of the service.
     * @return The service name.
     */
    const std::string get_name () const
    { return "chat-core"; }

    /** Returns the description of the service.
     * @return: The service description.
     */
    const std::string get_description () const
    { return "\tChat managing object"; }

    /*** Public API ***/
  public:

    /** Adds a dialect to the ContactCore service.
     * @param The dialect to be added.
     */
    void add_dialect (Dialect& dialect);

    /** Triggers a callback for all Ekiga::Dialect dialects of the
     * ChatCore service.
     * @param The callback (the return value means "go on" and allows stopping
     * the visit)
     */
    void visit_dialects (sigc::slot<bool, Dialect&> visitor);

    /** This signal is emitted when an Ekiga::Dialect has been added to
     * the ChatCore service.
     */
    sigc::signal<void, Dialect&> dialect_added;

  private:

    std::set<Dialect*> dialects;

    /*** Misc ***/
  public:

    /** Create the menu for the ChatCore and its actions.
     * @param A MenuBuilder object to populate.
     */
    bool populate_menu (MenuBuilder &builder);

    /** This signal is emitted when the ChatCore service has been updated.
     */
    sigc::signal<void> updated;

    /** This chain allows the ChatCore to present forms to the user
     */
    ChainOfResponsibility<FormRequest*> questions;

    /** FIXME: start of Damien's temporary code :
     **
     **/
  public:
    void add_manager (ChatManager &manager)
    {
      managers.insert (&manager);
      manager.im_failed.connect (sigc::bind (sigc::mem_fun (this, &ChatCore::on_im_failed), &manager));
      manager.im_received.connect (sigc::bind (sigc::mem_fun (this, &ChatCore::on_im_received), &manager));
      manager.im_sent.connect (sigc::bind (sigc::mem_fun (this, &ChatCore::on_im_sent), &manager));
      manager.new_chat.connect (sigc::bind (sigc::mem_fun (this, &ChatCore::on_new_chat), &manager));
    }

    bool send_message (const std::string & uri,
		       const std::string & message)
    {
      for (std::set<ChatManager*>::iterator iter = managers.begin ();
	   iter != managers.end ();
	   iter++) {

	if ((*iter)->send_message (uri, message))
	  return true;
      }

      return false;
    }

    sigc::signal<void, const ChatManager &, const std::string, const std::string> im_failed;
    sigc::signal<void, const ChatManager &, const std::string, const std::string, const std::string> im_received;
    sigc::signal<void, const ChatManager &, const std::string, const std::string> im_sent;
    sigc::signal<void, const ChatManager &, const std::string, const std::string> new_chat;

  private:
    std::set<ChatManager *> managers;

    void on_im_failed (const std::string uri, const std::string reason, ChatManager *manager)
    { im_failed.emit (*manager, uri, reason); }

    void on_im_sent (const std::string uri, const std::string message, ChatManager *manager)
    { im_sent.emit (*manager, uri, message); }

    void on_im_received (const std::string display_name, const std::string uri, const std::string message, ChatManager *manager)
    { im_received.emit (*manager, display_name, uri, message); }

    void on_new_chat (const std::string display_name, const std::string uri, ChatManager *manager)
    { new_chat.emit (*manager, display_name, uri); }


    /** FIXME: end of Damien's temporary code
     **
     **/
  };

  /**
   * @}
   */
};

#endif
