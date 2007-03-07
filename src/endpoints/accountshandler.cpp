
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


#include "../../config.h"

#include "accountshandler.h"

#include "common.h"

#include "main.h"
#include "history.h"
#include "statusicon.h"

#include "manager.h"
#include "sip.h"
#include "h323.h"
#include "ekiga.h"

#include "misc.h"

#include "gmconf.h"


/* Class to register accounts in a thread.
*/
GMAccountsEndpoint::GMAccountsEndpoint (GmAccount *a,
					GMManager & endpoint)
:PThread (1000, NoAutoDeleteThread), ep (endpoint)
{
  account = gm_account_copy (a);

  this->Resume ();
  thread_sync_point.Wait ();
}


GMAccountsEndpoint::~GMAccountsEndpoint ()
{
  /* Nothing to do here */
  PWaitAndSignal m(quit_mutex);

  gm_account_delete (account);
}


void GMAccountsEndpoint::Main ()
{
  GtkWidget *main_window = NULL;
  GtkWidget *status_icon = NULL;

  gboolean stun_support = FALSE;
 
  GSList *accounts = NULL;
  GSList *accounts_iter = NULL;

  GmAccount *list_account = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  status_icon = GnomeMeeting::Process ()->GetStatusicon ();

  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();

  gnomemeeting_threads_enter ();
  gm_main_window_set_busy (main_window, TRUE);
  gm_statusicon_set_busy (status_icon, TRUE);
  stun_support = (gm_conf_get_int (NAT_KEY "method") == 1);
  gnomemeeting_threads_leave ();

  if (stun_support && ep.GetSTUN () == NULL) 
    ep.CreateSTUNClient (FALSE, FALSE, TRUE, NULL);

  /* Let's go */
  if (account) {

    if (!strcmp (account->protocol_name, "SIP")) 
      SIPRegister (account);
    else
      H323Register (account);
  }
  else {

    accounts = gnomemeeting_get_accounts_list ();

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
  }

  gnomemeeting_threads_enter ();
  gm_main_window_set_busy (main_window, FALSE);
  gm_statusicon_set_busy (status_icon, FALSE);
  gnomemeeting_threads_leave ();
}


void GMAccountsEndpoint::SIPRegister (GmAccount *a)
{
  GtkWidget *accounts_window = NULL;
  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;

  gchar *msg = NULL;
  gchar *url = NULL;

  gboolean result = FALSE;

  GMSIPEndpoint *sipEP = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();

  sipEP = ep.GetSIPEndpoint ();

  if (!a)
    return;

  if (PString (a->username).Find("@") != P_MAX_INDEX)
    url = g_strdup (a->username);
  else
    url = g_strdup_printf ("%s@%s", a->auth_username, a->host);

  /* Account is enabled, and we are not registered */
  if (a->enabled && !sipEP->IsRegistered (url)) {

    gnomemeeting_threads_enter ();
    gm_accounts_window_update_account_state (accounts_window,
					     TRUE,
					     a->host,
					     a->username,
					     _("Registering"),
					     NULL);
    gnomemeeting_threads_leave ();

    result = sipEP->Register (a->host, 
			      a->username, 
			      a->auth_username, 
			      a->password, 
			      PString::Empty(), 
			      a->timeout);
    sipEP->MWISubscribe (a->domain, a->username); 

    if (!result) {

      msg = g_strdup_printf (_("Registration of %s to %s failed"), 
			     a->username?a->username:"", 
			     a->host?a->host:"");

      gnomemeeting_threads_enter ();
      gm_main_window_push_message (main_window, "%s", msg);
      gm_history_window_insert (history_window, "%s", msg);
      gm_accounts_window_update_account_state (accounts_window,
					       FALSE,
					       a->host,
					       a->username,
					       _("Registration failed"),
					       NULL);
      gnomemeeting_threads_leave ();

      g_free (msg);
    }
  }
  else if (!a->enabled && sipEP->IsRegistered (url)) {

    gnomemeeting_threads_enter ();
    gm_accounts_window_update_account_state (accounts_window,
					     TRUE,
					     a->host,
					     a->username,
					     _("Unregistering"),
					     NULL);
    gnomemeeting_threads_leave ();

    sipEP->Unregister (a->host,
		       a->username);
  }

  g_free (url);
}


void GMAccountsEndpoint::H323Register (GmAccount *a)
{
  GtkWidget *accounts_window = NULL;
  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;

  gchar *msg = NULL;

  gboolean result = FALSE;

  GMH323Endpoint *h323EP = NULL;
  H323Gatekeeper *gatekeeper = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();

  h323EP = ep.GetH323Endpoint ();

  if (!a)
    return;

  /* Account is enabled, and we are not registered, only one
   * account can be enabled at a time */
  if (a->enabled) {

    h323EP->H323EndPoint::RemoveGatekeeper (0);

    gnomemeeting_threads_enter ();
    gm_accounts_window_update_account_state (accounts_window,
					     TRUE,
					     a->host,
					     a->username,
					     _("Registering"),
					     NULL);
    gnomemeeting_threads_leave ();

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
	  msg = g_strdup (_("Gatekeeper registration failed: duplicate alias"));
	  break;
	case H323Gatekeeper::SecurityDenied :
	  msg = 
	    g_strdup (_("Gatekeeper registration failed: bad username/password"));
	  break;
	case H323Gatekeeper::TransportError :
	  msg = g_strdup (_("Gatekeeper registration failed: transport error"));
	  break;
	default :
	  msg = g_strdup (_("Gatekeeper registration failed"));
	  break;
	}
      }
      else
	msg = g_strdup (_("Gatekeeper registration failed"));
    }
    else
      msg = g_strdup_printf (_("Registered to %s"), a->host);

    gnomemeeting_threads_enter ();
    gm_main_window_push_message (main_window, "%s", msg);
    gm_history_window_insert (history_window, "%s", msg);
    gm_accounts_window_update_account_state (accounts_window,
					     FALSE,
					     a->host,
					     a->username,
					     result?
					     _("Registered")
					     :_("Registration failed"),
					     NULL);
    gm_main_window_set_account_info (main_window, 
				     ep.GetRegisteredAccounts ());
    gnomemeeting_threads_leave ();
    g_free (msg);
  }
  else if (!a->enabled && h323EP->IsRegisteredWithGatekeeper (a->host)) {

    msg = g_strdup_printf (_("Unregistered from %s"), a->host);
    gnomemeeting_threads_enter ();
    gm_accounts_window_update_account_state (accounts_window,
					     TRUE,
					     a->host,
					     a->username,
					     _("Unregistering"),
					     NULL);
    gnomemeeting_threads_leave ();

    h323EP->H323EndPoint::RemoveGatekeeper (0);
    h323EP->RemoveAliasName (a->username);

    gnomemeeting_threads_enter ();
    gm_main_window_push_message (main_window, "%s", msg);
    gm_history_window_insert (history_window, "%s", msg);
    gm_accounts_window_update_account_state (accounts_window,
					     FALSE,
					     a->host,
					     a->username,
					     _("Unregistered"),
					     NULL);
    gm_main_window_set_account_info (main_window, 
				     ep.GetRegisteredAccounts ());
    gnomemeeting_threads_leave ();
    g_free (msg);
  }
}

