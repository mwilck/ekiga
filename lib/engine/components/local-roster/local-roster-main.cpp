
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
 *                         local-roster-main.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : code to hook the local roster to the main program
 *
 */

#include "local-roster-main.h"
#include "presence-core.h"
#include "friend-or-foe.h"
#include "local-cluster.h"

struct LOCALROSTERSpark: public Ekiga::Spark
{
  LOCALROSTERSpark (): result(false)
  {}

  bool try_initialize_more (Ekiga::ServiceCore& core,
			    int* /*argc*/,
			    char** /*argv*/[])
  {
    boost::shared_ptr<Ekiga::PresenceCore> presence_core = core.get<Ekiga::PresenceCore> ("presence-core");
    boost::shared_ptr<Ekiga::FriendOrFoe> iff = core.get<Ekiga::FriendOrFoe> ("friend-or-foe");

    if (presence_core && iff) {

      boost::shared_ptr<Local::Cluster> cluster (new Local::Cluster (presence_core));
      boost::shared_ptr<Local::Heap> heap(new Local::Heap (presence_core, cluster));
      if (core.add (cluster)) {

	iff->add_helper (heap);
	cluster->set_heap (heap);
	presence_core->add_cluster (cluster);
	result = true;
      }
    }

    return result;
  }

  Ekiga::Spark::state get_state () const
  { return result?FULL:BLANK; }

  const std::string get_name () const
  { return "LOCALROSTER"; }

  bool result;
};

void
local_roster_init (Ekiga::KickStart& kickstart)
{
  boost::shared_ptr<Ekiga::Spark> spark(new LOCALROSTERSpark);
  kickstart.add_spark (spark);
}
