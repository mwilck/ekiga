 
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2002 Damien Sandras
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
 *                         vfakeio.cpp  -  description
 *                         ---------------------------
 *   begin                : Tue Jul 30 2003
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains a descendant of a Fake Input 
 *                          Device that display the GM logo when connected to
 *                          a remote party without using a camera.
 *   email                : dsandras@seconix.com
 *
 */


#include "../config.h"

#include <gnome.h>


#include "vfakeio.h"
#include "misc.h"


#include "../pixmaps/text_logo.xpm"


static void RGBtoYUV420PSameSize (const BYTE *, BYTE *, unsigned, BOOL, 
				  int, int);


#define rgbtoyuv(r, g, b, y, u, v) \
  y=(BYTE)(((int)30*r  +(int)59*g +(int)11*b)/100); \
  u=(BYTE)(((int)-17*r  -(int)33*g +(int)50*b+12800)/100); \
  v=(BYTE)(((int)50*r  -(int)42*g -(int)8*b+12800)/100); \



static void RGBtoYUV420PSameSize (const BYTE * rgb,
				  BYTE * yuv,
				  unsigned rgbIncrement,
				  BOOL flip, 
				  int srcFrameWidth, int srcFrameHeight) 
{
  const unsigned planeSize = srcFrameWidth*srcFrameHeight;
  const unsigned halfWidth = srcFrameWidth >> 1;
  
  // get pointers to the data
  BYTE * yplane  = yuv;
  BYTE * uplane  = yuv + planeSize;
  BYTE * vplane  = yuv + planeSize + (planeSize >> 2);
  const BYTE * rgbIndex = rgb;

  for (unsigned y = 0; y < srcFrameHeight; y++) {
    BYTE * yline  = yplane + (y * srcFrameWidth);
    BYTE * uline  = uplane + ((y >> 1) * halfWidth);
    BYTE * vline  = vplane + ((y >> 1) * halfWidth);

    if (flip)
      rgbIndex = rgb + (srcFrameWidth*(srcFrameHeight-1-y)*rgbIncrement);

    for (unsigned x = 0; x < srcFrameWidth; x+=2) {
      rgbtoyuv(rgbIndex[0], rgbIndex[1], rgbIndex[2],*yline, *uline, *vline);
      rgbIndex += rgbIncrement;
      yline++;
      rgbtoyuv(rgbIndex[0], rgbIndex[1], rgbIndex[2],*yline, *uline, *vline);
      rgbIndex += rgbIncrement;
      yline++;
      uline++;
      vline++;
    }
  }
}


GMH323FakeVideoInputDevice::GMH323FakeVideoInputDevice (gchar *video_image)
{
  GdkPixbuf *data_pix_tmp = NULL;
  data_pix = NULL;

  gnomemeeting_threads_enter ();
  if (video_image) {

    data_pix_tmp =  gdk_pixbuf_new_from_file (video_image, NULL);

    if (data_pix_tmp) {

      data_pix = gdk_pixbuf_scale_simple (data_pix_tmp, 176, 144, 
					  GDK_INTERP_NEAREST);

      g_object_unref (data_pix_tmp);
    }
  }

  if (!data_pix)
    data_pix = gdk_pixbuf_new_from_xpm_data ((const char **) text_logo_xpm);

  data = gdk_pixbuf_get_pixels (data_pix);
  rgb_increment = gdk_pixbuf_get_n_channels (data_pix);

  gnomemeeting_threads_leave ();
}


GMH323FakeVideoInputDevice::~GMH323FakeVideoInputDevice ()
{
  gnomemeeting_threads_enter ();

  g_object_unref (G_OBJECT (data_pix));

  gnomemeeting_threads_leave ();
}


BOOL GMH323FakeVideoInputDevice::GetFrameDataNoDelay (BYTE *frame, PINDEX *i)
{
  unsigned width = 0;
  unsigned height = 0;
  

  GetFrameSize (width, height);

  grabCount++;

  RGBtoYUV420PSameSize (data, frame, rgb_increment, FALSE, width, height);

  return TRUE;
}
