
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


#include "../../config.h"

#include "h323.h"
#include "ekiga.h"
#include "pcss.h"

#include "misc.h"

#include "gmconf.h"
#include "gmdialog.h"

#include <opal/transcoders.h>


#define new PNEW


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
  
  
  /* Initialise internal parameters */
  DisableH245Tunneling (!h245_tunneling);
  DisableFastStart (!fast_start);
  DisableH245inSetup (!early_h245);

  SetInitialBandwidth (40000);
}


BOOL 
GMH323Endpoint::StartListener (PString iface,
			       WORD port)
{
  PString ip;
  PIPSocket::InterfaceTable ifaces;
  PINDEX i = 0;
  
  gboolean ok = FALSE;

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

  /* Start the listener thread for incoming calls */
  if (!listen_to)
    return FALSE;

  ok = StartListeners (PStringArray (listen_to));
  g_free (listen_to);

  return ok;
}


OpalMediaFormatList
GMH323Endpoint::GetAvailableAudioMediaFormats ()
{
  OpalMediaFormatList list;
  OpalMediaFormatList media_formats;
  OpalMediaFormatList h323_list;

  GMPCSSEndpoint *pcssEP = endpoint.GetPCSSEndpoint ();

  media_formats = pcssEP->GetMediaFormats ();
  list += OpalTranscoder::GetPossibleFormats (media_formats);

  for (int i = 0 ; i < list.GetSize () ; i++) {

    if (list [i].GetDefaultSessionID () == 1) { 
      
      if (PString (list [i].GetEncodingName ()).GetLength () > 0) {

        if (list [i].IsValidForProtocol ("H323")
            && list [i].GetPayloadType () != RTP_DataFrame::MaxPayloadType)
          h323_list += list [i];

      }
    }
  }

  return h323_list;
}


OpalMediaFormatList
GMH323Endpoint::GetAvailableVideoMediaFormats ()
{
  OpalMediaFormatList list;
  OpalMediaFormatList media_formats;
  OpalMediaFormatList h323_list;

  GMPCSSEndpoint *pcssEP = endpoint.GetPCSSEndpoint ();

  media_formats = pcssEP->GetMediaFormats ();
  list += OpalTranscoder::GetPossibleFormats (media_formats);

  for (int i = 0 ; i < list.GetSize () ; i++) {

    if (list [i].GetDefaultSessionID () == 2) { 
      
      if (PString (list [i].GetEncodingName ()).GetLength () > 0) {

        if (list [i].IsValidForProtocol ("H323")
            && list [i].GetPayloadType () != RTP_DataFrame::MaxPayloadType)
          h323_list += list [i];

      }
    }
  }

  return h323_list;
}


void
GMH323Endpoint::SetUserNameAndAlias ()
{
  PString default_local_name;

  default_local_name = endpoint.GetDefaultDisplayName ();

  if (!default_local_name.IsEmpty ()) {
    
    SetDefaultDisplayName (default_local_name);
    SetLocalUserName (default_local_name);
  }
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
    }
}


BOOL 
GMH323Endpoint::UseGatekeeper (const PString & address,
			       const PString & domain,
			       const PString & iface)
{
  BOOL result = 
    H323EndPoint::UseGatekeeper (address, domain, iface);

  PWaitAndSignal m(gk_name_mutex);
  
  gk_name = address;

  return result;
}
  

BOOL 
GMH323Endpoint::RemoveGatekeeper (const PString & address)
{
  if (IsRegisteredWithGatekeeper (address))
    return H323EndPoint::RemoveGatekeeper (0);

  return FALSE;
}
  
  
BOOL 
GMH323Endpoint::IsRegisteredWithGatekeeper (const PString & address)
{
  PWaitAndSignal m(gk_name_mutex);
  
  return (gk_name *= address);
}


BOOL 
GMH323Endpoint::OnIncomingConnection (OpalConnection &connection)
{
  PSafePtr<OpalConnection> con = NULL;
  PSafePtr<OpalCall> call = NULL;
  
  gchar *forward_host = NULL;

  IncomingCallMode icm;
  gboolean busy_forward = FALSE;
  gboolean always_forward = FALSE;

  BOOL res = FALSE;

  int no_answer_timeout = 0;
  int reason = 0;
  
  PTRACE (3, "GMH323Endpoint\tIncoming connection");


  gnomemeeting_threads_enter ();
  forward_host = gm_conf_get_string (H323_KEY "forward_host");
  busy_forward = gm_conf_get_bool (CALL_FORWARDING_KEY "forward_on_busy");
  always_forward = gm_conf_get_bool (CALL_FORWARDING_KEY "always_forward");
  icm =
    (IncomingCallMode) gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode");
  no_answer_timeout =
    gm_conf_get_int (CALL_OPTIONS_KEY "no_answer_timeout");
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
  else if (endpoint.GetCallingState () != GMManager::Standby) {

    if (forward_host && busy_forward)
      reason = 2; // Forward
    else
      reason = 1; // Reject
  }
  else if (icm == AUTO_ANSWER)
    reason = 4; // Auto Answer
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


