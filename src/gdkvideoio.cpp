
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2001 Damien Sandras
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
 *                         gdkvideoio.cpp  -  description
 *                         ------------------------------
 *   begin                : Sat Feb 17 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : Class to permit to display in GDK Drawing Area
 *   email                : dsandras@seconix.com
 *
 */

#include <sys/time.h>

#include "../config.h"

#include "gdkvideoio.h"
#include "common.h"
#include "gnomemeeting.h"
#include "misc.h"

#define new PNEW


/* Global Variables */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;

/* The Methods */

GDKVideoOutputDevice::GDKVideoOutputDevice(GM_window_widgets *w)
{
  transmitted_frame_number = 0;
  received_frame_number = 0;

  /* Used to distinguish between input and output device. */
  device_id = 0; 

  gw = w;
}


GDKVideoOutputDevice::GDKVideoOutputDevice(int idno, GM_window_widgets *w)
{
 transmitted_frame_number = 0;
 received_frame_number = 0;

 /* Used to distinguish between input and output device. */
 device_id = idno; 

 /* If we don't transmit, then display is the remote image by default. */
 if (idno == 0)
   display_config = 1;
 
 /* If we transmit, then display is the local image by default. */
 if (idno == 1)
   display_config = 0;

 gw = w;
}



GDKVideoOutputDevice::~GDKVideoOutputDevice()
{
  redraw_mutex.Wait ();
  redraw_mutex.Signal ();
}


void GDKVideoOutputDevice::SetCurrentDisplay (int choice)
{
  display_config = choice;
}


BOOL GDKVideoOutputDevice::Redraw (const void * frame)
{
  GdkPixbuf *src_pic = NULL;
  GdkPixbuf *zoomed_pic = NULL;

  int unref = 1; /* unreference zoomed_pic */

  int xpos = 0 , ypos = 0;
  
  int zoomed_width = (int) (frameWidth * gw->zoom);
  int zoomed_height = (int) (frameHeight * gw->zoom);

  /* Take the mutex before the redraw */

  redraw_mutex.Wait ();

  gnomemeeting_threads_enter ();

  if (buffer.GetSize () != frameWidth * frameHeight * 3)
    buffer.SetSize(frameWidth * frameHeight * 3);

  H323VideoDevice::Redraw(frame);

  /* The real size picture */
  src_pic =  
    gdk_pixbuf_new_from_data ((const guchar *) buffer,
			      GDK_COLORSPACE_RGB, FALSE, 8, frameWidth,
			      frameHeight, frameWidth * 3, NULL, NULL);

  /* The zoomed picture */
  if (gw->zoom != 1) {
    
    zoomed_pic = 
      gdk_pixbuf_scale_simple (src_pic, zoomed_width, 
			       zoomed_height, GDK_INTERP_NEAREST);
  }
  else {

    zoomed_pic = src_pic;
    unref = 0;
  }

  gnomemeeting_threads_leave ();

  /* Need to redefine screen size ? */
  if (((gw->drawing_area->allocation.width != zoomed_width) || 
      (gw->drawing_area->allocation.height != zoomed_height)) &&
      (device_id == !display_config)) {

    gnomemeeting_threads_enter ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (gw->drawing_area), 
			   zoomed_width, zoomed_height);
    gtk_widget_set_usize (GTK_WIDGET (gw->video_frame),
			  zoomed_width + GM_FRAME_SIZE, zoomed_height);
    gnomemeeting_threads_leave ();
  }

  
  xpos = (gw->drawing_area->allocation.width - zoomed_width) / 2;
  ypos = (gw->drawing_area->allocation.height - zoomed_height) / 2;


  /* We display what we transmit, or what we receive */
  if (((device_id == 0 && display_config == 1) ||
      (device_id == 1 && display_config == 0)) &&
      (display_config != 2)) {
    
    gnomemeeting_threads_enter ();
    gdk_pixbuf_render_to_drawable(zoomed_pic, gw->pixmap,
				  gw->drawing_area->style->black_gc, 
				  0, 0,
				  xpos, ypos,
				  zoomed_width, zoomed_height, 
				  GDK_RGB_DITHER_NORMAL, 
				  0, 0);

    gtk_widget_draw (gw->drawing_area, NULL);    
    gnomemeeting_threads_leave ();
  }


  /* we display both of them */
  if (display_config == 2) {

    /* What we receive, in big */
    if (device_id == 0)	{

      gnomemeeting_threads_enter ();
      gdk_pixbuf_render_to_drawable(zoomed_pic, gw->pixmap,
				    gw->drawing_area->style->black_gc, 
				    0, 0,
				    xpos, ypos,
				    zoomed_width, zoomed_height, 
				    GDK_RGB_DITHER_NORMAL, 
				    0, 0);
      gnomemeeting_threads_leave ();
    }

    /* What we transmit, in small */
    if (device_id == 1)	{

      gnomemeeting_threads_enter ();

      GdkPixbuf *local_pic = 
	gdk_pixbuf_scale_simple (zoomed_pic, zoomed_width / 4, 
				 zoomed_height / 4, 
				 GDK_INTERP_NEAREST);

      gdk_pixbuf_render_to_drawable(local_pic, gw->pixmap,
				    gw->drawing_area->style->black_gc, 
				    0, 0,
				    0, 0,
				    zoomed_width / 4, zoomed_height / 4, 
				    GDK_RGB_DITHER_NORMAL, 
				    0, 0);
      
      gtk_widget_draw (gw->drawing_area, NULL);     
      
      gdk_pixbuf_unref (local_pic);

      gnomemeeting_threads_leave ();
    }
  }


  if (device_id == 0) {
    received_frame_number++;
  }


  if (device_id == 1) {
    transmitted_frame_number++;
  }


  gnomemeeting_threads_enter ();

  gdk_pixbuf_unref (src_pic);

  if (unref == 1) {

    gdk_pixbuf_unref (zoomed_pic);
  }

  gnomemeeting_threads_leave ();


  redraw_mutex.Signal ();

	
  return TRUE;
}


BOOL GDKVideoOutputDevice::WriteLineSegment(int x, int y, unsigned len, 
					    const BYTE * data)
{
  PINDEX offs = 3 * (x + (y * frameWidth));
  memcpy(buffer.GetPointer() + offs, data, len*3);
  return TRUE;
}


void GDKVideoOutputDevice::Wait (void)
{
  redraw_mutex.Wait ();
  redraw_mutex.Signal ();
}
