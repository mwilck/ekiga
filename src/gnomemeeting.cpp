
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
 *                         main.cpp  -  description
 *                         ------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains the main class
 *   email                : dsandras@seconix.com
 *
 */


#include "../config.h"

#include "endpoint.h"
#include "callbacks.h"
#include "gnomemeeting.h"
#include "main_window.h"
#include "toolbar.h"
#include "config.h"
#include "misc.h"
#include <gconf/gconf-client.h>


#define new PNEW

/* Declarations */

GtkWidget *gm;
GnomeMeeting *MyApp;	


/* DESCRIPTION  :  This Timer is called when Gnome erroneously thinks that
 *                 it has nothing to do.
 * BEHAVIOR     :  It treats signals if needed.
 * PRE          :  /
 */
static gint gnome_idle_timer (void);


/* DESCRIPTION  :  This Timer is called evry second.
 * BEHAVIOR     :  Elapsed time since the beginning of the connection 
 *                 is displayed.
 * PRE          :
 */
static gint AppbarUpdate (GtkWidget *);


/* GTK Callbacks */

gint gnome_idle_timer (void)
{
  /* we can't call gnomemeeting_threads_enter as idles and timers
     are executed in the main thread */
  gdk_threads_enter ();
  while (gtk_events_pending())
    gtk_main_iteration(); 
  gdk_threads_leave ();

  usleep (500);
  return TRUE;
}


gint AppbarUpdate (GtkWidget *statusbar)
{
  long minutes, seconds;

  gdk_threads_enter ();
/*
  if (MyApp->Endpoint ()) {

      if ((MyApp->Endpoint ()->GetCurrentConnection ()) 
	  &&(MyApp->Endpoint ()->GetCallingState () == 2)) {

	PTimeInterval t =
	  PTime () - MyApp->Endpoint ()->GetCurrentConnection ()
	  ->GetConnectionStartTime();

	if (t.GetSeconds () > 2) {

	  minutes = t.GetMinutes () % 60;
	  seconds = t.GetSeconds () % 60;
	  
	  gchar *msg = g_strdup_printf 
	    (_("Connection Time: %.2ld:%.2ld:%.2ld"), 
	     t.GetHours (), minutes, seconds);
	  
	  gnome_appbar_push (GNOME_APPBAR (statusbar), msg);
	  
	  g_free (msg);
	}
      }
  }*/

      gchar *msg = g_strdup (_("sdfdsf"));

      GtkWidget *msg_box = gnome_message_box_new (msg, GNOME_MESSAGE_BOX_ERROR, 
						  "OK", NULL);
    //  gtk_widget_show (msg_box);

      g_free (msg);
  gdk_threads_leave ();
  return TRUE;
}


/* The main GnomeMeeting Class  */

GnomeMeeting::GnomeMeeting ()
  : PProcess("", "", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE,
	     BUILD_NUMBER)

{
  /* no endpoint for the moment */
  endpoint = NULL;

  gw = gnomemeeting_get_main_window (gm);
  lw = gnomemeeting_get_ldap_window (gm);

  MyApp = (this);

  endpoint = new GMH323EndPoint ();

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
  gtk_entry_set_text (GTK_ENTRY (GTK_WIDGET(GTK_COMBO(gw->combo)->entry)), 
		      ip);
  /* 20 = max number of contacts to store on HD, put here the value 
     got from preferences if anyi */
  gnomemeeting_add_contact_entry(gw, 20);
}
  

void GnomeMeeting::Connect()
{
  PString call_address;
  PString current_call_token;
  H323Connection *connection;
  GtkWidget *li;
  
  /* We need a connection to use AnsweringCall */
  current_call_token = endpoint->GetCurrentCallToken ();
  connection = endpoint->FindConnectionWithLock (current_call_token);
  
  call_address = (PString) gtk_entry_get_text 
    (GTK_ENTRY (GTK_WIDGET(GTK_COMBO(gw->combo)->entry)));

  /* If connection, then answer it */
  if (connection != NULL) {
    
      gnomemeeting_enable_disconnect ();
      gnomemeeting_disable_connect ();
      endpoint->SetCallingState (2);
      connection->AnsweringCall (H323Connection::AnswerCallNow);
      connection->Unlock ();
      
      gnomemeeting_log_insert (_("Answering incoming call"));
  }
  else {

    gtk_entry_set_text (GTK_ENTRY (GTK_WIDGET(GTK_COMBO(gw->combo)->entry)),
			call_address);
    /* 20 = max number of contacts to store on HD, put here the value */
    /* got from preferences if any*/
    gnomemeeting_add_contact_entry(gw, 20);
    
    /* if we call somebody */
    if (!call_address.IsEmpty ()) {

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
      gnomemeeting_log_insert (msg);
      gnome_appbar_push (GNOME_APPBAR (gw->statusbar), msg);
      g_free (msg);				 
      gnomemeeting_enable_disconnect ();
      gnomemeeting_disable_connect ();
    }			
  }
}


void GnomeMeeting::Disconnect()
{
  /* If somebody is calling us, then we do not accept the connection
     else we finish it */
  H323Connection *connection = endpoint->GetCurrentConnection ();
  PString current_call_token = endpoint->GetCurrentCallToken ();

  if (!current_call_token.IsEmpty ()) {

    /* if we are trying to call somebody */
    if (endpoint->GetCallingState () == 1) {

      gnomemeeting_log_insert (_("Trying to stop calling"));

      /* End of Call */
      endpoint->ClearAllCalls (H323Connection::EndedByLocalUser, FALSE);
    }
    else {
      
      /* if somebody is calling us, or if we are in call with somebody */
      
      if (endpoint->GetCallingState () == 2) {
	
	gnomemeeting_log_insert (_("Stopping current call"));

	/* End of Call */
	endpoint->ClearAllCalls (H323Connection::EndedByLocalUser, FALSE);
      }
      else {

	gnomemeeting_log_insert (_("Refusing Incoming call"));
	connection->AnsweringCall (H323Connection::AnswerCallDenied);	
      }
    }
  } 
}


GMH323EndPoint *GnomeMeeting::Endpoint ()
{
  return endpoint;
}


void GnomeMeeting::Main ()
{
  /* Nothing interesting here */
}


/* The main () */

int main (int argc, char ** argv, char ** envp)
{
  PProcess::PreInitialise (argc, argv, envp);

  /* The different structures needed by most of the classes and functions */
  GM_window_widgets *gw = NULL;
  GM_ldap_window_widgets *lw = NULL;
  GM_pref_window_widgets *pw = NULL;


  /* Init the GM_window_widgets */
  gw = new (GM_window_widgets);
  gw->pixmap = NULL;
  gw->pref_window = NULL;
  gw->ldap_window = NULL;
  gw->video_grabber_thread_count = 0;
  gw->cleaner_thread_count = 0;
  gw->zoom = 1;


  /* Init the GM_pref_window_widgets structure */
  pw = new (GM_pref_window_widgets); 
  pw->gw = gw;
  pw->ldap_changed = 0;
  pw->audio_mixer_changed = 0;
  pw->gk_changed = 0;
  pw->capabilities_changed = 0;
  pw->audio_codecs_changed = 0;
  pw->vid_tr_changed = 0;


  /* Init the GM_ldap_window_widgets structure */
  lw = new (GM_ldap_window_widgets);
  lw->ldap_servers_list = NULL;


  /* Threads + Locale Init + Gconf */
  g_thread_init(NULL);
  gconf_init (argc, argv, 0);

  textdomain (PACKAGE);
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);


  /* GnomeMeeting main initialisation */
  gnomemeeting_init (gw, pw, lw, argc, argv, envp);


  /* Quick hack to make the GUI refresh even on high load from the other
     threads */
 //  gtk_idle_add ((GtkFunction) gnome_idle_timer, gw);

  gtk_timeout_add (100, (GtkFunction) AppbarUpdate, 
 		   gw->statusbar);


  /* The GTK loop */
  gtk_main ();

  delete (gw);
  delete (lw);
  delete (pw);

  return 0;
}

/* Needed for RedHat Systems */
/* Plz check this */

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
