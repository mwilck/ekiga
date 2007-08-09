
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
 *                         roster-view-gtk.cpp -  description
 *                         ------------------------------------------
 *   begin                : written in 2006 by Julien Puydt
 *   copyright            : (c) 2006 by Julien Puydt
 *   description          : implementation of the roster view
 *
 */

#include <algorithm>
#include <iostream>

#include "roster-view-gtk.h"
#include "menu-builder-gtk.h"
#include "gm-cell-renderer-bitext.h"

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

  SignalCentralizer (Ekiga::PresenceCore &_core): core(_core)
  { }

  void start ()
  {
    core.visit_clusters (sigc::mem_fun (this,
					&SignalCentralizer::on_cluster_added));
    core.cluster_added.connect (sigc::mem_fun (this,
					       &SignalCentralizer::on_cluster_added));
  }

  void repopulate ()
  {
    core.visit_clusters (sigc::bind (sigc::mem_fun (this,
						    &SignalCentralizer::list_cluster), false));
  }

  /* signals emitted by this centralizer */

  sigc::signal<void, Ekiga::Heap &> heap_added;
  sigc::signal<void, Ekiga::Heap &> heap_updated;
  sigc::signal<void, Ekiga::Heap &> heap_removed;
  sigc::signal<void, Ekiga::Heap &, Ekiga::Presentity &> presentity_added;
  sigc::signal<void, Ekiga::Heap &, Ekiga::Presentity &> presentity_updated;
  sigc::signal<void, Ekiga::Heap &, Ekiga::Presentity &> presentity_removed;

private:

  Ekiga::PresenceCore &core;

  /* first level of the stack : clusters */

  void list_cluster (Ekiga::Cluster &cluster,
		     bool connect_signals = false)
  {
    if (connect_signals) {

      cluster.heap_added.connect (sigc::mem_fun (this, &SignalCentralizer::on_heap_added));
      cluster.heap_updated.connect (heap_updated.make_slot ());
      cluster.heap_removed.connect (heap_removed.make_slot ());
      cluster.visit_heaps (sigc::mem_fun (this,
					  &SignalCentralizer::on_heap_added));
    } else
      cluster.visit_heaps (sigc::bind (sigc::mem_fun (this, &SignalCentralizer::list_heap), false));
  }

  void on_cluster_added (Ekiga::Cluster &cluster)
  {
    list_cluster (cluster, true);
  }

  /* second level of the stack : heaps */

  void list_heap (Ekiga::Heap &heap,
		  bool connect_signals = false)
  {
    if (connect_signals) {

      heap.presentity_added.connect (sigc::bind (sigc::mem_fun (this, &SignalCentralizer::on_presentity_added), &heap));
      heap.presentity_updated.connect (sigc::bind (sigc::mem_fun (this, &SignalCentralizer::on_presentity_updated), &heap));
      heap.presentity_removed.connect (sigc::bind (sigc::mem_fun (this, &SignalCentralizer::on_presentity_removed), &heap));
    }
    heap.visit_presentities (sigc::bind (sigc::mem_fun (this, &SignalCentralizer::on_presentity_added), &heap));
  }

  void on_heap_added (Ekiga::Heap &heap)
  {
    heap_added.emit (heap);
    list_heap (heap, true);
  }

  /* third level of the stack : presentities */

  void on_presentity_added (Ekiga::Presentity &presentity,
			    Ekiga::Heap *heap)
  { presentity_added.emit (*heap, presentity); }

  void on_presentity_updated (Ekiga::Presentity &presentity,
			      Ekiga::Heap *heap)
  { presentity_updated.emit (*heap, presentity); }

  void on_presentity_removed (Ekiga::Presentity &presentity,
			      Ekiga::Heap *heap)
  { presentity_removed.emit (*heap, presentity); }

};

struct _RosterViewGtkPrivate
{

  _RosterViewGtkPrivate (Ekiga::PresenceCore &core)
    : centralizer(core)
  {}

  SignalCentralizer centralizer;
  GtkTreeStore *store;
  GtkTreeView *tree_view;
  GtkWidget *vbox;
  GtkWidget *scrolled_window;
};

static GObjectClass *parent_class = NULL;

/* the different type of things which will appear in the view */
enum {

  TYPE_HEAP,
  TYPE_GROUP,
  TYPE_PRESENTITY
};

/* This is how things will be stored roughly :
 * - the heaps are at the top ;
 * - under each heap come the groups ;
 * - under each group come the presentities.
 *
 * For the heaps, we show the name.
 *
 * For the groups, we show the name.
 *
 * For the presentities, we show the name, the status and the presence.
 *
 * This means we can share and put the name in a column.
 *
 */
enum {

  COLUMN_TYPE,
  COLUMN_HEAP,
  COLUMN_PRESENTITY,
  COLUMN_NAME,
  COLUMN_STATUS,
  COLUMN_PRESENCE,
  COLUMN_NUMBER
};

/* declaration of click management */

static gint on_view_clicked (GtkWidget *tree_view,
			     GdkEventButton *event,
			     gpointer data);

/* declaration of renderer managing function */

static void show_cell_data_func (GtkTreeViewColumn *column,
				 GtkCellRenderer *renderer,
				 GtkTreeModel *model,
				 GtkTreeIter *iter,
				 gpointer data);

/* declaration of iter handling functions */

static void find_iter_for_heap (RosterViewGtk *view,
				Ekiga::Heap &heap,
				GtkTreeIter *iter);

static void find_iter_for_group (RosterViewGtk *view,
				 GtkTreeIter *heap_iter,
				 const std::string name,
				 GtkTreeIter *iter);

static void find_iter_for_presentity (RosterViewGtk *view,
				      GtkTreeIter *group_iter,
				      Ekiga::Presentity &presentity,
				      GtkTreeIter *iter);

/* declaration of helper */

static void clean_empty_groups (RosterViewGtk *view,
				GtkTreeIter *heap_iter);

/* declaration of centralizer observer interface */

static void on_heap_added (Ekiga::Heap &heap,
			   gpointer data);

static void on_heap_updated (Ekiga::Heap &heap,
			     gpointer data);

static void on_heap_removed (Ekiga::Heap &heap,
			     gpointer data);

static void on_presentity_added (Ekiga::Heap &heap,
				 Ekiga::Presentity &presentity,
				 gpointer data);

static void on_presentity_updated (Ekiga::Heap &heap,
				   Ekiga::Presentity &presentity,
				   gpointer data);

static void on_presentity_removed (Ekiga::Heap &heap,
				   Ekiga::Presentity &presentity,
				   gpointer data);

/* implementation of click management */

static gint
on_view_clicked (GtkWidget *tree_view,
		 GdkEventButton *event,
		 gpointer data)
{
  RosterViewGtk *self = (RosterViewGtk *)data;
  GtkTreeModel *model = GTK_TREE_MODEL (self->priv->store);
  GtkTreePath *path = NULL;
  GtkTreeIter iter;
  gint column_type;
  Ekiga::Heap *heap = NULL;
  Ekiga::Presentity *presentity = NULL;

  if (event->type == GDK_BUTTON_PRESS || event->type == GDK_KEY_PRESS) {

    if (event->button == 3) {

      if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (tree_view),
					 (gint) event->x, (gint) event->y,
					 &path, NULL, NULL, NULL)) {

	if (gtk_tree_model_get_iter (model, &iter, path)) {

	  gtk_tree_model_get (model, &iter,
			      COLUMN_TYPE, &column_type,
			      COLUMN_HEAP, &heap,
			      COLUMN_PRESENTITY, &presentity,
			      -1);

	  switch (column_type) {

	  case TYPE_HEAP: {

	    MenuBuilderGtk builder;
	    heap->populate_menu (builder);
	    if (!builder.empty ()) {

	      gtk_widget_show_all (builder.menu);
	      gtk_menu_popup (GTK_MENU (builder.menu), NULL, NULL,
			      NULL, NULL, event->button, event->time);
	      g_signal_connect (G_OBJECT (builder.menu), "hide",
				GTK_SIGNAL_FUNC (g_object_unref),
				(gpointer) builder.menu);
	    }
	    g_object_ref_sink (G_OBJECT (builder.menu));
	    break;
	  }

	  case TYPE_GROUP: {

	    /* FIXME: what about making it possible for a heap to have actions on groups?
	     * It would allow for example (and optional to each Heap) group renaming,
	     * or group invitations to a MUC or anything collaborative!
	     * What is needed is :
	     * - store_set the &heap when adding a group so we have access to it here in heap
	     * - in this function we should also store_get on COLUMN_NAME to have the group name
	     *   (don't forget the g_free!)
	     * - add the proper populate_group_menu api to Ekiga::Heap
	     */
	    break;
	  }

	  case TYPE_PRESENTITY: {

	    MenuBuilderGtk builder;
	    presentity->populate_menu (builder);
	    if (!builder.empty ()) {

	      gtk_widget_show_all (builder.menu);
	      gtk_menu_popup (GTK_MENU (builder.menu), NULL, NULL,
			      NULL, NULL, event->button, event->time);
	      g_signal_connect (G_OBJECT (builder.menu), "hide",
				GTK_SIGNAL_FUNC (g_object_unref),
				(gpointer) builder.menu);
	    }
	    g_object_ref_sink (G_OBJECT (builder.menu));
	    break;
	  }

	  default:
	    break; // shouldn't happen
	  }
	  }
	}
	gtk_tree_path_free (path);
      }
    }
  return TRUE;
}

/* implementation of renderer managing function */

// shows the renderer only if the iter points to a line of type GPOINTER_TO_INT (data)

static void
show_cell_data_func (GtkTreeViewColumn */*column*/,
		     GtkCellRenderer *renderer,
		     GtkTreeModel *model,
		     GtkTreeIter *iter,
		     gpointer data)
{
  gint column_type;

  gtk_tree_model_get (model, iter, COLUMN_TYPE, &column_type, -1);

  if (column_type == GPOINTER_TO_INT (data))
    g_object_set (renderer, "visible", TRUE, NULL);
  else
    g_object_set (renderer, "visible", FALSE, NULL);

}

/* implementation of iter handling functions */

static void
find_iter_for_heap (RosterViewGtk *view,
		    Ekiga::Heap &heap,
		    GtkTreeIter *iter)
{
  GtkTreeModel *model = NULL;
  Ekiga::Heap *iter_heap = NULL;
  gboolean found = FALSE;

  model = GTK_TREE_MODEL (view->priv->store);

  if (gtk_tree_model_get_iter_first (model, iter)) {

    do {

      gtk_tree_model_get (model, iter, COLUMN_HEAP, &iter_heap, -1);
      if (iter_heap == &heap)
	found = TRUE;
    } while (!found && gtk_tree_model_iter_next (model, iter));
  }

  if (!found)
    gtk_tree_store_append (view->priv->store, iter, NULL);
}

static void
find_iter_for_group (RosterViewGtk *view,
		     GtkTreeIter *heap_iter,
		     const std::string name,
		     GtkTreeIter *iter)
{
  GtkTreeModel *model = NULL;
  gchar *group_name = NULL;
  gboolean found = FALSE;

  model = GTK_TREE_MODEL (view->priv->store);

  if (gtk_tree_model_iter_nth_child (model, iter, heap_iter, 0)) {

    do {

      gtk_tree_model_get (model, iter, COLUMN_NAME, &group_name, -1);
      if (group_name != NULL && name == group_name)
	found = TRUE;
      if (group_name != NULL)
	g_free (group_name);
    } while (!found && gtk_tree_model_iter_next (model, iter));
  }

  if (!found) {

    gtk_tree_store_append (view->priv->store, iter, heap_iter);
    gtk_tree_store_set (view->priv->store, iter,
			COLUMN_TYPE, TYPE_GROUP,
			COLUMN_NAME, name.c_str (),
			-1);
  }
}

static void
find_iter_for_presentity (RosterViewGtk *view,
			  GtkTreeIter *group_iter,
			  Ekiga::Presentity &presentity,
			  GtkTreeIter *iter)
{
  GtkTreeModel *model = NULL;
  Ekiga::Presentity *iter_presentity = NULL;
  gboolean found = FALSE;

  model = GTK_TREE_MODEL (view->priv->store);

  if (gtk_tree_model_iter_nth_child (model, iter, group_iter, 0)) {

    do {

      gtk_tree_model_get (model, iter, COLUMN_PRESENTITY, &iter_presentity, -1);
      if (iter_presentity == &presentity)
	found = TRUE;
    } while (!found && gtk_tree_model_iter_next (model, iter));
  }

  if (!found)
    gtk_tree_store_append (view->priv->store, iter, group_iter);
}

/* implementation of helper */

static void
clean_empty_groups (RosterViewGtk *view,
		    GtkTreeIter *heap_iter)
{
  GtkTreeModel *model = NULL;
  gboolean go_on = FALSE;
  GtkTreeIter iter;

  model = GTK_TREE_MODEL (view->priv->store);

  if (gtk_tree_model_iter_nth_child (model, &iter, heap_iter, 0)) {

    do {

      if (gtk_tree_model_iter_has_child (model, &iter))
	go_on = gtk_tree_model_iter_next (model, &iter);
      else
	go_on = gtk_tree_store_remove (view->priv->store, &iter);
    } while (go_on);
  }
}

/* implementation of centralizer observer interface */

static void
on_heap_added (Ekiga::Heap &heap,
	       gpointer data)
{
  RosterViewGtk *self = (RosterViewGtk *)data;
  GtkTreeIter iter;

  find_iter_for_heap (self, heap, &iter);

  gtk_tree_store_set (self->priv->store, &iter,
		      COLUMN_TYPE, TYPE_HEAP,
		      COLUMN_HEAP, &heap,
		      COLUMN_NAME, heap.get_name ().c_str (),
		      -1);
}

static void
on_heap_updated (Ekiga::Heap &heap,
		 gpointer data)
{
  RosterViewGtk *self = (RosterViewGtk *)data;
  GtkTreeIter iter;

  find_iter_for_heap (self, heap, &iter);

  gtk_tree_store_set (self->priv->store, &iter,
		      COLUMN_TYPE, TYPE_HEAP,
		      COLUMN_HEAP, &heap,
		      COLUMN_NAME, heap.get_name ().c_str (),
		      -1);
}

static void
on_heap_removed (Ekiga::Heap &heap,
		 gpointer data)
{
  RosterViewGtk *self = (RosterViewGtk *)data;
  GtkTreeIter iter;

  find_iter_for_heap (self, heap, &iter);
  gtk_tree_store_remove (self->priv->store, &iter);
}

static void
on_presentity_added (Ekiga::Heap &heap,
		     Ekiga::Presentity &presentity,
		     gpointer data)
{
  RosterViewGtk *self = (RosterViewGtk *)data;
  GtkTreeIter heap_iter;
  std::list<std::string> groups = presentity.get_groups ();
  GtkTreeIter group_iter;
  GtkTreeIter iter;

  find_iter_for_heap (self, heap, &heap_iter);

  for (std::list<std::string>::const_iterator group = groups.begin ();
       group != groups.end ();
       group++) {

    find_iter_for_group (self, &heap_iter, *group, &group_iter);
    find_iter_for_presentity (self, &group_iter, presentity, &iter);

    gtk_tree_store_set (self->priv->store, &iter,
			COLUMN_TYPE, TYPE_PRESENTITY,
			COLUMN_PRESENTITY, &presentity,
			COLUMN_NAME, presentity.get_name ().c_str (),
			COLUMN_STATUS, presentity.get_status ().c_str (),
			COLUMN_PRESENCE, presentity.get_presence ().c_str (),
			-1);
  }

  if (groups.empty ()) {

    find_iter_for_group (self, &heap_iter, "Unsorted", &group_iter);
    find_iter_for_presentity (self, &group_iter, presentity, &iter);
    gtk_tree_store_set (self->priv->store, &iter,
			COLUMN_TYPE, TYPE_PRESENTITY,
			COLUMN_PRESENTITY, &presentity,
			COLUMN_NAME, presentity.get_name ().c_str (),
			COLUMN_STATUS, presentity.get_status ().c_str (),
			COLUMN_PRESENCE, presentity.get_presence ().c_str (),
			-1);
  }
}

static void
on_presentity_updated (Ekiga::Heap &heap,
		       Ekiga::Presentity &presentity,
		       gpointer data)
{
  RosterViewGtk *self = (RosterViewGtk *)data;
  GtkTreeModel *model;
  GtkTreeIter heap_iter;
  GtkTreeIter group_iter;
  GtkTreeIter iter;
  bool had_to_remove = false;
  gchar *group_name = NULL;
  std::list<std::string> groups = presentity.get_groups ();

  model = GTK_TREE_MODEL (self->priv->store);

  if (groups.empty ())
    groups.push_back ("Unsorted");

  // this makes sure we are in all groups where we should
  on_presentity_added (heap, presentity, data);

  // now let's remove from all the others
  find_iter_for_heap (self, heap, &heap_iter);

  had_to_remove = false;
  if (gtk_tree_model_iter_nth_child (model, &group_iter, &heap_iter, 0)) {

    do {

      gtk_tree_model_get (model, &group_iter,
			  COLUMN_NAME, &group_name,
			  -1);
      if (group_name != NULL) {

	if (std::find (groups.begin (), groups.end (), group_name) == groups.end ()) {

	  find_iter_for_presentity (self, &group_iter, presentity, &iter);
	  gtk_tree_store_remove (self->priv->store, &iter);
	  had_to_remove = true;
	}
	g_free (group_name);
      }
    } while (gtk_tree_model_iter_next (model, &group_iter));
  }

  if (had_to_remove)
    clean_empty_groups (self, &heap_iter);
}

static void
on_presentity_removed (Ekiga::Heap &heap,
		       Ekiga::Presentity &presentity,
		       gpointer data)
{
  RosterViewGtk *self = (RosterViewGtk *)data;
  GtkTreeModel *model = NULL;
  GtkTreeIter heap_iter;
  GtkTreeIter group_iter;
  GtkTreeIter iter;

  find_iter_for_heap (self, heap, &heap_iter);
  model = GTK_TREE_MODEL (self->priv->store);

  if (gtk_tree_model_iter_nth_child (model, &group_iter, &heap_iter, 0)) {

    do {

      find_iter_for_presentity (self, &group_iter, presentity, &iter);
      gtk_tree_store_remove (self->priv->store, &iter);
    } while (gtk_tree_model_iter_next (model, &group_iter));
  }

  clean_empty_groups (self, &heap_iter);
}

/* public methods implementation */

GtkWidget *
roster_view_gtk_new (Ekiga::PresenceCore &core)
{
  RosterViewGtk *result = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeViewColumn *col = NULL;
  GtkCellRenderer *renderer = NULL;

  result = (RosterViewGtk *)g_object_new (ROSTER_VIEW_GTK_TYPE, NULL);

  result->priv = new _RosterViewGtkPrivate (core);

  result->priv->vbox = gtk_vbox_new (FALSE, 2);
  result->priv->scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW
				  (result->priv->scrolled_window),
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  result->priv->store = gtk_tree_store_new (COLUMN_NUMBER,
					    G_TYPE_INT, // type
					    G_TYPE_POINTER, // heap
					    G_TYPE_POINTER, // presentity
					    G_TYPE_STRING, // name
					    G_TYPE_STRING, // status
					    G_TYPE_STRING); // presence
  result->priv->tree_view = GTK_TREE_VIEW (gtk_tree_view_new_with_model (GTK_TREE_MODEL (result->priv->store)));

  gtk_tree_view_set_headers_visible (result->priv->tree_view, FALSE);

  selection = gtk_tree_view_get_selection (result->priv->tree_view);
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

  g_signal_connect (G_OBJECT (result->priv->tree_view), "event-after",
		    G_CALLBACK (on_view_clicked), result);

  gtk_container_add (GTK_CONTAINER (result), GTK_WIDGET (result->priv->vbox));
  gtk_container_add (GTK_CONTAINER (result->priv->vbox),
		     GTK_WIDGET (result->priv->scrolled_window));
  gtk_container_add (GTK_CONTAINER (result->priv->scrolled_window),
		     GTK_WIDGET (result->priv->tree_view));

  col = gtk_tree_view_column_new ();

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer,
				      "text", COLUMN_NAME);
  g_object_set (renderer, "weight", PANGO_WEIGHT_BOLD, NULL);
  g_object_set (renderer, "cell-background", "blue", NULL);
  gtk_tree_view_column_set_cell_data_func (col, renderer,
					   show_cell_data_func, GINT_TO_POINTER (TYPE_HEAP),
					   NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer,
				      "text", COLUMN_NAME);
  g_object_set (renderer, "weight", PANGO_WEIGHT_BOLD, NULL);
  g_object_set (renderer, "cell-background", "lightblue", NULL);
  gtk_tree_view_column_set_cell_data_func (col, renderer,
					   show_cell_data_func, GINT_TO_POINTER (TYPE_GROUP),
					   NULL);

  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer,
				      "stock-id",
				      COLUMN_PRESENCE);
  gtk_tree_view_column_set_cell_data_func (col, renderer,
					   show_cell_data_func, GINT_TO_POINTER (TYPE_PRESENTITY),
					   NULL);  

  renderer = gm_cell_renderer_bitext_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer,
				      "primary-text",
				      COLUMN_NAME);
  gtk_tree_view_column_add_attribute (col, renderer,
				      "secondary-text",
				      COLUMN_STATUS);
  gtk_tree_view_column_set_cell_data_func (col, renderer,
					   show_cell_data_func, GINT_TO_POINTER (TYPE_PRESENTITY),
					   NULL);

  gtk_tree_view_append_column (result->priv->tree_view, col);

  result->priv->centralizer.heap_added.connect (sigc::bind (sigc::ptr_fun (on_heap_added), (gpointer)result));
  result->priv->centralizer.heap_updated.connect (sigc::bind (sigc::ptr_fun (on_heap_updated), (gpointer)result));
  result->priv->centralizer.heap_removed.connect (sigc::bind (sigc::ptr_fun (on_heap_removed), (gpointer)result));
  result->priv->centralizer.presentity_added.connect (sigc::bind (sigc::ptr_fun (on_presentity_added), (gpointer)result));
  result->priv->centralizer.presentity_updated.connect (sigc::bind (sigc::ptr_fun (on_presentity_updated), (gpointer)result));
  result->priv->centralizer.presentity_removed.connect (sigc::bind (sigc::ptr_fun (on_presentity_removed), (gpointer)result));

  result->priv->centralizer.start ();

  return (GtkWidget *) result;
}

/* GObject boilerplate code */

static void
roster_view_gtk_dispose (GObject *obj)
{
  RosterViewGtk *view = NULL;

#ifdef __GNUC__
  g_print ("%s\n", __PRETTY_FUNCTION__);
#endif

  view = (RosterViewGtk *)obj;

  if (view->priv->tree_view) {

    g_signal_handlers_disconnect_matched (gtk_tree_view_get_selection (view->priv->tree_view),
					  (GSignalMatchType) G_SIGNAL_MATCH_DATA,
					  0, /* signal_id */
					  (GQuark) 0, /* detail */
					  NULL,	/* closure */
					  NULL,	/* func */
					  view); /* data */
    gtk_tree_store_clear (view->priv->store);

    view->priv->store = NULL;
    view->priv->tree_view = NULL;
  }

  parent_class->dispose (obj);
}


static void
roster_view_gtk_finalize (GObject *obj)
{
  RosterViewGtk *view = NULL;

#ifdef __GNUC__
  g_print ("%s\n", __PRETTY_FUNCTION__);
#endif

  view = (RosterViewGtk *)obj;

  delete view->priv;

  parent_class->finalize (obj);
}

static void
roster_view_gtk_class_init (gpointer g_class,
			    gpointer /*class_data*/)
{
  GObjectClass *gobject_class = NULL;

  parent_class = (GObjectClass *)g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass *)g_class;
  gobject_class->dispose = roster_view_gtk_dispose;
  gobject_class->finalize = roster_view_gtk_finalize;
}

GType
roster_view_gtk_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (RosterViewGtkClass),
      NULL,
      NULL,
      roster_view_gtk_class_init,
      NULL,
      NULL,
      sizeof (RosterViewGtk),
      0,
      NULL,
      NULL
    };

    result = g_type_register_static (GTK_TYPE_FRAME,
				     "RosterViewGtkType",
				     &info, (GTypeFlags) 0);
  }

  return result;
}
