
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
#include "gnomemeeting.h"
#include "eggtrayicon.h"
#include "menu.h"
#include "callbacks.h"
#include "stock-icons.h"


/* Declarations */

extern GtkWidget *gm;

static gint tray_clicked_callback (GtkWidget *, GdkEventButton *, gpointer);
static gint tray_icon_embedded (GtkWidget *, gpointer);
static gint tray_icon_destroyed (GtkWidget *, gpointer);
static void gnomemeeting_build_tray (GtkContainer *);


/* The functions  */

/* DESCRIPTION  :  This callback is called when the tray appears on the panel
 * BEHAVIOR     :  Store the info in the object
 * PRE          :  
 */
static gint tray_icon_embedded (GtkWidget *tray_icon, gpointer)
{
  static bool first_time = true;

  if (first_time) {
    GConfClient *client = gconf_client_get_default ();
    if (gconf_client_get_bool (client, VIEW_KEY "start_docked", 0)) {
      gtk_widget_hide (gm);
    }
    first_time = false;
  }
  g_object_set_data (G_OBJECT (tray_icon), "embedded", GINT_TO_POINTER (1));

  return true;
}

/* DESCRIPTION  :  This callback is called when the panel gets closed
 *                 after the tray has been embedded
 * BEHAVIOR     :  Create a new tray_icon and substitute the old one
 * PRE          :  A GtkAccelGroup
 */
static gint tray_icon_destroyed (GtkWidget *tray, gpointer data) 
{
  /* Somehow the delete_event never got called, so we use "destroy" */
  if (tray != GnomeMeeting::Process ()->GetMainWindow ()->docklet)
    return true;
  GtkWidget *new_tray = gnomemeeting_init_tray ();

  g_warning ("FIX ME: update tray icon");
  
  GnomeMeeting::Process ()->GetMainWindow ()->docklet = GTK_WIDGET (new_tray);
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
  GmWindow *gw = NULL;
   
  if (event->type == GDK_BUTTON_PRESS) {

    event_button = (GdkEventButton *) event;
    if (event_button->button == 1) {

      if (GTK_WIDGET_VISIBLE (gm))
	gtk_widget_hide (gm);
      else
	gtk_widget_show (gm);
      
      return TRUE;
    }
    else if (event_button->button == 2) {

      gw = GnomeMeeting::Process ()->GetMainWindow ();

      gnomemeeting_component_view (NULL, (gpointer) gw->ldap_window);

      return TRUE;
    }
  }
  
  return FALSE;
}


/* DESCRIPTION  :  Builds up the tray icon
 * BEHAVIOR     :  Adds needed widgets to the docklet window
 * PRE          :  docklet must be a valid pointer to a GtkWindow.
 */
static void 
gnomemeeting_build_tray (GtkContainer *tray_icon)
{
  GmWindow *gw = NULL;
  
  GtkWidget *image = NULL;
  GtkWidget *eventbox = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  /* Add the popup menu to the tray */
  gw->tray_popup_menu =
    gnomemeeting_tray_init_menu (GTK_WIDGET (tray_icon));

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
		    G_CALLBACK (tray_icon_destroyed), NULL);
  g_signal_connect (G_OBJECT (eventbox), "button_press_event",
		    G_CALLBACK (tray_clicked_callback), NULL);
}


/* The nctions */
GtkWidget *gnomemeeting_init_tray ()
{
  EggTrayIcon *tray_icon;

#ifndef WIN32
  tray_icon = egg_tray_icon_new (_("GnomeMeeting Tray Icon"));
  gnomemeeting_build_tray (GTK_CONTAINER (tray_icon));
  gnomemeeting_tray_show (GTK_WIDGET (tray_icon));
  g_warning ("FIX ME: update tray icon");
#endif
  
  return GTK_WIDGET (tray_icon);
}


void gnomemeeting_tray_set_content (GtkWidget *tray, int choice)
{
  gpointer image = NULL;

  /* if choice = 0, set the phone as content
     if choice = 1, set the ringing phone as content */
  if (choice == 0)  {
    image = g_object_get_data (G_OBJECT (tray), "image");
  
    /* if that was was not already the pixmap */
    if (image != NULL)	{
      gtk_image_set_from_stock (GTK_IMAGE (image), GM_STOCK_STATUS_AVAILABLE, 
				GTK_ICON_SIZE_MENU);
      g_object_set_data (G_OBJECT (tray), "available", GINT_TO_POINTER (1));
    }
  }

  if (choice == 1) {

    image = g_object_get_data (G_OBJECT (tray), "image");
    
    if (image != NULL)	{
      gtk_image_set_from_stock (GTK_IMAGE (image), GM_STOCK_STATUS_RINGING,
				GTK_ICON_SIZE_MENU);
      g_object_set_data (G_OBJECT (tray), "available", GINT_TO_POINTER (0));
    }
  }

  if (choice == 2) {

    image = g_object_get_data (G_OBJECT (tray), "image");

    if (image != NULL) {
      gtk_image_set_from_stock (GTK_IMAGE (image), GM_STOCK_STATUS_OCCUPIED,
				GTK_ICON_SIZE_MENU);
      g_object_set_data (G_OBJECT (tray), "available", GINT_TO_POINTER (0));
    }
  }
}


void gnomemeeting_tray_show (GtkWidget *tray)
{
  gtk_widget_show (tray);
}


void gnomemeeting_tray_hide (GtkWidget *tray)
{
  gtk_widget_hide (GTK_WIDGET (tray));
}


void gnomemeeting_tray_flash (GtkWidget *tray)
{
  gpointer data;

  /* we can't call gnomemeeting_threads_enter as idles and timers
     are executed in the main thread */
  data = g_object_get_data (G_OBJECT (tray), "available");

  if (GPOINTER_TO_INT (data) == 1) {
    gnomemeeting_tray_set_content (tray, 1);
  } else {
    gnomemeeting_tray_set_content (tray, 0);
  }
}


gboolean gnomemeeting_tray_is_ringing (GtkWidget *tray)
{
  g_assert (EGG_IS_TRAY_ICON (tray));

  gpointer data = g_object_get_data (G_OBJECT (tray), "available");

  return (GPOINTER_TO_INT (data) == 0);
}

/* DESCRIPTION  : Returns true if the tray is visible
 * BEHAVIOR     : 
 * PRE          : /
 */
gboolean gnomemeeting_tray_is_visible (GtkWidget *tray)
{
  return GPOINTER_TO_INT (g_object_get_data (G_OBJECT (tray), "embedded"));
}
