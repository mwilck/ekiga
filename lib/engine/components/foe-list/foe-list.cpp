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
 *                         foe-list.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2014 by Julien Puydt
 *   copyright            : (c) 2014 by Julien Puydt
 *   description          : interface of a delegate
 *
 */

#include <glib/gi18n.h>

#include "ekiga-settings.h"

#include "foe-list.h"
#include <iostream>

Ekiga::FoeList::FoeList(boost::shared_ptr<FriendOrFoe> fof):
  friend_or_foe(fof)
{
}


Ekiga::FoeList::~FoeList()
{
}

Ekiga::FriendOrFoe::Identification
Ekiga::FoeList::decide (const std::string /*domain*/,
			const std::string uri) const
{
  boost::scoped_ptr<Ekiga::Settings> settings(new Ekiga::Settings (CONTACTS_SCHEMA));
  std::list<std::string> foes = settings->get_string_list ("foe-list");
  Ekiga::FriendOrFoe::Identification result = Ekiga::FriendOrFoe::Unknown;

  if (std::find (foes.begin (), foes.end (), uri) != foes.end ())
    result = Ekiga::FriendOrFoe::Foe;

  boost::shared_ptr<FriendOrFoe> fof = friend_or_foe.lock ();
  if (fof) {
    if (result != Ekiga::FriendOrFoe::Foe)
      fof->add_helper_action (Ekiga::ActionPtr (new Ekiga::Action ("blacklist", _("_Blacklist"),
                                                                   boost::bind (&Ekiga::FoeList::add_foe, (Ekiga::FoeList *) this, uri))));
    else
      fof->remove_helper_action ("blacklist");
  }

  return result;
}

void
Ekiga::FoeList::add_foe (const std::string token)
{
  boost::scoped_ptr<Ekiga::Settings> settings(new Ekiga::Settings (CONTACTS_SCHEMA));
  std::list<std::string> foes = settings->get_string_list ("foe-list");
  foes.push_back (token);
  settings->set_string_list ("foe-list", foes);

  boost::shared_ptr<FriendOrFoe> fof = friend_or_foe.lock ();
  if (fof)
    fof->remove_helper_action ("blacklist");
}
