
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
 *                         zeroconf_publish.cpp  -  description
 *                         ------------------------------------
 *   begin                : Thu Nov 4 2004
 *   copyright            : (C) 2004 by Sebastien Estienne 
 *   				        Benjamin Leviant
 *   description          : This file contains the zeroconf publisher 
 *   			    and resolver
 *
 */



#include <howl.h>
#include <stdio.h>

#include <gm_conf.h>

#include "zeroconf_publisher.h"
#include "misc.h"



/* Declarations */

/* DESCRIPTION  : Callback function used when the zeroconf service 
 *		  is  published.
 * BEHAVIOR     : Return ok.
 * PRE          : extra is (void *). it can be used to configure 
 *		  the function.
 */
static sw_result HOWL_API publish_reply (sw_discovery discovery,
					 sw_discovery_oid id,
					 sw_discovery_publish_status status,
					 sw_opaque extra);


/* Implementation */
static sw_result HOWL_API
publish_reply (sw_discovery discovery,
	       sw_discovery_oid id,
	       sw_discovery_publish_status status,
	       sw_opaque extra)
{
  return SW_OKAY;
}


/* Start of GMZeroconfPublisher definition */



GMZeroconfPublisher::GMZeroconfPublisher()
:PThread (1000, NoAutoDeleteThread)
{
  if (sw_discovery_init (&discovery) != SW_OKAY)
    PTRACE (1, "GMZeroconfPublisher: Can't publish!");

  publish_id = 0;
  text_record = NULL;
  name = NULL;
  port = 1720;

  if (discovery) {
   
    Start ();
    thread_sync_point.Wait ();
  }
}


/* DESCRIPTION  : / 
 * BEHAVIOR     : GMZeroconfPublisher destructor
 *		  -release the discovery zeroconf session
 * PRE          : must be call to finish zeroconf session
 */

GMZeroconfPublisher::~GMZeroconfPublisher()
{
  if (discovery)
    sw_discovery_stop_run (discovery);

  PWaitAndSignal m(quit_mutex);
}


int
GMZeroconfPublisher::Publish()
{
  sw_result err = SW_OKAY;

  if (!discovery)
    return (SW_FALSE);

  if (publish_id)
    Stop ();

  GetPersonalData ();

  if (name)
    err = sw_discovery_publish (discovery, 0, name, ZC_H323, NULL, NULL, 
				port, 
				sw_text_record_bytes (text_record), 
				sw_text_record_len (text_record), 
				publish_reply, NULL, &publish_id);

  return err;
}


int
GMZeroconfPublisher::GetPersonalData()
{
  gchar	*lastname = NULL;
  gchar	*firstname = NULL;
  gchar	*gm_conf_gchar = NULL;
  int gm_conf_int = 0;

  sw_result err = SW_OKAY;

  gnomemeeting_threads_enter ();
  /* fetch first and last name to create a fullname */
  firstname = gm_conf_get_string (PERSONAL_DATA_KEY "firstname");
  lastname = gm_conf_get_string (PERSONAL_DATA_KEY "lastname");
  /* Port number that will be published */
  port = gm_conf_get_int (PORTS_KEY "listen_port");
  gnomemeeting_threads_leave ();

  /*  Create the fullname that will be published in Srv record */
  if (firstname && lastname && strcmp (firstname, ""))
    if (strcmp (lastname, ""))
      name = g_strconcat (firstname, " ", lastname, NULL);
    else
      name = g_strdup (firstname);
  else
    if (strcmp (lastname, ""))
      name = g_strdup (lastname);
    else
      name = NULL;

  g_free (lastname);
  g_free (firstname);


  /* Init of text_record: it will publish the Txt record in mDns */
  err = sw_text_record_init (&text_record);
  sw_check_okay (err, exit);

  gnomemeeting_threads_enter ();
  /* Email */
  if ((gm_conf_gchar = gm_conf_get_string (PERSONAL_DATA_KEY "mail")) 
      && gm_conf_gchar && strcmp (gm_conf_gchar, "")) {

    err = 
      sw_text_record_add_key_and_string_value (text_record, "email", 
					       gm_conf_gchar);
    g_free (gm_conf_gchar);
    sw_check_okay (err, exit);

  }

  /* Location */
  if ((gm_conf_gchar = gm_conf_get_string (PERSONAL_DATA_KEY "location")) 
      && gm_conf_gchar && strcmp (gm_conf_gchar, "")) {

    err = 
      sw_text_record_add_key_and_string_value (text_record, "location", 
					       gm_conf_gchar);
    g_free (gm_conf_gchar);
    sw_check_okay (err, exit);
  }

  /* Comment */
  if ((gm_conf_gchar = gm_conf_get_string (PERSONAL_DATA_KEY "comment")) 
      && gm_conf_gchar && strcmp (gm_conf_gchar, ""))
    {
      err = 
	sw_text_record_add_key_and_string_value (text_record, "comment", 
						 gm_conf_gchar);
      g_free (gm_conf_gchar);
      sw_check_okay (err, exit);
    }

  /* Incoming Call Mode */
  gm_conf_int = gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode");
  if ((gm_conf_gchar = g_strdup_printf ("%d", gm_conf_int)))
    {
      err = 
	sw_text_record_add_key_and_string_value (text_record, "state", 
						 gm_conf_gchar);
      g_free (gm_conf_gchar);
      sw_check_okay (err, exit);
    }
  gnomemeeting_threads_leave ();

  /* H323 Software */
  err = 
    sw_text_record_add_key_and_string_value (text_record, "software", 
					     "GnomeMeeting");
  sw_check_okay(err, exit);

exit:
  return err;
}


void
GMZeroconfPublisher::Main ()
{
  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();

  sw_discovery_run (discovery);
}


int 
GMZeroconfPublisher::Start ()
{
  if (discovery)
    Resume ();

  return 0;
}


int 
GMZeroconfPublisher::Stop()
{
  sw_result err = SW_FALSE;

  if (discovery) {

    err = sw_discovery_cancel (discovery, publish_id);
    sw_check_okay (err, exit);
  }


  if (text_record)
    err = sw_text_record_fina (text_record);

exit:
  g_free (name);
  name = NULL;

  return err;
}
