
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
 *                         docklet.cpp  -  description
 *                         ---------------------------
 *   begin                : Wed Oct 3 2001
 *   copyright            : (C) 2000-2002 by Miguel Rodríguez
 *   description          : This file contains all functions needed for
 *                          Gnome Panel docklet.
 *   email                : migrax@terra.es (all the new code)
 *                          dsandras@seconix.com (old applet code).
 *
 */

#include "../config.h"

#include "docklet.h"
#include "gnomemeeting.h"
#include "menu.h"
#include "callbacks.h"
#include "misc.h"

#include <gconf/gconf-client.h>

#include <gdk/gdkx.h>
#include <X11/Xatom.h>

#include "../pixmaps/globe-22.xpm"
#include "../pixmaps/globe2-22.xpm" 
#include "../pixmaps/connect_16.xpm"
#include "../pixmaps/disconnect_16.xpm"

/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

static void docklet_popup_menu_connect_callback (GtkWidget *, gpointer);
static void docklet_popup_menu_disconnect_callback (GtkWidget *, gpointer);
static void docklet_toggle_callback (GtkWidget *, gpointer);
static int docklet_clicked (GtkWidget *, GdkEventButton *, gpointer);
static void gnomemeeting_init_docklet_popup_menu (GtkWidget *);
static void gnomemeeting_setup_docklet_properties (GdkWindow *);
static void gnomemeeting_build_docklet (GtkWindow *);

/* GTK Callbacks */

/* DESCRIPTION  :  This callback is called when the user chooses
 *                 to connect in the docklet menu.
 * BEHAVIOR     :  Answer incoming call or call somebody
 * PRE          :  /
 */
void docklet_popup_menu_connect_callback (GtkWidget *, gpointer)
{
  MyApp->Connect ();
}


/* DESCRIPTION  :  This callback is called when the user chooses
 *                 to disconnect in the docklet menu.
 * BEHAVIOR     :  Refuse incoming call or stops current call
 * PRE          :  /
 */
void docklet_popup_menu_disconnect_callback (GtkWidget *, gpointer)
{
  MyApp->Disconnect ();
}


/* DESCRIPTION  :  This callback is called when the user chooses
 *                 toggle in the docklet menu
 * BEHAVIOR     :  Hide or show main window
 * PRE          :  /
 */
void docklet_toggle_callback (GtkWidget *, gpointer)
{
  if (GTK_WIDGET_VISIBLE (GTK_WIDGET (gm))) {
    gtk_widget_hide (gm);
  }
  else {
    gtk_widget_show (gm);
  }
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the docklet.
 * BEHAVIOR     :  If double clic : hide or show main window.
 * PRE          :  /
*/
int docklet_clicked (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  g_return_val_if_fail (event != NULL, false);

  if ((event->button == 1) && (event->type == GDK_BUTTON_PRESS)) {

    docklet_toggle_callback (widget, data);
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
void gnomemeeting_init_docklet_popup_menu (GtkWidget *widget)
{
  GtkWidget *popup_menu_widget;
  
  static GnomeUIInfo popup_menu [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("Connect"), N_("Create A New Connection"),
	(void *)docklet_popup_menu_connect_callback, GINT_TO_POINTER(0), NULL,
	GNOME_APP_PIXMAP_DATA, connect_16_xpm,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("Disconnect"), N_("Close The Current Connection"),
	(void *)docklet_popup_menu_disconnect_callback, GINT_TO_POINTER(1), 
	NULL,
	GNOME_APP_PIXMAP_DATA, disconnect_16_xpm,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("Show/Hide Main Window"), N_("Show/hide The Main Window"),
	(void *)docklet_toggle_callback, GINT_TO_POINTER(2), NULL,
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


/* DESCRIPTION  :  This function sets up the window manager hints.
 * BEHAVIOR     :  Sets both the KWM_DOCKWINDOW and 
 *                 KDE_NET_WM_SYSTE_TRAY_WINDOW_FOR properties.
 * PRE          :  /
 */
void gnomemeeting_setup_docklet_properties (GdkWindow *window)
{
  glong data[1]; 
  
  GdkAtom kwm_dockwindow_atom;
  GdkAtom kde_net_system_tray_window_for_atom;
  
  kwm_dockwindow_atom = gdk_atom_intern("KWM_DOCKWINDOW", FALSE);
  kde_net_system_tray_window_for_atom = gdk_atom_intern("_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR", FALSE);
  
  /* This is the old KDE 1.0 and GNOME 1.2 way... */
  data[0] = TRUE;
  gdk_property_change(window, kwm_dockwindow_atom, 
		      kwm_dockwindow_atom, 32,
		      GDK_PROP_MODE_REPLACE, (guchar *)&data, 1);
  
  /* This is needed to support KDE 2.0 
     can be set to zero or the root win I think */
  data[0] = 0;
  gdk_property_change(window, kde_net_system_tray_window_for_atom, 
		      (GdkAtom) XA_WINDOW, 32,
		      GDK_PROP_MODE_REPLACE, (guchar *)&data, 1);
  
}


/* DESCRIPTION  :  Builds up the docklet
 * BEHAVIOR     :  Adds needed widgets to the docklet window
 * PRE          :  docklet must be a valid pointer to a GtkWindow
 */
static void gnomemeeting_build_docklet (GtkWindow *docklet)
{
  GtkWidget *image;
  GdkPixbuf *pixbuf;

  pixbuf =  gdk_pixbuf_new_from_xpm_data (globe_22_xpm);
  image = gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref (pixbuf);

  GTK_WIDGET_SET_FLAGS(image, GTK_NO_WINDOW);
  image->requisition.width = 22;
  image->requisition.height = 22;
  
  GtkWidget *eventbox = gtk_event_box_new ();
  
  gtk_widget_set_events (GTK_WIDGET (eventbox), 
			 gtk_widget_get_events (eventbox)
			 | GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK);
  
  g_signal_connect (G_OBJECT (eventbox), "button_press_event",
		    G_CALLBACK (docklet_clicked), NULL);
  
  gtk_widget_show (eventbox);
  
  /* add the status to the plug */
  g_object_set_data (G_OBJECT (docklet), "pixmapg", image);
  gtk_container_add (GTK_CONTAINER (eventbox), image);
  gtk_container_add (GTK_CONTAINER(docklet), eventbox);
  
  gtk_widget_show (image);
  
  /* Add the popup menu to the plug */
  gnomemeeting_init_docklet_popup_menu (GTK_WIDGET (eventbox));
}


/* DESCRIPTION  :  Creates the docklet window
 * BEHAVIOR     :  Creates a new window for the docklet and call
 *                 the functions needed to set the contents
 * PRE          :  /
 */
GtkWidget *gnomemeeting_init_docklet ()
{
  GtkWindow *docklet;
  GConfClient *client = gconf_client_get_default ();

  docklet = GTK_WINDOW (gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gtk_window_set_title (GTK_WINDOW(docklet), "gnomemeeting_status_plugin");
  gtk_window_set_wmclass (GTK_WINDOW(docklet), "GM_StatusDocklet", "gnomemeeting");
  gtk_widget_set_size_request (GTK_WIDGET(docklet), 22, 22);

  gtk_widget_realize (GTK_WIDGET(docklet));

  gnomemeeting_build_docklet (docklet);
  
  gnomemeeting_setup_docklet_properties (GTK_WIDGET (docklet)->window);


  if (gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_docklet", 0))
    gnomemeeting_docklet_show (GTK_WIDGET (docklet));

  return GTK_WIDGET (docklet);
}


/* DESCRIPTION  :  Changes the image displayed in the docklet
 * BEHAVIOR     :  /
 * PRE          :  Choice must be 0 or 1.
 */
void gnomemeeting_docklet_set_content (GtkWidget *docklet, int choice)
{
  GtkImage *image = NULL;
  GdkPixmap *Pixmap;
  GdkBitmap *mask;
  GdkPixbuf *pixbuf;

  /* if choice = 0, set the world as content
     if choice = 1, set the globe2 as content */
  if (choice == 0)  {

    image = GTK_IMAGE (g_object_get_data (G_OBJECT (docklet), "pixmapm"));
  
    /* if the world was not already the pixmap */
    if (image != NULL)	{

      pixbuf = gdk_pixbuf_new_from_xpm_data (globe_22_xpm);
      gtk_image_set_from_pixbuf (image, pixbuf);
      g_object_unref (pixbuf);

      g_object_set_data (G_OBJECT (docklet), "pixmapm", NULL);
      g_object_set_data (G_OBJECT (docklet), "pixmapg", image);
    }
  }

  if (choice == 1) {

    image = GTK_IMAGE (g_object_get_data (G_OBJECT (docklet),
					  "pixmapg"));
    
    if (image != NULL)	{

      pixbuf =  gdk_pixbuf_new_from_xpm_data (globe2_22_xpm);
      gtk_image_set_from_pixbuf (image, pixbuf);
      g_object_unref (pixbuf);

      g_object_set_data (G_OBJECT (docklet), "pixmapg", NULL);
      g_object_set_data (G_OBJECT (docklet), "pixmapm", image);
    }
  }
}


/* DESCRIPTION  :  Shows the docklet window
 * BEHAVIOR     :  /
 * PRE          :  /
 */
void gnomemeeting_docklet_show (GtkWidget *docklet)
{
  gtk_widget_show (docklet);
}


/* DESCRIPTION  :  Hides the docklet window
 * BEHAVIOR     :  /
 * PRE          :  /
 */
void gnomemeeting_docklet_hide (GtkWidget *docklet)
{
  gtk_widget_hide (docklet);
}


/* DESCRIPTION  :  Changes the content of the docklet
 *                 based on the current one. Calling this function
 *                 from a timer created a flash effect in the docklet.
 * BEHAVIOR     :  /
 * PRE          :  /
 */
gint gnomemeeting_docklet_flash (GtkWidget *docklet)
{
  gpointer object;

  /* we can't call gnomemeeting_threads_enter as idles and timers
     are executed in the main thread */
  gdk_threads_enter ();
  object = g_object_get_data (G_OBJECT (docklet), "pixmapg");
  gdk_threads_leave ();
  
  gdk_threads_enter ();
  if (object != NULL)
    gnomemeeting_docklet_set_content (docklet, 1);
  else
    gnomemeeting_docklet_set_content (docklet, 0);
  gdk_threads_leave ();

  return TRUE;
}
