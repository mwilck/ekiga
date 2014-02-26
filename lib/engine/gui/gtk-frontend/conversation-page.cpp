
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
 *                        multiple-chat-page.cpp  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Declaration of a page displaying a Conversation
 *
 */

#include "conversation-page.h"
#include "chat-area.h"
#include "heap-view.h"

#include "scoped-connections.h"

struct _ConversationPagePrivate {

  Ekiga::ConversationPtr conversation;
  Ekiga::scoped_connections connections;
  GtkWidget* area;
  GtkWidget* heapview;
};

enum {
  UPDATED_SIGNAL,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0,};

G_DEFINE_TYPE (ConversationPage, conversation_page, GTK_TYPE_BOX);


static void
on_conversation_updated (ConversationPage* self)
{
  g_signal_emit (self, signals[UPDATED_SIGNAL], 0);
}

static void
on_page_grab_focus (GtkWidget* widget,
		    G_GNUC_UNUSED gpointer data)
{
  ConversationPage* self = (ConversationPage*)widget;

  gtk_widget_grab_focus (self->priv->area);
}

static void
conversation_page_finalize (GObject* obj)
{
  ConversationPage* self = (ConversationPage*)obj;

  delete self->priv;

  G_OBJECT_CLASS (conversation_page_parent_class)->finalize (obj);
}

static void
conversation_page_init (ConversationPage* self)
{
  self->priv = new ConversationPagePrivate;

  g_signal_connect (self, "grab-focus",
                    G_CALLBACK (on_page_grab_focus), NULL);
}

static void
conversation_page_class_init (ConversationPageClass* klass)
{
  GObjectClass* gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = conversation_page_finalize;

  signals[UPDATED_SIGNAL] =
    g_signal_new ("updated", G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (ConversationPageClass, updated),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
}

/* implementation of the public api */

GtkWidget*
conversation_page_new (Ekiga::ConversationPtr conversation)
{
  ConversationPage* result = NULL;
  GtkWidget* area = NULL;
  GtkWidget* heapview = NULL;

  result = (ConversationPage*)g_object_new (TYPE_CONVERSATION_PAGE, NULL);

  result->priv->conversation = conversation;

  result->priv->connections.add (conversation->updated.connect (boost::bind (&on_conversation_updated, result)));

  area = chat_area_new (conversation);
  result->priv->area = area;
  gtk_box_pack_start (GTK_BOX (result), area,
		      TRUE,TRUE, 2);
  gtk_widget_show (area);

  heapview = heap_view_new (conversation->get_heap());

  result->priv->heapview = heapview;
  gtk_box_pack_start (GTK_BOX (result), heapview,
		      TRUE,TRUE, 2);
  gtk_widget_show (heapview);

  return GTK_WIDGET (result);
}

const gchar*
conversation_page_get_title (GtkWidget* widget)
{
  g_return_val_if_fail (IS_CONVERSATION_PAGE (widget), NULL);

  return ((ConversationPage*)widget)->priv->conversation->get_title().c_str();
}

guint
conversation_page_get_unread_count (GtkWidget* widget)
{
  g_return_val_if_fail (IS_CONVERSATION_PAGE (widget), 0);

  return ((ConversationPage*)widget)->priv->conversation->get_unread_messages_count ();
}
