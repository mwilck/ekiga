/***************************************************************************
                          endpoint.h  -  description
                             -------------------
    begin                : Sat Dec 23 2000
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : This file contains the class declaration for the
                           class that manages the local endpoint
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
#include <stdio.h>
#include <lpc10codec.h>
#include <pthread.h>

#include "common.h"
#include "webcam.h"
#include "gdkvideoio.h"


/******************************************************************************/
// DESCRIPTION  :  This callback is called by a timeout function
// BEHAVIOR     :  Plays the sound choosen in the gnome contro center
// PRE          :  The pointer to the applet must be valid
 gint PlaySound (GtkWidget *);

/******************************************************************************/


/******************************************************************************/
/*   GMH323EndPoint : manages the local endpoint                              */
/******************************************************************************/
 
class GMH323EndPoint : public H323EndPoint
{
  PCLASSINFO(GMH323EndPoint, H323EndPoint);

  
 public:
  
  // DESCRIPTION  :  The constructor
  // BEHAVIOR     :  Creates the local endpoint
  // PRE          :  GM_window_widgets is a valid pointer to a valid
  //                 struct containing all the widgets needed to manage
  //                 and update the main GUI, options * is valid too
  GMH323EndPoint (GM_window_widgets *, options *);


  // DESCRIPTION  :  The destructor
  // BEHAVIOR     :  Deletes the GMH323Webcam grabbing device
  // PRE          :
  ~GMH323EndPoint ();

  
  // COMMON NOTICE :The following virtual functions override from H323EndPoint

  
  // DESCRIPTION  :  This callback is called if we create a connection
  //                 or if somebody calls and we accept the call
  // BEHAVIOR     :  Creates a connection using the call reference
  //                 given as parameter which is given by OpenH323
  // PRE          :  /
  virtual H323Connection *CreateConnection (unsigned);
  

  // DESCRIPTION  :  This callback is called on an incoming call
  // BEHAVIOR     :  If a call is already running, returns FALSE
  //                 -> the incoming call is not accepted, else
  //                 returns TRUE which was the default behavior
  //                 if we had not defined it
  // PRE          :  /
  virtual BOOL OnIncomingCall(H323Connection &, const H323SignalPDU &,
			      H323SignalPDU &);


  
  // DESCRIPTION  :  This callback is called when the connection is established
  //                 and everything is ok
  //                 It means that a connection to a remote endpoint is ok,
  //                 with one control channel and x >= 0 logical channel(s)
  //                 opened
  // BEHAVIOR     :  Sets the proper values for the current connection parameters
  //                 (and updates the applet, log window and statusbar)
  // PRE          :  /
  virtual void OnConnectionEstablished (H323Connection &,
				        const PString &);

  
  // DESCRIPTION  :  This callback is called when the connection to a remote
  //                 endpoint is cleared
  // BEHAVIOR     :  Sets the proper values for the current connection parameters
  //                 (and updates the applet, log window and statusbar)
  // PRE          :  /
  virtual void OnConnectionCleared (H323Connection &,
				    const PString &);


  // DESCRIPTION  :  This callback is called when a video device has to be opened
  // BEHAVIOR     :  Creates a GDKVideoOutputDevice for the local and remote
  //                 image display
  // PRE          :  /
  virtual BOOL OpenVideoChannel (H323Connection &,
				 BOOL, H323VideoCodec &);

  
  // DESCRIPTION  :  This callback is called when an audio channel has to
  //                 be opened
  // BEHAVIOR     :  Opens the Audio Channel or warns the user if it was
  //                 impossible
  // PRE          :  /
  virtual BOOL OpenAudioChannel (H323Connection &, BOOL,
				 unsigned, H323AudioCodec &);

  
  // DESCRIPTION  :  /
  // BEHAVIOR     :  Initialises the endpoint's parameters following the config
  //                 file and register to LDAP server if needed and sets audio
  //                 sources
  // PRE          :  /
  BOOL Initialise ();

  
  // DESCRIPTION  :  /
  // BEHAVIOR     :  ReInitialises the endpoint's parameters following the config
  //                 file which has been updated 
  // PRE          :  /
  void ReInitialise ();


  // DESCRIPTION  :  /
  // BEHAVIOR     :  Remove the capability corresponding to the PString and
  //                 return the remaining capabilities list
  // PRE          :  /
  H323Capabilities RemoveCapability (PString);


  // DESCRIPTION  :  /
  // BEHAVIOR     :  Remove all capabilities of the endpoint
  // PRE          :  /
  void RemoveAllCapabilities (void);

  
  // DESCRIPTION  :  /
  // BEHAVIOR     :  Add audio capabilities following the user's preferences
  // PRE          :  /
  void AddAudioCapabilities (void);

  
  // DESCRIPTION  :  /
  // BEHAVIOR     :  Add video capabilities, with QCIF as first video
  //                 capability if the parameter is 0, else CIF will be the
  //                 first video capability
  // PRE          :  /
  void AddVideoCapabilities (int);

  
  // DESCRIPTION  :  /
  // BEHAVIOR     :  Changes the webcam image to be displayed in the GUI :
  //                   O : local image
  //                   1 : remote image
  //                   2 : both images
  // PRE          :  /
  void DisplayConfig (int);

  
  // DESCRIPTION  :  /
  // BEHAVIOR     :  Sets the current calling state :
  //                   0 : not in a call
  //                   1 : calling somebody
  //                   2 : currently in a call 
  // PRE          :  /
  void SetCallingState (int);


  // DESCRIPTION  :  /
  // BEHAVIOR     :  Gets the current calling state :
  //                   0 : not in a call
  //                   1 : calling somebody
  //                   2 : currently in a call 
  // PRE          :  /
  int CallingState (void);



  // DESCRIPTION  :  /
  // BEHAVIOR     :  Returns the current IP of the endpoint, even if the endpoint
  //                 is listening on many interfaces
  // PRE          :  EndPoint has to be initialised
  char *IP (void);

  
  // DESCRIPTION  :  /
  // BEHAVIOR     :  Returns the current connection or NULL if there is no one
  // PRE          :
  H323Connection *Connection (void);

  
  // DESCRIPTION  :  /
  // BEHAVIOR     :  Set the current connection to the parameter
  // PRE          :  a valid pointer to the current connection
  void SetCurrentConnection (H323Connection *);

  
  // DESCRIPTION  :  /
  // BEHAVIOR     :  Set the current call token
  // PRE          :  a valid PString for a call token (given by OpenH323)
  void SetCurrentCallToken (PString);

  
  // DESCRIPTION  :  /
  // BEHAVIOR     :  Returns the current call token that will be empty if
  //                 no call is in progress (can be tested with .isEmpty ())
  // PRE          :  /
  PString CallToken (void);


  // DESCRIPTION  :  /
  // BEHAVIOR     :  Returns the current webcam grabbing device
  // PRE          :  /
  GMH323Webcam *Webcam (void);
  
 protected:
  
  PString current_call_token;  // the current Call Token
  H323Connection *current_connection;  // pointer to the current connection
  options *opts;  // pointer to options (will be read in the config file)
  int calling_state; // current calling state
  int applet_timeout; // timeout associated with the animated applet
  int sound_timeout; // timeout associated with the sound
  int display_config; // webcam image to display
  GDKVideoOutputDevice *transmitted_video_device; // GDKVideoOutputDevice : sent
  GDKVideoOutputDevice *received_video_device; // GDKVideoOutputDevice : received
  GM_window_widgets *gw; // main window widgets that need to be updated
  GMH323Webcam *webcam;
};

/******************************************************************************/

#endif
