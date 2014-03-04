
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2014 Damien Sandras <dsandras@seconix.com>
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
 *                         hal-gudev-monitor.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2014 by Julien Puydt
 *   copyright            : (c) 2014 Julien Puydt
 *   description          : implementation of the GUDev device monitor
 *
 */

#include "hal-gudev-monitor.h"

#define DEBUG 1

#if DEBUG
static void
print_gudev_device (GUdevDevice* device)
{
  g_print ("GUdevDevice:\n");
  g_print ("\tname: %s\n", g_udev_device_get_name (device));
  g_print ("\tfile: %s\n", g_udev_device_get_device_file (device));

  g_print ("\tproperties:\n");
  const gchar* const *keys = g_udev_device_get_property_keys (device);
  for (int num = 0;
       keys[num] != NULL;
       num++) {

    const gchar* key = keys[num];
    const gchar* val = g_udev_device_get_property (device, key);
    g_print ("\t\t%s -> %s\n", key, val);
  }
}
#endif

void
gudev_monitor_videoinput_uevent_handler (G_GNUC_UNUSED GUdevClient* client,
					 const gchar* action,
					 GUdevDevice* device,
					 GUDevMonitor* monitor)
{
  if (g_str_equal ("remove", action)) {

    monitor->videoinput_remove (device);
  }
  if (g_str_equal ("add", action)) {

    monitor->videoinput_add (device);
  }
}

GUDevMonitor::GUDevMonitor ()
{
  const gchar* videoinput_subsystems[] = {"video4linux", NULL};
  videoinput = g_udev_client_new (videoinput_subsystems);
  g_signal_connect (G_OBJECT (videoinput), "uevent",
		    G_CALLBACK (gudev_monitor_videoinput_uevent_handler), this);
}

GUDevMonitor::~GUDevMonitor ()
{
  g_object_unref (videoinput);
}

void
GUDevMonitor::videoinput_add (GUdevDevice* device)
{
#if DEBUG
  g_print ("%s\n", __PRETTY_FUNCTION__);
  print_gudev_device (device);
#endif
  gint v4l_version = 0;

  // first check the api version
  v4l_version = g_udev_device_get_property_as_int (device, "ID_V4L_VERSION");

  if (v4l_version == 1 || v4l_version == 2) {

    // then check it can actually capture video
    const char* caps = g_udev_device_get_property (device, "ID_V4L_CAPABILITIES");
    if (caps != NULL && g_strstr_len (caps, -1, ":capture:") != NULL) {

      // we're almost good!
      const gchar* file = g_udev_device_get_device_file (device);

      if (file != NULL) {
	VideoInputDevice vdevice = {"video4linux", file, v4l_version};
	videoinput_devices.push_back(vdevice);
	videoinput_device_added ("video4linux", file, v4l_version);
      }
    }
  }
}

void
GUDevMonitor::videoinput_remove (GUdevDevice* device)
{
  const gchar* file = g_udev_device_get_device_file (device);
  bool found = false;
  std::vector<VideoInputDevice>::iterator iter;

  for (iter = videoinput_devices.begin ();
       iter != videoinput_devices.end ();
       ++iter) {

    if (iter->name == file) {

      found = true;
      break;
    }
  }
  if (found) {

    videoinput_device_removed (iter->framework, iter->name, iter->caps);
    videoinput_devices.erase (iter);
  }
}
