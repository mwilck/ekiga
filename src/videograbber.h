/***************************************************************************
                          videograbber.h  -  description
                             -------------------
    begin                : Mon Feb 12 2001
    copyright            : (C) 2001 by Damien Sandras
    description          : Video4Linux compliant functions to manipulate the 
                           webcam device
    email                : dsandras@acm.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _VIDEO_GRABBER_H_
#define _VIDEO_GRABBER_H_

#include <fcntl.h>
#include <unistd.h>

#ifdef __linux__
#include <linux/videodev.h>
#endif
#ifdef __FreeBSD__
#include <machine/ioctl_meteor.h>
#endif

#include <sys/ioctl.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkrgb.h>
#include <stdlib.h>
#include <pthread.h>
#include <gnome.h>
#include <ptlib.h>

#include "gdkvideoio.h"
#include "config.h"
#include "common.h"


/**
 * VideoGrabber Class : Cope with the video4linux devices.
 */
class GMVideoGrabber : public PThread
{
  PCLASSINFO(GMVideoGrabber, PThread);

 public:

  GMVideoGrabber (GM_window_widgets *, options *);
  ~GMVideoGrabber (void);
  

  /**
   * The main thread, executes operations like Open/Close/Grab from the video device
   * video channel specified in the options structure.
   */
  void Main (void);


  /**
   * Puts the is_grabbing flag to 1 so that the main thread starts to grab, i.e. read from
   * the specified device and display images in the main interface.
   */
  void Start (void);


  /**
   * Puts the is_grabbing flag to 0 so that the main thread stops to grab, i.e. read from
   * the specified device and display images in the main interface.
   */
  void Stop (void);


  /**
   * Puts the has_to_open flag to 1 so that the main thread opens the specified video
   * device. If the argument is 1, starts to grab images just after the device opening.
   * #PRE: int = 0 or int = 1
   */
  void Open (int = 0, int = 0);

  
  /**
   * Puts the has_to_close flag to 1 so that the main thread closes the specified video
   * device.
   */
  void Close (void);


  /**
   * Puts the has_to_reset flag to 1 so that the main thread closes and reopens the specified,
   * if the preview button is active, then preview will be active after the reset.
   */
  void Reset (void);

  
  /**
   * Returns 1 if the device is opened, 0 otherwise.
   */
  int IsOpened (void);


  /**
   * Returns the GDKVideoOutputDevice that will be used to display the camera images.
   */
  GDKVideoOutputDevice *GetEncodingDevice (void);


  /**
   * Returns the PVideoChannel associated with the device.
   */
  PVideoChannel *GetVideoChannel (void);

  
  /**
   * Sets the colour for the specified device.
   * #PRE: 0 <= int <= 65535
   */
  void SetColour (int);


  /**
   * Sets the brightness for the specified device.
   * #PRE: 0 <= int <= 65535
   */
  void SetBrightness (int);


  /**
   * Sets the whiteness for the specified device.
   * #PRE: 0 <= int <= 65535
   */
  void SetWhiteness (int);


  /**
   * Sets the contrast for the specified device.
   * #PRE: 0 <= int <= 65535
   */
  void SetContrast (int);


  /**
   * Returns respectively the whiteness, brightness, colour, contrast for the specified device.
   * #PRE: Allocated pointers to int.
   */
  void GetParameters (int *, int *, int *, int *);

 protected:
  void VGOpen (void);  // That functions really opens the video device.
  void VGClose (void); // That functions really closes the video device.

  GM_window_widgets *gw;
  options *opts;

  int height, width;
  int whiteness, brightness, colour, contrast;

  char video_buffer [3 * GM_CIF_WIDTH * GM_CIF_HEIGHT];

  PVideoChannel *channel;
  PVideoInputDevice *grabber;
  GDKVideoOutputDevice *encoding_device;

  int is_running;
  int is_grabbing;
  int is_opened;
  int has_to_open;
  int has_to_close;
  int has_to_reset;

  PMutex quit_mutex;     // Mutex to quit safely, after the Main method has ended.
  PMutex grabbing_mutex; // Mutex to quit safely, after the last image was grabbed from
                         // the device.
};

/******************************************************************************/
#endif
