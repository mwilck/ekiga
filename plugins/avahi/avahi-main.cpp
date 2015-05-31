/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2012 Damien Sandras <dsandras@seconix.com>
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
 *                         avahi-main.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2012 by Julien Puydt
 *   copyright            : (c) 2012 Julien Puydt
 *   description          : declare the avahi code as a plugin
 *
 */

#include "kickstart.h"
#include "avahi-publisher.h"
#include "avahi-cluster.h"

struct AVAHISpark: public Ekiga::Spark
{
  AVAHISpark (): result(false)
  {}

  bool try_initialize_more (Ekiga::ServiceCore& core,
			    int* /*argc*/,
			    char** /*argv*/[])
  {
    boost::shared_ptr<Ekiga::PresenceCore> presence_core = core.get<Ekiga::PresenceCore> ("presence-core");
    boost::shared_ptr<Ekiga::CallCore> call_core = core.get<Ekiga::CallCore> ("call-core");
    boost::shared_ptr<Ekiga::PersonalDetails> details = core.get<Ekiga::PersonalDetails> ("personal-details");

    if (presence_core && call_core && details) {

      boost::shared_ptr<Avahi::PresencePublisher> publisher (new Avahi::PresencePublisher (core, *details, *call_core));
      if (core.add (publisher)) {

	presence_core->add_presence_publisher (publisher);
	result = true;
      }

      boost::shared_ptr<Avahi::Cluster> cluster = Avahi::Cluster::create (core);
      if (core.add (cluster)) {
	presence_core->add_cluster (cluster);
	result = true;
      }
    }

    return result;
  }

  Ekiga::Spark::state get_state () const
  { return result?FULL:BLANK; }

  const std::string get_name () const
  { return "AVAHI"; }

  bool result;
};

extern "C" void
ekiga_plugin_init (Ekiga::KickStart& kickstart)
{
  boost::shared_ptr<Ekiga::Spark> spark(new AVAHISpark);
  kickstart.add_spark (spark);
}
