/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2015 Damien Sandras <dsandras@seconix.com>
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
 *                         opal-call-manager.h  -  description
 *                         -----------------------------------
 *   begin                : Sat Dec 23 2000
 *   authors              : Damien Sandras
 *   description          : This file contains the engine CallManager.
 *
 */


#ifndef __OPAL_CALL_MANAGER_H_
#define __OPAL_CALL_MANAGER_H_

#include "call-manager.h"
#include "opal-endpoint.h"

#include "ekiga-settings.h"

namespace Opal {

  /* This is the engine CallManager implementation.
   * It uses the Opal::Manager object to implement the engine
   * CallManager interface.
   */
  class CallManager : public Ekiga::CallManager
  {
public:
    CallManager (Ekiga::ServiceCore& _core,
                 Opal::EndPoint& _manager);
    ~CallManager ();

    /* CallManager Methods */
    bool dial (const std::string & uri);
    void hang_up ();
    bool is_supported_uri (const std::string & uri);
    void set_codecs (Ekiga::CodecList & codecs);
    const Ekiga::CodecList & get_codecs () const;
    void set_display_name (const std::string & name);
    const std::string & get_display_name () const;

    void set_echo_cancellation (bool enabled);
    bool get_echo_cancellation () const;
    void set_silence_detection (bool enabled);
    bool get_silence_detection () const;
    void set_maximum_jitter (unsigned max_val);
    unsigned get_maximum_jitter () const;
    void set_reject_delay (unsigned delay);
    unsigned get_reject_delay () const;
    void set_auto_answer (bool enabled);
    bool get_auto_answer () const;

    /* Set up endpoint: all options or a specific setting */
    virtual void setup (const std::string & setting = "");

private:
    /* We use a callback instead of directly connecting the signal
     * to the add_call method of the CallCore to prevent boost to
     * keep a reference to the CallCore until the ECallManager is
     * destroyed, which would be a crossed reference.
     */
    void on_created_call (Opal::Call *call);

    Ekiga::SettingsPtr nat_settings;
    Ekiga::SettingsPtr audio_codecs_settings;
    Ekiga::SettingsPtr video_codecs_settings;
    Ekiga::SettingsPtr video_devices_settings;
    Ekiga::SettingsPtr protocols_settings;
    Ekiga::SettingsPtr ports_settings;
    Ekiga::SettingsPtr call_options_settings;
    Ekiga::SettingsPtr call_forwarding_settings;
    Ekiga::SettingsPtr personal_data_settings;

    std::string display_name;

protected:
    Ekiga::ServiceCore& core;
    EndPoint& endpoint;
  };
};
#endif
