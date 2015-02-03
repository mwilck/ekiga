/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>

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
 *                         videooutput-manager.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Matthias Schneider
 *   copyright            : (c) 2007 by Matthias Schneider
 *   description          : Declaration of the interface of a videooutput manager
 *                          implementation backend.
 *
 */


#ifndef __VIDEOOUTPUT_MANAGER_H__
#define __VIDEOOUTPUT_MANAGER_H__

#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <glib.h>

#include "videooutput-core.h"

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
   */
  class VideoOutputManager
    {

  public:
      typedef enum { LOCAL, REMOTE, EXTENDED, MAX_VIEWS } VideoView;

      /** The constructor
       */
      VideoOutputManager () {}

      /** The destructor
       */
      virtual ~VideoOutputManager () {}

      virtual void quit () { };

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
       * @param type the type of the frame: 0 - local video source or >0 from the remote end.
       * @param devices_nbr 1 if only local or remote device has been opened, 2 if both have been opened.
       */
      virtual void set_frame_data (const char *data,
                                   unsigned width,
                                   unsigned height,
                                   VideoView type,
                                   int devices_nbr) = 0;

      virtual void set_display_info (G_GNUC_UNUSED const gpointer local,
                                     G_GNUC_UNUSED const gpointer remote) { };
      virtual void set_ext_display_info (G_GNUC_UNUSED const gpointer ext) { };


      /*** API to act on VideoOutputDevice events ***/

      /** This signal is emitted when a video output device is opened.
       * @param type is the opened device VideoView (local, remote, extended).
       * @param width is the opened device width.
       * @param height is the opened device height.
       * @param both_streams if a frame from both local and remote stream has been received.
       * @param ext_stream if a frame from an extended video stream has been received.
       */
      boost::signals2::signal<void(VideoView, unsigned, unsigned, bool, bool)> device_opened;

      /** This signal is emitted when a video output device is closed.
       */
      boost::signals2::signal<void(void)> device_closed;

      /** This signal is emitted when an error occurs when opening a video output device.
       */
      boost::signals2::signal<void(void)> device_error;

      /** This signal is emitted the video output size has changed.
       * This signal is called whenever the size of the widget carrying the video signal
       * has to be changed. This happens when the displayed video changes in resolution.
       * @param type the current device VideoView.
       * @param width the new width of the widget.
       * @param height the new height of the widget.
       */
      boost::signals2::signal<void(VideoView, unsigned, unsigned)> size_changed;

  protected:
    };

/**
 * @}
 */

};

#endif
