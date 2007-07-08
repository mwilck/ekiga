
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
 *                         book-view-gtk.cpp -  description
 *                         ------------------------------------------
 *   begin                : written in 2006 by Julien Puydt
 *   copyright            : (c) 2006 by Julien Puydt
 *   description          : implementation of an addressbook view
 *
 */

#include <algorithm>
#include <iostream>

#include "book-view-gtk.h"
#include "menu-builder-gtk.h"

/* Ok, here is the problem : we have gtk+ code which listens to libsigc++
 * signals, so we need a way to un-listen those signals when the gtk+ object
 * dies. For this, we use the fact that a sigc::trackable object is cleanly
 * and automatically destroyed with respect to signals.
 * So we will make a sigc::trackable object which will take care of registering
 * to signals, and forward them to us. We store it in our private structure,
 * so it will die with the gtk+ object.
 * Problem solved!
 */

class SignalCentralizer: public sigc::trackable
{
public:

  SignalCentralizer () { /* nothing */ }

  /* signals emitted by this centralizer */

  sigc::signal<void, Ekiga::Contact &> contact_added;
  sigc::signal<void, Ekiga::Contact &> contact_updated;
  sigc::signal<void, Ekiga::Contact &> contact_removed;

  /* starting the show */

  void watch_book (Ekiga::Book &book)
  {
    book.contact_added.connect (contact_added.make_slot ());
    book.contact_updated.connect (contact_updated.make_slot ());
    book.contact_removed.connect (contact_removed.make_slot ());
    repopulate (book);
  }

  void repopulate (Ekiga::Book &book)
  {
    book.visit_contacts (sigc::mem_fun (this, &SignalCentralizer::add_contact));
  }

private:

  void add_contact (Ekiga::Contact &contact)
  {
    contact_added.emit (contact);
  }
};

struct _BookViewGtkPrivate
{

  _BookViewGtkPrivate (Ekiga::Book &_book) : book (_book) { }

  SignalCentralizer centralizer;
  GtkTreeView *tree_view;
  GtkWidget *vbox;
  GtkWidget *scrolled_window;
  Ekiga::Book &book;
  gboolean groups_enabled;
  std::string filter;
  std::list<std::string> hidden_groups;
};

enum
{

  BOOK_VIEW_GTK_PROP_FILTERED = 1,
  BOOK_VIEW_GTK_PROP_FILTER,
  BOOK_VIEW_GTK_PROP_HIDDEN_GROUPS
};

enum
{

  COLUMN_CONTACT_POINTER,
  COLUMN_GROUP_NAME,
  COLUMN_NAME,
  COLUMN_NUMBER
};

static GObjectClass *parent_class = NULL;

/* declarations */

static void on_contact_updated (Ekiga::Contact &contact,
				gpointer data);

static void on_contact_added (Ekiga::Contact &contact,
			      gpointer data);

static void on_contact_removed (Ekiga::Contact &contact,
				gpointer data);

static void book_view_gtk_enable_groups (BookViewGtk *view,
					 GtkTreeViewColumn *column,
					 const gchar *background_color);

static gboolean book_view_gtk_is_group_iter (BookViewGtk *view,
					     GtkTreeIter *iter);

static void book_view_gtk_show_renderer_on_group_iterators (BookViewGtk *view,
							    GtkTreeViewColumn *column,
							    GtkCellRenderer *renderer);

static void book_view_gtk_hide_renderer_on_group_iterators (BookViewGtk *view,
							    GtkTreeViewColumn *column,
							    GtkCellRenderer *renderer);

/* implementation */

/* group cell management */

static void
show_group_cell_data_func (GtkTreeViewColumn */*tree_column*/,
			   GtkCellRenderer *cell,
			   GtkTreeModel */*tree_model*/,
			   GtkTreeIter *iter,
			   gpointer data)
{
  if (book_view_gtk_is_group_iter ((BookViewGtk *)data, iter))
    g_object_set (cell, "visible", TRUE, NULL);
  else
    g_object_set (cell, "visible", FALSE, NULL);
}

static void
hide_group_cell_data_func (GtkTreeViewColumn */*tree_column*/,
			   GtkCellRenderer *cell,
			   GtkTreeModel */*tree_model*/,
			   GtkTreeIter *iter,
			   gpointer data)
{
  if (book_view_gtk_is_group_iter ((BookViewGtk *)data, iter))
    g_object_set (cell, "visible", FALSE, NULL);
  else
    g_object_set (cell, "visible", TRUE, NULL);
}

static void
book_view_gtk_enable_groups (BookViewGtk *view,
			     GtkTreeViewColumn *column,
			     const gchar *background_color)
{
  GtkCellRenderer *renderer = NULL;

  view->priv->groups_enabled = TRUE;

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, renderer, TRUE);
  gtk_tree_view_column_add_attribute (column, renderer,
				      "text", COLUMN_GROUP_NAME);
  g_object_set (renderer, "weight", PANGO_WEIGHT_BOLD, NULL);
  if (background_color != NULL)
    g_object_set (renderer, "cell-background", background_color, NULL);
  book_view_gtk_show_renderer_on_group_iterators (view, column, renderer);
}

static gboolean
book_view_gtk_is_group_iter (BookViewGtk *view,
			     GtkTreeIter *iter)
{
  Ekiga::Contact *contact = NULL;
  GtkTreeModel *model = NULL;

  model = gtk_tree_view_get_model (view->priv->tree_view);

  gtk_tree_model_get (model, iter, COLUMN_CONTACT_POINTER, &contact, -1);
  if (contact)
    return FALSE;
  else
    return TRUE;
}

static void
book_view_gtk_show_renderer_on_group_iterators (BookViewGtk *view,
						GtkTreeViewColumn *column,
						GtkCellRenderer *renderer)
{
  gtk_tree_view_column_set_cell_data_func (column, renderer,
					   show_group_cell_data_func, view,
					   NULL);
}

static void
book_view_gtk_hide_renderer_on_group_iterators (BookViewGtk *view,
						GtkTreeViewColumn *column,
						GtkCellRenderer *renderer)
{
  gtk_tree_view_column_set_cell_data_func (column, renderer,
					   hide_group_cell_data_func, view,
					   NULL);
}

static void
book_view_gtk_update_contact (BookViewGtk *self,
			      Ekiga::Contact &contact,
			      GtkTreeIter *iter)
{
  GtkTreeStore *store = NULL;
  const gchar *name = NULL;

  name = contact.get_name ().c_str ();
  store = GTK_TREE_STORE (gtk_tree_view_get_model (self->priv->tree_view));
  gtk_tree_store_set (store, iter, COLUMN_NAME, name, -1);
}

/* internal helpers */

static void
on_filter_entry_changed (GtkWidget *entry,
			 gpointer data)
{
  g_object_set (data, "filter", gtk_entry_get_text (GTK_ENTRY (entry)), NULL);
}

static gboolean
contact_satisfies_filter (BookViewGtk *view,
			  Ekiga::Contact &contact)
{
  if (view->priv->filter.empty ())
    return TRUE;
  else
    return contact.is_found (view->priv->filter);
}

static void
clean_empty_groups (BookViewGtk *view)
{
  GtkTreeModel *model = NULL;
  GtkTreeStore *store = NULL;
  GtkTreeIter iter;
  gboolean go_on = TRUE;

  model = gtk_tree_view_get_model (view->priv->tree_view);
  store = GTK_TREE_STORE (model);

  if (gtk_tree_model_get_iter_first (model, &iter))
    while (go_on)
      if (gtk_tree_model_iter_has_child (model, &iter))
	go_on = gtk_tree_model_iter_next (model, &iter);
      else
	go_on = gtk_tree_store_remove (store, &iter);
}

static void
find_iter_for_group (BookViewGtk *view,
		     std::string name,
		     GtkTreeIter *iter)
{
  GtkTreeModel *model = NULL;
  GtkTreeStore *store = NULL;
  gchar *group_name = NULL;
  gboolean found = FALSE;

  model = gtk_tree_view_get_model (view->priv->tree_view);
  store = GTK_TREE_STORE (model);

  if (gtk_tree_model_get_iter_first (model, iter))
    do {

      gtk_tree_model_get (model, iter, COLUMN_GROUP_NAME, &group_name, -1);
      if (group_name != NULL && name == group_name)
	found = TRUE;
      if (group_name != NULL)
	g_free (group_name);
    } while (!found && gtk_tree_model_iter_next (model, iter));

  if (!found) {

    gtk_tree_store_append (store, iter, NULL);
    gtk_tree_store_set (store, iter, COLUMN_GROUP_NAME, name.c_str (), -1);
  }
}

static gboolean
find_iter_for_contact (BookViewGtk *view,
		       Ekiga::Contact &contact,
		       GtkTreeIter *group_iter,
		       GtkTreeIter *iter)
{
  GtkTreeModel *model = NULL;
  GtkTreeStore *store = NULL;
  Ekiga::Contact *iter_contact = NULL;
  gboolean found = FALSE;

  model = gtk_tree_view_get_model (view->priv->tree_view);
  store = GTK_TREE_STORE (model);

  if (gtk_tree_model_iter_children (model, iter, group_iter)) {

      do {

	gtk_tree_model_get (model, iter,
			    COLUMN_CONTACT_POINTER, &iter_contact,
			    -1);
	if (iter_contact == &contact)
	  found = TRUE;

      } while (!found && gtk_tree_model_iter_next (model, iter));

      return found;
    } else
      return FALSE;
}

static void
remove_contact (BookViewGtk *self,
		Ekiga::Contact &contact)
{
  GtkTreeModel *model = NULL;
  GtkTreeStore *store = NULL;
  GtkTreeIter group_iter;
  GtkTreeIter iter;

  model = gtk_tree_view_get_model (self->priv->tree_view);
  store = GTK_TREE_STORE (model);

  if (self->priv->groups_enabled) {

    if (gtk_tree_model_get_iter_first (model, &group_iter))
      do {

	while (find_iter_for_contact (self, contact, &group_iter, &iter))
	  gtk_tree_store_remove (store, &iter);
      } while (gtk_tree_model_iter_next (model, &group_iter));

      clean_empty_groups (self);
  } else
    while (find_iter_for_contact (self, contact, NULL, &iter))
      gtk_tree_store_remove (store, &iter);
}

static void
add_contact (BookViewGtk *self,
	     Ekiga::Contact &contact)
{
  GtkTreeModel *model = NULL;
  GtkTreeStore *store = NULL;
  std::list<std::string> groups;
  GtkTreeIter group_iter;
  GtkTreeIter iter;
  GtkTreePath *path = NULL;

  model = gtk_tree_view_get_model (self->priv->tree_view);
  store = GTK_TREE_STORE (model);

  if (!contact_satisfies_filter (self, contact))
    return;

  if (self->priv->groups_enabled) {

      groups = contact.get_groups ();

      if (groups.empty ())
	groups.push_front ("Unsorted");

      for (std::list<std::string>::iterator group_name_iter = groups.begin ();
	   group_name_iter != groups.end ();
	   group_name_iter++) {

	if (std::find (self->priv->hidden_groups.begin (),
		       self->priv->hidden_groups.end (),
		       *group_name_iter)
	    == self->priv->hidden_groups.end ()) {

	  find_iter_for_group (self, *group_name_iter, &group_iter);
	  if (!find_iter_for_contact (self, contact, &group_iter, &iter)) {

	    gtk_tree_store_append (store, &iter, &group_iter);
	    gtk_tree_store_set (store, &iter,
				COLUMN_CONTACT_POINTER, &contact,
				-1);
	  }
	  path = gtk_tree_model_get_path (model, &group_iter);
	  gtk_tree_view_expand_row (self->priv->tree_view, path, TRUE);
	  gtk_tree_path_free (path);
	  book_view_gtk_update_contact (self, contact, &iter);
	}
      }

  } else {

    gtk_tree_store_append (store, &iter, NULL);
    gtk_tree_store_set (store, &iter, COLUMN_CONTACT_POINTER, &contact, -1);
    book_view_gtk_update_contact (self, contact, &iter);
  }
}

static void
clean_and_populate (BookViewGtk *view)
{
  GtkTreeModel *model = NULL;
  GtkTreeStore *store = NULL;

  /* clean */
  model = gtk_tree_view_get_model (view->priv->tree_view);
  store = GTK_TREE_STORE (model);
  gtk_tree_store_clear (store);

  /* populate */
  view->priv->centralizer.repopulate (view->priv->book);
}

/* contact observer */

static void
on_contact_updated (Ekiga::Contact &contact,
		    gpointer data)
{
  std::list<std::string> groups;
  BookViewGtk *view = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeStore *store = NULL;
  GtkTreeIter group_iter;
  gchar *group_name = NULL;
  GtkTreeIter iter;
  gboolean had_to_remove = FALSE;

  groups = contact.get_groups ();
  view = (BookViewGtk *)data;
  model = gtk_tree_view_get_model (view->priv->tree_view);
  store = GTK_TREE_STORE (model);
  if (groups.empty ())
    groups.push_front ("Unsorted");

  /* this makes sure the contact is in the groups it should be in */
  add_contact (view, contact);

  if (contact_satisfies_filter (view, contact)) {

    /* now let's make sure it doesn't appear in groups it isn't in anymore */
    if (view->priv->groups_enabled) {

     had_to_remove = FALSE;
      if (gtk_tree_model_get_iter_first (model, &group_iter)) {

	do {

	  gtk_tree_model_get (model, &group_iter,
			      COLUMN_GROUP_NAME, &group_name,
			      -1);

	  if (group_name != NULL
	      && std::find (groups.begin (), groups.end (),
			    group_name) == groups.end ())
	    if (find_iter_for_contact (view, contact, &group_iter, &iter)) {

	      gtk_tree_store_remove (store, &iter);
	      had_to_remove = TRUE;
	    }

	  g_free (group_name);
	} while (gtk_tree_model_iter_next (model, &group_iter));
      }

      if (had_to_remove)
	clean_empty_groups (view);
    }
  } else /* the updated contact doesn't satisfy the filter */
    remove_contact (view, contact);
}

/* selection observer */

static void
on_selection_changed (GtkTreeSelection */*selection*/,
		      gpointer data)
{
  g_signal_emit_by_name (data, "updated", NULL);
}


static gint
on_contact_clicked (GtkWidget *tree_view,
		    GdkEventButton *event,
		    gpointer data)
{
  GtkTreePath *path = NULL;
  GtkTreeIter iter;
  GtkTreeModel *model = NULL;
  Ekiga::Contact *contact = NULL;

  if (event->type == GDK_BUTTON_PRESS || event->type == GDK_KEY_PRESS) {

    if (event->button == 3) {

      if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (tree_view),
					 (gint) event->x, (gint) event->y,
					 &path, NULL, NULL, NULL)) {

	model
	  = gtk_tree_view_get_model (((BookViewGtk *) data)->priv->tree_view);

	if (gtk_tree_model_get_iter (model, &iter, path)) {

	  gtk_tree_model_get (model, &iter,
			      COLUMN_CONTACT_POINTER, &contact,
			      -1);

	  if (contact) {

	    MenuBuilderGtk builder;
	    contact->populate_menu (builder);
	    if (!builder.empty ()) {

	      gtk_widget_show_all (builder.menu);
	      gtk_menu_popup (GTK_MENU (builder.menu), NULL, NULL,
			      NULL, NULL, event->button, event->time);
	      g_signal_connect (G_OBJECT (builder.menu), "hide",
				GTK_SIGNAL_FUNC (g_object_unref),
				(gpointer) builder.menu);
	    }
	    g_object_ref_sink (G_OBJECT (builder.menu));
	  }
	}
	gtk_tree_path_free (path);
      }
    }
  }
  return TRUE;
}

/* book observer */

static void
on_contact_added (Ekiga::Contact &contact,
		  gpointer data)
{
  add_contact ((BookViewGtk *)data, contact);
}


static void
on_contact_removed (Ekiga::Contact &contact,
		    gpointer data)
{
  BookViewGtk *view = NULL;

  view = (BookViewGtk *)data;

  remove_contact (view, contact);
}

/* public methods implementation */

GtkWidget *
book_view_gtk_new (Ekiga::Book &book)
{
  BookViewGtk *result = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeStore *store = NULL;
  GtkTreeViewColumn *col = NULL;
  GtkCellRenderer *renderer = NULL;

  result = (BookViewGtk *)g_object_new (BOOK_VIEW_GTK_TYPE, NULL);

  result->priv = new _BookViewGtkPrivate (book);

  result->priv->vbox = gtk_vbox_new (FALSE, 2);
  result->priv->scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW
				  (result->priv->scrolled_window),
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  result->priv->groups_enabled = FALSE;
  result->priv->hidden_groups.push_front ("Roster");

  result->priv->tree_view = GTK_TREE_VIEW (gtk_tree_view_new ());
#if GTK_CHECK_VERSION(2,10,0)
  g_object_set (result->priv->tree_view, "show-expanders", FALSE, NULL);
#endif

  gtk_container_add (GTK_CONTAINER (result), GTK_WIDGET (result->priv->vbox));
  gtk_container_add (GTK_CONTAINER (result->priv->vbox),
		     GTK_WIDGET (result->priv->scrolled_window));
  gtk_container_add (GTK_CONTAINER (result->priv->scrolled_window),
		     GTK_WIDGET (result->priv->tree_view));

  selection = gtk_tree_view_get_selection (result->priv->tree_view);
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
  g_signal_connect (selection, "changed",
		    G_CALLBACK (on_selection_changed), result);

  g_signal_connect (G_OBJECT (result->priv->tree_view), "event-after",
		    G_CALLBACK (on_contact_clicked), result);

  store = gtk_tree_store_new (COLUMN_NUMBER,
			      G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING);

  gtk_tree_view_set_headers_visible (result->priv->tree_view, FALSE);
  gtk_tree_view_set_model (result->priv->tree_view, GTK_TREE_MODEL (store));

  col = gtk_tree_view_column_new ();

  book_view_gtk_enable_groups (result, col, "blue");

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer,
				      "text", COLUMN_NAME);
  book_view_gtk_hide_renderer_on_group_iterators (result, col, renderer);

  gtk_tree_view_append_column (result->priv->tree_view, col);

  result->priv->centralizer.contact_added.connect (sigc::bind (sigc::ptr_fun (on_contact_added), (gpointer)result));

  result->priv->centralizer.contact_updated.connect (sigc::bind (sigc::ptr_fun (on_contact_updated), (gpointer)result));

  result->priv->centralizer.contact_removed.connect (sigc::bind (sigc::ptr_fun (on_contact_removed), (gpointer)result));

  result->priv->centralizer.watch_book (book);

  return (GtkWidget *) result;
}

void
book_view_gtk_populate_menu (BookViewGtk *self,
			     GtkWidget *menu)
{
  g_return_if_fail (IS_BOOK_VIEW_GTK (self));
  g_return_if_fail (GTK_IS_MENU (menu));

  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  Ekiga::Contact *contact = NULL;
  GtkWidget *item = NULL;
  MenuBuilderGtk builder (menu);

  self->priv->book.populate_menu (builder);

  selection = gtk_tree_view_get_selection (self->priv->tree_view);

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (model, &iter, COLUMN_CONTACT_POINTER, &contact, -1);

    if (contact) {

      item = gtk_separator_menu_item_new ();
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
      item = gtk_menu_item_new_with_label ("Selected contact");
      gtk_widget_set_sensitive (GTK_WIDGET (item), FALSE);
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
      item = gtk_separator_menu_item_new ();
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
      contact->populate_menu (builder);
    }
  }
}

/* GObject boilerplate code */

static void
book_view_gtk_set_property (GObject *obj,
			    guint prop_id,
			    const GValue *value,
			    GParamSpec *spec)
{
  BookViewGtk *view = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *label = NULL;
  GtkWidget *entry = NULL;
  GSList *ptr = NULL;

  view = (BookViewGtk *)obj;

  switch (prop_id) {

  case BOOK_VIEW_GTK_PROP_FILTERED:
    if (g_value_get_boolean (value)) {

      hbox = gtk_hbox_new (FALSE, 2);
      label = gtk_label_new ("Quick live filter : ");
      entry = gtk_entry_new ();
      gtk_container_add (GTK_CONTAINER (view->priv->vbox), hbox);
      gtk_box_set_child_packing (GTK_BOX (view->priv->vbox), hbox,
				 FALSE, FALSE, 2, GTK_PACK_END);
      gtk_container_add (GTK_CONTAINER (hbox), label);
      gtk_container_add (GTK_CONTAINER (hbox), entry);

      g_signal_connect (G_OBJECT (entry), "changed",
			G_CALLBACK (on_filter_entry_changed),
			(gpointer) view);
    }
    break;

  case BOOK_VIEW_GTK_PROP_FILTER:
    view->priv->filter = g_value_get_string (value);
    clean_and_populate (view);
    break;

  case BOOK_VIEW_GTK_PROP_HIDDEN_GROUPS:
    view->priv->hidden_groups.clear ();
    for (ptr = (GSList *) g_value_get_pointer (value);
	 ptr != NULL;
	 ptr = g_slist_next (ptr))
      view->priv->hidden_groups.push_front ((const gchar *) ptr->data);
    clean_and_populate (view);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}

static void
book_view_gtk_dispose (GObject *obj)
{
  BookViewGtk *view = NULL;

#ifdef __GNUC__
  g_print ("%s\n", __PRETTY_FUNCTION__);
#endif

  view = (BookViewGtk *)obj;

  if (view->priv->tree_view) {

    g_signal_handlers_disconnect_matched (gtk_tree_view_get_selection (view->priv->tree_view),
					  (GSignalMatchType) G_SIGNAL_MATCH_DATA,
					  0, /* signal_id */
					  (GQuark) 0, /* detail */
					  NULL,	/* closure */
					  NULL,	/* func */
					  view); /* data */
    gtk_tree_store_clear (GTK_TREE_STORE (gtk_tree_view_get_model (view->priv->tree_view)));

    view->priv->tree_view = NULL;
  }

  parent_class->dispose (obj);
}


static void
book_view_gtk_finalize (GObject *obj)
{
  BookViewGtk *view = NULL;

#ifdef __GNUC__
  g_print ("%s\n", __PRETTY_FUNCTION__);
#endif

  view = (BookViewGtk *)obj;

  delete view->priv;

  parent_class->finalize (obj);
}

static void
book_view_gtk_class_init (gpointer g_class,
			  gpointer /*class_data*/)
{
  GObjectClass *gobject_class = NULL;
  GParamSpec *spec = NULL;

  parent_class = (GObjectClass *)g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass *)g_class;
  gobject_class->set_property = book_view_gtk_set_property;
  gobject_class->dispose = book_view_gtk_dispose;
  gobject_class->finalize = book_view_gtk_finalize;

  spec = g_param_spec_boolean ("filtered",
			       "is filtered",
			       "Decide whether to show an filter label+entry",
			       FALSE,
			       (GParamFlags) (G_PARAM_CONSTRUCT_ONLY
					      | G_PARAM_WRITABLE));
  g_object_class_install_property (gobject_class,
				   BOOK_VIEW_GTK_PROP_FILTERED, spec);

  spec = g_param_spec_string ("filter",
			      "filter applied to contacts",
			      "Set filter applied to contacts",
			      NULL, (GParamFlags) (G_PARAM_WRITABLE));
  g_object_class_install_property (gobject_class,
				   BOOK_VIEW_GTK_PROP_FILTER, spec);

  spec = g_param_spec_pointer ("hidden-groups",
			       "groups hidden in the view",
			       "Set the list of groups hidden in the view",
			       (GParamFlags) (G_PARAM_WRITABLE));
  g_object_class_install_property (gobject_class,
				   BOOK_VIEW_GTK_PROP_FILTER, spec);

  (void)g_signal_new ("updated",
		      G_OBJECT_CLASS_TYPE (g_class),
		      G_SIGNAL_RUN_FIRST,
		      0, NULL, NULL,
		      g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, NULL);
}

GType
book_view_gtk_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (BookViewGtkClass),
      NULL,
      NULL,
      book_view_gtk_class_init,
      NULL,
      NULL,
      sizeof (BookViewGtk),
      0,
      NULL,
      NULL
    };

    result = g_type_register_static (GTK_TYPE_FRAME,
				     "BookViewGtkType",
				     &info, (GTypeFlags) 0);
  }

  return result;
}
