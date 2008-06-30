
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
 *                         accounts_manager.cpp  -  description
 *                         ------------------------------------
 *   begin                : Sun Feb 13 2005
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *   			    manipulate H323/SIP/... provider accounts.
 */


#include "config.h"

#include "accountshandler.h"

#include "common.h"

#include "main.h"
#include "statusicon.h"

#include "manager.h"
#include "sip.h"
#include "h323.h"
#include "ekiga.h"

#include "misc.h"

#include "gmconf.h"


/* Class to register accounts in a thread.
*/
GMAccountsEndpoint::GMAccountsEndpoint (Opal::CallManager & endpoint)
  :PThread (1000, NoAutoDeleteThread),
   ep (endpoint), accounts(NULL), active(TRUE)
{
  // TODO CallCore
  endpoint.ready.connect (sigc::mem_fun (this, &GMAccountsEndpoint::on_call_core_ready));
}


GMAccountsEndpoint::~GMAccountsEndpoint ()
{
  active = FALSE;
  
  PWaitAndSignal m(quit_mutex);
}


void GMAccountsEndpoint::Main ()
{
  GSList *defined_accounts = NULL;
  GSList *accounts_iter = NULL;
  
  GmAccount *list_account = NULL;

  PWaitAndSignal m(quit_mutex);

  /* Register all accounts */
  defined_accounts = gnomemeeting_get_accounts_list ();
  accounts_iter = defined_accounts;
  while (accounts_iter) {

    if (accounts_iter->data) {

      list_account = GM_ACCOUNT (accounts_iter->data);

      /* Register SIP account */
      if (list_account->protocol_name) {

        if (!strcmp (list_account->protocol_name, "SIP")) 
          SIPRegister (list_account);
        else
          H323Register (list_account);
      }
    }

    accounts_iter = g_slist_next (accounts_iter);
  }

  g_slist_foreach (defined_accounts, (GFunc) gm_account_delete, NULL);
  g_slist_free (defined_accounts);

  while (active) {       

    accounts_mutex.Wait ();     
    accounts_iter = accounts;   
    while (accounts_iter) {     

      if (accounts_iter->data) {        

        list_account = GM_ACCOUNT (accounts_iter)->data;        

        /* Register SIP account */      
        if (list_account->protocol_name) {      

          if (!strcmp (list_account->protocol_name, "SIP"))     
            SIPRegister (list_account);         
          else          
            H323Register (list_account);        
        }       
      }         

      accounts_iter = g_slist_next (accounts_iter);     
    }   

    g_slist_foreach (accounts, (GFunc) gm_account_delete, NULL);        
    g_slist_free (accounts);    
    accounts = NULL;    
    accounts_mutex.Signal ();   

    PThread::Sleep (100);       
  }
}


void GMAccountsEndpoint::RegisterAccount (GmAccount *account)
{
  GmAccount *acc = NULL;
  
  PWaitAndSignal m(accounts_mutex);

  acc = gm_account_copy (account);
  accounts = g_slist_append (accounts, (gpointer) acc);
}


void GMAccountsEndpoint::SIPRegister (GmAccount * /*a*/)
{
  return;

}


void GMAccountsEndpoint::H323Register (GmAccount *a)
{
  std::string aor;
  Opal::H323::CallProtocolManager *h323_manager = dynamic_cast<Opal::H323::CallProtocolManager *> (ep.get_protocol_manager ("h323"));

  // TODO Move this to the engine and drop the dynamic cast
  aor = a->username;
  if (aor.find ("@") == string::npos)
    aor = aor + "@" + a->host;

  if (h323_manager)
    h323_manager->Register (aor.c_str (), a->auth_username, a->password, a->domain, a->timeout, !a->enabled);
}


void GMAccountsEndpoint::on_call_core_ready ()
{
  this->Resume ();
}
