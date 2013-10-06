
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2013 Damien Sandras <dsandras@seconix.com>
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
 *                         ekiga-ettings.h  -  description
 *                         -------------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2013 by Damien Sandras
 *   description          : This file defines gsettings keys
 *
 */


#ifndef EKIGA_SETTINGS_H_
#define EKIGA_SETTINGS_H_

#include <boost/smart_ptr.hpp>
#include <boost/signals2.hpp>

#include <gio/gio.h>

#include "config.h"

#define USER_INTERFACE "org.gnome." PACKAGE_NAME ".general.user-interface"

#define PERSONAL_DATA_SCHEMA "org.gnome." PACKAGE_NAME ".general.personal-data"
#define SOUND_EVENTS_SCHEMA "org.gnome." PACKAGE_NAME ".general.sound-events"
#define AUDIO_DEVICES_SCHEMA "org.gnome." PACKAGE_NAME ".devices.audio"
#define VIDEO_DEVICES_SCHEMA "org.gnome." PACKAGE_NAME ".devices.video"
#define VIDEO_DISPLAY_SCHEMA USER_INTERFACE ".video-display"
#define PROTOCOLS_SCHEMA "org.gnome." PACKAGE_NAME ".protocols"
#define SIP_SCHEMA PROTOCOLS_SCHEMA ".sip"

namespace Ekiga {

  /*
   * This is a C++ wrapper around the GSettings signal function.
   *
   * Please use it in C++ code.
   */
  class Settings : boost::noncopyable {

    static void f_callback (G_GNUC_UNUSED GSettings *settings,
                            gchar *key,
                            const Settings* self)
    {
      self->changed (std::string (key));
    }


public:

    Settings (const std::string & _schema)
    {
      gsettings = g_settings_new (_schema.c_str ());
      handler = g_signal_connect (gsettings, "changed", G_CALLBACK (&f_callback), this);
    }

    ~Settings ()
    {
      g_signal_handler_disconnect (gsettings, handler);
      g_clear_object (&gsettings);
    }

    GSettings* get_g_settings ()
    {
      return gsettings;
    }

    const std::string get_string (const std::string & key)
    {
      gchar *value = g_settings_get_string (gsettings, key.c_str ());
      std::string result;

      if (value)
	result = value;

      g_free (value);
      return result;
    }

    void set_string (const std::string & key, const std::string & value)
    {
      g_settings_set_string (gsettings, key.c_str (), value.c_str ());
    }
    
    int get_int (const std::string & key)
    {
      return g_settings_get_int (gsettings, key.c_str ());
    }

    void set_int (const std::string & key, int i)
    {
      g_settings_set_int (gsettings, key.c_str (), i);
    }

    bool get_bool (const std::string & key)
    {
      return g_settings_get_boolean (gsettings, key.c_str ());
    }

    void set_bool (const std::string & key, bool i)
    {
      g_settings_set_boolean (gsettings, key.c_str (), i);
    }

    boost::signals2::signal<void(std::string)> changed;

private:
    gulong handler;
    GSettings *gsettings;
  };
}

#endif /* EKIGA_SETTINGS_H */
