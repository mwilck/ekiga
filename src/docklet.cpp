/***************************************************************************
                          docklet.cpp  -  description
                             -------------------
    begin                : Wed Oct 3 2001
    copyright            : (C) 2001 by Miguel Rodríguez
    description          : This file contains all functions needed for
                           Gnome Panel docklet
    email                : migrax@terra.es
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

static void GM_build_docklet (StatusDocklet *docklet, GtkWidget *plug, gpointer)
{
  GdkPixmap *pixmap;
  GdkBitmap *mask;
  GtkWidget *image;
  GdkPixbuf *pixbuf;

  pixbuf =  gdk_pixbuf_new_from_xpm_data (globe_22_xpm);
  gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &mask, 127);

  image = gtk_pixmap_new(pixmap, mask);

  gtk_widget_show (image);

  GtkWidget *eventbox = gtk_event_box_new ();
    
  gtk_widget_set_events (GTK_WIDGET (eventbox), 
			 gtk_widget_get_events (eventbox)
			 | GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK);

  gtk_signal_connect (GTK_OBJECT (eventbox), "button_press_event",
		      GTK_SIGNAL_FUNC (docklet_clicked), NULL);

  gtk_widget_show (eventbox);

  /* add the status to the plug */
  gtk_container_add (GTK_CONTAINER (eventbox), image);
  gtk_container_add(GTK_CONTAINER(plug), eventbox);

  gtk_object_set_data (GTK_OBJECT (docklet->plug), "pixmapg", image);

  /* Add the popup menu to the plug */
  docklet_create_popup_menu (GTK_WIDGET (eventbox));
}

GtkObject *GM_docklet_init ()
{
  GtkObject *docklet = status_docklet_new ();
  gtk_signal_connect (docklet, "build_plug",
		      GTK_SIGNAL_FUNC (GM_build_docklet), 0);
  status_docklet_run (STATUS_DOCKLET (docklet));

  return docklet;
}


void GM_docklet_set_content (GtkObject *docklet, int choice)
{
  GtkWidget *pixmap = NULL;
  GdkPixmap *Pixmap;
  GdkBitmap *mask;
  GdkPixbuf *pixbuf;

  // if choice = 0, set the world as content
  // if choice = 1, set the globe2 as content
  if (choice == 0)
    {
      pixmap = (GtkWidget*) gtk_object_get_data (GTK_OBJECT (STATUS_DOCKLET (docklet)->plug),
						 "pixmapm");
  
      // if the world was not already the pixmap
      if (pixmap != NULL)
	{
	  pixbuf =  gdk_pixbuf_new_from_xpm_data (globe_22_xpm);
	  gdk_pixbuf_render_pixmap_and_mask (pixbuf, &Pixmap, &mask, 127);
	  
	  gtk_pixmap_set (GTK_PIXMAP (pixmap), Pixmap, mask);
	  gtk_object_remove_data (GTK_OBJECT (STATUS_DOCKLET (docklet)->plug), "pixmapm");
	  gtk_object_set_data (GTK_OBJECT (STATUS_DOCKLET (docklet)->plug), "pixmapg", pixmap);
	}
    }

  if (choice == 1)
    {
      pixmap = (GtkWidget*) gtk_object_get_data (GTK_OBJECT (STATUS_DOCKLET (docklet)->plug),
						 "pixmapg");
           
      if (pixmap != NULL)
	{
	  pixbuf =  gdk_pixbuf_new_from_xpm_data (globe2_22_xpm);
	  gdk_pixbuf_render_pixmap_and_mask (pixbuf, &Pixmap, &mask, 127);
	  
	  gtk_pixmap_set (GTK_PIXMAP (pixmap), Pixmap, mask);
	  gtk_object_remove_data (GTK_OBJECT (STATUS_DOCKLET (docklet)->plug), "pixmapg");
	  gtk_object_set_data (GTK_OBJECT (STATUS_DOCKLET (docklet)->plug), "pixmapm", pixmap);
	}
    }
}


gint docklet_flash (GtkWidget *docklet)
{
  GtkWidget *object;

  // First we check if it is the mic or the globe that is displayed

  gdk_threads_enter ();
  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (STATUS_DOCKLET (docklet)->plug),
					      "pixmapg");
  gdk_threads_leave ();
  
  gdk_threads_enter ();
  if (object != NULL)
    GM_docklet_set_content (GTK_OBJECT (docklet), 1);
  else
    GM_docklet_set_content (GTK_OBJECT (docklet), 0);
  gdk_threads_leave ();

  return TRUE;
}

/******************************************************************************/
