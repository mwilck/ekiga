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

#include "common.h"

/******************************************************************************/
/* The functions                                                              */
/******************************************************************************/

// DESCRIPTION  :  /
// BEHAVIOR     :  Open the webcam device (if not already opened) and return 1
//                 if success, 0 otherwise
// PRE          :  The parameter are : a pointer to a device, and a video channel
int GM_cam (gchar *, int);


// DESCRIPTION  :  /
// BEHAVIOR     :  BGR -> RGB conversion
// PRE          :  A pointer to a BGR buffer of 176x144
void GM_rgb_swap (void *);


// DESCRIPTION  :  /
// BEHAVIOR     :  Starts to capture in a separate thread from the webcam device
//                 and displays in the main window
// PRE          :  The first parameter is the device, the second is a pointer
//                 to a valid GM_window_widgets
int GM_cam_capture_start (gchar *, GM_window_widgets *);


// DESCRIPTION  :  /
// BEHAVIOR     :  Stops the capture thread
// PRE          :  The parameter is a pointer to a valid GM_window_widgets
int GM_cam_capture_stop (GM_window_widgets *);


// DESCRIPTION  :  /
// BEHAVIOR     :  Starts the capture thread (capture till gw->dev != -1), if
//                 device not opened, opens it. If no device, do nothing
// PRE          :  The parameter is a pointer to a valid GM_window_widgets
void * GM_cam_capture_thread (GM_window_widgets *);


// DESCRIPTION  :  /
// BEHAVIOR     :  Displays webcam info in a text widget (second parameter)
// PRE          :  The first parameter is a pointer to a valid GM_window_widgets
//                 and the second one, a valid pointer to a valid text widget
int GM_cam_info (GM_window_widgets *, GtkWidget *);


// DESCRIPTION  :  /
// BEHAVIOR     :  Sets the colour a values for the webcam driver
// PRE          :  The first parameter is a pointer to GM_window_widgets
void GM_cam_set_colour (GM_window_widgets *, int);


// DESCRIPTION  :  /
// BEHAVIOR     :  Sets the contrast a values for the webcam driver
// PRE          :  The first parameter is a pointer to GM_window_widgets
void GM_cam_set_contrast (GM_window_widgets *, int);


// DESCRIPTION  :  /
// BEHAVIOR     :  Sets the brightness a values for the webcam driver
// PRE          :  The first parameter is a pointer to GM_window_widgets
void GM_cam_set_brightness (GM_window_widgets *, int);


// DESCRIPTION  :  /
// BEHAVIOR     :  Sets the whiteness a values for the webcam driver
// PRE          :  The first parameter is a pointer to GM_window_widgets
void GM_cam_set_whiteness (GM_window_widgets *, int);


// DESCRIPTION  :  /
// BEHAVIOR     :  Gets the params values for the webcam driver
// PRE          :  The first parameter is a pointer to valid options,
//                 then whiteness, brightness, colour, contrast (between 0 and 1)
void GM_cam_get_params (options *, int *, int *, int *, int *);

/******************************************************************************/

#endif
