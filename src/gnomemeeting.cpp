 
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

#include <esd.h>
#include <gconf/gconf-client.h>


/* Declarations */

GtkWidget *gm;
GnomeMeeting *MyApp;	

int i = 0;

/* GTK Callbacks */
gint StressTest (gpointer data)
{
  gdk_threads_enter ();


  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  if (!GTK_TOGGLE_BUTTON (gw->connect_button)->active) {

    i++;
    cout << "Call " << i << endl << flush;
  }

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->connect_button), 
				!GTK_TOGGLE_BUTTON (gw->connect_button)->active);

  gdk_threads_leave ();
  return TRUE;
}

gint AppbarUpdate (gpointer data)
{
  long minutes, seconds;
  float tr_audio_speed = 0, tr_video_speed = 0;
  float re_audio_speed = 0, re_video_speed = 0;
  RTP_Session *audio_session = NULL;
  RTP_Session *video_session = NULL;
  H323Connection *connection = NULL;
  GM_rtp_data *rtp = (GM_rtp_data *) data; 
  GM_window_widgets *gw = NULL;


  if (MyApp->Endpoint ()) {

    PString current_call_token = MyApp->Endpoint ()->GetCurrentCallToken ();

    if (current_call_token.IsEmpty ())
      return TRUE;
    
    gdk_threads_enter ();

    gw = gnomemeeting_get_main_window (gm);

    connection = MyApp->Endpoint ()->GetCurrentConnection ();

    if ((connection)&&(MyApp->Endpoint ()->GetCallingState () == 2)) {

      PTimeInterval t =
	PTime () - connection->GetConnectionStartTime();

      if (t.GetSeconds () > 1) {

	audio_session = 
	  connection->GetSession(RTP_Session::DefaultAudioSessionID);
	  
	video_session = 
	  connection->GetSession(RTP_Session::DefaultVideoSessionID);
	  
	if (audio_session != NULL) {

	  tr_audio_speed = 
	    (float) (audio_session->GetOctetsSent()-rtp->tr_audio_bytes)/1024.00;
	  rtp->tr_audio_bytes = audio_session->GetOctetsSent();

	  re_audio_speed = 
	    (float) (audio_session->GetOctetsReceived()-rtp->re_audio_bytes)/1024.00;
	  rtp->re_audio_bytes = audio_session->GetOctetsReceived();
	}

	if (video_session != NULL) {

	  tr_video_speed = 
	    (float) (video_session->GetOctetsSent()-rtp->tr_video_bytes)/1024.00;
	  rtp->tr_video_bytes = video_session->GetOctetsSent();

	  re_video_speed = 
	    (float) (video_session->GetOctetsReceived()-rtp->re_video_bytes)/1024.00;
	  rtp->re_video_bytes = video_session->GetOctetsReceived();
	}

	minutes = t.GetMinutes () % 60;
	seconds = t.GetSeconds () % 60;
	
	gchar *msg = g_strdup_printf 
	  (_("%.2ld:%.2ld:%.2ld  A:%.2f/%.2f   V:%.2f/%.2f"), 
	   (long) t.GetHours (), (long) minutes, (long) seconds, 
	   tr_audio_speed, re_audio_speed,
	   tr_video_speed, re_video_speed);
	
	if (t.GetSeconds () > 3) {

	  gnome_appbar_clear_stack (GNOME_APPBAR (gw->statusbar));
	  gnome_appbar_push (GNOME_APPBAR (gw->statusbar), msg);
	}

	g_free (msg);
      }
    }

    gdk_threads_leave ();
  }

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

void GnomeMeeting::Connect()
{
  PString call_address;
  PString current_call_token;
  H323Connection *connection;
  
  /* We need a connection to use AnsweringCall */
  current_call_token = endpoint->GetCurrentCallToken ();
  connection = endpoint->GetCurrentConnection ();
  
  call_address = (PString) gtk_entry_get_text 
    (GTK_ENTRY (GTK_WIDGET(GTK_COMBO(gw->combo)->entry)));

  /* If connection, then answer it */
  if (connection != NULL) {
    
#ifdef HAS_IXJ
      OpalLineInterfaceDevice *lid = NULL;
      lid = endpoint->GetLidDevice ();
      if (lid)
	lid->StopTone (0);
#endif

      endpoint->SetCallingState (2);
      connection->AnsweringCall (H323Connection::AnswerCallNow);
      
      gnomemeeting_log_insert (_("Answering incoming call"));
      connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 1);
  }
  else {

    gtk_entry_set_text (GTK_ENTRY (GTK_WIDGET(GTK_COMBO(gw->combo)->entry)),
			call_address);
    /* 20 = max number of contacts to store on HD, put here the value */
    /* got from preferences if any*/
    gnomemeeting_history_combo_box_add_entry (GTK_COMBO (gw->combo),
	 				      "/apps/gnomemeeting/history/called_hosts",
		 			      gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry)));
    
    /* if we call somebody */
    if (!call_address.IsEmpty ()) {

      call_number++;
      gchar *msg = NULL;
      H323Connection *con = NULL;

      con = endpoint->MakeCallLocked (call_address, current_call_token);
      endpoint->SetCurrentConnection (con);
      endpoint->SetCurrentCallToken (current_call_token);
      endpoint->SetCallingState (1);

#ifdef HAS_IXJ
      OpalLineInterfaceDevice *lid = NULL;
      lid = endpoint->GetLidDevice ();
      if (lid)
	lid->PlayTone (0, OpalLineInterfaceDevice::RingTone);
#endif

      con->Unlock ();
      gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);

      /* Enable disconnect: we must be able to stop calling */
      GnomeUIInfo *call_menu_uiinfo =
	(GnomeUIInfo *) g_object_get_data (G_OBJECT (gm), "call_menu_uiinfo");
      gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [1].widget), 
				TRUE);

      msg = g_strdup_printf (_("Call %d: calling %s"), 
			     call_number,
			     (const char *) call_address);
      gnomemeeting_log_insert (msg);
      gnome_appbar_push (GNOME_APPBAR (gw->statusbar), msg);
      connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 1);
      g_free (msg);				 
    }
    else  /* We untoggle the connect button in the case it was toggled */
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->connect_button), 
				    FALSE);
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
      connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 0);

      /* End of Call */
      endpoint->ClearCall (current_call_token);
    }
    else {
      
      /* if somebody is calling us, or if we are in call with somebody */
      
      if (endpoint->GetCallingState () == 2) {
	
	gnomemeeting_log_insert (_("Stopping current call"));
	connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 
				      0);
	/* End of Call */
	endpoint->ClearAllCalls (H323Connection::EndedByLocalUser, FALSE);
      }
      else {

	gnomemeeting_log_insert (_("Refusing Incoming call"));
	connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 
				      0);
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
  GmTextChat *chat = NULL;
  GM_rtp_data *rtp = NULL;


  /* Init the GM_window_widgets */
  gw = new (GM_window_widgets);
  gw->pref_window = NULL;
  gw->ldap_window = NULL;
  gw->incoming_call_popup = NULL;
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


  /* Init the RTP stats structure */
  rtp = new (GM_rtp_data);
  rtp->re_audio_bytes = 0;
  rtp->re_video_bytes = 0;
  rtp->tr_video_bytes = 0;
  rtp->tr_audio_bytes = 0;


  /* Init the TextChat structure */
  chat = new (GmTextChat);


  /* Threads + Locale Init + Gconf */
  g_thread_init (NULL);
  gdk_threads_init ();

  gdk_threads_enter ();
  gconf_init (argc, argv, 0);

  textdomain (GETTEXT_PACKAGE);
  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");

  /* GnomeMeeting main initialisation */
  gnomemeeting_init (gw, pw, lw, rtp, chat, argc, argv, envp);
  /* Set a default gconf error handler */
  gconf_client_set_error_handling (gconf_client_get_default (),
				   GCONF_CLIENT_HANDLE_UNRETURNED);
  gconf_client_set_global_default_error_handler (gconf_error_callback);


  /* Quick hack to make the GUI refresh even on high load from the other
     threads */
  gtk_timeout_add (1000, (GtkFunction) AppbarUpdate, 
  		   rtp);
//   gtk_timeout_add (10000, (GtkFunction) StressTest, 
//  		   NULL);
  
  /* The GTK loop */
  gtk_main ();
  gdk_threads_leave ();

  delete (gw);
  delete (lw);
  delete (pw);
  delete (rtp);
  delete (chat); 

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
