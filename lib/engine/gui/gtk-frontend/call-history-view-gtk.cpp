
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
 *                         call-history-view-gtk.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of a call history view widget
 *
 */

#include <sstream>
#include <iostream>
#include <iomanip>
#include <glib/gi18n.h>

#include "call-history-view-gtk.h"

#include "menu-builder-tools.h"
#include "menu-builder-gtk.h"
#include "gm-cell-renderer-bitext.h"
#include "gmstockicons.h"


struct _CallHistoryViewGtkPrivate
{
  _CallHistoryViewGtkPrivate (boost::shared_ptr<History::Book> book_)
    : book(book_)
  {}

  boost::shared_ptr<History::Book> book;
  GtkTreeView* tree;
  std::vector<boost::signals::connection> connections;
};

/* this is what we put in the view */
enum {
  COLUMN_CONTACT,
  COLUMN_PIXBUF,
  COLUMN_NAME,
  COLUMN_INFO,
  COLUMN_NUMBER
};

/* and this is the list of signals supported */
enum {
  CONTACT_SELECTED_SIGNAL,
  SELECTION_CHANGED_SIGNAL,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GObjectClass* parent_class = NULL;

/* react to a new call being inserted in history */
static void
on_contact_added (Ekiga::ContactPtr contact,
		  GtkListStore *store)
{
  time_t t;
  struct tm *timeinfo = NULL;
  char buffer [80];
  std::stringstream info;
  const gchar *id = NULL;

  boost::shared_ptr<History::Contact> hcontact = boost::dynamic_pointer_cast<History::Contact> (contact);
  GtkTreeIter iter;

  if (hcontact) {

    switch (hcontact->get_type ()) {

    case History::RECEIVED:

      id = GM_STOCK_CALL_RECEIVED;
      break;

    case History::PLACED:

      id = GM_STOCK_CALL_PLACED;
      break;

    case History::MISSED:

      id = GM_STOCK_CALL_MISSED;
      break;

    default:
      id = GTK_STOCK_MISSING_IMAGE;
    }
  }

  t = hcontact->get_call_start ();
  timeinfo = localtime (&t);
  if (timeinfo != NULL) {
    strftime (buffer, 80, "%x %X", timeinfo);
    info << buffer;
    if (!hcontact->get_call_duration ().empty ())
      info << " (" << hcontact->get_call_duration () << ")";
  }
  else
    info << hcontact->get_call_duration ();

  gtk_list_store_prepend (store, &iter);
  gtk_list_store_set (store, &iter,
		      COLUMN_CONTACT, contact.get (),
		      COLUMN_PIXBUF, id,
		      COLUMN_NAME, contact->get_name ().c_str (),
		      COLUMN_INFO, info.str ().c_str (),
		      -1);
}

static bool
on_visit_contacts (Ekiga::ContactPtr contact,
		   GtkListStore *store)
{
  on_contact_added (contact, store);
  return true;
}

/* react to user clicks */
static gint
on_clicked (GtkWidget *tree,
	    GdkEventButton *event,
	    gpointer data)
{
  History::Book *book = NULL;
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;
  Ekiga::Contact *contact = NULL;

  book = (History::Book*)data;
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree));


  if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (tree),
				     (gint) event->x, (gint) event->y,
				     &path, NULL, NULL, NULL)) {

    if (gtk_tree_model_get_iter (model, &iter, path)) {

      gtk_tree_model_get (model, &iter,
			  COLUMN_CONTACT, &contact,
			  -1);


      if (event->type == GDK_BUTTON_PRESS && event->button == 3) {

	MenuBuilderGtk builder;
	if (contact != NULL)
	  contact->populate_menu (builder);
	if (!builder.empty())
	  builder.add_separator ();
	builder.add_action ("clear", _("Clear List"),
			    boost::bind (&History::Book::clear, book));
	gtk_widget_show_all (builder.menu);
	gtk_menu_popup (GTK_MENU (builder.menu), NULL, NULL,
			NULL, NULL, event->button, event->time);
	g_object_ref_sink (builder.menu);
	g_object_unref (builder.menu);
      }
      if (event->type == GDK_2BUTTON_PRESS) {

	if (contact != NULL) {

	  Ekiga::TriggerMenuBuilder builder;

	  contact->populate_menu (builder);
	}
      }

    }
    gtk_tree_path_free (path);
  }

  return TRUE;
}

static void
on_selection_changed (GtkTreeSelection* selection,
		      gpointer data)
{
  CallHistoryViewGtk* self = NULL;
  GtkTreeModel* model = NULL;
  GtkTreeIter iter;

  self = CALL_HISTORY_VIEW_GTK (data);

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    History::Contact* contact = NULL;
    gtk_tree_model_get (model, &iter,
			COLUMN_CONTACT, &contact,
			-1);
    g_signal_emit (self, signals[CONTACT_SELECTED_SIGNAL], 0, contact);
  } else
    g_signal_emit (self, signals[CONTACT_SELECTED_SIGNAL], 0, NULL);

  g_signal_emit (self, signals[SELECTION_CHANGED_SIGNAL], 0);
}

/* GObject stuff */
static void
call_history_view_gtk_dispose (GObject* obj)
{
  CallHistoryViewGtk* view = NULL;

  view = CALL_HISTORY_VIEW_GTK (obj);

  for (std::vector<boost::signals::connection>::iterator iter
	 = view->priv->connections.begin ();
       iter != view->priv->connections.end ();
       iter++)
    iter->disconnect ();

  if (view->priv->tree) {

    GtkTreeSelection* selection = NULL;

    selection = gtk_tree_view_get_selection (view->priv->tree);

    g_signal_handlers_disconnect_matched (selection,
					  (GSignalMatchType) G_SIGNAL_MATCH_DATA,
					  0, /* signal_id */
					  (GQuark) 0, /* detail */
					  NULL,	/* closure */
					  NULL,	/* func */
					  view /* data */);

    g_signal_handlers_disconnect_matched (view->priv->tree,
					  (GSignalMatchType) G_SIGNAL_MATCH_DATA,
					  0, /* signal_id */
					  (GQuark)0, /* detail */
					  NULL, /* closure */
					  NULL, /* func */
					  &(*(view->priv->book)) /* data */);
    view->priv->tree = NULL;
  }

  parent_class->dispose (obj);
}

static void
call_history_view_gtk_finalize (GObject* obj)
{
  CallHistoryViewGtk* view = NULL;

  view = CALL_HISTORY_VIEW_GTK (obj);

  delete view->priv;

  parent_class->finalize (obj);
}

static void
call_history_view_gtk_class_init (gpointer g_class,
				  gpointer /*class_data*/)
{
  CallHistoryViewGtkClass* call_history_view_gtk_class = NULL;
  GObjectClass* gobject_class = NULL;

  parent_class = (GObjectClass*) g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass*)g_class;
  gobject_class->dispose = call_history_view_gtk_dispose;
  gobject_class->finalize = call_history_view_gtk_finalize;

  signals[CONTACT_SELECTED_SIGNAL] =
    g_signal_new ("contact-selected",
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (CallHistoryViewGtkClass, contact_selected),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__POINTER,
		  G_TYPE_NONE, 1,
		  G_TYPE_POINTER);

  signals[SELECTION_CHANGED_SIGNAL] =
    g_signal_new ("contact-selected",
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (CallHistoryViewGtkClass, contact_selected),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  /* FIXME: is it useful? */
  call_history_view_gtk_class = (CallHistoryViewGtkClass*)g_class;
  call_history_view_gtk_class->contact_selected = NULL;
  call_history_view_gtk_class->selection_changed = NULL;
}

GType
call_history_view_gtk_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (CallHistoryViewGtkClass),
      NULL,
      NULL,
      call_history_view_gtk_class_init,
      NULL,
      NULL,
      sizeof (CallHistoryViewGtk),
      0,
      NULL,
      NULL
    };

    result = g_type_register_static (GTK_TYPE_SCROLLED_WINDOW,
				     "CallHistoryViewGtk",
				     &info, (GTypeFlags)0);
  }

  return result;
}

/* public api */

GtkWidget *
call_history_view_gtk_new (boost::shared_ptr<History::Book> book)
{
  CallHistoryViewGtk* self = NULL;

  GtkListStore *store = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkCellRenderer *renderer = NULL;
  GtkTreeSelection *selection = NULL;

  boost::signals::connection conn;

  g_return_val_if_fail (book, (GtkWidget*)NULL);

  self = (CallHistoryViewGtk*)g_object_new (CALL_HISTORY_VIEW_GTK_TYPE, NULL);

  self->priv = new _CallHistoryViewGtkPrivate (book);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (self),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  /* build the store then the tree */
  store = gtk_list_store_new (COLUMN_NUMBER,
			      G_TYPE_POINTER,
			      G_TYPE_STRING,
			      G_TYPE_STRING,
			      G_TYPE_STRING);

  self->priv->tree = (GtkTreeView*)gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
  g_object_unref (store);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (self->priv->tree), FALSE);
  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (self->priv->tree));

  /* one column should be enough for everyone */
  column = gtk_tree_view_column_new ();

  /* show icon */
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_add_attribute (column, renderer,
				      "stock-id", COLUMN_PIXBUF);

  /* show name and text */
  renderer = gm_cell_renderer_bitext_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_add_attribute (column, renderer,
				      "primary-text", COLUMN_NAME);
  gtk_tree_view_column_add_attribute (column, renderer,
				      "secondary-text", COLUMN_INFO);
  gtk_tree_view_append_column (self->priv->tree, column);

  /* react to user clicks */
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (self->priv->tree));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (selection), "changed",
		    G_CALLBACK (on_selection_changed), self);
  g_signal_connect (G_OBJECT (self->priv->tree), "event-after",
		    G_CALLBACK (on_clicked), &(*book));

  /* connect to the signals */
  conn = book->cleared.connect (boost::bind (&gtk_list_store_clear, store));
  self->priv->connections.push_back (conn);
  conn = book->contact_added.connect (boost::bind (&on_contact_added, _1, store));
  self->priv->connections.push_back (conn);

  /* populate */
  book->visit_contacts (boost::bind (&on_visit_contacts, _1, store));

  return (GtkWidget*)self;
}

void
call_history_view_gtk_get_selected (CallHistoryViewGtk* self,
				    History::Contact** contact)
{
  g_return_if_fail (IS_CALL_HISTORY_VIEW_GTK (self) && contact != NULL);

  GtkTreeSelection* selection = NULL;
  GtkTreeModel* model = NULL;
  GtkTreeIter iter;

  selection = gtk_tree_view_get_selection (self->priv->tree);

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (model, &iter,
			COLUMN_CONTACT, contact,
			-1);
  } else
    *contact = NULL;
}
