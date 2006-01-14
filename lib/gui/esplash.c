
/*
 * GnomeMeeting -- A Video-Conferencing application
 *
 * Copyright (C) 2000-2006 Damien Sandras
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "../../config.h"
#include "esplash.h"

#include <gtk/gtk.h>

#ifdef WIN32
#include "../winpaths.h"
#endif

/**
 * e_splash_new:
 *
 * Create a new ESplash widget.
 * 
 * Return value: A pointer to the newly created ESplash widget.
 **/
GtkWidget *
e_splash_new ()
{
  GtkWidget *window = NULL;
  GtkWidget *image = NULL;
  GdkPixbuf *pixbuf_icon;
  gchar     *filename = NULL;

  filename = g_build_filename (DATA_DIR, "pixmaps", PACKAGE_NAME, PACKAGE_NAME "-splash.png",
			       NULL);
  image = gtk_image_new_from_file (filename);
  g_free (filename);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position (GTK_WINDOW (window), 
			   GTK_WIN_POS_CENTER);

  filename = g_build_filename (DATA_DIR, "pixmaps", PACKAGE_NAME "-icon.png", NULL);
  pixbuf_icon = gdk_pixbuf_new_from_file (filename, NULL); 
  g_free (filename);

  gtk_window_set_icon (GTK_WINDOW (window), pixbuf_icon);
  gtk_window_set_title (GTK_WINDOW (window), "Ekiga");

  gtk_container_add (GTK_CONTAINER (window), image);

  return GTK_WIDGET (window);
}


