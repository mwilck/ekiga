
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
#include "contact-action.h"


using namespace Ekiga;

CallCore::CallCore ()
{
  nr_ready = 0;
}


void CallCore::add_manager (boost::shared_ptr<CallManager> manager)
{
  managers.insert (manager);
  manager_added (manager);

  manager->ready.connect (boost::bind (&CallCore::on_manager_ready, this, manager));
}


CallCore::iterator CallCore::begin ()
{
  return managers.begin ();
}


CallCore::const_iterator CallCore::begin () const
{
  return managers.begin ();
}


CallCore::iterator CallCore::end ()
{
  return managers.end ();
}


CallCore::const_iterator CallCore::end () const
{
  return managers.end ();
}


bool CallCore::dial (const std::string & uri)
{
  for (std::set<boost::shared_ptr<CallManager> >::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    if ((*iter)->dial (uri))
      return true;
  }

  return false;
}


void CallCore::hang_up ()
{
  for (std::set<boost::shared_ptr<CallManager> >::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++)
    (*iter)->hang_up ();
}


bool CallCore::transfer (const std::string & uri,
                         bool attended)
{
  for (std::set<boost::shared_ptr<CallManager> >::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    if ((*iter)->transfer (uri, attended))
      return true;
  }

  return false;
}


bool CallCore::message (const ContactPtr & contact,
                        const std::string & uri)
{
  for (std::set<boost::shared_ptr<CallManager> >::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    if ((*iter)->message (contact, uri))
      return true;
  }

  return false;
}


bool CallCore::is_supported_uri (const std::string & uri)
{
  for (std::set<boost::shared_ptr<CallManager> >::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    if ((*iter)->is_supported_uri (uri))
      return true;
  }

  return false;
}


void CallCore::register_actions (boost::shared_ptr<ContactCore> _contact_core)
{
  contact_core = _contact_core;
  contact_core->add_action (ActionPtr (
                              new ContactAction ("call", _("Call"),
                                                 boost::bind (&CallCore::dial,
                                                              this, _2),
                                                 boost::bind (&CallCore::is_supported_uri,
                                                              this, _2))));

  contact_core->add_action (ActionPtr (
                              new ContactAction ("message", _("Message"),
                                                 boost::bind (&CallCore::message,
                                                              this, _1, _2),
                                                 boost::bind (&CallCore::is_supported_uri,
                                                              this, _2))));
}


void CallCore::add_call (boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager)
{
  contact_core->remove_action ("call");
  boost::shared_ptr<Ekiga::scoped_connections> conns(new Ekiga::scoped_connections);

  conns->add (call->ringing.connect (boost::bind (&CallCore::on_ringing_call, this,
                                                  call, manager)));
  conns->add (call->setup.connect (boost::bind (&CallCore::on_setup_call, this,
                                                call, manager)));
  conns->add (call->missed.connect (boost::bind (&CallCore::on_missed_call, this,
                                                 call, manager)));
  conns->add (call->cleared.connect (boost::bind (&CallCore::on_cleared_call, this,
                                                  _1, call, manager)));
  conns->add (call->established.connect (boost::bind (&CallCore::on_established_call, this,
                                                      call, manager)));
  conns->add (call->held.connect (boost::bind (&CallCore::on_held_call, this,
                                               call, manager)));
  conns->add (call->retrieved.connect (boost::bind (&CallCore::on_retrieved_call, this,
                                                    call, manager)));
  conns->add (call->stream_opened.connect (boost::bind (&CallCore::on_stream_opened, this,
                                                        _1, _2, _3, call, manager)));
  conns->add (call->stream_closed.connect (boost::bind (&CallCore::on_stream_closed, this,
                                                        _1, _2, _3, call, manager)));
  conns->add (call->stream_paused.connect (boost::bind (&CallCore::on_stream_paused, this,
                                                        _1, _2, call, manager)));
  conns->add (call->stream_resumed.connect (boost::bind (&CallCore::on_stream_resumed, this,
                                                         _1, _2, call, manager)));
  conns->add (call->removed.connect (boost::bind (&CallCore::on_call_removed, this, call)));

  call_connections [call->get_id ()] = conns;
  contact_core->add_action (ActionPtr (
                              new ContactAction ("transfer", _("Transfer"),
                                                 boost::bind (&CallCore::transfer,
                                                              this, _2, false),
                                                 boost::bind (&CallCore::is_supported_uri,
                                                              this, _2))));
}


void CallCore::remove_call (boost::shared_ptr<Call> call)
{
  contact_core->remove_action ("transfer");

  call_connections.erase (call->get_id ());

  contact_core->add_action (ActionPtr (
                              new ContactAction ("call", _("Call"),
                                                 boost::bind (&CallCore::dial,
                                                              this, _2),
                                                 boost::bind (&CallCore::is_supported_uri,
                                                              this, _2))));
}


void CallCore::on_ringing_call (boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager)
{
  ringing_call (manager, call);
}


void CallCore::on_setup_call (boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager)
{
  setup_call (manager, call);
}


void CallCore::on_missed_call (boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager)
{
  missed_call (manager, call);
}


void CallCore::on_cleared_call (std::string reason, boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager)
{
  cleared_call (manager, call, reason); 
}


void CallCore::on_established_call (boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager)
{
  established_call (manager, call);
}


void CallCore::on_held_call (boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager)
{
  held_call (manager, call);
}


void CallCore::on_retrieved_call (boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager)
{
  retrieved_call (manager, call);
}


void CallCore::on_stream_opened (std::string name, Call::StreamType type, bool is_transmitting, boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager)
{
  stream_opened (manager, call, name, type, is_transmitting);
}


void CallCore::on_stream_closed (std::string name, Call::StreamType type, bool is_transmitting, boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager)
{
  stream_closed (manager, call, name, type, is_transmitting);
}


void CallCore::on_stream_paused (std::string name, Call::StreamType type, boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager)
{
  stream_paused (manager, call, name, type);
}


void CallCore::on_stream_resumed (std::string name, Call::StreamType type, boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager)
{
  stream_resumed (manager, call, name, type);
}


void CallCore::on_manager_ready (boost::shared_ptr<CallManager> manager)
{
  manager_ready (manager);
  nr_ready++;

  if (nr_ready >= managers.size ())
    ready ();
}


void CallCore::on_call_removed (boost::shared_ptr<Call> call)
{
  remove_call (call);
}
