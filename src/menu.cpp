
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
 *                         menu.cpp  -  description
 *                            -------------------
 *   begin                : Tue Dec 23 2000
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          :  Functions to create the menus.
 *   email                : dsandras@seconix.com
 *
 */

#include "callbacks.h"
#include "menu.h"
#include "common.h"
#include "docklet.h"

#include "../config.h"


/* Declarations */

extern GtkWidget *gm;

static void half_zoom_callback (GtkWidget *, gpointer);
static void normal_zoom_callback (GtkWidget *, gpointer);
static void double_zoom_callback (GtkWidget *, gpointer);
static void view_remote_user_info_callback (GtkWidget *, gpointer);
static void view_log_callback (GtkWidget *, gpointer);
static void view_audio_settings_callback (GtkWidget *, gpointer);
static void view_video_settings_callback (GtkWidget *, gpointer);
static void view_statusbar_callback (GtkWidget *, gpointer data);
static void view_notebook_callback (GtkWidget *, gpointer);
static void view_quickbar_callback (GtkWidget *, gpointer);
static void view_docklet_callback (GtkWidget *, gpointer);

/* GTK Callbacks */

/* DESCRIPTION  :  This callback is called when the user chooses 1:2 as zoom
 *                 factor in the popup menu.
 * BEHAVIOR     :  Sets the zoom to 0.5. That zoom will be read in 
 *                 GDKVideoOutputDEvice.
 * PRE          :  gpointer is a valid pointer to a GM_window_widgets structure.
 */
static void half_zoom_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;
  gw->zoom = 0.5;
}


/* DESCRIPTION  :  This callback is called when the user chooses 1:2 as zoom
 *                 factor in the popup menu.
 * BEHAVIOR     :  Sets the zoom to 1. That zoom will be read in 
 *                 GDKVideoOutputDEvice.
 * PRE          :  gpointer is a valid pointer to a GM_window_widgets structure.
 */
static void normal_zoom_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;
  gw->zoom = 1;
}


/* DESCRIPTION  :  This callback is called when the user chooses 1:2 as zoom
 *                 factor in the popup menu.
 * BEHAVIOR     :  Sets the zoom to 2.00. That zoom will be read in 
 *                 GDKVideoOutputDEvice.
 * PRE          :  gpointer is a valid pointer to a GM_window_widgets structure.
 */
static void double_zoom_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;
  gw->zoom = 2;
}


/* DESCRIPTION  :  This callback is called when the user toggles the 
 *                 corresponding option in the View Menu.
 * BEHAVIOR     :  Selects and show the good page in the main_notebook.
 * PRE          :  gpointer is a valid pointer to a GM_window_widgets structure.
 */
static void view_remote_user_info_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;

  gtk_notebook_set_page (GTK_NOTEBOOK (gw->main_notebook), 0);
}


/* DESCRIPTION  :  This callback is called when the user toggles the 
 *                 corresponding option in the View Menu.
 * BEHAVIOR     :  Selects and show the good page in the main_notebook.
 * PRE          :  gpointer is a valid pointer to a GM_window_widgets structure.
 */
static void view_log_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;

  gtk_notebook_set_page (GTK_NOTEBOOK (gw->main_notebook), 1);
}


/* DESCRIPTION  :  This callback is called when the user toggles the 
 *                 corresponding option in the View Menu.
 * BEHAVIOR     :  Selects and show the good page in the main_notebook.
 * PRE          :  gpointer is a valid pointer to a GM_window_widgets structure.
 */
static void view_audio_settings_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;

  gtk_notebook_set_page (GTK_NOTEBOOK (gw->main_notebook), 2);
}


/* DESCRIPTION  :  This callback is called when the user toggles the 
 *                 corresponding option in the View Menu.
 * BEHAVIOR     :  Selects and show the good page in the main_notebook.
 * PRE          :  gpointer is a valid pointer to a GM_window_widgets structure.
 */
static void view_video_settings_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;

  gtk_notebook_set_page (GTK_NOTEBOOK (gw->main_notebook), 3);
}


/* DESCRIPTION  :  This callback is called when the user toggles the 
 *                 corresponding option in the View Menu.
 * BEHAVIOR     :  Shows or hide the widget, and toggles the corresponding
 *                 button in the preferences window with the good value.
 * PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets 
 *                 structure.
 */
static void view_statusbar_callback (GtkWidget *widget, gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  if (GTK_WIDGET_VISIBLE (GTK_WIDGET (pw->gw->statusbar))) {

    gtk_widget_hide_all (pw->gw->statusbar);
    GTK_TOGGLE_BUTTON (pw->show_statusbar)->active = FALSE;
  }
  else {

    gtk_widget_show_all (pw->gw->statusbar);  
    GTK_TOGGLE_BUTTON (pw->show_statusbar)->active = TRUE;
  }
}


/* DESCRIPTION  :  This callback is called when the user toggles the 
 *                 corresponding option in the View Menu.
 * BEHAVIOR     :  Shows or hide the widget, and toggles the corresponding
 *                 button in the preferences window with the good value.
 * PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets 
 *                 structure.
 */
static void view_notebook_callback (GtkWidget *widget, gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  if (GTK_WIDGET_VISIBLE (GTK_WIDGET (pw->gw->main_notebook))) {
    
    gtk_widget_hide_all (pw->gw->main_notebook);
    GTK_TOGGLE_BUTTON (pw->show_notebook)->active = FALSE;
  }
  else {

    gtk_widget_show_all (pw->gw->main_notebook);  
    GTK_TOGGLE_BUTTON (pw->show_notebook)->active = TRUE;
  }
}


/* DESCRIPTION  :  This callback is called when the user toggles the 
 *                 corresponding option in the View Menu.
 * BEHAVIOR     :  Shows or hide the widget, and toggles the corresponding
 *                 button in the preferences window with the good value.
 * PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets 
 *                 structure.
 */
static void view_quickbar_callback (GtkWidget *widget, gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  if (GTK_WIDGET_VISIBLE (GTK_WIDGET (pw->gw->quickbar_frame))) {

    gtk_widget_hide_all (pw->gw->quickbar_frame);
    GTK_TOGGLE_BUTTON (pw->show_quickbar)->active = FALSE;
  }
  else {

    gtk_widget_show_all (pw->gw->quickbar_frame);  
    GTK_TOGGLE_BUTTON (pw->show_quickbar)->active = TRUE;
  }
}


/* DESCRIPTION  :  This callback is called when the user toggles the 
 *                 corresponding option in the View Menu.
 * BEHAVIOR     :  Shows or hide the widget, and toggles the corresponding
 *                 button in the preferences window with the good value.
 * PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets 
 *                 structure.
 */
static void view_docklet_callback (GtkWidget *widget, gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  if (GTK_WIDGET_VISIBLE (pw->gw->docklet)) {

    GM_docklet_hide (pw->gw->docklet);
    GTK_TOGGLE_BUTTON (pw->show_docklet)->active = FALSE;
  }
  else {

    GM_docklet_show (pw->gw->docklet);
    GTK_TOGGLE_BUTTON (pw->show_docklet)->active = TRUE;
  }
}


/* The functions */

void gnomemeeting_menu_init (GtkWidget *gapp, GM_window_widgets *gw, 
			     GM_pref_window_widgets *pw)
{
  static GnomeUIInfo file_menu_uiinfo [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("_Connect"), N_("Create A New Connection"),
	(void *) connect_cb, gw, NULL,
	GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_OPEN,
	'c', GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("_Disconnect"), N_("Close The Current Connection"),
	(void *) disconnect_cb, gw, NULL,
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
  
  
  static GnomeUIInfo notebook_view_uiinfo [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("_Remote User Info"), N_("View Remote User Info"),
	(void *) view_remote_user_info_callback, gw, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	NULL, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("_History"), N_("View the log"),
	(void *) view_log_callback, gw, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	NULL, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("_Audio Settings"), N_("View Audio Settings"),
	(void *) view_audio_settings_callback, gw, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	NULL, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("_Video Settings"), N_("View Video Settings"),
	(void *) view_video_settings_callback, gw, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	NULL, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };
  
  
  static GnomeUIInfo view_menu_uiinfo [] =
    {
      {
	GNOME_APP_UI_RADIOITEMS,
	NULL, NULL,
	notebook_view_uiinfo, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_SEPARATOR,
      {
	GNOME_APP_UI_TOGGLEITEM,
	N_("_Control Panel"), N_("View/Hide the Control Panel"),
	(void *) view_notebook_callback, pw, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	NULL, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_TOGGLEITEM,
	N_("_Status Bar"), N_("View/Hide the Status Bar"),
	(void *) view_statusbar_callback, pw, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	NULL, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_TOGGLEITEM,
	N_("_Quick Access Bar"), N_("View/Hide the Quick Access Bar"),
	(void *) view_quickbar_callback, pw, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	NULL, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_TOGGLEITEM,
	N_("_Docklet"), N_("View/Hide the Docklet"),
	(void *) view_docklet_callback, pw, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	NULL, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };
  
  
  static GnomeUIInfo settings_menu_uiinfo [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("_Settings"), N_("Change Your Preferences"),
	(void *) pref_callback, pw, NULL,
	GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_PREF,
	's', GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };
  
  
  static GnomeUIInfo help_menu_uiinfo [] =
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
  
  
  static GnomeUIInfo main_menu_uiinfo [] =
    {
      GNOMEUIINFO_SUBTREE (N_("_File"), file_menu_uiinfo),
      GNOMEUIINFO_SUBTREE (N_("_View"), view_menu_uiinfo),
      GNOMEUIINFO_SUBTREE (N_("_Settings"), settings_menu_uiinfo),
      GNOMEUIINFO_SUBTREE (N_("_Help"), help_menu_uiinfo),
      GNOMEUIINFO_END
    };
  
  
  gtk_object_set_data(GTK_OBJECT(gapp), "file_menu_uiinfo", file_menu_uiinfo);
  gtk_object_set_data(GTK_OBJECT(gapp), "settings_menu_uiinfo", 
		      settings_menu_uiinfo);
  gtk_object_set_data(GTK_OBJECT(gapp), "notebook_view_uiinfo", 
		      notebook_view_uiinfo);
  gtk_object_set_data(GTK_OBJECT(gapp), "view_menu_uiinfo", 
		      view_menu_uiinfo);

  gnome_app_create_menus (GNOME_APP (gapp), main_menu_uiinfo);
  gnome_app_install_menu_hints (GNOME_APP (gapp), main_menu_uiinfo);

  GTK_CHECK_MENU_ITEM (view_menu_uiinfo [2].widget)->active = 
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->show_notebook));
  GTK_CHECK_MENU_ITEM (view_menu_uiinfo [3].widget)->active = 
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->show_statusbar));
  GTK_CHECK_MENU_ITEM (view_menu_uiinfo [4].widget)->active =
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->show_quickbar));
  GTK_CHECK_MENU_ITEM (view_menu_uiinfo [5].widget)->active =
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->show_docklet));
}


void gnomemeeting_popup_menu_init (GtkWidget *widget, GM_window_widgets *gw)
{
  GtkWidget *popup_menu_widget;

  static GnomeUIInfo zoom_uiinfo[] =
    {
      { 
	GNOME_APP_UI_ITEM, N_("1:2"), N_("Zoom 1/2x"),
	(void *) half_zoom_callback, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL 
      },
      { 
	GNOME_APP_UI_ITEM, N_("1:1"), N_("Normal"),
	(void *) normal_zoom_callback, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL 
      },
      { 
	GNOME_APP_UI_ITEM, N_("2:1"), N_("Zoom 2x"),
	(void *) double_zoom_callback, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL 
      },
      GNOMEUIINFO_END
    };

  
  static GnomeUIInfo display_uiinfo[] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("Local"), N_("Local Video Image"),
	(void *) popup_menu_local_callback, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("Remote"), N_("Remote Video Image"),
	(void *) popup_menu_remote_callback, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("Both"), N_("Both Video Images"),
	(void *) popup_menu_both_callback, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };


  static GnomeUIInfo popup_menu_uiinfo [] =
    {
      {
	GNOME_APP_UI_RADIOITEMS,
	NULL, NULL,
	display_uiinfo, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_SEPARATOR,
      {
	GNOME_APP_UI_RADIOITEMS,
	NULL, NULL,
	zoom_uiinfo, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };


  /* Create a popup menu to attach it to the drawing area */
  popup_menu_widget = gnome_popup_menu_new (popup_menu_uiinfo);

  gnome_popup_menu_attach(popup_menu_widget, GTK_WIDGET (widget),
			  gw);

  GTK_CHECK_MENU_ITEM (zoom_uiinfo [0].widget)->active = FALSE;
  GTK_CHECK_MENU_ITEM (zoom_uiinfo [1].widget)->active = TRUE;
  GTK_CHECK_MENU_ITEM (zoom_uiinfo [2].widget)->active = FALSE;

  gtk_object_set_data (GTK_OBJECT (gm), "display_uiinfo", 
		       display_uiinfo);
}


void gnomemeeting_ldap_popup_menu_init (GtkWidget *widget, 
					GM_ldap_window_widgets *lw)
{
  GtkWidget *popup_menu_widget;

  static GnomeUIInfo popup_menu [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("Call This User"), N_("Call the selected user"),
	(void *)ldap_popup_menu_callback, GINT_TO_POINTER(0), NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };

  /* Create a popup menu to attach it to the drawing area */
  popup_menu_widget = gnome_popup_menu_new (popup_menu);
  gnome_popup_menu_attach (popup_menu_widget, GTK_WIDGET (widget),
                           lw);
}

/******************************************************************************/
