
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
 *                         gm_contacts-eds.c  -  description 
 *                         ---------------------------------
 *   begin                : Mon Apr 12 2004
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Functions to manage the GM Addressbook using EDS..
 *
 */

extern "C" {

#include <libebook/e-book.h>

}

#include "gm_contacts.h"


static ESourceGroup *
gnomemeeting_addressbook_get_local_source_group ()
{
  EBook *ebook = NULL;
  
  ESourceGroup *result = NULL;
  ESourceList *source_list = NULL;
  
  GSList *source_groups = NULL;
  GSList *addressbooks = NULL;
  GSList *l = NULL;
  GSList *j = NULL;

  gchar *uri = NULL;
  
  e_book_get_default_addressbook (&ebook, NULL);


  /* Get the list of possible sources */
  if (e_book_get_addressbooks (&source_list, NULL)) {

    source_groups = e_source_list_peek_groups (source_list);

    l = source_groups;
    while (l) {

      addressbooks = e_source_group_peek_sources (E_SOURCE_GROUP (l->data));
    
      j = addressbooks;
      while (j && !result) {


        uri = e_source_get_uri (E_SOURCE (j->data));
        if (g_str_has_prefix (uri, "file:"))
          result = E_SOURCE_GROUP (l->data);
        g_free (uri);

        j = g_slist_next (j);
      }
      
      g_slist_foreach (addressbooks, (GFunc) g_object_unref, NULL);
      g_slist_free (addressbooks);

      l = g_slist_next (l);
    }

    g_slist_foreach (source_groups, (GFunc) g_object_unref, NULL);
    g_slist_free (source_groups);
  }

  return result;
}


GmContact *
gm_contact_new ()
{
  GmContact *contact = NULL;
  EContact *econtact = NULL;

  contact = GM_CONTACT (g_malloc (sizeof (GmContact)));
  
  econtact = e_contact_new ();
  
  contact->fullname = NULL;
  contact->categories = NULL;
  contact->url = NULL;
  contact->speeddial = NULL;
  contact->comment = NULL;
  contact->software = NULL;
  contact->email = NULL;
  contact->state = 0;
  contact->video_capable = FALSE;
  contact->uid =  
    g_strdup ((const gchar *) e_contact_get_const (E_CONTACT (econtact), 
                                                   E_CONTACT_UID));
  g_object_unref (econtact);
}


void
gm_contact_delete (GmContact *contact)
{
  g_return_if_fail (contact != NULL);

  g_free (contact->uid);
  g_free (contact->fullname);
  g_free (contact->url);
  g_free (contact->speeddial);
  g_free (contact->categories);
  g_free (contact->comment);
  g_free (contact->software);
  g_free (contact->email);

  g_free (contact);
}


GmAddressbook *gm_addressbook_new ()
{
}


void gm_addressbook_delete (GmAddressbook *addressbook)
{
}


GSList *
gnomemeeting_get_local_addressbooks ()
{
  EBook *ebook = NULL;
  ESourceList *source_list = NULL;
  
  GSList *sources = NULL;
  GSList *groups = NULL;
  GSList *addressbooks = NULL;
  
  GSList *l = NULL;
  GSList *j = NULL;

  GmAddressbook *elmt = NULL;
  
  gchar *uri = NULL;
  
  e_book_get_default_addressbook (&ebook, NULL);

  if (e_book_get_addressbooks (&source_list, NULL)) {

    sources = e_source_list_peek_groups (source_list);
    l = sources;
    
    while (l) {
      
      groups = e_source_group_peek_sources (E_SOURCE_GROUP (l->data));

      j = groups;
      while (j) {

        uri = e_source_get_uri (E_SOURCE (j->data));

        if (g_str_has_prefix (uri, "file:")) {
  
          elmt = GM_ADDRESSBOOK (g_malloc (sizeof (GmAddressbook)));
          
          elmt->name = g_strdup (e_source_peek_name (E_SOURCE (j->data)));
          elmt->uid = g_strdup (uri); 
          
          addressbooks = g_slist_append (addressbooks, (gpointer) elmt);

        }
        j = g_slist_next (j);
      
        g_free (uri);
      }
      
      l = g_slist_next (l);
      
      g_slist_free (groups);
    }

    g_slist_free (sources);
  }

  return addressbooks;
}


GSList *
gnomemeeting_get_remote_addressbooks ()
{
  return NULL;
}


GSList *
gnomemeeting_addressbook_get_contacts (GmAddressbook *addressbook,
                                       gchar * filter)
{
  EBook *ebook = NULL;
  EBookQuery *query = NULL;

  GmContact *contact = NULL;

  GSList *contacts = NULL;  
  GList *list = NULL;
  GList *l = NULL;
  
  g_return_val_if_fail (addressbook != NULL, NULL);


  ebook = e_book_new ();

  if (e_book_load_uri (ebook, addressbook->uid, FALSE, NULL)) {

    query = e_book_query_field_exists (E_CONTACT_UID);
    if (e_book_get_contacts (ebook, query, &list, NULL)) {

      l = list;
      while (l) {

        contact = GM_CONTACT (g_malloc (sizeof (GmContact)));

        contact->uid =  
          g_strdup ((const gchar *) e_contact_get_const (E_CONTACT (l->data), 
                                                         E_CONTACT_UID));
        contact->fullname =  
          g_strdup ((const gchar *) e_contact_get_const (E_CONTACT (l->data), 
                                                         E_CONTACT_FULL_NAME));
        contact->url =  
          g_strdup ((const gchar *) e_contact_get_const (E_CONTACT (l->data), 
                                                         E_CONTACT_VIDEO_URL));
        contact->categories =  
          g_strdup ((const gchar *) e_contact_get_const (E_CONTACT (l->data), 
                                                         E_CONTACT_CATEGORIES));      
        contact->speeddial = NULL;  


        contacts = g_slist_append (contacts, (gpointer) contact);

        l = g_list_next (l);
      }

      g_list_foreach (list, (GFunc) g_object_unref, NULL);
      g_list_free (list);
    }
    e_book_query_unref (query);
  }


  return contacts;
}


gboolean 
gnomemeeting_addressbook_add (GmAddressbook *addressbook)
{
  ESource *source = NULL;
  ESourceGroup *source_group = NULL;

  g_return_val_if_fail (addressbook != NULL, FALSE);

  source = e_source_new ("", "");
  
  e_source_set_name (source, addressbook->name);
  e_source_set_relative_uri (source, e_source_peek_uid (source));

  source_group = gnomemeeting_addressbook_get_local_source_group ();
  
  e_source_group_add_source (source_group, source, -1); 
    
  return FALSE;
}


gboolean 
gnomemeeting_addressbook_delete (GmAddressbook *addressbook)
{
  GError *err = NULL;
  EBook *ebook = NULL;

  g_return_val_if_fail (addressbook != NULL, FALSE);


  ebook = e_book_new ();

  if (e_book_load_uri (ebook, addressbook->uid, FALSE, NULL)) 
    if (e_book_remove (ebook, &err)) 
      return TRUE;
    
  return FALSE;
}


gboolean 
gnomemeeting_addressbook_is_local (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, TRUE);
  
  if (g_str_has_prefix (addressbook->uid, "file:"))
    return TRUE;

  return FALSE;
}


gboolean 
gnomemeeting_addressbook_delete_contact (GmAddressbook *addressbook,
                                         GmContact *contact)
{
  GList *l = NULL;
  
  EBook *ebook = NULL;
  
  gchar *uid = NULL;

  g_return_val_if_fail (contact != NULL, FALSE);
  g_return_val_if_fail (addressbook != NULL, FALSE);


  ebook = e_book_new ();

  if (e_book_load_uri (ebook, addressbook->uid, FALSE, NULL)) {

    if (contact->uid) {
    
      l = g_list_append (l, (gpointer) contact->uid);
      e_book_remove_contacts (ebook, l, NULL);
      g_list_free (l);
    }
  }
  

  return TRUE;
};


gboolean
gnomemeeting_addressbook_add_contact (GmAddressbook *addressbook,
                                      GmContact *ctact)
{
  EBook *ebook = NULL;

  EContact *contact = NULL;
  
  g_return_val_if_fail (ctact != NULL, FALSE);
  g_return_val_if_fail (addressbook != NULL, FALSE);


  ebook = e_book_new ();

  if (e_book_load_uri (ebook, addressbook->uid, FALSE, NULL)) {

    contact = e_contact_new ();
    
    if (ctact->uid)
      e_contact_set (contact, E_CONTACT_UID, ctact->uid);
    if (ctact->fullname)
      e_contact_set (contact, E_CONTACT_FULL_NAME, ctact->fullname);
    if (ctact->url)
      e_contact_set (contact, E_CONTACT_VIDEO_URL, ctact->url);
    if (ctact->categories)
      e_contact_set (contact, E_CONTACT_CATEGORIES, ctact->categories);

    if (e_book_add_contact (ebook, contact, NULL))
      return TRUE;
  }

  return FALSE;
}


gboolean
gnomemeeting_addressbook_modify_contact (GmAddressbook *addressbook,
                                         GmContact *ctact)
{
  return FALSE;
}


GSList *
gnomemeeting_addressbook_get_attributes_list (GmAddressbook *addressbook)
{
}
