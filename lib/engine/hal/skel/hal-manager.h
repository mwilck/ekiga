
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


      /*                 
       * VIDINOUT MANAGEMENT 
       */               

      /** Create a call based on the remote uri given as parameter
       * @param uri  an uri
       * @return     true if a Ekiga::Call could be created
       */

      sigc::signal<void, std::string&, std::string&, unsigned> video_input_device_added;
      sigc::signal<void, std::string&, std::string&, unsigned> video_input_device_removed;

      sigc::signal<void, std::string&, std::string&> audio_input_device_added;
      sigc::signal<void, std::string&, std::string&> audio_input_device_removed;

      sigc::signal<void, std::string&, std::string&> audio_output_device_added;
      sigc::signal<void, std::string&, std::string&> audio_output_device_removed;

      sigc::signal<void, std::string&, std::string&> network_interface_up;
      sigc::signal<void, std::string&, std::string&> network_interface_down;

  protected:  

  };
/**
 * @}
 */

};

#endif
