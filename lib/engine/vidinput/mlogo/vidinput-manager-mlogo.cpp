
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
 *                         vidinput-manager-mlogo.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the interface of a vidinput core.
 *                          A vidinput core manages VidInputManagers.
 *
 */

#include "vidinput-manager-mlogo.h"
#include "icon.h"

#define DEVICE_TYPE "Moving Logo"
#define DEVICE_SOURCE "Moving Logo"
#define DEVICE_DEVICE "Moving Logo"

GMVidInputManager_mlogo::GMVidInputManager_mlogo (Ekiga::ServiceCore & _core)
:    core (_core), runtime (*(dynamic_cast<Ekiga::Runtime *> (_core.get ("runtime"))))
{
  current_state.opened  = false;
}

void GMVidInputManager_mlogo::get_vidinput_devices(std::vector <Ekiga::VidInputDevice> & vidinput_devices)
{
  Ekiga::VidInputDevice vidinput_device;
  vidinput_device.type   = DEVICE_TYPE;
  vidinput_device.source = DEVICE_SOURCE;
  vidinput_device.device = DEVICE_DEVICE;
  vidinput_devices.push_back(vidinput_device);
}

bool GMVidInputManager_mlogo::set_vidinput_device (const Ekiga::VidInputDevice & vidinput_device, int channel, Ekiga::VideoFormat format)
{
  if ( ( vidinput_device.type   == DEVICE_TYPE ) &&
       ( vidinput_device.source == DEVICE_SOURCE) &&
       ( vidinput_device.device == DEVICE_DEVICE) ) {

    PTRACE(4, "GMVidInputManager_mlogo\tSetting Device Moving Logo");
    current_state.vidinput_device = vidinput_device;
    current_state.channel = channel;
    current_state.format  = format;
    return true;
  }
  return false;
}

bool GMVidInputManager_mlogo::open (unsigned width, unsigned height, unsigned fps)
{
  Ekiga::VidInputConfig vidinput_config;

  PTRACE(4, "GMVidInputManager_mlogo\tOpening Moving Logo with " << width << "x" << height << "/" << fps);
  current_state.width  = width;
  current_state.height = height;
  current_state.fps    = fps;

  pos = 0;
  increment = 1;

  background_frame = (char*) malloc ((current_state.width * current_state.height * 3) >> 1);
  memset (background_frame, 0xd3, current_state.width*current_state.height); //ff
  memset (background_frame + (current_state.width * current_state.height), 0x7f, (current_state.width*current_state.height) >> 2);
  memset (background_frame + (current_state.width * current_state.height) + ((current_state.width*current_state.height) >> 2), 0x7f, (current_state.width*current_state.height) >> 2);

  m_Pacing.Restart();

  current_state.opened  = true;
  vidinput_config.whiteness = 127;
  vidinput_config.brightness = 127;
  vidinput_config.colour = 127;
  vidinput_config.contrast = 127;
  vidinput_config.modifyable = false;
  runtime.run_in_main (sigc::bind (vidinputdevice_opened.make_slot (), current_state.vidinput_device, vidinput_config));
  
  return true;
}

void GMVidInputManager_mlogo::close()
{
  PTRACE(4, "GMVidInputManager_mlogo\tClosing Moving Logo");
  free (background_frame);
  current_state.opened  = false;
  runtime.run_in_main (sigc::bind (vidinputdevice_closed.make_slot (), current_state.vidinput_device));
}

void GMVidInputManager_mlogo::get_frame_data (unsigned & width,
                     unsigned & height,
                     char *data)
{
  if (!current_state.opened) {
    PTRACE(1, "GMVidInputManager_mlogo\tTrying to get frame from closed device");
    return;
  }
  
  m_Pacing.Delay (1000 / current_state.fps);

  memcpy (data, background_frame, (current_state.width * current_state.height * 3) >> 1);

  CopyYUVArea  ((char*)&gm_icon_yuv, 
                gm_icon_width, gm_icon_height, 
                data, 
                (current_state.width - gm_icon_width) >> 1, 
                pos, 
                current_state.width, current_state.height);
  pos = pos + increment;

  if ( pos > current_state.height - gm_icon_height - 10) 
    increment = -1;
  if (pos < 10) 
    increment = +1;

  width  = current_state.width;
  height = current_state.height;
}

void GMVidInputManager_mlogo::CopyYUVArea (const char* srcFrame,
					 unsigned srcWidth,
					 unsigned srcHeight,
					 char* dstFrame,
					 unsigned dstX,
					 unsigned dstY,
					 unsigned dstWidth,
					 unsigned dstHeight)
{
  unsigned line = 0;
//Y
  dstFrame += dstY * dstWidth;
  for (line = 0; line<srcHeight; line++) {
    if (dstY + line < dstHeight)
      memcpy (dstFrame + dstX, srcFrame, srcWidth);
    srcFrame += srcWidth;
    dstFrame += dstWidth;
  }
  dstFrame += (dstHeight - dstY - srcHeight)* dstWidth;

  dstY = dstY >> 1;
  dstX = dstX >> 1;
  srcWidth  = srcWidth >> 1;
  srcHeight = srcHeight >> 1;
  dstWidth  = dstWidth >> 1;
  dstHeight = dstHeight >> 1;

//U
  dstFrame += dstY * dstWidth;
  for (line = 0; line<srcHeight; line++) {
    if (dstY + line < dstHeight)
      memcpy (dstFrame + dstX, srcFrame , srcWidth);
    srcFrame += srcWidth;
    dstFrame += dstWidth;
  }
  dstFrame += (dstHeight - dstY - srcHeight)* dstWidth;

//V
  dstFrame += dstY * dstWidth;
  for (line = 0; line<srcHeight; line++) {
    if (dstY + line < dstHeight)
      memcpy (dstFrame + dstX, srcFrame , srcWidth);
    srcFrame += srcWidth;
    dstFrame += dstWidth;
  }
}

bool GMVidInputManager_mlogo::has_device     (const std::string & /*source*/, const std::string & /*device*/, unsigned /*capabilities*/, Ekiga::VidInputDevice & /*vidinput_device*/)
{
  return false;
}
