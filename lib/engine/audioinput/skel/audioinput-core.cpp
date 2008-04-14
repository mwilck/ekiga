
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
 *                         audioinput-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : declaration of the interface of a audioinput core.
 *                          An audioinput core manages AudioInputManagers.
 *
 */

#include <iostream>
#include <sstream>
#include <math.h>

#include "audioinput-core.h"
#include "audioinput-manager.h"

using namespace Ekiga;

AudioPreviewManager::AudioPreviewManager (AudioInputCore& _audio_input_core, AudioOutputCore& _audio_output_core)
: PThread (1000, NoAutoDeleteThread, HighestPriority, "PreviewManager"),
    audio_input_core (_audio_input_core),
  audio_output_core (_audio_output_core)
{
/*  frame = NULL;
  // Since windows does not like to restart a thread that 
  // was never started, we do so here
  this->Resume ();
  PWaitAndSignal m(quit_mutex);*/
}

AudioPreviewManager::~AudioPreviewManager ()
{
/*  if (!stop_thread)
    stop();*/
}
/*
void PreviewManager::start (unsigned width, unsigned height)
{
  PTRACE(0, "PreviewManager\tStarting Preview");
  stop_thread = false;
  frame = (char*) malloc (unsigned (width * height * 3 / 2));

  display_core.start();
  this->Restart ();
  thread_sync_point.Wait ();
}

void PreviewManager::stop ()
{
  PTRACE(0, "PreviewManager\tStopping Preview");
  stop_thread = true;

  // Wait for the Main () method to be terminated 
  PWaitAndSignal m(quit_mutex);

  if (frame) {
    free (frame);
    frame = NULL;
  }  
  display_core.stop();
}
*/
void AudioPreviewManager::Main ()
{
/*  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();

  if (!frame)
    return;
    
  unsigned width = 176;
  unsigned height = 144;;
  while (!stop_thread) {

    audioinput_core.get_frame_data(width, height, frame);
    display_core.set_frame_data(width, height, frame, true, 1);

    // We have to sleep some time outside the mutex lock
    // to give other threads time to get the mutex
    // It will be taken into account by PAdaptiveDelay
    Current()->Sleep (5);
  }*/
}

AudioInputCore::AudioInputCore (Ekiga::Runtime & _runtime, AudioOutputCore& _audio_output_core)
:  runtime (_runtime),
   preview_manager(*this, _audio_output_core)

{
  PWaitAndSignal m_var(var_mutex);
  PWaitAndSignal m_vol(vol_mutex);

  preview_config.active = false;
  preview_config.channels = 0;
  preview_config.samplerate = 0;
  preview_config.bits_per_sample = 0;
  preview_config.volume = 0;
  preview_config.buffer_size = 0;
  preview_config.num_buffers = 0;
  new_preview_volume = 0;

  stream_config.active = false;
  stream_config.channels = 0;
  stream_config.samplerate = 0;
  stream_config.bits_per_sample = 0;
  stream_config.volume = 0;
  stream_config.buffer_size = 0;
  stream_config.num_buffers = 0;
  new_stream_volume = 0;

  current_manager = NULL;
  audioinput_core_conf_bridge = NULL;
  average_level = 0;
  calculate_average = false;
}

AudioInputCore::~AudioInputCore ()
{
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif

  PWaitAndSignal m(var_mutex);

  if (audioinput_core_conf_bridge)
    delete audioinput_core_conf_bridge;
}

void AudioInputCore::setup_conf_bridge ()
{
  PWaitAndSignal m(var_mutex);

  audioinput_core_conf_bridge = new AudioInputCoreConfBridge (*this);
}

void AudioInputCore::add_manager (AudioInputManager &manager)
{
  managers.insert (&manager);
  manager_added.emit (manager);

  manager.audioinputdevice_error.connect   (sigc::bind (sigc::mem_fun (this, &AudioInputCore::on_audioinputdevice_error), &manager));
  manager.audioinputdevice_opened.connect  (sigc::bind (sigc::mem_fun (this, &AudioInputCore::on_audioinputdevice_opened), &manager));
  manager.audioinputdevice_closed.connect  (sigc::bind (sigc::mem_fun (this, &AudioInputCore::on_audioinputdevice_closed), &manager));
}


void AudioInputCore::visit_managers (sigc::slot<bool, AudioInputManager &> visitor)
{
  PWaitAndSignal m(var_mutex);
  bool go_on = true;
  
  for (std::set<AudioInputManager *>::iterator iter = managers.begin ();
       iter != managers.end () && go_on;
       iter++)
      go_on = visitor (*(*iter));
}		      

void AudioInputCore::get_audioinput_devices (std::vector <AudioInputDevice> & audioinput_devices)
{
  PWaitAndSignal m(var_mutex);

  audioinput_devices.clear();
  
  for (std::set<AudioInputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++)
    (*iter)->get_audioinput_devices (audioinput_devices);

  if (PTrace::CanTrace(4)) {
     for (std::vector<AudioInputDevice>::iterator iter = audioinput_devices.begin ();
         iter != audioinput_devices.end ();
         iter++) {
      PTRACE(0, "AudioInputCore\tDetected Device: " << iter->type << "/" << iter->source << "/" << iter->device);
    }
  }

}

void AudioInputCore::set_audioinput_device(const AudioInputDevice & audioinput_device)
{
  PWaitAndSignal m(var_mutex);

  internal_set_audioinput_device(audioinput_device);

  desired_device  = audioinput_device;
}


void AudioInputCore::start_preview (unsigned channels, unsigned samplerate, unsigned bits_per_sample)
{
  PWaitAndSignal m(var_mutex);

  PTRACE(0, "AudioInputCore\tStarting preview " << channels << "x" << samplerate << "/" << bits_per_sample);

  if (preview_config.active || stream_config.active) {
    PTRACE(0, "AudioInputCore\tTrying to start preview in wrong state");
  }

  internal_open(channels, samplerate, bits_per_sample);

  preview_config.active = true;
  preview_config.channels = channels;
  preview_config.samplerate = samplerate;
  preview_config.bits_per_sample = bits_per_sample;
  preview_config.buffer_size = 320; //FIXME: verify
  preview_config.num_buffers = 5;

  if (current_manager)
    current_manager->set_buffer_size(preview_config.buffer_size, preview_config.num_buffers);
//    preview_manager.start(preview_config.channels,preview_config.samplerate);

  average_level = 0;
}

void AudioInputCore::stop_preview ()
{
  PWaitAndSignal m(var_mutex);

  PTRACE(0, "AudioInputCore\tStopping Preview");

  if (!preview_config.active || stream_config.active) {
    PTRACE(0, "AudioInputCore\tTrying to stop preview in wrong state");
  }

//     preview_manager.stop();
  internal_close();

  preview_config.active = false;
}


void AudioInputCore::set_stream_buffer_size (unsigned buffer_size, unsigned num_buffers)
{
  PWaitAndSignal m(var_mutex);

  PTRACE(0, "AudioInputCore\tSetting stream buffer size " << num_buffers << "/" << buffer_size);

  if (current_manager)
    current_manager->set_buffer_size(buffer_size, num_buffers);

  stream_config.buffer_size = buffer_size;
  stream_config.num_buffers = num_buffers;
}

void AudioInputCore::start_stream (unsigned channels, unsigned samplerate, unsigned bits_per_sample)
{
  PWaitAndSignal m(var_mutex);

  PTRACE(0, "AudioInputCore\tStarting stream " << channels << "x" << samplerate << "/" << bits_per_sample);

  if (preview_config.active || stream_config.active) {
    PTRACE(0, "AudioInputCore\tTrying to start stream in wrong state");
  }

  internal_open(channels, samplerate, bits_per_sample);

  stream_config.active = true;
  stream_config.channels = channels;
  stream_config.samplerate = samplerate;
  stream_config.bits_per_sample = bits_per_sample;

  average_level = 0;
}

void AudioInputCore::stop_stream ()
{
  PWaitAndSignal m(var_mutex);

  PTRACE(0, "AudioInputCore\tStopping Stream");

  if (preview_config.active || !stream_config.active) {
    PTRACE(0, "AudioInputCore\tTrying to stop stream in wrong state");
    return;
  }

  internal_close();

  stream_config.active = false;
  average_level = 0;
}

void AudioInputCore::get_frame_data (char *data,
                                     unsigned size,
				     unsigned & bytes_read)
{
  PWaitAndSignal m_var(var_mutex);

  if (current_manager) {
    if (!current_manager->get_frame_data(data, size, bytes_read)) {
      internal_close();
      internal_set_fallback();
      internal_open(stream_config.channels, stream_config.samplerate, stream_config.bits_per_sample);
      if (current_manager)
        current_manager->get_frame_data(data, size, bytes_read); // the default device must always return true
    }
    
    
    PWaitAndSignal m_vol(vol_mutex);
    if ((preview_config.active) && (new_preview_volume != preview_config.volume)) {
      current_manager->set_volume (new_preview_volume);
      preview_config.volume = new_preview_volume;
    }

    if ((stream_config.active) && (new_stream_volume != stream_config.volume)) {
      current_manager->set_volume (new_stream_volume);
      stream_config.volume = new_stream_volume;
    }
  }

  if (calculate_average) 
    calculate_average_level((const short*) data, bytes_read);
}

void AudioInputCore::set_volume (unsigned volume)
{
  PWaitAndSignal m(vol_mutex);

  if (preview_config.active)
    new_preview_volume = volume;

  if (stream_config.active)
    new_stream_volume = volume;
}


void AudioInputCore::on_audioinputdevice_error (AudioInputDevice audioinput_device, AudioInputErrorCodes error_code, AudioInputManager *manager)
{
  audioinputdevice_error.emit (*manager, audioinput_device, error_code);
}


void AudioInputCore::on_audioinputdevice_opened (AudioInputDevice audioinput_device,
                                             AudioInputConfig audioinput_config, 
                                             AudioInputManager *manager)
{
  audioinputdevice_opened.emit (*manager, audioinput_device, audioinput_config);
}

void AudioInputCore::on_audioinputdevice_closed (AudioInputDevice audioinput_device, AudioInputManager *manager)
{
  audioinputdevice_closed.emit (*manager, audioinput_device);
}

void AudioInputCore::internal_set_audioinput_device(const AudioInputDevice & audioinput_device)
{
  PTRACE(0, "AudioInputCore\tSetting device: " << audioinput_device.type << "/" << audioinput_device.source << "/" << audioinput_device.device);

  if (preview_config.active)
    preview_manager.stop();

  if (preview_config.active || stream_config.active)
    internal_close();

  internal_set_device (audioinput_device);

  if (preview_config.active) {
    internal_open(preview_config.channels, preview_config.samplerate, preview_config.bits_per_sample);

    if ((preview_config.buffer_size > 0) && (preview_config.num_buffers > 0 ) ) {
      if (current_manager)
        current_manager->set_buffer_size (preview_config.buffer_size, preview_config.num_buffers);
    }
//    preview_manager.start();
  }

  if (stream_config.active) {
    internal_open(stream_config.channels, stream_config.samplerate, stream_config.bits_per_sample);

    if ((stream_config.buffer_size > 0) && (stream_config.num_buffers > 0 ) ) {
      if (current_manager)
        current_manager->set_buffer_size (stream_config.buffer_size, stream_config.num_buffers);
    }
  }
}


void AudioInputCore::internal_open (unsigned channels, unsigned samplerate, unsigned bits_per_sample)
{
  PTRACE(0, "AudioInputCore\tOpening device with " << channels << "-" << samplerate << "/" << bits_per_sample );

  if (current_manager && !current_manager->open(channels, samplerate, bits_per_sample)) {

    internal_set_fallback();

    if (current_manager)
      current_manager->open(channels, samplerate, bits_per_sample);
  }
}

void AudioInputCore::internal_set_device (const AudioInputDevice & audioinput_device)
{
  current_manager = NULL;
  for (std::set<AudioInputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
     if ((*iter)->set_audioinput_device (audioinput_device)) {
       current_manager = (*iter);
     }
  }

  // If the desired manager could not be found,
  // we se the default device. The default device
  // MUST ALWAYS be loaded and openable
  if (current_manager) {
    current_device  = audioinput_device;
  }
  else {

    PTRACE(1, "AudioInputCore\tTried to set unexisting device " << audioinput_device.type << "/" << audioinput_device.source << "/" << audioinput_device.device);
    internal_set_fallback();
  }
}

void AudioInputCore::internal_close()
{
  PTRACE(0, "AudioInputCore\tClosing current device");
  if (current_manager)
    current_manager->close();
}

void AudioInputCore::calculate_average_level (const short *buffer, unsigned size)
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

void AudioInputCore::internal_set_fallback()
{
    PTRACE(1, "AudioInputCore\tFalling back to " << AUDIO_INPUT_FALLBACK_DEVICE_TYPE << "/" << AUDIO_INPUT_FALLBACK_DEVICE_SOURCE << "/" << AUDIO_INPUT_FALLBACK_DEVICE_DEVICE);
    current_device.type = AUDIO_INPUT_FALLBACK_DEVICE_TYPE;
    current_device.source = AUDIO_INPUT_FALLBACK_DEVICE_SOURCE;
    current_device.device = AUDIO_INPUT_FALLBACK_DEVICE_DEVICE;
    internal_set_device (current_device);
}

void AudioInputCore::add_device (std::string & source, std::string & device, HalManager* /*manager*/)
{
  PTRACE(0, "AudioInputCore\tAdding Device " << device);
  PWaitAndSignal m(var_mutex);

  AudioInputDevice audioinput_device;
  for (std::set<AudioInputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
     if ((*iter)->has_device (source, device, audioinput_device)) {
     
       if ( ( desired_device.type   == audioinput_device.type   ) &&
            ( desired_device.source == audioinput_device.source ) &&
            ( desired_device.device == audioinput_device.device ) ) {
         internal_set_audioinput_device(desired_device);
       }

       runtime.run_in_main (sigc::bind (audioinputdevice_added.make_slot (), audioinput_device));
     }
  }
}

void AudioInputCore::remove_device (std::string & source, std::string & device, HalManager* /*manager*/)
{
  PTRACE(0, "AudioInputCore\tRemoving Device " << device);
  PWaitAndSignal m(var_mutex);

  AudioInputDevice audioinput_device;
  for (std::set<AudioInputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
     if ((*iter)->has_device (source, device, audioinput_device)) {
       if ( ( current_device.type   == audioinput_device.type   ) &&
            ( current_device.source == audioinput_device.source ) &&
            ( current_device.device == audioinput_device.device ) ) {

            AudioInputDevice new_audioinput_device;
            new_audioinput_device.type = AUDIO_INPUT_FALLBACK_DEVICE_TYPE;
            new_audioinput_device.source = AUDIO_INPUT_FALLBACK_DEVICE_SOURCE;
            new_audioinput_device.device = AUDIO_INPUT_FALLBACK_DEVICE_DEVICE;
            internal_set_audioinput_device( new_audioinput_device);
       }

       runtime.run_in_main (sigc::bind (audioinputdevice_removed.make_slot (), audioinput_device));
     }
  }
}
