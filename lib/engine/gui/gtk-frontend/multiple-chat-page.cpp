
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
 *   description          : Declaration of a page displaying a MultipleChat
 *
 */

#include "multiple-chat-page.h"
#include "chat-area.h"

struct _MultipleChatPagePrivate {
  GtkWidget* area;
};

G_DEFINE_TYPE (MultipleChatPage, multiple_chat_page, GTK_TYPE_BOX);

static void on_page_grab_focus (GtkWidget*,
				gpointer);

static void on_page_grab_focus (GtkWidget* widget,
				G_GNUC_UNUSED gpointer data)
{
  MultipleChatPage* self = NULL;

  self = (MultipleChatPage*)widget;

  if (self->priv->area)
    gtk_widget_grab_focus (self->priv->area);
}

static void
multiple_chat_page_init (MultipleChatPage* self)
{
  self->priv = g_new0 (MultipleChatPagePrivate, 1);

  g_signal_connect (self, "grab-focus",
                    G_CALLBACK (on_page_grab_focus), NULL);
}

static void
multiple_chat_page_class_init (G_GNUC_UNUSED MultipleChatPageClass* klass)
{
}

/* implementation of the public api */

GtkWidget*
multiple_chat_page_new (Ekiga::MultipleChatPtr chat)
{
  MultipleChatPage* result = NULL;
  GtkWidget* area = NULL;

  result = (MultipleChatPage*)g_object_new (TYPE_MULTIPLE_CHAT_PAGE, NULL);

  area = chat_area_new (chat);
  result->priv->area = area;
  gtk_box_pack_start (GTK_BOX (result), area,
		      TRUE,TRUE, 2);
  gtk_widget_show (area);

  return GTK_WIDGET (result);
}
