
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
 *                         sip-endpoint.h  -  description
 *                         ------------------------------
 *   begin                : written in 2007 by Damien Sandras 
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : Code to provide a basic SIP endpoint
 *                          responsible of registering accounts,
 *                          presence, doing calls, ...
 *
 */

#ifndef __SIP_ENDPOINT_H__
#define __SIP_ENDPOINT_H__

#include "services.h"

#include <opal/buildopts.h>
#include <ptbuildopts.h>

#include <ptlib.h>

#include <opal/manager.h>
#include <opal/pcss.h>


namespace SIP
{
  class EndPoint : public Ekiga::Service
  {
  public:

    EndPoint (Ekiga::ServiceCore & _core) : core (_core)
    {}

    ~EndPoint ();

    const std::string get_name () const
    { return "sip-endpoint"; }

    const std::string get_description () const
    { return "\tObject bringing in basic SIP support"; }


    void call (std::string uri);

    void message (std::string uri,
                  std::string message);

    void subscribe (std::string uri,
                    int expire = 500);

    void unsubscribe (std::string uri);

    void OnRegistered (const PString & aor,
                       bool was_registering);

    void OnPresenceInfoReceived (const PString & user,
                                 const PString & basic,
                                 const PString & note);

  private:
    std::list <std::string> uris;    // List of subscribed uris
    std::list <std::string> domains; // List of registered domains
    Ekiga::ServiceCore & core;
  };
};

#endif
