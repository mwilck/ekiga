
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

#include "stock-icons.h"
#include "gm_contacts-eds.h"
#include "gm_conf.h"


struct GmAddressbookWindow_ {

  GtkWidget *aw_tree_view;
};
typedef struct GmAddressbookWindow_ GmAddressbookWindow;


/* The different cell renderers for the different contacts sections (servers
   or groups */
enum {

  COLUMN_PIXBUF,
  COLUMN_CONTACT_SECTION_NAME,
  COLUMN_NOTEBOOK_PAGE,
  COLUMN_PIXBUF_VISIBLE,
  COLUMN_WEIGHT,
  NUM_COLUMNS_CONTACTS
};


static void
gnomemeeting_aw_update_addressbooks_list (GtkWidget *addressbook_window)
{
  GmAddressbookWindow *aw = NULL;
  
  GdkPixbuf *contact_icon = NULL;

  GSList *addressbooks = NULL;
  GSList *l = NULL;

  GtkTreeIter iter, child_iter;
  GtkTreeModel *model = NULL;

  aw = 
    (GmAddressbookWindow *) g_object_get_data (G_OBJECT (addressbook_window), 
                                               "GMObject");
  
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->aw_tree_view));
  
  addressbooks = gnomemeeting_get_local_addressbooks ();
  

  /* Update the local address books list */
  gtk_tree_store_clear (GTK_TREE_STORE (model));
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  contact_icon = 
    gtk_widget_render_icon (aw->aw_tree_view, GM_STOCK_LOCAL_CONTACT,
			    GTK_ICON_SIZE_MENU, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter,
		      COLUMN_CONTACT_SECTION_NAME, _("On This Computer"), 
		      COLUMN_NOTEBOOK_PAGE, 0, 
		      COLUMN_PIXBUF_VISIBLE, FALSE,
		      COLUMN_WEIGHT, PANGO_WEIGHT_BOLD, -1);
  l = addressbooks;
  while (l) {

    gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (model),
			&child_iter, 
			COLUMN_PIXBUF, contact_icon,
			COLUMN_CONTACT_SECTION_NAME, 
                        ((GmAddressbook *) (l->data))->name,
			COLUMN_NOTEBOOK_PAGE, 0, 
			COLUMN_PIXBUF_VISIBLE, TRUE,
			COLUMN_WEIGHT, PANGO_WEIGHT_NORMAL, -1);


    l = g_slist_next (l);
  }
}


GtkWidget *
gnomemeeting_addressbook_window_new ()
{
  GmAddressbookWindow *aw = NULL;
  
  GtkWidget *window = NULL;
  GtkWidget *hpaned = NULL;
  
  GtkWidget *vbox = NULL;
  GtkWidget *vbox2 = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *menubar = NULL;
  GtkWidget *scroll = NULL;
  GdkPixbuf *icon = NULL;

  GtkCellRenderer *cell = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkAccelGroup *accel = NULL;
  GtkTreeStore *model = NULL;
  
  
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
  
  
  /* A vbox that will contain the menubar, and also the hbox containing
     the rest of the window */
  vbox = gtk_vbox_new (0, FALSE);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  menubar = gtk_menu_bar_new ();
 /* 
  static MenuEntry addressbook_menu [] =
    {
      GTK_MENU_NEW(_("_File")),

      GTK_MENU_ENTRY("new_server", _("New _Server"), NULL,
		     GM_STOCK_REMOTE_CONTACT, 0,
		     GTK_SIGNAL_FUNC (new_contact_section_cb),
		     GINT_TO_POINTER (CONTACTS_SERVERS), TRUE),
      GTK_MENU_ENTRY("new_group", _("New _Group"), NULL,
		     GM_STOCK_LOCAL_CONTACT, 0, 
		     GTK_SIGNAL_FUNC (new_contact_section_cb),
		     GINT_TO_POINTER (CONTACTS_GROUPS), TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("delete", _("_Delete"), NULL,
		     GTK_STOCK_DELETE, 'd', 
		     GTK_SIGNAL_FUNC (delete_cb), NULL, FALSE),

      GTK_MENU_ENTRY("rename", _("_Rename"), NULL,
		     NULL, 0,
		     GTK_SIGNAL_FUNC (modify_contact_section_cb),
		     GINT_TO_POINTER (1), TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("close", _("_Close"), NULL,
		     GTK_STOCK_CLOSE, 'w',
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) window, TRUE),

      GTK_MENU_NEW(_("C_ontact")),

      GTK_MENU_ENTRY("call", _("C_all Contact"), NULL,
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (call_user_cb), NULL, FALSE),
      GTK_MENU_ENTRY("transfer", _("_Tranfer Call to Contact"), NULL,
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (call_user_cb), NULL, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("new_contact", _("New _Contact"), NULL,
		     GTK_STOCK_NEW, 'n', 
		     GTK_SIGNAL_FUNC (new_contact_cb), 
		     NULL, TRUE),
      GTK_MENU_ENTRY("add", _("Add Contact to _Address Book"), NULL,
		     GTK_STOCK_ADD, 0,
		     GTK_SIGNAL_FUNC (edit_contact_cb), 
		     NULL, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("properties", _("Contact _Properties"), NULL,
		     GTK_STOCK_PROPERTIES, 0, 
		     GTK_SIGNAL_FUNC (edit_contact_cb), 
		     NULL, FALSE),

      GTK_MENU_END
    };

  gtk_build_menu (menubar, addressbook_menu, accel, NULL);
  lw->main_menu = menubar;
  */
  gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);

  
  /* A hpaned to put the tree and the ldap browser */
  hpaned = gtk_hpaned_new ();
  gtk_container_set_border_width (GTK_CONTAINER (hpaned), 6);
  gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 0);
  

  /* The GtkTreeView that will store the contacts sections */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_paned_add1 (GTK_PANED (hpaned), frame);
  model = gtk_tree_store_new (NUM_COLUMNS_CONTACTS, GDK_TYPE_PIXBUF, 
			      G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN,
			      G_TYPE_INT);

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
  gtk_tree_view_column_set_attributes (column, cell, "pixbuf", COLUMN_PIXBUF, 
				       "visible", COLUMN_PIXBUF_VISIBLE, NULL);

  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_set_attributes (column, cell, "text", 
				       COLUMN_CONTACT_SECTION_NAME, NULL);
  gtk_tree_view_column_add_attribute (column, cell, "weight", 
				      COLUMN_WEIGHT);
  gtk_tree_view_append_column (GTK_TREE_VIEW (aw->aw_tree_view),
			       GTK_TREE_VIEW_COLUMN (column));
  
  gnomemeeting_aw_update_addressbooks_list (window);


  /* a vbox to put the frames and the user list */
  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_paned_add2 (GTK_PANED (hpaned), vbox2);  

  /*
   * lw->notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (lw->notebook), 0);
  gtk_box_pack_start (GTK_BOX (vbox2), lw->notebook, 
		      TRUE, TRUE, 0);

  
  gnomemeeting_addressbook_sections_populate ();


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
  gtk_widget_show_all (GTK_WIDGET (vbox));
  

  return window;
}
