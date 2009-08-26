
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

#include <glib/gi18n.h>

#include "gmconf.h"

#include "config.h"

#include "loudmouth-bank.h"

#define KEY "/apps/" PACKAGE_NAME "/contacts/jabber"

LM::Bank::Bank (boost::shared_ptr<Ekiga::PersonalDetails> details_,
		boost::shared_ptr<Dialect> dialect_,
		boost::shared_ptr<Cluster> cluster_):
  details(details_), cluster(cluster_), dialect(dialect_), doc (NULL)
{
  gchar* c_raw = gm_conf_get_string (KEY);

  if (c_raw != NULL) { // we already have it in store

    const std::string raw = c_raw;
    doc = xmlRecoverMemory (raw.c_str (), raw.length ());
    xmlNodePtr root = xmlDocGetRootElement (doc);
    if (root == NULL) {

      root = xmlNewDocNode (doc, NULL, BAD_CAST "list", NULL);
      xmlDocSetRootElement (doc, root);
    }

    for (xmlNodePtr child = root->children; child != NULL; child = child->next) {

      if (child->type == XML_ELEMENT_NODE && child->name != NULL && xmlStrEqual (BAD_CAST ("entry"), child->name)) {

	add (child);
      }
    }
    g_free (c_raw);

  } else { // create a new XML document

    doc = xmlNewDoc (BAD_CAST "1.0");
    xmlNodePtr root = xmlNewDocNode (doc, NULL, BAD_CAST "list", NULL);
    xmlDocSetRootElement (doc, root);
  }
}

void
LM::Bank::add (xmlNodePtr node)
{
  boost::shared_ptr<Account> account (new Account (details, dialect, cluster, node));

  if (node == NULL) { // that was a new one

    xmlNodePtr root = xmlDocGetRootElement (doc);
    xmlAddChild (root, account->get_node ());

    save ();
  }

  account->trigger_saving.connect (sigc::mem_fun (this, &LM::Bank::save));
  add_account (account);
}

void
LM::Bank::save () const
{
  xmlChar* buffer = NULL;
  int size = 0;

  xmlDocDumpMemory (doc, &buffer, &size);

  gm_conf_set_string (KEY, (const char *)buffer);

  xmlFree (buffer);
}

LM::Bank::~Bank ()
{
}

bool
LM::Bank::populate_menu (Ekiga::MenuBuilder& builder)
{
  builder.add_action ("add", _("_Add a jabber/XMPP account"),
		      sigc::bind (sigc::mem_fun (this, &LM::Bank::add), (xmlNodePtr)NULL));
  return true;
}
