
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


#include "gatekeeper.h"
#include "gnomemeeting.h"
#include "misc.h"


/* Declarations */
extern GnomeMeeting *MyApp;
extern GtkWidget *gm;

/* The class */
GMH323Gatekeeper::GMH323Gatekeeper (options *o)
  :PThread (1000, AutoDeleteThread)
{
  gw = gnomemeeting_get_main_window (gm);
  opts = o;
  
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

  /* Register using the gatekeeper host */
  if (opts->gk == 1) {

    PString gk_host = opts->gk_host;
      
    H323TransportUDP *ras_channel = new H323TransportUDP (*MyApp->Endpoint ());
      
    if (MyApp->Endpoint ()->SetGatekeeper(gk_host, ras_channel)) {
 
      msg = g_strdup_printf (_("Gatekeeper set to %s"), 
			     (const char*) MyApp->Endpoint ()
			     ->GetGatekeeper ()->GetName ());
	  
      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (msg);
      gnomemeeting_threads_leave ();
      
      g_free (msg);
    } 
    else {

      msg = g_strdup_printf (_("Error while registering with Gatekeeper at %s."), opts->gk_host);
      
      gnomemeeting_threads_enter ();
      msg_box = gnome_message_box_new (msg, GNOME_MESSAGE_BOX_ERROR, 
				       "OK", NULL);
      
      gtk_widget_show (msg_box);
      gnomemeeting_threads_leave ();
      
      g_free (msg);
    }
  }
  

  /* Register using the gatekeeper ID */
  if (opts->gk == 2) {

    PString gk_id = opts->gk_id;
    
    if (MyApp->Endpoint ()->LocateGatekeeper(gk_id)) {
 
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
			     opts->gk_id);
      
      gnomemeeting_threads_enter ();
      msg_box = gnome_message_box_new (msg, GNOME_MESSAGE_BOX_ERROR, 
				       "OK", NULL);
	  
      gtk_widget_show (msg_box);
      gnomemeeting_threads_leave ();
      
      g_free (msg);
    }
  }
  
  /* Register after trying to discover the Gatekeeper */
  if (opts->gk == 3) {

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
