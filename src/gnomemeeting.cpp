/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
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
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains the main class
 *
 */


#include "../config.h"

#include "gnomemeeting.h"
#include "callbacks.h"
#include "sound_handling.h"
#include "ils.h"
#include "urlhandler.h"
#include "addressbook_window.h"
#include "menu.h"
#include "pref_window.h"
#include "chat_window.h"
#include "calls_history_window.h"
#include "druid.h"
#include "tools.h"
#include "tray.h"
#include "log_window.h"
#include "main_window.h"
#include "toolbar.h"
#include "misc.h"

#include "history-combo.h"
#include "dialog.h"
#include "e-splash.h"
#include "stock-icons.h"
#include "gm_conf.h"
#include <contacts/gm_contacts.h>

#ifndef WIN32
#include <signal.h>
#endif


static gint
gnomemeeting_tray_hack (gpointer);

GnomeMeeting *GnomeMeeting::GM = NULL;

GtkWidget *gm;


static gint
gnomemeeting_tray_hack (gpointer data)
{
  GmWindow *gw = NULL;

  gdk_threads_enter ();

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  if (!gnomemeeting_tray_is_embedded (gw->docklet)) {

    gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Notification area not detected"), _("You have chosen to start GnomeMeeting hidden, however the notification area is not present in your panel, GnomeMeeting can thus not start hidden."));
    gnomemeeting_window_show (gm);
  }
  
  gdk_threads_leave ();

  return FALSE;
}


/* The main GnomeMeeting Class  */
GnomeMeeting::GnomeMeeting ()
  : PProcess("", "", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)

{
  /* no endpoint for the moment */
  endpoint = NULL;
  url_handler = NULL;

  /* Init the different structures */
  gw = new GmWindow ();
  rtp = new GmRtpData ();

  memset ((void *) rtp, 0, sizeof (struct _GmRtpData));

  addressbook_window = NULL;
  
  gw->docklet =   
    gw->splash_win = gw->incoming_call_popup = 
    gw->transfer_call_popup = gw->audio_transmission_popup = 
    gw->audio_reception_popup = 
    NULL;

  GM = this;
  
  endpoint = new GMH323EndPoint ();
  
  call_number = 0;
}


GnomeMeeting::~GnomeMeeting()
{
  if (endpoint) {

    endpoint->RemoveVideoGrabber ();
#ifdef HAS_IXJ
    endpoint->RemoveLid ();
#endif
  }
  
  RemoveEndpoint ();

  if (addressbook_window) 
    gtk_widget_destroy (addressbook_window);  
  if (prefs_window)
    gtk_widget_destroy (prefs_window);
  if (history_window)
    gtk_widget_destroy (history_window);
  if (calls_history_window)
    gtk_widget_destroy (calls_history_window);
  if (gm)
    gtk_widget_destroy (gm);
  if (druid_window)
    gtk_widget_destroy (druid_window);
  
  delete (rtp);
}


void 
GnomeMeeting::Connect (PString url)
{
  /* If incoming connection, then answer it */
  if (endpoint->GetCallingState () == GMH323EndPoint::Called) {

    gnomemeeting_threads_enter ();
    gm_history_window_insert (history_window, _("Answering incoming call"));
    connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 1);
    gnomemeeting_threads_leave ();

    url_handler = new GMURLHandler ();
  }
  else {
    
    /* Update the GUI */
    gnomemeeting_threads_enter ();
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry), 
			url);

    if (!url.IsEmpty ())
      gm_history_combo_add_entry (GM_HISTORY_COMBO (gw->combo), 
				  USER_INTERFACE_KEY "main_window/urls_history",
				  url);
    gnomemeeting_threads_leave ();


    /* if we call somebody, and if the URL is not empty */
    if (!GMURL (url).IsEmpty ()) {
      call_number++;

      url_handler = new GMURLHandler (url);
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


void
GnomeMeeting::Disconnect (H323Connection::CallEndReason reason)
{
  gnomemeeting_threads_enter ();
  gnomemeeting_statusbar_push (gw->statusbar, NULL);
  gnomemeeting_threads_leave ();


  /* if we are trying to call somebody */
  if (endpoint->GetCallingState () == 1) {

    gnomemeeting_threads_enter ();
    gm_history_window_insert (history_window, _("Trying to stop calling"));
    gnomemeeting_threads_leave ();

    endpoint->ClearCall (endpoint->GetCurrentCallToken (), reason);
  }
  else {

    /* if we are in call with somebody */
    if (endpoint->GetCallingState () == 2) {

      gnomemeeting_threads_enter ();	
      gm_history_window_insert (history_window, _("Stopping current call"));
      connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 
				    0);
      gnomemeeting_threads_leave ();

      endpoint->ClearAllCalls (reason, FALSE);
    }
    else if (endpoint->GetCallingState () == 3) {

      gnomemeeting_threads_enter ();
      gm_history_window_insert (history_window, _("Refusing Incoming call"));
      connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 
				    0);
      gnomemeeting_threads_leave ();

      endpoint->ClearCall (endpoint->GetCurrentCallToken (),
			   H323Connection::EndedByLocalUser);
    }
  }
}


void
GnomeMeeting::Init ()
{
#ifndef WIN32
  /* Ignore SIGPIPE */
  signal (SIGPIPE, SIG_IGN);
#endif

  endpoint->Init ();
}


BOOL
GnomeMeeting::DetectDevices ()
{
  gchar *audio_plugin = NULL;
  gchar *video_plugin = NULL;

  PINDEX fake_idx;

  audio_plugin = gm_conf_get_string (AUDIO_DEVICES_KEY "plugin");
  video_plugin = gm_conf_get_string (VIDEO_DEVICES_KEY "plugin");
 
  PWaitAndSignal m(dev_access_mutex);
  

  /* Detect the devices */
  gnomemeeting_sound_daemons_suspend ();

  
  /* Detect the plugins */
  audio_managers = PSoundChannel::GetDriverNames ();
  video_managers = PVideoInputDevice::GetDriverNames ();

  
  /* No audio plugin => Exit */
  if (audio_managers.GetSize () == 0)
    return FALSE;
  
  
  /* Detect the devices */
  video_input_devices = PVideoInputDevice::GetDriversDeviceNames (video_plugin);
 
  if (PString ("Quicknet") == audio_plugin) {

    audio_input_devices = OpalIxJDevice::GetDeviceNames ();
    audio_output_devices = audio_input_devices;
  }
  else {
    
    audio_input_devices = 
      PSoundChannel::GetDeviceNames (audio_plugin, PSoundChannel::Recorder);
    audio_output_devices = 
      PSoundChannel::GetDeviceNames (audio_plugin, PSoundChannel::Player);
  }
    
  
  if (audio_input_devices.GetSize () == 0) 
    audio_input_devices += PString (_("No device found"));
  if (audio_output_devices.GetSize () == 0)
    audio_output_devices += PString (_("No device found"));
  if (video_input_devices.GetSize () == 0)
    video_input_devices += PString (_("No device found"));

  
  g_free (audio_plugin);
  g_free (video_plugin);


  fake_idx = video_managers.GetValuesIndex (PString ("FakeVideo"));
  if (fake_idx != P_MAX_INDEX)
    video_managers.RemoveAt (fake_idx);
  
  audio_managers += PString ("Quicknet");
  
  gnomemeeting_sound_daemons_resume ();

  return TRUE;
}


GMH323EndPoint *
GnomeMeeting::Endpoint ()
{
  GMH323EndPoint *ep = NULL;
  PWaitAndSignal m(ep_var_mutex);

  ep = endpoint;
  
  return ep;
}


GnomeMeeting *
GnomeMeeting::Process ()
{
  return GM;
}


GmWindow *
GnomeMeeting::GetMainWindow ()
{
  return gw;
}


GtkWidget *
GnomeMeeting::GetPrefsWindow ()
{
  return prefs_window;
}


GtkWidget *
GnomeMeeting::GetDruidWindow ()
{
  return druid_window;
}


GtkWidget *
GnomeMeeting::GetCallsHistoryWindow ()
{
  return calls_history_window;
}


GtkWidget *
GnomeMeeting::GetAddressbookWindow ()
{
  return addressbook_window;
}


GmRtpData *
GnomeMeeting::GetRtpData ()
{
  return rtp;
}


GtkWidget *
GnomeMeeting::GetHistoryWindow ()
{
  return history_window;
}


void GnomeMeeting::Main ()
{
}


void GnomeMeeting::BuildGUI ()
{
  bool show_splash = TRUE;
  OpalMediaFormat::List available_capabilities;


  /* Get the available capabilities list */
  available_capabilities = endpoint->GetAvailableAudioCapabilities ();
  
  
  /* Init the splash screen */
  gw->splash_win = e_splash_new ();
  g_signal_connect (G_OBJECT (gw->splash_win), "delete_event",
		    G_CALLBACK (gtk_widget_hide_on_delete), 0);

  show_splash = gm_conf_get_bool (USER_INTERFACE_KEY "show_splash_screen");
  if (show_splash) 
  {
    /* We show the splash screen */
    gtk_widget_show_all (gw->splash_win);

    while (gtk_events_pending ())
      gtk_main_iteration ();
  }

  
  /* Init the address book */
  gnomemeeting_addressbook_init (_("On This Computer"), _("Personal"));
  
  
  /* Init the stock icons */
  gnomemeeting_stock_icons_init ();

  
  /* Build the GUI */
  gw->chat_window = gnomemeeting_text_chat_new ();
  gw->tips = gtk_tooltips_new ();
  gw->pc_to_phone_window = gnomemeeting_pc_to_phone_window_new ();  
  prefs_window = gm_prefs_window_new ();  
  gm_prefs_window_update_audio_codecs_list (prefs_window, 
					    available_capabilities);
  
  calls_history_window = gnomemeeting_calls_history_window_new ();
  history_window = gm_history_window_new ();
  addressbook_window = gm_addressbook_window_new ();
  druid_window = gm_druid_window_new ();
#ifndef WIN32
  gw->docklet = gnomemeeting_tray_new ();
  gw->tray_popup_menu = gnomemeeting_tray_init_menu (gw->docklet);
#endif
  gm_main_window_new (gw);


#ifndef DISABLE_GNOME
 if (gm_conf_get_int (GENERAL_KEY "version") 
      < 1000 * MAJOR_VERSION + 10 * MINOR_VERSION + BUILD_NUMBER) {

   gtk_widget_show_all (GTK_WIDGET (druid_window));
  }
  else {
#endif
    /* Show the main window */
#ifndef WIN32
    if (!gm_conf_get_bool (USER_INTERFACE_KEY "start_hidden")) 
#endif
      gnomemeeting_window_show (gm);
#ifndef WIN32
    else
      gtk_timeout_add (15000, (GtkFunction) gnomemeeting_tray_hack, NULL);
#endif
#ifndef DISABLE_GNOME
  }
#endif

 
  /* Destroy the splash */
 if (gw->splash_win) {
   
   gtk_widget_destroy (gw->splash_win);
   gw->splash_win = NULL;
 }

  
  /* GM is started */
 gm_history_window_insert (history_window,
			   _("Started GnomeMeeting V%d.%d.%d for %s\n"), 
			   MAJOR_VERSION, MINOR_VERSION, BUILD_NUMBER,
			   g_get_user_name ());
}


void GnomeMeeting::RemoveEndpoint ()
{
  PWaitAndSignal m(ep_var_mutex);

  if (endpoint)
    delete (endpoint);
  
  endpoint = NULL;
}


PStringArray 
GnomeMeeting::GetVideoInputDevices ()
{
  PWaitAndSignal m(dev_access_mutex);

  return video_input_devices;
}


PStringArray 
GnomeMeeting::GetAudioInputDevices ()
{
  PWaitAndSignal m(dev_access_mutex);

  return audio_input_devices;
}



PStringArray 
GnomeMeeting::GetAudioOutpoutDevices ()
{
  PWaitAndSignal m(dev_access_mutex);

  return audio_output_devices;
}


PStringArray 
GnomeMeeting::GetAudioPlugins ()
{
  PWaitAndSignal m(dev_access_mutex);

  return audio_managers;
}


PStringArray 
GnomeMeeting::GetVideoPlugins ()
{
  PWaitAndSignal m(dev_access_mutex);

  return video_managers;
}
