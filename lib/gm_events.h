
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
 *                         gm_events.h  -  description
 *                         -------------------------------------
 *   begin                : Tue, 31 Aug 2004
 *   copyright            : (C) 2004 by Julien Puydt and Damien Sandras
 *   description          : New signals for GObjects, and an event dispatcher
 *
 */

#include <glib.h>
#include <glib-object.h>

#ifndef _GM_EVENTS_H_
#define _GM_EVENTS_H_
G_BEGIN_DECLS

/*
 * New signals for GObjects added by this code:
 *
 * - "call_begin"
 * void call_begin_cb (GObject *self,
 *                     gchar *call_token,
 *                     gpointer user_data);
 *
 *
 * - "call_end"
 * void call_end_cb (GObject *self,
 *                   gchar *call_token,
 *                   gchar *reason,
 *                   gpointer user_data);
 *
 *
 * - "endpoint-state-changed"
 * void endpoint_state_changed_cb (GObject *self,
 *                                 gint new_state,
 *                                 gpointer user_data);
 *
 */


/* DESCRIPTION  : This function is supposed to be called very early
 *                in gnomemeeting's life.
 * BEHAVIOR     : Creates the new signals.
 * PRE          : /
 */
void gm_events_init ();


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns a new events' dispatcher (an object that receives
 *                the new signals, and forwards them to all the observers)
 * PRE          : /
 */
GObject *gm_events_dispatcher_new ();


/* DESCRIPTION  : /
 * BEHAVIOR     : Adds an observer to the given dispatcher.
 * PRE          : Non-null dispatcher and observer.
 */
void gm_events_dispatcher_add_observer (GObject *dispatcher,
					GObject *observer);


G_END_DECLS
#endif
