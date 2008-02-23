
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
 *                         engine.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : Vroom.
 *
 */

#include "config.h"

#include "engine.h"

#include "services.h"

#include "presence-core.h"
#include "contact-core.h"
#include "call-core.h"
#include "display-core.h"
#include "vidinput-core.h"
#include "hal-core.h"
#include "history-main.h"
#include "local-roster-main.h"
#include "local-roster-bridge.h"
#include "gtk-core-main.h"
#include "gtk-frontend.h"
#include "gmconf-personal-details-main.h"

#ifndef WIN32
#include "display-main-x.h"
#endif

#ifdef HAVE_DX
#include "display-main-dx.h"
#endif

#include "vidinput-main-mlogo.h"

#include "vidinput-main-ptlib.h"

#ifdef HAVE_DBUS
#include "hal-main-dbus.h"
#endif

#include "opal-main.h"

#ifdef HAVE_AVAHI
#include "avahi-main.h"
#endif

#ifdef HAVE_EDS
#include "evolution-main.h"
#endif

#ifdef HAVE_LDAP
#include "ldap-main.h"
#endif


void
engine_init (int argc,
             char *argv [],
             Ekiga::Runtime *runtime,
             Ekiga::ServiceCore * &core)
{
  core = new Ekiga::ServiceCore; 
  Ekiga::PresenceCore *presence_core = new Ekiga::PresenceCore;
  Ekiga::ContactCore *contact_core = new Ekiga::ContactCore;
  Ekiga::CallCore *call_core = new Ekiga::CallCore;
  Ekiga::DisplayCore *display_core = new Ekiga::DisplayCore;
  Ekiga::VidInputCore *vidinput_core = new Ekiga::VidInputCore(*display_core);
  Ekiga::HalCore *hal_core = new Ekiga::HalCore;

  core->add (*contact_core);
  core->add (*presence_core);
  core->add (*call_core);
  core->add (*display_core);
  core->add (*vidinput_core);
  core->add (*hal_core);
  core->add (*runtime);

  if (!gmconf_personal_details_init (*core, &argc, &argv)) {
    delete core;
    return;
  }

#ifndef WIN32
  if (!display_x_init (*core, &argc, &argv)) {
    delete core;
    return;
  }
#endif

#ifdef HAVE_DX
  if (!display_dx_init (*core, &argc, &argv)) {
    delete core;
    return;
  }
#endif

  if (!vidinput_mlogo_init (*core, &argc, &argv)) {
    delete core;
    return;
  }

  if (!vidinput_ptlib_init (*core, &argc, &argv)) {
    delete core;
    return;
  }

#ifdef HAVE_DBUS
// Do not use the dbus HAL for now until the main loop is moved
//  if (!hal_dbus_init (*core, &argc, &argv)) {
//    delete core;
//    return;
//  }
#endif

  if (!opal_init (*core, &argc, &argv)) {
    delete core;
    return;
  }

#ifdef HAVE_AVAHI
  if (!avahi_init (*core, &argc, &argv)) {
    delete core;
    return;
  }
#endif

#ifdef HAVE_EDS
  if (!evolution_init (*core, &argc, &argv)) {
    delete core;
    return;
  }
#endif

#ifdef HAVE_LDAP
  if (!ldap_init (*core, &argc, &argv)) {
    delete core;
    return;
  }
#endif

  if (!history_init (*core, &argc, &argv)) {

    delete core;
    return;
  }

  if (!gtk_core_init (*core, &argc, &argv)) {
    delete core;
    return;
  }

  if (!gtk_frontend_init (*core, &argc, &argv)) {
    delete core;
    return;
  }

  if (!local_roster_init (*core, &argc, &argv)) {
    delete core;
    return;
  }

  if (!local_roster_bridge_init (*core, &argc, &argv)) {
    delete core;
    return;
  }

  vidinput_core->setup_conf_bridge();
}
