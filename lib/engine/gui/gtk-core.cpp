
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
 *                         gui-gtk.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of the user interface
 *
 */

#include <gtk/gtk.h>

#include "gtk-core.h"
#include "form-dialog-gtk.h"

static const char *presence_available[] = {
  "32 32 1 1",
  " 	c #49FF00",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                "};

static const char *presence_busy[] = {
  "32 32 1 1",
  " 	c #FF1417",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                "};

static const char *presence_unknown[] = {
  "32 32 2 1",
  " 	c None",
  ".	c #000000",
  "                                ",
  "                ....            ",
  "              ........          ",
  "           ............         ",
  "         ...............        ",
  "       ..................       ",
  "      ...................       ",
  "     ...........    .....       ",
  "     ........       ......      ",
  "     ......         ......      ",
  "     .....           .....      ",
  "     ....            .....      ",
  "                    ......      ",
  "                 .........      ",
  "               ...........      ",
  "              ...........       ",
  "            ...........         ",
  "           ...........          ",
  "           ........             ",
  "          ........              ",
  "          ......                ",
  "           ....                 ",
  "            ..                  ",
  "                                ",
  "                                ",
  "                                ",
  "            ...                 ",
  "           .....                ",
  "           .....                ",
  "           .....                ",
  "            ...                 ",
  "                                "};

Gtk::UI::UI (Ekiga::ServiceCore &_core): core(_core)
{
  // set the basic known icons
  GtkIconFactory *factory = gtk_icon_factory_new ();
  GtkIconSet *icon_set = NULL;
  GdkPixbuf *pixbuf = NULL;

  pixbuf = gdk_pixbuf_new_from_xpm_data (presence_available);
  icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
  gtk_icon_factory_add (factory, "presence-available", icon_set);
  gtk_icon_set_unref (icon_set);
  g_object_unref (pixbuf);

  pixbuf = gdk_pixbuf_new_from_xpm_data (presence_busy);
  icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
  gtk_icon_factory_add (factory, "presence-busy", icon_set);
  gtk_icon_set_unref (icon_set);
  g_object_unref (pixbuf);

  pixbuf = gdk_pixbuf_new_from_xpm_data (presence_unknown);
  icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
  gtk_icon_factory_add (factory, "presence-unknown", icon_set);
  gtk_icon_set_unref (icon_set);
  g_object_unref (pixbuf);

  gtk_icon_factory_add_default (factory);
  g_object_unref (factory);
}

void
Gtk::UI::run_form_request (Ekiga::FormRequest &request)
{
  FormDialog dialog (request);

  dialog.run ();
}
