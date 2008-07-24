
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras
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
 *                         chat-window.h  -  description
 *                         -----------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Implementation of a window to display chats
 *
 */

#include "chat-window.h"
#include "simple-chat-page.h"
#include "multiple-chat-page.h"

struct _ChatWindowPrivate
{
  _ChatWindowPrivate (Ekiga::ChatCore& core_): core(core_)
  {}

  Ekiga::ChatCore& core;
  sigc::connection dialect_added_connection;
  std::list<sigc::connection> simple_chat_added_connections;
  std::list<sigc::connection> multiple_chat_added_connections;

  GtkWidget* notebook;
};

static GObjectClass* parent_class = NULL;

/* signal callbacks (declarations) */

static void on_switch_page (GtkNotebook* notebook,
			    GtkNotebookPage* page,
			    guint num,
			    gpointer data);

static gboolean on_focus_in_event (GtkWidget* widget,
				   GdkEventFocus* event,
				   gpointer data);

static void on_message_notice_event (GtkWidget* page,
				     gpointer data);

static bool on_dialect_added (ChatWindow* self,
			      Ekiga::Dialect& dialect);
static bool on_simple_chat_added (ChatWindow* self,
				  bool on_user_request,
				  Ekiga::SimpleChat &chat);
static bool on_multiple_chat_added (ChatWindow* self,
				    bool on_user_request,
				    Ekiga::MultipleChat &chat);

/* signal callbacks (implementations) */

static void
on_switch_page (G_GNUC_UNUSED GtkNotebook* notebook,
		G_GNUC_UNUSED GtkNotebookPage* page_,
		guint num,
		gpointer data)
{
  ChatWindow* self = (ChatWindow*)data;
  GtkWidget* page = NULL;
  GtkWidget* label = NULL;

  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (self->priv->notebook), num);
  label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (self->priv->notebook),
				      page);
  gtk_label_set_text (GTK_LABEL (label),
		      (const gchar*)g_object_get_data (G_OBJECT (label),
						       "base-title"));
  g_object_set_data (G_OBJECT (label), "unread-count",
		     GUINT_TO_POINTER (0));
}

static gboolean
on_focus_in_event (G_GNUC_UNUSED GtkWidget* widget,
		   G_GNUC_UNUSED GdkEventFocus* event,
		   gpointer data)
{
  ChatWindow* self = (ChatWindow*)data;
  gint num;
  GtkWidget* page = NULL;
  GtkWidget* label = NULL;

  num = gtk_notebook_get_current_page (GTK_NOTEBOOK (self->priv->notebook));
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (self->priv->notebook), num);
  label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (self->priv->notebook),
				      page);
  gtk_label_set_text (GTK_LABEL (label),
		      (const gchar*)g_object_get_data (G_OBJECT (label),
						       "base-title"));
  g_object_set_data (G_OBJECT (label), "unread-count",
		     GUINT_TO_POINTER (0));

  return FALSE;
}

static void
on_message_notice_event (GtkWidget* page,
			 gpointer data)
{
  ChatWindow* self = (ChatWindow*)data;
  gint num = -1;

  for (gint ii = 0;
       ii < gtk_notebook_get_n_pages (GTK_NOTEBOOK (self->priv->notebook)) ;
       ii++) {

    if (page == gtk_notebook_get_nth_page (GTK_NOTEBOOK (self->priv->notebook),
					   ii)) {

      num = ii;
      break;
    }
  }

  if (num
      != gtk_notebook_get_current_page (GTK_NOTEBOOK (self->priv->notebook))) {

    GtkWidget* label = NULL;
    guint unread_count = 0;
    const gchar* base_title = NULL;
    gchar* txt = NULL;

    label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (self->priv->notebook),
					page);
    base_title = (const gchar*)g_object_get_data (G_OBJECT (label),
						  "base-title");
    unread_count = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (label),
							"unread-count"));
    unread_count = unread_count + 1;
    g_object_set_data (G_OBJECT (label), "unread-count",
		       GUINT_TO_POINTER (unread_count));

    txt = g_strdup_printf ("[%d] %s", unread_count, base_title);
    gtk_label_set_text (GTK_LABEL (label), txt);
    g_free (txt);
  }
}

static bool
on_dialect_added (ChatWindow* self,
		  Ekiga::Dialect& dialect)
{
  self->priv->simple_chat_added_connections.push_front (dialect.simple_chat_added.connect (sigc::hide_return (sigc::bind<0> (sigc::ptr_fun (on_simple_chat_added), self))));
  self->priv->multiple_chat_added_connections.push_front (dialect.multiple_chat_added.connect (sigc::hide_return (sigc::bind<0> (sigc::ptr_fun (on_multiple_chat_added), self))));

  dialect.visit_simple_chats (sigc::bind<0> (sigc::bind<0> (sigc::ptr_fun (on_simple_chat_added), self), false));
  dialect.visit_multiple_chats (sigc::bind<0> (sigc::bind<0> (sigc::ptr_fun (on_multiple_chat_added), self), false));

  return true;
}

static bool
on_simple_chat_added (ChatWindow* self,
		      bool on_user_request,
		      Ekiga::SimpleChat &chat)
{
  GtkWidget* page = NULL;
  GtkWidget* label = NULL;
  gint num;

  page = simple_chat_page_new (chat);
  label = gtk_label_new (chat.get_title ().c_str ());

  g_object_set_data_full (G_OBJECT (label), "base-title",
			  g_strdup (chat.get_title ().c_str ()),
			  g_free);

  num = gtk_notebook_append_page (GTK_NOTEBOOK (self->priv->notebook),
				  page, label);
  gtk_widget_show (page);
  g_signal_connect (page, "message-notice-event",
		    G_CALLBACK (on_message_notice_event), self);

  if (on_user_request) {

    gtk_notebook_set_current_page (GTK_NOTEBOOK (self->priv->notebook), num);
    gtk_widget_show (GTK_WIDGET (self));
    gtk_window_present (GTK_WINDOW (self));
  }

  return true;
}

static bool
on_multiple_chat_added (ChatWindow* self,
			bool on_user_request,
			Ekiga::MultipleChat &chat)
{
  GtkWidget* page = NULL;
  GtkWidget* label = NULL;
  gint num;

  page = multiple_chat_page_new (chat);
  label = gtk_label_new (chat.get_title ().c_str ());

  num = gtk_notebook_append_page (GTK_NOTEBOOK (self->priv->notebook),
				  page, label);
  gtk_widget_show_all (page);

  if (on_user_request) {

    gtk_notebook_set_current_page (GTK_NOTEBOOK (self->priv->notebook), num);
    gtk_widget_show (GTK_WIDGET (self));
    gtk_window_present (GTK_WINDOW (self));
  }

  return true;
}

/* GObject code */
static void
chat_window_dispose (GObject* obj)
{
  ChatWindow* self = NULL;

  self = CHAT_WINDOW (obj);

  parent_class->dispose (obj);
}

static void
chat_window_finalize (GObject* obj)
{
  ChatWindow* self = NULL;

  self = CHAT_WINDOW (obj);

  self->priv->dialect_added_connection.disconnect ();

  for (std::list<sigc::connection>::iterator iter
	 = self->priv->simple_chat_added_connections.begin ();
       iter != self->priv->simple_chat_added_connections.end ();
       ++iter)
    iter->disconnect ();

  for (std::list<sigc::connection>::iterator iter
	 = self->priv->multiple_chat_added_connections.begin ();
       iter != self->priv->multiple_chat_added_connections.end ();
       ++iter)
    iter->disconnect ();

  delete self->priv;
  self->priv = NULL;

  parent_class->finalize (obj);
}

static void
chat_window_class_init (gpointer g_class,
			G_GNUC_UNUSED gpointer class_data)
{
  GObjectClass* gobject_class = NULL;

  parent_class = (GObjectClass*) g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass*) g_class;
  gobject_class->dispose = chat_window_dispose;
  gobject_class->finalize = chat_window_finalize;
}

static void
chat_window_init (GTypeInstance* instance,
		  G_GNUC_UNUSED gpointer g_class)
{
  /* we can't do much here since we get the Chat as reference... */
  gtk_window_set_title (GTK_WINDOW (instance), "Chat window");
}

GType
chat_window_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (ChatWindowClass),
      NULL,
      NULL,
      chat_window_class_init,
      NULL,
      NULL,
      sizeof (ChatWindow),
      0,
      chat_window_init,
      NULL
    };

    result = g_type_register_static (GM_WINDOW_TYPE,
				     "ChatWindow",
				     &info, (GTypeFlags) 0);
  }

  return result;
}

/* public api */

GtkWidget*
chat_window_new (Ekiga::ChatCore& core,
		 const std::string key)
{
  ChatWindow* result = NULL;

  result = (ChatWindow*)g_object_new (CHAT_WINDOW_TYPE,
				      "key", key.c_str (),
				      NULL);

  result->priv = new ChatWindowPrivate (core);

  result->priv->notebook = gtk_notebook_new ();
  gtk_container_add (GTK_CONTAINER (result), result->priv->notebook);
  gtk_widget_show (result->priv->notebook);

  g_signal_connect (result, "focus-in-event",
		    G_CALLBACK (on_focus_in_event), result);
  g_signal_connect (result->priv->notebook, "switch-page",
		    G_CALLBACK (on_switch_page), result);

  result->priv->dialect_added_connection = core.dialect_added.connect (sigc::hide_return(sigc::bind<0>(sigc::ptr_fun (on_dialect_added), result)));
  core.visit_dialects (sigc::bind<0>(sigc::ptr_fun (on_dialect_added), result));

  return (GtkWidget*)result;
}
