
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


extern GtkWidget *gm;


/* The functions */
void
gnomemeeting_calls_history_window_add_call (int i,
					    char *date,
					    char *remote_user,
					    char *ip,
					    char *duration,
					    char *software)
{
  GtkListStore *list_store = NULL;
  GtkTreeIter iter;

  int n = 0;
  
  GmCallsHistoryWindow *chw = NULL;

  chw = gnomemeeting_get_calls_history_window (gm);
  
  switch (i) {

  case 0:
    list_store = chw->received_calls_list_store;
    break;
  case 1:
    list_store = chw->given_calls_list_store;
    break;
  case 2:
    list_store = chw->missed_calls_list_store;
    break;    
  }

  n = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store), NULL);


  if (n == 0
      || gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list_store),
					&iter, NULL, n - 1)) {

    gtk_list_store_append (list_store, &iter);

    gtk_list_store_set (list_store, &iter,
			0, date,
			1, remote_user,
			2, ip,
			3, duration,
			4, software, -1);
  }
}


void gnomemeeting_init_calls_history_window ()
{
  GtkWidget *notebook = NULL;
  GtkWidget *scr = NULL;
  GtkWidget *label = NULL;
  GtkWidget *tree_view = NULL;

  GtkTreeViewColumn *column = NULL;
  GtkCellRenderer *renderer = NULL;

  GtkListStore *list_store [3];
  
  gchar *label_text [3] =
    {N_("Received calls"), N_("Given calls"), N_("Missed calls")};
  label_text [0] = gettext (label_text [0]);
  label_text [1] = gettext (label_text [1]);
  label_text [2] = gettext (label_text [2]);
  
  /* Get the structs from the application */
  GmWindow *gw = gnomemeeting_get_main_window (gm);
  GmCallsHistoryWindow *chw = gnomemeeting_get_calls_history_window (gm);
  
  gw->calls_history_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (gw->calls_history_window), 
			_("Calls History"));
  gtk_window_set_position (GTK_WINDOW (gw->calls_history_window), 
			   GTK_WIN_POS_CENTER);
  gtk_window_set_default_size (GTK_WINDOW (gw->calls_history_window), 
			       330, 225);

  notebook = gtk_notebook_new ();
  gtk_container_add (GTK_CONTAINER (gw->calls_history_window), notebook);

  for (int i = 0 ; i < 3 ; i++) {

    label = gtk_label_new (N_(label_text [i]));
    scr = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), 
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);

    list_store [i] = 
      gtk_list_store_new (6, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
			  G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    
    tree_view = 
      gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store [i]));
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree_view), TRUE);
    
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Date"),
						       renderer,
						       "text", 
						       0,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Remote user"),
						       renderer,
						       "text", 
						       1,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    g_object_set (G_OBJECT (renderer), "weight", "bold",
		  "style", PANGO_STYLE_ITALIC, NULL);
        
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("IP"),
						       renderer,
						       "text", 
						       2,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    g_object_set (G_OBJECT (renderer), "foreground", "darkgray",
		  "style", PANGO_STYLE_ITALIC, NULL);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Call duration"),
						       renderer,
						       "text", 
						       3,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Software"),
						       renderer,
						       "text", 
						       4,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC, NULL);
	
    
    gtk_container_add (GTK_CONTAINER (scr), tree_view);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scr, label);
  }

  chw->received_calls_list_store = list_store [0];
  chw->given_calls_list_store = list_store [1];
  chw->missed_calls_list_store = list_store [2];

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
