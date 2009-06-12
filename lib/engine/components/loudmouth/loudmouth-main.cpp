
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras
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
 *                         loudmouth-main.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : code to hook loudmouth into the main program
 *
 */

#include "loudmouth-main.h"

#include "presence-core.h"
#include "account-core.h"
#include "chat-core.h"
#include "personal-details.h"

#include "loudmouth-cluster.h"
#include "loudmouth-bank.h"

struct LOUDMOUTHSpark: public Ekiga::Spark
{
  LOUDMOUTHSpark (): result(false)
  {}

  bool try_initialize_more (Ekiga::ServiceCore& core,
			    int* /*argc*/,
			    char** /*argv*/[])
  {
    Ekiga::ServicePtr service(core.get ("loudmouth-bank"));
    gmref_ptr<Ekiga::PresenceCore> presence (core.get ("presence-core"));
    gmref_ptr<Ekiga::AccountCore> account (core.get ("account-core"));
    gmref_ptr<Ekiga::ChatCore> chat (core.get ("chat-core"));
    gmref_ptr<Ekiga::PersonalDetails> details (core.get ("personal-details"));

    if ( !service && presence && account && chat && details) {

      LM::DialectPtr dialect(new LM::Dialect);
      LM::ClusterPtr cluster(new LM::Cluster);
      LM::BankPtr bank (new LM::Bank (details, dialect, cluster));
      core.add (bank);
      chat->add_dialect (dialect);
      account->add_bank (bank);
      presence->add_cluster (cluster);
      result = true;
    }

    return result;
  }

  Ekiga::Spark::state get_state () const
  { return result?FULL:BLANK; }

  const std::string get_name () const
  { return "LOUDMOUTH"; }

  bool result;
};

void
loudmouth_init (Ekiga::KickStart& kickstart)
{
  gmref_ptr<Ekiga::Spark> spark(new LOUDMOUTHSpark);
  kickstart.add_spark (spark);
}
