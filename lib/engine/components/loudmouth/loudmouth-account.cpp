
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
 *                         loudmouth-account.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of a loudmouth account
 *
 */

#include <iostream>
#include <glib/gi18n.h>

#include "form-request-simple.h"

#include "loudmouth-account.h"

/* here come the C callbacks, which just push to C++ code */
static void
on_connection_opened_c (LmConnection* /*unused*/,
			gboolean result,
			LM::Account* account)
{
  account->on_connection_opened (result);
}

static void
on_disconnected_c (LmConnection* /*unused*/,
		   LmDisconnectReason reason,
		   LM::Account* account)
{
  account->on_disconnected (reason);
}

static void
on_authenticate_c (LmConnection* /*unused*/,
		   gboolean result,
		   LM::Account* account)
{
  account->on_authenticate (result);
}

/* and here is the C++ code : */

LM::Account::Account (gmref_ptr<Ekiga::PersonalDetails> details_,
		      gmref_ptr<Dialect> dialect_,
		      gmref_ptr<Cluster> cluster_,
		      xmlNodePtr node_):
  details(details_), dialect(dialect_), cluster(cluster_), node(node_)
{
  xmlChar* xml_str = NULL;

  status = _("inactive");

  if (node == NULL) {

    node = xmlNewNode (NULL, BAD_CAST "entry");
    /* FIXME: make translatable */
    xmlSetProp (node, BAD_CAST "name", BAD_CAST "Jabber/XMPP account to configure");
    xmlSetProp (node, BAD_CAST "user", BAD_CAST "username");
    xmlSetProp (node, BAD_CAST "password", BAD_CAST "");
    xmlSetProp (node, BAD_CAST "resource", BAD_CAST "ekiga");
    xmlSetProp (node, BAD_CAST "server", BAD_CAST "localhost");
    xmlSetProp (node, BAD_CAST "port", BAD_CAST "5222");
    xmlSetProp (node, BAD_CAST "startup", BAD_CAST "false");
  }

  xml_str = xmlGetProp (node, BAD_CAST "name");
  if (xml_str != NULL) {

    name = (const char*)xml_str;
    xmlFree (xml_str);
  }

  xml_str = xmlGetProp (node, BAD_CAST "user");
  if (xml_str != NULL) {

    user = (const char*)xml_str;
    xmlFree (xml_str);
  }

  xml_str = xmlGetProp (node, BAD_CAST "password");
  if (xml_str != NULL) {

    password = (const char*)xml_str;
    xmlFree (xml_str);
  }

  xml_str = xmlGetProp (node, BAD_CAST "resource");
  if (xml_str != NULL) {

    resource = (const char*)xml_str;
    xmlFree (xml_str);
  }

  xml_str = xmlGetProp (node, BAD_CAST "server");
  if (xml_str != NULL) {

    server = (const char*)xml_str;
    xmlFree (xml_str);
  }

  xml_str = xmlGetProp (node, BAD_CAST "port");
  if (xml_str != NULL) {

    port = atoi ((const char*)xml_str);
    xmlFree (xml_str);
  } else {

    port = 5222;
  }

  xml_str = xmlGetProp (node, BAD_CAST "startup");
  if (xml_str != NULL) {

    if (xmlStrEqual (xml_str, BAD_CAST "true")) {

      enable_on_startup = true;
    } else {

      enable_on_startup = false;
    }
    xmlFree (xml_str);
  }

  connection = lm_connection_new (NULL);
  lm_connection_set_disconnect_function (connection, (LmDisconnectFunction)on_disconnected_c,
					 this, NULL);
  if (enable_on_startup) {

    enable ();
  }
}

void
LM::Account::enable ()
{
  GError *error = NULL;
  {
    gchar* jid = NULL;
    jid = g_strdup_printf ("%s@%s/%s", user.c_str (), server.c_str (), resource.c_str ());
    lm_connection_set_jid (connection, jid);
    g_free (jid);
  }
  lm_connection_set_server (connection, server.c_str ());
  if ( !lm_connection_open (connection,
			    (LmResultFunction)on_connection_opened_c,
			    this, NULL, &error)) {

    gchar* message = NULL;

    message = g_strdup_printf (_("error connecting (%s)"), error->message);
    status = message;
    g_free (message);
    g_error_free (error);
  } else {

    status = _("connecting");
  }

  enable_on_startup = true;
  trigger_saving.emit ();

  updated.emit ();
}

void
LM::Account::disable ()
{

  enable_on_startup = false;
  trigger_saving.emit ();

  lm_connection_close (connection, NULL);
}

LM::Account::~Account ()
{
  if (heap) {

    heap->disconnected ();
    heap.reset ();
  }

  if (lm_connection_is_open (connection)) {

    lm_connection_close (connection, NULL);
  }
  lm_connection_unref (connection);
  connection = 0;
}

void
LM::Account::on_connection_opened (bool result)
{
  if (result) {

    status = _("authenticating");
    lm_connection_authenticate (connection, user.c_str (), password.c_str (), resource.c_str (),
				(LmResultFunction)on_authenticate_c, this, NULL, NULL);
    updated.emit ();
  } else {

    /* FIXME: can't we report better? */
    status = _("error connecting");
    updated.emit ();
  }
}

void
LM::Account::on_disconnected (LmDisconnectReason /*reason*/)
{
  if (heap) {

    heap->disconnected ();
    heap.reset ();
    status = _("disconnected");
    updated.emit ();
  }
}

void
LM::Account::on_authenticate (bool result)
{
  if (result) {

    heap = gmref_ptr<Heap> (new Heap (details, dialect, connection));
    heap->set_name (name);
    cluster->add_heap (heap);
    status = _("connected");
    updated.emit ();
  } else {

    lm_connection_close (connection, NULL);
    // FIXME: can't we report something better?
    status = _("error authenticating loudmouth account");
    updated.emit ();
  }
}

xmlNodePtr
LM::Account::get_node () const
{
  return node;
}

void
LM::Account::edit ()
{
  Ekiga::FormRequestSimple request(sigc::mem_fun (this,
						  &LM::Account::on_edit_form_submitted));

  request.title (_("Edit account"));

  request.instructions (_("Please update the following fields:"));

  request.text ("name", _("Name:"), name);
  request.text ("user", _("User:"), user);
  request.text ("server", _("Server:"), server);
  request.text ("resource", _("Resource:"), resource);
  request.private_text ("password", _("Password:"), password);
  request.boolean ("enabled", _("Enable account"), enable_on_startup);

  if (!questions.handle_request (&request)) {

    // FIXME: better error reporting
#ifdef __GNUC__
    std::cout << "Unhandled form request in "
	      << __PRETTY_FUNCTION__ << std::endl;
#endif
  }
}

void
LM::Account::on_edit_form_submitted (bool submitted,
				     Ekiga::Form &result)
{
  if (!submitted)
    return;

  disable (); // don't stay connected!

  name = result.text ("name");
  user = result.text ("user");
  server = result.text ("server");
  resource = result.text ("resource");
  password = result.private_text ("password");
  enable_on_startup = result.boolean ("enabled");

  if (enable_on_startup)
    enable ();
}

bool
LM::Account::populate_menu (Ekiga::MenuBuilder& builder)
{
  if (lm_connection_is_open (connection)) {

    builder.add_action ("disable", _("_Disable"),
			sigc::mem_fun (this, &LM::Account::disable));
  } else {

    builder.add_action ("enable", _("_Enable"),
			sigc::mem_fun (this, &LM::Account::enable));
  }

  builder.add_separator ();

  builder.add_action ("edit", _("Edit"),
		      sigc::mem_fun (this, &LM::Account::edit));

  return true;
}
