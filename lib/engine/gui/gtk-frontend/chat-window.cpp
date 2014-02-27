
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
 *                         chat-window.h  -  description
 *                         -----------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Implementation of a window to display chats
 *
 */

#include <sstream>

#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>

#include "chat-core.h"
#include "notification-core.h"

#include "form-dialog-gtk.h"
#include "scoped-connections.h"

#include "chat-window.h"

#include "conversation-page.h"

struct _ChatWindowPrivate
{
  boost::shared_ptr<Ekiga::NotificationCore> notification_core;
  Ekiga::scoped_connections connections;

  GtkWidget* stack;
};

enum {
  UNREAD_COUNT,
  UNREAD_ALERT,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (ChatWindow, chat_window, GM_TYPE_WINDOW);

/* signal callbacks (declarations) */

static bool on_handle_questions (ChatWindow* self,
				 Ekiga::FormRequestPtr request);

static bool on_dialect_added (ChatWindow* self,
			      Ekiga::DialectPtr dialect);
static bool on_conversation_added (ChatWindow* self,
				   Ekiga::ConversationPtr conversation);
static void on_some_conversation_user_requested (ChatWindow* self,
						 const std::string name);

static void show_chat_window_cb (ChatWindow *self);

/* helper (implementation) */

static void
on_updated (ConversationPage* page,
	    gpointer data)
{
  ChatWindow* self = (ChatWindow*)data;
  guint unread_count = 0;

  gtk_container_child_set (GTK_CONTAINER (self->priv->stack), GTK_WIDGET (page),
			   "title", conversation_page_get_title (page),
			   NULL);

  GList* pages = gtk_container_get_children (GTK_CONTAINER (self->priv->stack));

  for (GList* ptr = pages; ptr != NULL; ptr = ptr->next) {

    unread_count += conversation_page_get_unread_count ((ConversationPage*)ptr->data);
  }

  g_list_free (pages);

  g_signal_emit (self, signals[UNREAD_COUNT], 0, unread_count);
  g_signal_emit (self, signals[UNREAD_ALERT], 0, NULL);

  if (unread_count > 0) {
    gchar* info = g_strdup_printf (ngettext ("You have %d unread text message",
					     "You have %d unread text messages",
					     unread_count), unread_count);
    boost::shared_ptr<Ekiga::Notification> notif (new Ekiga::Notification (Ekiga::Notification::Warning, info, "", _("Read"), boost::bind (show_chat_window_cb, self)));
    self->priv->notification_core->push_notification (notif);
    g_free (info);
  }
}

/* signal callbacks (implementations) */

static bool on_handle_questions (ChatWindow* self,
				 Ekiga::FormRequestPtr request)
{
  GtkWidget *parent = gtk_widget_get_toplevel (GTK_WIDGET (self));
  FormDialog dialog (request, parent);

  dialog.run ();

  return true;
}

static bool
on_dialect_added (ChatWindow* self,
		  Ekiga::DialectPtr dialect)
{
  self->priv->connections.add (dialect->conversation_added.connect (boost::bind(&on_conversation_added, self, _1)));

  dialect->visit_conversations (boost::bind (&on_conversation_added, self, _1));

  return true;
}

static bool
on_conversation_added (ChatWindow* self,
		       Ekiga::ConversationPtr conversation)
{
  GtkWidget* page = NULL;
  std::stringstream strstr;
  strstr << static_cast<void*>(conversation.get ());
  std::string name = strstr.str ();

  page = conversation_page_new (conversation);
  g_signal_connect (page, "updated",
		    G_CALLBACK (on_updated), self);

  gtk_stack_add_titled (GTK_STACK (self->priv->stack),
			page, name.c_str (),
			conversation_page_get_title (CONVERSATION_PAGE (page)));
  gtk_widget_show_all (page);

  self->priv->connections.add (conversation->user_requested.connect (boost::bind (&on_some_conversation_user_requested, self, name)));

  return true;
}

static void
on_some_conversation_user_requested (ChatWindow* self,
				     const std::string name)
{
  gtk_stack_set_visible_child_name (GTK_STACK (self->priv->stack), name.c_str ());
  gtk_widget_show (GTK_WIDGET (self));
  gtk_window_present (GTK_WINDOW (self));
}

static void
show_chat_window_cb (ChatWindow *self)
{
  gtk_widget_show (GTK_WIDGET (self));
  gtk_window_present (GTK_WINDOW (self));
}


/* GObject code */

static void
chat_window_finalize (GObject* obj)
{
  ChatWindow* self = NULL;

  self = CHAT_WINDOW (obj);

  delete self->priv;
  self->priv = NULL;

  G_OBJECT_CLASS (chat_window_parent_class)->finalize (obj);
}

static void
chat_window_class_init (ChatWindowClass* klass)
{
  GObjectClass* gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = chat_window_finalize;

  signals[UNREAD_COUNT] =
    g_signal_new ("unread-count",
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (ChatWindowClass, unread_count),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__UINT,
		  G_TYPE_NONE, 1,
		  G_TYPE_UINT);

  signals[UNREAD_ALERT] =
    g_signal_new ("unread-alert",
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (ChatWindowClass, unread_alert),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
}

static void
chat_window_init (ChatWindow* self)
{
  /* we can't do much here since we get the Chat as reference... */
  gtk_window_set_title (GTK_WINDOW (self), _("Chat Window"));
}

/* public api */

GtkWidget*
chat_window_new (Ekiga::ServiceCore& core,
		 const char* key)
{
  ChatWindow* self = NULL;
  GtkWidget* box = NULL;

  self = (ChatWindow*)g_object_new (CHAT_WINDOW_TYPE,
				    "key", key,
                                    "hide_on_esc", FALSE,
				    NULL);

  self->priv = new ChatWindowPrivate;

  self->priv->notification_core =
    core.get<Ekiga::NotificationCore>("notification-core");

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_container_add (GTK_CONTAINER (self), box);
  gtk_widget_show (box);

  GtkWidget* switcher = gtk_stack_switcher_new ();
  gtk_box_pack_start (GTK_BOX (box), switcher,
		      FALSE, FALSE, 0);
  gtk_widget_show (switcher);

  self->priv->stack = gtk_stack_new ();
  gtk_stack_switcher_set_stack (GTK_STACK_SWITCHER (switcher), GTK_STACK (self->priv->stack));
  gtk_box_pack_start (GTK_BOX (box), self->priv->stack,
		      TRUE, TRUE, 0);
  gtk_widget_show (self->priv->stack);

  boost::shared_ptr<Ekiga::ChatCore> chat_core =
    core.get<Ekiga::ChatCore> ("chat-core");
  self->priv->connections.add (chat_core->dialect_added.connect (boost::bind (&on_dialect_added, self, _1)));
  self->priv->connections.add (chat_core->questions.connect (boost::bind (&on_handle_questions, self, _1)));
  chat_core->visit_dialects (boost::bind (&on_dialect_added, self, _1));

  return (GtkWidget*)self;
}
