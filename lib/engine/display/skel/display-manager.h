
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
 *                         display-manager.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Matthias Schneider
 *   copyright            : (c) 2007 by Matthias Schneider
 *   description          : Declaration of the interface of a display manager
 *                          implementation backend.
 *
 */


#ifndef __DISPLAY_MANAGER_H__
#define __DISPLAY_MANAGER_H__

#include "display-core.h"

namespace Ekiga {

  class DisplayManager
    {

  public:

      /* The constructor
       */
      DisplayManager () {}

      /* The destructor
       */
      ~DisplayManager () {}


      /*                 
       * DISPLAY MANAGEMENT 
       */               

      /** Create a call based on the remote uri given as parameter
       * @param: An uri
       * @return: true if a Ekiga::Call could be created
       */
      virtual void start () { };

      virtual void stop () { };

      virtual void set_frame_data (unsigned width,
                                   unsigned height,
                                   const char *data,
                                   bool local,
                                   int devices_nbr) = 0;

      virtual void set_display_info (const DisplayInfo &) { };

      sigc::signal<void, DisplayMode> display_mode_changed;       /* gm_main_window_set_display_type */
      sigc::signal<void, FSToggle> fullscreen_mode_changed;   /* gm_main_window_toggle_fullscreen */
      sigc::signal<void, unsigned, unsigned> display_size_changed;        /* gm_main_window_set_resized_video_widget */
      sigc::signal<void> logo_update_required;                    /* gm_main_window_update_logo  */
      sigc::signal<void> display_info_update_required;            /* gm_main_window_update_zoom_display */
//      sigc::signal<void, DisplayManager &, VideoAccelStatus> update_video_accel_status; /* gm_main_window_update_video_accel_status */

  protected:  
      virtual void get_display_info (DisplayInfo &) { };
    };
};

#endif
