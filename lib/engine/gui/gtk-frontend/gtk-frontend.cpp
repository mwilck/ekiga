
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras
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
 *                         gtk-frontend.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : code to hook a gtk+ user interface to
 *                          the main program
 *
 */

#include <iostream>

#include <gtk/gtk.h>

#include "config.h"

#include "gtk-frontend.h"

#include "contact-core.h"
#include "presence-core.h"
#include "addressbook-window.h"
#include "roster-view-gtk.h"

#include "gmwindow.h"

static gboolean
on_delete_event_addressbook_window (GtkWidget *widget,
				    GdkEvent * /*event*/,
				    gpointer /*data*/)
{
  gtk_widget_hide_all (widget);

  return TRUE;
}

static void
on_quit ()
{
  /* FIXME: do something */
}

bool
gtk_frontend_init (Ekiga::ServiceCore &core,
		   int */*argc*/,
		   char **/*argv*/[])
{
  Ekiga::PresenceCore *presence_core = NULL;
  Ekiga::ContactCore *contact_core = NULL;
  GtkWidget *addressbook_window = NULL;
  GtkWidget *main_window = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *roster_view = NULL;

  contact_core = dynamic_cast<Ekiga::ContactCore*>(core.get ("contact-core"));
  presence_core = dynamic_cast<Ekiga::PresenceCore*>(core.get ("presence-core"));

  if (presence_core != NULL && contact_core != NULL) {

    addressbook_window =
      addressbook_window_new (contact_core,
			      "addressbook window");
    gm_window_set_key (GM_WINDOW (addressbook_window),
		       "/apps/" PACKAGE_NAME "/general/user_interface/addressbook_window");
    gtk_widget_show_all (addressbook_window);

    main_window = gm_window_new_with_key ("/apps/" PACKAGE_NAME "/general/user_interface/roster_window");
    gtk_window_set_title (GTK_WINDOW (main_window), "Main window");
    roster_view = roster_view_gtk_new (*presence_core);
    gtk_container_add (GTK_CONTAINER (main_window), roster_view);
    gtk_widget_show_all (main_window);
    return true;
  } else
    return false;
}
