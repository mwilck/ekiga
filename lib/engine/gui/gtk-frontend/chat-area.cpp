
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
 *                        chat-area.cpp  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Implementation of a Chat area (view and control)
 *
 */

#include "chat-area.h"

class ChatAreaHelper;

struct _ChatAreaPrivate
{
  Ekiga::Chat* chat;
  sigc::connection connection;
  ChatAreaHelper* helper;

  /* we contain those, so no need to unref them */
  GtkWidget* text_view;
  GtkWidget* entry;
};

enum {
  CHAT_AREA_PROP_CHAT = 1
};

static GObjectClass* parent_class = NULL;

/* declaration of internal api */

static void chat_area_add_notice (ChatArea* self,
				  const gchar* txt);

static void chat_area_add_message (ChatArea* self,
				   const gchar* from,
				   const gchar* txt);

/* declaration of the helping observer */
class ChatAreaHelper: public Ekiga::ChatObserver
{
public:
  ChatAreaHelper (ChatArea* area_): area(area_)
  {}

  ~ChatAreaHelper ()
  {}

  void message (const std::string from,
		const std::string msg)
  { chat_area_add_message (area, from.c_str (), msg.c_str ()); }

  void notice (const std::string msg)
  { chat_area_add_notice (area, msg.c_str ()); }

private:
  ChatArea* area;
};


/* declaration of callbacks */
static void on_entry_activated (GtkWidget* entry,
				gpointer data);

static void on_chat_removed (ChatArea* self);

/* implementation of internal api */

static void
chat_area_add_notice (ChatArea* self,
		      const gchar* txt)
{
  gchar* str = NULL;
  GtkTextBuffer* buffer = NULL;
  GtkTextIter iter;

  str = g_strdup_printf ("NOTICE: %s\n", txt);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->text_view));
  gtk_text_buffer_get_end_iter (buffer, &iter);
  gtk_text_buffer_insert (buffer, &iter, str, -1);
  g_free (str);
}

static void
chat_area_add_message (ChatArea* self,
		       const gchar* from,
		       const gchar* txt)
{
  gchar* str = NULL;
  GtkTextBuffer* buffer = NULL;
  GtkTextIter iter;

  str = g_strdup_printf ("%s says: %s\n", from, txt);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->priv->text_view));
  gtk_text_buffer_get_end_iter (buffer, &iter);
  gtk_text_buffer_insert (buffer, &iter, str, -1);
  g_free (str);
}

/* implementation of callbacks */

static void
on_chat_removed (ChatArea* self)
{
  gtk_widget_hide (self->priv->entry);
}

static void
on_entry_activated (GtkWidget* entry,
		    gpointer data)
{
  ChatArea* self = NULL;
  const gchar* text = NULL;

  self = CHAT_AREA (data);

  text = gtk_entry_get_text (GTK_ENTRY (entry));

  if (text != NULL && !g_str_equal (text, "")) {

    if (self->priv->chat->send_message (text))
      gtk_entry_set_text (GTK_ENTRY (entry), "");
  }
}

/* GObject code */

static void
chat_area_dispose (GObject* obj)
{
  ChatArea* self = NULL;

  self = (ChatArea*)obj;

  parent_class->dispose (obj);
}

static void
chat_area_finalize (GObject* obj)
{
  ChatArea* self = NULL;

  self = (ChatArea*)obj;

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
chat_area_get_property (GObject* obj,
			guint prop_id,
			GValue* value,
			GParamSpec* spec)
{
  ChatArea* self = NULL;

  self = (ChatArea*)obj;

  switch (prop_id) {

  case CHAT_AREA_PROP_CHAT:
    g_value_set_pointer (value, self->priv->chat);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}

static void
chat_area_set_property (GObject* obj,
			guint prop_id,
			const GValue* value,
			GParamSpec* spec)
{
  ChatArea* self = NULL;
  gpointer ptr = NULL;

  self = (ChatArea* )obj;

  switch (prop_id) {

  case CHAT_AREA_PROP_CHAT:
    ptr = g_value_get_pointer (value);
    self->priv->chat = (Ekiga::Chat *)ptr;
    self->priv->connection = self->priv->chat->removed.connect (sigc::bind (sigc::ptr_fun (on_chat_removed), self));
    self->priv->helper = new ChatAreaHelper (self);
    self->priv->chat->connect (*(self->priv->helper));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}

static void
chat_area_class_init (gpointer g_class,
		      G_GNUC_UNUSED gpointer class_data)
{
  GObjectClass* gobject_class = NULL;
  GParamSpec* spec = NULL;

  parent_class = (GObjectClass*)g_type_class_peek_parent (g_class);

  g_type_class_add_private (g_class, sizeof (ChatAreaPrivate));

  gobject_class = (GObjectClass*)g_class;
  gobject_class->dispose = chat_area_dispose;
  gobject_class->finalize = chat_area_finalize;
  gobject_class->get_property = chat_area_get_property;
  gobject_class->set_property = chat_area_set_property;

  spec = g_param_spec_pointer ("chat",
			       "displayed chat",
			       "Displayed chat",
			       (GParamFlags)(G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (gobject_class,
				   CHAT_AREA_PROP_CHAT,
				   spec);
}

static void
chat_area_init (GTypeInstance* instance,
		G_GNUC_UNUSED gpointer g_class)
{
  ChatArea* self = NULL;

  self = (ChatArea*)instance;

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
					    TYPE_CHAT_AREA,
					    ChatAreaPrivate);
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
chat_area_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (ChatAreaClass),
      NULL,
      NULL,
      chat_area_class_init,
      NULL,
      NULL,
      sizeof (ChatArea),
      0,
      chat_area_init,
      NULL
    };

    result = g_type_register_static (GTK_TYPE_VBOX,
				     "ChatArea",
				     &info, (GTypeFlags) 0);
  }

  return result;
}

/* public api */

GtkWidget* 
chat_area_new (Ekiga::Chat& chat)
{
  return (GtkWidget*)g_object_new (TYPE_CHAT_AREA,
				   "chat", &chat,
				   NULL);
}

const std::string
chat_area_get_title (ChatArea* chat)
{
  return chat->priv->chat->get_title ();
}
