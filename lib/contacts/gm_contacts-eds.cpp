
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


#include <string.h>

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

  contact = g_new (GmContact, 1);

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

  return contact;
}


void
gm_contact_delete (GmContact *contact)
{
  if (!contact)
    return;

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


GmAddressbook *
gm_addressbook_new ()
{
  GmAddressbook *addressbook = NULL;

  ESource *source = NULL;

  source = e_source_new ("", "");

  addressbook = g_new (GmAddressbook, 1);

  addressbook->uid = 
    g_strdup ((const gchar *) e_source_peek_uid (source));

  g_object_unref (source);

  return addressbook;
}


void 
gm_addressbook_delete (GmAddressbook *addressbook)
{
  if (!addressbook)
    return;

  g_free (addressbook->uid);
  g_free (addressbook->name);

  g_free (addressbook);  
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
                                       gchar *fullname,
                                       gchar *url,
                                       gchar *categorie)
{
  EBook *ebook = NULL;
  EBookQuery *query = NULL;
  EVCardAttribute *attr = NULL;

  GmContact *contact = NULL;

  GSList *contacts = NULL;  
  GList *list = NULL;
  GList *l = NULL;
  GList *attr_list = NULL;
  GList *attr_list_iter = NULL;
  GList *attr_param_list = NULL;
  GList *attr_param_list_iter = NULL;
  GList *param_values = NULL;
  GList *param_values_iter = NULL;
  GList *x = NULL;

  g_return_val_if_fail (addressbook != NULL, NULL);


  ebook = e_book_new ();

  if (e_book_load_uri (ebook, addressbook->uid, FALSE, NULL)) {

    /* Build the filter */ 
    if (fullname && strcmp (fullname, ""))
      query = e_book_query_field_test (E_CONTACT_FULL_NAME,
                                       E_BOOK_QUERY_CONTAINS,
                                       fullname);
    else if (url && strcmp (url, ""))
      query = e_book_query_field_test (E_CONTACT_VIDEO_URL,
                                       E_BOOK_QUERY_CONTAINS,
                                       url);
    else if (categorie && strcmp (categorie, ""))
      query = e_book_query_field_test (E_CONTACT_CATEGORIES,
                                       E_BOOK_QUERY_IS,
                                       categorie);
    else
      query = e_book_query_field_exists (E_CONTACT_UID);
    
    /* Get the contacts for that fitler */
    if (e_book_get_contacts (ebook, query, &list, NULL)) {

      l = list;
      while (l) {

        contact = gm_contact_new ();

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
       
        attr_list = e_contact_get_attributes (E_CONTACT (l->data),
                                              E_CONTACT_PHONE_TELEX);
        attr_list_iter = attr_list;
        while (attr_list_iter && !contact->speeddial) {

          attr_param_list = e_vcard_attribute_get_params ((EVCardAttribute *) attr_list_iter->data);
          attr_param_list_iter = attr_param_list;
          while (attr_param_list_iter && !contact->speeddial) {

            param_values = e_vcard_attribute_param_get_values ((EVCardAttributeParam *) attr_param_list_iter->data);
            param_values_iter = param_values;

            while (param_values_iter && !contact->speeddial) {


              if (param_values_iter->data 
                  && !strcmp ((char *) param_values_iter->data, "X-GNOMEMEETING-SPEEDDIAL")) {

                x = e_vcard_attribute_get_values ((EVCardAttribute *) attr_list_iter->data);

                if (x && x->data) 
                  contact->speeddial = g_strdup ((char *) x->data);
              }

              param_values_iter = g_list_next (param_values_iter);
            }
            
            g_list_foreach (param_values, (GFunc) g_free, NULL);
            g_list_free (param_values);

            attr_param_list_iter = g_list_next (attr_param_list_iter);
          }
          

          g_list_free (attr_param_list);

          attr_list_iter = g_list_next (attr_list_iter);
        }

        g_list_free (attr_list);
        

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
  EVCardAttribute *attr = NULL;

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
    if (ctact->speeddial) {

      attr = e_vcard_attribute_new (NULL, "TEL");
      e_vcard_attribute_add_param_with_value (attr, 
                                              e_vcard_attribute_param_new ("TYPE"),
                                              "X-GNOMEMEETING-SPEEDDIAL");
      e_vcard_attribute_add_value (attr, ctact->speeddial);

      e_vcard_add_attribute (E_VCARD (contact), attr);
    }

    if (e_book_add_contact (ebook, contact, NULL))
      return TRUE;
  }

  return FALSE;
}


gboolean
gnomemeeting_addressbook_modify_contact (GmAddressbook *addressbook,
                                         GmContact *ctact)
{
  EBook *ebook = NULL;

  EContact *contact = NULL;
  EVCardAttribute *attr = NULL;

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
    if (ctact->speeddial) {

      attr = e_vcard_attribute_new (NULL, "TEL");
      e_vcard_attribute_add_param_with_value (attr, 
                                              e_vcard_attribute_param_new ("TYPE"),
                                              "X-GNOMEMEETING-SPEEDDIAL");
      e_vcard_attribute_add_value (attr, ctact->speeddial);

      e_vcard_add_attribute (E_VCARD (contact), attr);
    }

    if (e_book_commit_contact (ebook, contact, NULL))
      return TRUE;
  }
  return FALSE;
}


GSList *
gnomemeeting_addressbook_get_attributes_list (GmAddressbook *addressbook)
{
}
