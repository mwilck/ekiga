
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
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
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         common.h  -  description
 *                         ------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
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

#ifndef DISABLE_GNOME
#include <gnome.h>
#else
#include <gtk/gtk.h>
#include "about/gnome-about.h"
#include "druid/gnome-druid.h"
#include "druid/gnome-druid-page-edge.h"
#include "druid/gnome-druid-page-standard.h"
#endif

#ifdef WIN32
#include <string.h>
#define strcasecmp strcmpi
#define vsnprintf _vsnprintf
#endif


#define GENERAL_KEY         "/apps/gnomemeeting/general/"
#define USER_INTERFACE_KEY "/apps/gnomemeeting/general/user_interface/"
#define VIDEO_DISPLAY_KEY USER_INTERFACE_KEY "video_display/"
#define SOUND_EVENTS_KEY  "/apps/gnomemeeting/general/sound_events/"
#define AUDIO_DEVICES_KEY "/apps/gnomemeeting/devices/audio/"
#define VIDEO_DEVICES_KEY "/apps/gnomemeeting/devices/video/"
#define PERSONAL_DATA_KEY "/apps/gnomemeeting/general/personal_data/"
#define CALL_OPTIONS_KEY "/apps/gnomemeeting/general/call_options/"
#define NAT_KEY "/apps/gnomemeeting/general/nat/"
#define H323_ADVANCED_KEY "/apps/gnomemeeting/protocols/h323/advanced/"
#define H323_GATEKEEPER_KEY "/apps/gnomemeeting/protocols/h323/gatekeeper/"
#define H323_GATEWAY_KEY "/apps/gnomemeeting/protocols/h323/gateway/"
#define PORTS_KEY "/apps/gnomemeeting/protocols/h323/ports/"
#define CALL_FORWARDING_KEY "/apps/gnomemeeting/protocols/h323/call_forwarding/"
#define LDAP_KEY "/apps/gnomemeeting/protocols/ldap/"
#define AUDIO_CODECS_KEY "/apps/gnomemeeting/codecs/audio/"
#define VIDEO_CODECS_KEY  "/apps/gnomemeeting/codecs/video/"

#define GM_CIF_WIDTH   352
#define GM_CIF_HEIGHT  288
#define GM_QCIF_WIDTH  176
#define GM_QCIF_HEIGHT 144
#define GM_SIF_WIDTH   320
#define GM_SIF_HEIGHT  240
#define GM_QSIF_WIDTH  160
#define GM_QSIF_HEIGHT 120
#define GM_FRAME_SIZE  10

#define GM_MAIN_NOTEBOOK_HIDDEN 4

#define GNOMEMEETING_PAD_SMALL 1


#ifndef _
#ifdef DISABLE_GNOME
#include <libintl.h>
#define _(x) gettext(x)
#ifdef gettext_noop
#define N_(String) gettext_noop (String)
#else
#define N_(String) (String)
#endif
#endif
#endif



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

  STATISTICS,
  DIALPAD,
  AUDIO_SETTINGS,
  VIDEO_SETTINGS,
  CLOSED,
  NUM_SECTIONS
} ControlPanelSection;


/* Video modes */
enum {

  LOCAL_VIDEO, 
  REMOTE_VIDEO, 
  BOTH_INCRUSTED, 
  BOTH_SIDE,
  BOTH,
  FULLSCREEN
};


#endif /* GM_COMMON_H */
