
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
 *                         sipendpoint.h  -  description
 *                         -----------------------------
 *   begin                : Wed 24 Nov 2004
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains the SIP Endpoint class.
 *
 */


#ifndef _SIP_ENDPOINT_H_
#define _SIP_ENDPOINT_H_


#include "../config.h"

#include "common.h"
#include "endpoint.h"
#include "sipregistrar.h"


/* Minimal SIP endpoint implementation */
class GMSIPEndPoint : public SIPEndPoint
{
  PCLASSINFO(GMSIPEndPoint, SIPEndPoint);

 public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Creates the H.323 EndPoint 
   * 		     and initialises the variables
   * PRE          :  /
   */
  GMSIPEndPoint (GMEndPoint &);

  
  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMSIPEndPoint ();
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Init the endpoint internal values and the various
   *                 components.
   * PRE          :  /
   */
  void Init ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Register to (or unregister from) the gatekeeper 
   *                 given in the options, if any. Starts or stop the force
   *                 renewal timer.
   * PRE          :  /
   */
  void RegistrarRegister (void);
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Starts the listener thread on the port choosen 
   *                 in the options.
   *                 returns TRUE if success and FALSE in case of error.
   * PRE          :  /
   */
  BOOL StartListener ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the local user name following the firstname and last 
   *                 name stored by the conf, set the gatekeeper alias, 
   *                 possibly as first alias.
   * PRE          :  /
   */
  void SetUserNameAndAlias ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Called regularly when new RTP statistics are available.
   * 		     Updates the main endpoint internal statistics structure
   * 		     so that it can be used periodically to refresh the stats
   * 		     window in the main UI.
   * PRE          :  /
   */
  void OnRTPStatistics (const SIPConnection &,
			const RTP_Session &) const;

  
  /* DESCRIPTION  :  Called when the registration is successfull. 
   * BEHAVIOR     :  Displays a message in the status bar and history. 
   * PRE          :  /
   */
  void OnRegistered (PString,
		     BOOL);
  
  
  /* DESCRIPTION  :  Called when the registration fails.
   * BEHAVIOR     :  Displays a message in the status bar and history. 
   * PRE          :  /
   */
  void OnRegistrationFailed (PString,
			     SIPEndPoint::RegistrationFailReasons,
			     BOOL);
  
  
  /* DESCRIPTION  :  Called when there is an incoming call.
   * BEHAVIOR     :  Calls the Manage function of the same name of forward
   * 		     the connection.
   * PRE          :  /
   */
  BOOL OnIncomingConnection (OpalConnection &);

  
 private:

  GMEndPoint & endpoint;
  GMSIPRegistrar *registrar;
};

#endif
