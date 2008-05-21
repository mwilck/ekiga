
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
 *                         chat-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras 
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : declaration of the interface of a chat core.
 *                          A chat core manages ChatManagers.
 *
 */

#include <iostream>

#include "config.h"

#include "chat-core.h"
#include "chat-manager.h"


using namespace Ekiga;


void ChatCore::add_manager (ChatManager &manager)
{
  managers.insert (&manager);
  manager_added.emit (manager);

  manager.im_failed.connect (sigc::bind (sigc::mem_fun (this, &ChatCore::on_im_failed), &manager));
  manager.im_received.connect (sigc::bind (sigc::mem_fun (this, &ChatCore::on_im_received), &manager));
  manager.im_sent.connect (sigc::bind (sigc::mem_fun (this, &ChatCore::on_im_sent), &manager));
  manager.new_chat.connect (sigc::bind (sigc::mem_fun (this, &ChatCore::on_new_chat), &manager));
}


ChatCore::iterator ChatCore::begin ()
{
  return managers.begin ();
}


ChatCore::const_iterator ChatCore::begin () const
{
  return managers.begin ();
}


ChatCore::iterator ChatCore::end ()
{
  return managers.end (); 
}


ChatCore::const_iterator ChatCore::end () const
{
  return managers.end (); 
}


bool ChatCore::send_message (const std::string & uri, 
                             const std::string & message)
{
  for (ChatCore::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {

    if ((*iter)->send_message (uri, message))
      return true;
  }

  return false;
}


void ChatCore::on_im_failed (const std::string uri, const std::string reason, ChatManager *manager)
{
  im_failed.emit (*manager, uri, reason);
}


void ChatCore::on_im_sent (const std::string uri, const std::string message, ChatManager *manager)
{
  im_sent.emit (*manager, uri, message);
}


void ChatCore::on_im_received (const std::string display_name, const std::string uri, const std::string message, ChatManager *manager)
{
  im_received.emit (*manager, display_name, uri, message);
}


void ChatCore::on_new_chat (const std::string display_name, const std::string uri, ChatManager *manager)
{
  new_chat.emit (*manager, display_name, uri);
}
