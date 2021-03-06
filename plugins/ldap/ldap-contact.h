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
 *                         ldap-contact.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *                        : completed in 2008 by Howard Chu
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of a LDAP contact
 *
 */

#ifndef __LDAP_CONTACT_H__
#define __LDAP_CONTACT_H__

#include "contact-core.h"
#include "dynamic-object.h"

namespace OPENLDAP
{

/**
 * @addtogroup contacts
 * @internal
 * @{
 */

  class Contact:
      public Ekiga::Contact,
      public Ekiga::DynamicObject<Contact>
  {
  public:

    static boost::shared_ptr<Contact> create (Ekiga::ServiceCore &_core,
                                              const std::string _name,
                                              const std::map<std::string, std::string> _uris);

    ~Contact ();

    const std::string get_name () const;

    bool has_uri (const std::string uri) const;

  private:
    Contact (Ekiga::ServiceCore &_core,
	     const std::string _name,
    	     const std::map<std::string, std::string> _uris);

    Ekiga::ServiceCore &core;

    std::string name;
    std::map<std::string, std::string> uris;
  };

  typedef boost::shared_ptr<Contact> ContactPtr;

/**
 * @}
 */

};

#endif
