/***************************************************************************
                          gatekeeper.cpp  -  description
                             -------------------
    begin                : Wed Sep 19 2001
    copyright            : (C) 2001 by Damien Sandras
    description          : Multithreaded class to register to gatekeepers
    email                : dsandras@seconix.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "gatekeeper.h"
#include "main_interface.h"
#include "main.h"

extern GnomeMeeting *MyApp;


GMH323Gatekeeper::GMH323Gatekeeper (GM_window_widgets *g, options *o)
  :PThread (1000, AutoDeleteThread)
{
  gw = g;
  opts = o;

  this->Resume ();
}


GMH323Gatekeeper::~GMH323Gatekeeper ()
{
  // Nothing to do here
}


void GMH323Gatekeeper::Main ()
{
  // When this function is called, 
  GtkWidget *msg_box = NULL;
  gchar *msg = NULL;

  // Register using the gatekeeper host
  if (opts->gk == 1)
    {
      PString gk_host = opts->gk_host;
      
      H323TransportUDP * ras_channel = new H323TransportUDP (*MyApp->Endpoint ());
      
      if (MyApp->Endpoint ()->SetGatekeeper(gk_host, ras_channel)) 
	{ 
	  msg = g_strdup_printf (_("Gatekeeper set to %s"), 
				 (const char*) MyApp->Endpoint ()->Gatekeeper ()->GetName ());
	  
	  gdk_threads_enter ();
	  GM_log_insert (gw->log_text, msg);
	  gdk_threads_leave ();
	  
	  g_free (msg);
	} 
      else 
	{
	  msg = g_strdup_printf (_("Error while registering with Gatekeeper at %s."),
				 opts->gk_host);

	  gdk_threads_enter ();
	  msg_box = gnome_message_box_new (msg, GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
	      
	  gtk_widget_show (msg_box);
	  gdk_threads_leave ();
	      
	  g_free (msg);
	}
    }
  
  // Register using the gatekeeper ID
  if (opts->gk == 2)
    {
      PString gk_id = opts->gk_id;
      
      if (MyApp->Endpoint ()->LocateGatekeeper(gk_id)) 
	{ 
	  msg = g_strdup_printf (_("Gatekeeper set to %s"), 
				 (const char*) MyApp->Endpoint ()->Gatekeeper ()->GetName ());

	  gdk_threads_enter ();
	  GM_log_insert (gw->log_text, msg);
	  gdk_threads_leave ();

	  g_free (msg);
	} 
      else 
	{
	  msg = g_strdup_printf (_("Error while registering with Gatekeeper %s."),
				 opts->gk_id);

	  gdk_threads_enter ();
	  msg_box = gnome_message_box_new (msg, GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
	  
	  gtk_widget_show (msg_box);
	  gdk_threads_leave ();

	  g_free (msg);
	}
    }
  
  // Register after trying to discover the Gatekeeper
  if (opts->gk == 3)
    {
      if (MyApp->Endpoint ()->DiscoverGatekeeper(new H323TransportUDP(*MyApp->Endpoint ())))
	{ 
	  msg = g_strdup_printf (_("Gatekeeper set to %s"), 
				 (const char*) MyApp->Endpoint ()->Gatekeeper ()->GetName ());

	  gdk_threads_enter ();
	  GM_log_insert (gw->log_text, msg);
	  gdk_threads_leave ();
	  
	  g_free (msg);
	} 
      else 
	{
	  gdk_threads_enter ();
	  msg_box = gnome_message_box_new (_("No gatekeeper found."), 
					   GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
	  
	  gtk_widget_show (msg_box);
	  gdk_threads_leave ();
	}
    }
}
/******************************************************************************/
