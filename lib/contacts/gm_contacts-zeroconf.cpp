
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
 *                         gm_contact-zeroconf.cpp  -  description
 *                         ---------------------------------------
 *   begin                : Thu Nov 4 2004
 *   copyright            : (C) 2004 by Sebastien Estienne 
 * 	 		                Benjamin Leviant
 *   description          : This file contains the zeroconf browser.
 *
 */


#include <howl.h>
#include <stdio.h>

#include "gm_contacts-zeroconf.h"

/* Declarations */


/* DESCRIPTION  : /
 * BEHAVIOR     : Compares two GmContact's using their fullname and returns
 *                the strcmp result.
 * PRE          : /
 */
static gint compare_func (gconstpointer, 
			  gconstpointer);


/* DESCRIPTION  : Callback function used when the zeroconf service 
 *		  is resolved.
 * BEHAVIOR     : Returns ok.
 *		  This function appends the new contact in list this->contacts
 * PRE          : extra is (void *). It can be used to configure 
 *		  the function.
 */
static sw_result HOWL_API resolve_reply (sw_discovery discovery,
					 sw_discovery_oid oid,
					 sw_uint32 interface_index,
					 sw_const_string name,
					 sw_const_string type,
					 sw_const_string domain,
					 sw_ipv4_address address,
					 sw_port port,
					 sw_octets text_record,
					 sw_uint32 text_record_len,
					 sw_opaque_t extra);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Return ok.
 *		  Callback function used when the zeroconf service 
 *		  is  browsed.
 *		  This function removes the old contact of the list 
 *		  this->contacts.
 *		  This function resolves the new contact.
 * PRE          : extra is (void *). It can be used to configure 
 *		  the function.
 */
static sw_result HOWL_API browse_reply (sw_discovery discovery,
					sw_discovery_oid oid,
					sw_discovery_browse_status status,
					sw_uint32 interface_index,
					sw_const_string name,
					sw_const_string type,
					sw_const_string domain,
					sw_opaque_t extra);


/* Callbacks: Implementation */
gint
compare_func (gconstpointer a, 
	      gconstpointer b)
{
  GmContact *contact1 = (GmContact *) a;
  GmContact *contact2 = (GmContact *) b;
  
  g_return_val_if_fail (a != NULL && b != NULL 
			&& contact1->fullname && contact2->fullname, 0);
			
  return (strcmp (contact1->fullname, contact2->fullname));
}


static sw_result HOWL_API
resolve_reply (sw_discovery discovery,
	       sw_discovery_oid oid,
	       sw_uint32 interface_index,
	       sw_const_string name,
	       sw_const_string type,
	       sw_const_string domain,
	       sw_ipv4_address address,
	       sw_port port,
	       sw_octets text_record,
	       sw_uint32 text_record_len,
	       sw_opaque_t extra)
{
  sw_int8 name_buf [16];
  sw_result err = SW_OKAY;
  sw_text_record_iterator it;
  sw_char key [255];
  sw_octet val [255];
  sw_ulong val_len = 0;

  GMZeroconfBrowser *zero = (GMZeroconfBrowser *) extra;

  GmContact *contact = NULL;
  GSList *tmp_list = NULL;

  
  /* Init of the new contact with the service name that we just discovered*/
  contact = gm_contact_new ();
  contact->fullname = g_strdup (name);

  
  /* Try to find if the service name that we discovered 
   * is already in our contact list */
  zero->mutex.Wait ();
  tmp_list = g_slist_find_custom (zero->contacts, 
				  contact,
				  compare_func);
  
  
  /* If tmp_list is not NULL it means that the service name was 
   * in the contact list */
  if (tmp_list && tmp_list->data) {
    
    gm_contact_delete ((GmContact *) tmp_list->data);
    tmp_list->data = contact;
  }

  /* creation of the call url */
  contact->url = 
    g_strdup_printf("h323://%s:%d",
		    sw_ipv4_address_name (address, (char *) name_buf, 16), 
		    port);
  zero->mutex.Signal ();

  
  if (sw_text_record_iterator_init (&it, text_record, text_record_len) 
      != SW_OKAY) {
  
    /* The contact was unused */
    if (!tmp_list)
      gm_contact_delete (contact);
    
    return -1;
  }
  
  contact->state = 0;
  
  while (sw_text_record_iterator_next (it, (char *) key, val, &val_len) 
	 == SW_OKAY) {

    zero->mutex.Wait ();
    if (!strcmp ((const char *) key, "email"))
      contact->email = g_strdup ((char *) val);
    else if (!strcmp((const char *) key, "location"))
      contact->location = g_strdup ((char *) val);
    else if (!strcmp ((const char *) key, "comment"))
      contact->comment = g_strdup ((char *) val);
    else if (!strcmp ((const char *) key, "software"))
      contact->software = g_strdup ((char *)val);
    else if (!strcmp((const char *) key, "state"))
      contact->state = (atoi ((const char *) val) == 2 ? 1 : 0);
    zero->mutex.Signal ();
  }
  sw_text_record_iterator_fina (it);

  /* If the list wasn't NULL it means that we found the contact 
   * so we just updated his info (email/location etc...) but we don't add it 
   * because it was already in the list.
   */
  if (tmp_list)
    return err;

  /* Add the new contact in the contacts list. 
   * The mutex preserve contacts from concurent access */
  zero->mutex.Wait();
  zero->contacts = g_slist_append (zero->contacts, (gpointer) contact);
  zero->mutex.Signal();

  return err;
}


static sw_result HOWL_API
browse_reply (sw_discovery discovery,
	      sw_discovery_oid oid,
	      sw_discovery_browse_status status,
	      sw_uint32 interface_index,
	      sw_const_string name,
	      sw_const_string type,
	      sw_const_string domain,
	      sw_opaque_t extra)
{
  sw_discovery_resolve_id rid;
  GMZeroconfBrowser *zero = (GMZeroconfBrowser *) extra;

  GmContact *contact = NULL;
  GSList *l = NULL;

  switch (status) {

  case SW_DISCOVERY_BROWSE_ADD_SERVICE:
    /* We discovered a new service, 
     * we must resolve its IP and TXT record */
    sw_discovery_resolve (discovery, interface_index, 
			  name, type, domain, resolve_reply, 
			  extra, &rid);
    break;


  case SW_DISCOVERY_BROWSE_REMOVE_SERVICE:
    /* A service dissappeared, we must remove it from the list */
    zero->mutex.Wait();
    for (l = zero->contacts; l && l->data; l = g_slist_next (l)) {

      contact = (GmContact *) l->data;
      if (contact && contact->fullname) {

	if (!strcmp (contact->fullname, name)) {

	  zero->contacts = 
	    g_slist_remove (zero->contacts, 
			    (gpointer) contact);
	  gm_contact_delete (contact);
	}
      }
    }

    zero->mutex.Signal();
    break;


  default:
    break;
  }

  return SW_OKAY;
}


/* The GmContact API function */
/* We can not do it without this global */
GMZeroconfBrowser *zcb = NULL;

GSList *
gnomemeeting_zero_addressbook_get_contacts (GmAddressbook *addressbook,
					    int &nbr,
					    gboolean partial_match,
					    gchar *fullname,
					    gchar *url,
					    gchar *categorie,
					    gchar *speeddial)
{
  GSList *l = NULL;
  
  if (!zcb)
    return NULL;

  zcb->Browse ();
  
  if (addressbook) {
    
    l = zcb->GetContacts ();
    if (l)
      nbr = g_slist_length (l);
  }
}


void gnomemeeting_zero_addressbook_init ()
{
  static GMZeroconfBrowser z;
  zcb = &z;
}


/* Start of the GMZeroconfBrowser implementation */
GMZeroconfBrowser::GMZeroconfBrowser ()
    :PThread (1000, NoAutoDeleteThread)
{
  if (sw_discovery_init(&discovery) != SW_OKAY)
    PTRACE (1, "GMZeroconfBrowser: Can't browse!");

  browse_id = 0;
  contacts = NULL;

  if (discovery) {

    Start ();
    thread_sync_point.Wait ();
  }
}


GMZeroconfBrowser::~GMZeroconfBrowser ()
{
  if (discovery)
    sw_discovery_stop_run (discovery);

  g_slist_foreach (contacts, (GFunc) gm_contact_delete, NULL);
  g_slist_free (contacts);

  PWaitAndSignal m(quit_mutex);
}


int
GMZeroconfBrowser::Browse ()
{
  sw_result err = SW_FALSE;
  
  if (discovery)
    err = sw_discovery_browse (discovery, 0, ZC_H323, NULL, 
			       browse_reply, this, &browse_id);
  
  return err;
}


GSList *
GMZeroconfBrowser::GetContacts ()
{
  GSList *l = NULL;
  GSList *ret = NULL;
  GmContact *c = NULL;
  GmContact *n = NULL;
  
  mutex.Wait();
  for (l = contacts; l && l->data; l = g_slist_next (l)) {
   
    c = GM_CONTACT (l->data);
    n = gm_contact_new ();
    
    n->fullname = g_strdup (c->fullname);
    n->email = g_strdup (c->email);
    n->url = g_strdup (c->url);
    n->comment = g_strdup (c->comment);
    n->state = c->state;
    
    ret = g_slist_append (ret, (gpointer) n);
  }
  mutex.Signal();
  
  return ret;
}


void
GMZeroconfBrowser::Main()
{
  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();

  sw_discovery_run (discovery);
}


int 
GMZeroconfBrowser::Start()
{
  if (discovery)
    Resume ();

  return 0;
}


int 
GMZeroconfBrowser::Stop ()
{
  sw_result err = SW_FALSE;

  if (discovery)
    err = sw_discovery_cancel (discovery, browse_id);
  
  return err;
}
