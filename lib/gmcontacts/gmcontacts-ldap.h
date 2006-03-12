
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
 *                         gmcontacts-ldap.h - description 
 *                         --------------------------------
 *   begin                : Mon May 23 2004
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Declaration of the LDAP addressbook access 
 *   			    functions. Use the API in gmcontacts.h instead.
 *
 */


#if !defined (_GM_CONTACTS_H_INSIDE__)
#error "Only <contacts/gmcontacts.h> can be included directly."
#endif


#include <glib.h>
#include "gmcontacts.h"

#ifndef _GM_CONTACTS_LDAP_H_
#define _GM_CONTACTS_LDAP_H_


G_BEGIN_DECLS


gboolean gnomemeeting_addressbook_is_ldap (GmAddressbook *);


GSList *gnomemeeting_get_ldap_addressbooks ();


GSList *gnomemeeting_ldap_addressbook_get_contacts (GmAddressbook *,
						    int &,
						    gboolean,
						    gchar *,
						    gchar *,
						    gchar *,
						    gchar *,
						    gchar *);

G_END_DECLS


#endif

