
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
 *                          urlhandler.cpp  -  description
 *                          ------------------------------
 *   begin                : Sat Jun 8 2002
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Multithreaded class to call a given URL or to
 *                          answer a call.
 *
 */


#include "../config.h" 


#include "urlhandler.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "calls_history_window.h"
#include "log_window.h"
#include "main_window.h"
#include "tray.h"

#include "dialog.h"
#include "contacts/gm_contacts.h"
#include "gm_conf.h"

#include <ptclib/pils.h>


/* Declarations */
GMURL::GMURL ()
{
  is_supported = false;
}


GMURL::GMURL (PString c)
{
  PINDEX j;
  
  c.Replace ("//", "");
  url = c.Trim ();
  
  if (url.Find ('#') == url.GetLength () - 1) {

    url.Replace ("callto:", "");
    url.Replace ("h323:", "");
    url.Replace ("sip:", "");
    type = PString ("shortcut");
    is_supported = true;
  }
  else if (url.Find ("callto:") == 0) {

    url.Replace ("callto:", "");
    type = PString ("callto");
    is_supported = true;
  }
  else if (url.Find ("h323:") == 0) {

    url.Replace ("h323:", "");
    type = PString ("h323");
    is_supported = true;
  }
  else if (url.Find ("sip:") == 0) {

    url.Replace ("sip:", "");

    type = PString ("sip");
    is_supported = true;
  }
  else if (url.Find ("h323:") == P_MAX_INDEX
	   && url.Find ("sip:") == P_MAX_INDEX
	   && url.Find ("callto:") == P_MAX_INDEX) {

    if (url.Find ("/") != P_MAX_INDEX) {
      
      type = PString ("callto");
      is_supported = true;
    }
    else {
      
      type = PString ("sip");
      is_supported = true;
    }
  }
  else
    is_supported = false;

  if (is_supported) {
    
    j = url.Find (":");
    if (j != P_MAX_INDEX) {

      port = url.Mid (j+1);
      url = url.Left (j);
    }
  }
}


GMURL::GMURL (const GMURL & u)
{
  is_supported = u.is_supported;
  type = u.type;
  url = u.url;
}


BOOL GMURL::IsEmpty ()
{
  return url.IsEmpty ();
}


BOOL GMURL::IsSupported ()
{
  return is_supported;
}


PString GMURL::GetType ()
{
  return type;
}


PString GMURL::GetValidURL ()
{
  PString valid_url;

  if (type == "shortcut")
    valid_url = url.Left (url.GetLength () -1);
  else if (type == "callto"
	   && url.Find ('/') != P_MAX_INDEX
	   && url.Find ("type") == P_MAX_INDEX) 
    valid_url = "callto:" + url + "+type=directory";
  else if (type == "sip") {
    if (port.IsEmpty ())
      port = "5060";
    valid_url = type + ":" + url + ":" + port;
  }
  else if (type == "h323") {
    if (port.IsEmpty ())
      port = "1720";
    valid_url = type + ":" + url + ":" + port;
  }
  else if (is_supported)
    valid_url = type + ":" + url;
    
  return valid_url;
}


PString GMURL::GetCanonicalURL ()
{
  return url;
}


PString GMURL::GetCalltoServer ()
{
  PINDEX i;
  
  if (type == "callto") {

    if ((i = url.Find ('/')) != P_MAX_INDEX) {

      return url.Left (i);
    }
    else
      return url;
  }
  else 
    return PString ();
}


PString GMURL::GetCalltoEmail ()
{
  PINDEX i;

  if (type == "callto") {

    if ((i = url.Find ('/')) != P_MAX_INDEX) {

      return url.Mid (i+1);
    }
  }
   
  return PString ();
}


PString GMURL::GetDefaultURL ()
{
  return PString ("sip:");
}


BOOL GMURL::Find (GMURL u)
{
  /* 
   * We have a callto address with an email, match on the callto email
   * of the given url if any, or on its canonical part.
   */
  if (!this->GetCalltoEmail ().IsEmpty ()) {
    
    if (!u.GetCalltoEmail ().IsEmpty ())
      return (this->GetCalltoEmail ().Find (u.GetCalltoEmail ()) == 0);
    else
      return (this->GetCalltoEmail ().Find (u.GetCanonicalURL ()) == 0);
  }
  /* 
   * We don't have a callto address with an email, match on the 
   * canonical part.
   */
  else 
    return (url.Find (u.GetCanonicalURL ()) == 0);

  return FALSE;
}


BOOL GMURL::operator == (GMURL u) 
{
  return (this->GetValidURL () *= u.GetValidURL ());
}


BOOL GMURL::operator != (GMURL u) 
{
  return !(this->GetValidURL () *= u.GetValidURL ());
}


/* The class */
GMURLHandler::GMURLHandler (PString c, 
			    BOOL transfer)
  :PThread (1000, AutoDeleteThread)
{
  url = GMURL (c);

  answer_call = !transfer;
  transfer_call = transfer;
  
  this->Resume ();
}


GMURLHandler::~GMURLHandler ()
{
  PWaitAndSignal m(quit_mutex);
  /* Nothing to do here except waiting that LDAP at worse
     cancels the search */
}


void GMURLHandler::Main ()
{
  GmAccount *account = NULL;

  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;
  GtkWidget *tray = NULL;

  GmContact *contact = NULL;
  GSList *l = NULL;

  GtkWidget *calls_history_window = NULL;
  
  PString default_gateway;
  PString call_address;
  PString current_call_token;

  GMURL old_url;
  
  gchar *conf_string = NULL;
  gchar *msg = NULL;

  int nbr = 0;
  gboolean result = FALSE;
  
  GMEndPoint *endpoint = NULL;

  PWaitAndSignal m(quit_mutex);
  
  gnomemeeting_threads_enter ();
  conf_string = gm_conf_get_string (H323_KEY "default_gateway");
  default_gateway = conf_string;
  g_free (conf_string);
  gnomemeeting_threads_leave ();

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  calls_history_window = GnomeMeeting::Process ()->GetCallsHistoryWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  tray = GnomeMeeting::Process ()->GetTray ();

  endpoint = GnomeMeeting::Process ()->Endpoint ();


  /* Answer/forward the current call in a separate thread if we are called
   * and return 	 
   */ 	 
  if (endpoint->GetCallingState () == GMEndPoint::Called) { 	 

    if (!transfer_call)
      endpoint->AcceptCurrentIncomingCall (); 	 
    else {

      PSafePtr<OpalCall> call = endpoint->FindCallWithLock (endpoint->GetCurrentCallToken ());
      PSafePtr<OpalConnection> con = endpoint->GetConnection (call, TRUE);
      con->ForwardCall (call_address);
    }

    return; 	 
  }
  

  /* We are not called to answer a call, but to do a call, or to 
   * transfer a call, check if the URL to call is empty or not.
   */
  if (url.IsEmpty ())
    return;

  
  /* Save the url */
  old_url = url;

  
  /* If it is a shortcut (# at the end of the URL), then we use it */
  if (url.GetType () == "shortcut") {

    l = gnomemeeting_addressbook_get_contacts (NULL, 
					       nbr,
					       FALSE,
					       NULL,
					       NULL,
					       NULL,
					       (gchar *) (const char *) url.GetValidURL ());
    
    if (l && l->data) {
      
      contact = GM_CONTACT (l->data);
      url = GMURL (contact->url);
      gm_contact_delete (contact);
    }
    else
      url = GMURL ();
    
    
    /* No speed dial found, call the number */
    if (url.IsEmpty ()) {

      gnomemeeting_threads_enter ();
      gm_history_window_insert (history_window,
				_("No contact with speed dial %s# found, will call number %s instead"),
				(const char *) old_url.GetValidURL (),
				(const char *) (GMURL ().GetDefaultURL () + old_url.GetValidURL ()));
      gnomemeeting_threads_leave ();
      
      url = GMURL (GMURL ().GetDefaultURL () + old_url.GetValidURL ());
    }
  } 


  /* The address to call */
  call_address = url.GetCanonicalURL ();

  if (!url.IsSupported ()) {

    gnomemeeting_threads_enter ();
    gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Invalid URL handler"), _("Please specify a valid URL handler. Currently both h323: and callto: are supported."));
    gnomemeeting_threads_leave ();

    return;
  }

  if (url.GetType () == "callto") { 

    PILSSession ils;
    int part1 = 0;
    int part2 = 0;
    int part3 = 0;
    int part4 = 0;
    gchar *ip = NULL;
    
    if (ils.Open (url.GetCalltoServer ())) {

      PILSSession::RTPerson person;

      if (ils.SearchPerson (url.GetCalltoEmail (), person)) {

	part1 = (person.sipAddress & 0xff000000) >> 24;
	part2 = (person.sipAddress & 0x00ff0000) >> 16;
	part3 = (person.sipAddress & 0x0000ff00) >> 8;
	part4 = person.sipAddress & 0x000000ff;

	ip = 
	  g_strdup_printf ("%s:%d.%d.%d.%d:%d", 
			   (const char *) person.sprotid[0],
			   part4, part3, part2, part1, 
			   (int) *person.sport);
	call_address = ip;
	g_free (ip);
      }
    }
  }
  else
    call_address = url.GetType () + ":" + call_address;
  
  /* If we are using a gateway, the real address is different */
  if (!default_gateway.IsEmpty ()
      && url.GetType () == "h323"
      && call_address.Find (default_gateway) == P_MAX_INDEX) 	
    call_address = call_address + "@" + default_gateway;
 
  
  /* If no SIP proxy is given, the real address is different, use
   * the default one 
   */
  if (url.GetType () == "sip") {

    account = gnomemeeting_get_default_account ("sip");
    if (account
	&& account->host 
	&& call_address.Find ("@") == P_MAX_INDEX)
      call_address = call_address + "@" + account->host;
    gm_account_delete (account);
  }


  /* Update the history */
  gnomemeeting_threads_enter ();
  if (!call_address.IsEmpty ()) {

    if (!transfer_call) 
      msg = g_strdup_printf (_("Calling %s"), 
			     (const char *) call_address);
    else
      msg = g_strdup_printf (_("Transferring call to %s"), 
			     (const char *) call_address);
    gm_history_window_insert (history_window, msg);
    gm_main_window_push_message (main_window, msg);
    g_free (msg);
  }
  gnomemeeting_threads_leave ();



  /* Connect to the URL */
  if (!transfer_call) {

    /* Update the state to "calling" */
    gnomemeeting_threads_enter ();
    gm_main_window_update_calling_state (main_window, GMEndPoint::Calling);
    if (tray)
      gm_tray_update_calling_state (tray, GMEndPoint::Calling);
    gnomemeeting_threads_leave ();

    endpoint->SetCallingState (GMEndPoint::Calling);

    result = endpoint->SetUpCall (call_address, current_call_token);
    
    /* If we have a valid URL, we a have a valid connection, if not
       we put things back in the initial state */
    if (result) {

      endpoint->SetCurrentCallToken (current_call_token);
    }
    else {


      /* The call failed, put back the state to "Standby", should 
       * be done in OnConnectionEstablished if con exists.
       */
      gnomemeeting_threads_enter ();
      if (tray)
	gm_tray_update_calling_state (tray, GMEndPoint::Standby);
      gm_main_window_update_calling_state (main_window, 
					   GMEndPoint::Standby);

      if (call_address.Find ("+type=directory") != P_MAX_INDEX) {

	gm_main_window_flash_message (main_window, _("User not found"));
	gm_calls_history_add_call (PLACED_CALL,
				   NULL,
				   call_address, 
				   "0.00",
				   _("User not found"),
				   NULL);
      }
      else {
	
	gm_main_window_flash_message (main_window, _("Failed to call user"));
	gm_calls_history_add_call (PLACED_CALL,
				   NULL,
				   call_address, 
				   "0.00",
				   _("Failed to call user"),
				   NULL);
      }
      gnomemeeting_threads_leave ();

      endpoint->SetCallingState (GMEndPoint::Standby);
    }
  }
  else {

    PSafePtr<OpalCall> call = endpoint->FindCallWithLock (endpoint->GetCurrentCallToken ());
    PSafePtr<OpalConnection> con = endpoint->GetConnection (call, TRUE);
    con->TransferConnection (call_address);
  }
}
