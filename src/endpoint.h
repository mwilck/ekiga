
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2002 Damien Sandras
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
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains the endpoint methods.
 *   email                : dsandras@seconix.com
 *
 */


#ifndef _ENDPOINT_H_
#define _ENDPOINT_H_

#include "common.h"
#include "gdkvideoio.h"
#include "lid.h"

#include <ptlib.h>
#include <h323.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>

#ifdef HAS_IXJ
#include <ixjlid.h>
#endif

 
class GMH323EndPoint : public H323EndPoint
{
  PCLASSINFO(GMH323EndPoint, H323EndPoint);

  
 public:
  
  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Creates the local endpoint and initialises the variables
   * PRE          :  /
   */
  GMH323EndPoint ();


  /* DESCRIPTION  :  The destructor
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMH323EndPoint ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Updates some of the internal values of the endpoint such 
   *                 as the local username, the capabilities, the tunneling,
   *                 the fast start, the audio devices to use, the video 
   *                 device to use, ...
   * PRE          :  /
   */
  void UpdateConfig ();

  /* DESCRIPTION  :  This callback is called to create an instance of
   *                 H323Gatekeeper for gatekeeper registration.
   * BEHAVIOR     :  Return an instance of H323GatekeeperWithNAT
   *                 that implements the Citron NAT Technology.
   * PRE          :  /
   *//*
  virtual H323Gatekeeper * CreateGatekeeper (H323Transport *);
     */
  
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
  virtual BOOL OnIncomingCall (H323Connection &, const H323SignalPDU &,
			       H323SignalPDU &);


  /* DESCRIPTION  :  This callback is called when a call is forwarded.
   * BEHAVIOR     :  Outputs a message in the history and statusbar.
   * PRE          :  /
   */
  virtual BOOL OnConnectionForwarded (H323Connection &, const PString &,
				      const H323SignalPDU &);

  
  /* DESCRIPTION  :  This callback is called when the connection is 
   *                 established and everything is ok.
   *                 It means that a connection to a remote endpoint is ok,
   *                 with one control channel and x >= 0 logical channel(s)
   *                 opened
   * BEHAVIOR     :  Sets the proper values for the current connection 
   *                 parameters (and updates the GUI)
   * PRE          :  /
   */
  virtual void OnConnectionEstablished (H323Connection &,
				        const PString &);
  
  
  /* DESCRIPTION  :  This callback is called when the connection to a remote
   *                 endpoint is cleared.
   * BEHAVIOR     :  Sets the proper values for the current connection 
   *                 parameters and updates the GUI.
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
   * BEHAVIOR     :  Add audio capabilities following the user config.
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
   * BEHAVIOR     :  Adds the User Input capabilities following the
   *                 configuration options. Can set: None, All, rfc2833,
   *                 String, Signal.
   * PRE          :  /
   */
  void AddUserInputCapabilities ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Saves the currently displayed picture in a file.
   * PRE          :  / 
   */
  void SavePicture (void);

  
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
   *                 even if the endpoint is listening on many interfaces
   * PRE          :  /
   */
  gchar *GetCurrentIP (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Translate to packets to the IP of the gateway.
   * PRE          :  /
   */
  virtual void TranslateTCPAddress(PIPSocket::Address &,
				   const PIPSocket::Address &);
  

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Starts an audio tester that will play any recorded
   *                 sound to the speakers in real time. Can be used to
   *                 check if the audio volumes are correct before 
   *                 a conference.
   * PRE          :  /
   */
  void StopAudioTester ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Stops the current audio tester if any.
   *                 a conference.
   * PRE          :  /
   */
  void StartAudioTester ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current connection or 
   *                 NULL if there is no one.
   * PRE          :  /
   */
  H323Connection *GetCurrentConnection (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current audio channel in use
   *                 NULL if there is no one.
   * PRE          :  /
   */
  H323Channel *GetCurrentAudioChannel (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current video channel in use
   *                 NULL if there is no one.
   * PRE          :  /
   */
  H323Channel *GetCurrentVideoChannel (void);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the number of video channels in use.
   *                 0 if there is no one, or if we are not in a call.
   * PRE          :  /
   */
  int GetVideoChannelsNumber (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current video codec in use
   *                 NULL if there is no one.
   * PRE          :  /
   */
  H323VideoCodec *GetCurrentVideoCodec (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current audio codec in use
   *                 NULL if there is no one.
   * PRE          :  /
   */
  H323AudioCodec *GetCurrentAudioCodec (void);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the current connection to the parameter.
   * PRE          :  A valid pointer to the current connection.
   */
  void SetCurrentConnection (H323Connection *);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the current call token.
   * PRE          :  A valid PString for a call token.
   */
  void SetCurrentCallToken (PString);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current call token (empty if
   *                 no call is in progress).
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
   * BEHAVIOR     :  Return the current ILS/LDAP client thread.
   * PRE          :  /
   */
  PThread* GetILSClientThread ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the local user name following the firstname and last 
   *                 name stored by gconf, set the gatekeeper alias.
   * PRE          :  /
   */
  void SetUserNameAndAlias ();


#ifdef HAS_IXJ
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current Lid Thread.
   * PRE          :  /
   */
  GMLid *GetLidThread ();
#endif


 protected:
  
  PString current_call_token;  
  H323Connection *current_connection;  
  H323ListenerTCP *listener;  

  int calling_state; 
  int docklet_timeout; 
  int sound_timeout; 
  int display_config; 

  GDKVideoOutputDevice *transmitted_video_device; 
  GDKVideoOutputDevice *received_video_device; 

  PSoundChannel *player_channel;
  PSoundChannel *recorder_channel;

  GmWindow *gw; 
  GmLdapWindow *lw;
  GmTextChat *chat;
  GConfClient *client;

  PThread *ils_client;
  PThread *audio_tester;

  PMutex var_access_mutex;

  int opened_audio_channels;
  int opened_video_channels;

#ifdef HAS_IXJ
  GMLid *lid_thread;
#endif
};

#endif
