
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
 *                         opal-main.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras 
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : code to hook Opal into the main program
 *
 */

#include "opal-main.h"
#include "contact-core.h"
#include "presence-core.h"
#include "call-core.h"
#include "display-core.h"

#include "manager.h"
#include "ekiga.h"
#include "sip.h"


static bool
is_sip_address (const std::string uri)
{
  return (uri.find ("sip:") == 0);
}


bool
opal_init (Ekiga::ServiceCore &core,
           int * /*argc*/,
           char ** /*argv*/[])
{
  bool result = true;

  Ekiga::ContactCore *contact_core = NULL;
  Ekiga::PresenceCore *presence_core = NULL;
  Ekiga::CallCore *call_core = NULL;
  Ekiga::DisplayCore *display_core = NULL;
  
  //GMDisplayManager *displayManager = new GMDisplayManager_X(core);
  contact_core = dynamic_cast<Ekiga::ContactCore *> (core.get ("contact-core"));
  presence_core = dynamic_cast<Ekiga::PresenceCore *> (core.get ("presence-core"));
  call_core = dynamic_cast<Ekiga::CallCore *> (core.get ("call-core"));
  display_core = dynamic_cast<Ekiga::DisplayCore *> (core.get ("display-core"));

  GMManager *manager = new GMManager (core);
  GMSIPEndpoint *sipEP = manager->GetSIPEndpoint ();

  call_core->add_manager (*manager);
  core.add (*manager); // FIXME temporary

  if (contact_core != NULL) 
    contact_core->add_contact_decorator (*sipEP);
  else
    return false;

  if (presence_core != NULL) {

    presence_core->add_presentity_decorator (*sipEP);
    presence_core->add_supported_uri (sigc::ptr_fun (is_sip_address));
    presence_core->add_presence_fetcher (*sipEP);
    presence_core->add_presence_publisher (*sipEP);
  }
  else 
    return false;

/*  if (display_core != NULL)
    display_core->add_manager (*displayManager);
  else 
    return false;
*/


  return result;
}
