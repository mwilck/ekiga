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

  gettimeofday(&start, NULL);

  gw = w;
}


GDKVideoOutputDevice::GDKVideoOutputDevice(int idno, GM_window_widgets *w)
{
 transmitted_frame_number = 0;
 received_frame_number = 0;

 device_id = idno; /* Used to distinguish between input and output device. */

 gettimeofday(&start, NULL);

 /* If we don't transmit, then display is the remote image by default. */
 if (idno == 0)
   display_config = 1;
 
 /* If we transmit, then display is the local image by default. */
 if (idno == 1)
   display_config = 0;

 gw = w;
}


void * GDKVideoOutputDevice::Resize (void *pic)
{
  char *picsmall = NULL;
  char *pic_char = (char *) pic;

  int localx = 0;
  int x = 0;
  int quotient = 9;

  picsmall = (char *) malloc (frameWidth * frameHeight);

  if (frameWidth == 352)
    quotient = 18;
 
  for (x = 0 ; x < frameWidth * frameHeight * 3; x = x + quotient)
    {
      picsmall [localx] = pic_char [x];
      picsmall [localx + 1] = pic_char [x + 1];
      picsmall [localx + 2] = pic_char [x + 2];
      
      localx=localx + 3;
    }
    

  return (void *) picsmall;
}
	

void GDKVideoOutputDevice::DisplayConfig (int choice)
{
  display_config = choice;
}


BOOL GDKVideoOutputDevice::Redraw(const void * frame)
{
  GdkRectangle update_rec;
  
  void *pic = NULL;
  void *small_pic = NULL;
  int xpos = 0 , ypos = 0;
  
  char statusbar_msg [150];
  char tmp [5];

  update_rec.x = 0;
  update_rec.y = 0;
  update_rec.width = frameWidth;
  update_rec.height = frameHeight;

  buffer.SetSize(frameWidth * frameHeight * 3);
  

  H323VideoDevice::Redraw(frame);

  pic = (void *) malloc(frameWidth*frameHeight*3);
  memcpy (pic, buffer, frameHeight*frameWidth*3);

  // Need to redefine screen size ?
  if (((gw->drawing_area->allocation.width != frameWidth) || 
      (gw->drawing_area->allocation.height != frameHeight)) &&
      (device_id == !display_config))
    {
      gdk_threads_enter ();
      gtk_drawing_area_size (GTK_DRAWING_AREA (gw->drawing_area), 
			     frameWidth, frameHeight);
      gtk_widget_set_usize (GTK_WIDGET (gw->video_frame),
			    frameWidth + 4, frameHeight + 4);
      gdk_threads_leave ();
    }

  xpos = (gw->drawing_area->allocation.width - frameWidth) / 2;
  ypos = (gw->drawing_area->allocation.height - frameHeight) / 2;


  if (((device_id == 0 && display_config == 1) ||
      (device_id == 1 && display_config == 0)) &&
      (display_config != 2))
  {
    gdk_threads_enter ();
    gdk_draw_rgb_image (gw->pixmap, gw->drawing_area->style->black_gc, 
			xpos, ypos, 
			frameWidth, frameHeight, GDK_RGB_DITHER_NORMAL, (guchar *) pic, 
			frameWidth*3);
    gdk_threads_leave ();

    gdk_threads_enter ();
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
			      ypos, frameWidth, frameHeight, GDK_RGB_DITHER_NORMAL, 
			      (guchar *) pic, frameWidth*3);
	  gdk_threads_leave ();
	}

      // What we transmit, in small
      if (device_id == 1)
	{
	  small_pic = Resize (pic);
	 
	  gdk_threads_enter ();
	  gdk_draw_rgb_image (gw->pixmap, gw->drawing_area->style->black_gc, 0, 
			      0, frameWidth / (int) (frameWidth / 56), frameHeight / 
			      (int) (frameWidth / 56), 
			      GDK_RGB_DITHER_NORMAL, 
			      (guchar *) small_pic, frameWidth * 3);
	  gdk_threads_leave ();

	  gdk_threads_enter ();
	  gtk_widget_draw (gw->drawing_area, &update_rec);     
	  gdk_threads_leave ();
	  free (small_pic);
	}
     }

  if (device_id == 0)
    received_frame_number++;

  if (device_id == 1)
    transmitted_frame_number++;

  gettimeofday(&cur, NULL);
 
  if ((cur.tv_sec - start.tv_sec) * 1000 > 1)
    {
      strcpy (statusbar_msg, "");

      strcpy (statusbar_msg, _("Transmitted FPS: "));
      sprintf (tmp, "%d", transmitted_frame_number);
      strcat (statusbar_msg, tmp);

      strcat (statusbar_msg, "  ");

      strcat (statusbar_msg, _("Received FPS: "));
      sprintf (tmp, "%d", received_frame_number);
      strcat (statusbar_msg, tmp);

      gdk_threads_enter ();
      gnome_appbar_push (GNOME_APPBAR (gw->statusbar), statusbar_msg);
      gdk_threads_leave ();

      transmitted_frame_number = 0;
      received_frame_number = 0;

      gettimeofday (&start, NULL);
    }

  buffer.SetSize(0);
  free (pic);
	
  return TRUE;
}


BOOL GDKVideoOutputDevice::WriteLineSegment(int x, int y, unsigned len, const BYTE * data)
{
  PINDEX offs = 3 * (x + (y * frameWidth));
  memcpy(buffer.GetPointer() + offs, data, len*3);
  return TRUE;
}

/******************************************************************************/
