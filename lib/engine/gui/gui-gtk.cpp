
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

#include "contact-core.h"

#include "gui-gtk.h"
#include "searchbook/search-window.h"


Gtk::UI::UI (Ekiga::ServiceCore &_core) : core(_core)
{
  core.service_added.connect (sigc::mem_fun (this, &Gtk::UI::on_service_added));

  search_window = NULL;
}

void
Gtk::UI::run ()
{
}

void
Gtk::UI::on_service_added (Ekiga::Service & /*service */)
{
  GtkWidget *addressbook_window = NULL;
  GtkWidget *main_window = NULL;
  Ekiga::ContactCore *contact_core = NULL;
  static bool done = false; // FIXME: see below

  if (done)
    return;

  /* FIXME: Ok, here we could do fancy tricks like partial user interface,
   * built step-by-step depending on what we have available.
   *
   * For the purpose of this standalone test, we just try to build our windows
   * anytime we get the signal -- and stop using an ugly static variable trick
   * when we are done
   */
  contact_core = dynamic_cast<Ekiga::ContactCore*>(core.get ("contact-core"));

  if (contact_core != NULL) {

    done = true; // FIXME: see above

    search_window = addressbook_window_new (contact_core,
                                            "addressbook window");
  }
}
