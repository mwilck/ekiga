
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
 *                         addressbook_window.cpp  -  description
 *                         ---------------------------------------
 *   begin                : Wed Feb 28 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras 
 *   description          : This file contains functions to build the 
 *                          addressbook window.
 *
 */

#include "../config.h"

#include "addressbook_window.h"
#include "callbacks.h"
#include "gnomemeeting.h"

#include "stock-icons.h"
#include <contacts/gm_contacts.h>
#include "gm_conf.h"
#include "gtk_menu_extensions.h"




struct GmAddressbookWindow_ {

  GtkWidget *aw_menu;		/* The main menu of the window */
  GtkWidget *aw_tree_view;      /* The GtkTreeView that contains the address 
				   books list */
  GtkWidget *aw_notebook;       /* The GtkNotebook that contains the different
				   listings for each of the address books */
  GtkWidget *aw_option_menu;    /* The option menu for the search */
  GtkWidget *aw_search_entry;   /* The search entry */ 
};


struct GmAddressbookWindowPage_ {

  GtkWidget *awp_tree_view; /* The GtkTreeView that contains the users list */
};


typedef struct GmAddressbookWindow_ GmAddressbookWindow;
typedef struct GmAddressbookWindowPage_ GmAddressbookWindowPage;


#define GM_ADDRESSBOOK_WINDOW(x) (GmAddressbookWindow *) (x)
#define GM_ADDRESSBOOK_WINDOW_PAGE(x) (GmAddressbookWindowPage *) (x)


/* The different cell renderers for the different contacts sections (servers
   or groups */
enum {

  COLUMN_PIXBUF,
  COLUMN_NAME,
  COLUMN_NOTEBOOK_PAGE,
  COLUMN_PIXBUF_VISIBLE,
  COLUMN_WEIGHT,
  COLUMN_URL,
  COLUMN_AID,
  COLUMN_CALL_ATTRIBUTE,
  NUM_COLUMNS_CONTACTS
};

enum {

  COLUMN_STATUS,
  COLUMN_FULLNAME,
  COLUMN_UURL,
  COLUMN_CATEGORIES,
  COLUMN_SPEED_DIAL,
  COLUMN_LOCATION,
  COLUMN_COMMENT,
  COLUMN_SOFTWARE,
  COLUMN_EMAIL,
  COLUMN_UUID,
  NUM_COLUMNS_GROUPS
};


/* Declarations */


/* GUI functions */

/* DESCRIPTION  : / 
 * BEHAVIOR     : Frees a GmAddressbookWindowPage and its content.
 * PRE          : A non-NULL pointer to a GmAddressbookWindowPage.
 */
static void gm_awp_destroy (gpointer);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Frees a GmAddressbookWindow and its content.
 * PRE          : A non-NULL pointer to a GmAddressbookWindow.
 */
static void gm_aw_destroy (gpointer);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to the private GmAddressbookWindow
 * 		  used by the address book GMObject.
 * PRE          : The given GtkWidget pointer must be an address book GMObject.
 */
static GmAddressbookWindow *gm_aw_get_aw (GtkWidget *);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to the private GmAddressbookWindowPage
 * 		  used by any page of the internal GtkNotebook of the 
 * 		  address book GMObject.
 * PRE          : The given GtkWidget pointer must point to a page
 * 		  of the internal GtkNotebook of the address book GMObject.
 */
static GmAddressbookWindowPage *gm_aw_get_awp (GtkWidget *);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to a newly allocated GmContact with
 * 		  all the info for the contact currently being selected
 * 		  in the address book window given as argument. NULL if none
 * 		  is selected.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject.
 */
static GmContact *gm_aw_get_selected_contact (GtkWidget *);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to a newly allocated GmAddressbook with
 * 		  all the info for the address book currently being selected
 * 		  in the address book window given as argument. NULL if none
 * 		  is selected.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject.
 */
static GmAddressbook *gm_aw_get_selected_addressbook (GtkWidget *);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Adds the given GmAddressbook to the address book window
 * 		  GMObject.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject. Non-NULL pointer to a GmAddressbook.
 */
static void gm_aw_add_addressbook (GtkWidget *, 
				   GmAddressbook *);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Deletes the given GmAddressbook to the address book window
 * 		  GMObject.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject. Non-NULL pointer to a GmAddressbook.
 */
static void gm_aw_delete_addressbook (GtkWidget *,
				      GmAddressbook *);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Modifies the given GmAddressbook to the address book window
 * 		  GMObject.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject. Non-NULL pointer to a GmAddressbook.
 */

static void gm_aw_modify_addressbook (GtkWidget *, 
				      GmAddressbook *); 


/* DESCRIPTION  : / 
 * BEHAVIOR     : Updates the content of the given GmAddressbook in the 
 * 		  address book window GMObject.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject. Non-NULL pointer to a GmAddressbook.
 */
static void gm_aw_update_addressbook (GtkWidget *, 
				      GmAddressbook *);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Updates the content of the given GmAddressbook in the 
 * 		  address book window GMObject.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject. The first boolean must indicate if a local 
 * 		  addressbook is selected, the second one if a remote 
 * 		  addressbook is selected, both may not be true at the same 
 * 		  time. All other situations are possible.
 */
static void gm_aw_update_menu_sensitivity (GtkWidget *,
					   gboolean,
					   gboolean);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns the GtkNotebook page containing the content of the
 * 		  given GmAddressbook.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject. The second argument must point to a valid 
 * 		  GmAddressbook. Both should be non-NULL.
 */
static gint gm_aw_get_notebook_page (GtkWidget *,
				     GmAddressbook *);


/* Callbacks */

/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when something is selected in the 
 * 		  aw_tree_view of the GmAddressbookWindow. It returns TRUE or
 * 		  FALSE following the selected item is an address book or a
 * 		  category of address book.
 * PRE          : /
 */
static gboolean aw_tree_selection_function_cb (GtkTreeSelection *,
					       GtkTreeModel *,
					       GtkTreePath *,
					       gboolean,
					       gpointer);

/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when a contact is double-clicked
 * 		  in the address book GMObject. It is called.
 * PRE          : /
 */
static void call_contact_cb (GtkTreeView *,
			     GtkTreePath *,
			     GtkTreeViewColumn *,
			     gpointer);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user chooses to add
 * 		  a contact. The address book dialog permitting to add or
 * 		  edit a contact is presented with empty fields.
 * PRE          : The gpointer must point to the address book window. 
 */
static void new_contact_cb (GtkWidget *,
			    gpointer);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user chooses to add
 * 		  an addressbook. The address book edition dialog is presented
 * 		  to the user.
 * PRE          : The gpointer must point to the address book window. 
 */
static void new_addressbook_cb (GtkWidget *w,
				gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user chooses to delete
 * 		  something. If a contact is selected, presents the dialog
 * 		  to delete a contact and delete it if required. If no contact
 * 		  is selected but an address book, present the dialog to delete
 * 		  an addressbook, and delete it if required.
 * PRE          : The gpointer must point to the address book window. 
 */
static void delete_cb (GtkWidget *,
		       gpointer);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user chooses to edit the
 * 		  properties of something. If a contact is selected, 
 * 		  presents the dialog to edit a contact. If no contact
 * 		  is selected but an address book, present the dialog to edit 
 * 		  an addressbook, and delete it if required.
 * PRE          : The gpointer must point to the address book window. 
 */
static void properties_cb (GtkWidget *,
			   gpointer);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user chooses to search
 * 		  the content of an address book. It launches the search
 * 		  for the selected fields and updates the content of the GUI
 * 		  once the search is over.
 * PRE          : The gpointer must point to the address book window. 
 */
static void search_addressbook_cb (GtkWidget *,
				   gpointer);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user clicks on a contact.
 * 		  It updates the menu sensitivity of the GUI following what
 * 		  is selected.
 * PRE          : /
 */
static void contact_selected_cb (GtkTreeSelection *,
				 gpointer);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user clicks on an address
 * 		  book. It unselects all contacts and updates the menu 
 * 		  sensitivity of the GUI accordingly.
 * PRE          : /
 */
static void addressbook_selected_cb (GtkTreeSelection *,
				     gpointer);



/* Implementation */
static void
gm_awp_destroy (gpointer awp)
{
  g_return_if_fail (awp != NULL);

  delete ((GmAddressbookWindowPage *) awp);
}


static void
gm_aw_destroy (gpointer aw)
{
  g_return_if_fail (aw != NULL);

  delete ((GmAddressbookWindow *) aw);
}


static GmAddressbookWindow *
gm_aw_get_aw (GtkWidget *addressbook_window)
{
  g_return_val_if_fail (addressbook_window != NULL, NULL);

  return GM_ADDRESSBOOK_WINDOW (g_object_get_data (G_OBJECT (addressbook_window), "GMObject"));
}


static GmAddressbookWindowPage *
gm_aw_get_awp (GtkWidget *p)
{
  g_return_val_if_fail (p != NULL, NULL);

  return GM_ADDRESSBOOK_WINDOW_PAGE (g_object_get_data (G_OBJECT (p), "GMObject"));
}


static GmContact *
gm_aw_get_selected_contact (GtkWidget *addressbook)
{
  GmAddressbookWindow *aw = NULL;
  GmAddressbookWindowPage *awp = NULL;

  GmContact *contact = NULL;

  GtkWidget *page = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;

  int page_num = 0;


  /* Get the required data from the GtkNotebook page */
  aw = gm_aw_get_aw (addressbook);
  page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (aw->aw_notebook));
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (aw->aw_notebook), page_num);
  awp = gm_aw_get_awp (page);

  g_return_val_if_fail (awp != NULL, NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (awp->awp_tree_view));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (awp->awp_tree_view));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    contact = gm_contact_new ();

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			COLUMN_FULLNAME, &contact->fullname, 
			COLUMN_SPEED_DIAL, &contact->speeddial,
			COLUMN_CATEGORIES, &contact->categories,
			COLUMN_UURL, &contact->url,
			COLUMN_UUID, &contact->uid,
			-1);
  }


  return contact;
}


static GmAddressbook *
gm_aw_get_selected_addressbook (GtkWidget *addressbook)
{
  GmAddressbookWindow *aw = NULL;
  GmAddressbook *abook = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;

  /* Get the required data from the GtkNotebook page */
  aw = gm_aw_get_aw (addressbook);

  g_return_val_if_fail (aw != NULL, NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (aw->aw_tree_view));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->aw_tree_view));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    abook = gm_addressbook_new ();

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			COLUMN_NAME, &abook->name, 
			COLUMN_URL, &abook->url,
			COLUMN_AID, &abook->aid,
			COLUMN_CALL_ATTRIBUTE, &abook->call_attribute,
			-1); 
  }

  return abook;
}


static void
gm_aw_add_addressbook (GtkWidget *addressbook_window,
		       GmAddressbook *addressbook)
{
  GmAddressbookWindow *aw = NULL;
  GmAddressbookWindowPage *awp = NULL;

  GtkWidget *page = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *scroll = NULL;

  GdkPixbuf *contact_icon = NULL;

  GtkTreeViewColumn *column = NULL;
  GtkListStore *list_store = NULL;
  GtkCellRenderer *renderer = NULL;

  GtkTreeModel *aw_tree_model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter, child_iter;

  gboolean is_local = FALSE;
  int pos = 0;

  g_return_if_fail (addressbook_window != NULL);
  g_return_if_fail (addressbook != NULL);


  /* Get the Data */
  aw = gm_aw_get_aw (addressbook_window);
  awp = new GmAddressbookWindowPage ();


  /* Add the given address book in the aw_tree_view GtkTreeView listing
   * all address books */
  contact_icon = 
    gtk_widget_render_icon (aw->aw_tree_view, 
			    GM_STOCK_LOCAL_CONTACT,
			    GTK_ICON_SIZE_MENU, NULL);

  aw_tree_model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->aw_tree_view));
  pos = gtk_notebook_get_n_pages (GTK_NOTEBOOK (aw->aw_notebook));

  if (gnomemeeting_addressbook_is_local (addressbook))
    is_local = TRUE;

  if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (aw_tree_model), 
					   &iter, is_local ? "1" : "0")) {

    gtk_tree_store_append (GTK_TREE_STORE (aw_tree_model), &child_iter, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (aw_tree_model),
			&child_iter, 
			COLUMN_PIXBUF, contact_icon,
			COLUMN_NAME, addressbook->name,
			COLUMN_NOTEBOOK_PAGE, pos, 
			COLUMN_PIXBUF_VISIBLE, TRUE,
			COLUMN_WEIGHT, PANGO_WEIGHT_NORMAL, 
			COLUMN_URL, addressbook->url, 
			COLUMN_AID, addressbook->aid, 
			COLUMN_CALL_ATTRIBUTE, addressbook->call_attribute, 
			-1);
  }

  gtk_tree_view_expand_all (GTK_TREE_VIEW (aw->aw_tree_view));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (aw->aw_tree_view));
  if (!gtk_tree_selection_get_selected (selection, NULL, NULL))
    gtk_tree_selection_select_iter (selection, &child_iter);


  /* Add the given address book in the aw_notebook GtkNotebook containing
   * the content of all address books */
  list_store = 
    gtk_list_store_new (NUM_COLUMNS_GROUPS, 
			GDK_TYPE_PIXBUF,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_STRING, 
			G_TYPE_STRING);

  vbox = gtk_vbox_new (FALSE, 0);
  scroll = gtk_scrolled_window_new (NULL, NULL);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), 
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);

  gtk_notebook_append_page (GTK_NOTEBOOK (aw->aw_notebook), vbox, NULL);
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (aw->aw_notebook), pos);
  g_object_set_data_full (G_OBJECT (page), "GMObject", 
			  awp, (GDestroyNotify) gm_awp_destroy);

  awp->awp_tree_view = 
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (awp->awp_tree_view), TRUE);


  renderer = gtk_cell_renderer_pixbuf_new ();
  /* Translators: This is "S" as in "Status" */
  column = gtk_tree_view_column_new_with_attributes (_("S"),
						     renderer,
						     "pixbuf", 
						     COLUMN_STATUS,
						     NULL);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 150);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);
  if (gnomemeeting_addressbook_is_local (addressbook))
    g_object_set (G_OBJECT (column), "visible", false, NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Name"),
						     renderer,
						     "text", 
						     COLUMN_FULLNAME,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_FULLNAME);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_column_set_min_width (GTK_TREE_VIEW_COLUMN (column), 125);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);
  g_object_set (G_OBJECT (renderer), "weight", "bold", NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("URL"),
						     renderer,
						     "text", 
						     COLUMN_UURL,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_UURL);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_column_set_min_width (GTK_TREE_VIEW_COLUMN (column), 280);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);
  g_object_set (G_OBJECT (renderer), "foreground", "blue",
		"underline", TRUE, NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Comment"),
						     renderer,
						     "text", 
						     COLUMN_COMMENT,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_COMMENT);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_column_set_min_width (GTK_TREE_VIEW_COLUMN (column), 280);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);
  if (gnomemeeting_addressbook_is_local (addressbook))
    g_object_set (G_OBJECT (column), "visible", false, NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Software"),
						     renderer,
						     "text", 
						     COLUMN_SOFTWARE,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_SOFTWARE);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_column_set_min_width (GTK_TREE_VIEW_COLUMN (column), 280);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);
  if (gnomemeeting_addressbook_is_local (addressbook))
    g_object_set (G_OBJECT (column), "visible", false, NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("E-Mail"),
						     renderer,
						     "text", 
						     COLUMN_EMAIL,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_EMAIL);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_column_set_min_width (GTK_TREE_VIEW_COLUMN (column), 280);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Location"),
						     renderer,
						     "text", 
						     COLUMN_LOCATION,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_LOCATION);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_column_set_min_width (GTK_TREE_VIEW_COLUMN (column), 280);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);
  if (gnomemeeting_addressbook_is_local (addressbook))
    g_object_set (G_OBJECT (column), "visible", false, NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Categories"),
						     renderer,
						     "text", 
						     COLUMN_CATEGORIES,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_CATEGORIES);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_column_set_min_width (GTK_TREE_VIEW_COLUMN (column), 280);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);
  if (!gnomemeeting_addressbook_is_local (addressbook))
    g_object_set (G_OBJECT (column), "visible", false, NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Speed Dial"),
						     renderer,
						     "text", 
						     COLUMN_SPEED_DIAL,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_SPEED_DIAL);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);
  if (!gnomemeeting_addressbook_is_local (addressbook))
    g_object_set (G_OBJECT (column), "visible", false, NULL);


  /* Add the tree view */
  gtk_container_add (GTK_CONTAINER (scroll), awp->awp_tree_view);
  gtk_container_set_border_width (GTK_CONTAINER (awp->awp_tree_view), 0);
  gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);
  gtk_widget_show_all (page);


  /* Connect the signals */
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (awp->awp_tree_view));
  g_signal_connect (G_OBJECT (selection), "changed",
		    G_CALLBACK (contact_selected_cb), 
		    addressbook_window);

  g_signal_connect (G_OBJECT (awp->awp_tree_view), "row-activated",
		    G_CALLBACK (call_contact_cb), 
		    addressbook_window); 


  /* Update the address book content in the GUI */
  if (gnomemeeting_addressbook_is_local (addressbook)) 
    gm_aw_update_addressbook (addressbook_window,
			      addressbook);
}


static void
gm_aw_delete_addressbook (GtkWidget *addressbook_window,
			  GmAddressbook *addressbook)
{
  GmAddressbookWindow *aw = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gchar *test = NULL;

  int p = -1;

  aw = gm_aw_get_aw (addressbook_window);

  g_return_if_fail (addressbook_window && addressbook && aw);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->aw_tree_view));

  for (int i = 0 ; i < 2 ; i++) {

    if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), 
					     &iter, (i == 0) ? "0:0" : "1:0")) {

      do {

	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			    COLUMN_AID, &test, 
			    COLUMN_NOTEBOOK_PAGE, &p,
			    -1);

	if (test && addressbook->aid && !strcmp (test, addressbook->aid)) {

	  gtk_notebook_remove_page (GTK_NOTEBOOK (aw->aw_notebook), p);
	  gtk_tree_store_remove (GTK_TREE_STORE (model), &iter);
	  g_free (test);
	  break;
	}
	g_free (test);

      } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
    }
  }
}


static void
gm_aw_modify_addressbook (GtkWidget *addressbook_window, 
			  GmAddressbook *addb) 
{
  GmAddressbookWindow *aw = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gchar *test = NULL;

  int p = -1;

  aw = gm_aw_get_aw (addressbook_window);

  g_return_if_fail (addressbook_window && addb && aw);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->aw_tree_view));

  for (int i = 0 ; i < 2 ; i++) {

    gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), 
					 &iter, (i == 0) ? "0:0" : "1:0");

    do {

      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			  COLUMN_AID, &test, 
			  COLUMN_NOTEBOOK_PAGE, &p,
			  -1);

      if (test && addb->aid && !strcmp (test, addb->aid)) {

	gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
			    COLUMN_AID, addb->aid,
			    COLUMN_URL, addb->url,
			    COLUMN_NAME, addb->name,
			    COLUMN_CALL_ATTRIBUTE, addb->call_attribute,
			    COLUMN_NOTEBOOK_PAGE, p,
			    -1);
	g_free (test);
	break;
      }
      g_free (test);

    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
  }

}


static void
gm_aw_update_addressbook (GtkWidget *addressbook_window,
			  GmAddressbook *addressbook)
{
  GmAddressbookWindow *aw = NULL;
  GmAddressbookWindowPage *awp = NULL;

  GSList *contacts = NULL;
  GSList *l = NULL;

  GmContact *contact = NULL;

  GtkWidget *page = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  GdkPixbuf *status_icon = NULL;

  int page_num = -1;
  int opt = -1;

  const char *filter = NULL;


  page_num = 
    gm_aw_get_notebook_page (addressbook_window,
			     addressbook);

  if (page_num == -1)
    return;

  aw = gm_aw_get_aw (addressbook_window);

  page =
    gtk_notebook_get_nth_page (GTK_NOTEBOOK (aw->aw_notebook), page_num);
  
  if (!page)
    return;
  
  awp = gm_aw_get_awp (page);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (awp->awp_tree_view));

  gtk_list_store_clear (GTK_LIST_STORE (model));

  /* Get the search parameters from the addressbook_window */
  opt = gtk_option_menu_get_history (GTK_OPTION_MENU (aw->aw_option_menu));
  filter = gtk_entry_get_text (GTK_ENTRY (aw->aw_search_entry));
  contacts = 
    gnomemeeting_addressbook_get_contacts (addressbook, 
					   (opt == 1)?(gchar *) filter:NULL,
					   (opt == 2)?(gchar *) filter:NULL,
					   (opt == 3)?(gchar *) filter:NULL,
					   NULL);
  l = contacts;
  while (l) {

    contact = (GmContact *) (l->data);
    gtk_list_store_append (GTK_LIST_STORE (model), &iter);

    if (contact->fullname)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_FULLNAME, contact->fullname, -1);
    if (contact->comment)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_COMMENT, contact->comment, -1);
    if (contact->location)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_LOCATION, contact->location, -1);
    if (contact->email)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_EMAIL, contact->email, -1);
    if (contact->url)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_UURL, contact->url, -1);
    if (contact->categories)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_CATEGORIES, contact->categories, -1);
    if (contact->speeddial)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_SPEED_DIAL, contact->speeddial, -1);
    if (contact->uid)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_UUID, contact->uid, -1);

    status_icon = 
      gtk_widget_render_icon (addressbook_window,
			      contact->state ? 
			      GM_STOCK_STATUS_BUSY 
			      :
			      GM_STOCK_STATUS_AVAILABLE,
			      GTK_ICON_SIZE_MENU, NULL);
    gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			COLUMN_STATUS, status_icon, -1);

    g_object_unref (status_icon);

    l = g_slist_next (l);
  }

  g_slist_foreach (contacts, (GFunc) gm_contact_delete, NULL);
  g_slist_free (contacts);
}


static void 
gm_aw_update_menu_sensitivity (GtkWidget *addressbook_window,
			       gboolean ls,
			       gboolean rs)
{
  GmAddressbookWindow *aw = NULL;

  g_return_if_fail (addressbook_window != NULL);
  g_return_if_fail (ls || rs || (!rs && !ls));

  aw = gm_aw_get_aw (addressbook_window);

  gtk_menu_set_sensitive (aw->aw_menu, "call", rs || ls);
  gtk_menu_set_sensitive (aw->aw_menu, "add", rs);
  gtk_menu_set_sensitive (aw->aw_menu, "properties", !rs);
}


static gint
gm_aw_get_notebook_page (GtkWidget *addressbook_window,
			 GmAddressbook *addressbook)
{
  GmAddressbookWindow *aw = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gchar *test = NULL;
  int p = 0;


  g_return_val_if_fail (addressbook_window != NULL, 0);
  g_return_val_if_fail (addressbook != NULL, 0);

  aw = gm_aw_get_aw (addressbook_window);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->aw_tree_view));

  for (int i = 0 ; i < 2 ; i++) {

    if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), 
					     &iter, (i == 0) ? "0:0" : "1:0")) {

      do {

	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			    COLUMN_AID, &test, 
			    COLUMN_NOTEBOOK_PAGE, &p,
			    -1);

	if (test && addressbook->aid && !strcmp (test, addressbook->aid)) {

	  g_free (test);
	  return p;
	}

      } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
    }
  }

  return -1;
}


/* The Callbacks */
static gboolean 
aw_tree_selection_function_cb (GtkTreeSelection *selection,
			       GtkTreeModel *model,
			       GtkTreePath *path,
			       gboolean path_currently_selected,
			       gpointer data)
{
  if (gtk_tree_path_get_depth (path) <= 1)
    return FALSE;
  else
    return TRUE;
}


static void
call_contact_cb (GtkTreeView *tree_view,
		 GtkTreePath *arg1,
		 GtkTreeViewColumn *arg2,
		 gpointer data)
{
  GtkWidget *addressbook_window = NULL;
  GmContact *contact = NULL;

  g_return_if_fail (data != NULL);

  addressbook_window = GTK_WIDGET (data);

  contact = gm_aw_get_selected_contact (addressbook_window);

  if (contact) {

    /* Call the selected contact */
    //GnomeMeeting->Connect (contact->url);

    g_warning ("connect here");
    gm_contact_delete (contact);
  }
}


static void
new_contact_cb (GtkWidget *w,
		gpointer data)
{
  GmAddressbook *abook = NULL;

  GtkWidget *addressbook = NULL;

  g_return_if_fail (data != NULL);

  addressbook = GTK_WIDGET (data);

  abook = gm_aw_get_selected_addressbook (addressbook);

  gm_addressbook_window_edit_contact_dialog_run (addressbook,
						 abook, 
						 NULL, 
						 addressbook);

  gm_addressbook_delete (abook);
}


static void
new_addressbook_cb (GtkWidget *w,
		    gpointer data)
{
  GtkWidget *addressbook_window = NULL;

  g_return_if_fail (data != NULL);

  addressbook_window = GTK_WIDGET (data);

  gm_addressbook_window_edit_addressbook_dialog_run (addressbook_window,
						     NULL,
						     addressbook_window);
}


static void
delete_cb (GtkWidget *w,
	   gpointer data)
{
  GmContact *contact = NULL;
  GmAddressbook *abook = NULL;

  GtkWidget *addressbook_window = NULL;

  g_return_if_fail (data != NULL);

  addressbook_window = GTK_WIDGET (data);

  contact = gm_aw_get_selected_contact (addressbook_window);
  abook = gm_aw_get_selected_addressbook (addressbook_window);

  if (contact)
    gm_addressbook_window_delete_contact_dialog_run (addressbook_window, 
						     abook, 
						     contact, 
						     addressbook_window);
  else if (abook)
    gm_addressbook_window_delete_addressbook_dialog_run (addressbook_window,
							 abook,
							 addressbook_window);

  gm_contact_delete (contact);  
  gm_addressbook_delete (abook);
}


static void
properties_cb (GtkWidget *w,
	       gpointer data)
{
  GmContact *contact = NULL;
  GmAddressbook *abook = NULL;

  GtkWidget *addressbook_window = NULL;

  g_return_if_fail (data != NULL);

  addressbook_window = GTK_WIDGET (data);

  contact = gm_aw_get_selected_contact (addressbook_window);
  abook = gm_aw_get_selected_addressbook (addressbook_window);

  if (contact)
    gm_addressbook_window_edit_contact_dialog_run (addressbook_window,
						   abook, 
						   contact, 
						   addressbook_window);
  else if (abook)
    gm_addressbook_window_edit_addressbook_dialog_run (addressbook_window,
						       abook,
						       addressbook_window);

  gm_contact_delete (contact);  
  gm_addressbook_delete (abook);
}


static void
search_addressbook_cb (GtkWidget *w,
		       gpointer data)
{
  GtkWidget *addressbook_window = NULL;

  g_return_if_fail (data != NULL);

  addressbook_window = GTK_WIDGET (data);

  class SearchThread : public PThread
  {
    PCLASSINFO (SearchThread, PThread);

public:
    SearchThread (GtkWidget *w)
      :PThread (1000, NoAutoDeleteThread), 
      addressbook_window (w) 
	{ 
	  Resume (); 
	}

    ~SearchThread ()
      {
	PWaitAndSignal m(quit_mutex);
      }

    void Main ()
      { 
	PWaitAndSignal m(quit_mutex);
	gdk_threads_enter ();
	addressbook = gm_aw_get_selected_addressbook (addressbook_window);
	gm_aw_update_addressbook (addressbook_window, 
				  addressbook);
	gm_addressbook_delete (addressbook);
	gdk_threads_leave ();
      }
protected:
    GmAddressbook *addressbook;
    GtkWidget *addressbook_window;

    PMutex quit_mutex;
  };

  new SearchThread (addressbook_window);
}


static void
contact_selected_cb (GtkTreeSelection *selection,
		     gpointer data)
{
  GmAddressbook *addressbook = NULL;

  gboolean ls = FALSE;
  gboolean rs = FALSE;

  g_return_if_fail (data != NULL);

  addressbook = GM_ADDRESSBOOK (gm_aw_get_selected_addressbook (GTK_WIDGET (data)));

  ls = gnomemeeting_addressbook_is_local (addressbook);
  rs = !ls;

  gm_aw_update_menu_sensitivity (GTK_WIDGET (data),
				 ls, rs);
}


static void
addressbook_selected_cb (GtkTreeSelection *selection,
			 gpointer data)
{
  GtkWidget *page = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeSelection *lselection = NULL;
  GtkTreeIter iter;

  GmAddressbookWindow *aw = NULL;
  GmAddressbookWindowPage *awp = NULL;

  gint page_num = -1;

  g_return_if_fail (data != NULL);

  aw = gm_aw_get_aw (GTK_WIDGET (data));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			COLUMN_NOTEBOOK_PAGE, &page_num, -1);

    /* Select the good notebook page for the contact section */
    if (page_num != -1) {

      /* Selects the good notebook page */
      gtk_notebook_set_current_page (GTK_NOTEBOOK (aw->aw_notebook), 
				     page_num);	

      /* Unselect all rows of the list store in that notebook page */
      page =
	gtk_notebook_get_nth_page (GTK_NOTEBOOK (aw->aw_notebook), page_num);

      if (page)
	awp = gm_aw_get_awp (GTK_WIDGET (page));

      if (awp) {

	lselection =
	  gtk_tree_view_get_selection (GTK_TREE_VIEW (awp->awp_tree_view));

	if (lselection)
	  gtk_tree_selection_unselect_all (GTK_TREE_SELECTION (lselection));
      }
    }
  }

  gm_aw_update_menu_sensitivity (GTK_WIDGET (data),
				 FALSE, FALSE);
}


/* Let's go for the implementation of the public API */

GtkWidget *
gm_addressbook_window_new ()
{
  GmAddressbookWindow *aw = NULL;

  GtkWidget *window = NULL;
  GtkWidget *hpaned = NULL;

  GtkWidget *main_vbox = NULL;
  GtkWidget *vbox2 = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *find_button = NULL;
  GtkWidget *handle = NULL;
  GtkWidget *menu = NULL;
  GtkWidget *menu_item = NULL;
  GtkWidget *statusbar = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *scroll = NULL;
  GdkPixbuf *icon = NULL;

  GtkCellRenderer *cell = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkAccelGroup *accel = NULL;
  GtkTreeStore *model = NULL;
  GtkTreeIter iter;

  GSList *addressbooks = NULL;
  GSList *l = NULL;

  int p = 0;


  /* The Top-level window */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  icon = gtk_widget_render_icon (GTK_WIDGET (window),
				 GM_STOCK_ADDRESSBOOK_16,
				 GTK_ICON_SIZE_MENU, NULL);
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("address_book_window"), g_free);

  gtk_window_set_title (GTK_WINDOW (window), 
			_("Address Book"));
  gtk_window_set_icon (GTK_WINDOW (window), icon);
  gtk_window_set_position (GTK_WINDOW (window), 
			   GTK_WIN_POS_CENTER);
  gtk_window_set_default_size (GTK_WINDOW (window), 670, 370);
  g_object_unref (icon);


  /* The GMObject data */
  aw = new GmAddressbookWindow ();
  g_object_set_data_full (G_OBJECT (window), "GMObject", 
			  aw, (GDestroyNotify) gm_aw_destroy);


  /* The accelerators */
  accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (window), accel);


  /* A vbox that will contain the menubar, the hpaned containing
     the rest of the window and the statusbar */
  main_vbox = gtk_vbox_new (0, FALSE);
  gtk_container_add (GTK_CONTAINER (window), main_vbox);
  aw->aw_menu = gtk_menu_bar_new ();

  static MenuEntry addressbook_menu [] =
    {
      GTK_MENU_NEW(_("_File")),

      GTK_MENU_ENTRY("new_addressbook", _("New _Address Book"), NULL,
		     GM_STOCK_REMOTE_CONTACT, '0', 
		     GTK_SIGNAL_FUNC (new_addressbook_cb),
		     window, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("delete", _("_Delete"), NULL,
		     GTK_STOCK_DELETE, 'd', 
		     GTK_SIGNAL_FUNC (delete_cb), 
		     window, TRUE),

      GTK_MENU_ENTRY("properties", _("_Properties"), NULL,
		     GTK_STOCK_PROPERTIES, 0, 
		     GTK_SIGNAL_FUNC (properties_cb), 
		     window, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("close", _("_Close"), NULL,
		     GTK_STOCK_CLOSE, 'w',
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) window, TRUE),

      GTK_MENU_NEW(_("C_ontact")),

      GTK_MENU_ENTRY("call", _("C_all Contact"), NULL,
		     NULL, 0, 
		     NULL, NULL, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("new_contact", _("New _Contact"), NULL,
		     GTK_STOCK_NEW, 'n', 
		     GTK_SIGNAL_FUNC (new_contact_cb), 
		     window, TRUE),
      GTK_MENU_ENTRY("add", _("Add Contact to _Address Book"), NULL,
		     GTK_STOCK_ADD, 0,
		     GTK_SIGNAL_FUNC (properties_cb), 
		     window, TRUE),

      GTK_MENU_END
    };

  gtk_build_menu (aw->aw_menu, addressbook_menu, accel, NULL);

  gtk_box_pack_start (GTK_BOX (main_vbox), aw->aw_menu, FALSE, FALSE, 0);


  /* A hpaned to put the tree and the LDAP browser */
  hpaned = gtk_hpaned_new ();
  gtk_container_set_border_width (GTK_CONTAINER (hpaned), 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), hpaned, TRUE, TRUE, 0);

  /* The GtkTreeView that will store the address books list */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_paned_add1 (GTK_PANED (hpaned), frame);
  model = gtk_tree_store_new (NUM_COLUMNS_CONTACTS, 
			      GDK_TYPE_PIXBUF, 
			      G_TYPE_STRING, 
			      G_TYPE_INT, 
			      G_TYPE_BOOLEAN,
			      G_TYPE_INT,
			      G_TYPE_STRING,
			      G_TYPE_STRING,
			      G_TYPE_STRING); 

  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), 
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (frame), scroll);

  aw->aw_tree_view = gtk_tree_view_new ();  
  gtk_tree_view_set_model (GTK_TREE_VIEW (aw->aw_tree_view), 
			   GTK_TREE_MODEL (model));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (aw->aw_tree_view));
  gtk_container_add (GTK_CONTAINER (scroll), aw->aw_tree_view);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (aw->aw_tree_view), FALSE);
  gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection),
			       GTK_SELECTION_BROWSE);
  gtk_tree_selection_set_select_function (selection, (GtkTreeSelectionFunc) 
					  aw_tree_selection_function_cb, 
					  NULL, NULL);


  /* Two renderers for one column */
  column = gtk_tree_view_column_new ();
  cell = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_set_attributes (column, cell, 
				       "pixbuf", COLUMN_PIXBUF, 
				       "visible", COLUMN_PIXBUF_VISIBLE, 
				       NULL);

  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_set_attributes (column, cell, 
				       "text", COLUMN_NAME, 
				       "weight", COLUMN_WEIGHT,
				       NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (aw->aw_tree_view),
			       GTK_TREE_VIEW_COLUMN (column));


  /* We update the address books list with the top-level categories */
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter,
		      COLUMN_NAME, _("On LDAP servers"), 
		      COLUMN_NOTEBOOK_PAGE, -1, 
		      COLUMN_PIXBUF_VISIBLE, FALSE,
		      COLUMN_WEIGHT, PANGO_WEIGHT_BOLD, 
		      -1);
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter,
		      COLUMN_NAME, _("On This Computer"), 
		      COLUMN_NOTEBOOK_PAGE, -1, 
		      COLUMN_PIXBUF_VISIBLE, FALSE,
		      COLUMN_WEIGHT, PANGO_WEIGHT_BOLD, 
		      -1);


  /* The LDAP browser in the second part of the GtkHPaned */
  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_paned_add2 (GTK_PANED (hpaned), vbox2);  

  /* Each page of the GtkNotebook contains a list of the users */
  aw->aw_notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (aw->aw_notebook), 0);
  gtk_box_pack_start (GTK_BOX (vbox2), aw->aw_notebook, 
		      TRUE, TRUE, 0);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (aw->aw_notebook), FALSE);

  /* The search entry */
  hbox = gtk_hbox_new (FALSE, 0);
  handle = gtk_handle_box_new ();
  gtk_box_pack_start (GTK_BOX (vbox2), handle, FALSE, FALSE, 0);  
  gtk_container_add (GTK_CONTAINER (handle), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (handle), 0);

  /* The option menu */
  menu = gtk_menu_new ();

  menu_item =
    gtk_menu_item_new_with_label (_("Find all contacts"));
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  menu_item =
    gtk_menu_item_new_with_label (_("Name contains"));
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  menu_item = gtk_menu_item_new_with_label (_("URL contains"));
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  menu_item = gtk_menu_item_new_with_label (_("Belongs to category"));
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  aw->aw_option_menu = gtk_option_menu_new ();
  gtk_option_menu_set_menu (GTK_OPTION_MENU (aw->aw_option_menu),
			    menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (aw->aw_option_menu),
			       0);
  gtk_box_pack_start (GTK_BOX (hbox), aw->aw_option_menu, FALSE, FALSE, 2);

  /* The entry */
  aw->aw_search_entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), aw->aw_search_entry, TRUE, TRUE, 2);

  /* The Find button */
  find_button = gtk_button_new_from_stock (GTK_STOCK_FIND);
  gtk_box_pack_start (GTK_BOX (hbox), find_button, FALSE, FALSE, 2);
  gtk_widget_show_all (handle);


  /* The statusbar */
  statusbar = gtk_statusbar_new ();
  gtk_box_pack_start (GTK_BOX (main_vbox), statusbar, FALSE, FALSE, 0);
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar), FALSE);


  g_signal_connect (G_OBJECT (selection), "changed",
		    G_CALLBACK (addressbook_selected_cb), 
		    window);

  g_signal_connect (G_OBJECT (find_button), "clicked",
		    G_CALLBACK (search_addressbook_cb),
		    window);

  g_signal_connect (G_OBJECT (aw->aw_search_entry), "activate",
		    G_CALLBACK (search_addressbook_cb),
		    window);

  g_signal_connect (G_OBJECT (window), "delete_event",
		    G_CALLBACK (delete_window_cb), NULL);

  /*
     gtk_drag_dest_set (GTK_WIDGET (lw->tree_view),
     GTK_DEST_DEFAULT_ALL,
     dnd_targets, 1,
     GDK_ACTION_COPY);
     g_signal_connect (G_OBJECT (lw->tree_view), "drag_motion",
     G_CALLBACK (dnd_drag_motion_cb), 0);
     g_signal_connect (G_OBJECT (lw->tree_view), "drag_data_received",
     G_CALLBACK (dnd_drag_data_received_cb), 0);

*/


  /* Add the various address books */
  addressbooks = gnomemeeting_get_remote_addressbooks ();
  l = addressbooks;
  while (l) {

    gm_aw_add_addressbook (window, GM_ADDRESSBOOK (l->data));

    p++;
    l = g_slist_next (l);
  }
  g_slist_foreach (addressbooks, (GFunc) gm_addressbook_delete, NULL);
  g_slist_free (addressbooks);

  addressbooks = gnomemeeting_get_local_addressbooks ();
  l = addressbooks;
  while (l) {

    gm_aw_add_addressbook (window, GM_ADDRESSBOOK (l->data));

    p++;
    l = g_slist_next (l);
  }
  g_slist_foreach (addressbooks, (GFunc) gm_addressbook_delete, NULL);
  g_slist_free (addressbooks);


  gtk_widget_show_all (GTK_WIDGET (main_vbox));


  return window;
}


void
gm_addressbook_window_edit_contact_dialog_run (GtkWidget *addressbook_window,
					       GmAddressbook *addressbook,
					       GmContact *contact,
					       GtkWidget *parent_window)
{
  GmContact *new_contact = NULL;

  GtkWidget *dialog = NULL;

  GtkWidget *fullname_entry = NULL;
  GtkWidget *url_entry = NULL;
  GtkWidget *categories_entry = NULL;
  GtkWidget *speeddial_entry = NULL;
  GtkWidget *table = NULL;
  GtkWidget *label = NULL;

  GtkWidget *menu = NULL;
  GtkWidget *menu_item = NULL;
  GtkWidget *option_menu = NULL;

  GmAddressbook *addb = NULL;
  GmAddressbook *addc = NULL;

  GSList *list = NULL;
  GSList *l = NULL;

  gchar *label_text = NULL;
  gint result = 0;
  gint current_menu_index = -1;
  gint pos = 0;

  gboolean display_addressbooks = FALSE;


  g_return_if_fail (addressbook != NULL);


  /* Create the dialog to easily modify the info 
   * of a specific contact */
  dialog =
    gtk_dialog_new_with_buttons (_("Edit the Contact Information"), 
				 GTK_WINDOW (parent_window),
				 GTK_DIALOG_MODAL,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
				 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
				 NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
				   GTK_RESPONSE_ACCEPT);

  table = gtk_table_new (5, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3 * GNOMEMEETING_PAD_SMALL);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3 * GNOMEMEETING_PAD_SMALL);


  /* Get the list of addressbooks */
  list = gnomemeeting_get_local_addressbooks ();


  /* The Full Name entry */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Name:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  fullname_entry = gtk_entry_new ();
  if (contact && contact->fullname)
    gtk_entry_set_text (GTK_ENTRY (fullname_entry), contact->fullname);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), fullname_entry, 1, 2, 0, 1, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (fullname_entry), TRUE);

  /* The URL entry */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("URL:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  url_entry = gtk_entry_new ();
  if (contact && contact->url)
    gtk_entry_set_text (GTK_ENTRY (url_entry), contact->url);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), url_entry, 1, 2, 1, 2, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (url_entry), TRUE);

  /* The Speed Dial */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Speed Dial:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  speeddial_entry = gtk_entry_new ();
  if (contact && contact->speeddial)
    gtk_entry_set_text (GTK_ENTRY (speeddial_entry),
			contact->speeddial);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), speeddial_entry,
		    1, 2, 2, 3, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (speeddial_entry), TRUE);

  /* The Categories */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Categories:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  categories_entry = gtk_entry_new ();
  if (contact && contact->categories)
    gtk_entry_set_text (GTK_ENTRY (categories_entry),
			contact->categories);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), categories_entry,
		    1, 2, 3, 4, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (categories_entry), TRUE);

  /* The different local addressbooks are not displayed when
   * we are editing a contact from a local addressbook */
  display_addressbooks = 
    (!gnomemeeting_addressbook_is_local (addressbook)
     || !contact);

  if (display_addressbooks) {

    label = gtk_label_new (NULL);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    label_text = g_strdup_printf ("<b>%s</b>", _("Local Addressbook:"));
    gtk_label_set_markup (GTK_LABEL (label), label_text);
    g_free (label_text);


    menu = gtk_menu_new ();

    l = list;
    pos = 0;
    while (l) {

      if (l->data) {

	addb = GM_ADDRESSBOOK (l->data);
	if (addb->name && addressbook->name 
	    && !strcmp (addb->name, addressbook->name))
	  current_menu_index = pos;
      }
      menu_item =
	gtk_menu_item_new_with_label (addb->name);
      gtk_widget_show (menu_item);
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

      l = g_slist_next (l);
      pos++;
    }

    option_menu = gtk_option_menu_new ();
    gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);
    gtk_option_menu_set_history (GTK_OPTION_MENU (option_menu), 
				 current_menu_index);

    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5, 
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL),
		      3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
    gtk_table_attach (GTK_TABLE (table), option_menu,
		      1, 2, 4, 5, 
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL),
		      GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  }


  /* Pack the gtk entries and the list store in the window */
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), table,
		      FALSE, FALSE, 3 * GNOMEMEETING_PAD_SMALL);
  gtk_widget_show_all (dialog);


  /* Now run the dialg */
  result = gtk_dialog_run (GTK_DIALOG (dialog));

  switch (result) {

  case GTK_RESPONSE_ACCEPT:

    new_contact = gm_contact_new ();
    new_contact->fullname = 
      g_strdup (gtk_entry_get_text (GTK_ENTRY (fullname_entry)));
    new_contact->speeddial = 
      g_strdup (gtk_entry_get_text (GTK_ENTRY (speeddial_entry)));
    new_contact->categories = 
      g_strdup (gtk_entry_get_text (GTK_ENTRY (categories_entry)));
    new_contact->url = 
      g_strdup (gtk_entry_get_text (GTK_ENTRY (url_entry)));

    /* We were editing an existing contact */
    if (contact && gnomemeeting_addressbook_is_local (addressbook)) {

      /* We keep the old UID */
      new_contact->uid = g_strdup (contact->uid);

      gnomemeeting_addressbook_modify_contact (addressbook, new_contact);
      gm_aw_update_addressbook (addressbook_window, addressbook);
    }
    else {

      /* Forget the selected addressbook and use the dialog one instead
       * if the user could choose it in the dialog */
      if (display_addressbooks) {

	current_menu_index =
	  gtk_option_menu_get_history (GTK_OPTION_MENU (option_menu));
	addc = GM_ADDRESSBOOK (g_slist_nth_data (list, current_menu_index)); 

	if (addc) {


	  gnomemeeting_addressbook_add_contact (addc, 
						new_contact);
	  gm_aw_update_addressbook (addressbook_window, addc);
	}
      }
      else { /* The user couldn't choose, so we are using the currently
		selected addressbook */ 

	gnomemeeting_addressbook_add_contact (addressbook, 
					      new_contact);
	gm_aw_update_addressbook (addressbook_window, addressbook);
      }
    }


    gm_contact_delete (new_contact);

    break;

  case GTK_RESPONSE_REJECT:

    break;
  }

  gtk_widget_destroy (dialog);

  g_slist_foreach (list, (GFunc) gm_addressbook_delete, NULL);
  g_slist_free (list);
}


void
gm_addressbook_window_delete_contact_dialog_run (GtkWidget *addressbook_window,
						 GmAddressbook *addressbook,
						 GmContact *contact,
						 GtkWidget *parent_window)
{
  GtkWidget *dialog = NULL;

  gchar *confirm_msg = NULL;

  g_return_if_fail (addressbook != NULL);
  g_return_if_fail (contact != NULL);


  confirm_msg = g_strdup_printf (_("Are you sure you want to delete %s from %s?"),
				 contact->fullname, addressbook->name);
  dialog =
    gtk_message_dialog_new (GTK_WINDOW (parent_window),
			    GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
			    GTK_BUTTONS_YES_NO, confirm_msg);
  g_free (confirm_msg);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
				   GTK_RESPONSE_YES);

  gtk_widget_show_all (dialog);


  /* Now run the dialg */
  switch (gtk_dialog_run (GTK_DIALOG (dialog))) {

  case GTK_RESPONSE_YES:

    gnomemeeting_addressbook_delete_contact (addressbook, contact);
    gm_aw_update_addressbook (addressbook_window, addressbook);

    break;
  }

  gtk_widget_destroy (dialog);
}


void
gm_addressbook_window_edit_addressbook_dialog_run (GtkWidget *addressbook_window,
						   GmAddressbook *addb,
						   GtkWidget *parent_window)
{
  GmAddressbook *addc = NULL;

  GtkWidget *dialog = NULL;

  GtkWidget *base_entry = NULL;
  GtkWidget *hostname_entry = NULL;
  GtkWidget *addressbook_name_entry = NULL;
  GtkWidget *port_entry = NULL;
  GtkWidget *search_attribute_entry = NULL;
  GtkWidget *type_option_menu = NULL;
  GtkWidget *scope_option_menu = NULL;

  GtkWidget *table = NULL;
  GtkWidget *label = NULL;

  GtkWidget *menu_item = NULL;
  GtkWidget *menu = NULL;

  const char *hostname = NULL;
  const char *port = NULL;
  const char *base = NULL;
  char *scope = NULL;
  char *prefix = NULL;

  PString entry;
  char default_hostname [256] = "";
  char default_port [256] = "";
  char default_base [256] = "";
  char default_scope [256] = "";
  char default_prefix [256] = "";
  int done = -1;
  int history = 0;

  gchar *label_text = NULL;
  int result = -1;

  g_return_if_fail (addressbook_window != NULL);


  /* Parse the URL if any */
  if (addb) {

    entry = addb->url;
    entry.Replace (":", " ", TRUE);
    entry.Replace ("/", " ", TRUE);
    entry.Replace ("?", " ", TRUE);

    done = sscanf ((const char *) entry, 
		   "%255s %255s %255s %255s %255s", 
		   default_prefix, default_hostname, 
		   default_port, default_base, default_scope);

    /* If we have no "scope", then it means there was no base, hackish */
    if (done == 4 && !strcmp (default_scope, "")) {

      strncpy (default_scope, default_base, 255);
      strcpy (default_base, "");
    }
  }


  /* Create the dialog to create a new addressbook */
  dialog =
    gtk_dialog_new_with_buttons (_("Add an address book"), 
				 GTK_WINDOW (parent_window),
				 GTK_DIALOG_MODAL,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
				 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
				 NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
				   GTK_RESPONSE_ACCEPT);

  table = gtk_table_new (7, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3 * GNOMEMEETING_PAD_SMALL);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3 * GNOMEMEETING_PAD_SMALL);


  /* The Server Name entry */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Name:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  addressbook_name_entry = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), addressbook_name_entry, 1, 2, 0, 1, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (addressbook_name_entry), TRUE);
  if (addb && addb->name)
    gtk_entry_set_text (GTK_ENTRY (addressbook_name_entry), addb->name);


  /* Addressbook type */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Type:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  menu = gtk_menu_new ();

  menu_item =
    gtk_menu_item_new_with_label (_("Local"));
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  menu_item =
    gtk_menu_item_new_with_label (_("Remote LDAP"));
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  menu_item =
    gtk_menu_item_new_with_label (_("Remote ILS"));
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  type_option_menu = gtk_option_menu_new ();
  gtk_option_menu_set_menu (GTK_OPTION_MENU (type_option_menu), menu);
  if (addb) {

    if (!strcmp (default_prefix, "ldap"))
      history = 1;
    else if (!strcmp (default_prefix, "ils"))
      history = 2;

    gtk_option_menu_set_history (GTK_OPTION_MENU (type_option_menu),
				 history);

    history = 0;
  } 

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), type_option_menu, 1, 2, 1, 2, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);


  /* Addressbook search scope */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Search Scope:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  menu = gtk_menu_new ();

  menu_item =
    gtk_menu_item_new_with_label (_("Subtree"));
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  menu_item =
    gtk_menu_item_new_with_label (_("One Level"));
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  scope_option_menu = gtk_option_menu_new ();
  gtk_option_menu_set_menu (GTK_OPTION_MENU (scope_option_menu), menu);
  if (addb) {

    if (!strcmp (default_scope, "one"))
      history = 1;

    gtk_option_menu_set_history (GTK_OPTION_MENU (scope_option_menu),
				 history);

    history = 0;
  }

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), scope_option_menu, 1, 2, 4, 5, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);


  /* The Server Name entry */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Hostname:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  hostname_entry = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), hostname_entry, 1, 2, 2, 3, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (hostname_entry), TRUE);
  if (addb)
    gtk_entry_set_text (GTK_ENTRY (hostname_entry), default_hostname);


  /* The Server Port entry */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Port:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  port_entry = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), port_entry, 1, 2, 3, 4, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (port_entry), TRUE);
  if (addb)
    gtk_entry_set_text (GTK_ENTRY (port_entry), default_port);
  else
    gtk_entry_set_text (GTK_ENTRY (port_entry), "389");


  /* The Base entry */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Base DN:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  base_entry = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 5, 6, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), base_entry, 1, 2, 5, 6, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (base_entry), TRUE);
  if (addb)
    gtk_entry_set_text (GTK_ENTRY (base_entry), default_base);
  else
    gtk_entry_set_text (GTK_ENTRY (base_entry), "objectclass=*");


  /* The search attribute entry */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Search Attribute:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  search_attribute_entry = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 6, 7, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), search_attribute_entry, 1, 2, 6, 7, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (search_attribute_entry), TRUE);
  if (addb && addb->call_attribute)
    gtk_entry_set_text (GTK_ENTRY (search_attribute_entry), 
			addb->call_attribute);
  else
    gtk_entry_set_text (GTK_ENTRY (search_attribute_entry), "rfc822mailbox");


  /* Pack the gtk entries and the list store in the window */
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), table,
		      FALSE, FALSE, 3 * GNOMEMEETING_PAD_SMALL);
  gtk_widget_show_all (dialog);


  /* Now run the dialg */
  result = gtk_dialog_run (GTK_DIALOG (dialog));

  switch (result) {

  case GTK_RESPONSE_ACCEPT:

    addc = gm_addressbook_new ();

    if (addb && addb->aid)
      addc->aid = g_strdup (addb->aid);
    addc->name = 
      g_strdup (gtk_entry_get_text (GTK_ENTRY (addressbook_name_entry)));
    addc->call_attribute =
      g_strdup (gtk_entry_get_text (GTK_ENTRY (search_attribute_entry)));

    if (gtk_option_menu_get_history (GTK_OPTION_MENU (type_option_menu)) != 0) {

      hostname = gtk_entry_get_text (GTK_ENTRY (hostname_entry));
      port = gtk_entry_get_text (GTK_ENTRY (port_entry));
      base = gtk_entry_get_text (GTK_ENTRY (base_entry));
      if (gtk_option_menu_get_history (GTK_OPTION_MENU (scope_option_menu))==0)
	scope = g_strdup ("sub");
      else
	scope = g_strdup ("one");
      if (gtk_option_menu_get_history (GTK_OPTION_MENU (type_option_menu))==1)
	prefix = g_strdup ("ldap");
      else
	prefix = g_strdup ("ils");

      addc->url = g_strdup_printf ("%s://%s:%s/%s??%s", 
				   prefix, hostname, port, base, scope);
    }

    if (addb) {

      gnomemeeting_addressbook_modify (addc);
      gm_aw_modify_addressbook (parent_window, addc);
    }
    else {

      gnomemeeting_addressbook_add (addc);
      gm_aw_add_addressbook (parent_window, addc);
    }
    gm_addressbook_delete (addc);

    g_free (scope);
    g_free (prefix);

    break;
  }

  gtk_widget_destroy (dialog);
}


void 
gm_addressbook_window_delete_addressbook_dialog_run (GtkWidget *addressbook_window,
						     GmAddressbook *addressbook,
						     GtkWidget *parent_window)
{
  GtkWidget *dialog = NULL;

  gchar *confirm_msg = NULL;

  g_return_if_fail (addressbook_window != NULL);
  g_return_if_fail (addressbook != NULL);


  /* Create the dialog to delete the addressbook */
  confirm_msg = 
    g_strdup_printf (_("Are you sure you want to delete %s and all its contacts?"), addressbook->name);
  dialog =
    gtk_message_dialog_new (GTK_WINDOW (addressbook_window),
			    GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
			    GTK_BUTTONS_YES_NO, confirm_msg);
  g_free (confirm_msg);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
				   GTK_RESPONSE_YES);

  gtk_widget_show_all (dialog);


  /* Now run the dialg */
  switch (gtk_dialog_run (GTK_DIALOG (dialog))) {

  case GTK_RESPONSE_YES:

    if (gnomemeeting_addressbook_delete (addressbook))
      gm_aw_delete_addressbook (addressbook_window, addressbook);

    break;
  }

  gtk_widget_destroy (dialog);
}




