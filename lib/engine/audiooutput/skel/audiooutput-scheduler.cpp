
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
 *                         audiooutput-scheduler.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of a scheduler for sound events that is run in 
 *                          a separate thread.
 *
 */

#include "audiooutput-core.h"
#include "audiooutput-scheduler.h"
#include "config.h"

using namespace Ekiga;

AudioEventScheduler::AudioEventScheduler (AudioOutputCore& _audio_output_core)
: PThread (1000, NoAutoDeleteThread, HighestPriority, "AudioEventScheduler"),
    audio_output_core (_audio_output_core)
{
  stop_thread = false;
  // Since windows does not like to restart a thread that 
  // was never started, we do so here
  this->Resume ();
  thread_sync_point.Wait ();
}

AudioEventScheduler::~AudioEventScheduler ()
{
  stop_thread = true;
  new_event.Signal ();

  /* Wait for the Main () method to be terminated */
  PWaitAndSignal m(quit_mutex);

}

void AudioEventScheduler::Main ()
{
  PWaitAndSignal m(quit_mutex);

  std::vector <AudioEvent> pending_event_list;
  unsigned idle_time = 32000;
  AudioEvent event;
  char* buffer = NULL;
  unsigned long buffer_len = 0;
  unsigned channels, sample_rate, bps;
  AudioOutputPrimarySecondary primarySecondary;

  thread_sync_point.Signal ();

  while (!stop_thread) {

    new_event.Wait (idle_time);

    if (stop_thread)
      break;
      
    get_pending_event_list(pending_event_list);
    PTRACE(0, "Checking pending list with " << pending_event_list.size() << " elements");

    while (pending_event_list.size() > 0) {
      PTRACE(0, "Processing pending event list of size " << pending_event_list.size());
      event = *(pending_event_list.begin()); pending_event_list.erase(pending_event_list.begin());
      load_wav(event.name, event.is_file_name, buffer, buffer_len, channels, sample_rate, bps, primarySecondary);
      if (buffer) {
         audio_output_core.play_buffer (primarySecondary, buffer, buffer_len, channels, sample_rate, bps);
        free (buffer);
        buffer = NULL;
      }
      Current()->Sleep (10);
    }
    idle_time = get_time_to_next_event();
    PTRACE(0, "Idling for " << idle_time);
  }
}

void AudioEventScheduler::get_pending_event_list (std::vector<AudioEvent> & pending_event_list)
{
  PWaitAndSignal m(event_list_mutex);

  AudioEvent event;
  std::vector <AudioEvent> new_event_list;
  unsigned long time = get_time_ms();

  pending_event_list.clear();

  while (event_list.size() > 0) {

    event = *(event_list.begin()); event_list.erase(event_list.begin());

    if (event.interval == 0) {
      pending_event_list.push_back(event);
    }
    else {
      PTRACE(0, "Checking recurring event with scheduled time " << event.time << ">=" << time);
      if (event.time <= time) {
        pending_event_list.push_back(event);
        event.repetitions--; 
        if (event.repetitions > 0) {
          event.time = time + event.interval;
          new_event_list.push_back(event);
        }
      }
      else {
        new_event_list.push_back(event);
      }
    }
  }

  event_list = new_event_list;
  PTRACE(0, "Event list length: " << event_list.size() << ", after removing pending events:  " << new_event_list.size());
}

unsigned long AudioEventScheduler::get_time_ms()
{
  GTimeVal time_val;
  g_get_current_time(&time_val);
  return (time_val.tv_sec * 1000 + (unsigned long) (time_val.tv_usec / 1000) );
}

unsigned AudioEventScheduler::get_time_to_next_event()
{
  PWaitAndSignal m(event_list_mutex);
  unsigned long time = get_time_ms();
  unsigned min_time = 32000;

  for (std::vector<AudioEvent>::iterator iter = event_list.begin ();
       iter !=event_list.end ();
       iter++) {

    if ( (iter->interval > 0) && ((iter->time - time) < min_time) )
      min_time = iter->time - time;
  }
  return min_time;
}

void AudioEventScheduler::add_event_to_queue(const std::string & name, bool is_file_name, unsigned interval, unsigned repetitions)
{
  PTRACE(0, "Adding Event " << name << " " << interval << "/" << repetitions);
  PWaitAndSignal m(event_list_mutex);
  AudioEvent event;
  event.name = name;
  event.is_file_name = is_file_name;
  event.interval = interval;
  event.repetitions = repetitions;
  event.time = get_time_ms();
  event_list.push_back(event);
  new_event.Signal();
}

void AudioEventScheduler::remove_event_from_queue(const std::string & name)
{
  PTRACE(0, "Removing Event " << name);
  PWaitAndSignal m(event_list_mutex);

  bool found = false;
  std::vector<AudioEvent>::iterator iter;

  for (iter = event_list.begin ();
       iter != event_list.end () ;
       iter++){
      if ( (iter->name == name) ) { 
        found = true;
        break;
      }
}
  if (found) {
    event_list.erase(iter);
  }
}

void AudioEventScheduler::load_wav(const std::string & event_name, bool is_file_name, char* & buffer, unsigned long & len, unsigned & channels, unsigned & sample_rate, unsigned & bps, AudioOutputPrimarySecondary & primarySecondary)
{
  PWAVFile* wav = NULL;
  std::string file_name;

  len = 0;
  buffer = NULL;

  // Shall we also try event name as file name?
  if (is_file_name) {
    file_name = event_name;
    primarySecondary = primary;
  }
  else 
    if (!get_file_name(event_name, file_name, primarySecondary)) // if this event is disabled
      return;
  PTRACE(0, "Trying filename " << file_name << " for event " << event_name);
  wav = new PWAVFile (file_name.c_str(), PFile::ReadOnly);

  if (!wav->IsValid ()) {
     /* it isn't a full path to a file : add our default path */
 
    delete wav;
    wav = NULL;
 
    gchar* filename = g_build_filename (DATA_DIR, "sounds", PACKAGE_NAME, file_name.c_str(), NULL);
    PTRACE(0, "Typeing filename " << filename << " for event " << event_name);

    wav = new PWAVFile (filename, PFile::ReadOnly);
    g_free (filename);
  }
  
  PTRACE(0, "Loaded wav " << file_name);
  if (wav->IsValid ()) {
    len = wav->GetDataLength();
    channels = wav->GetChannels ();
    sample_rate = wav->GetSampleRate ();
    bps = wav->GetSampleSize ();

    buffer = (char*) malloc (len);
    memset(buffer, 127, len);
    wav->Read(buffer, len);
  }

  delete wav;
}


bool AudioEventScheduler::get_file_name(const std::string & event_name, std::string & file_name, AudioOutputPrimarySecondary & primarySecondary)
{
  PWaitAndSignal m(event_file_list_mutex);

  file_name = "";

  for (std::vector<EventFileName>::iterator iter = event_file_list.begin ();
       iter != event_file_list.end ();
       iter++) {

    if (iter->event_name == event_name) {
      file_name = iter->file_name;
      primarySecondary = iter->primarySecondary;
      return (iter->enabled);
    }
  }

  return false;
}

void AudioEventScheduler::set_file_name(const std::string & event_name, const std::string & file_name, bool enabled, AudioOutputPrimarySecondary primarySecondary)
{
  PWaitAndSignal m(event_file_list_mutex);

  bool found = false;

  for (std::vector<EventFileName>::iterator iter = event_file_list.begin ();
       iter != event_file_list.end ();
       iter++) {

    if (iter->event_name == event_name) {
      iter->file_name = file_name;
      iter->enabled = enabled;
      iter->primarySecondary = primarySecondary;
      found = true;
      break;
    }
  }

  if (!found) {
    EventFileName event_file_name;
    event_file_name.event_name = event_name;
    event_file_name.file_name = file_name;
    event_file_name.enabled = enabled;
    event_file_name.primarySecondary = secondary;
    event_file_list.push_back(event_file_name);
  }
}
