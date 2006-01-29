/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the main class
 *
 */


#include "../../config.h"

#include "ekiga.h"
#include "callbacks.h"
#include "audio.h"
#if 0
#include "ils.h"
#endif 
#include "urlhandler.h"
#include "addressbook.h"
#include "preferences.h"
#include "chat.h"
#include "callshistory.h"
#include "druid.h"
#include "tools.h"
#include "statusicon.h"
#include "history.h"
#include "main.h"
#include "misc.h"

#ifdef HAS_DBUS
#include "dbus.h"
#endif

#include "gmdialog.h"
#include "gmstockicons.h"
#include "gmconf.h"
#include "gmcontacts.h"

#ifndef WIN32
#include <signal.h>
#endif

#define new PNEW


GnomeMeeting *GnomeMeeting::GM = NULL;

/* The main GnomeMeeting Class  */
GnomeMeeting::GnomeMeeting ()
  : PProcess("", "", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)

{
  /* no endpoint for the moment */
  endpoint = NULL;
  url_handler = NULL;

  
  addressbook_window = NULL;

  GM = this;
  
  endpoint = new GMManager ();
  
  call_number = 0;
}


GnomeMeeting::~GnomeMeeting()
{ 
  Exit ();

  if (addressbook_window) 
    gtk_widget_destroy (addressbook_window);  
  if (prefs_window)
    gtk_widget_destroy (prefs_window);
  if (history_window)
    gtk_widget_destroy (history_window);
  if (calls_history_window)
    gtk_widget_destroy (calls_history_window);
  if (main_window)
    gtk_widget_destroy (main_window);
  if (druid_window)
    gtk_widget_destroy (druid_window);
  if (statusicon)
    gtk_widget_destroy (statusicon);
#ifdef HAS_DBUS
    g_object_unref (dbus_component);
#endif
}


void 
GnomeMeeting::Connect (PString url)
{
  /* If incoming connection, then answer it */
  if (endpoint->GetCallingState () == GMManager::Called) {

    gm_history_window_insert (history_window, _("Answering incoming call"));

    url_handler = new GMURLHandler ("", FALSE);
  }
  else if (endpoint->GetCallingState () == GMManager::Standby
	   && !GMURL (url).IsEmpty ()) {
    
    /* Update the GUI */
    gm_main_window_set_call_url (main_window, url);
 
    /* if we call somebody, and if the URL is not empty */
    url_handler = new GMURLHandler (url);
  }
}


void
GnomeMeeting::Disconnect (H323Connection::CallEndReason reason)
{
  PString call_token;

  call_token = endpoint->GetCurrentCallToken ();
  
  /* if we are trying to call somebody */
  if (endpoint->GetCallingState () == GMManager::Calling) {

    endpoint->ClearCall (call_token, reason);
  }
  else {

    /* if we are in call with somebody */
    if (endpoint->GetCallingState () == GMManager::Connected) {

      endpoint->ClearAllCalls (OpalConnection::EndedByLocalUser, FALSE);
    }
    else if (endpoint->GetCallingState () == GMManager::Called) {

      endpoint->ClearCall (call_token,
			   H323Connection::EndedByAnswerDenied);
    }
    else {
      endpoint->ClearCall (call_token,
			   H323Connection::EndedByAnswerDenied);
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

  /* Init the endpoint */
  endpoint->Init ();
}


void
GnomeMeeting::Exit ()
{
  PWaitAndSignal m(ep_var_mutex);

  RemoveManager ();
}


BOOL
GnomeMeeting::DetectInterfaces ()
{
  PIPSocket::InterfaceTable ifaces;

  PINDEX i = 0;
  BOOL res = FALSE;
  
  PWaitAndSignal m(iface_access_mutex);
  
  /* Detect the valid interfaces */
  res = PIPSocket::GetInterfaceTable (ifaces);

  while (i < ifaces.GetSize ()) {
    
    if (ifaces [i].GetName ().Find ("ppp") != P_MAX_INDEX) {
      
      if (i > 0) {
	interfaces += interfaces [0];
	interfaces [0] = ifaces [i].GetName ();
      }
      else
	interfaces += ifaces [i].GetName ();
    }
    else if (ifaces [i].GetName () != "lo"
	&& ifaces [i].GetName () != "MS TCP Loopback interface")
      interfaces += ifaces [i].GetName ();
    
    i++;
  }
  
  
  /* Update the GUI, if it is already there */
  if (prefs_window)
    gm_prefs_window_update_interfaces_list (prefs_window, 
					    interfaces);
  
  return res;
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
  
  fake_idx = video_managers.GetValuesIndex (PString ("FakeVideo"));
  if (fake_idx != P_MAX_INDEX)
    video_managers.RemoveAt (fake_idx);

  PTRACE (1, "Detected audio plugins: " << setfill (',') << audio_managers
	  << setfill (' '));
  PTRACE (1, "Detected video plugins: " << setfill (',') << video_managers
	  << setfill (' '));

#ifdef HAX_IXJ
  audio_managers += PString ("Quicknet");
#endif

  PTRACE (1, "Detected audio plugins: " << setfill (',') << audio_managers
	  << setfill (' '));
  PTRACE (1, "Detected video plugins: " << setfill (',') << video_managers
	  << setfill (' '));
  

  fake_idx = video_managers.GetValuesIndex (PString ("Picture"));
  if (fake_idx == P_MAX_INDEX)
    return FALSE;

  /* No audio plugin => Exit */
  if (audio_managers.GetSize () == 0)
    return FALSE;
  
  
  /* Detect the devices */
  video_input_devices = PVideoInputDevice::GetDriversDeviceNames (video_plugin);
 
  audio_input_devices = 
    PSoundChannel::GetDeviceNames (audio_plugin, PSoundChannel::Recorder);
  audio_output_devices = 
    PSoundChannel::GetDeviceNames (audio_plugin, PSoundChannel::Player);

  
  if (audio_input_devices.GetSize () == 0) 
    audio_input_devices += PString (_("No device found"));
  if (audio_output_devices.GetSize () == 0)
    audio_output_devices += PString (_("No device found"));
  if (video_input_devices.GetSize () == 0)
    video_input_devices += PString (_("No device found"));


  PTRACE (1, "Detected the following audio input devices: "
	  << setfill (',') << audio_input_devices << setfill (' ')
	  << " with plugin " << audio_plugin);
  PTRACE (1, "Detected the following audio output devices: "
	  << setfill (',') << audio_output_devices << setfill (' ')
	  << " with plugin " << audio_plugin);
  PTRACE (1, "Detected the following video input devices: "
	  << setfill (',') << video_input_devices << setfill (' ')
	  << " with plugin " << video_plugin);
  
  PTRACE (1, "Detected the following audio input devices: " 
	  << setfill (',') << audio_input_devices << setfill (' ') 
	  << " with plugin " << audio_plugin);
  PTRACE (1, "Detected the following audio output devices: " 
	  << setfill (',') << audio_output_devices << setfill (' ') 
	  << " with plugin " << audio_plugin);
  PTRACE (1, "Detected the following video input devices: " 
	  << setfill (',') << video_input_devices << setfill (' ')  
	  << " with plugin " << video_plugin);

  g_free (audio_plugin);
  g_free (video_plugin);

  gnomemeeting_sound_daemons_resume ();

  /* Update the GUI, if it is already there */
  if (prefs_window)
    gm_prefs_window_update_devices_list (prefs_window, 
					 audio_input_devices,
					 audio_output_devices,
					 video_input_devices);
  return TRUE;
}


GMManager *
GnomeMeeting::GetManager ()
{
  GMManager *ep = NULL;
  PWaitAndSignal m(ep_var_mutex);

  ep = endpoint;
  
  return ep;
}


GnomeMeeting *
GnomeMeeting::Process ()
{
  return GM;
}


GtkWidget *
GnomeMeeting::GetMainWindow ()
{
  return main_window;
}


GtkWidget *
GnomeMeeting::GetPrefsWindow ()
{
  return prefs_window;
}


GtkWidget *
GnomeMeeting::GetChatWindow ()
{
  return chat_window;
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


GtkWidget *
GnomeMeeting::GetHistoryWindow ()
{
  return history_window;
}


GtkWidget *
GnomeMeeting::GetPC2PhoneWindow ()
{
  return pc2phone_window;
}


GtkWidget *
GnomeMeeting::GetAccountsWindow ()
{
  return accounts_window;
}


GtkWidget *
GnomeMeeting::GetStatusicon ()
{
  return statusicon;
}

#ifdef HAS_DBUS
GObject *
GnomeMeeting::GetDbusComponent ()
{
  return dbus_component;
}
#endif

void GnomeMeeting::Main ()
{
}


void GnomeMeeting::BuildGUI ()
{
  IncomingCallMode icm = AVAILABLE;
  BOOL forward_on_busy = FALSE;

  /* Init the address book */
  gnomemeeting_addressbook_init (_("On This Computer"), _("Personal"));
  
  /* Init the stock icons */
  gnomemeeting_stock_icons_init ();
  
  /* Build the GUI */
  pc2phone_window = gm_pc2phone_window_new ();  
  prefs_window = gm_prefs_window_new ();  
  calls_history_window = gm_calls_history_window_new ();
  history_window = gm_history_window_new ();
  addressbook_window = gm_addressbook_window_new ();
  chat_window = gm_text_chat_window_new ();
  druid_window = gm_druid_window_new ();
  accounts_window = gm_accounts_window_new ();
  main_window = gm_main_window_new ();
#ifdef HAS_DBUS
  dbus_component = gnomemeeting_dbus_component_new ();
#endif
  statusicon = gm_statusicon_new (); /* must come last (uses the windows) */

  /* we must put the statusicon in the right state */
  icm = (IncomingCallMode)
    gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode"); 
  forward_on_busy = gm_conf_get_bool (CALL_FORWARDING_KEY "forward_on_busy");
  gm_statusicon_update_full (statusicon,
			     GMManager::Standby, icm, forward_on_busy);
 
  /* GM is started */
  gm_history_window_insert (history_window,
			    _("Started Ekiga %d.%d.%d for user %s"), 
			    MAJOR_VERSION, MINOR_VERSION, BUILD_NUMBER,
			    g_get_user_name ());

  PTRACE (1, "GnomeMeeting version "
	  << MAJOR_VERSION << "." << MINOR_VERSION << "." << BUILD_NUMBER);
  PTRACE (1, "OPAL version " << "unknown");
  PTRACE (1, "PWLIB version " << PWLIB_VERSION);
#ifndef DISABLE_GNOME
  PTRACE (1, "GNOME support enabled");
#else
  PTRACE (1, "GNOME support disabled");
#endif
#ifdef HAS_SDL
  PTRACE (1, "Fullscreen support enabled");
#else
  PTRACE (1, "Fullscreen support disabled");
#endif
#ifdef HAS_DBUS
  PTRACE (1, "DBUS support enabled");
#else
  PTRACE (1, "DBUS support disabled");
#endif
}


void GnomeMeeting::RemoveManager ()
{
  PWaitAndSignal m(ep_var_mutex);

  if (endpoint) {

    endpoint->Exit ();
    delete (endpoint);
  }
  
  endpoint = NULL;
}


PStringArray 
GnomeMeeting::GetInterfaces ()
{
  PWaitAndSignal m(iface_access_mutex);

  return interfaces;
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
