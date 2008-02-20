
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
 *   description          : Declaration of the interface of a vidinput core.
 *                          A vidinput core manages VidInputManagers.
 *
 */

#ifndef __VIDINPUT_CORE_H__
#define __VIDINPUT_CORE_H__

#include "services.h"
#include "display-core.h"
#include "vidinput-gmconf-bridge.h"
#include "vidinput-info.h"

#include <sigc++/sigc++.h>
#include <glib.h>

#include "ptbuildopts.h"
#include "ptlib.h"

namespace Ekiga
{
  typedef struct DeviceConfig {
    bool active;
    unsigned width;
    unsigned height;
    unsigned fps;
  };

  class VidInputManager;
  class VidInputCore;
				      
  class PreviewManager : public PThread
  {
    PCLASSINFO(PreviewManager, PThread);
  
  public:
    PreviewManager(VidInputCore& _vidinput_core, DisplayCore& _display_core);
    ~PreviewManager();
    virtual void start(unsigned width, unsigned height);
    virtual void stop();
  
  protected:
    void Main (void);
    bool stop_thread;
    char* frame;
    PMutex quit_mutex;     /* To exit */
    PSyncPoint thread_sync_point;
    VidInputCore& vidinput_core;
    DisplayCore& display_core;
  };

/**
 * @defgroup vidinput Video VidInput
 * @{
 */



  /** Core object for the video vidinput support
   */
  class VidInputCore
    : public Service
    {

  public:

      /* The constructor
      */
      VidInputCore (DisplayCore& _display_core);

      /* The destructor
      */
      ~VidInputCore ();

      void setup_conf_bridge();

      /*** Service Implementation ***/

      /** Returns the name of the service.
       * @return The service name.
       */
      const std::string get_name () const
        { return "vidinput-core"; }


      /** Returns the description of the service.
       * @return The service description.
       */
      const std::string get_description () const
        { return "\tVidInput Core managing VidInput Manager objects"; }


      /** Adds a VidInputManager to the VidInputCore service.
       * @param The manager to be added.
       */
       void add_manager (VidInputManager &manager);

      /** Triggers a callback for all Ekiga::VidInputManager sources of the
       * VidInputCore service.
       */
       void visit_managers (sigc::slot<bool, VidInputManager &> visitor);

      /** This signal is emitted when a Ekiga::VidInputManager has been
       * added to the VidInputCore Service.
       */
       sigc::signal<void, VidInputManager &> manager_added;


      void get_vidinput_devices(std::vector <VidInputDevice> & vidinput_devices);

      void set_vidinput_device(const VidInputDevice & vidinput_device, int channel, VideoFormat format);
      
      /* To transmit a user specified image, pass a pointer to a raw YUV image*/      
      void set_image_data (unsigned width, unsigned height, const char* data);

      /*** VidInput Management ***/                 

      void set_preview_config (unsigned width, unsigned height, unsigned fps);
      
      void start_preview ();

      void stop_preview ();

      void set_stream_config (unsigned width, unsigned height, unsigned fps);

      void start_stream ();

      void stop_stream ();

      void get_frame_data (unsigned & width,
                           unsigned & height,
                           char *data);


      void set_colour     (unsigned colour);
      void set_brightness (unsigned brightness);
      void set_whiteness  (unsigned whiteness);
      void set_contrast   (unsigned contrast);

      /*** VidInput Related Signals ***/
      
      /** See vidinput-manager.h for the API
       */
      sigc::signal<void, VidInputManager &, VidInputErrorCodes> error;
      sigc::signal<void, VidInputManager &, VidInputDevice> vidinputdevice_added;
      sigc::signal<void, VidInputManager &, VidInputDevice> vidinputdevice_removed;

  private:
      void on_error (VidInputErrorCodes error_code, VidInputManager *manager);
      void on_vidinputdevice_added (VidInputDevice vidinput_device, VidInputManager *manager);
      void on_vidinputdevice_removed (VidInputDevice vidinput_device, VidInputManager *manager);

      void internal_open (unsigned width, unsigned height, unsigned fps);
      void internal_close();
      void internal_set_device (const VidInputDevice & vidinput_device, int channel, VideoFormat format);

      std::set<VidInputManager *> managers;

      DeviceConfig preview_config;
      DeviceConfig stream_config;

      VidInputManager* current_manager;
      VidInputDevice desired_device;
      VidInputDevice current_device;
      Ekiga::VideoFormat current_format;
      int current_channel;

      PMutex var_mutex;      /* To protect variables that are read and written */

      PreviewManager preview_manager;
      VidInputCoreConfBridge* vidinput_core_conf_bridge;
    };
/**
 * @}
 */
};

#endif
