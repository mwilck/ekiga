
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
		      const std::string user_,
		      const std::string password_,
		      const std::string resource_,
		      const std::string server_,
		      unsigned port_):
  details(details_), dialect(dialect_), cluster(cluster_), user(user_), password(password_), resource(resource_), server(server_), port(port_), connection(0)
{
  connection = lm_connection_new (NULL);
  lm_connection_set_disconnect_function (connection, (LmDisconnectFunction)on_disconnected_c,
					 this, NULL);
  connect ();
}

void
LM::Account::connect ()
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

    // FIXME: do better
    std::cout << error->message << std::endl;
    g_error_free (error);
  }
}

LM::Account::~Account ()
{
  std::cout << __PRETTY_FUNCTION__ << std::endl;

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

    lm_connection_authenticate (connection, user.c_str (), password.c_str (), resource.c_str (),
				(LmResultFunction)on_authenticate_c, this, NULL, NULL);
  } else {

    std::cout << "Error opening loudmouth connection" << std::endl; // FIXME: do better
  }
}

void
LM::Account::on_disconnected (LmDisconnectReason /*reason*/)
{
  if (heap) {

    heap->disconnected ();
    heap.reset ();
  }
}

void
LM::Account::on_authenticate (bool result)
{
  if (result) {

    heap = gmref_ptr<Heap> (new Heap (details, dialect, connection));
    cluster->add_heap (heap);
  } else {

    lm_connection_close (connection, NULL);
    std::cout << "Error authenticating loudmouth account" << std::endl;
  }
}
