
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


#ifndef __VIDEOOUTPUT_MANAGER_H__
#define __VIDEOOUTPUT_MANAGER_H__

#include "display-core.h"

namespace Ekiga
{

/**
 * @addtogroup videooutput
 * @{
 */

  /** Generic implementation for the Ekiga::VideoOutputManager class.
   *
   * Each VideoOutputManager represents a sink for video frames.
   * A VideoOutputManager can display the video signal, record single frames or record video signal.
   * The VideoOutputCore will control the different managers and pass the frames to all of them.
   */
  class VideoOutputManager
    {

  public:

      /** The constructor
       */
      VideoOutputManager () {}

      /** The destructor
       */
      ~VideoOutputManager () {}


      /*** API for video output ***/

      /** Open the device.
       * The device must be opened before calling set_frame_data().
       */
      virtual void open () { };

      /** Close the device.
       */
      virtual void close () { };

      /** Set one video frame buffer.
       * Requires the device to be opened.
       * @param data a pointer to the buffer with the data to be written. It will not be freed.
       * @param width the width in pixels of the frame to be written.
       * @param height the height in pixels of the frame to be written.
       * @param local true if the frame is a frame of the local video source, false if it is from the remote end.
       * @param devices_nbr 1 if only local or remote device has been opened, 2 if both have been opened. //FIXME
       */
      virtual void set_frame_data (const char *data,
                                   unsigned width,
                                   unsigned height,
                                   bool local,
                                   int devices_nbr) = 0;

      virtual void set_display_info (const DisplayInfo &) { };


      /*** API to act on VideoOutputDevice events ***/

      /** This signal is emitted when a video output device is opened.
       * @param hw_accel_status actual hardware acceleration support active on the video output device opened).
       */
      sigc::signal<void, VideoOutputAccel> device_opened;

      /** This signal is emitted when a video output device is closed.
       */
      sigc::signal<void> device_closed;

      sigc::signal<void, VideoOutputMode> videooutput_mode_changed;
      sigc::signal<void, FSToggle> fullscreen_mode_changed;
      sigc::signal<void, unsigned, unsigned> display_size_changed;
      sigc::signal<void> logo_update_required;

  protected:  
      virtual void get_display_info (DisplayInfo &) { };
    };

/**
 * @}
 */

};

#endif
