
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
 *                         opal-main.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : code to hook Opal into the main program
 *
 */

#include "config.h"

#include "opal-main.h"
#include "opal-process.h"

#include "account-core.h"
#include "chat-core.h"
#include "presence-core.h"
#include "audioinput-core.h"
#include "audiooutput-core.h"
#include "videoinput-core.h"
#include "videooutput-core.h"

#include "opal-plugins-hook.h"

#include "sip-endpoint.h"
#ifdef HAVE_H323
#include "h323-endpoint.h"
#endif

// opal manages its endpoints itself, so we must be wary
struct null_deleter
{
    void operator()(void const *) const
    { }
};


/* FIXME: add here an Ekiga::Service which will add&remove publishers,
 * and fetchers
 */
using namespace Opal;

struct OPALSpark: public Ekiga::Spark
{
  OPALSpark (): result(false)
  {}

  bool try_initialize_more (Ekiga::ServiceCore& core,
			    int* /*argc*/,
			    char** /*argv*/[])
  {
    boost::shared_ptr<Ekiga::ContactCore> contact_core = core.get<Ekiga::ContactCore> ("contact-core");
    boost::shared_ptr<Ekiga::PresenceCore> presence_core = core.get<Ekiga::PresenceCore> ("presence-core");
    boost::shared_ptr<Ekiga::CallCore> call_core = core.get<Ekiga::CallCore> ("call-core");
    boost::shared_ptr<Ekiga::ChatCore> chat_core = core.get<Ekiga::ChatCore> ("chat-core");
    boost::shared_ptr<Ekiga::AccountCore> account_core = core.get<Ekiga::AccountCore> ("account-core");
    boost::shared_ptr<Ekiga::AudioInputCore> audioinput_core = core.get<Ekiga::AudioInputCore> ("audioinput-core");
    boost::shared_ptr<Ekiga::VideoInputCore> videoinput_core = core.get<Ekiga::VideoInputCore> ("videoinput-core");
    boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core = core.get<Ekiga::AudioOutputCore> ("audiooutput-core");
    boost::shared_ptr<Ekiga::VideoOutputCore> videooutput_core = core.get<Ekiga::VideoOutputCore> ("videooutput-core");
    boost::shared_ptr<Ekiga::PersonalDetails> personal_details = core.get<Ekiga::PersonalDetails> ("personal-details");
    boost::shared_ptr<Bank> account_store = core.get<Bank> ("opal-account-store");

    if (contact_core && presence_core && call_core && chat_core
	&& account_core && audioinput_core && videoinput_core
	&& audiooutput_core && videooutput_core && personal_details
	&& !account_store) {

      hook_ekiga_plugins_to_opal (core);

      boost::shared_ptr<CallManager> call_manager (new CallManager (core));

      boost::shared_ptr<Bank> bank (new Bank (core, *call_manager.get ()));
      account_core->add_bank (bank);
      presence_core->add_cluster (bank);
      core.add (bank);
      call_manager->setup ();
      presence_core->add_presence_publisher (bank);

      call_core->add_manager (call_manager);

      result = true;
    }

    return result;
  }

  Ekiga::Spark::state get_state () const
  { return result?FULL:BLANK; }

  const std::string get_name () const
  { return "OPAL"; }

  bool result;
};

void
opal_init_pprocess (int argc,
                    char *argv [])
{
  /* Ekiga PTLIB Process initialisation */
  static GnomeMeeting instance;
  instance.GetArguments ().SetArgs (argc, argv);
  PArgList & args = instance.GetArguments ();
  args.Parse ("d-debug:", false);

  if (args.IsParsed ()) {
    int debug_level = args.GetOptionAs ('d', 0);
    if (debug_level == 0)
      return;
#ifndef WIN32
    char* text_label =  g_strdup_printf ("%d", debug_level);
    setenv ("PTLIB_TRACE_CODECS", text_label, TRUE);
    g_free (text_label);
#else
    char* text_label =  g_strdup_printf ("PTLIB_TRACE_CODECS=%d", debug_level);
    _putenv (text_label);
    g_free (text_label);
    if (debug_level != 0) {
      std::string desk_path = g_get_user_special_dir (G_USER_DIRECTORY_DESKTOP);
      if (!desk_path.empty ())
        std::freopen((desk_path + "\\ekiga-stderr.txt").c_str (), "w", stderr);
    }
#endif

#if PTRACING
    PTrace::Initialise (PMAX (PMIN (8, debug_level), 0), NULL,
                        PTrace::Timestamp | PTrace::Thread
                        | PTrace::Blocks | PTrace::DateAndTime);
    PTRACE (1, "Ekiga version "
            << MAJOR_VERSION << "." << MINOR_VERSION << "." << BUILD_NUMBER);
#ifdef EKIGA_REVISION
    PTRACE (1, "Ekiga git revision: " << EKIGA_REVISION);
#endif
    PTRACE (1, "PTLIB version " << PTLIB_VERSION);
    PTRACE (1, "OPAL version " << OPAL_VERSION);
#ifdef HAVE_DBUS
    PTRACE (1, "DBUS support enabled");
#else
    PTRACE (1, "DBUS support disabled");
#endif
#endif
  }
}

void
opal_init (Ekiga::KickStart& kickstart)
{
  boost::shared_ptr<Ekiga::Spark> spark(new OPALSpark);
  kickstart.add_spark (spark);
}
