
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 *                         dbus_component.h  -  description
 *                         -----------------------------
 *   begin                : Tue Nov 1  2005
 *   copyright            : (C) 2005 by Julien Puydt
 *   description          : This files contains the interface to the DBUS
 *                          interface of gnomemeeting.
 *
 */

#ifndef __DBUS_COMPONENT_H
#define __DBUS_COMPONENT_H

#include <glib-object.h>

#include "manager.h"

G_BEGIN_DECLS

enum {
  INVALID_ACCOUNT,
  UNREGISTERED,
  REGISTERED
};

enum {
  INVALID_CALL,
  STANDBY,
  CALLING,
  CONNECTED,
  CALLED
};

GObject *gnomemeeting_dbus_component_new ();

void gnomemeeting_dbus_component_set_call_state (GObject *obj,
						 const gchar *token,
						 GMManager::CallingState state);

void gnomemeeting_dbus_component_set_call_info (GObject *obj,
						const gchar *token,
						const gchar *name,
						const gchar *client,
						const gchar *url,
						const gchar *protocol_prefix);

void gnomemeeting_dbus_component_set_call_on_hold (GObject *obj,
						   const gchar *token,
						   gboolean is_on_hold);

gboolean gnomemeting_dbus_component_is_first_instance (GObject *obj);

void gnomemeeting_dbus_component_call (GObject *obj,
				       const gchar *uri);

void gnomemeeting_dbus_component_account_registration (GObject *obj,
						       const gchar *username,
						       const gchar *domain,
						       gboolean registered);
							      

G_END_DECLS

#endif /* __DBUS_COMPONENT_H */
