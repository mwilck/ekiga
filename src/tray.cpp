
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2002 Damien Sandras
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
 *                         tray.c  -  description
 *                         ---------------------------
 *   begin                : Wed Oct 3 2001
 *   copyright            : (C) 2000-2002 by Miguel Rodríguez
 *   description          : This file contains all functions needed for
 *                          system tray icon.
 *   email                : migrax@terra.es (all the new code)
 *                          dsandras@seconix.com (old applet code).
 *
 */

#include "../config.h"

#include "tray.h"
#include "eggtrayicon.h"
#include "gnomemeeting.h"
#include "menu.h"
#include "callbacks.h"
#include "misc.h"

#include <gconf/gconf-client.h>

#include "stock-icons.h"

/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

static void tray_popup_menu_connect_callback (GtkWidget *, gpointer);
static void tray_popup_menu_disconnect_callback (GtkWidget *, gpointer);
static void tray_toggle_callback (GtkWidget *, gpointer);
static int tray_clicked (GtkWidget *, GdkEventButton *, gpointer);
static void gnomemeeting_init_tray_popup_menu (GtkWidget *);
static void gnomemeeting_build_tray (GtkContainer *);

/* GTK Callbacks */

/* DESCRIPTION  :  This callback is called when the user chooses
 *                 to connect in the docklet menu.
 * BEHAVIOR     :  Answer incoming call or call somebody
 * PRE          :  /
 */
void tray_popup_menu_connect_callback (GtkWidget *, gpointer)
{
  MyApp->Connect ();
}


/* DESCRIPTION  :  This callback is called when the user chooses
 *                 to disconnect in the docklet menu.
 * BEHAVIOR     :  Refuse incoming call or stops current call
 * PRE          :  /
 */
void tray_popup_menu_disconnect_callback (GtkWidget *, gpointer)
{
  MyApp->Disconnect ();
}


/* DESCRIPTION  :  This callback is called when the user chooses
 *                 toggle in the tray icon menu
 * BEHAVIOR     :  Hide or show main window
 * PRE          :  /
 */
void tray_toggle_callback (GtkWidget *, gpointer)
{
  if (GTK_WIDGET_VISIBLE (GTK_WIDGET (gm))) {
    gtk_widget_hide (gm);
  }
  else {
    gtk_widget_show (gm);
  }
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the tray icon.
 * BEHAVIOR     :  If double clic : hide or show main window.
 * PRE          :  /
*/
int tray_clicked (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  g_return_val_if_fail (event != NULL, false);

  if ((event->button == 1) && (event->type == GDK_BUTTON_PRESS)) {
    tray_toggle_callback (widget, data);
    return true;
  }

  return false;
}


/* The functions  */

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates the popup menu and attach it to the GtkWidget
 *                 given as parameter (for the docklet).
 * PRE          :  /
 */
void gnomemeeting_init_tray_popup_menu (GtkWidget *widget)
{
  GtkWidget *popup_menu_widget;
  
  static GnomeUIInfo popup_menu [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("Connect"), N_("Create A New Connection"),
	(void *)tray_popup_menu_connect_callback, GINT_TO_POINTER(0), NULL,
	GNOME_APP_PIXMAP_STOCK, GM_STOCK_CONNECT,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("Disconnect"), N_("Close The Current Connection"),
	(void *)tray_popup_menu_disconnect_callback, GINT_TO_POINTER(1), 
	NULL,
	GNOME_APP_PIXMAP_STOCK, GM_STOCK_DISCONNECT,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("Show/Hide Main Window"), N_("Show/hide The Main Window"),
	(void *)tray_toggle_callback, GINT_TO_POINTER(2), NULL,
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

/* DESCRIPTION  :  Builds up the tray icon
 * BEHAVIOR     :  Adds needed widgets to the docklet window
 * PRE          :  docklet must be a valid pointer to a GtkWindow
 */
static void gnomemeeting_build_tray (GtkContainer *tray_icon)
{
  GtkWidget *image;

  image = gtk_image_new_from_stock (GM_STOCK_TRAY_DEFAULT, GTK_ICON_SIZE_SMALL_TOOLBAR);

  GtkWidget *eventbox = gtk_event_box_new ();
  
  gtk_widget_set_events (GTK_WIDGET (eventbox), 
			 gtk_widget_get_events (eventbox)
			 | GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK);
  
  g_signal_connect (G_OBJECT (eventbox), "button_press_event",
		    G_CALLBACK (tray_clicked), NULL);
  
  gtk_widget_show (image);
  gtk_widget_show (eventbox);
  
  /* add the status to the plug */
  g_object_set_data (G_OBJECT (tray_icon), "pixmapg", image);
  gtk_container_add (GTK_CONTAINER (eventbox), image);
  gtk_container_add (tray_icon, eventbox);
  
  /* Add the popup menu to the plug */
  gnomemeeting_init_tray_popup_menu (GTK_WIDGET (eventbox));
}


/* DESCRIPTION  :  Creates the tray icon widget
 * BEHAVIOR     :  Creater the tray icon and sets it up
 * PRE          :  /
 */
GObject *gnomemeeting_init_tray ()
{
  EggTrayIcon *tray_icon;
  GConfClient *client = gconf_client_get_default ();

  tray_icon = egg_tray_icon_new ("GnomeMeeting Tray Icon");

  gnomemeeting_build_tray (GTK_CONTAINER (tray_icon));
  
  if (gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_docklet", 0))
    gnomemeeting_tray_show (G_OBJECT (tray_icon));

  return G_OBJECT (tray_icon);
}


/* DESCRIPTION  :  Changes the image displayed in the tray icon
 * BEHAVIOR     :  /
 * PRE          :  Choice must be 0 or 1.
 */
void gnomemeeting_tray_set_content (GObject *tray, int choice)
{
  gpointer image = NULL;

  /* if choice = 0, set the world as content
     if choice = 1, set the globe2 as content */
  if (choice == 0)  {
    image = g_object_get_data (tray, "pixmapm");
  
    /* if the world was not already the pixmap */
    if (image != NULL)	{
      gtk_image_set_from_stock (GTK_IMAGE (image), GM_STOCK_TRAY_DEFAULT, 
				GTK_ICON_SIZE_SMALL_TOOLBAR);

      g_object_set_data (tray, "pixmapm", NULL);
      g_object_set_data (tray, "pixmapg", image);
    }
  }

  if (choice == 1) {

    image = g_object_get_data (tray,
			       "pixmapg");
    
    if (image != NULL)	{
      gtk_image_set_from_stock (GTK_IMAGE (image), GM_STOCK_TRAY_FLASH,
				GTK_ICON_SIZE_SMALL_TOOLBAR);

      g_object_set_data (tray, "pixmapg", NULL);
      g_object_set_data (tray, "pixmapm", image);
    }
  }
}


/* DESCRIPTION  :  Shows the tray icon
 * BEHAVIOR     :  /
 * PRE          :  /
 */
void gnomemeeting_tray_show (GObject *tray)
{
  gtk_widget_show (GTK_WIDGET (tray));
}


/* DESCRIPTION  :  Hides the tray icon
 * BEHAVIOR     :  /
 * PRE          :  /
 */
void gnomemeeting_tray_hide (GObject *tray)
{
  gtk_widget_hide (GTK_WIDGET (tray));
}


/* DESCRIPTION  :  Changes the content of the tray icon
 *                 based on the current one. Calling this function
 *                 from a timer created a flash effect in the tray icon.
 * BEHAVIOR     :  /
 * PRE          :  /
 */
gint gnomemeeting_tray_flash (GObject *tray)
{
  gpointer object;

  /* we can't call gnomemeeting_threads_enter as idles and timers
     are executed in the main thread */
  gdk_threads_enter ();
  object = g_object_get_data (tray, "pixmapg");

  if (object != NULL) {
    gnomemeeting_tray_set_content (tray, 1);
  } else {
    gnomemeeting_tray_set_content (tray, 0);
  }
  gdk_threads_leave ();

  return TRUE;
}
