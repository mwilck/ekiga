
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


#include "gmcontacts.h"

#ifndef _
#include <libintl.h>
#define _(x) gettext(x)
#ifdef gettext_noop
#define N_(String) gettext_noop (String)
#else
#define N_(String) (String)
#endif
#endif

#ifndef _GM_CONTACTS_H_INSIDE__
#define _GM_CONTACTS_H_INSIDE__
#include "gmcontacts-local.h"
#include "gmcontacts-remote.h"
#undef _GM_CONTACTS_H_INSIDE__
#endif

#include <string.h>
#include "toolbox.h"

/* helper functions */

static gboolean
stringlist_contains (const GSList *,
                     const gchar *);

static GSList *
stringlist_remove (const GSList *,
                   const gchar *);

static GSList *
stringlist_add (const GSList *,
                const gchar *);

static gchar *
stringlist_dump (const GSList *,
                 const gchar);

/* implementation */
/* helper functions */

static gboolean
stringlist_contains (const GSList *stringlist,
                     const gchar *string)
{
  GSList *stringlist_iter = NULL;

  g_return_val_if_fail (string != NULL, FALSE);

  if (!stringlist)
    return FALSE;

  stringlist_iter = (GSList *) stringlist;
  while (stringlist_iter)
    {
      if (stringlist_iter->data &&
          !strcmp ((const char *) stringlist_iter->data,
                   (const char *) string))
        return TRUE;
      stringlist_iter = g_slist_next (stringlist_iter);
    }
  return FALSE;
}


static GSList *
stringlist_remove (const GSList *list,
                   const gchar *string)
{
  GSList *new_list = NULL;
  GSList *list_iter = NULL;
  gboolean leave = FALSE;
  gboolean last_element = FALSE;

  if (!list)
    return NULL;
  if (!string)
    return (GSList *) list;

  if (!stringlist_contains (list, string))
    return (GSList *) list;

  new_list = (GSList *) list;

  while (!leave) {
    list_iter = new_list;
    last_element = (g_slist_length (list_iter) == 1);
    while (list_iter) {
      if (list_iter->data &&
          !strcmp ((const char *) list_iter->data,
                   string)) {
        g_free (list_iter->data);
        new_list = g_slist_delete_link (new_list,
                                        list_iter);
        if (last_element) new_list = NULL;
        break;
      }
      list_iter = g_slist_next (list_iter);
    }
    if (!stringlist_contains (list, string))
      leave = TRUE;
  }

  return new_list;
}


static GSList *
stringlist_add (const GSList *list,
                const gchar *string)
{
  if (!string)
    return (GSList *) list;

  return
    g_slist_append ((GSList *) list,
                    (gpointer) g_strdup (string));
}


static gchar *
stringlist_dump (const GSList *list,
                 gchar separator)
{
  GSList *list_iter = NULL;
  gchar *tmpstr[2] = { NULL, NULL };

  g_return_val_if_fail (separator != '\0', NULL);

  if (!list)
    return g_strdup ("");

  list_iter = (GSList *) list;
  while (list_iter) {
    if (list_iter->data) {
      if (tmpstr[0]) {
        tmpstr[1] = tmpstr[0];
        tmpstr[0] = g_strdup_printf ("%s%c%s",
                                     tmpstr[1],
                                     separator,
                                     (const gchar *) list_iter->data);
        g_free (tmpstr[1]);
      }
      else
        tmpstr[0] = g_strdup ((const gchar *) list_iter->data);
    }
    list_iter = g_slist_next (list_iter);
  }
  return tmpstr[0];
}

/* API */

GSList *
gmcontact_enum_categories (const GmContact * contact)
{
  GSList * categorylist = NULL;
  gchar ** split_categories = { NULL };
  gchar ** split_categories_iter = { NULL };

  g_return_val_if_fail (contact != NULL, NULL);

  if (!contact->categories)
    return NULL;

  if (g_ascii_strcasecmp (contact->categories, "") == 0)
    return NULL;

  split_categories =
    g_strsplit_set (contact->categories,
		    ",",
		    -1);

  for (split_categories_iter = split_categories;
       *split_categories_iter != NULL;
       split_categories_iter++) {
    if (g_ascii_strcasecmp (*split_categories_iter,"") != 0)
      categorylist =
	g_slist_append (categorylist, g_strdup (*split_categories_iter));
  }
  
  g_strfreev (split_categories);

  categorylist = gm_string_gslist_remove_dups (categorylist);

  return categorylist;
}


gboolean
gmcontact_is_in_category (const GmContact * contact,
			  const gchar * category)
{
  GSList * categorylist = NULL;
  GSList * categorylist_iter = NULL;
  
  g_return_val_if_fail (contact != NULL, FALSE);

  categorylist = gmcontact_enum_categories (contact);

  if (!categorylist)
    return FALSE;

  for (categorylist_iter = categorylist;
       categorylist_iter != NULL;
       categorylist_iter = g_slist_next (categorylist_iter))
    {
      if (g_ascii_strcasecmp ((const gchar*) categorylist_iter->data,
			      category) == 0)
	{
	  g_slist_free (categorylist);
	  return TRUE;
	}
    }

  g_slist_free (categorylist);
  return FALSE;
}


GSList *
gnomemeeting_addressbook_get_contacts (GmAddressbook *addressbook,
				       int &nbr,
				       gboolean partial_match,
				       gchar *fullname,
				       gchar *url,
				       gchar *categorie,
				       gchar *location,
				       gchar *speeddial)
{
  if (addressbook && !gnomemeeting_addressbook_is_local (addressbook)) 
    return gnomemeeting_remote_addressbook_get_contacts (addressbook,
							 nbr,
							 partial_match,
							 fullname,
							 url,
							 categorie,
							 location,
							 speeddial);
  else
    return gnomemeeting_local_addressbook_get_contacts (addressbook,
							nbr,
							partial_match,
							fullname,
							url, 
							categorie,
							location,
							speeddial);
}


GSList *
gnomemeeting_local_addressbook_enum_categories (GmAddressbook *addressbook)
{
  GSList *contactlist = NULL;
  GSList *contactlist_iter = NULL;
  GSList *grouplist = NULL;
  GSList *contact_grouplist = NULL;

  GSList *predefined_categories = NULL;

  gint nbr = 0;

  /* this only makes sense on a local addressbook */
  if (addressbook &&
      !gnomemeeting_addressbook_is_local (addressbook))
    return NULL;

  /* get all contacts of that addressbook (or all if NULL)  */
  contactlist =
    gnomemeeting_addressbook_get_contacts (addressbook,
					   nbr,
					   FALSE,
					   NULL, NULL, NULL, NULL, NULL);

  predefined_categories = 
    g_slist_append (predefined_categories,
                    g_strdup (_("Friends")));
  predefined_categories = 
    g_slist_append (predefined_categories,
                    g_strdup (_("Colleagues")));
  predefined_categories = 
    g_slist_append (predefined_categories,
                    g_strdup (_("Family")));

  if (!contactlist)
    return predefined_categories;

  for (contactlist_iter = contactlist;
       contactlist_iter != NULL;
       contactlist_iter = g_slist_next (contactlist_iter)) {

    if (contactlist_iter->data) {

      /* get all categories from that contact */
      contact_grouplist =
        gmcontact_enum_categories ((const GmContact*) contactlist_iter->data);
      grouplist = g_slist_concat (grouplist, contact_grouplist);
    }
  }

  grouplist = g_slist_concat (grouplist, predefined_categories);

  g_slist_foreach (contactlist, (GFunc) gmcontact_delete, NULL);
  g_slist_free (contactlist);

  grouplist =  gm_string_gslist_remove_dups (grouplist);

  return grouplist;
}


gboolean
gnomemeeting_local_addressbooks_rename_category (const gchar *fromname,
						 const gchar *toname)
{
  GSList *addressbooks = NULL;
  GSList *addressbooks_iter = NULL;
  GSList *contacts = NULL;
  GSList *contacts_iter = NULL;
  GSList *grouplist = NULL;
  gint nbr = 0;
  GmAddressbook *abook = NULL;
  GmContact *contact = NULL;

  g_return_val_if_fail (fromname != NULL && toname != NULL, FALSE);

  addressbooks = gnomemeeting_get_local_addressbooks ();

  addressbooks_iter = gnomemeeting_get_local_addressbooks ();
  while (addressbooks_iter) {
    if (addressbooks_iter->data) {

      abook = (GmAddressbook *) addressbooks_iter->data;
      contacts =
	gnomemeeting_local_addressbook_get_contacts (abook, nbr, TRUE,
						     NULL, NULL, NULL,
						     NULL, NULL);
      contacts_iter = contacts;
      while (contacts_iter) {
	if (contacts_iter->data) {
	  contact = (GmContact *) contacts_iter->data;
	  grouplist = gmcontact_enum_categories (contact);
	  if (stringlist_contains (grouplist, fromname)) {
	    grouplist = stringlist_remove (grouplist, fromname);
	    grouplist = stringlist_add (grouplist, toname);
	    if (contact->categories)
	      g_free (contact->categories);
	    contact->categories = stringlist_dump (grouplist, ',');
	    (void) gnomemeeting_addressbook_modify_contact (abook, contact);
	  }
	  g_slist_foreach (grouplist, (GFunc) g_free, NULL);
	  g_slist_free (grouplist);
	}
        contacts_iter = g_slist_next (contacts_iter);
      } /* while (contacts_iter) */
      g_slist_foreach (contacts, (GFunc) gmcontact_delete, NULL);
      g_slist_free (contacts);
    }
    addressbooks_iter = g_slist_next (addressbooks_iter);
  } /* while (addressbooks_iter) */
  g_slist_foreach (addressbooks, (GFunc) gm_addressbook_delete, NULL);
  g_slist_free (addressbooks);

  return TRUE;
}


gboolean
gnomemeeting_local_addressbooks_delete_category (const gchar *groupname)
{
  GSList *addressbooks = NULL;
  GSList *addressbooks_iter = NULL;
  GSList *contacts = NULL;
  GSList *contacts_iter = NULL;
  GSList *grouplist = NULL;
  gint nbr = 0;
  GmAddressbook *abook = NULL;
  GmContact *contact = NULL;

  g_return_val_if_fail (groupname != NULL, FALSE);

  addressbooks = gnomemeeting_get_local_addressbooks ();

  addressbooks_iter = gnomemeeting_get_local_addressbooks ();
  while (addressbooks_iter) {
    if (addressbooks_iter->data) {

      abook = (GmAddressbook *) addressbooks_iter->data;
      contacts =
	gnomemeeting_local_addressbook_get_contacts (abook, nbr, TRUE,
						     NULL, NULL, NULL,
						     NULL, NULL);
      contacts_iter = contacts;
      while (contacts_iter) {
        if (contacts_iter->data) {
          contact = (GmContact *) contacts_iter->data;
	  grouplist = gmcontact_enum_categories (contact);
	  if (stringlist_contains (grouplist, groupname)) {
	    grouplist = stringlist_remove (grouplist, groupname);
	    if (contact->categories)
	      g_free (contact->categories);
	    contact->categories = stringlist_dump (grouplist, ',');
	    (void) gnomemeeting_addressbook_modify_contact (abook, contact);
	  }
	  g_slist_foreach (grouplist, (GFunc) g_free, NULL);
	  g_slist_free (grouplist);
        }
	contacts_iter = g_slist_next (contacts_iter);
      } /* while (contacts_iter) */
      g_slist_foreach (contacts, (GFunc) gmcontact_delete, NULL);
      g_slist_free (contacts);
    }
    addressbooks_iter = g_slist_next (addressbooks_iter);
  } /* while (addressbooks_iter) */
  g_slist_foreach (addressbooks, (GFunc) gm_addressbook_delete, NULL);
  g_slist_free (addressbooks);

  return TRUE;
}


GmAddressbook *
gnomemeeting_local_addressbook_get_by_contact (GmContact * contact)
{
  GSList *local_addressbooks = NULL;
  GSList *local_addressbooks_iter = NULL;

  GSList *addressbook_contactlist = NULL;
  GSList *addressbook_contactlist_iter = NULL;

  GmAddressbook *tmp_addressbook = NULL;
  GmAddressbook *found_addressbook = NULL;

  GmContact *tmp_contact = NULL;

  int nbr = 0;

  g_return_val_if_fail (contact != NULL, NULL);

  local_addressbooks =
    gnomemeeting_get_local_addressbooks ();

  local_addressbooks_iter = local_addressbooks;
  while (local_addressbooks_iter) {

    /* iter through all local addressbooks */
    if (local_addressbooks_iter->data) {

      tmp_addressbook = (GmAddressbook*) local_addressbooks_iter->data;
      addressbook_contactlist =
        gnomemeeting_addressbook_get_contacts (tmp_addressbook,
                                               nbr,
                                               TRUE,
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL);

      addressbook_contactlist_iter = addressbook_contactlist;
      while (addressbook_contactlist_iter) {

        /* iter through all contacts of that addressbook */
        if (addressbook_contactlist_iter->data) {

          tmp_contact =
            (GmContact*) addressbook_contactlist_iter->data;

          /* strcmp fuxxors when you give a NULL */
          if (contact->uid &&
              tmp_contact->uid &&
              !strcmp ((const char*) contact->uid,
                       (const char*) tmp_contact->uid)) {

            found_addressbook = gm_addressbook_copy (tmp_addressbook);
          }
          if (found_addressbook) 
            break;
        }
        addressbook_contactlist_iter =
          g_slist_next (addressbook_contactlist_iter);
      }
      
      /* free the list of contacts */
      g_slist_foreach (addressbook_contactlist, (GFunc) gmcontact_delete, NULL);
      g_slist_free (addressbook_contactlist);
      
      addressbook_contactlist = NULL;

      if (found_addressbook) 
        break;
    }

    local_addressbooks_iter = g_slist_next (local_addressbooks_iter);
  }

  /* free the list of addressbooks */
  g_slist_foreach (local_addressbooks, (GFunc) gm_addressbook_delete, NULL);
  g_slist_free (local_addressbooks);

  return found_addressbook;
}


GmContact *
gnomemeeting_local_contact_get_by_uid (gchar *uid)
{
  GSList *labooks = NULL;
  GSList *labooks_iter = NULL;
  GSList *contacts = NULL;
  GSList *contacts_iter = NULL;
  GmAddressbook *abook = NULL;
  GmContact *contact = NULL;
  GmContact *found_contact = NULL;
  int nbr = 0;

  g_return_val_if_fail (uid != NULL, NULL);

  labooks = gnomemeeting_get_local_addressbooks ();
  labooks_iter = labooks;

  while (labooks_iter) {
    if (labooks_iter->data) {
      abook = (GmAddressbook*) labooks_iter->data;
      contacts = gnomemeeting_addressbook_get_contacts (abook,
							nbr,
							FALSE,
							NULL,
							NULL,
							NULL,
							NULL,
							NULL);
      contacts_iter = contacts;
      while (contacts_iter) {
	if (contacts_iter->data) {
	  contact = (GmContact*) contacts_iter->data;
	  if (contact->uid &&
	      !strcmp (contact->uid,
		       uid)) {
	    found_contact = gmcontact_copy (contact);
	  }
	}
	if (found_contact) break;
	contacts_iter = g_slist_next (contacts_iter);
      }
      g_slist_foreach (contacts, (GFunc) gmcontact_delete, NULL);
      g_slist_free (contacts);
    }
    if (found_contact) break;
    labooks_iter = g_slist_next (labooks_iter);
  }
  g_slist_foreach (labooks, (GFunc) gm_addressbook_delete, NULL);
  g_slist_free (labooks);

  return found_contact;
}


gboolean 
gnomemeeting_addressbook_add (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_add (addressbook);
  else
    return gnomemeeting_remote_addressbook_add (addressbook);
}


gboolean 
gnomemeeting_addressbook_delete (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_delete (addressbook);
  else
    return gnomemeeting_remote_addressbook_delete (addressbook);
}


gboolean 
gnomemeeting_addressbook_modify (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_modify (addressbook);
  else
    return gnomemeeting_remote_addressbook_modify (addressbook);
}


gboolean 
gnomemeeting_addressbook_is_local (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, TRUE);

  if (addressbook->url == NULL)
    return TRUE; 

  if (addressbook->url 
      && g_str_has_prefix (addressbook->url, "file:"))
    return TRUE;


  return FALSE;
}


gboolean
gnomemeeting_addressbook_add_contact (GmAddressbook *addressbook,
                                      GmContact *ctact)
{
  g_return_val_if_fail (ctact != NULL, FALSE);
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_add_contact (addressbook, ctact);

  return FALSE;
}


gboolean
gnomemeeting_addressbook_delete_contact (GmAddressbook *addressbook,
					 GmContact *ctact)
{
  g_return_val_if_fail (ctact != NULL, FALSE);
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_delete_contact (addressbook, ctact);

  return FALSE;
}


gboolean
gnomemeeting_addressbook_modify_contact (GmAddressbook *addressbook,
					 GmContact *ctact)
{
  g_return_val_if_fail (ctact != NULL, FALSE);
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_modify_contact (addressbook, ctact);

  return FALSE;
}


gboolean
gnomemeeting_addressbook_is_editable (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_is_editable (addressbook);
  else
    return gnomemeeting_remote_addressbook_is_editable (addressbook);
}


void
gnomemeeting_addressbook_init (gchar *group_name, 
			       gchar *addressbook_name)
{
  g_return_if_fail (group_name != NULL && addressbook_name != NULL);
  
  gnomemeeting_local_addressbook_init (group_name, addressbook_name);
  gnomemeeting_remote_addressbook_init ();
}


gboolean 
gnomemeeting_addressbook_has_fullname (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_has_fullname (addressbook);
  else
    return gnomemeeting_remote_addressbook_has_fullname (addressbook);
}


gboolean 
gnomemeeting_addressbook_has_url (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_has_url (addressbook);
  else
    return gnomemeeting_remote_addressbook_has_url (addressbook);
}


gboolean 
gnomemeeting_addressbook_has_speeddial (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_has_speeddial (addressbook);
  else
    return gnomemeeting_remote_addressbook_has_speeddial (addressbook);
}


gboolean 
gnomemeeting_addressbook_has_categories (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_has_categories (addressbook);
  else
    return gnomemeeting_remote_addressbook_has_categories (addressbook);
}


gboolean 
gnomemeeting_addressbook_has_location (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_has_location (addressbook);
  else
    return gnomemeeting_remote_addressbook_has_location (addressbook);
}


gboolean 
gnomemeeting_addressbook_has_comment (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_has_comment (addressbook);
  else
    return gnomemeeting_remote_addressbook_has_comment (addressbook);
}


gboolean 
gnomemeeting_addressbook_has_software (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_has_software (addressbook);
  else
    return gnomemeeting_remote_addressbook_has_software (addressbook);
}


gboolean 
gnomemeeting_addressbook_has_email (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_has_email (addressbook);
  else
    return gnomemeeting_remote_addressbook_has_email (addressbook);
}


gboolean 
gnomemeeting_addressbook_has_state (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if (gnomemeeting_addressbook_is_local (addressbook))
    return gnomemeeting_local_addressbook_has_state (addressbook);
  else
    return gnomemeeting_remote_addressbook_has_state (addressbook);
}
