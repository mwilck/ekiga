
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

#include "../config.h"


#include "common.h"

#include "addressbook_window.h"
#include "main_window.h"
#include "calls_history_window.h"
#include "gnomemeeting.h"
#include "callbacks.h" 
#include "misc.h"

#include <contacts/gm_contacts.h>
#include <gtk_menu_extensions.h>
#include <gm_conf.h>
#include <stock-icons.h>


/* internal representation */
struct _GmCallsHistoryWindow
{
  GtkWidget *chw_history_tree_view [3];
  GtkWidget *chw_search_entry;
  GtkWidget *chw_notebook;
};
typedef struct _GmCallsHistoryWindow GmCallsHistoryWindow;


#define GM_CALLS_HISTORY_WINDOW(x) (GmCallsHistoryWindow *) (x)


/* Declarations */

/* GUI functions */

/* DESCRIPTION  : / 
 * BEHAVIOR     : Frees a GmCallsHistoryWindow and its content.
 * PRE          : A non-NULL pointer to a GmCallsHistoryWindow.
 */
static void gm_chw_destroy (gpointer pointer);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to the private GmCallsHistoryWindow
 * 		  used by the calls history GMObject.
 * PRE          : The given GtkWidget pointer must be a calls history GMObject.
 */
static GmCallsHistoryWindow *gm_chw_get_chw (GtkWidget *);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to a newly allocated GmContact with
 * 		  all the info for the contact currently being selected
 * 		  in the calls history window given as argument. NULL if none
 * 		  is selected.
 * PRE          : The given GtkWidget pointer must point to the calls history
 * 		  window GMObject.
 */
static GmContact *gm_chw_get_selected_contact (GtkWidget *);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Creates a calls history item menu and returns it.
 * PRE          : The given GtkWidget pointer must point to the calls history
 * 		  window GMObject.
 */
static GtkWidget *gm_chw_contact_menu_new (GtkWidget *);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Populate the calls history with the stored history.
 * PRE          : The given GtkWidget pointer must point to the calls history
 * 		  window GMObject.
 */
static void gm_chw_populate (GtkWidget *);

	
/* Operations on the calls history directly */

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Add a call to the calls history.
 * PRE          :  /
 */
static void gm_calls_history_add_call (int,
				       const char *,
				       const char *,
				       const char *,
				       const char *,
				       const char *,
				       const char *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Clears the calls history.
 * PRE          :  /
 */
static void gm_calls_history_clear (int);


/* Callbacks */

/* DESCRIPTION  :  This callback is called when the user has clicked the clear
 *                 button.
 * BEHAVIOR     :  Clears the corresponding calls list using the config DB.
 * PRE          :  data = the calls history window GMObject.
 */
static void clear_button_clicked_cb (GtkButton *,
				     gpointer);


/* DESCRIPTION  :  This callback is called when the user has clicked the find
 *                 button.
 * BEHAVIOR     :  Hides the rows that do not contain the text in the search
 *                 entry.
 * PRE          :  data = the GtkNotebook containing the 3 lists of calls.
 */
static void find_button_clicked_cb (GtkButton *,
				    gpointer);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user clicks on a contact.
 * 		  Displays a popup.
 * PRE          : A valid pointer to the calls history window GMObject.
 */
static gint contact_clicked_cb (GtkWidget *w,
				GdkEventButton *e,
				gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user chooses to call 
 * 	  	  a contact using the menu.
 * PRE          : The calls history window as argument.  
 */
static void call_contact1_cb (GtkWidget *,
			      gpointer);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user chooses to call 
 * 	  	  a contact by double-clicking on it.
 * PRE          : The calls history window as argument.  
 */
static void call_contact2_cb (GtkTreeView *,
			      GtkTreePath *,
			      GtkTreeViewColumn *,
			      gpointer);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user chooses to add the
 * 		  contact in the address book.
 * 		  It presents the dialog to edit a contact. 
 * PRE          : The gpointer must point to the calls history window. 
 */
static void add_contact_cb (GtkWidget *,
			    gpointer);


/* DESCRIPTION  :  This function is called when the user drops the contact.
 * BEHAVIOR     :  Returns the dragged contact
 * PRE          :  Assumes data hides a calls history window (widget)
 */
static GmContact *dnd_get_contact (GtkWidget *widget, 
				   gpointer data);


/* Implementation */
static void
gm_chw_destroy (gpointer data)
{
  g_return_if_fail (data);
  
  delete ((GmCallsHistoryWindow *) data);
}


static GmCallsHistoryWindow *
gm_chw_get_chw (GtkWidget *calls_history_window)
{
  g_return_val_if_fail (calls_history_window != NULL, NULL);

  return GM_CALLS_HISTORY_WINDOW (g_object_get_data (G_OBJECT (calls_history_window), "GMObject"));
}


static GmContact *
gm_chw_get_selected_contact (GtkWidget *calls_history_window)
{
  GmContact *contact = NULL;

  GmCallsHistoryWindow *chw = NULL;

  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  
  int page_num = 0;

  
  g_return_val_if_fail (calls_history_window != NULL, NULL);

  chw = gm_chw_get_chw (calls_history_window);

  g_return_val_if_fail (chw != NULL, NULL);

  page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (chw->chw_notebook));
      
  g_return_val_if_fail ((page_num == RECEIVED_CALL || page_num == PLACED_CALL || page_num == MISSED_CALL), NULL);
  

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (chw->chw_history_tree_view [page_num]));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (chw->chw_history_tree_view [page_num]));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    contact = gm_contact_new ();

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			1, &contact->fullname,
			2, &contact->url,
			-1);
  }
  

  return contact;
}


GtkWidget *
gm_chw_contact_menu_new (GtkWidget *calls_history_window)
{
  GtkWidget *menu = NULL;

  menu = gtk_menu_new ();
  
      
  static MenuEntry contact_menu [] =
    {
      GTK_MENU_ENTRY("call", _("C_all Contact"), NULL,
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (call_contact1_cb), 
		     calls_history_window, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("add", _("Add Contact to _Address Book"), NULL,
		     GTK_STOCK_ADD, 0,
		     GTK_SIGNAL_FUNC (add_contact_cb), 
		     calls_history_window, TRUE),

      GTK_MENU_END
    };
  
    
  gtk_build_menu (menu, contact_menu, NULL, NULL);

  
  return menu;
}


static void
gm_chw_populate (GtkWidget *calls_history_window)
{
  GmCallsHistoryWindow *chw = NULL;
  
  GtkTreeIter iter;
  GtkListStore *list_store = NULL;

  gchar *conf_key = NULL;
  gchar **call_data = NULL;
  
  GSList *calls_list = NULL;

  chw = gm_chw_get_chw (calls_history_window);

  for (int i = 0 ; i < MAX_VALUE_CALL ; i++) {

    list_store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (chw->chw_history_tree_view [i])));
    
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


static void
gm_calls_history_add_call (int i,
			   const char *time,
			   const char *remote_user,
			   const char *ip,
			   const char *duration,
			   const char *reason,
			   const char *software)
{
  gchar *conf_key = NULL;
  gchar *call_data = NULL;
  
  GSList *calls_list = NULL;
  GSList *tmp = NULL;
  
  
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


static void 
gm_calls_history_clear (int i)
{
  /* Clears the configuration */
  switch (i) {

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


static GmContact *
dnd_get_contact (GtkWidget *widget, 
		 gpointer data)
{
  GtkWidget *chw = NULL;
  
  chw = GTK_WIDGET (data);
  
  return gm_chw_get_selected_contact (chw);
}


static void
clear_button_clicked_cb (GtkButton *b, 
			 gpointer data)
{
  GmCallsHistoryWindow *chw  = NULL;

  
  g_return_if_fail (data != NULL);
  
  chw = gm_chw_get_chw (GTK_WIDGET (data));

  g_return_if_fail (chw);
  
  gm_calls_history_window_clear (GTK_WIDGET (data),
				 gtk_notebook_get_current_page (GTK_NOTEBOOK (chw->chw_notebook)));
}


static void
find_button_clicked_cb (GtkButton *b, 
			gpointer data)
{ 
  GmCallsHistoryWindow *chw = NULL;

  GtkListStore *list_store = NULL;
  GtkTreeIter iter;
  const char *entry_text = NULL;
  gchar *date = NULL;
  gchar *software = NULL;
  gchar *remote_user = NULL;
  gchar *end_reason = NULL;

  BOOL removed = FALSE;
  BOOL ok = FALSE;

  gint page = 0;

  
  g_return_if_fail (data != NULL);

  /* Fill in the window */
  gm_chw_populate (GTK_WIDGET (data));  

  chw = gm_chw_get_chw (GTK_WIDGET (data));

  g_return_if_fail (chw);


  entry_text = gtk_entry_get_text (GTK_ENTRY (chw->chw_search_entry));

  page = gtk_notebook_get_current_page (GTK_NOTEBOOK (chw->chw_notebook));
    
  list_store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (chw->chw_history_tree_view [page])));


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


static gint
contact_clicked_cb (GtkWidget *w,
		    GdkEventButton *e,
		    gpointer data)
{
  GtkWidget *menu = NULL;
  
  g_return_val_if_fail (data != NULL, FALSE);

  if (e->type == GDK_BUTTON_PRESS || e->type == GDK_KEY_PRESS) {

    if (e->button == 3) {

      menu = gm_chw_contact_menu_new (GTK_WIDGET (data));
      gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
		      e->button, e->time);
      g_signal_connect (G_OBJECT (menu), "hide",
			GTK_SIGNAL_FUNC (g_object_unref), (gpointer) menu);
      g_object_ref (G_OBJECT (menu));
      gtk_object_sink (GTK_OBJECT (menu));
    }
  }

  return TRUE;
}


static void
call_contact1_cb (GtkWidget *w,
		  gpointer data)
{
  GmContact *contact = NULL;

  GtkWidget *calls_history_window = NULL;

  g_return_if_fail (data != NULL);

  calls_history_window = GTK_WIDGET (data);
  
  contact = gm_chw_get_selected_contact (calls_history_window);

  if (contact) {

    GnomeMeeting::Process ()->Connect (contact->url);
    gm_contact_delete (contact);
  }
}


static void
call_contact2_cb (GtkTreeView *tree_view,
		  GtkTreePath *arg1,
		  GtkTreeViewColumn *arg2,
		  gpointer data)
{
  g_return_if_fail (data != NULL);

  call_contact1_cb (NULL, data);
}


static void
add_contact_cb (GtkWidget *w,
		gpointer data)
{
  GmContact *contact = NULL;

  GtkWidget *calls_history_window = NULL;
  GtkWidget *addressbook_window = NULL;

  g_return_if_fail (data != NULL);

  calls_history_window = GTK_WIDGET (data);
  addressbook_window = GnomeMeeting::Process ()->GetAddressbookWindow ();
  
  contact = gm_chw_get_selected_contact (calls_history_window);

  if (contact) {
    
    gm_addressbook_window_edit_contact_dialog_run (addressbook_window,
						   NULL, 
						   contact, 
						   FALSE,
						   calls_history_window);
    gm_contact_delete (contact);  
  }
}


/* The functions */
void 
gm_calls_history_window_clear (GtkWidget *calls_history_window,
			       int i)
{
  GmCallsHistoryWindow *chw  = NULL;

  GtkWidget *main_window = NULL;
  GtkListStore *list_store = NULL;
  
  
  g_return_if_fail (calls_history_window != NULL);
  
  chw = gm_chw_get_chw (GTK_WIDGET (calls_history_window));

  g_return_if_fail (chw);

  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  

  /* Clear the selected history */
  list_store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (chw->chw_history_tree_view [i])));
  gtk_list_store_clear (list_store);

  
  /* Clears the configuration */
  gm_calls_history_clear (i);


  /* Update the urls history for the main window */
  gm_main_window_urls_history_update (main_window);
}


void 
gm_calls_history_window_add_call (GtkWidget *calls_history_window,
				  int i,
				  const char *remote_user,
				  const char *ip,
				  const char *duration,
				  const char *reason,
				  const char *software)
{
  GmCallsHistoryWindow *chw = NULL;
  
  GtkTreeIter iter;
  GtkListStore *list_store = NULL;

  GtkWidget *main_window = NULL;
  
  PString time;
  
  g_return_if_fail (calls_history_window != NULL);
  chw = gm_chw_get_chw (calls_history_window);
  g_return_if_fail (chw != NULL);
  
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  time = PTime ().AsString ("yyyy/MM/dd hh:mm:ss");

  
  /* Update the calls history window */
  list_store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (chw->chw_history_tree_view [i])));

  gtk_list_store_append (list_store, &iter);
  gtk_list_store_set (list_store,
		      &iter,
		      0, (const char *) time ? (const char *) time : "",
		      1, remote_user ? remote_user : "",
		      2, ip ? ip : "",
		      3, duration ? duration : "",
		      4, reason ? reason : "",
		      5, software ? software : "",
		      -1);

  
  /* Add it to the configuration */
  gm_calls_history_add_call (i,
			     time, 
			     remote_user, 
			     ip, 
			     duration, 
			     reason, 
			     software);

  
  /* Update the urls history for the main window */
  gm_main_window_urls_history_update (main_window);
}


GSList *
gm_calls_history_get_calls ()
{
  GmContact *contact = NULL;

  GSList *calls_list = NULL;
  GSList *result = NULL;

  gchar **call_data = NULL;
  gchar *conf_key = NULL;


  for (int i = 0 ; i < MAX_VALUE_CALL ; i++) {

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

    calls_list = gm_conf_get_string_list (conf_key);

    while (calls_list && calls_list->data) {
      
      call_data = g_strsplit ((char *) calls_list->data, "|", 0);
      
      if (call_data) {
	
	contact = gm_contact_new ();
	
	if (call_data [1])
	  contact->fullname = g_strdup (call_data [1]);
	
	if (call_data [2])
	  contact->url = g_strdup (call_data [2]);
      
	result = g_slist_append (result, (gpointer) contact);
      }
      
      g_strfreev (call_data);

      calls_list = g_slist_next (calls_list);
    }

    g_slist_foreach (calls_list, (GFunc) g_free, NULL);
    g_slist_free (calls_list);
  }


  return result;
}


GtkWidget *
gm_calls_history_window_new ()
{
  GtkWidget *hbox = NULL;
  GtkWidget *button = NULL;
  GtkWidget *window = NULL;
  GtkWidget *scr = NULL;
  GtkWidget *label = NULL;
  GdkPixbuf *icon = NULL;
  GmCallsHistoryWindow *chw = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkCellRenderer *renderer = NULL;

  GtkListStore *list_store = NULL;

  gchar *label_text [3] =
    {N_("Received Calls"), N_("Placed Calls"), N_("Missed Calls")};
  label_text [0] = gettext (label_text [0]);
  label_text [1] = gettext (label_text [1]);
  label_text [2] = gettext (label_text [2]);

  
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

  chw = new GmCallsHistoryWindow ();
  g_object_set_data_full (G_OBJECT (window), "GMObject", 
			  chw, gm_chw_destroy);


  /* The notebook containing the 3 lists of calls */
  chw->chw_notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (chw->chw_notebook), 6);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), 
		      chw->chw_notebook, TRUE, TRUE, 0);


  for (int i = 0 ; i < MAX_VALUE_CALL ; i++) {

    label = gtk_label_new (N_(label_text [i]));
    
    scr = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), 
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);

    list_store = 
      gtk_list_store_new (6,
			  G_TYPE_STRING,
			  G_TYPE_STRING,
			  G_TYPE_STRING,
			  G_TYPE_STRING,
			  G_TYPE_STRING,
			  G_TYPE_STRING);
    
    chw->chw_history_tree_view [i] = 
      gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (chw->chw_history_tree_view [i]), TRUE);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Date"),
						       renderer,
						       "text", 
						       0,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (chw->chw_history_tree_view [i]), column);
    
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Remote User"),
						       renderer,
						       "text", 
						       1,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (chw->chw_history_tree_view [i]), column);
    g_object_set (G_OBJECT (renderer), "weight", "bold", NULL);
        
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Call Duration"),
						       renderer,
						       "text", 
						       3,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (chw->chw_history_tree_view [i]), column);
    if (i == 2)
      gtk_tree_view_column_set_visible (column, false);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Call End Reason"),
						       renderer,
						       "text", 
						       4,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (chw->chw_history_tree_view [i]), column);
    g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC, NULL);
    
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Software"),
						       renderer,
						       "text", 
						       5,
						       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (chw->chw_history_tree_view [i]), column);
    g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC, NULL);
	
    
    gtk_container_add (GTK_CONTAINER (scr), chw->chw_history_tree_view [i]);
    gtk_notebook_append_page (GTK_NOTEBOOK (chw->chw_notebook), scr, label);


    /* Signal to call the person on the double-clicked row */
    g_signal_connect (G_OBJECT (chw->chw_history_tree_view [i]), 
		      "row_activated", 
		      G_CALLBACK (call_contact2_cb), 
		      window);

    /* The drag and drop information */
    gm_contacts_dnd_set_source (GTK_WIDGET (chw->chw_history_tree_view [i]),
				dnd_get_contact, window);

    /* Right-click on a contact */
    g_signal_connect (G_OBJECT (chw->chw_history_tree_view [i]), "event_after",
		      G_CALLBACK (contact_clicked_cb), 
		      window);
  }


  /* The hbox added below the notebook that contains the Search field,
     and the search and clear buttons */
  hbox = gtk_hbox_new (FALSE, 0);  
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
  chw->chw_search_entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), chw->chw_search_entry, TRUE, TRUE, 2);
  g_signal_connect (G_OBJECT (chw->chw_search_entry), "activate",
		    G_CALLBACK (find_button_clicked_cb),
		    (gpointer) window);  
  
  button = gtk_button_new_from_stock (GTK_STOCK_FIND);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (find_button_clicked_cb),
		    (gpointer) window);

  button = gtk_button_new_from_stock (GTK_STOCK_CLEAR);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (clear_button_clicked_cb),
		    (gpointer) window);

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
  gm_chw_populate (window);

  
  gtk_widget_show_all (GTK_WIDGET (GTK_DIALOG (window)->vbox));
  
  return window;
}
