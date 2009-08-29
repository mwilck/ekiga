
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
 *                         loudmouth-dialect.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of the loudmouth dialect
 *
 */

#include "loudmouth-dialect.h"

LM::Dialect::Dialect (Ekiga::ServiceCore& core_):
  core(core_)
{
}

LM::Dialect::~Dialect ()
{
}

void
LM::Dialect::push_message (PresentityPtr presentity,
			   const std::string msg)
{
  bool found = false;

  for (simple_iterator iter = simple_begin ();
       iter != simple_end ();
       ++iter) {

    if (presentity == (*iter)->get_presentity ()) {

      (*iter)->got_message (msg);
      found = true;
      break;
    }
  }

  if ( !found) {

    SimpleChatPtr chat(new SimpleChat (core, presentity));

    add_simple_chat (chat);
    chat->got_message (msg);
  }
}

void
LM::Dialect::open_chat (PresentityPtr presentity)
{
  SimpleChatPtr chat(new SimpleChat (core, presentity));
  add_simple_chat (chat);
  chat->user_requested ();
}

bool
LM::Dialect::populate_menu (Ekiga::MenuBuilder& /*builder*/)
{
  return false;
}
