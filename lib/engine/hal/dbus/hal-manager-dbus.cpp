
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras

 * This program is free software; you can  redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ekiga is licensed under the GPL license and as a special exception, you
 * have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OPAL, OpenH323 and PWLIB
 * programs, as long as you do follow the requirements of the GNU GPL for all
 * the rest of the software thus combined.
 */


/*
 *                         hal-manager-dbus.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the interface of a hal core.
 *                          A hal core manages HalManagers.
 *
 */

#include "hal-manager-dbus.h"
#include "hal-marshal.h"

//FIXME: for tracing
#include "ptbuildopts.h"
#include "ptlib.h"

HalManager_dbus::HalManager_dbus (Ekiga::ServiceCore & _core)
:    core (_core), runtime (*(dynamic_cast<Ekiga::Runtime *> (_core.get ("runtime"))))
{
  PTRACE(4, "HalManager_dbus\tInitialising HAL Manager");
  GError *error = NULL;
  bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
  if (error != NULL) {
    PTRACE (1, "HalManager_dbus\tConnecting to system bus failed: " << error->message);
    g_error_free(error);
    return;
  }

  hal_proxy = dbus_g_proxy_new_for_name (bus, "org.freedesktop.Hal",
                                              "/org/freedesktop/Hal/Manager",
                                              "org.freedesktop.Hal.Manager");

  //FIXME: Is this necessary?
//   dbus_g_object_register_marshaller(gm_hal_dbus_marshal_VOID__STRING, G_TYPE_NONE, G_TYPE_STRING, G_TYPE_INVALID);

  dbus_g_proxy_add_signal(hal_proxy, "DeviceRemoved", G_TYPE_STRING, G_TYPE_INVALID);
  dbus_g_proxy_connect_signal(hal_proxy, "DeviceRemoved", G_CALLBACK(&device_removed_cb_proxy), this, NULL);

  dbus_g_proxy_add_signal(hal_proxy, "DeviceAdded", G_TYPE_STRING, G_TYPE_INVALID);
  dbus_g_proxy_connect_signal(hal_proxy, "DeviceAdded", G_CALLBACK(&device_added_cb_proxy), this, NULL);

  populate_devices_list();

  nm_proxy = dbus_g_proxy_new_for_name (bus, "org.freedesktop.NetworkManager",
                                              "/org/freedesktop/NetworkManager",
                                              "org.freedesktop.NetworkManager");

  dbus_g_proxy_add_signal(nm_proxy, "DeviceNoLongerActive", DBUS_TYPE_G_OBJECT_PATH, G_TYPE_INVALID);
  dbus_g_proxy_connect_signal(nm_proxy, "DeviceNoLongerActive", G_CALLBACK(&device_no_longer_active_cb_proxy), this, NULL);

  dbus_g_proxy_add_signal(nm_proxy, "DeviceNowActive", DBUS_TYPE_G_OBJECT_PATH, G_TYPE_INVALID);
  dbus_g_proxy_connect_signal(nm_proxy, "DeviceNowActive", G_CALLBACK(&device_now_active_cb_proxy), this, NULL);

// The Main loop should be used from Ekiga itself
       static GMainLoop *loop = g_main_loop_new (NULL, FALSE);

       GMainContext *ctx = g_main_loop_get_context (loop);
       while (g_main_context_pending (ctx))
         g_main_context_iteration (ctx, FALSE);

        dbus_g_connection_flush (bus);
        g_main_loop_run (loop);
// The Main loop should be used from Ekiga itself
}

HalManager_dbus::~HalManager_dbus ()
{
  g_object_unref(hal_proxy);
  g_object_unref(nm_proxy);
  dbus_g_connection_unref(bus);
}

void HalManager_dbus::device_added_cb_proxy (DBusGProxy */*object*/, const char *device, gpointer user_data)
{
  HalManager_dbus* hal_manager_dbus = reinterpret_cast<HalManager_dbus *> (user_data);
  hal_manager_dbus->device_added_cb(device);
}

void HalManager_dbus::device_removed_cb_proxy (DBusGProxy */*object*/, const char *device, gpointer user_data)
{
  HalManager_dbus* hal_manager_dbus = reinterpret_cast<HalManager_dbus *> (user_data);
  hal_manager_dbus->device_removed_cb(device);
}

void HalManager_dbus::device_now_active_cb_proxy (DBusGProxy */*object*/, const char *device, gpointer user_data)
{
  HalManager_dbus* hal_manager_dbus = reinterpret_cast<HalManager_dbus *> (user_data);
  hal_manager_dbus->device_now_active_cb(device);
}

void HalManager_dbus::device_no_longer_active_cb_proxy (DBusGProxy */*object*/, const char *device, gpointer user_data)
{
  HalManager_dbus* hal_manager_dbus = reinterpret_cast<HalManager_dbus *> (user_data);
  hal_manager_dbus->device_no_longer_active_cb(device);
}

void HalManager_dbus::device_added_cb (const char *device)
{
  std::string type, name;
  HalDevice hal_device;
  hal_device.key = device;
  get_device_type_name(device, hal_device.category, hal_device.product, hal_device.parent_product);
  hal_devices.push_back(hal_device);
  PTRACE(4, "HalManager_dbus\tAdded device " << hal_device.category << "," << hal_device.product << " - " << hal_device.parent_product);
}

void HalManager_dbus::device_removed_cb (const char *device)
{
  bool found = false;
  std::vector<HalDevice>::iterator iter;

  for (iter = hal_devices.begin ();
       iter != hal_devices.end () ;
       iter++)
      if (iter->key == device) { 
        found = true;
        break;
      }

  if (found) {
    PTRACE(4, "HalManager_dbus\tRemoved device " << iter->category << "," << iter->product << " - " << iter->parent_product);
    hal_devices.erase(iter);
  }
}

void HalManager_dbus::device_now_active_cb (const char *device)
{
  PTRACE(4, "HalManager_dbus\tActivated network device " << device);
}

void HalManager_dbus::device_no_longer_active_cb (const char *device)
{
  PTRACE(4, "HalManager_dbus\tDeactivated network device " << device);
}

void HalManager_dbus::get_string_property(DBusGProxy *proxy, const char * property, std::string & value)
{
  char* c_value = NULL;
  GError *error = NULL;

  dbus_g_proxy_call (proxy, "GetPropertyString", &error, G_TYPE_STRING, property, G_TYPE_INVALID, G_TYPE_STRING, &c_value, G_TYPE_INVALID);

  if (error != NULL)
    g_error_free(error);
   else
     if (c_value) 
       value = c_value;

  g_free (c_value);
}

void HalManager_dbus::get_device_type_name (const char * device, std::string & category, std::string & product, std::string & parent_product)
{
  DBusGProxy * device_proxy = NULL;
  DBusGProxy * parent_proxy = NULL;
  device_proxy = dbus_g_proxy_new_for_name (bus, "org.freedesktop.Hal",
                                                 device,
                                                 "org.freedesktop.Hal.Device");
  std::string parent;
  get_string_property(device_proxy, "info.category", category);
  get_string_property(device_proxy, "info.product", product);
  get_string_property(device_proxy, "info.parent", parent);

  parent_proxy = dbus_g_proxy_new_for_name (bus, "org.freedesktop.Hal",
                                                 parent.c_str(),
                                                 "org.freedesktop.Hal.Device");

  get_string_property(parent_proxy, "info.product", parent_product);

  g_object_unref(device_proxy);
  g_object_unref(parent_proxy);
}

void HalManager_dbus::populate_devices_list ()
{
  GError *error = NULL;

  PTRACE(4, "HalManager_dbus\tPopulating device list");
  char **device_list;
  char **device_list_ptr;
  HalDevice hal_device;

  dbus_g_proxy_call (hal_proxy, "GetAllDevices", &error, G_TYPE_INVALID, G_TYPE_STRV, &device_list, G_TYPE_INVALID);
  if (error != NULL) {
    PTRACE(1, "HalManager_dbus\tPopulating full device list failed - " << error->message);
    g_error_free(error);
    return;
  }

  for (device_list_ptr = device_list; *device_list_ptr; device_list_ptr++) {
    hal_device.key = *device_list_ptr;

    if (hal_device.key != "/org/freedesktop/Hal/devices/computer") {
      get_device_type_name(*device_list_ptr, hal_device.category, hal_device.product, hal_device.parent_product);
      hal_devices.push_back(hal_device);
    }
  }

  g_strfreev(device_list);
  PTRACE(4, "HalManager_dbus\tPopulated device list with " << hal_devices.size() << " devices");
}