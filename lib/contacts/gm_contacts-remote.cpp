
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


/*
 * Remote users can be either LDAP or ILS or ZeroConf users
 */

#include <string.h>
#include <lib/gm_conf.h>

#include "gm_contacts.h"
#ifndef _GM_CONTACTS_H_INSIDE__
#define _GM_CONTACTS_H_INSIDE__
#include "gm_contacts-remote.h"
#include "gm_contacts-ldap.h"
#ifdef HAS_HOWL
#include "gm_contacts-zeroconf.h"
#endif
#undef _GM_CONTACTS_H_INSIDE__
#endif


#ifndef _
#include <libintl.h>
#define _(x) gettext(x)
#ifdef gettext_noop
#define N_(String) gettext_noop (String)
#else
#define N_(String) (String)
#endif
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

    if (elmt->aid && elmt->name) 
      addressbooks = g_slist_append (addressbooks, (gpointer) elmt);
    else
      gm_addressbook_delete (elmt);

    j = g_slist_next (j);
  }

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

#ifdef HAS_HOWL
  elmt = gm_addressbook_new ();
  elmt->aid = g_strdup ("1086500000@ethium01");
  elmt->name = g_strdup (_("Network Neighborhood"));
  elmt->url = g_strdup ("zero://local");
  addressbooks = g_slist_append (addressbooks, (gpointer) elmt);
#endif
  
  return addressbooks;
}


GSList *
gnomemeeting_remote_addressbook_get_contacts (GmAddressbook *addressbook,
					      int &nbr,
					      gboolean partial_match,
					      gchar *fullname,
					      gchar *url,
					      gchar *categorie,
					      gchar *speeddial)
{
  if (addressbook && gnomemeeting_addressbook_is_ldap (addressbook)) 
    return gnomemeeting_ldap_addressbook_get_contacts (addressbook,
						       nbr,
						       partial_match,
						       fullname,
						       url,
						       categorie,
						       speeddial);
#ifdef HAS_HOWL /* If it is not an ldap addressbook, then it is a ZC one */
  else
    return gnomemeeting_zero_addressbook_get_contacts (addressbook,
						       nbr,
						       partial_match,
						       fullname,
						       url,
						       categorie,
						       speeddial);
#endif
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


gboolean
gnomemeeting_remote_addressbook_is_editable (GmAddressbook *addressbook)
{
  if (addressbook && gnomemeeting_addressbook_is_ldap (addressbook))
    return TRUE;
  else
    return FALSE;
}


void
gnomemeeting_remote_addressbook_init ()
{
#ifdef HAS_HOWL
  gnomemeeting_zero_addressbook_init ();
#endif
}

