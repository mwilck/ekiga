
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
 *                         ldap.cpp  -  description
 *                         ------------------------
 *   begin                : Wed Feb 28 2001
 *   copyright            : (C) 2000-2002 by Damien Sandras 
 *   description          : This file contains functions to build the ldap
 *                          window.
 *   email                : dsandras@seconix.com
 *                          migrax@terra.es
 *
 */

#include "../config.h"

#include "ldap_window.h"
#include "misc.h"
#include "ils.h"
#include "gnomemeeting.h"
#include "callbacks.h"
#include "menu.h"

#ifndef DISABLE_GNOME
#include <gnome.h>
#endif

#include "../pixmaps/xdap-directory.xpm"

/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

static void row_activated (GtkTreeView *, GtkTreePath *, GtkTreeViewColumn *);
static gint ldap_window_clicked (GtkWidget *, GdkEvent *, gpointer);
static void tree_selection_changed_cb (GtkTreeSelection *, gpointer);
static void gnomemeeting_init_ldap_window_notebook (int, gchar *);


/* GTK Callbacks */

/* DESCRIPTION  :  This callback is called when the user double clicks on
 *                 a row corresonding to an user.
 * BEHAVIOR     :  Add the user name in the combo box and call him.
 * PRE          :  /
 */
void row_activated (GtkTreeView *tree_view, GtkTreePath *path,
		    GtkTreeViewColumn *column) {

  GtkListStore *xdap_users_list = NULL;
  GtkTreeSelection *selection = NULL;
  GtkWidget *page = NULL;
  GConfClient *client = NULL;
  int gk_method = 0;
  PString url;

  GtkTreeIter tree_iter;

  GmLdapWindow *lw = gnomemeeting_get_ldap_window (gm);
  GmWindow *gw = gnomemeeting_get_main_window (gm);
  client = gconf_client_get_default ();

  gchar *text = NULL;
  gk_method = 
    gconf_client_get_int (client, 
			  "/apps/gnomemeeting/gatekeeper/registering_method",
			  NULL);

  page = 
    gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook),
			       gtk_notebook_get_current_page (GTK_NOTEBOOK
							      (lw->notebook)));

  
  /* Get data for the current page */
  xdap_users_list = 
    GTK_LIST_STORE (g_object_get_data (G_OBJECT (page), "list_store"));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));  
  gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), NULL,
				   &tree_iter);
  gtk_tree_model_get (GTK_TREE_MODEL (xdap_users_list), &tree_iter,
		      COLUMN_IP, &text, -1);


  /* if we are waiting for a call, add the IP
     to the history, and call that user       */
  if (MyApp->Endpoint ()->GetCallingState () == 0) {

    /* If we are registered to a gatekeeper, we add the "@" */
    if (gk_method)
      url = "callto://@" + PString (text);
    else
      url = "callto://" + PString (text);

    /* this function will store a copy of text */
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry), url);
      
    connect_cb (NULL, NULL);
  }
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 closes the window (destroy or delete_event signals).
 * BEHAVIOR     :  Hide the window.
 * PRE          :  gpointer is a valid pointer to a GmLdapWindow.
 */
gint ldap_window_clicked (GtkWidget *widget, GdkEvent *ev, gpointer data)
{
  GmWindow *gw = (GmWindow *) data;

  if (gw->ldap_window)
    if (GTK_WIDGET_VISIBLE (gw->ldap_window))
      gtk_widget_hide_all (gw->ldap_window);

  return TRUE;
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a server in the tree_store.
 * BEHAVIOR     :  Selects the corresponding GtkNotebook page.
 * PRE          :  /
 */
static void
tree_selection_changed_cb (GtkTreeSelection *selection, gpointer data)
{
  char *name = NULL;
  int page_num = -1;
  GtkTreeIter iter;
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;

  GmWindow *gw = NULL;
  GmLdapWindow *lw = NULL;

  gw = gnomemeeting_get_main_window (gm);
  lw = gnomemeeting_get_ldap_window (gm);

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    path = gtk_tree_model_get_path  (model, &iter);

    if (path) {

      if (gtk_tree_path_get_depth (path) >= 2) {

	if (gtk_tree_path_get_indices (path) [0] == 0)
	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 0, &name,
			      1, &name, 2, &page_num, -1);

	if (page_num != - 1) 
	  gtk_notebook_set_current_page (GTK_NOTEBOOK (lw->notebook), 
					 page_num);
      }

      gtk_tree_path_free (path);
    }
  }
}


/* DESCRIPTION  :  This callback is called when the user activates 
 *                 (double click) a server in the tree_store.
 * BEHAVIOR     :  Browse the selected server.
 * PRE          :  /
 */
void contacts_tree_view_row_activated_cb (GtkTreeView *tree_view, 
					  GtkTreePath *path,
					  GtkTreeViewColumn *column) 
{
  int page_num = -1;
  gchar *name = NULL;

  GtkTreeIter iter;
  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkListStore *xdap_users_list_store = NULL;

  GtkWidget *page = NULL;

  GmWindow *gw = NULL;
  GmLdapWindow *lw = NULL;

  GMILSBrowser *ils_browser = NULL;

  gw = gnomemeeting_get_main_window (gm);
  lw = gnomemeeting_get_ldap_window (gm);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    if (path) {

      if (gtk_tree_path_get_depth (path) >= 2) {

	if (gtk_tree_path_get_indices (path) [0] == 0) {
      
	  /* Get the server name */
	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 0, &name, 
			      1, &name, 2, &page_num, -1);
	}
      }
    }

    if (page_num != - 1) {
    
      gtk_notebook_set_current_page (GTK_NOTEBOOK (lw->notebook), page_num);
      page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), 
					page_num);
 
      xdap_users_list_store = 
	GTK_LIST_STORE (g_object_get_data (G_OBJECT (page), "list_store"));
      gtk_list_store_clear (xdap_users_list_store);

      /* Check if there is already a search running */
      ils_browser = (GMILSBrowser *) g_object_get_data (G_OBJECT (page), 
							"GMILSBrowser");
      if (!ils_browser && page_num != -1) {
	
	/* Browse it */
	ils_browser = new GMILSBrowser (name, "");

	/* Set the pointer to the thread as data of that GTK notebook page */
	g_object_set_data (G_OBJECT (page), "GMILSBrowser", ils_browser);
      }
    }
  }
}



static gint
contacts_tree_view_event_after_callback (GtkWidget *w, GdkEventButton *e,
					 gpointer data)
{
  GtkWidget *menu = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeView *tree_view = NULL;
  GtkTreePath *path = NULL;

  tree_view = GTK_TREE_VIEW (w);

  if (e->window != gtk_tree_view_get_bin_window (tree_view)) 
    return FALSE;
    

  if (e->type == GDK_BUTTON_PRESS) {

    if (gtk_tree_view_get_path_at_pos (tree_view, (int) e->x, (int) e->y,
				       &path, NULL, NULL, NULL)) {

      selection = gtk_tree_view_get_selection (tree_view);

      if (e->button == 3 && 
	  gtk_tree_selection_path_is_selected (selection, path)) {
	
	menu = gtk_menu_new ();
	
	MenuEntry delete_server_menu [] =
	  {
	    {_("Delete"), NULL,
	     NULL, 0, MENU_ENTRY, 
	     GTK_SIGNAL_FUNC (NULL), 
	     NULL, NULL},
	    {NULL, NULL, NULL, 0, MENU_END, NULL, NULL, NULL}
	  };

	MenuEntry new_server_menu [] =
	  {
	    {_("New"), NULL,
	     NULL, 0, MENU_ENTRY, 
	     GTK_SIGNAL_FUNC (NULL), 
	     NULL, NULL},
	    {NULL, NULL, NULL, 0, MENU_END, NULL, NULL, NULL}
	  };
	
	if (gtk_tree_path_get_depth (path) >= 2)
	  gnomemeeting_build_menu (menu, delete_server_menu, NULL);
	else
	  gnomemeeting_build_menu (menu, new_server_menu, NULL);

	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
			e->button, e->time);
	g_signal_connect (G_OBJECT (menu), "hide",
			  GTK_SIGNAL_FUNC (g_object_unref), (gpointer) menu);
	g_object_ref (G_OBJECT (menu));
	gtk_object_sink (GTK_OBJECT (menu));
	
	gtk_tree_path_free (path);

	return TRUE;
      }
    }
  }

  return FALSE;
}


/* The functions */
void gnomemeeting_init_ldap_window ()
{
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *frame = NULL;
  GdkPixbuf *xdap_pixbuf = NULL;

  GtkCellRenderer *cell = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkTreeStore *model = NULL;
  GtkTreeIter iter;
  GtkTreeIter child_iter;

  GSList *ldap_servers_list = NULL;
  GSList *ldap_servers_list_iter = NULL;

  int cpt = 0;

  GmWindow *gw = NULL;
  GmLdapWindow *lw = NULL;

  GConfClient *client = NULL;

  /* Get the structs from the application */
  gw = gnomemeeting_get_main_window (gm);
  lw = gnomemeeting_get_ldap_window (gm);
  client = gconf_client_get_default ();

  xdap_pixbuf = 
    gdk_pixbuf_new_from_xpm_data ((const gchar **) xdap_directory_xpm); 

  gw->ldap_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (gw->ldap_window), 
			_("XDAP Server Browser"));
  gtk_window_set_icon (GTK_WINDOW (gw->ldap_window), xdap_pixbuf);
  gtk_window_set_position (GTK_WINDOW (gw->ldap_window), 
			   GTK_WIN_POS_CENTER);
  gtk_window_set_default_size (GTK_WINDOW (gw->ldap_window), 550, 270);
  g_object_unref (G_OBJECT (xdap_pixbuf));


  /* A hbox to put the tree and the ldap browser */
  hbox = gtk_hbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_container_add (GTK_CONTAINER (gw->ldap_window), hbox);
  

  /* The Tree view that will store the contacts */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
  model = gtk_tree_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
  lw->tree_view = gtk_tree_view_new ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (lw->tree_view), 
			   GTK_TREE_MODEL (model));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (lw->tree_view));
  gtk_container_add (GTK_CONTAINER (frame), lw->tree_view);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (lw->tree_view), FALSE);

  gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection),
			       GTK_SELECTION_BROWSE);

  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter, 0, _("Servers"), 1, _("Servers"), 2, 0, -1);

  ldap_servers_list =
    gconf_client_get_list (client, CONTACTS_SERVERS_KEY "ldap_servers_list",
			   GCONF_VALUE_STRING, NULL); 
    

  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Contacts"),
						     cell, "text", 0, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (lw->tree_view),
			       GTK_TREE_VIEW_COLUMN (column));


  /* a vbox to put the frames, the toolbar and the user list */
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

  /* Ldap users list */
  frame = gtk_frame_new (_("ILS Users List"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);

  /* We will put a GtkNotebook that will contain the ILS dir list */
  lw->notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (lw->notebook), 0);
  gtk_box_pack_start (GTK_BOX (vbox), lw->notebook, 
		      TRUE, TRUE, 0);

  /* Populate the GtkTreeStore and create the corresponding notebook 
     pages */
  ldap_servers_list_iter = ldap_servers_list;
  while (ldap_servers_list_iter) {

    gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (model),
			&child_iter, 
			0, ldap_servers_list_iter->data, 
			1, ldap_servers_list_iter->data, 
			2, cpt, -1);

    gnomemeeting_init_ldap_window_notebook (cpt, (char *)
					   ldap_servers_list_iter->data);
    ldap_servers_list_iter = ldap_servers_list_iter->next;
    cpt++;
  }
  g_slist_free (ldap_servers_list);

  gtk_tree_view_expand_all (GTK_TREE_VIEW (lw->tree_view));
  g_signal_connect (G_OBJECT (selection), "changed",
		    G_CALLBACK (tree_selection_changed_cb), NULL);
  g_signal_connect (G_OBJECT (lw->tree_view), "row_activated",
		    G_CALLBACK (contacts_tree_view_row_activated_cb), NULL);  
  g_signal_connect_object (G_OBJECT (lw->tree_view), "event-after",
			   G_CALLBACK (contacts_tree_view_event_after_callback), NULL, (enum GConnectFlags) 0);

  /* The toolbar */
  // GtkWidget *handle = gtk_handle_box_new ();
//   GtkWidget *toolbar = gtk_toolbar_new ();
//   gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH_HORIZ);
//   gtk_box_pack_start (GTK_BOX (vbox), handle, FALSE, FALSE, 0);  
//   gtk_container_add (GTK_CONTAINER (handle), toolbar);
//   gtk_container_set_border_width (GTK_CONTAINER (handle), 0);
//   gtk_container_set_border_width (GTK_CONTAINER (toolbar), 0);


//   /* Find button */
//   label = gtk_label_new (_("Find users on"));
//   gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), GTK_WIDGET (label),
// 			     NULL, NULL);


//   /* ILS directories combo box */
//   lw->ils_server_combo = gm_history_combo_new (GM_HISTORY_LDAP_SERVERS);

//   gm_history_combo_update (GM_HISTORY_COMBO (lw->ils_server_combo));

//   gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), 
// 			     GTK_WIDGET (lw->ils_server_combo),
// 			     NULL, NULL);
//   gtk_combo_disable_activate (GTK_COMBO (lw->ils_server_combo));


//   /* Text label */
//   /* Translators: Please keep the 2* 2 spaces */
//   label = gtk_label_new (_("  whose  "));
//   gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), 
// 			     GTK_WIDGET (label),
// 			     NULL, NULL);


//   /* option menu */
//   menu = gtk_menu_new ();
//   menu_item = gtk_menu_item_new_with_label (_("first name contains"));
//   gtk_widget_show (menu_item);
//   gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

//   menu_item = gtk_menu_item_new_with_label (_("last name contains"));
//   gtk_widget_show (menu_item);
//   gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

//   menu_item = gtk_menu_item_new_with_label (_("e-mail contains"));
//   gtk_widget_show (menu_item);
//   gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

//   lw->option_menu = gtk_option_menu_new ();
//   gtk_option_menu_set_menu (GTK_OPTION_MENU (lw->option_menu),
// 			    menu);
//   gtk_option_menu_set_history (GTK_OPTION_MENU (lw->option_menu),
// 			       1);

//   gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), 
// 			     GTK_WIDGET (lw->option_menu),
// 			     NULL, NULL);


//   /* entry */
//   lw->search_entry = gtk_entry_new ();
//   gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), 
// 			     GTK_WIDGET (lw->search_entry),
// 			     NULL, NULL);


//   /* Find button */
//   gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));
//   button = gtk_button_new_from_stock (GTK_STOCK_REFRESH);
//   gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), 
// 			     GTK_WIDGET (button),
// 			     NULL, NULL);

//   gtk_widget_show_all (GTK_WIDGET (toolbar));

  /* Signals */
 //  g_signal_connect (G_OBJECT (lw->search_entry), 
//  		    "activate",
//  		    G_CALLBACK (refresh_button_clicked),
//  		    (gpointer) lw);

//  g_signal_connect (G_OBJECT (GTK_COMBO (lw->ils_server_combo)->entry), 
//  		    "activate",
//  		    G_CALLBACK (refresh_button_clicked),
//  		    (gpointer) lw);

//   g_signal_connect (G_OBJECT (button), "clicked",
// 		    G_CALLBACK (refresh_button_clicked), (gpointer) lw);

//   g_signal_connect (G_OBJECT (lw->notebook), "switch-page",
// 		    G_CALLBACK (ldap_notebook_clicked), (gpointer) lw);

  g_signal_connect (G_OBJECT (gw->ldap_window), "delete_event",
		    G_CALLBACK (ldap_window_clicked), (gpointer) gw);
}


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Build the notebook inside the LDAP window.
 * PRE          :  The current page and the server name.
 */
void gnomemeeting_init_ldap_window_notebook (int page_num, gchar *text_label)
{
  GtkWidget *page;
  GtkWidget *scroll;
  GtkWidget *vbox;
  GtkWidget *statusbar;

  /* For the GTK TreeView */
  GtkWidget *tree_view;
  GtkListStore *xdap_users_list_store;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;

  GmLdapWindow *lw = gnomemeeting_get_ldap_window (gm);
  
  xdap_users_list_store = gtk_list_store_new (NUM_COLUMNS,
					      GDK_TYPE_PIXBUF,
					      G_TYPE_BOOLEAN,
					      G_TYPE_BOOLEAN,
					      G_TYPE_STRING,
					      G_TYPE_STRING,
					      G_TYPE_STRING,
					      G_TYPE_STRING,
					      G_TYPE_STRING,
					      G_TYPE_STRING,
					      G_TYPE_STRING,
					      G_TYPE_STRING);

  vbox = gtk_vbox_new (FALSE, 2);
  scroll = gtk_scrolled_window_new (NULL, NULL);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), 
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);

  tree_view = 
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (xdap_users_list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree_view), TRUE);
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (tree_view),
				   COLUMN_FIRSTNAME);

  /* Set all Colums */
  renderer = gtk_cell_renderer_pixbuf_new ();
  /* Translators: This is "S" as in "Status" */
  column = gtk_tree_view_column_new_with_attributes (_("S"),
						     renderer,
						     "pixbuf", 
						     COLUMN_STATUS,
						     NULL);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  renderer = gtk_cell_renderer_toggle_new ();
  /* Translators: This is "A" as in "Audio" */
  column = gtk_tree_view_column_new_with_attributes (_("A"),
						     renderer,
						     "active", 
						     COLUMN_AUDIO,
						     NULL);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  renderer = gtk_cell_renderer_toggle_new ();
  /* Translators: This is "V" as in "Video" */
  column = gtk_tree_view_column_new_with_attributes (_("V"),
						     renderer,
						     "active", 
						     COLUMN_VIDEO,
						     NULL);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("First Name"),
						     renderer,
						     "text", 
						     COLUMN_FIRSTNAME,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_FIRSTNAME);
  gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
				      COLUMN_COLOR);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Last Name"),
						     renderer,
						     "text", 
						     COLUMN_LASTNAME,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_LASTNAME);
  gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
				      COLUMN_COLOR);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("E-Mail"),
						     renderer,
						     "text", 
						     COLUMN_EMAIL,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_EMAIL);
  gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
				      COLUMN_COLOR);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Location"),
						     renderer,
						     "text", 
						     COLUMN_LOCATION,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_LOCATION);
  gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
				      COLUMN_COLOR);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Comment"),
						     renderer,
						     "text", 
						     COLUMN_COMMENT,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_COMMENT);
  gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
				      COLUMN_COLOR);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Version"),
						     renderer,
						     "text", 
						     COLUMN_VERSION,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_VERSION);
  gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
				      COLUMN_COLOR);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);


  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("IP"),
						     renderer,
						     "text", 
						     COLUMN_IP,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_IP);
  gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
				      COLUMN_COLOR);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  gtk_container_add (GTK_CONTAINER (scroll), tree_view);
  gtk_container_set_border_width (GTK_CONTAINER (tree_view), 0);

  statusbar = gtk_statusbar_new ();
  gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), statusbar, FALSE, FALSE, 0);

  gtk_notebook_append_page (GTK_NOTEBOOK (lw->notebook), vbox, NULL);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (lw->notebook), FALSE);

  gtk_notebook_set_current_page (GTK_NOTEBOOK (lw->notebook), page_num);
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), page_num);


  /* Store the list_store and the tree view as data for the page */
  g_object_set_data (G_OBJECT (page), "list_store", 
		     (gpointer) (xdap_users_list_store));
  g_object_set_data (G_OBJECT (page), "tree_view",
		     (gpointer) (tree_view));
  g_object_set_data (G_OBJECT (page), "statusbar", 
		     (gpointer) (statusbar));
  g_object_set_data (G_OBJECT (page), "server_name",
		     (gpointer) (text_label));

  /* Signal to call the person on the clicked row */
  g_signal_connect (G_OBJECT (tree_view), "row_activated", 
		    G_CALLBACK (row_activated), NULL);
}
