
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
 *                         gmcontacts.h - description 
 *                         ---------------------------
 *   begin                : Mon Apr 12 2004
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Declaration of the addressbooks functions.
 *
 */


#include <gtk/gtk.h>

#ifndef _GM_CONTACTS_H_
#define _GM_CONTACTS_H_


G_BEGIN_DECLS

typedef enum {

  CONTACT_ONLINE,
  CONTACT_OFFLINE,
  CONTACT_UNKNOWN,
  CONTACT_AWAY,
  CONTACT_BUSY,
  CONTACT_DND,
  CONTACT_FREEFORCHAT,
  CONTACT_LAST_STATE
} ContactState;

/* A Contact is identified by his UID. The UID must be unique.
 */
struct GmContact_ {

  char *uid;                    /* Unique UID */
  char *fullname;               /* User Full Name */
  char *url;                    /* URL to use when calling the user */
  char *speeddial;              /* Speed dial for that user */
  char *categories;             /* Categories the user belongs to, 
                                   comma separated */
  char *location;		/* Location of the user */
  char *comment;                /* Comment about the user */
  char *software;               /* Software he is using */
  char *email;                  /* E-mail address of the user */
  ContactState  state;          /* Status of the user */
  gboolean video_capable;       /* Endpoint can send video */
};


/* An Address Book is identified by its aid.
 * The UID must be unique.
 * An Address Book can be local or remote 
 */
struct GmAddressbook_ {

  char *aid;			/* Addressbook ID */
  char *name;                   /* Addressbook Name */
  char *url;                    /* Unique ID. Must be the URL in case
                                   of a remote address book */
  char *call_attribute;		/* The attribute to use when calling somebody,
				   only used in case of remote address books */
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
GmContact *gmcontact_new ();


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns a copy of the given GmContact
 * PRE          : /
 */
GmContact *gmcontact_copy (GmContact *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Deletes the contact given as argument and frees the 
 *                corresponding memory.
 * PRE          : /
 */
void gmcontact_delete (GmContact *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Enumerates the categories a contact belongs to in a
 *                GSList of gchar*'s, or NULL if no category or error
 *                Takes care of "empty" categories, e.g.
 *                ``Friends,Ekiga Roster,,Coders'' (the ",," part)
 * PRE          : a valid pointer to a GmContact
 */
GSList* gmcontact_enum_categories (const GmContact *);


/* DESCRIPTION  : /
 * BEHAVIOR     : checks if a contact has a specific category entry
 * PRE          : a valid pointer to a GmContact, a string (category)
 */
gboolean gmcontact_is_in_category (const GmContact *,
				   const gchar*);

/* DESCRIPTION  : /
 * BEHAVIOR     : Returns an empty GmAddressBook. Only the UID field has
 *                a unique value.
 * PRE          : /
 */
GmAddressbook *gm_addressbook_new ();


/* DESCRIPTION  : /
 * BEHAVIOR     : Duplicates the addressbook given as argument.
 * PRE          : /
 */
GmAddressbook *gm_addressbook_copy (GmAddressbook *);


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
 * BEHAVIOR     : Returns the addressbook (infact the first one found) a given
 *                contact is stored in. The search is done by the UID of the
 *                given contact.
 * PRE          : /
 */
GmAddressbook *gnomemeeting_local_addressbook_get_by_contact (GmContact *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns the first contact found given the UID
 * PRE          : /
 */
GmContact *gnomemeeting_local_contact_get_by_uid (gchar *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns a GSList of GmAddressbook elements corresponding
 *                to the remote address books (ils, ldap).
 * PRE          : /
 */
GSList *gnomemeeting_get_remote_addressbooks ();


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns a GSList of GmContact elements members of a given
 *                GmAddressbook. Only return the elements corresponding to the
 *                given filter (full name, url, categorie, location,
 *                speed dial). The second parameter contains the number of
 *                registered users, or -1 in case of error. The
 *                speed dial is ignored on remote address books. If not 
 *                GmAddressbook is given, then the search is done on all
 *                local address books. Searching for speed dial "*" will return
 *                all contacts with a speed dial. If you specify a full name 
 *                and an url, all contacts with the specified full name OR url
 *                will be returned. If you specify a full name, an url and a
 *                speed dial, all contacts with that full name OR that url
 *                and the given speed dial will be returned. If the boolean
 *                is set to TRUE, the search will check for partial matches 
 *                except for the speed dial where an exact match is always
 *                queried. The int parameter is filled with the number of 
 *                contacts in the address book. It can be bigger than the
 *                number of returned contacts if there are hidden or offline
 *                contacts.
 * PRE          : Only one filter at a time.
 */
GSList *gnomemeeting_addressbook_get_contacts (GmAddressbook *addressbook,
					       int &nbr,
					       gboolean partial_match,
                                               gchar *fullname,
                                               gchar *url,
                                               gchar *categorie,
                                               gchar *location,
					       gchar *speeddial);

/* DESCRIPTION  : /
 * BEHAVIOR     : Returns a GSList of gchar* which represents all categories found
 *                in the local addressbook "addressbook". If addressbook is NULL,
 *                all local addressbooks are scanned. The list is free of duplicate
 *                categories.
 * PRE          : /
 */
GSList *gnomemeeting_local_addressbook_enum_categories (GmAddressbook *addressbook);

/* DESCRIPTION  : Rename a group specification in all local addressbooks
 * BEHAVIOR     : Alters all local contacts it finds and renames the given
 *                category name to the given new name in the contact's
 *                category list. Firts parameter is old name, second
 *                parameter is new name. Returns FALSE when the renaming
 *                didn't succeed for some reason, TRUE otherwise.
 * PRE          : /
 */
gboolean gnomemeeting_local_addressbooks_rename_category (const gchar *, const gchar *);

/* DESCRIPTION  : Delete a group specification in all local addressbooks
 * BEHAVIOR     : Alters all local contacts it finds and deletes the given
 *                category name in the contact's category list. The only
 *                parameter is the category name to delete. Returns FALSE
 *                when the delete didn't succeed for some reason, TRUE
 *                otherwise.
 * PRE          : /
 */
gboolean gnomemeeting_local_addressbooks_delete_category (const gchar *);

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
 * BEHAVIOR     : Modifies the given GmAddressbook from the address books list.
 *                Return TRUE on success and FALSE on failure.
 * PRE          : /
 */
gboolean gnomemeeting_addressbook_modify (GmAddressbook *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns TRUE if the given address book is local, FALSE 
 *               otherwise.
 * PRE          : The uri field of the GmAddressbook must exist.
 */
gboolean gnomemeeting_addressbook_is_local (GmAddressbook *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Add the given GmContact to the given GmAddressbook.
 *                Return TRUE on success and FALSE on failure.
 * PRE          : The uri field of the GmAddressbook must exist.
 */
gboolean gnomemeeting_addressbook_add_contact (GmAddressbook *,
                                               GmContact *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Deletes the given GmContact from the given GmAddressbook.
 *                Return TRUE on success and FALSE on failure.
 * PRE          : The uri field of the GmAddressbook must exist.
 */
gboolean gnomemeeting_addressbook_delete_contact (GmAddressbook *,
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
 * BEHAVIOR     : Returns TRUE if the given address book properties can
 * 		  be edited, FALSE if it is a "static" address book.
 * PRE          : The uri field of the GmAddressbook must exist. 
 */
gboolean gnomemeeting_addressbook_is_editable (GmAddressbook *);

	
/* DESCRIPTION  : /
 * BEHAVIOR     : Creates the initial addressbooks if none are found, 
 * 		  do nothing otherwise.
 * PRE          : Non-Null group name and address book name.
 */
void gnomemeeting_addressbook_init (gchar *, gchar *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns TRUE if the address book is able to give information
 * 		  about the given field.
 * PRE          : Non-Null address book.
 */
gboolean gnomemeeting_addressbook_has_fullname (GmAddressbook *);
gboolean gnomemeeting_addressbook_has_url (GmAddressbook *);
gboolean gnomemeeting_addressbook_has_speeddial (GmAddressbook *);
gboolean gnomemeeting_addressbook_has_categories (GmAddressbook *);
gboolean gnomemeeting_addressbook_has_location (GmAddressbook *);
gboolean gnomemeeting_addressbook_has_comment (GmAddressbook *);
gboolean gnomemeeting_addressbook_has_software (GmAddressbook *);
gboolean gnomemeeting_addressbook_has_email (GmAddressbook *);
gboolean gnomemeeting_addressbook_has_state (GmAddressbook *);

G_END_DECLS


/* supplementary apis */
#define _GM_CONTACTS_H_INSIDE__
#include "gmcontacts-dnd.h"
#undef _GM_CONTACTS_H_INSIDE__

#endif

