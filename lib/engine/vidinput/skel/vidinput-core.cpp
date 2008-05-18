
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
 *                          A vidinput core manages VideoInputManagers.
 *
 */

#include <iostream>
#include <sstream>

#include "config.h"

#include "vidinput-core.h"
#include "vidinput-manager.h"

using namespace Ekiga;

PreviewManager::PreviewManager (VideoInputCore& _videoinput_core, VideoOutputCore& _videooutput_core)
: PThread (1000, NoAutoDeleteThread, HighestPriority, "PreviewManager"),
    videoinput_core (_videoinput_core),
  videooutput_core (_videooutput_core)
{
  frame = NULL;
  // Since windows does not like to restart a thread that 
  // was never started, we do so here
  this->Resume ();
  PWaitAndSignal m(quit_mutex);
}

PreviewManager::~PreviewManager ()
{
  if (!stop_thread)
    stop();
}

void PreviewManager::start (unsigned width, unsigned height)
{
  PTRACE(4, "PreviewManager\tStarting Preview");
  stop_thread = false;
  frame = (char*) malloc (unsigned (width * height * 3 / 2));

  videooutput_core.start();
  this->Restart ();
  thread_sync_point.Wait ();
}

void PreviewManager::stop ()
{
  PTRACE(4, "PreviewManager\tStopping Preview");
  stop_thread = true;

  /* Wait for the Main () method to be terminated */
  PWaitAndSignal m(quit_mutex);

  if (frame) {
    free (frame);
    frame = NULL;
  }  
  videooutput_core.stop();
}

void PreviewManager::Main ()
{
  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();

  if (!frame)
    return;
    
  unsigned width = 176;
  unsigned height = 144;;
  while (!stop_thread) {

    videoinput_core.get_frame_data(frame, width, height);
    videooutput_core.set_frame_data(frame, width, height, true, 1);

    // We have to sleep some time outside the mutex lock
    // to give other threads time to get the mutex
    // It will be taken into account by PAdaptiveDelay
    Current()->Sleep (5);
  }
}

VideoInputCore::VideoInputCore (Ekiga::Runtime & _runtime, VideoOutputCore& _videooutput_core)
:  runtime (_runtime),
   preview_manager(*this, _videooutput_core)
{
  PWaitAndSignal m_var(var_mutex);
  PWaitAndSignal m_set(set_mutex);

  preview_config.active = false;
  preview_config.width = 176;
  preview_config.height = 144;
  preview_config.fps = 30;
  preview_config.settings.brightness = 0;
  preview_config.settings.whiteness = 0;
  preview_config.settings.colour = 0;
  preview_config.settings.contrast = 0;

  new_preview_settings.brightness = 0;
  new_preview_settings.whiteness = 0;
  new_preview_settings.colour = 0;
  new_preview_settings.contrast = 0;

  stream_config.active = false;
  stream_config.width = 176;
  stream_config.height = 144;
  stream_config.fps = 30;
  stream_config.settings.brightness = 0;
  stream_config.settings.whiteness = 0;
  stream_config.settings.colour = 0;
  stream_config.settings.contrast = 0;

  new_stream_settings.brightness = 0;
  new_stream_settings.whiteness = 0;
  new_stream_settings.colour = 0;
  new_stream_settings.contrast = 0;

  current_manager = NULL;
  videoinput_core_conf_bridge = NULL;
}

VideoInputCore::~VideoInputCore ()
{
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif

  PWaitAndSignal m(var_mutex);

  if (videoinput_core_conf_bridge)
    delete videoinput_core_conf_bridge;
}

void VideoInputCore::setup_conf_bridge ()
{
  PWaitAndSignal m(var_mutex);

  videoinput_core_conf_bridge = new VideoInputCoreConfBridge (*this);
}

void VideoInputCore::add_manager (VideoInputManager &manager)
{
  managers.insert (&manager);
  manager_added.emit (manager);

  manager.device_opened.connect (sigc::bind (sigc::mem_fun (this, &VideoInputCore::on_device_opened), &manager));
  manager.device_closed.connect (sigc::bind (sigc::mem_fun (this, &VideoInputCore::on_device_closed), &manager));
  manager.device_error.connect (sigc::bind (sigc::mem_fun (this, &VideoInputCore::on_device_error), &manager));
}


void VideoInputCore::visit_managers (sigc::slot<bool, VideoInputManager &> visitor)
{
  PWaitAndSignal m(var_mutex);
  bool go_on = true;
  
  for (std::set<VideoInputManager *>::iterator iter = managers.begin ();
       iter != managers.end () && go_on;
       iter++)
      go_on = visitor (*(*iter));
}		      

void VideoInputCore::get_devices (std::vector <VideoInputDevice> & devices)
{
  PWaitAndSignal m(var_mutex);

  devices.clear();
  
  for (std::set<VideoInputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++)
    (*iter)->get_devices (devices);

  if (PTrace::CanTrace(4)) {
     for (std::vector<VideoInputDevice>::iterator iter = devices.begin ();
         iter != devices.end ();
         iter++) {
      PTRACE(4, "VidInputCore\tDetected Device: " << *iter);
    }
  }
}

void VideoInputCore::set_device(const VideoInputDevice & device, int channel, VideoInputFormat format)
{
  PWaitAndSignal m(var_mutex);
  internal_set_vidinput_device(device, channel, format);
  desired_device  = device;
}


void VideoInputCore::set_preview_config (unsigned width, unsigned height, unsigned fps)
{
  PWaitAndSignal m(var_mutex);

  PTRACE(4, "VidInputCore\tSetting new preview config: " << width << "x" << height << "/" << fps);
  // There is only one state where we have to reopen the preview device:
  // we have preview enabled, no stream is active and some value has changed
  if ( ( preview_config.active && !stream_config.active) &&
       ( preview_config.width != width ||
         preview_config.height != height ||
         preview_config.fps != fps ) )
  {
    preview_manager.stop();
    internal_close();

    internal_open(width, height, fps);
    preview_manager.start(width, height);
  }

  preview_config.width = width;
  preview_config.height = height;
  preview_config.fps = fps;
}


void VideoInputCore::start_preview ()
{
  PWaitAndSignal m(var_mutex);

  PTRACE(4, "VidInputCore\tStarting preview " << preview_config.width << "x" << preview_config.height << "/" << preview_config.fps);
  if (!preview_config.active && !stream_config.active) {
    internal_open(preview_config.width, preview_config.height, preview_config.fps);
    preview_manager.start(preview_config.width,preview_config.height);
  }

  preview_config.active = true;
}

void VideoInputCore::stop_preview ()
{
  PWaitAndSignal m(var_mutex);

  PTRACE(4, "VidInputCore\tStopping Preview");
  if (preview_config.active && !stream_config.active) {
    preview_manager.stop();
    internal_close();
    internal_set_device(desired_device, current_channel, current_format);
  }

  preview_config.active = false;
}

void VideoInputCore::set_stream_config (unsigned width, unsigned height, unsigned fps)
{
  PWaitAndSignal m(var_mutex);

  PTRACE(4, "VidInputCore\tSetting new stream config: " << width << "x" << height << "/" << fps);
  // We do not support switching of framerate or resolution within a stream
  // since many endpoints will probably have problems with that. Also, it would add
  // a lot of complexity due to the capabilities exchange. Thus these values will 
  // not be used until the next start_stream.

  if (!stream_config.active) {
    stream_config.width = width;
    stream_config.height = height;
    stream_config.fps = fps;
  }
}

void VideoInputCore::start_stream ()
{
  PWaitAndSignal m(var_mutex);

  PTRACE(4, "VidInputCore\tStarting stream " << stream_config.width << "x" << stream_config.height << "/" << stream_config.fps);
  if (preview_config.active && !stream_config.active) {
    preview_manager.stop();
    if ( preview_config.width != stream_config.width ||
         preview_config.height != stream_config.height ||
         preview_config.fps != stream_config.fps ) 
    {
      internal_close();
      internal_open(stream_config.width, stream_config.height, stream_config.fps);
    }
  }

  if (!preview_config.active && !stream_config.active) {
    internal_open(stream_config.width, stream_config.height, stream_config.fps);
  }

  stream_config.active = true;
}

void VideoInputCore::stop_stream ()
{
  PWaitAndSignal m(var_mutex);

  PTRACE(4, "VidInputCore\tStopping Stream");
  if (preview_config.active && stream_config.active) {
    if ( preview_config.width != stream_config.width ||
         preview_config.height != stream_config.height ||
         preview_config.fps != stream_config.fps ) 
    {
      internal_close();
      internal_set_device(desired_device, current_channel, current_format);
      internal_open(preview_config.width, preview_config.height, preview_config.fps);
    }
    preview_manager.start(preview_config.width, preview_config.height);
  }

  if (!preview_config.active && stream_config.active) {
    internal_close();
    internal_set_device(desired_device, current_channel, current_format);
  }

  stream_config.active = false;
}

void VideoInputCore::get_frame_data (char *data,
                                   unsigned & width,
                                   unsigned & height)
{
  PWaitAndSignal m(var_mutex);

  if (current_manager) {
    if (!current_manager->get_frame_data(data, width, height)) {

      internal_close();

      internal_set_fallback();

      if (preview_config.active && !stream_config.active)
        internal_open(preview_config.width, preview_config.height, preview_config.fps);

      if (stream_config.active)
        internal_open(stream_config.width, stream_config.height, stream_config.fps);

      if (current_manager)
        current_manager->get_frame_data(data, width, height); // the default device must always return true
    }
    apply_settings();
  }
}

void VideoInputCore::apply_settings()
{
  PWaitAndSignal m_set(set_mutex);
  if (preview_config.active && !stream_config.active) {

    if (new_preview_settings.colour != preview_config.settings.colour) {
      current_manager->set_colour (new_preview_settings.colour);
      preview_config.settings.colour = new_preview_settings.colour;
    }

    if (new_preview_settings.brightness != preview_config.settings.brightness) {
      current_manager->set_brightness (new_preview_settings.brightness);
      preview_config.settings.brightness = new_preview_settings.brightness;
    }

    if (new_preview_settings.whiteness != preview_config.settings.whiteness) {
      current_manager->set_whiteness (new_preview_settings.whiteness);
      preview_config.settings.whiteness = new_preview_settings.whiteness;
    }

    if (new_preview_settings.contrast != preview_config.settings.contrast) {
      current_manager->set_contrast (new_preview_settings.contrast);
      preview_config.settings.contrast = new_preview_settings.contrast;
    }

  }

  if (!preview_config.active && stream_config.active) {

    if (new_stream_settings.colour != stream_config.settings.colour) {
      current_manager->set_colour (new_stream_settings.colour);
      stream_config.settings.colour = new_stream_settings.colour;
    }

    if (new_stream_settings.brightness != stream_config.settings.brightness) {
      current_manager->set_brightness (new_stream_settings.brightness);
      stream_config.settings.brightness = new_stream_settings.brightness;
    }

    if (new_stream_settings.whiteness != stream_config.settings.whiteness) {
      current_manager->set_whiteness (new_stream_settings.whiteness);
      stream_config.settings.whiteness = new_stream_settings.whiteness;
    }

    if (new_stream_settings.contrast != stream_config.settings.contrast) {
      current_manager->set_contrast (new_stream_settings.contrast);
      stream_config.settings.contrast = new_stream_settings.contrast;
    }

  }
}

void VideoInputCore::set_colour (unsigned colour)
{
  PWaitAndSignal m(set_mutex);
  if (preview_config.active && !stream_config.active)
    new_preview_settings.colour = colour;

  if (!preview_config.active && stream_config.active)
    new_stream_settings.colour = colour;
}

void VideoInputCore::set_brightness (unsigned brightness)
{
  PWaitAndSignal m(set_mutex);
  if (preview_config.active && !stream_config.active)
    new_preview_settings.brightness  = brightness;

  if (!preview_config.active && stream_config.active)
    new_stream_settings.brightness = brightness;
}

void VideoInputCore::set_whiteness  (unsigned whiteness)
{
  PWaitAndSignal m(set_mutex);
  if (preview_config.active && !stream_config.active)
    new_preview_settings.whiteness = whiteness;

  if (!preview_config.active && stream_config.active)
    new_stream_settings.whiteness = whiteness;
}

void VideoInputCore::set_contrast   (unsigned contrast)
{
  PWaitAndSignal m(set_mutex);
  if (preview_config.active && !stream_config.active)
    new_preview_settings.contrast = contrast ;

  if (!preview_config.active && stream_config.active)
    new_stream_settings.contrast = contrast ;
}

void VideoInputCore::on_device_opened (VideoInputDevice device,
                                     VidInputConfig vidinput_config, 
                                     VideoInputManager *manager)
{
  device_opened.emit (*manager, device, vidinput_config);
}

void VideoInputCore::on_device_closed (VideoInputDevice device, VideoInputManager *manager)
{
  device_closed.emit (*manager, device);
}

void VideoInputCore::on_device_error (VideoInputDevice device, VideoInputErrorCodes error_code, VideoInputManager *manager)
{
  device_error.emit (*manager, device, error_code);
}

void VideoInputCore::internal_set_vidinput_device(const VideoInputDevice & device, int channel, VideoInputFormat format)
{
  PTRACE(4, "VidInputCore\tSetting device: " << device);

  if (preview_config.active && !stream_config.active)
    preview_manager.stop();

  if (preview_config.active || stream_config.active)
    internal_close();

  internal_set_device (device, channel, format);

  if (preview_config.active && !stream_config.active) {
    internal_open(preview_config.width, preview_config.height, preview_config.fps);
    preview_manager.start(preview_config.width,preview_config.height);
  }

  if (stream_config.active)
    internal_open(stream_config.width, stream_config.height, stream_config.fps);
}

void VideoInputCore::internal_open (unsigned width, unsigned height, unsigned fps)
{
  PTRACE(4, "VidInputCore\tOpening device with " << width << "x" << height << "/" << fps );

  if (current_manager && !current_manager->open(width, height, fps)) {

    internal_set_fallback();
    if (current_manager)
      current_manager->open(width, height, fps);
  }
}

void VideoInputCore::internal_set_device (const VideoInputDevice & device, int channel, VideoInputFormat format)
{
  current_manager = NULL;
  for (std::set<VideoInputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
     if ((*iter)->set_device (device, channel, format)) {
       current_manager = (*iter);
     }
  }

  // If the desired manager could not be found,
  // we se the default device. The default device
  // MUST ALWAYS be loaded and openable
  if (current_manager) {
    current_device  = device;
  }
  else {
    PTRACE(1, "VidInputCore\tTried to set unexisting device " << device);
    internal_set_fallback();
  }

  current_channel = channel;
  current_format  = format;
}

void VideoInputCore::internal_close()
{
  PTRACE(4, "VidInputCore\tClosing current device");
  if (current_manager)
    current_manager->close();
}

void VideoInputCore::internal_set_fallback ()
{
  current_device.type   = VIDEO_INPUT_FALLBACK_DEVICE_TYPE;
  current_device.source = VIDEO_INPUT_FALLBACK_DEVICE_SOURCE;
  current_device.name   = VIDEO_INPUT_FALLBACK_DEVICE_NAME;
  PTRACE(3, "VidInputCore\tFalling back to " << current_device);

  internal_set_device(current_device, current_channel, current_format);
}

void VideoInputCore::add_device (const std::string & source, const std::string & device_name, unsigned capabilities, HalManager* /*manager*/)
{
  PTRACE(0, "VidInputCore\tAdding Device " << device_name);
  PWaitAndSignal m(var_mutex);

  VideoInputDevice device;
  for (std::set<VideoInputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
     if ((*iter)->has_device (source, device_name, capabilities, device)) {

       if ( ( desired_device.type   == device.type   ) &&
            ( desired_device.source == device.source ) &&
            ( desired_device.name   == device.name   ) ) {
         internal_set_vidinput_device(device, current_channel, current_format);
       }

       runtime.run_in_main (sigc::bind (device_added.make_slot (), device));
     }
  }
}

void VideoInputCore::remove_device (const std::string & source, const std::string & device_name, unsigned capabilities, HalManager* /*manager*/)
{
  PTRACE(0, "VidInputCore\tRemoving Device " << device_name);
  PWaitAndSignal m(var_mutex);

  VideoInputDevice device;
  for (std::set<VideoInputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
     if ((*iter)->has_device (source, device_name, capabilities, device)) {
       if ( ( current_device.type   == device.type   ) &&
            ( current_device.source == device.source ) &&
            ( current_device.name   == device.name   ) ) {

            VideoInputDevice new_device;
            new_device.type   = VIDEO_INPUT_FALLBACK_DEVICE_TYPE;
            new_device.source = VIDEO_INPUT_FALLBACK_DEVICE_SOURCE;
            new_device.name   = VIDEO_INPUT_FALLBACK_DEVICE_NAME;
            internal_set_vidinput_device(new_device, current_channel, current_format);
       }

       runtime.run_in_main (sigc::bind (device_removed.make_slot (), device));
     }
  }
}

