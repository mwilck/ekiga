
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
 *                         calls_history_window.cpp  -  description
 *                         -----------------------
 *   begin                : Sun Sep 1 2002
 *   copyright            : (C) 2000-2004 by Damien Sandras 
 *   description          : This file defines functions to manage the
 *                          calls history
 */

#include "common.h"

#include "calls_history_window.h"
#include "callbacks.h"
#include "gnomemeeting.h"
#include "misc.h"

#include "contacts/gm_contacts.h"
#include "gm_conf.h"
#include "gnome_prefs_window.h"
#include "stock-icons.h"


/* internal representation */
struct _GmCallsHistory
{
  GtkListStore *given_calls_list_store;
  GtkListStore *received_calls_list_store;
  GtkListStore *missed_calls_list_store;
  GtkWidget *search_entry;
};

/* Helpers' declarations */
static void gm_calls_history_destroy (gpointer pointer);

#if 0
static void dnd_drag_data_get_cb (GtkWidget *,
				  GdkDragContext *,
				  GtkSelectionData *,
				  guint,
				  guint,
				  gpointer);
#endif

static void clear_button_clicked_cb (GtkButton *,
				     gpointer);

static void find_button_clicked_cb (GtkButton *,
				    gpointer);

/* Helpers' definitions */

/* DESCRIPTION  :  Called when the chat window is destroyed
 * BEHAVIOR     :  Frees the GmTextChat* that is embedded in the window
 * PRE          :  /
 */
static void
gm_calls_history_destroy (gpointer pointer)
{
  delete (GmCallsHistory *)pointer;
}


#if 0
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
  GmContact *contact = NULL;

        
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
      
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			1, &contact_name,
			2, &contact_url, -1);

    if (contact_name && contact_url) {
      
      contact = gm_contact_new ();
      contact->fullname = g_strdup (contact_name);
      contact->url = g_strdup (contact_url);
    
      gtk_selection_data_set (selection_data, selection_data->target, 
			      8, (guchar *)&contact, sizeof (contact));
    }
  }
}
#endif


/* DESCRIPTION  :  This callback is called when the user has clicked the clear
 *                 button.
 * BEHAVIOR     :  Clears the corresponding calls list using the config DB.
 * PRE          :  data = the GtkNotebook containing the 3 lists of calls.
 */
static void
clear_button_clicked_cb (GtkButton *b, gpointer data)
{
  g_return_if_fail (data != NULL);
  
  switch (gtk_notebook_get_current_page (GTK_NOTEBOOK (data))) {

  case RECEIVED_CALL:
    gm_conf_set_string_list (USER_INTERFACE_KEY "calls_history_window/received_calls_history", NULL);
    break;
  case PLACED_CALL:
    gm_conf_set_string_list (USER_INTERFACE_KEY "calls_history_window/placed_calls_history", NULL);
    break;
  case MISSED_CALL:
    gm_conf_set_string_list (USER_INTERFACE_KEY "calls_history_window/missed_calls_history", NULL);
    break;
  }
}

/* DESCRIPTION  :  This callback is called when the user has clicked the find
 *                 button.
 * BEHAVIOR     :  Hides the rows that do not contain the text in the search
 *                 entry.
 * PRE          :  data = the GtkNotebook containing the 3 lists of calls.
 */
static void
find_button_clicked_cb (GtkButton *b, gpointer data)
{ 
  GtkWidget *chw = NULL;
  GmCallsHistory *ch = NULL;

  GtkListStore *list_store = NULL;
  GtkTreeIter iter;
  const char *entry_text = NULL;
  gchar *date = NULL;
  gchar *software = NULL;
  gchar *remote_user = NULL;
  gchar *end_reason = NULL;

  BOOL removed = FALSE;
  BOOL ok = FALSE;

  g_return_if_fail (data != NULL);

  /* Fill in the window */
  chw = GnomeMeeting::Process ()->GetCallsHistoryWindow ();
  ch = (GmCallsHistory *)g_object_get_data (G_OBJECT (chw), "GMObject");
  gnomemeeting_calls_history_window_populate (chw);  
  entry_text = gtk_entry_get_text (GTK_ENTRY (ch->search_entry));

  switch (gtk_notebook_get_current_page (GTK_NOTEBOOK (data))) {
    
  case RECEIVED_CALL:
    list_store = ch->received_calls_list_store;
    break;
  case PLACED_CALL:
    list_store = ch->given_calls_list_store;
    break;
  case MISSED_CALL:
    list_store = ch->missed_calls_list_store;
    break;
  }

  if (strcmp (entry_text, "")
      && gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list_store), &iter)) {

    do {

      ok = FALSE;
      removed = FALSE;
      gtk_tree_model_get (GTK_TREE_MODEL (list_store),
			  &iter,
			  0, &date,
			  1, &remote_user,
			  4, &end_reason,
			  5, &software,
			  -1);

      if (!(PString (date).Find (entry_text) != P_MAX_INDEX
	  || PString (remote_user).Find (entry_text) != P_MAX_INDEX
	  || PString (end_reason).Find (entry_text) != P_MAX_INDEX
	    || PString (software).Find (entry_text) != P_MAX_INDEX)) {
	
	ok = gtk_list_store_remove (GTK_LIST_STORE (list_store), &iter);
	removed = TRUE;
      }
      
      g_free (date);
      g_free (remote_user);
      g_free (end_reason);
      g_free (software);

      if (!removed)
	ok = gtk_tree_model_iter_next (GTK_TREE_MODEL (list_store), &iter);
	
    } while (ok);
  }
}

/* The functions */
void
gnomemeeting_calls_history_window_populate (GtkWidget *chw)
{
  GmCallsHistory *ch = NULL;
  GtkTreeIter iter;
  GtkListStore *list_store = NULL;

  gchar *conf_key = NULL;
  gchar **call_data = NULL;
  
  GSList *calls_list = NULL;

  ch = (GmCallsHistory *)g_object_get_data (G_OBJECT (chw), "GMObject");

  for (int i = 0 ; i < MAX_VALUE_CALL ; i++) {
    
    switch (i) {

    case RECEIVED_CALL:
      list_store = ch->received_calls_list_store;
      conf_key =
	g_strdup (USER_INTERFACE_KEY "calls_history_window/received_calls_history");
      break;
    case PLACED_CALL:
      list_store = ch->given_calls_list_store;
      conf_key =
	g_strdup (USER_INTERFACE_KEY "calls_history_window/placed_calls_history");
      break;
    case MISSED_CALL:
      list_store = ch->missed_calls_list_store;
      conf_key =
	g_strdup (USER_INTERFACE_KEY "calls_history_window/missed_calls_history");
      break;
    }

    gtk_list_store_clear (list_store);
    
    calls_list = gm_conf_get_string_list (conf_key);

    while (calls_list && calls_list->data) {
      
      call_data = g_strsplit ((char *) calls_list->data, "|", 0);
      
      if (call_data) {
	
	gtk_list_store_append (list_store, &iter);
	gtk_list_store_set (list_store,
			    &iter,
			    0, call_data [0],
			    1, call_data [1],
			    2, call_data [2],
			    3, call_data [3],
			    4, call_data [4],
			    5, call_data [5],
			    -1);
      }
      
      g_strfreev (call_data);

      calls_list = g_slist_next (calls_list);
    }
    
    g_free (conf_key);
    g_slist_free (calls_list);
  }
}


void
gnomemeeting_calls_history_window_add_call (GtkWidget *chw,
					    int i,
					    const char *remote_user,
					    const char *ip,
					    const char *duration,
					    const char *reason,
					    const char *software)
{
  PString time;

  gchar *conf_key = NULL;
  gchar *call_data = NULL;
  
  GSList *calls_list = NULL;
  GSList *tmp = NULL;
  
  time = PTime ().AsString ("yyyy/MM/dd hh:mm:ss");
  
  switch (i) {

  case RECEIVED_CALL:
    conf_key =
      g_strdup (USER_INTERFACE_KEY "calls_history_window/received_calls_history");
    break;
  case PLACED_CALL:
    conf_key =
      g_strdup (USER_INTERFACE_KEY "calls_history_window/placed_calls_history");
    break;
  case MISSED_CALL:
    conf_key =
      g_strdup (USER_INTERFACE_KEY "calls_history_window/missed_calls_history");
    break;
  }

  
  call_data =
    g_strdup_printf ("%s|%s|%s|%s|%s|%s",
		     (const char *) time ? (const char *) time : "",
		     remote_user ? remote_user : "",
		     ip ? ip : "",
		     duration ? duration : "",
		     reason ? reason : "",
		     software ? software : "");
  
  calls_list = gm_conf_get_string_list (conf_key);
  calls_list = g_slist_append (calls_list, (gpointer) call_data);

  while (g_slist_length (calls_list) > 100) {

    tmp = g_slist_nth (calls_list, 0);
    calls_list = g_slist_remove_link (calls_list, tmp);

    g_slist_free_1 (tmp);
  }
  
  gm_conf_set_string_list (conf_key, calls_list);
  
  g_free (conf_key);
  g_slist_free (calls_list);
}


GtkWidget *
gnomemeeting_calls_history_window_new ()
{
  GtkWidget *hbox = NULL;
  GtkWidget *button = NULL;
  GtkWidget *window = NULL;
  GtkWidget *notebook = NULL;
  GtkWidget *scr = NULL;
  GtkWidget *label = NULL;
  GtkWidget *tree_view = NULL;
  GdkPixbuf *icon = NULL;
  GmCallsHistory *ch = NULL;
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
      {"GMContact", GTK_TARGET_SAME_APP, 0}
    };

  
  window = gtk_dialog_new ();
  gtk_dialog_add_button (GTK_DIALOG (window), GTK_STOCK_CLOSE, 0);

  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("calls_history_window"), g_free);
  
  gtk_window_set_title (GTK_WINDOW (window), _("Calls History"));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  icon = gtk_widget_render_icon (GTK_WIDGET (window),
				 GM_STOCK_CALLS_HISTORY,
				 GTK_ICON_SIZE_MENU, NULL);
  gtk_window_set_icon (GTK_WINDOW (window), icon);
  g_object_unref (icon);

  ch = new GmCallsHistory ();
  g_object_set_data_full (G_OBJECT (window), "GMObject", 
			  ch, gm_calls_history_destroy);
  /* The notebook containing the 3 lists of calls */
  notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (notebook), 6);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), notebook,
		      TRUE, TRUE, 0);


  for (int i = 0 ; i < MAX_VALUE_CALL ; i++) {

    label = gtk_label_new (N_(label_text [i]));
    
    scr = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), 
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);

    list_store [i] = 
      gtk_list_store_new (6,
			  G_TYPE_STRING,
			  G_TYPE_STRING,
			  G_TYPE_STRING,
			  G_TYPE_STRING,
			  G_TYPE_STRING,
			  G_TYPE_STRING);
    
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
    column = gtk_tree_view_column_new_with_attributes (_("Remote User"),
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
    gtk_tree_view_column_set_visible (column, false);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Call Duration"),
						       renderer,
						       "text", 
						       3,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    if (i == 2)
      gtk_tree_view_column_set_visible (column, false);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Call End Reason"),
						       renderer,
						       "text", 
						       4,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC, NULL);
    
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Software"),
						       renderer,
						       "text", 
						       5,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC, NULL);
	
    
    gtk_container_add (GTK_CONTAINER (scr), tree_view);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scr, label);


    /* Signal to call the person on the double-clicked row */
    //g_signal_connect (G_OBJECT (tree_view), "row_activated", 
//		      G_CALLBACK (contact_activated_cb), GINT_TO_POINTER (3));
    g_warning ("FIX ME: Not reimplemented yet");

    /* The drag and drop information */
    gtk_drag_source_set (GTK_WIDGET (tree_view),
			 GDK_BUTTON1_MASK, dnd_targets, 1,
			 GDK_ACTION_COPY);
#if 0
    g_signal_connect (G_OBJECT (tree_view), "drag_data_get",
		      G_CALLBACK (dnd_drag_data_get_cb), NULL);
#endif 

    /* Right-click on a contact */
  //  g_signal_connect (G_OBJECT (tree_view), "event_after",
//		    G_CALLBACK (contact_clicked_cb), GINT_TO_POINTER (0));

    g_warning ("FIX ME: Not reimplemented yet");
  }

  ch->received_calls_list_store = list_store [0];
  ch->given_calls_list_store = list_store [1];
  ch->missed_calls_list_store = list_store [2];


  /* The hbox added below the notebook that contains the Search field,
     and the search and clear buttons */
  hbox = gtk_hbox_new (FALSE, 0);  
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
  ch->search_entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), ch->search_entry, TRUE, TRUE, 2);
  g_signal_connect (G_OBJECT (ch->search_entry), "activate",
		    G_CALLBACK (find_button_clicked_cb),
		    (gpointer) notebook);  
  
  button = gtk_button_new_from_stock (GTK_STOCK_FIND);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (find_button_clicked_cb),
		    (gpointer) notebook);

  button = gtk_button_new_from_stock (GTK_STOCK_CLEAR);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (clear_button_clicked_cb),
		    (gpointer) notebook);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), hbox,
		      FALSE, FALSE, 0); 

  
  /* Generic signals */
  g_signal_connect_swapped (GTK_OBJECT (window), 
			    "response", 
			    G_CALLBACK (gnomemeeting_window_hide),
			    (gpointer) window);

  g_signal_connect (GTK_OBJECT (window), "delete-event", 
                    G_CALLBACK (delete_window_cb), NULL);

  
  /* Fill in the window with old calls */
  gnomemeeting_calls_history_window_populate (window);

  
  gtk_widget_show_all (GTK_WIDGET (GTK_DIALOG (window)->vbox));
  
  return window;
}
