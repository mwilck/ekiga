
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

#include "trigger.h"
#include "gtk-frontend.h"
#include "gmstockicons.h"
#include "account-core.h"
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
#include "opal-bank.h"
#include "book-view-gtk.h"
#include "notification-core.h"
#include "personal-details.h"
#include "audioinput-core.h"
#include "audiooutput-core.h"
#include "videoinput-core.h"
#include "videooutput-core.h"
#include "call-core.h"

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
  boost::shared_ptr<Opal::Bank> opal_bank = core.get<Opal::Bank> ("opal-account-store");
  boost::shared_ptr<Ekiga::Trigger> local_cluster_trigger = core.get<Ekiga::Trigger> ("local-cluster");
  boost::shared_ptr<Ekiga::PersonalDetails> details = core.get<Ekiga::PersonalDetails> ("personal-details");
  boost::shared_ptr<Ekiga::NotificationCore> notification_core = core.get<Ekiga::NotificationCore> ("notification-core");
  boost::shared_ptr<Ekiga::AccountCore> account_core = core.get<Ekiga::AccountCore> ("account-core");
  boost::shared_ptr<Ekiga::VideoInputCore> videoinput_core = core.get<Ekiga::VideoInputCore> ("videoinput-core");
  boost::shared_ptr<Ekiga::VideoOutputCore> videooutput_core = core.get<Ekiga::VideoOutputCore> ("videooutput-core");
  boost::shared_ptr<Ekiga::AudioInputCore> audioinput_core = core.get<Ekiga::AudioInputCore> ("audioinput-core");
  boost::shared_ptr<Ekiga::AudioOutputCore> audioooutput_core = core.get<Ekiga::AudioOutputCore> ("audiooutput-core");
  boost::shared_ptr<Ekiga::CallCore> call_core = core.get<Ekiga::CallCore> ("call-core");

  if (presence_core && contact_core && chat_core && history_source && opal_bank && local_cluster_trigger
      && notification_core && details && account_core && audioooutput_core && audioinput_core
      && videooutput_core && videoinput_core && call_core) {

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
  boost::shared_ptr<Ekiga::PersonalDetails> details = core.get<Ekiga::PersonalDetails> ("personal-details");
  boost::shared_ptr<Ekiga::ChatCore> chat_core = core.get<Ekiga::ChatCore> ("chat-core");
  boost::shared_ptr<Ekiga::AccountCore> account_core = core.get<Ekiga::AccountCore> ("account-core");

  /* Init the stock icons */
  gnomemeeting_stock_icons_init ();
  gtk_window_set_default_icon_name (GM_ICON_LOGO);

  addressbook_window =
    boost::shared_ptr<GtkWidget>(addressbook_window_new (contact_core), gtk_widget_destroy);
  gm_window_set_key (GM_WINDOW (addressbook_window.get ()),
		     "/apps/" PACKAGE_NAME "/general/user_interface/addressbook_window");

  accounts_window =
    boost::shared_ptr<GtkWidget> (accounts_window_new (account_core, details), gtk_widget_destroy);
  gm_window_set_key(GM_WINDOW (accounts_window.get ()),
		    "/apps/" PACKAGE_NAME "/general/user_interface/accounts_window");

  // BEWARE: uses the main window during runtime
  assistant_window =
    boost::shared_ptr<GtkWidget> (assistant_window_new (core), gtk_widget_destroy);

  call_window =
    boost::shared_ptr<GtkWidget> (call_window_new (core), gtk_widget_destroy);

  chat_window =
    boost::shared_ptr<GtkWidget> (chat_window_new (core), gtk_widget_destroy);
  gm_window_set_key(GM_WINDOW(chat_window.get ()), "/apps/" PACKAGE_NAME "/general/user_interface/chat_window");

  preferences_window =
    boost::shared_ptr<GtkWidget> (preferences_window_new (core), gtk_widget_destroy);

  // BEWARE: the status icon needs the chat window at startup
  status_icon =
    boost::shared_ptr<StatusIcon> (status_icon_new (core), g_object_unref);
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
