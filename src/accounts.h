
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
 * GnomeMeeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         accounts.h  -  description
 *                         --------------------------
 *   begin                : Sun Feb 13 2005
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *   			    manipulate H.323/SIP/... provider accounts.
 */


#ifndef _ACCOUNTS_H_
#define _ACCOUNTS_H_

#include "common.h"

/* An account is uniquely identified by its Account ID */
struct GmAccount_ {

  gchar *aid;			/* Account ID */
  gchar *account_name;		/* Account Name */
  gchar *protocol_name;		/* Protocol for the account */
  gchar *host;			/* Host to register to */
  gchar *domain;		/* Registration domain/realm */
  gchar *login;			/* Login */
  gchar *password;		/* Password */
  gboolean enabled;		/* Account active or not */
  int timeout;			/* Registration timeout */
  int method;			/* Registration method */
}; 

typedef GmAccount_ GmAccount;


#define GM_ACCOUNT(x)     (GmAccount *) (x)


/* The API */


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns an empty GmAccount. Only the UID field has a unique
 *                value.
 * PRE          : /
 */
GmAccount *gm_account_new ();


/* DESCRIPTION  : /
 * BEHAVIOR     : Deletes the given account and frees the associated memory.
 * PRE          : /
 */
void gm_account_delete (GmAccount *account);


/* DESCRIPTION  : /
 * BEHAVIOR     : Makes a copy of the given account, keeps the same ID.
 * PRE          : /
 */
GmAccount *gm_account_copy (GmAccount *account);


/* DESCRIPTION  : /
 * BEHAVIOR     : Adds the given account to the accounts list. The given 
 * 		  account and its ID must not exist yet in the list.
 * 		  Returns TRUE on success, FALSE on failure.
 * PRE          : /
 */
gboolean gnomemeeting_account_add (GmAccount *account);


/* DESCRIPTION  : /
 * BEHAVIOR     : Deletes the given account from the accounts list. 
 * 		  Returns TRUE on success, FALSE on failure.
 * PRE          : /
 */
gboolean gnomemeeting_account_delete (GmAccount *account);


/* DESCRIPTION  : /
 * BEHAVIOR     : Modifies the given account in the accounts list. 
 * 		  Returns TRUE on success, FALSE on failure.
 * PRE          : /
 */
gboolean gnomemeeting_account_modify (GmAccount *account);


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns the list of configured GmAccounts.
 * PRE          : /
 */
GSList *gnomemeeting_get_accounts_list ();


/* DESCRIPTION  : /
 * BEHAVIOR     : Toggles the active state of the given account.
 * 		  Returns TRUE on success, FALSE on failure.
 * PRE          : /
 */
gboolean gnomemeeting_account_toggle_active (GmAccount *account);


/* Class to register accounts in a thread.
 * SIP Accounts are registered asynchronously, H.323 accounts
 * are registered synchronously.
 */
class GMAccountsManager : public PThread
{
  PCLASSINFO(GMAccountsManager, PThread);


public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Initialise the parameters and launches the registration
   * 		     thread. If an account is specified, only that account
   * 		     is updated and the thread exits.
   * 		     If no account is specified, the 
   * 		     GMAccountsManager will refresh the state for all accounts
   * 		     specified in the GmConfig.
   * PRE          :  /
   */
  GMAccountsManager (GmAccount *);


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMAccountsManager ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Updates the registration status for the given account
   * 		     or for all accounts.
   * PRE          :  /
   */
  void Main ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Updates the registration for a SIP GmAccount.
   * PRE          :  A valid pointer to a valid SIP GmAccount.
   */
  void SIPRegister (GmAccount *);

  
protected:


  GmAccount *account;

  PMutex quit_mutex;
  PSyncPoint thread_sync_point;
};

#endif
