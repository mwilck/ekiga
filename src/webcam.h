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


class GMH323Webcam : public PThread
{
  PCLASSINFO(GMH323Webcam, PThread);

 public:
  GMH323Webcam (GM_window_widgets *, options *);
  ~GMH323Webcam ();
  
  void Main ();
  void Start ();
  void Stop ();
  int Running ();
  PVideoChannel *Channel ();
  GDKVideoOutputDevice *Device (void);
  void Restart (void);
  void SetColour (int);
  void SetBrightness (int);
  void SetWhiteness (int);
  void SetContrast (int);
  void GetParameters (int *, int *, int *, int *);
  void Terminate (void);
  void Initialise (void);
  void ReInitialise (options *);

 protected:
  GM_window_widgets *gw;
  options *opts;
  char video_buffer [230400];
  PVideoChannel *channel;
  PVideoInputDevice *grabber;
  GDKVideoOutputDevice *encoding_device;
  int running;
  int grabbing;
  int reinit;
  int logo;
};

/******************************************************************************/
#endif
