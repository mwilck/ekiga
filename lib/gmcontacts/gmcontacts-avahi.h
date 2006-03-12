
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
 *                         gmcontacts-zeroconf.h  -  description
 *                         --------------------------------------
 *   begin                : Sun Aug 21 2005
 *   copyright            : (C) 2005 by Sebastien Estienne 
 *   description          : This file contains zeroconf browser related headers
 *
 */


#ifndef _GM_CONTACTS_AVAHI_H_
#define _GM_CONTACTS_AVAHI_H_

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/alternative.h>
#include <avahi-glib/glib-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>

#define ZC_H323 "_h323._tcp"
#define ZC_SIP "_sip._udp"

#include <gmcontacts.h>
#include <ptlib.h>
#include "gmconf.h"


/* Overloaded functions */
GSList *gnomemeeting_get_zero_addressbooks ();


GSList *gnomemeeting_zero_addressbook_get_contacts (GmAddressbook *,
						    int &,
						    gboolean,
						    gchar *,
						    gchar *,
						    gchar *,
						    gchar *,
						    gchar *);


void gnomemeeting_zero_addressbook_init ();

#endif
