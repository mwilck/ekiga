
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

#include "vfakeio.h"
#include <ptlib/vconvert.h>

#include "misc.h"

#include "../pixmaps/text_logo.xpm"


GMH323FakeVideoInputDevice::GMH323FakeVideoInputDevice ()
{
  data_pix = NULL;
  logo_pix = NULL;

  //  if (image)
  //video_image = g_strdup (image);
  //else
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



BOOL
GMH323FakeVideoInputDevice::Open (const PString &,
				  BOOL start_immediate)
{
  return TRUE;
}


BOOL
GMH323FakeVideoInputDevice::IsOpen ()
{
  return TRUE;
}


BOOL
GMH323FakeVideoInputDevice::Close ()
{
  return TRUE;
}

  
BOOL
GMH323FakeVideoInputDevice::Start ()
{
  return TRUE;
}

  
BOOL
GMH323FakeVideoInputDevice::Stop ()
{
  return TRUE;
}


BOOL
GMH323FakeVideoInputDevice::IsCapturing ()
{
  return IsCapturing ();
}


PStringList
GMH323FakeVideoInputDevice::GetInputDeviceNames ()
{
  PStringList l;

  l.AppendString ("MovingLogo");
  l.AppendString ("StaticPicture");

  return l;
}


BOOL
GMH323FakeVideoInputDevice::SetFrameSize (unsigned int width,
					  unsigned int height)
{
  if (!PVideoDevice::SetFrameSize (width, height))
    return FALSE;

  return TRUE;
}


BOOL
GMH323FakeVideoInputDevice::GetFrame (PBYTEArray &a)
{
  PINDEX returned;

  if (!GetFrameData (a.GetPointer (), &returned))
    return FALSE;

  a.SetSize (returned);
  
  return TRUE;
}


BOOL
GMH323FakeVideoInputDevice::GetFrameData (BYTE *a, PINDEX *i)
{
  WaitFinishPreviousFrame ();

  GetFrameDataNoDelay (a, i);

  *i = CalculateFrameBytes (frameWidth, frameHeight, colourFormat);

  return TRUE;
}


BOOL GMH323FakeVideoInputDevice::GetFrameDataNoDelay (BYTE *frame, PINDEX *i)
{
  GdkPixbuf *data_pix_tmp = NULL;

  guchar *data = NULL;

  unsigned width = 0;
  unsigned height = 0;

  GetFrameSize (width, height);

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

  if (converter)
    converter->Convert (data, frame);

  //  RGBtoYUV420PSameSize (data, frame, rgb_increment, FALSE, 
  //		width, height);
  

  gnomemeeting_threads_leave ();


  return TRUE;
}


BOOL
GMH323FakeVideoInputDevice::TestAllFormats ()
{
  return TRUE;
}


PINDEX
GMH323FakeVideoInputDevice::GetMaxFrameBytes ()
{
  return CalculateFrameBytes (frameWidth, frameHeight, colourFormat);
}


void
GMH323FakeVideoInputDevice::WaitFinishPreviousFrame ()
{
  frameTimeError += msBetweenFrames;

  PTime now;
  PTimeInterval delay = now - previousFrameTime;
  frameTimeError -= (int)delay.GetMilliSeconds();
  frameTimeError += 1000 / frameRate;
  previousFrameTime = now;

  if (frameTimeError > 0) {
    PTRACE(6, "FakeVideo\t Sleep for " << frameTimeError << " milli seconds");
#ifdef P_LINUX
    usleep(frameTimeError * 1000);
#else
    PThread::Current()->Sleep(frameTimeError);
#endif
  }
}


BOOL
GMH323FakeVideoInputDevice::SetVideoFormat (VideoFormat newFormat)
{
  return PVideoDevice::SetVideoFormat (newFormat);
}


int
GMH323FakeVideoInputDevice::GetNumChannels()
{
  return 1;
}


BOOL
GMH323FakeVideoInputDevice::SetChannel (int newChannel)
{
  return PVideoDevice::SetChannel (newChannel);
}


BOOL
GMH323FakeVideoInputDevice::SetColourFormat (const PString &newFormat)
{
  if (newFormat == "BGR32") 
    return PVideoDevice::SetColourFormat (newFormat);

  return FALSE;  
}


BOOL
GMH323FakeVideoInputDevice::SetFrameRate (unsigned rate)
{
  PVideoDevice::SetFrameRate (12);
 
  return TRUE;
}


BOOL
GMH323FakeVideoInputDevice::GetFrameSizeLimits (unsigned & minWidth,
						unsigned & minHeight,
						unsigned & maxWidth,
						unsigned & maxHeight)
{
  minWidth  = 10;
  minHeight = 10;
  maxWidth  = 1000;
  maxHeight =  800;

  return TRUE;
}


