
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

#include "../config.h"

#include "callbacks.h"
#include "menu.h"
#include "common.h"
#include "docklet.h"
#include "misc.h"
#include "druid.h"

#include <gconf/gconf-client.h>

#include "../pixmaps/connect_16.xpm"
#include "../pixmaps/disconnect_16.xpm"


/* Declarations */

extern GtkWidget *gm;

static void gnomemeeting_init_druid_callback (GtkWidget *, gpointer);
static void half_zoom_callback (GtkWidget *, gpointer);
static void normal_zoom_callback (GtkWidget *, gpointer);
static void double_zoom_callback (GtkWidget *, gpointer);


/* GTK Callbacks */

/* DESCRIPTION  :  This callback is called when the druid is called from the
 *                 Edit menu.
 * BEHAVIOR     :  Builds the druid.
 * PRE          :  gpointer is a valid pointer to a char* containing "menu"
 *                 to indicate that we are called from the menu.
 */
static void gnomemeeting_init_druid_callback (GtkWidget *w, gpointer data)
{
  gnomemeeting_init_druid (data);
}


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
 *                 corresponding option in the View Menu (it is a toggle menu)
 * BEHAVIOR     :  Shows the notebook with the correct page.
 * PRE          :  gpointer is a valid pointer to a GonfClient structure.
 */
static void view_menu_toggles_changed (GtkWidget *widget, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();
  GnomeUIInfo *notebook_view_uiinfo =
    (GnomeUIInfo *) g_object_get_data (G_OBJECT (gm),
				       "notebook_view_uiinfo");
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  gtk_widget_show_all (gw->main_notebook);
  gconf_client_set_bool (client, "/apps/gnomemeeting/view/show_control_panel", 1, NULL);

  /* Only do something when a new CHECK_MENU_ITEM becomes active,
     not when it becomes inactive */
  if (GTK_CHECK_MENU_ITEM (widget)->active) {

    for (int i = 0; i < 3; i++) 
      if (GTK_CHECK_MENU_ITEM (notebook_view_uiinfo[i].widget)->active) 
	gtk_notebook_set_current_page (GTK_NOTEBOOK (gw->main_notebook), i);
  }
}


/* DESCRIPTION  :  This callback is called when the user toggles the 
 *                 corresponding option in the View Menu. (it is a check menu)
 * BEHAVIOR     :  Updates the gconf cache.
 * PRE          :  data is the key.
 */
static void menu_toggle_changed (GtkWidget *widget, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();

  gconf_client_set_bool (client,
			 (gchar *) data,
			 GTK_CHECK_MENU_ITEM (widget)->active, NULL);
}


/* The functions */

void gnomemeeting_init_menu ()
{
  /* Get the data */
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);
  GConfClient *client = gconf_client_get_default ();
  GtkWidget *menu = NULL;


  static GnomeUIInfo file_menu_uiinfo [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("_Save"), N_("Save A Snapshot of the Current Video Stream"),
	(void *)save_callback, gw, NULL,
	GNOME_APP_PIXMAP_STOCK, GTK_STOCK_SAVE,
	'S', GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_SEPARATOR,
      {
	GNOME_APP_UI_ITEM,
	N_("_Quit"), N_("Quit GnomeMeeting"),
	(void *)quit_callback, gw, NULL,
	GNOME_APP_PIXMAP_STOCK, GTK_STOCK_QUIT,
	'Q', GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };
  
  
  static GnomeUIInfo notebook_view_uiinfo [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("History"), N_("View the log"),
	(void *) view_menu_toggles_changed, 
	NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("_Audio Settings"), N_("View Audio Settings"),
	(void *) view_menu_toggles_changed, 
	NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("_Video Settings"), N_("View Video Settings"),
	(void *) view_menu_toggles_changed, 
	NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };
  
  
  static GnomeUIInfo view_menu_uiinfo [] =
    {
      {
	GNOME_APP_UI_RADIOITEMS,
	NULL, NULL,
	(void *) notebook_view_uiinfo, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_SEPARATOR,
      {
	GNOME_APP_UI_TOGGLEITEM,
	N_("Control Panel"), N_("View/Hide the Control Panel"),
	(void *) menu_toggle_changed, 
	(gpointer) "/apps/gnomemeeting/view/show_control_panel",
	NULL, GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_TOGGLEITEM,
	N_("Chat Window"), N_("View/Hide the Chat Window"),
	(void *) menu_toggle_changed, 
	(gpointer) "/apps/gnomemeeting/view/show_chat_window",
	NULL, GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_TOGGLEITEM,
	N_("Status Bar"), N_("View/Hide the Status Bar"),
	(void *) menu_toggle_changed, 
	(gpointer) "/apps/gnomemeeting/view/show_status_bar",
	NULL, GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_TOGGLEITEM,
	N_("Docklet"), N_("View/Hide the Docklet"),
	(void *) menu_toggle_changed, 
	(gpointer) "/apps/gnomemeeting/view/show_docklet",
	NULL, GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_TOGGLEITEM,
	N_("Left Toolbar"), N_("View/Hide the left Toolbar"),
	(void *) menu_toggle_changed, 
	(gpointer) "/apps/gnomemeeting/view/left_toolbar",
	NULL, GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };  
  
  static GnomeUIInfo settings_menu_uiinfo [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("Configuration Druid"), N_("Start The Configuration Druid"),
	(void *) gnomemeeting_init_druid_callback, (gpointer) "menu", NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_SEPARATOR,
      {
	GNOME_APP_UI_ITEM,
	N_("_Preferences..."), N_("Change Your Preferences"),
	(void *) gnomemeeting_component_view, gw->pref_window, NULL,
	GNOME_APP_PIXMAP_STOCK, GTK_STOCK_PREFERENCES,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };
  

  static GnomeUIInfo call_menu_uiinfo [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("_Connect"), N_("Create A New Connection"),
	(void *) connect_cb, gw, NULL,
	GNOME_APP_PIXMAP_DATA, connect_16_xpm,
	'c', GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("_Disconnect"), N_("Close The Current Connection"),
	(void *) disconnect_cb, gw, NULL,
	GNOME_APP_PIXMAP_DATA, disconnect_16_xpm,
	'd', GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_SEPARATOR,
      {
	GNOME_APP_UI_TOGGLEITEM,
	N_("Do _Not Disturb"), N_("Do Not Disturb"),
	(void *) menu_toggle_changed, 
	(gpointer) "/apps/gnomemeeting/general/do_not_disturb", NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	'n', GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_TOGGLEITEM,
	N_("Aut_o Answer"), N_("Auto Answer"),
	(void *) menu_toggle_changed, 
	(gpointer) "/apps/gnomemeeting/general/auto_answer", NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	'o', GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_SEPARATOR,
      {
	GNOME_APP_UI_ITEM,
	N_("_Audio Mute"), N_("Mute the audio transmission"),
	(void *) pause_audio_callback, gw, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	'u', GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("_Video Mute"), N_("Mute the video transmission"),
	(void *) pause_video_callback, gw, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	'i', GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };


  static GnomeUIInfo help_menu_uiinfo [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("_About GnomeMeeting"), N_("View information about GnomeMeeting"),
	(void *)about_callback, gm, NULL,
	GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_ABOUT,
	'A', GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };
  
  
  static GnomeUIInfo main_menu_uiinfo [] =
    {
      GNOMEUIINFO_SUBTREE (N_("_File"), file_menu_uiinfo),
      GNOMEUIINFO_SUBTREE (N_("_Edit"), settings_menu_uiinfo),
      GNOMEUIINFO_SUBTREE (N_("_View"), view_menu_uiinfo),
      GNOMEUIINFO_SUBTREE (N_("C_all"), call_menu_uiinfo),
      GNOMEUIINFO_SUBTREE (N_("_Help"), help_menu_uiinfo),
      GNOMEUIINFO_END
    };
  
  
  g_object_set_data(G_OBJECT(gm), "file_menu_uiinfo", file_menu_uiinfo);
  g_object_set_data(G_OBJECT(gm), "settings_menu_uiinfo", 
		    settings_menu_uiinfo);
  g_object_set_data(G_OBJECT(gm), "notebook_view_uiinfo", 
		    notebook_view_uiinfo);
  g_object_set_data(G_OBJECT(gm), "view_menu_uiinfo", 
		    view_menu_uiinfo);
  g_object_set_data(G_OBJECT(gm), "call_menu_uiinfo", 
		    call_menu_uiinfo);

  gnome_app_create_menus (GNOME_APP (gm), main_menu_uiinfo);
  gnome_app_install_menu_hints (GNOME_APP (gm), main_menu_uiinfo);


  /* Update to the initial values */
  GTK_CHECK_MENU_ITEM (view_menu_uiinfo [2].widget)->active = 
    gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_control_panel", 0);

  GTK_CHECK_MENU_ITEM (view_menu_uiinfo [3].widget)->active = 
    gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_chat_window", 0);

  GTK_CHECK_MENU_ITEM (view_menu_uiinfo [4].widget)->active = 
    gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_status_bar", 0);

  GTK_CHECK_MENU_ITEM (view_menu_uiinfo [5].widget)->active =
    gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_docklet", 0);

  GTK_CHECK_MENU_ITEM (view_menu_uiinfo [6].widget)->active =
    gconf_client_get_bool (client, "/apps/gnomemeeting/view/left_toolbar", 0);

  GTK_CHECK_MENU_ITEM (call_menu_uiinfo [3].widget)->active =
    gconf_client_get_bool (client, "/apps/gnomemeeting/general/do_not_disturb", 0);

  GTK_CHECK_MENU_ITEM (call_menu_uiinfo [4].widget)->active =
    gconf_client_get_bool (client, "/apps/gnomemeeting/general/auto_answer", 0);

  if (GTK_CHECK_MENU_ITEM (call_menu_uiinfo [3].widget)->active)
    gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [4].widget), FALSE);

  if (GTK_CHECK_MENU_ITEM (call_menu_uiinfo [4].widget)->active)
    gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [3].widget), FALSE);


  /* Disable disconnect */
  gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [1].widget), FALSE);

  
  /* Pause is unsensitive when not in a call */
  gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [6].widget), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [7].widget), FALSE);
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

  g_object_set_data (G_OBJECT (gm), "display_uiinfo", 
		     display_uiinfo);
}

