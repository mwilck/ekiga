
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         gm_contacts-dnd.h  -  description
 *                         ------------------------------------------
 *   begin                : July 2004
 *   copyright            : (C) 2004 by Julien Puydt
 *   description          : Drag'n drop of contacts made easy (declarations)
 *
 */

#if !defined (_GM_CONTACTS_H_INSIDE__)
#error "Only <contacts/gm_contacts.h> can be included directly."
#endif


#include <gtk/gtk.h>
#include "gm_contacts.h"

#ifndef __GM_CONTACTS_DND_H
#define __GM_CONTACTS_DND_H

G_BEGIN_DECLS

/* 
 * the various contacts' formats supported by this code
 */

typedef enum {
  DND_GMCONTACT,
  DND_VCARD,
  DND_NUMBER_OF_TARGETS
} dnd_type;

static GtkTargetEntry dnd_targets [] =
  {
    {"GmContact", GTK_TARGET_SAME_APP, DND_GMCONTACT},
    {"text/x-vcard", 0, DND_VCARD}
  };

/*
 * Declaration of the gtk callbacks
 */

/* All of them expect a widget with either a "GmDnd-Source" or "GmDndTarget"
 * data attached, containing the user-provided data when the widget was set up
 * as source/target, and a user_data pointer that is the user-defined callback
 * that manages only GmContact* contacts. That means they all check they get
 * non-NULL widget & callbacks. Additionally, those that get the info argument
 * check that it is a valid dnd_type.
 *
 * They are all gtk callbacks that translate the raw contact information to
 * a GmContact* contact information, then call the user-defined callbacks.
 */

void drag_data_get_cb (GtkWidget *widget, GdkDragContext *context,
		       GtkSelectionData *data, guint info, guint time,
		       gpointer user_data);


void drag_data_received_cb (GtkWidget *widget, GdkDragContext *context,
			    gint x, gint y, GtkSelectionData *data,
			    guint info, guint time, gpointer user_data);


gboolean drag_motion_cb (GtkWidget *widget, GdkDragContext *context,
			 int x, int y, guint time, gpointer user_data);


G_END_DECLS

#endif // __GM_CONTACTS_DND_H
