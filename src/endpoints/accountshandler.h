
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
 *                         accounts_manager.h  -  description
 *                         ----------------------------------
 *   begin                : Sun Feb 13 2005
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *   			    manipulate H.323/SIP/... provider accounts.
 */


#ifndef _ACCOUNTS_MANAGER_H_
#define _ACCOUNTS_MANAGER_H_

#include "common.h"
#include "accounts.h"

class GMManager;


/* Class to register accounts in a thread.
 * SIP Accounts are registered asynchronously, H.323 accounts
 * are registered synchronously.
 */
class GMAccountsEndpoint : public PThread
{
  PCLASSINFO(GMAccountsEndpoint, PThread);


public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Initialise the parameters and launches the registration
   * 		     thread. If an account is specified, only that account
   * 		     is updated and the thread exits.
   * 		     If no account is specified, the 
   * 		     GMAccountsEndpoint will refresh the state for all accounts
   * 		     specified in the GmConfig.
   * PRE          :  /
   */
  GMAccountsEndpoint (GmAccount *,
		     GMManager &);


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMAccountsEndpoint ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Updates the registration status for the given account
   * 		     or for all accounts. STUN is used if configured.
   * PRE          :  /
   */
  void Main ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Updates the registration for a SIP GmAccount.
   * PRE          :  A valid pointer to a valid SIP GmAccount.
   */
  void SIPRegister (GmAccount *);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Updates the registration for a H.323 GmAccount.
   * PRE          :  A valid pointer to a valid H.323 GmAccount.
   */
  void H323Register (GmAccount *);
  
protected:


  GmAccount *account;

  PMutex quit_mutex;
  PSyncPoint thread_sync_point;

  GMManager & ep;
};

#endif
