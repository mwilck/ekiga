
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
 *                         endpoint.h  -  description
 *                         --------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains miscellaneous functions.
 *   email                : dsandras@seconix.com
 *
 */


#ifndef _ENDPOINT_H_
#define _ENDPOINT_H_

#include <ptlib.h>
#include <h323.h>
#include <gnome.h>
#include <gsmcodec.h>
#include <mscodecs.h>
#include <h261codec.h>
#include <videoio.h>
#include <gnome.h>
#include <lpc10codec.h>
#include <pthread.h>

#include "common.h"
#include "videograbber.h"
#include "gdkvideoio.h"
#include "ldap_window.h"

 
class GMH323EndPoint : public H323EndPoint
{
  PCLASSINFO(GMH323EndPoint, H323EndPoint);

  
 public:
  
  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Creates the local endpoint.
   *                 Initialise the variables, VideoGrabber and ILSClient..
   * PRE          :  /
   */
  GMH323EndPoint (options *);


  /* DESCRIPTION  :  The destructor
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMH323EndPoint ();

  
  /* COMMON NOTICE :
     The following virtual functions override from H323EndPoint */

  
  /* DESCRIPTION  :  This callback is called if we create a connection
   *                 or if somebody calls and we accept the call.
   * BEHAVIOR     :  Create a connection using the call reference
   *                 given as parameter which is given by OpenH323.
   * PRE          :  /
   */
  virtual H323Connection *CreateConnection (unsigned);
  

  /* DESCRIPTION  :  This callback is called on an incoming call.
   * BEHAVIOR     :  If a call is already running, returns FALSE
   *                 -> the incoming call is not accepted, else
   *                 returns TRUE which was the default behavior
   *                 if we had not defined it.
   *
   * PRE          :  /
   */
  virtual BOOL OnIncomingCall(H323Connection &, const H323SignalPDU &,
			      H323SignalPDU &);


  
  /* DESCRIPTION  :  This callback is called when the connection is 
   *                  established and everything is ok.
   *                 It means that a connection to a remote endpoint is ok,
   *                 with one control channel and x >= 0 logical channel(s)
   *                 opened
   * BEHAVIOR     :  Sets the proper values for the current connection 
   *                 parameters
   *                 (and updates the docklet, log window and statusbar)
   * PRE          :  /
   */
  virtual void OnConnectionEstablished (H323Connection &,
				        const PString &);
  
  
  /* DESCRIPTION  :  This callback is called when the connection to a remote
   *                 endpoint is cleared.
   * BEHAVIOR     :  Sets the proper values for the current connection 
   *                 parameters
   *                 (and updates the docklet, log window and statusbar)
   * PRE          :  /
   */
  virtual void OnConnectionCleared (H323Connection &,
				    const PString &);


  /* DESCRIPTION  :  This callback is called when a video device 
   *                 has to be opened.
   * BEHAVIOR     :  Create a GDKVideoOutputDevice for the local and remote
   *                 image display.
   * PRE          :  /
   */
  virtual BOOL OpenVideoChannel (H323Connection &,
				 BOOL, H323VideoCodec &);

  
  /* DESCRIPTION  :  This callback is called when an audio channel has to
   *                 be opened.
   * BEHAVIOR     :  Opens the Audio Channel or warns the user if it was
   *                 impossible.
   * PRE          :  /
   */
  virtual BOOL OpenAudioChannel (H323Connection &, BOOL,
				 unsigned, H323AudioCodec &);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Starts the listener thread on the port choosen 
   *                 in the options.
   *                 returns TRUE if success and FALSE in case of error.
   */
  BOOL StartListener ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Initialise the endpoint's parameters following 
   *                 the config file.
   * PRE          :  /
   */
  BOOL Initialise ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  ReInitialises the endpoint's parameters 
   *                 following the config.
   * PRE          :  /
   */
  void Reset ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Remove the capability corresponding to the PString and
   *                 return the remaining capabilities list.
   * PRE          :  /
   */
  H323Capabilities RemoveCapability (PString);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Remove all capabilities of the endpoint.
   * PRE          :  /
   */
  void RemoveAllCapabilities (void);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Add audio capabilities following the config file.
   * PRE          :  /
   */
  void AddAudioCapabilities (void);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Add video capabilities, with QCIF as first video
   *                 capability if the parameter is 0, else CIF will be the
   *                 first video capability.
   * PRE          :  /
   */
  void AddVideoCapabilities (int);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Change the webcam image to be displayed in the GUI :
   *                   O : local image
   *                   1 : remote image
   *                   2 : both images
   * PRE          :  / 
   */
  void SetCurrentDisplay (int);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the current calling state :
   *                   0 : not in a call
   *                   1 : calling somebody
   *                   2 : currently in a call 
   * PRE          :  /
   */
  void SetCallingState (int);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Get the current calling state :
   *                   0 : not in a call
   *                   1 : calling somebody
   *                   2 : currently in a call 
   * PRE          :  /
   */
  int GetCallingState (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current IP of the endpoint, 
   *                 even if the endpoint
   *                 is listening on many interfaces
   * PRE          :  EndPoint has to be initialised
   */
  gchar *GetCurrentIP (void);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current connection or 
   *                 NULL if there is no one.
   * PRE          :  /
   */
  H323Connection *GetCurrentConnection (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current video codec in use
   *                 NULL if there is no one.
   * PRE          :  /
   */
  H323VideoCodec *GetCurrentVideoCodec (void);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the current connection to the parameter.
   * PRE          :  A valid pointer to the current connection.
   */
  void SetCurrentConnection (H323Connection *);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the current call token.
   * PRE          :  a valid PString for a call token (given by OpenH323)
   */
  void SetCurrentCallToken (PString);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current call token that will be empty if
   *                 no call is in progress (can be tested with .isEmpty ())
   * PRE          :  /
   */
  PString GetCurrentCallToken (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current gatekeeper.
   * PRE          :  /
   */
  H323Gatekeeper *GetGatekeeper (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Register to the gatekeeper given in the options, 
   *                 if any.
   * PRE          :  /
   */
  void GatekeeperRegister (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current webcam grabbing device.
   * PRE          :  /
   */
  GMVideoGrabber *GetVideoGrabber (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  If we are in a call, and that audio channels are opened
   *                 using codecs that support silence detection, changes
   *                 the silence detection.
   * PRE          :  /
   */
  void ChangeSilenceDetection (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current ILS/LDAP client thread.
   * PRE          :  /
   */
  PThread* GetILSClient ();


 protected:
  
  PString current_call_token;  
  H323Connection *current_connection;  
  H323ListenerTCP *listener;  
  options *opts;  
  int calling_state; 
  int docklet_timeout; 
  int sound_timeout; 
  int display_config; 
  GDKVideoOutputDevice *transmitted_video_device; 
  GDKVideoOutputDevice *received_video_device; 
  GM_window_widgets *gw; 
  GM_ldap_window_widgets *lw;

  PThread *ils_client;
  PThread *video_grabber;
};

#endif
