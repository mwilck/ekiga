
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
 *                        gtk-chat-view.cpp  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Implementation of a Chat "view" (it has controls)
 *
 */

#include "gtk-chat-view.h"

class GtkChatViewHelper;

struct _GtkChatViewPrivate
{
  Ekiga::Chat *chat;
  sigc::connection connection;
  GtkChatViewHelper *helper;

  /* we contain those, so no need to unref them */
  GtkWidget *text_view;
  GtkWidget *entry;
};

enum {
  GTK_CHAT_VIEW_PROP_CHAT = 1
};

static GObjectClass *parent_class = NULL;

/* declaration of internal api */

static void gtk_chat_view_add_notice (GtkChatView *self,
				      const gchar *txt);

static void gtk_chat_view_add_message (GtkChatView *self,
				       const gchar *from,
				       const gchar *txt);

/* declaration of the helping observer */
class GtkChatViewHelper: public Ekiga::ChatObserver
{
public:
  GtkChatViewHelper (GtkChatView *view_): view(view_)
  {}

  ~GtkChatViewHelper ()
  {}

  void message (const std::string from,
		const std::string msg)
  { gtk_chat_view_add_message (view, from.c_str (), msg.c_str ()); }

  void notice (const std::string msg)
  { gtk_chat_view_add_notice (view, msg.c_str ()); }

private:
  GtkChatView *view;
};


/* declaration of callbacks */
static void on_entry_activated (GtkWidget *entry,
				gpointer data);

static void on_chat_removed (GtkChatView *self);

/* implementation of internal api */

static void
gtk_chat_view_add_notice (GtkChatView *self,
			  const gchar *txt)
{
  gchar *str = NULL;
  GtkTextBuffer *buffer = NULL;
  GtkTextIter iter;

  str = g_strdup_printf ("NOTICE: %s\n", txt);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->text_view));
  gtk_text_buffer_get_end_iter (buffer, &iter);
  gtk_text_buffer_insert (buffer, &iter, str, -1);
  g_free (str);
}

static void
gtk_chat_view_add_message (GtkChatView *self,
			   const gchar *from,
			   const gchar *txt)
{
  gchar *str = NULL;
  GtkTextBuffer *buffer = NULL;
  GtkTextIter iter;

  str = g_strdup_printf ("%s says: %s\n", from, txt);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->text_view));
  gtk_text_buffer_get_end_iter (buffer, &iter);
  gtk_text_buffer_insert (buffer, &iter, str, -1);
  g_free (str);
}

/* implementation of callbacks */

static void
on_chat_removed (GtkChatView *self)
{
  gtk_widget_hide (self->priv->entry);
}

static void
on_entry_activated (GtkWidget *entry,
		    gpointer data)
{
  GtkChatView *self = NULL;
  const gchar *text = NULL;

  self = GTK_CHAT_VIEW (data);

  text = gtk_entry_get_text (GTK_ENTRY (entry));

  if (text != NULL && !g_str_equal (text, "")) {

    if (self->priv->chat->send_message (text))
      gtk_entry_set_text (GTK_ENTRY (entry), "");
  }
}

/* GObject code */

static void
gtk_chat_view_dispose (GObject *obj)
{
  GtkChatView *self = NULL;

  self = (GtkChatView *)obj;

  parent_class->dispose (obj);
}

static void
gtk_chat_view_finalize (GObject *obj)
{
  GtkChatView *self = NULL;

  self = (GtkChatView *)obj;

  if (self->priv->chat) {

    self->priv->connection.disconnect ();
    if (self->priv->helper) {
      self->priv->chat->disconnect (*(self->priv->helper));
      delete self->priv->helper;
      self->priv->helper = NULL;
    }
    self->priv->chat = NULL;
  }

  parent_class->finalize (obj);
}

static void
gtk_chat_view_get_property (GObject *obj,
			    guint prop_id,
			    GValue *value,
			    GParamSpec *spec)
{
  GtkChatView *self = NULL;

  self = (GtkChatView *)obj;

  switch (prop_id) {

  case GTK_CHAT_VIEW_PROP_CHAT:
    g_value_set_pointer (value, self->priv->chat);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}

static void
gtk_chat_view_set_property (GObject *obj,
			    guint prop_id,
			    const GValue *value,
			    GParamSpec *spec)
{
  GtkChatView *self = NULL;
  gpointer ptr = NULL;

  self = (GtkChatView *)obj;

  switch (prop_id) {

  case GTK_CHAT_VIEW_PROP_CHAT:
    ptr = g_value_get_pointer (value);
    self->priv->chat = (Ekiga::Chat *)ptr;
    self->priv->connection = self->priv->chat->removed.connect (sigc::bind (sigc::ptr_fun (on_chat_removed), self));
    self->priv->helper = new GtkChatViewHelper (self);
    self->priv->chat->connect (*(self->priv->helper));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}

static void
gtk_chat_view_class_init (gpointer g_class,
			  gpointer class_data)
{
  GObjectClass *gobject_class = NULL;
  GParamSpec *spec = NULL;

  (void)class_data; /* -Wextra */

  parent_class = (GObjectClass *)g_type_class_peek_parent (g_class);

  g_type_class_add_private (g_class, sizeof (GtkChatViewPrivate));

  gobject_class = (GObjectClass *) g_class;
  gobject_class->dispose = gtk_chat_view_dispose;
  gobject_class->finalize = gtk_chat_view_finalize;
  gobject_class->get_property = gtk_chat_view_get_property;
  gobject_class->set_property = gtk_chat_view_set_property;

  spec = g_param_spec_pointer ("chat",
			       "displayed chat",
			       "Displayed chat",
			       (GParamFlags)(G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (gobject_class,
				   GTK_CHAT_VIEW_PROP_CHAT,
				   spec);
}

static void
gtk_chat_view_init (GTypeInstance *instance,
		    gpointer g_class)
{
  GtkChatView *self = NULL;

  (void)g_class; /* -Wextra */

  self = (GtkChatView *)instance;

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
					    GTK_TYPE_CHAT_VIEW,
					    GtkChatViewPrivate);
  self->priv->chat = NULL;

  self->priv->text_view = gtk_text_view_new ();
  gtk_widget_show (self->priv->text_view);
  gtk_box_pack_start (GTK_BOX (self), self->priv->text_view,
		      TRUE, TRUE, 2);

  self->priv->entry = gtk_entry_new ();
  gtk_widget_show (self->priv->entry);
  gtk_box_pack_end (GTK_BOX (self), self->priv->entry,
		    FALSE, TRUE, 2);
  g_signal_connect (self->priv->entry, "activate",
		    G_CALLBACK (on_entry_activated), self);
}


GType
gtk_chat_view_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (GtkChatViewClass),
      NULL,
      NULL,
      gtk_chat_view_class_init,
      NULL,
      NULL,
      sizeof (GtkChatView),
      0,
      gtk_chat_view_init,
      NULL
    };

    result = g_type_register_static (GTK_TYPE_VBOX,
				     "GtkChatView",
				     &info, (GTypeFlags) 0);
  }

  return result;
}

/* public api */

GtkWidget *
gtk_chat_view_new (Ekiga::Chat &chat)
{
  return (GtkWidget *)g_object_new (GTK_TYPE_CHAT_VIEW,
				    "chat", &chat,
				    NULL);
}

const std::string
gtk_chat_view_get_title (GtkChatView* chat)
{
  return chat->priv->chat->get_title ();
}
