
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
 */

/*
 *                         ldap.cpp  -  description
 *                         ------------------------
 *   begin                : Wed Feb 28 2003
 *   copyright            : (C) 2000-2003 by Damien Sandras 
 *   description          : This file contains functions to build the 
 *                          addressbook window.
 *   email                : dsandras@seconix.com
 *
 */

#include "../config.h"

#include "ldap_window.h"
#include "misc.h"
#include "ils.h"
#include "gnomemeeting.h"
#include "menu.h"

#ifndef DISABLE_GNOME
#include <gnome.h>
#endif

#include "../pixmaps/xdap-directory.xpm"


/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	


static gboolean dnd_drag_motion_cb (GtkWidget *, GdkDragContext *, int, int,
				    guint, gpointer);
static void dnd_drag_data_received_cb (GtkWidget *, GdkDragContext *, int, int,
				       GtkSelectionData *, guint, guint,
				       gpointer);
static void dnd_drag_data_get_cb (GtkWidget *, GdkDragContext *,
				  GtkSelectionData *, guint, guint, gpointer);
static void add_contact_to_group_cb (GtkWidget *, gpointer);
static void edit_contact_cb (GtkWidget *, gpointer);
static void delete_contact_from_group_cb (GtkWidget *, gpointer);
static void new_contact_section_cb (GtkWidget *, gpointer);
static void delete_contact_section_cb (GtkWidget *, gpointer);
static void call_user_cb (GtkWidget *, gpointer);
static void contact_activated_cb (GtkTreeView *, GtkTreePath *,
				      GtkTreeViewColumn *, gpointer);
static gint ldap_window_clicked (GtkWidget *, GdkEvent *, gpointer);
static void contact_section_changed_cb (GtkTreeSelection *, gpointer);
static void server_content_refresh (GtkWidget *, gint);
static void refresh_server_content_cb (GtkWidget *, gpointer);
static void contact_section_activated_cb (GtkTreeView *, GtkTreePath *,
						 GtkTreeViewColumn *);
static gboolean is_contact_member_of_group (gchar *, gchar *);
static GSList *find_contact_in_group_content (gchar *, GSList *);
static gboolean get_selected_contact_info (GtkNotebook *, gchar **,
					   gchar **, gchar **);
static gint contact_section_event_after_cb (GtkWidget *, GdkEventButton *,
					    gpointer);
static gint contact_event_after_cb (GtkWidget *, GdkEventButton *, gpointer);


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

  GmLdapWindow *lw = NULL;
  
  GValue value =  {0, };
  GtkTreeIter iter;

  lw = gnomemeeting_get_ldap_window (gm);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

  /* Get the callto field of the contact info from the source GtkTreeView */
  if (get_selected_contact_info (GTK_NOTEBOOK (lw->notebook), &contact_section,
				 &contact_name, &contact_callto)) {
    

    /* See if the path in the destination GtkTreeView corresponds to a valid
       row (ie a group row, and a row corresponding to a group the user
       doesn't belong to */
    if (gtk_tree_view_get_dest_row_at_pos (GTK_TREE_VIEW (tree_view),
					   x, y, &path, NULL)) {

      if (gtk_tree_model_get_iter (model, &iter, path)) {
	    
	gtk_tree_model_get_value (model, &iter, 0, &value);
	group_name = g_strdup (g_value_get_string (&value));
	g_value_unset (&value);


	/* If the user doesn't belong to the selected group and if
	   the selected row corresponds to a group and not a server */
	if (group_name && contact_callto &&
	    !is_contact_member_of_group (contact_callto, group_name)) {
      
	  if (gtk_tree_path_get_depth (path) >= 2 &&
	      gtk_tree_path_get_indices (path) [0] >= 1)
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
      
	gtk_tree_model_get_value (model, &iter, 0, &value);
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


/* DESCRIPTION  :  This callback is called when the user has chosen to add
 *                 a contact to a group.
 * BEHAVIOR     :  Adds the user to the given group by modifying the gconf key.
 * PRE          :  data = the group name.
 */
void
add_contact_to_group_cb (GtkWidget *widget,
			 gpointer data)
{
  GSList *group_list = NULL;

  GmLdapWindow *lw = NULL;

  gchar *gconf_key = NULL;
  gchar *contact_name = NULL;
  gchar *contact_section = NULL;
  gchar *contact_callto = NULL;
  gchar *contact_info = NULL;

  GConfClient *client = NULL;
  
  client = gconf_client_get_default ();
  lw = gnomemeeting_get_ldap_window (gm);


  /* If we got a group name as data */
  if (data) {
     
    /* The key is for example CONTACTS_GROUPS_KEY/Friends */
    gconf_key = 
      g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, 
		       (char *) data);

    group_list = 
      gconf_client_get_list (client, gconf_key, GCONF_VALUE_STRING, NULL);

    if (get_selected_contact_info (GTK_NOTEBOOK (lw->notebook),
				   &contact_section, &contact_name,
				   &contact_callto)) {

      contact_info = 
	g_strdup_printf ("%s|%s", contact_name, contact_callto);
      
      /* Update the gconf key if needed */
      if (!is_contact_member_of_group (contact_callto, (char *) data)) {

	group_list = g_slist_append (group_list, contact_info);
	gconf_client_set_list (client, gconf_key, GCONF_VALUE_STRING, 
			       group_list, NULL);
      }

      g_free (contact_info);
    }

    g_slist_free (group_list);
    g_free (gconf_key);
  }
}


/* DESCRIPTION  :  This callback is called when the user choose to edit the
 *                 info of a contact from a group.
 * BEHAVIOR     :  Opens a popup, and save the modified info by modifying the
 *                 gconf key when the user clicks on "ok".
 * PRE          :  /
 */
void
edit_contact_cb (GtkWidget *widget,
		 gpointer data)
{
  GConfClient *client = NULL;

  GtkWidget *table = NULL;
  GtkWidget *dialog = NULL;
  GtkWidget *label = NULL;
  GtkWidget *name_entry = NULL;
  GtkWidget *callto_entry = NULL;
  
  gchar *contact_name = NULL;
  gchar *contact_callto = NULL;
  gchar *gconf_key = NULL;
  gchar *contact_info = NULL;
  gchar *old_contact_callto = NULL;
  gchar *contact_section = NULL;

  int result = 0;
  
  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;

  GmLdapWindow *lw = NULL;
  GmWindow *gw = NULL;
  
  lw = gnomemeeting_get_ldap_window (gm);
  gw = gnomemeeting_get_main_window (gm);
  client = gconf_client_get_default ();


  /* We don't care if it fails as long as contact_section is correct */
  get_selected_contact_info (GTK_NOTEBOOK (lw->notebook),
			     &contact_section, &contact_name,
			     &contact_callto);

  gconf_key =
    g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, contact_section);
      
  /* Store the old callto to be able to delete the user later */
  old_contact_callto = g_strdup (contact_callto);
    
  /* Create the dialog to easily modify the info of a specific contact */
  dialog = gtk_dialog_new_with_buttons (_("Edit the contact information"), 
					GTK_WINDOW (gw->ldap_window),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
					GTK_STOCK_CANCEL,
					GTK_RESPONSE_REJECT,
					NULL);
    
  table = gtk_table_new (2, 2, FALSE);
  label = gtk_label_new (_("Name:"));
  name_entry = gtk_entry_new ();
  if (contact_name)
    gtk_entry_set_text (GTK_ENTRY (name_entry), contact_name);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, 
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), name_entry, 1, 2, 0, 1, 
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
    
  label = gtk_label_new (_("callto URL:"));
  callto_entry = gtk_entry_new ();
  gtk_widget_set_size_request (GTK_WIDGET (callto_entry), 300, -1);
  if (contact_callto)
    gtk_entry_set_text (GTK_ENTRY (callto_entry), contact_callto);
  else
    gtk_entry_set_text (GTK_ENTRY (callto_entry), "callto://");
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2, 
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), callto_entry, 1, 2, 1, 2, 
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    (GtkAttachOptions) (GTK_FILL | GTK_SHRINK),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
    
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), table,
		      FALSE, FALSE, 4);
  gtk_widget_show_all (dialog);
    
  result = gtk_dialog_run (GTK_DIALOG (dialog));
    
  switch (result) {
      
  case GTK_RESPONSE_ACCEPT:
      
    contact_info =
      g_strdup_printf ("%s|%s",
		       gtk_entry_get_text (GTK_ENTRY (name_entry)),
		       gtk_entry_get_text (GTK_ENTRY (callto_entry)));
      
    group_content =
      gconf_client_get_list (client, gconf_key, GCONF_VALUE_STRING, NULL);
      
      
    /* Once we find the contact corresponding to the old saved callto,
       we delete it and insert the new one at the same position */
    if (old_contact_callto) {
	
      group_content_iter =
	find_contact_in_group_content (old_contact_callto, group_content);
	
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
      
    gconf_client_set_list (client, gconf_key, GCONF_VALUE_STRING,
			   group_content, NULL);
      
    g_free (contact_info);
    g_slist_free (group_content);
      
    break;
  }
    
  g_free (gconf_key);
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

  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;
  
  GmLdapWindow *lw = NULL;

  lw = gnomemeeting_get_ldap_window (gm);
  client = gconf_client_get_default ();

  if (get_selected_contact_info (GTK_NOTEBOOK (lw->notebook),
				 &contact_section, &contact_name,
				 &contact_callto)) {

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
					GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
					GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
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
 * PRE          :  data = the current GtkTreePath in the contacts GtkTreeView
 *                 as a string.
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
  GtkTreeIter iter;
  GtkTreeModel *model = NULL;
  
  gchar *gconf_key = NULL;
  gchar *name = NULL;
  
  lw = gnomemeeting_get_ldap_window (gm);
  client = gconf_client_get_default ();

  if (data) {
    
    path = gtk_tree_path_new_from_string ((gchar *) data);
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (lw->tree_view));
    gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 0, &name, -1);
  
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

  GmLdapWindow *lw = gnomemeeting_get_ldap_window (gm);
  GmWindow *gw = gnomemeeting_get_main_window (gm);
  
  
  if (get_selected_contact_info (GTK_NOTEBOOK (lw->notebook),
				 &contact_section, &contact_name,
				 &contact_callto)) {
    
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


/* DESCRIPTION  :  This callback is called when the user clicks to
 *                 close the window (destroy or delete_event signals).
 * BEHAVIOR     :  Hides the window.
 * PRE          :  gpointer is a valid pointer to a GmLdapWindow.
 */
gint
ldap_window_clicked (GtkWidget *widget,
		     GdkEvent *ev,
		     gpointer data)
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
void
contact_section_changed_cb (GtkTreeSelection *selection,
			    gpointer data)
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

	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 0, &name,
			    1, &page_num, -1);

	if (page_num != -1) 
	  gtk_notebook_set_current_page (GTK_NOTEBOOK (lw->notebook), 
					 page_num);
      }

      gtk_tree_path_free (path);
    }
  }
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Browse the selected server with the given filter.
 * PRE          :  The GtkNotebook containing the servers listing, the page
 *                 number to browse.
 */
void
server_content_refresh (GtkWidget *notebook, gint page_num)
{
  int option_menu_option = 0;
  
  gchar *filter = NULL;
  gchar *search_entry_text = NULL;
  gchar *name = NULL;
  
  GMILSBrowser *ils_browser = NULL;
  
  GtkListStore *xdap_users_list_store = NULL;

  GtkWidget *page = NULL;
  GtkWidget *search_entry = NULL;
  GtkWidget *option_menu = NULL;


  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 
				 page_num);
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 
				    page_num);

  name =
    (gchar *) g_object_get_data (G_OBJECT (page), "contact_section");
  
  xdap_users_list_store = 
    GTK_LIST_STORE (g_object_get_data (G_OBJECT (page),
				       "list_store"));
  search_entry =
    GTK_WIDGET (g_object_get_data (G_OBJECT (page),
				   "search_entry"));
  option_menu =
    GTK_WIDGET (g_object_get_data (G_OBJECT (page),
				   "option_menu"));

  search_entry_text =
    (gchar *) gtk_entry_get_text (GTK_ENTRY (search_entry));

  if (search_entry_text && strcmp (search_entry_text, "")) {

    option_menu_option =
      gtk_option_menu_get_history (GTK_OPTION_MENU (option_menu));
	      
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
	    
  gtk_list_store_clear (xdap_users_list_store);

  /* Check if there is already a search running */
  ils_browser = (GMILSBrowser *) g_object_get_data (G_OBJECT (page), 
						    "GMILSBrowser");
  if (!ils_browser && page_num != -1) {
	
    /* Browse it */
    ils_browser = new GMILSBrowser (name, filter);
	      	      
    /* Set the pointer to the thread as data of that 
       GTK notebook page */
    g_object_set_data (G_OBJECT (page), "GMILSBrowser", ils_browser);
  }

  g_free (filter);
}


/* DESCRIPTION  :  This callback is called when the user chooses to refresh
 *                 the server content.
 * BEHAVIOR     :  Browse the selected server.
 * PRE          :  data = page_num of GtkNotebook containing the server.
 */
void refresh_server_content_cb (GtkWidget *w, gpointer data)
{
  GmLdapWindow *lw = NULL;

  lw = gnomemeeting_get_ldap_window (gm);

  server_content_refresh (GTK_WIDGET (lw->notebook), GPOINTER_TO_INT (data));
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
	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 0, &name, 
			      1, &page_num, -1);

	  if (page_num != - 1) {
    
	    server_content_refresh (GTK_WIDGET (lw->notebook), page_num);
	  }
	}
      }
    }
  }
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
 *                 selected.
 * PRE          :  /
 */
gboolean
get_selected_contact_info (GtkNotebook *notebook,
			   gchar **contact_section,
			   gchar **contact_name,
			   gchar **contact_callto)
{
  GtkWidget *page = NULL;

  GtkTreeView *tree_view = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;

  GtkTreeIter iter;

  int page_num = 0, page_type = 0;

  
  /* Get the required data from the GtkNotebook page */
  page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_num);
  
  if (page) {
    
    tree_view = 
      GTK_TREE_VIEW (g_object_get_data (G_OBJECT (page), "tree_view"));
    *contact_section = 
      (char *) g_object_get_data (G_OBJECT (page), "contact_section");
    page_type =
      GPOINTER_TO_INT (g_object_get_data (G_OBJECT (page), "page_type"));
    
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
      
      /* If the callback is called because we add a contact from the
	 server listing */
      if (page_type == CONTACTS_SERVERS) {
	
	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			    COLUMN_ILS_NAME, contact_name, 
			    COLUMN_ILS_CALLTO, contact_callto,
			    -1);
	}
      else {
	
	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			    COLUMN_NAME, contact_name, 
			    COLUMN_CALLTO, contact_callto,
			    -1);
      }
    }
    else
      return false;
  }
  else
    return false;

  
  return true;
}


/* DESCRIPTION  :  This callback is called when there is an "event_after"
 *                 signal on one of the contact section.
 * BEHAVIOR     :  Displays a popup menu with the required options.
 * PRE          :  /
 */
static gint
contact_section_event_after_cb (GtkWidget *w,
				GdkEventButton *e,
				gpointer data)
{
  GtkWidget *menu = NULL;

  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeView *tree_view = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;

  gchar *path_string = NULL;
  int page_num = -1;
  
  tree_view = GTK_TREE_VIEW (w);

  if (e->window != gtk_tree_view_get_bin_window (tree_view)) 
    return FALSE;
    

  if (e->type == GDK_BUTTON_PRESS) {

    if (gtk_tree_view_get_path_at_pos (tree_view, (int) e->x, (int) e->y,
				       &path, NULL, NULL, NULL)) {

      selection = gtk_tree_view_get_selection (tree_view);
      model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
      if (gtk_tree_selection_get_selected (selection, &model, &iter)) 
	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			    1, &page_num, -1);

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
	     (gpointer) path_string, NULL},
	    {NULL, NULL, NULL, 0, MENU_END, NULL, NULL, NULL},
	  };
	
	MenuEntry delete_group_new_contact_section_menu [] =
	  {
	    {_("New contact"), NULL,
	     NULL, 0, MENU_ENTRY, 
	     GTK_SIGNAL_FUNC (edit_contact_cb), 
	     NULL, NULL},
	    {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},
	    {_("Delete"), NULL,
	     GTK_STOCK_DELETE, 0, MENU_ENTRY, 
	     GTK_SIGNAL_FUNC (delete_contact_section_cb), 
	     (gpointer) path_string, NULL},
	    {NULL, NULL, NULL, 0, MENU_END, NULL, NULL, NULL},
	  };

	/* Build the appropriate popup menu */
	if (gtk_tree_path_get_depth (path) >= 2)
	  if (gtk_tree_path_get_indices (path) [0] == 0)
	    gnomemeeting_build_menu (menu, delete_refresh_contact_section_menu, NULL);
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
	
	gtk_tree_path_free (path);

	return TRUE;
      }
    }
  }

  return FALSE;
}


/* DESCRIPTION  :  This callback is called when there is an "event_after"
 *                 signal on one of the contacts.
 * BEHAVIOR     :  Displays a popup menu with the required options.
 * PRE          :  /
 */
static gint
contact_event_after_cb (GtkWidget *w,
			GdkEventButton *e,
			gpointer data)
{
  GtkWidget *menu = NULL;

  GtkTreeSelection *selection = NULL;
  GtkTreeView *tree_view = NULL;
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;
  
  GConfClient *client = NULL;

  GSList *contacts_list = NULL;
  GSList *contacts_list_iter = NULL;
  GSList *last = NULL;

  gchar *contact_callto = NULL;
  int cpt = 0, i = 0, index = 0;

  tree_view = GTK_TREE_VIEW (w);

  if (e->window != gtk_tree_view_get_bin_window (tree_view)) 
    return FALSE;
    
  client = gconf_client_get_default ();

  if (e->type == GDK_BUTTON_PRESS) {

    if (gtk_tree_view_get_path_at_pos (tree_view, (int) e->x, (int) e->y,
				       &path, NULL, NULL, NULL)) {

      selection = gtk_tree_view_get_selection (tree_view);
      model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

      gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);

      if (GPOINTER_TO_INT (data) == CONTACTS_GROUPS)
	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			    COLUMN_CALLTO, &contact_callto, -1);
      else
	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			    COLUMN_ILS_CALLTO, &contact_callto, -1);
      
      if (e->button == 3 && 
	  gtk_tree_selection_path_is_selected (selection, path)) {
	
	menu = gtk_menu_new ();

	contacts_list =
	  gconf_client_get_list (client, CONTACTS_KEY "groups_list", 
				 GCONF_VALUE_STRING, NULL); 

	if (contacts_list) {

	  last = g_slist_last (contacts_list);
	  i = g_slist_position (contacts_list, last);

	  i = i + 5; /* There are maximum : i+1 elements + Contact
			+ Add to group + Remove user */

	  MenuEntry user_menu [i];

	  MenuEntry call_entry =
	    {_("Call this user"), NULL,
	     NULL, 0, MENU_ENTRY, 
	     GTK_SIGNAL_FUNC (call_user_cb), data, NULL};
	  user_menu [0] = call_entry;
	  cpt = 1;
	  index = 2;
	  
	  if (GPOINTER_TO_INT (data) == CONTACTS_GROUPS) {
	    
	    MenuEntry edit_entry =
	      {_("Edit this user information"), NULL,
	       NULL, 0, MENU_ENTRY,
	       GTK_SIGNAL_FUNC (edit_contact_cb),
	       NULL, NULL};
	    user_menu [cpt] = edit_entry;

	    MenuEntry delete_entry =
	      {_("Delete this user"), NULL,
	       GTK_STOCK_DELETE, 0, MENU_ENTRY,
	       GTK_SIGNAL_FUNC (delete_contact_from_group_cb),
	       NULL, NULL};
	    user_menu [cpt + 1] = delete_entry;
	    
	    cpt = 3;
	    index = 4;
	  }

	  MenuEntry group_entry =
	    {_("Add to group"), NULL,
	     GTK_STOCK_ADD, 0, MENU_SUBMENU_NEW, 
	     NULL, NULL, NULL};
	  user_menu [cpt] = group_entry;
	  cpt++;
	    
	  contacts_list_iter = contacts_list;
	  while (contacts_list_iter) {

	    /* An user is uniquely identified by his callto URL,
	       we will check if the user already belongs to the group or
	       not to only propose to add him to the groups he doesn't
	       belong to */
	    if (!is_contact_member_of_group (contact_callto,
					     (gchar *) contacts_list_iter->data)){
	    
	      MenuEntry entry =
		{(char *) contacts_list_iter->data, NULL,
		 NULL, 0, MENU_ENTRY, 
		 GTK_SIGNAL_FUNC (add_contact_to_group_cb), 
		 (char *) contacts_list_iter->data, NULL};

	      user_menu [cpt] =  entry;
	      cpt++;
	    }
	    
	    contacts_list_iter = contacts_list_iter->next;
	  }
	  
	  MenuEntry end_menu = 
	    {NULL, NULL, NULL, 0, MENU_END, NULL, NULL, NULL};
	  user_menu [cpt] = end_menu;

	  gnomemeeting_build_menu (menu, user_menu, NULL);
	
	  gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
			  e->button, e->time);
	  g_signal_connect (G_OBJECT (menu), "hide",
			    GTK_SIGNAL_FUNC (g_object_unref), (gpointer) menu);
	  g_object_ref (G_OBJECT (menu));
	  gtk_object_sink (GTK_OBJECT (menu));
	
	  gtk_tree_path_free (path);
	  g_slist_free (contacts_list);

	  return TRUE;
	}
      }
    }
  }
  return FALSE;
}


/* The functions */
void
gnomemeeting_init_ldap_window ()
{
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *frame = NULL;
  GdkPixbuf *xdap_pixbuf = NULL;

  GtkCellRenderer *cell = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkTreeStore *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;
  GtkTreeIter child_iter;

  GSList *ldap_servers_list = NULL;
  GSList *groups_list = NULL;
  GSList *ldap_servers_list_iter = NULL;
  GSList *groups_list_iter = NULL;

  int p = 0, cpt = 0;

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
			_("GnomeMeeting Addressbook"));
  gtk_window_set_icon (GTK_WINDOW (gw->ldap_window), xdap_pixbuf);
  gtk_window_set_position (GTK_WINDOW (gw->ldap_window), 
			   GTK_WIN_POS_CENTER);
  gtk_window_set_default_size (GTK_WINDOW (gw->ldap_window), 550, 310);
  g_object_unref (G_OBJECT (xdap_pixbuf));


  /* A hbox to put the tree and the ldap browser */
  hbox = gtk_hbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_container_add (GTK_CONTAINER (gw->ldap_window), hbox);
  

  /* The GtkTreeView that will store the contacts sections */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
  model = gtk_tree_store_new (2, G_TYPE_STRING, G_TYPE_INT);
  lw->tree_view = gtk_tree_view_new ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (lw->tree_view), 
			   GTK_TREE_MODEL (model));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (lw->tree_view));
  gtk_container_add (GTK_CONTAINER (frame), lw->tree_view);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (lw->tree_view), FALSE);

  gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection),
			       GTK_SELECTION_BROWSE);

  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Contacts"),
						     cell, "text", 0, NULL);
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

  /* Populate the tree view : servers */
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter, 0, _("Servers"), 1, 0, -1);

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
			0, ldap_servers_list_iter->data, 
			1, p, -1);

    ldap_servers_list_iter = ldap_servers_list_iter->next;
    cpt++;
  }
  g_slist_free (ldap_servers_list);

  /* Populate the tree view : groups */
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter, 0, _("Groups"), 1, 0, -1);

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
			0, groups_list_iter->data, 
			1, p, -1);

    groups_list_iter = groups_list_iter->next;
    cpt++;
  }
  g_slist_free (groups_list);


  /* Expand all */
  gtk_tree_view_expand_all (GTK_TREE_VIEW (lw->tree_view));
  path = gtk_tree_path_new_from_string ("0:0");
  gtk_tree_view_set_cursor (GTK_TREE_VIEW (lw->tree_view), path,
			    NULL, false);
  gtk_tree_path_free (path);
  
  /* Drag and Drop Setup */
  gtk_drag_dest_set (GTK_WIDGET (lw->tree_view), GTK_DEST_DEFAULT_ALL,
		     dnd_targets, 1,
		     GDK_ACTION_COPY);
  g_signal_connect (G_OBJECT (lw->tree_view), "drag_motion",
		    G_CALLBACK (dnd_drag_motion_cb), 0);
  g_signal_connect (G_OBJECT (lw->tree_view), "drag_data_received",
		    G_CALLBACK (dnd_drag_data_received_cb), 0);
  
  /* Click on a server name or on a contact group */
  g_signal_connect (G_OBJECT (selection), "changed",
		    G_CALLBACK (contact_section_changed_cb), NULL);

  /* Double-click on a server name or on a contact group */
  g_signal_connect (G_OBJECT (lw->tree_view), "row_activated",
		    G_CALLBACK (contact_section_activated_cb), NULL);  

  /* Right-click on a server name or on a contact group */
  g_signal_connect_object (G_OBJECT (lw->tree_view), "event-after",
			   G_CALLBACK (contact_section_event_after_cb), 
			   NULL, (enum GConnectFlags) 0);

  g_signal_connect (G_OBJECT (gw->ldap_window), "delete_event",
		    G_CALLBACK (ldap_window_clicked), (gpointer) gw);
}


int
gnomemeeting_init_ldap_window_notebook (gchar *text_label,
					int type)
{
  GtkWidget *page = NULL;
  GtkWidget *scroll = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *statusbar = NULL;
  GtkWidget *handle = NULL;
  GtkWidget *toolbar = NULL;
  GtkWidget *menu = NULL;
  GtkWidget *menu_item = NULL;
  GtkWidget *option_menu = NULL;
  GtkWidget *search_entry = NULL;
  GtkWidget *refresh_button = NULL;
  
  GConfClient *client = NULL;
  
  /* For the GTK TreeView */
  GtkWidget *tree_view = NULL;
  GtkListStore *xdap_users_list_store = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkCellRenderer *renderer = NULL;

  int cpt = 0, page_num = 0;
  char *contact_section = NULL;

  static GtkTargetEntry dnd_targets [] =
    {
      {"text/plain", GTK_TARGET_SAME_APP, 0}
    };
 
  GmLdapWindow *lw = gnomemeeting_get_ldap_window (gm);
  client = gconf_client_get_default ();
 
  /* We check that the requested server name is not already in the list,
   * if it is already in the list, we return its page number */
  while ((page =
	  gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook), cpt)) ){

    contact_section = 
      (gchar *) g_object_get_data (G_OBJECT (page), "contact_section");

    if (contact_section && !strcmp (contact_section, text_label)) 
      return cpt;

    cpt++;
  }

  
  if (type == CONTACTS_SERVERS)
    xdap_users_list_store = 
      gtk_list_store_new (NUM_COLUMNS_SERVERS, GDK_TYPE_PIXBUF, G_TYPE_BOOLEAN,
			  G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING,
			  G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
			  G_TYPE_STRING, G_TYPE_STRING);
  else
    xdap_users_list_store = 
      gtk_list_store_new (NUM_COLUMNS_GROUPS, G_TYPE_STRING, G_TYPE_STRING,
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
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

    renderer = gtk_cell_renderer_toggle_new ();
    /* Translators: This is "A" as in "Audio" */
    column = gtk_tree_view_column_new_with_attributes (_("A"),
						       renderer,
						       "active", 
						       COLUMN_ILS_AUDIO,
						       NULL);
    gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

    renderer = gtk_cell_renderer_toggle_new ();
    /* Translators: This is "V" as in "Video" */
    column = gtk_tree_view_column_new_with_attributes (_("V"),
						       renderer,
						       "active", 
						       COLUMN_ILS_VIDEO,
						       NULL);
    gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

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
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    g_object_set (G_OBJECT (renderer), "weight", "bold",
		  "style", PANGO_STYLE_ITALIC, NULL);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Comment"),
						       renderer,
						       "text", 
						       COLUMN_ILS_COMMENT,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ILS_COMMENT);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC, NULL);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Location"),
						       renderer,
						       "text", 
						       COLUMN_ILS_LOCATION,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ILS_LOCATION);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Callto"),
						       renderer,
						       "text", 
						       COLUMN_ILS_CALLTO,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ILS_CALLTO);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
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
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC, NULL);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("IP"),
						       renderer,
						       "text", 
						       COLUMN_ILS_IP,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_ILS_IP);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
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
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC,
		  "weight", "bold", NULL);
    
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Callto URL"),
						       renderer,
						       "text", 
						       COLUMN_CALLTO,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_CALLTO);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_column_set_min_width (GTK_TREE_VIEW_COLUMN (column), 175);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
    g_object_set (G_OBJECT (renderer), "foreground", "blue",
		  "underline", TRUE, NULL);
    
  }

  gtk_container_add (GTK_CONTAINER (scroll), tree_view);
  gtk_container_set_border_width (GTK_CONTAINER (tree_view), 0);
  gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);

  if (type == CONTACTS_SERVERS) {

    /* The toolbar */
    handle = gtk_handle_box_new ();
    toolbar = gtk_toolbar_new ();
    gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH_HORIZ);
    gtk_box_pack_start (GTK_BOX (vbox), handle, FALSE, FALSE, 0);  
    gtk_container_add (GTK_CONTAINER (handle), toolbar);
    gtk_container_set_border_width (GTK_CONTAINER (handle), 0);
    gtk_container_set_border_width (GTK_CONTAINER (toolbar), 0);

    
    /* option menu */
    gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));
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

    option_menu = gtk_option_menu_new ();
    gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu),
			      menu);
    gtk_option_menu_set_history (GTK_OPTION_MENU (option_menu),
				 1);
    gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), 
			       GTK_WIDGET (option_menu),
			       NULL, NULL);
    gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));

    
    /* entry */
    search_entry = gtk_entry_new ();
    gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), 
			       GTK_WIDGET (search_entry),
			       NULL, NULL);
    gtk_widget_set_size_request (GTK_WIDGET (search_entry), 240, -1);
    gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));
    gtk_widget_show_all (handle);


    /* The Refresh button */
    refresh_button = gtk_button_new_from_stock (GTK_STOCK_REFRESH);
    gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), 
			       GTK_WIDGET (refresh_button),
			       NULL, NULL);
    gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));
    
    
    /* The statusbar */
    statusbar = gtk_statusbar_new ();
    gtk_box_pack_start (GTK_BOX (vbox), statusbar, FALSE, FALSE, 0);
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar), FALSE);
  }


  /* The drag and drop information */
  gtk_drag_source_set (GTK_WIDGET (tree_view),
		       GDK_BUTTON1_MASK, dnd_targets, 1,
		       GDK_ACTION_COPY);
  g_signal_connect (G_OBJECT (tree_view), "drag_data_get",
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
  if (search_entry) {
    
    g_signal_connect (G_OBJECT (search_entry), "activate",
		      G_CALLBACK (refresh_server_content_cb), 
		      GINT_TO_POINTER (page_num));
    g_signal_connect (G_OBJECT (refresh_button), "clicked",
		      G_CALLBACK (refresh_server_content_cb), 
		      GINT_TO_POINTER (page_num));
  }
  
  
  /* If the type of page is "groups", then we populate the page */
  if (type == CONTACTS_GROUPS) 
    gnomemeeting_addressbook_group_populate (xdap_users_list_store,
					     text_label);
  
  
  /* Store the list_store and the tree view as data for the page */
  g_object_set_data (G_OBJECT (page), "list_store", 
		     (gpointer) (xdap_users_list_store));
  g_object_set_data (G_OBJECT (page), "tree_view",
		     (gpointer) (tree_view));
  g_object_set_data (G_OBJECT (page), "statusbar", 
		     (gpointer) (statusbar));
  g_object_set_data (G_OBJECT (page), "contact_section",
		     (gpointer) (text_label));
  g_object_set_data (G_OBJECT (page), "page_type",
		     GINT_TO_POINTER (type));
  g_object_set_data (G_OBJECT (page), "option_menu",
		     (gpointer) option_menu);
  g_object_set_data (G_OBJECT (page), "search_entry",
		     (gpointer) search_entry);
  
  /* Signal to call the person on the double-clicked row */
  g_signal_connect (G_OBJECT (tree_view), "row_activated", 
		    G_CALLBACK (contact_activated_cb), NULL);

  /* Right-click on a contact */
  g_signal_connect (G_OBJECT (tree_view), "event-after",
		    G_CALLBACK (contact_event_after_cb), 
		    GINT_TO_POINTER (type));

  return page_num;
}


void
gnomemeeting_addressbook_group_populate (GtkListStore *list_store,
					 char *group_name)
{
  GtkTreeIter list_iter;
  
  GSList *groups_list = NULL;
  GSList *group_content = NULL;

  char **contact_info = NULL;

  GConfClient *client = NULL;

  client = gconf_client_get_default ();
  
  groups_list = 
    gconf_client_get_list (client, CONTACTS_KEY "groups_list",
			   GCONF_VALUE_STRING, NULL);

  gtk_list_store_clear (GTK_LIST_STORE (list_store));
  
  while (groups_list) {

    if (groups_list->data && group_name
	&& !strcmp ((char *) group_name, (char *) groups_list->data)) {

      gchar *gconf_key =
	g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY,
			 (char *) groups_list->data);

      group_content =
	gconf_client_get_list (client, gconf_key, GCONF_VALUE_STRING, NULL);

      while (group_content) {

	gtk_list_store_append (list_store, &list_iter);

	contact_info =
	  g_strsplit ((char *) group_content->data, "|", 0);

	if (contact_info [0])
	  gtk_list_store_set (list_store, &list_iter,
			      COLUMN_NAME, contact_info [0], -1);
	if (contact_info [1])
	  gtk_list_store_set (list_store, &list_iter,
			      COLUMN_CALLTO, contact_info [1], -1);

	g_strfreev (contact_info);
	group_content = g_slist_next (group_content);
      }

      g_free (gconf_key);
      g_slist_free (group_content);
    }

    groups_list = g_slist_next (groups_list);
  }

  g_slist_free (groups_list);
}
					      
