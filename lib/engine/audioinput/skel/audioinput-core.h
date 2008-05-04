
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
 *                         audioinput-core.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the interface of a audioinput core.
 *                          A audioinput core manages AudioInputManagers.
 *
 */

#ifndef __AUDIOINPUT_CORE_H__
#define __AUDIOINPUT_CORE_H__

#include "services.h"
#include "runtime.h"
#include "audiooutput-core.h"
#include "hal-core.h"
#include "audioinput-gmconf-bridge.h"
#include "audioinput-info.h"

#include <sigc++/sigc++.h>
#include <glib.h>
#include <set>

#include "ptbuildopts.h"
#include "ptlib.h"

#define AUDIO_INPUT_FALLBACK_DEVICE_TYPE "NULL"
#define AUDIO_INPUT_FALLBACK_DEVICE_SOURCE "NULL"
#define AUDIO_INPUT_FALLBACK_DEVICE_DEVICE "NULL"

namespace Ekiga
{
  typedef struct AudioDeviceConfig {
    bool active;
    unsigned channels;
    unsigned samplerate;
    unsigned bits_per_sample;
    unsigned buffer_size;
    unsigned num_buffers;
    unsigned volume;
  };

  class AudioInputManager;
  class AudioInputCore;
				      
  class AudioPreviewManager : public PThread
  {
    PCLASSINFO(AudioPreviewManager, PThread);
  
  public:
    AudioPreviewManager(AudioInputCore& _audio_input_core, AudioOutputCore& _audio_output_core);
    ~AudioPreviewManager();
    virtual void start(){};
    virtual void stop(){};
  
  protected:
    void Main (void);
    bool stop_thread;
    char* frame;
    PMutex quit_mutex;     /* To exit */
    PSyncPoint thread_sync_point;
    AudioInputCore& audio_input_core;
    AudioOutputCore& audio_output_core;
  };

/**
 * @defgroup audioinput Audio AudioInput
 * @{
 */



  /** Core object for the audio auioinput support
   */
  class AudioInputCore
    : public Service
    {

  public:

      /* The constructor
      */
      AudioInputCore (Ekiga::Runtime & _runtime, AudioOutputCore& _audio_output_core);

      /* The destructor
      */
      ~AudioInputCore ();

      void setup_conf_bridge();

      /*** Service Implementation ***/

      /** Returns the name of the service.
       * @return The service name.
       */
      const std::string get_name () const
        { return "audioinput-core"; }


      /** Returns the description of the service.
       * @return The service description.
       */
      const std::string get_description () const
        { return "\tAudioInput Core managing AudioInput Manager objects"; }


      /** Adds a AudioInputManager to the AudioInputCore service.
       * @param The manager to be added.
       */
       void add_manager (AudioInputManager &manager);

      /** Triggers a callback for all Ekiga::AudioInputManager sources of the
       * AudioInputCore service.
       */
       void visit_managers (sigc::slot<bool, AudioInputManager &> visitor);

      /** This signal is emitted when a Ekiga::AudioInputManager has been
       * added to the AudioInputCore Service.
       */
       sigc::signal<void, AudioInputManager &> manager_added;


      void get_audioinput_devices(std::vector <AudioInputDevice> & audioinput_devices);

      void set_audioinput_device(const AudioInputDevice & audioinput_device);
      
      /*** VidInput Management ***/                 

      void start_preview (unsigned channels, unsigned samplerate, unsigned bits_per_sample);

      void stop_preview ();

      void set_stream_buffer_size (unsigned buffer_size, unsigned num_buffers);

      void start_stream (unsigned channels, unsigned samplerate, unsigned bits_per_sample);

      void stop_stream ();

      void get_frame_data (char *data, unsigned size, unsigned & bytes_read);

      void set_volume (unsigned volume);

      void start_average_collection () { calculate_average = true; }

      void stop_average_collection () { calculate_average = false; }

      float get_average_level () { return average_level; }

      void add_device (const std::string & source, const std::string & device, HalManager* manager);
      void remove_device (const std::string & source, const std::string & device, HalManager* manager);

      /*** VidInput Related Signals ***/
      
      /** See audioinput-manager.h for the API
       */
      sigc::signal<void, AudioInputManager &, AudioInputDevice &, AudioInputErrorCodes> audioinputdevice_error;
      sigc::signal<void, AudioInputDevice> audioinputdevice_added;
      sigc::signal<void, AudioInputDevice> audioinputdevice_removed;
      sigc::signal<void, AudioInputManager &, AudioInputDevice &, AudioInputConfig&> audioinputdevice_opened;
      sigc::signal<void, AudioInputManager &, AudioInputDevice &> audioinputdevice_closed;

  private:
      void on_audioinputdevice_error (AudioInputDevice audioinput_device, AudioInputErrorCodes error_code, AudioInputManager *manager);
      void on_audioinputdevice_opened (AudioInputDevice audioinput_device,  
                                       AudioInputConfig vidinput_config, 
                                       AudioInputManager *manager);
      void on_audioinputdevice_closed (AudioInputDevice audioinput_device, AudioInputManager *manager);

      void internal_set_audioinput_device(const AudioInputDevice & audioinput_device);
      void internal_open (unsigned channels, unsigned samplerate, unsigned bits_per_sample);
      void internal_close();
      void internal_set_device (const AudioInputDevice & audioinput_device);
      void internal_set_fallback();
      void calculate_average_level (const short *buffer, unsigned size);

      Ekiga::Runtime & runtime;
	    
      std::set<AudioInputManager *> managers;

      AudioDeviceConfig preview_config;
      AudioDeviceConfig stream_config;
      unsigned new_preview_volume;
      unsigned new_stream_volume;

      AudioInputManager* current_manager;
      AudioInputDevice desired_device;
      AudioInputDevice current_device;

      PMutex var_mutex;      /* To protect variables that are read and written */
      PMutex vol_mutex;      /* To protect variables that are read and written */

      AudioPreviewManager preview_manager;
      AudioInputCoreConfBridge* audioinput_core_conf_bridge;

      float average_level;
      bool calculate_average;
    };
/**
 * @}
 */
};

#endif
