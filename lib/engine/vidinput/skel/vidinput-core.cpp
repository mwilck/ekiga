
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

#include <iostream>
#include <sstream>

#include "config.h"

#include "vidinput-core.h"
#include "vidinput-manager.h"

#define FALLBACK_DEVICE_TYPE "Moving Logo"
#define FALLBACK_DEVICE_SOURCE "Moving Logo"
#define FALLBACK_DEVICE_DEVICE "Moving Logo"

using namespace Ekiga;

PreviewManager::PreviewManager (VidInputCore& _vidinput_core, DisplayCore& _display_core)
: PThread (1000, NoAutoDeleteThread, HighestPriority, "PreviewManager"),
    vidinput_core (_vidinput_core),
  display_core (_display_core)
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

  display_core.start();
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
  display_core.stop();
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

    vidinput_core.get_frame_data(width, height, frame);
    display_core.set_frame_data(width, height, frame, true, 1);

    // We have to sleep some time outside the mutex lock
    // to give other threads time to get the mutex
    // It will be taken into account by PAdaptiveDelay
    Current()->Sleep (5);
  }
}

VidInputCore::VidInputCore (Ekiga::Runtime & _runtime, DisplayCore& _display_core)
:  runtime (_runtime),
   preview_manager(*this, _display_core)
{
  PWaitAndSignal m(var_mutex);

  preview_config.active = false;
  preview_config.width = 176;
  preview_config.height = 144;
  preview_config.fps = 30;


  stream_config.active = false;
  stream_config.width = 176;
  stream_config.height = 144;
  stream_config.fps = 30;

  current_manager = NULL;
  vidinput_core_conf_bridge = NULL;
}

VidInputCore::~VidInputCore ()
{
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif

  PWaitAndSignal m(var_mutex);

  if (vidinput_core_conf_bridge)
    delete vidinput_core_conf_bridge;
}

void VidInputCore::setup_conf_bridge ()
{
  PWaitAndSignal m(var_mutex);

  vidinput_core_conf_bridge = new VidInputCoreConfBridge (*this);
}

void VidInputCore::add_manager (VidInputManager &manager)
{
  managers.insert (&manager);
  manager_added.emit (manager);

  manager.vidinputdevice_error.connect (sigc::bind (sigc::mem_fun (this, &VidInputCore::on_vidinputdevice_error), &manager));
  manager.vidinputdevice_opened.connect (sigc::bind (sigc::mem_fun (this, &VidInputCore::on_vidinputdevice_opened), &manager));
  manager.vidinputdevice_closed.connect (sigc::bind (sigc::mem_fun (this, &VidInputCore::on_vidinputdevice_closed), &manager));
}


void VidInputCore::visit_managers (sigc::slot<bool, VidInputManager &> visitor)
{
  PWaitAndSignal m(var_mutex);
  bool go_on = true;
  
  for (std::set<VidInputManager *>::iterator iter = managers.begin ();
       iter != managers.end () && go_on;
       iter++)
      go_on = visitor (*(*iter));
}		      

void VidInputCore::get_vidinput_devices (std::vector <VidInputDevice> & vidinput_devices)
{
  PWaitAndSignal m(var_mutex);

  vidinput_devices.clear();
  
  for (std::set<VidInputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++)
    (*iter)->get_vidinput_devices (vidinput_devices);

  if (PTrace::CanTrace(4)) {
     for (std::vector<VidInputDevice>::iterator iter = vidinput_devices.begin ();
         iter != vidinput_devices.end ();
         iter++) {
      PTRACE(4, "VidInputCore\tDetected Device: " << iter->type << "/" << iter->source << "/" << iter->device);
    }
  }
}

void VidInputCore::set_vidinput_device(const VidInputDevice & vidinput_device, int channel, VideoFormat format)
{
  PTRACE(4, "VidInputCore\tSetting device: " << vidinput_device.type << "/" << vidinput_device.source << "/" << vidinput_device.device);

  if ( ( desired_device.type   != vidinput_device.type   ) ||
       ( desired_device.source != vidinput_device.source ) ||
       ( desired_device.device != vidinput_device.device ) ||
       ( current_channel       != channel ) ||
       ( current_format        != format  ) ) {

    if (preview_config.active && !stream_config.active)
      preview_manager.stop();

    if (preview_config.active || stream_config.active)
      internal_close();

    internal_set_device (vidinput_device, channel, format);

    if (preview_config.active && !stream_config.active) {
      internal_open(preview_config.width, preview_config.height, preview_config.fps);
      preview_manager.start(preview_config.width,preview_config.height);
    }

    if (stream_config.active)
      internal_open(stream_config.width, stream_config.height, stream_config.fps);

  }
  desired_device  = vidinput_device;
}


void VidInputCore::set_preview_config (unsigned width, unsigned height, unsigned fps)
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


void VidInputCore::start_preview ()
{
  PWaitAndSignal m(var_mutex);

  PTRACE(4, "VidInputCore\tStarting preview " << preview_config.width << "x" << preview_config.height << "/" << preview_config.fps);
  if (!preview_config.active && !stream_config.active) {
    internal_open(preview_config.width, preview_config.height, preview_config.fps);
    preview_manager.start(preview_config.width,preview_config.height);
  }

  preview_config.active = true;
}

void VidInputCore::stop_preview ()
{
  PWaitAndSignal m(var_mutex);

  PTRACE(4, "VidInputCore\tStopping Preview");
  if (preview_config.active && !stream_config.active) {
    preview_manager.stop();
    internal_close();
  }

  preview_config.active = false;
}

void VidInputCore::set_stream_config (unsigned width, unsigned height, unsigned fps)
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

void VidInputCore::start_stream ()
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

void VidInputCore::stop_stream ()
{
  PWaitAndSignal m(var_mutex);

  PTRACE(4, "VidInputCore\tStopping Stream");
  if (preview_config.active && stream_config.active) {
    if ( preview_config.width != stream_config.width ||
         preview_config.height != stream_config.height ||
         preview_config.fps != stream_config.fps ) 
    {
      internal_close();
      internal_open(preview_config.width, preview_config.height, preview_config.fps);
    }
    preview_manager.start(preview_config.width, preview_config.height);
  }

  if (!preview_config.active && stream_config.active) {
    internal_close();
  }

  stream_config.active = false;
}

void VidInputCore::get_frame_data (unsigned & width,
                                   unsigned & height,
                                   char *data)
{
  PWaitAndSignal m(var_mutex);

  if (current_manager) {
    if (!current_manager->get_frame_data(width, height, data)) {

      internal_close();

      internal_set_fallback();

      if (preview_config.active && !stream_config.active)
        internal_open(preview_config.width, preview_config.height, preview_config.fps);

      if (stream_config.active)
        internal_open(stream_config.width, stream_config.height, stream_config.fps);

      if (current_manager)
        current_manager->get_frame_data(width, height, data); // the default device must always return true
    }
  }
}

void VidInputCore::set_colour (unsigned colour)
{
  PWaitAndSignal m(var_mutex);

  if (current_manager)
    current_manager->set_colour (colour);
}

void VidInputCore::set_brightness (unsigned brightness)
{
  PWaitAndSignal m(var_mutex);

  if (current_manager)
    current_manager->set_brightness (brightness);
}

void VidInputCore::set_whiteness  (unsigned whiteness)
{
  PWaitAndSignal m(var_mutex);

  if (current_manager)
    current_manager->set_whiteness (whiteness);
}

void VidInputCore::set_contrast   (unsigned contrast)
{
  PWaitAndSignal m(var_mutex);

  if (current_manager)
    current_manager->set_contrast (contrast);
}

void VidInputCore::on_vidinputdevice_error (VidInputDevice vidinput_device, VidInputErrorCodes error_code, VidInputManager *manager)
{
  vidinputdevice_error.emit (*manager, vidinput_device, error_code);
}

void VidInputCore::on_vidinputdevice_opened (VidInputDevice vidinput_device,
                                             VidInputConfig vidinput_config, 
                                             VidInputManager *manager)
{
  vidinputdevice_opened.emit (*manager, vidinput_device, vidinput_config);
}

void VidInputCore::on_vidinputdevice_closed (VidInputDevice vidinput_device, VidInputManager *manager)
{
  vidinputdevice_closed.emit (*manager, vidinput_device);
}

void VidInputCore::internal_open (unsigned width, unsigned height, unsigned fps)
{
  PTRACE(4, "VidInputCore\tOpening device with " << width << "x" << height << "/" << fps );

  if (current_manager && !current_manager->open(width, height, fps)) {

    internal_set_fallback();
    if (current_manager)
      current_manager->open(width, height, fps);
  }
}

void VidInputCore::internal_set_device (const VidInputDevice & vidinput_device, int channel, VideoFormat format)
{
  current_manager = NULL;
  for (std::set<VidInputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
     if ((*iter)->set_vidinput_device (vidinput_device, channel, format)) {
       current_manager = (*iter);
     }
  }

  // If the desired manager could not be found,
  // we se the default device. The default device
  // MUST ALWAYS be loaded and openable
  if (current_manager) {
    current_device  = vidinput_device;
  }
  else {
    PTRACE(1, "VidInputCore\tTried to set unexisting device " << vidinput_device.type << "/" << vidinput_device.source << "/" << vidinput_device.device);
    internal_set_fallback();
  }

  current_channel = channel;
  current_format  = format;
}

void VidInputCore::internal_close()
{
  PTRACE(4, "VidInputCore\tClosing current device");
  if (current_manager)
    current_manager->close();
}

void VidInputCore::internal_set_fallback ()
{
  PTRACE(3, "VidInputCore\tFalling back to " << FALLBACK_DEVICE_TYPE << "/" << FALLBACK_DEVICE_SOURCE << "/" << FALLBACK_DEVICE_DEVICE);
  current_device.type = FALLBACK_DEVICE_TYPE;
  current_device.source = FALLBACK_DEVICE_SOURCE;
  current_device.device = FALLBACK_DEVICE_DEVICE;

  internal_set_device(current_device, current_channel, current_format);
}

void VidInputCore::add_device (std::string & source, std::string & device, unsigned capabilities, HalManager* /*manager*/)
{
  PTRACE(0, "VidInputCore\tAdding Device");
  VidInputDevice vidinput_device;
  for (std::set<VidInputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
     if ((*iter)->has_device (source, device, capabilities, vidinput_device)) {

       if ( ( desired_device.type   == vidinput_device.type   ) &&
            ( desired_device.source == vidinput_device.source ) &&
            ( desired_device.device == vidinput_device.device ) ) {
         set_vidinput_device(desired_device, current_channel, current_format);
       }

       runtime.run_in_main (sigc::bind (vidinputdevice_added.make_slot (), vidinput_device));
     }
  }
}

void VidInputCore::remove_device (std::string & source, std::string & device, unsigned capabilities, HalManager* /*manager*/)
{
  PTRACE(0, "VidInputCore\tRemoving Device");
  VidInputDevice vidinput_device;
  for (std::set<VidInputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
     if ((*iter)->has_device (source, device, capabilities, vidinput_device)) {
       runtime.run_in_main (sigc::bind (vidinputdevice_removed.make_slot (), vidinput_device));
     }
  }
}

