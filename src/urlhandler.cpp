
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2003 Damien Sandras
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
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : Multithreaded class to call a given URL or to
 *                          answer a call.
 *
 */


#include "../config.h" 


#include "urlhandler.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "ldap_window.h"
#include "main_window.h"
#include "toolbar.h"
#include "menu.h"
#include "tools.h"

#include "dialog.h"

/* Declarations */
extern GnomeMeeting *MyApp;
extern GtkWidget *gm;

static gint
TransferTimeOut (gpointer data)
{
  PString transfer_call_token;
  PString call_token;

  GMH323EndPoint *ep = NULL;
  
  gdk_threads_enter ();
  ep = MyApp->Endpoint ();

  if (ep)
    call_token = ep->GetCurrentCallToken ();

  if (!call_token.IsEmpty ())
    gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Call transfer failed"), _("The call transfer failed, the user was either unreachable, or simply busy when he received the call transfer request."));
  gdk_threads_leave ();

  
  return FALSE;
}



GMURL::GMURL ()
{
  is_supported = false;
}


GMURL::GMURL (PString c)
{
  url = c;

  url.Replace ("//", "");
  url.Replace (" ", "");
  
  if (url.Find ('#') == url.GetLength () - 1) {

    url.Replace ("callto:", "");
    url.Replace ("h323:", "");
    type = PString ("shortcut");
    is_supported = true;
  }
  else if (url.Find ("callto:") != P_MAX_INDEX) {

    url.Replace ("callto:", "");
    type = PString ("callto");
    is_supported = true;
  }
  else if (url.Find ("h323:") != P_MAX_INDEX) {

    url.Replace ("h323:", "");
    type = PString ("h323");
    is_supported = true;
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


PString GMURL::GetValidURL ()
{
  PString valid_url;

  if (type == "shortcut")
    valid_url = url.Left (url.GetLength () -1);
  else if (type == "callto"
	   && url.Find ('/') != P_MAX_INDEX
	   && url.Find ("type") == P_MAX_INDEX) 
    valid_url = "callto:" + url + "+type=directory";
  else if (is_supported)
    valid_url = type + ":" + url;
    
  return valid_url;
}


PString GMURL::GetDefaultURL ()
{
  return PString ("h323:");
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
GMURLHandler::GMURLHandler (PString c, BOOL transfer)
  :PThread (1000, AutoDeleteThread)
{
  gw = MyApp->GetMainWindow ();
  url = GMURL (c);

  answer_call = FALSE;
  transfer_call = transfer;
  
  this->Resume ();
}


GMURLHandler::GMURLHandler ()
  :PThread (1000, AutoDeleteThread)
{
  gw = MyApp->GetMainWindow ();

  answer_call = TRUE;
  
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
  GmWindow *gw = NULL;
  
  PString call_address;
  PString current_call_token;
  GMURL old_url;
  
  gchar *msg = NULL;
  
  GMH323EndPoint *endpoint = NULL;
  H323Connection *con = NULL;

  GConfClient *client = NULL;

  PWaitAndSignal m(quit_mutex);
  
  gnomemeeting_threads_enter ();
  client = gconf_client_get_default ();
  gw = MyApp->GetMainWindow ();
  gnomemeeting_threads_leave ();
  
  endpoint = MyApp->Endpoint ();


  /* Answer the current call in a separate thread if required */
  if (answer_call && endpoint && endpoint->GetCallingState () == 3) {

    con = endpoint->GetCurrentConnection ();

    if (con)
      con->AnsweringCall (H323Connection::AnswerCallNow);

    return;
  }


  if (url.IsEmpty ())
    return;

  old_url = url;
  
  /* If it is a shortcut (# at the end of the URL), then we use it */
  if (url.GetType () == "shortcut")
    url =
      gnomemeeting_addressbook_get_url_from_speed_dial (url.GetValidURL ());


  /* The shortcut could lead to an empty URL here */
  if (!url.IsEmpty ()) {

    call_address = url.GetValidURL ();

    if (!url.IsSupported ()) {

      gnomemeeting_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Invalid URL handler"), _("Please specify a valid URL handler. Currently both h323: and callto: are supported."));
      gnomemeeting_threads_leave ();
    }

  
    /* If the user is using MicroTelco, but G.723.1 is not available for
       a reason or another, we add the MicroTelco prefix if the URL seems
       to be a phone number */
    gnomemeeting_threads_enter ();
    if (gconf_client_get_bool (client, SERVICES_KEY "enable_microtelco", 0)) {

      if (call_address.FindRegEx ("[A-Z][a-z]") == P_MAX_INDEX &&
	  call_address.Find (".") == P_MAX_INDEX &&
	  endpoint->GetCapabilities ().FindCapability ("G.723.1") == NULL)
	call_address = PString ("0610#") + call_address;
    }


    if (!call_address.IsEmpty ()) {

      /* Disable the preview, and enable Disconnect, this is done
	 in all cases, including when calling an unexisting callto */
      gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
      gnomemeeting_call_menu_connect_set_sensitive (1, TRUE);

      if (!transfer_call) {

	gnomemeeting_main_window_enable_statusbar_progress (true);
	msg = g_strdup_printf (_("Calling %s"), 
			       (const char *) call_address);
      }
      else
	msg = g_strdup_printf (_("Transferring call to %s"), 
			       (const char *) call_address);
      gnomemeeting_log_insert (gw->history_text_view, msg);
      gnomemeeting_statusbar_push (gw->statusbar, msg);
    }
    g_free (msg);

    /* The button is pressed (calling) */
    connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 1);
    gnomemeeting_threads_leave ();
  
    /* Connect to the URL */
    if (!transfer_call) {

      endpoint->SetCallingState (1);
      con = 
	endpoint->MakeCallLocked (call_address, current_call_token);
    }
    else
      if (!url.IsEmpty () && url.IsSupported ()) {
	
	endpoint->TransferCall (endpoint->GetCurrentCallToken (),
				call_address);
	g_timeout_add (11000, (GtkFunction) TransferTimeOut, NULL);
      }
  }
  else {

    /* If we are here, it is because the user specified an invalid
       speed dial to call */
    gnomemeeting_threads_enter ();
    gnomemeeting_statusbar_flash (gw->statusbar,
				  _("No contact with that speed dial found"));
    if (!transfer_call)
      gnomemeeting_calls_history_window_add_call (1,
						  old_url.GetValidURL () + "#",
						  NULL, "0", NULL);
    gnomemeeting_threads_leave ();
  }

  if (!transfer_call) {
    
    /* If we have a valid URL, we a have a valid connection, if not
       we put things back in the initial state */
    if (con) {

      endpoint->SetCurrentConnection (con);
      endpoint->SetCurrentCallToken (current_call_token);
      con->Unlock ();
    }
    else {

      gnomemeeting_threads_enter ();
      connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 0);
      gnomemeeting_main_window_enable_statusbar_progress (false);
      gnomemeeting_call_menu_connect_set_sensitive (1, FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), TRUE);

      if (call_address.Find ("+type=directory") != P_MAX_INDEX) {
	
	gnomemeeting_statusbar_flash (gw->statusbar, _("User not found"));
	if (!transfer_call)
	  gnomemeeting_calls_history_window_add_call (1, call_address,
						      NULL, "0", NULL);
      }
      
      gnomemeeting_threads_leave ();

      endpoint->SetCallingState (0);
    }
  }
}
