
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2010 Damien Sandras <dsandras@seconix.com>
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
 *                        heap-view.cpp  -  description
 *                        --------------------------------
 *   begin                : written in november 2010
 *   copyright            : (C) 2010 by Julien Puydt
 *   description          : Implementation of a widget displaying an Ekiga::Heap
 *
 */

#include <glib/gi18n.h>

#include "heap-view.h"

#include "gm-cell-renderer-bitext.h"
#include "gmcellrendererexpander.h"
#include "menu-builder-tools.h"
#include "menu-builder-gtk.h"
#include "form-dialog-gtk.h"
#include "scoped-connections.h"

struct _HeapViewPrivate
{
  Ekiga::HeapPtr heap;
  Ekiga::scoped_connections connections;

  GtkTreeStore* store;
  GtkTreeView* view;
};

/* what objects will we display? */
enum {

  TYPE_GROUP,
  TYPE_PRESENTITY
};

/* what data will will we display?
 *
 * For a group, only the name
 *
 * For a presentity, the name, status and presence
 *
 */
enum {

  COLUMN_TYPE,
  COLUMN_PRESENTITY,
  COLUMN_NAME,
  COLUMN_STATUS,
  COLUMN_PRESENCE,
  COLUMN_NUMBER
};

enum {

  SELECTION_CHANGED_SIGNAL,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* Helpers' declarations */

static void expand_cell_data_func (GtkTreeViewColumn* column,
				   GtkCellRenderer* renderer,
				   GtkTreeModel* model,
				   GtkTreeIter* iter,
				   gpointer data);

static void clear_empty_groups (HeapView* self);

static void find_iter_for_group (HeapView* self,
				 const gchar* name,
				 GtkTreeIter* iter);

static void find_iter_for_presentity (HeapView* self,
				      Ekiga::Presentity* presentity,
				      GtkTreeIter* group_iter,
				      GtkTreeIter* iter);

static bool visit_presentities (HeapView* self,
				Ekiga::PresentityPtr presentity);
static void on_heap_removed (HeapView* self);
static void on_presentity_added (HeapView* self, Ekiga::PresentityPtr presentity);
static void on_presentity_updated (HeapView* self, Ekiga::PresentityPtr presentity);
static void on_presentity_removed (HeapView* self, Ekiga::PresentityPtr presentity);
static void hide_show_depending_on_type (GtkTreeViewColumn* column,
					 GtkCellRenderer* renderer,
					 GtkTreeModel* model,
					 GtkTreeIter* iter,
					 gpointer data);
static void on_selection_changed (GtkTreeSelection* selection,
				  gpointer data);
static void on_clicked_show_group_menu (HeapView* self,
					const gchar* name,
					GdkEventButton* event);
static void on_clicked_show_presentity_menu (HeapView* self,
					     Ekiga::Presentity* presentity,
					     GdkEventButton* event);
static gint on_right_click_in_the_view (GtkWidget* view,
					GdkEventButton* event,
					gpointer data);
static void heap_view_set_heap (HeapView* self,
				Ekiga::HeapPtr heap);

static bool on_questions (HeapView* self,
			  Ekiga::FormRequestPtr request);

/* Helpers' implementations */

static void
expand_cell_data_func (GtkTreeViewColumn* /*column*/,
		       GtkCellRenderer* renderer,
		       GtkTreeModel* model,
		       GtkTreeIter* iter,
		       gpointer data)
{
  HeapView* self = HEAP_VIEW (data);
  GtkTreePath *path = NULL;
  gint column_type;
  gboolean row_expanded = FALSE;

  path = gtk_tree_model_get_path (model, iter);
  row_expanded = gtk_tree_view_row_expanded (self->priv->view, path);
  gtk_tree_path_free (path);

  gtk_tree_model_get (model, iter, COLUMN_TYPE, &column_type, -1);

  if (column_type == TYPE_PRESENTITY)
    g_object_set (renderer, "visible", FALSE, NULL);
  else
    g_object_set (renderer, "visible", TRUE, NULL);

  g_object_set (renderer,
                "expander-style", row_expanded ? GTK_EXPANDER_EXPANDED : GTK_EXPANDER_COLLAPSED,
                NULL);
}


static void
clear_empty_groups (HeapView* self)
{
  GtkTreeModel* model = GTK_TREE_MODEL (self->priv->store);
  GtkTreeIter iter;
  bool go_on = true;

  if (gtk_tree_model_get_iter_first (model, &iter)) {

    do {

      if (gtk_tree_model_iter_has_child (model, &iter))
	go_on = gtk_tree_model_iter_next (model, &iter);
      else
	go_on = gtk_tree_store_remove (self->priv->store, &iter);

    } while (go_on);
  }
}

static void
find_iter_for_group (HeapView* self,
		     const gchar* name,
		     GtkTreeIter* iter)
{
  GtkTreeModel* model = NULL;
  gchar* group_name = NULL;
  bool found = false;

  model = GTK_TREE_MODEL (self->priv->store);

  if (gtk_tree_model_get_iter_first (model, iter)) {

    do {

      gtk_tree_model_get (model, iter, COLUMN_NAME, &group_name, -1);

      if (g_strcmp0 (name, group_name) == 0)
	found = true;

      g_free (group_name);
    } while (!found && gtk_tree_model_iter_next (model, iter));
  }

  if (!found) {

    gtk_tree_store_append (self->priv->store, iter, NULL);
    gtk_tree_store_set (self->priv->store, iter,
			COLUMN_TYPE, TYPE_GROUP,
			COLUMN_NAME, name,
			-1);
  }
}

static void
find_iter_for_presentity (HeapView* self,
			  Ekiga::Presentity* presentity,
			  GtkTreeIter* group_iter,
			  GtkTreeIter* iter)
{
  GtkTreeModel* model = NULL;
  Ekiga::Presentity* iter_presentity = NULL;
  bool found = false;

  model = GTK_TREE_MODEL (self->priv->store);

  if (gtk_tree_model_iter_nth_child (model, iter, group_iter, 0)) {

    do {

      gtk_tree_model_get (model, iter, COLUMN_PRESENTITY, &iter_presentity, -1);

      if (iter_presentity == presentity)
	found = true;
    } while (!found && gtk_tree_model_iter_next (model, iter));
  }

  if (!found) {

    gtk_tree_store_append (self->priv->store, iter, group_iter);
    // ugly, but I didn't find how to make a group appear as expanded
    // by default
    GtkTreePath* path
      = gtk_tree_model_get_path (GTK_TREE_MODEL (self->priv->store),
				 group_iter);
    (void)gtk_tree_view_expand_row (self->priv->view, path, TRUE);
    gtk_tree_path_free (path);
  }
}

static bool
visit_presentities (HeapView* self,
		    Ekiga::PresentityPtr presentity)
{
  on_presentity_added (self, presentity);

  return true;
}

static void
on_heap_removed (HeapView* self)
{
  heap_view_set_heap (self, Ekiga::HeapPtr());
}

static void
on_presentity_added (HeapView* self,
		     Ekiga::PresentityPtr presentity)
{
  GtkTreeIter group_iter;
  GtkTreeIter iter;
  std::set<std::string> groups = presentity->get_groups ();
  GtkTreeSelection* selection = gtk_tree_view_get_selection (self->priv->view);
  bool should_emit = false;

  if (groups.empty ())
    groups.insert (_("Unsorted"));

  for (std::set<std::string>::const_iterator group = groups.begin ();
       group != groups.end (); ++group) {

    find_iter_for_group (self, group->c_str (), &group_iter);
    find_iter_for_presentity (self, presentity.get (), &group_iter, &iter);

    if (gtk_tree_selection_iter_is_selected (selection, &iter))
      should_emit = true;

    std::string icon = "avatar-default";
    if (presentity->get_presence () != "unknown")
      icon = "user-" + presentity->get_presence ();

    gtk_tree_store_set (self->priv->store, &iter,
			COLUMN_TYPE, TYPE_PRESENTITY,
			COLUMN_PRESENTITY, presentity.get (),
			COLUMN_NAME, presentity->get_name ().c_str (),
			COLUMN_PRESENCE, icon.c_str (),
			COLUMN_STATUS, presentity->get_status ().c_str (),
			-1);
  }

  if (should_emit)
    g_signal_emit (self, signals[SELECTION_CHANGED_SIGNAL], 0);
}

static void
on_presentity_updated (HeapView* self,
		       Ekiga::PresentityPtr presentity)
{
  // first, make sure we are in all the groups we should be in, with up to date information
  on_presentity_added (self, presentity);

  // now, let's remove ourselves from the others
  GtkTreeModel* model;
  GtkTreeIter group_iter;
  GtkTreeIter iter;
  gchar* group_name = NULL;
  std::set<std::string> groups = presentity->get_groups ();

  if (groups.empty ())
    groups.insert (_("Unsorted"));

  model = GTK_TREE_MODEL (self->priv->store);

  if (gtk_tree_model_get_iter_first (model, &group_iter)) {

    do {

      gtk_tree_model_get (model, &group_iter, COLUMN_NAME, &group_name, -1);

      if (group_name != NULL) {

	if (groups.find (group_name) == groups.end ()) {

	  find_iter_for_presentity (self, presentity.get (), &group_iter, &iter);
	  gtk_tree_store_remove (self->priv->store, &iter);
	}
	g_free (group_name);
      }

    } while (gtk_tree_model_iter_next (model, &group_iter));
  }

  clear_empty_groups (self);
}

static void
on_presentity_removed (HeapView* self,
		       Ekiga::PresentityPtr presentity)
{
  GtkTreeModel* model;
  GtkTreeIter group_iter;
  GtkTreeIter iter;

  model = GTK_TREE_MODEL (self->priv->store);

  if (gtk_tree_model_get_iter_first (model, &group_iter)) {

    do {

      find_iter_for_presentity (self, presentity.get (), &group_iter, &iter);
      gtk_tree_store_remove (self->priv->store, &iter);
    } while (gtk_tree_model_iter_next (model, &group_iter));
  }

  clear_empty_groups (self);
}

static void
hide_show_depending_on_type (G_GNUC_UNUSED GtkTreeViewColumn* column,
			     GtkCellRenderer* renderer,
			     GtkTreeModel* model,
			     GtkTreeIter* iter,
			     gpointer data)
{
  gint column_type;

  gtk_tree_model_get (model, iter, COLUMN_TYPE, &column_type, -1);

  if (column_type == GPOINTER_TO_INT (data))
    g_object_set (renderer, "visible", TRUE, NULL);
  else
    g_object_set (renderer, "visible", FALSE, NULL);
}

static void
on_selection_changed (G_GNUC_UNUSED GtkTreeSelection* selection,
		      gpointer data)
{
  g_signal_emit (data, signals[SELECTION_CHANGED_SIGNAL], 0);
}

static void
on_clicked_show_group_menu (HeapView* self,
			    const gchar* name,
			    GdkEventButton* event)
{
  MenuBuilderGtk builder;
  self->priv->heap->populate_menu_for_group (name, builder);
  if (!builder.empty ()) {

    gtk_widget_show_all (builder.menu);
    gtk_menu_popup (GTK_MENU (builder.menu), NULL, NULL,
		    NULL, NULL, event->button, event->time);
  }
  g_object_ref_sink (builder.menu);
  g_object_unref (builder.menu);
}

static void
on_clicked_show_presentity_menu (HeapView* self,
				 Ekiga::Presentity* presentity,
				 GdkEventButton* event)
{
  Ekiga::TemporaryMenuBuilder temp;
  MenuBuilderGtk builder;

  self->priv->heap->populate_menu (temp);
  presentity->populate_menu (builder);

  if (!temp.empty ()) {

    builder.add_separator ();
    temp.populate_menu (builder);
  }

  if (!builder.empty ()) {

    gtk_widget_show_all (builder.menu);
    gtk_menu_popup (GTK_MENU (builder.menu), NULL, NULL,
		    NULL, NULL, event->button, event->time);
  }
  g_object_ref_sink (builder.menu);
  g_object_unref (builder.menu);
}

static gint
on_right_click_in_the_view (G_GNUC_UNUSED GtkWidget* view,
			    GdkEventButton* event,
			    gpointer data)
{
  HeapView* self = NULL;
  GtkTreeModel* model = NULL;
  GtkTreePath* path = NULL;
  GtkTreeIter iter;

  if (event->type != GDK_BUTTON_PRESS && event->type != GDK_2BUTTON_PRESS)
    return FALSE;

  self = HEAP_VIEW (data);
  model = gtk_tree_view_get_model (self->priv->view);

  if (gtk_tree_view_get_path_at_pos (self->priv->view, (gint)event->x, (gint)event->y,
				     &path, NULL, NULL, NULL)) {

    if (gtk_tree_model_get_iter (model, &iter, path)) {

      gint column_type;
      gchar* name = NULL;
      Ekiga::Presentity* presentity = NULL;
      gtk_tree_model_get (model, &iter,
			  COLUMN_TYPE, &column_type,
			  COLUMN_NAME, &name,
			  COLUMN_PRESENTITY, &presentity,
			  -1);

      switch (column_type) {

      case TYPE_GROUP:

	if (event->type == GDK_BUTTON_PRESS && event->button == 3)
	  on_clicked_show_group_menu (self, name, event);
	break;
      case TYPE_PRESENTITY:

	if (event->type == GDK_BUTTON_PRESS && event->button == 3)
	  on_clicked_show_presentity_menu (self, presentity, event);
	break;

      default:

	g_assert_not_reached ();
	break;
      }

      g_free (name);
    }

    gtk_tree_path_free (path);
  }

  return TRUE;
}

static void
heap_view_set_heap (HeapView* self,
		    Ekiga::HeapPtr heap)
{
  self->priv->connections.clear ();

  if (heap) {

    boost::signals2::connection conn;

    conn = heap->removed.connect (boost::bind (&on_heap_removed, self));
    self->priv->connections.add (conn);
    conn = heap->presentity_added.connect (boost::bind (&on_presentity_added, self, _1));
    self->priv->connections.add (conn);
    conn = heap->presentity_updated.connect (boost::bind (&on_presentity_updated, self, _1));
    self->priv->connections.add (conn);
    conn = heap->presentity_removed.connect (boost::bind (&on_presentity_removed, self, _1));
    self->priv->connections.add (conn);
    conn = heap->questions.connect (boost::bind (&on_questions, self, _1));
  }

  gtk_tree_store_clear (self->priv->store);
  self->priv->heap = heap;

  if (self->priv->heap)
    heap->visit_presentities (boost::bind (&visit_presentities, self, _1));
}

static bool
on_questions (HeapView* self,
	      Ekiga::FormRequestPtr request)
{
  GtkWidget* parent = gtk_widget_get_toplevel (GTK_WIDGET (self));
  FormDialog dialog (request, parent);

  dialog.run ();

  return true;
}

/* implementation of the GObject */

G_DEFINE_TYPE (HeapView, heap_view, GTK_TYPE_BOX);

static void
heap_view_finalize (GObject* obj)
{
  delete HEAP_VIEW (obj)->priv;
}

static void
heap_view_init (HeapView* self)
{
  GtkTreeViewColumn* col = NULL;
  GtkCellRenderer* renderer = NULL;
  GtkTreeSelection* selection = NULL;

  self->priv = new HeapViewPrivate;

  /* prepare the store */
  self->priv->store = gtk_tree_store_new (COLUMN_NUMBER,
					  G_TYPE_INT,     // type
					  G_TYPE_POINTER, // presentity
					  G_TYPE_STRING,  // name
					  G_TYPE_STRING,  // status
					  G_TYPE_STRING); // presence

  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (self->priv->store),
					COLUMN_NAME, GTK_SORT_ASCENDING);

  /* prepare the view */
  self->priv->view = GTK_TREE_VIEW (gtk_tree_view_new_with_model (GTK_TREE_MODEL (self->priv->store)));
  g_object_unref (self->priv->store);
  gtk_tree_view_set_headers_visible (self->priv->view, FALSE);
  gtk_box_pack_start (GTK_BOX(self), GTK_WIDGET (self->priv->view), TRUE, TRUE, 0);

  /* hidden column to hide the default expanders */
  col = gtk_tree_view_column_new ();
  gtk_tree_view_append_column (self->priv->view, col);
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_set_spacing (col, 0);
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  g_object_set (col, "visible", FALSE, NULL);
  gtk_tree_view_set_expander_column (self->priv->view, col);


  /* the real thing! */
  col = gtk_tree_view_column_new ();
  gtk_tree_view_append_column (self->priv->view, col);

  /* our own expanders */
  renderer = gm_cell_renderer_expander_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  g_object_set (renderer,
                "xalign", 0.0,
                "xpad", 0,
                "ypad", 0,
                "visible", TRUE,
                "expander-style", GTK_EXPANDER_COLLAPSED,
                NULL);
  gtk_tree_view_column_set_cell_data_func (col, renderer,
					   expand_cell_data_func, self, NULL);

  /* show the name of a group */
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_set_spacing (col, 0);
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COLUMN_NAME);
  gtk_tree_view_column_set_alignment (col, 0.0);
  g_object_set (renderer, "weight", PANGO_WEIGHT_BOLD, NULL);
  gtk_tree_view_column_set_cell_data_func (col, renderer,
                                           hide_show_depending_on_type,
					   GINT_TO_POINTER (TYPE_GROUP), NULL);

  /* show the presence of a presentity */
  renderer = gtk_cell_renderer_pixbuf_new ();
  g_object_set (renderer, "yalign", 0.5, "xpad", 5, NULL);
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_add_attribute (col, renderer,
				      "icon-name",
				      COLUMN_PRESENCE);
  gtk_tree_view_column_set_cell_data_func (col, renderer,
                                           hide_show_depending_on_type,
					   GINT_TO_POINTER (TYPE_PRESENTITY), NULL);

  /* show the name+status of a presentity */
  renderer = gm_cell_renderer_bitext_new ();
  gtk_tree_view_column_set_spacing (col, 0);
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_add_attribute (col, renderer, "primary-text", COLUMN_NAME);
  gtk_tree_view_column_add_attribute (col, renderer, "secondary-text", COLUMN_STATUS);
  gtk_tree_view_column_set_alignment (col, 0.0);
  g_object_set (renderer, "xalign", 0.5, "ypad", 0, NULL);
  gtk_tree_view_column_set_cell_data_func (col, renderer,
                                           hide_show_depending_on_type,
					   GINT_TO_POINTER (TYPE_PRESENTITY), NULL);

  /* notify when the selection changed */
  selection = gtk_tree_view_get_selection (self->priv->view);
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
  g_signal_connect (selection, "changed", G_CALLBACK (on_selection_changed), self);

  /* show a popup menu when right-click */
  g_signal_connect (self->priv->view, "event-after", G_CALLBACK (on_right_click_in_the_view), self);

  gtk_widget_show_all (GTK_WIDGET (self->priv->view));
}

static void
heap_view_class_init (HeapViewClass* klass)
{
  GObjectClass* gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = heap_view_finalize;

  signals[SELECTION_CHANGED_SIGNAL] =
    g_signal_new ("selection-changed",
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (HeapViewClass, selection_changed),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
}

/* public api */

GtkWidget*
heap_view_new (Ekiga::HeapPtr heap)
{
  GtkWidget* result = NULL;

  result = GTK_WIDGET (g_object_new (TYPE_HEAP_VIEW, NULL));

  heap_view_set_heap (HEAP_VIEW (result), heap);

  return result;
}


bool
heap_view_populate_menu_for_selected (HeapView* self,
				      Ekiga::MenuBuilder& builder)
{
  bool result = false;
  GtkTreeSelection* selection = NULL;
  GtkTreeModel* model = NULL;
  GtkTreeIter iter;

  g_return_val_if_fail (IS_HEAP_VIEW (self), false);

  selection = gtk_tree_view_get_selection (self->priv->view);
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gint column_type;
    gchar* name = NULL;
    Ekiga::Presentity* presentity = NULL;

    gtk_tree_model_get (model, &iter,
			COLUMN_TYPE, &column_type,
			COLUMN_NAME, &name,
			COLUMN_PRESENTITY, &presentity,
			-1);

    switch (column_type) {

    case TYPE_GROUP:

      result = self->priv->heap->populate_menu_for_group (name, builder);
      break;
    case TYPE_PRESENTITY:

      result = presentity->populate_menu (builder);
      break;

    default:
      break;
    }

    g_free (name);
  }

  return result;
}
