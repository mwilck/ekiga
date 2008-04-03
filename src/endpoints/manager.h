
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
 *                         endpoint.h  -  description
 *                         --------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the Endpoint class.
 *
 */


#ifndef _ENDPOINT_H_
#define _ENDPOINT_H_

#include "config.h"

#include "common.h"

#ifdef HAVE_AVAHI
#include "avahi.h"
#endif

#include "accounts.h"

#include "stun.h"

#include "accountshandler.h"

#include "gmconf-bridge.h"
#include "runtime.h"
#include "contact-core.h"
#include "presence-core.h"
#include "call-manager.h"
#include "call-core.h"
#include "call.h"
#include "audiooutput-core.h"

#include <sigc++/sigc++.h>
#include <string>


class GMLid;
class GMH323Gatekeeper;
class GMPCSSEndpoint;
class GMH323Endpoint;
class GMSIPEndpoint;


PDICTIONARY (mwiDict, PString, PString);


/**
 * COMMON NOTICE: The Endpoint must be initialized with Init after its
 * creation.
 */

class GMManager: 
    public OpalManager,
    public Ekiga::Service,
    public Ekiga::ContactDecorator,
    public Ekiga::PresentityDecorator,
    public Ekiga::CallManager
{
  PCLASSINFO(GMManager, OpalManager);

  friend class GMAccountsEndpoint;
  friend class GMPCSSEndpoint;
  friend class GMH323Endpoint;
  friend class GMSIPEndpoint;
  
  
 public:

  enum CallingState {Standby, Calling, Connected, Called};

  
  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Creates the supported endpoints 
   * 		     and initialises the variables
   * PRE          :  /
   */
  GMManager (Ekiga::ServiceCore & _core);


  /* DESCRIPTION  :  The destructor
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMManager ();

  /**/
  const std::string get_name () const
    { return "opal-component"; }

  const std::string get_description () const
    { return "\tObject bringing in Opal support (calls, text messaging, sip, h323, ...)"; }

  bool populate_menu (Ekiga::Contact &contact,
                      Ekiga::MenuBuilder &builder);

  bool populate_menu (const std::string uri,
                      Ekiga::MenuBuilder & builder);

  bool menu_builder_add_actions (const std::string & fullname,
                                 std::map<std::string, std::string> & uris,
                                 Ekiga::MenuBuilder & builder);

  void get_jitter_buffer_size (unsigned & min_val,
                               unsigned & max_val);
  void set_jitter_buffer_size (unsigned min_val,
                               unsigned max_val);

  bool get_silence_detection ();
  void set_silence_detection (bool enabled);

  bool get_echo_cancelation ();
  void set_echo_cancelation (bool enabled);

  void get_port_ranges (unsigned & min_udp_port, 
                        unsigned & max_udp_port,
                        unsigned & min_tcp_port, 
                        unsigned & max_tcp_port,
                        unsigned & min_rtp_port, 
                        unsigned & max_rtp_port);
  void set_port_ranges (unsigned min_udp_port, 
                        unsigned max_udp_port,
                        unsigned min_tcp_port, 
                        unsigned max_tcp_port,
                        unsigned min_rtp_port, 
                        unsigned max_rtp_port);

  struct VideoOptions 
    {
      VideoOptions () 
        : size (0), 
        maximum_frame_rate (0), 
        temporal_spatial_tradeoff (0), 
        maximum_received_bitrate (0), 
        maximum_transmitted_bitrate (0) {};

      unsigned size;
      unsigned maximum_frame_rate;
      unsigned temporal_spatial_tradeoff;
      unsigned maximum_received_bitrate;
      unsigned maximum_transmitted_bitrate;
    };

  void set_video_options (const VideoOptions & option);

  void get_video_options (VideoOptions & option);


  /**/
  bool dial (const std::string uri); 

  bool send_message (const std::string uri, 
                     const std::string message);

  /**/
  void set_fullname (const std::string name);
  const std::string get_fullname () const;

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Prepare the endpoint to exit by removing all
   * 		     associated threads and components.
   * PRE          :  /
   */
  void Exit ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Init the endpoint internal values and the various
   *                 components.
   * PRE          :  /
   */
  void Init ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Reset the listeners for all components. Displays an
   * 		     error dialog if it fails. The interface and ports
   * 		     are retrieved from the GmConf database.
   * 		     Remove old listeners if any.
   * PRE          :  / 
   */
  void ResetListeners ();
  
  

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Accepts the current incoming call, if any.
   * 		     There is always only one current incoming call at a time.
   * 		     If there is another incoming call in-between, 
   * 		     it is rejected.
   * PRE          :  /
   */
  bool AcceptCurrentIncomingCall ();

  

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the local or remote OpalConnection for the 
   * 		     given call. If there are several remote connections,
   * 		     the first one is returned.
   * PRE          :  /
   */
  PSafePtr<OpalConnection> GetConnection (PSafePtr<OpalCall>, 
					  bool);


  OpalCall *CreateCall ();

  
  /* DESCRIPTION  :  Called when there is an incoming SIP/H323/PCSS connection.
   * BEHAVIOR     :  Updates the GUI and forward, reject the connection
   * 		     if required. If the connection is not forwarded, then
   * 		     OnShowIncoming will be called on the PCSS Endpoint.
   * PRE          :  /
   */
  bool OnIncomingConnection (OpalConnection &,
			     unsigned,
			     const std::string & forward_uri);

  
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


  /* DESCRIPTION  :  Called when a message has been received.
   * BEHAVIOR     :  Updates the text chat window, updates the tray icon in
   * 		     flashing state if the text chat window is hidden.
   * PRE          :  /
   */
  void OnMessageReceived (const SIPURL & from,
			  const PString & body);

  void OnMessageFailed (const SIPURL & messageUrl,
                        SIP_PDU::StatusCodes reason);

  void OnMessageSent (const PString & to,
                      const PString & body);


  /** Return the list of available codecs
   * @return a set of the codecs and their descriptions
   */
  Ekiga::CodecList get_codecs ();
  

  /** Enable the given codecs
   * @param codecs is a set of the codecs and their descriptions
   */
  void set_codecs (Ekiga::CodecList & codecs); 
  

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Sets the User Input Mode following the
   *                 configuration options for each of the endpoints. 
   * PRE          :  /
   */
  void SetUserInputMode ();

  
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
  GMH323Endpoint *GetH323Endpoint ();
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the SIP endpoint.
   * PRE          :  /
   */
  GMSIPEndpoint *GetSIPEndpoint ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the PCSS endpoint.
   * PRE          :  /
   */
  GMPCSSEndpoint *GetPCSSEndpoint ();
  

  /* DESCRIPTION  : /
   * BEHAVIOR     : Return the current listener address of the endpoint. 
   * 		    for the given protocol. (default sip).
   * PRE          : /
   */
  PString GetCurrentAddress (PString protocol = PString::Empty ());
  
  
  /* DESCRIPTION  : /
   * BEHAVIOR     : Returns the default url for the given protocol (if any).
   * 		    The returned url is the best guess. It is in general more
   * 		    accurate with SIP than with H.323. If there is no default
   * 		    account configured and enabled, the IP address is returned.
   * PRE          : Non-empty protocol.
   */
  PString GetURL (PString);


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


#ifdef HAVE_AVAHI
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Create a zeroconf client that will publish information
   *                 about Ekiga.
   * PRE          :  /
   */
  void CreateZeroconfClient ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Remove any running Zeroconf client.
   * PRE          :  /
   */
  void RemoveZeroconfClient ();
#endif

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Create a STUN client.
   * PRE          :  First parameter : TRUE if a progress dialog should be
   * 		                       displayed.
   * 		     Second parameter: TRUE if a config dialog should be
   * 		                       displayed to ask the user to use
   * 		                       STUN or not.
   * 		     Third parameter : TRUE if should wait for the result
   * 		     		       before returning.
   * 		     Fourth parameter: Parent window for the other dialogs.
   */
  void CreateSTUNClient (bool,
			 bool,
			 bool,
			 GtkWidget *);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Remove the STUN client.
   * PRE          :  /
   */
  void RemoveSTUNClient ();

  
  /* DESCRIPTION :  /
   * BEHAVIOR    :  Update the various registrations (ILS, zeroconf, etc)
   * PRE         :  /
   */
  void UpdatePublishers (void);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Register (or unregister) all accounts or the one provided.
   * 		     It launches an accounts manager to register the given
   * 		     account or all accounts.
   * PRE          :  /
   */
  void Register (GmAccount * = NULL);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Remove the account manager.
   * PRE          :  /
   */
  void RemoveAccountsEndpoint ();
  

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  TRUE if the video should automatically be transmitted
   *                 when a call begins.
   * PRE          :  /
   */
  void SetAutoStartTransmitVideo (bool a) {autoStartTransmitVideo = a;}


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  TRUE if the video should automatically be received
   *                 when a call begins.
   * PRE          :  /
   */
  void SetAutoStartReceiveVideo (bool a) {autoStartReceiveVideo = a;}

  
  /* DESCRIPTION  :  Callback called when OpenH323 opens a new logical channel
   * BEHAVIOR     :  Updates the log window with information about it, returns
   *                 FALSE if error, TRUE if OK
   * PRE          :  /
   */
  virtual bool OnOpenMediaStream (OpalConnection &,
				  OpalMediaStream &);


  /* DESCRIPTION  :  Callback called when OpenH323 closes a new logical channel
   * BEHAVIOR     :  Close the channel and update the GUI..
   * PRE          :  /
   */
  virtual void OnClosedMediaStream (const OpalMediaStream &);


  sigc::signal<void, std::string, std::string, unsigned int> mwi_event;

 private:
  void on_dial (std::string uri);

  void on_message (std::string name,
                   std::string uri);

  
 protected:
  
  
  void OnMWIReceived (const PString & account, 
                      const PString & mwi);

  void OnRegistering (const PString & aor,
                      bool isRegistering);

  void OnRegistered (const PString & aor,
                     bool wasRegistering);

  void OnRegistrationFailed (const PString & aor,
                             bool wasRegistering,
                             std::string info);

  void OnHold (OpalConnection & connection);

  bool DeviceVolume (PSoundChannel *, bool, bool, unsigned int &);

 public:

  /* DESCRIPTION  :  Notifier called every minute to check for IP changes.
   * BEHAVIOR     :  Update the listeners. 
   * PRE          :  /
   */
  PDECLARE_NOTIFIER(PTimer, GMManager, OnIPChanged);

  /* DESCRIPTION  :  Notifier called periodically to update the gateway IP.
   * BEHAVIOR     :  Update the gateway IP to use for the IP translation
   *                 if IP Checking is enabled in the config database.
   * PRE          :  /
   */
  PDECLARE_NOTIFIER(PTimer, GMManager, OnGatewayIPTimeout);  

 private:
  void GetAllowedFormats (OpalMediaFormatList & full_list);

  PString current_call_token;

  CallingState calling_state; 

  PTimer GatewayIPTimer;
  PTimer IPChangedTimer;
    
  /* MWI */
  mwiDict mwiData;

  /* Different channels */
  bool is_transmitting_video;
  bool is_transmitting_audio;
  bool is_receiving_video;
  bool is_receiving_audio;  


  /* The various related endpoints */
  GMH323Endpoint *h323EP;
  GMSIPEndpoint *sipEP;
  GMPCSSEndpoint *pcssEP;


  /* The various components of the endpoint */
  GMAccountsEndpoint *manager;
  GMH323Gatekeeper *gk;
  GMStunClient *sc;


  /* Various mutexes to ensure thread safeness around internal
     variables */
  PMutex vg_access_mutex;
  PMutex cs_access_mutex;
  PMutex ct_access_mutex;
  PMutex tct_access_mutex;
  PMutex lid_access_mutex;
  PMutex mwi_access_mutex;
  PMutex rc_access_mutex;
  PMutex manager_access_mutex;
  PMutex sc_mutex;

  
  /* Used codecs */
  PString re_audio_codec;
  PString tr_audio_codec;
  PString re_video_codec;
  PString tr_video_codec;


#ifdef HAVE_AVAHI
  GMZeroconfPublisher *zcp;
  PMutex zcp_access_mutex;
#endif

  Ekiga::ServiceCore & core;
  Ekiga::Runtime & runtime;
  Ekiga::ConfBridge *bridge;
  Ekiga::CodecList codecs; 
  Ekiga::CallCore *call_core;
  Ekiga::AudioOutputCore & audiooutput_core;
};

#endif
