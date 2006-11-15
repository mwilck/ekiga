
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
 *                         gmroster.cpp  -  description 
 *                         ------------------------------------------
 *   begin                : Sun Mar 26 2006
 *   copyright            : (C) 2000-2006 by Jan Schampera, Damien Sandras
 *   description          : Implementation of a roster. 
 *
 */

/* CODING GUIDELINES:
 * - a contact is uniqly identified by its UID
 * - regarding presence operations (status change, ...) the master of
 *   desaster is the contact's URI (GmContact*)->url as an URI is online,
 *   not a contact!
 */

/* FIXME:
 *  needed internal functions
 *  - get the GtkTreeIters by a given contact UID
 *  - get the GtkTreeIters by a given contact URI
 */

/* BRIEF API DOCUMENTATION
 *
 *
 * SIGNALS:
 *
 * "current-uid-changed"
 *  Handler function:
 *  void foo (GMRoster *roster, gpointer user-data);
 *  It's usually useless to know that.
 *
 * "current-uri-changed"
 *  Handler function:
 *  void foo (GMRoster *roster, gpointer user-data);
 *  Could be used by the main UI
 *
 * "contact-clicked"
 *  Handler function:
 *  void foo (GMRoster *roster, gpointer user-data);
 *  Emitted when a contact was RIGHTCLICKED
 *
 * "contact-doubleclicked"
 *  Handler function:
 *  void foo (GMRoster *roster, gpointer user-data);
 *  Emitted when a contact was DOUBLECLICKED with LEFT button
 */

#include "gmroster.h"
#include "gmconf.h"

#include <stdlib.h>
#include <string.h>

#define GMROSTER_GMCONF_SUBKEY_EXPGRP "roster_expanded_groups"

struct _GMRosterURIStatus {
  gchar *uri;
  /*!< the URI, the status is from */

  ContactState status;
  /*!< the status itself */
};

typedef _GMRosterURIStatus GMRosterURIStatus;

struct _GMRosterPrivate {
  gboolean initial_sync;
  /*!< set by gmroster_init, re-set on the first sync: causes the syncer
   * to read from config */

  gchar *gmconf_key;
  /*!< the key to use to store persistent values */

  gchar *selected_uri;
  /*!< holds the currently selected URI */

  gchar *selected_uid;
  /*!< holds the currently selected contact UID */

  gchar *selected_group;
  /*!< holds the group, the current selection is in, or the group that
   * is selected */

  GSList *saved_expanded_groups;
  /*!< a GSList of (gchar*) to save the expanded groups over refresh */

  gchar *saved_selected_uid;
  /*!< buffer for #selected_uid during refreshs */

  gchar *saved_selected_uri;
  /*!< buffer for #selected_uri during refreshs */

  gchar *saved_selected_group;
  /*!< buffer for #selected_group during refreshs */

  GSList *saved_uri_presence;
  /*!< a GSList of (GMRosterURIStatus*) to save the presence status
   * over refresh */

  GtkTreeStore *tree_store;
  /*!< the tree store we work on */

  GMRosterURIStatus *last_uri_change;
  /*!< for data hand over */

  /* FIXME this is fairly redundant, do some beauty in future... */
};

enum {
  COLUMN_PIXBUF,
  COLUMN_PIXBUF_VISIBLE,
  COLUMN_NAME,
  COLUMN_UID,
  COLUMN_URI,
  COLUMN_GROUPNAME,
  COLUMN_STATUS,
  NUM_COLUMS_ENTRIES
};

enum {
  SIG_CONTACT_CLICKED,
  SIG_CONTACT_DCLICKED,
  SIG_UID_SEL_CHG,
  SIG_URI_SEL_CHG,
  SIG_C_STATUS_CHANGE,
  SIG_LAST
};

static guint gmroster_signals[SIG_LAST] = { 0 };


/* internal signal handlers - prototypes */
static gint gmroster_sighandler_button_event (GtkWidget *unused,
					      GdkEventButton *event,
					      gpointer data);

static void gmroster_sighandler_selection_changed (GtkTreeSelection *,
						   gpointer);



static void gmroster_class_init (GMRosterClass *klass);

static void gmroster_init (GMRoster *roster);

static void gmroster_destroy (GtkObject *roster);

gboolean gmroster_has_contact (GMRoster *,
                               GmContact *);

void gmroster_add_contact (GMRoster *,
                           GmContact *);

void gmroster_del_contact (GMRoster *,
                           GmContact *);

gboolean gmroster_has_group (GMRoster *,
                             gchar *);

void gmroster_add_group (GMRoster *,
                         gchar *);


GSList *gmroster_grouplist_filter_roster_group (GMRoster *,
						GSList *);

void gmroster_add_contact_to_group (GMRoster *,
                                    GmContact *,
                                    gchar *);

gboolean gmroster_get_iter_from_contact (GMRoster *,
                                         GmContact *,
                                         GtkTreeIter *);

gboolean gmroster_get_iter_from_group (GMRoster *,
                                       gchar *,
                                       GtkTreeIter *);

gchar *gmroster_get_group_from_iter (GMRoster *,
                                     GtkTreeIter *);

void gmroster_save_expanded_groups (GtkTreeView *, GtkTreePath *, gpointer);

void gmroster_view_delete (GMRoster* roster);

void gmroster_view_refresh_save_all (GMRoster* roster);

void gmroster_view_refresh_restore_all (GMRoster* roster);

gboolean gmroster_update_status_icons (GtkTreeModel *, GtkTreePath *, GtkTreeIter *, gpointer);

gboolean gmroster_update_presence_status (GtkTreeModel *, GtkTreePath *, GtkTreeIter *, gpointer);

void gmroster_update_status_texts (GMRoster *, ContactState);

void gmroster_presence_list_append (GMRoster *, gchar *, ContactState);

static gint gmroster_tree_sort_compare_func (GtkTreeModel *, GtkTreeIter *, GtkTreeIter *, gpointer);

/* Implementation */

/* internal signal handlers */
static gint
gmroster_sighandler_button_event (GtkWidget *self,
				  GdkEventButton *event,
				  gpointer data)
{
  GMRoster *roster = NULL;

  g_return_val_if_fail (data != NULL, 1);

  roster = GMROSTER (data);

  /* responsible to emit:
   * - "contact-clicked"
   * - "contact-doubleclicked"
   * - "contact-left-clicked" FIXME
   * - "group-clicked" FIXME
   */


  g_return_val_if_fail (data != NULL, 0);

  /* right-click */
  if (event->type == GDK_BUTTON_PRESS && event->button == 3)
    {
      if (roster->privdata->selected_uid) {
	g_signal_emit_by_name (roster,
			       "contact-clicked",
			       NULL);
      }
    }
  /* double-click */
  if (event->type == GDK_2BUTTON_PRESS && event->button == 1)
    {
      if (roster->privdata->selected_uid) {
	g_signal_emit_by_name (roster,
			       "contact-doubleclicked",
			       NULL);
      }
    }

  return 1;
}


static void
gmroster_sighandler_selection_changed (GtkTreeSelection * selection,
				       gpointer data)
{
  GMRoster *roster = NULL;
  GtkTreeIter current_iter;
  GtkTreeModel *model = NULL;

  gchar *selected_uid = NULL;
  gchar *selected_uri = NULL;
  gchar *selected_group = NULL;

  /* internal event markers */
  gboolean uid_changed = FALSE;
  gboolean uri_changed = FALSE;

  roster = GMROSTER (data);

  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  //g_message ("gmroster_sighandler_selection_changed: triggered");

  if (gtk_tree_selection_get_selected (selection, &model, &current_iter))
    {
      /* get the current UID */
      gtk_tree_model_get (model,
			  &current_iter,
			  COLUMN_UID,
			  &selected_uid,
			  COLUMN_URI,
			  &selected_uri,
			  COLUMN_GROUPNAME,
			  &selected_group, -1);

      /* compare to already stored UID
       * strcmp() doesn't like NULLs */
      if (roster->privdata->selected_uid &&
	  selected_uid &&
	  strcmp ((const char*) roster->privdata->selected_uid,
		  (const char*) selected_uid))
	{
	  /* both are not NULL and different */
	  g_free (roster->privdata->selected_uid);

	  roster->privdata->selected_uid = g_strdup (selected_uid);

	  uid_changed = TRUE;
	}
      else if ((roster->privdata->selected_uid &&
		!selected_uid) ||
	       (!roster->privdata->selected_uid &&
		selected_uid))
	{
	  /* one of them is NULL */
	  if (roster->privdata->selected_uid)
	    g_free (roster->privdata->selected_uid);
	  roster->privdata->selected_uid = NULL;

	  if (selected_uid)
	    roster->privdata->selected_uid = g_strdup (selected_uid);

	  uid_changed = TRUE;
	}

      /* FIXME:
       * cases:
       * - UID == NULL: a group was selected, do the same for current group
       */
    }

  if (uid_changed && roster->privdata->selected_uid)
    {
      /* if a new UID was selected, but the URI doesn't differ, there's
       * no need to emit a signal */
      if (roster->privdata->selected_uri &&
	  selected_uri &&
	  strcmp ((const char*) roster->privdata->selected_uri,
		  (const char*) selected_uri))
	{
	  /* both !NULL and different */
	  g_free (roster->privdata->selected_uri);

	  roster->privdata->selected_uri = g_strdup (selected_uri);

	  uri_changed = TRUE;
	}
      else if ((roster->privdata->selected_uri &&
		!selected_uri) ||
	       (!roster->privdata->selected_uri &&
		selected_uri))
	{
	  /* one of them is NULL */
	  if (roster->privdata->selected_uri)
	    g_free (roster->privdata->selected_uri);
	  roster->privdata->selected_uri = NULL;

	  if (selected_uri)
	    roster->privdata->selected_uri = g_strdup (selected_uri);

	  uri_changed = TRUE;
	}
    }

  /* save the current selected group/group the selection is in */
  if (roster->privdata->selected_group)
    g_free (roster->privdata->selected_group);
  roster->privdata->selected_group = g_strdup (selected_group);

  if (uid_changed &&
      !roster->privdata->selected_uid &&
      roster->privdata->selected_uri)
    {
      /* no URI without a UID ... */
      g_free (roster->privdata->selected_uri);
      roster->privdata->selected_uri = NULL;
      uri_changed = TRUE;
    }

  if (uid_changed)
    g_signal_emit_by_name (roster,
			   "current-uid-changed",
			   NULL);

  if (uri_changed)
    g_signal_emit_by_name (roster,
			   "current-uri-changed",
			   NULL);

  if (selected_uid)
    g_free (selected_uid);
  if (selected_uri)
    g_free (selected_uri);
  if (selected_group)
    g_free (selected_group);

}



GType
gmroster_get_type (void) {
  
  static GType gmroster_type = 0;

  if (!gmroster_type) {
    
    static const GTypeInfo gmroster_info =
      {
        sizeof (GMRosterClass),
        NULL,
        NULL,
        (GClassInitFunc) gmroster_class_init,
        NULL,
        NULL,
        sizeof (GMRoster),
        0,
        (GInstanceInitFunc) gmroster_init,
      };
    
    gmroster_type = g_type_register_static (GTK_TYPE_TREE_VIEW,
                                            "GMRoster",
                                            &gmroster_info, (GTypeFlags) 0);
  }
  
  return gmroster_type;
}


static void
gmroster_class_init (GMRosterClass* klass) 
{
  GtkObjectClass *gtkobject_class = NULL;

  static gboolean initialized = FALSE;

  if (initialized)
    return;

  initialized = TRUE;

  gtkobject_class = GTK_OBJECT_CLASS (klass);

  gtkobject_class->destroy = gmroster_destroy;

  /* SIGNAL "contact-clicked"
   * - emitted when a contact is RIGHTCLICKED
   * - the handler can obtain the current info by the API
   */
  gmroster_signals[SIG_CONTACT_CLICKED] =
    g_signal_new ("contact-clicked",
		  G_OBJECT_CLASS_TYPE (klass),
		  G_SIGNAL_RUN_FIRST,
		  0, NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE,
		  0, NULL);

  /* SIGNAL "contact-doublecklicked"
   * - emitted when a contact is DOUBLECKLICKED with the LEFT button
   * - the handler can obtain the current info by the API
   */
  gmroster_signals[SIG_CONTACT_DCLICKED] =
    g_signal_new ("contact-doubleclicked",
		  G_OBJECT_CLASS_TYPE (klass),
		  G_SIGNAL_RUN_FIRST,
		  0, NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE,
		  0, NULL);

  /* SIGNAL "current-uri-changed"
   * - emitted when a contact is selected and the UID is different from
   *   the last selected contact (may be the same)
   * - the handler can obtain the current UID/Contact by
   *   - gmroster_get_selected_uid() (copy of UID string)
   */
  gmroster_signals[SIG_UID_SEL_CHG] =
    g_signal_new ("current-uid-changed",
		  G_OBJECT_CLASS_TYPE (klass),
		  G_SIGNAL_RUN_FIRST,
		  0, NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE,
		  0, NULL);

  /* SIGNAL "current-uri-changed"
   * - emmitted when a contact is selected and the URI is different from
   *   the last selected contact (may be the same)
   * - the handler can obtain the current URI/contact by
   *   - gmroster_get_selected_uri() (copy of URI string)
   */
  gmroster_signals[SIG_URI_SEL_CHG] =
    g_signal_new ("current-uri-changed",
		  G_OBJECT_CLASS_TYPE (klass),
		  G_SIGNAL_RUN_FIRST,
		  0, NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE,
		  0, NULL);
}


static void
gmroster_init (GMRoster* roster) 
{
  GtkCellRenderer *cell = NULL;
  GtkTreeSelection *treeselection = NULL;
  GtkTreeViewColumn *column = NULL;
  
  gint index = 0;

  roster->privdata = g_new (GMRosterPrivate, 1);
  roster->privdata->initial_sync = TRUE;
  roster->privdata->gmconf_key = NULL;
  roster->privdata->selected_uid = NULL;
  roster->privdata->selected_uri = NULL;
  roster->privdata->selected_group = NULL;
  roster->privdata->saved_selected_uid = NULL;
  roster->privdata->saved_selected_uri = NULL;
  roster->privdata->saved_selected_group = NULL;
  roster->privdata->saved_expanded_groups = NULL;
  roster->privdata->saved_uri_presence = NULL;
  roster->privdata->tree_store = NULL;
  roster->privdata->last_uri_change = g_new (GMRosterURIStatus, 1);

  for (index = 0 ; index < CONTACT_LAST_STATE ; index++) {
    
    roster->icons [index] = NULL;
    roster->statustexts [index] = NULL;
  }

  roster->unknown_group_name = g_strdup ("unknown");

  roster->show_offlines = TRUE;

  roster->show_groupless_contacts = TRUE;
  
  roster->show_in_multiple_groups = FALSE;

  roster->roster_group = g_strdup ("Roster");

  roster->privdata->tree_store =
    gtk_tree_store_new (NUM_COLUMS_ENTRIES,
			GDK_TYPE_PIXBUF,
			G_TYPE_BOOLEAN,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_INT);



  gtk_tree_sortable_set_sort_column_id
    (GTK_TREE_SORTABLE (roster->privdata->tree_store),
     COLUMN_NAME, GTK_SORT_ASCENDING);

  gtk_tree_sortable_set_sort_func
    (GTK_TREE_SORTABLE (roster->privdata->tree_store),
     COLUMN_NAME, (GtkTreeIterCompareFunc) gmroster_tree_sort_compare_func,
     NULL, NULL);

  gtk_tree_view_set_model (&roster->treeview,
			   GTK_TREE_MODEL (roster->privdata->tree_store));

  gtk_tree_view_set_headers_visible (&roster->treeview, FALSE);

  column = gtk_tree_view_column_new ();
  cell = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_set_spacing (column, 5);
  gtk_tree_view_column_set_attributes (column, cell,
				       "pixbuf", COLUMN_PIXBUF,
				       "visible", COLUMN_PIXBUF_VISIBLE,
				       NULL);

  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_set_attributes (column, cell,
				       "markup", COLUMN_NAME,
				       NULL);

  gtk_tree_view_append_column (&roster->treeview,
			       GTK_TREE_VIEW_COLUMN (column));

  g_signal_connect (G_OBJECT (&roster->treeview), "event_after",
		    G_CALLBACK (gmroster_sighandler_button_event),
		    (gpointer) roster);

  treeselection = gtk_tree_view_get_selection (&roster->treeview);

  g_signal_connect (G_OBJECT (treeselection), "changed",
		    G_CALLBACK (gmroster_sighandler_selection_changed),
		    (gpointer) roster);
}


static void
gmroster_destroy (GtkObject *object)
{
  GMRoster *roster =NULL;
  gchar *gmconf_key = NULL;

  int index = 0;
  
  roster = GMROSTER (object);
  g_return_if_fail (roster != NULL);

  if (roster->privdata->gmconf_key) {
    gmroster_view_refresh_save_all (roster);
    gmconf_key = g_strdup_printf ("%s/%s",
                                  roster->privdata->gmconf_key,
                                  GMROSTER_GMCONF_SUBKEY_EXPGRP);

    gm_conf_set_string_list (gmconf_key, roster->privdata->saved_expanded_groups);
    g_free (gmconf_key);
    g_free (roster->privdata->gmconf_key);
    roster->privdata->gmconf_key = NULL;
  }

  for (index = 0 ; index < CONTACT_LAST_STATE ; index++) {
    
    if (roster->icons [index])
      g_object_unref (roster->icons [index]);
    roster->icons [index] = NULL;
    
    if (roster->statustexts [index])
      g_free (roster->statustexts [index]);
    roster->statustexts [index] = NULL;
  }

  g_free (roster->roster_group);
  roster->roster_group = NULL;

  /* FIXME free everything! */
}


GtkWidget*
gmroster_new (void) 
{
  return GTK_WIDGET (g_object_new (gmroster_get_type (), NULL));
}


void gmroster_set_gmconf_key (GMRoster *roster,
                              const gchar *new_gmconf_key)
{
  gchar *gmconf_key = NULL;
  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  if (roster->privdata->gmconf_key)
    g_free (roster->privdata->gmconf_key);

  roster->privdata->gmconf_key =
    g_strdup (new_gmconf_key);

  gmroster_view_refresh_save_all (roster);
  /* delete the list and re-read it from the config key */
  g_slist_foreach (roster->privdata->saved_expanded_groups,
		   (GFunc) g_free,
		   NULL);
  g_slist_free (roster->privdata->saved_expanded_groups);

  gmconf_key = g_strdup_printf ("%s/%s",
				roster->privdata->gmconf_key,
				GMROSTER_GMCONF_SUBKEY_EXPGRP);

  roster->privdata->saved_expanded_groups =
    gm_conf_get_string_list (gmconf_key);
  g_free (gmconf_key);
  gmroster_view_refresh_restore_all (roster);
}


gchar *gmroster_get_gmconf_key (GMRoster *roster)
{
  g_return_val_if_fail (roster != NULL, NULL);
  g_return_val_if_fail (IS_GMROSTER (roster), NULL);

  return g_strdup (roster->privdata->gmconf_key);
}


void 
gmroster_add_entry (GMRoster *roster,
                    GmContact *contact)
{
  GmContact* newcontact = NULL;
  
  GSList* grouplist = NULL;
  GSList* grouplist_iter = NULL;

  g_return_if_fail (roster != NULL);
  g_return_if_fail (contact != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  if (gmroster_has_contact (roster, contact))
    return;

  /* check if the roster group is matched */
  if (roster->roster_group &&
      !gmcontact_is_in_category (contact,
				 roster->roster_group))
      return;

  newcontact = gmcontact_copy (contact);

  /* get the category enumeration and remove references to the roster group */
  grouplist = gmcontact_enum_categories (newcontact);
  grouplist = gmroster_grouplist_filter_roster_group (roster, grouplist);

  /* an error or an empty group list - groupless contact */
  if (!grouplist) {
    
    if (roster->show_groupless_contacts && roster->unknown_group_name) {
      
	gmroster_add_group (roster, roster->unknown_group_name);
	gmroster_add_contact_to_group (roster,
				       (GmContact*) newcontact,
				       roster->unknown_group_name);
    }
    gmcontact_delete (newcontact);
    return;
  }


  grouplist_iter = grouplist;
  while (grouplist_iter != NULL) {

    gmroster_add_group (roster, (gchar *) grouplist_iter->data);
    gmroster_add_contact_to_group (roster, newcontact, 
                                   (gchar *) grouplist_iter->data);

    if (roster->show_in_multiple_groups) 
      grouplist_iter = g_slist_next (grouplist_iter);
    else
      break;
  }


  g_slist_foreach (grouplist, (GFunc) g_free, NULL);
  g_slist_free (grouplist);
  gmcontact_delete (newcontact);
}


gboolean gmroster_has_contact (GMRoster *roster,
                               GmContact *contact)
{
  GtkTreeIter pseudo_iter;

  g_return_val_if_fail (roster != NULL, TRUE);
  g_return_val_if_fail (contact != NULL, TRUE);
  g_return_val_if_fail (IS_GMROSTER (roster), TRUE);

  /* we use gmroster_get_iter_from_contact here, as
   * it does exactly the same */

  return gmroster_get_iter_from_contact (roster,
					 contact,
					 &pseudo_iter);
}


gboolean gmroster_has_group (GMRoster *roster,
			     gchar *group)
{
  /* FIXME should be expressed as wrapper around _get_iter_from_group */
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gchar *comparegroup = NULL;

  g_return_val_if_fail (roster != NULL, TRUE);
  g_return_val_if_fail (group != NULL, TRUE);
  g_return_val_if_fail (IS_GMROSTER (roster), TRUE);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (roster));

  g_return_val_if_fail (model != NULL, TRUE);

  if (gtk_tree_model_get_iter_first (model, &iter)) {
    
    do {
      
      comparegroup = gmroster_get_group_from_iter (roster, &iter);
      if (g_ascii_strcasecmp (comparegroup, group) == 0) {
       
        g_free (comparegroup);
        return TRUE;
      }

      g_free (comparegroup);

    } while (gtk_tree_model_iter_next (model, &iter));
  }

  return FALSE;
}


void 
gmroster_add_group (GMRoster *roster,
                    gchar *group)
{
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gchar *group_name = NULL;

  g_return_if_fail (roster != NULL);
  g_return_if_fail (group != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  if (gmroster_has_group (roster, group)) 
    return;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (roster));

  g_return_if_fail (model != NULL);
  
  group_name = g_strdup_printf ("<b>%s</b>", group);
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter,
		      COLUMN_NAME, group_name,
		      COLUMN_GROUPNAME, group,
		      COLUMN_UID, NULL,
		      COLUMN_STATUS, CONTACT_LAST_STATE,
		      -1);
  g_free (group_name);
}


GSList
*gmroster_grouplist_filter_roster_group (GMRoster * roster,
					 GSList * grouplist)
{
  GSList *grouplist_iter = NULL;

  g_return_val_if_fail (roster != NULL, NULL);
  g_return_val_if_fail (IS_GMROSTER (roster), NULL);
  g_return_val_if_fail (grouplist != NULL, NULL);

  for (grouplist_iter = grouplist;
       grouplist_iter != NULL;
       grouplist_iter = g_slist_next (grouplist_iter))
    {
      if (!strcmp ((const gchar*) grouplist_iter->data,
	  (const gchar*) roster->roster_group))
	grouplist =
	  g_slist_delete_link (grouplist, grouplist_iter);
    }

  return grouplist;
}




void gmroster_add_contact_to_group (GMRoster* roster,
				    GmContact* contact,
				    gchar* group)
{
  GtkTreeModel* model = NULL;
  GtkTreeIter parent_iter, child_iter;
  GdkPixbuf* pixbuf = NULL;
  gchar* userinfotext = NULL;

  g_return_if_fail (roster != NULL);
  g_return_if_fail (contact != NULL);
  g_return_if_fail (group != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (roster));

  g_return_if_fail (model != NULL);

  if (!gmroster_has_group (GMROSTER (roster), group)) return;

  if (!roster->show_offlines && contact->state == CONTACT_OFFLINE) return;

  userinfotext =
    g_strdup_printf ("%s\n<span foreground=\"gray\" size=\"small\">%s</span>", 
                     contact->fullname, 
                     contact->url);

  pixbuf =
    roster->icons [contact->state];

  if (gmroster_get_iter_from_group (GMROSTER (roster), group, &parent_iter)) {

    gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &parent_iter);
    gtk_tree_store_set (GTK_TREE_STORE (model),
                        &child_iter,
                        COLUMN_PIXBUF, pixbuf,
                        COLUMN_PIXBUF_VISIBLE, TRUE,
                        COLUMN_NAME, userinfotext,
                        COLUMN_UID, contact->uid,
			COLUMN_URI, contact->url,
                        COLUMN_GROUPNAME, group,
			COLUMN_STATUS, contact->state,
                        -1);
  }

  g_free (userinfotext);
}


gboolean gmroster_get_iter_from_contact (GMRoster* roster,
					 GmContact* contact,
					 GtkTreeIter* iter)
{
  GtkTreeModel* model = NULL;
  GtkTreeIter group_iter;
  gint num_contacts, i = 0;
  gchar* compareuid = NULL;

  g_return_val_if_fail (roster != NULL, FALSE);
  g_return_val_if_fail (contact != NULL, FALSE);
  g_return_val_if_fail (IS_GMROSTER (roster), FALSE);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (roster));

  g_return_val_if_fail (model != NULL, FALSE);

  if (gtk_tree_model_get_iter_first (model, &group_iter)) {
    do {

      if (gtk_tree_model_iter_has_child (model, &group_iter))
	{
	  num_contacts = gtk_tree_model_iter_n_children (model, &group_iter);
	  for (i = 0; i <= num_contacts; i++)
	    {
	      if (gtk_tree_model_iter_nth_child (model,
						 iter,
						 &group_iter,
						 i))
		{
		  gtk_tree_model_get (model, iter,
				      COLUMN_UID, &compareuid,
				      -1);
		  if (g_ascii_strcasecmp (compareuid, contact->uid) == 0) {

                    g_free (compareuid);
		    return TRUE;
                  }
                  g_free (compareuid);
		}
	    }
	}

    } while (gtk_tree_model_iter_next (model, &group_iter));
  }
  return FALSE;
}


gboolean gmroster_get_iter_from_group (GMRoster* roster,
				       gchar* group,
				       GtkTreeIter* iter)
{
  GtkTreeModel* model = NULL;
  gchar* comparegroup = NULL;
  
  g_return_val_if_fail (roster != NULL, FALSE);
  g_return_val_if_fail (group != NULL, FALSE);
  g_return_val_if_fail (IS_GMROSTER (roster), FALSE);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (roster));

  g_return_val_if_fail (model != NULL, FALSE);

  if (!gmroster_has_group (GMROSTER (roster), group)) return FALSE;

  if (gtk_tree_model_get_iter_first (model, iter)) {
    do {
      comparegroup = gmroster_get_group_from_iter (roster, iter);

      if (g_ascii_strcasecmp (comparegroup, group) == 0)
	{
	  g_free (comparegroup);
	  return TRUE;
	}
      g_free (comparegroup);
    } while (gtk_tree_model_iter_next (model, iter));
  }
  return FALSE;
}


gchar *
gmroster_get_group_from_iter (GMRoster *roster,
                              GtkTreeIter *iter)
{
  GtkTreeModel *model = NULL;

  gchar *comparegroup = NULL;
  
  g_return_val_if_fail (IS_GMROSTER (roster), NULL);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (roster));

  g_return_val_if_fail (model != NULL, NULL);

  gtk_tree_model_get (model, iter,
                      COLUMN_GROUPNAME, &comparegroup,
                      -1);
  
  return comparegroup;
}


void
gmroster_save_expanded_groups (GtkTreeView *treeview,
			       GtkTreePath *path,
			       gpointer user_data)
{
  GtkTreeIter expanded_iter;
  GtkTreeModel *model = NULL;
  gchar *groupname = NULL;
  GMRoster *roster = NULL;

  g_return_if_fail (treeview != NULL);
  g_return_if_fail (path != NULL);
  g_return_if_fail (user_data != NULL);
  g_return_if_fail (GTK_IS_TREE_VIEW (treeview));

  model = gtk_tree_view_get_model (treeview);
  g_return_if_fail (model != NULL);

  roster = GMROSTER (user_data);

  /* Assumption: only groups can be "expanded" */
  /* get the iter from the given path */
  if (gtk_tree_model_get_iter (model, &expanded_iter, path))
    {
      /* get the groupname */
      gtk_tree_model_get (model, &expanded_iter,
			  COLUMN_GROUPNAME, &groupname, -1);

      roster->privdata->saved_expanded_groups =
	g_slist_append (roster->privdata->saved_expanded_groups,
			g_strdup (groupname));
    }
}


void
gmroster_presence_list_append (GMRoster * roster,
			       gchar * uri,
			       ContactState status)
{
  GSList *presence_list_iter = NULL;
  GMRosterURIStatus *uri_status = NULL;
  gboolean found = FALSE;

  g_return_if_fail (uri != NULL);

  for (presence_list_iter = roster->privdata->saved_uri_presence;
       presence_list_iter != NULL;
       presence_list_iter = g_slist_next (presence_list_iter))
    {
      if (presence_list_iter->data)
	{
	  uri_status = (GMRosterURIStatus*) presence_list_iter->data;

	  if (uri_status->uri &&
	      !strcmp ((const char*) uri_status->uri,
		       (const char*) uri))
	    found = TRUE;

	  uri_status = NULL;
	}
    }

  if (!found)
    {
      uri_status = g_new (GMRosterURIStatus, 1);
      uri_status->uri = g_strdup (uri);
      uri_status->status = status;
      roster->privdata->saved_uri_presence =
	g_slist_append (roster->privdata->saved_uri_presence,
			(gpointer) uri_status);
      uri_status = NULL;
    }
}


static gint
gmroster_tree_sort_compare_func (GtkTreeModel *model,
				 GtkTreeIter *a_iter,
				 GtkTreeIter *b_iter,
				 gpointer data)
{
  gchar *a_name = NULL;
  gchar *b_name = NULL;
  gint to_return = 1;

  /* sort at the end on errors */
  g_return_val_if_fail (model != NULL, 1);
  g_return_val_if_fail (a_iter != NULL, 1);
  g_return_val_if_fail (b_iter != NULL, 1);

  /* get the names to compare out of the model */
  gtk_tree_model_get (model, a_iter, COLUMN_NAME, &a_name, -1);
  gtk_tree_model_get (model, b_iter, COLUMN_NAME, &b_name, -1);

  to_return = g_utf8_collate (a_name, b_name);

  g_free (a_name);
  g_free (b_name);

  return to_return;
}


void
gmroster_view_refresh_save_all (GMRoster* roster)
{
  GtkTreeModel* model = NULL;
  GtkTreeIter iter_parent, iter_child;

  gchar *uri = NULL;
  ContactState status = CONTACT_LAST_STATE;

  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (roster));

  /* 1. SAFE ALL GROUPNAMES THAT ARE EXPANDED IN THE VIEW */
  gtk_tree_view_map_expanded_rows (GTK_TREE_VIEW (roster),
				   (GtkTreeViewMappingFunc) gmroster_save_expanded_groups,
				   roster);

  /* 2. THE CURRENT SELECTION CAN BE IDENTIFIED BY privdata->selected_uid AND
   *                                               privdata->selected_group
   *    NO EXTRA CODE NEEDED HERE, just copy to the buffer */
  roster->privdata->saved_selected_uid = g_strdup (roster->privdata->selected_uid);
  roster->privdata->saved_selected_uri = g_strdup (roster->privdata->selected_uri);
  roster->privdata->saved_selected_group = g_strdup (roster->privdata->selected_group);

  /* 3. SAVE THE PRESENCE STATUS OF EVERY URI */
  if (gtk_tree_model_get_iter_first   (model, &iter_parent))
    {
      do {
	if (gtk_tree_model_iter_has_child (model, &iter_parent))
	  {
	    gtk_tree_model_iter_children (model, &iter_child, &iter_parent);
	    do {
	      gtk_tree_model_get (model, &iter_child,
				  COLUMN_URI, &uri,
				  COLUMN_STATUS, &status,
				  -1);
	      gmroster_presence_list_append (roster,
					     uri, status);
	    } while (gtk_tree_model_iter_next (model, &iter_child));
	  }
      } while (gtk_tree_model_iter_next (model, &iter_parent));
    }
}

void
gmroster_view_refresh_restore_all (GMRoster* roster)
{
  GSList *saved_expanded_groups_iter = NULL;
  GtkTreeIter group_tree_iter;
  GtkTreePath *group_tree_path = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;

  GtkTreeIter selection_parent_iter;
  GtkTreeIter selection_child_iter;
  gchar *selection_uid = NULL;
  gchar *selection_group = NULL;
  gboolean selection_found = FALSE;

  GSList *presence_iter = NULL;

  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (roster));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (roster));

  /* 1. RESTORE "EXPANDED" GROUPS */
  if (roster->privdata->saved_expanded_groups)
    {
      for (saved_expanded_groups_iter = roster->privdata->saved_expanded_groups;
	   saved_expanded_groups_iter != NULL;
	   saved_expanded_groups_iter = g_slist_next (saved_expanded_groups_iter))
	{
	  if (saved_expanded_groups_iter->data)
	    {
	      if (gmroster_get_iter_from_group (roster,
						(gchar *) saved_expanded_groups_iter->data,
						&group_tree_iter))
		{
		  group_tree_path =
		    gtk_tree_model_get_path (model, &group_tree_iter);
		  
		  (void) gtk_tree_view_expand_row (GTK_TREE_VIEW (roster),
						   group_tree_path,
						   TRUE);

		  gtk_tree_path_free (group_tree_path);
		}
	      g_free (saved_expanded_groups_iter->data);
	    }
	}
      g_slist_free (roster->privdata->saved_expanded_groups);
      roster->privdata->saved_expanded_groups = NULL;
    }

  /* 2. RESTORE THE CURRENT SELECTION */
  /* FIXME
   * search the iter where COLUMN_UID == privdata->saved_selected_uid &&
   *                       COLUMN_GROUPNAME == privdata->saved_selected_group
   * and select it */

  if (roster->privdata->saved_selected_group &&
      roster->privdata->saved_selected_uid)
    {
      /* when both saved strings are found (UID and group),
       * iter through the tree, find the matching row and select it
       */
      if (roster->privdata->saved_selected_uid &&
	  gtk_tree_model_get_iter_first (model, &selection_parent_iter))
	{
	  do {
	    if (gtk_tree_model_iter_has_child (model, &selection_parent_iter))
	      {
		(void) gtk_tree_model_iter_children (model,
						     &selection_child_iter,
						     &selection_parent_iter);
		do {
		  gtk_tree_model_get (model,
				      &selection_child_iter,
				      COLUMN_UID, &selection_uid,
				      COLUMN_GROUPNAME, &selection_group,
				      -1);

		  if (selection_uid &&
		      selection_group &&
		      !strcmp ((const char*) selection_uid,
			       (const char*) 
			       roster->privdata->saved_selected_uid) &&
		      !strcmp ((const char*) selection_group,
			       (const char*)
			       roster->privdata->saved_selected_group))
		    {
		      selection_found = TRUE;
		      gtk_tree_selection_select_iter (selection, &selection_child_iter);
		      g_free (selection_uid);
		      g_free (selection_group);
		      break;
		    }
		  g_free (selection_uid);
		  g_free (selection_group);
		  if (selection_found) break;
		} while (gtk_tree_model_iter_next (model, &selection_child_iter));
	      }
	  } while (gtk_tree_model_iter_next (model, &selection_parent_iter));
	  if (!selection_found)
	    {
	      /* if the search failed, select the first iter */
	      if (gtk_tree_model_get_iter_first (model, &selection_parent_iter))
		gtk_tree_selection_select_iter (selection, &selection_parent_iter);
	    }
	}
    }
  else if (!roster->privdata->saved_selected_uid &&
	   roster->privdata->saved_selected_group)
    {
      /* no saved UID set, but a saved group, select the group */
      /* check if we have the group, if not: select the first iter available */
      if (!gmroster_has_group (roster, roster->privdata->saved_selected_group))
	{
	  if (gtk_tree_model_get_iter_first (model, &selection_parent_iter))
	    gtk_tree_selection_select_iter (selection, &selection_parent_iter);
	}
      else
	{
	  if (gmroster_get_iter_from_group (roster,
					    roster->privdata->saved_selected_group,
					    &selection_parent_iter))
	    gtk_tree_selection_select_iter (selection, &selection_parent_iter);
	}
    }
  else
    {
      /* if saved UID set, but no saved group, or if nothing saved at all:
       * select the first iter available */
      if (gtk_tree_model_get_iter_first (model, &selection_parent_iter))
	gtk_tree_selection_select_iter (selection, &selection_parent_iter);
    }
  if (roster->privdata->saved_selected_uid)
    g_free (roster->privdata->saved_selected_uid);
  if (roster->privdata->saved_selected_uri)
    g_free (roster->privdata->saved_selected_uri);
  if (roster->privdata->saved_selected_group)
    g_free (roster->privdata->saved_selected_group);

  /* 3. RESTORE PRESENCE STATUS */
  for (presence_iter = roster->privdata->saved_uri_presence;
       presence_iter != NULL;
       presence_iter = g_slist_next (presence_iter))
    {
      if (presence_iter->data)
	{
	  /* FIXME this isn't a nice way of data handover */
	  roster->privdata->last_uri_change =
	    (GMRosterURIStatus*) presence_iter->data;

	  gtk_tree_model_foreach (model,
				  (GtkTreeModelForeachFunc) gmroster_update_presence_status,
				  (gpointer) roster);

	  g_free (roster->privdata->last_uri_change->uri);
	  g_free (roster->privdata->last_uri_change);
	  roster->privdata->last_uri_change = NULL;
	}
    }
  g_slist_free (roster->privdata->saved_uri_presence);
  roster->privdata->saved_uri_presence = NULL;
}


void
gmroster_view_delete (GMRoster* roster)
{
  GtkTreeModel* model = NULL;

  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (roster));
  
  gtk_tree_store_clear (GTK_TREE_STORE (model));

  /* FIXME:
   * - save the presence status of all contacts
   */
}


void
gmroster_set_status_icon (GMRoster* roster,
			  ContactState status,
			  const gchar *stock)
{
  GtkTreeModel *model = NULL;

  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  g_return_if_fail ((status > 0 || status < CONTACT_LAST_STATE));

  if (roster->icons [status])
    g_object_unref (roster->icons [status]);

  if (stock)
    roster->icons [status] = 
      gtk_widget_render_icon (GTK_WIDGET (roster), stock, 
                              GTK_ICON_SIZE_MENU, NULL);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (roster));

  gtk_tree_model_foreach (model,
			  (GtkTreeModelForeachFunc) gmroster_update_status_icons,
			  (gpointer) roster);
}


void
gmroster_set_status_text (GMRoster* roster, 
			  ContactState status, 
			  gchar* text)
{
  GtkTreeModel *model = NULL;

  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  g_return_if_fail ((status > 0 || status < CONTACT_LAST_STATE));

  if (roster->statustexts [status])
    g_free (roster->statustexts [status]);

  if (text)
    roster->statustexts [status] = g_strdup (text);
  else
    roster->statustexts [status] = NULL;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (roster));

  gmroster_update_status_texts (roster, status);
}


gboolean
gmroster_update_status_icons (GtkTreeModel *model,
			      GtkTreePath *path,
			      GtkTreeIter *iter,
			      gpointer data)
{
  GMRoster *roster = NULL;
  ContactState status = CONTACT_LAST_STATE;

  g_return_val_if_fail (data != NULL, FALSE);
  roster = GMROSTER (data);

  gtk_tree_model_get (model,
		      iter,
		      COLUMN_STATUS, &status,
		      -1);

  if (status < CONTACT_LAST_STATE)
    {
      gtk_tree_store_set (roster->privdata->tree_store,
			  iter,
			  COLUMN_PIXBUF, roster->icons [status],
			  -1);
    }

  return FALSE;
}


void
gmroster_update_status_texts (GMRoster* roster,
			      ContactState status)
{
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (roster));

  if (gtk_tree_model_get_iter_first (model, &iter)) {
    do {
      /* FIXME iter through all stuff an re-set the status texts */
      /* see gtk_tree_store_set */
    } while (gtk_tree_model_iter_next (model, &iter));
  }
}


void
gmroster_set_show_offlines (GMRoster* roster,
			    gboolean show_status)
{
  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  /* FIXME no effect if not done before adding the first contact */

  roster->show_offlines = show_status;
}


gboolean
gmroster_get_show_offlines (GMRoster* roster)
{
  g_return_val_if_fail (roster != NULL, FALSE);
  g_return_val_if_fail (IS_GMROSTER (roster), FALSE);

  return roster->show_offlines;
}


void
gmroster_set_show_in_multiple_groups (GMRoster* roster,
				      gboolean show_multiple)
{
  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  /* FIXME no effect if not done before adding the first contact */

  if (roster->show_in_multiple_groups != show_multiple) {
    roster->show_in_multiple_groups = show_multiple;
  }
}


gboolean
gmroster_get_show_in_multiple_groups (GMRoster* roster)
{
  g_return_val_if_fail (roster != NULL, FALSE);
  g_return_val_if_fail (IS_GMROSTER (roster), FALSE);

  return roster->show_in_multiple_groups;
}


void
gmroster_set_show_groupless_contacts (GMRoster* roster,
				      gboolean show_groupless)
{
  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  /* FIXME no effect if not done before adding the first contact */

  if (show_groupless == roster->show_groupless_contacts)
    return;

  roster->show_groupless_contacts = show_groupless;
}


gboolean
gmroster_get_show_groupless_contacts (GMRoster* roster)
{
  g_return_val_if_fail (roster != NULL, FALSE);
  g_return_val_if_fail (IS_GMROSTER (roster), FALSE);

  return roster->show_groupless_contacts;
}


void
gmroster_sync_with_contacts (GMRoster *roster,
			     GSList *contacts)
{
  gchar *gmconf_key = NULL;
  GSList *contacts_iter = NULL;
  GmContact *contact = NULL;

  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  gmroster_view_refresh_save_all (roster);
  gmroster_view_delete (roster);

  contacts_iter = contacts;

  while (contacts_iter) {
    contact = GM_CONTACT (contacts_iter->data);
    if (contact->url && strcmp (contact->url, ""))
      gmroster_add_entry (GMROSTER (roster), contact);
    contacts_iter = g_slist_next (contacts_iter);
  }

  if (roster->privdata->initial_sync &&
      !roster->privdata->gmconf_key)
    roster->privdata->initial_sync = FALSE;

  if (roster->privdata->initial_sync &&
      roster->privdata->gmconf_key) {
    g_slist_foreach (roster->privdata->saved_expanded_groups,
		     (GFunc) g_free, NULL);
    g_slist_free (roster->privdata->saved_expanded_groups);

    gmconf_key = g_strdup_printf ("%s/%s",
                                  roster->privdata->gmconf_key,
                                  GMROSTER_GMCONF_SUBKEY_EXPGRP);
    roster->privdata->saved_expanded_groups =
      gm_conf_get_string_list (gmconf_key);
    g_free (gmconf_key);

    roster->privdata->initial_sync = FALSE;
  }

  gmroster_view_refresh_restore_all (roster);
}


gboolean
gmroster_update_presence_status (GtkTreeModel *model,
				 GtkTreePath *path,
				 GtkTreeIter *iter,
				 gpointer data)
{
  GMRoster *roster = NULL;
  gchar *current_uri = NULL;

  g_return_val_if_fail (data != NULL, FALSE);
  roster = GMROSTER (data);

  gtk_tree_model_get (model,
                      iter,
                      COLUMN_URI, &current_uri,
                      -1);

  if (current_uri &&
      roster->privdata->last_uri_change &&
      roster->privdata->last_uri_change->uri &&
      !strcmp ((const char*) current_uri,
	       (const char*) roster->privdata->last_uri_change->uri)) {
    
    gtk_tree_store_set (roster->privdata->tree_store,
                        iter,
                        COLUMN_STATUS,
                        roster->privdata->last_uri_change->status,
                        COLUMN_PIXBUF,
                        roster->icons [roster->privdata->last_uri_change->status],
                        -1);
    g_free (current_uri);
    return FALSE;
  }
  
  g_free (current_uri);
  return FALSE;
}


void gmroster_presence_set_status (GMRoster * roster,
                                   gchar * uri,
                                   ContactState status)
{
  GMRosterURIStatus uri_status;
  GtkTreeModel *model = NULL;

  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));
  g_return_if_fail (uri != NULL);

  uri_status.uri = g_strdup (uri);
  uri_status.status = status;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (roster));
  
  /* FIXME this isn't a nice way of data handover */
  roster->privdata->last_uri_change = &uri_status;

  gtk_tree_model_foreach (model,
			  (GtkTreeModelForeachFunc) gmroster_update_presence_status,
			  (gpointer) roster);

  roster->privdata->last_uri_change = NULL;

  g_free (uri_status.uri);
  uri_status.uri = NULL;
}


void
gmroster_set_roster_group (GMRoster * roster,
			   const gchar * rostergroup)
{
  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  if (roster->roster_group)
    {
      g_free (roster->roster_group);
      roster->roster_group = NULL;
    }

  roster->roster_group =
    g_strdup (rostergroup);
}


const
gchar *gmroster_get_roster_group (GMRoster * roster)
{
  g_return_val_if_fail (roster != NULL, NULL);
  g_return_val_if_fail (IS_GMROSTER (roster), NULL);

  return (const gchar*) roster->roster_group;
}


void
gmroster_set_unknown_group_name (GMRoster *roster,
				 const gchar *unknown_group)
{
  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  if (roster->unknown_group_name &&
      !strcmp (unknown_group, roster->unknown_group_name))
    return;

  if (roster->unknown_group_name)
    g_free (roster->unknown_group_name);

  roster->unknown_group_name =
    g_strdup (unknown_group);
}


const
gchar *gmroster_get_unknown_group_name (GMRoster *roster)
{
  g_return_val_if_fail (roster != NULL, NULL);
  g_return_val_if_fail (IS_GMROSTER (roster), NULL);

  return (const gchar*) roster->unknown_group_name;
}

gchar
*gmroster_get_selected_uid (GMRoster * roster) {
  g_return_val_if_fail (roster != NULL, NULL);
  g_return_val_if_fail (IS_GMROSTER (roster), NULL);

  return g_strdup (roster->privdata->selected_uid);
}

gchar
*gmroster_get_selected_uri (GMRoster * roster)
{
  g_return_val_if_fail (roster != NULL, NULL);
  g_return_val_if_fail (IS_GMROSTER (roster), NULL);

  return g_strdup (roster->privdata->selected_uri);
}

