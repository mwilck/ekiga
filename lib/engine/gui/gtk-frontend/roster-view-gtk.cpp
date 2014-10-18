
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2014 Damien Sandras <dsandras@seconix.com>
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
 *   copyright            : (c) 2007 by Julien Puydt
 *                          (c) 2014 by Damien Sandras
 *   description          : implementation of the roster view
 *
 */

#include <ctime>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>

#include "ekiga-settings.h"

#include "gm-cell-renderer-bitext.h"
#include "gm-cell-renderer-expander.h"
#include "menu-builder-tools.h"
#include "roster-view-gtk.h"
#include "menu-builder-gtk.h"
#include "form-dialog-gtk.h"
#include "scoped-connections.h"
#include "gactor-menu.h"

/*
 * The Roster
 */
struct _RosterViewGtkPrivate
{
  Ekiga::scoped_connections connections;
  Ekiga::Settings *settings;
  GtkTreeStore *store;
  GtkTreeView *tree_view;
  GSList *folded_groups;
  gboolean show_offline_contacts;

  Ekiga::GActorMenuPtr presentity_menu;
  Ekiga::GActorMenuPtr heap_menu;
};

typedef struct _StatusIconInfo {

  GtkTreeModel *model;
  GtkTreeIter *iter;
  int cpt;

} StatusIconInfo;

/* the different type of things which will appear in the view */
enum {

  TYPE_HEAP,
  TYPE_GROUP,
  TYPE_PRESENTITY
};

enum {
  ACTIONS_CHANGED_SIGNAL,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (RosterViewGtk, roster_view_gtk, GTK_TYPE_FRAME);

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
  COLUMN_PRESENCE_ICON,
  COLUMN_AVATAR_PIXBUF,
  COLUMN_FOREGROUND_COLOR,
  COLUMN_BACKGROUND_COLOR,
  COLUMN_GROUP_NAME,
  COLUMN_PRESENCE,
  COLUMN_OFFLINE,
  COLUMN_TIMEOUT,
  COLUMN_NUMBER
};

/*
 * Debug helpers
 */
//static void dump_model_content (GtkTreeModel* model);

/*
 * Time out callbacks
 */
static int roster_view_gtk_icon_blink_cb (gpointer data);

static void status_icon_info_delete (gpointer info);


/*
 * Helpers
 */

/* DESCRIPTION : Called when the user clicks in a view.
 * BEHAVIOUR   : Folds/unfolds.
 */
static void on_clicked_fold (RosterViewGtk* self,
			     GtkTreePath* path,
			     const gchar* name);

static void on_clicked_trigger_presentity (Ekiga::Presentity* presentity);

/* DESCRIPTION : Called whenever a (online/total) count has to be updated
 * BEHAVIOUR   : Updates things...
 * PRE         : Both arguments have to be correct
 */
static void update_offline_count (RosterViewGtk* self,
				  GtkTreeIter* iter);

/* DESCRIPTION : Called when the user changes the preference for offline
 * BEHAVIOUR   : Updates things...
 * PRE         : The gpointer must be a RosterViewGtk
 */
static void show_offline_contacts_changed_cb (GSettings *settings,
					      gchar *key,
					      gpointer data);

/* DESCRIPTION  : Called when the user selects a presentity.
 * BEHAVIOR     : Rebuilds menus and emit the actions_changed signal.
 * PRE          : The gpointer must point to the RosterViewGtk GObject.
 */
static void on_selection_changed (GtkTreeSelection* actions,
                                  gpointer data);

/* DESCRIPTION  : Called when the user clicks or presses Enter
 *                on a heap, group or presentity.
 * BEHAVIOR     : Update the menu and displays it as a popup.
 * PRE          : The gpointer must point to the RosterViewGtk GObject.
 */
static gint on_view_event_after (GtkWidget *tree_view,
			         GdkEventButton *event,
			         gpointer data);

/* DESCRIPTION : Helpers for the next function
 */

static gboolean presentity_hide_show_offline (RosterViewGtk* self,
					      GtkTreeModel* model,
					      GtkTreeIter* iter);
static gboolean group_hide_show_offline (RosterViewGtk* self,
					 GtkTreeModel* model,
					 GtkTreeIter* iter);

/* DESCRIPTION : Called to decide whether to show a line ; used to hide/show
 *               offline contacts on demand.
 * BEHAVIOUR   : Returns TRUE if the line should be shown.
 * PRE         : The gpointer must point to a RosterViewGtk object.
 */
static gboolean tree_model_filter_hide_show_offline (GtkTreeModel *model,
						     GtkTreeIter *iter,
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
 * BEHAVIOR     : Expand the expander renderer if required.
 *                Hide the expander renderer for Presentity.
 *                and Heap.
 * PRE          : /
 */
static void expand_cell_data_func (GtkTreeViewColumn *column,
                                   GtkCellRenderer *renderer,
                                   GtkTreeModel *model,
                                   GtkTreeIter *iter,
                                   gpointer data);


/* DESCRIPTION  : Called when a new cluster has been added
 * BEHAVIOR     : Visits the cluster's heaps, and add them to the view
 * PRE          : /
 */
static bool on_visit_clusters (RosterViewGtk* self,
			       Ekiga::ClusterPtr cluster);


/* DESCRIPTION  : Called when a new cluster has been added
 * BEHAVIOR     : Visits the cluster's heaps, and add them to the view
 * PRE          : /
 */
static void on_cluster_added (RosterViewGtk* self,
			      Ekiga::ClusterPtr cluster);


/* DESCRIPTION  : Called when visiting a new cluster
 * BEHAVIOR     : Adds in the cluster heaps
 * PRE          : /
 */
static bool visit_heaps (RosterViewGtk* self,
			 Ekiga::ClusterPtr cluster,
			 Ekiga::HeapPtr heap);

/* DESCRIPTION  : Called when the or heap_added signal has been emitted
 * BEHAVIOR     : Add or Update the Heap in the GtkTreeView.
 * PRE          : /
 */
static void on_heap_added (RosterViewGtk* self,
			   Ekiga::ClusterPtr cluster,
			   Ekiga::HeapPtr heap);

/* DESCRIPTION  : Called when the heap_updated signal has been emitted
 * BEHAVIOR     : Add or Update the Heap in the GtkTreeView.
 * PRE          : /
 */
static void on_heap_updated (RosterViewGtk* self,
			     Ekiga::ClusterPtr cluster,
			     Ekiga::HeapPtr heap);


/* DESCRIPTION  : Called when the heap_removed signal has been emitted
 *                by the SignalCentralizer of the BookViewGtk.
 * BEHAVIOR     : Removes the Heap from the GtkTreeView. All children,
 *                ie associated Presentity entities are also removed from
 *                the view.
 * PRE          : /
 */
static void on_heap_removed (RosterViewGtk* self,
			     Ekiga::ClusterPtr cluster,
			     Ekiga::HeapPtr heap);


/* DESCRIPTION  : Called when visiting a new heap
 * BEHAVIOR     : Adds in the heap presentities
 * PRE          : /
 */
static bool visit_presentities (RosterViewGtk* self,
				Ekiga::ClusterPtr cluster,
				Ekiga::HeapPtr heap,
				Ekiga::PresentityPtr presentity);


/* DESCRIPTION  : Called when the presentity_added signal has been emitted
 *                by the SignalCentralizer of the BookViewGtk.
 * BEHAVIOR     : Add the given Presentity into the Heap on which it was
 *                added.
 * PRE          : A valid Heap.
 */
static void on_presentity_added (RosterViewGtk* self,
				 Ekiga::ClusterPtr cluster,
				 Ekiga::HeapPtr heap,
				 Ekiga::PresentityPtr presentity);


/* DESCRIPTION  : Called when the presentity_updated signal has been emitted
 *                by the SignalCentralizer of the BookViewGtk.
 * BEHAVIOR     : Update the given Presentity into the Heap.
 * PRE          : A valid Heap.
 */
static void on_presentity_updated (RosterViewGtk* self,
				   Ekiga::ClusterPtr cluster,
				   Ekiga::HeapPtr heap,
				   Ekiga::PresentityPtr presentity);


/* DESCRIPTION  : Called when the presentity_removed signal has been emitted
 *                by the SignalCentralizer of the BookViewGtk.
 * BEHAVIOR     : Remove the given Presentity from the given Heap.
 * PRE          : A valid Heap.
 */
static void on_presentity_removed (RosterViewGtk* self,
				   Ekiga::ClusterPtr cluster,
				   Ekiga::HeapPtr heap,
				   Ekiga::PresentityPtr presentity);


/* DESCRIPTION  : Called when the PresenceCore has a form request to handle
 * BEHAVIOR     : Runs the form request in gtk+
 * PRE          : The given pointer is the roster view widget
 */
static bool on_handle_questions (RosterViewGtk* self,
				 Ekiga::FormRequestPtr request);


/*
 * Static helper functions
 */

/* DESCRIPTION  : /
 * BEHAVIOR     : Update the iter parameter so that it points to
 *                the GtkTreeIter corresponding to the given Heap.
 * PRE          : /
 */
static void roster_view_gtk_find_iter_for_heap (RosterViewGtk *view,
                                                Ekiga::HeapPtr heap,
                                                GtkTreeIter *iter);


/* DESCRIPTION  : /
 * BEHAVIOR     : Update the iter parameter so that it points to
 *                the GtkTreeIter corresponding to the given group name
 *                in the given Heap.
 * PRE          : /
 */
static void roster_view_gtk_find_iter_for_group (RosterViewGtk *view,
						 Ekiga::HeapPtr heap,
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
                                                      Ekiga::PresentityPtr presentity,
                                                      GtkTreeIter *iter);


/* DESCRIPTION  : /
 * BEHAVIOR     : Do a clean up in the RosterViewGtk to clean all empty groups
 *                from the view. It also folds or unfolds groups following
 *                the value of the appropriate GMConf key.
 * PRE          : /
 */
static void roster_view_gtk_update_groups (RosterViewGtk *view,
                                           GtkTreeIter *heap_iter);

/* Implementation of the debuggers */

// static void
// dump_model_content (GtkTreeModel* model)
// {
//   g_return_if_fail (GTK_IS_TREE_MODEL (model));

//   GtkTreeIter heaps;
//   gchar* name = NULL;

//   if (gtk_tree_model_get_iter_first (model, &heaps)) {

//     do {

//       gtk_tree_model_get (model, &heaps,
// 			  COLUMN_NAME, &name,
// 			  -1);
//       if (name)
// 	g_print ("%s\n", name);
//       else
// 	g_print ("(NULL name)\n");
//       g_free (name);

//       GtkTreeIter groups;
//       if (gtk_tree_model_iter_nth_child (model, &groups, &heaps, 0)) {

// 	do {

// 	  gtk_tree_model_get (model, &groups,
// 			      COLUMN_NAME, &name,
// 			      -1);
// 	  if (name)
// 	    g_print ("\t%s\n", name);
// 	  else
// 	    g_print ("\t(NULL name)\n");
// 	  g_free (name);

// 	  GtkTreeIter presentities;
// 	  if (gtk_tree_model_iter_nth_child (model, &presentities, &groups, 0)) {

// 	    do {

// 	      gtk_tree_model_get (model, &presentities,
// 				  COLUMN_NAME, &name,
// 				  -1);
// 	      if (name)
// 		g_print ("\t\t%s\n", name);
// 	      else
// 		g_print ("\t\t(NULL name)\n");
// 	      g_free (name);
// 	    } while (gtk_tree_model_iter_next (model, &presentities));
// 	  } else
// 	    g_print ("\t\t(empty group)");

// 	} while (gtk_tree_model_iter_next (model, &groups));
//       } else
// 	g_print ("\t(empty heap)\n");

//     } while (gtk_tree_model_iter_next (model, &heaps));

//   } else
//     g_print ("(empty model)\n");
// }


/* Implementation of the timer callbacks */
static int
roster_view_gtk_icon_blink_cb (gpointer data)
{
  gchar *presence = NULL;
  g_return_val_if_fail (data != NULL, FALSE);

  StatusIconInfo *info = (StatusIconInfo*) data;
  time_t now = time (0);
  tm *ltm = localtime (&now);

  gtk_tree_model_get (GTK_TREE_MODEL (info->model), info->iter,
                      COLUMN_PRESENCE, &presence, -1);

  std::string icon = "user-offline";
  if (info->cpt == 0) {
    gtk_tree_store_set (GTK_TREE_STORE (info->model), info->iter,
                        COLUMN_PRESENCE_ICON, "exit",
                        -1);
  }
  else if (ltm->tm_sec % 3 == 0 && info->cpt > 2) {
    if (presence && strcmp (presence, "unknown"))
      icon = "user-" + std::string(presence);
    gtk_tree_store_set (GTK_TREE_STORE (info->model), info->iter,
                        COLUMN_PRESENCE_ICON, icon.c_str (),
                        -1);
    return FALSE;
  }

  info->cpt++;
  return TRUE;
}

static void status_icon_info_delete (gpointer data)
{
  g_return_if_fail (data != NULL);
  StatusIconInfo *info = (StatusIconInfo*) data;

  gtk_tree_iter_free (info->iter);

  delete info;
}

/* Implementation of the helpers */
static void
on_clicked_fold (RosterViewGtk* self,
		 GtkTreePath* path,
		 const gchar* name)
{
  gboolean row_expanded = TRUE;
  GSList* existing_group = NULL;

  row_expanded
    = gtk_tree_view_row_expanded (GTK_TREE_VIEW (self->priv->tree_view), path);

  existing_group = g_slist_find_custom (self->priv->folded_groups,
					name,
					(GCompareFunc) g_ascii_strcasecmp);
  if (!row_expanded) {

    if (existing_group == NULL) {
      self->priv->folded_groups = g_slist_append (self->priv->folded_groups,
						  g_strdup (name));
    }
  }
  else {

    if (existing_group != NULL) {

      self->priv->folded_groups
        = g_slist_remove_link (self->priv->folded_groups, existing_group);

      g_free ((gchar *) existing_group->data);
      g_slist_free_1 (existing_group);
    }
  }

  /* Update gsettings */
  self->priv->settings->set_slist ("roster-folded-groups", self->priv->folded_groups);
}

static void
on_clicked_trigger_presentity (Ekiga::Presentity* presentity)
{
  Ekiga::TriggerMenuBuilder builder;

  presentity->populate_menu (builder);
}

static void
update_offline_count (RosterViewGtk* self,
		      GtkTreeIter* iter)
{
  GtkTreeModel *model = NULL;
  GtkTreeIter loop_iter;
  gint total = 0;
  gint offline_count = 0;
  gint column_type;
  Ekiga::Presentity* presentity = NULL;
  gchar *name = NULL;
  gchar *name_with_count = NULL;

  model = GTK_TREE_MODEL (self->priv->store);

  if (gtk_tree_model_iter_nth_child (model, &loop_iter, iter, 0)) {

    do {

      gtk_tree_model_get (model, &loop_iter,
                          COLUMN_TYPE, &column_type,
                          COLUMN_PRESENTITY, &presentity,
                          -1);
      if (column_type == TYPE_PRESENTITY
          && (presentity->get_presence () == "offline"
              || presentity->get_presence () == "unknown"))
        offline_count++;
    } while (gtk_tree_model_iter_next (model, &loop_iter));
  }

  total = gtk_tree_model_iter_n_children (model, iter);
  gtk_tree_model_get (model, iter, COLUMN_GROUP_NAME, &name, -1);
  name_with_count = g_strdup_printf ("%s - (%d/%d)", name, total - offline_count, total);
  gtk_tree_store_set (GTK_TREE_STORE (model), iter,
                      COLUMN_NAME, name_with_count, -1);
  g_free (name);
  g_free (name_with_count);
}

static void
show_offline_contacts_changed_cb (GSettings *settings,
				  gchar *key,
				  gpointer data)
{
  RosterViewGtk *self = NULL;
  GtkTreeModel *model = NULL;

  g_return_if_fail (data != NULL);

  self = ROSTER_VIEW_GTK (data);

  self->priv->show_offline_contacts = g_settings_get_boolean (settings, key);

  /* beware: model is filtered here */
  model = gtk_tree_view_get_model (self->priv->tree_view);
  gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (model));

  /* beware: we want the unfiltered model now */
  model = GTK_TREE_MODEL (self->priv->store);

  /* there's an interesting problem there : hiding makes the rows
   * unexpanded... so they don't come back as they should! */
  GtkTreeIter heaps;
  GtkTreePath* path = NULL;
  if (gtk_tree_model_get_iter_first (model, &heaps)) {

    do {

      path = gtk_tree_model_get_path (model, &heaps);
      gtk_tree_view_expand_row (self->priv->tree_view, path, FALSE);
      gtk_tree_path_free (path);

      roster_view_gtk_update_groups (self, &heaps);
    } while (gtk_tree_model_iter_next (model, &heaps));
  }
}


static void
on_selection_changed (GtkTreeSelection* selection,
		      gpointer data)
{
  RosterViewGtk* self = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gint column_type;
  Ekiga::Heap *heap = NULL;
  Ekiga::Presentity *presentity = NULL;
  Ekiga::MenuBuilder builder;

  gchar *name = NULL;
  gchar *group_name = NULL;

  self = ROSTER_VIEW_GTK (data);
  model = gtk_tree_view_get_model (self->priv->tree_view);

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (model, &iter,
                        COLUMN_NAME, &name,
                        COLUMN_GROUP_NAME, &group_name,
                        COLUMN_TYPE, &column_type,
                        COLUMN_HEAP, &heap,
                        COLUMN_PRESENTITY, &presentity,
                        -1);

    /* Reset old data. This also ensures GIO actions are
     * properly removed before adding new ones.
     */
    self->priv->presentity_menu.reset ();
    self->priv->heap_menu.reset ();

    switch (column_type) {

    case TYPE_HEAP:

      if (heap != NULL) {
        self->priv->heap_menu = Ekiga::GActorMenuPtr (new Ekiga::GActorMenu (*heap));
        g_signal_emit (self, signals[ACTIONS_CHANGED_SIGNAL], 0,
                       self->priv->heap_menu->get_model ());
      }
      break;
    case TYPE_GROUP:
        g_signal_emit (self, signals[ACTIONS_CHANGED_SIGNAL], 0, NULL);
      break;
    case TYPE_PRESENTITY:

      if (presentity != NULL) {
        self->priv->presentity_menu = Ekiga::GActorMenuPtr (new Ekiga::GActorMenu (*presentity));
        g_signal_emit (self, signals[ACTIONS_CHANGED_SIGNAL], 0,
                       self->priv->presentity_menu->get_model ());
      }
      break;
    default:

      g_assert_not_reached ();
      break; // shouldn't happen
    }
    g_free (group_name);
    g_free (name);
  }
  else
    g_signal_emit (self, signals[ACTIONS_CHANGED_SIGNAL], 0, NULL);
}

static gint
on_view_event_after (GtkWidget *tree_view,
		     GdkEventButton *event,
		     gpointer data)
{
  RosterViewGtk *self = NULL;
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;

  // take into account only clicks and Enter keys
  if (event->type != GDK_BUTTON_PRESS && event->type != GDK_2BUTTON_PRESS && event->type != GDK_KEY_PRESS)
    return FALSE;
  if (event->type == GDK_KEY_PRESS && ((GdkEventKey*)event)->keyval != GDK_KEY_Return && ((GdkEventKey*)event)->keyval != GDK_KEY_KP_Enter)
    return FALSE;

  self = ROSTER_VIEW_GTK (data);
  model = gtk_tree_view_get_model (self->priv->tree_view);

  // get the line clicked or currently selected
  gboolean ret = true;
  if (event->type == GDK_KEY_PRESS)
    gtk_tree_view_get_cursor (GTK_TREE_VIEW (tree_view), &path, NULL);
  else
    ret = gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (tree_view),
                                         (gint) event->x, (gint) event->y,
                                         &path, NULL, NULL, NULL);

  if (!ret)
    return TRUE;  // click on an empty line

  if (gtk_tree_model_get_iter (model, &iter, path)) {

    gint column_type;
    gchar *name = NULL;
    gchar *group_name = NULL;
    Ekiga::Heap *heap = NULL;
    Ekiga::Presentity *presentity = NULL;
    gtk_tree_model_get (model, &iter,
                        COLUMN_NAME, &name,
                        COLUMN_GROUP_NAME, &group_name,
                        COLUMN_TYPE, &column_type,
                        COLUMN_HEAP, &heap,
                        COLUMN_PRESENTITY, &presentity,
                        -1);

    switch (column_type) {

    case TYPE_HEAP:
      if (event->type == GDK_BUTTON_PRESS && event->button == 1 && name)
        on_clicked_fold (self, path, name);
      else if (event->type == GDK_BUTTON_PRESS && event->button == 3)
        gtk_menu_popup (GTK_MENU (self->priv->heap_menu->get_menu ()),
                        NULL, NULL, NULL, NULL, event->button, event->time);
      break;
    case TYPE_GROUP:
      if (event->type == GDK_BUTTON_PRESS && event->button == 1 && group_name)
        on_clicked_fold (self, path, group_name);
      break;
    case TYPE_PRESENTITY:
      if (event->type == GDK_BUTTON_PRESS && event->button == 3)
        gtk_menu_popup (GTK_MENU (self->priv->presentity_menu->get_menu ()),
                        NULL, NULL, NULL, NULL, event->button, event->time);
      else if (event->type == GDK_2BUTTON_PRESS || event->type == GDK_KEY_PRESS)
        on_clicked_trigger_presentity (presentity);
      break;
    default:

      g_assert_not_reached ();
      break; // shouldn't happen
    }
    g_free (name);
    g_free (group_name);
  }
  gtk_tree_path_free (path);

  return TRUE;
}

static gboolean
presentity_hide_show_offline (RosterViewGtk* self,
			      GtkTreeModel* model,
			      GtkTreeIter* iter)
{
  gboolean result = FALSE;

  if (self->priv->show_offline_contacts)
    result = TRUE;
  else
    gtk_tree_model_get (model, iter,
			COLUMN_OFFLINE, &result,
			-1);

  return result;
}

static gboolean
group_hide_show_offline (RosterViewGtk* self,
                         GtkTreeModel* model,
                         GtkTreeIter* iter)
{
  gboolean result = FALSE;

  if (self->priv->show_offline_contacts)
    result = TRUE;
  else {

    GtkTreeIter child_iter;
    if (gtk_tree_model_iter_nth_child (model, &child_iter, iter, 0)) {

      do {

        result = presentity_hide_show_offline (self, model, &child_iter);
      } while (!result && gtk_tree_model_iter_next (model, &child_iter));
    }
  }

  return result;
}


static gboolean
tree_model_filter_hide_show_offline (GtkTreeModel *model,
				     GtkTreeIter *iter,
				     gpointer data)
{
  gboolean result = FALSE;
  RosterViewGtk *self = NULL;
  gint column_type;

  self = ROSTER_VIEW_GTK (data);

  gtk_tree_model_get (model, iter,
		      COLUMN_TYPE, &column_type,
		      -1);

  switch (column_type) {

  case TYPE_PRESENTITY:

    result = presentity_hide_show_offline (self, model, iter);
    break;

  case TYPE_GROUP:

    result = group_hide_show_offline (self, model, iter);
    break;

  case TYPE_HEAP:
  default:
    result = TRUE;
  }

  return result;
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
                       gpointer /*data*/)
{
  GtkTreePath *path = NULL;
  gint column_type;
  gboolean row_expanded = FALSE;

  path = gtk_tree_model_get_path (model, iter);
  row_expanded = gtk_tree_view_row_expanded (GTK_TREE_VIEW (gtk_tree_view_column_get_tree_view (column)), path);
  gtk_tree_path_free (path);

  gtk_tree_model_get (model, iter, COLUMN_TYPE, &column_type, -1);

  if (column_type == TYPE_PRESENTITY || column_type == TYPE_HEAP)
    g_object_set (renderer, "visible", FALSE, NULL);
  else
    g_object_set (renderer, "visible", TRUE, NULL);

  g_object_set (renderer,
                "is-expanded", row_expanded,
                NULL);
}

static bool
on_visit_clusters (RosterViewGtk* self,
		   Ekiga::ClusterPtr cluster)
{
  on_cluster_added (self, cluster);
  return true;
}

static void
on_cluster_added (RosterViewGtk* self,
		  Ekiga::ClusterPtr cluster)
{
  cluster->visit_heaps (boost::bind (&visit_heaps, self, cluster, _1));
}

static bool
visit_heaps (RosterViewGtk* self,
	     Ekiga::ClusterPtr cluster,
	     Ekiga::HeapPtr heap)
{
  on_heap_updated (self, cluster, heap);
  heap->visit_presentities (boost::bind (&visit_presentities, self, cluster, heap, _1));

  return true;
}

static void
on_heap_added (RosterViewGtk* self,
	       Ekiga::ClusterPtr cluster,
	       Ekiga::HeapPtr heap)
{
  on_heap_updated (self, cluster, heap);
  heap->visit_presentities (boost::bind (&visit_presentities, self, cluster, heap, _1));
}

static void
on_heap_updated (RosterViewGtk* self,
		 G_GNUC_UNUSED Ekiga::ClusterPtr cluster,
		 Ekiga::HeapPtr heap)
{
  GdkRGBA fg_color, bg_color;
  GtkStyleContext *context = NULL;
  gchar *heap_name = NULL;
  GtkTreeIter iter;

  context = gtk_widget_get_style_context (GTK_WIDGET (self->priv->tree_view));

  roster_view_gtk_find_iter_for_heap (self, heap, &iter);
  gtk_style_context_get_color (context, GTK_STATE_FLAG_ACTIVE, &fg_color);
  heap_name = g_strdup_printf ("<span weight=\"bold\" stretch=\"expanded\" variant=\"smallcaps\">%s</span>",
                               heap->get_name ().c_str ());

  gtk_tree_store_set (self->priv->store, &iter,
		      COLUMN_TYPE, TYPE_HEAP,
                      COLUMN_FOREGROUND_COLOR, &fg_color,
		      COLUMN_HEAP, heap.get (),
		      COLUMN_NAME, heap_name, -1);

  g_free (heap_name);
}

static void
on_heap_removed (RosterViewGtk* self,
		 G_GNUC_UNUSED Ekiga::ClusterPtr cluster,
		 Ekiga::HeapPtr heap)
{
  GtkTreeIter iter;
  GtkTreeIter heap_iter;
  GtkTreeIter group_iter;
  guint timeout = 0;

  roster_view_gtk_find_iter_for_heap (self, heap, &heap_iter);

  // Remove all timeout-based effects for the heap presentities
  if (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (self->priv->store),
                                     &group_iter, &heap_iter, 0)) {
    do {
      if (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (self->priv->store),
                                         &iter, &group_iter, 0)) {
        do {
          gtk_tree_model_get (GTK_TREE_MODEL (self->priv->store), &iter,
                              COLUMN_TIMEOUT, &timeout,
                              -1);
          if (timeout > 0)
            g_source_remove (timeout);
        } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (self->priv->store), &iter));
      }
    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (self->priv->store), &group_iter));
  }

  gtk_tree_store_remove (self->priv->store, &heap_iter);
}


static bool
visit_presentities (RosterViewGtk* self,
		    Ekiga::ClusterPtr cluster,
		    Ekiga::HeapPtr heap,
		    Ekiga::PresentityPtr presentity)
{
  on_presentity_added (self, cluster, heap, presentity);

  return true;
}

static void
on_presentity_added (RosterViewGtk* self,
		     G_GNUC_UNUSED Ekiga::ClusterPtr cluster,
		     Ekiga::HeapPtr heap,
		     Ekiga::PresentityPtr presentity)
{
  GdkRGBA color;
  GdkPixbuf *pixbuf = NULL;
  GtkTreeIter heap_iter;
  std::list<std::string> groups = presentity->get_groups ();
  GtkTreeSelection* selection = gtk_tree_view_get_selection (self->priv->tree_view);
  GtkTreeModelFilter* filtered_model = GTK_TREE_MODEL_FILTER (gtk_tree_view_get_model (self->priv->tree_view));
  GtkTreeIter group_iter;
  GtkTreeIter iter;
  GtkTreeIter filtered_iter;
  bool active = false;
  bool away = false;
  guint timeout = 0;
  gchar *old_presence = NULL;
  gboolean should_emit = FALSE;

  roster_view_gtk_find_iter_for_heap (self, heap, &heap_iter);

  active = presentity->get_presence () != "offline";
  away = presentity->get_presence () == "away";

  if (groups.empty ())
    groups.push_back (_("Unsorted"));

  for (std::list<std::string>::const_iterator group = groups.begin ();
       group != groups.end ();
       group++) {

    roster_view_gtk_find_iter_for_group (self, heap, &heap_iter,
					 *group, &group_iter);
    roster_view_gtk_find_iter_for_presentity (self, &group_iter, presentity, &iter);

    if (gtk_tree_store_iter_is_valid (self->priv->store, &iter)
        && gtk_tree_store_iter_is_valid (self->priv->store, &filtered_iter)
        && gtk_tree_model_filter_convert_child_iter_to_iter (filtered_model, &filtered_iter, &iter))
      if (gtk_tree_selection_iter_is_selected (selection, &filtered_iter))
	should_emit = TRUE;

    // Find out what our presence was
    gtk_tree_model_get (GTK_TREE_MODEL (self->priv->store), &iter,
                        COLUMN_PRESENCE, &old_presence, -1);

    if (old_presence && presentity->get_presence () != old_presence
        && presentity->get_presence () != "unknown" && presentity->get_presence () != "offline"
        && (!g_strcmp0 (old_presence, "unknown") || !g_strcmp0 (old_presence, "offline"))) {

      StatusIconInfo *info = new StatusIconInfo ();
      info->model = GTK_TREE_MODEL (self->priv->store);
      info->iter = gtk_tree_iter_copy (&iter);
      info->cpt = 0;

      timeout = g_timeout_add_seconds_full (G_PRIORITY_DEFAULT, 1, roster_view_gtk_icon_blink_cb,
                                            (gpointer) info, (GDestroyNotify) status_icon_info_delete);
      gtk_tree_store_set (self->priv->store, &iter,
                          COLUMN_TIMEOUT, timeout, -1);
    }
    else {

      std::string icon = "user-offline";
      if (!old_presence) {
        gtk_tree_store_set (self->priv->store, &iter,
                            COLUMN_PRESENCE_ICON, icon.c_str (),
                            -1);
      }
      else if (old_presence != presentity->get_presence ()) {
        if (presentity->get_presence () != "unknown")
          icon = "user-" + presentity->get_presence ();
        gtk_tree_store_set (self->priv->store, &iter,
                            COLUMN_PRESENCE_ICON, icon.c_str (),
                            -1);
      }
    }

    gtk_style_context_get_color (gtk_widget_get_style_context (GTK_WIDGET (self->priv->tree_view)),
                                 (!active||away)?GTK_STATE_FLAG_INSENSITIVE:GTK_STATE_FLAG_NORMAL,
                                 &color);

    pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
                                       "avatar-default-symbolic",
                                       48,
                                       GTK_ICON_LOOKUP_GENERIC_FALLBACK,
                                       NULL);

    gtk_tree_store_set (self->priv->store, &iter,
                        COLUMN_TYPE, TYPE_PRESENTITY,
                        COLUMN_OFFLINE, active,
                        COLUMN_HEAP, heap.get (),
                        COLUMN_PRESENTITY, presentity.get (),
                        COLUMN_NAME, presentity->get_name ().c_str (),
                        COLUMN_STATUS, presentity->get_status ().c_str (),
                        COLUMN_PRESENCE, presentity->get_presence ().c_str (),
                        COLUMN_AVATAR_PIXBUF, pixbuf,
                        COLUMN_FOREGROUND_COLOR, &color, -1);
    gtk_tree_model_get (GTK_TREE_MODEL (self->priv->store), &iter,
                        COLUMN_TIMEOUT, &timeout,
                        -1);

    g_free (old_presence);
    g_object_unref (pixbuf);
  }

  GtkTreeModel* model = gtk_tree_view_get_model (self->priv->tree_view);
  gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (model));

  roster_view_gtk_update_groups (self, &heap_iter);

  if (should_emit)
    g_signal_emit (self, signals[ACTIONS_CHANGED_SIGNAL], 0);
}


static void
on_presentity_updated (RosterViewGtk* self,
		       Ekiga::ClusterPtr cluster,
		       Ekiga::HeapPtr heap,
		       Ekiga::PresentityPtr presentity)
{
  GtkTreeModel *model;
  GtkTreeIter heap_iter;
  GtkTreeIter group_iter;
  GtkTreeIter iter;
  gchar *group_name = NULL;
  int timeout = 0;
  std::list<std::string> groups = presentity->get_groups ();

  model = GTK_TREE_MODEL (self->priv->store);

  if (groups.empty ())
    groups.push_back (_("Unsorted"));

  // This makes sure we are in all groups where we should
  on_presentity_added (self, cluster, heap, presentity);

  // Now let's remove from all the others
  roster_view_gtk_find_iter_for_heap (self, heap, &heap_iter);

  if (gtk_tree_model_iter_nth_child (model, &group_iter, &heap_iter, 0)) {

    do {

      gtk_tree_model_get (model, &group_iter,
                          COLUMN_GROUP_NAME, &group_name,
                          -1);
      if (group_name != NULL) {

        if (std::find (groups.begin (), groups.end (), group_name) == groups.end ()) {

          roster_view_gtk_find_iter_for_presentity (self, &group_iter, presentity, &iter);
          gtk_tree_model_get (GTK_TREE_MODEL (self->priv->store), &iter,
                              COLUMN_TIMEOUT, &timeout,
                              -1);
          if (timeout > 0)
            g_source_remove (timeout);
          gtk_tree_store_remove (self->priv->store, &iter);
        }
        g_free (group_name);
      }
    } while (gtk_tree_model_iter_next (model, &group_iter));
  }

  model = gtk_tree_view_get_model (self->priv->tree_view);
  gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (model));

  roster_view_gtk_update_groups (self, &heap_iter);
}


static void
on_presentity_removed (RosterViewGtk* self,
		       G_GNUC_UNUSED Ekiga::ClusterPtr cluster,
		       Ekiga::HeapPtr heap,
		       Ekiga::PresentityPtr presentity)
{
  GtkTreeModel *model = NULL;
  GtkTreeIter heap_iter;
  GtkTreeIter group_iter;
  GtkTreeIter iter;
  int timeout = 0;

  roster_view_gtk_find_iter_for_heap (self, heap, &heap_iter);
  model = GTK_TREE_MODEL (self->priv->store);

  if (gtk_tree_model_iter_nth_child (model, &group_iter, &heap_iter, 0)) {

    do {

      roster_view_gtk_find_iter_for_presentity (self, &group_iter, presentity, &iter);
      gtk_tree_model_get (GTK_TREE_MODEL (self->priv->store), &iter,
                          COLUMN_TIMEOUT, &timeout,
                          -1);
      if (timeout > 0)
        g_source_remove (timeout);
      gtk_tree_store_remove (self->priv->store, &iter);
    } while (gtk_tree_model_iter_next (model, &group_iter));
  }

  roster_view_gtk_update_groups (self, &heap_iter);
}

static bool
on_handle_questions (RosterViewGtk* self,
		     Ekiga::FormRequestPtr request)
{
  GtkWidget *parent = gtk_widget_get_toplevel (GTK_WIDGET (self));
  FormDialog dialog (request, parent);

  dialog.run ();

  return true;
}


/*
 * Implementation of the static helpers.
 */
static void
roster_view_gtk_find_iter_for_heap (RosterViewGtk *view,
                                    Ekiga::HeapPtr heap,
                                    GtkTreeIter *iter)
{
  GtkTreeModel *model = NULL;
  Ekiga::Heap *iter_heap = NULL;
  gboolean found = FALSE;

  model = GTK_TREE_MODEL (view->priv->store);

  if (gtk_tree_model_get_iter_first (model, iter)) {

    do {

      gtk_tree_model_get (model, iter, COLUMN_HEAP, &iter_heap, -1);
      if (iter_heap == heap.get ())
        found = TRUE;
    } while (!found && gtk_tree_model_iter_next (model, iter));
  }

  if (!found)
    gtk_tree_store_append (view->priv->store, iter, NULL);
}


static void
roster_view_gtk_find_iter_for_group (RosterViewGtk *view,
				     Ekiga::HeapPtr heap,
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

      gtk_tree_model_get (model, iter, COLUMN_GROUP_NAME, &group_name, -1);
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
                        COLUMN_HEAP, heap.get (),
                        COLUMN_NAME, name.c_str (),
                        COLUMN_GROUP_NAME, name.c_str (),
                        -1);
  }
}


static void
roster_view_gtk_find_iter_for_presentity (RosterViewGtk *view,
                                          GtkTreeIter *group_iter,
                                          Ekiga::PresentityPtr presentity,
                                          GtkTreeIter *iter)
{
  GtkTreeModel *model = NULL;
  Ekiga::Presentity *iter_presentity = NULL;
  gboolean found = FALSE;

  model = GTK_TREE_MODEL (view->priv->store);

  if (gtk_tree_model_iter_nth_child (model, iter, group_iter, 0)) {

    do {

      gtk_tree_model_get (model, iter, COLUMN_PRESENTITY, &iter_presentity, -1);
      if (iter_presentity == presentity.get ())
        found = TRUE;
    } while (!found && gtk_tree_model_iter_next (model, iter));
  }

  if (!found)
    gtk_tree_store_append (view->priv->store, iter, group_iter);
}


static void
roster_view_gtk_update_groups (RosterViewGtk *view,
                               GtkTreeIter *heap_iter)
{
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;

  GSList *existing_group = NULL;

  int timeout = 0;
  gboolean go_on = FALSE;
  gchar *name = NULL;

  model = GTK_TREE_MODEL (view->priv->store);

  if (gtk_tree_model_iter_nth_child (model, &iter, heap_iter, 0)) {

    do {

      // If this node has children, see if it must be
      // folded or unfolded
      if (gtk_tree_model_iter_has_child (model, &iter)) {

        update_offline_count (view, &iter);
        gtk_tree_model_get (model, &iter,
                            COLUMN_GROUP_NAME, &name, -1);
        if (name) {

          if (view->priv->folded_groups)
            existing_group = g_slist_find_custom (view->priv->folded_groups,
                                                  name,
                                                  (GCompareFunc) g_ascii_strcasecmp);

          path = gtk_tree_model_get_path (model, heap_iter);
          gtk_tree_view_expand_row (view->priv->tree_view, path, FALSE);
          gtk_tree_path_free (path);

          path = gtk_tree_model_get_path (model, &iter);
          if (path) {

            if (existing_group == NULL) {
              if (!gtk_tree_view_row_expanded (view->priv->tree_view, path)) {
                gtk_tree_view_expand_row (view->priv->tree_view, path, TRUE);
              }
            }
            else {
              if (gtk_tree_view_row_expanded (view->priv->tree_view, path)) {
                gtk_tree_view_collapse_row (view->priv->tree_view, path);
              }
            }

            gtk_tree_path_free (path);
          }

          go_on = gtk_tree_model_iter_next (model, &iter);
        }

        g_free (name);
      }
      // else remove the node (no children)
      else {

        gtk_tree_model_get (GTK_TREE_MODEL (view->priv->store), &iter,
                            COLUMN_TIMEOUT, &timeout,
                            -1);
        go_on = gtk_tree_store_remove (view->priv->store, &iter);
      }
    } while (go_on);
  }
}

/*
 * GObject stuff
 */

static void
roster_view_gtk_finalize (GObject *obj)
{
  RosterViewGtk *view = NULL;

  view = (RosterViewGtk *)obj;

  delete view->priv->settings;

  g_slist_foreach (view->priv->folded_groups, (GFunc) g_free, NULL);
  g_slist_free (view->priv->folded_groups);
  view->priv->folded_groups = NULL;
  delete view->priv;

  G_OBJECT_CLASS (roster_view_gtk_parent_class)->finalize (obj);
}

static void
roster_view_gtk_init (RosterViewGtk* self)
{
  GtkWidget *scrolled_window;
  GtkWidget *vbox = NULL;
  GtkTreeModel *filtered = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeViewColumn *col = NULL;
  GtkCellRenderer *renderer = NULL;

  self->priv = new RosterViewGtkPrivate;

  self->priv->settings = new Ekiga::Settings (CONTACTS_SCHEMA);
  self->priv->folded_groups = self->priv->settings->get_slist ("roster-folded-groups");
  self->priv->show_offline_contacts = self->priv->settings->get_bool ("show-offline-contacts");
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 0);
  gtk_frame_set_shadow_type (GTK_FRAME (self), GTK_SHADOW_NONE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  self->priv->store = gtk_tree_store_new (COLUMN_NUMBER,
                                          G_TYPE_INT,         // type
                                          G_TYPE_POINTER,     // heap
                                          G_TYPE_POINTER,     // presentity
                                          G_TYPE_STRING,      // name
                                          G_TYPE_STRING,      // status
                                          G_TYPE_STRING,      // presence
                                          GDK_TYPE_PIXBUF,    // Avatar
                                          GDK_TYPE_RGBA,      // cell foreground color
                                          GDK_TYPE_RGBA,      // cell background color
                                          G_TYPE_STRING,      // group name (invisible)
                                          G_TYPE_STRING,      // presence
					  G_TYPE_BOOLEAN,     // offline
                                          G_TYPE_INT);        // timeout source

  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (self->priv->store),
                                        COLUMN_NAME, GTK_SORT_ASCENDING);
  filtered = gtk_tree_model_filter_new (GTK_TREE_MODEL (self->priv->store),
					NULL);
  g_object_unref (self->priv->store);
  self->priv->tree_view =
    GTK_TREE_VIEW (gtk_tree_view_new_with_model (filtered));
  g_object_unref (filtered);
  gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (filtered),
					  tree_model_filter_hide_show_offline,
					  self, NULL);

  gtk_tree_view_set_headers_visible (self->priv->tree_view, FALSE);
  gtk_tree_view_set_grid_lines (self->priv->tree_view, GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);

  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (vbox));
  gtk_box_pack_start (GTK_BOX (vbox),
		      GTK_WIDGET (scrolled_window), TRUE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (scrolled_window),
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
  renderer = gm_cell_renderer_expander_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  g_object_set (renderer,
                "xalign", 0.0,
                "xpad", 0,
                "ypad", 0,
                "visible", TRUE,
                "is-expander", TRUE,
                "is-expanded", FALSE,
                NULL);
  gtk_tree_view_column_add_attribute (col, renderer, "cell-background-rgba", COLUMN_BACKGROUND_COLOR);
  gtk_tree_view_column_set_cell_data_func (col, renderer, expand_cell_data_func, NULL, NULL);
  gtk_tree_view_append_column (self->priv->tree_view, col);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_set_spacing (col, 0);
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "markup", COLUMN_NAME);
  gtk_tree_view_column_add_attribute (col, renderer, "foreground-rgba", COLUMN_FOREGROUND_COLOR);
  gtk_tree_view_column_add_attribute (col, renderer, "cell-background-rgba", COLUMN_BACKGROUND_COLOR);
  gtk_tree_view_column_set_alignment (col, 0.0);
  g_object_set (renderer,
                "xalign", 0.5, "ypad", 0, NULL);
  gtk_tree_view_column_set_cell_data_func (col, renderer,
                                           show_cell_data_func, GINT_TO_POINTER (TYPE_HEAP), NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer,
				      "text", COLUMN_NAME);
  gtk_tree_view_column_add_attribute (col, renderer, "foreground-rgba", COLUMN_FOREGROUND_COLOR);
  gtk_tree_view_column_add_attribute (col, renderer, "cell-background-rgba", COLUMN_BACKGROUND_COLOR);
  g_object_set (renderer, "weight", PANGO_WEIGHT_BOLD, NULL);
  gtk_tree_view_column_set_cell_data_func (col, renderer,
                                           show_cell_data_func, GINT_TO_POINTER (TYPE_GROUP), NULL);

  renderer = gtk_cell_renderer_pixbuf_new ();
  g_object_set (renderer, "yalign", 0.5, "xpad", 6, "stock-size", 1, NULL);
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_add_attribute (col, renderer,
				      "icon-name",
				      COLUMN_PRESENCE_ICON);
  gtk_tree_view_column_add_attribute (col, renderer, "cell-background-rgba", COLUMN_BACKGROUND_COLOR);
  gtk_tree_view_column_set_cell_data_func (col, renderer,
                                           show_cell_data_func, GINT_TO_POINTER (TYPE_PRESENTITY), NULL);

  renderer = gm_cell_renderer_bitext_new ();
  g_object_set (renderer, "ellipsize", PANGO_ELLIPSIZE_END, "width-chars", 30, NULL);
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_add_attribute (col, renderer, "primary-text", COLUMN_NAME);
  gtk_tree_view_column_add_attribute (col, renderer, "secondary-text", COLUMN_STATUS);
  gtk_tree_view_column_add_attribute (col, renderer, "foreground-rgba", COLUMN_FOREGROUND_COLOR);
  gtk_tree_view_column_add_attribute (col, renderer, "cell-background-rgba", COLUMN_BACKGROUND_COLOR);
  gtk_tree_view_column_set_cell_data_func (col, renderer,
                                           show_cell_data_func, GINT_TO_POINTER (TYPE_PRESENTITY), NULL);

  renderer = gtk_cell_renderer_pixbuf_new ();
  g_object_set (renderer, "yalign", 1.0, "ypad", 3, "xpad", 6, NULL);
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_add_attribute (col, renderer,
				      "pixbuf",
				      COLUMN_AVATAR_PIXBUF);
  gtk_tree_view_column_add_attribute (col, renderer, "cell-background-rgba", COLUMN_BACKGROUND_COLOR);
  gtk_tree_view_column_set_cell_data_func (col, renderer,
                                           show_cell_data_func, GINT_TO_POINTER (TYPE_PRESENTITY), NULL);

  /* Callback when the selection has been changed */
  selection = gtk_tree_view_get_selection (self->priv->tree_view);
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
  g_signal_connect (selection, "changed",
		    G_CALLBACK (on_selection_changed), self);
  g_signal_connect (self->priv->tree_view, "event-after",
		    G_CALLBACK (on_view_event_after), self);

  g_signal_connect (self->priv->settings->get_g_settings (), "changed::show-offline-contacts",
		    G_CALLBACK (&show_offline_contacts_changed_cb), self);
}

static void
roster_view_gtk_class_init (RosterViewGtkClass* klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = roster_view_gtk_finalize;

  signals[ACTIONS_CHANGED_SIGNAL] =
    g_signal_new ("actions-changed",
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (RosterViewGtkClass, selection_changed),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__OBJECT,
		  G_TYPE_NONE, 1, G_TYPE_MENU_MODEL);
}

/*
 * Public API
 */
GtkWidget *
roster_view_gtk_new (boost::shared_ptr<Ekiga::PresenceCore> core)
{
  RosterViewGtk* self = NULL;
  boost::signals2::connection conn;

  self = (RosterViewGtk *) g_object_new (ROSTER_VIEW_GTK_TYPE, NULL);

  conn = core->cluster_added.connect (boost::bind (&on_cluster_added, self, _1));
  self->priv->connections.add (conn);
  conn = core->heap_added.connect (boost::bind (&on_heap_added, self, _1, _2));
  self->priv->connections.add (conn);
  conn = core->heap_updated.connect (boost::bind (&on_heap_updated, self, _1, _2));
  self->priv->connections.add (conn);
  conn = core->heap_removed.connect (boost::bind (&on_heap_removed, self, _1, _2));
  self->priv->connections.add (conn);
  conn = core->presentity_added.connect (boost::bind (&on_presentity_added, self, _1, _2, _3));
  self->priv->connections.add (conn);
  conn = core->presentity_updated.connect (boost::bind (&on_presentity_updated, self, _1, _2, _3));
  self->priv->connections.add (conn);
  conn = core->presentity_removed.connect (boost::bind (&on_presentity_removed, self, _1, _2, _3));
  self->priv->connections.add (conn);
  conn = core->questions.connect (boost::bind (&on_handle_questions, self, _1));
  self->priv->connections.add (conn);

  core->visit_clusters (boost::bind (&on_visit_clusters, self, _1));

  return (GtkWidget *) self;
}
