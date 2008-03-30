
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
 *                         audioinput-manager.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the interface of an audioinput manager
 *                          implementation backend.
 *
 */


#ifndef __AUDIOINPUT_MANAGER_H__
#define __AUDIOINPUT_MANAGER_H__

#include "audiooutput-core.h"

namespace Ekiga
{

/**
 * @addtogroup vidinput
 * @{
 */

  typedef struct ManagerState {
    bool opened;
    unsigned channels;
    unsigned samplerate;
    unsigned bits_per_sample;
    AudioOutputDevice audiooutput_device;
  };
  

  class AudioOutputManager
    {

  public:

      /* The constructor
       */
      AudioOutputManager () {}

      /* The destructor
       */
      ~AudioOutputManager () {}


      /*                 
       * AUDIOINPUT MANAGEMENT 
       */               

      /** Create a call based on the remote uri given as parameter
       * @param uri  an uri
       * @return     true if a Ekiga::Call could be created
       */

      virtual void get_audiooutput_devices (std::vector <AudioOutputDevice> & audiooutput_devices) = 0;

      virtual bool set_audiooutput_device (AudioOutputPrimarySecondary primarySecondary, const AudioOutputDevice & audiooutput_device) = 0;

      virtual bool open (AudioOutputPrimarySecondary primarySecondary, unsigned channels, unsigned samplerate, unsigned bits_per_sample) = 0;

      virtual void set_buffer_size (AudioOutputPrimarySecondary /*primarySecondary*/, unsigned /*buffer_size*/, unsigned /*num_buffers*/) {};

      virtual bool set_frame_data (AudioOutputPrimarySecondary primarySecondary, 
                                   char *data, 
                                   unsigned size,
				   unsigned & written) = 0;

      virtual void close(AudioOutputPrimarySecondary /*primarySecondary*/) {};

      virtual void set_volume (AudioOutputPrimarySecondary /*primarySecondary*/, unsigned /* volume */ ) {};

      virtual bool has_device     (const std::string & sink, const std::string & device, AudioOutputDevice & audiooutput_device) = 0;
      
      sigc::signal<void, AudioOutputPrimarySecondary, AudioOutputDevice, AudioOutputErrorCodes> audiooutputdevice_error;
      sigc::signal<void, AudioOutputPrimarySecondary, AudioOutputDevice, AudioOutputConfig> audiooutputdevice_opened;
      sigc::signal<void, AudioOutputPrimarySecondary, AudioOutputDevice> audiooutputdevice_closed;

  protected:  
      ManagerState current_state[2];
  };
/**
 * @}
 */

};

#endif
