
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
 *                         gm_contacts-remote.h - description 
 *                         ----------------------------------
 *   begin                : Mon May 23 2004
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Declaration of the remote addressbook access 
 *   			    functions. Use the API in gm_contacts.h instead.
 *
 */


#if !defined (_GM_CONTACTS_H_INSIDE__)
#error "Only <contacts/gm_contacts.h> can be included directly."
#endif


#include <glib.h>
#include "gm_contacts.h"

#ifndef _GM_CONTACTS_REMOTE_H_
#define _GM_CONTACTS_REMOTE_H_


G_BEGIN_DECLS



GSList *gnomemeeting_remote_addressbook_get_contacts (GmAddressbook *,
						      int &,
						      gboolean,
						      gchar *,
						      gchar *,
						      gchar *,
						      gchar *);


gboolean gnomemeeting_remote_addressbook_add (GmAddressbook *);


gboolean gnomemeeting_remote_addressbook_delete (GmAddressbook *);


gboolean gnomemeeting_remote_addressbook_modify (GmAddressbook *);


G_END_DECLS


#endif

