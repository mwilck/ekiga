
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
static void tray_popup_menu_show_callback (GtkWidget *, gpointer);
static int tray_clicked (GtkWidget *, GdkEventButton *, gpointer);
static void gnomemeeting_init_tray_popup_menu (GtkWidget *);
static void gnomemeeting_build_tray (GtkContainer *);
static void tray_popup_menu_dnd_callback (GtkWidget *, gpointer);
static void tray_popup_menu_aa_callback (GtkWidget *, gpointer);


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
void tray_popup_menu_show_callback (GtkWidget *menu_item, gpointer)
{
  bool visible = GTK_WIDGET_VISIBLE (GTK_WIDGET (gm));

  if (visible) {
    gtk_widget_hide (gm);
  }
  else {
    gtk_widget_show (gm);
  }

  GTK_CHECK_MENU_ITEM (menu_item)->active = visible;
  gtk_widget_queue_draw (GTK_WIDGET (menu_item));
}

/* DESCRIPTION  :  This callback is called when the user chooses
 *                 do not disturb or auto_answerin the tray icon menu
 * BEHAVIOR     :  Hide or show main window
 * PRE          :  /
 */
void tray_popup_menu_dnd_callback (GtkWidget *widget, gpointer)
{
  GConfClient *client = gconf_client_get_default ();

  gconf_client_set_bool (client, 
			 "/apps/gnomemeeting/general/do_not_disturb",
			 gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget)),
			 0);
}

/* DESCRIPTION  :  This callback is called when the user chooses
 *                 auto answer or auto_answerin the tray icon menu
 * BEHAVIOR     :  Hide or show main window
 * PRE          :  /
 */
void tray_popup_menu_aa_callback (GtkWidget *widget, gpointer)
{
  GConfClient *client = gconf_client_get_default ();

  gconf_client_set_bool (client, 
			 "/apps/gnomemeeting/general/auto_answer",
			 gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget)),
			 0);
}

/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the tray icon.
 * BEHAVIOR     :  If double clic : hide or show main window.
 * PRE          :  data is a pointer to the activated popup widget
*/
int tray_clicked (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  g_return_val_if_fail (event != NULL, false);

  if ((event->button == 1) && (event->type == GDK_BUTTON_PRESS)) {
    tray_popup_menu_show_callback (GTK_WIDGET (data), NULL);
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
	(void *)tray_popup_menu_connect_callback, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, GM_STOCK_CONNECT,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("Disconnect"), N_("Close The Current Connection"),
	(void *)tray_popup_menu_disconnect_callback, NULL, 
	NULL,
	GNOME_APP_PIXMAP_STOCK, GM_STOCK_DISCONNECT,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_SEPARATOR,
      {
	GNOME_APP_UI_TOGGLEITEM,
	N_("Do not Disturb"), N_("Do Not Accept Calls"),
	(void *)tray_popup_menu_dnd_callback,
	NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_TOGGLEITEM,
	N_("Auto Answer"), N_("Automatically Answer Calls"),
	(void *)tray_popup_menu_aa_callback, NULL,
	NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_SEPARATOR,
      {
	GNOME_APP_UI_TOGGLEITEM,
	N_("Show Main Window"), N_("Show The Main Window"),
	(void *)tray_popup_menu_show_callback, NULL,
	NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      GNOMEUIINFO_END
    };

  /* Create a popup menu to attach it to the drawing area */
  popup_menu_widget = gnome_popup_menu_new (popup_menu);
  gnome_popup_menu_attach (popup_menu_widget, GTK_WIDGET (widget),
                           NULL);

  /* Set the state of toggle items */
  GConfClient *client = gconf_client_get_default ();

  GTK_CHECK_MENU_ITEM (popup_menu[3].widget)->active = 
    gconf_client_get_bool (client, 
			   "/apps/gnomemeeting/general/do_not_disturb", 0);
  GTK_CHECK_MENU_ITEM (popup_menu[4].widget)->active = 
    gconf_client_get_bool (client, 
			   "/apps/gnomemeeting/general/auto_answer", 0);
  GTK_CHECK_MENU_ITEM (popup_menu[6].widget)->active =
    !gconf_client_get_bool (client,
			   "/apps/gnomemeeting/view/start_docked", 0);

  gtk_widget_queue_draw (GTK_WIDGET (popup_menu[3].widget));
  gtk_widget_queue_draw (GTK_WIDGET (popup_menu[4].widget));
  gtk_widget_queue_draw (GTK_WIDGET (popup_menu[6].widget));

  g_object_set_data (G_OBJECT (widget), "tray_menu_uiinfo",
		     popup_menu);
}

/* DESCRIPTION  :  Builds up the tray icon
 * BEHAVIOR     :  Adds needed widgets to the docklet window
 * PRE          :  docklet must be a valid pointer to a GtkWindow
 */
static void gnomemeeting_build_tray (GtkContainer *tray_icon)
{
  GtkWidget *image;

  /* Add the popup menu to the tray */
  gnomemeeting_init_tray_popup_menu (GTK_WIDGET (tray_icon));

  image = gtk_image_new_from_stock (GM_STOCK_PANEL_AVAILABLE,
				    GTK_ICON_SIZE_MENU);

  GtkWidget *eventbox = gtk_event_box_new ();
  
  gtk_widget_set_events (GTK_WIDGET (eventbox), 
			 gtk_widget_get_events (eventbox)
			 | GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK);
  
  g_signal_connect (G_OBJECT (eventbox), "button_press_event",
		    G_CALLBACK (tray_clicked),
		    gnomemeeting_tray_get_uiinfo (G_OBJECT (tray_icon), 6));
  /* 6 is the position of the show docklet entry in the popup menu */
  
  gtk_widget_show (image);
  gtk_widget_show (eventbox);
  
  /* add the status to the plug */
  g_object_set_data (G_OBJECT (tray_icon), "image", image);
  g_object_set_data (G_OBJECT (tray_icon), "available", GINT_TO_POINTER (1));
  gtk_container_add (GTK_CONTAINER (eventbox), image);
  gtk_container_add (tray_icon, eventbox);
}


/* The functions */
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


void gnomemeeting_tray_set_content (GObject *tray, int choice)
{
  gpointer image = NULL;

  /* if choice = 0, set the phone as content
     if choice = 1, set the ringing phone as content */
  if (choice == 0)  {
    image = g_object_get_data (tray, "image");
  
    /* if that was was not already the pixmap */
    if (image != NULL)	{
      gtk_image_set_from_stock (GTK_IMAGE (image), GM_STOCK_PANEL_AVAILABLE, 
				GTK_ICON_SIZE_MENU);
      g_object_set_data (tray, "available", GINT_TO_POINTER (1));
    }
  }

  if (choice == 1) {

    image = g_object_get_data (tray, "image");
    
    if (image != NULL)	{
      gtk_image_set_from_stock (GTK_IMAGE (image), GM_STOCK_PANEL_RING,
				GTK_ICON_SIZE_MENU);
      g_object_set_data (tray, "available", GINT_TO_POINTER (0));
    }
  }

  if (choice == 2) {

    image = g_object_get_data (tray, "image");

    if (image != NULL) {
      gtk_image_set_from_stock (GTK_IMAGE (image), GM_STOCK_PANEL_BUSY,
				GTK_ICON_SIZE_MENU);
      g_object_set_data (tray, "available", GINT_TO_POINTER (0));
    }
  }
}


void gnomemeeting_tray_show (GObject *tray)
{
  gtk_widget_show (GTK_WIDGET (tray));
}


void gnomemeeting_tray_hide (GObject *tray)
{
  gtk_widget_hide (GTK_WIDGET (tray));
}


gint gnomemeeting_tray_flash (GObject *tray)
{
  gpointer data;

  /* we can't call gnomemeeting_threads_enter as idles and timers
     are executed in the main thread */
  gdk_threads_enter ();
  data = g_object_get_data (tray, "available");

  if (GPOINTER_TO_INT (data) == 1) {
    gnomemeeting_tray_set_content (tray, 1);
  } else {
    gnomemeeting_tray_set_content (tray, 0);
  }
  gdk_threads_leave ();

  return TRUE;
}

/* DESCRIPTION  : Returns a popup menu item
 * BEHAVIOR     : 
 * PRE          : /
 */
GObject *gnomemeeting_tray_get_uiinfo (GObject *tray, int index)
{
  GnomeUIInfo *uiinfo = (GnomeUIInfo *) g_object_get_data (tray, 
							   "tray_menu_uiinfo");
  g_return_val_if_fail (uiinfo != NULL, NULL);

 return G_OBJECT (uiinfo[index].widget);
}
