
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
 *                         gnomemeeting.h  -  description
 *                         ------------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains the main class
 *
 */


#ifndef _GNOMEMEETING_H_
#define _GNOMEMEETING_H_

#include "common.h"
#include "endpoint.h"


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
   * BEHAVIOR     :  Answer a call, or call the person with the given URL.
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
   * BEHAVIOR     :  Detects the available audio and video managers
   *                 and audio, video devices corresponding to the managers
   *                 selected in config and puts the result in the internal
   *                 GmWindow structure. Returns FALSE if no audio manager
   *                 is detected. Returns TRUE in other cases, even if no
   *                 devices are found.
   * PRE          :  /
   */
  BOOL DetectDevices ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the GmWindow structure
   *                 of widgets.
   * PRE          :  /
   */
  GmWindow *GetMainWindow ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the preferences window GMObject.
   * PRE          :  /
   */
  GtkWidget *GetPrefsWindow ();


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
   * BEHAVIOR     :  Returns a pointer to the GmRtpData
   *                 structure of widgets.
   * PRE          :  /
   */
  GmRtpData *GetRtpData ();
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the history window.
   * PRE          :  /
   */
  GtkWidget *GetHistoryWindow ();

  
  /* Needed for PProcess */
  void Main();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Removes the current endpoint.
   * PRE          :  /
   */  
  void RemoveEndpoint ();
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the current endpoint.
   * PRE          :  /
   */
  GMH323EndPoint *Endpoint (void);
  

  static GnomeMeeting *Process ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Builds the GUI of GnomeMeeting. The config, GNOME
   *                 and GTK need to have been initialized before.
   *                 The GUI is built accordingly to the preferences
   *                 stored in config and then show or hidden following
   *                 them. Notice that a druid is displayed if it is
   *                 a first time run.
   * PRE          :  /
   */
  void BuildGUI ();

  
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
  GMH323EndPoint *endpoint;
  PThread *url_handler;
  
  GmWindow *gw;
  GmRtpData *rtp;

  PMutex ep_var_mutex;
  PMutex dev_access_mutex;
  int call_number;


  /* Detected devices and plugins */
  PStringArray video_input_devices;
  PStringArray audio_input_devices;
  PStringArray audio_output_devices;
  PStringArray audio_managers;
  PStringArray video_managers;

  
  /* The different components of the GUI */
  GtkWidget *addressbook_window;
  GtkWidget *calls_history_window;
  GtkWidget *history_window;
  GtkWidget *druid_window;
  GtkWidget *prefs_window;

  static GnomeMeeting *GM;
};

#endif
