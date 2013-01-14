
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>

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
 *                         videoinput-gmconf-bridge.cpp -  description
 *                         -------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the bridge between the gmconf
 *                          and the videoinput-core..
 *
 */

#include "config.h"

#include "videoinput-gmconf-bridge.h"
#include "videoinput-core.h"

using namespace Ekiga;

VideoInputCoreConfBridge::VideoInputCoreConfBridge (Ekiga::Service & _service)
 : Ekiga::ConfBridge (_service)
{
  Ekiga::ConfKeys keys;
  property_changed.connect (boost::bind (&VideoInputCoreConfBridge::on_property_changed, this, _1, _2));

  keys.push_back (VIDEO_DEVICES_KEY "size"); 
  keys.push_back (VIDEO_DEVICES_KEY "max_frame_rate"); 
  keys.push_back (VIDEO_DEVICES_KEY "input_device"); 
  keys.push_back (VIDEO_DEVICES_KEY "channel"); 
  keys.push_back (VIDEO_DEVICES_KEY "format"); 
  keys.push_back (VIDEO_DEVICES_KEY "image"); 
  keys.push_back (VIDEO_DEVICES_KEY "enable_preview");
  load (keys);
}

void VideoInputCoreConfBridge::on_property_changed (std::string key, GmConfEntry * /*entry*/)
{
  VideoInputCore & videoinput_core = (VideoInputCore &) service;

  if ( (key == VIDEO_DEVICES_KEY "size") ||
       (key == VIDEO_DEVICES_KEY "max_frame_rate") ) {

    PTRACE(4, "VidInputCoreConfBridge\tUpdating preview size and fps");

    unsigned size = gm_conf_get_int (VIDEO_DEVICES_KEY "size");
    if (size >= NB_VIDEO_SIZES) {
      PTRACE(1, "VidInputCoreConfBridge\t" << VIDEO_DEVICES_KEY "size" << " out of range, ajusting to 0");
      size = 0;
    }

    unsigned max_frame_rate = gm_conf_get_int (VIDEO_DEVICES_KEY "max_frame_rate");
    if ( (max_frame_rate < 1) || (max_frame_rate > 30) ) {
      PTRACE(1, "VidInputCoreConfBridge\t" << VIDEO_DEVICES_KEY "max_frame_rate" << " out of range, ajusting to 30");
      max_frame_rate = 30;
    }
    videoinput_core.set_preview_config (VideoSizes[size].width,
                                        VideoSizes[size].height,
                                        max_frame_rate);
  }
  else if ( (key == VIDEO_DEVICES_KEY "input_device") ||
            (key == VIDEO_DEVICES_KEY "channel") ||
            (key == VIDEO_DEVICES_KEY "format") ) {

    std::vector <VideoInputDevice> devices;
    bool found = false;
    gchar *value = gm_conf_get_string (VIDEO_DEVICES_KEY "input_device");
    videoinput_core.get_devices (devices);
    if (value != NULL) {
      for (std::vector<VideoInputDevice>::iterator it = devices.begin ();
           it < devices.end ();
           it++) {
        if ((*it).GetString () == value) {
          found = true;
          break;
        }
      }
    }
    PTRACE(4, "VidInputCoreConfBridge\tUpdating device");

    VideoInputDevice device;
    if (found)
      device.SetFromString (value);
    else 
      device.SetFromString (devices.begin ()->GetString ());
    g_free (value);

    if ( (device.type == "" )   ||
         (device.source == "")  ||
         (device.name == "" ) ) {
      PTRACE(1, "VidinputCore\tTried to set malformed device");
      device.type = VIDEO_INPUT_FALLBACK_DEVICE_TYPE;
      device.source = VIDEO_INPUT_FALLBACK_DEVICE_SOURCE;
      device.name = VIDEO_INPUT_FALLBACK_DEVICE_NAME;
    }

    unsigned video_format = gm_conf_get_int (VIDEO_DEVICES_KEY "format");
    if (video_format >= VI_FORMAT_MAX) {
      PTRACE(1, "VidInputCoreConfBridge\t" << VIDEO_DEVICES_KEY "format" << " out of range, ajusting to 3");
      video_format = 3;
    }

    videoinput_core.set_device (device, gm_conf_get_int (VIDEO_DEVICES_KEY "channel") ,(VideoInputFormat) video_format);
  }
  else if (key == VIDEO_DEVICES_KEY "enable_preview") {

    static bool startup = true;

    if (!startup) {

      PTRACE(4, "VidInputCoreConfBridge\tUpdating preview");
      if (gm_conf_get_bool ( VIDEO_DEVICES_KEY "enable_preview"))
        videoinput_core.start_preview();
      else
        videoinput_core.stop_preview();
    } else {

      startup = false;
      if (gm_conf_get_bool ( VIDEO_DEVICES_KEY "enable_preview"))
	Ekiga::Runtime::run_in_main (boost::bind (&VideoInputCore::start_preview, boost::ref (videoinput_core)), 5);
    }
  }
  else if (key == VIDEO_DEVICES_KEY "image") {
    PTRACE(4, "VidInputCoreConfBridge\tUpdating image");
  }
}

