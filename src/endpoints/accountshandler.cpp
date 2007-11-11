
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
#include "urlhandler.h"

#include "misc.h"

#include "gmconf.h"


/* Class to register accounts in a thread.
*/
GMAccountsEndpoint::GMAccountsEndpoint (GMManager & endpoint)
:PThread (1000, NoAutoDeleteThread), ep (endpoint)
{
  this->Resume ();
  thread_sync_point.Wait ();

  active = TRUE;
  accounts = NULL;
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

  gboolean stun_support = FALSE;

  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();

  gnomemeeting_threads_enter ();
  stun_support = (gm_conf_get_int (NAT_KEY "method") == 1);
  gnomemeeting_threads_leave ();
  
  /* Enable STUN if required */
  if (stun_support && ep.GetSTUN () == NULL) 
    ep.CreateSTUNClient (FALSE, FALSE, TRUE, NULL);
  
  /* Register all accounts */
  defined_accounts = gnomemeeting_get_accounts_list ();
  accounts_iter = defined_accounts;
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

    publishers_mutex.Wait ();
    for (int i = 0 ; i < publishers.GetSize () ; i++) 
      SIPPublishPresence (publishers [i], 
                          publishers_status [i].AsInteger ());
    publishers.RemoveAll ();
    publishers_status.RemoveAll ();
    
    publishers_mutex.Signal ();
    
    PThread::Sleep (100);
  }
}


void GMAccountsEndpoint::PublishPresence (guint status)
{
  GSList *defined_accounts = NULL;
  GSList *accounts_iter = NULL;

  GmAccount *list_account = NULL;

  gchar *aor = NULL;

  PWaitAndSignal m(publishers_mutex);
  
  defined_accounts = gnomemeeting_get_accounts_list ();
  accounts_iter = defined_accounts;
  while (accounts_iter) {

    if (accounts_iter->data) {

      list_account = GM_ACCOUNT (accounts_iter)->data;

      /* Publish presence for SIP account */
      if (list_account->protocol_name
          && list_account->enabled
          && !strcmp (list_account->protocol_name, "SIP")) {

        if (PString (list_account->username).Find("@") != P_MAX_INDEX)
          aor = g_strdup (list_account->username);
        else
          aor = g_strdup_printf ("%s@%s", 
                                 list_account->username, 
                                 list_account->host);

        publishers += aor;
        publishers_status += status;

        g_free (aor);
      }
    }

    accounts_iter = g_slist_next (accounts_iter);
  }

  g_slist_foreach (defined_accounts, (GFunc) gm_account_delete, NULL);
  g_slist_free (defined_accounts);
}


void GMAccountsEndpoint::RegisterAccount (GmAccount *account)
{
  GmAccount *acc = NULL;
  
  PWaitAndSignal m(accounts_mutex);

  acc = gm_account_copy (account);
  accounts = g_slist_append (accounts, (gpointer) acc);
}


void GMAccountsEndpoint::SIPPublishPresence (const PString & to,
                                             guint status)
{
  GMSIPEndpoint *sipEP = NULL;

  sipEP = ep.GetSIPEndpoint ();

  sipEP->PublishPresence (to, status);
}


void GMAccountsEndpoint::SIPRegister (GmAccount *a)
{
  std::string aor;

  gboolean result = FALSE;

  GMSIPEndpoint *sipEP = NULL;

  sipEP = ep.GetSIPEndpoint ();

  if (!a)
    return;

  aor = a->username;
  if (aor.find ("@") == string::npos)
    aor = aor + "@" + a->host;

  /* Account is enabled, and we are not registered */
  if (a->enabled && !sipEP->IsRegistered (aor)) {

    /* Signal the OpalManager */
    ep.OnRegistering (aor, true);

    result = sipEP->Register (a->host,
                              aor.c_str (),
			      a->auth_username,
			      a->password,
			      PString::Empty(),
			      a->timeout);

    if (!result) 
      ep.OnRegistrationFailed (aor, true, _("Failed"));
  }
  else if (!a->enabled && sipEP->IsRegistered (aor.c_str ())) {

    sipEP->Unregister (aor);
  }
}


void GMAccountsEndpoint::H323Register (GmAccount *a)
{
  GtkWidget *accounts_window = NULL;
  GtkWidget *main_window = NULL;

  std::string info;
  std::string aor;

  gboolean result = FALSE;

  GMH323Endpoint *h323EP = NULL;
  H323Gatekeeper *gatekeeper = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();

  h323EP = ep.GetH323Endpoint ();

  if (!a)
    return;

  aor = a->username;
  aor += "@";
  aor += a->host;
    
  /* Account is enabled, and we are not registered, only one
   * account can be enabled at a time */
  if (a->enabled) {

    h323EP->H323EndPoint::RemoveGatekeeper (0);

    /* Signal the OpalManager */
    ep.OnRegistering (aor, true);

    if (a->username && strcmp (a->username, "")) {
      h323EP->SetLocalUserName (a->username);
      h323EP->AddAliasName (ep.GetDefaultDisplayName ());
    }
      
    h323EP->SetGatekeeperPassword (a->password);
    h323EP->SetGatekeeperTimeToLive (a->timeout * 1000);
    result = h323EP->UseGatekeeper (a->host, a->domain);

    /* There was an error (missing parameter or registration failed)
       or the user chose to not register */
    if (!result) {

      /* Registering failed */
      gatekeeper = h323EP->GetGatekeeper ();
      if (gatekeeper) {

	switch (gatekeeper->GetRegistrationFailReason()) {

	case H323Gatekeeper::DuplicateAlias :
	  info = _("Duplicate alias");
	  break;
	case H323Gatekeeper::SecurityDenied :
	  info = _("Bad username/password");
	  break;
	case H323Gatekeeper::TransportError :
	  info = _("Transport error");
	  break;
	case H323Gatekeeper::RegistrationSuccessful:
	  break;
	case H323Gatekeeper::UnregisteredLocally:
	case H323Gatekeeper::UnregisteredByGatekeeper:
	case H323Gatekeeper::GatekeeperLostRegistration:
	case H323Gatekeeper::InvalidListener:
	case H323Gatekeeper::NumRegistrationFailReasons:
	case H323Gatekeeper::RegistrationRejectReasonMask:
	default :
	  info = _("Failed");
	  break;
	}
      }
      else
	info = _("Failed");

      /* Signal the OpalManager */
      ep.OnRegistrationFailed (aor, true, info);
    }
    else {
      /* Signal the OpalManager */
      ep.OnRegistered (aor, true);
    }
  }
  else if (!a->enabled && h323EP->IsRegisteredWithGatekeeper (a->host)) {

    h323EP->H323EndPoint::RemoveGatekeeper (0);
    h323EP->RemoveAliasName (a->username);

    /* Signal the OpalManager */
    ep.OnRegistered (aor, false);
  }
}

