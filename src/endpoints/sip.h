
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


PDICTIONARY (msgDict, PString, PString);


class Ekiga::PersonalDetails;


/* Minimal SIP endpoint implementation */
class GMSIPEndpoint 
:   public SIPEndPoint, 
    public Ekiga::PresenceFetcher,
    public Ekiga::PresencePublisher
{
  PCLASSINFO(GMSIPEndpoint, SIPEndPoint);

 public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Creates the H.323 Endpoint 
   * 		     and initialises the variables
   * PRE          :  /
   */
  GMSIPEndpoint (GMManager &ep, Ekiga::ServiceCore & core);

  
  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMSIPEndpoint ();
  
  /***/
  void fetch (const std::string uri);
  void unfetch (const std::string uri);
  void publish (const Ekiga::PersonalDetails & details);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Starts the listener thread on the port choosen 
   *                 in the options after having removed old listeners.
   *                 returns TRUE if success and FALSE in case of error.
   * PRE          :  The interface.
   */
  bool StartListener (PString iface,
		      WORD port);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the local user name following the full 
   *                 name stored by the conf.
   * PRE          :  /
   */
  void SetUserNameAndAlias ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Adds the User Input Mode following the
   *                 configuration options. 
   * PRE          :  /
   */
  void SetUserInputMode (unsigned int mode);

  
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
  
  /* DESCRIPTION  :  Notifier called when an incoming call
   *                 has not been answered in the required time.
   * BEHAVIOR     :  Reject the call, or forward if forward on no answer is
   *                 enabled in the config database.
   * PRE          :  /
   */
  PDECLARE_NOTIFIER(PTimer, GMSIPEndpoint, OnNoAnswerTimeout);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Init the endpoint internal values and the various
   *                 components.
   * PRE          :  /
   */
  void Init ();


  PTimer NoAnswerTimer;

  GMManager & endpoint;

  PMutex msgDataMutex;
  msgDict msgData;

  std::list<std::string> uris;    // List of subscribed uris
  std::list<std::string> domains; // List of registered domains
  std::list<std::string> aors;     // List of registered aor
  Ekiga::ServiceCore & core;

  std::string uri_prefix;
};
#endif
