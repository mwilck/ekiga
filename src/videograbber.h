
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2001 Damien Sandras
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 *                         videograbber.h  -  description
 *                         ------------------------------
 *   begin                : Mon Feb 12 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : Video4Linux compliant functions to manipulate the 
 *                          webcam device.
 *   email                : dsandras@seconix.com
 *
 */


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
#include <stdlib.h>
#include <pthread.h>
#include <ptlib.h>

#include <gconf/gconf-client.h>

#include "gdkvideoio.h"
#include "config.h"
#include "common.h"

#define GM_VIDEO_GRABBER(x) (GMVideoGrabber *)(x)


class GMVideoGrabber : public PThread
{
  PCLASSINFO(GMVideoGrabber, PThread);

 public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Initialise the parameters.
   * PRE          :  /
   */
  GMVideoGrabber ();


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  Initialise the parameters.
   * PRE          :  /
   */
  ~GMVideoGrabber (void);
  

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  The main thread, executes operations like 
   *                 Open/Close/Grab from the video device
   *                 video channel specified in the options structure.
   * PRE          :  /
   */
  void Main (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Puts the is_grabbing flag to 1 so that the main thread 
   *                 starts to grab, i.e. read from the specified device 
   *                 and display images in the main interface.
   * PRE          :  /
   */
  void Start (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Puts the is_grabbing flag to 0 so that the main thread 
   *                 stops to grab, i.e. read from the specified device 
   *                 and display images in the main interface.
   * PRE          :  /
   */
  void Stop (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Puts the has_to_open flag to 1 so that the main thread 
   *                 opens the specified video device. If the argument is 1, 
   *                 starts to grab images just after the device opening.
   *                 Internally reads the gconf config before opening.
   * PRE          :  int = 0 or int = 1, the video grabber device 
   *                 should be closed.
   */
  void Open (int = 0, int = 0);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Puts the has_to_close flag to 1 so that the main 
   *                 thread closes the specified video device.
   * PRE          :  The video grabber must be opened, the argument = async 
   *                 or not.
   */
  void Close (int = 0);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Puts the has_to_reset flag to 1 so that the main 
   *                 thread closes and reopens the specified,
   *                 if the preview button is active, then preview will be 
   *                 active after the reset.
   * PRE          :  /
   */
  void Reset (void);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns 1 if the device is opened, 0 otherwise.
   * PRE          :  /
   */
  int IsOpened (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the GDKVideoOutputDevice that will be used 
   *                 to display the camera images.
   * PRE          :  /
   */
  GDKVideoOutputDevice *GetEncodingDevice (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the PVideoChannel associated with the device.
   * PRE          :  /
   */
  PVideoChannel *GetVideoChannel (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the grabber frame rate, if there is a grabber.
   * PRE          :  /
   */
  void SetFrameRate (int);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Sets the colour for the specified device.
   * PRE          :  0 <= int <= 65535
   */
  void SetColour (int);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Sets the brightness for the specified device.
   * PRE          :  0 <= int <= 65535
   */
  void SetBrightness (int);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Sets the whiteness for the specified device.
   * PRE          :  0 <= int <= 65535
   */
  void SetWhiteness (int);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Sets the contrast for the specified device.
   * PRE          :  0 <= int <= 65535
   */
  void SetContrast (int);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Sets the new frame size.
   * PRE          :  size = CIF or QCIF
   */
  void SetFrameSize (int height, int width);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Sets the new channel.
   * PRE          :  /
   */
  void SetChannel (int);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns respectively the whiteness, brightness, 
   *                 colour, contrast for the specified device.
   * PRE          :  Allocated pointers to int. Grabber must be opened.
   */
  void GetParameters (int *, int *, int *, int *);


 protected:
  void VGOpen (void);  /* That function really opens the video device */
  void VGClose (int = 1); /* That function really closes the video device, 
			     display the logo at the end, except if parameter
			     = 0. */
  void VGStop ();
  void UpdateConfig (void); /* That function updates the internal values */

  GmWindow *gw;
  GmPrefWindow *pw;

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
  int has_to_stop;

  gchar *video_device;
  gchar *color_format;
  int video_channel;
  int video_size;
  PVideoDevice::VideoFormat video_format;

  PMutex quit_mutex;     /* Mutex to quit safely, after the Main method 
			    has ended */
  PMutex grabbing_mutex; /* Mutex to quit safely, after the last image was 
			    grabbed from the device */
  PMutex device_mutex;   /* Here to prevent the device to be accessed 2 times
			    by different threads */
  PMutex var_mutex;      /* To protect variables that are set 
			    from various threads */
  GConfClient *client;   /* The gconf client */
};


class GMVideoTester : public PThread
{
  PCLASSINFO(GMVideoTester, PThread);


public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  
   * PRE          :  /
   */
  GMVideoTester ();


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMVideoTester ();


  void Main ();


protected:

  PMutex quit_mutex;
};
#endif
