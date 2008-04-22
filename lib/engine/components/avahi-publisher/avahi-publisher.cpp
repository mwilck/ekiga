
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
 *                          (C) 2008 by Damien Sandras
 *   description          : This file contains the Avahi zeroconf publisher. 
 *
 */


#include <iostream>

#include "config.h"

#include "avahi-publisher.h"
#include "personal-details.h"

using namespace Avahi;


/* Glib callback */
static gboolean on_disconnect (gpointer data);


/* Avahi callbacks */
static void avahi_client_callback (AvahiClient *client, 
                                   AvahiClientState state, 
                                   void *data); 

static void avahi_entry_group_callback (AvahiEntryGroup *group, 
                                        AvahiEntryGroupState state, 
                                        void *data);


/* Implementation of the callbacks */
static gboolean
on_disconnect (gpointer data)
{
  if (data != NULL)
    ((PresencePublisher *) data)->disconnect ();
  
  return FALSE;
}


static void 
avahi_client_callback (AvahiClient *client,
                       AvahiClientState state, 
                       void *data) 
{
  if (data != NULL)
    ((PresencePublisher *) data)->client_callback (client, state);
}


static void 
avahi_entry_group_callback (AvahiEntryGroup *group, 
                            AvahiEntryGroupState state, 
                            void *data) 
{
  if (data != NULL)
    ((PresencePublisher *) data)->entry_group_callback (group, state);
}



/* Implementation of the class */
PresencePublisher::PresencePublisher (Ekiga::ServiceCore & _core)
: Ekiga::PresencePublisher (_core),
  core (_core)
{
  /* Create the GLIB Adaptor */
  glib_poll = avahi_glib_poll_new (NULL, G_PRIORITY_DEFAULT);
  poll_api = avahi_glib_poll_get (glib_poll);
  
  name = NULL;
  client = NULL;
  group = NULL;
  text_record = NULL;
}


PresencePublisher::~PresencePublisher()
{
  if (text_record) {
    avahi_string_list_free (text_record);
    text_record = NULL;
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


void PresencePublisher::publish (const Ekiga::PersonalDetails & details)
{
  std::string short_status;
  int error = 0;

  if (text_record) {
    avahi_string_list_free (text_record);
    text_record = NULL;
  }

  if (name) {
    g_free (name); 
    name = NULL;
  }

  name = g_strdup (details.get_display_name ().c_str ());
  short_status = "presence-" + details.get_short_status ();
  text_record = avahi_string_list_add_printf (text_record, "presence=%s", short_status.c_str ());
  text_record = avahi_string_list_add (text_record, "software=Ekiga/" PACKAGE_VERSION);

  if (client && group) {

    avahi_entry_group_reset (group);
    if (avahi_client_get_state(client) == AVAHI_CLIENT_S_RUNNING)
      connect ();
  }
  else {

    /* Allocate a new client */
    if (name) {

      client = avahi_client_new (poll_api, (AvahiClientFlags) 0, avahi_client_callback, this, &error);
      group = avahi_entry_group_new (client, avahi_entry_group_callback, this);
    }
  }
}


bool PresencePublisher::connect () 
{
  int ret = 0;

  if (group != NULL) {
  
    ret = avahi_entry_group_add_service_strlst (group,
						AVAHI_IF_UNSPEC,
						AVAHI_PROTO_UNSPEC,
						(AvahiPublishFlags) 0,
						name,
						ZC_SIP,
						NULL,
						NULL,
						5060,
						text_record);
    if (ret >= 0) {

      /* Commit changes */
      ret = avahi_entry_group_commit (group);
      if (ret < 0) 
        return false;
    }
  }

  return true;
}


void PresencePublisher::disconnect ()
{
  if (client) {
    avahi_client_free (client);
    client = NULL;
  }

  group = NULL;
}


void PresencePublisher::client_callback (AvahiClient *_client, 
                                         AvahiClientState state)
{
  /* Called whenever the client or server state changes */
  if (state == AVAHI_CLIENT_S_RUNNING) {
   
    /* The server has startup successfully and registered its host
     * name on the network, so it's time to create our services */
    connect ();
  }
  else {
    
    if (state == AVAHI_CLIENT_S_COLLISION) {
   
      /* Let's drop our registered services. When the server is back
       * in AVAHI_SERVER_RUNNING state we will register them
       * again with the new host name. */
      if (group)
	avahi_entry_group_reset (group);

    } else if (state == AVAHI_CLIENT_FAILURE) {

      if (avahi_client_errno (_client) == AVAHI_ERR_DISCONNECTED) {

	g_timeout_add (60000, on_disconnect, this);
      }
    }
  }
}


void PresencePublisher::entry_group_callback (AvahiEntryGroup *_group,
                                              AvahiEntryGroupState state)
{
  char *n = NULL;
  
  /* Called whenever the entry group state changes */
  if (state == AVAHI_ENTRY_GROUP_COLLISION) {

    /* A service name collision happened. Let's pick a new name */
    n = avahi_alternative_service_name (name);
    avahi_free (name);
    name = n;
  }

  /* And recreate the services */
  group = _group;
  client = avahi_entry_group_get_client (group);

  connect ();
}

