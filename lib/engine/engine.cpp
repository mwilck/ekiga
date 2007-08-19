
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
#include "local-roster-main.h"
#include "local-roster-bridge.h"
#include "gtk-core-main.h"
#include "gtk-frontend.h"

#include "sip-main.h"

#ifdef HAVE_EDS
#include "evolution-main.h"
#endif

bool
engine_init (int argc,
             char *argv [])
{
  Ekiga::ServiceCore *core = new Ekiga::ServiceCore; // FIXME: leaked
  Ekiga::PresenceCore *presence_core = new Ekiga::PresenceCore;
  Ekiga::ContactCore *contact_core = new Ekiga::ContactCore;

  core->add (*contact_core);
  core->add (*presence_core);

  if (!sip_init (*core, &argc, &argv))
    return false;

#ifdef HAVE_EDS
  if (!evolution_init (*core, &argc, &argv))
    return false;
#endif

  if (!gtk_core_init (*core, &argc, &argv))
    return false;

  if (!gtk_frontend_init (*core, &argc, &argv))
    return false;

  if (!local_roster_init (*core, &argc, &argv))
    return false;

  if (!local_roster_bridge_init (*core, &argc, &argv))
    return false;

}
