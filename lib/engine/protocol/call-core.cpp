
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>

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
 *                         call-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : declaration of the interface of a call core.
 *                          A call core manages CallManagers.
 *
 */

#include "config.h"

#include <glib/gi18n.h>

#include "call-core.h"

#include "call-manager.h"


using namespace Ekiga;

CallCore::CallCore (boost::shared_ptr<Ekiga::FriendOrFoe> iff_): iff(iff_)
{
}


void CallCore::add_manager (boost::shared_ptr<CallManager> manager)
{
  manager_added (manager);

  add_object (manager);
}


void CallCore::remove_manager (boost::shared_ptr<CallManager> manager)
{
  manager_removed (manager);
  remove_object (manager);
}


CallCore::iterator CallCore::begin ()
{
  return RefLister<CallManager>::begin ();
}


CallCore::const_iterator CallCore::begin () const
{
  return RefLister<CallManager>::begin ();
}


CallCore::iterator CallCore::end ()
{
  return RefLister<CallManager>::end ();
}


CallCore::const_iterator CallCore::end () const
{
  return RefLister<CallManager>::end ();
}


bool CallCore::dial (const std::string & uri)
{
  for (CallCore::iterator iter = begin ();
       iter != end ();
       iter++) {
    if ((*iter)->dial (uri))
      return true;
  }

  return false;
}


void CallCore::hang_up ()
{
  for (CallCore::iterator iter = begin ();
       iter != end ();
       iter++)
    (*iter)->hang_up ();
}


bool CallCore::is_supported_uri (const std::string & uri)
{
  for (CallCore::iterator iter = begin ();
       iter != end ();
       iter++) {
    if ((*iter)->is_supported_uri (uri))
      return true;
  }

  return false;
}


Ekiga::CodecList
CallCore::get_codecs () const
{
  Ekiga::CodecList codecs;
  for (CallCore::const_iterator iter = begin ();
       iter != end ();
       iter++) {
    codecs.append ((*iter)->get_codecs ());
  }

  return codecs;
}


void
CallCore::set_codecs (Ekiga::CodecList & codecs)
{
  for (CallCore::iterator iter = begin ();
       iter != end ();
       iter++) {
    (*iter)->set_codecs (codecs);
  }
}


void CallCore::add_call (boost::shared_ptr<Call> call)
{
  Ekiga::FriendOrFoe::Identification id = iff->decide ("call", call->get_remote_uri ());

  if (id == Ekiga::FriendOrFoe::Foe) {

    call->hang_up ();
    return;
  }

  created_call (call);

  boost::shared_ptr<Ekiga::scoped_connections> conns(new Ekiga::scoped_connections);

  conns->add (call->ringing.connect (boost::bind (boost::ref (ringing_call), call)));
  conns->add (call->setup.connect (boost::bind (boost::ref (setup_call), call)));
  conns->add (call->missed.connect (boost::bind (boost::ref (missed_call), call)));
  conns->add (call->cleared.connect (boost::bind (boost::ref (cleared_call), call, _1)));
  conns->add (call->established.connect (boost::bind (boost::ref (established_call), call)));
  conns->add (call->held.connect (boost::bind (boost::ref (held_call), call)));
  conns->add (call->retrieved.connect (boost::bind (boost::ref (retrieved_call), call)));
  conns->add (call->stream_opened.connect (boost::bind (boost::ref (stream_opened), call, _1, _2, _3)));
  conns->add (call->stream_closed.connect (boost::bind (boost::ref (stream_closed), call, _1, _2, _3)));
  conns->add (call->stream_paused.connect (boost::bind (boost::ref (stream_paused), call, _1, _2)));
  conns->add (call->stream_resumed.connect (boost::bind (boost::ref (stream_resumed), call, _1, _2)));
  conns->add (call->removed.connect (boost::bind (&CallCore::on_call_removed, this, call)));

  call_connections [call->get_id ()] = conns;
}


void CallCore::remove_call (boost::shared_ptr<Call> call)
{
  call_connections.erase (call->get_id ());
}


void CallCore::on_call_removed (boost::shared_ptr<Call> call)
{
  remove_call (call);
}
