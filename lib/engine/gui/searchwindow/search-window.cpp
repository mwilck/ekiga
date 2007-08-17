
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras
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
 *                         search-window.cpp  -  description
 *                         ---------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of a gtk+ interface on ContactCore
 *
 */

#include <iostream>
#include <map>

#include "gmstockicons.h"

#include "search-window.h"
#include "book-view-gtk.h"
#include "menu-builder-gtk.h"

/* 
 * The signals centralizer relays signals from the Core and Book to
 * the GObject and dies with it.
 */
class SignalCentralizer: public sigc::trackable
{
public:

    /* Watch the ContactCore for changes, and
     * watch the Source objects and their Books
     */
    void watch_core (Ekiga::ContactCore *core);

    /* Signals emitted by this centralizer and interesting
     * for our GObject 
     */
    sigc::signal<void> core_updated;
    sigc::signal<void, Ekiga::Book &> book_added;
    sigc::signal<void, Ekiga::Book &> book_removed;
    sigc::signal<void, Ekiga::Book &> book_updated;

private:

    void on_source_added (Ekiga::Source &source);

    void watch_source (Ekiga::Source &source);
};


void SignalCentralizer::watch_core (Ekiga::ContactCore *core)
{
  core->updated.connect (core_updated.make_slot ());

  // Trigger on_source_added when a Source is added
  core->source_added.connect (sigc::mem_fun (this, &SignalCentralizer::on_source_added));

  // Trigger on_source_added for all Sources of the Ekiga::ContactCore
  core->visit_sources (sigc::mem_fun (this, &SignalCentralizer::on_source_added));
}


void SignalCentralizer::on_source_added (Ekiga::Source &source)
{
  // Emit the book_added signal for all Books of the Ekiga::Source
  source.visit_books (sigc::mem_fun (book_added, &sigc::signal<void, Ekiga::Book &>::emit));

  // Connect all signals of the SignalCentralizer to the corresponding Ekiga::Source signals
  watch_source (source);
}


void SignalCentralizer::watch_source (Ekiga::Source &source)
{
  source.book_added.connect (book_added.make_slot ());
  source.book_updated.connect (book_updated.make_slot ());
  source.book_removed.connect (book_removed.make_slot ());
}


/* 
 * The Search Window 
 */
struct _SearchWindowPrivate
{
  _SearchWindowPrivate (Ekiga::ContactCore *_core):core (_core) { }

  Ekiga::ContactCore *core;
  SignalCentralizer centralizer;
  GtkTreeStore *store;
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


static GObjectClass *parent_class = NULL;


/*
 * Callbacks
 */

/* DESCRIPTION  : Called when the ContactCore is updated, ie the core_updated
 *                signal has been emitted. 
 * BEHAVIOR     : Update the SearchWindow main menu with new functions.
 * PRE          : The given GtkWidget pointer must be an SearchBook GObject.
 */
static void on_core_updated (gpointer data);


/* DESCRIPTION  : Called when a Book has been added to the ContactCore,
 *                ie the book_added signal has been emitted.
 * BEHAVIOR     : Add a view of the Book in the SearchWindow.
 * PRE          : The given GtkWidget pointer must be an SearchBook GObject.
 */
static void on_book_added (Ekiga::Book &book,
			   gpointer data);


/* DESCRIPTION  : Called when a Book has been removed from the ContactCore,
 *                ie the book_removed signal has been emitted.
 * BEHAVIOR     : Remove the view of the Book from the SearchWindow.
 * PRE          : The given GtkWidget pointer must be an SearchBook GObject.
 */
static void on_book_removed (Ekiga::Book &book,
			     gpointer data);


/* DESCRIPTION  : Called when a Book has been updated,
 *                ie the book_updated signal has been emitted.
 * BEHAVIOR     : Update the Book in the SearchWindow.
 * PRE          : The given GtkWidget pointer must be an SearchBook GObject.
 */
static void on_book_updated (Ekiga::Book &book,
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
 * BEHAVIOR     : Add a view of the given Book in the SearchWindow.
 * PRE          : /
 */
static void search_window_add_book (SearchWindow * self,
                                    Ekiga::Book &book);


/* DESCRIPTION  : /
 * BEHAVIOR     : Update the Book description of the given Book 
 *                in the SearchWindow.
 * PRE          : /
 */
static void search_window_update_book (SearchWindow *self,
                                       Ekiga::Book &book);


/* DESCRIPTION  : /
 * BEHAVIOR     : Remove the given Book from the SearchWindow.
 * PRE          : /
 */
static void search_window_remove_book (SearchWindow *self,
                                       Ekiga::Book &book);


/* DESCRIPTION  : /
 * BEHAVIOR     : Find the GtkTreeIter for the given Book
 *                in the SearchWindow.
 *                Return TRUE if iter is valid and corresponds
 *                to the given Book, FALSE otherwise.
 * PRE          : /
 */
static gboolean find_iter_for_book (SearchWindow *search_window,
				    Ekiga::Book &book,
				    GtkTreeIter *iter);


/* Implementation of the callbacks */
static void
on_core_updated (gpointer data)
{
  SearchWindow *self = NULL;
  
  GtkWidget *item = NULL;

  MenuBuilderGtk menu_builder;

  self = (SearchWindow *) data;

  self->priv->core->populate_menu (menu_builder);

  item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_builder.menu), item);

  item = gtk_image_menu_item_new_from_stock (GTK_STOCK_CLOSE, self->priv->accel);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_builder.menu), item);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (self->priv->menu_item_core),
			     menu_builder.menu);
  
  gtk_widget_show_all (menu_builder.menu);
}


static void
on_book_added (Ekiga::Book &book,
	       gpointer data)
{
  search_window_add_book (SEARCH_WINDOW (data), book);
}


static void
on_book_removed (Ekiga::Book &book,
		 gpointer data)
{
  search_window_remove_book (SEARCH_WINDOW (data), book);
}


static void
on_book_updated (Ekiga::Book &book,
		 gpointer data)
{
  search_window_update_book (SEARCH_WINDOW (data), book);
}


static void
on_view_updated (BookViewGtk *view,
		 gpointer data)
{
  SearchWindow *self = NULL;
  GtkWidget *menu = NULL;

  self = SEARCH_WINDOW (data);

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
  SearchWindow *self = NULL;

  self = SEARCH_WINDOW (data);

  on_book_selection_changed (self->priv->selection, self);
}


static void
on_book_selection_changed (GtkTreeSelection *selection,
			   gpointer data)
{
  SearchWindow *self = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  GtkWidget *view = NULL;
  gint page = -1;
  GtkWidget *menu = NULL;
  GtkWidget *item = NULL;

  self = SEARCH_WINDOW (data);

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (model, &iter, COLUMN_VIEW, &view, -1);
    page = gtk_notebook_page_num (GTK_NOTEBOOK (self->priv->notebook), view);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (self->priv-> notebook), page);

    menu = gtk_menu_new ();
    item = gtk_menu_item_new_with_label ("Selected book");
    gtk_widget_set_sensitive (GTK_WIDGET (item), FALSE);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    item = gtk_separator_menu_item_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    book_view_gtk_populate_menu (BOOK_VIEW_GTK (view), menu);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (self->priv-> menu_item_view),
			       menu);
    gtk_widget_show_all (menu);
    gtk_widget_set_sensitive (self->priv->menu_item_view, TRUE);
    g_object_unref (view);
  } 
  else {

    gtk_widget_set_sensitive (self->priv->menu_item_view, FALSE);
    gtk_menu_item_remove_submenu (GTK_MENU_ITEM (self->priv->menu_item_view));
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

	model = GTK_TREE_MODEL (((SearchWindow *)data)->priv->store);
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
	    g_signal_connect (G_OBJECT (menu_builder.menu), "hide",
			      GTK_SIGNAL_FUNC (g_object_unref),
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
search_window_add_book (SearchWindow *self,
                        Ekiga::Book &book)
{
  GtkTreeIter iter;
  GtkWidget *view = NULL;
  GdkPixbuf *book_icon = NULL;
  gint page = -1;

  view = book_view_gtk_new (book);

  page = gtk_notebook_append_page (GTK_NOTEBOOK (self->priv->notebook),
				   view, NULL);

  if (GTK_WIDGET_VISIBLE (self))
    gtk_widget_show_all (view);

  g_signal_connect (view, "updated", G_CALLBACK (on_view_updated), self);

  book_icon =  gtk_widget_render_icon (view,
                                       GM_STOCK_LOCAL_CONTACT,
                                       GTK_ICON_SIZE_MENU, NULL);

  gtk_tree_store_append (self->priv->store, &iter, NULL);
  gtk_tree_store_set (self->priv->store, &iter,
                      COLUMN_PIXBUF, book_icon,
		      COLUMN_NAME, book.get_name ().c_str (),
		      COLUMN_BOOK_POINTER, &book, COLUMN_VIEW, view,
		      -1);
  gtk_tree_selection_select_iter (self->priv->selection, &iter);
}


static void
search_window_update_book (SearchWindow *self,
                           Ekiga::Book &book)
{
  GtkTreeIter iter;

  if (find_iter_for_book (self, book, &iter))
    gtk_tree_store_set (self->priv->store, &iter,
			COLUMN_NAME, book.get_name ().c_str (),
			-1);
}


static void
search_window_remove_book (SearchWindow *self,
                           Ekiga::Book &book)
{
  GtkTreeIter iter;
  gint page = -1;
  GtkWidget *view = NULL;

  gtk_notebook_set_current_page (GTK_NOTEBOOK (self->priv->notebook), 0);
  gtk_widget_set_sensitive (self->priv->menu_item_view, FALSE);
  gtk_menu_item_remove_submenu (GTK_MENU_ITEM (self->priv->menu_item_view));

  while (find_iter_for_book (self, book, &iter)) {

    gtk_tree_model_get (GTK_TREE_MODEL (self->priv->store), &iter,
                        COLUMN_VIEW, &view,
                        -1);

    g_signal_handlers_disconnect_matched (view,
                                          (GSignalMatchType)G_SIGNAL_MATCH_DATA,
                                          0, /* signal_id */
                                          (GQuark) 0, /* detail */
                                          NULL,	/* closure */
                                          NULL,	/* func */
                                          self); /* data */
    gtk_tree_store_remove (self->priv->store, &iter);
    page = gtk_notebook_page_num (GTK_NOTEBOOK (self->priv-> notebook), view);
    g_object_unref (view);
    if (page > 0)
      gtk_notebook_remove_page (GTK_NOTEBOOK (self->priv->notebook), page);
  }
}


static gboolean
find_iter_for_book (SearchWindow *self,
		    Ekiga::Book &book,
		    GtkTreeIter *iter)
{
  Ekiga::Book *book_iter = NULL;

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (self->priv->store),
                                     iter)) {

    while (gtk_tree_store_iter_is_valid (self->priv->store, iter)) {

      gtk_tree_model_get (GTK_TREE_MODEL (self->priv->store), iter,
                          COLUMN_BOOK_POINTER, &book_iter,
                          -1);

      if (&book == book_iter)
        break;

      gtk_tree_model_iter_next (GTK_TREE_MODEL (self->priv->store),
                                iter);
    }

    return gtk_tree_store_iter_is_valid (self->priv->store, iter);
  } 

  return FALSE;
}


/* Implementation of the GObject stuff */
static void
search_window_dispose (GObject *obj)
{
  SearchWindow *self = NULL;

  self = SEARCH_WINDOW (obj);

  if (self->priv->store) {

    gtk_tree_store_clear (self->priv->store);
    g_object_unref (self->priv->store);
    self->priv->store = NULL;
  }

  if (self->priv->menu_item_view) {

    g_object_unref (self->priv->menu_item_view);
    self->priv->menu_item_view = NULL;
  }

  if (self->priv->menu_item_core) {

    g_object_unref (self->priv->menu_item_core);
    self->priv->menu_item_core = NULL;
  }

  parent_class->dispose (obj);
}


static void
search_window_finalize (GObject *obj)
{
  SearchWindow *self = NULL;

  self = SEARCH_WINDOW (obj);

  delete self->priv;
}


static void
search_window_class_init (SearchWindowClass *klass)
{
  GObjectClass *gobject_class = NULL;

  parent_class = (GObjectClass *) g_type_class_peek_parent (klass);

  gobject_class = (GObjectClass *) klass;
  gobject_class->dispose = search_window_dispose;
  gobject_class->finalize = search_window_finalize;
}


GType
search_window_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (SearchWindowClass),
      NULL,
      NULL,
      (GClassInitFunc) search_window_class_init,
      NULL,
      NULL,
      sizeof (SearchWindow),
      0,
      NULL,
      NULL
    };

    result = g_type_register_static (GM_WINDOW_TYPE,
				     "SearchWindowType",
				     &info, (GTypeFlags) 0);
  }

  return result;
}


/*
 * Public API 
 */
GtkWidget *
search_window_new (Ekiga::ContactCore *core,
                   std::string title)
{
  SearchWindow *self = NULL;

  GtkWidget *menu_bar = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *hpaned = NULL;
  GtkWidget *view = NULL;
  GtkWidget *label = NULL;

  GtkCellRenderer *cell = NULL;
  GtkTreeViewColumn *column = NULL;

  self = (SearchWindow *) g_object_new (SEARCH_WINDOW_TYPE, NULL);
  self->priv = new SearchWindowPrivate (core);

  /* Start building the window */
  vbox = gtk_vbox_new (FALSE, 2);
  gtk_window_set_title (GTK_WINDOW (self), title.c_str ());

  /* The menu */
  menu_bar = gtk_menu_bar_new ();

  self->priv->accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (self), self->priv->accel);

  self->priv->menu_item_core = 
    gtk_menu_item_new_with_mnemonic ("_Search Window");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar),
			 self->priv->menu_item_core);
  g_object_ref (self->priv->menu_item_core);
  self->priv->centralizer.core_updated.connect (sigc::bind (sigc::ptr_fun (on_core_updated), (gpointer) self));
  on_core_updated (self); // This will add static and dynamic actions

  self->priv->menu_item_view = gtk_menu_item_new_with_label ("Book");
  gtk_widget_set_sensitive (self->priv->menu_item_view, FALSE);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar),
			 self->priv->menu_item_view);
  g_object_ref (self->priv->menu_item_view);

  gtk_container_add (GTK_CONTAINER (vbox), menu_bar);
  gtk_box_set_child_packing (GTK_BOX (vbox), menu_bar,
			     FALSE, FALSE, 2, GTK_PACK_START);
  gtk_container_add (GTK_CONTAINER (self), vbox);


  /* A hpaned to put the list of Books and their content */
  hpaned = gtk_hpaned_new ();
  gtk_container_set_border_width (GTK_CONTAINER (hpaned), 6);
  gtk_container_add (GTK_CONTAINER (vbox), hpaned);

  /* The store listing the Books */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  self->priv->store = gtk_tree_store_new (NUM_COLUMNS,
                                          GDK_TYPE_PIXBUF,
                                          G_TYPE_STRING,
                                          G_TYPE_POINTER,
                                          G_TYPE_OBJECT);
  view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (self->priv->store));
  g_object_ref (self->priv->store);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (view), FALSE);
  gtk_container_add (GTK_CONTAINER (frame), view);
  gtk_widget_set_size_request (GTK_WIDGET (view), 125, -1);
  gtk_paned_add1 (GTK_PANED (hpaned), frame);

  /* Two renderers for one column */
  column = gtk_tree_view_column_new ();
  cell = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_set_attributes (column, cell,
                                       "pixbuf", COLUMN_PIXBUF,
                                       NULL);

  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_set_attributes (column, cell,
                                       "text", COLUMN_NAME,
                                       NULL);

  gtk_tree_view_column_set_sort_column_id (column, COLUMN_NAME);
  gtk_tree_view_column_set_min_width (GTK_TREE_VIEW_COLUMN (column), 125);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
                                   GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (column), true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (view),
                               GTK_TREE_VIEW_COLUMN (column));


  self->priv->selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
  gtk_tree_selection_set_mode (GTK_TREE_SELECTION (self->priv->selection),
			       GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (self->priv->selection), "changed",
		    G_CALLBACK (on_book_selection_changed), self);
  g_signal_connect (G_OBJECT (view), "event-after",
		    G_CALLBACK (on_book_clicked), self);

  /* The notebook containing the books */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  self->priv->notebook = gtk_notebook_new ();
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (self->priv->notebook), FALSE);
  g_signal_connect (self->priv->notebook, "realize",
		    G_CALLBACK (on_notebook_realize), self);
  gtk_container_add (GTK_CONTAINER (frame), self->priv->notebook);
  gtk_paned_add2 (GTK_PANED (hpaned), frame);

  self->priv->centralizer.book_updated.connect (sigc::bind (sigc::ptr_fun (on_book_updated), (gpointer) self));
  self->priv->centralizer.book_added.connect (sigc::bind (sigc::ptr_fun (on_book_added), (gpointer) self));
  self->priv->centralizer.book_removed.connect (sigc::bind (sigc::ptr_fun (on_book_removed), (gpointer) self));

  self->priv->centralizer.watch_core (core);

  return GTK_WIDGET (self);
}
