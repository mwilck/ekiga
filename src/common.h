
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
 *                         common.h  -  description
 *                         ------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains things common to the whole soft.
 *
 */


#ifndef GM_COMMON_H_
#define GM_COMMON_H_

#include <opal/buildopts.h>
#include <ptbuildopts.h>

#include <ptlib.h>

#include <opal/manager.h>
#include <opal/pcss.h>

#include <h323/h323.h>
#include <sip/sip.h>

#include <glib.h>
#include <gtk/gtk.h>

#ifdef WIN32
#include <gdk/gdkwin32.h>
#else
#include <gdk/gdkx.h>
#ifdef BadRequest
#undef BadRequest
#endif /*BadRequest */
#endif /* WIN32 */

#define GENERAL_KEY         "/apps/" PACKAGE_NAME "/general/"
#define USER_INTERFACE_KEY "/apps/" PACKAGE_NAME "/general/user_interface/"
#define VIDEO_DISPLAY_KEY USER_INTERFACE_KEY "video_display/"
#define SOUND_EVENTS_KEY  "/apps/" PACKAGE_NAME "/general/sound_events/"
#define AUDIO_DEVICES_KEY "/apps/" PACKAGE_NAME "/devices/audio/"
#define VIDEO_DEVICES_KEY "/apps/" PACKAGE_NAME "/devices/video/"
#define PERSONAL_DATA_KEY "/apps/" PACKAGE_NAME "/general/personal_data/"
#define CALL_OPTIONS_KEY "/apps/" PACKAGE_NAME "/general/call_options/"
#define NAT_KEY "/apps/" PACKAGE_NAME "/general/nat/"
#define PROTOCOLS_KEY "/apps/" PACKAGE_NAME "/protocols/"
#define H323_KEY "/apps/" PACKAGE_NAME "/protocols/h323/"
#define SIP_KEY "/apps/" PACKAGE_NAME "/protocols/sip/"
#define PORTS_KEY "/apps/" PACKAGE_NAME "/protocols/ports/"
#define CALL_FORWARDING_KEY "/apps/" PACKAGE_NAME "/protocols/call_forwarding/"
#define LDAP_KEY "/apps/" PACKAGE_NAME "/protocols/ldap/"
#define CODECS_KEY "/apps/" PACKAGE_NAME "/codecs/"
#define AUDIO_CODECS_KEY "/apps/" PACKAGE_NAME "/codecs/audio/"
#define VIDEO_CODECS_KEY  "/apps/" PACKAGE_NAME "/codecs/video/"

#define GM_4CIF_WIDTH  704
#define GM_4CIF_HEIGHT 576
#define GM_CIF_WIDTH   352
#define GM_CIF_HEIGHT  288
#define GM_QCIF_WIDTH  176
#define GM_QCIF_HEIGHT 144
#define GM_4SIF_WIDTH  640
#define GM_4SIF_HEIGHT 480
#define GM_SIF_WIDTH   320
#define GM_SIF_HEIGHT  240
#define GM_QSIF_WIDTH  160
#define GM_QSIF_HEIGHT 120
#define GM_FRAME_SIZE  10

#define GNOMEMEETING_PAD_SMALL 1

/* Contact state */
typedef enum {

  CONTACT_ONLINE,
  CONTACT_AWAY,
  CONTACT_DND,
  CONTACT_FREEFORCHAT,
  CONTACT_INVISIBLE,
  CONTACT_OFFLINE,
  CONTACT_UNKNOWN,
  CONTACT_LAST_STATE
} ContactState;

/* Incoming Call Mode */
typedef enum {

  AVAILABLE,
  AUTO_ANSWER,
  DO_NOT_DISTURB,
  FORWARD,
  NUM_MODES
} IncomingCallMode;


/* Control Panel Section */
typedef enum {

  CONTACTS,
  DIALPAD,
  CALL,
  NUM_SECTIONS
} PanelSection;


/* Video modes */
typedef enum {

  LOCAL_VIDEO, 
  REMOTE_VIDEO, 
  PIP,
  PIP_WINDOW,
  FULLSCREEN,
  UNSET
} VideoMode;

/* Toggle operations for Fullscreen */
typedef enum {

  ON,
  OFF,
  TOGGLE
} FSToggle;

/* Video Acceleration Status */
typedef enum {

  NONE,
  REMOTE_ONLY,
  ALL,
  NO_VIDEO
} VideoAccelStatus;

#define NB_VIDEO_SIZES 5

const static struct { 
  int width; 
  int height; 
  } 
  video_sizes[NB_VIDEO_SIZES] = {
    {  GM_QCIF_WIDTH,  GM_QCIF_HEIGHT },
    {  GM_CIF_WIDTH,   GM_CIF_HEIGHT  },
    {  GM_4CIF_WIDTH,  GM_4CIF_HEIGHT },
    {  GM_SIF_WIDTH,   GM_SIF_HEIGHT  },
    {  GM_4SIF_WIDTH,  GM_4SIF_HEIGHT },
};

class VideoInfo
{
public:
  VideoInfo() {
    widgetInfoSet = FALSE;
    x = 0;
    y = 0;
#ifdef WIN32
    hwnd = 0;
#else
    gc = 0;
    window = 0;
    xdisplay = NULL;
#endif

    gconfInfoSet = FALSE;
    onTop = FALSE;
    disableHwAccel = FALSE;
    allowPipSwScaling = TRUE;
    swScalingAlgorithm = 0;

    display = UNSET;
    zoom = 0;
  };
  
  void operator= ( const VideoInfo& rhs) {

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
    if (rhs.display != UNSET) display = rhs.display;
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

  VideoMode display;
  unsigned int zoom;
  
};

//will be moved at a later stage
typedef struct {
  unsigned rxFPS;
  unsigned rxWidth;
  unsigned rxHeight;
  unsigned txFPS;
  unsigned txWidth;
  unsigned txHeight;
  VideoAccelStatus videoAccelStatus;
} VideoStats;

#endif /* GM_COMMON_H */
