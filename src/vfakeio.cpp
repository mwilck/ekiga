
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2003 Damien Sandras
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
 *                         vfakeio.cpp  -  description
 *                         ---------------------------
 *   begin                : Tue Jul 30 2003
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains a descendant of a Fake Input 
 *                          Device that display the GM logo when connected to
 *                          a remote party without using a camera.
 *
 */


#include "../config.h"

#ifndef DISABLE_GNOME
#include <gnome.h>
#else
#include <gtk/gtk.h>
#endif

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

  for (int y = 0; y < (int) srcFrameHeight; y++) {
    BYTE * yline  = yplane + (y * srcFrameWidth);
    BYTE * uline  = uplane + ((y >> 1) * halfWidth);
    BYTE * vline  = vplane + ((y >> 1) * halfWidth);

    if (flip)
      rgbIndex = rgb + (srcFrameWidth*(srcFrameHeight-1-y)*rgbIncrement);

    for (int x = 0; x < (int) srcFrameWidth; x+=2) {
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


GMH323FakeVideoInputDevice::GMH323FakeVideoInputDevice (gchar *image)
{
  data_pix = NULL;
  logo_pix = NULL;

  if (image)
    video_image = g_strdup (image);
  else
    video_image = NULL;

  pos = 0;
  increment = 1;

  picture = false;

  gnomemeeting_threads_enter ();

  logo_pix = 
    gdk_pixbuf_new_from_xpm_data ((const char **) text_logo_xpm);

  gnomemeeting_threads_leave ();
}


GMH323FakeVideoInputDevice::~GMH323FakeVideoInputDevice ()
{
  gnomemeeting_threads_enter ();

  if (data_pix)
    g_object_unref (G_OBJECT (data_pix));

  if (logo_pix)
    g_object_unref (G_OBJECT (logo_pix));

  gnomemeeting_threads_leave ();

  g_free (video_image);
}


BOOL GMH323FakeVideoInputDevice::GetFrameDataNoDelay (BYTE *frame, PINDEX *i)
{
  GdkPixbuf *data_pix_tmp = NULL;

  unsigned width = 0;
  unsigned height = 0;

  GetFrameSize (width, height);

  grabCount++;

  gnomemeeting_threads_enter ();
  if ((video_image)&&(!data_pix)) {

    data_pix_tmp =  gdk_pixbuf_new_from_file (video_image, NULL);

    if (data_pix_tmp) {

      data_pix = gdk_pixbuf_scale_simple (data_pix_tmp, 
					  width, height, 
					  GDK_INTERP_NEAREST);

      g_object_unref (data_pix_tmp);

      if (data_pix)
	picture = true;
    }
  }

  if (!data_pix) {

    data_pix = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, 
			       width, height);
    picture = false;
  }

  if (!picture) {

    gdk_pixbuf_fill (data_pix, 0x000000FF); /* Opaque black */
    gdk_pixbuf_copy_area (logo_pix, 0, 0, 176, 60, 
			  data_pix, (width - 176) / 2, pos);

    pos = pos + increment;

    if ((int) pos > (int) height - 60 - 10) increment = -1;
    if (pos < 10) increment = +1;
  }

  data = gdk_pixbuf_get_pixels (data_pix);
  rgb_increment = gdk_pixbuf_get_n_channels (data_pix);

  RGBtoYUV420PSameSize (data, frame, rgb_increment, FALSE, 
			width, height);


  gnomemeeting_threads_leave ();


  return TRUE;
}
