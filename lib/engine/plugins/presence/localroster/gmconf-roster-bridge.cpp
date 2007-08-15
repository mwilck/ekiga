
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
 *                         gmconf-roster-bridge.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : code to push contacts into the gmconf roster
 *
 */

#include <iostream>

#include "gmconf-roster-bridge.h"
#include "contact-core.h"
#include "gmconf-cluster.h"

/* declaration&implementation of the bridge */

namespace GMConf
{
  class ContactDecorator:
    public Ekiga::Service,
    public Ekiga::ContactDecorator
  {
  public:

    ContactDecorator (Cluster &_cluster): cluster(_cluster)
    {}

    ~ContactDecorator ()
    {
#ifdef __GNUC__
      std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
    }

    const std::string get_name () const
    { return "gmconf-roster-bridge"; }

    const std::string get_description () const
    { return "\tComponent to push contacts into the gmconf roster"; }

    void populate_menu (Ekiga::Contact &contact,
			Ekiga::MenuBuilder &builder);

  private:

    Cluster &cluster;
  };
};


void
GMConf::ContactDecorator::populate_menu (Ekiga::Contact &contact,
                                         Ekiga::MenuBuilder &builder)
{
  std::list<std::pair<std::string, std::string> > uris
    = contact.get_uris ();

  for (std::list<std::pair<std::string, std::string> >::iterator iter
       = uris.begin ();
       iter != uris.end ();
       iter++) {

    if (cluster.is_supported_uri (iter->second)) {

      Cluster::iterator heapiter = cluster.begin (); // no loop : only one

      if (!heapiter->has_presentity_with_uri (iter->second)) {

        if (iter->first.empty ()) {

          builder.add_action ("Add to internal roster",
                              sigc::bind (sigc::mem_fun (*heapiter, &GMConf::Heap::build_new_presentity_form),
                                          contact.get_name (), iter->second));
        } else {

          builder.add_action ("Add ("
                              + iter->first + ") to internal roster ",
                              sigc::bind (sigc::mem_fun (*heapiter, &GMConf::Heap::build_new_presentity_form),
                                          contact.get_name () + "(" + iter->first + ")", iter->second));
        }
      }
    }
  }
}


/* public api */
bool
gmconf_roster_bridge_init (Ekiga::ServiceCore &core,
			   int * /*argc*/,
			   char ** /*argv*/[])
{
  bool result = false;
  Ekiga::ContactCore *contact_core = NULL;
  GMConf::Cluster *cluster = NULL;
  GMConf::ContactDecorator *decorator = NULL;

  contact_core
    = dynamic_cast<Ekiga::ContactCore*>(core.get ("contact-core"));

  cluster
    = dynamic_cast<GMConf::Cluster*>(core.get ("gmconf-cluster"));

  if (cluster != NULL && contact_core != NULL) {

    decorator = new GMConf::ContactDecorator (*cluster);
    core.add (*decorator);
    contact_core->add_contact_decorator (*decorator);
    result = true;
  }

  return result;
}
