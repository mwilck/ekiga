
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2010 Damien Sandras <dsandras@seconix.com>
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
 *                        heap-view.h  -  description
 *                        --------------------------------
 *   begin                : written in november 2010
 *   copyright            : (C) 2010 by Julien Puydt
 *   description          : Declaration of a widget displaying an Ekiga::Heap
 *
 */

#ifndef __HEAP_VIEW_H__
#define __HEAP_VIEW_H__

#include <gtk/gtk.h>
#include "heap.h"

G_BEGIN_DECLS

/* public api */

typedef struct _HeapView HeapView;

/* creating the widget, connected to an Ekiga::Heap object */
GtkWidget* heap_view_new (Ekiga::HeapPtr heap);

/* populates the given builder with the actions possible on the selected item, whatever that is (group, presentity) */
bool heap_view_populate_menu_for_selected (HeapView* self,
					   Ekiga::MenuBuilder& builder);

/* GObject boilerplate */

typedef struct _HeapViewPrivate HeapViewPrivate;
typedef struct _HeapViewClass HeapViewClass;

struct _HeapView {
  GtkFrame parent;

  HeapViewPrivate* priv;
};

struct _HeapViewClass {
  GtkFrameClass parent_class;

  void (*selection_changed) (HeapView* self);
};

#define TYPE_HEAP_VIEW             (heap_view_get_type())
#define HEAP_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),TYPE_HEAP_VIEW,HeapView))
#define HEAP_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),TYPE_HEAP_VIEW,HeapViewClass))
#define IS_HEAP_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),TYPE_HEAP_VIEW))
#define IS_HEAP_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),TYPE_HEAP_VIEW))
#define HEAP_VIEW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),TYPE_HEAP_VIEW,HeapViewClass))

GType heap_view_get_type () G_GNUC_CONST;

G_END_DECLS

#endif
