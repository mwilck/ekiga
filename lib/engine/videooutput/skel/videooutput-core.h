
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras

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
 *                         videooutput-core.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Matthias Schneider
 *   copyright            : (c) 2007 by Matthias Schneider
 *   description          : declaration of the interface of a videooutput core.
 *                          A videooutput core manages VideoOutputManagers.
 *
 */

#ifndef __VIDEOOUTPUT_CORE_H__
#define __VIDEOOUTPUT_CORE_H__

#include "services.h"

#include "videooutput-gmconf-bridge.h"
#include "videooutput-info.h"

#include <sigc++/sigc++.h>
#include <set>
#include <map>

#include <glib.h>

#include "ptbuildopts.h"
#include "ptlib.h"

namespace Ekiga
{

/**
 * @defgroup videooutput
 * @{
 */

  class VideoOutputManager;

  /** Core object for the video display support
   */
  class VideoOutputCore
    : public Service
    {

  public:

      /* The constructor
      */
      VideoOutputCore ();

      /* The destructor
      */
      ~VideoOutputCore ();

      void setup_conf_bridge();

      /*** Service Implementation ***/

      /** Returns the name of the service.
       * @return The service name.
       */
      const std::string get_name () const
        { return "videooutput-core"; }


      /** Returns the description of the service.
       * @return The service description.
       */
      const std::string get_description () const
        { return "VideoOutput Core managing VideoOutput Manager objects"; }


      /** Adds a VideoOutputManager to the VideoOutputCore service.
       * @param The manager to be added.
       */
      void add_manager (VideoOutputManager &manager);

      /** Triggers a callback for all Ekiga::VideoOutputManager sources of the
       * VideoOutputCore service.
       */
      void visit_managers (sigc::slot<bool, VideoOutputManager &> visitor);

      /** This signal is emitted when a Ekiga::VideoOutputManager has been
       * added to the VideoOutputCore Service.
       */
      sigc::signal<void, VideoOutputManager &> manager_added;


      /*** Display Management ***/                 

      void start ();

      void stop ();

      void set_frame_data (const char *data,
                           unsigned width,
                           unsigned height,
                           bool local,
                           int devices_nbr);

      void set_display_info (const DisplayInfo & _display_info);

      /*** Display Related Signals ***/
      
      /** See display-manager.h for the API
       */
      sigc::signal<void, VideoOutputManager &, VideoOutputAccel> device_opened;
      sigc::signal<void, VideoOutputManager &> device_closed;
      sigc::signal<void, VideoOutputManager &, VideoOutputMode> videooutput_mode_changed;
      sigc::signal<void, VideoOutputManager &, FSToggle> fullscreen_mode_changed;
      sigc::signal<void, VideoOutputManager &, unsigned, unsigned> display_size_changed;
      sigc::signal<void, VideoOutputManager &> logo_update_required;

      /*** Statistics ***/
      void get_videooutput_stats (VideoOutputStats & _videooutput_stats) {
        _videooutput_stats = videooutput_stats;
      };
  private:
      void on_device_opened (VideoOutputAccel videooutput_accel, VideoOutputManager *manager);
      void on_device_closed (VideoOutputManager *manager);

      void on_videooutput_mode_changed (VideoOutputMode mode, VideoOutputManager *manager);
      void on_fullscreen_mode_changed (FSToggle toggle, VideoOutputManager *manager);
      void on_display_size_changed ( unsigned width, unsigned height, VideoOutputManager *manager);
      void on_logo_update_required (VideoOutputManager *manager);

      std::set<VideoOutputManager *> managers;

      VideoOutputStats videooutput_stats;
      GTimeVal last_stats;
      int number_times_started;

      PMutex var_mutex;     /* Protect start, stop and number_times_started */

      VideoOutputCoreConfBridge* videooutput_core_conf_bridge;
    };
/**
 * @}
 */
};

#endif
