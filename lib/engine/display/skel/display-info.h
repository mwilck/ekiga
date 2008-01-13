
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras

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
 *                         display-info.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Matthias Schneider 
 *   copyright            : (c) 2007 by Matthias Schneider
 *   description          : declaration of structs and classes used for communication
 *                          with the DisplayManagers
 *
 */

#ifndef __DISPLAY_INFO_H__
#define __DISPLAY_INFO_H__

#include "X11/Xlib.h"
#ifdef None
#undef None
#endif

//namespace Ekiga {
  /* Video modes */
  typedef enum {
  
    LOCAL_VIDEO_new, 
    REMOTE_VIDEO_new, 
    PIP_new,
    PIP_WINDOW_new,
    FULLSCREEN_new,
    UNSET_new
  } DisplayMode;
  
  /* Toggle operations for Fullscreen */
  typedef enum {
  
    ON_new,
    OFF_new,
    TOGGLE_new
  } FSToggle_new;
  
  /* Video Acceleration Status */
  typedef enum {
  
    NONE_new,
    REMOTE_ONLY_new,
    ALL_new,
    NO_VIDEO_new
  } VideoAccelStatus_new;  //FIXME


  typedef struct {
    unsigned rx_fps;
    unsigned rx_width;
    unsigned rx_height;
    unsigned tx_fps;
    unsigned tx_width;
    unsigned tx_height;
    VideoAccelStatus_new video_accel_status;
  } DisplayStats;

  class DisplayInfo
  {
  public:
    DisplayInfo() {
      widgetInfoSet = false;
      x = 0;
      y = 0;
  #ifdef WIN32
      hwnd = 0;
  #else
      gc = 0;
      window = 0;
      xdisplay = NULL;
  #endif
  
      gconfInfoSet = false;
      onTop = false;
      disableHwAccel = false;
      allowPipSwScaling = true;
      swScalingAlgorithm = 0;
  
      display = UNSET_new;
      zoom = 0;
    };
    
    void operator= ( const DisplayInfo& rhs) {
  
    if (rhs.widgetInfoSet) {
        widgetInfoSet = rhs.widgetInfoSet;
        x = rhs.x;
        y = rhs.y;
  #ifdef WIN32
        hwnd = rhs.hwnd;
  #else
        gc = rhs.gc;
        window = rhs.window;
        xdisplay = rhs.xdisplay;
  #endif
      }
  
      if (rhs.gconfInfoSet) {
        gconfInfoSet = rhs.gconfInfoSet;
        onTop = rhs.onTop;
        disableHwAccel = rhs.disableHwAccel;
        allowPipSwScaling = rhs.allowPipSwScaling;
        swScalingAlgorithm =  rhs.swScalingAlgorithm;
      }
      if (rhs.display != UNSET_new) display = rhs.display;
      if (rhs.zoom != 0) zoom = rhs.zoom;
    };
  
    bool widgetInfoSet;
    int x;
    int y;
              
  #ifdef WIN32
    HWND hwnd;
  #else
    GC gc;
    Window window;
    Display* xdisplay;
  #endif
  
    bool gconfInfoSet;
    bool onTop;
    bool disableHwAccel;
    bool allowPipSwScaling;
    unsigned int swScalingAlgorithm;
  
    DisplayMode display;
    unsigned int zoom;
  };
  
//};

#endif
