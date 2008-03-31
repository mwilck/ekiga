
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
 *                         vidinput-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : declaration of the interface of a vidinput core.
 *                          A vidinput core manages VidInputManagers.
 *
 */

#include "config.h"

#include "vidinput-gmconf-bridge.h"
#include "vidinput-core.h"

#define VIDEO_DEVICES_KEY "/apps/" PACKAGE_NAME "/devices/video/"
#define VIDEO_CODECS_KEY  "/apps/" PACKAGE_NAME "/codecs/video/"

using namespace Ekiga;

VidInputCoreConfBridge::VidInputCoreConfBridge (Ekiga::Service & _service)
 : Ekiga::ConfBridge (_service)
{
  Ekiga::ConfKeys keys;
  property_changed.connect (sigc::mem_fun (this, &VidInputCoreConfBridge::on_property_changed));

  keys.push_back (VIDEO_DEVICES_KEY "size"); 
  keys.push_back (VIDEO_CODECS_KEY "max_frame_rate"); 
  keys.push_back (VIDEO_DEVICES_KEY "input_device"); 
  keys.push_back (VIDEO_DEVICES_KEY "plugin"); 
  keys.push_back (VIDEO_DEVICES_KEY "channel"); 
  keys.push_back (VIDEO_DEVICES_KEY "format"); 
  keys.push_back (VIDEO_DEVICES_KEY "image"); 
  keys.push_back (VIDEO_DEVICES_KEY "enable_preview");
  load (keys);
}

void VidInputCoreConfBridge::on_property_changed (std::string key, GmConfEntry *entry)
{
  VidInputCore & vidinput_core = (VidInputCore &) service;

  if ( (key == VIDEO_DEVICES_KEY "size") ||
       (key == VIDEO_DEVICES_KEY "max_frame_rate") ) {

    PTRACE(4, "VidInputCoreConfBridge\tUpdating preview size and fps");

    if ( (gm_conf_get_int (VIDEO_DEVICES_KEY "size") < 0 ) || 
         (gm_conf_get_int (VIDEO_DEVICES_KEY "size") >= NB_VIDEO_SIZES )) {
      PTRACE(1, "VidInputCoreConfBridge\t" << VIDEO_DEVICES_KEY "size" << " out of range, ajusting to 0");
      gm_conf_set_int (VIDEO_DEVICES_KEY "size", 0);
    }

    if ( (gm_conf_get_int (VIDEO_DEVICES_KEY "max_frame_rate") < 0 ) || 
         (gm_conf_get_int (VIDEO_DEVICES_KEY "max_frame_rate") > 30)) {
      PTRACE(1, "VidInputCoreConfBridge\t" << VIDEO_DEVICES_KEY "max_frame_rate" << " out of range, ajusting to 30");
      gm_conf_set_int (VIDEO_DEVICES_KEY "max_frame_rate", 30);
    }

    vidinput_core.set_preview_config (VideoSizes[gm_conf_get_int (VIDEO_DEVICES_KEY "size")].width,
                                      VideoSizes[gm_conf_get_int (VIDEO_DEVICES_KEY "size")].height,
                                      gm_conf_get_int (VIDEO_CODECS_KEY "max_frame_rate"));
  }
  else if ( (key == VIDEO_DEVICES_KEY "input_device") ||
            (key == VIDEO_DEVICES_KEY "plugin") ||
            (key == VIDEO_DEVICES_KEY "channel") ||
            (key == VIDEO_DEVICES_KEY "format") ) {

    PTRACE(4, "VidInputCoreConfBridge\tUpdating device");
    std::string type_source = gm_conf_get_string (VIDEO_DEVICES_KEY "plugin");
    VidInputDevice vidinput_device;

    vidinput_device.type   = type_source.substr ( 0, type_source.find_first_of("/"));
    vidinput_device.source = type_source.substr ( type_source.find_first_of("/") + 1, type_source.size() - 1 );
    vidinput_device.device = gm_conf_get_string (VIDEO_DEVICES_KEY "input_device");
    vidinput_core.set_vidinput_device (vidinput_device,
                                       gm_conf_get_int (VIDEO_DEVICES_KEY "channel"),
                                       (VideoFormat) gm_conf_get_int (VIDEO_DEVICES_KEY "format"));
  }
  else if (key == VIDEO_DEVICES_KEY "enable_preview") {

    PTRACE(4, "VidInputCoreConfBridge\tUpdating preview");
    if (gm_conf_get_bool ( VIDEO_DEVICES_KEY "enable_preview"))
        vidinput_core.start_preview(); 
      else
        vidinput_core.stop_preview();
  }
  else if (key == VIDEO_DEVICES_KEY "image") {
    PTRACE(4, "VidInputCoreConfBridge\tUpdating image");
  }
}

