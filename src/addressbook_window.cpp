
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
#include "contacts/gm_contacts.h"
#include "gm_conf.h"
#include "gtk_menu_extensions.h"




struct GmAddressbookWindow_ {

  GtkWidget *aw_tree_view; /* The GtkTreeView that contains the address books
                              list */
  GtkWidget *aw_notebook;  /* The GtkNotebook that contains the different
                              listings for each of the address books */
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
  COLUMN_UID,
  NUM_COLUMNS_CONTACTS
};

enum {

  COLUMN_FULLNAME,
  COLUMN_URL,
  COLUMN_CATEGORIES,
  COLUMN_SPEED_DIAL,
  COLUMN_UUID,
  NUM_COLUMNS_GROUPS
};


static GmAddressbookWindow *gnomemeeting_aw_get_aw (GtkWidget *);
static GmAddressbookWindowPage *gnomemeeting_aw_get_awp (GtkWidget *);
static GmContact *get_selected_contact (GtkWidget *);
static GmAddressbook *get_selected_addressbook (GtkWidget *);
static void gnomemeeting_aw_update_addressbook (GtkWidget *, GmAddressbook *);


static void
edit_contact_dialog_run (GmAddressbook *addressbook,
                         GmContact *contact)
{
  GtkWidget *dialog = NULL;
  
  GtkWidget *addressbook_window = NULL;
  
  GtkWidget *fullname_entry = NULL;
  GtkWidget *url_entry = NULL;
  GtkWidget *categories_entry = NULL;
  GtkWidget *speeddial_entry = NULL;
  GtkWidget *table = NULL;
  GtkWidget *label = NULL;

  gchar *label_text = NULL;
  gint result = 0;

  
  addressbook_window = GnomeMeeting::Process ()->GetAddressbookWindow ();

  
  /* Create the dialog to easily modify the info of a specific contact */
  dialog =
    gtk_dialog_new_with_buttons (_("Edit the Contact Information"), 
                                 GTK_WINDOW (addressbook_window),
				 GTK_DIALOG_MODAL,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
				 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
				 NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
				   GTK_RESPONSE_ACCEPT);
  
  table = gtk_table_new (4, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3 * GNOMEMEETING_PAD_SMALL);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3 * GNOMEMEETING_PAD_SMALL);
  
  /* The Full Name entry */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
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
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
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
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  label_text = g_strdup_printf ("<b>%s</b>", _("Speed Dial:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  speeddial_entry = gtk_entry_new ();
  if (contact && contact->speeddial)
    gtk_entry_set_text (GTK_ENTRY (contact->speeddial),
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
  
  /* The Speed Dial */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
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
 
  
  /* Pack the gtk entries and the list store in the window */
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), table,
		      FALSE, FALSE, 3 * GNOMEMEETING_PAD_SMALL);
  gtk_widget_show_all (dialog);
  

  /* Now run the dialg */
  result = gtk_dialog_run (GTK_DIALOG (dialog));

  switch (result) {

  case GTK_RESPONSE_ACCEPT:

    g_free (contact->fullname);
    contact->fullname = g_strdup (gtk_entry_get_text (GTK_ENTRY (fullname_entry)));

    gnomemeeting_addressbook_modify_contact (addressbook, contact);
    gnomemeeting_aw_update_addressbook (addressbook_window, addressbook);

    break;

  case GTK_RESPONSE_REJECT:

    break;
  }

  gtk_widget_destroy (dialog);
}


static void
edit_contact_cb (GtkWidget *w,
                 gpointer data)
{
  GmContact *contact = NULL;
  GmAddressbook *abook = NULL;
  
  GtkWidget *addressbook = NULL;

  addressbook = GnomeMeeting::Process ()->GetAddressbookWindow ();

  contact = get_selected_contact (addressbook);
  abook = get_selected_addressbook (addressbook);
  
  edit_contact_dialog_run (abook, contact);


  gm_contact_delete (contact);
  gm_addressbook_delete (abook);
}


static void
addressbook_changed_cb (GtkTreeSelection *selection,
                        gpointer data)
{
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gint page_num = -1;

  g_return_if_fail (data != NULL);

  
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			COLUMN_NOTEBOOK_PAGE, &page_num, -1);
    
    /* Select the good notebook page for the contact section */
    if (page_num != -1) {

      /* Selects the good notebook page */
      gtk_notebook_set_current_page (GTK_NOTEBOOK (data), 
				     page_num);	
    }
  }
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  
 * PRE          :  /
 */
static GmContact *
get_selected_contact (GtkWidget *addressbook)
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
  aw = gnomemeeting_aw_get_aw (addressbook);
  page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (aw->aw_notebook));
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (aw->aw_notebook), page_num);
  awp = gnomemeeting_aw_get_awp (page);
  
  g_return_val_if_fail (awp != NULL, NULL);
  
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (awp->awp_tree_view));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (awp->awp_tree_view));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    contact = gm_contact_new ();
      
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
                        COLUMN_FULLNAME, &contact->fullname, 
                        COLUMN_SPEED_DIAL, &contact->speeddial,
                        COLUMN_CATEGORIES, &contact->categories,
                        COLUMN_URL, &contact->url,
                        COLUMN_UUID, &contact->uid,
                        -1);
  }
  
  
  return contact;
}


static GmAddressbook *
get_selected_addressbook (GtkWidget *addressbook)
{
  GmAddressbookWindow *aw = NULL;
  GmAddressbook *abook = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;

  /* Get the required data from the GtkNotebook page */
  aw = gnomemeeting_aw_get_aw (addressbook);
  
  g_return_val_if_fail (aw != NULL, NULL);
  
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (aw->aw_tree_view));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->aw_tree_view));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    abook = gm_addressbook_new ();
      
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
                        COLUMN_NAME, &abook->name, 
                        COLUMN_UID, &abook->uid,
                        -1); 
  }
    
  return abook;
}


static GmAddressbookWindow *
gnomemeeting_aw_get_aw (GtkWidget *addressbook_window)
{
  g_return_val_if_fail (addressbook_window != NULL, NULL);
  
  return GM_ADDRESSBOOK_WINDOW (g_object_get_data (G_OBJECT (addressbook_window), "GMObject"));
}


static GmAddressbookWindowPage *
gnomemeeting_aw_get_awp (GtkWidget *p)
{
  g_return_val_if_fail (p != NULL, NULL);
  
  return GM_ADDRESSBOOK_WINDOW_PAGE (g_object_get_data (G_OBJECT (p), "GMObject"));
}


static void
gnomemeeting_aw_add_tree_view_section (GtkWidget *addressbook_window, 
                                       GmAddressbook *addressbook,
                                       int pos)
{
  GmAddressbookWindow *aw = NULL;

  GdkPixbuf *contact_icon = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeIter iter, child_iter;
  
  g_return_if_fail (addressbook_window != NULL);
  g_return_if_fail (addressbook != NULL);

  aw = gnomemeeting_aw_get_aw (addressbook_window);

  contact_icon = 
    gtk_widget_render_icon (aw->aw_tree_view, 
                            GM_STOCK_LOCAL_CONTACT,
			    GTK_ICON_SIZE_MENU, NULL);
  
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->aw_tree_view));
  

  if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &iter, "1")) {

   gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
   gtk_tree_store_set (GTK_TREE_STORE (model),
                       &child_iter, 
                       COLUMN_PIXBUF, contact_icon,
                       COLUMN_NAME, addressbook->name,
                       COLUMN_NOTEBOOK_PAGE, pos, 
                       COLUMN_PIXBUF_VISIBLE, TRUE,
                       COLUMN_WEIGHT, PANGO_WEIGHT_NORMAL, 
                       COLUMN_UID, addressbook->uid, -1);
  }
  
  gtk_tree_view_expand_all (GTK_TREE_VIEW (aw->aw_tree_view));
}


static gint
gnomemeeting_aw_get_notebook_page (GtkWidget *addressbook_window,
                                   GmAddressbook *addressbook)
{
  GmAddressbookWindow *aw = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  
  gchar *test = NULL;
  int p = 0;


  g_return_val_if_fail (addressbook_window != NULL, 0);
  g_return_val_if_fail (addressbook != NULL, 0);

  aw = gnomemeeting_aw_get_aw (addressbook_window);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->aw_tree_view));

  gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &iter, "1:0");

  do {
      
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
                        COLUMN_UID, &test, 
                        -1);

    if (test && addressbook->uid && !strcmp (test, addressbook->uid)) {

      g_free (test);
      return p;
    }

    p++;
  } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
  

  return -1;
}


static void
gnomemeeting_aw_add_notebook_page (GtkWidget *addressbook_window,
                                   GmAddressbook *addressbook,
                                   int pos)
{
  GmAddressbookWindow *aw = NULL;
  GmAddressbookWindowPage *awp = NULL;
  
  GtkWidget *page = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *scroll = NULL;

  GtkTreeViewColumn *column = NULL;
  GtkListStore *list_store = NULL;
  GtkCellRenderer *renderer = NULL;

  aw = gnomemeeting_aw_get_aw (addressbook_window);
  awp = new GmAddressbookWindowPage ();
  

  list_store = 
      gtk_list_store_new (NUM_COLUMNS_GROUPS, 
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


  gtk_notebook_insert_page (GTK_NOTEBOOK (aw->aw_notebook), vbox, NULL, pos);
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (aw->aw_notebook), pos);
  g_object_set_data (G_OBJECT (page), "GMObject", awp);
  g_warning ("Must be freed");
  
  awp->awp_tree_view = 
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (awp->awp_tree_view), TRUE);


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
                                                     COLUMN_URL,
                                                     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_URL);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_column_set_min_width (GTK_TREE_VIEW_COLUMN (column), 280);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);
  g_object_set (G_OBJECT (renderer), "foreground", "blue",
                "underline", TRUE, NULL);

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
  
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Speed Dial"),
                                                     renderer,
                                                     "text", 
                                                     COLUMN_SPEED_DIAL,
                                                     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_SPEED_DIAL);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);

  /* The invisible column containint the uid for the GmAddressbook */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL,
                                                     renderer,
                                                     "text", 
                                                     COLUMN_UUID,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (aw->aw_tree_view), column);
  g_object_set (G_OBJECT (renderer), "visible", false, NULL);
  
  /* Add the tree view */
  gtk_container_add (GTK_CONTAINER (scroll), awp->awp_tree_view);
  gtk_container_set_border_width (GTK_CONTAINER (awp->awp_tree_view), 0);
  gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);

  
  /* Update the address book content in the GUI */
  gnomemeeting_aw_update_addressbook (addressbook_window,
                                      addressbook);
}


static void
gnomemeeting_aw_update_addressbook (GtkWidget *addressbook_window,
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

  int page_num = -1;


  page_num = 
    gnomemeeting_aw_get_notebook_page (addressbook_window,
                                       addressbook);

  if (page_num == -1)
    return;
  

  aw = gnomemeeting_aw_get_aw (addressbook_window);
  
  page =
    gtk_notebook_get_nth_page (GTK_NOTEBOOK (aw->aw_notebook), page_num);
  awp = gnomemeeting_aw_get_awp (page);
  
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (awp->awp_tree_view));

  gtk_list_store_clear (GTK_LIST_STORE (model));
                                   
  contacts = gnomemeeting_addressbook_get_contacts (addressbook, "*");
  l = contacts;
  while (l) {

    contact = (GmContact *) (l->data);
    gtk_list_store_append (GTK_LIST_STORE (model), &iter);
    
    if (contact->fullname)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_FULLNAME, contact->fullname, -1);
    if (contact->url)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_URL, contact->url, -1);
    if (contact->categories)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_CATEGORIES, contact->categories, -1);
    
    if (contact->uid)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_UUID, contact->uid, -1);

    l = g_slist_next (l);
  }

  g_warning ("must be freed");
  g_slist_free (contacts);
}


GtkWidget *
gnomemeeting_addressbook_window_new ()
{
  GmAddressbookWindow *aw = NULL;
  
  GtkWidget *window = NULL;
  GtkWidget *hpaned = NULL;
  
  GtkWidget *main_vbox = NULL;
  GtkWidget *vbox2 = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *search_entry = NULL;
  GtkWidget *find_button = NULL;
  GtkWidget *handle = NULL;
  GtkWidget *menu = NULL;
  GtkWidget *menu_item = NULL;
  GtkWidget *statusbar = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *menubar = NULL;
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
  g_object_set_data (G_OBJECT (window), "GMObject", aw);
  g_warning ("Must be freed");
  
  
  /* The accelerators */
  accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (window), accel);
  
  
  /* A vbox that will contain the menubar, the hpaned containing
     the rest of the window and the statusbar */
  main_vbox = gtk_vbox_new (0, FALSE);
  gtk_container_add (GTK_CONTAINER (window), main_vbox);
  menubar = gtk_menu_bar_new ();
  
  static MenuEntry addressbook_menu [] =
    {
      GTK_MENU_NEW(_("_File")),

      GTK_MENU_ENTRY("new_server", _("New _Server"), NULL,
		     GM_STOCK_REMOTE_CONTACT, 0,
                     NULL,
		     GINT_TO_POINTER (CONTACTS_SERVERS), TRUE),
      GTK_MENU_ENTRY("new_group", _("New _Group"), NULL,
		     GM_STOCK_LOCAL_CONTACT, 0, 
		     NULL,
		     GINT_TO_POINTER (CONTACTS_GROUPS), TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("delete", _("_Delete"), NULL,
		     GTK_STOCK_DELETE, 'd', 
		     NULL, NULL, FALSE),

      GTK_MENU_ENTRY("rename", _("_Rename"), NULL,
		     NULL, 0,
		     NULL,
		     GINT_TO_POINTER (1), TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("close", _("_Close"), NULL,
		     GTK_STOCK_CLOSE, 'w',
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) window, TRUE),

      GTK_MENU_NEW(_("C_ontact")),

      GTK_MENU_ENTRY("call", _("C_all Contact"), NULL,
		     NULL, 0, 
		     NULL, NULL, FALSE),
      GTK_MENU_ENTRY("transfer", _("_Tranfer Call to Contact"), NULL,
		     NULL, 0, 
		     NULL, NULL, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("new_contact", _("New _Contact"), NULL,
		     GTK_STOCK_NEW, 'n', 
		     GTK_SIGNAL_FUNC (edit_contact_cb), 
		     NULL, TRUE),
      GTK_MENU_ENTRY("add", _("Add Contact to _Address Book"), NULL,
		     GTK_STOCK_ADD, 0,
		     NULL, 
		     NULL, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("properties", _("Contact _Properties"), NULL,
		     GTK_STOCK_PROPERTIES, 0, 
		     NULL, 
		     NULL, FALSE),

      GTK_MENU_END
    };

  gtk_build_menu (menubar, addressbook_menu, accel, NULL);
  
  gtk_box_pack_start (GTK_BOX (main_vbox), menubar, FALSE, FALSE, 0);

  
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
                                       NULL);
  gtk_tree_view_column_add_attribute (column, cell, 
                                      "weight", COLUMN_WEIGHT);
  gtk_tree_view_append_column (GTK_TREE_VIEW (aw->aw_tree_view),
			       GTK_TREE_VIEW_COLUMN (column));

  /* The invisible column containint the uid for the GmAddressbook */
  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL,
                                                     cell,
                                                     "text", 
                                                     COLUMN_UUID,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (aw->aw_tree_view), column);
  g_object_set (G_OBJECT (cell), "visible", false, NULL);
  

  /* We update the address books list with the top-level categories */
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter,
		      COLUMN_NAME, _("On LDAP servers"), 
		      COLUMN_NOTEBOOK_PAGE, -1, 
		      COLUMN_PIXBUF_VISIBLE, FALSE,
		      COLUMN_WEIGHT, PANGO_WEIGHT_BOLD, -1);
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter,
		      COLUMN_NAME, _("On This Computer"), 
		      COLUMN_NOTEBOOK_PAGE, -1, 
		      COLUMN_PIXBUF_VISIBLE, FALSE,
		      COLUMN_WEIGHT, PANGO_WEIGHT_BOLD, -1);

  
  /* The LDAP browser in the second part of the GtkHPaned */
  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_paned_add2 (GTK_PANED (hpaned), vbox2);  
  
  /* Each page of the GtkNotebook contains a list of the users */
  aw->aw_notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (aw->aw_notebook), 0);
  gtk_box_pack_start (GTK_BOX (vbox2), aw->aw_notebook, 
		      TRUE, TRUE, 0);

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
    gtk_menu_item_new_with_label (_("First name contains"));
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  menu_item = gtk_menu_item_new_with_label (_("Last name contains"));
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  menu_item = gtk_menu_item_new_with_label (_("E-mail contains"));
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  GtkWidget *option_menu = gtk_option_menu_new ();
  gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu),
                            menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (option_menu),
                               0);
  gtk_box_pack_start (GTK_BOX (hbox), option_menu, FALSE, FALSE, 2);

  /* The entry */
  search_entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), search_entry, TRUE, TRUE, 2);
  gtk_widget_set_sensitive (GTK_WIDGET (search_entry), FALSE);

  /* The Find button */
  find_button = gtk_button_new_from_stock (GTK_STOCK_FIND);
  gtk_box_pack_start (GTK_BOX (hbox), find_button, FALSE, FALSE, 2);
  gtk_widget_show_all (handle);

  
  /* The statusbar */
  statusbar = gtk_statusbar_new ();
  gtk_box_pack_start (GTK_BOX (main_vbox), statusbar, FALSE, FALSE, 0);
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar), FALSE);


  g_signal_connect (G_OBJECT (selection), "changed",
                    G_CALLBACK (addressbook_changed_cb), 
                    aw->aw_notebook);

  /*
     gtk_drag_dest_set (GTK_WIDGET (lw->tree_view),
     GTK_DEST_DEFAULT_ALL,
     dnd_targets, 1,
     GDK_ACTION_COPY);
     g_signal_connect (G_OBJECT (lw->tree_view), "drag_motion",
		    G_CALLBACK (dnd_drag_motion_cb), 0);
  g_signal_connect (G_OBJECT (lw->tree_view), "drag_data_received",
		    G_CALLBACK (dnd_drag_data_received_cb), 0);
  
  g_signal_connect (G_OBJECT (lw->tree_view), "row_activated",
		    G_CALLBACK (contact_section_activated_cb), NULL);  

  g_signal_connect (G_OBJECT (selection), "changed",
		    G_CALLBACK (contact_section_changed_cb), NULL);
  g_signal_connect_object (G_OBJECT (lw->tree_view), "event_after",
			   G_CALLBACK (contact_section_clicked_cb), 
			   NULL, (GConnectFlags) 0);

  g_signal_connect (G_OBJECT (window), "delete_event",
                    G_CALLBACK (delete_window_cb), NULL);
 */ 
  /* Add the various address books */
  addressbooks = gnomemeeting_get_local_addressbooks ();
  l = addressbooks;
  while (l) {

    g_warning ("Change in gnomemeeting_aw_add_addressbook ");

    gnomemeeting_aw_add_tree_view_section (window, GM_ADDRESSBOOK (l->data), p);
    gnomemeeting_aw_add_notebook_page (window, GM_ADDRESSBOOK (l->data), p);

    p++;
    l = g_slist_next (l);
  }
  g_slist_free (l);

  gtk_widget_show_all (GTK_WIDGET (main_vbox));
  

  return window;
}
