
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


#include "../../config.h"

#include "common.h"

#include "manager.h"


PDICTIONARY (msgDict, PString, PString);


/* Minimal SIP endpoint implementation */
class GMSIPEndpoint : public SIPEndPoint
{
  PCLASSINFO(GMSIPEndpoint, SIPEndPoint);

 public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Creates the H.323 Endpoint 
   * 		     and initialises the variables
   * PRE          :  /
   */
  GMSIPEndpoint (GMManager &);

  
  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMSIPEndpoint ();
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Init the endpoint internal values and the various
   *                 components.
   * PRE          :  /
   */
  void Init ();
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Starts the listener thread on the port choosen 
   *                 in the options after having removed old listeners.
   *                 returns TRUE if success and FALSE in case of error.
   * PRE          :  The interface.
   */
  BOOL StartListener (PString,
		      WORD);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the local user name following the firstname and last 
   *                 name stored by the conf, set the gatekeeper alias, 
   *                 possibly as first alias.
   * PRE          :  /
   */
  void SetUserNameAndAlias ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Adds the User Input Mode following the
   *                 configuration options. Only RFC2833 is supported
   *                 for now.
   * PRE          :  /
   */
  void SetUserInputMode ();
  
  
  /* DESCRIPTION  :  Called when the registration is successful. 
   * BEHAVIOR     :  Displays a message in the status bar and history. 
   * PRE          :  /
   */
  void OnRegistered (const PString &,
		     const PString &,
		     BOOL);
  
  
  /* DESCRIPTION  :  Called when the registration fails.
   * BEHAVIOR     :  Displays a message in the status bar and history. 
   * PRE          :  /
   */
  void OnRegistrationFailed (const PString &,
			     const PString &,
			     SIP_PDU::StatusCodes reason,
			     BOOL);
  
  
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
  BOOL OnIncomingConnection (OpalConnection &);


  /* DESCRIPTION  :  Called when there is a MWI.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  void OnMWIReceived (const PString & remoteAddress,
		      const PString & user,
		      SIPMWISubscribe::MWIType type,
		      const PString & msgs);

  
  /* DESCRIPTION  :  Called when a message has been received.
   * BEHAVIOR     :  Updates the text chat window, updates the tray icon in
   * 		     flashing state if the text chat window is hidden.
   * PRE          :  /
   */
  void OnMessageReceived (const SIPURL & from,
			  const PString & body);

  
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
      

  /* DESCRIPTION  :  / 
   * BEHAVIOR     :  Returns the number of registered accounts.
   * PRE          :  /
   */
  int GetRegisteredAccounts ();

  
 private:

  GMManager & endpoint;

  msgDict msgData;
};

#endif
