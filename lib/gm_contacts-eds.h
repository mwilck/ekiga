
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
 *                         gm_contacts-eds.h  -  description 
 *                         ---------------------------------
 *   begin                : Mon Apr 12 2004
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Functions to manage the GM Addressbook using EDS..
 *
 */

#include <libebook/e-book.h>
#include "gm_contacts.h"

G_BEGIN_DECLS


/* An Address Book is identified by its name and its uid.
 * The UID is its URL, and must be unique.
 * An Address Book can be local or remote 
 */
struct GmAddressbook_ {

  char *uid;
  char *name;
};


typedef struct GmAddressbook_ GmAddressbook;


#define GM_ADDRESSBOOK(x) (GmAddressbook *) (x)


/* The API */


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns an empty GmContact. Only the UID field has a unique
 *                value.
 * PRE          : /
 */
GmContact *gm_contact_new ();


/* DESCRIPTION  : /
 * BEHAVIOR     : Deletes the contact given as argument and frees the 
 *                corresponding memory.
 * PRE          : /
 */
void gm_contact_delete (GmContact *);


/*
GmAddressbook *gm_addressbook_new ();

gm_addressbook_delete (GmAddressbook *);
*/


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns a GSList of GmAddressbook elements corresponding
 *                to the local address books.
 * PRE          : /
 */
GSList *gnomemeeting_get_local_addressbooks ();


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns a GSList of GmAddressbook elements corresponding
 *                to the remote address books (ils, ldap).
 * PRE          : /
 */
GSList *gnomemeeting_get_remote_addressbooks ();


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns a GSList of GmContact elements members of a given
 *                GmAddressbook.
 * PRE          : /
 */
GSList *gnomemeeting_addressbook_get_contacts (GmAddressbook *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Add the given GmAddressbook in the address books list.
 *                Return TRUE on success and FALSE on failure.
 * PRE          : /
 */
gboolean gnomemeeting_addressbook_add (GmAddressbook *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Deletes the given GmAddressbook from the address books list.
 *                Return TRUE on success and FALSE on failure.
 * PRE          : The uri field of the GmAddressbook must exist.
 */
gboolean gnomemeeting_addressbook_delete (GmAddressbook *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Deletes the given GmContact from the given GmAddressbook.
 *                Return TRUE on success and FALSE on failure.
 * PRE          : The uri field of the GmAddressbook must exist.
 */
gboolean gnomemeeting_addressbook_delete_contact (GmAddressbook *,
                                                  GmContact *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Add the given GmContact to the given GmAddressbook.
 *                Return TRUE on success and FALSE on failure.
 * PRE          : The uri field of the GmAddressbook must exist.
 */
gboolean gnomemeeting_addressbook_add_contact (GmAddressbook *,
                                               GmContact *);


G_END_DECLS
