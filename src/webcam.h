/***************************************************************************
                          webcam.cxx  -  description
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

#ifndef _WEBCAM_H_
#define _WEBCAM_H_

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

int GM_cam (gchar *, int);

class GMVideoGrabber : public PThread
{
  PCLASSINFO(GMVideoGrabber, PThread);

 public:
  GMVideoGrabber (GM_window_widgets *, options *);
  ~GMVideoGrabber (void);
  
  void Main (void);
  void Start (void);
  void Open (int = 0);
  void Close (void);
  void Reset (void);
  void Stop (void);
  int IsOpened (void);
  GDKVideoOutputDevice *GetEncodingDevice (void);
  PVideoChannel *GetVideoChannel (void);
  void SetColour (int);
  void SetBrightness (int);
  void SetWhiteness (int);
  void SetContrast (int);
  void GetParameters (int *, int *, int *, int *);

 protected:
  void VGOpen (void);
  void VGClose (void);

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

  PMutex quit_mutex;
  PMutex grabbing_mutex;
};

/******************************************************************************/
#endif
