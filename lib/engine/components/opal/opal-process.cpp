
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
GnomeMeeting::GnomeMeeting (Ekiga::ServiceCore& _core)
  : PProcess("", "", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER),
    core(_core)
{
  GM = this;
}

GnomeMeeting::~GnomeMeeting ()
{
  boost::shared_ptr<Ekiga::AccountCore> acore = account_core.lock ();
  boost::shared_ptr<Ekiga::PresenceCore> pcore = presence_core.lock ();

  acore->remove_bank (bank);
  pcore->remove_presence_publisher (bank);
  pcore->remove_cluster (bank);

  core.remove (bank);

  boost::shared_ptr<Ekiga::CallCore> ccore = call_core.lock ();
  ccore->remove_manager (call_manager);
}

GnomeMeeting *
GnomeMeeting::Process ()
{
  return GM;
}


void GnomeMeeting::Main ()
{
}


void GnomeMeeting::Start ()
{
  call_core = boost::weak_ptr<Ekiga::CallCore> (core.get<Ekiga::CallCore> ("call-core"));
  presence_core = boost::weak_ptr<Ekiga::PresenceCore> (core.get<Ekiga::PresenceCore> ("presence-core"));
  account_core = boost::weak_ptr<Ekiga::AccountCore> (core.get<Ekiga::AccountCore> ("account-core"));

  boost::shared_ptr<Ekiga::AccountCore> acore = account_core.lock ();
  boost::shared_ptr<Ekiga::PresenceCore> pcore = presence_core.lock ();
  boost::shared_ptr<Ekiga::CallCore> ccore = call_core.lock ();

  call_manager = boost::shared_ptr<Opal::CallManager> (new Opal::CallManager (core));
  bank = boost::shared_ptr<Opal::Bank> (new Opal::Bank (core, call_manager));

  acore->add_bank (bank);
  pcore->add_cluster (bank);
  core.add (bank);
  call_manager->setup ();
  pcore->add_presence_publisher (bank);

  ccore->add_manager (call_manager);
}
