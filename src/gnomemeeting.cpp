 
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
#include "ils.h"
#include "urlhandler.h"

#include <esd.h>
#include <gconf/gconf-client.h>


/* Declarations */
GtkWidget *gm;
GnomeMeeting *MyApp;	


/* The main GnomeMeeting Class  */

GnomeMeeting::GnomeMeeting ()
  : PProcess("", "", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE,
	     BUILD_NUMBER)

{
  /* no endpoint for the moment */
  endpoint = NULL;

  url_handler = NULL;

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
  H323Connection *connection = NULL;
  
  /* We need a connection to use AnsweringCall */
  current_call_token = endpoint->GetCurrentCallToken ();
  connection = endpoint->GetCurrentConnection ();
  
  gnomemeeting_threads_enter ();
  gnome_appbar_clear_stack (GNOME_APPBAR (gw->statusbar));
  call_address = (PString) gtk_entry_get_text 
    (GTK_ENTRY (GTK_WIDGET(GTK_COMBO(gw->combo)->entry)));
  gnomemeeting_threads_leave ();

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
      
      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (_("Answering incoming call"));
      connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 1);
      gnomemeeting_threads_leave ();
  }
  else {

    gnomemeeting_threads_enter ();
    gtk_entry_set_text (GTK_ENTRY (GTK_WIDGET(GTK_COMBO(gw->combo)->entry)),
			call_address);
    /* 20 = max number of contacts to store on HD, put here the value */
    /* got from preferences if any*/
    gnomemeeting_history_combo_box_add_entry (GTK_COMBO (gw->combo), "/apps/gnomemeeting/history/called_hosts", gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry)));
    gnomemeeting_threads_leave ();


    /* if we call somebody */
    if (!call_address.IsEmpty ()) {
      
      H323Connection *con = NULL;
      call_number++;
      GMILSClient *ils_client = (GMILSClient *) endpoint->GetILSClient ();

      url_handler = new GMURLHandler (call_address);

      gnomemeeting_threads_enter ();
      gw->progress_timeout =
	gtk_timeout_add (20, gnomemeeting_window_appbar_update, 
			 gw->statusbar);
      gtk_widget_show (GTK_WIDGET (gnome_appbar_get_progress (GNOME_APPBAR(gw->statusbar))));
      gnomemeeting_threads_leave ();

#ifdef HAS_IXJ
      OpalLineInterfaceDevice *lid = NULL;
      lid = endpoint->GetLidDevice ();
      if (lid)
	lid->PlayTone (0, OpalLineInterfaceDevice::RingTone);
#endif

    }
    else  /* We untoggle the connect button in the case it was toggled */
      {

	gnomemeeting_threads_enter ();
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->connect_button), 
				      FALSE);
	gnomemeeting_threads_leave ();
      }
  }
}


void GnomeMeeting::Disconnect ()
{
  /* If somebody is calling us, then we do not accept the connection
     else we finish it */
  H323Connection *connection = endpoint->GetCurrentConnection ();
  PString current_call_token = endpoint->GetCurrentCallToken ();


  /* Update the button */
  gnomemeeting_threads_enter ();

  connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 0);

  if (gw->progress_timeout) {

    gtk_timeout_remove (gw->progress_timeout);
    gw->progress_timeout = 0;
    gtk_widget_hide (GTK_WIDGET (gnome_appbar_get_progress (GNOME_APPBAR (gw->statusbar))));
  }
  gnome_appbar_clear_stack (GNOME_APPBAR (gw->statusbar));

  gnomemeeting_threads_leave ();


  if (!current_call_token.IsEmpty ()) {

    /* if we are trying to call somebody */
    if (endpoint->GetCallingState () == 1) {

      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (_("Trying to stop calling"));
      gnomemeeting_threads_leave ();

      endpoint->ClearCall (current_call_token);
    }
    else {

      /* if somebody is calling us, or if we are in call with somebody */
      
      if (endpoint->GetCallingState () == 2) {

	gnomemeeting_threads_enter ();	
	gnomemeeting_log_insert (_("Stopping current call"));
	connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 
				      0);
	gnomemeeting_threads_leave ();

	/* End of Call */
	endpoint->ClearAllCalls (H323Connection::EndedByLocalUser, FALSE);
      }
      else {

	gnomemeeting_threads_enter ();
	gnomemeeting_log_insert (_("Refusing Incoming call"));
	connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 
				      0);
	gnomemeeting_threads_leave ();

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
