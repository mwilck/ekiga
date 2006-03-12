
/* Ekiga -- A VoIP and Video-Conferencing application
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         gnomemeeting.h  -  description
 *                         ------------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the main class
 *
 */


#ifndef _GNOMEMEETING_H_
#define _GNOMEMEETING_H_

#include "common.h"
#include "manager.h"

#include <ptlib/ipsock.h>


/**
 * COMMON NOTICE: The Application must be initialized with Init after its
 * creation.
 */

class GnomeMeeting : public PProcess
{
  PCLASSINFO(GnomeMeeting, PProcess);

 public:


  /* DESCRIPTION  :  Constructor.
   * BEHAVIOR     :  Init variables.
   * PRE          :  /
   */
  GnomeMeeting ();


  /* DESCRIPTION  :  Destructor.
   * BEHAVIOR     :  
   * PRE          :  /
   */
  ~GnomeMeeting ();

  
  /* DESCRIPTION  :  To connect to a remote endpoint, or to answer a call.
   * BEHAVIOR     :  Answer a call, or call the person with the given URL
   * 		     and put the called URL in the history.
   * PRE          :  /
   */
  void Connect (PString = PString ());


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  To refuse a call, or interrupt the current call.
   * PRE          :  The reason why the call was not disconnected.
   */
  void Disconnect (H323Connection::CallEndReason
		   = H323Connection::EndedByLocalUser);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Init the endpoint component of the application.
   * PRE          :  /
   */
  void Init ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Prepare the endpoint to exit by removing all
   * 		     associated threads and components.
   * PRE          :  /
   */
  void Exit ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Detects the available interfaces.
   *                 Returns FALSE if none is is detected, TRUE
   *                 otherwise. 
   *                 Updates the preferences window.
   * PRE          :  /
   */
  BOOL DetectInterfaces ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Detects the available audio and video managers
   *                 and audio, video devices corresponding to the managers
   *                 selected in config. Returns FALSE if no audio manager
   *                 is detected. Returns TRUE in other cases, even if no
   *                 devices are found.
   *                 Updates the preferences window.
   * PRE          :  /
   */
  BOOL DetectDevices ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the GmWindow structure
   *                 of widgets.
   * PRE          :  /
   */
  GtkWidget *GetMainWindow ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the preferences window GMObject.
   * PRE          :  /
   */
  GtkWidget *GetPrefsWindow ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the chat window GMObject.
   * PRE          :  /
   */
  GtkWidget *GetChatWindow ();
 
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the druid window GMObject.
   * PRE          :  /
   */
  GtkWidget *GetDruidWindow ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the calls history window.
   * PRE          :  /
   */
  GtkWidget *GetCallsHistoryWindow ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the address book window. 
   * PRE          :  /
   */
  GtkWidget *GetAddressbookWindow ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the history window.
   * PRE          :  /
   */
  GtkWidget *GetHistoryWindow ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the PC-2-Phone window.
   * PRE          :  /
   */
  GtkWidget *GetPC2PhoneWindow ();
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the accounts window.
   * PRE          :  /
   */
  GtkWidget *GetAccountsWindow ();
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the tray.
   * PRE          :  /
   */
  GtkWidget *GetStatusicon ();
  
  
#ifdef HAS_DBUS
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the dbus component.
   * PRE          :  /
   */
  GObject *GetDbusComponent ();
#endif
  
  /* Needed for PProcess */
  void Main();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Removes the current endpoint.
   * PRE          :  /
   */  
  void RemoveManager ();
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the current endpoint.
   * PRE          :  /
   */
  GMManager *GetManager ();
  

  static GnomeMeeting *Process ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Builds the GUI of GnomeMeeting. The config, GNOME
   *                 and GTK need to have been initialized before.
   *                 The GUI is built accordingly to the preferences
   *                 stored in config.
   * PRE          :  /
   */
  void BuildGUI ();

  
  /* DESCRIPTION  : / 
   * BEHAVIOR     : Returns the list of detected interfaces. 
   * 		    That doesn't force a redetection. Use DetectInterfaces 
   * 		    for that.
   * PRE          : /
   */
  PStringArray GetInterfaces ();

  
  /* DESCRIPTION  : / 
   * BEHAVIOR     : Returns the list of detected video devices. 
   * 		    That doesn't force a redetection. Use DetectDevices 
   * 		    for that.
   * PRE          : /
   */
  PStringArray GetVideoInputDevices ();
  
  
  /* DESCRIPTION  : / 
   * BEHAVIOR     : Returns the list of detected audio input devices. 
   * 		    That doesn't force a redetection. Use DetectDevices 
   * 		    for that.
   * PRE          : /
   */
  PStringArray GetAudioInputDevices ();
  
  
  /* DESCRIPTION  : / 
   * BEHAVIOR     : Returns the list of detected audio output devices. 
   * 		    That doesn't force a redetection. Use DetectDevices 
   * 		    for that.
   * PRE          : /
   */
  PStringArray GetAudioOutpoutDevices ();
  
  
  /* DESCRIPTION  : / 
   * BEHAVIOR     : Returns the list of detected audio plugins. 
   * 		    That doesn't force a redetection. Use DetectDevices 
   * 		    for that.
   * PRE          : /
   */
  PStringArray GetAudioPlugins ();
  
  
  /* DESCRIPTION  : / 
   * BEHAVIOR     : Returns the list of detected video plugins. 
   * 		    That doesn't force a redetection. Use DetectDevices 
   * 		    for that.
   * PRE          : /
   */
  PStringArray GetVideoPlugins ();
  
  
 private:
  GMManager *endpoint;
  PThread *url_handler;
  
  PMutex ep_var_mutex;
  PMutex dev_access_mutex;
  PMutex iface_access_mutex;
  int call_number;


  /* Detected devices and plugins */
  PStringArray video_input_devices;
  PStringArray audio_input_devices;
  PStringArray audio_output_devices;
  PStringArray audio_managers;
  PStringArray video_managers;


  /* Detected interfaces */
  PStringArray interfaces;


  /* The different components of the GUI */
  GtkWidget *main_window;
  GtkWidget *addressbook_window;
  GtkWidget *calls_history_window;
  GtkWidget *history_window;
  GtkWidget *chat_window;
  GtkWidget *druid_window;
  GtkWidget *prefs_window;
  GtkWidget *pc2phone_window;
  GtkWidget *accounts_window;
  GtkWidget *statusicon;

  /* other things */
#ifdef HAS_DBUS
  GObject *dbus_component;
#endif

  static GnomeMeeting *GM;
};

#endif
