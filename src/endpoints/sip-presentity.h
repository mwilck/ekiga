
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
 *                         sip-presentity.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : declaration of a presentity for SIP chats
 *
 */



#ifndef __SIP_PRESENTITY_H__
#define __SIP_PRESENTITY_H__

#include "form.h"
#include "presence-core.h"
#include "presentity.h"

namespace SIP
{

/**
 * @addtogroup presence
 * @internal
 * @{
 */

  /**
   * This class implements an Ekiga::Presentity.
   */
  class Presentity: public Ekiga::Presentity
  {
  public:

    /**
     * Constructor and destructor
     */
    Presentity (Ekiga::ServiceCore& _core,
		std::string name_,
		std::string uri_);

    ~Presentity ();


    /**
     * Getters for the presentity
     */
    const std::string get_name () const;

    const std::string get_presence () const;

    const std::string get_status () const;

    const std::string get_avatar () const;

    const std::set<std::string> get_groups () const;

    const std::string get_uri () const;


    /**
     * Two setters for use by the SIP::Dialect
     */
    void set_presence (const std::string _presence);

    void set_status (const std::string _status);


    /** Populates the given Ekiga::MenuBuilder with the actions.
     * @param: A MenuBuilder.
     */
    bool populate_menu (Ekiga::MenuBuilder& builder);

  private:

    Ekiga::ServiceCore &core;
    Ekiga::PresenceCore *presence_core;

    std::string name;
    std::string uri;
    std::string presence;
    std::string status;
    std::string avatar;

    std::set<std::string> groups;
  };

/**
 * @}
 */
};

#endif
