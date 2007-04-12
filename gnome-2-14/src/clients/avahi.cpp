
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
#include "ekiga.h"
#include "misc.h"


/* Avahi callbacks */
static int create_services (AvahiClient *c, 
			    void *userdata);

static void client_callback (AvahiClient *c, 
			     AvahiClientState state, 
			     void *userdata); 

static void entry_group_callback (AvahiEntryGroup *group, 
				  AvahiEntryGroupState state, 
				  void *userdata);


/* Implementation of the callbacks */
static void 
entry_group_callback (AvahiEntryGroup *group, 
		      AvahiEntryGroupState state, 
		      void *userdata) 
{
  GMZeroconfPublisher *zero = (GMZeroconfPublisher *) userdata;
  zero->EntryGroupCallback(group,state,userdata);
}


static void 
client_callback (AvahiClient *c, 
		 AvahiClientState state, 
		 void *userdata) 
{
  GMZeroconfPublisher *zero = (GMZeroconfPublisher *) userdata;
  zero->ClientCallback(c, state, userdata);
}


static int
create_services (AvahiClient *c, 
		 void *userdata) 
{
  GMZeroconfPublisher *zero = (GMZeroconfPublisher *) userdata;
  return zero->CreateServices (c, userdata);
}


/* Implementation of the class */
GMZeroconfPublisher::GMZeroconfPublisher (GMManager & m)
  : manager (m)
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


GMZeroconfPublisher::~GMZeroconfPublisher()
{
  if (h323_text_record) {
    
    avahi_string_list_free (h323_text_record);
    h323_text_record = NULL;
  }

  if (sip_text_record) {
    
    avahi_string_list_free (sip_text_record);
    sip_text_record = NULL;
  }

  if (group) {
    
    avahi_entry_group_free (group);
    group = NULL;
  }
  
  if (client) {
    
    avahi_client_free (client);
    client = NULL;
  }
  
  if (glib_poll) {
    
    avahi_glib_poll_free (glib_poll);
    glib_poll = NULL;
  }

  if (name) {
    
    g_free (name);
    name = NULL;
  }
}


void
GMZeroconfPublisher::EntryGroupCallback (AvahiEntryGroup *group,
					 AvahiEntryGroupState state, 
					 void *userdata) 
{
  char *n = NULL;
  
  /* Called whenever the entry group state changes */
  if (state == AVAHI_ENTRY_GROUP_COLLISION) {

    /* A service name collision happened. Let's pick a new name */
    n = avahi_alternative_service_name (name);
    avahi_free (name);
    name = n;

    /* And recreate the services */
    create_services (avahi_entry_group_get_client (group), userdata);
  }
}


int
GMZeroconfPublisher::CreateServices (AvahiClient *c, 
				     void *userdata) 
{
  int ret = 0;
  BOOL failure = FALSE;

  if (group == NULL) {
    
    group = avahi_entry_group_new (c, entry_group_callback, userdata);
    
    if (group == NULL) {

      PTRACE (1, "AVAHI\tavahi_entry_group_new failed: " << 
	      avahi_strerror (avahi_client_errno(c)));
      failure = TRUE;
    }
  }
  
  if (failure == FALSE) {
  
    PTRACE(1, "AVAHI\tAdding service " << name);

    /* H.323 */
    ret = avahi_entry_group_add_service_strlst (group,
						AVAHI_IF_UNSPEC,
						AVAHI_PROTO_UNSPEC,
						(AvahiPublishFlags) 0,
						name,
						ZC_H323,
						NULL,
						NULL,
						h323_port,
						h323_text_record);
    if (ret < 0) {

      PTRACE (1, "AVAHI\tFailed to add service: " << avahi_strerror (ret));
      failure = TRUE;
    }
  }

  if (failure == FALSE) {

    /* SIP */
    ret = avahi_entry_group_add_service_strlst (group,
						AVAHI_IF_UNSPEC,
						AVAHI_PROTO_UNSPEC,
						(AvahiPublishFlags) 0,
						name,
						ZC_SIP,
						NULL,
						NULL,
						sip_port,
						sip_text_record);
    if (ret < 0) {
    
      PTRACE (1, "AVAHI\tFailed to add service: " << avahi_strerror(ret));
      failure = TRUE;
    }
  }

  if (failure == FALSE) {

    /* Commit changes */
    ret = avahi_entry_group_commit (group);
    if (ret < 0) {
    
      PTRACE (1, "AVAHI\tFailed to commit entry_group: " << avahi_strerror (ret));
      failure = TRUE;
    }
  }

  return failure;
}


void
GMZeroconfPublisher::ClientCallback (AvahiClient *c, 
				     AvahiClientState state, 
				     void *userdata) 
{
  /* Called whenever the client or server state changes */
  if (state == AVAHI_CLIENT_S_RUNNING) {
   
    /* The server has startup successfully and registered its host
     * name on the network, so it's time to create our services */
    create_services(c, userdata);
  }
  else {
    
    if (state == AVAHI_CLIENT_S_COLLISION) {
   
      /* Let's drop our registered services. When the server is back
       * in AVAHI_SERVER_RUNNING state we will register them
       * again with the new host name. */
      if (group)
	avahi_entry_group_reset (group);

    } else if (state == AVAHI_CLIENT_FAILURE) {

      PTRACE(1, "AVAHI\tDbus Server connection terminated.");
    }
  }
}


int
GMZeroconfPublisher::Publish()
{
  int error = 0;

  GetPersonalData ();

  if (client && group) {
    
    avahi_entry_group_reset (group);
    if (avahi_client_get_state(client) == AVAHI_CLIENT_S_RUNNING)
      create_services (client, this);
    else
      return -1;
  }
  else {

    /* Allocate a new client */
    if (name) {
      
      client = avahi_client_new (poll_api, 
				 (AvahiClientFlags) 0, 
				 client_callback, 
				 this, 
				 &error);
      
      if (client == NULL) {
	  
	PTRACE (1, "AVAHI\tError initializing Avahi: %s" << 
		avahi_strerror (error));
	return -1;
      }
    }
  }

  return 0;
}


int
GMZeroconfPublisher::GetPersonalData()
{
  gchar	*lastname = NULL;
  gchar	*firstname = NULL;
  
  int state = 0;

  gnomemeeting_threads_enter ();
  firstname = gm_conf_get_string (PERSONAL_DATA_KEY "firstname");
  lastname = gm_conf_get_string (PERSONAL_DATA_KEY "lastname");
  h323_port = gm_conf_get_int (H323_KEY "listen_port");
  sip_port = gm_conf_get_int (SIP_KEY "listen_port");
  state = gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode");
  gnomemeeting_threads_leave ();

  /* Cleanups */
  if (h323_text_record) {
    
    avahi_string_list_free (h323_text_record);
    h323_text_record = NULL;
  }
  
  if (sip_text_record) {
    
    avahi_string_list_free (sip_text_record);
    sip_text_record = NULL;
  }
  
  if (name) {
    
    g_free (name); 
    name = NULL;
  }

  /* Update the internal state */
  name = gnomemeeting_create_fullname (firstname, lastname); 
  if (manager.GetCallingState () != GMManager::Standby)
    state = 2;
  
  h323_text_record = 
    avahi_string_list_add_printf (h323_text_record,"state=%d", state);
  sip_text_record = 
    avahi_string_list_add_printf (sip_text_record,"state=%d", state);

  h323_text_record = 
    avahi_string_list_add (h323_text_record, "software=Ekiga/" PACKAGE_VERSION);
  sip_text_record = 
    avahi_string_list_add (sip_text_record, "software=Ekiga/" PACKAGE_VERSION);


  g_free (lastname);
  g_free (firstname);

  return 0;
}


