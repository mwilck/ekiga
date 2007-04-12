
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
 *                         stunclient.h  -  description
 *                         ----------------------------
 *   begin                : Thu Sep 30 2004
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Multithreaded class for the stun client.
 *
 */


#ifndef _STUNCLIENT_H_
#define _STUNCLIENT_H_

#include "common.h"

class GMManager;

class GMStunClient : public PThread
{
  PCLASSINFO(GMStunClient, PThread);


public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Initialise the parameters.
   * PRE          :  The first parameter indicates if a progress dialog
   * 		     should be displayed or not.
   * 		     The second one will ask the user if he wants to enable
   * 		     STUN or not.
   * 		     The third one indicates if it should wait for the
   * 		     result before returning.
   * 		     The fourth one is the parent window if any. A parent
   * 		     window must be provided if parameters 2 or 3 are TRUE.
   * 		     The last parameter is a reference to the GMManager.
   */
  GMStunClient (BOOL display_progress_,
		BOOL display_config_dialog_,
		BOOL wait_,
		GtkWidget *parent_window,
		GMManager &endpoint);


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMStunClient ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the translated NAT name from its type number.
   * PRE          :  /
   */
  static PString GetNatName (int nat_type);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the appropriate STUN server on the endpoint and 
   * 		     detect the NAT type.
   *                 Uses parameters in config. 
   * PRE          :  /
   */
  void Main ();
  

protected:


  BOOL display_progress;
  BOOL display_config_dialog;
  BOOL wait;

  PString stun_host;
  PString nat_type;

  GtkWidget *parent;

  PMutex quit_mutex;
  PSyncPoint sync;

  GMManager & ep;
};


#endif
