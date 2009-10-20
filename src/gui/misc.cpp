
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         misc.cpp  -  description
 *                         ------------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains miscellaneous functions.
 *   Additional Code      : De Michele Cristiano, Miguel Rodríguez 
 *
 */


#include "config.h"

#include "misc.h"
#include "ekiga.h"
#include "callbacks.h"

#include "gmdialog.h"
#include "gmwindow.h"
#include "gmconf.h"

#include <glib/gi18n.h>


/* The functions */
GtkWidget *
gnomemeeting_button_new (const char *lbl, 
			 GtkWidget *pixmap)
{
  GtkWidget *button = NULL;
  GtkWidget *hbox2 = NULL;
  GtkWidget *label = NULL;

  button = gtk_button_new ();
  label = gtk_label_new_with_mnemonic (lbl);
  hbox2 = gtk_hbox_new (FALSE, 0);

  gtk_box_pack_start(GTK_BOX (hbox2), pixmap, TRUE, TRUE, 0);  
  gtk_box_pack_start(GTK_BOX (hbox2), label, TRUE, TRUE, 0);

  gtk_container_add (GTK_CONTAINER (button), hbox2);

  return button;
}


void
gnomemeeting_window_show (GtkWidget *w)
{
  int x = 0;
  int y = 0;

  gchar *window_name = NULL;
  gchar *conf_key_size = NULL;
  gchar *conf_key_position = NULL;
  gchar *size = NULL;
  gchar *position = NULL;
  gchar **couple = NULL;
  
  g_return_if_fail (GTK_IS_WINDOW (w));

  if (gm_window_is_visible (w)) {

    gtk_window_present (GTK_WINDOW (w));
    return;
  } // else we do the show :
  
  window_name = (char *) g_object_get_data (G_OBJECT (w), "window_name");

  g_return_if_fail (window_name != NULL);
  
  conf_key_position =
    g_strdup_printf ("%s%s/position", USER_INTERFACE_KEY, window_name);
  conf_key_size =
    g_strdup_printf ("%s%s/size", USER_INTERFACE_KEY, window_name);  

  if (!gm_window_is_visible (w)) {
    
    position = gm_conf_get_string (conf_key_position);
    if (position)
      couple = g_strsplit (position, ",", 0);

    if (couple && couple [0])
      x = atoi (couple [0]);
    if (couple && couple [1])
      y = atoi (couple [1]);


    if (x != 0 && y != 0)
      gtk_window_move (GTK_WINDOW (w), x, y);

    g_strfreev (couple);
    couple = NULL;
    g_free (position);


    if (gtk_window_get_resizable (GTK_WINDOW (w))) {

      size = gm_conf_get_string (conf_key_size);
      if (size)
	couple = g_strsplit (size, ",", 0);

      if (couple && couple [0])
	x = atoi (couple [0]);
      if (couple && couple [1])
	y = atoi (couple [1]);

      if (x > 0 && y > 0)
	gtk_window_resize (GTK_WINDOW (w), x, y);

      g_strfreev (couple);
      g_free (size);
    }

    gnomemeeting_threads_dialog_show (w);
  }
  
  g_free (conf_key_position);
  g_free (conf_key_size);
}


void
gnomemeeting_window_hide (GtkWidget *w)
{
  int x = 0;
  int y = 0;

  gchar *window_name = NULL;
  gchar *conf_key_size = NULL;
  gchar *conf_key_position = NULL;
  gchar *size = NULL;
  gchar *position = NULL;
  
  g_return_if_fail (w != NULL);
  
  window_name = (char *) g_object_get_data (G_OBJECT (w), "window_name");

  g_return_if_fail (window_name != NULL);
 
  conf_key_position =
    g_strdup_printf ("%s%s/position", USER_INTERFACE_KEY, window_name);
  conf_key_size =
    g_strdup_printf ("%s%s/size", USER_INTERFACE_KEY, window_name);

  
  /* If the window is visible, save its position and hide the window */
  if (gm_window_is_visible (w)) {
    
    gtk_window_get_position (GTK_WINDOW (w), &x, &y);
    position = g_strdup_printf ("%d,%d", x, y);
    gm_conf_set_string (conf_key_position, position);
    g_free (position);

    if (gtk_window_get_resizable (GTK_WINDOW (w))) {

      gtk_window_get_size (GTK_WINDOW (w), &x, &y);
      size = g_strdup_printf ("%d,%d", x, y);
      gm_conf_set_string (conf_key_size, size);
      g_free (size);
    }

	
    gnomemeeting_threads_dialog_hide (w);
  }
    
  
  g_free (conf_key_position);
  g_free (conf_key_size);
}
