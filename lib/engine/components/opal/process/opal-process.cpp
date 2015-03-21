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
 *                         gnomemeeting.cpp  -  description
 *                         --------------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the main class
 *
 */

#include "config.h"

#include "opal-process.h"

#include <gtk/gtk.h>

#include "runtime.h"

#include "call-core.h"

GnomeMeeting *GnomeMeeting::GM = 0;

/* The main GnomeMeeting Class  */
GnomeMeeting::GnomeMeeting () : PProcess("", "", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)
{
  GM = this;
}

GnomeMeeting::~GnomeMeeting ()
{
  std::cout << "PPROCESS END Start" << std::endl << std::flush;
  std::cout << "bank use count" << bank.use_count () << std::endl << std::flush;

  boost::shared_ptr<Ekiga::AccountCore> acore = account_core.lock ();
  // First remove all Opal::Accounts from our Bank.
  //
  // Do it forcibly so we're sure the accounts are freed before our
  // reference to the endpoints. Indeed they try to unregister from
  // presence when killed, and that gives a crash if the call manager
  // is already gone!
  /*
     bank->clear ();
     acore->remove_bank (bank);

     boost::shared_ptr<Ekiga::PresenceCore> pcore = presence_core.lock ();
     pcore->remove_presence_publisher (bank);
     pcore->remove_cluster (bank);

     core.remove (bank);

     std::cout << "bank use count" << bank.use_count () << std::endl << std::flush;
   */

  std::cout << "PPROCESS END END" << std::endl << std::flush;
}

GnomeMeeting *
GnomeMeeting::Process ()
{
  return GM;
}


void GnomeMeeting::Main ()
{
}


void GnomeMeeting::Start (Ekiga::ServiceCore& core)
{
  endpoint = new Opal::EndPoint (core);
}


Opal::EndPoint&
GnomeMeeting::GetEndPoint ()
{
  return *endpoint;
}


void GnomeMeeting::on_ready (
#ifdef HAVE_H323
                             Opal::H323::EndPoint* /*h323_endpoint*/,
#endif
                             Opal::Sip::EndPoint* /*sip_endpoint*/)
{
  /*
  presence_core = boost::weak_ptr<Ekiga::PresenceCore> (core.get<Ekiga::PresenceCore> ("presence-core"));
  account_core = boost::weak_ptr<Ekiga::AccountCore> (core.get<Ekiga::AccountCore> ("account-core"));

  bank = boost::shared_ptr<Opal::Bank> (new Opal::Bank (core,
#ifdef HAVE_H323
                                                        h323_endpoint,
#endif
                                                        sip_endpoint));

  boost::shared_ptr<Ekiga::AccountCore> acore = account_core.lock ();
  boost::shared_ptr<Ekiga::PresenceCore> pcore = presence_core.lock ();
  acore->add_bank (bank);
  pcore->add_cluster (bank);
  core.add (bank);
  pcore->add_presence_publisher (bank);
  */
}
