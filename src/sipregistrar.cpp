
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
 *                         sipregistrar.cpp  -  description
 *                         --------------------------------
 *   begin                : Wed Dec 8 2004
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Multithreaded class to register to registrars
 *                          given the options in config.
 *
 */


#include "../config.h" 

#include "sipregistrar.h"
#include "sipendpoint.h"
#include "gnomemeeting.h"
#include "main_window.h"
#include "misc.h"
#include "log_window.h"

#include "dialog.h"
#include "gm_conf.h"


/* The class */
GMSIPRegistrar::GMSIPRegistrar ()
  :PThread (1000, NoAutoDeleteThread)
{
  gchar *conf_string = NULL;
    
  /* Query the config database for options */
  gnomemeeting_threads_enter ();
  registering_method =
    gm_conf_get_int (SIP_KEY "registrar_registering_method");

  /* Login */
  conf_string = gm_conf_get_string (SIP_KEY "registrar_login");
  if (conf_string) {
    
    registrar_login = PString (conf_string);
    g_free (conf_string);
  }
  
  /* Realm */
  conf_string = gm_conf_get_string (SIP_KEY "registrar_realm");
  if (conf_string) {
    
    registrar_realm = PString (conf_string);
    g_free (conf_string);
  }
  
  /* Password */
  conf_string = gm_conf_get_string (SIP_KEY "registrar_password");
  if (conf_string) {
    
    registrar_password = PString (conf_string);
    g_free (conf_string);
  }

  /* Host */
  conf_string = gm_conf_get_string (SIP_KEY "registrar_host");
  if (conf_string) {

    registrar_host = PString (conf_string);
    g_free (conf_string);
  }

  gnomemeeting_threads_leave ();
  
  this->Resume ();
}


GMSIPRegistrar::~GMSIPRegistrar ()
{
  /* Nothing to do here */
  PWaitAndSignal m(quit_mutex);
}


void GMSIPRegistrar::Main ()
{
  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;

  PString gk_name;
  gchar *msg = NULL;

  GMEndPoint *endpoint = NULL;
  GMSIPEndPoint *sipEP = NULL;
  
  BOOL no_error = TRUE;
  
  endpoint = GnomeMeeting::Process ()->Endpoint ();
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  sipEP = endpoint->GetSIPEndPoint ();
  

  PWaitAndSignal m(quit_mutex);
  
  
  if (registering_method == 0) {
    
    sipEP->Unregister (registrar_host);
  }
  /* Check if we have all the needed information, if so we register */
  else if (registering_method == 1) {

    if (registrar_host.IsEmpty ()) {

      gnomemeeting_threads_enter ();
      gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Invalid registrar hostname"), _("Please provide a hostname to use for the registrar."));
      gnomemeeting_threads_leave ();

      return;
    }
    else {

      if (!sipEP->Register (registrar_host, 
			    registrar_login, 
			    registrar_password))
	no_error = FALSE;
    }
  }
  

  /* There was an error (missing parameter or registration failed)
     or the user chose to not register */
  if (!no_error) {    
    
    msg = g_strdup_printf (_("Registration to %s failed"), 
			   (const char *) registrar_host);

    gnomemeeting_threads_enter ();
    gm_main_window_push_message (main_window, msg);
    gm_history_window_insert (history_window, msg);
    g_free (msg);
    gnomemeeting_threads_leave ();
  }
}
