
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
 *                         h323endpoint.cpp  -  description
 *                         --------------------------------
 *   begin                : Wed 24 Nov 2004
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains the H.323 Endpoint class.
 *
 */


#include "../config.h"

#include "h323endpoint.h"
#include "h323gatekeeper.h"
#include "gnomemeeting.h"

#include "misc.h"

#include <lib/gm_conf.h>
#include <lib/dialog.h>


#define new PNEW


/* The class */
GMH323EndPoint::GMH323EndPoint (GMEndPoint & ep)
	: H323EndPoint (ep), endpoint (ep)
{
  gk = NULL;
}


GMH323EndPoint::~GMH323EndPoint ()
{
  if (gk)
    delete (gk);
}


void 
GMH323EndPoint::Init ()
{
  GtkWidget *main_window = NULL;
  
  gboolean early_h245 = FALSE;
  gboolean h245_tunneling = FALSE;
  gboolean fast_start = FALSE;

  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  
  gnomemeeting_threads_enter ();
  fast_start = gm_conf_get_bool (H323_KEY "enable_fast_start");
  h245_tunneling = gm_conf_get_bool (H323_KEY "enable_h245_tunneling");
  early_h245 = gm_conf_get_bool (H323_KEY "enable_early_h245");
  gnomemeeting_threads_leave ();
  
  /* Start the listener */
  if (!StartListener ()) 
    gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Error while starting the listener for the H.323 protocol"), _("You will not be able to receive incoming H.323 calls. Please check that no other program is already running on the port used by GnomeMeeting."));

  
  /* Register to gatekeeper */
  if (gm_conf_get_int (H323_KEY "gatekeeper_registering_method"))
    GatekeeperRegister ();
  

  /* Initialise internal parameters */
  DisableH245Tunneling (!h245_tunneling);
  DisableFastStart (!fast_start);
  DisableH245inSetup (!early_h245);
}


void
GMH323EndPoint::GatekeeperRegister ()
{
  int timeout = 0;

  gnomemeeting_threads_enter ();   
  timeout = gm_conf_get_int (H323_KEY "gatekeeper_registration_timeout");
  gnomemeeting_threads_leave ();

  if (gk)
    delete (gk);

  registrationTimeToLive =
    PTimeInterval (0, PMAX (120, PMIN (3600, timeout * 60)));

  gk = new GMH323Gatekeeper ();
}


BOOL 
GMH323EndPoint::StartListener ()
{
  gboolean ok = FALSE;

  int listen_port = 1720;
  gchar *listen_to = NULL;
  
  gnomemeeting_threads_enter ();
  listen_port = gm_conf_get_int (H323_KEY "listen_port");
  gnomemeeting_threads_leave ();

  
  /* Start the listener thread for incoming calls */
  listen_to = g_strdup_printf ("tcp$*:%d", listen_port);
  ok = StartListeners (PStringArray (listen_to));
  g_free (listen_to);
   
  return ok;
}


void
GMH323EndPoint::SetUserNameAndAlias ()
{
  PString default_local_name;
  
  gchar *alias = NULL;

  
  gnomemeeting_threads_enter ();
  alias = gm_conf_get_string (H323_KEY "gatekeeper_login");  
  gnomemeeting_threads_leave ();


  default_local_name = endpoint.GetDefaultDisplayName ();

  if (!default_local_name.IsEmpty ()) {
    
    SetDefaultDisplayName (default_local_name);
    SetLocalUserName (default_local_name);
  }
  
  if (!PString (alias).IsEmpty ()) 
    AddAliasName (alias);
  
  g_free (alias);
}


void 
GMH323EndPoint::OnRTPStatistics (const H323Connection & connection,
				 const RTP_Session & session) const
{
  endpoint.UpdateRTPStats (connection.GetConnectionStartTime (),
			   session);
}


BOOL 
GMH323EndPoint::OnIncomingConnection (OpalConnection &connection)
{
  gchar *forward_host = NULL;

  gboolean busy_forward = FALSE;
  gboolean always_forward = FALSE;

  BOOL res = FALSE;

  int reason = 0;
  
  PTRACE (3, "GMH323EndPoint\tIncoming connection");


  gnomemeeting_threads_enter ();
  forward_host = gm_conf_get_string (H323_KEY "forward_host");
  busy_forward = gm_conf_get_bool (CALL_FORWARDING_KEY "forward_on_busy");
  always_forward = gm_conf_get_bool (CALL_FORWARDING_KEY "always_forward");
  gnomemeeting_threads_leave ();
  

  if (forward_host && always_forward)
    reason = 2; // Forward
  /* We are in a call */
  else if (endpoint.GetCallingState () != GMEndPoint::Standby) {

    if (forward_host && busy_forward)
      reason = 2; // Forward
    else
      reason = 1; // Reject
  }
  else
    reason = 0; // Ask the user

  res = endpoint.OnIncomingConnection (connection, reason, forward_host);

  g_free (forward_host);

  return res;
}

