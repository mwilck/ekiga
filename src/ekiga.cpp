
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
#include "assistant.h"
#include "main_window.h"
#include "gmstockicons.h"

#include <opal/buildopts.h>

#define new PNEW


GnomeMeeting *GnomeMeeting::GM = NULL;

/* The main GnomeMeeting Class  */
GnomeMeeting::GnomeMeeting ()
  : PProcess("", "", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)

{
  GM = this;

  assistant_window = NULL;
  main_window = NULL;
  assistant_window = NULL;
}


void
GnomeMeeting::Exit ()
{
  if (main_window)
    gtk_widget_destroy (main_window);
  main_window = NULL;

  if (assistant_window)
    gtk_widget_destroy (assistant_window);
  assistant_window = NULL;
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
GnomeMeeting::GetAssistantWindow ()
{
  return assistant_window;
}


void GnomeMeeting::Main ()
{
}


void GnomeMeeting::BuildGUI (Ekiga::ServiceCore& services)
{
  /* Init the stock icons */
  gnomemeeting_stock_icons_init ();

  /* Build the GUI */
  gtk_window_set_default_icon_name (GM_ICON_LOGO);
  assistant_window = ekiga_assistant_new (services);
  main_window = gm_main_window_new (services);
  // FIXME should be moved in ekiga_assistant_new
  gtk_window_set_transient_for (GTK_WINDOW (assistant_window), GTK_WINDOW (main_window));

  PTRACE (1, "Ekiga version "
          << MAJOR_VERSION << "." << MINOR_VERSION << "." << BUILD_NUMBER);
#ifdef EKIGA_REVISION
  PTRACE (1, "Ekiga git revision: " << EKIGA_REVISION);
#endif
  PTRACE (1, "PTLIB version " << PTLIB_VERSION);
  PTRACE (1, "OPAL version " << OPAL_VERSION);
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
