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
 *                         loudmouth-handler.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2011 by Julien Puydt
 *   copyright            : (c) 2011 by Julien Puydt
 *   description          : declaration of an abstract loudmouth handler class
 *
 */

#ifndef __LOUDMOUTH_HANDLER_H__
#define __LOUDMOUTH_HANDLER_H__

#include <loudmouth/loudmouth.h>

namespace LM
{

  class Handler
  {
  public:

    virtual ~Handler () {}

    virtual void handle_up (LmConnection* connection,
			    const std::string name) = 0;

    virtual void handle_down (LmConnection* connection) = 0;

    virtual LmHandlerResult handle_iq (LmConnection* connection,
				       LmMessage* message) = 0;

    virtual LmHandlerResult handle_message (LmConnection* connection,
					    LmMessage* message) = 0;

    virtual LmHandlerResult handle_presence (LmConnection* connection,
					     LmMessage* message) = 0;
  };
};

#endif
