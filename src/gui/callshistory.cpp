
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 *                         calls_history_window.cpp  -  description
 *                         -----------------------
 *   begin                : Sun Sep 1 2002
 *   copyright            : (C) 2000-2006 by Damien Sandras 
 *   description          : This file defines functions to manage the
 *                          calls history
 */

#include "../../config.h"


#include "common.h"

#include "contacts.h"
#include "main.h"
#include "chat.h"
#include "callshistory.h"
#include "ekiga.h"
#include "callbacks.h" 
#include "urlhandler.h"
#include "misc.h"

#include "gmcontacts.h"
#include "gmmenuaddon.h"
#include "gmconf.h"
#include "gmstockicons.h"


/* internal representation */
struct _GmCallsHistoryComponent
{
  GtkWidget *chc_tree_view;
  GtkWidget *chc_search_entry;
};
typedef struct _GmCallsHistoryComponent GmCallsHistoryComponent;

enum {

  COLUMN_TYPE,
  COLUMN_DATE,
  COLUMN_NAME,
  COLUMN_URL,
  COLUMN_DURATION,
  NUM_COLUMNS_HISTORY
};

#define GM_calls_history_component(x) (GmCallsHistoryComponent *) (x)


/* Declarations */

/* Call history item functions */

static GmCallsHistoryItem *gm_calls_history_item_new_from_string (const gchar *string_item);

static gchar *gm_calls_history_item_to_string (const GmCallsHistoryItem *item);

/* GUI functions */

/* DESCRIPTION  : / 
 * BEHAVIOR     : Frees a GmCallsHistoryComponent and its content.
 * PRE          : A non-NULL pointer to a GmCallsHistoryComponent.
 */
static void gm_chc_destroy (gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to the private GmCallsHistoryComponent
 * 		  used by the calls history GMObject.
 * PRE          : The given GtkWidget pointer must be a calls history GMObject.
 */
static GmCallsHistoryComponent *gm_chc_get_chc (GtkWidget *calls_history_component);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to a newly allocated GmContact with
 * 		  all the info for the contact currently being selected
 * 		  in the calls history window given as argument. NULL if none
 * 		  is selected.
 * PRE          : The given GtkWidget pointer must point to the calls history
 * 		  window GMObject.
 */
static GmContact *gm_chc_get_selected_contact (GtkWidget *calls_history_component);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Creates a calls history item menu and returns it.
 * PRE          : The given GtkWidget pointer must point to the calls history
 * 		  window GMObject.
 */
static GtkWidget *gm_chc_contact_menu_new (GtkWidget *calls_history_component);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Populate the calls history with the stored history.
 * PRE          : The given GtkWidget pointer must point to the calls history
 * 		  window GMObject.
 */
static void gm_chc_update (GtkWidget *calls_history_component);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns the config key associated with the calls history.
 * 		  indice.
 * PRE          : /
 */
static gchar *gm_chc_get_conf_key ();


/* DESCRIPTION  :  This function is called when the user drops the contact.
 * BEHAVIOR     :  Returns the dragged contact
 * PRE          :  Assumes data hides a calls history window (widget)
 */
static GmContact *dnd_get_contact (GtkWidget *widget, 
				   gpointer data);


/* Callbacks */

/* DESCRIPTION  :  This callback is called when the user has clicked the clear
 *                 button.
 * BEHAVIOR     :  Clears the corresponding calls list using the config DB.
 * PRE          :  data = the calls history window GMObject.
 */
static void clear_button_clicked_cb (GtkButton *widget,
				     gpointer data);


/* DESCRIPTION  :  This callback is called when the user modifies the
 *                 search entry.
 * BEHAVIOR     :  Hides the rows that do not contain the text in the search
 *                 entry.
 * PRE          :  data = the GtkNotebook containing the 3 lists of calls.
 */
static void search_entry_changed_cb (GtkButton *widget,
                                     gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user clicks on a contact.
 * 		  Displays a popup.
 * PRE          : A valid pointer to the calls history window GMObject.
 */
static gint contact_clicked_cb (GtkWidget *widget,
				GdkEventButton *event,
				gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user chooses to call 
 * 	  	  a contact using the menu.
 * PRE          : The calls history window as argument.  
 */
static void call_contact1_cb (GtkWidget *widget,
			      gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user chooses to call 
 * 	  	  a contact by double-clicking on it.
 * PRE          : The calls history window as argument.  
 */
static void call_contact2_cb (GtkTreeView *tree_view,
			      GtkTreePath *arg1,
			      GtkTreeViewColumn *arg2,
			      gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user chooses to add the
 * 		  contact in the address book.
 * 		  It presents the dialog to edit a contact. 
 * PRE          : The gpointer must point to the calls history window. 
 */
static void add_contact_cb (GtkWidget *widget,
			    gpointer data);


/* DESCRIPTION  :  This function is called to compare 1 GmContact to an URL.
 * BEHAVIOR     :  Returns 0 if both URLs are equal.
 * PRE          :  /
 */
static gint contact_compare_cb (gconstpointer contact,
				gconstpointer url);


/* DESCRIPTION  :  This callback is called when one of the calls history 	 *                 config value changes. 	 
 * BEHAVIOR     :  Rebuild its content, regenerate the cache of urls in 
 * 		   the main and chat windows.
 * PRE          :  A valid pointer to the calls history window GMObject. 	 
 */ 	 
static void 	 
calls_history_changed_nt (gpointer id,
			  GmConfEntry *entry,
			  gpointer data);


/* Implementation */
static GmCallsHistoryItem *
gm_calls_history_item_new_from_string (const gchar *serialized_item)
{
  GmCallsHistoryItem *result = NULL;
  gchar **item_data = NULL;

  g_return_val_if_fail (serialized_item != NULL, NULL);

  result = gm_calls_history_item_new ();
  item_data = g_strsplit (serialized_item, "|", 0);

  result->type = (CallType) atoi (item_data[0]);
  result->date = g_strdup (item_data[1]);
  result->name = g_strdup (item_data[2]);
  result->url = g_strdup (item_data[3]);
  result->duration = g_strdup (item_data[4]);
  result->end_reason = g_strdup (item_data[5]);
  result->software = g_strdup (item_data[6]);

  g_strfreev (item_data);

  return result;
}

static gchar *
gm_calls_history_item_to_string (const GmCallsHistoryItem *item)
{
  gchar *result = NULL;

  result = g_strdup_printf ("%d|%s|%s|%s|%s|%s|%s",
                            item->type,
			    item->date?item->date:"",
			    item->name?item->name:"",
			    item->url?item->url:"",
			    item->duration?item->duration:"",
			    item->end_reason?item->end_reason:"",
			    item->software?item->software:"");
  return result;
}

static void
gm_chc_destroy (gpointer data)
{
  g_return_if_fail (data);
  
  delete ((GmCallsHistoryComponent *) data);
}


static GmCallsHistoryComponent *
gm_chc_get_chc (GtkWidget *calls_history_component)
{
  g_return_val_if_fail (calls_history_component != NULL, NULL);

  return GM_calls_history_component (g_object_get_data (G_OBJECT (calls_history_component), "GMObject"));
}


static GmContact *
gm_chc_get_selected_contact (GtkWidget *calls_history_component)
{
  GmContact *contact = NULL;

  GmCallsHistoryComponent *chc = NULL;

  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  
  
  g_return_val_if_fail (calls_history_component != NULL, NULL);

  chc = gm_chc_get_chc (calls_history_component);

  g_return_val_if_fail (chc != NULL, NULL);


  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (chc->chc_tree_view));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (chc->chc_tree_view));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    contact = gmcontact_new ();

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			COLUMN_NAME, &contact->fullname,
			COLUMN_URL, &contact->url,
			-1);
  }
  

  return contact;
}


GtkWidget *
gm_chc_contact_menu_new (GtkWidget *calls_history_component)
{
  GtkWidget *menu = NULL;

  menu = gtk_menu_new ();
  
      
  static MenuEntry contact_menu [] =
    {
      GTK_MENU_ENTRY("call", _("C_all Contact"), NULL,
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (call_contact1_cb), 
		     calls_history_component, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("add", _("Add Contact to _Address Book"), NULL,
		     GTK_STOCK_ADD, 0,
		     GTK_SIGNAL_FUNC (add_contact_cb), 
		     calls_history_component, TRUE),

      GTK_MENU_END
    };
  
    
  gtk_build_menu (menu, contact_menu, NULL, NULL);

  
  return menu;
}


static void
gm_chc_update (GtkWidget *calls_history_component)
{
  GmCallsHistoryComponent *chc = NULL;
  
  GtkTreeIter iter;
  GtkListStore *list_store = NULL;

  gchar *conf_key = NULL;
  GmCallsHistoryItem *item = NULL;
  
  GSList *calls_list = NULL;
  GSList *calls_list_iter = NULL;

  GdkPixbuf *type_icon [3] = {NULL, NULL, NULL};
  
  type_icon [PLACED_CALL]= gtk_widget_render_icon (calls_history_component,
                                                   GM_STOCK_CALL_PLACED,
                                                   GTK_ICON_SIZE_MENU, NULL);
  type_icon [MISSED_CALL]= gtk_widget_render_icon (calls_history_component,
                                                   GM_STOCK_CALL_MISSED,
                                                   GTK_ICON_SIZE_MENU, NULL);
  type_icon [RECEIVED_CALL]= gtk_widget_render_icon (calls_history_component,
                                                     GM_STOCK_CALL_RECEIVED,
                                                     GTK_ICON_SIZE_MENU, NULL);

  chc = gm_chc_get_chc (calls_history_component);

  list_store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (chc->chc_tree_view)));

  conf_key = gm_chc_get_conf_key ();

  gtk_list_store_clear (list_store);

  calls_list = gm_conf_get_string_list (conf_key);

  calls_list_iter = calls_list;
  while (calls_list_iter && calls_list_iter->data) {

    item = gm_calls_history_item_new_from_string ((const char *)calls_list_iter->data);
    if (item) {

      gtk_list_store_prepend (list_store, &iter);
      gtk_list_store_set (list_store,
                          &iter,
                          COLUMN_DATE, item->date,
                          COLUMN_NAME, item->name,
                          COLUMN_URL, item->url,
                          COLUMN_DURATION, item->duration,
                          -1);
    }

    gtk_list_store_set (GTK_LIST_STORE (list_store), &iter,
                        COLUMN_TYPE, type_icon [item->type], -1);

    gm_calls_history_item_free (item);

    calls_list_iter = g_slist_next (calls_list_iter);
  }

  for (int i = 0 ; i < 3 ; i++)
    g_object_unref (type_icon [i]);

  g_free (conf_key);

  g_slist_foreach (calls_list, (GFunc) g_free, NULL);
  g_slist_free (calls_list);
}


static gchar *
gm_chc_get_conf_key ()
{
  gchar *conf_key = NULL;
  
  conf_key =
    g_strdup (USER_INTERFACE_KEY "calls_history_component/calls_history");

  return conf_key;
}


static GmContact *
dnd_get_contact (GtkWidget *widget, 
		 gpointer data)
{
  GtkWidget *chc = NULL;
  
  chc = GTK_WIDGET (data);
  
  return gm_chc_get_selected_contact (chc);
}


static void
clear_button_clicked_cb (GtkButton *widget, 
			 gpointer data)
{
  gm_calls_history_clear ();
}


static void
search_entry_changed_cb (GtkButton *widget,
                         gpointer data)
{
  GmCallsHistoryComponent *chc = NULL;

  GtkListStore *list_store = NULL;
  GtkTreeIter iter;
  const char *entry_text = NULL;
  gchar *date = NULL;
  gchar *remote_user = NULL;
  gchar *url = NULL;

  BOOL removed = FALSE;
  BOOL ok = FALSE;

  g_return_if_fail (data != NULL);

  /* Fill in the window */
  gm_chc_update (GTK_WIDGET (data));  

  chc = gm_chc_get_chc (GTK_WIDGET (data));

  g_return_if_fail (chc);


  entry_text = gtk_entry_get_text (GTK_ENTRY (chc->chc_search_entry));

  list_store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (chc->chc_tree_view)));

  if (strcmp (entry_text, "")
      && gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list_store), &iter)) {

    do {

      ok = FALSE;
      removed = FALSE;
      gtk_tree_model_get (GTK_TREE_MODEL (list_store),
			  &iter,
			  COLUMN_DATE, &date,
			  COLUMN_NAME, &remote_user,
                          COLUMN_URL, &url,
			  -1);

      if (!(PCaselessString (date).Find (entry_text) != P_MAX_INDEX
            || PCaselessString (remote_user).Find (entry_text) != P_MAX_INDEX
            || PCaselessString (url).Find (entry_text) != P_MAX_INDEX)) {
	
	ok = gtk_list_store_remove (GTK_LIST_STORE (list_store), &iter);
	removed = TRUE;
      }
      
      g_free (date);
      g_free (remote_user);
      g_free (url);

      if (!removed)
	ok = gtk_tree_model_iter_next (GTK_TREE_MODEL (list_store), &iter);
	
    } while (ok);
  }
}


static gint
contact_clicked_cb (GtkWidget *widget,
		    GdkEventButton *event,
		    gpointer data)
{
  GmCallsHistoryComponent *chc = NULL;
  GtkWidget *menu = NULL;

  GtkTreeSelection *selection = NULL;
  GtkTreePath *path = NULL;
  
  g_return_val_if_fail (data != NULL, FALSE);

  chc = gm_chc_get_chc (GTK_WIDGET (data));

  g_return_val_if_fail (chc != NULL, FALSE);

  if (event->type == GDK_BUTTON_PRESS || event->type == GDK_KEY_PRESS) {

    if (event->button == 3) {
      selection = gtk_tree_view_get_selection
	(GTK_TREE_VIEW (chc->chc_tree_view));

      if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(chc->chc_tree_view),
					(gint) event->x,
					(gint) event->y,
					&path, NULL, NULL, NULL)) {

	/* select the clicked row */
	gtk_tree_selection_unselect_all (selection);
	gtk_tree_selection_select_path (selection, path);
	gtk_tree_path_free (path);

	menu = gm_chc_contact_menu_new (GTK_WIDGET (data));

	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
			event->button, event->time);

	g_signal_connect (G_OBJECT (menu), "hide",
			  GTK_SIGNAL_FUNC (g_object_unref), (gpointer) menu);

	g_object_ref (G_OBJECT (menu));
	gtk_object_sink (GTK_OBJECT (menu));
      }
    }
  }

  return TRUE;
}


static void
call_contact1_cb (GtkWidget *widget,
		  gpointer data)
{
  GmContact *contact = NULL;

  GtkWidget *calls_history_component = NULL;

  g_return_if_fail (data != NULL);

  calls_history_component = GTK_WIDGET (data);
  
  contact = gm_chc_get_selected_contact (calls_history_component);

  if (contact) {

    GnomeMeeting::Process ()->Connect (contact->url);
    gmcontact_delete (contact);
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
add_contact_cb (GtkWidget *widget,
		gpointer data)
{
  GmContact *contact = NULL;

  GtkWidget *calls_history_component = NULL;
  GtkWidget *main_window = NULL;

  g_return_if_fail (data != NULL);

  calls_history_component = GTK_WIDGET (data);
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  
  contact = gm_chc_get_selected_contact (calls_history_component);

  if (contact) {

    gm_contacts_new_contact_dialog_run (contact, 
                                        NULL, 
                                        GTK_WINDOW (main_window));
    gmcontact_delete (contact);  
  }
}


static gint 
contact_compare_cb (gconstpointer contact,
		    gconstpointer url)
{
  GmContact *gmcontact = NULL;
  
  if (!contact || !url)
    return 1;
  
  gmcontact = GM_CONTACT (contact);
  
  if (gmcontact->url && url) {
  
    if (GMURL (gmcontact->url) == GMURL ((char *) url))
      return 0;
    else
      return 1;
  }
  else 
    return 1;
}


static void 	 
calls_history_changed_nt (gpointer id, 	 
			  GmConfEntry *entry, 	 
			  gpointer data) 	 
{ 	 
  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;

  g_return_if_fail (data != NULL);
  g_return_if_fail (gm_conf_entry_get_type (entry) == GM_CONF_LIST); 	 

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();

  gdk_threads_enter (); 	 
  gm_chc_update (GTK_WIDGET (data));
  gm_main_window_urls_history_update (main_window);
  gm_text_chat_window_urls_history_update (chat_window);
  gdk_threads_leave (); 	 
}


/* The functions */
GmCallsHistoryItem *
gm_calls_history_item_new ()
{
  return g_new0 (GmCallsHistoryItem, 1);
}


void
gm_calls_history_item_free (GmCallsHistoryItem *item)
{
  g_return_if_fail (item != NULL);

  if (item->date)
    g_free (item->date);

  if (item->name)
    g_free (item->name);

  if (item->url)
    g_free (item->url);

  if (item->duration)
    g_free (item->duration);

  if (item->end_reason)
    g_free (item->end_reason);

  if (item->software)
    g_free (item->software);

  g_free (item);
}


GmCallsHistoryItem *
gm_calls_history_item_copy (const GmCallsHistoryItem *item)
{
  GmCallsHistoryItem *result = NULL;

  if (item == NULL)
    return NULL;

  result = gm_calls_history_item_new ();

  result->type = item->type;
  
  if (item->date)
    result->date = g_strdup (item->date);

  if (item->name)
    result->name = g_strdup (item->name);

  if (item->url)
    result->url = g_strdup (item->url);

  if (item->duration)
    result->duration = g_strdup (item->duration);

  if (item->end_reason)
    result->end_reason = g_strdup (item->end_reason);

  if (item->software)
    result->software = g_strdup (item->software);

  return result;
}


GtkWidget *
gm_calls_history_component_new ()
{
  GtkWidget *vbox = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *button = NULL;
  GtkWidget *scr = NULL;
  GtkWidget *search_icon = NULL;
  
  GmCallsHistoryComponent *chc = NULL;
  
  GtkTreeViewColumn *column = NULL;
  GtkCellRenderer *renderer = NULL;

  GtkListStore *list_store = NULL;

  gchar *conf_key = NULL;
  

  /* Build the component */
  vbox = gtk_vbox_new (FALSE, 0);
  
  chc = new GmCallsHistoryComponent ();
  g_object_set_data_full (G_OBJECT (vbox), "GMObject", 
			  chc, gm_chc_destroy);


  /* The calls history list */
  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), 
                                  GTK_POLICY_NEVER,
                                  GTK_POLICY_AUTOMATIC);

  list_store = 
    gtk_list_store_new (NUM_COLUMNS_HISTORY,
                        GDK_TYPE_PIXBUF,
                        G_TYPE_STRING,
                        G_TYPE_STRING,
                        G_TYPE_STRING,
                        G_TYPE_STRING);

  chc->chc_tree_view = 
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (chc->chc_tree_view), TRUE);

  renderer = gtk_cell_renderer_pixbuf_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Type"),
                                                     renderer,
                                                     "pixbuf", 
                                                     COLUMN_TYPE,
                                                     NULL);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 100);
  gtk_tree_view_append_column (GTK_TREE_VIEW (chc->chc_tree_view), column);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Date"),
                                                     renderer,
                                                     "text", 
                                                     COLUMN_DATE,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (chc->chc_tree_view), column);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Name"),
                                                     renderer,
                                                     "text", 
                                                     COLUMN_NAME,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (chc->chc_tree_view), column);
  g_object_set (G_OBJECT (renderer), "weight", "bold", NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Duration"),
                                                     renderer,
                                                     "text", 
                                                     COLUMN_DURATION,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (chc->chc_tree_view), column);

  gtk_container_add (GTK_CONTAINER (scr), chc->chc_tree_view);
  gtk_box_pack_start (GTK_BOX (vbox), 
                      scr, TRUE, TRUE, 0);

  /* Signal to call the person on the double-clicked row */
  g_signal_connect (G_OBJECT (chc->chc_tree_view), 
                    "row_activated", 
                    G_CALLBACK (call_contact2_cb), 
                    vbox);

  /* The drag and drop information */
  gmcontacts_dnd_set_source (GTK_WIDGET (chc->chc_tree_view),
                             dnd_get_contact, vbox);

  /* Right-click on a contact */
  g_signal_connect (G_OBJECT (chc->chc_tree_view), "event_after",
                    G_CALLBACK (contact_clicked_cb), 
                    vbox);

  /* The notifier */  
  conf_key = gm_chc_get_conf_key ();
  gm_conf_notifier_add (conf_key, 
                        calls_history_changed_nt, (gpointer) vbox);

  g_free (conf_key);


  /* The hbox added below the notebook that contains the Search field,
     and the search and clear buttons */
  hbox = gtk_hbox_new (FALSE, 0);  
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
  search_icon = gtk_image_new_from_stock (GM_STOCK_SYSTEM_SEARCH,
                                          GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), search_icon, FALSE, FALSE, 2);
  chc->chc_search_entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), chc->chc_search_entry, TRUE, TRUE, 2);
  g_signal_connect (G_OBJECT (chc->chc_search_entry), "changed",
		    G_CALLBACK (search_entry_changed_cb),
		    (gpointer) vbox);  
  
  button = gtk_button_new_from_stock (GTK_STOCK_CLEAR);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (clear_button_clicked_cb), NULL);

  gtk_box_pack_start (GTK_BOX (vbox), hbox,
		      FALSE, FALSE, 0); 

  
  /* Fill in the component with old calls */
  gm_chc_update (vbox);

  
  gtk_widget_show_all (GTK_WIDGET (vbox));
  
  return vbox;
}


void
gm_calls_history_add_call (const GmCallsHistoryItem *item)
{
  gchar *conf_key = NULL;
  gchar *call_data = NULL;

  GSList *calls_list = NULL;
  
  call_data = gm_calls_history_item_to_string (item);

  conf_key = gm_chc_get_conf_key ();

  calls_list = gm_conf_get_string_list (conf_key);
  calls_list = g_slist_append (calls_list, (gpointer) call_data);

  while (g_slist_length (calls_list) > 100) 
    calls_list = g_slist_delete_link (calls_list, calls_list);
  
  gm_conf_set_string_list (conf_key, calls_list);
  
  g_free (conf_key);
  
  g_slist_foreach (calls_list, (GFunc) g_free, NULL);
  g_slist_free (calls_list);
}


void 
gm_calls_history_clear ()
{
  gchar *conf_key = NULL;

  conf_key = gm_chc_get_conf_key ();
  gm_conf_set_string_list (conf_key, NULL);
  g_free (conf_key);
}


GSList *
gm_calls_history_get_calls (int calltype,
			    int at_most,
			    gboolean unique,
			    gboolean reversed)
{
  GmContact *contact = NULL;
  GmCallsHistoryItem *item = NULL;

  GSList *calls_list = NULL;
  GSList *calls_list_iter = NULL;
  GSList *work_list = NULL;
  GSList *work_list_iter = NULL;
  GSList *result = NULL;

  gchar *conf_key = NULL;

  gboolean found = FALSE;

  conf_key = gm_chc_get_conf_key ();
  calls_list = gm_conf_get_string_list (conf_key);

  if (reversed) 
    calls_list = g_slist_reverse (calls_list);

  calls_list_iter = calls_list;
  while (calls_list_iter && calls_list_iter->data) {

    item = 
      gm_calls_history_item_new_from_string ((gchar *) calls_list_iter->data);
    if ((calltype == MAX_VALUE_CALL && at_most != -1) 
        || (calltype == item->type)) {

      contact = gmcontact_new ();

      if (item->name)
        contact->fullname = g_strdup (item->name);

      if (item->url)
        contact->url = g_strdup (item->url);

      found = (g_slist_find_custom (work_list, 
                                    (gconstpointer) contact->url,
                                    (GCompareFunc) contact_compare_cb) 
               != NULL);

      if ((unique && !found) || (!unique)) {

        work_list = g_slist_append (work_list, (gpointer) contact);
      }
      else
        gmcontact_delete (contact);
    }
    gm_calls_history_item_free (item);

    calls_list_iter = g_slist_next (calls_list_iter);
  }

  g_slist_foreach (calls_list, (GFunc) g_free, NULL);
  g_slist_free (calls_list);

  g_free (conf_key);

  
  /* #INV: work_list contains the result, with unique items or not */
  if (at_most == -1 || calltype == MAX_VALUE_CALL) // Return all values
    result = work_list;
  else { // Return the last at_most results

    if (reversed) 
      work_list = g_slist_reverse (work_list);
    work_list_iter = work_list;
    while (work_list_iter && work_list_iter->data) {

      if (g_slist_position (work_list, work_list_iter) >=
	  (int) g_slist_length (work_list) - at_most) {

	result = 
	  g_slist_append (result, 
			  (gpointer) work_list_iter->data);
      }
      else
	g_free (work_list_iter->data);

      work_list_iter = g_slist_next (work_list_iter);
    }

    if (reversed) result = 
      g_slist_reverse (result);

    g_slist_free (work_list);
  }

  return result;
}
