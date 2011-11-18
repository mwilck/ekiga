
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
 *                         gnomemeeting.cpp  -  description
 *                         --------------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the main class
 *
 */


#include "config.h"

#include "ekiga.h"
#include "preferences.h"
#include "assistant.h"
#include "accounts.h"
#include "main_window.h"
#include "gmstockicons.h"

#ifdef HAVE_DBUS
#include "dbus.h"
#endif

#define new PNEW


GnomeMeeting *GnomeMeeting::GM = NULL;

/* The main GnomeMeeting Class  */
GnomeMeeting::GnomeMeeting ()
  : PProcess("", "", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)

{
  GM = this;

  call_number = 0;

  assistant_window = NULL;
  prefs_window = NULL;
#ifdef HAVE_DBUS
  dbus_component = NULL;
#endif
}


GnomeMeeting::~GnomeMeeting()
{
  Exit ();
}


void
GnomeMeeting::Exit ()
{
  PWaitAndSignal m(ep_var_mutex);

  if (prefs_window)
    gtk_widget_destroy (prefs_window);
  prefs_window = NULL;

  if (main_window)
    gtk_widget_destroy (main_window);
  main_window = NULL;

  if (assistant_window)
    gtk_widget_destroy (assistant_window);
  assistant_window = NULL;

  if (accounts_window)
    gtk_widget_destroy (accounts_window);
  accounts_window = NULL;

  if (statusicon)
    g_object_unref (statusicon);
  statusicon = NULL;

#ifdef HAVE_DBUS
  if (dbus_component)
    g_object_unref (dbus_component);
  dbus_component = NULL;
#endif
}


GnomeMeeting *
GnomeMeeting::Process ()
{
  return GM;
}


GtkWidget *
GnomeMeeting::GetMainWindow ()
{
  return main_window;
}


GtkWidget *
GnomeMeeting::GetPrefsWindow ()
{
  return prefs_window;
}


GtkWidget *
GnomeMeeting::GetAssistantWindow ()
{
  return assistant_window;
}


GtkWidget *
GnomeMeeting::GetAccountsWindow ()
{
  return accounts_window;
}


StatusIcon *
GnomeMeeting::GetStatusicon ()
{
  return statusicon;
}

void GnomeMeeting::Main ()
{
}


void GnomeMeeting::BuildGUI (Ekiga::ServiceCorePtr services)
{
  /* Init the stock icons */
  gnomemeeting_stock_icons_init ();

  /* Build the GUI */
  gtk_window_set_default_icon_name (GM_ICON_LOGO);
  accounts_window = gm_accounts_window_new (*services);
  prefs_window = gm_prefs_window_new (services.get ());
  assistant_window = ekiga_assistant_new (services.get ());

  statusicon = statusicon_new (*services);
  main_window = gm_main_window_new (*services);
  // FIXME should be moved inside the gm_accounts_window_new code
  gtk_window_set_transient_for (GTK_WINDOW (accounts_window), GTK_WINDOW (main_window));
  // FIXME should be moved in ekiga_assistant_new
  gtk_window_set_transient_for (GTK_WINDOW (prefs_window), GTK_WINDOW (main_window));
  // FIXME should be moved in ekiga_assistant_new
  gtk_window_set_transient_for (GTK_WINDOW (assistant_window), GTK_WINDOW (main_window));

  /* GM is started */
  PTRACE (1, "Ekiga version "
	  << MAJOR_VERSION << "." << MINOR_VERSION << "." << BUILD_NUMBER);
  PTRACE (1, "OPAL version " << OPAL_VERSION);
  PTRACE (1, "PTLIB version " << PTLIB_VERSION);
#if defined HAVE_XV || defined HAVE_DX
  PTRACE (1, "Accelerated rendering support enabled");
#else
  PTRACE (1, "Accelerated rendering support disabled");
#endif
#ifdef HAVE_DBUS
  PTRACE (1, "DBUS support enabled");
#else
  PTRACE (1, "DBUS support disabled");
#endif
#ifdef HAVE_GCONF
  PTRACE (1, "GConf support enabled");
#else
  PTRACE (1, "GConf support disabled");
#endif
}
