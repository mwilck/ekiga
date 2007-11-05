/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras
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
 *                         sip-endpoint.cpp  -  description
 *                         --------------------------------
 *   begin                : written in 2007 by Damien Sandras 
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : Code to provide a basic SIP endpoint
 *                          responsible of registering accounts,
 *                          presence, doing calls, ...
 *
 */

#include <iostream>
#include <algorithm>

#include "config.h"

#include "sip-endpoint.h"
#include "presence-core.h"

#include "ekiga.h"
#include "sip.h"

// Common notice :
// We are now using the API in src/endpoints. This is not correct
// and it will be changed in the future when things are reincorporated
// into the Engine.

SIP::EndPoint::~EndPoint ()
{
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

void 
SIP::EndPoint::call (std::string uri)
{
  if (!uri.empty ())
    GnomeMeeting::Process ()->Connect (uri.c_str ()); // FIXME should disappear in the future
}


void 
SIP::EndPoint::message (std::string uri,
                        std::string _message)
{
  // FIXME, this will be changed in the future
  GMSIPEndpoint *sip_ep =
    GnomeMeeting::Process ()->GetManager ()->GetSIPEndpoint ();

  sip_ep->Message (uri, _message);
}


void
SIP::EndPoint::subscribe (std::string uri,
                          int expire)
{
  // FIXME, this will be changed in the future
  GMSIPEndpoint *sip_ep =
    GnomeMeeting::Process ()->GetManager ()->GetSIPEndpoint ();

  std::string::size_type loc = uri.find ("@", 0);
  std::string domain;

  if (loc != string::npos) 
    domain = uri.substr (loc+1);

  if (std::find (uris.begin (), uris.end (), uri) == uris.end ())
    uris.push_back (uri);

  if (std::find (domains.begin (), domains.end (), domain) != domains.end ()
      && sip_ep && !sip_ep->IsSubscribed (SIPSubscribe::Presence, uri.c_str ())) {

    SIPSubscribe::SubscribeType type = SIPSubscribe::Presence;
    sip_ep->Subscribe (type, expire, PString (uri.c_str ()));
  }
}

void
SIP::EndPoint::unsubscribe (std::string uri)
{
  // FIXME, this will be changed in the future
  GMSIPEndpoint *sip_ep =
    GnomeMeeting::Process ()->GetManager ()->GetSIPEndpoint ();

  if (sip_ep && sip_ep->IsSubscribed (SIPSubscribe::Presence, uri.c_str ())) {

    SIPSubscribe::SubscribeType type = SIPSubscribe::Presence;
    sip_ep->Subscribe (type, 0, PString (uri.c_str ()));

    uris.remove (uri);
  }
}

void
SIP::EndPoint::OnRegistered (const PString & _aor,
                             BOOL was_registering)
{
  // FIXME, this will be changed in the future
  GMSIPEndpoint *sip_ep =
    GnomeMeeting::Process ()->GetManager ()->GetSIPEndpoint ();
  std::string aor = (const char *) _aor;


  std::string::size_type found;
  std::string::size_type loc = aor.find ("@", 0);
  std::string server;

  if (loc != string::npos) {
    server = aor.substr (loc+1);

    if (server.empty ())
      return;

    if (was_registering
        && std::find (domains.begin (), domains.end (), server) == domains.end ()) 
      domains.push_back (server);

    if (!was_registering
        && std::find (domains.begin (), domains.end (), server) != domains.end ()) 
      domains.remove (server);

    for (std::list<std::string>::const_iterator iter = uris.begin (); 
         iter != uris.end () ; 
         iter++) {

      found = (*iter).find (server, 0);
      if (found != string::npos
          && sip_ep
          && ((was_registering && !sip_ep->IsSubscribed (SIPSubscribe::Presence, (*iter).c_str ()))
              || (!was_registering && sip_ep->IsSubscribed (SIPSubscribe::Presence, (*iter).c_str ())))) {

        SIPSubscribe::SubscribeType type = SIPSubscribe::Presence;
        sip_ep->Subscribe (type, was_registering ? 500 : 0, PString ((*iter).c_str ()));
        if (!was_registering)
          uris.remove (*iter);
      }
    }
  }
}


void 
SIP::EndPoint::OnPresenceInfoReceived (const PString & user,
                                       const PString & basic,
                                       const PString & note)
{
  PCaselessString b = basic;
  PCaselessString s = note;

  std::string status = "presence-unknown";
  std::string presence;

  SIPURL sip_uri = SIPURL (user);
  sip_uri.AdjustForRequestURI ();
  std::string uri = sip_uri.AsString ();

  if (b.Find ("Closed") != P_MAX_INDEX) {
    presence = "presence-offline";
    status = _("Offline");
  }
  else if (s.Find ("Ready") != P_MAX_INDEX
           || s.Find ("Online") != P_MAX_INDEX) {
    presence = "presence-online";
    status = _("Online");
  }
  else if (s.Find ("Away") != P_MAX_INDEX) {
    presence = "presence-away";
    status = _("Away");
  }
  else if (s.Find ("On the phone") != P_MAX_INDEX
           || s.Find ("Ringing") != P_MAX_INDEX
           || s.Find ("Do Not Disturb") != P_MAX_INDEX) {
    presence = "presence-dnd";
    status = _("Do Not Disturb");
  }
  else if (s.Find ("Free For Chat") != P_MAX_INDEX) {
    presence = "presence-freeforchat";
    status = _("Free For Chat");
  }

  Ekiga::PresenceCore *presence_core =
    dynamic_cast<Ekiga::PresenceCore *>(core.get ("presence-core"));
  Ekiga::Runtime *runtime = 
    GnomeMeeting::Process ()->GetRuntime (); // FIXME, should disappear

  if (runtime) {
   
    runtime->run_in_main (sigc::bind (presence_core->presence_received.make_slot (), uri, presence));
    runtime->run_in_main (sigc::bind (presence_core->status_received.make_slot (), uri, status));
  }
}

