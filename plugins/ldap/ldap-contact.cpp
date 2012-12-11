
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
 *                         ldap-contact.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *                        : completed in 2008 by Howard Chu
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of a LDAP contact
 *
 */

#include "ldap-contact.h"
#include "menu-builder-tools.h"

/* at one point we will return a smart pointer on this... and if we don't use
 * a false smart pointer, we will crash : the reference count isn't embedded!
 */
struct null_deleter
{
    void operator()(void const *) const
    {
    }
};


OPENLDAP::Contact::Contact (Ekiga::ServiceCore &_core,
			    const std::string _name,
			    const std::map<std::string, std::string> _uris)
  : core(_core), name(_name), uris(_uris)
{
}


OPENLDAP::Contact::~Contact ()
{
}

const std::string
OPENLDAP::Contact::get_name () const
{
  return name;
}

bool
OPENLDAP::Contact::has_uri (const std::string uri) const
{
  bool result = false;

  for (std::map<std::string, std::string>::const_iterator iter = uris.begin ();
       !result && iter != uris.end ();
       iter++) {

    result = (uri == iter->second);
  }

  return result;
}

bool
OPENLDAP::Contact::populate_menu (Ekiga::MenuBuilder &builder)
{
  boost::shared_ptr<Ekiga::ContactCore> contact_core = core.get<Ekiga::ContactCore> ("contact-core");
  /* FIXME: add here the specific actions we want to allow
   * (before or after the uri-specific actions)
   */

  Ekiga::TemporaryMenuBuilder tmp_builder;

  bool result = false;
  for (std::map<std::string, std::string>::const_iterator iter
	 = uris.begin ();
       iter != uris.end ();
       iter++) {
    if (contact_core->populate_contact_menu (ContactPtr(this, null_deleter ()),
					     iter->second, tmp_builder)) {
      builder.add_ghost ("", iter->second);
      tmp_builder.populate_menu (builder);
      result = true;
    }
  }
  return result;
}
