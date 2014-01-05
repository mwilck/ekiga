
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2011 Damien Sandras

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
 *                         loudmouth-helpers.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008-2011 by Julien Puydt
 *   copyright            : (c) 2008-2011 by Julien Puydt
 *   description          : implementation of some helper functions
 *
 */

#include "loudmouth-helpers.h"


std::pair<std::string, std::string>
split_jid (const std::string jid)
{
  size_t pos = jid.find ('/');
  std::string base = jid;
  std::string resource;

  if (pos != std::string::npos) {

    base = jid.substr (0, pos);
    resource = jid.substr (pos);
  }

  return std::pair<std::string, std::string> (base, resource);
}


boost::shared_ptr<LmMessageHandler> ignore_message_handler;

struct handler_data
{
  handler_data (boost::function2<LmHandlerResult, LmConnection*, LmMessage*> callback_):
    callback(callback_)
  {}

  boost::function2<LmHandlerResult, LmConnection*, LmMessage*> callback;
};

static LmHandlerResult
ignorer_handler (LmMessageHandler* /*handler*/,
	   LmConnection* /*connection*/,
	   LmMessage* /*message*/,
	   gpointer /*data*/)
{
  return LM_HANDLER_RESULT_REMOVE_MESSAGE;
}

static LmHandlerResult
handler_function_c (LmMessageHandler* handler,
		    LmConnection* connection,
		    LmMessage* message,
		    handler_data* data)
{
  LmHandlerResult result = data->callback (connection, message);

  delete data;
  lm_message_handler_unref (handler);

  return result;
}

LmMessageHandler*
build_message_handler (boost::function2<LmHandlerResult, LmConnection*, LmMessage*> callback)
{
  handler_data* data = new handler_data (callback);
  LmMessageHandler* result = lm_message_handler_new ((LmHandleMessageFunction)handler_function_c, data, NULL);

  return result;
}

LmMessageHandler*
get_ignore_answer_handler ()
{
  if ( !ignore_message_handler) {

    ignore_message_handler = boost::shared_ptr<LmMessageHandler> (lm_message_handler_new (ignorer_handler, NULL, NULL), lm_message_handler_unref);
  }

  return ignore_message_handler.get ();
}
