
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
 *                         dbus_component.h  -  description
 *                         --------------------------
 *   begin                : Tue Oct 26 2004
 *   copyright            : (C) 2004 by Julien Puydt
 *   description          : External api of the DBUS component.
 *
 */


#ifndef _DBUS_COMPONENT_H_
#define _DBUS_COMPONENT_H_

#include <glib.h>
#include <glib-object.h>

#include "endpoint.h"

/*
 * This code makes gnomemeeting 'remote' controlled by DBUS
 * 
 * It provides two main types of functionality:
 * 1) it receives messages to DBUS, and makes gnomemeeting react to it
 * (get the list of connections, make or stop a call, etc) ; those work by
 * directly calling the endpoint's api.
 * 2) it sends messages to DBUS to notify from various events
 * (new call, call end, etc). Those work by getting signals from the events
 * dispatcher.
 *
 * It is a sort of bridge between a DBUS daemon and a gnomemeeting endpoint.
 */

/* Here is the description of DBUS messages understood by this component:
 *
 * Method calls:
 * =============
 *
 * "ConnectTo"
 * in    : string (url)
 * out   : nil
 * action: makes gnomemeeting call the given url
 *
 * "GetState"
 * in    : nil
 * out   : string (describes gnomemeeting's state: Standby, Calling, etc)
 * action: none
 *
 * "Disconnect"
 * in    : string (call token)
 * out   : nil
 * action: gnomemeeting disconnects the given call
 *
 * "GetCallsList"
 * in    : nil
 * out   : list of strings (call tokens, iterate to get them)
 * action: none
 *
 * "GetCallInfo"
 * in    : string (call token)
 * out   : string (name), string (url) and string (application)
 * action: none
 *
 * Signals:
 * ========
 *
 * "StateChanged"
 * data: string (state)
 * goal: gnomemeeting's state changed
 *
 * "AddCall"
 * data: string (call token)
 * goal: gnomemeeting manages a new call
 *
 * "DeleteCall"
 * data: string (call token)
 * goal: gnomemeeting closed a call (ie: the call token isn't valid anymore!)
 *
 */

/* Here are gnomemeeting's signals received by this component:
 * (those are described in lib/gm_events.h)
 * "call-begin" ;
 * "call-end" ;
 * "endpoint-state-changed".
 */

#define GM_DBUS_OBJECT_PATH "/org/gnomemeeting/Endpoint"
#define GM_DBUS_INTERFACE "org.gnomemeeting.CallService"
#define GM_DBUS_SERVICE "org.gnomemeeting.CallService"


G_BEGIN_DECLS


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns a valid DBUS component casted as a GObject
 * PRE          : a valid endpoint
 */
GObject *dbus_component_new(GMH323EndPoint *endpoint);


/* the two following functions, currently unused, could be used by Damien
 * to make so that if gnomemeeting is called with "-c url", then if there's
 * already another instance, pass it the call and exit.
 */

/* DESCRIPTION  : /
 * BEHAVIOR     : Returns TRUE if there's no other gnomemeeting registered
 *                on DBUS.
 * PRE          : A non-NULL DBUS component casted as a GObject
 */
gboolean dbus_component_is_first_instance (GObject *object);


/* DESCRIPTION  : /
 * BEHAVIOR     : Makes the gnomemeeting registered on DBUS call the given
 *                url.
 * PRE          : A non-NULL DBUS component casted as a GObject, and an URL
 */
void dbus_component_call_address (GObject *object, gchar *url);


G_END_DECLS

#endif
