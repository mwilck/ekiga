/***************************************************************************
                          toolbar.cpp  -  description
                             -------------------
    begin                : Sat Nov 18 2000
    copyright            : (C) 2000-2001 by Damien Sandras
    email                : dsandras@acm.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "toolbar.h"
#include "callbacks.h"
#include "main.h"
#include "common.h"

#include "../pixmaps/connect.xpm"
#include "../pixmaps/disconnect.xpm"
#include "../pixmaps/ils.xpm"
#include "../pixmaps/settings.xpm"

extern GnomeMeeting *MyApp;
extern GtkWidget *gm;

void GM_toolbar_init (GtkWidget *gapp, GM_window_widgets *gw)
{
  static GnomeUIInfo main_toolbar [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("Connect"), N_("Create A New Connection"),
	(void *)connect_cb, NULL, NULL,
	GNOME_APP_PIXMAP_DATA, connect_xpm,
	'C', GDK_CONTROL_MASK, NULL
	},
	{
	GNOME_APP_UI_ITEM,
	N_("Disconnect"), N_("Close The Current Connection"),
	(void *)disconnect_cb, NULL, NULL,
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
	(void *)pref_callback, gw, NULL,
	GNOME_APP_PIXMAP_DATA, settings_xpm,
	'S', GDK_CONTROL_MASK, NULL
	},
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_END
    };

  gtk_object_set_data(GTK_OBJECT (gapp), "toolbar", main_toolbar);
  gnome_app_create_toolbar (GNOME_APP (gapp), main_toolbar);
}


void disable_connect ()
{ 
  GtkWidget *object;
  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "toolbar");

  GnomeUIInfo *main_toolbar = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (main_toolbar [0].widget, FALSE);
  gtk_widget_set_sensitive (main_toolbar [3].widget, FALSE);
  gtk_widget_set_sensitive (main_toolbar [5].widget, FALSE);

  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "file_menu");

  GnomeUIInfo *file_menu = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (file_menu [0].widget, FALSE);
  gtk_widget_set_sensitive (file_menu [3].widget, FALSE);

  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "settings_menu");

  GnomeUIInfo *settings_menu = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (settings_menu [0].widget, FALSE);
}


void enable_connect ()
{
  GtkWidget *object;
  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "toolbar");

  GnomeUIInfo *main_toolbar = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (main_toolbar [0].widget, TRUE);
  gtk_widget_set_sensitive (main_toolbar [3].widget, TRUE);
  gtk_widget_set_sensitive (main_toolbar [5].widget, TRUE);

  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "file_menu");

  GnomeUIInfo *file_menu = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (file_menu [0].widget, TRUE);
  gtk_widget_set_sensitive (file_menu [3].widget, TRUE);


  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "settings_menu");

  GnomeUIInfo *settings_menu = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (settings_menu [0].widget, TRUE);
}


void enable_disconnect ()
{
  GtkWidget *object;
  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "toolbar");

  GnomeUIInfo *main_toolbar = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (main_toolbar [1].widget, TRUE);
  gtk_widget_set_sensitive (main_toolbar [3].widget, TRUE);
  gtk_widget_set_sensitive (main_toolbar [5].widget, TRUE);

  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "file_menu");

  GnomeUIInfo *file_menu = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (file_menu [1].widget, TRUE);
}


void disable_disconnect ()
{
  GtkWidget *object;
  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "toolbar");

  GnomeUIInfo *main_toolbar = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (main_toolbar [1].widget, FALSE);
  gtk_widget_set_sensitive (main_toolbar [3].widget, FALSE);
  gtk_widget_set_sensitive (main_toolbar [5].widget, FALSE);

  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
					      "file_menu");

  GnomeUIInfo *file_menu = (GnomeUIInfo *) object;

  gtk_widget_set_sensitive (file_menu [1].widget, FALSE);
}
