
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
 if (idno == 0) {

   display_config_mutex.Wait ();
   display_config = 1;
   display_config_mutex.Signal ();
 }
 
 /* If we transmit, then display is the local image by default. */
 if (idno == 1) {

   display_config_mutex.Wait ();
   display_config = 0;
   display_config_mutex.Signal ();
 }

 gw = w;
}


GDKVideoOutputDevice::~GDKVideoOutputDevice()
{
  redraw_mutex.Wait ();
  redraw_mutex.Signal ();
}


void GDKVideoOutputDevice::SetCurrentDisplay (int choice)
{
  display_config_mutex.Wait ();
  display_config = choice;
  display_config_mutex.Signal ();
}


BOOL GDKVideoOutputDevice::Redraw (const void * frame)
{
  GdkPixbuf *src_pic = NULL;
  GdkPixbuf *tmp = NULL;
  GdkPixbuf *zoomed_pic = NULL;
  GtkRequisition size_request;

  int unref = 1; /* unreference zoomed_pic */

  int xpos = 0 , ypos = 0;
  
  int zoomed_width = (int) (frameWidth * gw->zoom);
  int zoomed_height = (int) (frameHeight * gw->zoom);

  int display = 0;

  display_config_mutex.Wait ();
  display = display_config;
  display_config_mutex.Signal ();

  /* Take the mutexes before the redraw */
  redraw_mutex.Wait ();

  gnomemeeting_threads_enter ();

  if (buffer.GetSize () != frameWidth * frameHeight * 3)
    buffer.SetSize(frameWidth * frameHeight * 3);

  H323VideoDevice::Redraw(frame);

  /* The real size picture */

  tmp =  
    gdk_pixbuf_new_from_data ((const guchar *) buffer,
			      GDK_COLORSPACE_RGB, FALSE, 8, frameWidth,
			      frameHeight, frameWidth * 3, NULL, NULL);

  src_pic = gdk_pixbuf_copy (tmp);
  g_object_unref (tmp);

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


  /* Need to redefine screen size ? */
  gtk_widget_size_request (GTK_WIDGET (gw->video_frame), &size_request);

  if (((size_request.width - GM_FRAME_SIZE != zoomed_width) || 
       (size_request.height != zoomed_height)) &&
      (device_id == !display)) {

     gtk_widget_set_size_request (GTK_WIDGET (gw->video_frame),
				  zoomed_width + GM_FRAME_SIZE, zoomed_height);
  }

  gnomemeeting_threads_leave ();

  /* We display what we transmit, or what we receive */
  if (((device_id == 0 && display == 1) ||
      (device_id == 1 && display == 0)) &&
      (display != 2)) {
    
    gnomemeeting_threads_enter ();
    gtk_image_set_from_pixbuf (GTK_IMAGE (gw->video_image), 
			       GDK_PIXBUF (zoomed_pic));
    gtk_widget_queue_draw (GTK_WIDGET (gw->video_image));
    gnomemeeting_threads_leave ();
  }


  /* we display both of them */
  if (display == 2) {


    /* What we receive, in big */
    if (device_id == 0)	{

      gnomemeeting_threads_enter ();
      gtk_image_set_from_pixbuf (GTK_IMAGE (gw->video_image), GDK_PIXBUF (zoomed_pic));
      gnomemeeting_threads_leave ();
    }


    /* What we transmit, in small */
    if (device_id == 1)	{

      gnomemeeting_threads_enter ();

      GdkPixbuf *local_pic = 
 	gdk_pixbuf_scale_simple (zoomed_pic, zoomed_width / 4, 
 				 zoomed_height / 4, 
 				 GDK_INTERP_NEAREST);

      gtk_image_set_from_pixbuf (GTK_IMAGE (gw->video_image), 
				 GDK_PIXBUF (local_pic));
      gtk_widget_queue_draw (GTK_WIDGET (gw->video_image));
      
      g_object_unref (G_OBJECT (local_pic));

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


  g_object_unref (G_OBJECT (src_pic));

  if (unref == 1) {

    g_object_unref (G_OBJECT (zoomed_pic));
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


