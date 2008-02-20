
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras

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
 *                         vidinput-core.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the interface of a vidinput core.
 *                          A vidinput core manages VidInputManagers.
 *
 */

#ifndef __VIDINPUT_INFO_H__
#define __VIDINPUT_INFO_H__

#include "services.h"
#include "display-core.h"
#include "gmconf-bridge.h"

#include <sigc++/sigc++.h>
#include <set>
#include <map>

#include <glib.h>

#include "ptbuildopts.h"
#include "ptlib.h"

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

namespace Ekiga
{
#define NB_VIDEO_SIZES 5

  const static struct { 
    int width; 
    int height; 
  } 
  VideoSizes[NB_VIDEO_SIZES] = {
    {  GM_QCIF_WIDTH,  GM_QCIF_HEIGHT },
    {  GM_CIF_WIDTH,   GM_CIF_HEIGHT  },
    {  GM_4CIF_WIDTH,  GM_4CIF_HEIGHT },
    {  GM_SIF_WIDTH,   GM_SIF_HEIGHT  },
    {  GM_4SIF_WIDTH,  GM_4SIF_HEIGHT },
  };
  
  enum VideoFormat {
    PAL,
    NTSC,
    SECAM,
    Auto,
    NumVideoFormats
  };

  typedef struct VidInputDevice {
    std::string type;
    std::string source;
    std::string device;
  };

  enum VidInputErrorCodes {
    ERR_NONE = 0,
    ERR_DEVICE,
    ERR_FORMAT,
    ERR_CHANNEL,
    ERR_COLOUR,
    ERR_FPS,
    ERR_SCALE
  };
				      
};

#endif
