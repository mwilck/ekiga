
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
 *                         opal-account.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : implementation of an opal account
 *
 */

#include "config.h"

#include <string.h>
#include <stdlib.h>
#include <sstream>

#include <glib/gi18n.h>

#include "gmconf.h"
#include "menu-builder.h"
#include "form-request-simple.h"

#include "opal-bank.h"
#include "sip-endpoint.h"
#ifdef HAVE_H323
#include "h323-endpoint.h"
#endif

Opal::Bank::Bank (Ekiga::ServiceCore& core):
  sip_endpoint(core.get<Opal::Sip::EndPoint> ("opal-sip-endpoint")),
#ifdef HAVE_H323
  h323_endpoint(core.get<Opal::H323::EndPoint> ("opal-h323-endpoint")),
#endif
  presence_core(core.get<Ekiga::PresenceCore> ("presence-core")),
  notification_core(core.get<Ekiga::NotificationCore> ("notification-core")),
  personal_details(core.get<Ekiga::PersonalDetails> ("personal-details")),
  audiooutput_core(core.get<Ekiga::AudioOutputCore> ("audiooutput-core")),
  opal_component(core.get<CallManager> ("opal-component"))
{
  boost::shared_ptr<CallManager> opal = opal_component.lock ();
  boost::shared_ptr<Ekiga::PresenceCore> pcore = presence_core.lock ();
  if ( !(opal && pcore))
    return;

  GSList *accounts = gm_conf_get_string_list (PROTOCOLS_KEY "accounts_list");
  GSList *accounts_iter = accounts;

  while (accounts_iter) {

    boost::shared_ptr<Account> account
      = boost::shared_ptr<Account> (new Account (sip_endpoint,
#ifdef HAVE_H323
						 h323_endpoint,
#endif
						 pcore,
						 notification_core,
						 personal_details,
						 audiooutput_core,
						 opal,
						 (char *)accounts_iter->data));

    add_account (account);
    Ekiga::BankImpl<Account>::add_connection (account, account->trigger_saving.connect (boost::bind (&Opal::Bank::save, this)));
    Ekiga::BankImpl<Account>::add_connection (account, account->presence_received.connect (boost::ref (presence_received)));
    Ekiga::BankImpl<Account>::add_connection (account, account->status_received.connect (boost::ref (status_received)));
    accounts_iter = g_slist_next (accounts_iter);
  }

  g_slist_foreach (accounts, (GFunc) g_free, NULL);
  g_slist_free (accounts);
}


bool
Opal::Bank::populate_menu (Ekiga::MenuBuilder & builder)
{
  builder.add_action ("add", _("_Add an Ekiga.net Account"),
		      boost::bind (&Opal::Bank::new_account, this, Opal::Account::Ekiga, "", ""));
  builder.add_action ("add", _("_Add an Ekiga Call Out Account"),
		      boost::bind (&Opal::Bank::new_account, this, Opal::Account::DiamondCard, "", ""));
  builder.add_action ("add", _("_Add a SIP Account"),
		      boost::bind (&Opal::Bank::new_account, this, Opal::Account::SIP, "", ""));
  builder.add_action ("add", _("_Add an H.323 Account"),
		      boost::bind (&Opal::Bank::new_account, this, Opal::Account::H323, "", ""));

  return true;
}


void
Opal::Bank::new_account (Account::Type acc_type,
			 std::string username,
			 std::string password)
{
  boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&Opal::Bank::on_new_account_form_submitted, this, _1, _2, acc_type)));

  request->title (_("Edit account"));
  request->instructions (_("Please update the following fields:"));

  switch (acc_type) {

  case Opal::Account::Ekiga:
    request->link (_("Get an Ekiga.net SIP account"), "http://www.ekiga.net");
    request->hidden ("name", "Ekiga.net");
    request->hidden ("host", "ekiga.net");
    request->text ("user", _("_User:"), username, _("The user name, e.g. jim"));
    request->hidden ("authentication_user", username);
    request->private_text ("password", _("_Password:"), password, _("Password associated to the user"));
    request->hidden ("timeout", "3600");
    break;

  case Opal::Account::DiamondCard:
    request->link (_("Get an Ekiga Call Out account"),
		   "https://www.diamondcard.us/exec/voip-login?act=sgn&spo=ekiga");
    request->hidden ("name", "Ekiga Call Out");
    request->hidden ("host", "sip.diamondcard.us");
    request->text ("user", _("_Account ID:"), username, _("The user name, e.g. jim"));
    request->hidden ("authentication_user", username);
    request->private_text ("password", _("_PIN code:"), password, _("Password associated to the user"));
    request->hidden ("timeout", "3600");
    break;

  case Opal::Account::H323:
    request->text ("name", _("_Name:"), std::string (), _("Account name, e.g. MyAccount"));
    request->text ("host", _("_Gatekeeper:"), std::string (), _("The gatekeeper, e.g. ekiga.net"));
    request->text ("user", _("_User:"), username, _("The user name, e.g. jim"));
    request->hidden ("authentication_user", username);
    request->private_text ("password", _("_Password:"), password, _("Password associated to the user"));
    request->text ("timeout", _("_Timeout:"), "3600", _("Time in seconds after which the account registration is automatically retried"));
    break;

  case Opal::Account::SIP:
  default:
    request->text ("name", _("_Name:"), std::string (), _("Account name, e.g. MyAccount"));
    request->text ("host", _("_Registrar:"), std::string (), _("The registrar, e.g. ekiga.net"));
    request->text ("user", _("_User:"), username, _("The user name, e.g. jim"));
    request->text ("authentication_user", _("_Authentication user:"), std::string (), _("The user name used during authentication, if different than the user name; leave empty if you do not have one"));
    request->private_text ("password", _("_Password:"), password, _("Password associated to the user"));
    request->text ("timeout", _("_Timeout:"), "3600", _("Time in seconds after which the account registration is automatically retried"));
    break;
  }
  request->boolean ("enabled", _("Enable account"), true);

  if (!username.empty () && !password.empty ())
    request->submit (*request);
  else
    questions (request);
}


void Opal::Bank::on_new_account_form_submitted (bool submitted,
						Ekiga::Form &result,
                                                Account::Type acc_type)
{
  if (!submitted)
    return;

  boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&Opal::Bank::on_new_account_form_submitted, this, _1, _2, acc_type)));

  std::string error;
  std::string new_name = (acc_type == Opal::Account::SIP
			  || acc_type == Opal::Account::H323) ? result.text ("name") : result.hidden ("name");
  std::string new_host = (acc_type == Opal::Account::SIP
			  || acc_type == Opal::Account::H323) ? result.text ("host") : result.hidden ("host");
  std::string new_user = result.text ("user");
  std::string new_authentication_user = (acc_type == Opal::Account::SIP) ? result.text ("authentication_user") : new_user;
  std::string new_password = result.private_text ("password");
  bool new_enabled = result.boolean ("enabled");
  unsigned new_timeout = atoi ((acc_type == Opal::Account::SIP
				|| acc_type == Opal::Account::H323) ?
			       result.text ("timeout").c_str () : result.hidden ("timeout").c_str ());

  result.visit (*request);

  if (new_name.empty ())
    error = _("You did not supply a name for that account.");
  else if (new_host.empty ())
    error = _("You did not supply a host to register to.");
  else if (new_user.empty ())
    error = _("You did not supply a user name for that account.");
  else if (new_timeout < 10)
    error = _("The timeout should be at least 10 seconds.");

  if (!error.empty ()) {
    request->error (error);

    questions (request);
  }
  else {

    add (acc_type, new_name, new_host, new_user, new_authentication_user,
	 new_password, new_enabled, new_timeout);
    save ();
  }
}


void Opal::Bank::add (Account::Type acc_type,
                      std::string name,
                      std::string host,
                      std::string user,
                      std::string auth_user,
                      std::string password,
                      bool enabled,
                      unsigned timeout)
{
  boost::shared_ptr<CallManager> opal = opal_component.lock ();
  boost::shared_ptr<Ekiga::PresenceCore> pcore = presence_core.lock ();
  if ( !(opal && pcore))
    return;
  AccountPtr account
    = AccountPtr(new Opal::Account (sip_endpoint,
#ifdef HAVE_H323
				    h323_endpoint,
#endif
				    pcore,
				    notification_core,
				    personal_details,
				    audiooutput_core,
				    opal, acc_type, name,
				    host, user, auth_user,
				    password, enabled,
				    timeout));
  add_account (account);
  Ekiga::BankImpl<Account>::add_connection (account, account->trigger_saving.connect (boost::bind (&Opal::Bank::save, this)));
  Ekiga::BankImpl<Account>::add_connection (account, account->presence_received.connect (boost::ref (presence_received)));
  Ekiga::BankImpl<Account>::add_connection (account, account->status_received.connect (boost::ref (status_received)));
}

void
Opal::Bank::call_manager_ready ()
{
  for (iterator iter = begin ();
       iter != end ();
       ++iter) {

    if ((*iter)->is_enabled ())
      (*iter)->enable ();
  }
}

Opal::AccountPtr
Opal::Bank::find_account (const std::string& aor)
{
  AccountPtr result;

  for (iterator iter = begin ();
       iter != end ();
       ++iter) {
    if (aor.find ("@") != std::string::npos && (*iter)->get_aor () == aor)  // find by account name+host (aor)
        return *iter;
    else if ((*iter)->get_host () == aor)  // find by host
      return *iter;
  }
  return result;
}

void
Opal::Bank::save () const
{
  GSList *accounts = NULL;

  for (const_iterator it = begin ();
       it != end ();
       it++) {

    std::string acct_str = (*it)->as_string ();
    if ( !acct_str.empty ())
      accounts = g_slist_append (accounts, g_strdup (acct_str.c_str ()));
  }

  gm_conf_set_string_list (PROTOCOLS_KEY "accounts_list", accounts);

  g_slist_foreach (accounts, (GFunc) g_free, NULL);
  g_slist_free (accounts);
}

void
Opal::Bank::publish (const Ekiga::PersonalDetails& details)
{
  for (iterator iter = begin ();
       iter != end ();
       iter++)
    (*iter)->publish (details);
}

void
Opal::Bank::fetch (const std::string uri)
{
  for (iterator iter = begin ();
       iter != end ();
       iter++)
    (*iter)->fetch (uri);
}

void
Opal::Bank::unfetch (const std::string uri)
{
  for (iterator iter = begin ();
       iter != end ();
       iter++)
    (*iter)->unfetch (uri);
}
