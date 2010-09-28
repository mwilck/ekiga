
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
 *                         roster-view-gtk.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of the widget representing the roster
 *
 */

#ifndef __ROSTER_VIEW_GTK_H__
#define __ROSTER_VIEW_GTK_H__

#include <gtk/gtk.h>
#include "presence-core.h"

typedef struct _RosterViewGtk RosterViewGtk;
typedef struct _RosterViewGtkPrivate RosterViewGtkPrivate;
typedef struct _RosterViewGtkClass RosterViewGtkClass;

/*
 * Public API
 *
 */

/* Creating the widget, connected to an Ekiga::PresenceCore object */
GtkWidget *roster_view_gtk_new (Ekiga::PresenceCore &core);


/* Knowning what is selected in the view
 * If it's:
 * - a heap, you'll get it through 'heap' and the other two will be NULL ;
 * - a group, you'll get the heap and the name through 'heap' and 'group', and 'presentity' will be NULL ;
 *   (and you'll have to free the group name afterwards)
 * - a presentity, you'll get it through 'presentity', and the other two will be NULL ;
 */
void roster_view_gtk_get_selected (RosterViewGtk* self,
				   Ekiga::Heap** heap,
				   gchar** group, /* you'll have to free it */
				   Ekiga::Presentity** presentity);

/* Signals emitted by that widget :
 *
 * - "heap-selected", comes with a pointer which is an Ekiga::Heap*
 * (or NULL if no heap is selected anymore)
 *
 * - "heap-group-selected", comes with a pointer which is an Ekiga::Heap*
 * (or NULL if no group is selected anymore)
 * and a string which is the name of the group
 *
 * - "presentity-selected", comes with a pointer which is an Ekiga::Presentity*
 * (or NULL if no presentity is selected anymore)
 */


/* GObject thingies */
struct _RosterViewGtk
{
  GtkFrame parent;

  RosterViewGtkPrivate *priv;
};

struct _RosterViewGtkClass
{
  GtkFrameClass parent;

  void (*presentity_selected) (RosterViewGtk* self,
			       gpointer presentity);
  void (*heap_selected) (RosterViewGtk* self,
			 gpointer heap);
  void (*heap_group_selected) (RosterViewGtk* self,
			       gpointer heap,
			       const gchar* group_name);
};

#define ROSTER_VIEW_GTK_TYPE (roster_view_gtk_get_type ())

#define ROSTER_VIEW_GTK(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), ROSTER_VIEW_GTK_TYPE, RosterViewGtk))

#define IS_ROSTER_VIEW_GTK(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ROSTER_VIEW_GTK_TYPE))

#define ROSTER_VIEW_GTK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), ROSTER_VIEW_GTK_TYPE, RosterViewGtkClass))

#define IS_ROSTER_VIEW_GTK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), ROSTER_VIEW_GTK_TYPE))

#define ROSTER_VIEW_GTK_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), ROSTER_VIEW_GTK_TYPE, RosterViewGtkClass))

GType roster_view_gtk_get_type ();

#endif
