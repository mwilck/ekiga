
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


/* Declarations */
extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

struct GmEditContactDialog_ {

  GtkWidget *dialog;
  GtkWidget *name_entry;
  GtkWidget *url_entry;
  GtkWidget *speed_dial_entry;
  
  GtkListStore *groups_list_store;

  int selected_groups_number;
  gchar *old_contact_url;
};
typedef struct GmEditContactDialog_ GmEditContactDialog;


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

static GmEditContactDialog *addressbook_edit_contact_dialog_new (const char *,
								 const char *,
								 const char *);

static gboolean addressbook_edit_contact_valid (GmEditContactDialog *,
						gboolean);

static void delete_contact_from_group_cb (GtkWidget *,
					  gpointer);

static gint contact_clicked_cb (GtkWidget *,
				GdkEventButton *,
				gpointer);

static gchar* gnomemeeting_addressbook_get_speed_dial_from_url (GMURL);

/* Callbacks: Operations on contact sections */
static void new_contact_section_cb (GtkWidget *,
				    gpointer);

static void delete_contact_section_cb (GtkWidget *,
				       gpointer);

static void contact_section_changed_cb (GtkTreeSelection *,
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

static void gtk_dialog_response_accept (GtkWidget *,
					gpointer);

/* Local functions: Operations on a contact */
static gboolean is_contact_member_of_group (GMURL,
					    const char *);

static gboolean is_contact_member_of_addressbook (GMURL);

static gboolean is_group_member_of_addressbook (const char *);

static GSList *find_contact_in_group_content (const char *,
					      GSList *);

static gboolean get_selected_contact_info (gchar ** = NULL,
					   gchar ** = NULL,
					   gchar ** = NULL,
					   gchar ** = NULL,
					   gboolean * = NULL);

/* Misc */
static void notebook_page_destroy (gpointer data);

static void edit_dialog_destroy (gpointer data);

static void update_menu_sensitivity (gboolean,
				     gboolean,
				     gboolean);


/* GTK Callbacks */

/* DESCRIPTION  :  This callback is called when the user moves the drag.
 * BEHAVIOR     :  Draws a rectangle around the groups in which the user info
 *                 can be dropped.
 * PRE          :  /
 */
static gboolean
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
  gchar *contact_url = NULL;

  gboolean is_group = false;
  
  GmLdapWindow *lw = NULL;
  
  GValue value =  {0, };
  GtkTreeIter iter;

  lw = gnomemeeting_get_ldap_window (gm);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

  /* Get the url field of the contact info from the source GtkTreeView */
  if (get_selected_contact_info (&contact_section, &contact_name,
				 &contact_url, NULL, &is_group)) {
    
    /* See if the path in the destination GtkTreeView corresponds to a valid
       row (ie a group row, and a row corresponding to a group the user
       doesn't belong to */
    if (gtk_tree_view_get_dest_row_at_pos (GTK_TREE_VIEW (tree_view),
					   x, y, &path, NULL)) {

      if (gtk_tree_model_get_iter (model, &iter, path)) {
	    
	gtk_tree_model_get_value (model, &iter, 
				  COLUMN_CONTACT_SECTION_NAME, &value);
	group_name = g_utf8_strdown (g_value_get_string (&value), -1);
	g_value_unset (&value);

	/* If the user doesn't belong to the selected group and if
	   the selected row corresponds to a group and not a server */
	if (gtk_tree_path_get_depth (path) >= 2 &&
	    gtk_tree_path_get_indices (path) [0] >= 1 
	    && group_name && contact_url &&
	    !is_contact_member_of_group (GMURL (contact_url), group_name)) {
    
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

  g_free (contact_name);
  g_free (contact_url);
  g_free (contact_section);
  
  return true;
}


/* DESCRIPTION  :  This callback is called when the user has released
 *                 the drag.
 * BEHAVIOR     :  Adds the user gconf key of the group where the drop
 *                 occured.
 * PRE          :  /
 */
static void
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
	group_name = g_utf8_strdown (g_value_get_string (&value), -1);
	g_value_unset (&value);

	if (group_name && selection_data && selection_data->data) {

	  contact_info = g_strsplit ((char *) selection_data->data, "|", 0);

	  if (contact_info [1] &&
	      !is_contact_member_of_group (GMURL (contact_info [1]),
					   group_name)) {

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
  gchar *contact_name = NULL;
  gchar *contact_url = NULL;
  gchar *drag_data = NULL;


  if (get_selected_contact_info (NULL, &contact_name,
				 &contact_url, NULL, NULL)
      && contact_name && contact_url) {
      
    drag_data = g_strdup_printf ("%s|%s", contact_name, contact_url);
    
    gtk_selection_data_set (selection_data, selection_data->target, 
			    8, (const guchar *) drag_data,
			    strlen (drag_data));
    g_free (drag_data);
  }

  g_free (contact_name);
  g_free (contact_url);
}


/* DESCRIPTION  :  This callback is called when the user toggles a group in
 *                 the groups list in the popup permitting to edit
 *                 the properties of a contact.
 * BEHAVIOR     :  Update the toggles for that group, updates the number
 *                 of selected groups field of the GmEditContactDialog given
 *                 as parameter.
 * PRE          :  data = a valid GmEditContactDialog.
 */
static void
groups_list_store_toggled (GtkCellRendererToggle *cell,
			   gchar *path_str,
			   gpointer data)
{
  GmEditContactDialog *edit_dialog = (GmEditContactDialog *) data;
  GtkTreeIter iter;
  GtkTreeModel *model = GTK_TREE_MODEL (edit_dialog->groups_list_store);
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  
  gboolean member_of_group = false;

  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, 0, &member_of_group, -1);
  
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0,
		      !member_of_group, -1);

  if (member_of_group)
    edit_dialog->selected_groups_number--;
  else
    edit_dialog->selected_groups_number++;
}


/* DESCRIPTION  :  This callback is called when the user chooses to edit the
 *                 info of a contact from a group or to add an user.
 * BEHAVIOR     :  Opens a popup, and save the modified info by modifying the
 *                 gconf key when the user clicks on "ok".
 * PRE          :  If data = 1, then we add a new user, no need to see 
 *                 if something is selected and needs to be edited.
 */
static void
edit_contact_cb (GtkWidget *widget,
		 gpointer data)
{
  GConfClient *client = NULL;

  gchar *contact_name = NULL;
  gchar *contact_speed_dial = NULL;
  gchar *contact_url = NULL;
  gchar *contact_section = NULL;
  gchar *contact_info = NULL;
  gchar *group_name = NULL;
  gchar *group_name_no_case = NULL;
  gchar *gconf_key = NULL;
  gchar *speed_dial = NULL;

  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;
  
  const char *name_entry_text = NULL;
  const char *url_entry_text = NULL;
  const char *speed_dial_entry_text = NULL;
  
  GtkTreeIter iter;
  
  gboolean is_group = false;
  gboolean selected = false;
  gboolean valid_answer = false;
  int result = 0;

  GmEditContactDialog *edit_dialog = NULL;
  GmLdapWindow *lw = NULL;
  GmWindow *gw = NULL;
    
  lw = gnomemeeting_get_ldap_window (gm);
  gw = gnomemeeting_get_main_window (gm);
  
  client = gconf_client_get_default ();


  /* We don't care if it fails as long as contact_section and is_group
     are correct */
  get_selected_contact_info (&contact_section, &contact_name,
			     &contact_url, &contact_speed_dial,
			     &is_group);

  /* If the selected contact is in ILS, we try to find his speed_dial
     using GConf in our addressbook */
  if (!is_group && contact_url) {

    speed_dial =
      gnomemeeting_addressbook_get_speed_dial_from_url (GMURL (contact_url));

    g_free (contact_speed_dial); /* Free the old allocated string */
    contact_speed_dial = speed_dial; /* Needs to be freed later */
  }

  /* If we add a new user, we forget what is selected */
  if (GPOINTER_TO_INT (data) == 1) {

    g_free (contact_name);
    g_free (contact_url);
    g_free (contact_speed_dial);
    contact_name = NULL;
    contact_url = NULL;
    contact_speed_dial = NULL;
  }

  
  edit_dialog =
    addressbook_edit_contact_dialog_new (contact_name, contact_url,
					 contact_speed_dial);
  
  while (!valid_answer) {
    
    result = gtk_dialog_run (GTK_DIALOG (edit_dialog->dialog));
    
    switch (result) {
      
    case GTK_RESPONSE_ACCEPT:

      valid_answer =
	addressbook_edit_contact_valid (edit_dialog,
					(GPOINTER_TO_INT (data) == 1));

      if (valid_answer) {
	
	name_entry_text =
	  gtk_entry_get_text (GTK_ENTRY (edit_dialog->name_entry));
	url_entry_text =
	  gtk_entry_get_text (GTK_ENTRY (edit_dialog->url_entry));
	speed_dial_entry_text =
	  gtk_entry_get_text (GTK_ENTRY (edit_dialog->speed_dial_entry));
      
	contact_info =
	  g_strdup_printf ("%s|%s|%s", name_entry_text, url_entry_text,
			   speed_dial_entry_text);
      
	/* Determine the groups where we want to add the contact */
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (edit_dialog->groups_list_store), &iter)) {
	
	  do {
	  
	    gtk_tree_model_get (GTK_TREE_MODEL (edit_dialog->groups_list_store), &iter, 0, &selected, 1, &group_name, -1);
	  
	    if (group_name) {
	    
	      group_name_no_case = g_utf8_strdown (group_name, -1);

	      gconf_key =
		g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, 
				 group_name_no_case);
	    
	      group_content =
		gconf_client_get_list (client, gconf_key,
				       GCONF_VALUE_STRING, NULL);
	    
	      /* Once we find the contact corresponding to the old saved url
		 (if we are editing an existing user and not adding a new one),
		 we delete him and insert the new one at the same position;
		 if the group is not selected for that user, we delete him from
		 the group.
	      */
	      if (edit_dialog->old_contact_url) {
	      
		group_content_iter =
		  find_contact_in_group_content (edit_dialog->old_contact_url,
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
		if (selected)
		  group_content =
		    g_slist_append (group_content, (gpointer) contact_info);
	    
	      gconf_client_set_list (client, gconf_key,
				     GCONF_VALUE_STRING,
				     group_content, NULL);
	      valid_answer = true;
	    
	      g_free (group_name_no_case);
	      g_free (gconf_key);
	      g_slist_free (group_content);

	      update_menu_sensitivity (false, false, false);
	    }
	  
	    g_free (group_name);
	  
	  } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (edit_dialog->groups_list_store), &iter));

	}
      }
      break;


    case GTK_RESPONSE_REJECT:
      valid_answer = true;
      break;
      
    }
  }


  g_free (contact_name);
  g_free (contact_speed_dial);
  g_free (contact_url);
  g_free (contact_section);
  
  if (edit_dialog->dialog) 
    gtk_widget_destroy (edit_dialog->dialog);
}


/* DESCRIPTION  :  Called when the EditContactDialog gets destroyed.
 * BEHAVIOR     :  Frees the data when the EditContactDialog is destroyed.
 * PRE          :  data = valid pointer to a GmEditContactDialog.
 */
static void
edit_dialog_destroy (gpointer data)
{
  GmEditContactDialog *edit_dialog = (GmEditContactDialog *) data;

  if (data) {
    
    g_free (edit_dialog->old_contact_url);
    delete (edit_dialog);
  }
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates and display the EditContactDialog with the provided
 *                 default values.
 * PRE          :  /
 */
static GmEditContactDialog*
addressbook_edit_contact_dialog_new (const char *contact_name,
				     const char *contact_url,
				     const char *contact_speed_dial)
{
  GmWindow *gw = NULL;
  GmEditContactDialog *edit_dialog = NULL;
  
  GtkWidget *scroll = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *table = NULL;
  GtkWidget *label = NULL;

  GtkWidget *tree_view = NULL;
  GtkCellRenderer *renderer = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkTreeIter iter;

  GConfClient *client = NULL;
  
  GSList *groups_list = NULL;
  GSList *groups_list_iter = NULL;

  gchar *label_text = NULL;
  gboolean selected = false;

  
  gw = gnomemeeting_get_main_window (gm);
  client = gconf_client_get_default ();

  edit_dialog = new (GmEditContactDialog);
  memset (edit_dialog, 0, sizeof (GmEditContactDialog));
  
  /* Create the dialog to easily modify the info of a specific contact */
  edit_dialog->dialog =
    gtk_dialog_new_with_buttons (_("Edit the contact information"), 
				 GTK_WINDOW (gw->ldap_window),
				 GTK_DIALOG_MODAL,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
				 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
				 NULL);

  g_object_set_data_full (G_OBJECT (edit_dialog->dialog), "data", edit_dialog,
			  edit_dialog_destroy);
  
  table = gtk_table_new (4, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3 * GNOMEMEETING_PAD_SMALL);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3 * GNOMEMEETING_PAD_SMALL);
  
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  label_text = g_strdup_printf ("<b>%s</b>", _("Name:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  edit_dialog->name_entry = gtk_entry_new ();
  if (contact_name)
    gtk_entry_set_text (GTK_ENTRY (edit_dialog->name_entry), contact_name);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), edit_dialog->name_entry, 1, 2, 0, 1, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  g_signal_connect (G_OBJECT (edit_dialog->name_entry), "activate",
		    GTK_SIGNAL_FUNC (gtk_dialog_response_accept),
		    (gpointer) edit_dialog->dialog);
    
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  label_text = g_strdup_printf ("<b>%s</b>", _("URL:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);
  
  edit_dialog->url_entry = gtk_entry_new ();
  gtk_widget_set_size_request (GTK_WIDGET (edit_dialog->url_entry), 300, -1);
  if (contact_url) {
    
    gtk_entry_set_text (GTK_ENTRY (edit_dialog->url_entry), contact_url);
    edit_dialog->old_contact_url = g_strdup (contact_url);
  }
  else
    gtk_entry_set_text (GTK_ENTRY (edit_dialog->url_entry),
			GMURL ().GetDefaultURL ());
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), edit_dialog->url_entry, 1, 2, 1, 2, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  g_signal_connect (G_OBJECT (edit_dialog->url_entry), "activate",
		    GTK_SIGNAL_FUNC (gtk_dialog_response_accept),
		    (gpointer) edit_dialog->dialog);

  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  label_text = g_strdup_printf ("<b>%s</b>", _("Speed Dial:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  edit_dialog->speed_dial_entry = gtk_entry_new ();
  if (contact_speed_dial)
    gtk_entry_set_text (GTK_ENTRY (edit_dialog->speed_dial_entry),
			contact_speed_dial);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), edit_dialog->speed_dial_entry,
		    1, 2, 2, 3, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  g_signal_connect (G_OBJECT (edit_dialog->speed_dial_entry), "activate",
		    GTK_SIGNAL_FUNC (gtk_dialog_response_accept),
		    (gpointer) edit_dialog->dialog);

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
  edit_dialog->groups_list_store =
    gtk_list_store_new (2, G_TYPE_BOOLEAN, G_TYPE_STRING);
  tree_view = 
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (edit_dialog->groups_list_store));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), FALSE);
  
  renderer = gtk_cell_renderer_toggle_new ();
  column = gtk_tree_view_column_new_with_attributes ("", renderer,
						     "active", 0, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  g_signal_connect (G_OBJECT (renderer), "toggled",
		    G_CALLBACK (groups_list_store_toggled), 
		    gpointer (edit_dialog));

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("", renderer,
						     "text", 1, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), 
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scroll), tree_view);
  gtk_container_add (GTK_CONTAINER (frame), scroll);
  gtk_widget_set_size_request (GTK_WIDGET (frame), -1, 90);
  
  gtk_table_attach (GTK_TABLE (table), frame, 1, 2, 3, 4, 
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  
  /* Populate the list store with available groups */
  groups_list =
    gconf_client_get_list (client, CONTACTS_KEY "groups_list",
			   GCONF_VALUE_STRING, NULL);
  groups_list_iter = groups_list;
  
  while (groups_list_iter) {
    
    if (groups_list_iter->data) {

      selected =
	(contact_url
	 && is_contact_member_of_group (GMURL (contact_url),
					(char *) groups_list_iter->data));
      
      gtk_list_store_append (edit_dialog->groups_list_store, &iter);
      gtk_list_store_set (edit_dialog->groups_list_store, &iter,
			  0, selected,
			  1, groups_list_iter->data, -1);

      if (selected)
	edit_dialog->selected_groups_number++;
    }

    groups_list_iter = g_slist_next (groups_list_iter);
  }
  g_slist_free (groups_list);

  
  /* Pack the gtk entries and the list store in the window */
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (edit_dialog->dialog)->vbox), table,
		      FALSE, FALSE, 3 * GNOMEMEETING_PAD_SMALL);
  gtk_widget_show_all (edit_dialog->dialog);

  return edit_dialog;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns true if the EditContactDialog contains valid fields,
 *                 or false if it is not the case. A popup is displayed with
 *                 the error message in that case.
 * PRE          :  n is true if we are adding a new contact, false if we are
 *                 editing the properties of an existing contact.
 */
static gboolean
addressbook_edit_contact_valid (GmEditContactDialog *edit_dialog,
				gboolean n)
{
  const char *name_entry_text = NULL;
  const char *url_entry_text = NULL;
  const char *speed_dial_entry_text = NULL;

  GMURL other_speed_dial_url;
  GMURL entry_url;
  GMURL old_entry_url;
  
  name_entry_text = gtk_entry_get_text (GTK_ENTRY (edit_dialog->name_entry));
  url_entry_text = gtk_entry_get_text (GTK_ENTRY (edit_dialog->url_entry));
  entry_url = GMURL (url_entry_text);
  speed_dial_entry_text =
    gtk_entry_get_text (GTK_ENTRY (edit_dialog->speed_dial_entry));
  if (edit_dialog->old_contact_url)
    old_entry_url = GMURL (edit_dialog->old_contact_url);
    
  /* If there is no name or an empty url, display an error message
     and exit */
  if (!strcmp (name_entry_text, "") || entry_url.IsEmpty ()) {

    gnomemeeting_error_dialog (GTK_WINDOW (edit_dialog->dialog), _("Invalid user name or URL"), _("Please provide a valid name and URL for the contact you want to add to the address book."));
    return false;
  }


  /* If the user selected no groups, display an error message and exit */
  if (edit_dialog->selected_groups_number == 0) {

    gnomemeeting_error_dialog (GTK_WINDOW (edit_dialog->dialog), _("Invalid group"), _("You have to select a group to which you want to add your contact."));
    return false;
  }


  /* If we can find another url for the same speed dial, display an error
     message and exit */
  other_speed_dial_url =
    gnomemeeting_addressbook_get_url_from_speed_dial (speed_dial_entry_text);

  if (!other_speed_dial_url.IsEmpty () && !entry_url.IsEmpty ()
      && other_speed_dial_url != entry_url
      && other_speed_dial_url != old_entry_url) {
		
    gnomemeeting_error_dialog (GTK_WINDOW (edit_dialog->dialog), _("Invalid speed dial"), _("Another contact with the same speed dial already exists in the address book."));

    return false;
  }


  /* If the user is adding a new user, and there is already an identical
     url OR if the user modified an existing user to an user having
     an identical url, display a warning and exit */
  if (url_entry_text && is_contact_member_of_addressbook (entry_url)
      && (n || (!n && old_entry_url != entry_url))) {
    
    gnomemeeting_error_dialog (GTK_WINDOW (edit_dialog->dialog), _("Invalid URL"), _("Another contact with the same URL already exists in the address book."));
    return false;
  }

  return true;
}


/* DESCRIPTION  :  This callback is called when the user chooses to delete the
 *                 a contact from a group.
 * BEHAVIOR     :  Removes the user from the gconf key updates the gconf db.
 * PRE          :  /
 */
static void
delete_contact_from_group_cb (GtkWidget *widget,
			      gpointer data)
{
  GConfClient *client = NULL;
  
  gchar *contact_url = NULL;
  gchar *contact_name = NULL;
  gchar *gconf_key = NULL;
  gchar *contact_section = NULL;
  gchar *contact_section_no_case = NULL;

  gboolean is_group;
  
  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;
  
  GmLdapWindow *lw = NULL;

  lw = gnomemeeting_get_ldap_window (gm);
  client = gconf_client_get_default ();

  if (get_selected_contact_info (&contact_section, &contact_name,
				 &contact_url, NULL, &is_group)
      && is_group) {

    contact_section_no_case = g_utf8_strdown (contact_section, -1);
    gconf_key =
      g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, contact_section);
    g_free (contact_section_no_case);

    group_content =
      gconf_client_get_list (client, gconf_key, GCONF_VALUE_STRING, NULL);
    
    group_content_iter =
      find_contact_in_group_content (contact_url, group_content);
    
    group_content = g_slist_remove_link (group_content,
					 group_content_iter);
    
    gconf_client_set_list (client, gconf_key, GCONF_VALUE_STRING,
			   group_content, NULL);
    
    g_slist_free (group_content);

    g_free (gconf_key);
  }

  g_free (contact_section);
  g_free (contact_name);
  g_free (contact_url);
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
  GtkWidget *child = NULL;

  GConfClient *client = NULL;

  gchar *contact_url = NULL;
  gchar *contact_name = NULL;
  gchar *contact_section = NULL;
  gchar *msg = NULL;

  gboolean is_group = false;
  gboolean already_member = false;

  lw = gnomemeeting_get_ldap_window (gm);
  client = gconf_client_get_default ();

  if (e->type == GDK_BUTTON_PRESS || e->type == GDK_KEY_PRESS) {

    if (get_selected_contact_info (&contact_section, &contact_name,
				   &contact_url, NULL, &is_group)) {

      if (contact_url)
	already_member =
	  is_contact_member_of_addressbook (GMURL (contact_url));


      /* Update the main menu sensitivity */
      update_menu_sensitivity (is_group, false, !already_member);
      msg = g_strdup_printf (_("Add %s to Address Book"), contact_name);
			       
      child = GTK_BIN (lw->addressbook_menu [11].widget)->child;
      gtk_label_set_text (GTK_LABEL (child), msg);
			  
      if (e->button == 3) {
	
	menu = gtk_menu_new ();

       	MenuEntry server_contact_menu [5];

	MenuEntry call =
	  {_("C_all Contact"), NULL,
	   NULL, 0, MENU_ENTRY, 
	   GTK_SIGNAL_FUNC (call_user_cb), data, NULL};

	MenuEntry add =
	  {msg, NULL,
	   GTK_STOCK_ADD, 0, MENU_ENTRY,
	   GTK_SIGNAL_FUNC (edit_contact_cb),
	   GINT_TO_POINTER (0), NULL};
	
	MenuEntry props =
	  {_("Contact _Properties"), NULL,
	   GTK_STOCK_PROPERTIES, 0, MENU_ENTRY,
	   GTK_SIGNAL_FUNC (edit_contact_cb),
	   GINT_TO_POINTER (0), NULL};
	
	MenuEntry del =
	  {_("_Delete"), NULL,
	   GTK_STOCK_DELETE, 0, MENU_ENTRY,
	   GTK_SIGNAL_FUNC (delete_cb),
	   NULL, NULL};

	MenuEntry sep =
	  {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL};

	MenuEntry endt =
	  {NULL, NULL, NULL, 0, MENU_END, NULL, NULL, NULL};

	server_contact_menu [0] = call;
	    
	if (!already_member) {

	  server_contact_menu [1] = sep;
	  server_contact_menu [2] = add;
	  server_contact_menu [3] = endt;
	}
	else {

	  if (!is_group) {

	    server_contact_menu [1] = sep;
	    server_contact_menu [2]= props;
	    server_contact_menu [3] = endt;
	  }
	  else {

	    server_contact_menu [1]= props;
	    server_contact_menu [2] = sep;
	    server_contact_menu [3] = del;
	    server_contact_menu [4] = endt;
	  }
	}

	MenuEntry group_contact_menu [4];

	group_contact_menu [0] = props;
	group_contact_menu [1] = sep;
	group_contact_menu [2] = del;
	group_contact_menu [3] = endt;
	
	if (GPOINTER_TO_INT (data) == CONTACTS_SERVERS)
	  gnomemeeting_build_menu (menu, server_contact_menu, 0);
	else
	  gnomemeeting_build_menu (menu, group_contact_menu, NULL);
	
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
			e->button, e->time);
	g_signal_connect (G_OBJECT (menu), "hide",
			GTK_SIGNAL_FUNC (g_object_unref), (gpointer) menu);
	g_object_ref (G_OBJECT (menu));
	gtk_object_sink (GTK_OBJECT (menu));
	
	g_free (contact_url);
	g_free (contact_name);
	g_free (contact_section);
	
	return TRUE;
      }
    
      g_free (msg);
    }

    g_free (contact_name);
    g_free (contact_section);
    g_free (contact_url);
  }

  return FALSE;
}


/* DESCRIPTION  :  This callback is called when the user hits enter in
 *                 in a GtkEntry in a dialog.
 * BEHAVIOR     :  Emits the GTK_RESPONSE_ACCEPT signal for the dialog.
 * PRE          :  data = a pointer to a GtkDialog.
 */
static void
gtk_dialog_response_accept (GtkWidget *w,
			    gpointer data)
{
  if (data)
    gtk_dialog_response (GTK_DIALOG (data), GTK_RESPONSE_ACCEPT);
}


/* DESCRIPTION  :  This callback is called when the user chooses to add
 *                 a new contact section, server or group.
 * BEHAVIOR     :  Opens a pop up to ask for the contact section name
 *                 and updates the right gconf key if the contact section
 *                 doesn't already exist.
 * PRE          :  data = CONTACTS_GROUPS or CONTACTS_SERVERS
 */
static void
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
  gchar *dialog_error_text = NULL;
  gchar *dialog_title = NULL;
  gchar *gconf_key = NULL;
  
  gw = gnomemeeting_get_main_window (gm);
  client = gconf_client_get_default ();

  if (GPOINTER_TO_INT (data) == CONTACTS_SERVERS) {

    dialog_title = g_strdup (_("Add a new server"));
    dialog_text = g_strdup (_("Enter the server name:"));
    dialog_error_text = g_strdup (_("Sorry but there is already a server with the same name in the address book."));
    gconf_key = g_strdup (CONTACTS_KEY "ldap_servers_list");
  }
  else {

    dialog_title = g_strdup (_("Add a new group"));
    dialog_text = g_strdup (_("Enter the group name:"));
    dialog_error_text = g_strdup (_("Sorry but there is already a group with the same name in the address book."));
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
  g_signal_connect (G_OBJECT (entry), "activate",
		    GTK_SIGNAL_FUNC (gtk_dialog_response_accept),
		    (gpointer) dialog);
		    
  gtk_widget_show_all (dialog);
  
  result = gtk_dialog_run (GTK_DIALOG (dialog));

  switch (result) {

    case GTK_RESPONSE_ACCEPT:
	 
      entry_text = (gchar *) gtk_entry_get_text (GTK_ENTRY (entry));

      if (entry_text && strcmp (entry_text, "")) {

	if (is_group_member_of_addressbook (entry_text)) 
	  gnomemeeting_error_dialog (GTK_WINDOW (gw->ldap_window),
				     _("Invalid server or group name"),
				     dialog_error_text);
	else {
	  
	  PString s = PString (entry_text);
	  s.Replace (" ", "_", true);

	  contacts_list =
	    gconf_client_get_list (client, gconf_key,
				   GCONF_VALUE_STRING, NULL); 

	  contacts_list =
	    g_slist_append (contacts_list, (void *) (const char *) s);

	  gconf_client_set_list (client, gconf_key, GCONF_VALUE_STRING, 
				 contacts_list, NULL);

	  g_slist_free (contacts_list);
	}
      }
      
      break;
  }

  g_free (dialog_title);
  g_free (dialog_text);
  g_free (dialog_error_text);
  g_free (gconf_key);
  gtk_widget_destroy (dialog);
}


/* DESCRIPTION  :  This callback is called when the user chooses to delete
 *                 a contact section, server or group.
 * BEHAVIOR     :  Removes the corresponding contact section from the gconf
 *                 key and updates it.
 * PRE          :  /
 */
static void
delete_contact_section_cb (GtkWidget *widget,
			   gpointer data)
{
  GmLdapWindow *lw = NULL;
  GmWindow *gw = NULL;
  
  GConfClient *client = NULL;

  GSList *contacts_list = NULL;
  GSList *contacts_list_iter = NULL;
 
  GtkTreePath *path = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;

  GtkWidget *dialog = NULL;

  gchar *confirm_msg = NULL;
  gchar *gconf_key = NULL;
  gchar *unset_group_gconf_key = NULL;
  gchar *name = NULL;
  gchar *name_no_case = NULL;

  gboolean is_group = false;

  gw = gnomemeeting_get_main_window (gm);
  lw = gnomemeeting_get_ldap_window (gm);
  client = gconf_client_get_default ();

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (lw->tree_view));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (lw->tree_view));
  
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    
    path = gtk_tree_model_get_path (model, &iter);
    
    gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			COLUMN_CONTACT_SECTION_NAME, &name, -1);
      
    if (gtk_tree_path_get_depth (path) >= 2) {
      
      if (gtk_tree_path_get_indices (path) [0] == 0) {
	
	gconf_key = g_strdup (CONTACTS_KEY "ldap_servers_list");
	confirm_msg = g_strdup_printf (_("Are you sure you want to delete server %s?"), name);
      }
      else {

	is_group = true;
	gconf_key = g_strdup (CONTACTS_KEY "groups_list");
	confirm_msg = g_strdup_printf (_("Are you sure you want to delete group %s and all its contacts?"), name);
      }

      dialog =
	gtk_message_dialog_new (GTK_WINDOW (gw->ldap_window),
				GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_YES_NO, confirm_msg);

      switch (gtk_dialog_run (GTK_DIALOG (dialog))) {

      case GTK_RESPONSE_YES:

	contacts_list =
	  gconf_client_get_list (client, gconf_key, GCONF_VALUE_STRING, NULL); 

  
	contacts_list_iter = contacts_list;
	while (name && contacts_list_iter) {
	
	  if (!strcasecmp ((char *) name,
			   (char *) contacts_list_iter->data)) {

	    contacts_list = 
	      g_slist_remove_link (contacts_list, contacts_list_iter);

	    if (is_group) {

	      name_no_case = g_utf8_strdown (name, -1);
	      unset_group_gconf_key =
		g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, name_no_case);
	      gconf_client_set_list (client, unset_group_gconf_key,
				     GCONF_VALUE_STRING, NULL, NULL);

	      gconf_client_remove_dir (client, "/apps/gnomemeeting", 0);
	      gconf_client_unset (client, unset_group_gconf_key, NULL);
	      gconf_client_add_dir (client, "/apps/gnomemeeting",
				    GCONF_CLIENT_PRELOAD_RECURSIVE, 0);
	      g_free (unset_group_gconf_key);
	      g_free (name_no_case);
	    }

	    contacts_list_iter = contacts_list;
	  }

	  if (contacts_list_iter)
	    contacts_list_iter = contacts_list_iter->next;
	}
	g_slist_free (contacts_list_iter);
      
	gconf_client_set_list (client, gconf_key, GCONF_VALUE_STRING, 
			       contacts_list, NULL);
      
	break;

      }

      gtk_tree_path_free (path);
      g_free (name);
      g_slist_free (contacts_list);
      g_free (gconf_key);
      g_free (confirm_msg);

      if (dialog)
	gtk_widget_destroy (dialog);
    }
  }
}


/* DESCRIPTION  :  This callback is called when there is a "changed" event
 *                 signal on one of the contact section.
 * BEHAVIOR     :  Selects the right notebook page and updates the menu.
 * PRE          :  /
 */
static void
contact_section_changed_cb (GtkTreeSelection *selection,
			    gpointer data)
{
  GtkWidget *page = NULL;
  GtkTreeSelection *lselection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gint page_num = -1;

  GmLdapWindow *lw = NULL;
  GmLdapWindowPage *lwp = NULL;

  lw = gnomemeeting_get_ldap_window (gm);

  /* Update the sensitivity of DELETE */
  update_menu_sensitivity (false, true, false);

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			COLUMN_NOTEBOOK_PAGE, &page_num, -1);
    
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

  GtkWidget *menu = NULL;
  
  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeView *tree_view = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;

  gint page_num;
  gchar *path_string = NULL;
  
  tree_view = GTK_TREE_VIEW (w);

  if (e->window != gtk_tree_view_get_bin_window (tree_view)) 
    return FALSE;

  lw = gnomemeeting_get_ldap_window (gm);

  if (e->type == GDK_BUTTON_PRESS || e->type == GDK_KEY_PRESS) {

    if (gtk_tree_view_get_path_at_pos (tree_view, (int) e->x, (int) e->y,
				       &path, NULL, NULL, NULL)) {

      
      model = gtk_tree_view_get_model (GTK_TREE_VIEW (lw->tree_view));
      selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (lw->tree_view));
      
      if (selection &&
	  gtk_tree_selection_get_selected (selection, &model, &iter)) 
	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			    COLUMN_NOTEBOOK_PAGE, &page_num, -1);
      
      /* If it is a right-click, then popup a menu */
      if (e->type == GDK_BUTTON_PRESS && e->button == 3 && 
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
static void
delete_cb (GtkWidget *w,
	   gpointer data)
{
  GmLdapWindow *lw = NULL;

  lw = gnomemeeting_get_ldap_window (gm);


  if (get_selected_contact_info (NULL, NULL, NULL, NULL, NULL)) 
    delete_contact_from_group_cb (NULL, NULL);
  /* No contact is selected, but perhaps a contact section to delete
     is selected */
  else 
    delete_contact_section_cb (NULL, NULL);
}


/* DESCRIPTION  :  This callback is called when the user chooses in the menu
 *                 to call.
 * BEHAVIOR     :  Calls the user of the selected line in the GtkTreeView.
 * PRE          :  /
 */
static void
call_user_cb (GtkWidget *w,
	      gpointer data)
{
  gboolean is_group = false;

  get_selected_contact_info (NULL, NULL, NULL, NULL, &is_group);
  contact_activated_cb (NULL, NULL, NULL, 
			is_group ? GINT_TO_POINTER (CONTACTS_GROUPS)
			: GINT_TO_POINTER (CONTACTS_SERVERS));
}


/* DESCRIPTION  :  This callback is called when the user double clicks on
 *                 a row corresonding to an user.
 * BEHAVIOR     :  Add the user name in the combo box and call him.
 * PRE          :  data is the page type.
 */
static void 
contact_activated_cb (GtkTreeView *tree,
		      GtkTreePath *path,
		      GtkTreeViewColumn *column,
		      gpointer data)
{
  gchar *contact_section = NULL;
  gchar *contact_name = NULL;
  gchar *contact_url = NULL;

  gboolean is_group;
  
  GmWindow *gw = NULL;

  gw = gnomemeeting_get_main_window (gm);
  
  if (get_selected_contact_info (&contact_section, &contact_name,
				 &contact_url, NULL, &is_group)) {
    
    /* if we are waiting for a call, add the IP
       to the history, and call that user       */
    if (MyApp->Endpoint ()->GetCallingState () == 0) {
      
      /* this function will store a copy of text */
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry),
			  contact_url);
      
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->connect_button),
				    true);
    }
  }

  g_free (contact_section);
  g_free (contact_name);
  g_free (contact_url);
}


/* DESCRIPTION  :  This callback is called when the user activates 
 *                 (double click) a server in the tree_store.
 * BEHAVIOR     :  Browse the selected server.
 * PRE          :  /
 */
static void
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

	  g_free (name);
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
static void
refresh_server_content_cb (GtkWidget *w,
			   gpointer data)
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
 * BEHAVIOR     :  Returns true if contact_url corresponds to a contact
 *                 of group group_name.
 * PRE          :  /
 */
static gboolean
is_contact_member_of_group (GMURL contact_url,
			    const char *g_name)
{
  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;

  bool found = false;
  gchar *gconf_key = NULL;
  gchar *group_name = NULL;
  gchar **contact_info = NULL;

  GConfClient *client = NULL;


  if (contact_url.IsEmpty () || !g_name)
    return false;
  
  group_name = g_utf8_strdown (g_name, -1);
  gconf_key =
    g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, group_name);

  client = gconf_client_get_default ();
  
  group_content =
    gconf_client_get_list (client, gconf_key, GCONF_VALUE_STRING, NULL);
  group_content_iter = group_content;
  
  while (group_content_iter && !found) {

    if (group_content_iter->data) {

      contact_info = g_strsplit ((gchar *) group_content_iter->data, "|", 0);

      if (contact_info && contact_info [COLUMN_URL])
	if (GMURL (contact_info [COLUMN_URL]) == contact_url) {
	  
	  found = true;
	  break;
	}
      g_strfreev (contact_info);
    }

    group_content_iter = g_slist_next (group_content_iter);
  }
  
  g_slist_free (group_content);
  g_free (gconf_key);
  g_free (group_name);

  return found;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns a link to the contact corresponding to the given
 *                 contact_url in the group_content list, or NULL if none.
 * PRE          :  /
 */
static GSList *
find_contact_in_group_content (const char *contact_url,
			       GSList *group_content)
{
  GSList *group_content_iter = NULL;
  
  gchar **group_content_split = NULL;
  
  group_content_iter = group_content;
  while (group_content_iter) {

    if (group_content_iter->data) {
      
      /* The member 1 of the split coming from the key is the
	 user url, compare it with the url of the user
	 before we edited it, to remove the old version of
	 the user in the gconf database */
      group_content_split =
	g_strsplit ((char *) group_content_iter->data, "|", 0);

      if ((group_content_split && group_content_split [1] && contact_url
	   && !strcasecmp (group_content_split [1], contact_url))
	  || (!contact_url && !group_content_split [1]))
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
 *                 server and not a group of contacts. All allocated
 *                 pointers should be freed.
 * PRE          :  /
 */
static gboolean
get_selected_contact_info (gchar **contact_section,
			   gchar **contact_name,
			   gchar **contact_url,
			   gchar **contact_speed_dial,
			   gboolean *is_group)
{
  GtkWidget *page = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;

  int page_num = 0;

  GmLdapWindowPage *lwp = NULL;
  GmLdapWindow *lw = NULL;

  lw = gnomemeeting_get_ldap_window (gm);
  
  /* Get the required data from the GtkNotebook page */
  page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (lw->notebook));
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), page_num);
  lwp = gnomemeeting_get_ldap_window_page (page);
  
  if (page && lwp) {

    if (contact_section)
      *contact_section = g_utf8_strdown (lwp->contact_section_name, -1);
    // should be freed
      
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (lwp->tree_view));
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (lwp->tree_view));
    
    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
      
      /* If the callback is called because we add a contact from the
	 server listing */
      if (lwp->page_type == CONTACTS_SERVERS) {

	if (contact_name)
	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			      COLUMN_ILS_NAME, contact_name, -1);

	if (contact_url)
	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			      COLUMN_ILS_URL, contact_url, -1);

	if (is_group)
	  *is_group = false;
	}
      else {

	if (contact_name)
	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			      COLUMN_NAME, contact_name, -1);
	
	if (contact_url)
	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			      COLUMN_URL, contact_url, -1);

	if (contact_speed_dial)
	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			      COLUMN_SPEED_DIAL, contact_speed_dial, -1);

	if (is_group)
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


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Updates the menu sensitivity following what is selected
 *                 in the addressbook: a contact of a group, a contact section,
 *                 a contact of ILS.
 * PRE          :  true if the contact is currently selected from a group,
 *                 true if a contact section is selected,
 *                 true if the selected contact is currently selected from ILS
 *                 and if he is not already member of a group.
 */
static void
update_menu_sensitivity (gboolean is_group,
			 gboolean is_section,
			 gboolean is_new)
{
  GmLdapWindow *lw = NULL;
  
  lw = gnomemeeting_get_ldap_window (gm);

  if (is_group || is_section) {
      
    gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu[4].widget),
			      TRUE);
  }
  else {
    
    gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu[4].widget),
			      FALSE);
  }
  
  if (is_group || !is_new) {
    
    gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu[8].widget),
			      TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu[11].widget),
			      FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu[13].widget),
			      TRUE);
  }
  else {
    
    gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu[8].widget),
			      TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu[11].widget),
			      TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu[13].widget),
			      FALSE);
  }

  
  if (is_section) {

    gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu[8].widget),
			      FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu[11].widget),
			      FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (lw->addressbook_menu[13].widget),
			      FALSE);
  }
}


/* The functions */
void
gnomemeeting_init_ldap_window ()
{
  GtkWidget *hpaned = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *menubar = NULL;
  GtkWidget *scroll = NULL;
  GdkPixbuf *icon = NULL;

  GtkCellRenderer *cell = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkAccelGroup *accel = NULL;
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

  gw->ldap_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  icon = gtk_widget_render_icon (GTK_WIDGET (gw->ldap_window),
				 GM_STOCK_ADDRESSBOOK_16,
				 GTK_ICON_SIZE_MENU, NULL);
  gtk_window_set_title (GTK_WINDOW (gw->ldap_window), 
			_("GnomeMeeting Address Book"));
  gtk_window_set_icon (GTK_WINDOW (gw->ldap_window), icon);
  gtk_window_set_position (GTK_WINDOW (gw->ldap_window), 
			   GTK_WIN_POS_CENTER);
  gtk_window_set_default_size (GTK_WINDOW (gw->ldap_window), 670, 370);
  g_object_unref (icon);
  
  accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (gw->ldap_window), accel);
  
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
       GTK_STOCK_DELETE, 'd', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (delete_cb), NULL, NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},

      {_("_Close"), NULL,
       GTK_STOCK_CLOSE, 'w', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (gnomemeeting_component_view),
       (gpointer) gw->ldap_window,
       NULL},

      {_("C_ontact"), NULL, NULL, 0, MENU_NEW, NULL, NULL, NULL},

      {_("C_all Contact"), NULL,
       NULL, 0, MENU_ENTRY, 
       GTK_SIGNAL_FUNC (call_user_cb), NULL, NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},

      {_("New _Contact"), NULL,
       GTK_STOCK_NEW, 'n', MENU_ENTRY, 
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
  gnomemeeting_build_menu (menubar, addressbook_menu, accel);
  gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);

  
  /* A hbox to put the tree and the ldap browser */
  hpaned = gtk_hpaned_new ();
  gtk_container_set_border_width (GTK_CONTAINER (hpaned), 6);
  gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 0);
  

  /* The GtkTreeView that will store the contacts sections */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_paned_add1 (GTK_PANED (hpaned), frame);
  model = gtk_tree_store_new (NUM_COLUMNS_CONTACTS, GDK_TYPE_PIXBUF, 
			      G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);

  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), 
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (frame), scroll);

  lw->tree_view = gtk_tree_view_new ();  
  gtk_tree_view_set_model (GTK_TREE_VIEW (lw->tree_view), 
			   GTK_TREE_MODEL (model));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (lw->tree_view));
  gtk_container_add (GTK_CONTAINER (scroll), lw->tree_view);
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
  gtk_paned_add2 (GTK_PANED (hpaned), vbox);  

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
  g_signal_connect (G_OBJECT (selection), "changed",
		    G_CALLBACK (contact_section_changed_cb), NULL);
  g_signal_connect_object (G_OBJECT (lw->tree_view), "event_after",
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
 *                 before this function is called, you can use
 *                 void gnomemeeting_ldap_window_destroy_notebook_pages ();
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


void gnomemeeting_ldap_window_destroy_notebook_pages ()
{
  GmLdapWindow *lw = NULL;
  GmLdapWindowPage *lwp = NULL;
  GtkWidget *page = NULL;
  int i = 0;

  lw = gnomemeeting_get_ldap_window (gm);
  
  while ((page =
	  gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), i))) {

    lwp = gnomemeeting_get_ldap_window_page (page);
    if (lwp) {

      lwp->search_quit_mutex.Wait ();
      gtk_widget_destroy (page);
    }
    
    i++;
  }
}


int
gnomemeeting_init_ldap_window_notebook (gchar *text_label,
					int type)
{
  GtkWidget *page = NULL;
  GtkWidget *scroll = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *handle = NULL;
  GtkWidget *menu = NULL;
  GtkWidget *menu_item = NULL;
  GtkWidget *refresh_button = NULL;

  PangoAttrList *attrs = NULL; 
  PangoAttribute *attr = NULL; 

  GConfClient *client = NULL;
  
  GtkListStore *users_list_store = NULL;
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
	&& !strcasecmp (current_lwp->contact_section_name, text_label))
      return cpt;

    cpt++;
  }

  
  GmLdapWindowPage *lwp = new (GmLdapWindowPage);
  lwp->contact_section_name = g_utf8_strdown (text_label, -1);
  lwp->ils_browser = NULL;
  lwp->search_entry = NULL;
  lwp->option_menu = NULL;
  lwp->page_type = type;
  
  if (type == CONTACTS_SERVERS)
    users_list_store = 
      gtk_list_store_new (NUM_COLUMNS_SERVERS, GDK_TYPE_PIXBUF, G_TYPE_BOOLEAN,
			  G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING,
			  G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
			  G_TYPE_STRING, G_TYPE_STRING);
  else
    users_list_store = 
      gtk_list_store_new (NUM_COLUMNS_GROUPS, G_TYPE_STRING, G_TYPE_STRING,
			  G_TYPE_STRING);
			  
  vbox = gtk_vbox_new (FALSE, 0);
  scroll = gtk_scrolled_window_new (NULL, NULL);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), 
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);

  lwp->tree_view = 
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (users_list_store));
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
    column = gtk_tree_view_column_new_with_attributes (_("URL"),
						       renderer,
						       "text", 
						       COLUMN_ILS_URL,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ILS_URL);
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
    column = gtk_tree_view_column_new_with_attributes (_("URL"),
						       renderer,
						       "text", 
						       COLUMN_URL,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_URL);
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


  /* Copied from the prefs window, please use the same logic */
  lwp->section_name = gtk_label_new (text_label);
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
  gtk_misc_set_alignment (GTK_MISC (lwp->section_name), 0.0, 0.5);
  gtk_container_add (GTK_CONTAINER (frame), lwp->section_name);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  attrs = pango_attr_list_new ();
  attr = pango_attr_scale_new (PANGO_SCALE_LARGE);
  attr->start_index = 0;
  attr->end_index = G_MAXUINT; 
  pango_attr_list_insert (attrs, attr); 
  attr = pango_attr_weight_new (PANGO_WEIGHT_HEAVY);
  attr->start_index = 0;
  attr->end_index = G_MAXUINT;
  pango_attr_list_insert (attrs, attr);
  gtk_label_set_attributes (GTK_LABEL (lwp->section_name), attrs);
  pango_attr_list_unref (attrs);


  /* Add the tree view*/
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
		    G_CALLBACK (dnd_drag_data_get_cb), NULL);

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
    gnomemeeting_addressbook_group_populate (users_list_store,
					     text_label);
  
  /* Signal to call the person on the double-clicked row */
  g_signal_connect (G_OBJECT (lwp->tree_view), "row_activated", 
		    G_CALLBACK (contact_activated_cb), NULL);

  /* Right-click on a contact */
  g_signal_connect (G_OBJECT (lwp->tree_view), "event_after",
		    G_CALLBACK (contact_clicked_cb), NULL);

  return page_num;
}


void
gnomemeeting_addressbook_group_populate (GtkListStore *list_store,
					 char *g_name)
{
  GtkTreeIter list_iter;
  
  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;

  char **contact_info = NULL;
  gchar *gconf_key = NULL;
  gchar *group_name = NULL;

  GConfClient *client = NULL;

  client = gconf_client_get_default ();
  
  gtk_list_store_clear (GTK_LIST_STORE (list_store));

  group_name = g_utf8_strdown (g_name, -1);
  gconf_key =
    g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, (char *) group_name);

  group_content =
    gconf_client_get_list (client, gconf_key, GCONF_VALUE_STRING, NULL);
  group_content_iter = group_content;
  
  while (group_content_iter && group_content_iter->data) {

    gtk_list_store_append (list_store, &list_iter);

    contact_info =
      g_strsplit ((char *) group_content_iter->data, "|", 0);

    if (contact_info [0])
      gtk_list_store_set (list_store, &list_iter,
			  COLUMN_NAME, contact_info [0], -1);
    if (contact_info [1])
      gtk_list_store_set (list_store, &list_iter,
			  COLUMN_URL, contact_info [1], -1);

    if (contact_info [2])
      gtk_list_store_set (list_store, &list_iter,
			  COLUMN_SPEED_DIAL, contact_info [2], -1);
    
    g_strfreev (contact_info);
    group_content_iter = g_slist_next (group_content_iter);
  }

  g_free (gconf_key);
  g_free (group_name);
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
  

  /* Update sensitivity */
  update_menu_sensitivity (false, true, false);
}


GMURL
gnomemeeting_addressbook_get_url_from_speed_dial (const char *url)
{
  gchar *group_content_gconf_key = NULL;
  gchar *group_name = NULL;
  char **contact_info = NULL;
  
  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;
  GSList *groups = NULL;
  GSList *groups_iter = NULL;

  GMURL result;
  
  GConfClient *client = NULL;

  if (!url || (url && !strcmp (url, "")))
    return result;

  client = gconf_client_get_default ();

  groups =
    gconf_client_get_list (client, CONTACTS_KEY "groups_list",
			   GCONF_VALUE_STRING, NULL);
  groups_iter = groups;
  
  while (groups_iter && groups_iter->data) {
    
    group_name = g_utf8_strdown ((char *) groups_iter->data, -1);
    group_content_gconf_key =
      g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, group_name);
		       

    group_content =
      gconf_client_get_list (client, group_content_gconf_key,
			     GCONF_VALUE_STRING, NULL);
    group_content_iter = group_content;
    
    while (group_content_iter
	   && group_content_iter->data && result.IsEmpty ()) {
      
      contact_info =
	g_strsplit ((char *) group_content_iter->data, "|", 0);

      if (contact_info [2] && strcmp (contact_info [2], "")
	  && !strcasecmp (contact_info [2], (const char *) url)) 
	result = GMURL (contact_info [1]);

      g_strfreev (contact_info);
      group_content_iter = g_slist_next (group_content_iter);
    }

    g_free (group_content_gconf_key);
    g_slist_free (group_content);

    groups_iter = g_slist_next (groups_iter);
    g_free (group_name);
  }

  g_slist_free (groups);

  return result;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns the speed dial corresponding to the
 *                 URL given as parameter. NULL if none.
 * PRE          :  /
 *
 */
static gchar *
gnomemeeting_addressbook_get_speed_dial_from_url (GMURL url)
{
  gchar *group_content_gconf_key = NULL;
  char **contact_info = NULL;
  
  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;

  GSList *groups = NULL;
  GSList *groups_iter = NULL;

  gchar *group_name = NULL;
  gchar *result = NULL;
  
  GConfClient *client = NULL;

  if (!url.IsEmpty ())
    return result;

  client = gconf_client_get_default ();

  groups =
    gconf_client_get_list (client, CONTACTS_KEY "groups_list",
			   GCONF_VALUE_STRING, NULL);
  groups_iter = groups;
  
  while (groups_iter && groups_iter->data) {
    
    group_name = g_utf8_strdown ((char *) groups_iter->data, -1);
    group_content_gconf_key =
      g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, group_name);


    group_content =
      gconf_client_get_list (client, group_content_gconf_key,
			     GCONF_VALUE_STRING, NULL);
    group_content_iter = group_content;
    
    while (group_content_iter
	   && group_content_iter->data && !result) {
      
      contact_info =
	g_strsplit ((char *) group_content_iter->data, "|", 0);
      
      if (contact_info [1] && url == GMURL (contact_info [1]))
	result = g_strdup (contact_info [2]);

      g_strfreev (contact_info);
      group_content_iter = g_slist_next (group_content_iter);
    }

    g_free (group_content_gconf_key);
    g_slist_free (group_content);

    groups_iter = g_slist_next (groups_iter);
    g_free (group_name);
  }

  g_slist_free (groups);

  return result;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns true if the contact corresponding to the given URL
 *                 is member of the addressbook.
 * PRE          :  /
 *
 */
static gboolean
is_contact_member_of_addressbook (GMURL url)
{
  gchar *group_name = NULL;
  gchar *group_content_gconf_key = NULL;
  char **contact_info = NULL;

  gboolean result = false;
  
  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;
  
  GSList *groups = NULL;
  GSList *groups_iter = NULL;
  
  GConfClient *client = NULL;

  if (url.IsEmpty ())
    return result;

  client = gconf_client_get_default ();

  groups =
    gconf_client_get_list (client, CONTACTS_KEY "groups_list",
			   GCONF_VALUE_STRING, NULL);
  groups_iter = groups;
  
  while (groups_iter && groups_iter->data) {
    
    group_name = g_utf8_strdown ((char *) groups_iter->data, -1);
    group_content_gconf_key =
      g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, group_name);
		       

    group_content =
      gconf_client_get_list (client, group_content_gconf_key,
			     GCONF_VALUE_STRING, NULL);
    group_content_iter = group_content;
    
    while (group_content_iter && group_content_iter->data && !result) {
      
      contact_info =
	g_strsplit ((char *) group_content_iter->data, "|", 0);
      
      if (contact_info [1] && (GMURL (contact_info [1]) ==  url))
	result = true;

      g_strfreev (contact_info);
      group_content_iter = g_slist_next (group_content_iter);
    }

    g_free (group_content_gconf_key);
    g_free (group_name);
    g_slist_free (group_content);

    groups_iter = g_slist_next (groups_iter);
  }

  g_slist_free (groups);

  return result;
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns true if the group given as parameter already
 *                 is member of the addressbook.
 * PRE          :  /
 *
 */
static gboolean
is_group_member_of_addressbook (const char *group)
{
  gboolean result = false;
  
  GSList *groups = NULL;
  GSList *groups_iter = NULL;
  
  GConfClient *client = NULL;

  if (!group)
    return result;

  client = gconf_client_get_default ();

  groups =
    gconf_client_get_list (client, CONTACTS_KEY "groups_list",
			   GCONF_VALUE_STRING, NULL);
  groups_iter = groups;
  
  while (groups_iter && groups_iter->data && !result) {
    
    if (!strcasecmp (group, (char *) groups_iter->data))
      result = true;

    groups_iter = g_slist_next (groups_iter);
  }

  g_slist_free (groups);

  return result;
}
