
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
 *                         hal-manager.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the interface of a hal manager
 *                          implementation backend.
 *
 */


#ifndef __HAL_MANAGER_H__
#define __HAL_MANAGER_H__

#include "hal-core.h"

namespace Ekiga
{

/**
 * @addtogroup hal
 * @{
 */

  class HalManager
    {

  public:

      /* The constructor
       */
      HalManager () {}

      /* The destructor
       */
      ~HalManager () {}


      /*** API to act on HAL events ***/

      /** This signal is emitted when a video input device is added to the system
       * @param: source         : the video input framework (e.g. video4linux, etc.)
       *         device         : the device name
       *         capabilities   : source-dependent device capabilites (e.g. V4L1 or V4L2 for video4linux)
       */
      sigc::signal<void, std::string, std::string, unsigned> video_input_device_added;

      /** This signal is emitted when a video input device is removed from the system
       * @param: source         : the video input framework (e.g. video4linux, etc.)
       *         device         : the device name
       *         capabilities   : source-dependent device capabilites (e.g. V4L1 or V4L2 for video4linux)
       */
      sigc::signal<void, std::string, std::string, unsigned> video_input_device_removed;

      /** This signal is emitted when an audio input device is added to the system
       * @param: source         : the audio input framework (e.g. alsa, oss, etc.)
       *         device         : the device name
       */
      sigc::signal<void, std::string, std::string> audio_input_device_added;

      /** This signal is emitted when an audio input device is removed from the system
       * @param: source         : the audio input framework (e.g. alsa, oss, etc.)
       *         device         : the device name
       */
      sigc::signal<void, std::string, std::string> audio_input_device_removed;

      /** This signal is emitted when an audio output device is added to the system
       * @param: source         : the audio output framework (e.g. alsa, oss, etc.)
       *         device         : the device name
       */
      sigc::signal<void, std::string, std::string> audio_output_device_added;

      /** This signal is emitted when an audio output device is removed from the system
       * @param: source         : the audio output framework (e.g. alsa, oss, etc.)
       *         device         : the device name
       */
      sigc::signal<void, std::string, std::string> audio_output_device_removed;

      /** This signal is emitted when a network device comes up
       * @param: interface_name : The interface name (e.g. eth0, etc.)
       *         ip4_address    : The IPv4 address (e.g. "192.168.0.1")
       */
      sigc::signal<void, std::string, std::string> network_interface_up;

      /** This signal is emitted when a network device goes down
       * @param: interface_name : The interface name (e.g. eth0, etc.)
       *         ip4_address    : The IPv4 address (e.g. "192.168.0.1")
       */
      sigc::signal<void, std::string, std::string> network_interface_down;
  };

/**
 * @}
 */

};

#endif
