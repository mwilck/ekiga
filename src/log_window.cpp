
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
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
 *                         log_window.cpp  -  description
 *                         -------------------------
 *   begin                : Sun Sep 1 2002
 *   copyright            : (C) 2000-2004 by Damien Sandras 
 *   description          : This file defines functions to manage the log
 *
 */

#include "common.h"

#include "log_window.h"

#include "callbacks.h"
#include "misc.h"

GtkWidget *
gnomemeeting_log_window_new ()
{
  GtkWidget *window = NULL;
  GtkWidget *scr = NULL;
  GtkWidget *text_view = NULL;
  GtkTextMark *mark = NULL;
  GtkTextBuffer *buffer = NULL;
  GtkTextIter end;

  window = gtk_dialog_new ();
  gtk_dialog_add_button (GTK_DIALOG (window), GTK_STOCK_CLOSE, 0);
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("log_window"), g_free);
  
  gtk_window_set_title (GTK_WINDOW (window), _("General History"));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  
  text_view = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (text_view), FALSE);

  gtk_text_view_set_editable (GTK_TEXT_VIEW (text_view), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view), GTK_WRAP_WORD);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (text_view), FALSE);
  g_object_set_data (G_OBJECT (window), "text_view", (gpointer)text_view);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
  gtk_text_buffer_get_end_iter (buffer, &end);
  mark = gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (buffer), 
				      "current-position", &end, FALSE);

  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_set_border_width (GTK_CONTAINER (scr), 6);
  
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);

  gtk_container_add (GTK_CONTAINER (scr), text_view);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), scr,
		      TRUE, TRUE, 0);
  
  g_signal_connect_swapped (GTK_OBJECT (window), 
			    "response", 
			    G_CALLBACK (gnomemeeting_window_hide),
			    (gpointer) window);

  g_signal_connect (GTK_OBJECT (window), "delete-event", 
                    G_CALLBACK (delete_window_cb), NULL);
  
  gtk_widget_show_all (GTK_WIDGET (GTK_DIALOG (window)->vbox));
  
  return window;
}

void 
gnomemeeting_log_insert (GtkWidget *log_window, const char *format,
			 ...)
{
  va_list args;
  GtkWidget *text_view = NULL;
  GtkTextIter end;
  GtkTextMark *mark;
  GtkTextBuffer *buffer;
	
  time_t *timeptr;
  char *time_str;
  gchar *text_buffer = NULL;
  char buf [1025];

  g_return_if_fail (log_window != NULL && format != NULL);
  
  text_view = GTK_WIDGET (g_object_get_data (G_OBJECT (log_window),
					     "text_view"));
      
  va_start (args, format);

  vsnprintf (buf, 1024, format, args);

  time_str = (char *) malloc (21);
  timeptr = new (time_t);

  time (timeptr);
  strftime(time_str, 20, "%H:%M:%S", localtime (timeptr));

  text_buffer = g_strdup_printf ("%s %s\n", time_str, buf);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

  gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (buffer), &end, -1);
  gtk_text_buffer_insert (GTK_TEXT_BUFFER (buffer), &end, text_buffer, -1);

  mark = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (buffer), 
				   "current-position");

  if (mark)
    gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (text_view), mark, 
 				  0.0, FALSE, 0,0);
  
  g_free (text_buffer);
  free (time_str);
  delete (timeptr);
}
