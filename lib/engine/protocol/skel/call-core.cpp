
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras

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

#include <iostream>

#include "config.h"

#include "call-core.h"
#include "call-manager.h"


using namespace Ekiga;

void CallCore::add_manager (CallManager &manager)
{
  managers.insert (&manager);
  manager_added.emit (manager);

  // If this Manager emits the new_call signal, let's relay it at the CallCore level
  manager.new_call.connect (sigc::bind (sigc::mem_fun (this, &CallCore::on_new_call), &manager));

  // IM stuff
  manager.im_failed.connect (sigc::bind (sigc::mem_fun (this, &CallCore::on_im_failed), &manager));
  manager.im_received.connect (sigc::bind (sigc::mem_fun (this, &CallCore::on_im_received), &manager));
  manager.im_sent.connect (sigc::bind (sigc::mem_fun (this, &CallCore::on_im_sent), &manager));
}


void CallCore::visit_managers (sigc::slot<void, CallManager &> visitor)
{
  for (std::set<CallManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++)
    visitor (*(*iter));
}


bool CallCore::dial (const std::string uri)
{
  for (std::set<CallManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    if ((*iter)->dial (uri))
      return true;
  }

  return false;
}


bool CallCore::send_message (std::string uri, 
                             std::string message)
{
  for (std::set<CallManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    if ((*iter)->send_message (uri, message))
      return true;
  }

  return false;
}


void CallCore::on_new_call (Call *call, CallManager *manager)
{
  call->setup.connect (sigc::bind (sigc::mem_fun (this, &CallCore::on_setup_call), call, manager));
  call->missed.connect (sigc::bind (sigc::mem_fun (this, &CallCore::on_missed_call), call, manager));
  call->cleared.connect (sigc::bind (sigc::mem_fun (this, &CallCore::on_cleared_call), call, manager));
  call->established.connect (sigc::bind (sigc::mem_fun (this, &CallCore::on_established_call), call, manager));
  call->held.connect (sigc::bind (sigc::mem_fun (this, &CallCore::on_held_call), call, manager));
  call->retrieved.connect (sigc::bind (sigc::mem_fun (this, &CallCore::on_retrieved_call), call, manager));
  call->stream_opened.connect (sigc::bind (sigc::mem_fun (this, &CallCore::on_stream_opened), call, manager));
  call->stream_closed.connect (sigc::bind (sigc::mem_fun (this, &CallCore::on_stream_closed), call, manager));
  call->stream_paused.connect (sigc::bind (sigc::mem_fun (this, &CallCore::on_stream_paused), call, manager));
  call->stream_resumed.connect (sigc::bind (sigc::mem_fun (this, &CallCore::on_stream_resumed), call, manager));
}


void CallCore::on_setup_call (Call *call, CallManager *manager)
{
  setup_call.emit (*manager, *call);
}


void CallCore::on_missed_call (Call *call, CallManager *manager)
{
  missed_call.emit (*manager, *call);
}


void CallCore::on_cleared_call (std::string reason, Call *call, CallManager *manager)
{
  cleared_call.emit (*manager, *call, reason);
}


void CallCore::on_established_call (Call *call, CallManager *manager)
{
  established_call.emit (*manager, *call);
}


void CallCore::on_held_call (Call *call, CallManager *manager)
{
  held_call.emit (*manager, *call);
}


void CallCore::on_retrieved_call (Call *call, CallManager *manager)
{
  retrieved_call.emit (*manager, *call);
}


void CallCore::on_stream_opened (std::string name, Call::StreamType type, bool is_transmitting, Call *call, CallManager *manager)
{
  stream_opened.emit (*manager, *call, name, type, is_transmitting);
}


void CallCore::on_stream_closed (std::string name, Call::StreamType type, bool is_transmitting, Call *call, CallManager *manager)
{
  stream_closed.emit (*manager, *call, name, type, is_transmitting);
}


void CallCore::on_stream_paused (std::string name, Call::StreamType type, Call *call, CallManager *manager)
{
  stream_paused.emit (*manager, *call, name, type);
}


void CallCore::on_stream_resumed (std::string name, Call::StreamType type, Call *call, CallManager *manager)
{
  stream_resumed.emit (*manager, *call, name, type);
}


void CallCore::on_im_failed (std::string uri, std::string reason, CallManager *manager)
{
  im_failed.emit (*manager, uri, reason);
}


void CallCore::on_im_sent (std::string uri, std::string message, CallManager *manager)
{
  im_sent.emit (*manager, uri, message);
}


void CallCore::on_im_received (std::string display_name, std::string uri, std::string message, CallManager *manager)
{
  im_received.emit (*manager, display_name, uri, message);
}

