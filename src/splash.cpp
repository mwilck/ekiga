
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
 *                         splash.cpp  -  description
 *                         --------------------------
 *   begin                : Mon Mar 19 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains functions to display the splash
 *                          screen at startup.
 *   email                : dsandras@seconix.com
 *
 */


#include "splash.h"


/* The functions */

GtkWidget* gnomemeeting_splash_init (void) 
{
  GtkWidget *splash_win;
  GtkWidget *pixmap;
  GtkWidget *frame;
  GtkWidget *frame2;
  GtkWidget *vbox;
  GtkWidget *label;
  GtkWidget *progressbar;

  splash_win = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_window_set_title (GTK_WINDOW (splash_win), "GnomeMeeting");
  gtk_window_set_policy (GTK_WINDOW (splash_win), FALSE, FALSE, TRUE);
  gtk_widget_realize(splash_win);

  /* Center it on the screen */
  gtk_window_set_position(GTK_WINDOW (splash_win), GTK_WIN_POS_CENTER);

  frame2 = gtk_frame_new (NULL);
  gtk_widget_show (frame2);
  gtk_container_add (GTK_CONTAINER (splash_win), frame2);
  gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_OUT);

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_container_add (GTK_CONTAINER (frame2), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  pixmap = gnome_pixmap_new_from_file 
    ("/usr/share/pixmaps/gnomemeeting-logo.png");
  gtk_box_pack_start (GTK_BOX (vbox), pixmap, TRUE, TRUE, 0);
  gtk_widget_show (pixmap);
  gtk_widget_show(splash_win);

  label = gtk_label_new ("");
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_object_set_data(GTK_OBJECT(splash_win), "label", label);

  progressbar = gtk_progress_bar_new ();
  gtk_widget_show (progressbar);
  gtk_box_pack_start (GTK_BOX (vbox), progressbar, FALSE, FALSE, 0);
  gtk_progress_configure (GTK_PROGRESS (progressbar), 0, 0, 10);
  gtk_object_set_data(GTK_OBJECT(splash_win), "progress", progressbar);

  /* Force it to draw now */
  gdk_flush();
  
  /* Go into main loop, processing events */
  while(!GTK_WIDGET_REALIZED (pixmap)
	|| !GTK_WIDGET_REALIZED (progressbar)
	|| !GTK_WIDGET_REALIZED (label)
	|| gtk_events_pending ()) {

    gtk_main_iteration();
  }

  return splash_win;
}


void gnomemeeting_splash_advance_progress(GtkWidget * splash_win, 
					  char* message, gfloat per) 
{
  GtkProgressBar* progress;
  GtkLabel* label;
  struct timeval start;
  struct timeval cur;
  
  progress = GTK_PROGRESS_BAR((GtkWidget*) 
			      gtk_object_get_data (GTK_OBJECT (splash_win),
                                                   "progress"));

  label = GTK_LABEL((GtkWidget*) 
		    gtk_object_get_data (GTK_OBJECT (splash_win),
					 "label"));

  if (message) gtk_label_set_text(label, message);
  gtk_progress_bar_update(progress, per);


  if (per >= 1) return;

  while (gtk_events_pending()) gtk_main_iteration();
  gdk_flush ();

  gettimeofday(&start, NULL);
  do {

    gettimeofday(&cur, NULL);
  }
 
  while ((cur.tv_sec-start.tv_sec)*1000 
	 + (cur.tv_usec-start.tv_usec)/1000 < 300); 
}
