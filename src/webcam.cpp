/***************************************************************************
                          webcam.cxx  -  description
                             -------------------
    begin                : Mon Feb 12 2001
    copyright            : (C) 2001 by Damien Sandras
    description          : Video4Linux compliant functions to manipulate the 
                           webcam device
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


#include "webcam.h"
#include "common.h"
#include "main_interface.h"
#include "gdkvideoio.h"

#include <iostream.h> // 


extern GtkWidget *gm;


void GM_rgb_swap (void *pic)
{
  char  c;
  char *p = (char *) pic;
  int   i = 176 * 144;
  
  while (--i) 
    {
      c = p[0]; p[0] = p[2]; p[2] = c;
      p += 3;
    }
}


int GM_cam_capture_start (gchar *device, GM_window_widgets *gw)
{
  /* Nothing here in this release */

  return 0;
}


int GM_cam_capture_stop (GM_window_widgets *gw)
{
  /* Nothing here in this release */

  return 0;
}


void * GM_cam_capture_thread (GM_window_widgets *gw)
{
/*
  int xpos, ypos;
  int len;
  void *pic;
  GdkRectangle update_rec;
  struct video_window vid_win;

  update_rec.x = 0;
  update_rec.y = 0;
  update_rec.width = 176;
  update_rec.height = 144;

  vid_win.width = 176;
  vid_win.height = 144;


  // if video device not opened
  if (gw->dev == -1)
    gw->dev = open("/dev/video", O_RDWR);

  ioctl (gw->dev, VIDIOCSWIN, &vid_win);

  pic = malloc (176* 144 * 3);  

  while (gw->dev != -1)
    {
      if (GTK_WIDGET_VISIBLE (GTK_WIDGET (gm)))
	{
	  len = read (gw->dev, (void *) pic, 176 * 144 * 3);

	  GM_rgb_swap (pic);
  
	  xpos = (gw->drawing_area->allocation.width - 176) / 2;
	  ypos = (gw->drawing_area->allocation.height - 144) / 2;


	  gdk_threads_enter ();
	  gdk_draw_rgb_image (gw->pixmap, gw->drawing_area->style->black_gc, 
			      xpos, ypos, 
			      176, 144, GDK_RGB_DITHER_NORMAL, (guchar *) pic, 
			      176*3);
	  gdk_threads_leave ();
	  
	  gdk_threads_enter ();
	  gtk_widget_draw (gw->drawing_area, &update_rec);    
	  gdk_threads_leave ();
	}
      else
	usleep (200);
    }

  free (pic);
*/
  return (0);
}


int GM_cam_info (GM_window_widgets *gw, GtkWidget *text)
{
/*
  struct video_capability vid_cap;
  char *maxh, *maxw, *minh, *minw;
  int was_opened = 1;

  maxh = (char *) malloc (12);
  maxw = (char *) malloc (12);
  minh = (char *) malloc (12);
  minw = (char *) malloc (12);

  // If the webcam device is not opened, then open it
  if (gw->dev == -1)
    {
      gw->dev = open("/dev/video", O_RDWR);
      was_opened = 0; // webcam was not opened, so we will close it
    }

  ioctl (gw->dev, VIDIOCGCAP, &vid_cap);

  gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL, 
		   " Webcam : ", -1);
  gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL, vid_cap.name, -1);
  gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL, 
		   "\n Maximum Height : ", -1);
  sprintf (maxh, "%d", vid_cap.maxheight);
  gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL, maxh, -1);
  gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL, 
		   "\n Maximum Width : ", -1);
  sprintf (maxw, "%d", vid_cap.maxwidth);
  gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL, maxw, -1);
  gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL, 
		   "\n Minimum Height : ", -1);
  sprintf (minh, "%d", vid_cap.minheight);
  gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL, minh, -1);
  gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL, 
		   "\n Minimum Width  : ", -1);
  sprintf (minw, "%d", vid_cap.minwidth);
  gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL, minw, -1);
	
  if (vid_cap.type & VID_TYPE_CAPTURE) 
    gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL, 
		     "\n Can capture ", -1);
  else
    gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL, 
		     "\n Can not capture  : ", -1);

  if (was_opened == 0)
    {
      close (gw->dev);
      gw->dev = -1;
    }

  g_free (maxw);
  g_free (maxh);
  g_free (minw);
  g_free (minh);
*/
  return 0;
}


void GM_cam_set_colour (GM_window_widgets *gw, int colour)
{
  if (gw->grabber != NULL)
    gw->grabber->SetColour (colour);
}


void GM_cam_set_brightness (GM_window_widgets *gw, int brightness)
{
  if (gw->grabber != NULL)
    gw->grabber->SetBrightness (brightness);
}


void GM_cam_set_whiteness (GM_window_widgets *gw, int whiteness)
{
  if (gw->grabber != NULL)
    gw->grabber->SetWhiteness (whiteness);
}


void GM_cam_set_contrast (GM_window_widgets *gw, int constrast)
{
  if (gw->grabber != NULL)
    gw->grabber->SetContrast (constrast);
}



void GM_cam_get_params (options *opts, int *whiteness,
			int *brightness, int *colour, int *contrast)
{
  PVideoInputDevice *grabber;

  grabber = new PVideoInputDevice ();

  if (grabber->Open (opts->video_device, FALSE) 
      && grabber->SetChannel (opts->video_channel))
    {
      *whiteness = (int) grabber->GetWhiteness () / 256;
      *brightness = (int) grabber->GetBrightness () / 256;
      *colour = (int) grabber->GetColour () / 256;
      *contrast = (int) grabber->GetContrast () / 256;
    }
 
  delete (grabber);
}

 
int GM_cam (gchar *video_device, int video_channel)
{

  PVideoInputDevice *grabber;

  grabber = new PVideoInputDevice ();

  if (!grabber->Open (video_device, FALSE) || !grabber->SetChannel(video_channel))
    {
      delete (grabber);
      return 0;
    }
  else
    {
      delete (grabber);
      return 1;
    }
}

/******************************************************************************/
		

