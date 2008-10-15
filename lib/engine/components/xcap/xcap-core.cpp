
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras
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
 *                         xcap.cpp  -  description
 *                         ------------------------------------
 *   begin                : Mon 29 September 2008
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Implementation of the XCAP support code
 *
 */

#include "config.h"

#include "xcap-core.h"

#include <libsoup/soup.h>
#include <iostream>

/* declaration of XCAP::CoreImpl */

class XCAP::CoreImpl
{
public:

  CoreImpl ();

  ~CoreImpl ();

  void read (gmref_ptr<XCAP::Path> path,
	     sigc::slot<void,XCAP::Core::ResultType,std::string> callback);

private:

  SoupSession* session;
};

/* soup callback */

struct cb_data
{
  sigc::slot<void,XCAP::Core::ResultType, std::string> callback;
};

static void
soup_callback (G_GNUC_UNUSED SoupSession* session,
	       SoupMessage* message,
	       gpointer data)
{
  cb_data* cb = (cb_data*)data;

  switch (message->status_code) {

  case SOUP_STATUS_OK:

    cb->callback (XCAP::Core::SUCCESS, message->response_body->data);
    break;

  default:

    cb->callback (XCAP::Core::ERROR, message->reason_phrase);
    break;
  }

  delete (cb_data*)data;
}

/* implementation of XCAP::CoreImpl */

XCAP::CoreImpl::CoreImpl ()
{
  session = soup_session_async_new_with_options ("user-agent", "ekiga",
						 NULL);
}

XCAP::CoreImpl::~CoreImpl ()
{
  g_object_unref (session);
}

void
XCAP::CoreImpl::read (gmref_ptr<Path> path,
		      sigc::slot<void, XCAP::Core::ResultType, std::string> callback)
{
  SoupMessage* message = NULL;
  cb_data* data = NULL;

  message = soup_message_new ("GET", path->to_uri ().c_str ());
  data = new cb_data;
  data->callback = callback;

  soup_session_queue_message (session, message,
			      soup_callback, data);
}


/* XCAP::Core just pimples : */
XCAP::Core::Core ()
{
  impl = new CoreImpl ();
}

XCAP::Core::~Core ()
{
  delete impl;
}

void
XCAP::Core::read (gmref_ptr<XCAP::Path> path,
		  sigc::slot<void, XCAP::Core::ResultType,std::string> callback)
{
  std::cout << "XCAP trying to read " << path->to_uri () << std::endl;
  impl->read (path, callback);
}
