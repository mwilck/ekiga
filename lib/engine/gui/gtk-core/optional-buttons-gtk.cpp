
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
 *                         optional-buttons-gtk.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2009 by Julien Puydt
 *   copyright            : (c) 2009 by Julien Puydt
 *   description          : implementation of a gtk+ optional buttons group
 *
 */

#include "optional-buttons-gtk.h"

/* here is some pretty simple stuff to keep data around correctly in a GObject,
 * (and react correctly to clicks on the buttons)
 */

struct OptionalButtonsGtkHelper
{
  boost::function0<void> callback;
};

static void
optional_buttons_gtk_helper_destroy (struct OptionalButtonsGtkHelper* helper)
{
  delete helper;
}

static void
on_optional_buttons_gtk_clicked (gpointer object,
				 G_GNUC_UNUSED gpointer data)
{
  struct OptionalButtonsGtkHelper* helper = 
    (struct OptionalButtonsGtkHelper*)g_object_get_data (G_OBJECT (object),
							 "ekiga-optional-buttons-gtk-helper");
  helper->callback ();
}

// here comes the implementation of the public interface :

OptionalButtonsGtk::OptionalButtonsGtk (): nbr_elements(0)
{
}

OptionalButtonsGtk::~OptionalButtonsGtk ()
{
  for (buttons_type::iterator iter = buttons.begin ();
       iter != buttons.end ();
       ++iter) {

    g_object_unref (iter->second);
  }
}

void
OptionalButtonsGtk::add_button (const std::string label,
				GtkButton* button)
{
  g_return_if_fail (GTK_IS_BUTTON (button));
  g_return_if_fail (buttons[label] == 0);

  g_object_ref (button);
  gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);
  buttons[label] = button;
  struct OptionalButtonsGtkHelper* helper = new struct OptionalButtonsGtkHelper;
  g_object_set_data_full (G_OBJECT (button), "ekiga-optional-buttons-gtk-helper",
			  (gpointer)helper,(GDestroyNotify)optional_buttons_gtk_helper_destroy);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (on_optional_buttons_gtk_clicked), NULL);
}

void
OptionalButtonsGtk::reset ()
{
  for (buttons_type::iterator iter = buttons.begin ();
       iter != buttons.end ();
       ++iter) {

    gtk_widget_set_sensitive (GTK_WIDGET (iter->second), FALSE);
    struct OptionalButtonsGtkHelper* helper =
      (struct OptionalButtonsGtkHelper*)g_object_get_data (G_OBJECT (iter->second),
							   "ekiga-optional-buttons-gtk-helper");
    helper->callback = boost::function0<void> ();
  }
  nbr_elements = 0;
}

void
OptionalButtonsGtk::add_action (const std::string icon,
				G_GNUC_UNUSED const std::string label,
				const boost::function0<void> callback)
{
  buttons_type::iterator iter = buttons.find (icon);

  if (iter != buttons.end ()) {

    struct OptionalButtonsGtkHelper* helper = 
      (struct OptionalButtonsGtkHelper*)g_object_get_data (G_OBJECT (iter->second),
							   "ekiga-optional-buttons-gtk-helper");
    helper->callback = callback;
    gtk_widget_set_sensitive (GTK_WIDGET (iter->second), TRUE);
    nbr_elements++;
  }
}
