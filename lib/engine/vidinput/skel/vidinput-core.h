
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
 *                         vidinput-core.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the interface of a videoinput core.
 *                          A videoinput core manages VideoInputManagers.
 *
 */

#ifndef __VIDEOINPUT_CORE_H__
#define __VIDEOINPUT_CORE_H__

#include "services.h"
#include "runtime.h"
#include "display-core.h"
#include "hal-core.h"
#include "vidinput-gmconf-bridge.h"
#include "vidinput-info.h"

#include <sigc++/sigc++.h>
#include <glib.h>
#include <set>

#include "ptbuildopts.h"
#include "ptlib.h"

#define VIDEO_INPUT_FALLBACK_DEVICE_TYPE   "Moving Logo"
#define VIDEO_INPUT_FALLBACK_DEVICE_SOURCE "Moving Logo"
#define VIDEO_INPUT_FALLBACK_DEVICE_NAME   "Moving Logo"

namespace Ekiga
{
  typedef struct DeviceConfig {
    bool active;
    unsigned width;
    unsigned height;
    unsigned fps;
    VidInputConfig settings;
  };

  class VideoInputManager;
  class VideoInputCore;
				      
  class PreviewManager : public PThread
  {
    PCLASSINFO(PreviewManager, PThread);
  
  public:
    PreviewManager(VideoInputCore& _videoinput_core, VideoOutputCore& _videooutput_core);
    ~PreviewManager();
    virtual void start(unsigned width, unsigned height);
    virtual void stop();
  
  protected:
    void Main (void);
    bool stop_thread;
    char* frame;
    PMutex quit_mutex;     /* To exit */
    PSyncPoint thread_sync_point;
    VideoInputCore & videoinput_core;
    VideoOutputCore & videooutput_core;
  };

/**
 * @defgroup vidinput Video VidInput
 * @{
 */



  /** Core object for the video vidinput support
   */
  class VideoInputCore
    : public Service
    {

  public:

      /* The constructor
      */
      VideoInputCore (Ekiga::Runtime & _runtime, VideoOutputCore& _videooutput_core);

      /* The destructor
      */
      ~VideoInputCore ();

      void setup_conf_bridge();

      /*** Service Implementation ***/

      /** Returns the name of the service.
       * @return The service name.
       */
      const std::string get_name () const
        { return "videoinput-core"; }


      /** Returns the description of the service.
       * @return The service description.
       */
      const std::string get_description () const
        { return "\tVideoInput Core managing VideoInput Manager objects"; }


      /** Adds a VideoInputManager to the VideoInputCore service.
       * @param The manager to be added.
       */
       void add_manager (VideoInputManager &manager);

      /** Triggers a callback for all Ekiga::VideoInputManager sources of the
       * VideoInputCore service.
       */
       void visit_managers (sigc::slot<bool, VideoInputManager &> visitor);

      /** This signal is emitted when a Ekiga::VideoInputManager has been
       * added to the VideoInputCore Service.
       */
       sigc::signal<void, VideoInputManager &> manager_added;


      void get_devices(std::vector <VideoInputDevice> & devices);

      void set_device(const VideoInputDevice & device, int channel, VideoInputFormat format);
      
      /* To transmit a user specified image, pass a pointer to a raw YUV image*/      
      void set_image_data (unsigned width, unsigned height, const char* data);

      /*** VidInput Management ***/                 

      void set_preview_config (unsigned width, unsigned height, unsigned fps);
      
      void start_preview ();

      void stop_preview ();

      void set_stream_config (unsigned width, unsigned height, unsigned fps);

      void start_stream ();

      void stop_stream ();

      void get_frame_data (char *data,
                           unsigned & width,
                           unsigned & height);


      void set_colour     (unsigned colour);
      void set_brightness (unsigned brightness);
      void set_whiteness  (unsigned whiteness);
      void set_contrast   (unsigned contrast);

      void add_device (const std::string & source, const std::string & device_name, unsigned capabilities, HalManager* manager);
      void remove_device (const std::string & source, const std::string & device_name, unsigned capabilities, HalManager* manager);

      /*** VidInput Related Signals ***/
      
      /** See vidinput-manager.h for the API
       */
      sigc::signal<void, VideoInputManager &, VideoInputDevice &, VidInputConfig&> device_opened;
      sigc::signal<void, VideoInputManager &, VideoInputDevice &> device_closed;
      sigc::signal<void, VideoInputManager &, VideoInputDevice &, VideoInputErrorCodes> device_error;
      sigc::signal<void, VideoInputDevice> device_added;
      sigc::signal<void, VideoInputDevice> device_removed;

  private:
      void on_device_opened (VideoInputDevice device,  
                             VidInputConfig vidinput_config, 
                             VideoInputManager *manager);
      void on_device_closed (VideoInputDevice device, VideoInputManager *manager);
      void on_device_error  (VideoInputDevice device, VideoInputErrorCodes error_code, VideoInputManager *manager);

      void internal_set_vidinput_device(const VideoInputDevice & vidinput_device, int channel, VideoInputFormat format);
      void internal_open (unsigned width, unsigned height, unsigned fps);
      void internal_close();
      void internal_set_device (const VideoInputDevice & vidinput_device, int channel, VideoInputFormat format);
      void internal_set_fallback ();
      void apply_settings();


      std::set<VideoInputManager *> managers;

      Ekiga::Runtime & runtime;

      DeviceConfig preview_config;
      DeviceConfig stream_config;

      VidInputConfig new_stream_settings; 
      VidInputConfig new_preview_settings;

      VideoInputManager* current_manager;
      VideoInputDevice desired_device;
      VideoInputDevice current_device;
      Ekiga::VideoInputFormat current_format;
      int current_channel;

      PMutex var_mutex;      /* To protect variables that are read and written */
      PMutex set_mutex;      /* To protect variables that are read and written */

      PreviewManager preview_manager;
      VideoInputCoreConfBridge* videoinput_core_conf_bridge;
    };
/**
 * @}
 */
};

#endif
