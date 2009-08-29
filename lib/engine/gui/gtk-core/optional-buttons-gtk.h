
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
 *                         optional-buttons-gtk.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2009 by Julien Puydt
 *   copyright            : (c) 2009 by Julien Puydt
 *   description          : declaration of a gtk+ optional buttons group
 *
 */

#ifndef __OPTIONAL_BUTTONS_GTK_H__
#define __OPTIONAL_BUTTONS_GTK_H__

#include <map>

#include <gtk/gtk.h>

#include "menu-builder.h"


/* Here is the main idea behind the following code : you build a set of buttons
 * for various actions, but you're not 100% sure the objets on which you'll act
 * will all have those actions available.
 *
 * The solution is to set the buttons up, and put them in an OptionalButtonsGtk,
 * which will take care of things.
 *
 * Basically, you build button1,...,buttonN and put them as you want in the
 * user interface, but don't hook any callback to them. You also have an object
 * of class OptionalButtonsGtk, call it "builder". You decide to manage the
 * buttons with it :
 * builder.add_button ("foo1", button1);
 * ...
 * builder.add_button ("fooN", buttonN);
 *
 * then you disable all actions (and clear the callbacks, which may free some
 * memory) :
 * builder.reset ();
 *
 * Finally, there's an object, call it obj, on which you want your buttons to
 * act ; you just do :
 * obj.populate_menu (builder);
 * and the builder will setup the buttons correctly : it will enable button1
 * only if obj has a "foo1" action, and it will hook the right callback to a
 * click on a button.
 *
 * If/when obj gets updated, then you just do the following again :
 * builder.reset ();
 * obj.populate_menu (builder);
 *
 * Likewise, if you know want to act on obj2, then :
 * builder.reset ();
 * obj2.populate_menu (builder);
 *
 */

class OptionalButtonsGtk: public Ekiga::MenuBuilder
{
public:

  OptionalButtonsGtk ();

  ~OptionalButtonsGtk ();

  /* here is the specific api */

  void add_button (const std::string label,
		   GtkButton* button);

  void reset ();

  /* this is the Ekiga::MenuBuilder implementation */

  void add_action (const std::string icon,
		   const std::string label,
		   const boost::function0<void> callback);

  void add_separator ()
  {}

  void add_ghost (G_GNUC_UNUSED const std::string icon,
		  G_GNUC_UNUSED const std::string label)
  {}

  int size () const
  { return nbr_elements; }

private:

  typedef std::map<std::string, GtkButton*> buttons_type;
  buttons_type buttons;
  int nbr_elements;
};

#endif
