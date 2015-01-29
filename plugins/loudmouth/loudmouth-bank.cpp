
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras

 * This program is free software; you can  redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ekiga is licensed under the GPL license and as a special exception, you
 * have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OPAL, OpenH323 and PWLIB
 * programs, as long as you do follow the requirements of the GNU GPL for all
 * the rest of the software thus combined.
 */


/*
 *                         loudmouth-bank.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of the loudmouth account manager
 *
 */

#include "loudmouth-bank.h"

#include <glib/gi18n.h>

#include "form-request-simple.h"

#define JABBER_KEY "jabber"

LM::Bank::Bank (boost::shared_ptr<Ekiga::PersonalDetails> details_,
		boost::shared_ptr<Dialect> dialect_,
		boost::shared_ptr<Cluster> cluster_):
  details(details_), cluster(cluster_), dialect(dialect_), doc (NULL)
{
  contacts_settings = boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (CONTACTS_SCHEMA));
  std::string raw = contacts_settings->get_string (JABBER_KEY);

  if (!raw.empty ()) { // we already have it in store

    doc = xmlRecoverMemory (raw.c_str (), raw.length ());
    xmlNodePtr root = xmlDocGetRootElement (doc);
    if (root == NULL) {

      root = xmlNewDocNode (doc, NULL, BAD_CAST "list", NULL);
      xmlDocSetRootElement (doc, root);
    }

    for (xmlNodePtr child = root->children; child != NULL; child = child->next) {

      if (child->type == XML_ELEMENT_NODE && child->name != NULL && xmlStrEqual (BAD_CAST ("entry"), child->name)) {

	boost::shared_ptr<Account> account (new Account (details, dialect, cluster, child));
	add (account);
      }
    }

  } else { // create a new XML document

    doc = xmlNewDoc (BAD_CAST "1.0");
    xmlNodePtr root = xmlNewDocNode (doc, NULL, BAD_CAST "list", NULL);
    xmlDocSetRootElement (doc, root);
  }
}

void
LM::Bank::add (boost::shared_ptr<Account> account)
{
  account->trigger_saving.connect (boost::bind (&LM::Bank::save, this));
  add_account (account);
}

void
LM::Bank::save () const
{
  xmlChar* buffer = NULL;
  int docsize = 0;

  xmlDocDumpMemory (doc, &buffer, &docsize);

  contacts_settings->set_string (JABBER_KEY, (const char *)buffer);

  xmlFree (buffer);
}

LM::Bank::~Bank ()
{
}

bool
LM::Bank::populate_menu (Ekiga::MenuBuilder& builder)
{
  builder.add_action ("add", _("_Add a Jabber/XMPP Account"),
		      boost::bind (&LM::Bank::new_account, this));
  return true;
}

void
LM::Bank::new_account ()
{
  boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&LM::Bank::on_new_account_form_submitted, this, _1, _2)));

  request->title (_("Edit account"));

  request->instructions (_("Please fill in the following fields:"));

  request->text ("name", _("_Name:"), "", _("Account name, e.g. MyAccount"));
  request->text ("user", _("_User:"), "", _("The user name, e.g. jim"));
  request->text ("server", _("Server:"), "", _("The server, e.g. jabber.org"));
  request->text ("resource", _("Resource:"), "", _("The resource, such as home or work, allowing to distinguish among several terminals registered to the same account; leave empty if you do not know what it is"));
  request->text ("password", _("_Password:"), "", _("Password associated to the user"), Ekiga::FormVisitor::PASSWORD);
  request->boolean ("enabled", _("_Enable account"), true);

  questions (request);
}

void
LM::Bank::on_new_account_form_submitted (bool submitted,
					 Ekiga::Form &result)
{
  if (!submitted)
    return;

  std::string name = result.text ("name");
  std::string user = result.text ("user");
  std::string server = result.text ("server");
  std::string resource = result.text ("resource");
  std::string password = result.text ("password");
  bool enable_on_startup = result.boolean ("enabled");

  boost::shared_ptr<Account> account (new Account (details, dialect, cluster,
						   name, user, server, LM_CONNECTION_DEFAULT_PORT,
						   resource, password,
						   enable_on_startup));
  xmlNodePtr root = xmlDocGetRootElement (doc);
  xmlAddChild (root, account->get_node ());

  save ();
  add (account);
}
