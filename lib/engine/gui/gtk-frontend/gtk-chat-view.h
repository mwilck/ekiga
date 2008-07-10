
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
 *                        gtk-chat-view.h  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Declaration of a Chat "view" (it has controls)
 *
 */

#ifndef __GTK_CHAT_VIEW_H__
#define __GTK_CHAT_VIEW_H__

#include <gtk/gtk.h>

#include "chat.h"

G_BEGIN_DECLS

typedef struct _GtkChatView GtkChatView;
typedef struct _GtkChatViewClass GtkChatViewClass;
typedef struct _GtkChatViewPrivate GtkChatViewPrivate;

struct _GtkChatView
{
  GtkVBox parent;

  GtkChatViewPrivate *priv;
};

struct _GtkChatViewClass
{
  GtkVBoxClass parent;
};

/* public api */
GtkWidget *gtk_chat_view_new (Ekiga::Chat &chat);

const std::string gtk_chat_view_get_title (GtkChatView* chat);

/* GObject thingies */

#define GTK_TYPE_CHAT_VIEW (gtk_chat_view_get_type ())
#define GTK_CHAT_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_CHAT_VIEW, GtkChatView))
#define GTK_CHAT_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_CHAT_VIEW, GtkChatViewClass))
#define GTK_IS_CHAT_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_CHAT_VIEW))
#define GTK_IS_CHAT_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CHAT_VIEW))
#define GTK_CHAT_VIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_CHAT_VIEW, GtkChatViewClass))

GType gtk_chat_view_get_type ();


G_END_DECLS
#endif
