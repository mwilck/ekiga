
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
 *                         endpoint.h  -  description
 *                         --------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains the Endpoint class.
 *
 */


#ifndef _ENDPOINT_H_
#define _ENDPOINT_H_


#include "../config.h"

#include "common.h"

#ifdef HAS_HOWL
#include "zeroconf_publisher.h"
#endif

#ifdef HAS_IXJ
#include <lids/ixjlid.h>
#endif


#include "gdkvideoio.h"
#include "videograbber.h"
#include "stunclient.h"

#include <contacts/gm_contacts.h>


class GMILSClient;
class GMLid;
class GMH323Gatekeeper;
class GMPCSSEndPoint;
class GMH323EndPoint;


/**
 * COMMON NOTICE: The Endpoint must be initialized with Init after its
 * creation.
 */

class GMEndPoint : public OpalManager
{
  PCLASSINFO(GMEndPoint, OpalManager);

  friend class GMPCSSEndPoint;
  friend class GMH323EndPoint;
  
 public:

  enum CallingState {Standby, Calling, Connected, Called};

  
  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Creates the supported endpoints 
   * 		     and initialises the variables
   * PRE          :  /
   */
  GMEndPoint ();


  /* DESCRIPTION  :  The destructor
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMEndPoint ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Init the endpoint internal values and the various
   *                 components.
   * PRE          :  /
   */
  void Init ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Makes a call to the given address, and fills in the
   *                 call taken. 
   * PRE          :  The called url, the call token.
   */
  BOOL SetUpCall (const PString &,
		  PString &);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Accepts the current incoming call, if any.
   * 		     There is always only one current incoming call at a time.
   * 		     If there is another incoming call in-between, 
   * 		     it is rejected.
   * PRE          :  /
   */
  BOOL AcceptCurrentIncomingCall ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Update the internal audio and video devices for playing
   *                 and recording following the config database content.
   *                 If a Quicknet card is used, it will be opened, and if
   *                 a video grabber is used in preview mode, it will also
   *"                be opened.
   * PRE          :  /
   */
  void UpdateDevices ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Creates a video grabber.
   * PRE          :  If TRUE, then the grabber will start
   *                 grabbing after its creation. If TRUE,
   *                 then the opening is done sync.
   */  
  GMVideoGrabber *CreateVideoGrabber (BOOL = TRUE, 
				      BOOL = FALSE);


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
     
  
  /* DESCRIPTION  :  This callback is called if we create a connection
   *                 or if somebody calls and we accept the call.
   * BEHAVIOR     :  Create a connection using the call reference
   *                 given as parameter which is given by OpenH323.
   * PRE          :  /
   */
  // FIXME
  //virtual H323Connection *CreateConnection (unsigned);


  /* DESCRIPTION  :  This callback is called when a call is forwarded.
   * BEHAVIOR     :  Outputs a message in the log and statusbar.
   * PRE          :  /
   */
  // FIXME
  //virtual BOOL OnConnectionForwarded (H323Connection &,
//				      const PString &,
//				      const H323SignalPDU &);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the local or remote OpalConnection for the 
   * 		     given call. If there are several remote connections,
   * 		     the first one is returned.
   * PRE          :  /
   */
  PSafePtr<OpalConnection> GetConnection (PSafePtr<OpalCall>, 
					  BOOL);


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
  void GetRemoteConnectionInfo (OpalConnection &,
				gchar * &,
				gchar * &,
				gchar * &);

  
  /* DESCRIPTION  :  This callback is called when the connection is 
   *                 established and everything is ok.
   * BEHAVIOR     :  Sets the proper values for the current connection 
   *                 parameters (and updates the GUI).
   *                 Notice there are 2 connections for each call.
   * PRE          :  /
   */
  void OnEstablished (OpalConnection &);


  /* DESCRIPTION  :  This callback is called when a connection to a remote
   *                 endpoint is cleared.
   * BEHAVIOR     :  Sets the proper values for the current connection 
   *                 parameters and updates the GUI.
   *                 Notice there are 2 connections for each call.
   * PRE          :  /
   */
  void OnReleased (OpalConnection &);

  
  /* DESCRIPTION  :  This callback is called when receiving an input string.
   * BEHAVIOR     :  Updates the text chat.
   * PRE          :  /
   */
  void OnUserInputString (OpalConnection &,
			  const PString &);
  
  
  /* DESCRIPTION  :  This callback is called when a video device 
   *                 has to be opened.
   * BEHAVIOR     :  Create a GDKVideoOutputDevice for the local and remote
   *                 image display.
   * PRE          :  /
   */
  //FIXME
//  virtual BOOL OpenVideoChannel (H323Connection &,
//				 BOOL, H323VideoCodec &);

			       
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Starts the listener thread on the port choosen 
   *                 in the options.
   *                 returns TRUE if success and FALSE in case of error.
   * PRE          :  /
   */
  BOOL StartListener ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Remove the capability corresponding to the PString and
   *                 return the remaining capabilities list.
   * PRE          :  /
   */
  //FIXME
  //H323Capabilities RemoveCapability (PString);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Remove all capabilities of the endpoint.
   * PRE          :  /
   */
  //FIXME
  //void RemoveAllCapabilities (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Add all possible capabilities for the endpoint.
   * PRE          :  /
   */
  //FIXME
  //void AddAllCapabilities (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the list of audio formats supported by
   * 		     the endpoint.
   * PRE          :  /
   */
  OpalMediaFormatList GetAudioMediaFormats ();
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Add audio capabilities following the user config.
   * PRE          :  /
   */
  //FIXME
  //  void AddAudioCapabilities (void);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Add video capabilities.
   * PRE          :  /
   */
  //FIXME
  //void AddVideoCapabilities ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Adds the User Input capabilities following the
   *                 configuration options. Can set: None, All, rfc2833,
   *                 String, Signal.
   * PRE          :  /
   */
  //FIXME
  //void AddUserInputCapabilities ();


  BOOL TranslateIPAddress (PIPSocket::Address &, 
			   const PIPSocket::Address &);
  
  
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
  void SetCallingState (CallingState);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the current calling state :
   *                   { Standby,
   *                     Calling,
   *                     Connected,
   *                     Called }
   * PRE          :  /
   */
  CallingState GetCallingState (void);


  /* Overrides from H323Endpoint */
  //FIXME
  //H323Connection *SetupTransfer (const PString &,
//				 const PString &,
//				 const PString &,
//				 PString &,
//				 void * = NULL);
  

  // FIXME
  GMH323EndPoint *GetH323EndPoint ();
  GMPCSSEndPoint *GetPCSSEndPoint ();
  

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the current IP of the endpoint, 
   *                 even if the endpoint is listening on many interfaces
   * PRE          :  /
   */
  PString GetCurrentIP (void);
  

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Starts an audio tester that will play any recorded
   *                 sound to the speakers in real time. Can be used to
   *                 check if the audio volumes are correct before 
   *                 a conference.
   * PRE          :  /
   */
  void StopAudioTester ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Stops the current audio tester if any for the given
   *                 audio manager, player and recorder.
   * PRE          :  /
   */
  void StartAudioTester (gchar *,
			 gchar *,
			 gchar *);


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
   * BEHAVIOR     :  Sets the given Stun Server as default STUN server for
   * 		     calls.
   * PRE          :  /
   */
  void SetSTUNServer (void);

  
#ifdef HAS_HOWL
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Update the information published by zeroconf.
   * PRE          :  /
   */
  void ZeroconfUpdate (void);
#endif

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Register to (or unregister from) the ILS server
   *                 given in the options, if any.
   * PRE          :  /
   */
  void ILSRegister ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Updates the user name and alias(es) for all managed
   * 		     endpoints following the preferences.
   * PRE          :  /
   */
  void SetUserNameAndAlias ();

    
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Update the RTP, TCP, UDP ports from the config database.
   * PRE          :  /
   */
  void SetPorts ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Update the audio device volume (playing then recording). 
   * PRE          :  /
   */
  BOOL SetDeviceVolume (PSoundChannel *, BOOL, unsigned int);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the audio device volume (playing then recording). 
   * PRE          :  /
   */
  BOOL GetDeviceVolume (PSoundChannel *, BOOL, unsigned int &);
  


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  TRUE if the video should automatically be transmitted
   *                 when a call begins.
   * PRE          :  /
   */
  void SetAutoStartTransmitVideo (BOOL a) {autoStartTransmitVideo = a;}


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  TRUE if the video should automatically be received
   *                 when a call begins.
   * PRE          :  /
   */
  void SetAutoStartReceiveVideo (BOOL a) {autoStartReceiveVideo = a;}

  
  /* DESCRIPTION  :  Callback called when OpenH323 opens a new logical channel
   * BEHAVIOR     :  Updates the log window with information about it, returns
   *                 FALSE if error, TRUE if OK
   * PRE          :  /
   */
  virtual BOOL OnOpenMediaStream (OpalConnection &,
				  OpalMediaStream &);


  /* DESCRIPTION  :  Callback called when OpenH323 closes a new logical channel
   * BEHAVIOR     :  Close the channel and update the GUI..
   * PRE          :  /
   */
  virtual void OnClosedMediaStream (OpalMediaStream &);
  

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Automatically starts transmitting (BOOL = FALSE) or
   *                 receiving (BOOL = TRUE) with the given
   *                 capability, for the given session RTP id and returns
   *                 TRUE if it was successful. Do that only if there is
   *                 a connection.
   * PRE          :  /
   */
  //FIXME
  //BOOL StartLogicalChannel (const PString &, unsigned int, BOOL);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Automatically stop transmitting (BOOL = FALSE) or
   *                 receiving (BOOL = TRUE) for the given RTP session id
   *                 and returns TRUE if it was successful.
   *                 Do that only if there is a connection.
   * PRE          :  /
   */
  //FIXME
  //BOOL StopLogicalChannel (unsigned int, BOOL);


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
   * PRE          :  Non-empty device name.
   */
  GMLid *CreateLid (PString);
#endif


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Sends the given text to the other end
   * PRE          :  /
   */
  void SendTextMessage (PString callToken, PString message);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Checks if the call is on hold
   * PRE          :  /
   */
  BOOL IsCallOnHold (PString callToken);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Sets the call on hold or retrieve it and returns
   * 		     FALSE if it fails, TRUE if it worked.
   * PRE          :  /
   */
  BOOL SetCallOnHold (PString callToken, 
		      gboolean state);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Send DTMF to the remote user if any.
   * PRE          :  /
   */
  void SendDTMF (PString,
		 PString);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Checks if the call has an audio channel
   * PRE          :  Non-empty call token.
   */
  gboolean IsCallWithAudio (PString callToken);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Checks if the call has a video channel
   * PRE          :  Non-empty call token.
   */
  gboolean IsCallWithVideo (PString callToken);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Checks if the call's audio channel is paused
   * PRE          :  Non-empty call token.
   */
  gboolean IsCallAudioPaused (PString callToken);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Checks if the call's video channel is paused
   * PRE          :  Non-empty call token.
   */
  gboolean IsCallVideoPaused (PString callToken);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Sets the call's audio channel on pause, or retrieve it
   * PRE          :  Non-empty call token.
   */
  BOOL SetCallAudioPause (PString callToken, 
			  BOOL state);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Sets the call's video channel on pause, or retrieve it
   * PRE          :  Non-empty call token.
   */
  BOOL SetCallVideoPause (PString callToken, 
			  BOOL state);
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Reset the missed calls number.
   * PRE          :  /
   */
  void ResetMissedCallsNumber ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the missed calls number.
   * PRE          :  /
   */
  int GetMissedCallsNumber ();
  
  
  /* DESCRIPTION  : /
   * BEHAVIOR     : Adds the observer to the list of GObject interested in
   *                the endpoint's signals.
   * PRE          : Non-empty GObject
   */
  void AddObserver (GObject *observer);


 protected:
  

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Updates the GUI. 
   * PRE          :  /
   */
  BOOL OnMediaStream (OpalMediaStream &, BOOL);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Updates the internal RTP statistics. 
   * PRE          :  /
   */
  void UpdateRTPStats (PTime,
		       const RTP_Session &);

  
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

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set (BOOL = TRUE) or get (BOOL = FALSE) the
   *                 audio playing and recording volumes for the
   *                 audio device.
   * PRE          :  /
   */
  BOOL DeviceVolume (PSoundChannel *, BOOL, BOOL, unsigned int &);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Check if the listening port is accessible from the
   *                 outside and returns the test result given by seconix.com.
   * PRE          :  /
   */
  PString CheckTCPPorts ();
  
  
  /* DESCRIPTION  :  Notifier called after 30 seconds of no media.
   * BEHAVIOR     :  Clear the calls.
   * PRE          :  /
   */
  PDECLARE_NOTIFIER(PTimer, GMEndPoint, OnNoIncomingMediaTimeout);

  
  /* DESCRIPTION  :  Notifier called periodically to update details on ILS.
   * BEHAVIOR     :  Register, unregister the user from ILS or udpate his
   *                 personal data using the GMILSClient (XDAP).
   * PRE          :  /
   */
  PDECLARE_NOTIFIER(PTimer, GMEndPoint, OnILSTimeout);


  /* DESCRIPTION  :  Notifier called periodically during calls.
   * BEHAVIOR     :  Refresh the statistics window of the Control Panel
   *                 if it is currently shown.
   * PRE          :  /
   */
  PDECLARE_NOTIFIER(PTimer, GMEndPoint, OnRTPTimeout);


  /* DESCRIPTION  :  Notifier called periodically to update the gateway IP.
   * BEHAVIOR     :  Update the gateway IP to use for the IP translation
   *                 if IP Checking is enabled in the config database.
   * PRE          :  /
   */
  PDECLARE_NOTIFIER(PTimer, GMEndPoint, OnGatewayIPTimeout);

  
  /* DESCRIPTION  :  Notifier called every 2 seconds while waiting for an
   *                 answer for an outging call.
   * BEHAVIOR     :  Display an animation in the main winow and play a ring
   *                 sound.
   * PRE          :  /
   */
  PDECLARE_NOTIFIER(PTimer, GMEndPoint, OnOutgoingCall);
  

  GtkWidget *audio_transmission_popup;
  GtkWidget *audio_reception_popup;
  
  PString called_address;
  PString current_call_token;
  PString transfer_call_token;

  CallingState calling_state; 
  GObject *dispatcher;

  PTimer ILSTimer;
  PTimer RTPTimer;
  PTimer GatewayIPTimer;
  PTimer OutgoingCallTimer;
  PTimer NoIncomingMediaTimer;
    
  BOOL ils_registered;


  int missed_calls;
  

  /* Different channels */
  BOOL is_transmitting_video;
  BOOL is_transmitting_audio;
  BOOL is_receiving_video;
  BOOL is_receiving_audio;  


  /* RTP tats */
  struct RTP_SessionStats {
    
    void Reset () 
      { 
	re_a_bytes = 
	  re_v_bytes = 
	  tr_a_bytes = 
	  tr_v_bytes =
	  jitter_buffer_size = 0;
	  
	a_re_bandwidth = 
	  v_re_bandwidth = 
	  a_tr_bandwidth = 
	  v_tr_bandwidth =
	  lost_packets = 
	  out_of_order_packets =
	  late_packets = 
	  total_packets = 0.0; 
      }

    int re_a_bytes; 
    int re_v_bytes;
    int tr_a_bytes;
    int tr_v_bytes;
    
    float a_re_bandwidth;
    float v_re_bandwidth;
    float a_tr_bandwidth;
    float v_tr_bandwidth;
    
    float lost_packets;
    float out_of_order_packets;
    float late_packets;
    float total_packets;

    int jitter_buffer_size;

    PTime last_tick;
    PTime start_time;
  };
  RTP_SessionStats stats;

  
  /* The various related endpoints */
  GMH323EndPoint *h323EP;
  GMPCSSEndPoint *pcssEP;


  /* The various components of the endpoint */
  GMVideoGrabber *video_grabber;
  GMH323Gatekeeper *gk;
  GMStunClient *sc;
  GMILSClient *ils_client;
  PThread *audio_tester;


  /* Various mutexes to ensure thread safeness around internal
     variables */
  PMutex vg_access_mutex;
  PMutex ils_access_mutex;
  PMutex cs_access_mutex;
  PMutex ct_access_mutex;
  PMutex tct_access_mutex;
  PMutex lid_access_mutex;
  PMutex at_access_mutex;
  PMutex lca_access_mutex;
  PMutex mc_access_mutex;
  PMutex rc_access_mutex;
  PMutex stats_access_mutex;

  
#ifdef HAS_HOWL
  GMZeroconfPublisher *zcp;
  PMutex zcp_access_mutex;
#endif

#ifdef HAS_IXJ
  GMLid *lid;
#endif
};

#endif
