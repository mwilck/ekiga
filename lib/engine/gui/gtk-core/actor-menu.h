
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2014 Damien Sandras <dsandras@seconix.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         live-object-menu.h  -  description
 *                         ----------------------------------
 *   begin                : written in 2014 by Damien Sandras
 *   copyright            : (c) 2014 by Damien Sandras
 *   description          : A live object menu definition
 *
 */

#ifndef __LIVE_OBJECT_MENU_H__
#define __LIVE_OBJECT_MENU_H__

#include "action.h"
#include "contact-action.h"

namespace Ekiga {

  /* ActorMenu
   *
   * We will have one ActorMenu per UI object displaying a Actor.
   *
   * Each Actor object exposes Action objects. Those actions are actions
   * specific to the Actor. They can be triggered, for example, by the
   * various ActorMenu associated with the different views of the
   * Actor.
   *
   * However, even though we can have several ActorMenu for the same
   * Actor (one per UI view), we need to register the associated actions
   * only once.
   */
  class ActorMenu
  {
  public:

    ActorMenu (const Actor & obj);

    ~ActorMenu ();

    virtual void activate (Ekiga::Action *action);
    virtual const std::string as_xml ();

    static const std::string get_xml_menu (const std::string & id,
                                           const std::string & content,
                                           bool full);

  protected:
    const Actor & obj;
  };

  class ContactActorMenu : public ActorMenu
  {
  public:

    ContactActorMenu (const Actor & obj);

    void set_data (ContactPtr _contact,
                   const std::string & _uri);

    const std::string as_xml ();

  protected:
    ContactPtr contact;
    std::string uri;
  };
}

#endif
