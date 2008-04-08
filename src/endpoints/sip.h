
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
 *                         sipendpoint.h  -  description
 *                         -----------------------------
 *   begin                : Wed 24 Nov 2004
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the SIP Endpoint class.
 *
 */


#ifndef _SIP_ENDPOINT_H_
#define _SIP_ENDPOINT_H_


#include "config.h"

#include "common.h"

#include "manager.h"
#include "presence-core.h"
#include "call-manager.h"


PDICTIONARY (msgDict, PString, PString);


class Ekiga::PersonalDetails;


class GMSIPEndpoint 
:   public SIPEndPoint, 
    public Ekiga::PresenceFetcher,
    public Ekiga::PresencePublisher,
    public Ekiga::PresentityDecorator,
    public Ekiga::ContactDecorator,
    public Ekiga::CallManager
{
  PCLASSINFO(GMSIPEndpoint, SIPEndPoint);

public:

  GMSIPEndpoint (GMManager &ep, Ekiga::ServiceCore & core);

  ~GMSIPEndpoint ();

  /**/
  bool dial (const std::string uri); 

  bool send_message (const std::string uri, 
                     const std::string message);

  Ekiga::CodecList get_codecs ();

  void set_codecs (Ekiga::CodecList & codecs); 

  /***/
  bool populate_menu (Ekiga::Contact &contact,
                      Ekiga::MenuBuilder &builder);

  bool populate_menu (const std::string uri,
                      Ekiga::MenuBuilder & builder);

  bool menu_builder_add_actions (const std::string & fullname,
                                 std::map<std::string, std::string> & uris,
                                 Ekiga::MenuBuilder & builder);
  /***/

  /***/
  void fetch (const std::string uri);
  void unfetch (const std::string uri);
  void publish (const Ekiga::PersonalDetails & details);

  /***/
  void set_outbound_proxy (const std::string & uri);
  void set_dtmf_mode (unsigned int mode);
  void set_nat_binding_delay (unsigned int delay);

  /***/
  /* TODO: 
   * It is probably needed to move some of those functions
   * in the core
   */
  bool start_listening ();
  bool set_udp_ports (const unsigned min, const unsigned max);
  bool set_listen_port (const unsigned listen);
  void set_forward_host (const std::string & uri);
  void set_forward_on_busy (const bool enabled);
  void set_unconditional_forward (const bool enabled);
  void set_forward_on_no_answer (const bool enabled);
  void set_no_answer_timeout (const unsigned timeout);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Register the SIP endpoint to the given SIP server. 
   * PRE          :  Correct parameters.
   */
  void Register (const PString & aor,
                 const PString & authUserName,
                 const PString & password,
                 unsigned int expires,
                 bool unregister);


  /* DESCRIPTION  :  Called when the registration is successful. 
   * BEHAVIOR     :  Displays a message in the status bar and history. 
   * PRE          :  /
   */
  void OnRegistered (const PString & aor,
                     bool wasRegistering);


  /* DESCRIPTION  :  Called when the registration fails.
   * BEHAVIOR     :  Displays a message in the status bar and history. 
   * PRE          :  /
   */
  void OnRegistrationFailed (const PString & aor,
                             SIP_PDU::StatusCodes reason,
                             bool wasRegistering);

  bool MakeConnection(OpalCall & call,
                      const PString & party,
                      void * userData = NULL,
                      unsigned int options = 0,
                      OpalConnection::StringOptions *stringOptions = NULL);


  /* DESCRIPTION  :  Called when there is an incoming SIP connection.
   * BEHAVIOR     :  Checks if the connection must be rejected or forwarded
   * 		     and call the manager function of the same name
   * 		     to update the GUI and take the appropriate action
   * 		     on the connection. If the connection is not forwarded,
   * 		     or rejected, OnShowIncoming will be called on the PCSS
   * 		     endpoint, allowing to auto-answer the call or do further
   * 		     updates of the GUI and internal timers.
   * PRE          :  /
   */
  bool OnIncomingConnection (OpalConnection &connection,
                             unsigned options,
                             OpalConnection::StringOptions * stroptions);


  /* DESCRIPTION  :  Called when there is a MWI.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  void OnMWIReceived (const PString & to,
                      SIPSubscribe::MWIType type,
                      const PString & msgs);


  /* DESCRIPTION  :  Called when presence information has been received.
   * BEHAVIOR     :  Updates the roster.
   * PRE          :  /
   */
  virtual void OnPresenceInfoReceived (const PString & user,
                                       const PString & basic,
                                       const PString & note);


  /* DESCRIPTION  :  Called when a message has been received.
   * BEHAVIOR     :  Checks if we already received the message and call
   * 		     OnMessageReceived.
   * PRE          :  /
   */
  virtual void OnReceivedMESSAGE (OpalTransport & transport,
                                  SIP_PDU & pdu);


  /* DESCRIPTION  :  Called when sending a message fails. 
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  void OnMessageFailed (const SIPURL & messageUrl,
                        SIP_PDU::StatusCodes reason);

  void Message (const PString & to,
                const PString & body);


  /* DESCRIPTION  :  / 
   * BEHAVIOR     :  Returns the account to use for outgoing PDU's.
   * PRE          :  /
   */
  SIPURL GetRegisteredPartyName (const SIPURL & host);


  /* DESCRIPTION  :  This callback is called when the connection is 
   *                 established and everything is ok.
   * BEHAVIOR     :  Stops the timers.
   * PRE          :  /
   */
  void OnEstablished (OpalConnection &);


  /* DESCRIPTION  :  This callback is called when a connection to a remote
   *                 endpoint is cleared.
   * BEHAVIOR     :  Stops the timers.
   * PRE          :  /
   */
  void OnReleased (OpalConnection &);


private:
  void on_dial (std::string uri);

  void on_message (std::string name,
                   std::string uri);


  GMManager & endpoint;

  PMutex msgDataMutex;
  msgDict msgData;

  std::list<std::string> subscribed_uris;    // List of subscribed uris
  std::list<std::string> domains; // List of registered domains
  std::list<std::string> aors;     // List of registered aor
  Ekiga::ServiceCore & core;
  Ekiga::Runtime & runtime;

  std::string uri_prefix;
  std::string forward_uri;

  bool forward_on_busy;
  bool unconditional_forward;
  bool forward_on_no_answer;
  unsigned no_answer_timeout;
  unsigned udp_min;
  unsigned udp_max;
  unsigned listen_port;
};
#endif
