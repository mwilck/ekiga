
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
 *                         contact-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of the main contact managing object
 *
 */

#include <iostream>

#include "contact-core.h"

static void
on_search ()
{
  std::cout << "Search not implemented yet" << std::endl;
}

Ekiga::ContactCore::~ContactCore ()
{
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

void
Ekiga::ContactCore::populate_menu (MenuBuilder &builder)
{
  builder.add_action ("Search", sigc::ptr_fun (on_search));

  for (std::list<Source *>::const_iterator iter = sources.begin ();
       iter != sources.end ();
       iter++)
    (*iter)->populate_menu (builder);
}

void
Ekiga::ContactCore::add_source (Source &source)
{
  sources.push_front (&source);
  source_added.emit (source);
}

void
Ekiga::ContactCore::visit_sources (sigc::slot<void, Source &> visitor)
{
  for (std::list<Source *>::iterator iter = sources.begin ();
       iter != sources.end ();
       iter++)
    visitor (*(*iter));
}

void
Ekiga::ContactCore::add_contact_decorator (ContactDecorator &decorator)
{
  contact_decorators.push_front (&decorator);
}

void
Ekiga::ContactCore::populate_contact_menu (Contact &contact,
					   MenuBuilder &builder)
{
  for (std::list<ContactDecorator *>::const_iterator iter
	 = contact_decorators.begin ();
       iter != contact_decorators.end ();
       iter++)
    (*iter)->populate_menu (contact, builder);
}
