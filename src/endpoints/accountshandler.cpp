
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
#include "history.h"
#include "statusicon.h"
#include "contacts.h"

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

    PStringList tmp;

    /* Presence Subscribe */
    subscribers_mutex.Wait ();
    tmp = active_subscribers;
    subscribers_mutex.Signal ();

    for (int i = 0 ; i < tmp.GetSize () ; i++)
      SIPPresenceSubscribe (tmp [i], FALSE);

    subscribers_mutex.Wait ();
    tmp = inactive_subscribers;
    subscribers_mutex.Signal ();
    
    for (int i = 0 ; i < tmp.GetSize () ; i++)
      SIPPresenceSubscribe (tmp [i], TRUE);

    subscribers_mutex.Wait ();
    active_subscribers.RemoveAll ();
    inactive_subscribers.RemoveAll ();
    subscribers_mutex.Signal ();

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


void GMAccountsEndpoint::PresenceSubscribe (GmContact *contact,
                                            BOOL unsubscribe)
{
  PWaitAndSignal m(subscribers_mutex);
  
  if (!contact 
      || GMURL (contact->url).GetType () != "sip"  
      || GMURL (contact->url).IsEmpty ())
    return ;

  if (unsubscribe)
    inactive_subscribers += contact->url;
  else
    active_subscribers += contact->url;
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


void GMAccountsEndpoint::SIPPresenceSubscribe (PString contact,
                                               BOOL unsubscribe)
{
  SIPSubscribe::SubscribeType t = SIPSubscribe::Presence;
  PString to;
  GMSIPEndpoint *sipEP = NULL;

  sipEP = ep.GetSIPEndpoint ();

  to = contact;
  to.Replace("sip:", "");
  if (unsubscribe && sipEP->IsSubscribed (SIPSubscribe::Presence, to)) 
    sipEP->Subscribe (t, 0, to);
  if (!unsubscribe && !sipEP->IsSubscribed (SIPSubscribe::Presence, to)) 
    sipEP->Subscribe (t, 500, to);//FIXME
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
  GtkWidget *accounts_window = NULL;
  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;

  gchar *msg = NULL;
  gchar *aor = NULL;

  gboolean result = FALSE;

  GMSIPEndpoint *sipEP = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();

  sipEP = ep.GetSIPEndpoint ();

  if (!a)
    return;

  if (PString (a->username).Find("@") != P_MAX_INDEX)
    aor = g_strdup (a->username);
  else
    aor = g_strdup_printf ("%s@%s", a->username, a->host);

  /* Account is enabled, and we are not registered */
  if (a->enabled && !sipEP->IsRegistered (aor)) {

    gnomemeeting_threads_enter ();
    gm_accounts_window_update_account_state (accounts_window,
					     TRUE,
					     aor,
					     _("Registering"),
					     NULL);
    gnomemeeting_threads_leave ();

    result = sipEP->Register (a->host,
                              aor,
			      a->auth_username,
			      a->password,
			      PString::Empty(),
			      a->timeout);

    if (!result) {

      msg = g_strdup_printf (_("Registration of %s to %s failed"), 
			     a->username?a->username:"", 
			     a->host?a->host:"");

      gnomemeeting_threads_enter ();
      gm_main_window_push_message (main_window, "%s", msg);
      gm_history_window_insert (history_window, "%s", msg);
      gm_accounts_window_update_account_state (accounts_window,
					       FALSE,
					       aor,
					       _("Registration failed"),
					       NULL);
      gnomemeeting_threads_leave ();

      g_free (msg);
    }
  }
  else if (!a->enabled) {
    
    if (sipEP->IsRegistered (aor)) {

      gnomemeeting_threads_enter ();
      gm_accounts_window_update_account_state (accounts_window,
                                               TRUE,
                                               aor,
                                               _("Unregistering"),
                                               NULL);
      gnomemeeting_threads_leave ();
    }

    sipEP->Unregister (aor);
  }

  g_free (aor);
}


void GMAccountsEndpoint::H323Register (GmAccount *a)
{
  GtkWidget *accounts_window = NULL;
  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;

  gchar *msg = NULL;
  gchar *aor = NULL;

  gboolean result = FALSE;

  GMH323Endpoint *h323EP = NULL;
  H323Gatekeeper *gatekeeper = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();

  h323EP = ep.GetH323Endpoint ();

  if (!a)
    return;

  aor = g_strdup_printf ("%s@%s", a->username, a->host);
    
  /* Account is enabled, and we are not registered, only one
   * account can be enabled at a time */
  if (a->enabled) {

    h323EP->H323EndPoint::RemoveGatekeeper (0);

    gnomemeeting_threads_enter ();
    gm_accounts_window_update_account_state (accounts_window,
					     TRUE,
                                             aor,
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
      msg = g_strdup_printf (_("Registered %s"), aor);

    gnomemeeting_threads_enter ();
    gm_main_window_push_message (main_window, "%s", msg);
    gm_history_window_insert (history_window, "%s", msg);
    gm_accounts_window_update_account_state (accounts_window,
					     FALSE,
					     aor,
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

    msg = g_strdup_printf (_("Unregistered %s"), aor);
    gnomemeeting_threads_enter ();
    gm_accounts_window_update_account_state (accounts_window,
					     TRUE,
					     aor,
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
                                             aor,
					     _("Unregistered"),
					     NULL);
    gm_main_window_set_account_info (main_window, 
				     ep.GetRegisteredAccounts ());
    gnomemeeting_threads_leave ();
    g_free (msg);
  }
  
  g_free (aor);
}

