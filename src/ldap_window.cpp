
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
#include "history-combo.h"

#include "../pixmaps/small-close.xpm"
#include "../pixmaps/xdap-directory.xpm"


#define GM_HISTORY_LDAP_SERVERS "/apps/gnomemeeting/history/ldap_servers_list"


/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

static void row_activated (GtkTreeView *, GtkTreePath *, GtkTreeViewColumn *);
static void ldap_notebook_clicked (GtkDialog *,  GtkNotebookPage *, 
				   gint, gpointer);
static gint ldap_window_clicked (GtkWidget *, GdkEvent *, gpointer);
static void refresh_button_clicked (GtkButton *, gpointer);
static void ldap_page_close_button_clicked (GtkWidget *, gpointer);
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
 *                 to show a new notebook page.
 * BEHAVIOR     :  Changes the page and update the ils combo entry.
 * PRE          :  gpointer is a valid pointer to a GmLdapWindow.
 */
void ldap_notebook_clicked (GtkDialog *widget,  GtkNotebookPage *p,
			    gint page_num, gpointer data)
{
  GmLdapWindow *lw = (GmLdapWindow *) data;
  GtkWidget *label = NULL;
  GtkWidget *page = NULL;
  gchar *text_label = NULL;

  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook),
				    page_num);

  label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (lw->notebook), 
				      GTK_WIDGET (page));
  label = (GtkWidget *) g_object_get_data (G_OBJECT (label), "label");

  text_label = (gchar *) gtk_label_get_text (GTK_LABEL (label));
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (lw->ils_server_combo)->entry),
		      text_label);
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
 *                 on the refresh button.
 * BEHAVIOR     :  If there is no fetch results thread, calls GM_ldap_populate
 *                 in a new thread to browse the ILS directory.
 * PRE          :  gpointer is a valid pointer to a GmLdapWindow.
 */
void refresh_button_clicked (GtkButton *button, gpointer data)
{
  GmLdapWindow *lw = gnomemeeting_get_ldap_window (gm);
  GtkWidget *active_item = NULL;
  GMILSBrowser *ils_browser;

  GtkListStore *xdap_users_list_store;
  GtkWidget *page;
  GtkWidget *label;

  int page_num = 0;
  int found = 0;
  int col = 0;
  gchar *text_label = NULL, *ldap_server = NULL, 
    *search_entry_content = NULL, *entry_content = NULL,
    *search_filter = NULL;
  
  /* we make a dup, because the entry text will change */
  entry_content = g_strdup (gtk_entry_get_text 
    (GTK_ENTRY (GTK_COMBO (lw->ils_server_combo)->entry)));  


  /* if it is an empty entry_content, return */
  if ((!entry_content) || (!strcasecmp (entry_content, ""))) {

      g_free (entry_content);
      return;
  }

  /* Put the current entry in the history of the combo */
  gm_history_combo_add_entry (GM_HISTORY_COMBO (lw->ils_server_combo),
			      GM_HISTORY_LDAP_SERVERS,
			      entry_content);


  /* browse all the notebook pages */
  while ((page = 
	  gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook),
				     page_num)) != NULL) {

    label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (lw->notebook), page);
    label = (GtkWidget *) g_object_get_data (G_OBJECT (label), "label");

    text_label = (gchar *) gtk_label_get_text (GTK_LABEL (label));
    ldap_server = (gchar *) gtk_entry_get_text 
      (GTK_ENTRY (GTK_COMBO (lw->ils_server_combo)->entry));

    /* if there is a page with the current ils server, that's ok */
    if ((text_label) && (ldap_server) 
	&& (!strcasecmp (text_label, ldap_server))) {

      found = 1;
      break;
    }
      
    page_num++;
  }


  if (!found) {

    /* if it was the first "No directory" page, destroy it */
    if ((page_num == 1)&&(!strcasecmp (_("No directory"), text_label))) {

      gtk_notebook_remove_page (GTK_NOTEBOOK (lw->notebook), 0);
      page_num--;
    }

    gnomemeeting_init_ldap_window_notebook (page_num, ldap_server);
  }
  else
    gtk_notebook_set_current_page (GTK_NOTEBOOK (lw->notebook), page_num);

  page = 
    gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), 
			       gtk_notebook_get_current_page (GTK_NOTEBOOK
							      (lw->notebook)));

  xdap_users_list_store = 
    GTK_LIST_STORE (g_object_get_data (G_OBJECT (page), "list_store"));
  gtk_list_store_clear (xdap_users_list_store);


  /* Check if there is already a search running */
  ils_browser = (GMILSBrowser *) g_object_get_data (G_OBJECT (page), 
						    "GMILSBrowser");
  if (!ils_browser) {

    /* What is the active menu item? */
    active_item = gtk_menu_get_active (GTK_MENU (GTK_OPTION_MENU 
						 (lw->option_menu)->menu));

    col = g_list_index (GTK_MENU_SHELL (GTK_OPTION_MENU (lw->option_menu)
					->menu)->children, 
			active_item);

    search_entry_content = 
      (gchar *) gtk_entry_get_text (GTK_ENTRY (lw->search_entry));

    if ((search_entry_content != NULL)&&(strcmp (search_entry_content, ""))) {

      switch (col) {
	
      case 0:
	search_filter = g_strdup_printf ("(givenname=%%%s%%)", 
					 search_entry_content);
	break;

      case 1:
	search_filter = g_strdup_printf ("(surname=%%%s%%)", 
					 search_entry_content);
	break;

      case 2:
	search_filter = g_strdup_printf ("(rfc822mailbox=%%%s%%)", 
					 search_entry_content);
	break;
      }
    }


    /* Browse it */
    ils_browser = new GMILSBrowser (ldap_server, search_filter);

    /* Set the pointer to the thread as data of that GTK notebook page */
    g_object_set_data (G_OBJECT (page), "GMILSBrowser", ils_browser);

    g_free (search_filter);
  }

  g_free (entry_content);
}


/* DESCRIPTION  :  This callback is called when the user clicks the close
 *                 on any notebook tab
 * BEHAVIOR     :  remove the tab from the notebook
 * PRE          :  gpointer is a valid pointer to a GmLdapWindow
 */
static void ldap_page_close_button_clicked (GtkWidget *button, gpointer data)
{
  int cpt = 0;
  int page_number = 0;
  GmLdapWindow *lw = (GmLdapWindow *) data;
  GtkWidget *page = GTK_WIDGET (g_object_get_data (G_OBJECT (button), "page"));

  page_number = gtk_notebook_page_num (GTK_NOTEBOOK (lw->notebook), page);

  /* We do not remove the first page if it is the only one, but we
     hide the tabs in that case */
  if (!((gtk_notebook_get_current_page (GTK_NOTEBOOK (lw->notebook)) == 0)&&
	(gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), 1) == NULL)))

    gtk_notebook_remove_page (GTK_NOTEBOOK (lw->notebook), page_number);

  while (gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), cpt)) 
    cpt++;
 
  /* Do not show the tabs if it remains only one page after closing */
  if (cpt == 1)
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (lw->notebook), FALSE);
}


/* The functions */

void gnomemeeting_init_ldap_window ()
{
  GtkWidget *label;
  GtkWidget *button, *image;
  GtkWidget *vbox;
  GtkWidget *frame;
  GdkPixbuf *xdap_pixbuf;
  GtkWidget *menu;
  GtkWidget *menu_item;


  /* Get the structs from the application */
  GmWindow *gw = gnomemeeting_get_main_window (gm);
  GmLdapWindow *lw = gnomemeeting_get_ldap_window (gm);

  lw->thread_count = 0;

  xdap_pixbuf = 
    gdk_pixbuf_new_from_xpm_data ((const gchar **) xdap_directory_xpm); 

  gw->ldap_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (gw->ldap_window), 
			_("XDAP Server Browser"));
  gtk_window_set_icon (GTK_WINDOW (gw->ldap_window), xdap_pixbuf);
  gtk_window_set_position (GTK_WINDOW (gw->ldap_window), 
			   GTK_WIN_POS_CENTER);
  gtk_window_set_default_size (GTK_WINDOW (gw->ldap_window), 650, 350);
  g_object_unref (G_OBJECT (xdap_pixbuf));

  /* a vbox to put the frames, the toolbar and the user list */
  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (gw->ldap_window), vbox);


  /* The toolbar */
  GtkWidget *handle = gtk_handle_box_new ();
  GtkWidget *toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH_HORIZ);
  gtk_box_pack_start (GTK_BOX (vbox), handle, FALSE, FALSE, 0);  
  gtk_container_add (GTK_CONTAINER (handle), toolbar);
  gtk_container_set_border_width (GTK_CONTAINER (handle), 0);
  gtk_container_set_border_width (GTK_CONTAINER (toolbar), 0);


  /* Find button */
  label = gtk_label_new (_("Find users on"));
  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), GTK_WIDGET (label),
			     NULL, NULL);


  /* ILS directories combo box */
  lw->ils_server_combo = gm_history_combo_new (GM_HISTORY_LDAP_SERVERS);

  gm_history_combo_update (GM_HISTORY_COMBO (lw->ils_server_combo));

  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), 
			     GTK_WIDGET (lw->ils_server_combo),
			     NULL, NULL);
  gtk_combo_disable_activate (GTK_COMBO (lw->ils_server_combo));


  /* Text label */
  /* Translators: Please keep the 2* 2 spaces */
  label = gtk_label_new (_("  whose  "));
  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), 
			     GTK_WIDGET (label),
			     NULL, NULL);


  /* option menu */
  menu = gtk_menu_new ();
  menu_item = gtk_menu_item_new_with_label (_("first name contains"));
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  menu_item = gtk_menu_item_new_with_label (_("last name contains"));
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  menu_item = gtk_menu_item_new_with_label (_("e-mail contains"));
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  lw->option_menu = gtk_option_menu_new ();
  gtk_option_menu_set_menu (GTK_OPTION_MENU (lw->option_menu),
			    menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (lw->option_menu),
			       1);

  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), 
			     GTK_WIDGET (lw->option_menu),
			     NULL, NULL);


  /* entry */
  lw->search_entry = gtk_entry_new ();
  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), 
			     GTK_WIDGET (lw->search_entry),
			     NULL, NULL);


  /* Find button */
  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));
  button = gtk_button_new_from_stock (GTK_STOCK_REFRESH);
  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), 
			     GTK_WIDGET (button),
			     NULL, NULL);

  gtk_widget_show_all (GTK_WIDGET (toolbar));


  /* Ldap users list */
  frame = gtk_frame_new (_("ILS Users List"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

  /* We will put a GtkNotebook that will contain the ILS dir list */
  lw->notebook = gtk_notebook_new ();

  gtk_container_set_border_width (GTK_CONTAINER (lw->notebook), 
				  GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (vbox), lw->notebook, 
		      TRUE, TRUE, 0);

  gnomemeeting_init_ldap_window_notebook (0, _("No directory"));


  /* Status Bar */
  lw->statusbar = gnome_appbar_new (FALSE, TRUE, GNOME_PREFERENCES_NEVER);
  gtk_box_pack_end (GTK_BOX (vbox), lw->statusbar, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (lw->statusbar), 0);


  /* Signals */
  g_signal_connect (G_OBJECT (lw->search_entry), 
 		    "activate",
 		    G_CALLBACK (refresh_button_clicked),
 		    (gpointer) lw);

 g_signal_connect (G_OBJECT (GTK_COMBO (lw->ils_server_combo)->entry), 
 		    "activate",
 		    G_CALLBACK (refresh_button_clicked),
 		    (gpointer) lw);

  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (refresh_button_clicked), (gpointer) lw);

  g_signal_connect (G_OBJECT (lw->notebook), "switch-page",
		    G_CALLBACK (ldap_notebook_clicked), (gpointer) lw);

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
  GtkWidget *label;
  GtkWidget *scroll;
  GtkWidget *close_button;
  GtkWidget *hbox;
  GtkWidget *close_image;
  GdkPixbuf *close_cross;

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
  gtk_container_set_border_width (GTK_CONTAINER (tree_view), GNOME_PAD_SMALL);


  /* Show or not the tabs following the number of pages */
 if (page_num > 0)
   gtk_notebook_set_show_tabs (GTK_NOTEBOOK (lw->notebook), TRUE);
 else
   gtk_notebook_set_show_tabs (GTK_NOTEBOOK (lw->notebook), FALSE);


  /* The page's "label" */
  hbox = gtk_hbox_new (false, 0);
  label = gtk_label_new (text_label);

  /* Put a reference to the label in the hbox, we need it later */
  g_object_set_data (G_OBJECT (hbox), "label", (gpointer) label);

  close_button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
  close_cross = gdk_pixbuf_new_from_xpm_data (small_close_xpm);
  close_image = gtk_image_new_from_pixbuf (close_cross);
  g_object_unref (G_OBJECT (close_cross));
  gtk_container_add (GTK_CONTAINER (close_button), 
		     GTK_WIDGET (close_image));
  gtk_widget_show_all (close_button);

  gtk_box_pack_start (GTK_BOX (hbox), label, true, true, 0);
  gtk_box_pack_end (GTK_BOX (hbox), close_button, false, false, 0);

  gtk_widget_show (label);
  gtk_widget_show (close_button);
  gtk_widget_show (hbox);

  /* Put a reference to the button in the hbox, we need it later */
  g_object_set_data (G_OBJECT (hbox), "close_button", (gpointer) close_button);


  /* Append the page to the notebook */
  gtk_widget_show (tree_view);
  gtk_widget_show (scroll);
  gtk_notebook_append_page (GTK_NOTEBOOK (lw->notebook), scroll, hbox);


  g_signal_connect (G_OBJECT (close_button), "clicked",
		    G_CALLBACK (ldap_page_close_button_clicked),
		    (gpointer) lw);

  gtk_notebook_set_current_page (GTK_NOTEBOOK (lw->notebook), page_num);
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), page_num);


  /* We need this to identify the page when the user clicks the
     close button */
  g_object_set_data (G_OBJECT (close_button), "page", 
		     GINT_TO_POINTER (page));

  /* Store the list_store and the tree view as data for the page */
  g_object_set_data (G_OBJECT (page), "list_store", 
		     (gpointer) (xdap_users_list_store));
  g_object_set_data (G_OBJECT (page), "tree_view",
		     (gpointer) (tree_view));

  /* Signal to call the person on the clicked row */
  g_signal_connect (G_OBJECT (tree_view), "row_activated", 
		    G_CALLBACK (row_activated), NULL);
}
