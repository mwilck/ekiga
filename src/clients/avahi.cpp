
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
 *                         avahi_publish.cpp  -  description
 *                         ------------------------------------
 *   begin                : Sun Aug 21 2005
 *   copyright            : (C) 2005 by Sebastien Estienne 
 *   description          : This file contains the Avahi zeroconf publisher. 
 *
 */


#include <gmconf.h>

#include "avahi.h"
#include "manager.h"
#include "gnomemeeting.h"
#include "misc.h"

static int create_services(AvahiClient *c, void *userdata);

void
GMZeroconfPublisher::EntryGroupCallback(AvahiEntryGroup *group, AvahiEntryGroupState state, void *userdata) 
{
  /* Called whenever the entry group state changes */

  if (state == AVAHI_ENTRY_GROUP_COLLISION) 
    {
      /* A collision occured with our name, we try to find another one */
      char *n;
      
      /* A service name collision happened. Let's pick a new name */
      n = avahi_alternative_service_name(name);
      avahi_free(name);
      name = n;

      /* And recreate the services */
      create_services(avahi_entry_group_get_client(group), userdata);
  }
}

static void 
entry_group_callback(AvahiEntryGroup *group, AvahiEntryGroupState state, void *userdata) 
{
  GMZeroconfPublisher *zero = (GMZeroconfPublisher *) userdata;
  zero->EntryGroupCallback(group,state,userdata);
}

int
GMZeroconfPublisher::CreateServices(AvahiClient *c, void *userdata) 
{
    int ret = 0;

  if (!group) {
    if (!(group = avahi_entry_group_new(c, entry_group_callback, userdata)))
      {
	PTRACE(1, "avahi_entry_group_new() failed: " << avahi_strerror(avahi_client_errno(c)));
	goto fail;
      }
  }
    
  PTRACE(1, "Adding service " << name);

  if ((ret = avahi_entry_group_add_service_strlst(group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, (AvahiPublishFlags)0, name, ZC_H323, NULL, NULL, h323_port, h323_text_record)) < 0) {
    PTRACE(1, "Failed to add service: " << avahi_strerror(ret));
    goto fail;
  }

  if ((ret = avahi_entry_group_add_service_strlst(group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, (AvahiPublishFlags)0, name, ZC_SIP, NULL, NULL, sip_port, sip_text_record)) < 0) {
    PTRACE(1, "Failed to add service: " << avahi_strerror(ret));
    goto fail;
  }


  if ((ret = avahi_entry_group_commit(group)) < 0) {
    PTRACE(1, "Failed to commit entry_group: " << avahi_strerror(ret));
    goto fail;
  }

 fail:
  return ret;
}

static int
create_services(AvahiClient *c, void *userdata) 
{
  GMZeroconfPublisher *zero = (GMZeroconfPublisher *) userdata;
  return zero->CreateServices(c, userdata);
}

void
GMZeroconfPublisher::ClientCallback(AvahiClient *c, AvahiClientState state, void * userdata) 
{
  /* Called whenever the client or server state changes */

  if (state == AVAHI_CLIENT_S_RUNNING)
    /* The serve has startup successfully and registered its host
     * name on the network, so it's time to create our services */
    create_services(c, userdata);
  else 
    if (state == AVAHI_CLIENT_S_COLLISION) {
      /* Let's drop our registered services. When the server is back
       * in AVAHI_SERVER_RUNNING state we will register them
       * again with the new host name. */
      if (group)
	  avahi_entry_group_reset(group);
    } else if (state == AVAHI_CLIENT_FAILURE) {

      PTRACE(1, "Dbus Server connection terminated.");
      // exit?
    }
}

static void 
client_callback(AvahiClient *c, AvahiClientState state, void * userdata) 
{
  GMZeroconfPublisher *zero = (GMZeroconfPublisher *) userdata;
  zero->ClientCallback(c, state, userdata);
}

GMZeroconfPublisher::GMZeroconfPublisher()
{  
  /* Create the GLIB Adaptor */
  glib_poll = avahi_glib_poll_new (NULL, G_PRIORITY_DEFAULT);
  poll_api = avahi_glib_poll_get (glib_poll);
  
  
  name = NULL;
  client = NULL;
  h323_text_record = NULL;
  sip_text_record = NULL;
  group = NULL;
}

int
GMZeroconfPublisher::Publish()
{
  int error;

  GetPersonalData ();

  if (client && group)
    {
      avahi_entry_group_reset(group);
      if (avahi_client_get_state(client) == AVAHI_CLIENT_S_RUNNING)
	create_services(client, this);
      else
	return -1;
    }
  else
    {
      /* Allocate a new client */
      if (name)
	if (!(client = avahi_client_new(poll_api, (AvahiClientFlags)0, client_callback, this, &error)))
	  {
	    g_warning ("Error initializing Avahi: %s", avahi_strerror (error));
	    return -1;
	  }
    }
  return 0;
}

int
GMZeroconfPublisher::GetPersonalData()
{
  GMManager *ep = NULL;
  
  gchar	*lastname = NULL;
  gchar	*firstname = NULL;
  gchar	*gm_conf_gchar = NULL;
  int gm_conf_int = 0;


  gnomemeeting_threads_enter ();
  /* fetch first and last name to create a fullname */
  firstname = gm_conf_get_string (PERSONAL_DATA_KEY "firstname");
  lastname = gm_conf_get_string (PERSONAL_DATA_KEY "lastname");
  /* Port number that will be published */
  h323_port = gm_conf_get_int (H323_KEY "listen_port");
  sip_port = gm_conf_get_int (SIP_KEY "listen_port");
  gnomemeeting_threads_leave ();

  ep = GnomeMeeting::Process ()->GetManager ();

  
  /*  Create the fullname that will be published in Srv record */
  g_free (name); 
  name = gnomemeeting_create_fullname (firstname, lastname); 
  g_free (lastname);
  g_free (firstname);


  /* Init of h323_text_record: it will publish the Txt record in mDns */
  if(h323_text_record)
    avahi_string_list_free(h323_text_record);
  h323_text_record = NULL;

  gnomemeeting_threads_enter ();
  /* Email */
  if ((gm_conf_gchar = gm_conf_get_string (PERSONAL_DATA_KEY "mail")) 
      && gm_conf_gchar && strcmp (gm_conf_gchar, "")) 
    {
      h323_text_record = avahi_string_list_add_printf(h323_text_record,"email=%s",gm_conf_gchar);
      g_free (gm_conf_gchar);
    }

  /* Location */
  if ((gm_conf_gchar = gm_conf_get_string (PERSONAL_DATA_KEY "location")) 
      && gm_conf_gchar && strcmp (gm_conf_gchar, "")) 
    {
      h323_text_record = avahi_string_list_add_printf(h323_text_record,"location=%s",gm_conf_gchar);
      g_free (gm_conf_gchar);
    }

  /* Comment */
  if ((gm_conf_gchar = gm_conf_get_string (PERSONAL_DATA_KEY "comment")) 
      && gm_conf_gchar && strcmp (gm_conf_gchar, ""))
    {
      h323_text_record = avahi_string_list_add_printf(h323_text_record,"comment=%s",gm_conf_gchar);
      g_free (gm_conf_gchar);
    }

  /* Incoming Call Mode */
  if ((ep->GetCallingState () != GMManager::Standby)
      || (gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode") 
	  == DO_NOT_DISTURB))
    gm_conf_int = 2;
  h323_text_record = avahi_string_list_add_printf(h323_text_record,"state=%d",gm_conf_int);

  gnomemeeting_threads_leave ();

  /* H323 Software */
  h323_text_record = avahi_string_list_add(h323_text_record,"software=GnomeMeeting");

  return 0;
}

GMZeroconfPublisher::~GMZeroconfPublisher()
{
  if(h323_text_record)
    avahi_string_list_free(h323_text_record);

  if(sip_text_record)
    avahi_string_list_free(sip_text_record);

  if (group)
    avahi_entry_group_free(group);
  
  if (client)
    avahi_client_free(client);
  
  if (glib_poll)
    avahi_glib_poll_free(glib_poll);

  g_free(name);
}
