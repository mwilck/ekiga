
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
 *                         ldap-source.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *                        : completed in 2008 by Howard Chu
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of a LDAP source
 *
 */

#include <cstdlib>
#include <string.h>

#include "config.h"

#include "ldap-source.h"

#define LDAP_KEY "ldap-servers"

OPENLDAP::Source::Source (Ekiga::ServiceCore &_core):
  core(_core), doc(), should_add_ekiga_net_book(false)
{
  xmlNodePtr root;
  contacts_settings = boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (CONTACTS_SCHEMA));
  std::string raw = contacts_settings->get_string (LDAP_KEY);

  if (!raw.empty ()) {

    doc = boost::shared_ptr<xmlDoc> (xmlRecoverMemory (raw.c_str (), raw.length ()), xmlFreeDoc);
    if ( !doc)
      doc = boost::shared_ptr<xmlDoc> (xmlNewDoc (BAD_CAST "1.0"), xmlFreeDoc);

    root = xmlDocGetRootElement (doc.get ());

    if (root == NULL) {

      root = xmlNewDocNode (doc.get (), NULL, BAD_CAST "list", NULL);
      xmlDocSetRootElement (doc.get (), root);
    }

    migrate_from_3_0_0 ();

    for (xmlNodePtr child = root->children ;
	 child != NULL ;
	 child = child->next)
      if (child->type == XML_ELEMENT_NODE
	  && child->name != NULL
	  && xmlStrEqual (BAD_CAST "server", child->name))
	add (child);

  } 
  else {

    doc = boost::shared_ptr<xmlDoc> (xmlNewDoc (BAD_CAST "1.0"), xmlFreeDoc);
    root = xmlNewDocNode (doc.get (), NULL, BAD_CAST "list", NULL);
    xmlDocSetRootElement (doc.get (), root);

    should_add_ekiga_net_book = true;
  }

  if (should_add_ekiga_net_book)
    new_ekiga_net_book ();
}

OPENLDAP::Source::~Source ()
{
}

void
OPENLDAP::Source::add (xmlNodePtr node)
{
  common_add (BookPtr(new Book (core, doc, node)));
}

void
OPENLDAP::Source::add (struct BookInfo bookinfo)
{
  xmlNodePtr root;

  root = xmlDocGetRootElement (doc.get ());
  BookPtr book (new Book (core, doc, bookinfo));

  xmlAddChild (root, book->get_node ());

  common_add (book);
}

void
OPENLDAP::Source::common_add (BookPtr book)
{
  book->trigger_saving.connect (boost::bind (&OPENLDAP::Source::save, this));
  add_book (book);
  save ();
}

bool
OPENLDAP::Source::populate_menu (Ekiga::MenuBuilder &builder)
{
  builder.add_action ("add", _("Add an LDAP Address Book"),
		      boost::bind (&OPENLDAP::Source::new_book, this));
  if (!has_ekiga_net_book ()) {

    builder.add_action ("add", _("Add the Ekiga.net Directory"),
			boost::bind (&OPENLDAP::Source::new_ekiga_net_book, this));
  }
  return true;
}

void
OPENLDAP::Source::new_book ()
{
  boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&OPENLDAP::Source::on_new_book_form_submitted, this, _1, _2)));
  struct BookInfo bookinfo;

  bookinfo.name = "";
  bookinfo.uri = "ldap://localhost/dc=net?cn,telephoneNumber?sub?(cn=$)";
  bookinfo.uri_host = "";
  bookinfo.authcID = "";
  bookinfo.password = "";
  bookinfo.saslMech = "";
  bookinfo.urld = boost::shared_ptr<LDAPURLDesc> ();
  bookinfo.sasl = false;
  bookinfo.starttls = false;

  OPENLDAP::BookInfoParse (bookinfo);
  OPENLDAP::BookForm (request, bookinfo, _("Create LDAP directory"));

  questions (request);
}

void
OPENLDAP::Source::new_ekiga_net_book ()
{
  struct BookInfo bookinfo;
  bookinfo.name = _("Ekiga.net Directory");
  bookinfo.uri =
    "ldap://ekiga.net/dc=ekiga,dc=net?givenName,telephoneNumber?sub?(cn=$)";
  bookinfo.uri_host = "";
  bookinfo.authcID = "";
  bookinfo.password = "";
  bookinfo.saslMech = "";
  bookinfo.urld = boost::shared_ptr<LDAPURLDesc> ();
  bookinfo.sasl = false;
  bookinfo.starttls = false;

  add (bookinfo);
}

void
OPENLDAP::Source::on_new_book_form_submitted (bool submitted,
					      Ekiga::Form &result)
{
  if (!submitted)
    return;

  std::string errmsg;
  struct BookInfo bookinfo;

  if (OPENLDAP::BookFormInfo (result, bookinfo, errmsg)) {
    boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&OPENLDAP::Source::on_new_book_form_submitted, this, _1, _2)));

    result.visit (*request);
    request->error (errmsg);

    questions (request);
    return;
  }

  add (bookinfo);
}

void
OPENLDAP::Source::save ()
{
  xmlChar *buffer = NULL;
  int lsize = 0;

  xmlDocDumpMemory (doc.get (), &buffer, &lsize);

  contacts_settings->set_string (LDAP_KEY, (const char *)buffer);

  xmlFree (buffer);
}

bool
OPENLDAP::Source::has_ekiga_net_book () const
{
  bool result = false;
  for (const_iterator iter = begin ();
       iter != end () && !result;
       ++iter)
    result = (*iter)->is_ekiga_net_book ();

  return result;
}

void
OPENLDAP::Source::migrate_from_3_0_0 ()
{
  gboolean found = false;
  xmlNodePtr root = xmlDocGetRootElement (doc.get ()); // can't be NULL

  for (xmlNodePtr server = root->children ;
       server != NULL && !found;
       server = server->next) {

    if (server->type == XML_ELEMENT_NODE
	&& server->name != NULL
	&& xmlStrEqual (BAD_CAST "server", server->name)) {

      for (xmlNodePtr child = server->children;
	   child != NULL && !found;
	   child = child->next) {

	if (child->type == XML_ELEMENT_NODE
	    && child->name != NULL
	    && xmlStrEqual (BAD_CAST "hostname", child->name)) {

	  // if it has a hostname, it's already an old config item
	  xmlChar* xml_str = xmlNodeGetContent (child);
	  if (xml_str != NULL) {

	    if (xmlStrEqual (BAD_CAST "ekiga.net", xml_str)) {

	      // ok, that's the one : let's get rid of it!
	      found = true;
	      xmlUnlinkNode (server);
	      xmlFreeNode (server);
	    }
	    xmlFree (xml_str);
	  }
	}
      }
    }
  }

  // eh, we removed it, but we should add it back!
  if (found)
    should_add_ekiga_net_book = true;
}
