/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Sat Dec 23 2000
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : This file contains all central functions to cope
                           with OpenH323
    email                : dsandras@acm.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "../config.h"

#include "endpoint.h"
#include "callbacks.h"
#include "main.h"
#include "main_interface.h"
#include "toolbar.h"
#include "config.h"

#define new PNEW


/******************************************************************************/
/* Global Variables                                                           */
/******************************************************************************/

GtkWidget *gm;
GnomeMeeting *MyApp;	

/******************************************************************************/


/******************************************************************************/
/* GTK Callbacks                                                              */
/******************************************************************************/

gint gnome_idle_timer (void)
{
  gdk_threads_enter ();
  while (gtk_events_pending())
    gtk_main_iteration(); 
  gdk_threads_leave ();

  usleep (100);
  return TRUE;
}


gint AppbarUpdate (GtkWidget *statusbar)
{
  long minutes, seconds;
  double audio_tr_ko, audio_re_ko, video_tr_ko, video_re_ko;

  if (MyApp->Endpoint ()) 
    {
      if (MyApp->Endpoint ()->Connection ())
	{
	  PTimeInterval t =
	    PTime () - MyApp->Endpoint ()->Connection ()
	    ->GetConnectionStartTime();

	  if (t.GetSeconds () > 2)
	    {
	      minutes = t.GetMinutes () % 60;
	      seconds = t.GetSeconds () % 60;

	      gchar *msg = g_strdup_printf (_("Connection Time: %.2ld:%.2ld:%.2ld"), 
					    t.GetHours (), minutes, seconds);

	      gnome_appbar_push (GNOME_APPBAR (statusbar), msg);
	      
	      g_free (msg);
	    }
	}
    }

  return TRUE;
}

/******************************************************************************/


/******************************************************************************/
/* The main GnomeMeeting Class                                                */
/******************************************************************************/

GnomeMeeting::GnomeMeeting (GM_window_widgets *s, GM_ldap_window_widgets *l,
			    options *o)
	: PProcess("", "", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE,
		   BUILD_NUMBER)

{
  // no endpoint for the moment
  endpoint = NULL;
  opts = o;
  gw = s;
  lw = l;
  MyApp = (this);
  endpoint = new GMH323EndPoint (gw, lw, opts);
  call_number = 0;
}


GnomeMeeting::~GnomeMeeting()
{
  delete (endpoint);
  endpoint = NULL;
}


void GnomeMeeting::AddContactIP (char *ip)
{
  GtkWidget *li;
  li = gtk_list_item_new_with_label(ip);
  gtk_container_add(GTK_CONTAINER(GTK_COMBO(gw->combo)->list), li);
  gtk_widget_show (li);
  gtk_entry_set_text (GTK_ENTRY (GTK_WIDGET(GTK_COMBO(gw->combo)->entry)), ip);
}
  

void GnomeMeeting::Connect()
{
  PString call_address;
  PString current_call_token;
  H323Connection *connection;
  GtkWidget *li;

  // We need a connection to use AnsweringCall
  current_call_token = endpoint->CallToken ();
  connection = endpoint->FindConnectionWithLock (current_call_token);
  call_address = (PString) gtk_entry_get_text 
    (GTK_ENTRY (GTK_WIDGET(GTK_COMBO(gw->combo)->entry)));

  // If connection, then answer it
  if (connection != NULL)
    {
      enable_disconnect ();
      disable_connect ();

      connection->AnsweringCall (H323Connection::AnswerCallNow);
      connection->Unlock ();


      GM_log_insert (gw->log_text, _("Answering incoming call"));
    }
  else
    {
      li = gtk_list_item_new_with_label(call_address);
      gtk_container_add(GTK_CONTAINER(GTK_COMBO(gw->combo)->list), li);
      gtk_widget_show (li);
      gtk_entry_set_text (GTK_ENTRY (GTK_WIDGET(GTK_COMBO(gw->combo)->entry)), 
			  call_address);

      // if we call somebody
      if (!call_address.IsEmpty ())
	{
	  call_number++;
	  gchar *msg = NULL;
	  endpoint->SetCurrentConnection (endpoint->MakeCall 
					  (call_address, 
					   current_call_token));
	  endpoint->SetCurrentCallToken (current_call_token);
	  endpoint->SetCallingState (1);
	  gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
	  msg = g_strdup_printf (_("Call %d: calling %s"), 
				 call_number,
				 (const char *) call_address);
	  GM_log_insert (gw->log_text, msg);
	  gnome_appbar_push (GNOME_APPBAR (gw->statusbar), msg);
	  g_free (msg);				 
	  enable_disconnect ();
	  disable_connect ();
	}			
    }
  
}


void GnomeMeeting::Disconnect()
{
  // If somebody is calling us, then we do not accept the connection
  // else we finish it
  H323Connection *connection = endpoint->Connection ();
  PString current_call_token = endpoint->CallToken ();

  if (!current_call_token.IsEmpty ())
    {
      // if we are trying to call somebody
      if (endpoint->CallingState () == 1)
	{
	  GM_log_insert (gw->log_text, _("Trying to stop calling"));
	  // End of Call
	  if (!endpoint->ClearCall(current_call_token))
	    GM_log_insert (gw->log_text, _("Failed to stop current call"));
	}
      else
	{
	  // if somebody is calling us, or if we are in call with somebody
	  
	  if (endpoint->CallingState () == 2)
	    {
	      GM_log_insert (gw->log_text, _("Stopping current call"));
	      // End of Call
	      if (!endpoint->ClearCall(current_call_token))
		GM_log_insert (gw->log_text, _("Failed to stop current call"));
	    }
	  else
	    {
	      GM_log_insert (gw->log_text, _("Refusing Incoming call"));
	      connection->AnsweringCall (H323Connection::AnswerCallDenied);	
	    }
	}
    } 
}


GMH323EndPoint *GnomeMeeting::Endpoint ()
{
  return endpoint;
}


void GnomeMeeting::Main()
{
}

/******************************************************************************/


/******************************************************************************/
/* The main ()                                                                */
/******************************************************************************/

int main (int argc, char ** argv, char ** envp)
{
  PProcess::PreInitialise (argc, argv, envp);

  // The different structures needed by most of the classes and functions
  GM_window_widgets *gw = NULL;
  GM_ldap_window_widgets *lw = NULL;
  options *opts = NULL;
  GM_pref_window_widgets *pw = NULL;

  // Init the GM_window_widgets
  gw = new (GM_window_widgets);
  gw->pixmap = NULL;
  gw->pref_window = NULL;
  gw->ldap_window = NULL;
  gw->video_grabber_thread_count = 0;
  gw->cleaner_thread_count = 0;

  // Init the GM_pref_window_widgets structure
  pw = new (GM_pref_window_widgets); 
  pw->gw = gw;
  pw->ldap_changed = 0;
  pw->audio_mixer_changed = 0;
  pw->gk_changed = 0;
  pw->capabilities_changed = 0;

  // Init the GM_ldap_window_widgets structure
  lw = new (GM_ldap_window_widgets);

  // Init the opts
  opts = new (options);
  memset (opts, 0, sizeof (options));

  // Threads + Locale Init
  g_thread_init(NULL);

  textdomain (PACKAGE);
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);

  GM_init (gw, pw, lw, opts, argc, argv, envp);

  gtk_idle_add ((GtkFunction) gnome_idle_timer, gw);

  gtk_timeout_add (1000, (GtkFunction) AppbarUpdate, 
		   gw->statusbar);

  gtk_main ();

  read_config_from_gui (gw, lw, opts);

  // We delete what we allocated, 
  // and save the options before to quit
  store_config (opts);

  g_options_free (opts);
  delete (opts);
  delete (gw);
  delete (lw);

  return 0;
}

// Needed for RedHat Systems

extern "C" 
{

  void SHA1_Init()
  {
  }

  void SHA1_Update()
  {
  }

  void SHA1_Final()
  {
  }

  void SHA1()
  {
  }
}


