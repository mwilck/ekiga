
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
 *                         ldap_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Wed Feb 28 2001
 *   copyright            : (C) 2000-2003 by Damien Sandras 
 *   description          : This file contains functions to build the 
 *                          addressbook window.
 *
 */

#include "../config.h"

#include "ldap_window.h"
#include "gnomemeeting.h"
#include "ils.h"
#include "menu.h"
#include "callbacks.h"
#include "dialog.h"
#include "stock-icons.h"

#include "../pixmaps/xdap-directory.xpm"


/* Declarations */
extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	


/* Callbacks: Drag and drop management */
static gboolean dnd_drag_motion_cb (GtkWidget *,
				    GdkDragContext *,
				    int,
				    int,
				    guint,
				    gpointer);

static void dnd_drag_data_received_cb (GtkWidget *,
				       GdkDragContext *,
				       int,
				       int,
				       GtkSelectionData *,
				       guint,
				       guint,
				       gpointer);

static void dnd_drag_data_get_cb (GtkWidget *,
				  GdkDragContext *,
				  GtkSelectionData *,
				  guint,
				  guint,
				  gpointer);

/* Callbacks: Operations on a contact */
static void groups_list_store_toggled (GtkCellRendererToggle *cell,
				       gchar *path_str,
				       gpointer data);

static void edit_contact_cb (GtkWidget *,
			     gpointer);

static void delete_contact_from_group_cb (GtkWidget *,
					  gpointer);

static gint contact_clicked_cb (GtkWidget *,
				GdkEventButton *,
				gpointer);

/* Callbacks: Operations on contact sections */
static void new_contact_section_cb (GtkWidget *,
				    gpointer);

static void delete_contact_section_cb (GtkWidget *,
				       gpointer);

static gint contact_section_clicked_cb (GtkWidget *,
					GdkEventButton *,
					gpointer);

/* Callbacks: Misc */
static void delete_cb (GtkWidget *,
		       gpointer);

static void call_user_cb (GtkWidget *,
			  gpointer);

static void contact_activated_cb (GtkTreeView *,
				  GtkTreePath *,
				  GtkTreeViewColumn *,
				  gpointer);

static void contact_section_activated_cb (GtkTreeView *,
					  GtkTreePath *,
					  GtkTreeViewColumn *);

static void refresh_server_content_cb (GtkWidget *,
				       gpointer);

/* Local functions: Operations on a contact */
static gboolean is_contact_member_of_group (gchar *,
					    gchar *);

static GSList *find_contact_in_group_content (gchar *,
					      GSList *);

static gboolean get_selected_contact_info (GtkNotebook *,
					   gchar ** = NULL,
					   gchar ** = NULL,
					   gchar ** = NULL,
					   gchar ** = NULL,
					   gboolean * = NULL);

/* Misc */
static void notebook_page_destroy (gpointer data);


/* GTK Callbacks */

/* DESCRIPTION  :  This callback is called when the user moves the drag.
 * BEHAVIOR     :  Draws a rectangle around the groups in which the user info
 *                 can be dropped.
 * PRE          :  /
 */
gboolean
dnd_drag_motion_cb (GtkWidget *tree_view,
		    GdkDragContext *context,
		    int x,
		    int y,
		    guint time,
		    gpointer data)		     
{
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;

  gchar *group_name = NULL;
  gchar *contact_name = NULL;
  gchar *contact_section = NULL;
  gchar *contact_callto = NULL;

  gboolean is_group;
  
  GmLdapWindow *lw = NULL;
  
  GValue value =  {0, };
  GtkTreeIter iter;

  lw = gnomemeeting_get_ldap_window (gm);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

  /* Get the callto field of the contact info from the source GtkTreeView */
  if (get_selected_contact_info (GTK_NOTEBOOK (lw->notebook), &contact_section,
				 &contact_name, &contact_callto, NULL,
				 &is_group)
      && is_group) {
    

    /* See if the path in the destination GtkTreeView corresponds to a valid
       row (ie a group row, and a row corresponding to a group the user
       doesn't belong to */
    if (gtk_tree_view_get_dest_row_at_pos (GTK_TREE_VIEW (tree_view),
					   x, y, &path, NULL)) {

      if (gtk_tree_model_get_iter (model, &iter, path)) {
	    
	gtk_tree_model_get_value (model, &iter, 
				  COLUMN_CONTACT_SECTION_NAME, &value);
	group_name = g_strdup (g_value_get_string (&value));
	g_value_unset (&value);


	/* If the user doesn't belong to the selected group and if
	   the selected row corresponds to a group and not a server */
	
	if (gtk_tree_path_get_depth (path) >= 2 &&
	    gtk_tree_path_get_indices (path) [0] >= 1 
	    && group_name && contact_callto &&
	    !is_contact_member_of_group (contact_callto, group_name)) {
    
	  gtk_tree_view_set_drag_dest_row (GTK_TREE_VIEW (tree_view),
					   path,
					   GTK_TREE_VIEW_DROP_INTO_OR_AFTER);
	}
	
	g_free (group_name);
	gtk_tree_path_free (path);
	gdk_drag_status (context, GDK_ACTION_COPY, time);
      }
    } 
  }
  else
    return false;
  
  return true;
}


/* DESCRIPTION  :  This callback is called when the user has released the drag.
 * BEHAVIOR     :  Adds the user gconf key of the group where the drop occured.
 * PRE          :  /
 */
void
dnd_drag_data_received_cb (GtkWidget *tree_view,
			   GdkDragContext *context,
			   int x,
			   int y,
			   GtkSelectionData *selection_data,
			   guint info,
			   guint time,
			   gpointer data)
{
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;

  GtkTreeIter iter;

  gchar **contact_info = NULL;
  gchar *group_name = NULL;
  gchar *gconf_key = NULL;

  GSList *group_content = NULL;

  GValue value = {0, };
  
  GConfClient *client = NULL;

  client = gconf_client_get_default ();


  /* Get the path at the current position in the destination GtkTreeView
     so that we know to what group it corresponds. Once we know the
     group, we can update the gconf key of that group with the data received */
  if (gtk_tree_view_get_dest_row_at_pos (GTK_TREE_VIEW (tree_view),
					 x, y, &path, NULL)) {

    if (gtk_tree_path_get_depth (path) >= 2 &&
	gtk_tree_path_get_indices (path) [0] >= 1) {

      model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

      if (gtk_tree_model_get_iter (model, &iter, path)) {
      
	gtk_tree_model_get_value (model, &iter, 
				  COLUMN_CONTACT_SECTION_NAME, &value);
	group_name = g_strdup (g_value_get_string (&value));
	g_value_unset (&value);

	if (group_name && selection_data && selection_data->data) {

	  contact_info = g_strsplit ((char *) selection_data->data, "|", 0);

	  if (contact_info [1] &&
	      !is_contact_member_of_group (contact_info [1], group_name)) {

	    gconf_key = 
	      g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, 
			       (char *) group_name);
	    
	    group_content = 
	      gconf_client_get_list (client, gconf_key, GCONF_VALUE_STRING,
				     NULL);
	    
	    group_content =
	      g_slist_append (group_content, (char *) selection_data->data);
	    
	    gconf_client_set_list (client, gconf_key, GCONF_VALUE_STRING,
				   group_content, NULL);
	  
	    g_slist_free (group_content);
	    g_free (gconf_key);
	  }

	  g_strfreev (contact_info);
	}
      }
    }
    
    gtk_tree_path_free (path);
  } 
}
  

/* DESCRIPTION  :  This callback is called when the user has released the drag.
 * BEHAVIOR     :  Puts the required data into the selection_data, we put
 *                 name and the callto fields for now.
 * PRE          :  data = the type of the page from where the drag occured :
 *                 CONTACTS_GROUPS or CONTACTS_SERVERS.
 */
void
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
  gchar *contact_callto = NULL;
  gchar *drag_data = NULL;


  model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    if (GPOINTER_TO_INT (data) == CONTACTS_SERVERS)
      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			  COLUMN_ILS_NAME, &contact_name, 
			  COLUMN_ILS_CALLTO, &contact_callto,
			  -1);
    else
      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			  COLUMN_NAME, &contact_name, 
			  COLUMN_CALLTO, &contact_callto,
			  -1);

    if (contact_name && contact_callto) {
      
      drag_data = g_strdup_printf ("%s|%s", contact_name, contact_callto);
      
      gtk_selection_data_set (selection_data, selection_data->target, 
			      8, (const guchar *) drag_data,
			      strlen (drag_data));
      g_free (drag_data);
    }
  }
}


/* DESCRIPTION  :  This callback is called when the user toggles a group in
 *                 the groups list in the popup permitting to edit
 *                 the properties of a contact.
 * BEHAVIOR     :  Update the toggles for that group.
 * PRE          :  data = a GtkTreeModel containing the groups list.
 */
static void
groups_list_store_toggled (GtkCellRendererToggle *cell,
			   gchar *path_str,
			   gpointer data)
{
  GtkTreeIter iter;
  GtkTreeModel *model = GTK_TREE_MODEL (data);
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  
  gboolean member_of_group = false;

  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, 0, &member_of_group, -1);
  
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0,
		      !member_of_group, -1);
}


/* DESCRIPTION  :  This callback is called when the user chooses to edit the
 *                 info of a contact from a group or to add an user.
 * BEHAVIOR     :  Opens a popup, and save the modified info by modifying the
 *                 gconf key when the user clicks on "ok".
 * PRE          :  If data = 1, then we add a new user, no need to see 
 *                 if something is selected and needs to be edited.
 */
void
edit_contact_cb (GtkWidget *widget,
		 gpointer data)
{
  GConfClient *client = NULL;

  gchar *contact_name = NULL;
  gchar *contact_speed_dial = NULL;
  gchar *contact_callto = NULL;
  gchar *gconf_key = NULL;
  gchar *contact_info = NULL;
  gchar *old_contact_callto = NULL;
  gchar *contact_section = NULL;
  gchar *group_name = NULL;
  gchar *label_text = NULL;
    
  PString other_speed_dial;

  GtkWidget *dialog = NULL;
  GtkWidget *table = NULL;
  GtkWidget *label = NULL;
  GtkWidget *name_entry = NULL;
  GtkWidget *callto_entry = NULL;
  GtkWidget *speed_dial_entry = NULL;
  GtkWidget *frame = NULL;

  GtkListStore *list_store = NULL;
  GtkWidget *tree_view = NULL;
  GtkCellRenderer *renderer = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkTreeIter iter;

  GSList *groups_list = NULL;
  GSList *groups_list_iter = NULL;
  
  gboolean is_group = false;
  gboolean selected = false;
  gboolean valid_answer = false;
  int result = 0;
  int groups_nbr = 0;
  
  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;

  GmLdapWindow *lw = NULL;
  GmWindow *gw = NULL;
    
  lw = gnomemeeting_get_ldap_window (gm);
  gw = gnomemeeting_get_main_window (gm);
  
  client = gconf_client_get_default ();


  /* We don't care if it fails as long as contact_section and is_group
     are correct */
  get_selected_contact_info (GTK_NOTEBOOK (lw->notebook),
			     &contact_section, &contact_name,
			     &contact_callto, &contact_speed_dial,
			     &is_group);


  /* If we edit the user in the group */
  if (GPOINTER_TO_INT (data) != 1) {
    
    /* Store the old callto to be able to delete the user later */
    old_contact_callto = g_strdup (contact_callto);
  }
  else {

    /* If we add a new user, we forget what is selected */
    contact_name = NULL;
    contact_callto = NULL;
    contact_speed_dial = NULL;
  }

  
  /* Create the dialog to easily modify the info of a specific contact */
  dialog = gtk_dialog_new_with_buttons (_("Edit the contact information"), 
					GTK_WINDOW (gw->ldap_window),
					GTK_DIALOG_MODAL,
					GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
					GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
					NULL);
    
  table = gtk_table_new (4, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3 * GNOMEMEETING_PAD_SMALL);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3 * GNOMEMEETING_PAD_SMALL);
  
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  label_text = g_strdup_printf ("<b>%s</b>", _("Name:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  name_entry = gtk_entry_new ();
  if (contact_name)
    gtk_entry_set_text (GTK_ENTRY (name_entry), contact_name);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), name_entry, 1, 2, 0, 1, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
    
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  label_text = g_strdup_printf ("<b>%s</b>", _("URL:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);
  
  callto_entry = gtk_entry_new ();
  gtk_widget_set_size_request (GTK_WIDGET (callto_entry), 300, -1);
  if (contact_callto)
    gtk_entry_set_text (GTK_ENTRY (callto_entry), contact_callto);
  else
    gtk_entry_set_text (GTK_ENTRY (callto_entry), "callto://");
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), callto_entry, 1, 2, 1, 2, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  label_text = g_strdup_printf ("<b>%s</b>", _("Speed dial:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  speed_dial_entry = gtk_entry_new ();
  if (contact_speed_dial)
    gtk_entry_set_text (GTK_ENTRY (speed_dial_entry), contact_speed_dial);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), speed_dial_entry, 1, 2, 2, 3, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  /* The list store that contains the list of possible groups */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  label_text = g_strdup_printf ("<b>%s</b>", _("Groups:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  frame = gtk_frame_new (NULL);
  list_store = gtk_list_store_new (2, G_TYPE_BOOLEAN, G_TYPE_STRING);
  tree_view = 
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), FALSE);
  
  renderer = gtk_cell_renderer_toggle_new ();
  column = gtk_tree_view_column_new_with_attributes ("", renderer,
						     "active", 0, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  g_signal_connect (G_OBJECT (renderer), "toggled",
		    G_CALLBACK (groups_list_store_toggled), 
		    GTK_TREE_MODEL (list_store));

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("", renderer,
						     "text", 1, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_container_add (GTK_CONTAINER (frame), tree_view);

  gtk_table_attach (GTK_TABLE (table), frame, 1, 2, 3, 4, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  
  /* Populate the list store with available groups */
  groups_list =
    gconf_client_get_list (client, CONTACTS_KEY "groups_list",
			   GCONF_VALUE_STRING, NULL);
  groups_list_iter = groups_list;
  
  while (groups_list_iter) {
    
    if (groups_list_iter->data) {

	gtk_list_store_append (list_store, &iter);
	gtk_list_store_set (list_store, &iter,
			    0, (is_group
			    && old_contact_callto
			    && is_contact_member_of_group (old_contact_callto, (char *) groups_list_iter->data)
			    && GPOINTER_TO_INT (data) != 1),
			    1, groups_list_iter->data, -1);
    }

    groups_list_iter = g_slist_next (groups_list_iter);
  }
  g_slist_free (groups_list);

  
  /* Pack the gtk entries and the list store in the window */
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), table,
		      FALSE, FALSE, GNOMEMEETING_PAD_SMALL);
  gtk_widget_show_all (dialog);
    

  while (!valid_answer) {
    
    result = gtk_dialog_run (GTK_DIALOG (dialog));
    
    switch (result) {
      
    case GTK_RESPONSE_ACCEPT:

      /* If there is no name or an empty callto, display an error message
	 and exit */
      if (!strcmp (gtk_entry_get_text (GTK_ENTRY (name_entry)), "")
	  || !strcmp (gtk_entry_get_text (GTK_ENTRY (callto_entry)), "")
	  || !strcmp (gtk_entry_get_text (GTK_ENTRY (callto_entry)), "callto://")){
	gnomemeeting_error_dialog (GTK_WINDOW (dialog), _("Please provide a valid name and callto for the contact."));
	valid_answer = false;
      }
      else {
	
	contact_info =
	  g_strdup_printf ("%s|%s|%s",
			   gtk_entry_get_text (GTK_ENTRY (name_entry)),
			   gtk_entry_get_text (GTK_ENTRY (callto_entry)),
			   gtk_entry_get_text (GTK_ENTRY (speed_dial_entry)));
          
	contact_callto = (char *) gtk_entry_get_text (GTK_ENTRY (callto_entry));

	/* Determine the groups where we want to add the contact */
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list_store), &iter)) {
      
	  do {
	
	    gtk_tree_model_get (GTK_TREE_MODEL (list_store), &iter,
				0, &selected, 1, &group_name, -1);
	  
	    if (group_name) {
	    
	      gconf_key =
		g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, group_name);

	      group_content =
		gconf_client_get_list (client, gconf_key,
				       GCONF_VALUE_STRING, NULL);
      
	      /* Once we find the contact corresponding to the old saved callto
		 (if we are editing an existing user and not adding a new one),
		 we delete him and insert the new one at the same position;
		 if the group is not selected for that user, we delete him from
		 the group.
	      */
	      if (old_contact_callto) {
	    
		group_content_iter =
		  find_contact_in_group_content (old_contact_callto,
						 group_content);

		/* Only reinsert the contact if the group is selected for him,
		   otherwise, only delete him from the group */
		if (selected)
		  group_content =
		    g_slist_insert (group_content, (gpointer) contact_info,
				    g_slist_position (group_content,
						      group_content_iter));

		group_content = g_slist_remove_link (group_content,
						     group_content_iter);
	      }
	      else
		group_content =
		  g_slist_append (group_content, (gpointer) contact_info);

	      /* If we are adding a new user for a selected group and that he
		 already belongs to that group, then display a warning */
	      if (GPOINTER_TO_INT (data) == 1 && selected
		  && is_contact_member_of_group (contact_callto, group_name)) {
	    
		gnomemeeting_error_dialog (GTK_WINDOW (dialog), _("Another contact with the callto %s already exists in group %s."), contact_callto, group_name);
	      }
	      else {

		other_speed_dial =
		  gnomemeeting_addressbook_get_speed_dial_url (gtk_entry_get_text (GTK_ENTRY (speed_dial_entry)));
	    
		if (!other_speed_dial.IsEmpty () 
		    && other_speed_dial != PString (contact_callto)) {

		  if (selected) {
		
		    gnomemeeting_error_dialog (GTK_WINDOW (dialog), _("Another contact with the same speed dial already exists."));
		    valid_answer = false;
		  }
		}
		else
		  if ((GPOINTER_TO_INT (data) == 1 && selected)
		      || (GPOINTER_TO_INT (data) == 0)) {
		
		    gconf_client_set_list (client, gconf_key, GCONF_VALUE_STRING,
					   group_content, NULL);
		    valid_answer = true;
		  }
	      }	  

	      if (selected)
		groups_nbr++;
	      g_free (gconf_key);
	    }
	
	
	  } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list_store), &iter));

	  /* If the user had selected no groups, we display a warning */
	  if (!groups_nbr) {

	    gnomemeeting_error_dialog (GTK_WINDOW (dialog),
				       _("You have to select a group to which you want to add your contact."));
	    valid_answer = false;
	  }
	}
	g_free (contact_info);
	g_slist_free (group_content);
      }
      break;


    case GTK_RESPONSE_REJECT:
      valid_answer = true;
      break;

    }

    groups_nbr = 0;
  }
  
  g_free (old_contact_callto);
    
  if (dialog)
    gtk_widget_destroy (dialog);  
}


/* DESCRIPTION  :  This callback is called when the user chooses to delete the
 *                 a contact from a group.
 * BEHAVIOR     :  Removes the user from the gconf key updates the gconf db.
 * PRE          :  /
 */
void
delete_contact_from_group_cb (GtkWidget *widget,
			      gpointer data)
{
  GConfClient *client = NULL;
  
  gchar *contact_callto = NULL;
  gchar *contact_name = NULL;
  gchar *gconf_key = NULL;
  gchar *contact_section = NULL;

  gboolean is_group;
  
  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;
  
  GmLdapWindow *lw = NULL;

  lw = gnomemeeting_get_ldap_window (gm);
  client = gconf_client_get_default ();

  if (get_selected_contact_info (GTK_NOTEBOOK (lw->notebook),
				 &contact_section, &contact_name,
				 &contact_callto, NULL, &is_group)
      && is_group) {

    gconf_key =
      g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, contact_section);
    
    group_content =
      gconf_client_get_list (client, gconf_key, GCONF_VALUE_STRING, NULL);
    
    group_content_iter =
      find_contact_in_group_content (contact_callto, group_content);
    
    group_content = g_slist_remove_link (group_content,
					 group_content_iter);
	
    gconf_client_set_list (client, gconf_key, GCONF_VALUE_STRING,
			   group_content, NULL);
    
    g_slist_free (group_content);

    g_free (gconf_key);
  }
}


/* DESCRIPTION  :  This callback is called when there is an "event_after"
 *                 signal on one of the contacts.
 * BEHAVIOR     :  Displays a popup menu with the required options.
 * PRE          :  /
 */
static gint
contact_clicked_cb (GtkWidget *w,
			GdkEventButton *e,
			gpointer data)
{
  GmLdapWindow *lw = NULL;
  GtkWidget *menu = NULL;

  GtkTreeSelection *selection = NULL;
  GtkTreeView *tree_view = NULL;
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;
  
  GConfClient *client = NULL;

  gchar *contact_callto = NULL;
  gchar *contact_name = NULL;
  
  tree_view = GTK_TREE_VIEW (w);

  lw = gnomemeeting_get_ldap_window (gm);

  if (e->window != gtk_tree_view_get_bin_window (tree_view)) 
    return FALSE;
    
  client = gconf_client_get_default ();

  if (e->type == GDK_BUTTON_PRESS) {

    if (gtk_tree_view_get_path_at_pos (tree_view, (int) e->x, (int) e->y,
				       &path, NULL, NULL, NULL)) {

      selection = gtk_tree_view_get_selection (tree_view);
      model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

      gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);

      if (GPOINTER_TO_INT (data) == CONTACTS_GROUPS) {

	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			    COLUMN_CALLTO, &contact_callto,
			    COLUMN_NAME, &contact_name, -1);

	gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu [4].widget),
				  TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu [9].widget),
				  FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu [11].widget),
				  TRUE);
      }
      else {

	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			    COLUMN_ILS_CALLTO, &contact_callto,
			    COLUMN_ILS_NAME, &contact_name, -1);

	gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu [4].widget),
				  FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu [9].widget),
				  TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu [11].widget),
				  FALSE);
      }

      gchar *msg = g_strdup_printf (_("Add %s to Address Book"), contact_name);
      GtkWidget *child = GTK_BIN (lw->addressbook_menu [9].widget)->child;
      gtk_label_set_text (GTK_LABEL (child), msg);
			  
      if (e->button == 3 && 
	  gtk_tree_selection_path_is_selected (selection, path)) {
	
	menu = gtk_menu_new ();

	MenuEntry server_contact_menu [] =
	  {
	    {_("C_all Contact"), NULL,
	     NULL, 0, MENU_ENTRY, 
	     GTK_SIGNAL_FUNC (call_user_cb), data, NULL},

	    {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},
	    
	    {msg, NULL,
	     GTK_STOCK_ADD, 0, MENU_ENTRY,
	     GTK_SIGNAL_FUNC (edit_contact_cb),
	     GINT_TO_POINTER (0), NULL},
	    
	    {NULL, NULL, NULL, 0, MENU_END, NULL, NULL, NULL}
	  };
      
      
	MenuEntry group_contact_menu [] =
	  {
	    {_("Contact _Properties"), NULL,
	     GTK_STOCK_PROPERTIES, 0, MENU_ENTRY, 
	     GTK_SIGNAL_FUNC (edit_contact_cb), GINT_TO_POINTER (0), NULL},

	    {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},
	    
	    {_("Delete"), NULL,
	     GTK_STOCK_DELETE, 0, MENU_ENTRY,
	     GTK_SIGNAL_FUNC (delete_contact_from_group_cb),
	     NULL, NULL},
	    
	    {NULL, NULL, NULL, 0, MENU_END, NULL, NULL, NULL}
	  };
	
	if (GPOINTER_TO_INT (data) == CONTACTS_SERVERS)  
	  gnomemeeting_build_menu (menu, server_contact_menu, NULL);
	else
	  gnomemeeting_build_menu (menu, group_contact_menu, NULL);
	
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
			e->button, e->time);
	g_signal_connect (G_OBJECT (menu), "hide",
			GTK_SIGNAL_FUNC (g_object_unref), (gpointer) menu);
	g_object_ref (G_OBJECT (menu));
	gtk_object_sink (GTK_OBJECT (menu));
	
	gtk_tree_path_free (path);
	
	return TRUE;
      }
    
      g_free (msg);
    }
  }

  return FALSE;
}


/* DESCRIPTION  :  This callback is called when the user chooses to add
 *                 a new contact section, server or group.
 * BEHAVIOR     :  Opens a pop up to ask for the contact section name
 *                 and updates the right gconf key.
 * PRE          :  data = CONTACTS_GROUPS or CONTACTS_SERVERS
 */
void
new_contact_section_cb (GtkWidget *widget,
			gpointer data)
{
  GmWindow *gw = NULL;
  GtkWidget *dialog = NULL;
  GtkWidget *label = NULL;
  GtkWidget *entry = NULL;
  GConfClient *client = NULL;
  GSList *contacts_list = NULL;

  gchar *entry_text = NULL;
  gint result = 0;

  gchar *dialog_text = NULL;
  gchar *dialog_title = NULL;
  gchar *gconf_key = NULL;
  
  gw = gnomemeeting_get_main_window (gm);
  client = gconf_client_get_default ();

  if (GPOINTER_TO_INT (data) == CONTACTS_SERVERS) {

    dialog_title = g_strdup (_("Add a new server"));
    dialog_text = g_strdup (_("Enter the server name:"));
    gconf_key = g_strdup (CONTACTS_KEY "ldap_servers_list");
  }
  else {

    dialog_title = g_strdup (_("Add a new group"));
    dialog_text = g_strdup (_("Enter the group name:"));
    gconf_key = g_strdup (CONTACTS_KEY "groups_list"); 
  }
    
  dialog = gtk_dialog_new_with_buttons (dialog_title, 
					GTK_WINDOW (gw->ldap_window),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
					GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
					NULL);
  label = gtk_label_new (dialog_text);
  entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), label,
		      FALSE, FALSE, 4);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), entry,
		      FALSE, FALSE, 4);
	
  gtk_widget_show_all (dialog);
  
  result = gtk_dialog_run (GTK_DIALOG (dialog));

  switch (result) {

    case GTK_RESPONSE_ACCEPT:
	 
      entry_text = (gchar *) gtk_entry_get_text (GTK_ENTRY (entry));

      if (entry_text && strcmp (entry_text, "")) {

	PString s = PString (entry_text);
	s.Replace (" ", "_", true);

	contacts_list =
	  gconf_client_get_list (client, gconf_key, GCONF_VALUE_STRING, NULL); 

	contacts_list =
	  g_slist_append (contacts_list, (void *) (const char *) s);

	gconf_client_set_list (client, gconf_key, GCONF_VALUE_STRING, 
			       contacts_list, NULL);

	g_slist_free (contacts_list);
      }
      
      break;
  }

  g_free (dialog_title);
  g_free (dialog_text);
  g_free (gconf_key);
  gtk_widget_destroy (dialog);
}


/* DESCRIPTION  :  This callback is called when the user chooses to delete
 *                 a contact section, server or group.
 * BEHAVIOR     :  Removes the corresponding contact section from the gconf
 *                 key and updates it.
 * PRE          :  /
 */
void
delete_contact_section_cb (GtkWidget *widget,
			   gpointer data)
{
  GmLdapWindow *lw = NULL;
  
  GConfClient *client = NULL;

  GSList *contacts_list = NULL;
  GSList *contacts_list_iter = NULL;
 
  GtkTreePath *path = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;

  gchar *gconf_key = NULL;
  gchar *name = NULL;
  
  lw = gnomemeeting_get_ldap_window (gm);
  client = gconf_client_get_default ();

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (lw->tree_view));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (lw->tree_view));
  
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    
    path = gtk_tree_model_get_path (model, &iter);
    
    gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			COLUMN_CONTACT_SECTION_NAME, &name, -1);
  
    if (gtk_tree_path_get_depth (path) >= 2)
      if (gtk_tree_path_get_indices (path) [0] == 0)
	gconf_key = g_strdup (CONTACTS_KEY "ldap_servers_list");
      else
	gconf_key = g_strdup (CONTACTS_KEY "groups_list");
  
    contacts_list =
      gconf_client_get_list (client, gconf_key, GCONF_VALUE_STRING, NULL); 

  
    contacts_list_iter = contacts_list;
    while (name && contacts_list_iter) {
    
      if (!strcmp ((char *) name, (char *) contacts_list_iter->data)) {      
	contacts_list = 
	  g_slist_remove_link (contacts_list, contacts_list_iter);
	g_slist_free (contacts_list_iter);
      
	/* Only remove the selected server */
	break;
      }
  
      contacts_list_iter = contacts_list_iter->next;
    }
  
    gconf_client_set_list (client, gconf_key, GCONF_VALUE_STRING, 
			   contacts_list, NULL);
  
    gtk_tree_path_free (path);
    g_free (data);
    g_slist_free (contacts_list);
    g_free (gconf_key);
  }
}


/* DESCRIPTION  :  This callback is called when there is an "event_after"
 *                 signal on one of the contact section.
 * BEHAVIOR     :  Displays a popup menu with the required options.
 * PRE          :  /
 */
static gint
contact_section_clicked_cb (GtkWidget *w,
				GdkEventButton *e,
				gpointer data)
{
  GmLdapWindow *lw = NULL;
  GmLdapWindowPage *lwp = NULL;

  GtkWidget *menu = NULL;
  GtkWidget *page = NULL;
  
  GtkTreeSelection *selection = NULL;
  GtkTreeSelection *lselection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeView *tree_view = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;

  gchar *path_string = NULL;
  gchar *group_name = NULL;
  int page_num = -1;
  
  tree_view = GTK_TREE_VIEW (w);

  if (e->window != gtk_tree_view_get_bin_window (tree_view)) 
    return FALSE;
    
  lw = gnomemeeting_get_ldap_window (gm);
  
  if (e->type == GDK_BUTTON_PRESS) {

    if (gtk_tree_view_get_path_at_pos (tree_view, (int) e->x, (int) e->y,
				       &path, NULL, NULL, NULL)) {

      selection = gtk_tree_view_get_selection (tree_view);
      model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
      if (gtk_tree_selection_get_selected (selection, &model, &iter)) 
	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			    COLUMN_CONTACT_SECTION_NAME, &group_name, 
			    COLUMN_NOTEBOOK_PAGE, &page_num, -1);

      /* Update the sensitivity of DELETE */
      if (gtk_tree_path_get_depth (path) < 2)
	gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu [4].widget),
				  FALSE);
      else
	gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu [4].widget),
				  TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu [9].widget),
				FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu [11].widget),
				FALSE);

      /* Selectes the good notebook page for the contact section */
      if (page_num != -1) {

	/* Selects the good notebook page */
	gtk_notebook_set_current_page (GTK_NOTEBOOK (lw->notebook), 
				       page_num);
	
	/* Unselect all rows of the list store in that notebook page */
	page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), 
					  page_num);
	lwp = gnomemeeting_get_ldap_window_page (page);
	
	if (lwp->tree_view) {
	  
	  lselection =
	    gtk_tree_view_get_selection (GTK_TREE_VIEW (lwp->tree_view));
	  
	  if (lselection)
	    gtk_tree_selection_unselect_all (GTK_TREE_SELECTION (lselection));
	}
      }

      
      /* If it is a right-click, then popup a menu */
      if (e->button == 3 && 
	  gtk_tree_selection_path_is_selected (selection, path)) {
	
	menu = gtk_menu_new ();

	path_string = gtk_tree_path_to_string (path);	
	
	MenuEntry new_server_menu [] =
	  {
	    {_("New server"), NULL,
	     GTK_STOCK_NEW, 0, MENU_ENTRY, 
	     GTK_SIGNAL_FUNC (new_contact_section_cb), 
	     GINT_TO_POINTER (CONTACTS_SERVERS), NULL},
	    {NULL, NULL, NULL, 0, MENU_END, NULL, NULL, NULL}
	  };
	
	MenuEntry new_group_menu [] =
	  {
	    {_("New group"), NULL,
	     GTK_STOCK_NEW, 0, MENU_ENTRY, 
	     GTK_SIGNAL_FUNC (new_contact_section_cb), 
	     GINT_TO_POINTER (CONTACTS_GROUPS), NULL},
	    {NULL, NULL, NULL, 0, MENU_END, NULL, NULL, NULL}
	  };
	
	MenuEntry delete_refresh_contact_section_menu [] =
	  {
	    {_("_Refresh"), NULL,
	     GTK_STOCK_REFRESH, 0, MENU_ENTRY, 
	     GTK_SIGNAL_FUNC (refresh_server_content_cb), 
	     GINT_TO_POINTER (page_num), NULL},
	    {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},
	    {_("Delete"), NULL,
	     GTK_STOCK_DELETE, 0, MENU_ENTRY, 
	     GTK_SIGNAL_FUNC (delete_contact_section_cb), 
	     NULL, NULL},
	    {NULL, NULL, NULL, 0, MENU_END, NULL, NULL, NULL},
	  };
	
	MenuEntry delete_group_new_contact_section_menu [] =
	  {
	    {_("New contact"), NULL,
	     NULL, 0, MENU_ENTRY, 
	     GTK_SIGNAL_FUNC (edit_contact_cb), 
	     GINT_TO_POINTER (1), NULL},
	    {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},
	    {_("Delete"), NULL,
	     GTK_STOCK_DELETE, 0, MENU_ENTRY, 
	     GTK_SIGNAL_FUNC (delete_contact_section_cb), 
	     NULL, NULL},
	    {NULL, NULL, NULL, 0, MENU_END, NULL, NULL, NULL},
	  };


	/* Build the appropriate popup menu */
	if (gtk_tree_path_get_depth (path) >= 2)
	  if (gtk_tree_path_get_indices (path) [0] == 0)
	    gnomemeeting_build_menu (menu, delete_refresh_contact_section_menu,
				     NULL);
	  else
	    gnomemeeting_build_menu (menu,
				     delete_group_new_contact_section_menu,
				     NULL);
	else
	  if (gtk_tree_path_get_indices (path) [0] == 0) 
	    gnomemeeting_build_menu (menu, new_server_menu, NULL);
	  else
	    gnomemeeting_build_menu (menu, new_group_menu, NULL);

	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
			e->button, e->time);
	g_signal_connect (G_OBJECT (menu), "hide",
			  GTK_SIGNAL_FUNC (g_object_unref), (gpointer) menu);
	g_object_ref (G_OBJECT (menu));
	gtk_object_sink (GTK_OBJECT (menu));       

	return TRUE;
      }

      gtk_tree_path_free (path);
    }
  }

  return FALSE;
}


/* DESCRIPTION  :  This callback is called when to delete a contact section
 *                 (server or group), or to delete a contact from a group.
 * BEHAVIOR     :  If a contact is selected, then deletes the contact, if
 *                 not, then deletes the selected contact section.
 * PRE          :  /
 */
void
delete_cb (GtkWidget *w,
	   gpointer data)
{
  GmLdapWindow *lw = NULL;

  gchar *contact_section = NULL;
  gchar *contact_name = NULL;
  gchar *contact_callto = NULL;
  
  gboolean is_group = false;
  
  lw = gnomemeeting_get_ldap_window (gm);


  if (get_selected_contact_info (GTK_NOTEBOOK (lw->notebook),
				 &contact_section, &contact_name,
				 &contact_callto, NULL, &is_group)) {

    delete_contact_from_group_cb (NULL, NULL);
  }
  /* No contact is selected, but perhaps a contact section to delete
     is selected */
  else {

      delete_contact_section_cb (NULL, NULL);
  }
}


/* DESCRIPTION  :  This callback is called when the user chooses in the menu
 *                 to call.
 * BEHAVIOR     :  Calls the user of the selected line in the GtkTreeView.
 * PRE          :  /
 */
void
call_user_cb (GtkWidget *w,
	      gpointer data)
{
  contact_activated_cb (NULL, NULL, NULL, data);
}


/* DESCRIPTION  :  This callback is called when the user double clicks on
 *                 a row corresonding to an user.
 * BEHAVIOR     :  Add the user name in the combo box and call him.
 * PRE          :  data is the page type.
 */
void 
contact_activated_cb (GtkTreeView *tree,
		      GtkTreePath *path,
		      GtkTreeViewColumn *column,
		      gpointer data)
{
  gchar *contact_section = NULL;
  gchar *contact_name = NULL;
  gchar *contact_callto = NULL;

  gboolean is_group;
  
  GmLdapWindow *lw = gnomemeeting_get_ldap_window (gm);
  GmWindow *gw = gnomemeeting_get_main_window (gm);
  
  
  if (get_selected_contact_info (GTK_NOTEBOOK (lw->notebook),
				 &contact_section, &contact_name,
				 &contact_callto, NULL, &is_group)) {
    
    /* if we are waiting for a call, add the IP
       to the history, and call that user       */
    if (MyApp->Endpoint ()->GetCallingState () == 0) {
      
      /* this function will store a copy of text */
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry),
			  PString (contact_callto));
      
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->connect_button),
				    true);
    }
  }
}


/* DESCRIPTION  :  This callback is called when the user activates 
 *                 (double click) a server in the tree_store.
 * BEHAVIOR     :  Browse the selected server.
 * PRE          :  /
 */
void
contact_section_activated_cb (GtkTreeView *tree_view, 
			      GtkTreePath *path,
			      GtkTreeViewColumn *column) 
{
  int page_num = -1;
  gchar *name = NULL;

  GtkTreeIter iter;
  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;

  GmWindow *gw = NULL;
  GmLdapWindow *lw = NULL;

  gw = gnomemeeting_get_main_window (gm);
  lw = gnomemeeting_get_ldap_window (gm);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    if (path) {

      if (gtk_tree_path_get_depth (path) >= 2) {

	/* We refresh the list only if the user double-clicked on a row
	   corresponding to a server */
	if (gtk_tree_path_get_indices (path) [0] == 0) {
      
	  /* Get the server name */
	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			      COLUMN_CONTACT_SECTION_NAME, &name, 
			      COLUMN_NOTEBOOK_PAGE, &page_num, -1);

	  if (page_num != - 1) {
    
	    refresh_server_content_cb (NULL, GINT_TO_POINTER (page_num));
	  }
	}
      }
    }
  }
}


/* DESCRIPTION  :  This callback is called when the user chooses to refresh
 *                 the server content.
 * BEHAVIOR     :  Browse the selected server.
 * PRE          :  data = page_num of GtkNotebook containing the server.
 */
void refresh_server_content_cb (GtkWidget *w, gpointer data)
{
  GmLdapWindow *lw = NULL;
  GmLdapWindowPage *lwp = NULL;

  int option_menu_option = 0;
  int page_num = GPOINTER_TO_INT (data);
  
  gchar *filter = NULL;
  gchar *search_entry_text = NULL;
  
  GtkWidget *page = NULL;

  lw = gnomemeeting_get_ldap_window (gm);
  gtk_notebook_set_current_page (GTK_NOTEBOOK (lw->notebook), 
				 page_num);
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), 
				    page_num);

  lwp = gnomemeeting_get_ldap_window_page (page);

  search_entry_text =
    (gchar *) gtk_entry_get_text (GTK_ENTRY (lwp->search_entry));

  if (search_entry_text && strcmp (search_entry_text, "")) {

    option_menu_option =
      gtk_option_menu_get_history (GTK_OPTION_MENU (lwp->option_menu));
	      
    switch (option_menu_option)
      {
      case 0:
	filter =
	  g_strdup_printf ("(givenname=%%%s%%)", search_entry_text);
	break;
		  
      case 1:
	filter =
	  g_strdup_printf ("(surname=%%%s%%)", search_entry_text);
	break;
		  
      case 2:
	filter =
	  g_strdup_printf ("(rfc822mailbox=%%%s%%)",
			   search_entry_text);
	break;
      };
  }
	    

  /* Check if there is already a search running */
  if (!lwp->ils_browser && page_num != -1) 
    lwp->ils_browser =
      new GMILSBrowser (lwp, lwp->contact_section_name, filter);

  g_free (filter);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns true if contact_callto corresponds to a contact
 *                 of group group_name.
 * PRE          :  /
 */
gboolean
is_contact_member_of_group (gchar *contact_callto,
			    gchar *group_name)
{
  GSList *group_content = NULL;

  bool found = false;
  gchar *gconf_key = NULL;
  gchar **contact_info = NULL;

  GConfClient *client = NULL;


  if (!contact_callto || !group_name)
    return false;
  
  gconf_key =
    g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, group_name);

  client = gconf_client_get_default ();
  
  group_content =
    gconf_client_get_list (client, gconf_key, GCONF_VALUE_STRING, NULL);

  while (group_content && !found) {

    if (group_content->data) {
      
      contact_info = g_strsplit ((gchar *) group_content->data, "|", 0);

      if (contact_info && contact_info [COLUMN_CALLTO])
	if (!strcmp (contact_info [COLUMN_CALLTO], contact_callto)) {
	  
	  found = true;
	  break;
	}
      g_strfreev (contact_info);
    }

    group_content = g_slist_next (group_content);
  }

  group_content = g_slist_nth (group_content, 0);
  
  g_slist_free (group_content);
  g_free (gconf_key);

  return found;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns a link to the contact corresponding to the given
 *                 contact_callto in the group_content list, or NULL if none.
 * PRE          :  /
 */
GSList *
find_contact_in_group_content (gchar *contact_callto,
			       GSList *group_content)
{
  GSList *group_content_iter = NULL;

  gchar **group_content_split = NULL;
  
  group_content_iter = group_content;
  while (group_content_iter) {

    if (group_content_iter->data) {
      
      /* The member 1 of the split coming from the key is the
	 user callto, compare it with the callto of the user
	 before we edited it, to remove the old version of
	 the user in the gconf database */
      group_content_split =
	g_strsplit ((char *) group_content_iter->data, "|", 0);

      if ((group_content_split && group_content_split [1]
	   && !strcmp (group_content_split [1], contact_callto))
	  || (contact_callto == NULL && group_content_split [1] == NULL))
	break;
	    
      g_strfreev (group_content_split);
      group_content_split = NULL;
    }

    group_content_iter = g_slist_next (group_content_iter);
  }

  return group_content_iter;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Updates the given pointers so that they point to the
 *                 good sections in the GtkNotebook holding the GtkTreeViews
 *                 storing the contacts. Returns false on failure (nothing
 *                 selected). The contact_speed_dial pointer is not udpated
 *                 if *is_group = FALSE, ie if the selection corresponds to
 *                 server and not a group of contacts. No pointer should be
 *                 freed as they point to internal things freed on exit.
 * PRE          :  /
 */
gboolean
get_selected_contact_info (GtkNotebook *notebook,
			   gchar **contact_section,
			   gchar **contact_name,
			   gchar **contact_callto,
			   gchar **contact_speed_dial,
			   gboolean *is_group)
{
  GtkWidget *page = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;

  int page_num = 0;

  GmLdapWindowPage *lwp = NULL;
  
  /* Get the required data from the GtkNotebook page */
  page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_num);
  lwp = gnomemeeting_get_ldap_window_page (page);
  
  if (page && lwp) {

    *contact_section = lwp->contact_section_name;
      
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (lwp->tree_view));
    model = GTK_TREE_MODEL (lwp->users_list_store);
    
    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
      
      /* If the callback is called because we add a contact from the
	 server listing */
      if (lwp->page_type == CONTACTS_SERVERS) {

	if (contact_name)
	  gtk_tree_model_get (GTK_TREE_MODEL (lwp->users_list_store), &iter, 
			      COLUMN_ILS_NAME, contact_name, -1);

	if (contact_callto)
	  gtk_tree_model_get (GTK_TREE_MODEL (lwp->users_list_store), &iter, 
			      COLUMN_ILS_CALLTO, contact_callto, -1);

	*is_group = false;
	}
      else {

	if (contact_name)
	  gtk_tree_model_get (GTK_TREE_MODEL (lwp->users_list_store), &iter, 
			      COLUMN_NAME, contact_name, -1);
	
	if (contact_callto)
	  gtk_tree_model_get (GTK_TREE_MODEL (lwp->users_list_store), &iter, 
			      COLUMN_CALLTO, contact_callto, -1);

	if (contact_speed_dial)
	  gtk_tree_model_get (GTK_TREE_MODEL (lwp->users_list_store), &iter, 
			      COLUMN_SPEED_DIAL, contact_speed_dial, -1);

	*is_group = true;
      }
    }
    else
      return false;
  }
  else
    return false;

  
  return true;
}


/* The functions */
void
gnomemeeting_init_ldap_window ()
{
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *menubar = NULL;

  GdkPixbuf *xdap_pixbuf = NULL;

  GtkCellRenderer *cell = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkTreeStore *model = NULL;

  GmWindow *gw = NULL;
  GmLdapWindow *lw = NULL;

  GConfClient *client = NULL;

  static GtkTargetEntry dnd_targets [] =
    {
      {"text/plain", GTK_TARGET_SAME_APP, 0}
    };

  
  /* Get the structs from the application */
  gw = gnomemeeting_get_main_window (gm);
  lw = gnomemeeting_get_ldap_window (gm);
  client = gconf_client_get_default ();

  xdap_pixbuf = 
    gdk_pixbuf_new_from_xpm_data ((const gchar **) xdap_directory_xpm); 

  gw->ldap_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (gw->ldap_window), 
			_("GnomeMeeting Address Book"));
  gtk_window_set_icon (GTK_WINDOW (gw->ldap_window), xdap_pixbuf);
  gtk_window_set_position (GTK_WINDOW (gw->ldap_window), 
			   GTK_WIN_POS_CENTER);
  gtk_window_set_default_size (GTK_WINDOW (gw->ldap_window), 650, 350);
  g_object_unref (G_OBJECT (xdap_pixbuf));

  
  /* A vbox that will contain the menubar, and also the hbox containing
     the rest of the window */
  vbox = gtk_vbox_new (0, FALSE);
  gtk_container_add (GTK_CONTAINER (gw->ldap_window), vbox);
  menubar = gtk_menu_bar_new ();
  
  static MenuEntry addressbook_menu [] =
    {
      {_("_File"), NULL, NULL, 0, MENU_NEW, NULL, NULL, NULL},

      {_("New _Server"), NULL,
       GM_STOCK_REMOTE_CONTACT, 0, MENU_ENTRY, 
       GTK_SIGNAL_FUNC (new_contact_section_cb),
       GINT_TO_POINTER (CONTACTS_SERVERS), NULL},

      {_("New _Group"), NULL,
       GM_STOCK_LOCAL_CONTACT, 0, MENU_ENTRY, 
       GTK_SIGNAL_FUNC (new_contact_section_cb),
       GINT_TO_POINTER (CONTACTS_GROUPS), NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},

      {_("_Delete"), NULL,
       GTK_STOCK_DELETE, 0, MENU_ENTRY, 
       GTK_SIGNAL_FUNC (delete_cb), NULL, NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},

      {_("Close"), NULL,
       GTK_STOCK_CLOSE, 0, MENU_ENTRY, 
       GTK_SIGNAL_FUNC (gnomemeeting_component_view),
       (gpointer) gw->ldap_window,
       NULL},

      {_("C_ontact"), NULL, NULL, 0, MENU_NEW, NULL, NULL, NULL},

      {_("New _Contact"), NULL,
       GTK_STOCK_NEW, 0, MENU_ENTRY, 
       GTK_SIGNAL_FUNC (edit_contact_cb), GINT_TO_POINTER (1),
       NULL},

      {_("Add Contact to _Address Book"), NULL,
       GTK_STOCK_ADD, 0, MENU_ENTRY, 
       GTK_SIGNAL_FUNC (edit_contact_cb), GINT_TO_POINTER (0),
       NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},
      
      {_("Contact _Properties"), NULL,
       GTK_STOCK_PROPERTIES, 0, MENU_ENTRY, 
       GTK_SIGNAL_FUNC (edit_contact_cb), GINT_TO_POINTER (0), NULL},  

      {NULL, NULL, NULL, 0, MENU_END, NULL, NULL, NULL}
    };

  lw->addressbook_menu = (MenuEntry *) addressbook_menu;
  gnomemeeting_build_menu (menubar, addressbook_menu, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);

  
  /* A hbox to put the tree and the ldap browser */
  hbox = gtk_hbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  

  /* The GtkTreeView that will store the contacts sections */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
  model = gtk_tree_store_new (NUM_COLUMNS_CONTACTS, GDK_TYPE_PIXBUF, 
			      G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);


  lw->tree_view = gtk_tree_view_new ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (lw->tree_view), 
			   GTK_TREE_MODEL (model));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (lw->tree_view));
  gtk_container_add (GTK_CONTAINER (frame), lw->tree_view);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (lw->tree_view), FALSE);

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
  gtk_tree_view_column_set_attributes (column, cell, "markup", 
				       COLUMN_CONTACT_SECTION_NAME, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (lw->tree_view),
			       GTK_TREE_VIEW_COLUMN (column));

  /* a vbox to put the frames and the user list */
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

  /* We will put a GtkNotebook that will contain the contacts list */
  lw->notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (lw->notebook), 0);
  gtk_box_pack_start (GTK_BOX (vbox), lw->notebook, 
		      TRUE, TRUE, 0);

  
  /* Populate the tree_viw with groups and servers */
  gnomemeeting_addressbook_sections_populate ();

 
  /* Drag and Drop Setup */
  gtk_drag_dest_set (GTK_WIDGET (lw->tree_view), GTK_DEST_DEFAULT_ALL,
		     dnd_targets, 1,
		     GDK_ACTION_COPY);
  g_signal_connect (G_OBJECT (lw->tree_view), "drag_motion",
		    G_CALLBACK (dnd_drag_motion_cb), 0);
  g_signal_connect (G_OBJECT (lw->tree_view), "drag_data_received",
		    G_CALLBACK (dnd_drag_data_received_cb), 0);
  
  /* Double-click on a server name or on a contact group */
  g_signal_connect (G_OBJECT (lw->tree_view), "row_activated",
		    G_CALLBACK (contact_section_activated_cb), NULL);  

  /* Click or right-click on a server name or on a contact group */
  g_signal_connect_object (G_OBJECT (lw->tree_view), "event-after",
			   G_CALLBACK (contact_section_clicked_cb), 
			   NULL, (GConnectFlags) 0);

  /* Hide but do not delete the ldap window */
  g_signal_connect (G_OBJECT (gw->ldap_window), "delete_event",
		    G_CALLBACK (gtk_widget_hide_on_delete), NULL);  
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Free the memory associated with the GmLdapWindowPage for
 *                 the notebook page currently destroyed.
 * PRE          :  data = pointer to the GmLdapWindowPage for that page.
 *                 GmLdapWindowPage contains a Mutex released when the server
 *                 corresponding to that page has been browsed. Another
 *                 function must use it to ensure that the browse is terminated
 *                 before this function is called.
 *
 */
static void
notebook_page_destroy (gpointer data)
{
  GmLdapWindowPage *lwp = (GmLdapWindowPage *) data;

  if (data) {
    
    g_free (lwp->contact_section_name);
    delete (lwp);
  }
}


int
gnomemeeting_init_ldap_window_notebook (gchar *text_label,
					int type)
{
  GtkWidget *page = NULL;
  GtkWidget *scroll = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *handle = NULL;
  GtkWidget *menu = NULL;
  GtkWidget *menu_item = NULL;
  GtkWidget *refresh_button = NULL;
    
  GConfClient *client = NULL;
  
  /* For the GTK TreeView */
  GtkTreeViewColumn *column = NULL;
  GtkCellRenderer *renderer = NULL;

  int cpt = 0, page_num = 0;
  
  static GtkTargetEntry dnd_targets [] =
    {
      {"text/plain", GTK_TARGET_SAME_APP, 0}
    };
 
  GmLdapWindow *lw = gnomemeeting_get_ldap_window (gm);
  GmLdapWindowPage *current_lwp = NULL;

  
  client = gconf_client_get_default ();

  while ((page =
	  gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), cpt))){

    current_lwp = gnomemeeting_get_ldap_window_page (page);

    if (current_lwp->contact_section_name && text_label
	&& !strcmp (current_lwp->contact_section_name, text_label))
      return cpt;

    cpt++;
  }

  
  GmLdapWindowPage *lwp = new (GmLdapWindowPage);
  lwp->contact_section_name = g_strdup (text_label);
  lwp->ils_browser = NULL;
  lwp->search_entry = NULL;
  lwp->option_menu = NULL;
  lwp->page_type = type;
  
  if (type == CONTACTS_SERVERS)
    lwp->users_list_store = 
      gtk_list_store_new (NUM_COLUMNS_SERVERS, GDK_TYPE_PIXBUF, G_TYPE_BOOLEAN,
			  G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING,
			  G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
			  G_TYPE_STRING, G_TYPE_STRING);
  else
    lwp->users_list_store = 
      gtk_list_store_new (NUM_COLUMNS_GROUPS, G_TYPE_STRING, G_TYPE_STRING,
			  G_TYPE_STRING);


  vbox = gtk_vbox_new (FALSE, 0);
  scroll = gtk_scrolled_window_new (NULL, NULL);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), 
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);

  lwp->tree_view = 
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (lwp->users_list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (lwp->tree_view), TRUE);
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (lwp->tree_view),
				   COLUMN_ILS_NAME);

  /* Set all Colums */
  if (type == CONTACTS_SERVERS) {
    
    renderer = gtk_cell_renderer_pixbuf_new ();
    /* Translators: This is "S" as in "Status" */
    column = gtk_tree_view_column_new_with_attributes (_("S"),
						       renderer,
						       "pixbuf", 
						       COLUMN_ILS_STATUS,
						       NULL);
    gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);

    renderer = gtk_cell_renderer_toggle_new ();
    /* Translators: This is "A" as in "Audio" */
    column = gtk_tree_view_column_new_with_attributes (_("A"),
						       renderer,
						       "active", 
						       COLUMN_ILS_AUDIO,
						       NULL);
    gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);

    renderer = gtk_cell_renderer_toggle_new ();
    /* Translators: This is "V" as in "Video" */
    column = gtk_tree_view_column_new_with_attributes (_("V"),
						       renderer,
						       "active", 
						       COLUMN_ILS_VIDEO,
						       NULL);
    gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Name"),
						       renderer,
						       "text", 
						       COLUMN_ILS_NAME,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ILS_NAME);
    gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
					COLUMN_ILS_COLOR);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);
    g_object_set (G_OBJECT (renderer), "weight", "bold", NULL);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Comment"),
						       renderer,
						       "text", 
						       COLUMN_ILS_COMMENT,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ILS_COMMENT);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);
    g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC, NULL);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Location"),
						       renderer,
						       "text", 
						       COLUMN_ILS_LOCATION,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ILS_LOCATION);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Callto"),
						       renderer,
						       "text", 
						       COLUMN_ILS_CALLTO,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ILS_CALLTO);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);
    g_object_set (G_OBJECT (renderer), "foreground", "blue",
		  "underline", TRUE, NULL);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Version"),
						       renderer,
						       "text", 
						       COLUMN_ILS_VERSION,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ILS_VERSION);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);
    g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC, NULL);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("IP"),
						       renderer,
						       "text", 
						       COLUMN_ILS_IP,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ILS_IP);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);
    g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC,
		  "foreground", "darkgray", NULL);

  }
  else {

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Name"),
						       renderer,
						       "text", 
						       COLUMN_NAME,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_NAME);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_column_set_min_width (GTK_TREE_VIEW_COLUMN (column), 125);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);
    g_object_set (G_OBJECT (renderer), "weight", "bold", NULL);
    
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Callto URL"),
						       renderer,
						       "text", 
						       COLUMN_CALLTO,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_CALLTO);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_column_set_min_width (GTK_TREE_VIEW_COLUMN (column), 280);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);
    g_object_set (G_OBJECT (renderer), "foreground", "blue",
		  "underline", TRUE, NULL);

        renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Speed Dial"),
						       renderer,
						       "text", 
						       COLUMN_SPEED_DIAL,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_SPEED_DIAL);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column (GTK_TREE_VIEW (lwp->tree_view), column);
  }

  gtk_container_add (GTK_CONTAINER (scroll), lwp->tree_view);
  gtk_container_set_border_width (GTK_CONTAINER (lwp->tree_view), 0);
  gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);

  if (type == CONTACTS_SERVERS) {

    hbox = gtk_hbox_new (FALSE, 0);
    
    /* The toolbar */
    handle = gtk_handle_box_new ();
    gtk_box_pack_start (GTK_BOX (vbox), handle, FALSE, FALSE, 0);  
    gtk_container_add (GTK_CONTAINER (handle), hbox);
    gtk_container_set_border_width (GTK_CONTAINER (handle), 0);

    
    /* option menu */
    menu = gtk_menu_new ();
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

    lwp->option_menu = gtk_option_menu_new ();
    gtk_option_menu_set_menu (GTK_OPTION_MENU (lwp->option_menu),
			      menu);
    gtk_option_menu_set_history (GTK_OPTION_MENU (lwp->option_menu),
				 1);
    gtk_box_pack_start (GTK_BOX (hbox), lwp->option_menu, FALSE, FALSE, 2);
    
    
    /* entry */
    lwp->search_entry = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (hbox), lwp->search_entry, TRUE, TRUE, 2);


    /* The Refresh button */
    refresh_button = gtk_button_new_from_stock (GTK_STOCK_REFRESH);
    gtk_box_pack_start (GTK_BOX (hbox), refresh_button, FALSE, FALSE, 2);
    gtk_widget_show_all (handle);
    
    /* The statusbar */
    lwp->statusbar = gtk_statusbar_new ();
    gtk_box_pack_start (GTK_BOX (vbox), lwp->statusbar, FALSE, FALSE, 0);
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (lwp->statusbar), FALSE);
  }


  /* The drag and drop information */
  gtk_drag_source_set (GTK_WIDGET (lwp->tree_view),
		       GDK_BUTTON1_MASK, dnd_targets, 1,
		       GDK_ACTION_COPY);
  g_signal_connect (G_OBJECT (lwp->tree_view), "drag_data_get",
		    G_CALLBACK (dnd_drag_data_get_cb),
		    GINT_TO_POINTER (type));

  gtk_notebook_append_page (GTK_NOTEBOOK (lw->notebook), vbox, NULL);
  gtk_widget_show_all (GTK_WIDGET (lw->notebook));
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (lw->notebook), FALSE);

  while ((page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook),
					    page_num))) 
    page_num++;

  page_num--;
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), page_num);


  /* Connect to the search entry, and refresh button, if any */
  if (lwp->search_entry) {
    
    g_signal_connect (G_OBJECT (lwp->search_entry), "activate",
		      G_CALLBACK (refresh_server_content_cb), 
		      GINT_TO_POINTER (page_num));
    g_signal_connect (G_OBJECT (refresh_button), "clicked",
		      G_CALLBACK (refresh_server_content_cb), 
		      GINT_TO_POINTER (page_num));
  }
  
  g_object_set_data_full (G_OBJECT (page), "lwp", (gpointer) lwp,
			  notebook_page_destroy);
			       
  /* If the type of page is "groups", then we populate the page */
  if (type == CONTACTS_GROUPS) 
    gnomemeeting_addressbook_group_populate (lwp->users_list_store,
					     text_label);
  
  
  /* Signal to call the person on the double-clicked row */
  g_signal_connect (G_OBJECT (lwp->tree_view), "row_activated", 
		    G_CALLBACK (contact_activated_cb), NULL);

  /* Right-click on a contact */
  g_signal_connect (G_OBJECT (lwp->tree_view), "event-after",
		    G_CALLBACK (contact_clicked_cb), 
		    GINT_TO_POINTER (type));

  return page_num;
}


void
gnomemeeting_addressbook_group_populate (GtkListStore *list_store,
					 char *group_name)
{
  GtkTreeIter list_iter;
  
  GSList *group_content = NULL;

  char **contact_info = NULL;
  gchar *gconf_key = NULL;

  GConfClient *client = NULL;

  client = gconf_client_get_default ();
  
  gtk_list_store_clear (GTK_LIST_STORE (list_store));

  gconf_key =
    g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, (char *) group_name);

  group_content =
    gconf_client_get_list (client, gconf_key, GCONF_VALUE_STRING, NULL);

  while (group_content && group_content->data) {

    gtk_list_store_append (list_store, &list_iter);

    contact_info =
      g_strsplit ((char *) group_content->data, "|", 0);

    if (contact_info [0])
      gtk_list_store_set (list_store, &list_iter,
			  COLUMN_NAME, contact_info [0], -1);
    if (contact_info [1])
      gtk_list_store_set (list_store, &list_iter,
			  COLUMN_CALLTO, contact_info [1], -1);

    if (contact_info [2])
      gtk_list_store_set (list_store, &list_iter,
			  COLUMN_SPEED_DIAL, contact_info [2], -1);
    
    g_strfreev (contact_info);
    group_content = g_slist_next (group_content);
  }

  g_free (gconf_key);
  g_slist_free (group_content);
}


void
gnomemeeting_addressbook_sections_populate ()
{
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter, child_iter;

  GdkPixbuf *contact_icon = NULL;

  gchar *markup = NULL;

  GSList *ldap_servers_list = NULL;
  GSList *ldap_servers_list_iter = NULL;
  GSList *groups_list = NULL;
  GSList *groups_list_iter = NULL;

  GConfClient *client = NULL;
  GmLdapWindow *lw = NULL;

  int p = 0, cpt = 0;
  
  lw = gnomemeeting_get_ldap_window (gm);
  client = gconf_client_get_default ();
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (lw->tree_view));

  gtk_tree_store_clear (GTK_TREE_STORE (model));
  
  /* Populate the tree view : servers */
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  markup = g_strdup_printf("<b>%s</b>", _("Servers"));
  contact_icon = 
    gtk_widget_render_icon (lw->tree_view, GM_STOCK_REMOTE_CONTACT,
			    GTK_ICON_SIZE_MENU, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter, COLUMN_CONTACT_SECTION_NAME, markup, 
		      COLUMN_NOTEBOOK_PAGE, 0, 
		      COLUMN_PIXBUF_VISIBLE, FALSE, -1);

  ldap_servers_list =
    gconf_client_get_list (client, CONTACTS_KEY "ldap_servers_list",
			   GCONF_VALUE_STRING, NULL); 
    
  ldap_servers_list_iter = ldap_servers_list;
  while (ldap_servers_list_iter) {

    /* This will only add a notebook page if the server was not already
     * present */
    p = 
      gnomemeeting_init_ldap_window_notebook ((char *)
					      ldap_servers_list_iter->data,
					      CONTACTS_SERVERS);

    gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (model),
			&child_iter, 
			COLUMN_PIXBUF, contact_icon,
			COLUMN_CONTACT_SECTION_NAME,
			ldap_servers_list_iter->data, 
			COLUMN_NOTEBOOK_PAGE, p, 
			COLUMN_PIXBUF_VISIBLE, TRUE, -1);

    ldap_servers_list_iter = ldap_servers_list_iter->next;
    cpt++;
  }
  g_slist_free (ldap_servers_list);
  g_object_unref (contact_icon);
  g_free (markup);


  /* Populate the tree view : groups */
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  markup = g_strdup_printf("<b>%s</b>", _("Groups"));
  contact_icon = 
    gtk_widget_render_icon (lw->tree_view, GM_STOCK_LOCAL_CONTACT,
			    GTK_ICON_SIZE_MENU, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter, COLUMN_CONTACT_SECTION_NAME, markup, 
		      COLUMN_NOTEBOOK_PAGE, 0, 
		      COLUMN_PIXBUF_VISIBLE, FALSE, -1);

  groups_list =
    gconf_client_get_list (client, CONTACTS_KEY "groups_list",
			   GCONF_VALUE_STRING, NULL); 
    
  groups_list_iter = groups_list;
  while (groups_list_iter) {

    /* This will only add a notebook page if the server was not already
     * present */
    p = 
      gnomemeeting_init_ldap_window_notebook ((char *)
					      groups_list_iter->data,
					      CONTACTS_GROUPS);

    gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (model),
			&child_iter, 
			COLUMN_PIXBUF, contact_icon,
			COLUMN_CONTACT_SECTION_NAME, groups_list_iter->data, 
			COLUMN_NOTEBOOK_PAGE, p,
			COLUMN_PIXBUF_VISIBLE, TRUE, -1);

    groups_list_iter = groups_list_iter->next;
    cpt++;
  }
  g_slist_free (groups_list);
  g_object_unref (contact_icon);
  g_free (markup);


  /* Expand servers and groups */
  path = gtk_tree_path_new_from_string ("0:0");
  gtk_tree_view_expand_all (GTK_TREE_VIEW (lw->tree_view));
  gtk_tree_view_set_cursor (GTK_TREE_VIEW (lw->tree_view), path,
			    NULL, false);
  gtk_tree_path_free (path);
}


PString
gnomemeeting_addressbook_get_speed_dial_url (PString url)
{
  gchar *group_content_gconf_key = NULL;
  char **contact_info = NULL;
  
  GSList *group_content = NULL;
  GSList *groups = NULL;

  PString result;
  
  GConfClient *client = NULL;

  if (url.IsEmpty ())
    return result;
  
  client = gconf_client_get_default ();

  groups =
    gconf_client_get_list (client, CONTACTS_KEY "groups_list",
			   GCONF_VALUE_STRING, NULL);

  while (groups && groups->data) {
    
    group_content_gconf_key =
      g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, (char *) groups->data);

    group_content =
      gconf_client_get_list (client, group_content_gconf_key,
			     GCONF_VALUE_STRING, NULL);

    while (group_content && group_content->data) {
      
      contact_info =
	g_strsplit ((char *) group_content->data, "|", 0);
      
      if (contact_info [2] && !strcmp (contact_info [2], (const char *) url)) {

	result = PString (contact_info [1]);
	break;
      }

      g_strfreev (contact_info);
      group_content = g_slist_next (group_content);
    }

    g_free (group_content_gconf_key);
    g_slist_free (group_content);

    groups = g_slist_next (groups);
  }

  g_slist_free (groups);

  return result;
}
