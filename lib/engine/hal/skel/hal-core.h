
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
 *                         hal-core.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the interface of a hal core.
 *                          A hal core manages HalManagers.
 *
 */

#ifndef __HAL_CORE_H__
#define __HAL_CORE_H__

#include "services.h"

#include <sigc++/sigc++.h>

#include <set>
#include <map>

namespace Ekiga
{
/**
 * @defgroup hal
 * @{
 */

  class HalManager;

  typedef struct HalVideoInputDevice {
    std::string category;
    std::string name;
  };

  typedef struct HalAudioInputDevice {
    std::string category;
    std::string name;
  };

  typedef struct HalAudioOutputDevice {
    std::string category;
    std::string name;
  };

  typedef struct HalNetworkInterface {
    std::string name;
  };

  /** Core object for hal support
   */
  class HalCore
    : public Service
    {

  public:

      /* The constructor
      */
      HalCore ();

      /* The destructor
      */
      ~HalCore ();

      /*** Service Implementation ***/

      /** Returns the name of the service.
       * @return The service name.
       */
      const std::string get_name () const
        { return "hal-core"; }


      /** Returns the description of the service.
       * @return The service description.
       */
      const std::string get_description () const
        { return "\tHal Core managing Hal Manager objects"; }


      /** Adds a HalManager to the HalCore service.
       * @param The manager to be added.
       */
       void add_manager (HalManager &manager);

      /** Triggers a callback for all Ekiga::HalManager sources of the
       * HalCore service.
       */
       void visit_managers (sigc::slot<bool, HalManager &> visitor);

      /** This signal is emitted when a Ekiga::HalManager has been
       * added to the HalCore Service.
       */
       sigc::signal<void, HalManager &> manager_added;

      /** See hal-manager.h for the API
       */
      sigc::signal<void, HalVideoInputDevice &> video_input_device_added;
      sigc::signal<void, HalVideoInputDevice &> video_input_device_removed;

      sigc::signal<void, HalAudioInputDevice &> audio_input_device_added;
      sigc::signal<void, HalAudioInputDevice &> audio_input_device_removed;

      sigc::signal<void, HalAudioOutputDevice &> audio_output_device_added;
      sigc::signal<void, HalAudioOutputDevice &> audio_output_device_removed;

      sigc::signal<void, HalNetworkInterface &> network_interface_up;
      sigc::signal<void, HalNetworkInterface &> network_interface_down;

  private:

      void on_video_input_device_added (HalVideoInputDevice & video_input_device);
      void on_video_input_device_removed (HalVideoInputDevice & video_input_device);

      void on_audio_input_device_added (HalAudioInputDevice & audio_input_device);
      void on_audio_input_device_removed (HalAudioInputDevice & audio_input_device);

      void on_audio_output_device_added (HalAudioOutputDevice & audio_output_device);
      void on_audio_output_device_removed (HalAudioOutputDevice & audio_output_device);

      void on_network_interface_up (HalNetworkInterface & network_interface);
      void on_network_interface_down (HalNetworkInterface & network_interface);

      std::set<HalManager *> managers;

    };
/**
 * @}
 */
};

#endif
