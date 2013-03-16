
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
 *
 * This program is free software; you can  redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ekiga is licensed under the GPL license and as a special exception, you
 * have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OPAL, OpenH323 and PWLIB
 * programs, as long as you do follow the requirements of the GNU GPL for all
 * the rest of the software thus combined.
 */

/*
 *                         addressbook-window.cpp  -  description
 *                         ---------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of a gtk+ interface on ContactCore
 *
 */

#include <iostream>
#include <map>
#include <vector>
#include <glib/gi18n.h>

#include "gmstockicons.h"

#include "addressbook-window.h"
#include "book-view-gtk.h"
#include "menu-builder-gtk.h"
#include "form-dialog-gtk.h"
#include "scoped-connections.h"

/* 
 * The Search Window 
 */
struct _AddressBookWindowPrivate
{
  _AddressBookWindowPrivate (Ekiga::ContactCore & _core):core (_core) { }

  Ekiga::ContactCore & core;
  Ekiga::scoped_connections connections;
  GtkWidget *tree_view;
  GtkWidget *notebook;
  GtkTreeSelection *selection;
  GtkWidget *menu_item_core;
  GtkWidget *menu_item_view;
  GtkAccelGroup *accel;
};

enum {

  COLUMN_PIXBUF,
  COLUMN_NAME,
  COLUMN_BOOK_POINTER,
  COLUMN_VIEW,
  NUM_COLUMNS
};

enum {

  DIALOG_COLUMN_NAME,
  DIALOG_COLUMN_SOURCE_POINTER
};

G_DEFINE_TYPE (AddressBookWindow, addressbook_window, GM_TYPE_WINDOW);

/*
 * Callbacks
 */

/* DESCRIPTION  : Called when the ContactCore is updated, ie the core_updated
 *                signal has been emitted. 
 * BEHAVIOR     : Update the AddressBookWindow main menu with new functions.
 * PRE          : The given GtkWidget pointer must be an SearchBook GObject.
 */
static void on_core_updated (gpointer data);


/* DESCRIPTION  : Called at startup to populate the window
 * BEHAVIOR     :
 * PRE          : The given GtkWidget pointer must be an addressbook window.
 */
static bool on_visit_sources (Ekiga::SourcePtr source,
			      gpointer data);

/* DESCRIPTION  : Called at startup to populate the window
 * BEHAVIOR     : 
 * PRE          : The given GtkWidget pointer must be an SearchBook GObject.
 */
static void on_source_added (Ekiga::SourcePtr source,
			     gpointer data);


/* DESCRIPTION  : Called at startup to populate the window
 * BEHAVIOR     : 
 * PRE          : The given GtkWidget pointer must be an SearchBook GObject.
 */
static bool visit_books (Ekiga::BookPtr book,
			 Ekiga::SourcePtr source,
			 gpointer data);


/* DESCRIPTION  : Called when a Book has been added to the ContactCore,
 *                ie the book_added signal has been emitted.
 * BEHAVIOR     : Add a view of the Book in the AddressBookWindow.
 * PRE          : The given GtkWidget pointer must be an SearchBook GObject.
 */
static void on_book_added (Ekiga::SourcePtr source,
			   Ekiga::BookPtr book,
                           gpointer data);


/* DESCRIPTION  : Called when a Book has been removed from the ContactCore,
 *                ie the book_removed signal has been emitted.
 * BEHAVIOR     : Remove the view of the Book from the AddressBookWindow.
 * PRE          : The given GtkWidget pointer must be an SearchBook GObject.
 */
static void on_book_removed (Ekiga::SourcePtr source,
			     Ekiga::BookPtr book,
                             gpointer data);


/* DESCRIPTION  : Called when a Book has been updated,
 *                ie the book_updated signal has been emitted.
 * BEHAVIOR     : Update the Book in the AddressBookWindow.
 * PRE          : The given GtkWidget pointer must be an SearchBook GObject.
 */
static void on_book_updated (Ekiga::SourcePtr source,
			     Ekiga::BookPtr book,
                             gpointer data);

/* DESCRIPTION  : Called when the ContactCore has a form request
 * BEHAVIOR     : Runs the form request in gtk+
 * PRE          : The given pointer is the parent window for the form.
 */
static bool on_handle_questions (Ekiga::FormRequestPtr request,
				 gpointer data);

/* DESCRIPTION  : Called when a view of a Book has been updated,
 *                ie the updated signal has been emitted by the GtkWidget.
 * BEHAVIOR     : Update the menu.
 * PRE          : The given GtkWidget pointer must be an SearchBook GObject.
 */
static void on_view_updated (BookViewGtk *view,
                             gpointer data);


/* DESCRIPTION  : Called when the notebook has been realized.
 * BEHAVIOR     : Calls on_book_selection_changed.
 * PRE          : /
 */
static void on_notebook_realize (GtkWidget *notebook,
                                 gpointer data);


/* DESCRIPTION  : Called when the user has selected another Book.
 * BEHAVIOR     : Updates the general menu. 
 * PRE          : /
 */
static void on_book_selection_changed (GtkTreeSelection *selection,
                                       gpointer data);


/* DESCRIPTION  : Called when the user right-clicks on a Book.
 * BEHAVIOR     : Popups the menu with the actions supported by that Book.
 * PRE          : /
 */
static gint on_book_clicked (GtkWidget *tree_view,
                             GdkEventButton *event,
                             gpointer data);

/*
 * Private functions
 */

/* DESCRIPTION  : /
 * BEHAVIOR     : Add a view of the given Book in the AddressBookWindow.
 * PRE          : /
 */
static void addressbook_window_add_book (AddressBookWindow * self,
                                         Ekiga::BookPtr book);


/* DESCRIPTION  : /
 * BEHAVIOR     : Update the Book description of the given Book 
 *                in the AddressBookWindow.
 * PRE          : /
 */
static void addressbook_window_update_book (AddressBookWindow *self,
                                            Ekiga::BookPtr book);


/* DESCRIPTION  : /
 * BEHAVIOR     : Remove the given Book from the AddressBookWindow.
 * PRE          : /
 */
static void addressbook_window_remove_book (AddressBookWindow *self,
                                            Ekiga::BookPtr book);


/* DESCRIPTION  : /
 * BEHAVIOR     : Find the GtkTreeIter for the given Book
 *                in the AddressBookWindow.
 *                Return TRUE if iter is valid and corresponds
 *                to the given Book, FALSE otherwise.
 * PRE          : /
 */
static gboolean find_iter_for_book (AddressBookWindow *addressbook_window,
                                    Ekiga::BookPtr book,
                                    GtkTreeIter *iter);


/* Implementation of the callbacks */
static void
on_core_updated (gpointer data)
{
  AddressBookWindow *self = NULL;

  GtkWidget *item = NULL;

  MenuBuilderGtk menu_builder;

  self = (AddressBookWindow *) data;

  if (self->priv->core.populate_menu (menu_builder)) {

    item = gtk_separator_menu_item_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL (menu_builder.menu), item);
  }

  item = gtk_image_menu_item_new_from_stock (GTK_STOCK_CLOSE, self->priv->accel);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_builder.menu), item);
  g_signal_connect_swapped (item, "activate",
                            G_CALLBACK (gtk_widget_hide),
                            (gpointer) self);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (self->priv->menu_item_core),
                             menu_builder.menu);

  gtk_widget_show_all (menu_builder.menu);
}

static bool
on_visit_sources (Ekiga::SourcePtr source,
		  gpointer data)
{
  on_source_added (source, data);
  return true;
}

static void
on_source_added (Ekiga::SourcePtr source,
		 gpointer data)
{
  source->visit_books (boost::bind (&visit_books, _1, source, data));
}


static bool visit_books (Ekiga::BookPtr book,
			 Ekiga::SourcePtr source,
			 gpointer data)
{
  on_book_added (source, book, data);

  return true;
}


static void
on_book_added (Ekiga::SourcePtr /*source*/,
	       Ekiga::BookPtr book,
               gpointer data)
{
  addressbook_window_add_book (ADDRESSBOOK_WINDOW (data), book);
}


static void
on_book_removed (Ekiga::SourcePtr /*source*/,
		 Ekiga::BookPtr book,
                 gpointer data)
{
  addressbook_window_remove_book (ADDRESSBOOK_WINDOW (data), book);
}


static void
on_book_updated (Ekiga::SourcePtr /*source*/,
		 Ekiga::BookPtr book,
                 gpointer data)
{
  addressbook_window_update_book (ADDRESSBOOK_WINDOW (data), book);
}

static bool
on_handle_questions (Ekiga::FormRequestPtr request,
		     gpointer data)
{
  FormDialog dialog (request, GTK_WIDGET (data));

  dialog.run ();

  return true;
}

static void
on_view_updated (BookViewGtk *view,
                 gpointer data)
{
  AddressBookWindow *self = NULL;
  GtkWidget *menu = NULL;

  self = ADDRESSBOOK_WINDOW (data);

  menu = gtk_menu_new ();
  book_view_gtk_populate_menu (view, menu);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (self->priv->menu_item_view), menu);
  gtk_widget_show_all (menu);
  gtk_widget_set_sensitive (self->priv->menu_item_view, TRUE);
}


static void
on_notebook_realize (GtkWidget * /*notebook*/,
                     gpointer data)
{
  AddressBookWindow *self = NULL;

  self = ADDRESSBOOK_WINDOW (data);

  on_book_selection_changed (self->priv->selection, self);
}


static void
on_book_selection_changed (GtkTreeSelection *selection,
                           gpointer data)
{
  AddressBookWindow *self = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  GtkWidget *view = NULL;
  gint page = -1;
  GtkWidget *menu = NULL;

  self = ADDRESSBOOK_WINDOW (data);

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (model, &iter, COLUMN_VIEW, &view, -1);
    page = gtk_notebook_page_num (GTK_NOTEBOOK (self->priv->notebook), view);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (self->priv-> notebook), page);

    menu = gtk_menu_new ();
    book_view_gtk_populate_menu (BOOK_VIEW_GTK (view), menu);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (self->priv-> menu_item_view),
                               menu);
    gtk_widget_show_all (menu);
    gtk_widget_set_sensitive (self->priv->menu_item_view, TRUE);
    g_object_unref (view);
  } 
  else {

    gtk_widget_set_sensitive (self->priv->menu_item_view, FALSE);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (self->priv->menu_item_view), NULL);
  }
}

static gint
on_book_clicked (GtkWidget *tree_view,
                 GdkEventButton *event,
                 gpointer data)
{
  GtkTreePath *path = NULL;
  GtkTreeIter iter;
  GtkTreeModel *model = NULL;

  Ekiga::Book *book_iter = NULL;

  if (event->type == GDK_BUTTON_PRESS || event->type == GDK_KEY_PRESS) {

    if (event->button == 3) {

      if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (tree_view),
                                         (gint) event->x, (gint) event->y,
                                         &path, NULL, NULL, NULL)) {

        model = gtk_tree_view_get_model (GTK_TREE_VIEW (((AddressBookWindow *) data)->priv->tree_view));
        if (gtk_tree_model_get_iter (model, &iter, path)) {

          MenuBuilderGtk menu_builder;

          gtk_tree_model_get (model, &iter,
                              COLUMN_BOOK_POINTER, &book_iter,
                              -1);

          book_iter->populate_menu (menu_builder);
          if (!menu_builder.empty ()) {

            gtk_widget_show_all (menu_builder.menu);
            gtk_menu_popup (GTK_MENU (menu_builder.menu),
                            NULL, NULL, NULL, NULL,
                            event->button, event->time);
            g_signal_connect (menu_builder.menu, "hide",
                              G_CALLBACK (g_object_unref),
                              (gpointer) menu_builder.menu);
          }
          g_object_ref_sink (G_OBJECT (menu_builder.menu));
        }
        gtk_tree_path_free (path);
      }
    }
  }

  return TRUE;
}


/* Implementation of the private functions */
static void
addressbook_window_add_book (AddressBookWindow *self,
                             Ekiga::BookPtr book)
{
  GtkTreeIter iter;
  GtkTreeModel *store = NULL;
  GtkWidget *view = NULL;

  view = book_view_gtk_new (book);

  gtk_notebook_append_page (GTK_NOTEBOOK (self->priv->notebook),
			    view, NULL);

  if (gtk_widget_get_visible (GTK_WIDGET (self)))
    gtk_widget_show_all (view);

  g_signal_connect (view, "updated", G_CALLBACK (on_view_updated), self);

  store = gtk_tree_view_get_model (GTK_TREE_VIEW (self->priv->tree_view));
  gtk_tree_store_append (GTK_TREE_STORE (store), &iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (store), &iter,
                      COLUMN_NAME, book->get_name ().c_str (),
                      COLUMN_BOOK_POINTER, book.get (),
                      COLUMN_VIEW, view,
                      COLUMN_PIXBUF, book->get_icon ().c_str (),
                      -1);

  if (!gtk_tree_selection_get_selected (self->priv->selection, &store, &iter)) {

    gtk_tree_model_get_iter_first (store, &iter);
    gtk_tree_selection_select_iter (self->priv->selection, &iter);
  }
}


static void
addressbook_window_update_book (AddressBookWindow *self,
                                Ekiga::BookPtr book)
{
  GtkTreeIter iter;
  GtkTreeModel *store = NULL;

  store = gtk_tree_view_get_model (GTK_TREE_VIEW (self->priv->tree_view));
  if (find_iter_for_book (self, book, &iter))
    gtk_tree_store_set (GTK_TREE_STORE (store), &iter,
                        COLUMN_NAME, book->get_name ().c_str (),
                        -1);
}


static void
addressbook_window_remove_book (AddressBookWindow *self,
                                Ekiga::BookPtr book)
{
  GtkTreeIter iter;
  gint page = -1;
  GtkWidget *view = NULL;
  GtkTreeModel *store = NULL;

  gtk_notebook_set_current_page (GTK_NOTEBOOK (self->priv->notebook), 0);
  gtk_widget_set_sensitive (self->priv->menu_item_view, FALSE);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (self->priv->menu_item_view), NULL);

  store = gtk_tree_view_get_model (GTK_TREE_VIEW (self->priv->tree_view));

  while (find_iter_for_book (self, book, &iter)) {

    gtk_tree_model_get (store, &iter,
                        COLUMN_VIEW, &view,
                        -1);

    g_signal_handlers_disconnect_matched (view,
                                          (GSignalMatchType)G_SIGNAL_MATCH_DATA,
                                          0, /* signal_id */
                                          (GQuark) 0, /* detail */
                                          NULL,	/* closure */
                                          NULL,	/* func */
                                          self); /* data */
    gtk_tree_store_remove (GTK_TREE_STORE (store), &iter);
    page = gtk_notebook_page_num (GTK_NOTEBOOK (self->priv-> notebook), view);
    g_object_unref (view);
    if (page > 0)
      gtk_notebook_remove_page (GTK_NOTEBOOK (self->priv->notebook), page);
  }
}


static gboolean
find_iter_for_book (AddressBookWindow *self,
                    Ekiga::BookPtr book,
                    GtkTreeIter *iter)
{
  Ekiga::Book *book_iter = NULL;
  GtkTreeModel *store = NULL;

  store = gtk_tree_view_get_model (GTK_TREE_VIEW (self->priv->tree_view));

  if (gtk_tree_model_get_iter_first (store, iter)) {

    while (gtk_tree_store_iter_is_valid (GTK_TREE_STORE (store), iter)) {

      gtk_tree_model_get (store, iter,
                          COLUMN_BOOK_POINTER, &book_iter,
                          -1);

      if (book.get () == book_iter) {

        break;
      }

      if (!gtk_tree_model_iter_next (store, iter))
        return FALSE;
    }

    return gtk_tree_store_iter_is_valid (GTK_TREE_STORE (store), iter);
  } 

  return FALSE;
}


/* Implementation of the GObject stuff */
static void
addressbook_window_dispose (GObject *obj)
{
  AddressBookWindow *self = ADDRESSBOOK_WINDOW (obj);

  if (self->priv->menu_item_view) {

    g_object_unref (self->priv->menu_item_view);
    self->priv->menu_item_view = NULL;
  }

  if (self->priv->menu_item_core) {

    g_object_unref (self->priv->menu_item_core);
    self->priv->menu_item_core = NULL;
  }

  G_OBJECT_CLASS (addressbook_window_parent_class)->dispose (obj);
}


static void
addressbook_window_finalize (GObject *obj)
{
  AddressBookWindow *self = ADDRESSBOOK_WINDOW (obj);

  delete self->priv;

  G_OBJECT_CLASS (addressbook_window_parent_class)->finalize (obj);
}

static void
addressbook_window_init (G_GNUC_UNUSED AddressBookWindow* self)
{
  /* can't do anything here... we're waiting for a core :-/ */
}

static void
addressbook_window_class_init (AddressBookWindowClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose = addressbook_window_dispose;
  gobject_class->finalize = addressbook_window_finalize;
}

/*
 * Public API 
 */
GtkWidget *
addressbook_window_new (Ekiga::ContactCore &core)
{
  AddressBookWindow *self = NULL;

  boost::signals::connection conn;

  GtkWidget *menu_bar = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *hpaned = NULL;

  GtkCellRenderer *cell = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkTreeStore *store = NULL;

  self = (AddressBookWindow *) g_object_new (ADDRESSBOOK_WINDOW_TYPE, NULL);
  self->priv = new AddressBookWindowPrivate (core);

  gtk_window_set_title (GTK_WINDOW (self), _("Address Book"));
  gtk_window_set_position (GTK_WINDOW (self), GTK_WIN_POS_CENTER);

  gtk_window_set_icon_name (GTK_WINDOW (self), "x-office-address-book");

  /* Start building the window */
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);

  /* The menu */
  menu_bar = gtk_menu_bar_new ();

  self->priv->accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (self), self->priv->accel);
  g_object_unref (self->priv->accel);

  self->priv->menu_item_core = 
    gtk_menu_item_new_with_mnemonic (_("Address _Book"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar),
                         self->priv->menu_item_core);
  g_object_ref (self->priv->menu_item_core);
  conn = core.updated.connect (boost::bind (&on_core_updated,
                                           (gpointer) self));
  self->priv->connections.add (conn);
  on_core_updated (self); // This will add static and dynamic actions

  self->priv->menu_item_view = gtk_menu_item_new_with_mnemonic (_("_Action"));
  gtk_widget_set_sensitive (self->priv->menu_item_view, FALSE);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar),
                         self->priv->menu_item_view);
  g_object_ref (self->priv->menu_item_view);

  gtk_container_add (GTK_CONTAINER (vbox), menu_bar);
  gtk_box_set_child_packing (GTK_BOX (vbox), menu_bar,
                             FALSE, FALSE, 2, GTK_PACK_START);
  gtk_container_add (GTK_CONTAINER (self), vbox);


  /* A hpaned to put the list of Books and their content */
  hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_container_set_border_width (GTK_CONTAINER (hpaned), 6);
  gtk_container_add (GTK_CONTAINER (vbox), hpaned);

  /* The store listing the Books */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  store = gtk_tree_store_new (NUM_COLUMNS,
                              G_TYPE_STRING,
                              G_TYPE_STRING,
                              G_TYPE_POINTER,
                              G_TYPE_OBJECT);
  self->priv->tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
  g_object_unref (store);
  gtk_container_add (GTK_CONTAINER (frame), self->priv->tree_view);
  gtk_widget_set_size_request (GTK_WIDGET (self->priv->tree_view), 185, -1);
  gtk_paned_add1 (GTK_PANED (hpaned), frame);

  /* Several renderers for one column */
  column = gtk_tree_view_column_new ();
  cell = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_add_attribute (column, cell,
                                      "icon-name", COLUMN_PIXBUF);

  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_set_attributes (column, cell,
                                       "text", COLUMN_NAME,
                                       NULL);

  gtk_tree_view_column_set_title (column, _("Category"));
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_NAME);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
                                   GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (column), true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (self->priv->tree_view),
                               GTK_TREE_VIEW_COLUMN (column));


  self->priv->selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (self->priv->tree_view));
  gtk_tree_selection_set_mode (GTK_TREE_SELECTION (self->priv->selection),
                               GTK_SELECTION_SINGLE);
  g_signal_connect (self->priv->selection, "changed",
                    G_CALLBACK (on_book_selection_changed), self);
  g_signal_connect (self->priv->tree_view, "event-after",
                    G_CALLBACK (on_book_clicked), self);

  /* The notebook containing the books */
  self->priv->notebook = gtk_notebook_new ();
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (self->priv->notebook), FALSE);
  g_signal_connect (self->priv->notebook, "realize",
                    G_CALLBACK (on_notebook_realize), self);
  gtk_paned_add2 (GTK_PANED (hpaned), self->priv->notebook);

  conn = core.source_added.connect (boost::bind (&on_source_added, _1, (gpointer) self));
  self->priv->connections.add (conn);

  conn = core.book_updated.connect (boost::bind (&on_book_updated, _1, _2,
                                                (gpointer) self));
  self->priv->connections.add (conn);
  conn = core.book_added.connect (boost::bind (&on_book_added, _1, _2,
                                              (gpointer) self));
  self->priv->connections.add (conn);
  conn =
    core.book_removed.connect (boost::bind (&on_book_removed, _1, _2,
                                           (gpointer) self));
  self->priv->connections.add (conn);

  conn = core.questions.connect (boost::bind (&on_handle_questions, _1, (gpointer) self));
  self->priv->connections.add (conn);

  core.visit_sources (boost::bind (on_visit_sources, _1, (gpointer) self));

  return GTK_WIDGET (self);
}


GtkWidget *
addressbook_window_new_with_key (Ekiga::ContactCore &_core,
                                 const std::string _key)
{
  AddressBookWindow *self = ADDRESSBOOK_WINDOW (addressbook_window_new (_core));
  g_object_set (self, "key", _key.c_str (), NULL);

  return GTK_WIDGET (self);
}

