
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

#undef G_DISABLE_DEPRECATED
#undef GTK_DISABLE_DEPRECATED
#undef GNOME_DISABLE_DEPRECATED
#include "../config.h" 


#include "gatekeeper.h"
#include "gnomemeeting.h"
#include "misc.h"


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
  GConfClient *client = gconf_client_get_default ();
  H323EndPoint *endpoint = NULL;
  GM_pref_window_widgets *pw = NULL;

  int method;

  /* Register using the gatekeeper host */
  method = gconf_client_get_int (GCONF_CLIENT (client),
				  "/apps/gnomemeeting/gatekeeper/registering_method", 0);

  endpoint = (H323EndPoint *) MyApp->Endpoint ();

  gconf_string = gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/gatekeeper/gk_alias", 0);

  /* set the alias */
  if ((gconf_string != NULL)&&(strcmp ("", gconf_string))) {

    /* Remove the old aliases */
    for (int i = endpoint->GetAliasNames ().GetSize () - 1; i >= 1; i--) 
      if (!endpoint->GetAliasNames () [i].IsEmpty ())
	endpoint->RemoveAliasName (endpoint->GetAliasNames () [i]);

    endpoint->AddAliasName (gconf_string);
  }

  g_free (gconf_string);
  gconf_string = NULL;

  /* Fetch the needed data */
  gnomemeeting_threads_enter ();
  pw = gnomemeeting_get_pref_window (gm);
  gnomemeeting_threads_leave ();
  

  /* Use the hostname */
  if (method == 1) {

    gconf_string = gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/gatekeeper/gk_host", 0);

    if (gconf_string == NULL) {
     
      gnomemeeting_threads_enter ();
      msg_box = gnome_message_box_new (_("Please provide a hostname to use for the gatekeeper"), GNOME_MESSAGE_BOX_ERROR, GNOME_STOCK_BUTTON_OK, NULL);
      gtk_widget_show (msg_box);
      gnomemeeting_threads_leave ();

      return;
    }

    H323TransportUDP *ras_channel = new H323TransportUDP (*MyApp->Endpoint ());
      
    if (MyApp->Endpoint ()->SetGatekeeper(PString (gconf_string), ras_channel)) {
 
      msg = g_strdup_printf (_("Gatekeeper set to %s"), 
			     (const char*) MyApp->Endpoint ()
			     ->GetGatekeeper ()->GetName ());
	  
      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (msg);
      gnomemeeting_threads_leave ();
      
      g_free (msg);
    } 
    else {

      msg = g_strdup_printf (_("Error while registering with Gatekeeper at %s."), gconf_string);
      
      gnomemeeting_threads_enter ();
      msg_box = gnome_message_box_new (msg, GNOME_MESSAGE_BOX_ERROR, 
				       "OK", NULL);
      
      gtk_widget_show (msg_box);
      gnomemeeting_threads_leave ();
      
      g_free (msg);
    }

    g_free (gconf_string);
  }
  

  /* Register using the gatekeeper ID */
  if (method == 2) {

    gconf_string = gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/gatekeeper/gk_id", 0);

    if (gconf_string == NULL) {
     
      gnomemeeting_threads_enter ();
      msg_box = gnome_message_box_new (_("Please provide a valid ID for the gatekeeper"), GNOME_MESSAGE_BOX_ERROR, GNOME_STOCK_BUTTON_OK, NULL);
      gtk_widget_show (msg_box);
      gnomemeeting_threads_leave ();

      return;
    }


    if (MyApp->Endpoint ()->LocateGatekeeper(PString (gconf_string))) {
 
      msg = g_strdup_printf (_("Gatekeeper set to %s"), 
			     (const char*) MyApp->Endpoint ()
			     ->GetGatekeeper ()->GetName ());

      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (msg);
      gnomemeeting_threads_leave ();
      
      g_free (msg);
    } 
    else {

      msg = g_strdup_printf (_("Error while registering with Gatekeeper %s."),
			     "");
      
      gnomemeeting_threads_enter ();
      msg_box = gnome_message_box_new (msg, GNOME_MESSAGE_BOX_ERROR, 
				       "OK", NULL);
	  
      gtk_widget_show (msg_box);
      gnomemeeting_threads_leave ();
      
      g_free (msg);
    }
  }
  

  /* Register after trying to discover the Gatekeeper */
  if (method == 3) {

    if (MyApp->Endpoint ()
	->DiscoverGatekeeper (new H323TransportUDP (*MyApp->Endpoint ()))) {
 
      msg = g_strdup_printf (_("Gatekeeper set to %s"), 
			     (const char*) MyApp->Endpoint ()
			     ->GetGatekeeper ()->GetName ());

      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (msg);
      gnomemeeting_threads_leave ();
      
      g_free (msg);
    } 
    else {

      gnomemeeting_threads_enter ();
      msg_box = gnome_message_box_new (_("No gatekeeper found."), 
				       GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
      
      gtk_widget_show (msg_box);
      gnomemeeting_threads_leave ();
    }
  }
}
