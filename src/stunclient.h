
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
 *                         stunclient.h  -  description
 *                         ----------------------------
 *   begin                : Thu Sep 30 2004
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Multithreaded class for the stun client.
 *
 */


#ifndef _STUNCLIENT_H_
#define _STUNCLIENT_H_

#include "common.h"

class GMEndPoint;

class GMStunClient : public PThread
{
  PCLASSINFO(GMStunClient, PThread);


public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Initialise the parameters.
   * PRE          :  if BOOL is FALSE, then only a detection of the NAT type
   * 		     is done, if not, then the GMH323EndPoint is also updated.
   * 		     The second parameter indicates if a progress dialog
   * 		     should be displayed or not.
   * 		     The third one will ask the user if he wants to enable
   * 		     STUN or not.
   * 		     The fourth one is the parent window if any. A parent
   * 		     window must be provided if parameters 2 or 3 are TRUE.
   * 		     The last parameter is a reference to the GMEndPoint.
   */
  GMStunClient (BOOL,
		BOOL,
		BOOL,
		GtkWidget *,
		GMEndPoint &);


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMStunClient ();


  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Set the appropriate STUN server on the endpoint and 
   * 		     detect the NAT type.
   *                 Uses parameters in config. 
   * PRE          :  /
   */
  void Main ();
  

protected:

  PSyncPoint thread_sync_point;

  BOOL update_endpoint;
  BOOL display_progress;
  BOOL display_config_dialog;

  PString stun_host;
  PString nat_type;

  GtkWidget *parent;

  PMutex quit_mutex;

  GMEndPoint & ep;
};


#endif
