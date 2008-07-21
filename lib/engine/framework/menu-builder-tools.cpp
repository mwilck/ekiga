
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
 *                         menu-builder-tools.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of tools to build menus
 *
 */

#include "menu-builder-tools.h"

Ekiga::ShortMenuBuilder::ShortMenuBuilder (MenuBuilder &builder_)
  : builder(builder_), active(true)
{
  /* nothing more */
}

void
Ekiga::ShortMenuBuilder::add_action (const std::string icon,
				     const std::string label,
				     sigc::slot<void> callback)
{
  if (active)
    builder.add_action (icon, label, callback);
}

void
Ekiga::ShortMenuBuilder::add_separator ()
{
  active = false;
}

int
Ekiga::ShortMenuBuilder::size () const
{
  return builder.size ();
}



Ekiga::TriggerMenuBuilder::TriggerMenuBuilder ()
  : active(true)
{
  /* nothing more */
}

void
Ekiga::TriggerMenuBuilder::add_action (const std::string /*icon*/,
				       const std::string /*label*/,
				       sigc::slot<void> callback)
{
  if (active) {

    active = false;
    callback ();
  }
}

void
Ekiga::TriggerMenuBuilder::add_separator ()
{
  /* nothing */
}

int
Ekiga::TriggerMenuBuilder::size () const
{
  if (active)
    return 0;
  else
    return 1;
}
