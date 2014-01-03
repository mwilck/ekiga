
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2013 Damien Sandras <dsandras@seconix.com>
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
 *                          (c) 2013 by Julien Puydt
 *   description          : implementation of an opal account
 *
 */

#include "config.h"

#include <stdlib.h>

#include <glib/gi18n.h>

#include "menu-builder.h"
#include "form-request-simple.h"

#include "opal-bank.h"
#include "sip-endpoint.h"


Opal::Bank::Bank (Ekiga::ServiceCore& core):
  sip_endpoint(core.get<Opal::Sip::EndPoint> ("opal-sip-endpoint")),
  presence_core(core.get<Ekiga::PresenceCore> ("presence-core")),
  notification_core(core.get<Ekiga::NotificationCore> ("notification-core")),
  personal_details(core.get<Ekiga::PersonalDetails> ("personal-details")),
  audiooutput_core(core.get<Ekiga::AudioOutputCore> ("audiooutput-core")),
  opal_component(core.get<CallManager> ("opal-component"))
{
  std::list<std::string> accounts;
  protocols_settings = new Ekiga::Settings (PROTOCOLS_SCHEMA);

  const std::string raw = protocols_settings->get_string ("accounts");

  doc = boost::shared_ptr<xmlDoc> (xmlRecoverMemory (raw.c_str (), raw.length ()), xmlFreeDoc);
  if (!doc)
    doc = boost::shared_ptr<xmlDoc> (xmlNewDoc (BAD_CAST "1.0"), xmlFreeDoc);

  node = xmlDocGetRootElement (doc.get ());
  if (node == NULL) {

    node = xmlNewDocNode (doc.get (), NULL, BAD_CAST "accounts", NULL);
    xmlDocSetRootElement (doc.get (), node);
  }

  for (xmlNodePtr child = node->children; child != NULL; child = child->next) {

    if (child->type == XML_ELEMENT_NODE
	&& child->name != NULL
	&& xmlStrEqual(BAD_CAST "account", child->name)) {

      boost::shared_ptr<Account> account(new Account (sip_endpoint, presence_core, notification_core,
						      personal_details, audiooutput_core, opal_component,
						      boost::bind(&Opal::Bank::existing_groups, this), child));

      add_account (account);
      Ekiga::BankImpl<Account>::add_connection (account, account->presentity_added.connect (boost::bind (boost::ref(presentity_added), account, _1)));
      Ekiga::BankImpl<Account>::add_connection (account, account->presentity_updated.connect (boost::bind (boost::ref(presentity_updated), account, _1)));
      Ekiga::BankImpl<Account>::add_connection (account, account->presentity_removed.connect (boost::bind (boost::ref(presentity_removed), account, _1)));
      Ekiga::BankImpl<Account>::add_connection (account, account->trigger_saving.connect (boost::bind (&Opal::Bank::save, this)));
    }
  }

  sip_endpoint->registration_event.connect (boost::bind(&Opal::Bank::on_registration_event, this, _1, _2, _3));
  sip_endpoint->mwi_event.connect (boost::bind(&Opal::Bank::on_mwi_event, this, _1, _2));

  account_added.connect (boost::bind (&Opal::Bank::update_sip_endpoint_aor_map, this));
  account_updated.connect (boost::bind (&Opal::Bank::update_sip_endpoint_aor_map, this));
  account_removed.connect (boost::bind (&Opal::Bank::update_sip_endpoint_aor_map, this));
  update_sip_endpoint_aor_map ();
}


Opal::Bank::~Bank ()
{
  // do it forcibly so we're sure the accounts are freed before our
  // reference to the call manager. Indeed they try to unregister from
  // presence when killed, and that gives a crash if the call manager
  // is already gone!
  Ekiga::RefLister<Opal::Account>::remove_all_objects ();

  delete protocols_settings;
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
#ifdef HAVE_H323
  builder.add_action ("add", _("_Add an H.323 Account"),
		      boost::bind (&Opal::Bank::new_account, this, Opal::Account::H323, "", ""));
#endif

  return true;
}


bool
Opal::Bank::populate_menu (Ekiga::ContactPtr contact,
			   const std::string uri,
			   Ekiga::MenuBuilder& builder)
{
  return populate_menu_helper (contact->get_name (), uri, builder);
}


bool
Opal::Bank::populate_menu (Ekiga::PresentityPtr presentity,
			   const std::string uri,
			   Ekiga::MenuBuilder& builder)
{
  return populate_menu_helper (presentity->get_name (), uri, builder);
}


bool
Opal::Bank::populate_menu_helper (const std::string fullname,
				  const std::string& uri,
				  Ekiga::MenuBuilder& builder)
{
  bool result = false;

  if (uri.find ("@") == string::npos) {

    // no domain: try to save the situation by trying all accounts

    for (iterator iter = begin ();
	 iter != end ();
	 ++iter)
      result = (*iter)->populate_menu (fullname, uri, builder) || result;
  } else {

    // there is a domain: just add the actions
    result = opal_component->populate_menu (fullname, uri, builder);
  }

  return result;
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


void
Opal::Bank::on_new_account_form_submitted (bool submitted,
					   Ekiga::Form& result,
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


void
Opal::Bank::add (Account::Type acc_type,
		 std::string name,
		 std::string host,
		 std::string user,
		 std::string auth_user,
		 std::string password,
		 bool enabled,
		 unsigned timeout)
{
  xmlNodePtr child = Opal::Account::build_node (acc_type, name, host, user, auth_user, password, enabled, timeout);

  xmlAddChild (node, child);

  save ();

  AccountPtr account
    = AccountPtr(new Opal::Account (sip_endpoint,
				    presence_core,
				    notification_core,
				    personal_details,
				    audiooutput_core,
				    opal_component,
				    boost::bind(&Opal::Bank::existing_groups, this),
				    child));
  add_account (account);
  Ekiga::BankImpl<Account>::add_connection (account, account->trigger_saving.connect (boost::bind (&Opal::Bank::save, this)));
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
  xmlChar *buffer = NULL;
  int doc_size = 0;

  xmlDocDumpMemory (doc.get (), &buffer, &doc_size);

  protocols_settings->set_string ("accounts", (const char*)buffer);

  xmlFree (buffer);
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
Opal::Bank::on_registration_event (std::string aor,
				   Opal::Account::RegistrationState state,
				   std::string msg)
{
  AccountPtr account = find_account (aor);

  if (account)
    account->handle_registration_event (state, msg);
}


void
Opal::Bank::on_mwi_event (std::string aor,
			  std::string info)
{
  AccountPtr account = find_account (aor);

  if (account)
    account->handle_message_waiting_information (info);
}


void
Opal::Bank::update_sip_endpoint_aor_map ()
{
  std::map<std::string, std::string> result;

  for (iterator iter = begin ();
       iter != end ();
       ++iter)
    result[(*iter)->get_host ()] = (*iter)->get_aor ();

  sip_endpoint->update_aor_map (result);
}


void
Opal::Bank::visit_heaps (boost::function1<bool, Ekiga::HeapPtr> visitor) const
{
  visit_objects (visitor);
}


const std::set<std::string>
Opal::Bank::existing_groups () const
{
  std::set<std::string> result;

  for (const_iterator iter = begin ();
       iter != end ();
       ++iter) {

    std::set<std::string> groups = (*iter)->get_groups ();
    result.insert (groups.begin (), groups.end ());
  }

  result.insert (_("Family"));
  result.insert (_("Friend"));
  /* Translator: http://www.ietf.org/rfc/rfc4480.txt proposes several
     relationships between you and your contact; associate means
     someone who is at the same "level" than you.
  */
  result.insert (_("Associate"));
  /* Translator: http://www.ietf.org/rfc/rfc4480.txt proposes several
     relationships between you and your contact; assistant means
     someone who is at a lower "level" than you.
  */
  result.insert (_("Assistant"));
  /* Translator: http://www.ietf.org/rfc/rfc4480.txt proposes several
     relationships between you and your contact; supervisor means
     someone who is at a higher "level" than you.
  */
  result.insert (_("Supervisor"));
  /* Translator: http://www.ietf.org/rfc/rfc4480.txt proposes several
     relationships between you and your contact; self means yourself.
  */
  result.insert (_("Self"));

  return result;
}
