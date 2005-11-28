
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

#ifdef HAS_AVAHI
#include "avahi_publisher.h"
#endif

#ifdef HAS_IXJ
#include <lids/ixjlid.h>
#endif


#include "accounts.h"

#include "gdkvideoio.h"
#include "videograbber.h"
#include "stunclient.h"

#include <contacts/gm_contacts.h>


class GMILSClient;
class GMLid;
class GMH323Gatekeeper;
class GMPCSSEndPoint;
class GMH323EndPoint;
class GMSIPEndPoint;


PDICTIONARY (mwiDict, PString, PString);


/**
 * COMMON NOTICE: The Endpoint must be initialized with Init after its
 * creation.
 */

class GMEndPoint : public OpalManager
{
  PCLASSINFO(GMEndPoint, OpalManager);

  friend class GMPCSSEndPoint;
  friend class GMH323EndPoint;
  friend class GMSIPEndPoint;
  
  
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
   * BEHAVIOR     :  Start the listeners for all components. Displays an
   * 		     error dialog if it fails. The interface and ports
   * 		     are retrieved from the GmConf database.
   * 		     Remove old listeners if any.
   * PRE          :  / 
   */
  void StartListeners ();
  
  
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


  /* DESCRIPTION  :  This callback is called when a call is forwarded.
   * BEHAVIOR     :  Outputs a message in the log and statusbar.
   * PRE          :  /
   */
  virtual BOOL OnForwarded (OpalConnection &,
			    const PString &);

  
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
   * 		     It will use the address book to try matching the full
   * 		     name.
   * PRE          :  /
   */
  void GetRemoteConnectionInfo (OpalConnection &,
				gchar * &,
				gchar * &,
				gchar * &);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the name and best guess for the URL to use when 
   *                 calling back the user (see GetRemoteConnectionInfo)
   *                 for the current active connection (if any).
   * PRE          :  /
   */
  void GetCurrentConnectionInfo (gchar *&,
				 gchar *&);
  
  
  /* DESCRIPTION  :  Called when there is an incoming SIP/H323/PCSS connection.
   * BEHAVIOR     :  Updates the GUI and forward, reject the connection
   * 		     if required. If the connection is not forwarded, then
   * 		     OnShowIncoming will be called on the PCSS Endpoint.
   * PRE          :  /
   */
  BOOL OnIncomingConnection (OpalConnection &,
			     int,
			     PString);

  
  /* DESCRIPTION  :  This callback is called when a call is established. 
   * BEHAVIOR     :  Updates the GUI to put it in the Established mode.
   * PRE          :  /
   */
  void OnEstablishedCall (OpalCall &);

  
  /* DESCRIPTION  :  This callback is called when the connection is 
   *                 established and everything is ok.
   * BEHAVIOR     :  Sets the proper values for the current connection 
   *                 parameters and updates the GUI.
   *                 Notice there are 2 connections for each call.
   * PRE          :  /
   */
  void OnEstablished (OpalConnection &);


  /* DESCRIPTION  :  This callback is called when a call is cleared. 
   * BEHAVIOR     :  Updates the GUI to put it back in the StandBy mode.
   * PRE          :  /
   */
  void OnClearedCall (OpalCall &);


  /* DESCRIPTION  :  This callback is called when a connection to a remote
   *                 endpoint is cleared.
   * BEHAVIOR     :  Sets the proper values for the current connection 
   *                 parameters, updates the calls history, and the GUI
   *                 to display the call end reason.
   *                 Notice there are 2 connections for each call.
   * PRE          :  /
   */
  void OnReleased (OpalConnection &);

  
  /* DESCRIPTION  :  This callback is called when a connection to a remote
   *                 endpoint is put on hold.
   * BEHAVIOR     :  Updates the GUI.
   * PRE          :  /
   */
  void OnHold (OpalConnection &);
  
  
  /* DESCRIPTION  :  This callback is called when receiving an input string.
   * BEHAVIOR     :  Updates the text chat, updates the tray icon in
   * 		     flashing state if the text chat window is hidden.
   * PRE          :  /
   */
  void OnUserInputString (OpalConnection &,
			  const PString &);
  
  
  /* DESCRIPTION  :  This callback is called when an input video device 
   *                 has to be opened.
   * BEHAVIOR     :  Initialise the PVideoInputDevice.
   * PRE          :  /
   */
  BOOL CreateVideoInputDevice (const OpalConnection &,
			       const OpalMediaFormat &,
			       PVideoInputDevice * &,
			       BOOL &);

  
  /* DESCRIPTION  :  This callback is called when an input video device 
   *                 has to be opened.
   * BEHAVIOR     :  Initialise the PVideoOutputDevice.
   * PRE          :  /
   */
  BOOL CreateVideoOutputDevice(const OpalConnection &,
			       const OpalMediaFormat &,
			       BOOL,
			       PVideoOutputDevice * &,
			       BOOL &);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Starts the listener thread on the port choosen 
   *                 in the options.
   *                 returns TRUE if success and FALSE in case of error.
   * PRE          :  /
   */
  BOOL StartListener ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the list of audio formats supported by
   * 		     the endpoint.
   * PRE          :  /
   */
  OpalMediaFormatList GetAvailableAudioMediaFormats ();
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Add all media formats following the user config.
   * PRE          :  /
   */
  void SetAllMediaFormats ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Add audio media formats following the user config.
   * PRE          :  /
   */
  void SetAudioMediaFormats ();
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Add video media formats following the user config.
   * PRE          :  /
   */
  void SetVideoMediaFormats ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Sets the User Input Mode following the
   *                 configuration options for each of the endpoints. 
   * PRE          :  /
   */
  void SetUserInputMode ();


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


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the H.323 endpoint.
   * PRE          :  /
   */
  GMH323EndPoint *GetH323EndPoint ();
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the SIP endpoint.
   * PRE          :  /
   */
  GMSIPEndPoint *GetSIPEndPoint ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the PCSS endpoint.
   * PRE          :  /
   */
  GMPCSSEndPoint *GetPCSSEndPoint ();
  

  /* DESCRIPTION  : /
   * BEHAVIOR     : Return the current IP of the endpoint 
   * 		    for the given protocol, even if the endpoint is 
   * 		    listening on many interfaces
   * PRE          : Non-empty protocol.
   */
  PString GetCurrentIP (PString);
  
  
  /* DESCRIPTION  : /
   * BEHAVIOR     : Returns the default url for the given protocol (if any).
   * 		    The returned url is the best guess. It is in general more
   * 		    accurate with SIP than with H.323. If there is no default
   * 		    account configured and enabled, the IP address is returned.
   * PRE          : Non-empty protocol.
   */
  PString GetURL (PString);
  

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

 
  /* DESCRIPTION :  /
   * BEHAVIOR    :  Update the various registrations (ILS, zeroconf, etc)
   * PRE         :  /
   */
  void UpdatePublishers (void);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Register (or unregister) all accounts or the one provided.
   * PRE          :  /
   */
  void Register (GmAccount * = NULL);


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
  virtual void OnClosedMediaStream (const OpalMediaStream &);


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
   * BEHAVIOR     :  Returns the current MWI (new/old) for a specific account.
   * PRE          :  host / user must be not be empty.
   */
  PString GetMWI (const PString &,
		  const PString &);
  

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the current MWI (new messages only) for all 
   * 		     accounts.
   * PRE          :  /
   */
  PString GetMWI ();
  
  
  /* DESCRIPTION  :  / 
   * BEHAVIOR     :  Returns the number of registered accounts.
   * PRE          :  /
   */
  int GetRegisteredAccounts ();
  
  
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
  
  
 protected:
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Add the current MWI to the given account.
   * PRE          :  host / user / value must not be empty.
   */
  void AddMWI (const PString &, 
	       const PString &, 
	       const PString &);

  
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
		       RTP_Session *,
		       RTP_Session *);

  
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
   *                 if it is currently shown. Update all status bars.
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

  PTimer ILSTimer;
  PTimer RTPTimer;
  PTimer GatewayIPTimer;
  PTimer OutgoingCallTimer;
  PTimer NoIncomingMediaTimer;
    
  BOOL ils_registered;


  /* Missed calls number and MWI */
  int missed_calls;
  mwiDict mwiData;
  

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
  GMSIPEndPoint *sipEP;
  GMPCSSEndPoint *pcssEP;


  /* The various components of the endpoint */
  GMAccountsManager *manager;
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
  PMutex mwi_access_mutex;
  PMutex rc_access_mutex;
  PMutex manager_access_mutex;

  
  /* Used codecs */
  PString re_audio_codec;
  PString tr_audio_codec;
  PString re_video_codec;
  PString tr_video_codec;

  
#if defined(HAS_HOWL) || defined(HAS_AVAHI)
  GMZeroconfPublisher *zcp;
  PMutex zcp_access_mutex;
#endif

#ifdef HAS_IXJ
  GMLid *lid;
#endif
};

#endif
