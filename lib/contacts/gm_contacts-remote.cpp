
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
    gm_conf_get_string_list ("/apps/gnomemeeting/contacts/remote_addressbooks_list");

  j = list;
  while (j) {
  
    elmt = gm_addressbook_new ();

    couple = g_strsplit ((char *) j->data, "|", 0);

    elmt->name = NULL;
    elmt->url = NULL;
    elmt->call_attribute = NULL;

    if (couple) {

      if (couple [0]) {
	elmt->aid = g_strdup (couple [0]);

	if (couple [1]) {
	  elmt->name = g_strdup (couple [1]);

	  if (couple [2]) {
	    elmt->url = g_strdup (couple [2]);

	    if (couple [3]) {
	      elmt->call_attribute = g_strdup (couple [3]);
	    }
	  }
	}
      }
      g_strfreev (couple);
    }

    if (couple && couple [0] && couple [1])
      addressbooks = g_slist_append (addressbooks, (gpointer) elmt);

    j = g_slist_next (j);
  }

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  return addressbooks;
}


GSList *
gnomemeeting_remote_addressbook_get_contacts (GmAddressbook *addressbook,
					      gboolean partial_match,
					      gchar *fullname,
					      gchar *url,
					      gchar *categorie,
					      gchar *speeddial)
{
  PLDAPSession ldap;
  PLDAPSession::SearchContext context;
  PStringList attrs;
  PStringArray arr, arr2;
  PString entry;
  PString purl;

  char prefix [256] = "";
  char hostname [256] = "";
  char port [256] = "";
  char base [256] = "";
  char scope [256] = "";

  gchar *firstname = NULL;
  gchar *surname = NULL;
  gchar *tmp = NULL;

  gboolean sub_scope = FALSE;
  gboolean is_ils = FALSE;

  int done = 0;
  int v = 0;

  GmContact *contact = NULL;
  GSList *list = NULL;
  
  g_return_val_if_fail (addressbook != NULL, NULL);

  attrs += "cn";
  attrs += "sappid";
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
  attrs += "ilsa26214430";
  attrs += "ilsa26279966";
  if (addressbook->call_attribute)
    attrs += addressbook->call_attribute;

  entry = addressbook->url;
  entry.Replace (":", " ", TRUE);
  entry.Replace ("/", " ", TRUE);
  entry.Replace ("?", " ", TRUE);
  
  done = sscanf ((const char *) entry, 
		 "%255s %255s %255s %255s %255s", 
		 prefix, hostname, port, base, scope);

  if (done < 4) 
    return NULL;
    
  /* If we have no "scope", then it means there was no base, hackish */
  if (done == 4 && !strcmp (scope, "")) {

    strncpy (scope, base, 255);
    strcpy (base, "");
  }
  
  if (!strcmp (scope, "sub"))
    sub_scope = TRUE;

  if (!strcmp (prefix, "ils"))
    is_ils = TRUE;
  
  if (!ldap.Open (hostname, atoi (port)))
    return NULL;


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
	  || ldap.GetSearchResult (context, "mail", arr)) 
	contact->email = g_strdup ((const char *) arr [0]);
      
      
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
      

      if (ldap.GetSearchResult (context, "comment", arr)
	  || ldap.GetSearchResult (context, "description", arr)) 
	contact->comment = g_strdup ((const char *) arr [0]);
 

      if (ldap.GetSearchResult (context, "ilsa26214430", arr))
	contact->state = atoi ((const char *) arr [0]);
      else
	contact->state = 0;

      
      if (ldap.GetSearchResult (context, "sappid", arr)) {

	tmp = g_strdup ((const char *) arr [0]);
	if (is_ils && ldap.GetSearchResult (context, "ilsa26279966", arr)) {

	  v = atoi ((const char *) arr [0]);
	  contact->software = 
	    g_strdup_printf ("%s %d.%d.%d",
			     tmp,
			     (v & 0xff000000) >> 24,
			     (v & 0x00ff0000) >> 16,
			     v & 0x0000ffff);
	}
	else
	  contact->software = g_strdup (tmp);

	g_free (tmp);
      }

  
      if (addressbook->call_attribute
	  && ldap.GetSearchResult (context, addressbook->call_attribute, arr)) {
	
	/* Some clever guessing */
	if (is_ils && !strcasecmp (addressbook->call_attribute, "rfc822Mailbox"))
	  purl = PString ("callto:") + PString (hostname)
	    + PString ("/") + PString ((const char *) arr [0]);
	else {
	  
	  purl = PString ("h323:") + PString ((const char *) arr [0]);
	  purl.Replace ("+", "");
	  purl.Replace ("-", "");
	  purl.Replace (" ", "");
	}
      
	contact->url = g_strdup ((const char *) purl);
      }
      
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
    gm_conf_get_string_list ("/apps/gnomemeeting/contacts/remote_addressbooks_list");

  entry = g_strdup_printf ("%s|%s|%s|%s", 
			   addressbook->aid, 
			   addressbook->name, 
			   addressbook->url,
			   addressbook->call_attribute);

  list = g_slist_append (list, (gpointer) entry);
  gm_conf_set_string_list ("/apps/gnomemeeting/contacts/remote_addressbooks_list", 
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
    gm_conf_get_string_list ("/apps/gnomemeeting/contacts/remote_addressbooks_list");

  entry = 
    g_strdup_printf ("%s|%s|%s|%s", 
		     addressbook->aid, 
		     addressbook->name, 
		     addressbook->url,
		     addressbook->call_attribute);

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

    gm_conf_set_string_list ("/apps/gnomemeeting/contacts/remote_addressbooks_list", 
			     list);
  }

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  g_free (entry);

  return found;
}


gboolean 
gnomemeeting_remote_addressbook_modify (GmAddressbook *addressbook)
{
  GSList *list = NULL;
  GSList *l = NULL;
  
  gchar *entry = NULL;
  gchar **couple = NULL;
  
  gboolean found = FALSE;
  
  list = 
    gm_conf_get_string_list ("/apps/gnomemeeting/contacts/remote_addressbooks_list");

  entry = 
    g_strdup_printf ("%s|%s|%s|%s", 
		     addressbook->aid,
		     addressbook->name, 
		     addressbook->url,
		     addressbook->call_attribute);

  l = list;
  while (l && !found) {

    if (l->data) {
      
      couple = g_strsplit ((const char *) l->data, "|", 0);
      if (couple && couple [0] && !strcmp (couple [0], addressbook->aid)) {

	found = TRUE;
	break;
      }
    }
    
    l = g_slist_next (l);
  }
  
  if (found) {

    list = g_slist_insert_before (list, l, (gpointer) entry);
    list = g_slist_remove_link (list, l);

    g_free (l->data);
    g_slist_free_1 (l);


    gm_conf_set_string_list ("/apps/gnomemeeting/contacts/remote_addressbooks_list", 
			     list);
  }

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  return found;
}

