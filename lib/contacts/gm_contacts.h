
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
 *                         gm_contacts.h - description 
 *                         ---------------------------
 *   begin                : Mon Apr 12 2004
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Declaration of the addressbooks functions.
 *
 */


#include <glib.h>

G_BEGIN_DECLS

/* A Contact is identified by his UID. The UID must be unique.
 */
struct GmContact_ {

  char *uid;                    /* Unique UID */
  char *fullname;               /* User Full Name */
  char *url;                    /* URL to use when calling the user */
  char *speeddial;              /* Speed dial for that user */
  char *categories;             /* Categories the user belongs to, 
                                   comma separated */
  char *comment;                /* Comment about the user */
  char *software;               /* Software he is using */
  char *email;                  /* E-mail address of the user */
  int  state;                   /* Status of the user */
  gboolean video_capable;       /* Endpoint can send video */
};


/* An Address Book is identified by its name and its uid.
 * The UID is its URL, and must be unique.
 * An Address Book can be local or remote 
 */
struct GmAddressbook_ {

  char *name;                   /* Addressbook Name */
  char *uid;                    /* Unique ID. Must be the URL in case
                                   of a remote address book */
};


typedef struct GmContact_ GmContact;
typedef struct GmAddressbook_ GmAddressbook;


#define GM_CONTACT(x)     (GmContact *) (x)
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


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns an empty GmAddressBook. Only the UID field has
 *                a unique value.
 * PRE          : /
 */
GmAddressbook *gm_addressbook_new ();


/* DESCRIPTION  : /
 * BEHAVIOR     : Deletes the addressbook given as argument and frees the 
 *                corresponding memory.
 * PRE          : /
 */
void gm_addressbook_delete (GmAddressbook *);



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
 *                GmAddressbook. Only return the elements corresponding to the
 *                given filter.
 * PRE          : /
 */
GSList *gnomemeeting_addressbook_get_contacts (GmAddressbook *,
                                               gchar *);


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
 * BEHAVIOR     : Returns TRUE if the given address book is local, FALSE 
 *               otherwise.
 * PRE          : The uri field of the GmAddressbook must exist.
 */
gboolean gnomemeeting_addressbook_is_local (GmAddressbook *);


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


/* DESCRIPTION  : /
 * BEHAVIOR     : Modify the given GmContact to the given GmAddressbook.
 *                Return TRUE on success and FALSE on failure.
 * PRE          : The uri field of the GmAddressbook must exist. The contact
 *                must already exist.
 */
gboolean gnomemeeting_addressbook_modify_contact (GmAddressbook *,
                                                  GmContact *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns the list of available attributes for a remote 
 *                address book.
 * PRE          : The address book must be a remote address book.
 */
GSList *gnomemeeting_addressbook_get_attributes_list (GmAddressbook *);

G_END_DECLS

