
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
 *                         audiooutput-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : declaration of the interface of a audiooutput core.
 *                          An audiooutput core manages AudioOutputManagers.
 *
 */
#include "audiooutput-core.h"
#include "audiooutput-manager.h"
#include <algorithm>

#define FALLBACK_DEVICE_TYPE "NULL"
#define FALLBACK_DEVICE_SOURCE "NULL"
#define FALLBACK_DEVICE_DEVICE "NULL"

using namespace Ekiga;
AudioOutputCore::AudioOutputCore (Ekiga::Runtime & _runtime)
:  runtime (_runtime),
   audio_event_scheduler(*this)
{
  PWaitAndSignal m_pri(var_mutex[primary]);
  PWaitAndSignal m_sec(var_mutex[secondary]);
  PWaitAndSignal m_vol(vol_mutex);

  current_primary_config.active = false;
  current_primary_config.channels = 0;
  current_primary_config.samplerate = 0;
  current_primary_config.bits_per_sample = 0;
  current_primary_config.buffer_size = 0;
  current_primary_config.num_buffers = 0;
  current_primary_config.volume = 0;
  new_primary_volume = 0;
  
  current_manager[primary] = NULL;
  current_manager[secondary] = NULL;
  audiooutput_core_conf_bridge = NULL;
  average_level = 0;
  calculate_average = false;
}

AudioOutputCore::~AudioOutputCore ()
{
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif

  PWaitAndSignal m_pri(var_mutex[primary]);
  PWaitAndSignal m_sec(var_mutex[secondary]);

  if (audiooutput_core_conf_bridge)
    delete audiooutput_core_conf_bridge;
}

void AudioOutputCore::setup_conf_bridge ()
{
  PWaitAndSignal m_pri(var_mutex[primary]);
  PWaitAndSignal m_sec(var_mutex[secondary]);

   audiooutput_core_conf_bridge = new AudioOutputCoreConfBridge (*this);
}

void AudioOutputCore::add_manager (AudioOutputManager &manager)
{
  managers.insert (&manager);
  manager_added.emit (manager);

  manager.audiooutputdevice_error.connect (sigc::bind (sigc::mem_fun (this, &AudioOutputCore::on_audiooutputdevice_error), &manager));
  manager.audiooutputdevice_opened.connect (sigc::bind (sigc::mem_fun (this, &AudioOutputCore::on_audiooutputdevice_opened), &manager));
  manager.audiooutputdevice_closed.connect (sigc::bind (sigc::mem_fun (this, &AudioOutputCore::on_audiooutputdevice_closed), &manager));
}

void AudioOutputCore::visit_managers (sigc::slot<bool, AudioOutputManager &> visitor)
{
  PWaitAndSignal m_pri(var_mutex[primary]);
  PWaitAndSignal m_sec(var_mutex[secondary]);
  bool go_on = true;
  
  for (std::set<AudioOutputManager *>::iterator iter = managers.begin ();
       iter != managers.end () && go_on;
       iter++)
      go_on = visitor (*(*iter));
}

void AudioOutputCore::add_event (const std::string & event_name, const std::string & file_name, bool enabled, AudioOutputPrimarySecondary primarySecondary)
{
  audio_event_scheduler.set_file_name(event_name, file_name, enabled, primarySecondary);
}

void AudioOutputCore::play_file (const std::string & file_name)
{
  audio_event_scheduler.add_event_to_queue(file_name, true, 0, 0);
}

void AudioOutputCore::play_event (const std::string & event_name)
{
  audio_event_scheduler.add_event_to_queue(event_name, false, 0, 0);
}

void AudioOutputCore::start_play_event (const std::string & event_name, unsigned interval, unsigned repetitions)
{
  audio_event_scheduler.add_event_to_queue(event_name, false, interval, repetitions);
}

void AudioOutputCore::stop_play_event (const std::string & event_name)
{
  audio_event_scheduler.remove_event_from_queue(event_name);
}

void AudioOutputCore::get_audiooutput_devices (std::vector <AudioOutputDevice> & audiooutput_devices)
{
  PWaitAndSignal m_pri(var_mutex[primary]);
  PWaitAndSignal m_sec(var_mutex[secondary]);

  audiooutput_devices.clear();
  
  for (std::set<AudioOutputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++)
    (*iter)->get_audiooutput_devices (audiooutput_devices);

  if (PTrace::CanTrace(4)) {
     for (std::vector<AudioOutputDevice>::iterator iter = audiooutput_devices.begin ();
         iter != audiooutput_devices.end ();
         iter++) {
      PTRACE(0, "AudioOutputCore\tDetected Device: " << iter->type << "/" << iter->source << "/" << iter->device);
    }
  }

}

void AudioOutputCore::set_audiooutput_device(AudioOutputPrimarySecondary primarySecondary, const AudioOutputDevice & audiooutput_device)
{
  PTRACE(0, "AudioOutputCore\tSetting device[" << primarySecondary << "]: " << audiooutput_device.type << "/" << audiooutput_device.source << "/" << audiooutput_device.device);

  PWaitAndSignal m_sec(var_mutex[secondary]);

  switch (primarySecondary) {
    case primary:
      var_mutex[primary].Wait();
      if (current_primary_config.active)
        internal_close(primary);

      if ( (audiooutput_device.type == current_device[secondary].type) &&
           (audiooutput_device.source == current_device[secondary].source) &&
           (audiooutput_device.device == current_device[secondary].device) )
      { 
        current_manager[secondary] = NULL;
        current_device[secondary].type = "";
        current_device[secondary].source = "";
        current_device[secondary].device = "";
      }

      internal_set_device(primary, audiooutput_device);

      if (current_primary_config.active)
        internal_open(primary, current_primary_config.channels, current_primary_config.samplerate, current_primary_config.bits_per_sample);

      if ((current_primary_config.buffer_size > 0) && (current_primary_config.num_buffers > 0 ) ) {
        if (current_manager[primary])
          current_manager[primary]->set_buffer_size (primary, current_primary_config.buffer_size, current_primary_config.num_buffers);
      }

      desired_primary_device = audiooutput_device;

      var_mutex[primary].Signal();

      break;
    case secondary:
        if ( (audiooutput_device.type == current_device[primary].type) &&
             (audiooutput_device.source == current_device[primary].source) &&
             (audiooutput_device.device == current_device[primary].device) )
        {
          current_manager[secondary] = NULL;
          current_device[secondary].type = "";
          current_device[secondary].source = "";
          current_device[secondary].device = "";
        }
        else {
          internal_set_device (secondary, audiooutput_device);
        }
        break;
    default:
      break;
  }
}

void AudioOutputCore::start (unsigned channels, unsigned samplerate, unsigned bits_per_sample)
{
  PWaitAndSignal m_pri(var_mutex[primary]);

  if (current_primary_config.active) {
    PTRACE(1, "AudioOutputCore\tTrying to start output device although already started");
    return;
  }

  average_level = 0;
  internal_open(primary, channels, samplerate, bits_per_sample);
  current_primary_config.active = true;
  current_primary_config.channels = channels;
  current_primary_config.samplerate = samplerate;
  current_primary_config.bits_per_sample = bits_per_sample;
  current_primary_config.buffer_size = 0;
  current_primary_config.num_buffers = 0;
}

void AudioOutputCore::stop()
{
  PWaitAndSignal m_pri(var_mutex[primary]);

  average_level = 0;
  internal_close(primary);
  current_primary_config.active = false;
}

void AudioOutputCore::set_buffer_size (unsigned buffer_size, unsigned num_buffers) {
  PWaitAndSignal m_pri(var_mutex[primary]);

  if (current_manager[primary])
    current_manager[primary]->set_buffer_size (primary, buffer_size, num_buffers);

  current_primary_config.buffer_size = buffer_size;
  current_primary_config.num_buffers = num_buffers;
}


void AudioOutputCore::set_frame_data (char *data,
                                      unsigned size,
				      unsigned & written)
{
  PWaitAndSignal m_pri(var_mutex[primary]);

  if (current_manager[primary]) {
    if (!current_manager[primary]->set_frame_data(primary,data, size, written)) {
      internal_close(primary);
      internal_set_primary_fallback();
      internal_open(primary, current_primary_config.channels, current_primary_config.samplerate, current_primary_config.bits_per_sample);
      if (current_manager[primary])
        current_manager[primary]->set_frame_data(primary,data, size, written); // the default device must always return true
    }

    PWaitAndSignal m_vol(vol_mutex);
    if (new_primary_volume != current_primary_config.volume) {
      current_manager[primary]->set_volume(primary, new_primary_volume);
      current_primary_config.volume = new_primary_volume;
    }
  }

  if (calculate_average) 
    calculate_average_level((const short*) data, written);
}

void AudioOutputCore::play_buffer(AudioOutputPrimarySecondary primarySecondary, char* buffer, unsigned long len, unsigned channels, unsigned sample_rate, unsigned bps)
{
PTRACE(0, "Play buffer " << primarySecondary);
  switch (primarySecondary) {
    case primary:
      var_mutex[primary].Wait();

      if (!current_manager[primary]) {
        PTRACE(0, "AudioOutputCore\tDropping sound event, primary manager not set");
        var_mutex[primary].Signal();
        return;
      }

      if (current_primary_config.active) {
        PTRACE(0, "AudioOutputCore\tDropping sound event, primary device not set");
        var_mutex[primary].Signal();
        return;
      }
      internal_play(primary, buffer, len, channels, sample_rate, bps);
      var_mutex[primary].Signal();

      break;
    case secondary:
        var_mutex[secondary].Wait();
 
        if (current_manager[secondary]) {
             internal_play(secondary, buffer, len, channels, sample_rate, bps);
          var_mutex[secondary].Signal();
        }
        else {
          var_mutex[secondary].Signal();
          PTRACE(0, "AudioOutputCore\tNo secondary audiooutput device defined, trying primary");
          play_buffer(primary, buffer, len, channels, sample_rate, bps);
        }
      break;
    default:
      break;
  }
}


void AudioOutputCore::internal_set_device (AudioOutputPrimarySecondary primarySecondary, const AudioOutputDevice & audiooutput_device)
{
  current_manager[primarySecondary] = NULL;
  for (std::set<AudioOutputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
     if ((*iter)->set_audiooutput_device (primarySecondary, audiooutput_device)) {
       current_manager[primarySecondary] = (*iter);
     }
  }

  if (current_manager[primarySecondary]) {
    current_device[primarySecondary]  = audiooutput_device;
  }
  else {
    if (primarySecondary == primary) {
      PTRACE(1, "AudioOutputCore\tTried to set unexisting primary device " << audiooutput_device.type << "/" << audiooutput_device.source << "/" << audiooutput_device.device);
      internal_set_primary_fallback();
    }
    else {
      PTRACE(1, "AudioOutputCore\tTried to set unexisting secondary device " << audiooutput_device.type << "/" << audiooutput_device.source << "/" << audiooutput_device.device);
      current_device[secondary].type = "";
      current_device[secondary].source = "";
      current_device[secondary].device = "";
    }
  }

}

void AudioOutputCore::on_audiooutputdevice_error (AudioOutputPrimarySecondary primarySecondary, AudioOutputDevice audiooutput_device, AudioOutputErrorCodes error_code, AudioOutputManager *manager)
{
  audiooutputdevice_error.emit (*manager, primarySecondary, audiooutput_device, error_code);
}

void AudioOutputCore::on_audiooutputdevice_opened (AudioOutputPrimarySecondary primarySecondary,
                                                AudioOutputDevice audiooutput_device,
                                                AudioOutputConfig audiooutput_config, 
                                                AudioOutputManager *manager)
{
  audiooutputdevice_opened.emit (*manager, primarySecondary, audiooutput_device, audiooutput_config);
}

void AudioOutputCore::on_audiooutputdevice_closed (AudioOutputPrimarySecondary primarySecondary, AudioOutputDevice audiooutput_device, AudioOutputManager *manager)
{
  audiooutputdevice_closed.emit (*manager, primarySecondary, audiooutput_device);
}


bool AudioOutputCore::internal_open (AudioOutputPrimarySecondary primarySecondary, unsigned channels, unsigned samplerate, unsigned bits_per_sample)
{
  PTRACE(0, "AudioOutputCore\tOpening device["<<primarySecondary<<"] with " << channels<< "-" << samplerate << "/" << bits_per_sample);

  if (!current_manager[primarySecondary]) {
    PTRACE(1, "AudioOutputCore\tUnable to obtain current manager for device["<<primarySecondary<<"]");
    return false;
  }

  if (!current_manager[primarySecondary]->open(primarySecondary, channels, samplerate, bits_per_sample)) {
    PTRACE(1, "AudioOutputCore\tUnable to open device["<<primarySecondary<<"]");
    if (primarySecondary == primary) {
      internal_set_primary_fallback();
      if (current_manager[primary])
        current_manager[primary]->open(primarySecondary, channels, samplerate, bits_per_sample);
      return true;
    }
    else {
      return false;
    }
  }
  return true;
}

void AudioOutputCore::internal_close(AudioOutputPrimarySecondary primarySecondary)
{
  PTRACE(0, "AudioOutputCore\tClosing current device");
  if (current_manager[primarySecondary])
    current_manager[primarySecondary]->close(primarySecondary);
}

void AudioOutputCore::internal_play(AudioOutputPrimarySecondary primarySecondary, char* buffer, unsigned long len, unsigned channels, unsigned sample_rate, unsigned bps)
{
  unsigned long pos = 0;
  unsigned written = 0;

  PTRACE(0, "AudioOutputCore\tinternal_play");
  if (!internal_open ( primarySecondary, channels, sample_rate, bps))
    return;

  if (current_manager[primarySecondary]) {
    current_manager[primarySecondary]->set_buffer_size (primarySecondary, 320, 4);
    do {
      if (!current_manager[primarySecondary]->set_frame_data(primarySecondary, buffer+pos, std::min((unsigned)320, (unsigned) (len - pos)), written))
        break;
      pos += 320;
    } while (pos < len);
  }

  internal_close( primarySecondary);
}

void AudioOutputCore::calculate_average_level (const short *buffer, unsigned size)
{
  int sum = 0;
  unsigned csize = 0;
  
  while (csize < (size>>1) ) {

    if (*buffer < 0)
      sum -= *buffer++;
    else
      sum += *buffer++;

    csize++;
  }
	  
  average_level = log10 (9.0*sum/size/32767+1)*1.0;
}

void AudioOutputCore::set_volume (AudioOutputPrimarySecondary primarySecondary, unsigned volume)
{
  PWaitAndSignal m_vol(vol_mutex);

  if (primarySecondary == primary) {
    new_primary_volume = volume;
  }
}

void AudioOutputCore::internal_set_primary_fallback()
{
  PTRACE(1, "AudioOutputCore\tFalling back to " << FALLBACK_DEVICE_TYPE << "/" << FALLBACK_DEVICE_SOURCE << "/" << FALLBACK_DEVICE_DEVICE);
  current_device[primary].type = FALLBACK_DEVICE_TYPE;
  current_device[primary].source = FALLBACK_DEVICE_SOURCE;
  current_device[primary].device = FALLBACK_DEVICE_DEVICE;
  internal_set_device(primary, current_device[primary]);
}

void AudioOutputCore::add_device (std::string & sink, std::string & device, HalManager* /*manager*/)
{
  PTRACE(0, "AudioOutputCore\tAdding Device");
  AudioOutputDevice audiooutput_device;
  for (std::set<AudioOutputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
     if ((*iter)->has_device (sink, device, audiooutput_device)) {

       if ( ( desired_primary_device.type   == audiooutput_device.type   ) &&
            ( desired_primary_device.source == audiooutput_device.source ) &&
            ( desired_primary_device.device == audiooutput_device.device ) ) {
         set_audiooutput_device(primary, desired_primary_device);
       }

       runtime.run_in_main (sigc::bind (audiooutputdevice_added.make_slot (), audiooutput_device));
     }
  }
}

void AudioOutputCore::remove_device (std::string & sink, std::string & device, HalManager* /*manager*/)
{
  PTRACE(0, "AudioOutputCore\tRemoving Device");
  AudioOutputDevice audiooutput_device;
  for (std::set<AudioOutputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
     if ((*iter)->has_device (sink, device, audiooutput_device)) {
       runtime.run_in_main (sigc::bind (audiooutputdevice_removed.make_slot (), audiooutput_device));
     }
  }
}
