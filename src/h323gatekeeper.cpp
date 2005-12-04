
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
 *                         h323gatekeeper.cpp  -  description
 *                         ----------------------------------
 *   begin                : Wed Sep 19 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Multithreaded class to register to gatekeepers
 *                          given the options in config.
 *
 */


#include "../config.h" 

#include "h323gatekeeper.h"
#include "h323endpoint.h"
#include "gnomemeeting.h"
#include "main_window.h"
#include "misc.h"
#include "log_window.h"

#include "dialog.h"
#include "gm_conf.h"


/* The class */
GMH323Gatekeeper::GMH323Gatekeeper ()
  :PThread (1000, NoAutoDeleteThread)
{
  gchar *conf_string = NULL;
    
  /* Query the config database for options */
  gnomemeeting_threads_enter ();
  registering_method =
    gm_conf_get_int (H323_KEY "gatekeeper_registering_method");

  /* Gatekeeper password */
  conf_string = gm_conf_get_string (H323_KEY "gatekeeper_password");
  if (conf_string) {
    
    gk_password = PString (conf_string);
    g_free (conf_string);
  }

  /* Gatekeeper host */
  if (registering_method == 1) {
    
    conf_string = gm_conf_get_string (H323_KEY "gatekeeper_host");
    if (conf_string) {
      
      gk_host = PString (conf_string);
      g_free (conf_string);
    }
  }

  gnomemeeting_threads_leave ();
  
  this->Resume ();
}


GMH323Gatekeeper::~GMH323Gatekeeper ()
{
  /* Nothing to do here */
  PWaitAndSignal m(quit_mutex);
}


void GMH323Gatekeeper::Main ()
{
  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;

  PString gk_name;
  gchar *msg = NULL;

  GMEndPoint *endpoint = NULL;
  GMH323EndPoint *h323EP = NULL;
  H323Gatekeeper *gatekeeper = NULL;
  
  BOOL no_error = TRUE;
  
  endpoint = GnomeMeeting::Process ()->Endpoint ();
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  h323EP = endpoint->GetH323EndPoint ();
  

  PWaitAndSignal m(quit_mutex);
  
  /* Remove the current Gatekeeper */
  gatekeeper = h323EP->GetGatekeeper ();
  if (gatekeeper) {

    gk_name = gatekeeper->GetName ();
    msg = g_strdup_printf (_("Unregistered from gatekeeper %s"),
			   (const char *) gk_name);
    gnomemeeting_threads_enter ();
    gm_history_window_insert (history_window, msg);
    gm_main_window_flash_message (main_window, msg);
    g_free (msg);
    gnomemeeting_threads_leave ();
  }
  h323EP->RemoveGatekeeper (0);  
  h323EP->SetUserNameAndAlias (); 

  
  /* Check if we have all the needed information, if so we register */
  if (registering_method == 1 && gk_host.IsEmpty ()) {
  
    gnomemeeting_threads_enter ();
    gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Invalid gatekeeper hostname"), _("Please provide a hostname to use for the gatekeeper."));
    gnomemeeting_threads_leave ();

    return;
  }
  else {

    /* Set the gatekeeper password */
    h323EP->SetGatekeeperPassword ("");
    if (!gk_password.IsEmpty ())
      h323EP->SetGatekeeperPassword (gk_password);

    
    /* Registers to the gk */
    if (registering_method == 2) {
    
      if (!h323EP->UseGatekeeper ())
	no_error = FALSE;
    }
    else if (registering_method == 1) {
      
      if (!h323EP->UseGatekeeper (gk_host, PString ()))
	no_error = FALSE;
    }
  }

  
  /* There was an error (missing parameter or registration failed)
     or the user chose to not register */
  if (!no_error) {
      
    /* Registering failed */
    gatekeeper = h323EP->GetGatekeeper ();
    if (gatekeeper) {

      switch (gatekeeper->GetRegistrationFailReason()) {

      case H323Gatekeeper::DuplicateAlias :
	msg = g_strdup (_("Gatekeeper registration failed: duplicate alias"));
	break;
      case H323Gatekeeper::SecurityDenied :
	msg = 
	  g_strdup (_("Gatekeeper registration failed: bad login/password"));
	break;
      case H323Gatekeeper::TransportError :
	msg = g_strdup (_("Gatekeeper registration failed: transport error"));
	break;
      default :
	msg = g_strdup (_("Gatekeeper registration failed"));
	break;
      }
    }
    else
      msg = g_strdup (_("Gatekeeper registration failed"));

    gnomemeeting_threads_enter ();
    gm_main_window_push_message (main_window, msg);
    gm_history_window_insert (history_window, msg);
    g_free (msg);
    gnomemeeting_threads_leave ();
  }
  /* Registering is ok */
  else if (registering_method != 0) {

    gatekeeper = h323EP->GetGatekeeper ();
    if (gatekeeper)
      gk_name = gatekeeper->GetName ();
    msg =
      g_strdup_printf (_("Gatekeeper set to %s"), (const char *) gk_name);
    
    gnomemeeting_threads_enter ();
    gm_history_window_insert (history_window, msg);
    gm_main_window_flash_message (main_window, msg);
    gnomemeeting_threads_leave ();
      
    g_free (msg);
  } 
}
