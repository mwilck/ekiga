/***************************************************************************
                          applet.cpp  -  description
                             -------------------
    begin                : Mon Mar 19 2000
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : This file contains all functions needed for
                           Gnome Panel applet
    email                : dsandras@acm.org
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

#include "applet.h"
#include "main.h"
#include "callbacks.h"

#include "../pixmaps/globe-36.xpm"
#include "../pixmaps/globe2-36.xpm" 
#include "../pixmaps/globe-24.xpm"
#include "../pixmaps/globe2-24.xpm" 
#include "../pixmaps/globe-12.xpm"
#include "../pixmaps/globe2-12.xpm" 


/******************************************************************************/
/* Global Variables                                                           */
/******************************************************************************/

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

/******************************************************************************/


/******************************************************************************/
/* GTK Callbacks                                                              */
/******************************************************************************/

// We must redefine another callback than the connect callback in callbacks.h
// because the first parameter must be AppletWidget * in this case 
void applet_connect (AppletWidget *applet, gpointer data)
{
  MyApp->Connect ();
}


void applet_disconnect (AppletWidget *applet, gpointer data)
{
  MyApp->Disconnect ();
}


void applet_toggle_callback (AppletWidget *applet, gpointer data)
{
  if (GTK_WIDGET_VISIBLE (GTK_WIDGET (gm)))
    gtk_widget_hide (gm);
  else
    gtk_widget_show (gm);
}


void applet_clicked (GtkWidget *widget, GdkEventButton *event, gpointer data)
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

GtkWidget *GM_applet_init (int argc, char **argv)
{
  GtkWidget *applet;
  GtkWidget *Pixmap;
  GtkWidget *frame;
  GtkWidget *fixed;
  GdkPixmap *pixmap;
  GdkBitmap *mask;
  int size;


  /* create a new applet_widget */
  applet = applet_widget_new("GnomeMeeting");

  size = applet_widget_get_panel_pixel_size (APPLET_WIDGET (applet));

  /* in the rare case that the communication with the panel
     failed, error out */
  if (!applet)
    g_error(_("Can't create applet!\n"));

  /* create the widget we are going to put on the applet */
  if (size == 24)
    gdk_imlib_data_to_pixmap (globe_24_xpm, &pixmap, &mask);
  else
    if (size == 12)
      gdk_imlib_data_to_pixmap (globe_12_xpm, &pixmap, &mask);
    else
      gdk_imlib_data_to_pixmap (globe_36_xpm, &pixmap, &mask);

  Pixmap = gtk_pixmap_new(pixmap, mask);

  gtk_widget_show(Pixmap);

  gtk_widget_set_events (applet, gtk_widget_get_events(applet) 
			 | GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK );

  gtk_signal_connect (GTK_OBJECT (applet), "button_press_event",
		      GTK_SIGNAL_FUNC (applet_clicked), NULL);

  applet_widget_register_callback (APPLET_WIDGET (applet),
				   "connect",
				   _("Connect"),
				   applet_connect, NULL);

  applet_widget_register_callback (APPLET_WIDGET (applet),
				   "disconnect",
				   _("Disconnect"),
				   applet_disconnect, NULL);

  applet_widget_register_callback (APPLET_WIDGET (applet),
				   "Toggle main window",
				   _("Show/hide main window"),
				   applet_toggle_callback, NULL);

  /* add the widget to the applet-widget, and thereby actually
     putting it "onto" the panel */
  frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
  gtk_widget_set_usize(frame, size, size);
  gtk_widget_show(frame);

  fixed = gtk_fixed_new ();
  gtk_widget_show (fixed);
  gtk_fixed_put (GTK_FIXED (fixed), Pixmap, 0, 0);

  gtk_container_add (GTK_CONTAINER (frame), fixed);
  applet_widget_add (APPLET_WIDGET (applet), frame);

  gtk_widget_show (applet);
  gtk_widget_realize (applet);

  gtk_object_set_data(GTK_OBJECT(applet), "pixmapg", Pixmap);
  
  gtk_pixmap_set(GTK_PIXMAP (Pixmap), pixmap, mask);


  return (applet);
}


void GM_applet_set_content (GtkWidget *applet, int choice)
{
  GtkWidget *pixmap = NULL;
  GdkPixmap *Pixmap;
  GdkBitmap *mask;

  int size;

  size = applet_widget_get_panel_pixel_size (APPLET_WIDGET (applet));

  // if choice = 0, set the world as content
  // if choice = 1, set the globe2 as content
  if (choice == 0)
    {
      pixmap = (GtkWidget*) gtk_object_get_data (GTK_OBJECT (applet),
						 "pixmapm");
  
      // if the world was not already the pixmap
      if (pixmap != NULL)
	{
	  if (size == 24)
	    gdk_imlib_data_to_pixmap (globe_24_xpm, &Pixmap, &mask);
	  else
	    if (size == 12)
	      gdk_imlib_data_to_pixmap (globe_12_xpm, &Pixmap, &mask);
	  else
	    gdk_imlib_data_to_pixmap (globe_36_xpm, &Pixmap, &mask);

	  gtk_pixmap_set (GTK_PIXMAP (pixmap), Pixmap, mask);
	  gtk_object_remove_data (GTK_OBJECT (applet), "pixmapm");
	  gtk_object_set_data (GTK_OBJECT(applet), "pixmapg", pixmap);
	}
    }

  if (choice == 1)
    {
      pixmap = (GtkWidget*) gtk_object_get_data (GTK_OBJECT (applet),
						 "pixmapg");
           
      if (pixmap != NULL)
	{
	  if (size == 24)
	    gdk_imlib_data_to_pixmap (globe2_24_xpm, &Pixmap, &mask);
	  else
	    if (size == 12)
	      gdk_imlib_data_to_pixmap (globe2_12_xpm, &Pixmap, &mask);
	  else
	    gdk_imlib_data_to_pixmap (globe2_36_xpm, &Pixmap, &mask);

	  gtk_pixmap_set (GTK_PIXMAP (pixmap), Pixmap, mask);
	  gtk_object_remove_data (GTK_OBJECT (applet), "pixmapg");
	  gtk_object_set_data (GTK_OBJECT(applet), "pixmapm", pixmap);
	}
    }

}


gint AppletFlash (GtkWidget *applet)
{
  GtkWidget *object;

  // First we check if it is the mic or the globe that is displayed

  gdk_threads_enter ();
  object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (applet),
					      "pixmapg");
  gdk_threads_leave ();
  
  gdk_threads_enter ();
  if (object != NULL)
    GM_applet_set_content (applet, 1);
  else
    GM_applet_set_content (applet, 0);
  gdk_threads_leave ();


  return TRUE;
}

/******************************************************************************/
