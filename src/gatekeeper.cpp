
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2001 Damien Sandras
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
 */

/*
 *                         gatekeeper.cpp  -  description
 *                         ------------------------------
 *   begin                : Wed Sep 19 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : Multithreaded class to register to gatekeepers.
 *   email                : dsandras@seconix.com
 *
 */

#include "../config.h" 


#include "gatekeeper.h"
#include "gnomemeeting.h"
#include "misc.h"

#ifndef DISABLE_GNOME
#include <gnome.h>
#endif


/* Declarations */
extern GnomeMeeting *MyApp;
extern GtkWidget *gm;

/* The class */
GMH323Gatekeeper::GMH323Gatekeeper ()
  :PThread (1000, AutoDeleteThread)
{
  gw = gnomemeeting_get_main_window (gm);
  
  this->Resume ();
}


GMH323Gatekeeper::~GMH323Gatekeeper ()
{
  /* Nothing to do here */
}


void GMH323Gatekeeper::Main ()
{
  GtkWidget *msg_box = NULL;
  gchar *msg = NULL;
  gchar *gconf_string = NULL;
  GConfClient *client = NULL;
  H323EndPoint *endpoint = NULL;
  GmPrefWindow *pw = NULL;
  GmWindow *gw = NULL;

  int method;

  /* Register using the gatekeeper host */
  gnomemeeting_threads_enter ();
  client = gconf_client_get_default ();
  method =
    gconf_client_get_int (GCONF_CLIENT (client),
			  "/apps/gnomemeeting/gatekeeper/registering_method",
			  0);
  gnomemeeting_threads_leave ();

  endpoint = (H323EndPoint *) MyApp->Endpoint ();

  gnomemeeting_threads_enter ();
  gconf_string = gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/gatekeeper/gk_password", 0);
  gnomemeeting_threads_leave ();

  endpoint->SetGatekeeperPassword ("");
  if ((gconf_string != NULL)&&(strcmp ("", gconf_string))) {

    endpoint->SetGatekeeperPassword (gconf_string);
  }
  g_free (gconf_string);
  gconf_string = NULL;

  MyApp->Endpoint ()->SetUserNameAndAlias ();


  /* Fetch the needed data */
  gnomemeeting_threads_enter ();
  pw = gnomemeeting_get_pref_window (gm);
  gw = gnomemeeting_get_main_window (gm);
  gnomemeeting_threads_leave ();
  

  /* Use the hostname */
  if (method == 1) {

    gnomemeeting_threads_enter ();
    gconf_string = gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/gatekeeper/gk_host", 0);
    gnomemeeting_threads_leave ();

    if ((gconf_string == NULL) || (!strcmp ("", gconf_string))) {
     
      gnomemeeting_threads_enter ();
      msg_box = 
	gtk_message_dialog_new (GTK_WINDOW (gm),
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				_("Please provide a hostname to use for the gatekeeper.\nDisabling registering."));

      gtk_widget_show (msg_box);
      g_signal_connect_swapped (GTK_OBJECT (msg_box), "response",
				G_CALLBACK (gtk_widget_destroy),
				GTK_OBJECT (msg_box));

      gconf_client_set_int (GCONF_CLIENT (client), "/apps/gnomemeeting/gatekeeper/registering_method", 0, NULL);
  
      /* We disable microtelco if it was enabled */
      gconf_client_set_bool (client, SERVICES_KEY "enable_microtelco",
			     false, 0);
      gnomemeeting_threads_leave ();

      return;
    }


    if (MyApp->Endpoint ()->SetGatekeeper(PString (gconf_string))) {
 
      msg = g_strdup_printf (_("Gatekeeper set to %s"), 
			     (const char*) MyApp->Endpoint ()
			     ->GetGatekeeper ()->GetName ());
	  
      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (gw->history_text_view, msg);
      gnomemeeting_statusbar_flash (gw->statusbar, msg);

      /* If the host to which we registered is the MicroTelco GK, enable
	 MicroTelco, if not disable it, in case it was enabled */
      if (PString (gconf_string).Find ("gk.microtelco.com") != P_MAX_INDEX)
	gconf_client_set_bool (client, SERVICES_KEY "enable_microtelco",
			       true, 0);
      else
	gconf_client_set_bool (client, SERVICES_KEY "enable_microtelco",
			       false, 0);
      gnomemeeting_threads_leave ();

      g_free (msg);
    } 
    else {

      msg = g_strdup_printf (_("Error while registering with Gatekeeper at %s."), 
			     gconf_string);
      
      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (gw->history_text_view, msg);
      msg_box = 
	gtk_message_dialog_new (GTK_WINDOW (gm),
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				msg);

      gtk_widget_show (msg_box);
      g_signal_connect_swapped (GTK_OBJECT (msg_box), "response",
				G_CALLBACK (gtk_widget_destroy),
				GTK_OBJECT (msg_box));
      
      gconf_client_set_int (GCONF_CLIENT (client), "/apps/gnomemeeting/gatekeeper/registering_method", 0, NULL);

      /* We disable microtelco if it was enabled */
      gconf_client_set_bool (client, SERVICES_KEY "enable_microtelco",
			     false, 0);
      gnomemeeting_threads_leave ();

      g_free (msg);
    }

    g_free (gconf_string);
  }
  

  /* Register using the gatekeeper ID */
  if (method == 2) {

    /* We disable microtelco if it was enabled */
    gnomemeeting_threads_enter ();
    gconf_client_set_bool (client, SERVICES_KEY "enable_microtelco",
			   false, 0);
    gconf_string = gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/gatekeeper/gk_id", 0);
    gnomemeeting_threads_leave ();

    if ((gconf_string == NULL) || (!strcmp ("", gconf_string))) {
     
      gnomemeeting_threads_enter ();
      msg_box = 
	gtk_message_dialog_new (GTK_WINDOW (gm),
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				_("Please provide a valid ID for the gatekeeper.\nDisabling registering."));

      gtk_widget_show (msg_box);
      g_signal_connect_swapped (GTK_OBJECT (msg_box), "response",
				G_CALLBACK (gtk_widget_destroy),
				GTK_OBJECT (msg_box));
    
      gconf_client_set_int (GCONF_CLIENT (client), "/apps/gnomemeeting/gatekeeper/registering_method", 0, NULL);
      gnomemeeting_threads_leave ();

      return;
    }


    if (MyApp->Endpoint ()->LocateGatekeeper(PString (gconf_string))) {
 
      msg = g_strdup_printf (_("Gatekeeper set to %s"), 
			     (const char*) MyApp->Endpoint ()
			     ->GetGatekeeper ()->GetName ());

      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (gw->history_text_view, msg);
      gnomemeeting_statusbar_flash (gw->statusbar, msg);
      gnomemeeting_threads_leave ();
      
      g_free (msg);
    } 
    else {

      msg = g_strdup_printf (_("Error while registering with Gatekeeper."));
      
      gnomemeeting_threads_enter ();
      msg_box = 
	gtk_message_dialog_new (GTK_WINDOW (gm),
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				msg);

      gtk_widget_show (msg_box);
      g_signal_connect_swapped (GTK_OBJECT (msg_box), "response",
				G_CALLBACK (gtk_widget_destroy),
				GTK_OBJECT (msg_box));
  
      gconf_client_set_int (GCONF_CLIENT (client), "/apps/gnomemeeting/gatekeeper/registering_method", 0, NULL);
      gnomemeeting_threads_leave ();

      g_free (msg);
    }
  }
  

  /* Register after trying to discover the Gatekeeper */
  if (method == 3) {

    /* We disable microtelco if it was enabled */
    gnomemeeting_threads_enter ();
    gconf_client_set_bool (client, SERVICES_KEY "enable_microtelco",
			   false, 0);
    gnomemeeting_threads_leave ();

    if (MyApp->Endpoint ()->DiscoverGatekeeper ()) {
 
      msg = g_strdup_printf (_("Gatekeeper set to %s"), 
			     (const char*) MyApp->Endpoint ()
			     ->GetGatekeeper ()->GetName ());

      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (gw->history_text_view, msg);
      gnomemeeting_statusbar_flash (gw->statusbar, msg);
      gnomemeeting_threads_leave ();
      
      g_free (msg);
    } 
    else {

      gnomemeeting_threads_enter ();
      msg_box = 
	gtk_message_dialog_new (GTK_WINDOW (gm),
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				_("No gatekeeper found"));

      gtk_widget_show (msg_box);
      g_signal_connect_swapped (GTK_OBJECT (msg_box), "response",
				G_CALLBACK (gtk_widget_destroy),
				GTK_OBJECT (msg_box));
  
      gconf_client_set_int (GCONF_CLIENT (client), "/apps/gnomemeeting/gatekeeper/registering_method", 0, NULL);
      gnomemeeting_threads_leave ();

    }
  }

  gnomemeeting_threads_enter ();
  if (method == 0)
    /* We disable microtelco if it was enabled */
    gconf_client_set_bool (client, SERVICES_KEY "enable_microtelco",
			   false, 0);
  gnomemeeting_threads_leave ();
}
