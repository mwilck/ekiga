
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2003 Damien Sandras
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
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         tray.cpp  -  description
 *                         ------------------------
 *   begin                : Wed Oct 3 2001
 *   copyright            : (C) 2000-2002 by Miguel Rodríguez
 *   description          : This file contains all functions needed for
 *                          system tray icon.
 *   Additional code      : migrax@terra.es (all the new code)
 *                          dsandras@seconix.com (old applet code).
 *
 */


#include "../config.h"

#include "tray.h"
#include "eggtrayicon.h"
#include "menu.h"
#include "stock-icons.h"


/* Declarations */

extern GtkWidget *gm;

static gint tray_clicked_callback (GtkWidget *, GdkEventButton *, gpointer);
static gint tray_icon_embedded (GObject *, gpointer);
static gint tray_icon_destroyed (GObject *, gpointer);
static void gnomemeeting_build_tray (GtkContainer *, GtkAccelGroup *);


/* The functions  */

/* DESCRIPTION  :  This callback is called when the tray appears on the panel
 * BEHAVIOR     :  Store the info in the object
 * PRE          :  
 */
static gint tray_icon_embedded (GObject *tray_icon, gpointer)
{
  static bool first_time = true;

  if (first_time) {
    GConfClient *client = gconf_client_get_default ();
    if (gconf_client_get_bool (client, VIEW_KEY "start_docked", 0)) {
      gtk_widget_hide (gm);
    }
    first_time = false;
  }
  g_object_set_data (tray_icon, "embedded", GINT_TO_POINTER (1));

  return true;
}

/* DESCRIPTION  :  This callback is called when the panel gets closed
 *                 after the tray has been embedded
 * BEHAVIOR     :  Create a new tray_icon and substitute the old one
 * PRE          :  A GtkAccelGroup
 */
static gint tray_icon_destroyed (GObject *tray, gpointer accel) 
{
  GConfClient *client = gconf_client_get_default ();

  /* Somehow the delete_event never got called, so we use "destroy" */
  if (tray != G_OBJECT (gnomemeeting_get_main_window (gm)->docklet))
    return true;
  GObject *new_tray = gnomemeeting_init_tray (GTK_ACCEL_GROUP (accel));

  if (gconf_client_get_bool 
      (client, GENERAL_KEY "do_not_disturb", 0)) 
    gnomemeeting_tray_set_content (new_tray, 2);

  
  gnomemeeting_get_main_window (gm)->docklet = GTK_WIDGET (new_tray);
  gtk_widget_show (gm);

  return true;
}

/* DESCRIPTION  :  This callback is called when the user double clicks on the 
 *                 tray event-box.
 * BEHAVIOR     :  Show / hide the GnomeMeeting GUI.
 * PRE          :  data != NULL.
 */
static gint
tray_clicked_callback (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  GdkEventButton *event_button = NULL;
  
  if (event->type == GDK_BUTTON_PRESS) {

    event_button = (GdkEventButton *) event;
    if (event_button->button == 1) {

      if (GTK_WIDGET_VISIBLE (GTK_WIDGET (gm)))
	gtk_widget_hide (gm);
      else
	gtk_widget_show (gm);
      
      return TRUE;
    }
  }
  
  return FALSE;
}


/* DESCRIPTION  :  Builds up the tray icon
 * BEHAVIOR     :  Adds needed widgets to the docklet window
 * PRE          :  docklet must be a valid pointer to a GtkWindow,
 *                 the GtkAccelGroup.
 */
static void 
gnomemeeting_build_tray (GtkContainer *tray_icon, GtkAccelGroup *accel)
{
  GtkWidget *image = NULL;
  GtkWidget *eventbox = NULL;

  /* Add the popup menu to the tray */
  gnomemeeting_popup_menu_tray_init (GTK_WIDGET (tray_icon), accel);

  image = gtk_image_new_from_stock (GM_STOCK_STATUS_AVAILABLE,
				    GTK_ICON_SIZE_MENU);

  eventbox = gtk_event_box_new ();
    
  gtk_widget_show (image);
  gtk_widget_show (eventbox);
  
  /* add the status to the plug */
  g_object_set_data (G_OBJECT (tray_icon), "image", image);
  g_object_set_data (G_OBJECT (tray_icon), "available", GINT_TO_POINTER (1));
  g_object_set_data (G_OBJECT (tray_icon), "embedded", GINT_TO_POINTER (0));
  gtk_container_add (GTK_CONTAINER (eventbox), image);
  gtk_container_add (tray_icon, eventbox);

  g_signal_connect (G_OBJECT (tray_icon), "embedded",
		    G_CALLBACK (tray_icon_embedded), NULL);
  g_signal_connect (G_OBJECT (tray_icon), "destroy",
		    G_CALLBACK (tray_icon_destroyed), accel);
  g_signal_connect (G_OBJECT (eventbox), "button_press_event",
		    G_CALLBACK (tray_clicked_callback), NULL);
}


/* The nctions */
GObject *gnomemeeting_init_tray (GtkAccelGroup *accel)
{
  EggTrayIcon *tray_icon;

#ifndef WIN32
  tray_icon = egg_tray_icon_new (_("GnomeMeeting Tray Icon"));
  gnomemeeting_build_tray (GTK_CONTAINER (tray_icon), accel);
  gnomemeeting_tray_show (G_OBJECT (tray_icon));
#endif
  
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
      gtk_image_set_from_stock (GTK_IMAGE (image), GM_STOCK_STATUS_AVAILABLE, 
				GTK_ICON_SIZE_MENU);
      g_object_set_data (tray, "available", GINT_TO_POINTER (1));
    }
  }

  if (choice == 1) {

    image = g_object_get_data (tray, "image");
    
    if (image != NULL)	{
      gtk_image_set_from_stock (GTK_IMAGE (image), GM_STOCK_STATUS_RINGING,
				GTK_ICON_SIZE_MENU);
      g_object_set_data (tray, "available", GINT_TO_POINTER (0));
    }
  }

  if (choice == 2) {

    image = g_object_get_data (tray, "image");

    if (image != NULL) {
      gtk_image_set_from_stock (GTK_IMAGE (image), GM_STOCK_STATUS_OCCUPIED,
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


gboolean gnomemeeting_tray_is_ringing (GObject *tray)
{
  g_assert (EGG_IS_TRAY_ICON (tray));

  gpointer data = g_object_get_data (G_OBJECT (tray), "available");

  return (GPOINTER_TO_INT (data) == 0);
}

/* DESCRIPTION  : Returns true if the tray is visible
 * BEHAVIOR     : 
 * PRE          : /
 */
gboolean gnomemeeting_tray_is_visible (GObject *tray)
{
  return GPOINTER_TO_INT (g_object_get_data (tray, "embedded"));
}
