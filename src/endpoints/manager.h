
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

#include "accounts.h"

#include "stun.h"

#include "accountshandler.h"

#include "gmconf-bridge.h"
#include "runtime.h"
#include "contact-core.h"
#include "presence-core.h"
#include "call-core.h"
#include "call-manager.h"
#include "call.h"
#include "audiooutput-core.h"

#include <sigc++/sigc++.h>
#include <string>


class GMLid;
class GMH323Gatekeeper;
class GMH323Endpoint;
class GMSIPEndpoint;
class GMPCSSEndpoint;


PDICTIONARY (mwiDict, PString, PString);


/**
 * COMMON NOTICE: The Endpoint must be initialized with Init after its
 * creation.
 */

class GMManager: 
    public Ekiga::Service,
    public OpalManager
{
  PCLASSINFO(GMManager, OpalManager);

  friend class GMAccountsEndpoint;
  friend class GMH323Endpoint;
  friend class GMSIPEndpoint;
  
  
 public:

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

  /**/
  void set_fullname (const std::string name);
  const std::string get_fullname () const;

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Prepare the endpoint to exit by removing all
   * 		     associated threads and components.
   * PRE          :  /
   */
  void Exit ();

  OpalCall *CreateCall ();

  
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


  /** Return the list of available codecs
   * @return a set of the codecs and their descriptions
   */
  Ekiga::CodecList get_codecs ();
  

  /** Enable the given codecs
   * @param codecs is a set of the codecs and their descriptions
   */
  void set_codecs (Ekiga::CodecList & codecs); 
  

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

 private:
  void GetAllowedFormats (OpalMediaFormatList & full_list);

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


  Ekiga::ServiceCore & core;
  Ekiga::Runtime & runtime;
  Ekiga::ConfBridge *bridge;
  Ekiga::CodecList codecs; 
  Ekiga::CallCore *call_core;
  Ekiga::AudioOutputCore & audiooutput_core;
};

#endif
