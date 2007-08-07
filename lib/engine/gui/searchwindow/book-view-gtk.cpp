
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

#include "config.h"

#include <algorithm>
#include <iostream>

#include "book-view-gtk.h"
#include "menu-builder-gtk.h"

/* 
 * The signals centralizer relays signals from the Book to
 * the GObject and dies with it.
 */
class SignalCentralizer: public sigc::trackable
{
public:

    /* Watch the Book for changes, 
     * connect the signals 
     */
    void watch_book (Ekiga::Book &book);

    /* Repopulate the book */
    void repopulate (Ekiga::Book &book);

    /* signals emitted by this centralizer and of interest
     * for our GObject 
     */
    sigc::signal<void, Ekiga::Contact &> contact_added;
    sigc::signal<void, Ekiga::Contact &> contact_updated;
    sigc::signal<void, Ekiga::Contact &> contact_removed;

private:

    void on_contact_added (Ekiga::Contact &contact);
};


void SignalCentralizer::watch_book (Ekiga::Book &book)
{
  book.contact_added.connect (contact_added.make_slot ());
  book.contact_updated.connect (contact_updated.make_slot ());
  book.contact_removed.connect (contact_removed.make_slot ());
  repopulate (book);
}


void SignalCentralizer::repopulate (Ekiga::Book &book)
{
  book.visit_contacts (sigc::mem_fun (this, &SignalCentralizer::on_contact_added));
}


void SignalCentralizer::on_contact_added (Ekiga::Contact &contact)
{
  contact_added.emit (contact);
}


/* 
 * The Book View
 */
struct _BookViewGtkPrivate
{
  _BookViewGtkPrivate (Ekiga::Book &_book) : book (_book) { }

  SignalCentralizer centralizer;
  GtkTreeView *tree_view;
  GtkWidget *vbox;
  GtkWidget *scrolled_window;

  Ekiga::Book &book;
};


enum
{
  COLUMN_CONTACT_POINTER,
  COLUMN_GROUP_NAME,
  COLUMN_NAME,
  COLUMN_VIDEO_URL,
  COLUMN_PHONE,
  COLUMN_NUMBER
};

static GObjectClass *parent_class = NULL;


/*
 * Callbacks
 */


/* DESCRIPTION  : Called when the a contact has been added in a Book.
 * BEHAVIOR     : Update the BookView.
 * PRE          : The gpointer must point to the BookViewGtk GObject.
 */
static void on_contact_added (Ekiga::Contact &contact,
			      gpointer data);


/* DESCRIPTION  : Called when the a contact has been updated in a Book.
 * BEHAVIOR     : Update the BookView.
 * PRE          : The gpointer must point to the BookViewGtk GObject.
 */
static void on_contact_updated (Ekiga::Contact &contact,
				gpointer data);


/* DESCRIPTION  : Called when the a contact has been removed from a Book.
 * BEHAVIOR     : Update the BookView.
 * PRE          : The gpointer must point to the BookViewGtk GObject.
 */
static void on_contact_removed (Ekiga::Contact &contact,
				gpointer data);


/* DESCRIPTION  : Called when the a contact selection has been changed.
 * BEHAVIOR     : Emits the "updated" signal on the GObject passed as
 *                second parameter..
 * PRE          : The gpointer must point to a BookViewGtk GObject.
 */
static void on_selection_changed (GtkTreeSelection * /*selection*/,
                                  gpointer data);


/* DESCRIPTION  : Called when the a contact is clicked.
 * BEHAVIOR     : Displays a popup menu.
 * PRE          : The gpointer must point to a BookViewGtk GObject.
 */
static gint on_contact_clicked (GtkWidget *tree_view,
                                GdkEventButton *event,
                                gpointer data);


/* Static functions */

/* DESCRIPTION  : / 
 * BEHAVIOR     : Add the contact to the BookViewGtk.
 * PRE          : /
 */
static void
book_view_gtk_add_contact (BookViewGtk *self,
                           Ekiga::Contact &contact);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Updated the contact in the BookViewGtk.
 * PRE          : /
 */
static void
book_view_gtk_update_contact (BookViewGtk *self,
                              Ekiga::Contact &contact,
                              GtkTreeIter *iter);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Remove the contact from the BookViewGtk.
 * PRE          : /
 */
static void
book_view_gtk_remove_contact (BookViewGtk *self,
                              Ekiga::Contact &contact);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Return TRUE and update the GtkTreeIter if we found
 *                the iter corresponding to the Contact in the BookViewGtk.
 * PRE          : /
 */
static gboolean
book_view_gtk_find_iter_for_contact (BookViewGtk *view,
                                     Ekiga::Contact &contact,
                                     GtkTreeIter *iter);



/* Implementation of the callbacks */
static void
on_contact_added (Ekiga::Contact &contact,
		  gpointer data)
{
  book_view_gtk_add_contact (BOOK_VIEW_GTK (data), contact);
}


static void
on_contact_updated (Ekiga::Contact &contact,
		    gpointer data)
{
  BookViewGtk *view = NULL;
  GtkTreeIter iter;

  view = BOOK_VIEW_GTK (data);
  
  if (book_view_gtk_find_iter_for_contact (view, contact, &iter)) {
    book_view_gtk_update_contact (view, contact, &iter);
  }
}


static void
on_contact_removed (Ekiga::Contact &contact,
		    gpointer data)
{
  BookViewGtk *view = NULL;

  view = BOOK_VIEW_GTK (data);
  book_view_gtk_remove_contact (view, contact);
}


static void
on_selection_changed (GtkTreeSelection * /*selection*/,
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
	  = gtk_tree_view_get_model (BOOK_VIEW_GTK (data)->priv->tree_view);

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


/* Implementation of the static functions */
static void
book_view_gtk_add_contact (BookViewGtk *self,
                           Ekiga::Contact &contact)
{
  GtkTreeModel *model = NULL;
  GtkListStore *store = NULL;
  GtkTreeIter iter;

  model = gtk_tree_view_get_model (self->priv->tree_view);
  store = GTK_LIST_STORE (model);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter, COLUMN_CONTACT_POINTER, &contact, -1);
  book_view_gtk_update_contact (self, contact, &iter);
}


static void
book_view_gtk_update_contact (BookViewGtk *self,
			      Ekiga::Contact &contact,
			      GtkTreeIter *iter)
{
  GtkListStore *store = NULL;
  std::string phone;

  store = GTK_LIST_STORE (gtk_tree_view_get_model (self->priv->tree_view));
  gtk_list_store_set (store, iter, COLUMN_NAME, contact.get_name ().c_str (), -1);

  std::list < std::pair <std::string, std::string> > uris = contact.get_uris ();

  for (std::list < std::pair <std::string, std::string> >::const_iterator uri = uris.begin () ; 
       uri != uris.end () ;
       uri++) {

    std::string::size_type loc = uri->second.find ("sip:", 0);
    if (loc != std::string::npos) {
      gtk_list_store_set (store, iter, COLUMN_VIDEO_URL, uri->second.c_str (), -1);
    }
    else if (!uri->second.empty ()) {
      if (!phone.empty ())
        phone += ", ";
      phone += uri->second;
    }
  }

  gtk_list_store_set (store, iter, COLUMN_PHONE, phone.c_str (), -1);
}


static void
book_view_gtk_remove_contact (BookViewGtk *self,
                              Ekiga::Contact &contact)
{
  GtkTreeModel *model = NULL;
  GtkListStore *store = NULL;
  GtkTreeIter iter;

  model = gtk_tree_view_get_model (self->priv->tree_view);
  store = GTK_LIST_STORE (model);

  while (book_view_gtk_find_iter_for_contact (self, contact, &iter))
    gtk_list_store_remove (store, &iter);
}


static gboolean
book_view_gtk_find_iter_for_contact (BookViewGtk *view,
                                     Ekiga::Contact &contact,
                                     GtkTreeIter *iter)
{
  GtkTreeModel *model = NULL;
  Ekiga::Contact *iter_contact = NULL;
  gboolean found = FALSE;

  model = gtk_tree_view_get_model (view->priv->tree_view);

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), iter)) {

    do {

      gtk_tree_model_get (model, iter,
                          COLUMN_CONTACT_POINTER, &iter_contact,
                          -1);
      if (iter_contact == &contact)
        found = TRUE;

    } while (!found && gtk_tree_model_iter_next (model, iter));
  }

  return found;
}


/* GObject boilerplate code */
static void
book_view_gtk_dispose (GObject *obj)
{
  BookViewGtk *view = NULL;

  view = BOOK_VIEW_GTK (obj);

  if (view->priv->tree_view) {

    g_signal_handlers_disconnect_matched (gtk_tree_view_get_selection (view->priv->tree_view),
					  (GSignalMatchType) G_SIGNAL_MATCH_DATA,
					  0, /* signal_id */
					  (GQuark) 0, /* detail */
					  NULL,	/* closure */
					  NULL,	/* func */
					  view); /* data */
    gtk_list_store_clear (GTK_LIST_STORE (gtk_tree_view_get_model (view->priv->tree_view)));

    view->priv->tree_view = NULL;
  }

  parent_class->dispose (obj);
}


static void
book_view_gtk_finalize (GObject *obj)
{
  BookViewGtk *view = NULL;

  view = BOOK_VIEW_GTK (obj);

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
  gobject_class->dispose = book_view_gtk_dispose;
  gobject_class->finalize = book_view_gtk_finalize;

  g_signal_new ("updated",
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


/* public methods implementation */
GtkWidget *
book_view_gtk_new (Ekiga::Book &book)
{
  BookViewGtk *result = NULL;
  GtkTreeSelection *selection = NULL;
  GtkListStore *store = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkCellRenderer *renderer = NULL;

  result = (BookViewGtk *)g_object_new (BOOK_VIEW_GTK_TYPE, NULL);

  result->priv = new _BookViewGtkPrivate (book);

  result->priv->vbox = gtk_vbox_new (FALSE, 2);
  result->priv->scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW
				  (result->priv->scrolled_window),
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  result->priv->tree_view = GTK_TREE_VIEW (gtk_tree_view_new ());

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

  store = gtk_list_store_new (COLUMN_NUMBER,
			      G_TYPE_POINTER, 
                              G_TYPE_STRING, 
                              G_TYPE_STRING, 
                              G_TYPE_STRING, 
                              G_TYPE_STRING);

  gtk_tree_view_set_model (result->priv->tree_view, GTK_TREE_MODEL (store));

  /* Name */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Name"),
                                                     renderer,
                                                     "text",
                                                     COLUMN_NAME,
                                                     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_NAME);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
                                   GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (result->priv->tree_view), column);
  g_object_set (G_OBJECT (renderer), "weight", PANGO_WEIGHT_BOLD, NULL);


  /* URI */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("VoIP URI"),
                                                     renderer,
                                                     "text",
                                                     COLUMN_VIDEO_URL,
                                                     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_VIDEO_URL);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
                                   GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (result->priv->tree_view), column);
  g_object_set (G_OBJECT (renderer), "foreground", "blue",
                "underline", TRUE, NULL);

  /* Phone */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Phone",
                                                     renderer,
                                                     "text",
                                                     COLUMN_PHONE,
                                                     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_PHONE);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
                                   GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (result->priv->tree_view), column);
  g_object_set (G_OBJECT (renderer), "foreground", "darkgray", NULL);


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
