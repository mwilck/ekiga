/***************************************************************************
                          gdkvideoio.cxx  -  description
                             -------------------
    begin                : Sat Feb 17 2001
    copyright            : (C) 2001 by Damien Sandras
    description          : Class needed to display in a GDK video outpur device
    email                : dsandras@acm.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <sys/time.h>

#include "../config.h"

#include "gdkvideoio.h"
#include "common.h"

#define new PNEW

/******************************************************************************/
/* Global Variables                                                           */
/******************************************************************************/

extern GtkWidget *gm;

/******************************************************************************/


/******************************************************************************/
/* The functions                                                              */
/******************************************************************************/

GDKVideoOutputDevice::GDKVideoOutputDevice(GM_window_widgets *w)
{
  transmitted_frame_number = 0;
  received_frame_number = 0;

  device_id = 0; /* Used to distinguish between input and output device. */

  gw = w;
}


GDKVideoOutputDevice::GDKVideoOutputDevice(int idno, GM_window_widgets *w)
{
 transmitted_frame_number = 0;
 received_frame_number = 0;

 device_id = idno; /* Used to distinguish between input and output device. */

 /* If we don't transmit, then display is the remote image by default. */
 if (idno == 0)
   display_config = 1;
 
 /* If we transmit, then display is the local image by default. */
 if (idno == 1)
   display_config = 0;

 gw = w;
}


void GDKVideoOutputDevice::Resize (void *dst_pic, void *src_pic,
				   double scale)
{
  char *pic_char = (char *) src_pic;
  char *dst_pic_char = (char *) dst_pic;

  int localx = 0;
  int x = 0;
  int quotient = 0;

  /* if we shrink the picture */
  quotient = 3 * (int) (1 / scale);
  for (x = 0 ; x < frameWidth * frameHeight * 3; x = x + quotient)
    {
      dst_pic_char [localx] = pic_char [x];
      dst_pic_char [localx + 1] = pic_char [x + 1];
      dst_pic_char [localx + 2] = pic_char [x + 2];
      
      localx=localx + 3;
    }
}
	

void GDKVideoOutputDevice::DisplayConfig (int choice)
{
  display_config = choice;
}


BOOL GDKVideoOutputDevice::Redraw(const void * frame)
{
  GdkRectangle update_rec;
  
  void *pic = NULL;
  void *pic2 = NULL;
  void *small_pic = NULL;
  int xpos = 0 , ypos = 0;
  
  char statusbar_msg [150];
  char tmp [5];

  double zoom = 0.5;
  int zoomed_width = (int) (frameWidth * zoom);
  int zoomed_height = (int) (frameHeight * zoom);

  update_rec.x = 0;
  update_rec.y = 0;
  update_rec.width = frameWidth;
  update_rec.height = zoomed_height;

  buffer.SetSize(frameWidth * frameHeight * 3);

  H323VideoDevice::Redraw(frame);

  /* The real size picture */
  pic = (void *) malloc(frameWidth * frameHeight * 3);
  memcpy (pic, buffer, frameHeight * frameWidth * 3);

  /* The zoomed picture */
  pic2 = (void *) malloc((int) (frameWidth * frameHeight * 3 * zoom));

  /* We process the resize */
  Resize (pic2, pic, zoom);

  // Need to redefine screen size ?
  if (((gw->drawing_area->allocation.width != zoomed_width) || 
      (gw->drawing_area->allocation.height != zoomed_height)) &&
      (device_id == !display_config))
    {
      gdk_threads_enter ();
      gtk_drawing_area_size (GTK_DRAWING_AREA (gw->drawing_area), 
			     zoomed_width, zoomed_height);
      gtk_widget_set_usize (GTK_WIDGET (gw->video_frame),
			    zoomed_width + GM_FRAME_SIZE, zoomed_height);
      gdk_threads_leave ();
    }

  xpos = (gw->drawing_area->allocation.width - zoomed_width) / 2;
  ypos = (gw->drawing_area->allocation.height - zoomed_height) / 2;


  if (((device_id == 0 && display_config == 1) ||
      (device_id == 1 && display_config == 0)) &&
      (display_config != 2))
  {
    gdk_threads_enter ();
    gdk_draw_rgb_image (gw->pixmap, gw->drawing_area->style->black_gc, 
			xpos, ypos, 
			zoomed_width, zoomed_height, GDK_RGB_DITHER_NORMAL, 
			(guchar *) pic2, 
			zoomed_width*3);

    gtk_widget_draw (gw->drawing_area, &update_rec);    
    gdk_threads_leave ();
 
  }

  if (display_config == 2)
    {
      // What we receive, in big
      if (device_id == 0)
	{
	  gdk_threads_enter ();
	  gdk_draw_rgb_image (gw->pixmap, gw->drawing_area->style->black_gc, 
			      xpos, 
			      ypos, zoomed_width, zoomed_height, 
			      GDK_RGB_DITHER_NORMAL, 
			      (guchar *) pic2, zoomed_width*3);

	  gdk_threads_leave ();
	}

      // What we transmit, in small
      if (device_id == 1)
	{
/*	  small_pic = Resize (pic);
	 
	  gdk_threads_enter ();
	  gdk_draw_rgb_image (gw->pixmap, gw->drawing_area->style->black_gc, 0, 
			      0, zoomed_width / (int) (zoomed_width / 56), zoomed_height / 
			      (int) (zoomed_width / 56), 
			      GDK_RGB_DITHER_NORMAL, 
			      (guchar *) small_pic, zoomed_width * 3);
	  update_rec.x = 0;
	  update_rec.y = 0;
	  update_rec.width = gw->drawing_area->allocation.width;
	  update_rec.height = gw->drawing_area->allocation.height;

	  gtk_widget_draw (gw->drawing_area, &update_rec);     
	  gdk_threads_leave ();

	  free (small_pic);*/
	}
     }

  if (device_id == 0)
    received_frame_number++;

  if (device_id == 1)
    transmitted_frame_number++;

  buffer.SetSize(0);
  free (pic);
  free (pic2);
	
  return TRUE;
}


BOOL GDKVideoOutputDevice::WriteLineSegment(int x, int y, unsigned len, const BYTE * data)
{
  PINDEX offs = 3 * (x + (y * frameWidth));
  memcpy(buffer.GetPointer() + offs, data, len*3);
  return TRUE;
}

/******************************************************************************/
