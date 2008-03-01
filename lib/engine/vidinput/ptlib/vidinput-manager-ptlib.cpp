
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
 *                         vidinput-manager-ptlib.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : declaration of the interface of a vidinput core.
 *                          A vidinput core manages VidInputManagers.
 *
 */

#include "vidinput-manager-ptlib.h"
#include "ptbuildopts.h"
#include "ptlib.h"

#define DEVICE_TYPE "PTLIB"

GMVidInputManager_ptlib::GMVidInputManager_ptlib (Ekiga::ServiceCore & _core)
:    core (_core), runtime (*(dynamic_cast<Ekiga::Runtime *> (_core.get ("runtime"))))
{
  current_state.colour = 127;
  current_state.brightness = 127;
  current_state.whiteness = 127;
  current_state.contrast = 127;
  current_state.opened = false;
  input_device = NULL;
}

void GMVidInputManager_ptlib::get_vidinput_devices(std::vector <Ekiga::VidInputDevice> & vidinput_devices)
{
  PStringArray video_sources;
  PStringArray video_devices;
  char **sources_array;
  char **devices_array;

  Ekiga::VidInputDevice vidinput_device;
  vidinput_device.type   = DEVICE_TYPE;

  video_sources = PVideoInputDevice::GetDriverNames ();
  sources_array = video_sources.ToCharArray ();
  for (PINDEX i = 0; sources_array[i] != NULL; i++) {

    vidinput_device.source = sources_array[i];

    if ( (vidinput_device.source != "FakeVideo") &&
         (vidinput_device.source != "EKIGA"    ) ) {
      video_devices = PVideoInputDevice::GetDriversDeviceNames (vidinput_device.source);
      devices_array = video_devices.ToCharArray ();
  
      for (PINDEX j = 0; devices_array[j] != NULL; j++) {
  
        vidinput_device.device = devices_array[j];
        vidinput_devices.push_back(vidinput_device);  
      }
      free (devices_array);
    }
  }  
  free (sources_array);
}

bool GMVidInputManager_ptlib::set_vidinput_device (const Ekiga::VidInputDevice & vidinput_device, int channel, Ekiga::VideoFormat format)
{
  if ( vidinput_device.type == DEVICE_TYPE ) {

    PTRACE(4, "GMVidInputManager_ptlib\tSetting Device " << vidinput_device.source << "/" <<  vidinput_device.device);
    current_state.vidinput_device = vidinput_device;  
    current_state.channel = channel;
    current_state.format = format;
    return true;
  }
		    
  return false;
}

bool GMVidInputManager_ptlib::open (unsigned width, unsigned height, unsigned fps)
{
  PTRACE(4, "GMVidInputManager_ptlib\tOpening Device " << current_state.vidinput_device.source << "/" <<  current_state.vidinput_device.device);
  PTRACE(4, "GMVidInputManager_ptlib\tOpening Device with " << width << "x" << height << "/" << fps);
  PVideoDevice::VideoFormat pvideo_format;

  current_state.width  = width;
  current_state.height = height;
  current_state.fps    = fps;

  pvideo_format = (PVideoDevice::VideoFormat)current_state.format;
  input_device = PVideoInputDevice::CreateOpenedDevice (current_state.vidinput_device.source, current_state.vidinput_device.device, FALSE);

  Ekiga::VidInputErrorCodes error_code = Ekiga::ERR_NONE;
  if (!input_device)
    error_code = Ekiga::ERR_DEVICE;
  else if (!input_device->SetVideoFormat (pvideo_format))
    error_code = Ekiga::ERR_FORMAT;
  else if (!input_device->SetChannel (current_state.channel))
    error_code = Ekiga::ERR_CHANNEL;
  else if (!input_device->SetColourFormatConverter ("YUV420P"))
    error_code = Ekiga::ERR_COLOUR;
  else if (!input_device->SetFrameRate (current_state.fps))
    error_code = Ekiga::ERR_FPS;
  else if (!input_device->SetFrameSizeConverter (current_state.width, current_state.height, PVideoFrameInfo::eScale))
    error_code = Ekiga::ERR_SCALE;
  else input_device->Start ();

  if (error_code != Ekiga::ERR_NONE) {
    PTRACE(1, "GMVidInputManager_ptlib\tEncountered error " << error_code << " while opening device ");
    runtime.run_in_main (sigc::bind (error.make_slot (), error_code));
    return false;
  }

  input_device->SetWhiteness(current_state.whiteness << 8);
  input_device->SetBrightness(current_state.brightness << 8);
  input_device->SetColour(current_state.colour << 8);
  input_device->SetContrast(current_state.contrast << 8);
  current_state.opened = true;

  return true;
}

void GMVidInputManager_ptlib::close()
{
  PTRACE(4, "GMVidInputManager_ptlib\tClosing device " << current_state.vidinput_device.source << "/" <<  current_state.vidinput_device.device);
  if (input_device) {
    delete input_device;
    input_device = NULL;
  }
  current_state.opened = false;
}

void GMVidInputManager_ptlib::get_frame_data (unsigned & width,
                     unsigned & height,
                     char *data)
{
  if (!current_state.opened) {
    PTRACE(1, "GMVidInputManager_ptlib\tTrying to get frame from closed device");
    return;
  }

  width = current_state.width;
  height = current_state.height;

  PINDEX I = 0;

  if (input_device)
    input_device->GetFrameData ((BYTE*)data, &I);
}

void GMVidInputManager_ptlib::set_colour (unsigned colour)
{
  PTRACE(4, "GMVidInputManager_ptlib\tSetting colour to " << colour);
  current_state.colour = colour;
  if (input_device)
    input_device->SetColour(colour);
}

void GMVidInputManager_ptlib::set_brightness (unsigned brightness)
{
  PTRACE(4, "GMVidInputManager_ptlib\tSetting brightness to " << brightness);
  current_state.brightness = brightness;
  if (input_device)
    input_device->SetBrightness(brightness);
}

void GMVidInputManager_ptlib::set_whiteness (unsigned whiteness)
{
  PTRACE(4, "GMVidInputManager_ptlib\tSetting whiteness to " << whiteness);
  current_state.whiteness = whiteness;
  if (input_device)
    input_device->SetWhiteness(whiteness);
}

void GMVidInputManager_ptlib::set_contrast (unsigned contrast)
{
  PTRACE(4, "GMVidInputManager_ptlib\tSetting contrast to " << contrast);
  current_state.contrast = contrast;
  if (input_device)
    input_device->SetContrast(contrast);
}
