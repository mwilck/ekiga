
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
 *                         gdkvideoio.cpp  -  description
 *                         ------------------------------
 *   begin                : Sat Feb 17 2001
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : Class to permit to display in GDK Drawing Area
 *   email                : dsandras@seconix.com
 *
 */

#include "../config.h"


#ifdef HAS_SDL
#include <SDL.h>
#endif

#include <sys/time.h>


#include "gdkvideoio.h"
#include "common.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "menu.h"


#define new PNEW


/* Global Variables */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;




/* The Methods */

GDKVideoOutputDevice::GDKVideoOutputDevice(GmWindow *w)
{
  transmitted_frame_number = 0;
  received_frame_number = 0;

  /* Used to distinguish between input and output device. */
  device_id = 0; 

  gw = w;
}


GDKVideoOutputDevice::GDKVideoOutputDevice(int idno, GmWindow *w)
{
  transmitted_frame_number = 0;
  received_frame_number = 0;

  /* Used to distinguish between input and output device. */
  device_id = idno; 

  gw = w;
}


GDKVideoOutputDevice::~GDKVideoOutputDevice()
{
  /* Display again the local image */
  redraw_mutex.Wait ();
  redraw_mutex.Signal ();
 
  gnomemeeting_threads_enter ();
  gnomemeeting_video_submenu_select (0);
  gnomemeeting_threads_leave ();

  redraw_mutex.Wait ();
  redraw_mutex.Signal ();
}


BOOL GDKVideoOutputDevice::Redraw (const void * frame)
{
  GdkPixbuf *src_pic = NULL;
  GdkPixbuf *zoomed_pic = NULL;
  GdkPixbuf *tmp = NULL;
  GnomeUIInfo *video_view_menu_uiinfo = NULL;
  GtkRequisition size_request;
  PMutex both_mutex;

  int unref = 1; /* unreference zoomed_pic */

  int xpos = 0 , ypos = 0;
  
  int zoomed_width = (int) (frameWidth * gw->zoom);
  int zoomed_height = (int) (frameHeight * gw->zoom);

  int display = 0;

  static GdkPixbuf *local_pic = NULL;


  /* We do have 2 gdkvideoio devices, one for encoding, one for decoding,
     but the SDL surface must be the same */
#ifdef HAS_SDL
  static gboolean has_to_fs = false; /* Do Full Screen */
  static gboolean has_to_switch_fs = false;

  static SDL_Surface *screen = NULL;
  static SDL_Overlay *overlay = NULL;

  SDL_Rect dest;
#endif


  /* Take the mutexes before the redraw */
  redraw_mutex.Wait ();

  gnomemeeting_threads_enter ();
  video_view_menu_uiinfo = (GnomeUIInfo *) 
    g_object_get_data (G_OBJECT(gm), "video_view_menu_uiinfo");


  if (GTK_CHECK_MENU_ITEM (video_view_menu_uiinfo [0].widget)->active)
    display = 0;
  if (GTK_CHECK_MENU_ITEM (video_view_menu_uiinfo [1].widget)->active)
    display = 1;
  if (GTK_CHECK_MENU_ITEM (video_view_menu_uiinfo [2].widget)->active)
    display = 2;
  if (GTK_CHECK_MENU_ITEM (video_view_menu_uiinfo [3].widget)->active)
    display = 3;

  
#ifdef HAS_SDL
  /* If it is needed, delete the SDL window */
  if ((display != 3) && (screen)) {
    
    SDL_FreeSurface (screen);
    if (overlay)
      SDL_FreeYUVOverlay (overlay);
    screen = NULL;
    overlay = NULL;
    SDL_Quit ();
  }
  else
    /* SDL init */
    if ((display == 3) && (!screen))
      SDL_Init (SDL_INIT_VIDEO);
#endif


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


  /* Need to go full screen or to close the SDL window ? */
#ifdef HAS_SDL  
  gnomemeeting_threads_enter ();
  SDL_Event event;
  
  if (display == 3)
    while (SDL_PollEvent (&event)) {
  
      if ((event.type == SDL_KEYDOWN) && (event.key.keysym.sym == SDLK_f)) {

	has_to_fs = !has_to_fs;
	has_to_switch_fs = true;
      }
    }
  gnomemeeting_threads_leave ();
#endif


  /* We display what we transmit, or what we receive */
  if ( (((device_id == 0 && display == 1) ||
	(device_id == 1 && display == 0)) && (display != 2)) 
       || ((display == 3) && (device_id == 0)) ) {
    
    gnomemeeting_threads_enter ();
    gtk_image_set_from_pixbuf (GTK_IMAGE (gw->video_image), 
 			       GDK_PIXBUF (zoomed_pic));
    gnomemeeting_threads_leave ();
  }


#ifdef HAS_SDL
  /* Display local in a SDL window */
  if ((display == 3) && (device_id == 1)) {
  
    gnomemeeting_threads_enter ();
    if ((!overlay) || (!screen) || (has_to_switch_fs) ||
	(screen->w != zoomed_width) || (screen->h != zoomed_height)){

      if (!has_to_fs) {

	screen = SDL_SetVideoMode (zoomed_width, zoomed_height, 0, 
				   SDL_SWSURFACE | SDL_HWSURFACE | 
				   SDL_ANYFORMAT);
	SDL_ShowCursor (SDL_ENABLE);
	has_to_switch_fs = false;
      }
      else 
	if (has_to_switch_fs) {

	  if (frameWidth * gw->zoom < 640)
	    screen = SDL_SetVideoMode (640, 480, 0, 
				       SDL_SWSURFACE | SDL_HWSURFACE | 
				       SDL_ANYFORMAT);
	  else
	    screen = SDL_SetVideoMode (800, 600, 0, 
				       SDL_SWSURFACE | SDL_HWSURFACE | 
				       SDL_ANYFORMAT);
	    
	  SDL_WM_ToggleFullScreen (screen);
	  SDL_ShowCursor (SDL_DISABLE);
	  has_to_switch_fs = false;
	}

      SDL_FreeYUVOverlay (overlay);
      overlay = SDL_CreateYUVOverlay (frameWidth, frameHeight, 
				      SDL_IYUV_OVERLAY, screen);
    }

    unsigned char *base = (unsigned char *) frame;
    overlay->pixels [0] = base;
    overlay->pixels [1] = base + (frameWidth * frameHeight);
    overlay->pixels [2] = base + (frameWidth * frameHeight * 5/4);
  
    dest.x = (int) (screen->w - zoomed_width) / 2;
    dest.y = (int) (screen->h - zoomed_height) / 2;
    dest.w = zoomed_width;
    dest.h = zoomed_height;

    SDL_LockYUVOverlay (overlay);
    SDL_DisplayYUVOverlay (overlay, &dest);    
    SDL_UnlockYUVOverlay (overlay);

    gnomemeeting_threads_leave ();
  }
#endif



  /* we display both of them */
  if (display == 2) {

    if (device_id == 1) {

      both_mutex.Wait ();
      gnomemeeting_threads_enter ();
  
      if (local_pic)
	g_object_unref (local_pic);

      local_pic = 
 	gdk_pixbuf_scale_simple (zoomed_pic, zoomed_width / 3, 
 				 zoomed_height / 3, 
 				 GDK_INTERP_NEAREST);
      gnomemeeting_threads_leave ();
      both_mutex.Signal ();
    }

    /* What we transmit, in small */
    if ((device_id == 0) && (local_pic))  {

      both_mutex.Wait ();
      gnomemeeting_threads_enter ();

      if (local_pic) {
	gdk_pixbuf_copy_area  (local_pic, 
			       0 , 0,
			       zoomed_width / 3, zoomed_height / 3, 
			       zoomed_pic,
			       zoomed_width - zoomed_width / 3, 
			       zoomed_height - zoomed_height / 3);
	
	gtk_image_set_from_pixbuf (GTK_IMAGE (gw->video_image), 
				   GDK_PIXBUF (zoomed_pic));
      }
    
      gnomemeeting_threads_leave ();
      both_mutex.Signal ();
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


