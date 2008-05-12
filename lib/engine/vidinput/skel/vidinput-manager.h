
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
 *                         vidinput-manager.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the interface of a videoinput manager
 *                          implementation backend.
 *
 */


#ifndef __VIDEOINPUT_MANAGER_H__
#define __VIDEOINPUT_MANAGER_H__

#include "vidinput-core.h"

namespace Ekiga
{

/**
 * @addtogroup videoinput
 * @{
 */

  

  class VideoInputManager
    {

  public:

      /* The constructor
       */
      VideoInputManager () {}

      /* The destructor
       */
      ~VideoInputManager () {}


      /*                 
       * VIDINOUT MANAGEMENT 
       */               

      /** Create a call based on the remote uri given as parameter
       * @param uri  an uri
       * @return     true if a Ekiga::Call could be created
       */

      virtual void get_devices (std::vector <VidInputDevice> & devices) = 0;

      virtual bool set_device (const VidInputDevice & device, int channel, VideoFormat format) = 0;

      virtual void set_image_data (unsigned /* width */, unsigned /* height */, const char* /*data*/ ) {};

      virtual bool open (unsigned width, unsigned height, unsigned fps) = 0;

      virtual void close() {};

      virtual bool get_frame_data (unsigned & width,
                           unsigned & height,
                           char *data) = 0;

      virtual void set_colour     (unsigned /* colour     */ ) {};
      virtual void set_brightness (unsigned /* brightness */ ) {};
      virtual void set_whiteness  (unsigned /* whiteness  */ ) {};
      virtual void set_contrast   (unsigned /* contrast   */ ) {};

      virtual bool has_device     (const std::string & source, const std::string & device_name, unsigned capabilities, VidInputDevice & device) = 0;
      
      sigc::signal<void, VidInputDevice, VidInputErrorCodes> device_error;
      sigc::signal<void, VidInputDevice, VidInputConfig> device_opened;
      sigc::signal<void, VidInputDevice> device_closed;

  protected:  
      typedef struct ManagerState {
        bool opened;
        unsigned width;
        unsigned height;
        unsigned fps;
        VidInputDevice device;
        VideoFormat format;
        int channel;
      };
      ManagerState current_state;
  };
/**
 * @}
 */

};

#endif
