
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras
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
 *                         search-window.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2006 by Julien Puydt
 *   copyright            : (c) 2006-2007 by Julien Puydt
 *   description          : declaration of the user interface for contacts-core
 *
 */

#ifndef __SEARCH_WINDOW_H__
#define __SEARCH_WINDOW_H__

#include <gtk/gtk.h>
#include "contact-core.h"

typedef struct _SearchWindow SearchWindow;
typedef struct _SearchWindowPrivate SearchWindowPrivate;
typedef struct _SearchWindowClass SearchWindowClass;

/* GObject thingies */
struct _SearchWindow
{
  GtkWindow parent;

  SearchWindowPrivate *priv;
};

struct _SearchWindowClass
{
  GtkWindowClass parent;
};


#define SEARCH_WINDOW_TYPE (search_window_get_type ())

#define SEARCH_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SEARCH_WINDOW_TYPE, SearchWindow))

#define IS_SEARCH_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SEARCH_WINDOW_TYPE))

#define SEARCH_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SEARCH_WINDOW_TYPE, SearchWindowClass))

#define IS_SEARCH_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SEARCH_WINDOW_TYPE))

#define SEARCH_WINDOW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), SEARCH_WINDOW_TYPE, SearchWindowClass))

GType search_window_get_type ();


/* public api */
GtkWidget *search_window_new (Ekiga::ContactCore *core,
                              std::string title);

#endif
