
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2003 Damien Sandras
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
 *                         gnomemeeting.cpp  -  description
 *                         --------------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains the main class
 *
 */


#include "../config.h"

#include "gnomemeeting.h"
#include "sound_handling.h"
#include "ils.h"
#include "urlhandler.h"
#include "main_window.h"
#include "toolbar.h"
#include "misc.h"
#include "history-combo.h"
#include "dialog.h"

#ifndef WIN32
#include <esd.h>
#include <signal.h>
#endif


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
  video_grabber = NULL;

  client = gconf_client_get_default ();
  gw = gnomemeeting_get_main_window (gm);
  lw = gnomemeeting_get_ldap_window (gm);

  endpoint = new GMH323EndPoint ();
  MyApp = (this);

  vg = new PIntCondMutex (0, 0);

  Init ();
  call_number = 0;
}


GnomeMeeting::~GnomeMeeting()
{
  endpoint->ClearAllCalls (H323Connection::EndedByLocalUser, TRUE);
  RemoveVideoGrabber (true);
  RemoveEndpoint ();
}


void 
GnomeMeeting::Connect()
{
  PString call_address;
  PString current_call_token;
  H323Connection *connection = NULL;
    
  /* We need a connection to use AnsweringCall */
  current_call_token = endpoint->GetCurrentCallToken ();
  connection = endpoint->GetCurrentConnection ();
  
  gnomemeeting_threads_enter ();
  gnomemeeting_statusbar_push  (gw->statusbar, NULL);
  call_address = (PString) gtk_entry_get_text 
          (GTK_ENTRY (GTK_COMBO (gw->combo)->entry));  
  gnomemeeting_threads_leave ();


  /* If connection, then answer it */
  if (connection != NULL) {

    gnomemeeting_threads_enter ();
    gnomemeeting_log_insert (gw->history_text_view,
			     _("Answering incoming call"));
    connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 1);
    gnomemeeting_threads_leave ();

    url_handler = new GMURLHandler ();
  }
  else 
  {
    gnomemeeting_threads_enter ();
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry), 
			call_address);
    
    if (!call_address.IsEmpty ())
      gm_history_combo_add_entry (GM_HISTORY_COMBO (gw->combo), 
				  "/apps/gnomemeeting/history/called_urls_list", 
				  gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry)));
    gnomemeeting_threads_leave ();


    /* if we call somebody, and if the URL is not empty */
    if (!GMURL (call_address).IsEmpty ())
    {
      call_number++;

      url_handler = new GMURLHandler (call_address);

      gnomemeeting_threads_enter ();
      gw->progress_timeout =
	gtk_timeout_add (5, gnomemeeting_window_appbar_update, 
			 gw->progressbar);
      gtk_widget_show (gw->progressbar);
      gnomemeeting_threads_leave ();

#ifdef HAS_IXJ
      OpalLineInterfaceDevice *lid = NULL;
      GMLid *lid_thread = NULL;
      lid_thread = endpoint->GetLidThread ();

      lid_thread = endpoint->GetLidThread ();
      if (lid_thread)
	lid = lid_thread->GetLidDevice ();
      if (lid) {

	lid->PlayTone (0, OpalLineInterfaceDevice::RingTone);
	lid->RingLine (0, 0);
      }
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


void GnomeMeeting::Disconnect (H323Connection::CallEndReason reason)
{
  /* If somebody is calling us, then we do not accept the connection
     else we finish it */
  H323Connection *connection = endpoint->GetCurrentConnection ();
  PString current_call_token = endpoint->GetCurrentCallToken ();


  gnomemeeting_threads_enter ();

  if (gw->progress_timeout) {

    gtk_timeout_remove (gw->progress_timeout);
    gw->progress_timeout = 0;
    gtk_widget_hide (gw->progressbar);
  }
  gnomemeeting_statusbar_push (gw->statusbar, NULL);

  gnomemeeting_threads_leave ();


  if (!current_call_token.IsEmpty ()) {

    /* if we are trying to call somebody */
    if (endpoint->GetCallingState () == 1) {

      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (gw->history_text_view,
			       _("Trying to stop calling"));
      gnomemeeting_threads_leave ();

      endpoint->ClearCall (current_call_token, reason);
    }
    else {

      /* if somebody is calling us, or if we are in call with somebody */
      
      if (endpoint->GetCallingState () == 2) {

	gnomemeeting_threads_enter ();	
	gnomemeeting_log_insert (gw->history_text_view,
				 _("Stopping current call"));
	connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 
				      0);
	gnomemeeting_threads_leave ();

	/* End of Call */
	endpoint->ClearAllCalls (reason, FALSE);
      }
      else {

	gnomemeeting_threads_enter ();
	gnomemeeting_log_insert (gw->history_text_view,
				 _("Refusing Incoming call"));
	connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 
				      0);
	gnomemeeting_threads_leave ();

	/* Either the user clicks on disconnect when we are called,
	   either the reason is different */
	if (reason == H323Connection::EndedByLocalUser)
	  connection->AnsweringCall (H323Connection::AnswerCallDenied);
	else
	  endpoint->ClearAllCalls (reason, FALSE);
      }
    }
  } 
}


GMH323EndPoint *GnomeMeeting::Endpoint ()
{
  GMH323EndPoint *ep = NULL;
  PWaitAndSignal m(ep_var_mutex);

  ep = endpoint;
  
  return ep;
}


void GnomeMeeting::Main ()
{
}


void GnomeMeeting::Init ()
{
#ifndef WIN32
  /* Ignore SIGPIPE */
  signal (SIGPIPE, SIG_IGN);
#endif
  
  //  if (clo->debug_level != 0)
  // PTrace::Initialise (clo->debug_level);


  /* Start the video preview */
  if (gconf_client_get_bool (client, DEVICES_KEY "video_preview", NULL))
    MyApp->CreateVideoGrabber ();

  endpoint->SetUserNameAndAlias ();

  /* Register to gatekeeper */
  if (gconf_client_get_int (client, GATEKEEPER_KEY "registering_method", 0))
    endpoint->GatekeeperRegister ();

  /* The LDAP part, if needed */
  if (gconf_client_get_bool (GCONF_CLIENT (client), LDAP_KEY "register", NULL)) 
  {
      GMILSClient *gm_ils_client = 
	GM_ILS_CLIENT (endpoint->GetILSClientThread ());
      gm_ils_client->Register ();
  }
  
  
  if (!endpoint->StartListener ()) 
    gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Error while starting the listener"), _("You will not be able to receive incoming calls. Please check that no other program is already running on the port used by GnomeMeeting."));

  gnomemeeting_sound_daemons_suspend ();
  /* Detect the devices */
  gw->audio_player_devices = gnomemeeting_get_audio_player_devices ();
  gw->audio_recorder_devices = gnomemeeting_get_audio_recorder_devices ();
  gw->video_devices = PVideoInputDevice::GetInputDeviceNames ();
#ifdef TRY_1394DC
  gw->video_devices += PVideoInput1394DcDevice::GetInputDeviceNames();
#endif
#ifdef TRY_1394AVC
  gw->video_devices += PVideoInput1394AvcDevice::GetInputDeviceNames();
#endif

  gw->audio_mixers = gnomemeeting_get_mixers ();
  gnomemeeting_sound_daemons_resume ();
  gnomemeeting_mixers_mic_select ();
}


void GnomeMeeting::RemoveEndpoint ()
{
  PWaitAndSignal m(ep_var_mutex);

  if (endpoint)
    delete (endpoint);
  
  endpoint = NULL;
}


void GnomeMeeting::CreateVideoGrabber (BOOL start_grabbing, BOOL synchronous)
{
  PWaitAndSignal m(vg_var_mutex);
  
  if (!video_grabber)
    video_grabber =
      new GMVideoGrabber (vg, start_grabbing, synchronous);
}


void GnomeMeeting::RemoveVideoGrabber (BOOL synchronous)
{
  PWaitAndSignal m(vg_var_mutex);

  if (video_grabber) {

    video_grabber->Close ();
  }      
  video_grabber = NULL;

  if (synchronous)
    vg->WaitCondition ();
}


GMVideoGrabber *GnomeMeeting::GetVideoGrabber ()
{
  GMVideoGrabber *vg = NULL;
  PWaitAndSignal m(vg_var_mutex);

  vg = video_grabber;
  
  return vg;
}
