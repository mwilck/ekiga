
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
 *                         accounts.cpp  -  description
 *                         ----------------------------
 *   begin                : Sun Feb 13 2005
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *   			    manipulate H.323/SIP/... provider accounts.
 */


#include "../config.h"

#include "accounts.h"
#include "misc.h"
#include "main_window.h"
#include "pref_window.h"
#include "log_window.h"

#include "endpoint.h"
#include "sipendpoint.h"
#include "gnomemeeting.h"

#include "gm_conf.h"

GmAccount *
gm_account_new ()
{
  GmAccount *account = NULL;
  
  account = g_new (GmAccount, 1);
 
  account->aid = g_strdup (OpalGloballyUniqueID ().AsString ());
  account->account_name = NULL;
  account->protocol_name = NULL;
  account->host = NULL;
  account->domain = NULL;
  account->login = NULL;
  account->password = NULL;
  account->enabled = FALSE;
  account->timeout = 0;
  account->method = 0;

  return account;
}


void
gm_account_delete (GmAccount *account)
{
  if (!account)
    return;

  g_free (account->aid);
  g_free (account->account_name);
  g_free (account->protocol_name);
  g_free (account->domain);
  g_free (account->login);
  g_free (account->password);
  g_free (account->host);

  g_free (account);
}


GmAccount *
gm_account_copy (GmAccount *a)
{
  GmAccount *account = NULL;
  
  if (!a)
    return account;
  
  account = g_new (GmAccount, 1);
 
  account->aid = g_strdup (a->aid);
  account->account_name = g_strdup (a->account_name);
  account->protocol_name = g_strdup (a->protocol_name);
  account->host = g_strdup (a->host);
  account->domain = g_strdup (a->domain);
  account->login = g_strdup (a->login);
  account->password = g_strdup (a->password);
  account->enabled = a->enabled;
  account->timeout = a->timeout;
  account->method = a->method;

  return account;
}


gboolean 
gnomemeeting_account_add (GmAccount *account)
{
  GSList *list = NULL;
  gchar *entry = NULL;
  
  list = 
    gm_conf_get_string_list (PROTOCOLS_KEY "accounts_list");

  entry = g_strdup_printf ("%d|%s|%s|%s|%s|%s|%s|%s|%d|%d", 
			   account->enabled,
			   account->aid, 
			   account->account_name, 
			   account->protocol_name,
			   account->host,
			   account->domain,
			   account->login,
			   account->password,
			   account->timeout,
			   account->method);
  
  list = g_slist_append (list, (gpointer) entry);
  gm_conf_set_string_list (PROTOCOLS_KEY "accounts_list", 
			   list);

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  return TRUE;
}


gboolean 
gnomemeeting_account_delete (GmAccount *account)
{
  GSList *list = NULL;
  GSList *l = NULL;
  
  gchar *entry = NULL;
  
  gboolean found = FALSE;
  
  list = 
    gm_conf_get_string_list (PROTOCOLS_KEY "accounts_list");

  entry = g_strdup_printf ("%d|%s|%s|%s|%s|%s|%s|%s|%d|%d", 
			   account->enabled,
			   account->aid, 
			   account->account_name, 
			   account->protocol_name,
			   account->host,
			   account->domain,
			   account->login,
			   account->password,
			   account->timeout,
			   account->method);

  l = list;
  while (l && !found) {

    if (l->data && !strcmp ((const char *) l->data, entry)) {

      found = TRUE;
      break;
    }
    
    l = g_slist_next (l);
  }
  
  if (found) {

    list = g_slist_remove_link (list, l);

    g_free (l->data);
    g_slist_free_1 (l);

    gm_conf_set_string_list (PROTOCOLS_KEY "accounts_list", 
			     list);
  }

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  g_free (entry);

  return found;
}


gboolean 
gnomemeeting_account_modify (GmAccount *account)
{
  GSList *list = NULL;
  GSList *l = NULL;
  
  gchar *entry = NULL;
  gchar **couple = NULL;
  
  gboolean found = FALSE;
  
  list = 
    gm_conf_get_string_list (PROTOCOLS_KEY "accounts_list");

  entry = g_strdup_printf ("%d|%s|%s|%s|%s|%s|%s|%s|%d|%d", 
			   account->enabled,
			   account->aid, 
			   account->account_name, 
			   account->protocol_name,
			   account->host,
			   account->domain,
			   account->login,
			   account->password,
			   account->timeout,
			   account->method);
  
  
  l = list;
  while (l && !found) {

    if (l->data) {
      
      couple = g_strsplit ((const char *) l->data, "|", 0);
      if (couple && couple [1] && !strcmp (couple [1], account->aid)) {

	found = TRUE;
	break;
      }
    }
    
    l = g_slist_next (l);
  }
  
  if (found) {

    list = g_slist_insert_before (list, l, (gpointer) entry);
    list = g_slist_remove_link (list, l);

    g_free (l->data);
    g_slist_free_1 (l);

    gm_conf_set_string_list (PROTOCOLS_KEY "accounts_list", 
			     list);
  }

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  return found;
}


GSList *
gnomemeeting_get_accounts_list ()
{
  GSList *result = NULL;

  GSList *accounts_data_iter = NULL;
  GSList *accounts_data = NULL;
  
  GmAccount *account = NULL;

  gint size = 0;
  gchar **couple = NULL;
  
  accounts_data = 
    gm_conf_get_string_list (PROTOCOLS_KEY "accounts_list");
  
  accounts_data_iter = accounts_data;
  while (accounts_data_iter) {

    couple = g_strsplit ((gchar *) accounts_data_iter->data, "|", 0);

    if (couple) {
      
      while (couple [size])
	size++;
      size = size + 1;
      
      account = gm_account_new ();
      
      if (size >= 1 && couple [0])
	account->enabled = atoi (couple [0]);
      if (size >= 2 && couple [1])
	account->aid = g_strdup (couple [1]);
      if (size >= 3 && couple [2])
	account->account_name = g_strdup (couple [2]);
      if (size >= 4 && couple [3])
	account->protocol_name = g_strdup (couple [3]);
      if (size >= 5 && couple [4])
	account->host = g_strdup (couple [4]);
      if (size >= 6 && couple [5])
	account->domain = g_strdup (couple [5]);
      if (size >= 7 && couple [6])
	account->login = g_strdup (couple [6]);
      if (size >= 8 && couple [7])
	account->password = g_strdup (couple [7]);
      if (size >= 9 && couple [8])
	account->timeout = atoi (couple [8]);
      if (size >= 10 && couple [9])
	account->method = atoi (couple [9]);
   
      result = g_slist_append (result, (void *) account);

      g_strfreev (couple);
    }

    accounts_data_iter = g_slist_next (accounts_data_iter);
  }

  g_slist_foreach (accounts_data, (GFunc) g_free, NULL);
  g_slist_free (accounts_data);

  return result;
}


gboolean 
gnomemeeting_account_toggle_active (GmAccount *account)
{
  if (!account)
    return FALSE;
  
  account->enabled = !account->enabled;

  return gnomemeeting_account_modify (account);
}


/* Class to register accounts in a thread.
 */
GMAccountsManager::GMAccountsManager (GmAccount *a)
  :PThread (1000, NoAutoDeleteThread)
{
  account = gm_account_copy (a);

  this->Resume ();
  thread_sync_point.Wait ();
}


GMAccountsManager::~GMAccountsManager ()
{
  /* Nothing to do here */
  PWaitAndSignal m(quit_mutex);

  gm_account_delete (account);
}


void GMAccountsManager::Main ()
{
  GSList *accounts = NULL;
  GSList *accounts_iter = NULL;

  GmAccount *list_account = NULL;

  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();
 

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
}


void GMAccountsManager::SIPRegister (GmAccount *a)
{
  GtkWidget *prefs_window = NULL;
  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;

  gchar *msg = NULL;

  gboolean result = FALSE;

  GMEndPoint *endpoint = NULL;
  GMSIPEndPoint *sipEP = NULL;

  endpoint = GnomeMeeting::Process ()->Endpoint ();

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();

  sipEP = endpoint->GetSIPEndPoint ();

  if (!a)
    return;

  /* Account is enabled, and we are not registered */
  if (a->enabled && !sipEP->IsRegistered (a->host)) {
  
    gnomemeeting_threads_enter ();
    gm_prefs_window_update_account_state (prefs_window,
					  TRUE,
					  a->host,
					  a->login,
					  _("Registering"));
    gnomemeeting_threads_leave ();
    
    result = sipEP->Register (a->host, a->login, a->password);

    if (!result) {

      msg = g_strdup_printf (_("Registration of %s to %s failed"), 
			     a->login?a->login:"", 
			     a->host?a->host:"");

      gnomemeeting_threads_enter ();
      gm_main_window_push_message (main_window, msg);
      gm_history_window_insert (history_window, msg);
      gm_prefs_window_update_account_state (prefs_window,
					    FALSE,
					    a->host,
					    a->login,
					    _("Registration failed"));
      gnomemeeting_threads_leave ();

      g_free (msg);
    }
  }
  else if (!a->enabled && sipEP->IsRegistered (a->host)) {

    gnomemeeting_threads_enter ();
    gm_prefs_window_update_account_state (prefs_window,
					  TRUE,
					  a->host,
					  a->login,
					  _("Unregistering"));
    gnomemeeting_threads_leave ();

    sipEP->Unregister (a->host,
		       a->login);
  }
}


void GMAccountsManager::H323Register (GmAccount *a)
{
  GtkWidget *prefs_window = NULL;
  GtkWidget *main_window = NULL;
  GtkWidget *history_window = NULL;

  gchar *msg = NULL;

  gboolean result = FALSE;

  GMEndPoint *endpoint = NULL;
  H323EndPoint *h323EP = NULL;
  H323Gatekeeper *gatekeeper = NULL;

  endpoint = GnomeMeeting::Process ()->Endpoint ();

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();

  h323EP = (H323EndPoint *) endpoint->GetH323EndPoint ();

  if (!a)
    return;

  /* Account is enabled, and we are not registered, only one
   * account can be enabled at a time */
  if (a->enabled) {
  
    h323EP->RemoveGatekeeper (0);

    gnomemeeting_threads_enter ();
    gm_prefs_window_update_account_state (prefs_window,
					  TRUE,
					  a->host,
					  a->login,
					  _("Registering"));
    gnomemeeting_threads_leave ();

    if (a->login && strcmp (a->login, ""))
      h323EP->AddAliasName (a->login);
    h323EP->SetGatekeeperPassword (a->password);
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
	    g_strdup (_("Gatekeeper registration failed: bad login/password"));
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
    gm_main_window_push_message (main_window, msg);
    gm_history_window_insert (history_window, msg);
    gm_prefs_window_update_account_state (prefs_window,
					  FALSE,
					  a->host,
					  a->login,
					  result?
					  _("Registered")
					  :_("Registration failed"));
    gnomemeeting_threads_leave ();
    g_free (msg);
  }
  else if (!a->enabled && h323EP->IsRegisteredWithGatekeeper ()) {

    gnomemeeting_threads_enter ();
    gm_prefs_window_update_account_state (prefs_window,
					  TRUE,
					  a->host,
					  a->login,
					  _("Unregistering"));
    gnomemeeting_threads_leave ();
    
    h323EP->RemoveAliasName (a->login);
    h323EP->RemoveGatekeeper (0);
    
    gnomemeeting_threads_enter ();
    gm_prefs_window_update_account_state (prefs_window,
					  FALSE,
					  a->host,
					  a->login,
					  _("Unregistered"));
    gnomemeeting_threads_leave ();
  }
}

