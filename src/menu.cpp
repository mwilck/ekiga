
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
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          :  Functions to create the menus.
 *   email                : dsandras@seconix.com
 *
 */

#include "../config.h"

#include "callbacks.h"
#include "menu.h"
#include "gnomemeeting.h"
#include "common.h"
#include "docklet.h"
#include "misc.h"
#include "druid.h"

#include <gconf/gconf-client.h>

#include "../pixmaps/connect_16.xpm"
#include "../pixmaps/disconnect_16.xpm"


/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;

static void gnomemeeting_init_druid_callback (GtkWidget *, gpointer);
static void half_zoom_callback (GtkWidget *, gpointer);
static void normal_zoom_callback (GtkWidget *, gpointer);
static void double_zoom_callback (GtkWidget *, gpointer);
static void toggle_fullscreen_callback (GtkWidget *, gpointer);
static void video_view_changed_callback (GtkWidget *, gpointer);
static void view_menu_toggles_changed (GtkWidget *, gpointer);
static void menu_toggle_changed (GtkWidget *, gpointer);


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
 * PRE          :  /
 */
static void half_zoom_callback (GtkWidget *widget, gpointer data)
{
  GmWindow *gw = gnomemeeting_get_main_window (gm);

  if (gw->zoom / 2 >= 0.5)
    gw->zoom = gw->zoom / 2;
}


/* DESCRIPTION  :  This callback is called when the user chooses 1:2 as zoom
 *                 factor in the popup menu.
 * BEHAVIOR     :  Sets the zoom to 1. That zoom will be read in 
 *                 GDKVideoOutputDEvice.
 * PRE          :  gpointer is a valid pointer to a GmWindow structure.
 */
static void normal_zoom_callback (GtkWidget *widget, gpointer data)
{
  GmWindow *gw = gnomemeeting_get_main_window (gm);

  gw->zoom = 1;
}


/* DESCRIPTION  :  This callback is called when the user chooses 1:2 as zoom
 *                 factor in the popup menu.
 * BEHAVIOR     :  Sets the zoom to 2.00. That zoom will be read in 
 *                 GDKVideoOutputDEvice.
 * PRE          :  gpointer is a valid pointer to a GmWindow structure.
 */
static void double_zoom_callback (GtkWidget *widget, gpointer data)
{
  GmWindow *gw = gnomemeeting_get_main_window (gm);

  if (gw->zoom * 2 <= 2)
    gw->zoom = gw->zoom * 2;
}


/* DESCRIPTION  :  This callback is called when the user toggles fullscreen
 *                 factor in the popup menu.
 * BEHAVIOR     :  Toggles fullscreen.
 * PRE          :  gpointer is a valid pointer to a GmWindow structure.
 */
static void toggle_fullscreen_callback (GtkWidget *widget, gpointer data)
{
  GmWindow *gw = gnomemeeting_get_main_window (gm);
  gw->fullscreen = !gw->fullscreen;
}


/* DESCRIPTION  :  This callback is called when the user changes the current
 *                 video view.
 * BEHAVIOR     :  Disable fullscreen in menu if choice = Both images.
 * PRE          :  gpointer is a valid pointer to a GmWindow structure.
 */
static void video_view_changed_callback (GtkWidget *widget, gpointer data)
{
  GnomeUIInfo *right_menu_uiinfo = NULL;
  GnomeUIInfo *bad_menu_uiinfo = NULL;

  GnomeUIInfo *popup_menu_uiinfo = (GnomeUIInfo *)
    g_object_get_data (G_OBJECT(gm), "popup_menu_uiinfo");
  GnomeUIInfo *view_menu_uiinfo = (GnomeUIInfo *)
    g_object_get_data (G_OBJECT(gm), "view_menu_uiinfo");
  GnomeUIInfo *video_view_menu_uiinfo = (GnomeUIInfo *)
    g_object_get_data (G_OBJECT(gm), "video_view_menu_uiinfo");
  GnomeUIInfo *popup_video_view_menu_uiinfo = (GnomeUIInfo *)
    g_object_get_data (G_OBJECT(gm), "popup_video_view_menu_uiinfo");


  if (!strcmp ((char *) data, "view")) {

    right_menu_uiinfo = video_view_menu_uiinfo;
    bad_menu_uiinfo = popup_video_view_menu_uiinfo;
  }
  else {

    right_menu_uiinfo = popup_video_view_menu_uiinfo;
    bad_menu_uiinfo = video_view_menu_uiinfo;
  }

  for (int i = 0 ; i < 4 ; i++) {

    GTK_CHECK_MENU_ITEM (bad_menu_uiinfo [i].widget)->active =
      GTK_CHECK_MENU_ITEM (right_menu_uiinfo [i].widget)->active;
    gtk_widget_queue_draw (GTK_WIDGET (bad_menu_uiinfo [i].widget));
  }
    

  if (GTK_CHECK_MENU_ITEM (video_view_menu_uiinfo [2].widget)->active
      || GTK_CHECK_MENU_ITEM (video_view_menu_uiinfo [3].widget)->active) {

    gtk_widget_set_sensitive (GTK_WIDGET (popup_menu_uiinfo [6].widget), 
			      FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (view_menu_uiinfo [12].widget), 
			      FALSE);
  }
  else {

    gtk_widget_set_sensitive (GTK_WIDGET (popup_menu_uiinfo [6].widget), 
			      TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (view_menu_uiinfo [12].widget), 
			      TRUE);
  }
}


/* DESCRIPTION  :  This callback is called when the user toggles the 
 *                 corresponding option in the View Menu (it is a toggle menu)
 * BEHAVIOR     :  Sets the gconf key.
 * PRE          :  data is the gconf key.
 */
static void view_menu_toggles_changed (GtkWidget *widget, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();
  int active = 0;
  GnomeUIInfo *notebook_view_uiinfo =
    (GnomeUIInfo *) g_object_get_data (G_OBJECT (gm),
				       "notebook_view_uiinfo");

  /* Only do something when a new CHECK_MENU_ITEM becomes active,
     not when it becomes inactive */
  if (GTK_CHECK_MENU_ITEM (widget)->active) {

    for (int i = 0; i <= 3; i++) 
      if (GTK_CHECK_MENU_ITEM (notebook_view_uiinfo[i].widget)->active) 
 	active = i;
  }

  gconf_client_set_int (client, "/apps/gnomemeeting/view/control_panel_section", active, 0);
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
  GmWindow *gw = gnomemeeting_get_main_window (gm);
  GConfClient *client = gconf_client_get_default ();


  static GnomeUIInfo file_menu_uiinfo [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("_Save"), N_("Save A Snapshot of the Current Video"),
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
	NULL, (gpointer) "/apps/gnomemeeting/view/control_panel_section",
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("_Audio Settings"), N_("View Audio Settings"),
	(void *) view_menu_toggles_changed, 
	NULL, (gpointer) "/apps/gnomemeeting/view/control_panel_section",
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("_Video Settings"), N_("View Video Settings"),
	(void *) view_menu_toggles_changed, 
	NULL, (gpointer) "/apps/gnomemeeting/view/control_panel_section",
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("Off"), N_("Hide the control panel"),
	(void *) view_menu_toggles_changed, 
	NULL, (gpointer) "/apps/gnomemeeting/view/control_panel_section",
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };


  static GnomeUIInfo video_view_menu_uiinfo [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("Local Video"), N_("Local Video Image"),
	(void *) video_view_changed_callback, (gpointer) "view", NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("Remote Video"), N_("Remote Video Image"),
	(void *) video_view_changed_callback, (gpointer) "view", NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("Both (Local Video Incrusted)"), N_("Both Video Images"),
	(void *) video_view_changed_callback, (gpointer) "view", NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("Both (Local Video in New Window)"), N_("Both Video Images"),
	(void *) video_view_changed_callback, (gpointer) "view", NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END,
    };

 
  static GnomeUIInfo notebook_view_submenu_uiinfo [] =
    {
      {
	GNOME_APP_UI_RADIOITEMS,
	NULL, NULL,
	(void *) notebook_view_uiinfo, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };


  static GnomeUIInfo view_menu_uiinfo [] =
    {
      {
	GNOME_APP_UI_TOGGLEITEM,
	N_("Toolbar"), N_("View/Hide the Toolbar"),
	(void *) menu_toggle_changed, 
	(gpointer) "/apps/gnomemeeting/view/left_toolbar",
	NULL, GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_TOGGLEITEM,
	N_("Text Chat"), N_("View/Hide the Text Chat Window"),
	(void *) menu_toggle_changed, 
	(gpointer) "/apps/gnomemeeting/view/show_chat_window",
	NULL, GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_SUBTREE (N_("Control Panel"), notebook_view_submenu_uiinfo),
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
      GNOMEUIINFO_SEPARATOR,
      {
 	GNOME_APP_UI_RADIOITEMS,
 	NULL, NULL,
 	(void *) video_view_menu_uiinfo, NULL, NULL,
 	GNOME_APP_PIXMAP_NONE, NULL,
 	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_SEPARATOR,
      { 
	GNOME_APP_UI_ITEM, N_("Zoom In"), N_("Zoom In"),
	(void *) double_zoom_callback, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, GTK_STOCK_ZOOM_IN,
	'+', GDK_CONTROL_MASK, NULL 
      },
      { 
	GNOME_APP_UI_ITEM, N_("Zoom Out"), N_("Zoom Out"),
	(void *) half_zoom_callback, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, GTK_STOCK_ZOOM_OUT,
	'-', GDK_CONTROL_MASK, NULL 
      },
      { 
	GNOME_APP_UI_ITEM, N_("Normal Size"), N_("Normal Size"),
	(void *) normal_zoom_callback, NULL, NULL,
       	GNOME_APP_PIXMAP_STOCK, GTK_STOCK_ZOOM_100,
	'=', GDK_CONTROL_MASK, NULL 
      },
      GNOMEUIINFO_SEPARATOR,
      { 
	GNOME_APP_UI_ITEM, N_("Toggle Fullscreen"), N_("Toggle Fullscreen"),
	(void *) toggle_fullscreen_callback, NULL, NULL,
       	GNOME_APP_PIXMAP_STOCK, GTK_STOCK_ZOOM_FIT,
	'f', GDK_CONTROL_MASK, NULL 
      },
      GNOMEUIINFO_END
    };  
  

  static GnomeUIInfo settings_menu_uiinfo [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("Configuration Assistant"), N_("Start The Configuration Assistant"),
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
  g_object_set_data(G_OBJECT(gm), "video_view_menu_uiinfo", 
		    video_view_menu_uiinfo);

  gnome_app_create_menus (GNOME_APP (gm), main_menu_uiinfo);
  gnome_app_install_menu_hints (GNOME_APP (gm), main_menu_uiinfo);


  /* Update to the initial values */
  GTK_CHECK_MENU_ITEM (view_menu_uiinfo [0].widget)->active =
    gconf_client_get_bool (client, "/apps/gnomemeeting/view/left_toolbar", 0);
  GTK_CHECK_MENU_ITEM (view_menu_uiinfo [1].widget)->active = 
    gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_chat_window",
			   0);
  GTK_CHECK_MENU_ITEM (view_menu_uiinfo [3].widget)->active = 
    gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_status_bar", 
			   0);
  GTK_CHECK_MENU_ITEM (view_menu_uiinfo [4].widget)->active =
    gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_docklet", 0);

  
  for (int i = 0 ; i < 4 ; i++) {

    if (gconf_client_get_int (client, "/apps/gnomemeeting/view/control_panel_section", 0) == i) 
      GTK_CHECK_MENU_ITEM (notebook_view_uiinfo [i].widget)->active = TRUE;
    else
      GTK_CHECK_MENU_ITEM (notebook_view_uiinfo [i].widget)->active = FALSE;

    gtk_widget_queue_draw (GTK_WIDGET (notebook_view_uiinfo [i].widget));
  }
  

  GTK_CHECK_MENU_ITEM (call_menu_uiinfo [3].widget)->active =
    gconf_client_get_bool (client, "/apps/gnomemeeting/general/do_not_disturb", 0);
  GTK_CHECK_MENU_ITEM (call_menu_uiinfo [4].widget)->active =
    gconf_client_get_bool (client, "/apps/gnomemeeting/general/auto_answer", 
			   0);


  /* Disable disconnect */
  gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [1].widget), FALSE);


  /* Pause is unsensitive when not in a call */
  gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [6].widget), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [7].widget), FALSE);
}


void gnomemeeting_zoom_submenu_set_sensitive (gboolean b)
{
  GnomeUIInfo *view_menu_uiinfo = 
    (GnomeUIInfo *) g_object_get_data (G_OBJECT(gm), "view_menu_uiinfo");
  GnomeUIInfo *popup_menu_uiinfo = 
    (GnomeUIInfo *) g_object_get_data (G_OBJECT(gm), "popup_menu_uiinfo");

  gtk_widget_set_sensitive (GTK_WIDGET (view_menu_uiinfo [8].widget), b);
  gtk_widget_set_sensitive (GTK_WIDGET (view_menu_uiinfo [9].widget), b);
  gtk_widget_set_sensitive (GTK_WIDGET (view_menu_uiinfo [10].widget), b);
  gtk_widget_set_sensitive (GTK_WIDGET (view_menu_uiinfo [12].widget), b);
  gtk_widget_set_sensitive (GTK_WIDGET (popup_menu_uiinfo [6].widget), b);
  gtk_widget_set_sensitive (GTK_WIDGET (popup_menu_uiinfo [2].widget), b);
  gtk_widget_set_sensitive (GTK_WIDGET (popup_menu_uiinfo [3].widget), b);
  gtk_widget_set_sensitive (GTK_WIDGET (popup_menu_uiinfo [4].widget), b);
}


void gnomemeeting_fullscreen_option_set_sensitive (gboolean b)
{
  GnomeUIInfo *view_menu_uiinfo = 
    (GnomeUIInfo *) g_object_get_data (G_OBJECT(gm), "view_menu_uiinfo");
  GnomeUIInfo *popup_menu_uiinfo = 
    (GnomeUIInfo *) g_object_get_data (G_OBJECT(gm), "popup_menu_uiinfo");

  gtk_widget_set_sensitive (GTK_WIDGET (view_menu_uiinfo [12].widget), b);
  gtk_widget_set_sensitive (GTK_WIDGET (popup_menu_uiinfo [6].widget), b);
}


void gnomemeeting_video_submenu_set_sensitive (gboolean b)
{
  GnomeUIInfo *video_view_menu_uiinfo = 
    (GnomeUIInfo *) g_object_get_data (G_OBJECT(gm), "video_view_menu_uiinfo");
  GnomeUIInfo *popup_video_view_menu_uiinfo = 
    (GnomeUIInfo *) g_object_get_data (G_OBJECT(gm), 
				       "popup_video_view_menu_uiinfo");

  gtk_widget_set_sensitive (GTK_WIDGET (video_view_menu_uiinfo [1].widget), b);
  gtk_widget_set_sensitive (GTK_WIDGET (video_view_menu_uiinfo [2].widget), b);
  gtk_widget_set_sensitive (GTK_WIDGET (video_view_menu_uiinfo [3].widget), b);

  gtk_widget_set_sensitive (GTK_WIDGET (popup_video_view_menu_uiinfo [1].widget), b);
  gtk_widget_set_sensitive (GTK_WIDGET (popup_video_view_menu_uiinfo [2].widget), b);
  gtk_widget_set_sensitive (GTK_WIDGET (popup_video_view_menu_uiinfo [3].widget), b);
}


void gnomemeeting_video_submenu_select (int j)
{
  GnomeUIInfo *video_view_menu_uiinfo = 
    (GnomeUIInfo *) g_object_get_data (G_OBJECT(gm), "video_view_menu_uiinfo");
  GnomeUIInfo *popup_video_view_menu_uiinfo = 
    (GnomeUIInfo *) g_object_get_data (G_OBJECT(gm), 
				       "popup_video_view_menu_uiinfo");

  for (int i = 0 ; i < 4 ; i++) {

    GTK_CHECK_MENU_ITEM (video_view_menu_uiinfo [i].widget)->active = (i == j);
    gtk_widget_queue_draw (GTK_WIDGET (video_view_menu_uiinfo [i].widget)); 

    GTK_CHECK_MENU_ITEM (popup_video_view_menu_uiinfo [i].widget)->active 
      = (i == j);
    gtk_widget_queue_draw (GTK_WIDGET (popup_video_view_menu_uiinfo [i].widget)); 
  }
}


void gnomemeeting_popup_menu_init (GtkWidget *widget)
{
  GtkWidget *popup_menu_widget;

  /* Get the data */
  GmWindow *gw = gnomemeeting_get_main_window (gm);

  
  /* Need to redefine it, bug in GTK ? */
  static GnomeUIInfo popup_video_view_menu_uiinfo [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("Local Video"), N_("Local Video Image"),
	(void *) video_view_changed_callback, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("Remote Video"), N_("Remote Video Image"),
	(void *) video_view_changed_callback, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("Both (Local Video Incrusted)"), N_("Both Video Images"),
	(void *) video_view_changed_callback, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("Both (Local Video in New Window)"), N_("Both Video Images"),
	(void *) video_view_changed_callback, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END,
    };


  static GnomeUIInfo popup_menu_uiinfo [] =
    {
      {
	GNOME_APP_UI_RADIOITEMS,
	NULL, NULL,
	popup_video_view_menu_uiinfo, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_SEPARATOR,
      { 
	GNOME_APP_UI_ITEM, N_("Zoom In"), N_("Zoom In"),
	(void *) double_zoom_callback, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, GTK_STOCK_ZOOM_IN,
	'+', GDK_CONTROL_MASK, NULL 
      },
      { 
	GNOME_APP_UI_ITEM, N_("Zoom Out"), N_("Zoom Out"),
	(void *) half_zoom_callback, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, GTK_STOCK_ZOOM_OUT,
	'-', GDK_CONTROL_MASK, NULL 
      },
      { 
	GNOME_APP_UI_ITEM, N_("Normal Size"), N_("Normal Size"),
	(void *) normal_zoom_callback, NULL, NULL,
       	GNOME_APP_PIXMAP_STOCK, GTK_STOCK_ZOOM_100,
	'=', GDK_CONTROL_MASK, NULL 
      },
      GNOMEUIINFO_SEPARATOR,
      { 
	GNOME_APP_UI_ITEM, N_("Toggle Fullscreen"), N_("Toggle Fullscreen"),
	(void *) toggle_fullscreen_callback, NULL, NULL,
       	GNOME_APP_PIXMAP_STOCK, GTK_STOCK_ZOOM_FIT,
	'f', GDK_CONTROL_MASK, NULL 
      },
      GNOMEUIINFO_END
    };


  /* Create a popup menu to attach it to the drawing area */
  popup_menu_widget = gnome_popup_menu_new (popup_menu_uiinfo);

  gnome_popup_menu_attach (popup_menu_widget, GTK_WIDGET (widget),
			   gw);

  g_object_set_data (G_OBJECT (gm), "popup_menu_uiinfo", 
		     popup_menu_uiinfo);
  g_object_set_data (G_OBJECT (gm), "popup_video_view_menu_uiinfo", 
		     popup_video_view_menu_uiinfo);
}
