
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


/* this API that takes care about drag'n dropping of contacts */


/* the three types of callback functions with this code:
 * 1) "get contact" is called on the source when a drop occurred,
 *    to get the GmContact that was dragged ;
 * 2) "put contact" is called on the target when the drop occurred, it also
 *    receives the contact information ;
 * 3) "allow drop" is called on the target during the drag, in response to
 *    motion events, and allow the target to react accordingly (highlight
 *    itself, for example) ; it receives the position of the cursor in the
 *    widget.
 * All of them also get user data.
 */
typedef GmContact *(*GmDndGetContact) (GtkWidget *, gpointer);
typedef void (*GmDndPutContact) (GtkWidget *, GmContact *, 
				 gint, gint, gpointer);
typedef gboolean (*GmDndAllowDrop) (GtkWidget *, gint, gint, gpointer);


/* DESCRIPTION  : /
 * BEHAVIOR     : Sets the widget as a source of contact information ;
 *                the given callback will get the widget as first argument,
 *                and the gpointer as last argument.
 * PRE          : Assumes the widget and the callback are non-NULL.
 */
void gm_contacts_dnd_set_source (GtkWidget *, GmDndGetContact, gpointer);


/* DESCRIPTION  : /
 * BEHAVIOR     : Sets the widget as a target of contact information ;
 *                the given callback will get the widget as first argument,
 *                and the gpointer as last argument.
 * PRE          : Assumes the widget and the callback are non-NULL.
 */
void gm_contacts_dnd_set_dest (GtkWidget *, GmDndPutContact, gpointer);


/* DESCRIPTION  : /
 * BEHAVIOR     : Sets the widget as a target of contact information ;
 *                the given callback will get the widget as first argument,
 *                and the gpointer as last argument. The difference with
 *                the former is that there's also a callback to probe if the
 *                cursor really is in a drop zone.
 * PRE          : Assumes the widget and the callbacks are non-NULL.
 */
void gm_contacts_dnd_set_dest_conditional (GtkWidget *, 
					   GmDndPutContact, GmDndAllowDrop,
					   gpointer);


G_END_DECLS

#endif // __GM_CONTACTS_DND_H
