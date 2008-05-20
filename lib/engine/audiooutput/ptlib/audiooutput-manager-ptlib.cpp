
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
 *                         audiooutput-manager-ptlib.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : declaration of the interface of a audiooutput core.
 *                          A audioinput core manages AudioOutputManagers.
 *
 */

#include "audiooutput-manager-ptlib.h"
#include "ptbuildopts.h"
#include "ptlib.h"

#define DEVICE_TYPE "PTLIB"

GMAudioOutputManager_ptlib::GMAudioOutputManager_ptlib (Ekiga::ServiceCore & _core)
: core (_core),
  runtime (*(dynamic_cast<Ekiga::Runtime *> (_core.get ("runtime"))))
{
  current_state[Ekiga::primary].opened = false;
  current_state[Ekiga::secondary].opened = false;
  output_device[Ekiga::primary] = NULL;
  output_device[Ekiga::secondary] = NULL;
}

void GMAudioOutputManager_ptlib::get_devices(std::vector <Ekiga::AudioOutputDevice> & devices)
{
  PStringArray audio_sources;
  PStringArray audio_devices;
  char **sources_array;
  char **devices_array;

  Ekiga::AudioOutputDevice device;
  device.type   = DEVICE_TYPE;

  audio_sources = PSoundChannel::GetDriverNames ();
  sources_array = audio_sources.ToCharArray ();
  for (PINDEX i = 0; sources_array[i] != NULL; i++) {

    device.source = sources_array[i];

    if (device.source != "EKIGA") {
      audio_devices = PSoundChannel::GetDeviceNames (device.source, PSoundChannel::Player);
      devices_array = audio_devices.ToCharArray ();

      for (PINDEX j = 0; devices_array[j] != NULL; j++) {

        device.name = devices_array[j];
        devices.push_back(device);
      }
      free (devices_array);
    }
  }
  free (sources_array);
}

bool GMAudioOutputManager_ptlib::set_device (Ekiga::AudioOutputPrimarySecondary primarySecondary, const Ekiga::AudioOutputDevice & device)
{
  if ( device.type == DEVICE_TYPE ) {

    PTRACE(4, "GMAudioOutputManager_ptlib\tSetting Device[" << primarySecondary << "] " << device);
    current_state[primarySecondary].device = device;  
    return true;
  }

  return false;
}

bool GMAudioOutputManager_ptlib::open (Ekiga::AudioOutputPrimarySecondary primarySecondary, unsigned channels, unsigned samplerate, unsigned bits_per_sample)
{
  PTRACE(4, "GMAudioOutputManager_ptlib\tOpening Device " << current_state[primarySecondary].device);
  PTRACE(4, "GMAudioOutputManager_ptlib\tOpening Device with " << channels << "-" << samplerate << "/" << bits_per_sample);

  current_state[primarySecondary].channels        = channels;
  current_state[primarySecondary].samplerate      = samplerate;
  current_state[primarySecondary].bits_per_sample = bits_per_sample;

  output_device[primarySecondary] = PSoundChannel::CreateOpenedChannel (current_state[primarySecondary].device.source, 
                                                                        current_state[primarySecondary].device.name,
                                                                        PSoundChannel::Player,
                                                                        channels,
                                                                        samplerate,
                                                                        bits_per_sample);

  Ekiga::AudioOutputErrorCodes error_code = Ekiga::AO_ERROR_NONE;
  if (!output_device[primarySecondary])
    error_code = Ekiga::AO_ERROR_DEVICE;

  if (error_code != Ekiga::AO_ERROR_NONE) {
    PTRACE(1, "GMAudioOutputManager_ptlib\tEncountered error " << error_code << " while opening device[" << primarySecondary << "]");
    runtime.run_in_main (sigc::bind (device_error.make_slot (), primarySecondary, current_state[primarySecondary].device, error_code));
    return false;
  }

  unsigned volume;
  output_device[primarySecondary]->GetVolume (volume);
  current_state[primarySecondary].opened = true;

  Ekiga::AudioOutputConfig config;
  config.volume = volume;
  config.modifyable = true;
  runtime.run_in_main (sigc::bind (device_opened.make_slot (), primarySecondary, current_state[primarySecondary].device, config));

  return true;
}

void GMAudioOutputManager_ptlib::close(Ekiga::AudioOutputPrimarySecondary primarySecondary)
{
  PTRACE(4, "GMAudioOutputManager_ptlib\tClosing device[" << primarySecondary << "] " << current_state[primarySecondary].device);
  if (output_device[primarySecondary]) {
     delete output_device[primarySecondary];
     output_device[primarySecondary] = NULL;
  }
  current_state[primarySecondary].opened = false;
  runtime.run_in_main (sigc::bind (device_closed.make_slot (), primarySecondary, current_state[primarySecondary].device));
}

void GMAudioOutputManager_ptlib::set_buffer_size (Ekiga::AudioOutputPrimarySecondary primarySecondary, unsigned buffer_size, unsigned num_buffers)
{
  PTRACE(4, "GMAudioOutputManager_ptlib\tSetting buffer size of device[" << primarySecondary << "] " << buffer_size << "/" <<  num_buffers);

  if (output_device[primarySecondary])
    output_device[primarySecondary]->SetBuffers (buffer_size, num_buffers);
}


bool GMAudioOutputManager_ptlib::set_frame_data (Ekiga::AudioOutputPrimarySecondary primarySecondary, 
                                   const char *data, 
                                   unsigned size,
				   unsigned & bytes_written)
{
  bool ret = false;
  bytes_written = 0;

  if (!current_state[primarySecondary].opened) {
    PTRACE(1, "GMAudioOutputManager_ptlib\tTrying to get frame from closed device[" << primarySecondary << "]");
    return false;
  }

  if (output_device[primarySecondary]) {
    ret = output_device[primarySecondary]->Write ((void*)data, size);
    if (ret) {
      bytes_written = output_device[primarySecondary]->GetLastWriteCount();
    }
    else {
      PTRACE(1, "GMAudioOutputManager_ptlib\tEncountered error while trying to write data");
      runtime.run_in_main (sigc::bind (device_error.make_slot (), primarySecondary, current_state[primarySecondary].device, Ekiga::AO_ERROR_WRITE));

    }
  }
  return ret;
}

void GMAudioOutputManager_ptlib::set_volume (Ekiga::AudioOutputPrimarySecondary primarySecondary, unsigned volume )
{
  PTRACE(4, "GMAudioOutputManager_ptlib\tSetting volume of device [" << primarySecondary << "] to " << volume);
  if (output_device[primarySecondary])
    output_device[primarySecondary]->SetVolume(volume);
}

bool GMAudioOutputManager_ptlib::has_device(const std::string & sink, const std::string & device_name, Ekiga::AudioOutputDevice & device)
{
  if (sink == "alsa") {
    device.type = DEVICE_TYPE;
    device.source = "ALSA";
    device.name = device_name;
    return true;
  }
/*  if (source == "oss") {
    device.type = DEVICE_TYPE;
    device.source = "OSS";
    device.device = device_name;
    return true;
  }*/
  return false;
}
