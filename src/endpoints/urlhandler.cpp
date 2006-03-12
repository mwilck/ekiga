
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
 *                          urlhandler.cpp  -  description
 *                          ------------------------------
 *   begin                : Sat Jun 8 2002
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Multithreaded class to call a given URL or to
 *                          answer a call.
 *
 */


#include "../../config.h" 


#include "urlhandler.h"
#include "ekiga.h"
#include "misc.h"
#include "callshistory.h"
#include "history.h"
#include "main.h"
#include "chat.h"
#include "statusicon.h"

#ifdef HAS_DBUS
#include "dbus.h"
#endif

#include "gmdialog.h"
#include "gmcontacts.h"
#include "gmconf.h"

#include <ptclib/pils.h>


/* Declarations */
GMURL::GMURL ()
{
  is_supported = false;
}


GMURL::GMURL (PString c)
{
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


void GMURL::Parse ()
{
  PINDEX j = 0;
  PString default_h323_gateway;

  gchar *conf_string = NULL;

  GmAccount *account = NULL;
  GmAccount *ekiga_account = NULL;
  GmAccount *phone_account = NULL;

  conf_string = gm_conf_get_string (H323_KEY "default_gateway");
  default_h323_gateway = conf_string;

  account = gnomemeeting_get_default_account ("sip");
  ekiga_account = gnomemeeting_get_account ("ekiga.net");
  phone_account = gnomemeeting_get_account ("eugw.ast.diamondcard.us");
  
  if (!url.IsEmpty ()) {

    if (type == "sip") {

      if (account
	  && account->host 
	  && (url.Find ("@") == P_MAX_INDEX 
	      && url.Find (".") == P_MAX_INDEX 
	      && url.Find ("+") == P_MAX_INDEX)) {

	// We add a dirty workaround for PC-To-Phone calls
	// if the default account is ekiga.net and if it is
	// enabled.
	if (url.Find ("00") == 0 
	    && ekiga_account 
	    && ekiga_account->enabled && ekiga_account->default_account
	    && phone_account
	    && phone_account->enabled) {

	    url = url.Mid (2) + "@" + phone_account->host;
	}
	else
	  url = url + "@" + account->host;
      }
    }
    else if (type == "h323") {

      if (!default_h323_gateway.IsEmpty ()
	  && url.Find (default_h323_gateway) == P_MAX_INDEX) 	
	url = url + "@" + default_h323_gateway;
    }
  }

  j = url.Find (":");
  if (j != P_MAX_INDEX) {

    port = url.Mid (j+1);
    url = url.Left (j);
  }

  g_free (conf_string);
  gm_account_delete (account);
  gm_account_delete (phone_account);
  gm_account_delete (ekiga_account);
}


PString GMURL::GetFullURL ()
{
  PString full_url;

  if (is_supported) 
    Parse ();

  /* Compute the full URL */
  if (type == "shortcut") {
    
    full_url = url.Left (url.GetLength () - 1);
  }
  else if (type == "callto"
	   && url.Find ('/') != P_MAX_INDEX
	   && url.Find ("type") == P_MAX_INDEX) {
    
    full_url = "callto:" + url + "+type=directory";
  }
  else if (type == "sip") {
    
    if (port.IsEmpty ())
      port = "5060";
    
    full_url = type + ":" + url + ":" + port;
  }
  else if (type == "h323") {
    
    if (port.IsEmpty ())
      port = "1720";
    
    full_url = type + ":" + url + ":" + port;
  }
  else if (is_supported) {
    
    full_url = type + ":" + url;
  }
    

  return full_url;
}


PString GMURL::GetCanonicalURL ()
{
  PString canonical_url = url;

  if (!canonical_url.IsEmpty () && !port.IsEmpty ())
    canonical_url = canonical_url + ":" + port;
  
  return canonical_url; 
}


PString GMURL::GetURL ()
{
  if (is_supported) 
    Parse ();

  return type + ":" + GetCanonicalURL ();
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
  return (this->GetFullURL () *= u.GetFullURL ());
}


BOOL GMURL::operator != (GMURL u) 
{
  return !(this->GetFullURL () *= u.GetFullURL ());
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
  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;
  GtkWidget *history_window = NULL;
  GtkWidget *statusicon = NULL;
#ifdef HAS_DBUS
  GObject *dbus_component = NULL;
#endif
  GtkWidget *calls_history_window = NULL;
  
  PString call_address;
  PString current_call_token;

  GMURL old_url;
  
  GSList *l = NULL;
  GmContact *contact = NULL;

  gchar *conf_string = NULL;
  gchar *msg = NULL;

  int nbr = 0;

  gboolean result = FALSE;
  
  GMManager *endpoint = NULL;

  PWaitAndSignal m(quit_mutex);
  
  gnomemeeting_threads_enter ();

  g_free (conf_string);
  gnomemeeting_threads_leave ();

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  calls_history_window = GnomeMeeting::Process ()->GetCallsHistoryWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  statusicon = GnomeMeeting::Process ()->GetStatusicon ();
#ifdef HAS_DBUS
  dbus_component = GnomeMeeting::Process ()->GetDbusComponent ();
#endif

  endpoint = GnomeMeeting::Process ()->GetManager ();

  
  /* Answer/forward the current call in a separate thread if we are called
   * and return 	 
   */ 	 
  if (endpoint->GetCallingState () == GMManager::Called) { 	 

    if (!transfer_call)
      endpoint->AcceptCurrentIncomingCall (); 	 
    else {

      PSafePtr<OpalCall> call = endpoint->FindCallWithLock (endpoint->GetCurrentCallToken ());
      PSafePtr<OpalConnection> con = endpoint->GetConnection (call, TRUE);
      con->ForwardCall (call_address);
    }

    return; 	 
  }
  
  /* #INV: We have not been called to answer a call
   */
  if (url.IsEmpty ())
    return;


  /* The address to call */
  call_address = url.GetURL ();

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
  else if (url.GetType () == "shortcut") {

    l = gnomemeeting_addressbook_get_contacts (NULL, nbr, FALSE,
					       NULL, NULL, NULL, NULL,
					       (gchar *) (const char *) url.GetFullURL ());

    if (l && l->data) {

      contact = GM_CONTACT (l->data);
      call_address = GMURL (contact->url).GetURL ();
      gmcontact_delete (contact);
    }
  }
  else 
    call_address = url.GetURL ();


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
    gm_main_window_update_calling_state (main_window, GMManager::Calling);
    gm_chat_window_update_calling_state (chat_window, 
					 NULL,
					 call_address, 
					 GMManager::Calling);
    gm_statusicon_update_menu (statusicon, GMManager::Calling);
    gnomemeeting_threads_leave ();

    endpoint->SetCallingState (GMManager::Calling);

    result = endpoint->SetUpCall (call_address, current_call_token);
    
    /* If we have a valid URL, we a have a valid connection, if not
       we put things back in the initial state */
    if (result) {

      endpoint->SetCurrentCallToken (current_call_token);
#ifdef HAS_DBUS
      gnomemeeting_dbus_component_set_call_state (dbus_component,
						  current_call_token,
						  GMManager::Calling);
      gnomemeeting_dbus_component_set_call_info (dbus_component,
						 current_call_token,
						 NULL, NULL, call_address,
						 NULL);
#endif
    }
    else {


      /* The call failed, put back the state to "Standby", should 
       * be done in OnConnectionEstablished if con exists.
       */
      gnomemeeting_threads_enter ();
      gm_statusicon_update_menu (statusicon, GMManager::Standby);
      gm_chat_window_update_calling_state (chat_window, 
					   NULL, 
					   NULL,
					   GMManager::Standby);
      gm_main_window_update_calling_state (main_window, 
					   GMManager::Standby);

      if (call_address.Find ("+type=directory") != P_MAX_INDEX) {

	gm_main_window_flash_message (main_window, _("User not found"));
	gm_calls_history_add_call (PLACED_CALL,
				   NULL,
				   call_address, 
				   "0.00",
				   _("User not found"),
				   NULL);
	endpoint->SetCallingState (GMManager::Standby);
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
    }
  }
  else {

    PSafePtr<OpalCall> call = endpoint->FindCallWithLock (endpoint->GetCurrentCallToken ());
    PSafePtr<OpalConnection> con = endpoint->GetConnection (call, TRUE);
    con->TransferConnection (call_address);
  }
}
