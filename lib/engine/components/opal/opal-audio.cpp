
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         audiodev.cpp  -  description
 *                         --------------------------------
 *   begin                : Thu Mar 06 2008
 *   copyright            : (C) 2008 by Matthias Schneider
 *   description          : Audio dummy device to allow opal make use of 
 *                          the Ekiga Engine 
 *
 */

#pragma implementation "opal-audio.h"

#include "opal-audio.h"

PSoundChannel_EKIGA::PSoundChannel_EKIGA (boost::shared_ptr<Ekiga::AudioInputCore> _audioinput_core,
					  boost::shared_ptr<Ekiga::AudioOutputCore> _audiooutput_core):
  audioinput_core (_audioinput_core),
  audiooutput_core (_audiooutput_core)
{
  opened = false;
}


PSoundChannel_EKIGA::PSoundChannel_EKIGA (const PString & /*_device*/,
					  Directions dir,
					  unsigned numChannels,
					  unsigned sampleRate,
					  unsigned bitsPerSample,
					  boost::shared_ptr<Ekiga::AudioInputCore> _audioinput_core,
					  boost::shared_ptr<Ekiga::AudioOutputCore> _audiooutput_core):
  audioinput_core (_audioinput_core),
  audiooutput_core (_audiooutput_core)
{
  opened = false;
  Open (device, dir, numChannels, sampleRate, bitsPerSample);
}


PSoundChannel_EKIGA::~PSoundChannel_EKIGA()
{
  Close();
}


PString PSoundChannel_EKIGA::GetDefaultDevice(Directions dir)
{
  PStringArray devicenames;
  devicenames = PSoundChannel_EKIGA::GetDeviceNames (dir);

  return devicenames[0];
}


bool PSoundChannel_EKIGA::Open (const PString & /*_device*/,
                              Directions _dir,
                              unsigned _numChannels,
                              unsigned _sampleRate,
                              unsigned _bitsPerSample)
{
  direction = _dir;

  if (_dir == Recorder) {
    audioinput_core->start_stream(_numChannels, _sampleRate, _bitsPerSample);
  }
  else {
    audiooutput_core->start (_numChannels, _sampleRate, _bitsPerSample);
  }

  mNumChannels   = _numChannels;
  mSampleRate    = _sampleRate;
  mBitsPerSample = _bitsPerSample;

  opened = true;
  return true;
}


bool PSoundChannel_EKIGA::Close()
{
  if (opened == false)
    return true;

  if (direction == Recorder) {
    audioinput_core->stop_stream();
  }
  else {
    audiooutput_core->stop();
  }
  opened = false;
  return true;
}


bool PSoundChannel_EKIGA::Write (const void *buf, PINDEX len)
{
  unsigned bytesWritten = 0;

  if (direction == Player) {
    audiooutput_core->set_frame_data((char*)buf, len, bytesWritten);
  }

  lastWriteCount = bytesWritten;
  return true;
}


bool PSoundChannel_EKIGA::Read (void * buf, PINDEX len)
{
  unsigned bytesRead = 0;

  if (direction == Recorder) {
    audioinput_core->get_frame_data((char*)buf, len, bytesRead);
  }

  lastReadCount = bytesRead;
  return true;
}


unsigned PSoundChannel_EKIGA::GetChannels()   const
{
  return mNumChannels;
}


unsigned PSoundChannel_EKIGA::GetSampleRate() const
{
  return mSampleRate;
}


unsigned PSoundChannel_EKIGA::GetSampleSize() const
{
  return mBitsPerSample;
}


bool PSoundChannel_EKIGA::SetBuffers (PINDEX size, PINDEX count)
{
  if (direction == Recorder) {
    audioinput_core->set_stream_buffer_size(size, count);
  }
  else {
    audiooutput_core->set_buffer_size(size, count);
  }

  storedPeriods = count;
  storedSize = size;

  isInitialised = false;

  return true;
}


bool PSoundChannel_EKIGA::GetBuffers(PINDEX & size, PINDEX & count)
{
  size = storedSize;
  count = storedPeriods;
  
  return false;
}

bool PSoundChannel_EKIGA::IsOpen () const
{
  return opened;
}

