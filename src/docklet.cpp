/***************************************************************************
                          docklet.cpp  -  description
                             -------------------
    begin                : Wed Oct 3 2001
    copyright            : (C) 2000-2001 by Damien Sandras & Miguel Rodríguez
    description          : This file contains all functions needed for
                           Gnome Panel docklet
    email                : migrax@terra.es, dsandras@seconix.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../config.h"

#include "docklet.h"
#include "main.h"
#include "callbacks.h"

#include <gdk/gdkx.h>
#include <X11/Xatom.h>

#include "../pixmaps/globe-22.xpm"
#include "../pixmaps/globe2-22.xpm" 


/******************************************************************************/
/* Global Variables                                                           */
/******************************************************************************/

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

/******************************************************************************/


/******************************************************************************/
/* GTK Callbacks                                                              */
/******************************************************************************/

/* We must redefine another callback than the connect callback in callbacks.h
 * because the first parameter must be AppletWidget * in this case 
 * FIXME: I don't undestand the above comment (damien?)
*/
static void docklet_popup_menu_connect_callback (GtkWidget *, gpointer)
{
  MyApp->Connect ();
}


static void docklet_popup_menu_disconnect_callback (GtkWidget *, gpointer)
{
  MyApp->Disconnect ();
}


void docklet_toggle_callback (GtkWidget *, gpointer)
{
  if (GTK_WIDGET_VISIBLE (GTK_WIDGET (gm)))
    gtk_widget_hide (gm);
  else
    gtk_widget_show (gm);
}

static void docklet_create_popup_menu (GtkWidget *widget)
{
  GtkWidget *popup_menu_widget;
  
  static GnomeUIInfo popup_menu [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("Connect"), N_("Connect call"),
	(void *)docklet_popup_menu_connect_callback, GINT_TO_POINTER(0), NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("Disconnect"), N_("Drop call"),
	(void *)docklet_popup_menu_disconnect_callback, GINT_TO_POINTER(1), NULL,
	GNOME_APP_PIXMAP_NONE, NULL,
	0, GDK_CONTROL_MASK, NULL
      },
      {
	GNOME_APP_UI_ITEM,
	N_("Show/hide main window"), N_("Show/hide the main window"),
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

void docklet_clicked (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  if (event == NULL) 
    return;

  if ((event->button == 1) && (event->type == GDK_BUTTON_PRESS))
    {
      if (GTK_WIDGET_VISIBLE (GTK_WIDGET (gm)))
	gtk_widget_hide (gm);
      else
	gtk_widget_show (gm);
    }
}

/******************************************************************************/


/******************************************************************************/
/* The functions                                                              */
/******************************************************************************/
/*
  This function sets up the window manager hints. 

 KDE1 and GNOME use the KWM_DOCKWINDOW property, while KDE2 uses
 _KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR.

*/
static void GM_setup_docklet_properties(GdkWindow *window)
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
  
  /* This is needed to support KDE 2.0 */
  /* can be set to zero or the root win I think */
  data[0] = 0;
  gdk_property_change(window, kde_net_system_tray_window_for_atom, 
		      XA_WINDOW, 32,
		      GDK_PROP_MODE_REPLACE, (guchar *)&data, 1);
  
}

static void GM_build_docklet (GtkWindow *docklet)
{
  GdkPixmap *pixmap;
  GdkBitmap *mask;
  GtkWidget *image;
  GdkPixbuf *pixbuf;

  pixbuf =  gdk_pixbuf_new_from_xpm_data (globe_22_xpm);
  gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &mask, 127);

  image = gtk_pixmap_new(pixmap, mask);

  GTK_WIDGET_SET_FLAGS(image, GTK_NO_WINDOW);
  image->requisition.width = 22;
  image->requisition.height = 22;
  
  GtkWidget *eventbox = gtk_event_box_new ();
  
  gtk_widget_set_events (GTK_WIDGET (eventbox), 
			 gtk_widget_get_events (eventbox)
			 | GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK);
  
  gtk_signal_connect (GTK_OBJECT (eventbox), "button_press_event",
		      GTK_SIGNAL_FUNC (docklet_clicked), NULL);
  
  gtk_widget_show (eventbox);
  
  /* add the status to the plug */
  gtk_object_set_data (GTK_OBJECT (docklet), "pixmapg", image);
  gtk_container_add (GTK_CONTAINER (eventbox), image);
  gtk_container_add(GTK_CONTAINER(docklet), eventbox);
  
  gtk_widget_show (image);
  
  /* Add the popup menu to the plug */
  docklet_create_popup_menu (GTK_WIDGET (eventbox));
}

GtkWidget *GM_docklet_init ()
{
  GtkWindow *docklet;

  docklet = (GtkWindow *)gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(docklet), "gnomemeeting_status_plugin");
  gtk_window_set_wmclass(GTK_WINDOW(docklet), "GM_StatusDocklet", "gnomemeeting");
  gtk_widget_set_usize(GTK_WIDGET(docklet), 22, 22);

  gtk_widget_realize (GTK_WIDGET(docklet));

  GM_build_docklet (docklet);
  
  GM_setup_docklet_properties (GTK_WIDGET (docklet)->window);

  return GTK_WIDGET (docklet);
}


void GM_docklet_set_content (GtkWidget *docklet, int choice)
{
  GtkWidget *pixmap = NULL;
  GdkPixmap *Pixmap;
  GdkBitmap *mask;
  GdkPixbuf *pixbuf;

  // if choice = 0, set the world as content
  // if choice = 1, set the globe2 as content
  if (choice == 0)
    {
      pixmap = (GtkWidget*) gtk_object_get_data (GTK_OBJECT (docklet), "pixmapm");
  
      // if the world was not already the pixmap
      if (pixmap != NULL)
	{
	  pixbuf =  gdk_pixbuf_new_from_xpm_data (globe_22_xpm);
	  gdk_pixbuf_render_pixmap_and_mask (pixbuf, &Pixmap, &mask, 127);
	  
	  gtk_pixmap_set (GTK_PIXMAP (pixmap), Pixmap, mask);
	  gtk_object_remove_data (GTK_OBJECT (docklet), "pixmapm");
	  gtk_object_set_data (GTK_OBJECT (docklet), "pixmapg", pixmap);
	}
    }

  if (choice == 1)
    {
      pixmap = (GtkWidget*) gtk_object_get_data (GTK_OBJECT (docklet),
						 "pixmapg");
           
      if (pixmap != NULL)
	{
	  pixbuf =  gdk_pixbuf_new_from_xpm_data (globe2_22_xpm);
	  gdk_pixbuf_render_pixmap_and_mask (pixbuf, &Pixmap, &mask, 127);
	  
	  gtk_pixmap_set (GTK_PIXMAP (pixmap), Pixmap, mask);
	  gtk_object_remove_data (GTK_OBJECT (docklet), "pixmapg");
	  gtk_object_set_data (GTK_OBJECT (docklet), "pixmapm", pixmap);
	}
    }
}

void GM_docklet_show (GtkWidget *docklet)
{
  gtk_widget_show (docklet);
}

void GM_docklet_hide (GtkWidget *docklet)
{
  gtk_widget_hide (docklet);
}


gint docklet_flash (GtkWidget *docklet)
{
  GtkWidget *object;

  // First we check if it is the mic or the globe that is displayed

  gdk_threads_enter ();
  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (docklet), "pixmapg");
  gdk_threads_leave ();
  
  gdk_threads_enter ();
  if (object != NULL)
    GM_docklet_set_content (docklet, 1);
  else
    GM_docklet_set_content (docklet, 0);
  gdk_threads_leave ();

  return TRUE;
}

/******************************************************************************/
