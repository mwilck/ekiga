
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
 *                         display-core.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Matthias Schneider
 *   copyright            : (c) 2007 by Matthias Schneider
 *   description          : declaration of the interface of a display core.
 *                          A display core manages DisplayManagers.
 *
 */

#ifndef __DISPLAY_CORE_H__
#define __DISPLAY_CORE_H__

#include "services.h"

#include "display-info.h"

#include <sigc++/sigc++.h>
#include <set>
#include <map>

namespace Ekiga {
  class DisplayManager;

  class DisplayCore
    : public Service
    {

  public:

      /* The constructor
      */
      DisplayCore () {}

      /* The destructor
      */
      ~DisplayCore () {}


      /*** Service Implementation ***/

      /** Returns the name of the service.
       * @return The service name.
       */
      const std::string get_name () const
        { return "display-core"; }


      /** Returns the description of the service.
       * @return The service description.
       */
      const std::string get_description () const
        { return "\tDisplay Core managing Display Manager objects"; }


      /** Adds a DisplayManager to the DisplayCore service.
       * @param The manager to be added.
       */
      void add_manager (DisplayManager &manager);

      /** Triggers a callback for all Ekiga::DisplayManager sources of the
       * DisplayCore service.
       */
      void visit_managers (sigc::slot<void, DisplayManager &> visitor);

      /** This signal is emitted when a Ekiga::DisplayManager has been
       * added to the DisplayCore Service.
       */
      sigc::signal<void, DisplayManager &> manager_added;


      /*** Display Management ***/                 

      void start ();

      void stop ();

      void setFrameData (unsigned width,
                         unsigned height,
                         const char *data,
                         bool local,
                         int devices_nbr);

      void setVideoInfo (const DisplayInfo & newVideoInfo);

      /*** Display Related Signals ***/
      
      /** See display-manager.h for the API
       */
      sigc::signal<void, DisplayManager &, DisplayMode> display_type_changed;       /* gm_main_window_set_display_type */
      sigc::signal<void, DisplayManager &, FSToggle_new> fullscreen_mode_changed;     /* gm_main_window_toggle_fullscreen */
      sigc::signal<void, DisplayManager &, unsigned, unsigned> size_changed;          /* gm_main_window_set_resized_video_widget */
      sigc::signal<void, DisplayManager &> logo_update_required;                      /* gm_main_window_update_logo  */
      sigc::signal<void, DisplayManager &> video_info_update_required;                /* gm_main_window_update_zoom_display */
//      sigc::signal<void, DisplayManager &, VideoAccelStatus> update_video_accel_status; /* gm_main_window_update_video_accel_status */


  private:
      void on_display_type_changed (DisplayMode display, DisplayManager *manager);
      void on_fullscreen_mode_changed (FSToggle_new toggle, DisplayManager *manager);
      void on_size_changed ( unsigned width, unsigned height, DisplayManager *manager);
      void on_logo_update_required (DisplayManager *manager);
      void on_video_info_update_required (DisplayManager *manager);

      std::set<DisplayManager *> managers;
    };  
};

#endif
