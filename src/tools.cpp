
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
#include "gnomemeeting.h"
#include "callbacks.h"
#include "ldap_window.h"
#include "misc.h"
#include "stock-icons.h"


extern GtkWidget *gm;
extern GnomeMeeting *MyApp;


static void dnd_drag_data_get_cb (GtkWidget *,
				  GdkDragContext *,
				  GtkSelectionData *,
				  guint,
				  guint,
				  gpointer);
  

/* DESCRIPTION  :  This callback is called when the user has released the drag.
 * BEHAVIOR     :  Puts the required data into the selection_data, we put
 *                 name and the url fields for now.
 * PRE          :  data = the type of the page from where the drag occured :
 *                 CONTACTS_GROUPS or CONTACTS_SERVERS.
 */
static void
dnd_drag_data_get_cb (GtkWidget *tree_view,
		      GdkDragContext *dc,
		      GtkSelectionData *selection_data,
		      guint info,
		      guint t,
		      gpointer data)
{
  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  
  gchar *contact_name = NULL;
  gchar *contact_url = NULL;
  gchar *drag_data = NULL;

        
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
      
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			1, &contact_name,
			2, &contact_url, -1);

    if (contact_name && contact_url) {
      
      drag_data = g_strdup_printf ("%s|%s", contact_name, contact_url);
    
      gtk_selection_data_set (selection_data,
			      selection_data->target, 
			      8,
			      (const guchar *) drag_data,
			      strlen (drag_data));
      g_free (drag_data);
    }
  }
  
  g_free (contact_name);
  g_free (contact_url);
}


/* The functions */
void
gnomemeeting_calls_history_window_add_call (int i,
					    const char *remote_user,
					    const char *ip,
					    const char *duration,
					    const char *software)
{
  GtkListStore *list_store = NULL;
  GtkTreeIter iter;

  gchar *utf8_time = NULL;

  int n = 0;
  
  GmCallsHistoryWindow *chw = NULL;

  chw = MyApp->GetCallsHistoryWindow ();
  
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

    utf8_time = 
      gnomemeeting_from_iso88591_to_utf8 (PTime ().AsString ("www dd MMM, hh:mm:ss"));

    /* The "s" is for "seconds" */
    gtk_list_store_set (list_store, &iter,
			0, utf8_time,
			1, remote_user ? remote_user : "",
			2, ip ? ip : "",
			3, duration ? duration : "",
			4, software ? software : "",
			-1);

    g_free (utf8_time);
  }
}


GtkWidget *
gnomemeeting_calls_history_window_new (GmCallsHistoryWindow *chw)
{
  GtkWidget *window = NULL;
  GtkWidget *notebook = NULL;
  GtkWidget *scr = NULL;
  GtkWidget *label = NULL;
  GtkWidget *tree_view = NULL;
  GdkPixbuf *icon = NULL;
  
  GtkTreeViewColumn *column = NULL;
  GtkCellRenderer *renderer = NULL;

  GtkListStore *list_store [3];
  
  gchar *label_text [3] =
    {N_("Received Calls"), N_("Placed Calls"), N_("Unanswered Calls")};
  label_text [0] = gettext (label_text [0]);
  label_text [1] = gettext (label_text [1]);
  label_text [2] = gettext (label_text [2]);

  static GtkTargetEntry dnd_targets [] =
    {
      {"text/plain", GTK_TARGET_SAME_APP, 0}
    };

  
  window = gtk_dialog_new ();
  gtk_dialog_add_button (GTK_DIALOG (window), GTK_STOCK_CLOSE, 0);

  gtk_window_set_title (GTK_WINDOW (window), _("Calls History"));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size (GTK_WINDOW (window), 330, 225);
  icon = gtk_widget_render_icon (GTK_WIDGET (window),
				 GM_STOCK_CALLS_HISTORY,
				 GTK_ICON_SIZE_MENU, NULL);
  gtk_window_set_icon (GTK_WINDOW (window), icon);
  g_object_unref (icon);

  notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (notebook), 6);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), notebook,
		      TRUE, TRUE, 0);

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
    g_object_set (G_OBJECT (renderer), "weight", "bold", NULL);
        
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("URL"),
						       renderer,
						       "text", 
						       2,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    g_object_set (G_OBJECT (renderer), "foreground", "blue",
		  "underline", TRUE, NULL);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Call duration"),
						       renderer,
						       "text", 
						       3,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    if (i == 2)
      gtk_tree_view_column_set_visible (column, false);


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


    /* Signal to call the person on the double-clicked row */
    g_signal_connect (G_OBJECT (tree_view), "row_activated", 
		      G_CALLBACK (contact_activated_cb), GINT_TO_POINTER (3));

    /* The drag and drop information */
    gtk_drag_source_set (GTK_WIDGET (tree_view),
			 GDK_BUTTON1_MASK, dnd_targets, 1,
			 GDK_ACTION_COPY);
    g_signal_connect (G_OBJECT (tree_view), "drag_data_get",
		      G_CALLBACK (dnd_drag_data_get_cb), NULL);

    /* Right-click on a contact */
    g_signal_connect (G_OBJECT (tree_view), "event_after",
		    G_CALLBACK (contact_clicked_cb), GINT_TO_POINTER (1));
  }

  chw->received_calls_list_store = list_store [0];
  chw->given_calls_list_store = list_store [1];
  chw->missed_calls_list_store = list_store [2];

  g_signal_connect (G_OBJECT (window), "delete_event",
		    G_CALLBACK (gtk_widget_hide_on_delete), NULL);

  g_signal_connect_swapped (GTK_OBJECT (window), 
			    "response", 
			    G_CALLBACK (gtk_widget_hide_all),
			    (gpointer) window);

  gtk_widget_show_all (GTK_WIDGET (GTK_DIALOG (window)->vbox));
  
  return window;
}


GtkWidget *gnomemeeting_history_window_new ()
{
  GtkWidget *window = NULL;
  GtkWidget *scr = NULL;
  GtkTextMark *mark = NULL;
  GtkTextBuffer *buffer = NULL;
  GtkTextIter end;

  GmWindow *gw = MyApp->GetMainWindow ();

  /* Fix me, create a structure for that so that we don't use
     gw here */
  window = gtk_dialog_new ();
  gtk_dialog_add_button (GTK_DIALOG (window), GTK_STOCK_CLOSE, 0);

  gtk_window_set_title (GTK_WINDOW (window), _("General History"));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size (GTK_WINDOW (window), 330, 225);

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
  gtk_container_set_border_width (GTK_CONTAINER (scr), 6);
  
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);

  gtk_container_add (GTK_CONTAINER (scr), gw->history_text_view);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), scr,
		      TRUE, TRUE, 0);
 
  g_signal_connect (G_OBJECT (window), "delete_event",
		    G_CALLBACK (gtk_widget_hide_on_delete), NULL);

  g_signal_connect_swapped (GTK_OBJECT (window), 
			    "response", 
			    G_CALLBACK (gtk_widget_hide_all),
			    (gpointer) window);

  gtk_widget_show_all (GTK_WIDGET (GTK_DIALOG (window)->vbox));
  
  return window;
}
