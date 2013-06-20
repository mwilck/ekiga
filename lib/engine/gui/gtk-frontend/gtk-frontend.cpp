
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
#include "gmstockicons.h"
#include "chat-core.h"
#include "contact-core.h"
#include "presence-core.h"
#include "addressbook-window.h"
#include "accounts-window.h"
#include "assistant-window.h"
#include "call-window.h"
#include "chat-window.h"
#include "main_window.h"
#include "statusicon.h"
#include "preferences-window.h"
#include "roster-view-gtk.h"
#include "history-source.h"
#include "book-view-gtk.h"
#include "call-history-view-gtk.h"

#include "gmwindow.h"

/* Private helpers */

// when the status icon is clicked, we want to either show or hide the main window
static void
on_status_icon_clicked (G_GNUC_UNUSED GtkWidget* widget,
                        gpointer data)
{
  GtkWidget *window = GTK_WIDGET (((GtkFrontend*)data)->get_main_window ());

  if (!gtk_widget_get_visible (window)
      || (gdk_window_get_state (GDK_WINDOW (gtk_widget_get_window (window))) & GDK_WINDOW_STATE_ICONIFIED)) {
    gtk_widget_show (window);
  }
  else {

    if (gtk_window_has_toplevel_focus (GTK_WINDOW (window)))
      gtk_widget_hide (window);
    else
      gtk_window_present (GTK_WINDOW (window));
  }
}

/* Public api */

bool
gtk_frontend_init (Ekiga::ServiceCore &core,
		   int * /*argc*/,
		   char ** /*argv*/[])
{
  bool result = false;

  boost::shared_ptr<Ekiga::PresenceCore> presence_core = core.get<Ekiga::PresenceCore> ("presence-core");
  boost::shared_ptr<Ekiga::ContactCore> contact_core = core.get<Ekiga::ContactCore> ("contact-core");
  boost::shared_ptr<Ekiga::ChatCore> chat_core = core.get<Ekiga::ChatCore> ("chat-core");
  boost::shared_ptr<History::Source> history_source = core.get<History::Source> ("call-history-store");

  if (presence_core && contact_core && chat_core && history_source) {

    // BEWARE: the GtkFrontend ctor could do everything, but the status
    // icon ctor and the main window ctor use GtkFrontend, so we must
    // keep the ctor+build setup
    boost::shared_ptr<GtkFrontend> gtk_frontend (new GtkFrontend (core));
    core.add (gtk_frontend);
    gtk_frontend->build ();
    result = true;
  }
  return result;
}


GtkFrontend::GtkFrontend (Ekiga::ServiceCore & _core) : core(_core)
{
}

GtkFrontend::~GtkFrontend ()
{
}


void
GtkFrontend::build ()
{
  boost::shared_ptr<Ekiga::ContactCore> contact_core = core.get<Ekiga::ContactCore> ("contact-core");
  boost::shared_ptr<Ekiga::ChatCore> chat_core = core.get<Ekiga::ChatCore> ("chat-core");

  /* Init the stock icons */
  gnomemeeting_stock_icons_init ();
  gtk_window_set_default_icon_name (GM_ICON_LOGO);

  addressbook_window =
    boost::shared_ptr<GtkWidget>(addressbook_window_new_with_key (*contact_core, "/apps/" PACKAGE_NAME "/general/user_interface/addressbook_window"),
				 gtk_widget_destroy);
  accounts_window =
    boost::shared_ptr<GtkWidget> (accounts_window_new_with_key (core, "/apps/" PACKAGE_NAME "/general/user_interface/accounts_window"),

				  gtk_widget_destroy);

  // BEWARE: uses the main window during runtime
  assistant_window =
    boost::shared_ptr<GtkWidget> (assistant_window_new (core),
				  gtk_widget_destroy);
  call_window =
    boost::shared_ptr<GtkWidget> (call_window_new (core),
				  gtk_widget_destroy);
  chat_window =
    boost::shared_ptr<GtkWidget> (chat_window_new (core, "/apps/" PACKAGE_NAME "/general/user_interface/chat_window"),
				  gtk_widget_destroy);
  preferences_window =
    boost::shared_ptr<GtkWidget> (preferences_window_new (core),
				  gtk_widget_destroy);

  // BEWARE: the status icon needs the chat window at startup
  status_icon =
    boost::shared_ptr<StatusIcon> (status_icon_new (core),
				   g_object_unref);
  g_signal_connect (status_icon.get (), "clicked",
		    G_CALLBACK (on_status_icon_clicked), this);

  // BEWARE: the main window uses the chat window at startup already,
  // and later on needs the call window, addressbook window,
  // preferences window and assistant window
  main_window =
    boost::shared_ptr<GtkWidget> (gm_main_window_new (core),
				 gtk_widget_destroy);

  gtk_window_set_transient_for (GTK_WINDOW (assistant_window.get ()), GTK_WINDOW (main_window.get ()));
}


const std::string
GtkFrontend::get_name () const
{
  return "gtk-frontend";
}


const std::string
GtkFrontend::get_description () const
{
  return "\tGtk+ frontend support";
}

const GtkWidget*
GtkFrontend::get_assistant_window () const
{
  return assistant_window.get ();
}

const GtkWidget*
GtkFrontend::get_main_window () const
{
  return main_window.get ();
}


const GtkWidget*
GtkFrontend::get_addressbook_window () const
{
  return addressbook_window.get ();
}


const GtkWidget*
GtkFrontend::get_accounts_window () const
{
  return accounts_window.get ();
}


const GtkWidget*
GtkFrontend::get_preferences_window () const
{
  return preferences_window.get ();
}


const GtkWidget*
GtkFrontend::get_call_window () const
{
  return call_window.get ();
}


const GtkWidget*
GtkFrontend::get_chat_window () const
{
  return chat_window.get ();
}


const StatusIcon*
GtkFrontend::get_status_icon () const
{
  return status_icon.get ();
}
