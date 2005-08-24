
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
 *                         vfakeio.cpp  -  description
 *                         ---------------------------
 *   begin                : Tue Jul 30 2003
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains a descendant of a Fake Input 
 *                          Device that display the GM logo when connected to
 *                          a remote party without using a camera.
 *
 */


#include "../config.h"

#include "vfakeio.h"

#include "misc.h"
#include "gm_conf.h"

#include "../pixmaps/text_logo.xpm"

#ifndef DISABLE_GNOME
#include <libgnomevfs/gnome-vfs.h>
const size_t PVideoInputDevice_Picture::buffer_size = 4096;
#endif


#include <ptlib/vconvert.h>

#define DISABLE_GNOME

PVideoInputDevice_Picture::PVideoInputDevice_Picture ()
{
  orig_pix = NULL;
  cached_pix = NULL;
	
  pos = 0;
  increment = 1;

  moving = false;

#ifndef DISABLE_GNOME
  loader_pix = NULL;
  filehandle = NULL;

  buffer = new guchar [buffer_size];
#endif

  SetFrameRate (12);
}


PVideoInputDevice_Picture::~PVideoInputDevice_Picture ()
{
  Close ();

#ifndef DISABLE_GNOME
  delete[] buffer;
#endif
}

#ifndef DISABLE_GNOME
void 
PVideoInputDevice_Picture::loader_area_updated_cb (GdkPixbufLoader *loader,
						   gint x, 
						   gint y, 
						   gint width,
						   gint height, 
						   gpointer thisclass)
{
  PVideoInputDevice_Picture *thisc = 
    static_cast<PVideoInputDevice_Picture *> (thisclass);

  PWaitAndSignal m(thisc->pixbuf_mutex);

  if (thisc->orig_pix)
    g_object_unref (G_OBJECT (thisc->orig_pix));
  
  if (thisc->cached_pix != NULL) {
    
    g_object_unref (G_OBJECT (thisc->cached_pix));
    thisc->cached_pix = NULL;
  }

  thisc->orig_pix = gdk_pixbuf_loader_get_pixbuf (loader);
  g_object_ref (G_OBJECT (thisc->orig_pix));
}


void PVideoInputDevice_Picture::async_close_cb (GnomeVFSAsyncHandle *fp,
						GnomeVFSResult result, 
						gpointer thisclass)
{
  PVideoInputDevice_Picture *thisc = 
    static_cast<PVideoInputDevice_Picture *> (thisclass);

  PWaitAndSignal m(thisc->pixbuf_mutex);

  if (thisc->loader_pix != NULL) {
    
    gdk_pixbuf_loader_close (thisc->loader_pix, NULL);
    g_object_unref (G_OBJECT (thisc->loader_pix));
    thisc->loader_pix = NULL;
  }
}


void 
PVideoInputDevice_Picture::async_read_cb (GnomeVFSAsyncHandle *fp,
					  GnomeVFSResult result, 
					  gpointer buffer,
					  GnomeVFSFileSize requested,
					  GnomeVFSFileSize bytes_read,
					  gpointer thisclass)
{
  PVideoInputDevice_Picture *thisc = 
    static_cast<PVideoInputDevice_Picture *> (thisclass);
  
  if (result != GNOME_VFS_OK && result != GNOME_VFS_ERROR_EOF) {
   
    gnome_vfs_async_close (fp, async_close_cb, thisclass);
    return;
  }
  
  if (thisc->loader_pix != NULL)
    gdk_pixbuf_loader_write (thisc->loader_pix,
			     thisc->buffer, 
			     thisc->buffer_size, 
			     NULL);
  
  if (result == GNOME_VFS_ERROR_EOF) {
    
    gnome_vfs_async_close (fp, async_close_cb, thisclass);    
  } 
  else if (result != GNOME_VFS_OK) {
  
    thisc->error_loading_pixbuf ();
  } 
  else {
  
    gnome_vfs_async_read (fp, 
			  thisc->buffer, 
			  thisc->buffer_size,
			  async_read_cb, 
			  thisclass);
  }
}


void 
PVideoInputDevice_Picture::async_open_cb (GnomeVFSAsyncHandle *fp,
					  GnomeVFSResult result, 
					  gpointer thisclass)
{
  PVideoInputDevice_Picture *thisc = 
    static_cast<PVideoInputDevice_Picture *> (thisclass);
  

  if (result != GNOME_VFS_OK) {

    gnome_vfs_async_close (fp, async_close_cb, thisclass);
    return thisc->error_loading_pixbuf ();
  }

  gnome_vfs_async_read (fp, 
			thisc->buffer, 
			buffer_size,
			async_read_cb, 
			thisclass);
}


gboolean 
PVideoInputDevice_Picture::async_cancel (gpointer data)
{
  gnome_vfs_async_cancel ((GnomeVFSAsyncHandle *) data);

  return FALSE;
}


void 
PVideoInputDevice_Picture::error_loading_pixbuf ()
{
  if (orig_pix)
    g_object_unref (G_OBJECT (orig_pix));

  orig_pix = gdk_pixbuf_new_from_xpm_data ((const char **) text_logo_xpm);
  g_object_ref (G_OBJECT (orig_pix));
}
#endif


BOOL
PVideoInputDevice_Picture::Open (const PString &name,
				 BOOL start_immediate)
{
  gchar *image_name = NULL;
    
  if (IsOpen ())
    return FALSE;
  
  if (name == "MovingLogo") {
  
    moving = true;
    orig_pix = gdk_pixbuf_new_from_xpm_data ((const char **) text_logo_xpm);
    
    return TRUE;
  }
  else {

    /* from there on, we're in the static picture case! */
    moving = false;

    image_name = gm_conf_get_string (VIDEO_DEVICES_KEY "image");

    PWaitAndSignal m(pixbuf_mutex);
    if (orig_pix != NULL) {

      g_object_unref (G_OBJECT (orig_pix));
      orig_pix = NULL;
    }

#ifdef DISABLE_GNOME
    orig_pix =  gdk_pixbuf_new_from_file (image_name, NULL);
    g_free (image_name);

    if (orig_pix) 
      return TRUE;

    return FALSE;
#else
    loader_pix = gdk_pixbuf_loader_new ();
    g_signal_connect (G_OBJECT (loader_pix), "area-updated",
		      G_CALLBACK (loader_area_updated_cb), this);

    gnome_vfs_async_open (&filehandle, 
			  image_name, 
			  GNOME_VFS_OPEN_READ,
			  GNOME_VFS_PRIORITY_DEFAULT,
			  async_open_cb,
			  this);

    g_free (image_name);

    return TRUE;
#endif
  }
}


BOOL
PVideoInputDevice_Picture::IsOpen ()
{
  if (orig_pix) 
    return TRUE;
  
  return FALSE;
}


BOOL
PVideoInputDevice_Picture::Close ()
{
  gnomemeeting_threads_enter ();
  
#ifndef DISABLE_GNOME
  if (filehandle != NULL) {
  
    g_idle_add (async_cancel, filehandle);
    filehandle = NULL;
  }
  
  if (loader_pix != NULL) {
    
    gdk_pixbuf_loader_close (loader_pix, NULL);
    g_object_unref (G_OBJECT (loader_pix));
    loader_pix = NULL;
  }
#endif
  
  PWaitAndSignal m(pixbuf_mutex);

  if (orig_pix != NULL) {
  
    g_object_unref (G_OBJECT (orig_pix));
    orig_pix = NULL;
  }
  
  if (cached_pix != NULL) {
  
    g_object_unref (G_OBJECT (cached_pix));
    cached_pix = NULL;
  }
  
  gnomemeeting_threads_leave ();
  
  return TRUE;
}

  
BOOL
PVideoInputDevice_Picture::Start ()
{
  return TRUE;
}

  
BOOL
PVideoInputDevice_Picture::Stop ()
{
  return TRUE;
}


BOOL
PVideoInputDevice_Picture::IsCapturing ()
{
  return IsCapturing ();
}


PStringList
PVideoInputDevice_Picture::GetInputDeviceNames ()
{
  PStringList l;

  l.AppendString ("MovingLogo");
  l.AppendString ("StaticPicture");

  return l;
}


BOOL
PVideoInputDevice_Picture::SetFrameSize (unsigned int width,
					  unsigned int height)
{
  if (!PVideoDevice::SetFrameSize (width, height))
    return FALSE;

  return TRUE;
}


BOOL
PVideoInputDevice_Picture::GetFrameData (BYTE *a, PINDEX *i)
{
  WaitFinishPreviousFrame ();

  GetFrameDataNoDelay (a, i);

  *i = CalculateFrameBytes (frameWidth, frameHeight, colourFormat);

  return TRUE;
}


BOOL PVideoInputDevice_Picture::GetFrameDataNoDelay (BYTE *frame, PINDEX *i)
{
  GdkPixbuf *scaled_pix = NULL;
  
  guchar *data = NULL;

  unsigned width = 0;
  unsigned height = 0;

  int orig_width = 0;
  int orig_height = 0;

  double scale_w = 0.0;
  double scale_h = 0.0;
  double scale = 0.0;
  
  GetFrameSize (width, height);

  PWaitAndSignal m(pixbuf_mutex);

  if (orig_pix == NULL)
    return FALSE;

  gnomemeeting_threads_enter ();
  
  if (!cached_pix) {
    
    cached_pix = gdk_pixbuf_new (GDK_COLORSPACE_RGB, 
				 TRUE, 
				 8,
                                 width, 
				 height);
    gdk_pixbuf_fill (cached_pix, 0x000000FF); /* Opaque black */


    if (!moving) { /* create the ever-displayed picture */
 
      orig_width = gdk_pixbuf_get_width (orig_pix);
      orig_height = gdk_pixbuf_get_height (orig_pix);
      
      if ((unsigned) orig_width <= width && (unsigned) orig_height <= height) {
	
	/* the picture fits in the  target space: center it */
        gdk_pixbuf_copy_area (orig_pix, 
			      0, 0, orig_width, orig_height,
			      cached_pix, 
                              (width - orig_width) / 2, 
                              (height - orig_height) / 2);
      }
      else { 
	
	/* the picture doesn't fit: scale 1:1, and center */
	scale_w = (double) width / orig_width;
	scale_h = (double) height / orig_height;
	
	if (scale_w < scale_h) /* one of them is known to be < 1 */
	  scale = scale_w;
	else
	  scale = scale_h;
	
	scaled_pix = 
	  gdk_pixbuf_scale_simple (orig_pix, 
				   (int) (scale * orig_width),
				   (int) (scale * orig_height), 
				   GDK_INTERP_BILINEAR);
	
	gdk_pixbuf_copy_area (scaled_pix, 
			      0, 0, 
			      (int) (scale * orig_width), 
			      (int) (scale * orig_height), 
			      cached_pix,
			      (width - (int) (scale * orig_width)) / 2, 
			      (height - (int)(scale * orig_height)) / 2);
	
	g_object_unref (G_OBJECT (scaled_pix));
      }
    }
  }
  else { /* Moving logo */
    
    orig_width = gdk_pixbuf_get_width (orig_pix);
    orig_height = gdk_pixbuf_get_height (orig_pix);

    gdk_pixbuf_fill (cached_pix, 0x000000FF); /* Opaque black */
    gdk_pixbuf_copy_area (orig_pix, 
			  0, 0, 
			  orig_width, orig_height, 
			  cached_pix, 
			  (width - orig_width) / 2, 
			  pos);

    pos = pos + increment;

    if ((int) pos > (int) height - orig_height - 10) 
      increment = -1;
    if (pos < 10) 
      increment = +1;
  }

  data = gdk_pixbuf_get_pixels (cached_pix);

  if (converter)
    converter->Convert (data, frame);

  gnomemeeting_threads_leave ();

  return TRUE;
}


BOOL
PVideoInputDevice_Picture::TestAllFormats ()
{
  return TRUE;
}


PINDEX
PVideoInputDevice_Picture::GetMaxFrameBytes ()
{
  return CalculateFrameBytes (frameWidth, frameHeight, colourFormat);
}


void
PVideoInputDevice_Picture::WaitFinishPreviousFrame ()
{
  frameTimeError += msBetweenFrames;

  PTime now;
  PTimeInterval delay = now - previousFrameTime;
  frameTimeError -= (int) delay.GetMilliSeconds();
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
PVideoInputDevice_Picture::SetVideoFormat (VideoFormat newFormat)
{
  return PVideoDevice::SetVideoFormat (newFormat);
}


int
PVideoInputDevice_Picture::GetNumChannels()
{
  return 1;
}


BOOL
PVideoInputDevice_Picture::SetChannel (int newChannel)
{
  return PVideoDevice::SetChannel (newChannel);
}


BOOL
PVideoInputDevice_Picture::SetColourFormat (const PString &newFormat)
{
  if (newFormat == "RGB32") 
    return PVideoDevice::SetColourFormat (newFormat);

  return FALSE;  
}


BOOL
PVideoInputDevice_Picture::SetFrameRate (unsigned rate)
{
  PVideoDevice::SetFrameRate (12);
 
  return TRUE;
}


BOOL
PVideoInputDevice_Picture::GetFrameSizeLimits (unsigned & minWidth,
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


BOOL PVideoInputDevice_Picture::GetParameters (int *whiteness,
						int *brightness,
						int *colour,
						int *contrast,
						int *hue)
{
  *whiteness = 0;
  *brightness = 0;
  *colour = 0;
  *contrast = 0;
  *hue = 0;

  return TRUE;
}
