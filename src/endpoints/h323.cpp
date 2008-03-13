
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
 *                         h323endpoint.cpp  -  description
 *                         --------------------------------
 *   begin                : Wed 24 Nov 2004
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the H.323 Endpoint class.
 *
 */


#include "config.h"

#include "h323.h"
#include "ekiga.h"
#include "pcss.h"

#include "misc.h"

#include "gmconf.h"
#include "gmdialog.h"

#include <opal/transcoders.h>


#define new PNEW


/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with the H.245 Tunneling changes.
 * BEHAVIOR     :  It updates the endpoint.
 * PRE          :  data is a pointer to the H323EndPoint.
 */
static void h245_tunneling_changed_nt (G_GNUC_UNUSED gpointer id,
                                       GmConfEntry *entry,
                                       gpointer data);



/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with the early H.245 key changes.
 * BEHAVIOR     :  It updates the endpoint.
 * PRE          :  data is a pointer to the H323EndPoint.
 */
static void early_h245_changed_nt (G_GNUC_UNUSED gpointer id,
                                   GmConfEntry *entry,
                                   gpointer data);



/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with the Fast Start changes.
 * BEHAVIOR     :  It updates the endpoint.
 * PRE          :  data is a pointer to the H323EndPoint.
 */
static void fast_start_changed_nt (G_GNUC_UNUSED gpointer id,
                                   GmConfEntry *entry,
                                   gpointer data);



static void
h245_tunneling_changed_nt (G_GNUC_UNUSED gpointer id,
			   GmConfEntry *entry,
			   G_GNUC_UNUSED gpointer data)
{
  GMH323Endpoint *h323EP = (GMH323Endpoint *) data;
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    h323EP->DisableH245Tunneling (!gm_conf_entry_get_bool (entry));
  }
}


static void
early_h245_changed_nt (G_GNUC_UNUSED gpointer id,
		       GmConfEntry *entry,
		       G_GNUC_UNUSED gpointer data)
{
  GMH323Endpoint *h323EP = (GMH323Endpoint *) data;
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    h323EP->DisableH245inSetup (!gm_conf_entry_get_bool (entry));
  }
}


static void
fast_start_changed_nt (G_GNUC_UNUSED gpointer id,
		       GmConfEntry *entry,
		       G_GNUC_UNUSED gpointer data)
{
  GMH323Endpoint *h323EP = (GMH323Endpoint *) data;

  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    h323EP->DisableFastStart (!gm_conf_entry_get_bool (entry));
  }
}


/* The class */
GMH323Endpoint::GMH323Endpoint (GMManager & ep)
	: H323EndPoint (ep), endpoint (ep)
{
  NoAnswerTimer.SetNotifier (PCREATE_NOTIFIER (OnNoAnswerTimeout));
}


GMH323Endpoint::~GMH323Endpoint ()
{
}


void 
GMH323Endpoint::Init ()
{
  bool early_h245 = FALSE;
  bool h245_tunneling = FALSE;
  bool fast_start = FALSE;

  gnomemeeting_threads_enter ();
  fast_start = gm_conf_get_bool (H323_KEY "enable_fast_start");
  h245_tunneling = gm_conf_get_bool (H323_KEY "enable_h245_tunneling");
  early_h245 = gm_conf_get_bool (H323_KEY "enable_early_h245");
  gnomemeeting_threads_leave ();
  
  /* Initialise internal parameters */
  DisableH245Tunneling (!h245_tunneling);
  DisableFastStart (!fast_start);
  DisableH245inSetup (!early_h245);

  SetInitialBandwidth (40000);

  /* Be notified for configuration changes */
  gm_conf_notifier_add (H323_KEY "enable_h245_tunneling",
			h245_tunneling_changed_nt, this);

  gm_conf_notifier_add (H323_KEY "enable_early_h245",
			early_h245_changed_nt, this);

  gm_conf_notifier_add (H323_KEY "enable_fast_start",
			fast_start_changed_nt, this);
}


bool 
GMH323Endpoint::StartListener (PString iface,
			       WORD port)
{
  PString iface_noip;
  PString ip;
  PIPSocket::InterfaceTable ifaces;
  PINDEX i = 0;
  PINDEX pos = 0;
  
  gboolean ok = FALSE;
  gboolean found = FALSE;
  
  gchar *listen_to = NULL;

  RemoveListener (NULL);

  /* Detect the valid interfaces */
  PIPSocket::GetInterfaceTable (ifaces);

  while (i < ifaces.GetSize ()) {

    ip = " [" + ifaces [i].GetAddress ().AsString () + "]";
    
    if (ifaces [i].GetName () + ip == iface)
      listen_to = 
	g_strdup_printf ("tcp$%s:%d", 
			 (const char *) ifaces [i].GetAddress().AsString(),
			 port);
      
    i++;
  }

  i = 0;
  pos = iface.Find("[");
  if (pos != P_MAX_INDEX)
    iface_noip = iface.Left (pos).Trim ();
  while (i < ifaces.GetSize() && !found) {

    if (ifaces [i].GetName () == iface_noip) {
      listen_to = 
	g_strdup_printf ("tcp$%s:%d", 
			 (const char *) ifaces [i].GetAddress().AsString(),
			 port);
      found = TRUE;
    }
    
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
GMH323Endpoint::SetUserInputMode ()
{
  int mode = 0;

  gnomemeeting_threads_enter ();
  mode = gm_conf_get_int (H323_KEY "dtmf_mode");
  gnomemeeting_threads_leave ();

  switch (mode) 
    {
    case 0:
      SetSendUserInputMode (OpalConnection::SendUserInputAsString);
      break;
    case 1:
      SetSendUserInputMode (OpalConnection::SendUserInputAsTone);
      break;
    case 2:
      SetSendUserInputMode (OpalConnection::SendUserInputAsInlineRFC2833);
      break;
    case 3:
      SetSendUserInputMode (OpalConnection::SendUserInputAsQ931);
      break;
    default:
      break;
    }
}


void
GMH323Endpoint::Register (const PString & aor,
                          const PString & authUserName,
                          const PString & password,
                          const PString & gatekeeperID,
                          unsigned int expires,
                          bool unregister)
{
  PString info;
  PString host;
  PINDEX i = 0;

  bool result = false;

  i = aor.Find ("@");
  if (i == P_MAX_INDEX)
    return;

  host = aor.Mid (i+1);

  if (!unregister && !IsRegisteredWithGatekeeper (host)) {

    H323EndPoint::RemoveGatekeeper (0);

    /* Signal the OpalManager */
    endpoint.OnRegistering (aor, true);

    if (!authUserName.IsEmpty ()) {
      SetLocalUserName (authUserName);
      AddAliasName (endpoint.GetDefaultDisplayName ());
    }
      
    SetGatekeeperPassword (password);
    SetGatekeeperTimeToLive (expires * 1000);
    result = UseGatekeeper (host, gatekeeperID);

    /* There was an error (missing parameter or registration failed)
       or the user chose to not register */
    if (!result) {

      /* Registering failed */
      if (gatekeeper) {

	switch (gatekeeper->GetRegistrationFailReason()) {

	case H323Gatekeeper::DuplicateAlias :
	  info = _("Duplicate alias");
	  break;
	case H323Gatekeeper::SecurityDenied :
	  info = _("Bad username/password");
	  break;
	case H323Gatekeeper::TransportError :
	  info = _("Transport error");
	  break;
	case H323Gatekeeper::RegistrationSuccessful:
	  break;
	case H323Gatekeeper::UnregisteredLocally:
	case H323Gatekeeper::UnregisteredByGatekeeper:
	case H323Gatekeeper::GatekeeperLostRegistration:
	case H323Gatekeeper::InvalidListener:
	case H323Gatekeeper::NumRegistrationFailReasons:
	case H323Gatekeeper::RegistrationRejectReasonMask:
	default :
	  info = _("Failed");
	  break;
	}
      }
      else
	info = _("Failed");

      /* Signal the OpalManager */
      endpoint.OnRegistrationFailed (aor, true, info);
    }
    else {
      /* Signal the OpalManager */
      endpoint.OnRegistered (aor, true);
    }
  }
  else if (unregister && IsRegisteredWithGatekeeper (host)) {

    H323EndPoint::RemoveGatekeeper (0);
    RemoveAliasName (authUserName);

    /* Signal the OpalManager */
    endpoint.OnRegistered (aor, false);
  }
}


bool 
GMH323Endpoint::UseGatekeeper (const PString & address,
			       const PString & domain,
			       const PString & iface)
{
  bool result = 
    H323EndPoint::UseGatekeeper (address, domain, iface);

  PWaitAndSignal m(gk_name_mutex);
  
  gk_name = address;

  return result;
}
  

bool 
GMH323Endpoint::RemoveGatekeeper (const PString & address)
{
  if (IsRegisteredWithGatekeeper (address))
    return H323EndPoint::RemoveGatekeeper (0);

  return FALSE;
}
  
  
bool 
GMH323Endpoint::IsRegisteredWithGatekeeper (const PString & address)
{
  PWaitAndSignal m(gk_name_mutex);
  
  return ((gk_name *= address) && H323EndPoint::IsRegisteredWithGatekeeper ());
}


bool 
GMH323Endpoint::OnIncomingConnection (OpalConnection &connection,
                                      G_GNUC_UNUSED unsigned options,
                                      G_GNUC_UNUSED OpalConnection::StringOptions *str_options)
{
  PSafePtr<OpalConnection> con = NULL;
  PSafePtr<OpalCall> call = NULL;
  
  guint status = CONTACT_ONLINE;
  gboolean busy_forward = FALSE;
  gboolean always_forward = FALSE;

  bool res = FALSE;

  int no_answer_timeout = 0;
  unsigned reason = 0;
  gchar *forward_host = NULL;

  PTRACE (3, "GMH323Endpoint\tIncoming connection");


  gnomemeeting_threads_enter ();
  forward_host = gm_conf_get_string (H323_KEY "forward_host");
  busy_forward = gm_conf_get_bool (CALL_FORWARDING_KEY "forward_on_busy");
  always_forward = gm_conf_get_bool (CALL_FORWARDING_KEY "always_forward");
  status = gm_conf_get_int (PERSONAL_DATA_KEY "status");
  no_answer_timeout =
    gm_conf_get_int (CALL_OPTIONS_KEY "no_answer_timeout");
  gnomemeeting_threads_leave ();
  
  
  call = endpoint.FindCallWithLock (endpoint.GetCurrentCallToken());
  if (call)
    con = endpoint.GetConnection (call, TRUE);
  if ((con && con->GetIdentifier () == connection.GetIdentifier()) 
      || (status == CONTACT_DND))
    reason = 1;

  else if (forward_host && always_forward)
    reason = 2; // Forward
  /* We are in a call */
  else if (endpoint.GetCallingState () != GMManager::Standby) {

    if (forward_host && busy_forward)
      reason = 2; // Forward
    else
      reason = 1; // Reject
  }
  else
    reason = 0; // Ask the user

  if (reason == 0)
    NoAnswerTimer.SetInterval (0, PMIN (no_answer_timeout, 60));

  res = endpoint.OnIncomingConnection (connection, reason, forward_host);

  g_free (forward_host);
  return res;
}


void 
GMH323Endpoint::OnRegistrationConfirm ()
{
  H323EndPoint::OnRegistrationConfirm ();
}

  
void 
GMH323Endpoint::OnRegistrationReject ()
{
  PWaitAndSignal m(gk_name_mutex);

  gk_name = PString::Empty ();

  H323EndPoint::OnRegistrationReject ();
}


void 
GMH323Endpoint::OnEstablished (OpalConnection &connection)
{
  NoAnswerTimer.Stop ();

  PTRACE (3, "GMSIPEndpoint\t H.323 connection established");
  H323EndPoint::OnEstablished (connection);
}


void 
GMH323Endpoint::OnReleased (OpalConnection &connection)
{
  NoAnswerTimer.Stop ();

  PTRACE (3, "GMSIPEndpoint\t H.323 connection released");
  H323EndPoint::OnReleased (connection);
}


void
GMH323Endpoint::OnNoAnswerTimeout (PTimer &,
                                   INT) 
{
  gchar *forward_host = NULL;
  gboolean forward_on_no_answer = FALSE;
  
  if (endpoint.GetCallingState () == GMManager::Called) {
   
    gnomemeeting_threads_enter ();
    forward_host = gm_conf_get_string (H323_KEY "forward_host");
    forward_on_no_answer = 
      gm_conf_get_bool (CALL_FORWARDING_KEY "forward_on_no_answer");
    gnomemeeting_threads_leave ();

    if (forward_host && forward_on_no_answer) {
      
      PSafePtr<OpalCall> call = 
        endpoint.FindCallWithLock (endpoint.GetCurrentCallToken ());
      PSafePtr<OpalConnection> con = 
        endpoint.GetConnection (call, TRUE);
    
      con->ForwardCall (forward_host);
    }
    else
      ClearAllCalls (OpalConnection::EndedByNoAnswer, FALSE);

    g_free (forward_host);
  }
}


