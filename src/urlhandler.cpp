
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
#include "toolbar.h"
#include "menu.h"

#include "dialog.h"

/* Declarations */
extern GnomeMeeting *MyApp;
extern GtkWidget *gm;


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
GMURLHandler::GMURLHandler (PString c)
  :PThread (1000, AutoDeleteThread)
{
  gw = gnomemeeting_get_main_window (gm);
  url = GMURL (c);

  answer_call = FALSE;
  
  this->Resume ();
}


GMURLHandler::GMURLHandler ()
  :PThread (1000, AutoDeleteThread)
{
  gw = gnomemeeting_get_main_window (gm);

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

  gchar *msg = NULL;
  
  GMH323EndPoint *endpoint = NULL;
  H323Connection *con = NULL;

  GConfClient *client = NULL;

  PWaitAndSignal m(quit_mutex);
  
  gnomemeeting_threads_enter ();
  client = gconf_client_get_default ();
  gw = gnomemeeting_get_main_window (gm);
  gnomemeeting_threads_leave ();
  
  endpoint = MyApp->Endpoint ();

  if (answer_call && endpoint || endpoint->GetCallingState ()) {

    con = endpoint->GetCurrentConnection ();

    if (con)
      con->AnsweringCall (H323Connection::AnswerCallNow);

    return;
  }
  
  if (url.IsEmpty ())
    return;

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

    gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
    gnomemeeting_call_menu_connect_set_sensitive (1, TRUE);
    msg = g_strdup_printf (_("Calling %s"), 
			   (const char *) call_address);
    gnomemeeting_log_insert (gw->history_text_view, msg);
    gnomemeeting_statusbar_push (gw->statusbar, msg);
    connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 1);
    g_free (msg);		
    gnomemeeting_threads_leave ();
  
    /* Connect to the URL */
    endpoint->SetCallingState (1);
  
    con = 
      endpoint->MakeCallLocked (call_address, current_call_token);
  }
  else
    gnomemeeting_statusbar_flash (gw->statusbar, _("No contact with that speed dial found"));


  /* If we have a valid URL, we a have a valid connection, if not
     we put things back in the initial state */
  if (con) {
    
    endpoint->SetCurrentConnection (con);
    endpoint->SetCurrentCallToken (current_call_token);
    con->Unlock ();
  }
  else {
    
    endpoint->SetCallingState (0);

    gnomemeeting_threads_enter ();
    connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 0);
    gnomemeeting_call_menu_connect_set_sensitive (1, FALSE);

    if (gw->progress_timeout) {

      gtk_timeout_remove (gw->progress_timeout);
      gw->progress_timeout = 0;
      gtk_widget_hide (gw->progressbar);
    }

    if (call_address.Find ("+type=directory") != P_MAX_INDEX)
      gnomemeeting_statusbar_flash (gw->statusbar, _("User not found"));

    gnomemeeting_threads_leave ();
  }
}
