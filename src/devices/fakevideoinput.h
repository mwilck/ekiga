
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 *                         vfakeio.h  -  description
 *                         -------------------------
 *   begin                : Tue Jul 30 2003
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains a descendant of a Fake Input 
 *                          Device that display the GM logo when connected to
 *                          a remote party without using a camera.
 *
 */


#ifndef _VFAKEIO_H_
#define _VFAKEIO_H_

#define P_FORCE_STATIC_PLUGIN

#include "common.h"
 
class PVideoInputDevice_Picture : public PVideoInputDevice 
{
  PCLASSINFO(PVideoInputDevice_Picture, PVideoInputDevice);

  PMutex pixbuf_mutex;      /* To protect the pixbufs that are read and written
			    from various threads */

  GdkPixbuf *cached_pix;
  GdkPixbuf *orig_pix;
  
 public:
  
  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Creates the Fake Input Device.
   * PRE          :  /
   */
  PVideoInputDevice_Picture ();


  /* DESCRIPTION  :  The destructor
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~PVideoInputDevice_Picture ();

  
  BOOL Open (const PString &name,
	     BOOL start_immediate = TRUE);

  
  /**Determine of the device is currently open.
   */
  BOOL IsOpen() ;

  
  /**Close the device.
   */
  BOOL Close();

  
  /**Start the video device I/O.
   */
  BOOL Start();

  
  /**Stop the video device I/O capture.
   */
  BOOL Stop();


  /**Determine if the video device I/O capture is in progress.
   */
  BOOL IsCapturing();

  
  /**Get a list of all of the drivers available.
   */
  static PStringList GetInputDeviceNames();

  
  BOOL SetFrameSize (unsigned int width,
		     unsigned int height);
  
  
  /* DESCRIPTION  :  The destructor
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  BOOL GetFrameData (BYTE *a, PINDEX *i = NULL);


  /* DESCRIPTION  :  The destructor
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  BOOL GetFrameDataNoDelay (BYTE *frame, PINDEX *i = NULL);

  
  BOOL TestAllFormats ();

  
  /**Get the maximum frame size in bytes.
  */
  PINDEX GetMaxFrameBytes();

  
  /** Given a preset interval of n milliseconds, this function
      returns n msecs after the previous frame capture was initiated.
  */
  void WaitFinishPreviousFrame();

  
  /**Set the video format to be used.

  Default behaviour sets the value of the videoFormat variable and then
  returns the IsOpen() status.
  */
  BOOL SetVideoFormat (VideoFormat newFormat);

  
  /**Get the number of video channels available on the device.

  Default behaviour returns 1.
  */
  int GetNumChannels() ;

  
  /**Set the video channel to be used on the device.

  Default behaviour sets the value of the channelNumber variable and then
  returns the IsOpen() status.
  */
  BOOL SetChannel (int newChannel);
			

  /**Set the colour format to be used.

  Default behaviour sets the value of the colourFormat variable and then
  returns the IsOpen() status.
  */
  BOOL SetColourFormat (const PString &newFormat);

  
  /**Set the video frame rate to be used on the device.

  Default behaviour sets the value of the frameRate variable and then
  return the IsOpen() status.
  */
  BOOL SetFrameRate (unsigned rate);

  
  BOOL GetFrameSizeLimits (unsigned &minWidth,
			   unsigned &minHeight,
			   unsigned &maxWidth,
			   unsigned &maxHeight);
  
  BOOL GetParameters (int *whiteness,
		      int *brightness,
		      int *colour,
		      int *contrast,
		      int *hue);
  
  PBYTEArray data;
  bool moving;
  int rgb_increment;
  int pos;
  int increment;
  PStringList GetDeviceNames() const
  { return GetInputDeviceNames(); }
};

PCREATE_VIDINPUT_PLUGIN (Picture);

#endif
