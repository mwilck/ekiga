
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
 *   copyright            : (C) 2000-2002 by Damien Sandras & Miguel Rodríguez
 *   description          : This file contains functions to build the ldap
 *                          window.
 *   email                : dsandras@seconix.com
 *                          migrax@terra.es
 *
 */


#include "../config.h"

#include "ldap_window.h"
#include "ils.h"
#include "config.h"
#include "gnomemeeting.h"
#include "videograbber.h"
#include "common.h"
#include "main_window.h"
#include "menu.h"
#include "misc.h"

#include "../pixmaps/ldap_refresh.xpm"
#include "../pixmaps/small-close.xpm"


/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

static void ldap_window_clicked (GnomeDialog *, int, gpointer);
static void search_entry_modified (GtkWidget *, gpointer);
static void refresh_button_clicked (GtkButton *, gpointer);
static void apply_filter_button_clicked (GtkButton *, gpointer);
static void ldap_clist_row_selected (GtkWidget *, gint, gint,
				     GdkEventButton *, gpointer);
static void ldap_clist_column_clicked (GtkCList *, gint, gpointer);
static void ldap_page_close_button_clicked (GtkWidget *, gpointer);
static void gnomemeeting_init_ldap_window_notebook (int, gchar *);


/* GTK Callbacks */

/* DESCRIPTION  :  This callback is called when the user clicks
 *                 closes the window (destroy or delete_event signals).
 * BEHAVIOR     :  Hide the window.
 * PRE          :  gpointer is a valid pointer to a GM_ldap_window_widgets.
 */
void ldap_window_clicked (GnomeDialog *widget, int button, gpointer data)
{
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) data;

  if (GTK_WIDGET_VISIBLE (lw->gw->ldap_window))
    gtk_widget_hide_all (lw->gw->ldap_window);
}


/* DESCRIPTION  :  This callback is called when the user has modified
 *                 the search entry field.
 * BEHAVIOR     :  Reset the search counter.
 * PRE          :  gpointer is a valid pointer to a GM_ldap_window_widgets.
 */
void search_entry_modified (GtkWidget *widget, gpointer data)
{
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) data;
  GtkWidget *page, *clist;
  int current_page;

  current_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (lw->notebook));

  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook),
				    current_page);
  clist = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (page), "ldap_users_clist"));
  gtk_object_set_data (GTK_OBJECT (clist),
		       "last_selected_row", GINT_TO_POINTER (-1));
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a column of the users list.
 * BEHAVIOR     :  Reorder the column using an ascending or descending sort.
 * PRE          :  gpointer is a valid pointer to a GM_ldap_window_widgets
 */
void ldap_clist_column_clicked (GtkCList *clist, gint column, gpointer data)
{
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) data;  
  int current_page, sorted_column, sorted_order;

  current_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (lw->notebook));
  sorted_column = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (clist),
							"sorted_column"));
  sorted_order = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (clist),
							"sorted_order"));

  if (column > 1) {

      if (sorted_column != column) {
	  gtk_clist_set_sort_column (GTK_CLIST (clist), column);
	  gtk_clist_set_sort_type (GTK_CLIST (clist), GTK_SORT_ASCENDING);
	  gtk_clist_sort (GTK_CLIST (clist));
	  gtk_object_set_data (GTK_OBJECT (clist), "sorted_column",
			       GINT_TO_POINTER (column));
	  gtk_object_set_data (GTK_OBJECT (clist), "sorted_order",
			       GINT_TO_POINTER (0));
      }
      else {
	if (sorted_order == 0) {
	  gtk_object_set_data (GTK_OBJECT (clist), "sorted_order",
			       GINT_TO_POINTER (1));
	  gtk_clist_set_sort_type (GTK_CLIST (clist), GTK_SORT_DESCENDING);
	  gtk_clist_sort (GTK_CLIST (clist));
	}
	else {
	  gtk_object_set_data (GTK_OBJECT (clist), "sorted_order",
			       GINT_TO_POINTER (0));
	  gtk_clist_set_sort_type (GTK_CLIST (clist), GTK_SORT_ASCENDING);
	  gtk_clist_sort (GTK_CLIST (clist));
	}	      
      }
  }
  
  gtk_object_set_data (GTK_OBJECT (clist), "last_selected_row",
		       GINT_TO_POINTER (0));
}


/* DESCRIPTION  :  This callback is called when the user selects
 *                 a row in the users list.
 * BEHAVIOR     :  Simply selects the row and put the selected row number in a
 *                 variable.
 * PRE          :  gpointer is a valid pointer to a GM_ldap_window_widgets
 */
void ldap_clist_row_selected (GtkWidget *widget, gint row, gint column, 
			      GdkEventButton *event, gpointer data)
{
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) data;
  int current_page;

  current_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (lw->notebook));
  gtk_object_set_data (GTK_OBJECT (widget), "last_selected_row", 
		       GINT_TO_POINTER (row));
}


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the refresh button.
 * BEHAVIOR     :  If there is no fetch results thread, calls GM_ldap_populate
 *                 in a new thread to browse the ILS directory.
 * PRE          :  gpointer is a valid pointer to a GM_ldap_window_widgets.
 */
void refresh_button_clicked (GtkButton *button, gpointer data)
{
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) data;
  GMH323EndPoint *endpoint = MyApp->Endpoint ();
  GMILSClient *ils_client = (GMILSClient *) endpoint->GetILSClient ();

  GtkWidget *page;
  GtkWidget *label;

  int page_num = 0;
  int found = 0;
  gchar *text_label, *ldap_server, *entry_content;
  
  lw->thread_count++;

  /* we make a dup, because the entry text will change */
  entry_content = g_strdup (gtk_entry_get_text 
    (GTK_ENTRY (GTK_COMBO (lw->ils_server_combo)->entry)));  

  /* if it is an empty entry_content, return */
  if (!g_strcasecmp (entry_content, "")) {
      lw->thread_count = 0;
      g_free (entry_content);
      return;
  }


  /* Put the current entry in the history of the combo */
  gnomemeeting_history_combo_box_add_entry (GTK_COMBO (lw->ils_server_combo),
					    "/apps/gnomemeeting/history/ldap_servers",
					    entry_content);
					    
  /* if we are not already browsing */
  if (lw->thread_count == 1) {

    /* browse all the notebook pages */
    while ((page = 
	    gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook),
				       page_num)) != NULL) {

      label = GTK_WIDGET 
	(g_list_first (gtk_container_children (GTK_CONTAINER
					       (gtk_notebook_get_tab_label 
						(GTK_NOTEBOOK (lw->notebook), 
						 page))))->data);
      gtk_label_get (GTK_LABEL (label), &text_label);
      ldap_server = gtk_entry_get_text 
	(GTK_ENTRY (GTK_COMBO (lw->ils_server_combo)->entry));
      
      /* if there is a page with the current ils server, that's ok */
      if (!g_strcasecmp (text_label, ldap_server)) {

	found = 1;
	break;
      }
      
      page_num++;
    }

    if (!found) {
      gnomemeeting_init_ldap_window_notebook (page_num, ldap_server);
      
      /* if it was the first "No directory" page, destroy it */
      if ((page_num == 1)&&(!g_strcasecmp (_("No directory"), text_label))) 
	gtk_notebook_remove_page (GTK_NOTEBOOK (lw->notebook), 0);
    }
    else
      gtk_notebook_set_page (GTK_NOTEBOOK (lw->notebook), page_num);

    GtkWidget *page, *clist;
    page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), 
				      gtk_notebook_get_current_page (GTK_NOTEBOOK
								     (lw->notebook)));
    clist = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (page),
					     "ldap_users_clist"));
    gtk_clist_freeze (GTK_CLIST (clist));
    gtk_clist_clear (GTK_CLIST (clist));
    gtk_clist_thaw (GTK_CLIST (clist));

    ils_client->ils_browse (page_num);
  }   
}


/* DESCRIPTION  :  This callback is called when the user clicks on the apply
 *                 filter button
 * BEHAVIOR     :  make an incremental search on the entry content
 * PRE          :  gpointer is a valid pointer to a GM_ldap_window_widgets
 */
void apply_filter_button_clicked (GtkButton *button, gpointer data)
{
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) data;

  /* should not be freed : entry is a pointer to the text part of an entry
     and text is a pointer to the text part of a clist */
  gchar *entry = NULL, *text = NULL;
  GtkWidget *active_item, *page, *clist;

  int current_page;
  int cpt = 0, col = 0;
  int last_selected_col, last_selected_row;

  active_item = gtk_menu_get_active (GTK_MENU (GTK_OPTION_MENU 
					       (lw->option_menu)->menu));

  col = g_list_index (GTK_MENU_SHELL (GTK_OPTION_MENU (lw->option_menu)
				      ->menu)->children, 
		      active_item)
        + 2;

  /* we will make a search on the current page */
  current_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (lw->notebook));
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), current_page);
  clist = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (page), "ldap_users_clist"));
  last_selected_col = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (clist),
							    "last_selected_col"));
  last_selected_row = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (clist),
							    "last_selected_row"));

  if (col != last_selected_col) {
    gtk_object_set_data (GTK_OBJECT (clist), "last_selected_row",
			 GINT_TO_POINTER (-1));
    last_selected_row = -1;
  }
  
  entry = gtk_entry_get_text (GTK_ENTRY (lw->search_entry));
  
  for (cpt = last_selected_row + 1; cpt < GTK_CLIST (clist)->rows; cpt++) {
    gtk_clist_get_text (GTK_CLIST (clist), cpt, col, &text);
    
    if (text && !strcasecmp (entry, text)) {

      gtk_clist_select_row (GTK_CLIST (clist), cpt, col);
      gtk_object_set_data (GTK_OBJECT (clist), "last_selected_row",
			   GINT_TO_POINTER (cpt));
      gtk_object_set_data (GTK_OBJECT (clist), "last_selected_col",
			   GINT_TO_POINTER (col));
      
      gtk_clist_moveto (GTK_CLIST (clist), cpt, 0, 0, 0);
      
      break;
    }
  }
}

/* DESCRIPTION  :  This callback is called when the user clicks the close
 *                 on any notebook tab
 * BEHAVIOR     :  remove the tab from the notebook
 * PRE          :  gpointer is a valid pointer to a GM_ldap_window_widgets
 */
static void ldap_page_close_button_clicked (GtkWidget *button, gpointer data)
{
  int page_number;
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) data;
  GtkWidget *page = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (button), "page"));

  page_number = gtk_notebook_page_num (GTK_NOTEBOOK (lw->notebook), page);

  /* We do not remove the first page if it is the only one */
  if (!((gtk_notebook_get_current_page (GTK_NOTEBOOK (lw->notebook)) == 0)&&
	(gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), 1) == NULL)))
    gtk_notebook_remove_page (GTK_NOTEBOOK (lw->notebook), page_number);
}


/* The functions */

void gnomemeeting_init_ldap_window ()
{
  GtkWidget *table, *entry_table;
  GtkWidget *vbox;
  GtkWidget *frame;
  GtkWidget *apply_filter_button;
  GtkWidget *who_pixmap;
  GtkWidget *menu;
  GtkWidget *menu_item;

  /* Get the structs from the application */
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);
  GM_ldap_window_widgets *lw = gnomemeeting_get_ldap_window (gm);

  lw->thread_count = 0;
  lw->gw = gw;

  who_pixmap =  gnome_pixmap_new_from_xpm_d ((char **) ldap_refresh_xpm);

  lw->gw->ldap_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_policy (GTK_WINDOW (lw->gw->ldap_window), 
			 FALSE, TRUE, TRUE);
  gtk_window_set_title (GTK_WINDOW (lw->gw->ldap_window), 
			_("LDAP Server Browser"));
  gtk_window_set_position (GTK_WINDOW (lw->gw->ldap_window), 
			   GTK_WIN_POS_CENTER);
  gtk_window_set_default_size (GTK_WINDOW (lw->gw->ldap_window), 450, 350);

  /* a vbox to put the frames and the user list */
  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (lw->gw->ldap_window), vbox);

  /* ILS directories combo box */
  frame = gtk_frame_new (_("ILS directories to browse"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

  /* Put a table in that first frame */
  table = gtk_table_new (1, 3, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

  GtkWidget *label = gtk_label_new (_("ILS directory:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  lw->ils_server_combo = gtk_combo_new ();
  lw->ils_server_combo = gnomemeeting_history_combo_box_new ("/apps/gnomemeeting/"
							     "history/ldap_servers");
  gtk_combo_disable_activate (GTK_COMBO(lw->ils_server_combo));
  gtk_table_attach (GTK_TABLE (table), lw->ils_server_combo, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  lw->refresh_button = gnomemeeting_button (_("Refresh"), who_pixmap);
  gtk_widget_set_usize (GTK_WIDGET (lw->refresh_button), 120, 30);

  gtk_table_attach (GTK_TABLE (table), lw->refresh_button, 2, 3, 0, 1,
		    (GtkAttachOptions) NULL, 
		    (GtkAttachOptions) NULL,
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

  /* Search filter entry */
  entry_table = gtk_table_new (2, 10, FALSE);

  /* option menu */
  menu = gtk_menu_new ();
  menu_item = gtk_menu_item_new_with_label (_("first name"));
  gtk_menu_append (GTK_MENU (menu), menu_item);

  menu_item = gtk_menu_item_new_with_label (_("last name"));
  gtk_menu_append (GTK_MENU (menu), menu_item);

  menu_item = gtk_menu_item_new_with_label (_("e-mail"));
  gtk_menu_append (GTK_MENU (menu), menu_item);

  menu_item = gtk_menu_item_new_with_label (_("location"));
  gtk_menu_append (GTK_MENU (menu), menu_item);

  lw->option_menu = gtk_option_menu_new ();
  gtk_option_menu_set_menu (GTK_OPTION_MENU (lw->option_menu),
			    menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (lw->option_menu),
			       1);

  gtk_table_attach (GTK_TABLE (entry_table), lw->option_menu, 1, 4, 0, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  /* entry */
  lw->search_entry = gtk_entry_new ();

  gtk_table_attach (GTK_TABLE (entry_table), lw->search_entry, 4, 8, 0, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  /* the filter button */
  apply_filter_button = gtk_button_new_with_label (_("Apply filter on"));

  gtk_table_attach (GTK_TABLE (entry_table), apply_filter_button, 0, 1, 0, 2,
		    (GtkAttachOptions) (GTK_FILL), 
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);


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

  /* Put the search filter frame at the end */
  frame = gtk_frame_new (_("Search Filter"));
  gtk_container_add (GTK_CONTAINER (frame), entry_table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

  /* Status Bar */
  lw->statusbar = gnome_appbar_new (TRUE, TRUE, GNOME_PREFERENCES_NEVER);
  gtk_box_pack_end (GTK_BOX (vbox), lw->statusbar, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (lw->statusbar), 0);

  /* Signals */
  gtk_signal_connect (GTK_OBJECT (GTK_COMBO (lw->ils_server_combo)->entry), 
		      "activate",
		      GTK_SIGNAL_FUNC (refresh_button_clicked),
		      (gpointer) lw);

  gtk_signal_connect (GTK_OBJECT (lw->refresh_button), "pressed",
		      GTK_SIGNAL_FUNC (refresh_button_clicked), 
		      (gpointer) lw);

  gtk_signal_connect (GTK_OBJECT (apply_filter_button), "pressed",
		      GTK_SIGNAL_FUNC (apply_filter_button_clicked), 
		      (gpointer) lw);

  gtk_signal_connect (GTK_OBJECT(lw->search_entry), "changed",
		      GTK_SIGNAL_FUNC(search_entry_modified), (gpointer) lw);

  gtk_signal_connect (GTK_OBJECT(gw->ldap_window), "delete_event",
		     GTK_SIGNAL_FUNC(ldap_window_clicked), (gpointer) lw);

  gtk_signal_connect (GTK_OBJECT (gw->ldap_window), "destroy",
		      GTK_SIGNAL_FUNC (ldap_window_clicked), (gpointer) lw);
}


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Build the notebook inside the LDAP window.
 * PRE          :  The current page and the server name.
 */
void gnomemeeting_init_ldap_window_notebook (int page_num, gchar *text_label)
{
  GtkWidget *clist, *page;
  GtkWidget *label;
  GtkWidget *scroll;
  GtkWidget *close_button;
  GtkWidget *hbox;
  GdkPixbuf *close_cross;
  GtkPixmap *close_gtkpixmap;
  GdkPixmap *close_pixmap;
  GdkBitmap *close_bitmap;

  GM_ldap_window_widgets *lw = gnomemeeting_get_ldap_window (gm);
  
  /* FIXME: How about an icons instead of text for audio and video
     here and a tick/cross in the list? */
  gchar * clist_titles [] = 
    {
     /* Translators: This is as in "Audio". */
     N_("A"),
     /* Translators: This is as in "Video". */
     N_("V"),
     N_("First Name"), N_("Last name"), N_("E-mail"), 
     N_("Location"), N_("Comment"), N_("Version"), N_("IP")};

  /* Intl */
  for (int i = 0 ; i < 9 ; i++)
    clist_titles [i] = gettext (clist_titles [i]);

  clist = gtk_clist_new_with_titles (9, clist_titles);
  gtk_object_set_data (GTK_OBJECT (clist), "last_selected_row",
		       GINT_TO_POINTER (-1));

  scroll = gtk_scrolled_window_new (NULL, NULL);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), 
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);

  for (int i = 0; i < 10; i++)
    gtk_clist_set_column_auto_resize (GTK_CLIST (clist), i, true);

  gtk_clist_set_shadow_type (GTK_CLIST (clist), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (scroll), clist);
  gtk_container_set_border_width (GTK_CONTAINER (clist), GNOME_PAD_SMALL);

  /* The popup menu */
  gnomemeeting_init_ldap_window_popup_menu (clist);

  /* The page's "label" */
  hbox = gtk_hbox_new (false, 0);
  label = gtk_label_new (text_label);
  close_button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
  close_cross = gdk_pixbuf_new_from_xpm_data (small_close_xpm);
  gdk_pixbuf_render_pixmap_and_mask (close_cross, &close_pixmap, 
				     &close_bitmap, 127);
  gdk_pixbuf_unref (close_cross);
  close_gtkpixmap = GTK_PIXMAP (gtk_pixmap_new (close_pixmap, close_bitmap));
  gtk_container_add (GTK_CONTAINER (close_button), GTK_WIDGET (close_gtkpixmap));
  gtk_widget_show_all (close_button);

  gtk_box_pack_start (GTK_BOX (hbox), label, true, true, 0);
  gtk_box_pack_end (GTK_BOX (hbox), close_button, false, false, 0);

  gtk_widget_show (label);
  gtk_widget_show (close_button);
  gtk_widget_show (hbox);

  /* Append the page to the notebook */
  gtk_widget_show (clist);
  gtk_widget_show (scroll);
  gtk_notebook_append_page (GTK_NOTEBOOK (lw->notebook), scroll, hbox);

  gtk_signal_connect (GTK_OBJECT (clist), 
		      "select_row",
		      GTK_SIGNAL_FUNC (ldap_clist_row_selected), 
		      (gpointer) lw);

  gtk_signal_connect (GTK_OBJECT (clist), 
		      "click-column",
		      GTK_SIGNAL_FUNC (ldap_clist_column_clicked), 
		      (gpointer) lw);

  gtk_signal_connect (GTK_OBJECT (close_button),
		      "clicked",
		      GTK_SIGNAL_FUNC (ldap_page_close_button_clicked),
		      (gpointer) lw);

  gtk_notebook_set_page (GTK_NOTEBOOK (lw->notebook), page_num);
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), page_num);
  gtk_object_set_data (GTK_OBJECT (page), "ldap_users_clist", clist);

  /* We need this to identify the page when the user clicks the
     close button */
  gtk_object_set_data (GTK_OBJECT (close_button), "page", page);
}
