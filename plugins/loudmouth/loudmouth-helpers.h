
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
 *                         loudmouth-helpers.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008-2011 by Julien Puydt
 *   copyright            : (c) 2008-2011 by Julien Puydt
 *   description          : declaration of some helper functions
 *
 */

#ifndef __LOUDMOUTH_HELPERS_H__
#define __LOUDMOUTH_HELPERS_H__

#include <boost/smart_ptr.hpp>
#include <boost/signals2.hpp>

#include <loudmouth/loudmouth.h>

/* This function is intended to make it easy to use a C++ function
 * as a callback for the lm_connection_send_with_reply C function ;
 * so when launching an IQ request/order, then it's easy to get the
 * result back in C++
 */
LmMessageHandler* build_message_handler (boost::function2<LmHandlerResult, LmConnection*, LmMessage*> callback);

/* sometimes it's too cumbersome to write the code to handle errors
 * properly ; this function just makes it ignored. Looking for places
 * where that function is used is a simple way to find where such code
 * is lacking.
 */
LmMessageHandler* get_ignore_answer_handler ();

#endif
