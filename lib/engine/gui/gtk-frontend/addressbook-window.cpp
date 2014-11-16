
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

#include <glib/gi18n.h>

#include "ekiga-settings.h"
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
  _AddressBookWindowPrivate (boost::shared_ptr<Ekiga::ContactCore> _core): core(_core)
  { }

  boost::shared_ptr<Ekiga::ContactCore> core;
  Ekiga::scoped_connections connections;
  GtkWidget *tree_view;
  GtkWidget *notebook;
  GtkTreeSelection *selection;
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
 * BEHAVIOR     : Create and return the window GtkHeaderBar.
 * PRE          : /
 */
static GtkWidget *addressbook_window_build_headerbar (AddressBookWindow *self);


/* DESCRIPTION  : /
 * BEHAVIOR     : Create and return the window GtkTreeView.
 * PRE          : /
 */
static GtkWidget *addressbook_window_build_tree_view (AddressBookWindow *self);


/* DESCRIPTION  : /
 * BEHAVIOR     : Create and return the window GtkNotebook.
 * PRE          : /
 */
static GtkWidget *addressbook_window_build_notebook (AddressBookWindow *self);


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

    g_object_unref (view);
  }
  else {

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
static GtkWidget *
addressbook_window_build_headerbar (AddressBookWindow *self)
{
  GtkWidget *image = NULL;
  GtkWidget *button = NULL;

  GtkWidget *headerbar = NULL;

  /* Build it */
  headerbar = gtk_header_bar_new ();
  gtk_header_bar_set_title (GTK_HEADER_BAR (headerbar), _("Address Book"));
  gtk_window_set_titlebar (GTK_WINDOW (self), headerbar);

  /* Pack buttons */
  button = gtk_button_new ();
  image = gtk_image_new_from_icon_name ("call-start-symbolic", GTK_ICON_SIZE_MENU);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_widget_set_tooltip_text (GTK_WIDGET (button),
                               _("Call the selected contact"));
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "win.call");
  gtk_header_bar_pack_start (GTK_HEADER_BAR (headerbar), button);

  button = gtk_menu_button_new ();
  g_object_set (G_OBJECT (button), "use-popover", true, NULL);
  image = gtk_image_new_from_icon_name ("open-menu-symbolic", GTK_ICON_SIZE_MENU);
  gtk_button_set_image (GTK_BUTTON (button), image);
  /*
  gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (button),
                                  G_MENU_MODEL (gtk_builder_get_object (mw->priv->builder, "menubar")));
  */
  gtk_header_bar_pack_end (GTK_HEADER_BAR (headerbar), button);

  gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (headerbar), TRUE);

  return headerbar;
}


static GtkWidget *
addressbook_window_build_tree_view (AddressBookWindow *self)
{
  GtkWidget *tree_view = NULL;

  GtkListStore *store = NULL;

  GtkTreeViewColumn *column = NULL;
  GtkCellRenderer *cell = NULL;


  /* The store listing the Books */
  store = gtk_list_store_new (NUM_COLUMNS,
                              G_TYPE_STRING,
                              G_TYPE_STRING,
                              G_TYPE_POINTER,
                              G_TYPE_OBJECT);
  tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), FALSE);
  g_object_unref (store);

  /* Several renderers for one column */
  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_spacing (column, 0);
  gtk_tree_view_column_set_alignment (column, 0.0);

  cell = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_add_attribute (column, cell,
                                      "icon-name", COLUMN_PIXBUF);
  g_object_set (cell, "xalign", 0.0, "xpad", 6, "stock-size", 1, NULL);

  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_end (column, cell, TRUE);
  gtk_tree_view_column_add_attribute (column, cell,
                                      "text", COLUMN_NAME);
  g_object_set (cell, "ellipsize", PANGO_ELLIPSIZE_END, "width-chars", 30, NULL);

  gtk_tree_view_column_set_sort_column_id (column, COLUMN_NAME);
  gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (column), FALSE);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
                               GTK_TREE_VIEW_COLUMN (column));

  self->priv->selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  gtk_tree_selection_set_mode (GTK_TREE_SELECTION (self->priv->selection),
                               GTK_SELECTION_SINGLE);
  g_signal_connect (self->priv->selection, "changed",
                    G_CALLBACK (on_book_selection_changed), self);
  g_signal_connect (tree_view, "event-after",
                    G_CALLBACK (on_book_clicked), self);

  return tree_view;
}


static GtkWidget *
addressbook_window_build_notebook (AddressBookWindow *self)
{
  GtkWidget *notebook = gtk_notebook_new ();
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), FALSE);

  g_signal_connect (notebook, "realize",
                    G_CALLBACK (on_notebook_realize), self);

  return notebook;
}


static void
addressbook_window_add_book (AddressBookWindow *self,
                             Ekiga::BookPtr book)
{
  GtkTreeIter iter;
  GtkTreeModel *store = NULL;
  GtkWidget *view = NULL;

  view = book_view_gtk_new (book);
  gtk_widget_show_all (view);

  gtk_notebook_append_page (GTK_NOTEBOOK (self->priv->notebook),
			    view, NULL);

  g_signal_connect (view, "updated", G_CALLBACK (on_view_updated), self);

  store = gtk_tree_view_get_model (GTK_TREE_VIEW (self->priv->tree_view));
  gtk_list_store_append (GTK_LIST_STORE (store), &iter);
  gtk_list_store_set (GTK_LIST_STORE (store), &iter,
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
    gtk_list_store_set (GTK_LIST_STORE (store), &iter,
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
    gtk_list_store_remove (GTK_LIST_STORE (store), &iter);
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

    while (gtk_list_store_iter_is_valid (GTK_LIST_STORE (store), iter)) {

      gtk_tree_model_get (store, iter,
                          COLUMN_BOOK_POINTER, &book_iter,
                          -1);

      if (book.get () == book_iter) {

        break;
      }

      if (!gtk_tree_model_iter_next (store, iter))
        return FALSE;
    }

    return gtk_list_store_iter_is_valid (GTK_LIST_STORE (store), iter);
  }

  return FALSE;
}


/* Implementation of the GObject stuff */
static void
addressbook_window_dispose (GObject *obj)
{
  AddressBookWindow *self = ADDRESSBOOK_WINDOW (obj);

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
addressbook_window_new (GmApplication *app)
{
  AddressBookWindow *self = NULL;

  boost::signals2::connection conn;

  GtkWidget *headerbar = NULL;
  GtkWidget *hpaned = NULL;

  Ekiga::ServiceCorePtr core = gm_application_get_core (app);

  self = (AddressBookWindow *) g_object_new (ADDRESSBOOK_WINDOW_TYPE,
                                             "application", GTK_APPLICATION (app),
                                             "key", USER_INTERFACE ".addressbook-window",
                                             "hide_on_delete", FALSE,
                                             "hide_on_esc", FALSE,
                                             NULL);
  boost::shared_ptr<Ekiga::ContactCore> contact_core =
    core->get<Ekiga::ContactCore> ("contact-core");
  self->priv = new AddressBookWindowPrivate (contact_core);

  gtk_window_set_position (GTK_WINDOW (self), GTK_WIN_POS_CENTER);
  gtk_window_set_icon_name (GTK_WINDOW (self), GM_ICON_LOGO);

  /* Accels */
  self->priv->accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (self), self->priv->accel);
  g_object_unref (self->priv->accel);

  /* Start building the window */
  /* Headerbar */
  headerbar = addressbook_window_build_headerbar (self);
  gtk_widget_show_all (GTK_WIDGET (headerbar));

  /* A hpaned to put the list of Books and their content */
  hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_container_add (GTK_CONTAINER (self), hpaned);
  gtk_container_set_border_width (GTK_CONTAINER (hpaned), 0);

  self->priv->tree_view = addressbook_window_build_tree_view (self);
  gtk_widget_show_all (GTK_WIDGET (self->priv->tree_view));
  gtk_paned_pack1 (GTK_PANED (hpaned), self->priv->tree_view, TRUE, TRUE);

  self->priv->notebook = addressbook_window_build_notebook (self);
  gtk_widget_show_all (GTK_WIDGET (self->priv->notebook));
  gtk_paned_pack2 (GTK_PANED (hpaned), self->priv->notebook, TRUE, TRUE);
  gtk_widget_show (GTK_WIDGET (hpaned));


  /* Signals */
  conn = contact_core->source_added.connect (boost::bind (&on_source_added,
                                                          _1, (gpointer) self));
  self->priv->connections.add (conn);

  conn = contact_core->book_updated.connect (boost::bind (&on_book_updated,
                                                          _1, _2, (gpointer) self));
  self->priv->connections.add (conn);

  conn = contact_core->book_added.connect (boost::bind (&on_book_added,
                                                        _1, _2, (gpointer) self));
  self->priv->connections.add (conn);

  conn = contact_core->book_removed.connect (boost::bind (&on_book_removed,
                                                          _1, _2, (gpointer) self));
  self->priv->connections.add (conn);

  conn = contact_core->questions.connect (boost::bind (&on_handle_questions, _1, (gpointer) self));
  self->priv->connections.add (conn);

  contact_core->visit_sources (boost::bind (on_visit_sources, _1, (gpointer) self));

  return GTK_WIDGET (self);
}
