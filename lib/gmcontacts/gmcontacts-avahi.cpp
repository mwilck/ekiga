/* GnomeMeeting -- A Video-Conferencing application
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
 *                         gmcontact-avahi.cpp  -  description
 *                         ---------------------------------------
 *   begin                : Sun Aug 21 2005
 *   copyright            : (C) 2005 by Sebastien Estienne 
 *   description          : This file contains the zeroconf browser.
 *
 */


#include <stdio.h>

#include "../../config.h"

#ifndef _
#include <libintl.h>
#define _(x) gettext(x)
#ifdef gettext_noop
#define N_(String) gettext_noop (String)
#else
#define N_(String) (String)
#endif
#endif


#include "gmcontacts-avahi.h"

/* Declarations */


/* DESCRIPTION  : /
 * BEHAVIOR     : Compares two GmContact's using their fullname and returns
 *                the strcmp result.
 * PRE          : /
 */
gint
compare_func (gconstpointer a, 
	      gconstpointer b)
{
  GmContact *contact1 = (GmContact *) a;
  GmContact *contact2 = (GmContact *) b;
  
  g_return_val_if_fail (a != NULL && b != NULL 
			&& contact1->url && contact2->url, 0);
			
  return (strcmp (contact1->url, contact2->url));
}

/* The GmContact API function */
/* We can not do it without this global */
GMZeroconfBrowser *zcb = NULL;

GSList *
gnomemeeting_get_zero_addressbooks ()
{
  GSList *addressbooks = NULL;
  
  GmAddressbook *elmt = NULL;
  
  elmt = gm_addressbook_new ();
  elmt->aid = g_strdup ("1086500000@ethium01");
  elmt->name = g_strdup (_("Contacts Near Me"));
  elmt->url = g_strdup ("zero://local");
  addressbooks = g_slist_append (addressbooks, (gpointer) elmt);
  
  return addressbooks;
}


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
  GmContact *lc = NULL;
  
  GSList *f = NULL;
  GmContact *fc = NULL;

  gboolean match = FALSE;

  gchar *down_fullname = NULL;
  gchar *down_lcfullname = NULL;
  gchar *down_url = NULL;
  gchar *down_lcurl = NULL;
  
  if (!zcb)
    return NULL;
  
  if (addressbook) {
    
    l = zcb->GetContacts ();
    if (l) {

      /* No categorie and no speed dial for ZeroConf contacts, only
       * search for URL or Fullname*/

      while (l) {

	if (l->data) {

	  match = FALSE;
	  lc = GM_CONTACT (l->data);


	  /* Search filter */
	  if (!fullname && !url) {

	    match = TRUE;
	  }
	  else {

	    /* Full Name search */
	    if (fullname && strcmp (fullname, "")) {

	      down_fullname = g_utf8_strdown (fullname, -1);
	      down_lcfullname = g_utf8_strdown (lc->fullname, -1);
	      
	      if (partial_match 
		  && down_lcfullname 
		  && g_strrstr (down_lcfullname, down_fullname))
		match = TRUE;

	      if (!partial_match
		  && down_lcfullname
		  && !strcmp (down_lcfullname, down_fullname))
		match = TRUE;

	      g_free (down_fullname);
	      down_fullname = NULL;

	      g_free (down_lcfullname);
	      down_lcfullname = NULL;
	    }


	    /* URL search */
	    if (url && strcmp (url, "")) {

	      down_url = g_utf8_strdown (url, -1);
	      down_lcurl = g_utf8_strdown (lc->url, -1);

	      if (partial_match 
		  && down_lcurl 
		  && g_strrstr (down_lcurl, down_url))
		match = TRUE;

	      if (!partial_match
		  && lc->url
		  && !strcmp (down_lcurl, down_url))
		match = TRUE;

	      g_free (down_url);
	      down_url = NULL;

	      g_free (down_lcurl);
	      down_lcurl = NULL;
	    }
	  }

	  /* Match ? */
	  if (match) {

	    fc = gmcontact_new ();
	    
	    fc->fullname = g_strdup (lc->fullname);
	    fc->categories = g_strdup (lc->categories);
	    fc->url = g_strdup (lc->url);
	    fc->location = g_strdup (lc->location);
	    fc->speeddial = g_strdup (lc->speeddial);
	    fc->comment = g_strdup (lc->comment);
	    fc->software = g_strdup (lc->software);
	    fc->email = g_strdup (lc->email);
	    fc->state = lc->state;
	    fc->video_capable = lc->video_capable;

	    f = g_slist_append (f, (gpointer) fc);
	  }
	}

	l = g_slist_next (l);
      }
    }
    
    if (f)
      nbr = g_slist_length (f);
  }

  g_slist_foreach (l, (GFunc) gmcontact_delete, NULL);
  g_slist_free (l);
  
  return f;
}


void gnomemeeting_zero_addressbook_init ()
{
  static GMZeroconfBrowser z;
  zcb = &z;
  zcb->Browse ();
}



void 
GMZeroconfBrowser::ResolveCallback(
		 AvahiServiceResolver *r,
		 AvahiIfIndex interface,
		 AvahiProtocol protocol,
		 AvahiResolverEvent event,
		 const char *name,
		 const char *type,
		 const char *domain,
		 const char *host_name,
		 const AvahiAddress *address,
		 uint16_t port,
		 AvahiStringList *txt,
		 void* userdata) 
{
  GmContact *contact = NULL;
  GSList *tmp_list = NULL;
  AvahiStringList *txt_tmp;

  /* Called whenever a service has been resolved successfully or timed out */

  if (event == AVAHI_RESOLVER_TIMEOUT)
    PTRACE(1, "Failed to resolve service '" << name << "' of type '" << type << "' in domain '" << domain <<"'.");
  else {
    char a[128], *t;

    assert(event == AVAHI_RESOLVER_FOUND);
        
    avahi_address_snprint(a, sizeof(a), address);
    t = avahi_string_list_to_string(txt);

    /* Init of the new contact with the service name that we just discovered*/
    contact = gmcontact_new ();
    contact->fullname = g_strdup (name);
    
    /* creation of the call url */
    if (!strcmp(ZC_H323, type))
      contact->url = g_strdup_printf("h323:%s:%d", a, port);
    else
      contact->url = g_strdup_printf("sip:%s:%d", a, port);
    
    /* Try to find if the service url that we discovered 
     * is already in our contact list */
    tmp_list = g_slist_find_custom (contacts, 
				    contact,
				    compare_func);
    
    
    /* If tmp_list is not NULL it means that the service name was 
     * in the contact list */
    if (tmp_list && tmp_list->data) {
      
      gmcontact_delete ((GmContact *) tmp_list->data);
      tmp_list->data = contact;
    }
    
      
    contact->state = 0;
    for (txt_tmp = txt; txt_tmp; txt_tmp = txt_tmp->next) 
      {
	char *key, *value;
	size_t size;

	if (avahi_string_list_get_pair(txt_tmp, &key, &value, &size) < 0) 
	  {
	    PTRACE(1, "Not enough memory!");
	    gmcontact_delete (contact);
	    return;
	  }
	if (!strcmp ((const char *) key, "email"))
	  contact->email = g_strdup ((char *) value);
	else if (!strcmp((const char *) key, "location"))
	  contact->location = g_strdup ((char *) value);
	else if (!strcmp ((const char *) key, "comment"))
	  contact->comment = g_strdup ((char *) value);
	else if (!strcmp ((const char *) key, "software"))
	  contact->software = g_strdup ((char *)value);
	else if (!strcmp((const char *) key, "state"))
	  contact->state = (atoi ((const char *) value) == 2 ? 1 : 0);
	
	/* Ignore other keys */
	
	avahi_free(key);
	avahi_free(value);
      }
    
    /* If the list wasn't NULL it means that we found the contact 
     * so we just updated his info (email/location etc...) but we don't add it 
     * because it was already in the list.
     */
    if (tmp_list)
      return;
    
    /* Add the new contact in the contacts list. */
    contacts = g_slist_append (contacts, (gpointer) contact);
    
    avahi_free(t);
  }
  
  avahi_service_resolver_free(r);
}

static void resolve_callback(
			     AvahiServiceResolver *r,
			     AvahiIfIndex interface,
			     AvahiProtocol protocol,
			     AvahiResolverEvent event,
			     const char *name,
			     const char *type,
			     const char *domain,
			     const char *host_name,
			     const AvahiAddress *address,
			     uint16_t port,
			     AvahiStringList *txt,
			     void* userdata) 
{
  GMZeroconfBrowser *zero = (GMZeroconfBrowser *) userdata;
  zero->ResolveCallback(r,interface,protocol,event,name,type,domain,host_name,address,port,txt,userdata);
}

void 
GMZeroconfBrowser::BrowseCallback(
			    AvahiServiceBrowser *b,
			    AvahiIfIndex interface,
			    AvahiProtocol protocol,
			    AvahiBrowserEvent event,
			    const char *name,
			    const char *type,
			    const char *domain,
			    void* userdata)
{
  GmContact *contact = NULL;
  GSList *l = NULL;

  /* Called whenever a new services becomes available on the LAN or is removed from the LAN */

  /* If it's new, let's resolve it */
  if (event == AVAHI_BROWSER_NEW)
    {        
      /* We ignore the returned resolver object. In the callback function
	 we free it. If the server is terminated before the callback
	 function is called the server will free the resolver for us. */
      
      if (!(avahi_service_resolver_new(client, interface, protocol, name, type, domain, AVAHI_PROTO_UNSPEC, resolve_callback, userdata)))
	PTRACE(1, "Failed to resolve service '" << name << "' :" << avahi_strerror(avahi_client_errno(client)));
    }
  else
    {
      for (l = contacts; l && l->data; l = g_slist_next (l)) {
	
	contact = (GmContact *) l->data;
	if (contact && contact->fullname) {
	  
	  if (!strcmp (contact->fullname, name)) {
	    
	    contacts = 
	      g_slist_remove (contacts, 
			      (gpointer) contact);
	    gmcontact_delete (contact);
	  }
	}
      }
    }
}

static void browse_callback(
			    AvahiServiceBrowser *b,
			    AvahiIfIndex interface,
			    AvahiProtocol protocol,
			    AvahiBrowserEvent event,
			    const char *name,
			    const char *type,
			    const char *domain,
			    void* userdata)
{
  assert(b);
  GMZeroconfBrowser *zero = (GMZeroconfBrowser *) userdata;
  zero->BrowseCallback(b,interface,protocol,event,name,type,domain,userdata);
}

static void client_callback(AvahiClient *c, AvahiClientState state, void * userdata)
{
  assert(c);
  //   GMZeroconfBrowser *zero = (GMZeroconfBrowser *) userdata;
  /* Called whenever the client or server state changes */

  if (state == AVAHI_CLIENT_DISCONNECTED) {
    PTRACE(1, "Server connection terminated.");
    //exit?
  }
}

/* Start of the GMZeroconfBrowser implementation */
GMZeroconfBrowser::GMZeroconfBrowser ()
{
  /* Create the GLIB Adaptor */
  glib_poll = avahi_glib_poll_new (NULL, G_PRIORITY_DEFAULT);
  poll_api = avahi_glib_poll_get (glib_poll);
  
  
  client = NULL;
  contacts = NULL;
  h323_sb = NULL;
  sip_sb = NULL;
  client = NULL;

}

GMZeroconfBrowser::~GMZeroconfBrowser ()
{
  g_slist_foreach (contacts, (GFunc) gmcontact_delete, NULL);
  g_slist_free (contacts);

  /* Cleanup things */
  if (h323_sb)
    avahi_service_browser_free(h323_sb);
  if (sip_sb)
    avahi_service_browser_free(sip_sb);
    
  if (client)
    avahi_client_free(client);

  if (glib_poll)
    avahi_glib_poll_free(glib_poll);

}

GSList *
GMZeroconfBrowser::GetContacts ()
{
  GSList *l = NULL;
  GSList *ret = NULL;
  GmContact *c = NULL;
  GmContact *n = NULL;
  
  for (l = contacts; l && l->data; l = g_slist_next (l)) {
   
    c = GM_CONTACT (l->data);
    n = gmcontact_new ();
    
    n->fullname = g_strdup (c->fullname);
    n->email = g_strdup (c->email);
    n->url = g_strdup (c->url);
    n->comment = g_strdup (c->comment);
    n->software = g_strdup (c->software);
    n->state = c->state;
    
    ret = g_slist_append (ret, (gpointer) n);
  }
  
  return ret;
}

int
GMZeroconfBrowser::Browse()
{
  int error;

  if (poll_api)
    client = avahi_client_new(poll_api, client_callback, this, &error);
  
  /* Check wether creating the client object succeeded */
  if (!client) {
    PTRACE(1, "Failed to create client: " << avahi_strerror(error));
    goto fail;
  }
    
  /* Create the service browser */
  if (!(h323_sb = avahi_service_browser_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, ZC_H323, NULL, browse_callback, this))) {
    PTRACE(1, "Failed to create service browser: " << avahi_strerror(avahi_client_errno(client)));
    goto fail;
  }

  /* Create the service browser */
  if (!(sip_sb = avahi_service_browser_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, ZC_SIP, NULL, browse_callback, this))) {
    PTRACE(1, "Failed to create service browser: " << avahi_strerror(avahi_client_errno(client)));
    goto fail;
  }

 fail:  
  return 0;
}
