
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
 *                         gm_contacts-remote.cpp - description 
 *                         ------------------------------------
 *   begin                : Mon May 23 2004
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Declaration of the remote addressbook access 
 *   			    functions. Use the API in gm_contacts.h instead.
 *
 */

#include <ptlib.h>
#include <ptclib/pldap.h>

#include <lib/gm_conf.h>

#include "gm_contacts.h"
#ifndef _GM_CONTACTS_H_INSIDE__
#define _GM_CONTACTS_H_INSIDE__
#include "gm_contacts-remote.h"
#undef _GM_CONTACTS_H_INSIDE__
#endif


GSList *
gnomemeeting_get_remote_addressbooks ()
{
  GSList *j = NULL;
  
  GSList *list = NULL;
  GSList *addressbooks = NULL;

  GmAddressbook *elmt = NULL;
  
  gchar **couple = NULL;

  list = 
    gm_conf_get_string_list ("/apps/gnomemeeting/contacts/ldap_servers_list");

  j = list;
  while (j) {
  
    elmt = gm_addressbook_new ();

    couple = g_strsplit ((char *) j->data, "|", 0);

    elmt->name = NULL;
    elmt->uid = NULL;

    if (couple) {
      
      if (couple [0])
	elmt->name = g_strdup (couple [0]);

      if (couple [1])
	elmt->uid = g_strdup (couple [1]);
      else
	elmt->uid = g_strdup (elmt->name); 
    }

    addressbooks = g_slist_append (addressbooks, (gpointer) elmt);

    j = g_slist_next (j);
  }

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  return addressbooks;
}


GSList *
gnomemeeting_remote_addressbook_get_contacts (GmAddressbook *addressbook,
					      gchar *fullname,
					      gchar *url,
					      gchar *categorie)
{
  PLDAPSession ldap;
  PLDAPSession::SearchContext context;
  PStringList attrs;
  PStringArray arr, arr2;
  PString entry;

  char prefix [256] = "";
  char hostname [256] = "";
  char port [256] = "";
  char base [256] = "";
  char scope [256] = "";

  gchar *firstname = NULL;
  gchar *surname = NULL;

  gboolean sub_scope = FALSE;
  gboolean is_ils = FALSE;

  int done = 0;
  
  GmContact *contact = NULL;
  GSList *list = NULL;
  
  g_return_val_if_fail (addressbook != NULL, NULL);

  attrs += "cn";
  attrs += "rfc822mailbox";
  attrs += "mail";
  attrs += "surname";
  attrs += "sn";
  attrs += "givenname";
  attrs += "location";
  attrs += "comment";
  attrs += "description";
  attrs += "l";
  attrs += "localityname";

  entry = addressbook->uid;
  entry.Replace (":", " ", TRUE);
  entry.Replace ("/", " ", TRUE);
  entry.Replace ("?", " ", TRUE);
  
  done = sscanf ((const char *) entry, 
		 "%255s %255s %255s %255s %255s", 
		 prefix, hostname, port, base, scope);

  if (done < 4) 
    return NULL;
    
  if (!strcmp (scope, "sub"))
    sub_scope = TRUE;

  if (!strcmp (prefix, "ils"))
    is_ils = TRUE;
  
  if (!ldap.Open (hostname, atoi (port)))
    cout << "Failed" << endl << flush;
 

  if (ldap.Search (context, 
		   (is_ils) 
		   ? "(&(cn=%))"
		   : "(cn=*)", 
		   attrs, 
		   base, 
		   (sub_scope) 
		   ? PLDAPSession::ScopeSubTree
		   : PLDAPSession::ScopeSingleLevel)) {

    do {

      contact = gm_contact_new ();
      
      if (ldap.GetSearchResult (context, "rfc822mailbox", arr)
	  || ldap.GetSearchResult (context, "mail", arr)) {
	
	contact->email = g_strdup ((const char *) arr [0]);
	contact->url = g_strdup_printf ("callto://ils.seconix.com/%s", 
					contact->email);
      }
      else {
	
	contact->email = g_strdup ("");
	contact->url = g_strdup ("");
      }
      
      if (ldap.GetSearchResult (context, "givenname", arr))
	firstname = g_strdup ((const char *) arr [0]);
      if (ldap.GetSearchResult (context, "surname", arr)
	  || ldap.GetSearchResult (context, "sn", arr))
	surname = g_strdup ((const char *) arr [0]);
	  
      if (firstname || surname)
	contact->fullname = g_strdup_printf ("%s %s", 
					     firstname?firstname:"", 
					     surname?surname:"");
      else if (ldap.GetSearchResult (context, "cn", arr)) 
	  contact->fullname = g_strdup ((const char *) arr [0]);
      else
	contact->fullname = g_strdup ("");

      if (ldap.GetSearchResult (context, "location", arr)
	  || ldap.GetSearchResult (context, "l", arr) 
	  || ldap.GetSearchResult (context, "localityname", arr)) 
	contact->location = g_strdup ((const char *) arr [0]);
      else
	contact->location = g_strdup ("");

      if (ldap.GetSearchResult (context, "comment", arr)
	  || ldap.GetSearchResult (context, "description", arr)) 
	contact->comment = g_strdup ((const char *) arr [0]);
      else 
	contact->comment = g_strdup ("");

      list = g_slist_append (list, (gpointer) contact);

      g_free (surname);
      g_free (firstname);
      surname = NULL;
      firstname = NULL;

    } while (ldap.GetNextSearchResult (context));
  }

  return list;
}


gboolean 
gnomemeeting_remote_addressbook_add (GmAddressbook *addressbook)
{
  GSList *list = NULL;
  gchar *entry = NULL;
  
  list = 
    gm_conf_get_string_list ("/apps/gnomemeeting/contacts/ldap_servers_list");

  entry = g_strdup_printf ("%s|%s", addressbook->name, addressbook->uid);

  list = g_slist_append (list, (gpointer) entry);
  gm_conf_set_string_list ("/apps/gnomemeeting/contacts/ldap_servers_list", 
			   list);

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  return TRUE;
}


gboolean 
gnomemeeting_remote_addressbook_delete (GmAddressbook *addressbook)
{
  GSList *list = NULL;
  GSList *l = NULL;
  
  gchar *entry = NULL;
  
  gboolean found = FALSE;
  
  list = 
    gm_conf_get_string_list ("/apps/gnomemeeting/contacts/ldap_servers_list");

  entry = g_strdup_printf ("%s|%s", addressbook->name, addressbook->uid);

  l = list;
  while (l && !found) {

    if (l->data && !strcmp ((const char *) l->data, entry)) {

      found = TRUE;
      break;
    }
    
    l = g_slist_next (l);
  }
  
  if (found) {

    list = g_slist_remove_link (list, l);

    g_free (l->data);
    g_slist_free_1 (l);

    gm_conf_set_string_list ("/apps/gnomemeeting/contacts/ldap_servers_list", 
			     list);
  }

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  g_free (entry);

  return found;
}


gboolean 
gnomemeeting_remote_addressbook_modify (GmAddressbook *addressbook,
					GmAddressbook *naddressbook)
{
  GSList *list = NULL;
  GSList *l = NULL;
  
  gchar *entry = NULL;
  gchar *nentry = NULL;
  
  gboolean found = FALSE;
  
  list = 
    gm_conf_get_string_list ("/apps/gnomemeeting/contacts/ldap_servers_list");

  entry = g_strdup_printf ("%s|%s", addressbook->name, addressbook->uid);
  nentry = g_strdup_printf ("%s|%s", naddressbook->name, naddressbook->uid);

  l = list;
  while (l && !found) {

    if (l->data && !strcmp ((const char *) l->data, entry)) {

      found = TRUE;
      break;
    }
    
    l = g_slist_next (l);
  }
  
  if (found) {

    list = g_slist_insert_before (list, l, (gpointer) nentry);
    list = g_slist_remove_link (list, l);

    g_free (l->data);
    g_slist_free_1 (l);


    gm_conf_set_string_list ("/apps/gnomemeeting/contacts/ldap_servers_list", 
			     list);
  }

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  g_free (entry);
  
  return found;
}

