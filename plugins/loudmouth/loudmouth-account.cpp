
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

#include <sstream>

#include <glib/gi18n.h>

#include "form-request-simple.h"

#include "loudmouth-account.h"

#define DEBUG 0

#if DEBUG
#include <iostream>
#endif

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

static LmHandlerResult
iq_handler_c (LmMessageHandler* /*handler*/,
	      LmConnection* /*connection*/,
	      LmMessage* message,
	      LM::Account* account)
{
  return account->handle_iq (message);
}

static LmHandlerResult
presence_handler_c (LmMessageHandler* /*handler*/,
		    LmConnection* /*connection*/,
		    LmMessage* message,
		    LM::Account* account)
{
  return account->handle_presence (message);
}

static LmHandlerResult
message_handler_c (LmMessageHandler* /*handler*/,
		   LmConnection* /*connection*/,
		   LmMessage* message,
		   LM::Account* account)
{
  return account->handle_message (message);
}

/* and here is the C++ code : */

LM::Account::Account (boost::shared_ptr<Ekiga::PersonalDetails> details_,
		      boost::shared_ptr<Dialect> dialect_,
		      boost::shared_ptr<Cluster> cluster_,
		      xmlNodePtr node_):
  details(details_), dialect(dialect_), cluster(cluster_), node(node_)
{
  if (node == NULL) throw std::logic_error ("NULL node pointer received");

  status = _("Inactive");

  xmlChar* xml_str = xmlGetProp (node, BAD_CAST "startup");
  bool enable_on_startup = false;
  if (xml_str != NULL) {

    if (xmlStrEqual (xml_str, BAD_CAST "true")) {

      enable_on_startup = true;
    } else {

      enable_on_startup = false;
    }
  }
  xmlFree (xml_str);

  connection = lm_connection_new (NULL);

  LmMessageHandler* iq_lm_handler = lm_message_handler_new ((LmHandleMessageFunction)iq_handler_c, this, NULL);
  lm_connection_register_message_handler (connection, iq_lm_handler, LM_MESSAGE_TYPE_IQ, LM_HANDLER_PRIORITY_NORMAL);
  lm_message_handler_unref (iq_lm_handler);

  LmMessageHandler* presence_lm_handler = lm_message_handler_new ((LmHandleMessageFunction)presence_handler_c, this, NULL);
  lm_connection_register_message_handler (connection, presence_lm_handler, LM_MESSAGE_TYPE_PRESENCE, LM_HANDLER_PRIORITY_NORMAL);
  lm_message_handler_unref (presence_lm_handler);

  LmMessageHandler* message_lm_handler = lm_message_handler_new ((LmHandleMessageFunction)message_handler_c, this, NULL);
  lm_connection_register_message_handler (connection, message_lm_handler, LM_MESSAGE_TYPE_MESSAGE, LM_HANDLER_PRIORITY_NORMAL);
  lm_message_handler_unref (message_lm_handler);

  lm_connection_set_disconnect_function (connection, (LmDisconnectFunction)on_disconnected_c,
					 this, NULL);
  if (enable_on_startup) {

    enable ();
  }
}

LM::Account::Account (boost::shared_ptr<Ekiga::PersonalDetails> details_,
		      boost::shared_ptr<Dialect> dialect_,
		      boost::shared_ptr<Cluster> cluster_,
		      const std::string name, const std::string user,
		      const std::string server, int port,
		      const std::string resource, const std::string password,
		      bool enable_on_startup):
  details(details_), dialect(dialect_), cluster(cluster_)
{
  status = _("Inactive");

  node = xmlNewNode (NULL, BAD_CAST "entry");
  xmlSetProp (node, BAD_CAST "name", BAD_CAST name.c_str ());
  xmlSetProp (node, BAD_CAST "user", BAD_CAST user.c_str ());
  xmlSetProp (node, BAD_CAST "server", BAD_CAST server.c_str ());
  {
    std::stringstream sstream;
    sstream << port;
    xmlSetProp (node, BAD_CAST "port", BAD_CAST sstream.str ().c_str ());
  }
  xmlSetProp (node, BAD_CAST "resource", BAD_CAST resource.c_str ());
  xmlSetProp (node, BAD_CAST "password", BAD_CAST password.c_str ());

  if (enable_on_startup) {

    xmlSetProp (node, BAD_CAST "startup", BAD_CAST "true");
  } else {

    xmlSetProp (node, BAD_CAST "startup", BAD_CAST "false");
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
  xmlChar* server =  NULL;
  unsigned port = LM_CONNECTION_DEFAULT_PORT;
  LmSSL* ssl = NULL;

  server = xmlGetProp (node, BAD_CAST "server");
  {
    xmlChar* port_str = xmlGetProp (node, BAD_CAST "port");

    port = atoi ((const char*)port_str);
    xmlFree (port_str);
  }

  {
    gchar* jid = NULL;
    xmlChar* user = NULL;
    xmlChar* resource = NULL;
    user = xmlGetProp (node, BAD_CAST "user");
    resource = xmlGetProp (node, BAD_CAST "resource");
    jid = g_strdup_printf ("%s@%s/%s", user, server, resource);
    lm_connection_set_jid (connection, jid);
    g_free (jid);
    xmlFree (user);
    xmlFree (resource);
  }

  /* ugly but necessary */
  if (g_strcmp0 ("gmail.com", (const char*)server) == 0)
    lm_connection_set_server (connection, "xmpp.l.google.com");
  else
    lm_connection_set_server (connection, (const char*)server);

  lm_connection_set_port (connection, port);

  ssl = lm_ssl_new (NULL, NULL, NULL, NULL);
  lm_ssl_use_starttls (ssl, TRUE, TRUE);
  lm_connection_set_ssl(connection, ssl);
  lm_ssl_unref (ssl);

  if ( !lm_connection_open (connection,
			    (LmResultFunction)on_connection_opened_c,
			    this, NULL, &error)) {

    gchar* message = NULL;

    message = g_strdup_printf (_("Could not connect (%s)"), error->message);
    status = message;
    g_free (message);
    g_error_free (error);
  } else {

    status = _("Connecting...");
  }

  xmlFree (server);

  xmlSetProp (node, BAD_CAST "startup", BAD_CAST "true");
  trigger_saving ();

  updated ();
}

void
LM::Account::disable ()
{
  xmlSetProp (node, BAD_CAST "startup", BAD_CAST "false");
  trigger_saving ();

  lm_connection_close (connection, NULL);
}

LM::Account::~Account ()
{
  if (lm_connection_is_open (connection)) {

    handle_down ();
    lm_connection_close (connection, NULL);
  }

  lm_connection_unref (connection);
  connection = 0;
}

void
LM::Account::on_connection_opened (bool result)
{
  if (result) {

    xmlChar* user = xmlGetProp (node, BAD_CAST "user");
    xmlChar* password = xmlGetProp (node, BAD_CAST "password");
    xmlChar* resource = xmlGetProp (node, BAD_CAST "resource");
    status = _("Authenticating...");
    lm_connection_authenticate (connection, (const char*)user,
				(const char*)password, (const char*)resource,
				(LmResultFunction)on_authenticate_c, this, NULL, NULL);
    xmlFree (password);
    xmlFree (resource);
    updated ();
  } else {

    /* FIXME: can't we report better? */
    status = _("Could not connect");
    updated ();
  }
}

void
LM::Account::on_disconnected (LmDisconnectReason /*reason*/)
{
  handle_down ();

  status = _("Disconnected");
  updated ();
}

void
LM::Account::on_authenticate (bool result)
{
  if (result) {

    handle_up ();
    status = _("Connected");
    updated ();
  } else {

    lm_connection_close (connection, NULL);
    // FIXME: can't we report something better?
    status = _("Could not authenticate");
    updated ();
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
  boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&LM::Account::on_edit_form_submitted, this, _1, _2)));
  xmlChar* xml_str = NULL;

  request->title (_("Edit account"));

  request->instructions (_("Please update the following fields:"));

  xml_str = xmlGetProp (node, BAD_CAST "name");
  request->text ("name", _("Name:"), (const char*)xml_str, _("Account name, e.g. MyAccount"));
  xmlFree (xml_str);

  xml_str = xmlGetProp (node, BAD_CAST "user");
  request->text ("user", _("User:"), (const char*)xml_str, _("The user name, e.g. jim"));
  xmlFree (xml_str);

  xml_str = xmlGetProp (node, BAD_CAST "server");
  request->text ("server", _("Server:"), (const char*)xml_str, _("The server, e.g. jabber.org"));
  xmlFree (xml_str);

  xml_str = xmlGetProp (node, BAD_CAST "port");
  request->text ("port", _("Port:"), (const char*)xml_str, _("The transport protocol port, if different than the default"));
  xmlFree (xml_str);

  xml_str = xmlGetProp (node, BAD_CAST "resource");
  request->text ("resource", _("Resource:"), (const char*)xml_str, _("The resource, such as home or work, allowing to distinguish among several terminals registered to the same account; leave empty if you do not know what it is"));
  xmlFree (xml_str);

  xml_str = xmlGetProp (node, BAD_CAST "password");
  request->text ("password", _("Password:"), (const char*)xml_str, _("Password associated to the user"), Ekiga::FormVisitor::PASSWORD);
  xmlFree (xml_str);

  xml_str = xmlGetProp (node, BAD_CAST "startup");
  bool enable_on_startup = false;
  if (xmlStrEqual (xml_str, BAD_CAST "true")) {

    enable_on_startup = true;
  } else {

    enable_on_startup = false;

  }
  xmlFree (xml_str);
  request->boolean ("enabled", _("Enable account"), enable_on_startup);

  questions (request);
}

void
LM::Account::on_edit_form_submitted (bool submitted,
				     Ekiga::Form &result)
{
  if (!submitted)
    return;

  disable (); // don't stay connected!

  std::string name = result.text ("name");
  std::string user = result.text ("user");
  std::string server = result.text ("server");
  std::string port = result.text ("port");
  std::string resource = result.text ("resource");
  std::string password = result.text ("password");
  bool enable_on_startup = result.boolean ("enabled");

  xmlSetProp (node, BAD_CAST "name", BAD_CAST name.c_str ());
  xmlSetProp (node, BAD_CAST "user", BAD_CAST user.c_str ());
  xmlSetProp (node, BAD_CAST "server", BAD_CAST server.c_str ());
  xmlSetProp (node, BAD_CAST "port", BAD_CAST port.c_str ());
  xmlSetProp (node, BAD_CAST "resource", BAD_CAST resource.c_str ());
  xmlSetProp (node, BAD_CAST "password", BAD_CAST password.c_str ());

  if (enable_on_startup) {

    xmlSetProp (node, BAD_CAST "startup", BAD_CAST "true");
    enable ();
  } else {

    xmlSetProp (node, BAD_CAST "startup", BAD_CAST "false");
    updated ();
  }
}

void
LM::Account::remove ()
{
  disable ();

  xmlUnlinkNode (node);
  xmlFreeNode (node);

  trigger_saving ();
  removed ();
}

bool
LM::Account::populate_menu (Ekiga::MenuBuilder& builder)
{
  if (lm_connection_is_open (connection)) {

    builder.add_action ("user-offline", _("_Disable"),
			boost::bind (&LM::Account::disable, this));
  } else {

    builder.add_action ("user-available", _("_Enable"),
			boost::bind (&LM::Account::enable, this));
  }

  builder.add_separator ();

  builder.add_action ("edit", _("Edit"),
		      boost::bind (&LM::Account::edit, this));
  builder.add_action ("remove", _("_Remove"),
		      boost::bind (&LM::Account::remove, this));

  return true;
}

const std::string
LM::Account::get_status () const
{
  return status;
}

bool
LM::Account::is_enabled () const
{
  bool result = false;
  xmlChar* xml_str = xmlGetProp (node, BAD_CAST "startup");

  if (xml_str != NULL) {

    if (xmlStrEqual (xml_str, BAD_CAST "true")) {

      result = true;
    } else {

      result = false;
    }

    xmlFree (xml_str);
  }

  return result;
}

bool
LM::Account::is_active () const
{
  if (!is_enabled ())
    return false;

  return true; // Isn't there a way to know if an account is active?
}

const std::string
LM::Account::get_name () const
{
  std::string name;
  xmlChar* xml_str = xmlGetProp (node, BAD_CAST "name");

  name = (const char*)xml_str;

  xmlFree (xml_str);

  return name;
}

void
LM::Account::handle_up ()
{
  dialect->handle_up (connection, get_name ());
  cluster->handle_up (connection, get_name ());
}

void
LM::Account::handle_down ()
{
  dialect->handle_down (connection);
  cluster->handle_down (connection);
}

LmHandlerResult
LM::Account::handle_iq (LmMessage* message)
{
  LmHandlerResult result = LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS;

  if (result == LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS) {

    result = dialect->handle_iq (connection, message);
  }

  if (result == LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS) {

    result = cluster->handle_iq (connection, message);
  }

#if DEBUG
  if (result == LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS) {

    std::cout << "Nobody cared about : " << lm_message_node_to_string (lm_message_get_node (message)) << std::endl;
  }
#endif

  return result;
}

LmHandlerResult
LM::Account::handle_message (LmMessage* message)
{
  LmHandlerResult result = LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS;

  if (result == LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS) {

    result = dialect->handle_message (connection, message);
  }

  if (result == LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS) {

    result = cluster->handle_message (connection, message);
  }

#if DEBUG
  if (result == LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS) {

    std::cout << "Nobody cared about : " << lm_message_node_to_string (lm_message_get_node (message)) << std::endl;
  }
#endif

  return result;
}

LmHandlerResult
LM::Account::handle_presence (LmMessage* message)
{
  LmHandlerResult result = LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS;

  if (result == LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS) {

    result = dialect->handle_presence (connection, message);
  }

  if (result == LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS) {

    result = cluster->handle_presence (connection, message);
  }

#if DEBUG
  if (result == LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS) {

    std::cout << "Nobody cared about : " << lm_message_node_to_string (lm_message_get_node (message)) << std::endl;
  }
#endif

  return result;
}
