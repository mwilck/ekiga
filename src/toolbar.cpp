
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2001 Damien Sandras
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 *                        toolbar.cpp  -  description
 *                        ---------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          create the toolbar.
 *   email                : dsandras@seconix.com
 *
 */

#include "toolbar.h"
#include "callbacks.h"
#include "gnomemeeting.h"
#include "common.h"
#include "misc.h" 

#include "../pixmaps/connect.xpm"
#include "../pixmaps/disconnect.xpm"
#include "../pixmaps/ils.xpm"
#include "../pixmaps/settings.xpm"


/* Declarations */

extern GnomeMeeting *MyApp;
extern GtkWidget *gm;


/* The functions */

void gnomemeeting_init_toolbar ()
{
   GM_window_widgets *gw = gnomemeeting_get_main_window (gm);
   GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  static GnomeUIInfo main_toolbar [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("Connect"), N_("Create A New Connection"),
	(void *)connect_cb, gw, NULL,
	GNOME_APP_PIXMAP_DATA, connect_xpm,
	'C', GDK_CONTROL_MASK, NULL
	},
	{
	GNOME_APP_UI_ITEM,
	N_("Disconnect"), N_("Close The Current Connection"),
	(void *)disconnect_cb, gw, NULL,
	GNOME_APP_PIXMAP_DATA, disconnect_xpm,
	'D', GDK_CONTROL_MASK, NULL
	},
	GNOMEUIINFO_SEPARATOR,
	{
	GNOME_APP_UI_ITEM,
	N_("ILS Directory"), N_("Find friends on ILS"),
	(void *)ldap_callback, gw, NULL,
	GNOME_APP_PIXMAP_DATA, ils_xpm,
	'I', GDK_CONTROL_MASK, NULL
	},
	GNOMEUIINFO_SEPARATOR,
	{
	GNOME_APP_UI_ITEM,
	N_("Settings"), N_("Change Your Preferences"),
	(void *)pref_callback, pw, NULL,
	GNOME_APP_PIXMAP_DATA, settings_xpm,
	'S', GDK_CONTROL_MASK, NULL
	},
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_END
    };

  gtk_object_set_data(GTK_OBJECT (gm), "toolbar", main_toolbar);
  gnome_app_create_toolbar (GNOME_APP (gm), main_toolbar);
}
