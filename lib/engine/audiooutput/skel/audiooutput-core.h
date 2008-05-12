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
 *                         audiooutput-core.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the interface of a audiooutput core.
 *                          A audioout core manages AudioOutputManagers.
 *
 */

#ifndef __AUDIOOUTPUT_CORE_H__
#define __AUDIOOUTPUT_CORE_H__

#include "services.h"
#include "runtime.h"
#include "audiooutput-core.h"
#include "hal-core.h"
#include "audiooutput-gmconf-bridge.h"
#include "audiooutput-info.h"
#include "audiooutput-scheduler.h"

#include <sigc++/sigc++.h>
#include <glib.h>
#include <set>

#include "ptbuildopts.h"
#include "ptlib.h"

#define AUDIO_OUTPUT_FALLBACK_DEVICE_TYPE "NULL"
#define AUDIO_OUTPUT_FALLBACK_DEVICE_SOURCE "NULL"
#define AUDIO_OUTPUT_FALLBACK_DEVICE_DEVICE "NULL"

namespace Ekiga
{
  typedef struct AudioOutputDeviceConfig {
    bool active;
    unsigned channels;
    unsigned samplerate;
    unsigned bits_per_sample;
    unsigned buffer_size;
    unsigned num_buffers;
    unsigned volume;
  };

/**
 * @defgroup audiooutput Audio AudioInput
 * @{
 */

   class AudioOutputManager;
   class AudioOutputCore;

  /** Core object for the audio auioinput support
   */
  class AudioOutputCore
    : public Service
    {

  public:

      /* The constructor
      */
      AudioOutputCore (Ekiga::Runtime & _runtime);

      /* The destructor
      */
      ~AudioOutputCore ();

      void setup_conf_bridge();

      /*** Service Implementation ***/

      /** Returns the name of the service.
       * @return The service name.
       */
      const std::string get_name () const
        { return "audiooutput-core"; }


      /** Returns the description of the service.
       * @return The service description.
       */
      const std::string get_description () const
        { return "\tAudioOutput Core managing AudioOutput Manager objects"; }


      /** Adds a AudioOutputManager to the AudioOutputCore service.
       * @param The manager to be added.
       */
       void add_manager (AudioOutputManager &manager);

      /** Triggers a callback for all Ekiga::AudioOutputManager sources of the
       * AudioOutputCore service.
       */
       void visit_managers (sigc::slot<bool, AudioOutputManager &> visitor);

      /** This signal is emitted when a Ekiga::AudioOutputManager has been
       * added to the AudioOutputCore Service.
       */
       sigc::signal<void, AudioOutputManager &> manager_added;

      void add_event (const std::string & event_name, const std::string & file_name, bool enabled, AudioOutputPrimarySecondary primarySecondary);

      void play_file (const std::string & file_name);

      void play_event (const std::string & event_name);

      void start_play_event (const std::string & event_name, unsigned interval, unsigned repetitions);

      void stop_play_event (const std::string & event_name);


      void get_audiooutput_devices(std::vector <AudioOutputDevice> & audiooutput_devices);

      void set_audiooutput_device(AudioOutputPrimarySecondary primarySecondary, const AudioOutputDevice & audiooutput_device);
      
      /*** VidInput Management ***/                 

      void start (unsigned channels, unsigned samplerate, unsigned bits_per_sample);

      void stop ();

      void set_buffer_size (unsigned buffer_size, unsigned num_buffers);

      void set_frame_data (char *data, unsigned size, unsigned & written); 

      void set_volume (AudioOutputPrimarySecondary primarySecondary, unsigned volume);

      void play_buffer(AudioOutputPrimarySecondary primarySecondary, char* buffer, unsigned long len, unsigned channels, unsigned sample_rate, unsigned bps);

      void start_average_collection () { calculate_average = true; }

      void stop_average_collection () { calculate_average = false; }

      float get_average_level () { return average_level; }
      
      void add_device (const std::string & sink, const std::string & device, HalManager* manager);
      void remove_device (const std::string & sink, const std::string & device, HalManager* manager);
		  
      /*** VidInput Related Signals ***/
      
      /** See audiooutput-manager.h for the API
       */
      sigc::signal<void, AudioOutputManager &, AudioOutputPrimarySecondary, AudioOutputDevice&, AudioOutputConfig&> device_opened;
      sigc::signal<void, AudioOutputManager &, AudioOutputPrimarySecondary, AudioOutputDevice&> device_closed;
      sigc::signal<void, AudioOutputManager &, AudioOutputPrimarySecondary, AudioOutputDevice&, AudioOutputErrorCodes> device_error;
      sigc::signal<void, AudioOutputDevice> device_added;
      sigc::signal<void, AudioOutputDevice> device_removed;

  private:
      void on_device_opened (AudioOutputPrimarySecondary primarySecondary, 
                             AudioOutputDevice device,
                             AudioOutputConfig config, 
                             AudioOutputManager *manager);
      void on_device_closed (AudioOutputPrimarySecondary primarySecondary, AudioOutputDevice device, AudioOutputManager *manager);
      void on_device_error  (AudioOutputPrimarySecondary primarySecondary, AudioOutputDevice device, AudioOutputErrorCodes error_code, AudioOutputManager *manager);

      void internal_set_prim_audiooutput_device(const AudioOutputDevice & audiooutput_device);
      void internal_set_device (AudioOutputPrimarySecondary primarySecondary, const AudioOutputDevice & audiooutput_device);
      bool internal_open (AudioOutputPrimarySecondary primarySecondary, unsigned channels, unsigned samplerate, unsigned bits_per_sample);
      void internal_close(AudioOutputPrimarySecondary primarySecondary);
      void internal_play(AudioOutputPrimarySecondary primarySecondary, char* buffer, unsigned long len, unsigned channels, unsigned sample_rate, unsigned bps);
      void internal_set_primary_fallback();
      void calculate_average_level (const short *buffer, unsigned size);

      std::set<AudioOutputManager *> managers;

      Ekiga::Runtime & runtime;

       AudioOutputDeviceConfig current_primary_config;
       unsigned new_primary_volume;
       AudioOutputManager* current_manager[2];
       AudioOutputDevice desired_primary_device;
       AudioOutputDevice current_device[2];

      PMutex var_mutex[2];      /* To protect variables that are read and written */
      PMutex vol_mutex;      /* To protect variables that are read and written */

      AudioOutputCoreConfBridge* audiooutput_core_conf_bridge;
      AudioEventScheduler audio_event_scheduler;
      float average_level;
      bool calculate_average;
    };
/**
 * @}
 */
};

#endif
