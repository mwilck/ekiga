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
#include "applet.h"
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

gint gnome_idle_timer ()
{
  gdk_threads_enter ();
  while (gtk_events_pending())
    gtk_main_iteration();
  gdk_threads_leave ();

  
  usleep (50);
  return TRUE;
}

/******************************************************************************/


/******************************************************************************/
/* The main GnomeMeeting Class                                                */
/******************************************************************************/

GnomeMeeting::GnomeMeeting (GM_window_widgets *s, options *o)
	: PProcess("", "", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE,
		   BUILD_NUMBER)

{
  // no endpoint for the moment
  endpoint = NULL;
  opts = o;

  endpoint = new GMH323EndPoint (s, o);
  endpoint->Initialise();
	
  // as the pointer given as parameter will not be deleted until the end of
  // the execution, we make a simple copy of it.
  gw = s;

  MyApp = (this);
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
      call_address = (PString) gtk_entry_get_text 
	(GTK_ENTRY (GTK_WIDGET(GTK_COMBO(gw->combo)->entry)));

      li = gtk_list_item_new_with_label(call_address);
      gtk_container_add(GTK_CONTAINER(GTK_COMBO(gw->combo)->list), li);
      gtk_widget_show (li);
      gtk_entry_set_text (GTK_ENTRY (GTK_WIDGET(GTK_COMBO(gw->combo)->entry)), 
			  call_address);

      // if we call somebody
      if (!call_address.IsEmpty ())
	{
	  endpoint->SetCurrentConnection (endpoint->MakeCall 
					  (call_address, 
					   current_call_token));
	  endpoint->SetCurrentCallToken (current_call_token);
	  endpoint->SetCallingState (1);

	  GM_log_insert (gw->log_text, _("Calling..."));
	  gnome_appbar_push (GNOME_APPBAR (gw->statusbar), _("Calling..."));

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
	  connection->ClearCall();	// End of Call
	}
      else
	{
	  // if somebody is calling us, or if we are in call with somebody
	  
	  if (endpoint->CallingState () == 2)
	    {
	      GM_log_insert (gw->log_text, _("Stopping current call"));
	      connection->ClearCall();	// End of Call
	    }
	  else
	    {
	      GM_log_insert (gw->log_text, _("Refusing incoming call"));
	      connection = endpoint->FindConnectionWithLock
		(endpoint->CallToken ());
	      connection->AnsweringCall (H323Connection::AnswerCallDenied);	
	      connection->Unlock ();
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

  GM_window_widgets *gw = NULL;
  options *opts;

  gw = new (GM_window_widgets);
  opts = new (options);
  memset (opts, 0, sizeof (options));

  if (config_first_time ())
      init_config ();

  read_config (opts);

  // Init
  g_thread_init(NULL);

  textdomain (PACKAGE);
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  gw->pixmap = NULL;
  gw->applet = NULL;
  gw->pref_window = NULL;
  gw->ldap_window = NULL;

  static struct poptOption arguments[] =
    {
      {"noapplet", 'a', POPT_ARG_NONE, 
       0, 0, N_("Startup without applet support"), NULL},
      {NULL, 0, 0, NULL, 0, NULL, NULL}
    };

  for (int i = 0; i < argc; i++) 
    {
      if (!strcmp (argv[i], "-a") || !strcmp (argv[i], "--noapplet"))
	opts->applet = 0;
      else
	opts->applet = 1;  
    }
  
  // Gnome Initialisation
  if (opts->applet == 0)
    gnome_init_with_popt_table (PACKAGE, VERSION, argc, argv,
				arguments, 0, NULL);
  else
    {
      applet_widget_init( PACKAGE, VERSION, argc, argv,
			  arguments, 0, NULL );
      
      gw->applet = GM_applet_init (argc, argv);
      gtk_widget_show (gw->applet);
    }

  gm = gnome_app_new ("gnomemeeting", "Gnome Meeting");
  gtk_window_set_policy (GTK_WINDOW (gm), FALSE, FALSE, TRUE);

  // Main interface creation
  GM_main_interface_init (gw, opts);

  PTrace::Initialise (6);

  gtk_idle_add ((GtkFunction) gnome_idle_timer, NULL);

  // Set icon
  gtk_widget_push_visual(gdk_imlib_get_visual());
  gtk_widget_push_colormap(gdk_imlib_get_colormap());
  gnome_window_icon_set_from_file 
    (GTK_WINDOW (gm), "/usr/share/pixmaps/gnomemeeting-logo-icon.png");

  // if the user tries to close the window : delete_event
  gtk_signal_connect (GTK_OBJECT (gm), "delete_event",
		      GTK_SIGNAL_FUNC (toggle_window_callback),
		      NULL);

  if (!(opts->show_notebook))
    gtk_widget_hide (gw->main_notebook);

  if (!(opts->show_statusbar))
    gtk_widget_hide (gw->statusbar);

  if (opts->applet)
    applet_widget_gtk_main ();
  else
    gtk_main ();

  delete (opts);
  delete (gw);
 
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


