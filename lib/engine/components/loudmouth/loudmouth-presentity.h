
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
 *                         loudmouth-presentity.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : declaration of a loudmouth presentity
 *
 */

#ifndef __LOUDMOUTH_PRESENTITY_H__
#define __LOUDMOUTH_PRESENTITY_H__

#include <loudmouth/loudmouth.h>

#include "presentity.h"

namespace LM
{
  class Presentity:
    public Ekiga::Presentity
  {
  public:
    Presentity (LmConnection* connection_,
		LmMessageNode* item_);

    ~Presentity ();

    /* usual presentity stuff */

    const std::string get_name () const;

    const std::string get_presence () const;

    const std::string get_status () const;

    const std::string get_avatar () const;

    const std::set<std::string> get_groups () const;

    bool populate_menu (Ekiga::MenuBuilder& builder);

    /* special presentity stuff */

    const std::string get_jid () const;

    void update (LmMessageNode* item_);

  private:
    LmConnection* connection;
    LmMessageNode* item;

    void edit_presentity ();

    void edit_presentity_form_submitted (bool submitted,
					 Ekiga::Form& result);

    void revoke_from ();
    void ask_to ();
    void stop_to ();

    void remove_presentity ();
  };
};

#endif
