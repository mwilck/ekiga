
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
#include "pref_window.h"
#include "log_window.h"
#include "misc.h"

#include <lib/gm_conf.h>
#include <lib/dialog.h>

#include <ptlib/ethsock.h>

#define new PNEW


/* The class */
GMSIPEndPoint::GMSIPEndPoint (GMEndPoint & ep)
: SIPEndPoint (ep), endpoint (ep)
{
}


GMSIPEndPoint::~GMSIPEndPoint ()
{
}


void 
GMSIPEndPoint::Init ()
{
  GtkWidget *main_window = NULL;

  gchar *outbound_proxy_host = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  gnomemeeting_threads_enter ();
  outbound_proxy_host = gm_conf_get_string (SIP_KEY "outbound_proxy_host");
  gnomemeeting_threads_leave ();


  /* Timeouts */
  SetPduCleanUpTimeout (PTimeInterval (0, 1));
  SetInviteTimeout (PTimeInterval (0, 10));
  SetNonInviteTimeout (PTimeInterval (0, 10));
  SetRetryTimeouts (1000, 10000);

  /* Update the User Agent */
  SetUserAgent ("GnomeMeeting/" PACKAGE_VERSION);
  

  /* Initialise internal parameters */
  if (outbound_proxy_host && !PString (outbound_proxy_host).IsEmpty ())
    SetProxy (outbound_proxy_host);

  g_free (outbound_proxy_host);
}


BOOL 
GMSIPEndPoint::StartListener (PString iface, 
			      WORD port)
{
  PIPSocket::InterfaceTable ifaces;
  PINDEX i = 0;
  
  gboolean ok = FALSE;

  gchar *listen_to = NULL;

  RemoveListener (NULL);

  /* Detect the valid interfaces */
  PIPSocket::GetInterfaceTable (ifaces);

  while (i < ifaces.GetSize ()) {
    
    if (ifaces [i].GetName () == iface)
      listen_to = 
	g_strdup_printf ("udp$%s:%d", 
			 (const char *) ifaces [i].GetAddress().AsString(),
			 port);
      
    i++;
  }

  /* Start the listener thread for incoming calls */
  if (!listen_to)
    return FALSE;

  ok = StartListeners (PStringArray (listen_to));
  g_free (listen_to);

  return ok;
}


void
GMSIPEndPoint::SetUserNameAndAlias ()
{
  PString default_local_name;

  default_local_name = endpoint.GetDefaultDisplayName ();

  if (!default_local_name.IsEmpty ()) {

    SetDefaultDisplayName (default_local_name);
  }
}


void 
GMSIPEndPoint::SetUserInputMode ()
{
  // Do nothing, only RFC2833 is supported.
}


void
GMSIPEndPoint::OnRegistered (const PString & domain,
			     const PString & username,
			     BOOL wasRegistering)
{
  GtkWidget *accounts_window = NULL;
  GtkWidget *history_window = NULL;
  GtkWidget *main_window = NULL;

  gchar *msg = NULL;

  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();

  gnomemeeting_threads_enter ();
  /* Registering is ok */
  if (wasRegistering) {

    msg = g_strdup_printf (_("Registered to %s"), 
			   (const char *) domain);
    gm_accounts_window_update_account_state (accounts_window, 
					     FALSE,
					     (const char *) domain, 
					     (const char *) username, 
					     _("Registered"),
					     NULL);
  }
  else {

    msg = g_strdup_printf (_("Unregistered from %s"),
			   (const char *) domain); 
    gm_accounts_window_update_account_state (accounts_window, 
					     FALSE,
					     (const char *) domain, 
					     (const char *) username, 
					     _("Unregistered"),
					     NULL);
  }

  gm_history_window_insert (history_window, msg);
  gm_main_window_flash_message (main_window, msg);
  gm_main_window_set_account_info (main_window, 
				   endpoint.GetRegisteredAccounts());
  gnomemeeting_threads_leave ();


  /* MWI Subscribe */
  if (wasRegistering && !IsSubscribed (domain, username))
    MWISubscribe (domain, username); 

  /* Signal the SIPEndPoint */
  SIPEndPoint::OnRegistered (domain, username, wasRegistering);

  g_free (msg);
}


void
GMSIPEndPoint::OnRegistrationFailed (const PString & domain,
				     const PString & user,
				     SIP_PDU::StatusCodes r,
				     BOOL wasRegistering)
{
  GtkWidget *accounts_window = NULL;
  GtkWidget *history_window = NULL;
  GtkWidget *main_window = NULL;

  gchar *msg_reason = NULL;
  gchar *msg = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();

  gnomemeeting_threads_enter ();
  /* Registering is ok */
  switch (r) {

  case SIP_PDU::Failure_BadRequest:
    msg_reason = g_strdup (_("Bad request"));
    break;

  case SIP_PDU::Failure_PaymentRequired:
    msg_reason = g_strdup (_("Payment required"));
    break;

  case SIP_PDU::Failure_UnAuthorised:
  case SIP_PDU::Failure_Forbidden:
    msg_reason = g_strdup (_("Forbidden"));
    break;

  case SIP_PDU::Failure_RequestTimeout:
    msg_reason = g_strdup (_("Timeout"));
    break;

  case SIP_PDU::Failure_Conflict:
    msg_reason = g_strdup (_("Conflict"));
    break;

  case SIP_PDU::Failure_TemporarilyUnavailable:
    msg_reason = g_strdup (_("Temporarily unavailable"));
    break;

  default:
    msg_reason = g_strdup (_("Registration failed"));
  }

  if (wasRegistering) {

    msg = g_strdup_printf (_("Registration to %s failed: %s"), 
			   (const char *) domain,
			   msg_reason);

    gm_accounts_window_update_account_state (accounts_window, 
					     FALSE,
					     (const char *) domain, 
					     (const char *) user, 
					     _("Registration failed"),
					     NULL);
  }
  else {

    msg = g_strdup_printf (_("Unregistration from %s failed: %s"), 
			   (const char *) domain,
			   msg_reason);

    gm_accounts_window_update_account_state (accounts_window, 
					     FALSE,
					     (const char *) domain, 
					     (const char *) user, 
					     _("Unregistration failed"),
					     NULL);
  }

  gm_history_window_insert (history_window, msg);
  gm_main_window_push_message (main_window, msg);
  gnomemeeting_threads_leave ();


  /* Signal the SIP EndPoint */
  SIPEndPoint::OnRegistrationFailed (domain, user, r, wasRegistering);


  g_free (msg);
}


BOOL 
GMSIPEndPoint::OnIncomingConnection (OpalConnection &connection)
{
  PSafePtr<OpalConnection> con = NULL;
  PSafePtr<OpalCall> call = NULL;

  gchar *forward_host = NULL;

  IncomingCallMode icm;
  gboolean busy_forward = FALSE;
  gboolean always_forward = FALSE;

  BOOL res = FALSE;

  int reason = 0;

  PTRACE (3, "GMSIPEndPoint\tIncoming connection");

  gnomemeeting_threads_enter ();
  forward_host = gm_conf_get_string (SIP_KEY "forward_host");
  busy_forward = gm_conf_get_bool (CALL_FORWARDING_KEY "forward_on_busy");
  always_forward = gm_conf_get_bool (CALL_FORWARDING_KEY "always_forward");
  icm =
    (IncomingCallMode) gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode");
  gnomemeeting_threads_leave ();


  call = endpoint.FindCallWithLock (endpoint.GetCurrentCallToken());
  if (call)
    con = endpoint.GetConnection (call, TRUE);
  if ((con && con->GetIdentifier () == connection.GetIdentifier()) 
      || (icm == DO_NOT_DISTURB))
    reason = 1;
  else if (forward_host && always_forward)
    reason = 2; // Forward
  /* We are in a call */
  else if (endpoint.GetCallingState () != GMEndPoint::Standby) {

    if (forward_host && busy_forward)
      reason = 2; // Forward
    else
      reason = 1; // Reject
  }
  else if (icm == AUTO_ANSWER)
    reason = 4; // Auto Answer
  else
    reason = 0; // Ask the user

  res = endpoint.OnIncomingConnection (connection, reason, forward_host);

  g_free (forward_host);

  return res;
}


void 
GMSIPEndPoint::OnMWIReceived (const PString & remoteAddress,
			      const PString & user,
			      SIPMWISubscribe::MWIType type,
			      const PString & msgs)
{
  GtkWidget *main_window = NULL;
  GtkWidget *accounts_window = NULL;

  gchar *info = NULL;

  endpoint.AddMWI (remoteAddress, user, msgs);

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();

  gnomemeeting_threads_enter ();
  info = g_strdup_printf (_("Missed calls: %d - Voice Mails: %s"),
			  endpoint.GetMissedCallsNumber (),
			  (const char *) endpoint.GetMWI ());
  gm_accounts_window_update_account_state (accounts_window,
					   FALSE,
					   remoteAddress,
					   user,
					   NULL,
					   (const char *) msgs);
  gm_main_window_push_info_message (main_window, info);
  g_free (info);
  gnomemeeting_threads_leave ();
}


void 
GMSIPEndPoint::OnMessageReceived (const SIPURL & from,
				  const PString & body)
{
  cout << "Message received from " << from << endl << flush;
  cout << "Body " << endl << body << endl << endl << flush;
}


void 
GMSIPEndPoint::OnMessageFailed (const SIPURL & messageUrl,
				SIP_PDU::StatusCodes reason)
{
  cout << "Failed to send message to " << messageUrl << ": " << reason << endl << flush;
}
      

int
GMSIPEndPoint::GetRegisteredAccounts ()
{
  return GetRegistrationsCount ();
}
