
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
 *                         tools.cpp  -  description
 *                         -------------------------
 *   begin                : Sun Sep 1 2002
 *   copyright            : (C) 2000-2003 by Damien Sandras 
 *   description          : This file contains functions to build the simple
 *                          tools of the tools menu.
 *
 */


#include "../config.h"

#include "tools.h"
#include "common.h"
#include "misc.h"

#ifndef DISABLE_GNOME
#include <gnome.h>
#else
#include <gtk/gtk.h>
#endif


extern GtkWidget *gm;


/* The functions */

void gnomemeeting_init_calls_history_window ()
{
  GtkWidget *frame = NULL;
  GtkWidget *scr = NULL;
  GtkTextBuffer *buffer = NULL;
  GtkTextMark *mark;
  GtkTextIter end;

  /* Get the structs from the application */
  GmWindow *gw = gnomemeeting_get_main_window (gm);
  
  gw->calls_history_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (gw->calls_history_window), 
			_("Calls History"));
  gtk_window_set_position (GTK_WINDOW (gw->calls_history_window), 
			   GTK_WIN_POS_CENTER);
  gtk_window_set_default_size (GTK_WINDOW (gw->calls_history_window), 
			       325, 175);

  frame = gtk_frame_new (_("Calls History"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);

  gw->calls_history_text_view = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (gw->calls_history_text_view), 
			      FALSE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (gw->calls_history_text_view), FALSE);

  gtk_text_view_set_editable (GTK_TEXT_VIEW (gw->calls_history_text_view), 
			      FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (gw->calls_history_text_view),
			       GTK_WRAP_WORD);

  buffer = 
    gtk_text_view_get_buffer (GTK_TEXT_VIEW (gw->calls_history_text_view));
  gtk_text_buffer_get_end_iter (buffer, &end);
  mark = gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (buffer), 
				      "current-position", &end, FALSE);

  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (frame), scr);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_container_set_border_width (GTK_CONTAINER (scr), 2);
  
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);

  gtk_container_add (GTK_CONTAINER (scr), gw->calls_history_text_view);
  gtk_container_add (GTK_CONTAINER (gw->calls_history_window), frame);    
 
  g_signal_connect (G_OBJECT (gw->calls_history_window), "delete_event",
		    G_CALLBACK (gtk_widget_hide_on_delete), NULL); 
}


void gnomemeeting_init_history_window ()
{
  GtkWidget *frame = NULL;
  GtkWidget *scr = NULL;
  GtkTextMark *mark = NULL;
  GtkTextBuffer *buffer = NULL;
  GtkTextIter end;


  /* Get the structs from the application */
  GmWindow *gw = gnomemeeting_get_main_window (gm);
  
  gw->history_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (gw->history_window), 
			_("General History"));
  gtk_window_set_position (GTK_WINDOW (gw->history_window), 
			   GTK_WIN_POS_CENTER);
  gtk_window_set_default_size (GTK_WINDOW (gw->history_window), 
			       325, 175);

  frame = gtk_frame_new (_("General History"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);

  gw->history_text_view = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (gw->history_text_view), 
			      FALSE);

  gtk_text_view_set_editable (GTK_TEXT_VIEW (gw->history_text_view), 
			      FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (gw->history_text_view),
			       GTK_WRAP_WORD);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (gw->history_text_view), 
				    FALSE);

  buffer = 
    gtk_text_view_get_buffer (GTK_TEXT_VIEW (gw->history_text_view));
  gtk_text_buffer_get_end_iter (buffer, &end);
  mark = gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (buffer), 
				      "current-position", &end, FALSE);

  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (frame), scr);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_container_set_border_width (GTK_CONTAINER (scr), 2);
  
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);

  gtk_container_add (GTK_CONTAINER (scr), gw->history_text_view);
  gtk_container_add (GTK_CONTAINER (gw->history_window), frame);    
 
  g_signal_connect (G_OBJECT (gw->history_window), "delete_event",
		    G_CALLBACK (gtk_widget_hide_on_delete), NULL); 
}
