
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


#include <lib/gm_conf.h>
#include "gm_contacts.h"
#ifndef _GM_CONTACTS_H_INSIDE__
#define _GM_CONTACTS_H_INSIDE__
#include "gm_contacts-local.h"
#undef _GM_CONTACTS_H_INSIDE__
#endif


static ESourceGroup *
gnomemeeting_addressbook_get_local_source_group (ESourceList **source_list)
{
  ESourceGroup *result = NULL;
  
  GSList *source_groups = NULL;
  GSList *addressbooks = NULL;
  GSList *l = NULL;
  GSList *j = NULL;

  gchar *uri = NULL;
  
  /* Get the list of possible sources */
  if (e_book_get_addressbooks (source_list, NULL)) {

    source_groups = e_source_list_peek_groups (*source_list);

    l = source_groups;
    while (l) {

      addressbooks = e_source_group_peek_sources (E_SOURCE_GROUP (l->data));
    
      j = addressbooks;
      while (j) {


        uri = e_source_get_uri (E_SOURCE (j->data));
        if (g_str_has_prefix (uri, "file:"))
          result = E_SOURCE_GROUP (l->data);
        g_free (uri);

        j = g_slist_next (j);
      }


      l = g_slist_next (l);
    }

  }
  
  return result;
}


static EVCardAttributeParam *
gm_addressbook_get_contact_speeddial_param (EContact *contact)
{
  EVCardAttributeParam *attr_param = NULL;

  const char *attr_param_name = NULL;
  
  GList *attr_list = NULL;
  GList *attr_list_iter = NULL;
  GList *attr_param_list = NULL;
  GList *attr_param_list_iter = NULL;

  attr_list = e_vcard_get_attributes (E_VCARD (contact));

  attr_list_iter = attr_list;
  while (attr_list_iter && !attr_param) {

    attr_param_list = 
      e_vcard_attribute_get_params ((EVCardAttribute *) attr_list_iter->data);
    attr_param_list_iter = attr_param_list;
    while (attr_param_list_iter && !attr_param) {

      attr_param_name = 
	e_vcard_attribute_param_get_name ((EVCardAttributeParam *) attr_param_list_iter->data);
      
      if (attr_param_name 
	  && !strcmp (attr_param_name, "X-GNOMEMEETING-SPEEDDIAL")) {

	attr_param = (EVCardAttributeParam *) attr_param_list_iter->data;
      }

      attr_param_list_iter = g_list_next (attr_param_list_iter);
    }


    attr_list_iter = g_list_next (attr_list_iter);
  }

  
  return attr_param;
}


static const char *
gm_addressbook_get_contact_speeddial (EContact *contact)
{
  EVCardAttributeParam *attr_param = NULL;

  const char *speeddial = NULL;
  const char *attr_param_name = NULL;
  
  GList *x = NULL;

  GList *attr_list = NULL;
  GList *attr_list_iter = NULL;
  GList *attr_param_list = NULL;
  GList *attr_param_list_iter = NULL;

  attr_list = e_contact_get_attributes (E_CONTACT (contact),
					E_CONTACT_PHONE_TELEX);
  attr_list_iter = attr_list;
  while (attr_list_iter && !attr_param) {

    attr_param_list = 
      e_vcard_attribute_get_params ((EVCardAttribute *) attr_list_iter->data);
    attr_param_list_iter = attr_param_list;
    while (attr_param_list_iter && !attr_param) {

      attr_param_name = 
	e_vcard_attribute_param_get_name ((EVCardAttributeParam *) attr_param_list_iter->data);
      
      if (attr_param_name 
	  && !strcmp (attr_param_name, "X-GNOMEMEETING-SPEEDDIAL")) {

	attr_param = (EVCardAttributeParam *) attr_param_list_iter->data;
      }

      attr_param_list_iter = g_list_next (attr_param_list_iter);
    }


    attr_list_iter = g_list_next (attr_list_iter);
  }

  if (attr_param) {

    x = e_vcard_attribute_param_get_values (attr_param);
    if (x && x->data)
      speeddial = g_strdup ((char *) x->data);

  }
  
  return speeddial;
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
  contact->location = NULL;
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

  ESourceList *list = NULL;
  ESourceGroup *source_group = NULL;
  ESource *source = NULL;

  addressbook = g_new (GmAddressbook, 1);
  
  addressbook->name = NULL;
  addressbook->url = NULL; 
  addressbook->aid = NULL;
  addressbook->call_attribute = NULL;

  source = e_source_new ("", "");
  source_group = gnomemeeting_addressbook_get_local_source_group (&list);

  if (source_group) {
  
    e_source_set_relative_uri (source, e_source_peek_uid (source));
    e_source_set_group (source, source_group);
    addressbook->name = NULL;
    addressbook->url = e_source_get_uri (source); 
    addressbook->aid = g_strdup (e_source_peek_uid (source));
    addressbook->call_attribute = NULL;
  }

  g_object_unref (source);

  return addressbook;
}


void 
gm_addressbook_delete (GmAddressbook *addressbook)
{
  if (!addressbook)
    return;

  g_free (addressbook->url);
  g_free (addressbook->aid);
  g_free (addressbook->name);
  g_free (addressbook->call_attribute);

  g_free (addressbook);  
}


GSList *
gnomemeeting_get_local_addressbooks ()
{
  ESourceList *source_list = NULL;

  GSList *sources = NULL;
  GSList *groups = NULL;
  GSList *addressbooks = NULL;

  GSList *l = NULL;
  GSList *j = NULL;

  GmAddressbook *elmt = NULL;

  gchar *uri = NULL;
  gchar *aid = NULL;


  if (e_book_get_addressbooks (&source_list, NULL)) {

    sources = e_source_list_peek_groups (source_list);
    l = sources;

    while (l) {

      groups = e_source_group_peek_sources (E_SOURCE_GROUP (l->data));

      j = groups;
      while (j) {

        aid = (gchar *) e_source_peek_uid (E_SOURCE (j->data));
	uri = (gchar *) e_source_get_uri (E_SOURCE (j->data));

        if (g_str_has_prefix (uri, "file:")) {

          elmt = gm_addressbook_new ();

          elmt->name = g_strdup (e_source_peek_name (E_SOURCE (j->data)));
          elmt->aid = g_strdup (aid); 
          elmt->url = g_strdup (uri); 

          addressbooks = g_slist_append (addressbooks, (gpointer) elmt);

        }
        j = g_slist_next (j);

        g_free (uri);
	g_free (aid);
      }

      l = g_slist_next (l);
    }

  }

  return addressbooks;
}


GSList *
gnomemeeting_local_addressbook_get_contacts (GmAddressbook *addbook,
					     int & nbr,
					     gboolean partial_match,
					     gchar *fullname,
					     gchar *url,
					     gchar *categorie,
					     gchar *speeddial)
{
  EBook *ebook = NULL;
  EBookQuery *query = NULL;
  EBookQuery *queries [4];
  
  GmContact *contact = NULL;
  GmAddressbook *addressbook = NULL;

  GSList *contacts = NULL;
  GSList *addressbooks = NULL;
  GSList *addressbooks_iter = NULL;
  
  GList *list = NULL;
  GList *l = NULL;

  gint cpt = 0;

  if (addbook) 
    addressbooks = g_slist_append (addressbooks, (gpointer) addbook);
  else
    addressbooks = gnomemeeting_get_local_addressbooks ();

  /* Build the filter */ 
  if (fullname && strcmp (fullname, "")) 
    queries [cpt++] = 
      e_book_query_field_test (E_CONTACT_FULL_NAME,
			       partial_match?
			       E_BOOK_QUERY_CONTAINS
			       :
			       E_BOOK_QUERY_IS,
			       fullname);

  if (url && strcmp (url, "")) 
    queries [cpt++] = 
      e_book_query_field_test (E_CONTACT_VIDEO_URL,
			       partial_match?
			       E_BOOK_QUERY_CONTAINS
			       :
			       E_BOOK_QUERY_IS,
			       url);
  if (categorie && strcmp (categorie, ""))
    queries [cpt++] = 
      e_book_query_field_test (E_CONTACT_CATEGORY_LIST,
			       partial_match?
			       E_BOOK_QUERY_CONTAINS
			       :
			       E_BOOK_QUERY_IS,
			       categorie);

  if (cpt == 0)
    queries [cpt++] = e_book_query_field_exists (E_CONTACT_UID);

  query = e_book_query_or (cpt, queries, TRUE);

  addressbooks_iter = addressbooks;
  while (addressbooks_iter) {

    addressbook = GM_ADDRESSBOOK (addressbooks_iter->data);
    if ((ebook = e_book_new_from_uri (addressbook->url, NULL))) {

      if (e_book_open (ebook, FALSE, NULL)) {
	
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
	    contact->email =  
	      g_strdup ((const gchar *) e_contact_get_const (E_CONTACT (l->data), 
							     E_CONTACT_EMAIL_1));
	    contact->categories =  
	      g_strdup ((const gchar *) e_contact_get_const (E_CONTACT (l->data), 
							     E_CONTACT_CATEGORIES));      

	    contact->speeddial = 
	      (gchar *) 
	      gm_addressbook_get_contact_speeddial (E_CONTACT (l->data));


	    /* If it is a search on a speed dial, then we only add
	     * the contact to the list if it has the correct speed dial
	     */
	    if ((speeddial 
		 && ((contact->speeddial && strcmp (speeddial, "") && 
		      !strcmp (speeddial, contact->speeddial)) 
		     || (!strcmp (speeddial, "*") && contact->speeddial
			 && strcmp (contact->speeddial, "")))
		 || !speeddial))
	      contacts = g_slist_append (contacts, (gpointer) contact);

	    l = g_list_next (l);
	  }


	  g_list_foreach (list, (GFunc) g_object_unref, NULL);
	  g_list_free (list);
	}
      }
    }
    
    addressbooks_iter = g_slist_next (addressbooks_iter);
  }
  
  
  e_book_query_unref (query);

  if (!addbook) {
    
    g_slist_foreach (addressbooks, (GFunc) gm_addressbook_delete, NULL);
    g_slist_free (addressbooks);
  }
  

  /* No hidden contacts in a local address book */
  nbr = g_slist_length (contacts);
  
  return contacts;
}


gboolean 
gnomemeeting_local_addressbook_add (GmAddressbook *addressbook)
{
  ESourceList *list = NULL;
  ESource *source = NULL;
  ESourceGroup *source_group = NULL;

  g_return_val_if_fail (addressbook != NULL, FALSE);

  source_group = gnomemeeting_addressbook_get_local_source_group (&list);
  
  source = e_source_new ("", "");

  e_source_set_name (source, addressbook->name);
  e_source_set_relative_uri (source, e_source_peek_uid (source));
  e_source_set_group (source, source_group);

  if (addressbook->aid) {
    
    g_free (addressbook->aid);
  }
  addressbook->aid = g_strdup (e_source_peek_uid (E_SOURCE (source)));
  addressbook->url = e_source_get_uri (source);

  e_source_group_add_source (source_group, source, -1); 

  e_source_list_sync (list, NULL);
 
  return TRUE;
}


gboolean 
gnomemeeting_local_addressbook_delete (GmAddressbook *addressbook)
{
  ESourceList *list = NULL;
  ESourceGroup *source_group = NULL;

  g_return_val_if_fail (addressbook != NULL, FALSE);

  source_group = gnomemeeting_addressbook_get_local_source_group (&list);

  if (addressbook->aid) {
    
    if (e_source_group_remove_source_by_uid (source_group, 
					     addressbook->aid)) 
      if (e_source_list_sync (list, NULL))
	return TRUE;
  }
  
  return FALSE;
}


gboolean
gnomemeeting_local_addressbook_modify (GmAddressbook *addressbook)
{
  ESourceList *list = NULL;
  ESourceGroup *source_group = NULL;
  ESource *source = NULL;

  g_return_val_if_fail (addressbook != NULL, FALSE);

  source_group = gnomemeeting_addressbook_get_local_source_group (&list);

  if (addressbook->aid) {
    
    source = e_source_group_peek_source_by_uid (source_group, addressbook->aid);

    if (addressbook->name && strcmp (addressbook->name, "")) {
      
      e_source_set_name (source, addressbook->name);

      if (e_source_list_sync (list, NULL))
	return TRUE;
    }
  }
  
  return FALSE;
}


gboolean
gnomemeeting_local_addressbook_add_contact (GmAddressbook *addressbook,
					    GmContact *ctact)
{
  GError *error = NULL;
  EBook *ebook = NULL;

  EContact *contact = NULL;
  EVCardAttribute *attr = NULL;
  EVCardAttributeParam *param = NULL;

  g_return_val_if_fail (ctact != NULL, FALSE);
  g_return_val_if_fail (addressbook != NULL, FALSE);

  if ((ebook = e_book_new_from_uri (addressbook->url, NULL))) {

    if (e_book_open (ebook, FALSE, &error)) {

      contact = e_contact_new ();

      if (ctact->uid)
	e_contact_set (contact, E_CONTACT_UID, ctact->uid);
      if (ctact->fullname)
	e_contact_set (contact, E_CONTACT_FULL_NAME, ctact->fullname);
      if (ctact->url)
	e_contact_set (contact, E_CONTACT_VIDEO_URL, ctact->url);
      if (ctact->email)
	e_contact_set (contact, E_CONTACT_EMAIL_1, ctact->email);
      if (ctact->categories)
	e_contact_set (contact, E_CONTACT_CATEGORIES, ctact->categories);
      if (ctact->speeddial) {

	attr = e_vcard_attribute_new (NULL, "TEL");
	param = e_vcard_attribute_param_new ("X-GNOMEMEETING-SPEEDDIAL");
	e_vcard_attribute_add_param_with_value (attr, 
						param, ctact->speeddial);
	e_vcard_add_attribute (E_VCARD (contact), attr);
      }

      if (e_book_add_contact (ebook, contact, NULL)) {

	return TRUE;
      }
    }
  }

  return FALSE;
}


gboolean 
gnomemeeting_local_addressbook_delete_contact (GmAddressbook *addressbook,
					       GmContact *contact)
{
  GList *l = NULL;

  EBook *ebook = NULL;

  g_return_val_if_fail (contact != NULL, FALSE);
  g_return_val_if_fail (addressbook != NULL, FALSE);


  if ((ebook = e_book_new_from_uri (addressbook->url, NULL))) {

    if (e_book_open (ebook, FALSE, NULL)) {

      if (contact->uid) {

	l = g_list_append (l, (gpointer) contact->uid);
	e_book_remove_contacts (ebook, l, NULL);
	g_list_free (l);
      }
    }
  }


  return TRUE;
};


gboolean
gnomemeeting_local_addressbook_modify_contact (GmAddressbook *addressbook,
					       GmContact *ctact)
{
  EBook *ebook = NULL;

  EContact *contact = NULL;
  EVCardAttribute *attr = NULL;
  EVCardAttributeParam *param = NULL;

  g_return_val_if_fail (ctact != NULL, FALSE);
  g_return_val_if_fail (addressbook != NULL, FALSE);


  if ((ebook = e_book_new_from_uri (addressbook->url, NULL))) {

      if (e_book_open (ebook, FALSE, NULL)) {


	if (ctact->uid) 
	  e_book_get_contact (ebook, ctact->uid, &contact, NULL);

	if (!contact)
	  contact = e_contact_new ();
	
	if (ctact->fullname)
	  e_contact_set (contact, E_CONTACT_FULL_NAME, ctact->fullname);
	if (ctact->url)
	  e_contact_set (contact, E_CONTACT_VIDEO_URL, ctact->url);
	if (ctact->email)
	  e_contact_set (contact, E_CONTACT_EMAIL_1, ctact->email);
	if (ctact->categories)
	  e_contact_set (contact, E_CONTACT_CATEGORIES, ctact->categories);
	if (ctact->speeddial) {

	  param = gm_addressbook_get_contact_speeddial_param (contact);

	  if (param) {

	    e_vcard_attribute_param_remove_values (param);
	    e_vcard_attribute_param_add_value (param, ctact->speeddial);
	  }
	  else {

	    attr = e_vcard_attribute_new (NULL, "TEL");
	    param = e_vcard_attribute_param_new ("X-GNOMEMEETING-SPEEDDIAL");
	    e_vcard_attribute_add_param_with_value (attr, 
						    param, ctact->speeddial);
	    e_vcard_add_attribute (E_VCARD (contact), attr);
	  }
	}

	if (e_book_commit_contact (ebook, contact, NULL))
	  return TRUE;
      }
  }
  
  return FALSE;
}


gboolean 
gnomemeeting_local_addressbook_is_editable (GmAddressbook *)
{
  return TRUE;
}


void
gnomemeeting_local_addressbook_init (gchar *group_name, gchar *source_name)
{
  ESourceGroup *source_group = NULL;
  ESourceGroup *on_this_computer = NULL;
  ESourceList *source_list = NULL;
  
  ESource *source = NULL;

  gchar *source_dir = NULL;

  g_return_if_fail (group_name != NULL && source_name != NULL);
  
  source_group =
    gnomemeeting_addressbook_get_local_source_group (&source_list);

  if (!source_group) {
    
    source_dir = g_strdup_printf ("file://%s/.evolution/addressbook/local", 
				  g_get_home_dir ());
    on_this_computer = e_source_group_new (group_name, source_dir);
    e_source_list_add_group (source_list, on_this_computer, -1);
    source = e_source_new ("", "");

    e_source_set_name (source, source_name);
    e_source_set_relative_uri (source, "system");
    e_source_set_group (source, on_this_computer);
    e_source_group_add_source (on_this_computer, source, -1); 

    e_source_list_sync (source_list, NULL);
    g_free (source_dir);
  }
}
