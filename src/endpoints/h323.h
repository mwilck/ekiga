
/* GnomeMeeting -- A Video-Conferencing application
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
 *                         h323endpoint.h  -  description
 *                         ------------------------------
 *   begin                : Wed 24 Nov 2004
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the H.323 Endpoint class.
 *
 */


#ifndef _H323_ENDPOINT_H_
#define _H323_ENDPOINT_H_


#include "../../config.h"

#include "common.h"

#include "manager.h"


/* Minimal H.323 endpoint implementation */
class GMH323Endpoint : public H323EndPoint
{
  PCLASSINFO(GMH323Endpoint, H323EndPoint);

 public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Creates the H.323 Endpoint 
   * 		     and initialises the variables
   * PRE          :  /
   */
  GMH323Endpoint (GMManager &);

  
  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMH323Endpoint ();
  
  
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
   *                 configuration options. String, Tone, and RFC2833 are 
   *                 supported for now.
   * PRE          :  /
   */
  void SetUserInputMode ();
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Use the given gatekeeper.
   * PRE          :  /
   */
  BOOL UseGatekeeper (const PString & address = PString::Empty (),
		      const PString & domain = PString::Empty (),
		      const PString & iface = PString::Empty ());
  

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Remove the given gatekeeper if we were registered to it.
   * 		     Returns TRUE if it worked.
   * PRE          :  Non-Empty address.
   */
  BOOL RemoveGatekeeper (const PString & address);
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns TRUE if we are registered with 
   * 		     the given gatekeeper.
   * PRE          :  Non-Empty address.
   */
  BOOL IsRegisteredWithGatekeeper (const PString & address);
  

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


  /* DESCRIPTION  :  Called when the gatekeeper accepts the registration.
   * BEHAVIOR     :  Update the endpoint state.
   * PRE          :  /
   */
  void OnRegistrationConfirm ();

  
  /* DESCRIPTION  :  Called when the gatekeeper rejects the registration.
   * BEHAVIOR     :  Update the endpoint state.
   * PRE          :  /
   */
  void OnRegistrationReject ();


 private:

  GMManager & endpoint;
  PMutex gk_name_mutex;
  PString gk_name;
};

#endif
