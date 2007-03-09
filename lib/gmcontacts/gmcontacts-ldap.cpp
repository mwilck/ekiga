
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
 *                         gmcontacts-ldap.cpp - description 
 *                         ----------------------------------
 *   begin                : Mon May 23 2004
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Declaration of the LDAP addressbook access 
 *   			    functions. Use the API in gmcontacts.h instead.
 *
 */

#include "../../config.h"

#include <ptlib.h>
#include <ptclib/pldap.h>

#include "gmconf.h"

#include "gmcontacts.h"
#ifndef _GM_CONTACTS_H_INSIDE__
#define _GM_CONTACTS_H_INSIDE__
#include "gmcontacts-ldap.h"
#undef _GM_CONTACTS_H_INSIDE__
#endif


static gchar *
get_fixed_utf8 (PString str)
{
  gchar *utf8_str = NULL;

  if (g_utf8_validate ((gchar *) (const unsigned char*) str, -1, NULL))
    utf8_str = g_strdup ((char *) (const char *) (str));
  else
    utf8_str =  g_convert ((const char *) str.GetPointer (),
			   str.GetSize (), "UTF-8", "ISO-8859-1", 
			   0, 0, 0);

  return utf8_str;
}


gboolean 
gnomemeeting_addressbook_is_ldap (GmAddressbook *addressbook)
{
  g_return_val_if_fail (addressbook != NULL, TRUE);

  if (addressbook->url == NULL)
    return TRUE; 

  if (addressbook->url 
      && g_str_has_prefix (addressbook->url, "ldap:"))
    return TRUE;

  if (addressbook->url 
      && g_str_has_prefix (addressbook->url, "ils:"))
    return TRUE;


  return FALSE;
}


GSList *gnomemeeting_get_ldap_addressbooks () 
{
  GSList *j = NULL;
  
  GSList *list = NULL;
  GSList *addressbooks = NULL;

  GmAddressbook *elmt = NULL;
  
  gchar **couple = NULL;

  list = 
    gm_conf_get_string_list ("/apps/" PACKAGE_NAME "/contacts/remote_addressbooks_list");

  j = list;
  while (j) {
  
    elmt = gm_addressbook_new ();

    couple = g_strsplit ((char *) j->data, "|", 0);

    if (couple) {

      if (couple [0]) {

	if (elmt->aid)
	  g_free (elmt->aid);
	elmt->aid = g_strdup (couple [0]);

	if (couple [1]) {
	  elmt->name = g_strdup (couple [1]);

	  if (couple [2]) {

	    if (elmt->url)
	      g_free (elmt->url);
	    elmt->url = g_strdup (couple [2]);

	    if (couple [3]) {
	      elmt->call_attribute = g_strdup (couple [3]);
	    }
	  }
	}
      }
      
      g_strfreev (couple);
    }

    if (elmt->aid && elmt->name) 
      addressbooks = g_slist_append (addressbooks, (gpointer) elmt);
    else
      gm_addressbook_delete (elmt);

    j = g_slist_next (j);
  }

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  return addressbooks;
}


GSList *
gnomemeeting_ldap_addressbook_get_contacts (GmAddressbook *addressbook,
					    int &nbr,
					    gboolean partial_match,
					    gchar *fullname,
					    gchar *url,
					    gchar *categorie,
					    gchar *location,
					    gchar *speeddial)
{
  PLDAPSession ldap;
  PLDAPSession::SearchContext context;
  PStringList attrs;
  PStringArray arr, arr2;
  PString entry;
  PString purl;
  PString filter;

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

  gchar *xstatuses = NULL;
  gchar **xs = NULL;

  int xstatus = 0;
  int done = 0;
  int v = 0;

  GmContact *contact = NULL;
  GSList *list = NULL;
  
  g_return_val_if_fail (addressbook != NULL, NULL);

  attrs += "cn";
  attrs += "sappid";
  attrs += "info";
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
  attrs += "sipaddress";
  attrs += "sport";
  attrs += "xstatus";
  if (addressbook->call_attribute)
    attrs += addressbook->call_attribute;

  entry = addressbook->url;
  entry.Replace (":", " ", TRUE);
  entry.Replace ("/", " ", TRUE);
  entry.Replace ("?", " ", TRUE);
  
  done = sscanf ((const char *) entry, 
		 "%255s %255s %255s %255s %255s", 
		 prefix, hostname, port, base, scope);

  if (done < 5) 
    return NULL;
  
  if (!strcmp (scope, "sub"))
    sub_scope = TRUE;

  if (!strcmp (prefix, "ils"))
    is_ils = TRUE;
  
  if (!ldap.Open (hostname, atoi (port))) {
   
    nbr = -1;
    return NULL;
  }
    
  if (is_ils)
    ldap.SetOption (0x0011, 2); /* means set LDAP_OPT_PROTOCOL_VERSION to 2! */

  if (!ldap.Bind ()) {
    
    nbr = -1;
    return NULL;
  }

  if (is_ils) /* No url in ILS, and no OR either */ {
   
    if (fullname && strcmp (fullname, ""))
      filter = "(&(cn=%)(sn=%" + PString (fullname) + "%))";
    else if (url && strcmp (url, ""))
      filter = "(&(cn=%)(mail=%" + PString (url) + "%))";
    else
      filter = g_strdup ("(&(cn=%))");
  }  
  else {

    if (fullname && strcmp (fullname, ""))
      filter = "(&(|(cn=*" + PString (fullname) + "*)(givenname=*" + PString (fullname) + "*)(sn=*" + PString (fullname) + "*))";
    else
      filter = "(&(|(cn=*)(givenname=*)(sn=*))";

    if (url && strcmp (url, ""))
      filter += "(mail=*" + PString (url) + "*)";

    if (location && strcmp (location, ""))
      filter += "(l=*" + PString (location) + "*)";

    filter += ")";
  }

  if (ldap.Search (context, 
		   (const char *) filter, 
		   attrs, 
		   base, 
		   (sub_scope) 
		   ? PLDAPSession::ScopeSubTree
		   : PLDAPSession::ScopeSingleLevel)) {

    do {

      contact = gmcontact_new ();
      
      if (ldap.GetSearchResult (context, "rfc822mailbox", arr)
	  || ldap.GetSearchResult (context, "mail", arr)) 
	contact->email = get_fixed_utf8 ((const char *) arr [0]);
      
      
      if (ldap.GetSearchResult (context, "givenname", arr))
	firstname = get_fixed_utf8 ((const char *) arr [0]);
      if (ldap.GetSearchResult (context, "surname", arr)
	  || ldap.GetSearchResult (context, "sn", arr))
	surname = get_fixed_utf8 ((const char *) arr [0]);
	  
      if (firstname || surname)
	contact->fullname = g_strdup_printf ("%s %s", 
					     firstname?firstname:"", 
					     surname?surname:"");
      else if (ldap.GetSearchResult (context, "cn", arr)) 
	  contact->fullname = get_fixed_utf8 ((const char *) arr [0]);
      else
	contact->fullname = get_fixed_utf8 ("");

      
      if (ldap.GetSearchResult (context, "location", arr)
	  || ldap.GetSearchResult (context, "l", arr) 
	  || ldap.GetSearchResult (context, "localityname", arr)) 
	contact->location = get_fixed_utf8 ((const char *) arr [0]);
      

      if (ldap.GetSearchResult (context, "comment", arr)
	  || ldap.GetSearchResult (context, "description", arr)) 
	contact->comment = get_fixed_utf8 ((const char *) arr [0]);

      
      /* Specific to seconix.com */
      if (ldap.GetSearchResult (context, "xstatus", arr)) {
	
	xstatuses = g_strdup ((const char *) arr [0]);
	xs = g_strsplit (xstatuses, ",", 0);
	if (xs[0] && xs[1])
	  xstatus = atoi (xs [1]);
	else
	  xstatus = 0;
	g_free (xstatuses);
	g_strfreev (xs);
      }
      else
	xstatus = 0;

      if (ldap.GetSearchResult (context, "sappid", arr)) {

	tmp = get_fixed_utf8 ((const char *) arr [0]);
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
	  contact->software = get_fixed_utf8 (tmp);

	g_free (tmp);
      }
      else if (ldap.GetSearchResult (context, "info", arr))
	contact->software = get_fixed_utf8 (arr [0]);


      if (ldap.GetSearchResult (context, "ilsa26214430", arr))
	contact->state = (ContactState) atoi ((const char *) arr [0]);
      else if (PString (base).Find ("dc=ekiga") != P_MAX_INDEX) {

	/* Hack for eKiga users. An user is offline if its software
	 * is NULL.
	 */
	if (!contact->software) 
	  contact->state = CONTACT_OFFLINE;
        else
          contact->state = CONTACT_ONLINE;
      }
      else 
	contact->state = CONTACT_ONLINE;

      purl = PString::Empty();
      if (is_ils) {

        if (ldap.GetSearchResult (context, "sipaddress", arr)) {

          int part1, part2, part3, part4 = 0;
          int sport = 1720;
          gchar *ip = NULL;

          part1 = (atoi (arr [0]) & 0xff000000) >> 24;
          part2 = (atoi (arr [0]) & 0x00ff0000) >> 16;
          part3 = (atoi (arr [0]) & 0x0000ff00) >> 8;
          part4 = atoi (arr [0]) & 0x000000ff;

          if (ldap.GetSearchResult (context, "sport", arr))
            sport = atoi (arr [0]);

          ip = g_strdup_printf ("h323:%d.%d.%d.%d:%d", 
                                part4, part3, part2, part1, sport);
          purl = ip;
          g_free (ip);
        }
      }
      else {

        if (addressbook->call_attribute
            && ldap.GetSearchResult (context, 
                                     addressbook->call_attribute, 
                                     arr)) {

          purl = PString ("sip:") + PString ((const char *) arr [0]);
          purl.Replace ("+", "");
          purl.Replace ("-", "");
          purl.Replace (" ", "");
        }
      }

      contact->url = get_fixed_utf8 ((const char *) purl);
      
      list = g_slist_append (list, (gpointer) contact);

      g_free (surname);
      g_free (firstname);
      surname = NULL;
      firstname = NULL;

    } while (ldap.GetNextSearchResult (context));
  }

  if (nbr != -1) {
   
    if (xstatus != 0)
      nbr = xstatus;
    else
      nbr = g_slist_length (list);
  }

  return list;
}
