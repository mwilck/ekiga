/***************************************************************************
                          menu.cpp  -  description
                             -------------------
    begin                : Sat Dec 23 2000
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : Functions to create the menus
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

#include "callbacks.h"
#include "menu.h"
#include "common.h"

#include "../config.h"


/******************************************************************************/
/* The functions                                                              */
/******************************************************************************/

void GM_menu_init (GtkWidget *gapp, GM_window_widgets *gw)
{
  static GnomeUIInfo file_menu [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("_Connect"), N_("Create A New Connection"),
	(void *)connect_cb, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_OPEN,
	'c', GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("_Disconnect"), N_("Close The Current Connection"),
	(void *)disconnect_cb, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_CLOSE,
	'd', GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_SEPARATOR,
      {
	GNOME_APP_UI_ITEM,
	N_("_Quit"), N_("Quit GnomeMeeting"),
	(void *)quit_callback, gw, NULL,
	GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_QUIT,
	'Q', GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };


  static GnomeUIInfo view_menu [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("_Remote User Info"), N_("View Remote User Info"),
	(void *)view_remote_user_info_callback, gw, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	NULL, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("_History"), N_("View the log"),
	(void *)view_log_callback, gw, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	NULL, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("_Audio Settings"), N_("View Audio Settings"),
	(void *)view_audio_settings_callback, gw, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	NULL, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("_Video Settings"), N_("View Video Settings"),
	(void *)view_video_settings_callback, gw, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	NULL, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_SEPARATOR,
      {
	GNOME_APP_UI_ITEM,
	N_("_Status Bar"), N_("View/Hide the Status Bar"),
	(void *)view_statusbar_callback, gw, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	NULL, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("_Control Panel"), N_("View/Hide the Control Panel"),
	(void *)view_notebook_callback, gw, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	NULL, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };


  static GnomeUIInfo settings_menu [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("_Settings"), N_("Change Your Preferences"),
	(void *)pref_callback, gw, NULL,
	GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_PREF,
	's', GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };


  static GnomeUIInfo help_menu [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("_About"), N_("About GnomeMeeting"),
	(void *)about_callback, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_ABOUT,
	'A', GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };
  
  
  static GnomeUIInfo main_menu [] =
    {
      GNOMEUIINFO_SUBTREE (N_("_File"), file_menu),
      GNOMEUIINFO_SUBTREE (N_("_View"), view_menu),
      GNOMEUIINFO_SUBTREE (N_("_Settings"), settings_menu),
      GNOMEUIINFO_SUBTREE (N_("_Help"), help_menu),
      GNOMEUIINFO_END
    };

  gtk_object_set_data(GTK_OBJECT(gapp), "file_menu", file_menu);
  gtk_object_set_data(GTK_OBJECT(gapp), "settings_menu", settings_menu);

  gnome_app_create_menus (GNOME_APP (gapp), main_menu);
  gnome_app_install_menu_hints (GNOME_APP (gapp), main_menu);
}


void GM_popup_menu_init (GtkWidget *widget)
{
  GtkWidget *popup_menu_widget;

  static GnomeUIInfo popup_menu [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("Local"), N_("Local Video Image"),
	(void *)popup_menu_local_callback, GINT_TO_POINTER(0), NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("Remote"), N_("Remote Video Image"),
	(void *)popup_menu_remote_callback, GINT_TO_POINTER(1), NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("Both"), N_("Both Video Images"),
	(void *)popup_menu_both_callback, GINT_TO_POINTER(2), NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };

  /* Create a popup menu to attach it to the drawing area */
  popup_menu_widget = gnome_popup_menu_new (popup_menu);
  gnome_popup_menu_attach (popup_menu_widget, GTK_WIDGET (widget),
                           NULL);
}

/******************************************************************************/
