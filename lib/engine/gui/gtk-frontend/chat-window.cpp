
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
#include "chat-area.h"

struct _ChatWindowPrivate
{
  _ChatWindowPrivate (Ekiga::ChatCore& core_): core(core_)
  {}

  Ekiga::ChatCore& core;
  sigc::connection dialect_added_connection;
  std::list<sigc::connection> simple_chat_added_connections;

  GtkWidget* notebook;
};

static GObjectClass* parent_class = NULL;

/* signal callbacks (declarations) */

static bool on_dialect_added (ChatWindow* self,
			      Ekiga::Dialect& dialect);
static bool on_simple_chat_added (ChatWindow* self,
				  bool on_user_request,
				  Ekiga::SimpleChat &chat);

/* signal callbacks (implementations) */

static bool
on_dialect_added (ChatWindow* self,
		  Ekiga::Dialect& dialect)
{
  self->priv->simple_chat_added_connections.push_front (dialect.simple_chat_added.connect (sigc::hide_return (sigc::bind<0> (sigc::ptr_fun (on_simple_chat_added), self))));

  dialect.visit_simple_chats (sigc::bind<0> (sigc::bind<0> (sigc::ptr_fun (on_simple_chat_added), self), false));

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

  page = chat_area_new (chat);
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
  ChatWindow* self = NULL;

  self = (ChatWindow*)instance;

  // hmmm...
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

  result->priv->dialect_added_connection = core.dialect_added.connect (sigc::hide_return(sigc::bind<0>(sigc::ptr_fun (on_dialect_added), result)));
  core.visit_dialects (sigc::bind<0>(sigc::ptr_fun (on_dialect_added), result));

  return (GtkWidget*)result;
}
