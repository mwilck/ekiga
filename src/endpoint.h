
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
 *                         endpoint.h  -  description
 *                         --------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains the Endpoint class.
 *
 */


#ifndef _ENDPOINT_H_
#define _ENDPOINT_H_

#include "../config.h"

#include "common.h"
#include "gdkvideoio.h"
#include "videograbber.h"
#include "ils.h"
#include "lid.h"

#ifdef HAS_IXJ
#include <ixjlid.h>
#endif

 
class GMH323EndPoint : public H323EndPoint
{
  PCLASSINFO(GMH323EndPoint, H323EndPoint);

  
 public:

  enum {Standby, Calling, Connected, Called};

  
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
   * BEHAVIOR     :  Makes a call to the given address, and fills in the
   *                 call taken. It returns a locked pointer to the connection
   *                 in case of success.
   * PRE          :  The called address, its call token.
   */
  H323Connection *MakeCallLocked (const PString &, PString &);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Updates some of the internal values of the endpoint such 
   *                 as the local username, the capabilities, the tunneling,
   *                 the fast start, the audio devices to use, the video 
   *                 device to use, ...
   * PRE          :  /
   */
  void UpdateConfig ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Creates a video grabber.
   * PRE          :  If TRUE, then the grabber will start
   *                 grabbing after its creation. If TRUE,
   *                 then the opening is done sync. If TRUE, then the channel
   *                 will be autodeleted.
   */  
  GMVideoGrabber *CreateVideoGrabber (BOOL = TRUE, BOOL = FALSE, BOOL = TRUE);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Removes the current video grabber, if any.
   * PRE          :  /
   */  
  void RemoveVideoGrabber ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the current videograbber, if any.
   *                 The pointer is locked, which means that the device
   *                 can't be deleted until the pointer is unlocked with
   *                 Unlock. This is a protection to always manipulate
   *                 existing objects. Notice that all methods in the
   *                 GMH323EndPoint class related to the GMVideoGrabber
   *                 use internal mutexes so that the pointer cannot be
   *                 returned during a RemoveVideoGrabber/CreateVideoGrabber,
   *                 among others. You should use those functions and not
   *                 manually delete the GMVideoGrabber.
   * PRE          :  /
   */
  GMVideoGrabber *GetVideoGrabber ();

  
  /* DESCRIPTION  :  This callback is called to create an instance of
   *                 H323Gatekeeper for gatekeeper registration.
   * BEHAVIOR     :  Return an instance of H323GatekeeperWithNAT
   *                 that implements the Citron NAT Technology.
   * PRE          :  /
   */
  virtual H323Gatekeeper * CreateGatekeeper (H323Transport *);
     
  
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
  virtual BOOL OnConnectionForwarded (H323Connection &,
				      const PString &,
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

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the remote party name (UTF-8), the
   *                 remote application name (UTF-8), and the best
   *                 guess for the URL to use when calling back the user
   *                 (the IP, or the alias or e164 number if the local
   *                 user is registered to a gatekeeper. Not always accurate,
   *                 for example if you are called by an user with an alias,
   *                 but not registered to the same GK as you.)
   * PRE          :  /
   */
  void GetRemoteConnectionInfo (H323Connection &,
				gchar * &,
				gchar * &,
				gchar * &);

  
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
   * BEHAVIOR     :  Add all possible capabilities for the endpoint.
   * PRE          :  /
   */
  void AddAllCapabilities (void);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Add audio capabilities following the user config.
   * PRE          :  /
   */
  void AddAudioCapabilities (void);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Add video capabilities.
   * PRE          :  /
   */
  void AddVideoCapabilities ();


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
   *                   { Standby,
   *                     Calling,
   *                     Connected,
   *                     Called }
   * PRE          :  /
   */
  void SetCallingState (unsigned);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the current calling state :
   *                   { Standby,
   *                     Calling,
   *                     Connected,
   *                     Called }
   * PRE          :  /
   */
  unsigned GetCallingState (void);


  /* Overrides from H323Endpoint */
  H323Connection *SetupTransfer (const PString &,
				 const PString &,
				 const PString &,
				 PString &,
				 void * = NULL);
  

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current IP of the endpoint, 
   *                 even if the endpoint is listening on many interfaces
   * PRE          :  /
   */
  PString GetCurrentIP (void);


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
   * BEHAVIOR     :  Register to (or unregister from) the gatekeeper 
   *                 given in the options, if any.
   * PRE          :  /
   */
  void GatekeeperRegister (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Register to (or unregister from) the ILS server
   *                 given in the options, if any.
   * PRE          :  /
   */
  void ILSRegister ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the local user name following the firstname and last 
   *                 name stored by gconf, set the gatekeeper alias.
   * PRE          :  /
   */
  void SetUserNameAndAlias ();


  /* FIX ME: Comments */
  BOOL SetSoundChannelPlayDevice(const PString &);
  BOOL SetSoundChannelRecordDevice(const PString &);
  BOOL SetSoundChannelManager (const PString &);
  PString GetSoundChannelManager () {return soundChannelManager;}
  BOOL SetDeviceVolume (unsigned int, unsigned int);
  BOOL GetDeviceVolume (unsigned int &, unsigned int &);
  void SetAutoStartTransmitVideo (BOOL a) {autoStartTransmitVideo = a;}
  void SetAutoStartReceiveVideo (BOOL a) {autoStartReceiveVideo = a;}
  BOOL StartLogicalChannel (const PString &, unsigned int, BOOL);
  BOOL StopLogicalChannel (unsigned int, BOOL);


#ifdef HAS_IXJ
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the current Lid Thread pointer, locked so that
   *                 the object content is protected against deletion. See
   *                 GetVideoGrabber ().
   * PRE          :  /
   */
  GMLid *GetLid ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Removes the current Lid.
   * PRE          :  /
   */
  void RemoveLid ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Create a new Lid.
   * PRE          :  /
   */
  void CreateLid ();
#endif


 protected:

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the last called call token.
   * PRE          :  /
   */
  PString GetLastCallAddress ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the current transfer call token.
   * PRE          :  A valid PString for a call token.
   */
  void SetTransferCallToken (PString);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current transfer call token (empty if
   *                 no call is in progress).
   * PRE          :  /
   */
  PString GetTransferCallToken (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns when the primary call of a transfer is terminated.
   * PRE          :  /
   */
  void TransferCallWait ();

  BOOL DeviceVolume (BOOL, unsigned int &, unsigned int &);

  PString CheckTCPPorts ();
  
  PDECLARE_NOTIFIER(PTimer, GMH323EndPoint, OnILSTimeout);
  PDECLARE_NOTIFIER(PTimer, GMH323EndPoint, OnRTPTimeout);
  PDECLARE_NOTIFIER(PTimer, GMH323EndPoint, OnGatewayIPTimeout);
  PDECLARE_NOTIFIER(PTimer, GMH323EndPoint, OnNoAnswerTimeout);
  PDECLARE_NOTIFIER(PTimer, GMH323EndPoint, OnCallPending);

  PString called_address;
  PString current_call_token;
  PString transfer_call_token;
  PString soundChannelManager;

  H323Connection *current_connection;  
  H323ListenerTCP *listener;  

  unsigned calling_state; 

  GDKVideoOutputDevice *transmitted_video_device; 
  GDKVideoOutputDevice *received_video_device; 

  PTimer ILSTimer;
  PTimer RTPTimer;
  PTimer GatewayIPTimer;
  PTimer NoAnswerTimer;
  PTimer CallPendingTimer;

  BOOL ils_registered;

  GmWindow *gw; 
  GmLdapWindow *lw;
  GmTextChat *chat;

  GMVideoGrabber *video_grabber;
  GMILSClient *ils_client;
  PThread *audio_tester;

  PMutex vg_access_mutex;
  PMutex ils_access_mutex;
  PMutex cs_access_mutex;
  PMutex ct_access_mutex;
  PMutex tct_access_mutex;
  PMutex lid_access_mutex;
  PMutex at_access_mutex;
  PMutex lca_access_mutex;
  
#ifdef HAS_IXJ
  GMLid *lid;
#endif

  BOOL is_transmitting_video;
  BOOL is_receiving_video;
  BOOL is_transmitting_audio;
  BOOL is_receiving_audio;
};

#endif
