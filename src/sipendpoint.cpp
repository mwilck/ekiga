
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
 *                         sipendpoint.cpp  -  description
 *                         --------------------------------
 *   begin                : Wed 8 Dec 2004
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains the SIP Endpoint class.
 *
 */


#include "../config.h"

#include "sipendpoint.h"
#include "gnomemeeting.h"

#include "main_window.h"
#include "log_window.h"
#include "misc.h"

#include <lib/gm_conf.h>
#include <lib/dialog.h>


#define new PNEW


/* The class */
GMSIPEndPoint::GMSIPEndPoint (GMEndPoint & ep)
	: SIPEndPoint (ep), endpoint (ep)
{
  registrar = NULL;
}


GMSIPEndPoint::~GMSIPEndPoint ()
{
  if (registrar)
    delete (registrar);
}


void 
GMSIPEndPoint::Init ()
{
  GtkWidget *main_window = NULL;

  gchar *outbound_proxy_host = NULL;
  gchar *outbound_proxy_login = NULL;
  gchar *outbound_proxy_password = NULL;

  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  
  gnomemeeting_threads_enter ();
  outbound_proxy_host = gm_conf_get_string (SIP_KEY "outbound_proxy_host");
  outbound_proxy_login = gm_conf_get_string (SIP_KEY "outbound_proxy_login");
  outbound_proxy_password = 
    gm_conf_get_string (SIP_KEY "outbound_proxy_password");
  gnomemeeting_threads_leave ();


  /* Timeout */
  SetPduCleanUpTimeout (PTimeInterval (0, 2));


  /* Update the User Agent */
  SetUserAgent ("GnomeMeeting " PACKAGE_VERSION);
  
  /* Start the listener */
  if (!StartListener ()) 
    gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Error while starting the listener for the SIP protocol"), _("You will not be able to receive incoming SIP calls. Please check that no other program is already running on the port used by GnomeMeeting."));
  
  /* Initialise internal parameters */
  if (outbound_proxy_host && !PString (outbound_proxy_host).IsEmpty ())
    SetProxy (outbound_proxy_host, 
	      outbound_proxy_login, 
	      outbound_proxy_password);

  /* Register to registrar */
  if (gm_conf_get_int (SIP_KEY "registrar_registering_method"))
    RegistrarRegister ();

  g_free (outbound_proxy_host);
  g_free (outbound_proxy_login);
  g_free (outbound_proxy_password);
}


void
GMSIPEndPoint::RegistrarRegister ()
{
  int timeout = 0;

  gnomemeeting_threads_enter ();   
  timeout = gm_conf_get_int (SIP_KEY "registrar_registration_timeout");
  gnomemeeting_threads_leave ();

  if (registrar)
    delete (registrar);

  SetRegistrarTimeToLive (PTimeInterval (0, PMAX (120, PMIN (3600, timeout * 60))));

  registrar = new GMSIPRegistrar ();
}


BOOL 
GMSIPEndPoint::StartListener ()
{
  gboolean ok = FALSE;

  int listen_port = 5060;
  gchar *listen_to = NULL;
  
  gnomemeeting_threads_enter ();
  listen_port = gm_conf_get_int (SIP_KEY "listen_port");
  gnomemeeting_threads_leave ();

  
  /* Start the listener thread for incoming calls */
  listen_to = g_strdup_printf ("udp$*:%d", listen_port);
  ok = StartListeners (PStringArray (listen_to));
  g_free (listen_to);
   
  return ok;
}


void
GMSIPEndPoint::SetUserNameAndAlias ()
{
  gchar *login = NULL;
  PString default_local_name;
  
  default_local_name = endpoint.GetDefaultDisplayName ();

  
  gnomemeeting_threads_enter ();
  login = gm_conf_get_string (SIP_KEY "registrar_login");  
  gnomemeeting_threads_leave ();


  if (!default_local_name.IsEmpty ()) {
    
    SetDefaultLocalPartyName (login);
    SetDefaultDisplayName (default_local_name);
  }

  g_free (login);
}


void 
GMSIPEndPoint::OnRTPStatistics (const SIPConnection & connection,
				 const RTP_Session & session) const
{
  endpoint.UpdateRTPStats (connection.GetConnectionStartTime (),
			   session);
}


void
GMSIPEndPoint::OnRegistered ()
{
  GtkWidget *history_window = NULL;
  GtkWidget *main_window = NULL;

  gchar *registrar_host = NULL;
  gchar *msg = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();

  cout << "ici" << endl << flush;
  gnomemeeting_threads_enter ();
  registrar_host = gm_conf_get_string (SIP_KEY "registrar_host");
  
  /* Registering is ok */
  msg =
    g_strdup_printf (_("Registrar set to %s"), (const char *) registrar_host);

  gm_history_window_insert (history_window, msg);
  gm_main_window_flash_message (main_window, msg);
  gnomemeeting_threads_leave ();

  g_free (msg);
  g_free (registrar_host);
}


void
GMSIPEndPoint::OnRegistrationFailed ()
{
  GtkWidget *history_window = NULL;
  GtkWidget *main_window = NULL;

  gchar *registrar_host = NULL;
  gchar *msg = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();

  gnomemeeting_threads_enter ();
  registrar_host = gm_conf_get_string (SIP_KEY "registrar_host");
  
  /* Registering is ok */
  msg =
    g_strdup_printf (_("Registration to %s failed"), 
		     (const char *) registrar_host);

  gm_history_window_insert (history_window, msg);
  gm_main_window_push_message (main_window, msg);
  gnomemeeting_threads_leave ();

  g_free (msg);
  g_free (registrar_host);
}
