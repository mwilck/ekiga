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
 *                         main_window.h  -  description
 *                         -----------------------------
 *   begin                : Mon Mar 26 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          build the main window.
 */


#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include "gmwindow.h"
#include "ekiga-app.h"

G_BEGIN_DECLS

#define EKIGA_TYPE_MAIN_WINDOW               (ekiga_main_window_get_type ())
#define EKIGA_MAIN_WINDOW(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), EKIGA_TYPE_MAIN_WINDOW, EkigaMainWindow))
#define EKIGA_MAIN_WINDOW_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), EKIGA_TYPE_MAIN_WINDOW, EkigaMainWindowClass))
#define EKIGA_IS_MAIN_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EKIGA_TYPE_MAIN_WINDOW))
#define EKIGA_IS_MAIN_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), EKIGA_TYPE_MAIN_WINDOW))

typedef struct _EkigaMainWindowPrivate       EkigaMainWindowPrivate;
typedef struct _EkigaMainWindow              EkigaMainWindow;

struct _EkigaMainWindow {
  GmWindow                parent;
  EkigaMainWindowPrivate *priv;
};

typedef GmWindowClass EkigaMainWindowClass;

GType        ekiga_main_window_get_type   ();
GtkWidget   *ekiga_main_window_new        (Ekiga::ServiceCore *core);

G_END_DECLS

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the main window and adds the popup to the image.
 * PRE          :  /
 */
GtkWidget *gm_main_window_new (GmApplication *app);
#endif
