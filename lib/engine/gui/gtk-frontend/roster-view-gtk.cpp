
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

#include "gm-cell-renderer-bitext.h"
#include "gmcellrendererexpander.h"
#include "gmstockicons.h"

#include "roster-view-gtk.h"
#include "menu-builder-gtk.h"


/*
 * The signals centralizer relays signals from the PresenceCore,
 * Heaps and Clusters of Heaps to the Gobject and dies with it.
 */
class SignalCentralizer: public sigc::trackable
{
public:

  SignalCentralizer (Ekiga::PresenceCore &_core);


  /* Watch the PresenceCore for changes : when clusters
   * are added. Connect callbacks to signals for Clusters present
   * when being initialized.
   */
  void watch_core ();


  /* Signals emitted by this centralizer and interesting
   * for our GObject
   */
  sigc::signal<void, Ekiga::Heap &> heap_added;
  sigc::signal<void, Ekiga::Heap &> heap_updated;
  sigc::signal<void, Ekiga::Heap &> heap_removed;

  sigc::signal<void, Ekiga::Heap &, Ekiga::Presentity &> presentity_added;
  sigc::signal<void, Ekiga::Heap &, Ekiga::Presentity &> presentity_updated;
  sigc::signal<void, Ekiga::Heap &, Ekiga::Presentity &> presentity_removed;


private:

  Ekiga::PresenceCore &core;

  void on_cluster_added (Ekiga::Cluster &cluster);

  void on_heap_added (Ekiga::Heap &heap);

  void on_presentity_added (Ekiga::Presentity &presentity,
                            Ekiga::Heap *heap);

  void on_presentity_updated (Ekiga::Presentity &presentity,
                              Ekiga::Heap *heap);

  void on_presentity_removed (Ekiga::Presentity &presentity,
                              Ekiga::Heap *heap);
};


SignalCentralizer::SignalCentralizer (Ekiga::PresenceCore &_core): core(_core)
{
}


void SignalCentralizer::watch_core ()
{
  // Trigger on_cluster_added when a Cluster is added
  core.cluster_added.connect (sigc::mem_fun (this, &SignalCentralizer::on_cluster_added));

  // Trigger on_cluster_added for all Clusters of the Ekiga::PresenceCore
  core.visit_clusters (sigc::mem_fun (this, &SignalCentralizer::on_cluster_added));
}


void SignalCentralizer::on_cluster_added (Ekiga::Cluster &cluster)
{
  cluster.heap_added.connect (sigc::mem_fun (this, &SignalCentralizer::on_heap_added));
  cluster.heap_updated.connect (heap_updated.make_slot ());
  cluster.heap_removed.connect (heap_removed.make_slot ());

  cluster.visit_heaps (sigc::mem_fun (this, &SignalCentralizer::on_heap_added));
}


void SignalCentralizer::on_heap_added (Ekiga::Heap &heap)
{
  heap_added.emit (heap);

  heap.presentity_added.connect (sigc::bind (sigc::mem_fun (this, &SignalCentralizer::on_presentity_added), &heap));
  heap.presentity_updated.connect (sigc::bind (sigc::mem_fun (this, &SignalCentralizer::on_presentity_updated), &heap));
  heap.presentity_removed.connect (sigc::bind (sigc::mem_fun (this, &SignalCentralizer::on_presentity_removed), &heap));

  heap.visit_presentities (sigc::bind (sigc::mem_fun (this, &SignalCentralizer::on_presentity_added), &heap));
}


void SignalCentralizer::on_presentity_added (Ekiga::Presentity &presentity,
                                             Ekiga::Heap *heap)
{ 
  presentity_added.emit (*heap, presentity); 
}


void SignalCentralizer::on_presentity_updated (Ekiga::Presentity &presentity,
                                               Ekiga::Heap *heap)
{ 
  presentity_updated.emit (*heap, presentity); 
}


void SignalCentralizer::on_presentity_removed (Ekiga::Presentity &presentity,
                                               Ekiga::Heap *heap)
{ 
  presentity_removed.emit (*heap, presentity); 
}


/*
 * The Roster
 */
struct _RosterViewGtkPrivate
{
  _RosterViewGtkPrivate (Ekiga::PresenceCore &core):centralizer (core) { }

  SignalCentralizer centralizer;
  GtkTreeStore *store;
  GtkTreeView *tree_view;
  GtkWidget *vbox;
  GtkWidget *scrolled_window;
};

/* the different type of things which will appear in the view */
enum {

  TYPE_HEAP,
  TYPE_GROUP,
  TYPE_PRESENTITY
};

static GObjectClass *parent_class = NULL;


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

/* 
 * Callbacks 
 */


/* DESCRIPTION  : Called when the user right-clicks on a heap, group or 
 *                presentity.
 * BEHAVIOR     : Update the menu and displays it as a popup.
 * PRE          : The gpointer must point to the RosterViewGtk GObject.
 */
static gint on_view_clicked (GtkWidget *tree_view,
			     GdkEventButton *event,
			     gpointer data);


/* DESCRIPTION  : Called for a given renderer in order to show or hide it.
 * BEHAVIOR     : Only show the renderer if current iter points to a line of 
 *                type GPOINTER_TO_INT (data).
 * PRE          : The gpointer must be TYPE_HEAP, TYPE_CLUSTER or TYPE_PRESENTITY
 *                once casted using GPOINTER_TO_INT.
 */
static void show_cell_data_func (GtkTreeViewColumn *column,
				 GtkCellRenderer *renderer,
				 GtkTreeModel *model,
				 GtkTreeIter *iter,
				 gpointer data);


/* DESCRIPTION  : Called for a given renderer in order to modify properties.
 * BEHAVIOR     : Expand the expander renderer if required. Change the backgrounds to 
 *                lightgray for Clusters, and hide the expander renderer for Presentity.
 * PRE          : /
 */
static void expand_cell_data_func (GtkTreeViewColumn *column,
                                   GtkCellRenderer *renderer,
                                   GtkTreeModel *model,
                                   GtkTreeIter *iter,
                                   gpointer data);

/* DESCRIPTION  : Called when the heap_updated or heap_added signals have been emitted
 *      ²         by the SignalCentralizer of the BookViewGtk.
 * BEHAVIOR     : Add or Update the Heap in the GtkTreeView. 
 * PRE          : /
 */
static void on_heap_updated (Ekiga::Heap &heap,
			     gpointer data);


/* DESCRIPTION  : Called when the heap_removed signal has been emitted
 *                by the SignalCentralizer of the BookViewGtk.
 * BEHAVIOR     : Removes the Heap from the GtkTreeView. All children,
 *                ie associated Presentity entities are also removed from
 *                the view.
 * PRE          : /
 */
static void on_heap_removed (Ekiga::Heap &heap,
			     gpointer data);


/* DESCRIPTION  : Called when the presentity_added signal has been emitted
 *                by the SignalCentralizer of the BookViewGtk.
 * BEHAVIOR     : Add the given Presentity into the Heap on which it was 
 *                added.
 * PRE          : A valid Heap.
 */
static void on_presentity_added (Ekiga::Heap &heap,
				 Ekiga::Presentity &presentity,
				 gpointer data);


/* DESCRIPTION  : Called when the presentity_updated signal has been emitted
 *                by the SignalCentralizer of the BookViewGtk.
 * BEHAVIOR     : Update the given Presentity into the Heap.
 * PRE          : A valid Heap.
 */
static void on_presentity_updated (Ekiga::Heap &heap,
				   Ekiga::Presentity &presentity,
				   gpointer data);


/* DESCRIPTION  : Called when the presentity_removed signal has been emitted
 *                by the SignalCentralizer of the BookViewGtk.
 * BEHAVIOR     : Remove the given Presentity from the given Heap.
 * PRE          : A valid Heap.
 */
static void on_presentity_removed (Ekiga::Heap &heap,
				   Ekiga::Presentity &presentity,
				   gpointer data);


/* 
 * Static helper functions
 */

/* DESCRIPTION  : /
 * BEHAVIOR     : Update the iter parameter so that it points to
 *                the GtkTreeIter corresponding to the given Heap.
 * PRE          : / 
 */
static void roster_view_gtk_find_iter_for_heap (RosterViewGtk *view,
                                                Ekiga::Heap &heap,
                                                GtkTreeIter *iter);


/* DESCRIPTION  : /
 * BEHAVIOR     : Update the iter parameter so that it points to
 *                the GtkTreeIter corresponding to the given group name
 *                in the given Heap.
 * PRE          : /
 */
static void roster_view_gtk_find_iter_for_group (RosterViewGtk *view,
                                                 GtkTreeIter *heap_iter,
                                                 const std::string name,
                                                 GtkTreeIter *iter);


/* DESCRIPTION  : /
 * BEHAVIOR     : Update the iter parameter so that it points to
 *                the GtkTreeIter corresponding to the given presentity
 *                in the given group.
 * PRE          : /
 */
static void roster_view_gtk_find_iter_for_presentity (RosterViewGtk *view,
                                                      GtkTreeIter *group_iter,
                                                      Ekiga::Presentity &presentity,
                                                      GtkTreeIter *iter);


/* DESCRIPTION  : /
 * BEHAVIOR     : Do a clean up in the RosterViewGtk to clean all empty groups
 *                from the view.
 * PRE          : /
 */
static void roster_view_gtk_clean_empty_groups (RosterViewGtk *view,
                                                GtkTreeIter *heap_iter);



/* Implementation of the callbacks */
static gint
on_view_clicked (GtkWidget *tree_view,
		 GdkEventButton *event,
		 gpointer data)
{
  RosterViewGtk *self = NULL;
  GtkTreeModel *model = NULL; 

  GtkTreePath *path = NULL;
  GtkTreeIter iter;
  gint column_type;

  Ekiga::Heap *heap = NULL;
  Ekiga::Presentity *presentity = NULL;

  self = ROSTER_VIEW_GTK (data);
  model = GTK_TREE_MODEL (self->priv->store);

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

          case TYPE_HEAP: 
              {
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

          case TYPE_GROUP:

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

          case TYPE_PRESENTITY:
              {
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


static void
show_cell_data_func (GtkTreeViewColumn * /*column*/,
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


static void
expand_cell_data_func (GtkTreeViewColumn *column,
                       GtkCellRenderer *renderer,
                       GtkTreeModel *model,
                       GtkTreeIter *iter,
                       gpointer data)
{
  GtkTreePath *path = NULL;
  gint column_type;
  gboolean row_expanded = FALSE;

  path = gtk_tree_model_get_path (model, iter);
  row_expanded = gtk_tree_view_row_expanded (GTK_TREE_VIEW (column->tree_view), path);
  gtk_tree_path_free (path);

  gtk_tree_model_get (model, iter, COLUMN_TYPE, &column_type, -1);

  if (column_type == TYPE_HEAP)
    g_object_set (renderer, "cell-background", "lightgray", NULL);
  else
    g_object_set (renderer, "cell-background", NULL, NULL);

  if (column_type == TYPE_PRESENTITY)
    g_object_set (renderer, "visible", FALSE, NULL);
  else
    g_object_set (renderer, "visible", TRUE, NULL);

  g_object_set (renderer,
                "expander-style", row_expanded ? GTK_EXPANDER_EXPANDED : GTK_EXPANDER_COLLAPSED,
                NULL);
}


static void
on_heap_updated (Ekiga::Heap &heap,
		 gpointer data)
{
  RosterViewGtk *self = ROSTER_VIEW_GTK (data);
  GtkTreeIter iter;

  roster_view_gtk_find_iter_for_heap (self, heap, &iter);

  gtk_tree_store_set (self->priv->store, &iter,
		      COLUMN_TYPE, TYPE_HEAP,
		      COLUMN_HEAP, &heap,
		      COLUMN_NAME, heap.get_name ().c_str (),
		      -1);
  gtk_tree_view_expand_all (self->priv->tree_view);
}


static void
on_heap_removed (Ekiga::Heap &heap,
		 gpointer data)
{
  RosterViewGtk *self = ROSTER_VIEW_GTK (data);
  GtkTreeIter iter;

  roster_view_gtk_find_iter_for_heap (self, heap, &iter);

  gtk_tree_store_remove (self->priv->store, &iter);
}


static void
on_presentity_added (Ekiga::Heap &heap,
		     Ekiga::Presentity &presentity,
		     gpointer data)
{
  RosterViewGtk *self = ROSTER_VIEW_GTK (data);
  GtkTreeIter heap_iter;
  std::list<std::string> groups = presentity.get_groups ();
  GtkTreeIter group_iter;
  GtkTreeIter iter;

  roster_view_gtk_find_iter_for_heap (self, heap, &heap_iter);

  for (std::list<std::string>::const_iterator group = groups.begin ();
       group != groups.end ();
       group++) {

    roster_view_gtk_find_iter_for_group (self, &heap_iter, *group, &group_iter);
    roster_view_gtk_find_iter_for_presentity (self, &group_iter, presentity, &iter);

    gtk_tree_store_set (self->priv->store, &iter,
			COLUMN_TYPE, TYPE_PRESENTITY,
			COLUMN_PRESENTITY, &presentity,
			COLUMN_NAME, presentity.get_name ().c_str (),
			COLUMN_STATUS, presentity.get_status ().c_str (),
			COLUMN_PRESENCE, GM_STOCK_STATUS_OFFLINE,
			-1);
  }

  if (groups.empty ()) {

    roster_view_gtk_find_iter_for_group (self, &heap_iter, _("Unsorted"), &group_iter);
    roster_view_gtk_find_iter_for_presentity (self, &group_iter, presentity, &iter);
    gtk_tree_store_set (self->priv->store, &iter,
			COLUMN_TYPE, TYPE_PRESENTITY,
			COLUMN_PRESENTITY, &presentity,
			COLUMN_NAME, presentity.get_name ().c_str (),
			COLUMN_STATUS, presentity.get_status ().c_str (),
			COLUMN_PRESENCE, GM_STOCK_STATUS_OFFLINE,
			-1);
  }
}


static void
on_presentity_updated (Ekiga::Heap &heap,
		       Ekiga::Presentity &presentity,
		       gpointer data)
{
  RosterViewGtk *self = ROSTER_VIEW_GTK (data);
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

  // This makes sure we are in all groups where we should
  on_presentity_added (heap, presentity, data);

  // Now let's remove from all the others
  roster_view_gtk_find_iter_for_heap (self, heap, &heap_iter);

  had_to_remove = false;
  if (gtk_tree_model_iter_nth_child (model, &group_iter, &heap_iter, 0)) {

    do {

      gtk_tree_model_get (model, &group_iter,
			  COLUMN_NAME, &group_name,
			  -1);
      if (group_name != NULL) {

	if (std::find (groups.begin (), groups.end (), group_name) == groups.end ()) {

	  roster_view_gtk_find_iter_for_presentity (self, &group_iter, presentity, &iter);
	  gtk_tree_store_remove (self->priv->store, &iter);
	  had_to_remove = true;
	}
	g_free (group_name);
      }
    } while (gtk_tree_model_iter_next (model, &group_iter));
  }

  if (had_to_remove)
    roster_view_gtk_clean_empty_groups (self, &heap_iter);
}


static void
on_presentity_removed (Ekiga::Heap &heap,
		       Ekiga::Presentity &presentity,
		       gpointer data)
{
  RosterViewGtk *self = ROSTER_VIEW_GTK (data);
  GtkTreeModel *model = NULL;
  GtkTreeIter heap_iter;
  GtkTreeIter group_iter;
  GtkTreeIter iter;

  roster_view_gtk_find_iter_for_heap (self, heap, &heap_iter);
  model = GTK_TREE_MODEL (self->priv->store);

  if (gtk_tree_model_iter_nth_child (model, &group_iter, &heap_iter, 0)) {

    do {

      roster_view_gtk_find_iter_for_presentity (self, &group_iter, presentity, &iter);
      gtk_tree_store_remove (self->priv->store, &iter);
    } while (gtk_tree_model_iter_next (model, &group_iter));
  }

  roster_view_gtk_clean_empty_groups (self, &heap_iter);
}



/* 
 * Implementation of the static helpers.
 */
static void
roster_view_gtk_find_iter_for_heap (RosterViewGtk *view,
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
roster_view_gtk_find_iter_for_group (RosterViewGtk *view,
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
roster_view_gtk_find_iter_for_presentity (RosterViewGtk *view,
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


static void
roster_view_gtk_clean_empty_groups (RosterViewGtk *view,
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


/* 
 * GObject stuff
 */
static void
roster_view_gtk_dispose (GObject *obj)
{
  RosterViewGtk *view = NULL;

  view = ROSTER_VIEW_GTK (obj);

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

  parent_class = (GObjectClass *) g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass *) g_class;
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


/* 
 * Public API
 */
GtkWidget *
roster_view_gtk_new (Ekiga::PresenceCore &core)
{
  RosterViewGtk *self = NULL;

  GtkTreeSelection *selection = NULL;
  GtkTreeViewColumn *col = NULL;
  GtkCellRenderer *renderer = NULL;

  self = (RosterViewGtk *) g_object_new (ROSTER_VIEW_GTK_TYPE, NULL);

  self->priv = new _RosterViewGtkPrivate (core);

  self->priv->vbox = gtk_vbox_new (FALSE, 2);
  self->priv->scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (self->priv->scrolled_window),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  self->priv->store = gtk_tree_store_new (COLUMN_NUMBER,
                                          G_TYPE_INT,           // type
                                          G_TYPE_POINTER,       // heap
                                          G_TYPE_POINTER,       // presentity
                                          G_TYPE_STRING,        // name
                                          G_TYPE_STRING,        // status
                                          G_TYPE_STRING);       // presence
  self->priv->tree_view = GTK_TREE_VIEW (gtk_tree_view_new_with_model (GTK_TREE_MODEL (self->priv->store)));

  gtk_tree_view_set_headers_visible (self->priv->tree_view, FALSE);

  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (self->priv->vbox));
  gtk_container_add (GTK_CONTAINER (self->priv->vbox),
		     GTK_WIDGET (self->priv->scrolled_window));
  gtk_container_add (GTK_CONTAINER (self->priv->scrolled_window),
		     GTK_WIDGET (self->priv->tree_view));

  /* Build the GtkTreeView */
  // We hide the normal GTK+ expanders and use our own
  col = gtk_tree_view_column_new ();
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_set_spacing (col, 0);
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  g_object_set (col, "visible", FALSE, NULL); 
  gtk_tree_view_append_column (self->priv->tree_view, col);
  gtk_tree_view_set_expander_column (self->priv->tree_view, col);

  col = gtk_tree_view_column_new ();
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_set_spacing (col, 0);
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COLUMN_NAME);
  gtk_tree_view_column_set_alignment (col, 0.0);
  g_object_set (renderer, "weight", PANGO_WEIGHT_BOLD, NULL);
  g_object_set (renderer, "cell-background", "lightgray", NULL);
  gtk_tree_view_column_set_cell_data_func (col, renderer, show_cell_data_func, GINT_TO_POINTER (TYPE_HEAP), NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer,
				      "text", COLUMN_NAME);
  gtk_tree_view_column_set_alignment (col, 0.0);
  g_object_set (renderer, "weight", PANGO_WEIGHT_BOLD, NULL);
  gtk_tree_view_column_set_cell_data_func (col, renderer, show_cell_data_func, GINT_TO_POINTER (TYPE_GROUP), NULL);

  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_add_attribute (col, renderer,
				      "stock-id",
				      COLUMN_PRESENCE);
  gtk_tree_view_column_set_alignment (col, 0.0);
  gtk_tree_view_column_set_cell_data_func (col, renderer, show_cell_data_func, GINT_TO_POINTER (TYPE_PRESENTITY), NULL);  

  renderer = gm_cell_renderer_bitext_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_add_attribute (col, renderer, "primary-text", COLUMN_NAME);
  gtk_tree_view_column_add_attribute (col, renderer, "secondary-text", COLUMN_STATUS);
  gtk_tree_view_column_set_alignment (col, 0.0);
  gtk_tree_view_column_set_cell_data_func (col, renderer, show_cell_data_func, GINT_TO_POINTER (TYPE_PRESENTITY), NULL);

  renderer = gm_cell_renderer_expander_new ();
  gtk_tree_view_column_pack_end (col, renderer, FALSE);
  g_object_set (renderer,
                "visible", TRUE,
                "expander-style", GTK_EXPANDER_COLLAPSED,
                NULL);
  gtk_tree_view_column_set_cell_data_func (col, renderer, expand_cell_data_func, NULL, NULL);
  gtk_tree_view_append_column (self->priv->tree_view, col);

  /* Callback when the selection has been changed */
  selection = gtk_tree_view_get_selection (self->priv->tree_view);
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (self->priv->tree_view), "event-after",
		    G_CALLBACK (on_view_clicked), self);


  /* Relay signals */
  self->priv->centralizer.heap_added.connect (sigc::bind (sigc::ptr_fun (on_heap_updated), (gpointer) self));
  self->priv->centralizer.heap_updated.connect (sigc::bind (sigc::ptr_fun (on_heap_updated), (gpointer) self));
  self->priv->centralizer.heap_removed.connect (sigc::bind (sigc::ptr_fun (on_heap_removed), (gpointer) self));
  
  self->priv->centralizer.presentity_added.connect (sigc::bind (sigc::ptr_fun (on_presentity_added), (gpointer) self));
  self->priv->centralizer.presentity_updated.connect (sigc::bind (sigc::ptr_fun (on_presentity_updated), (gpointer) self));
  self->priv->centralizer.presentity_removed.connect (sigc::bind (sigc::ptr_fun (on_presentity_removed), (gpointer) self));

  self->priv->centralizer.watch_core ();

  return (GtkWidget *) self;
}
