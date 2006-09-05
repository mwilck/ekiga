
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


#include "gmroster.h"

#include <stdlib.h>
#include <string.h>


enum {
  COLUMN_PIXBUF,
  COLUMN_PIXBUF_VISIBLE,
  COLUMN_NAME,
  COLUMN_UID,
  COLUMN_GROUPNAME,
  NUM_COLUMS_ENTRIES
};

static gint entry_clicked_cb (GtkWidget *unused,
				GdkEventButton *event,
				gpointer data);

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

void gmroster_del_group (GMRoster *,
                         gchar *);

gboolean gmroster_need_group (GMRoster *,
                              gchar *);

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

void gmroster_view_delete (GMRoster* roster);

void gmroster_view_rebuild (GMRoster* roster);


/* Implementation */
static gint 
entry_clicked_cb (GtkWidget *self,
                  GdkEventButton *event,
                  gpointer data)
{
  g_signal_emit_by_name (self, "entry-clicked", g_strdup ("test"));
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

  (void) g_signal_new ("entry-clicked",
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
  GtkTreeStore* model = NULL;
  GtkCellRenderer *cell = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeViewColumn *column = NULL;
  
  gint index = 0;

  roster->contacts = NULL;
  
  for (index = 0 ; index < CONTACT_LAST_STATE ; index++) {
    
    roster->icons [index] = NULL;
    roster->statustexts [index] = NULL;
  }

  roster->unknown_group_name = g_strdup ("unknown");

  roster->show_offlines = TRUE;

  roster->show_groupless_contacts = TRUE;
  
  roster->show_in_multiple_groups = FALSE;

  model = gtk_tree_store_new (NUM_COLUMS_ENTRIES,
			      GDK_TYPE_PIXBUF,
			      G_TYPE_INT,
			      G_TYPE_STRING,
			      G_TYPE_INT,
			      G_TYPE_STRING,
                              G_TYPE_STRING);

  gtk_tree_view_set_model (&roster->treeview,
			   GTK_TREE_MODEL (model));

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
		    G_CALLBACK (entry_clicked_cb), NULL);
}


static void
gmroster_destroy (GtkObject *object)
{
  GMRoster *roster =NULL;
  GSList *iter = NULL;

  int index = 0;
  
  roster = GMROSTER (object);
  g_return_if_fail (roster != NULL);

  iter = roster->contacts;

  g_slist_foreach (iter, (GFunc) gmcontact_delete, NULL);
  g_slist_free (iter);
  roster->contacts = NULL;
  
  for (index = 0 ; index < CONTACT_LAST_STATE ; index++) {
    
    if (roster->icons [index])
      g_object_unref (roster->icons [index]);
    roster->icons [index] = NULL;
    
    if (roster->statustexts [index])
      g_free (roster->statustexts [index]);
    roster->statustexts [index] = NULL;
  }
}


GtkWidget*
gmroster_new (void) 
{
  return GTK_WIDGET (g_object_new (gmroster_get_type (), NULL));
}


void 
gmroster_add_entry (GMRoster *roster,
                    GmContact *contact)
{
  GmContact* newcontact = NULL;
  
  GtkTreeModel* model = NULL;
  GtkTreeIter groups_iter;
  GtkTreeIter iter;
  
  gchar* tmpgroup = NULL;
  
  GSList* grouplist = NULL;
  GSList* grouplist_iter = NULL;

  g_return_if_fail (roster != NULL);
  g_return_if_fail (contact != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  if (gmroster_has_contact (GMROSTER(roster), contact))
    return;

  newcontact = gmcontact_copy (contact);

  /* add the contact entry to the internal contacts list */
  roster->contacts = g_slist_append (roster->contacts, newcontact);

  /* get the category enumeration */
  grouplist = gmcontact_enum_categories (newcontact);

  /* an error or an empty group list - groupless contact */
  if (!grouplist) {
    
    if (roster->show_groupless_contacts && roster->unknown_group_name) {
      
	gmroster_add_group (roster, roster->unknown_group_name);
	gmroster_add_contact_to_group (roster,
				       (GmContact*) newcontact,
				       roster->unknown_group_name);
    }
    return;
  }


  grouplist_iter = grouplist ;
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
}


void gmroster_del_entry (GMRoster *roster,
			 GmContact *contact)
{
  GtkTreeModel* model = NULL;
  GtkTreeIter iter;
  
  GSList* contacts_iter = NULL;
  GSList* grouplist = NULL;
  GSList* grouplist_iter = NULL;
  
  gint index = 0;
  
  GmContact* comparecontact = NULL;
  
  g_return_if_fail (roster != NULL);
  g_return_if_fail (contact != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  if (!gmroster_has_contact (GMROSTER(roster), contact))
    return;
  
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (roster));

  g_return_if_fail (model != NULL);

  if (gmroster_get_iter_from_contact (roster, contact, &iter)) {

    gtk_tree_store_remove (GTK_TREE_STORE (model), &iter);

    contacts_iter = roster->contacts; 
    while (contacts_iter) {

      comparecontact = GM_CONTACT (contacts_iter->data);

      if (g_ascii_strcasecmp (comparecontact->uid, contact->uid) == 0) {
        
        roster->contacts = g_slist_remove_link (roster->contacts,
                                                contacts_iter);
        gmcontact_delete (comparecontact);
      }

      contacts_iter = g_slist_next (contacts_iter);
    }
  }
}


void gmroster_modify_entry (GMRoster* roster,
			    GmContact* contact)
{
  g_return_if_fail (roster != NULL);
  g_return_if_fail (contact != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  if (!gmroster_has_contact (GMROSTER(roster), contact))
    return;

  /* FIXME */
}


gboolean gmroster_has_contact (GMRoster *roster,
			       GmContact *contact)
{
  GSList *contacts_iter = NULL;

  GmContact *comparecontact = NULL;

  g_return_val_if_fail (roster != NULL, TRUE);
  g_return_val_if_fail (contact != NULL, TRUE);
  g_return_val_if_fail (IS_GMROSTER (roster), TRUE);
  
  contacts_iter = roster->contacts; 
  while (contacts_iter) {

    comparecontact = GM_CONTACT (contacts_iter->data);
    if (g_ascii_strcasecmp (comparecontact->uid, contact->uid) == 0) 
      return TRUE;

    contacts_iter = g_slist_next (contacts_iter);
  }

  return FALSE;
}


gboolean gmroster_has_group (GMRoster *roster,
			     gchar *group)
{
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
		      -1);
  g_free (group_name);

  gtk_tree_view_expand_all (GTK_TREE_VIEW (roster));
}


void gmroster_del_group (GMRoster *roster,
			 gchar *group)
{
  GtkTreeModel* model = NULL;
  GtkTreeIter iter;
  gchar* comparegroup = NULL;
  
  g_return_if_fail (roster != NULL);
  g_return_if_fail (group != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  if (!gmroster_has_group (GMROSTER(roster), group)) 
    return;
  if (gmroster_need_group (GMROSTER(roster), group)) 
    return;

  /* search the iterator for that group */
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (roster));
  
  g_return_if_fail (model != NULL);

  if (gtk_tree_model_get_iter_first (model, &iter)) {
    do {

      gmroster_get_group_from_iter (roster, &iter);
      if (g_ascii_strcasecmp (comparegroup, group) == 0) {

        gtk_tree_store_remove (GTK_TREE_STORE (model), &iter);
        break;
      }
    } while (gtk_tree_model_iter_next (model, &iter));
  }
}


gboolean gmroster_need_group (GMRoster* roster,
			      gchar* group)
{
  GSList* contacts_iter = NULL;
  GmContact* comparecontact = NULL;

  g_return_val_if_fail (roster != NULL, TRUE);
  g_return_val_if_fail (group != NULL, TRUE);
  g_return_val_if_fail (IS_GMROSTER (roster), TRUE);

  for (contacts_iter = roster->contacts;
       contacts_iter != NULL;
       contacts_iter = g_slist_next (contacts_iter))
    {
      comparecontact = (GmContact*) contacts_iter->data;
      if (gmcontact_is_in_category (comparecontact, group)) return TRUE;
      comparecontact = NULL;
    }

  return FALSE;
}


void gmroster_add_contact_to_group (GMRoster* roster,
				    GmContact* contact,
				    gchar* group)
{
  GtkTreeModel* model = NULL;
  GtkTreeIter parent_iter, child_iter;
  GSList* grouplist = NULL;
  GSList* grouplist_iter = NULL;
  GdkPixbuf* pixbuf = NULL;
  gchar* userinfotext = NULL;

  g_return_if_fail (roster != NULL);
  g_return_if_fail (contact != NULL);
  g_return_if_fail (group != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (roster));

  g_return_if_fail (model != NULL);

  if (!gmroster_has_group (GMROSTER (roster), group))
    {
      g_message ("gmroster_has_group(): no such group: %s", group);
      return;
    }

  if (!roster->show_offlines && contact->state == CONTACT_OFFLINE) return;

  userinfotext =
    g_strdup_printf ("%s\n<span foreground=\"gray\" size=\"small\">%s</span>", contact->fullname, contact->url);

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
                        COLUMN_GROUPNAME, group,
                        -1);
  }
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

  if (!gmroster_has_contact (GMROSTER (roster), contact)) return FALSE;

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
		  if (g_ascii_strcasecmp (compareuid, contact->uid) == 0)
		    return TRUE;
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

      if (g_ascii_strcasecmp (comparegroup, group) == 0) return TRUE;
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
gmroster_view_delete (GMRoster* roster)
{
  GtkTreeModel* model = NULL;

  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (roster));
  
  gtk_tree_store_clear (GTK_TREE_STORE (model));
}


void
gmroster_view_rebuild (GMRoster* roster)
{
  GtkTreeModel* model = NULL;
  GtkTreeStore* treestore = NULL;
  GtkCellRenderer *cell = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeViewColumn *column = NULL;
  GSList* contacts_iter = NULL;
  GSList* grouplist = NULL;
  GSList* grouplist_iter = NULL;
  GmContact* tmpcontact = NULL;
  gchar* tmpgroup = NULL;

  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (roster));

  for (contacts_iter = roster->contacts;
       contacts_iter != NULL;
       contacts_iter = g_slist_next (contacts_iter))
    {
      /* get the category enumeration */
      grouplist =
	gmcontact_enum_categories ((GmContact*) contacts_iter->data);


      /* an error or an empty group list - groupless contact */
      if (!grouplist) {
	if (roster->show_groupless_contacts && roster->unknown_group_name)
	  {
	    gmroster_add_group (roster, roster->unknown_group_name);

	    gmroster_add_contact_to_group (roster,
					   (GmContact*) contacts_iter->data,
					   roster->unknown_group_name);
	  }
	return;
      }

      if (roster->show_in_multiple_groups) {
	/* show the contact in all its groups */
	for (grouplist_iter = grouplist;
	     grouplist_iter != NULL;
	     grouplist_iter = g_slist_next (grouplist_iter)) {
	  
	  tmpgroup =
	    g_strdup ((const gchar*) grouplist_iter->data);

	  gmroster_add_group (roster, tmpgroup);

	  gmroster_add_contact_to_group (roster,
					 (GmContact*) contacts_iter->data,
					 tmpgroup);

	  g_free (tmpgroup);

	}
      } else {
	/* show the contact only in his first group */
	tmpgroup =
	  g_strdup ((const gchar*) grouplist_iter->data);

	gmroster_add_group (roster, tmpgroup);

	gmroster_add_contact_to_group (roster,
				       (GmContact*) contacts_iter->data,
				       tmpgroup);

	g_free (tmpgroup);
      }

      g_slist_free (grouplist);
    }
  gtk_tree_view_expand_all (GTK_TREE_VIEW (roster));
}


void
gmroster_set_status_icon (GMRoster* roster,
			  ContactState status,
			  const gchar *stock)
{
  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  g_return_if_fail ((status > 0 || status < CONTACT_LAST_STATE));

  gmroster_view_delete (GMROSTER (roster));
  
  if (roster->icons [status])
    g_object_unref (roster->icons [status]);

  if (stock)
    roster->icons [status] = 
      gtk_widget_render_icon (GTK_WIDGET (roster), stock, 
                              GTK_ICON_SIZE_MENU, NULL); 

  gmroster_view_rebuild (GMROSTER (roster));
}


void
gmroster_set_status_text (GMRoster* roster, 
			  ContactState status, 
			  gchar* text)
{
  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  g_return_if_fail ((status > 0 || status < CONTACT_LAST_STATE));

  gmroster_view_delete (GMROSTER (roster));

  if (roster->statustexts [status])
    g_free (roster->statustexts [status]);

  if (text)
    roster->statustexts [status] = g_strdup (text);
  else
    roster->statustexts [status] = NULL;

  gmroster_view_rebuild (GMROSTER (roster));
}


void
gmroster_set_show_offlines (GMRoster* roster,
			    gboolean show_status)
{
  g_return_if_fail (roster != NULL);
  g_return_if_fail (IS_GMROSTER (roster));

  gmroster_view_delete (GMROSTER (roster));

  roster->show_offlines = show_status;

  gmroster_view_rebuild (GMROSTER (roster));
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

  if (roster->show_in_multiple_groups != show_multiple) {
    gmroster_view_delete (GMROSTER (roster));
    
    roster->show_in_multiple_groups = show_multiple;

    gmroster_view_rebuild (GMROSTER (roster));
  }
}


gboolean
gmroster_get_show_in_multiple_groups (GMRoster* roster)
{
  g_return_val_if_fail (roster != NULL, FALSE);
  g_return_val_if_fail (IS_GMROSTER (roster), FALSE);

  return roster->show_in_multiple_groups;
}

