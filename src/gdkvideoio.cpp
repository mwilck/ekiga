 
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

  gw = w;

  /* Used to distinguish between input and output device. */
  device_id = 0; 

#ifdef HAS_SDL
  screen = NULL;
  overlay = NULL;

  gw->fullscreen = 0;
#endif
}


GDKVideoOutputDevice::GDKVideoOutputDevice(int idno, GmWindow *w)
{
  transmitted_frame_number = 0;
  received_frame_number = 0;

  gw = w;

  /* Used to distinguish between input and output device. */
  device_id = idno; 

#ifdef HAS_SDL
  screen = NULL;
  overlay = NULL;

  gw->fullscreen = 0;
#endif
}


GDKVideoOutputDevice::~GDKVideoOutputDevice()
{
  redraw_mutex.Wait ();
  redraw_mutex.Signal ();

#ifdef HAS_SDL
  /* If it is needed, delete the SDL window */
  if (screen) {
    
    SDL_FreeSurface (screen);
    if (overlay)
      SDL_FreeYUVOverlay (overlay);
    screen = NULL;
    overlay = NULL;
    SDL_Quit ();
  }

  gw->fullscreen = 0;
#endif
}


BOOL GDKVideoOutputDevice::Redraw (const void * frame)
{
  GdkPixbuf *src_pic = NULL;
  GdkPixbuf *zoomed_pic = NULL;
  GdkPixbuf *tmp = NULL;
  GnomeUIInfo *video_view_menu_uiinfo = NULL;
  GtkRequisition size_request;
  PMutex both_mutex;
  GConfClient *client = NULL;

  int unref = 1; /* unreference zoomed_pic */

  int zoomed_width = (int) (frameWidth * gw->zoom);
  int zoomed_height = (int) (frameHeight * gw->zoom);

  int display = 0;

  static GdkPixbuf *local_pic = NULL;


  /* Common to the 2 instances of gdkvideoio */
#ifdef HAS_SDL
  static gboolean has_to_fs = false;
  static gboolean has_to_switch_fs = false;
  static gboolean old_fullscreen = false;

  static int fs_device = 0;
#endif

  client = gconf_client_get_default ();

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
#ifdef HAS_SDL
  if (GTK_CHECK_MENU_ITEM (video_view_menu_uiinfo [3].widget)->active)
    display = 3;
#endif
  
#ifdef HAS_SDL
  /* If it is needed, delete the SDL window */
  sdl_mutex.Wait ();
  if ((display != 3) && (screen) && (!gw->fullscreen)) {
    
    SDL_FreeSurface (screen);
    if (overlay)
      SDL_FreeYUVOverlay (overlay);
    screen = NULL;
    overlay = NULL;
    SDL_Quit ();
  }
  else
    /* SDL init */
    if (((display == 3) || (gw->fullscreen)) && (!screen))
      SDL_Init (SDL_INIT_VIDEO);
  sdl_mutex.Signal ();
#endif


  if ((int) (buffer.GetSize ()) != (int) (frameWidth * frameHeight * 3))
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


#ifdef HAS_SDL  
  /* Need to go full screen or to close the SDL window ? */
  gnomemeeting_threads_enter ();
  sdl_mutex.Wait ();
  has_to_fs = gw->fullscreen;

  /* Need to toggle fullscreen */
  if (old_fullscreen == !gw->fullscreen) {

    has_to_switch_fs = true;
    old_fullscreen = gw->fullscreen;
  }

  SDL_Event event;
  
  /* If we are in full screen, check that "Esc" is not pressed */
  if (has_to_fs)
    while (SDL_PollEvent (&event)) {
  
      if (event.type == SDL_KEYDOWN) {

	/* Exit Full Screen */
	if ((event.key.keysym.sym == SDLK_ESCAPE) ||
	    (event.key.keysym.sym == SDLK_f)) {

	  has_to_fs = !has_to_fs;
	  gw->fullscreen = has_to_fs;
	  has_to_switch_fs = true;
	}

	if (event.key.keysym.sym == SDLK_PLUS) {
	
	  if (gw->zoom * 2 <= 2)
	    gw->zoom = gw->zoom * 2;
	}

      }

    }
  

  if (has_to_switch_fs) {

    if (display == 0)
      fs_device = 1;
    else
    if (display == 1)
      fs_device = 0;
    else
      fs_device = display;
  }
    
  sdl_mutex.Signal ();
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
  /* Display local in a SDL window, or the selected device in fullscreen */
  if ( ((display == 3) && (device_id == 1) && (!has_to_fs))
       || ((has_to_fs) && (device_id == fs_device)) ) {
  
    gnomemeeting_threads_enter ();
    sdl_mutex.Wait ();
    /* If needed to open the screen */
    if ((!overlay) || (!screen) || (has_to_switch_fs) ||
	(screen->w != zoomed_width) || (screen->h != zoomed_height)){

      if (!has_to_fs) {

	screen = SDL_SetVideoMode (zoomed_width, zoomed_height, 0, 
				   SDL_SWSURFACE | SDL_HWSURFACE | 
				   SDL_ANYFORMAT);
	SDL_ShowCursor (SDL_ENABLE);
	has_to_switch_fs = false;
      }
    }


    /* Go fullscreen */
    if (has_to_switch_fs) {
      
	screen = SDL_SetVideoMode (640, 480, 0, 
				   SDL_SWSURFACE | SDL_HWSURFACE | 
				   SDL_ANYFORMAT);
      
      SDL_WM_ToggleFullScreen (screen);
      SDL_ShowCursor (SDL_DISABLE);
      has_to_switch_fs = false;
    }
    
    SDL_FreeYUVOverlay (overlay);
    overlay = SDL_CreateYUVOverlay (frameWidth, frameHeight, 
				    SDL_IYUV_OVERLAY, screen);

    unsigned char *base = (unsigned char *) frame;
    overlay->pixels [0] = base;
    overlay->pixels [1] = base + (frameWidth * frameHeight);
    overlay->pixels [2] = base + (frameWidth * frameHeight * 5/4);

    if (has_to_fs) {

      zoomed_width = 
	gconf_client_get_int (client, 
			      "/apps/gnomemeeting/general/fullscreen_width", 
			      NULL);
      zoomed_height = 
	gconf_client_get_int (client, 
			      "/apps/gnomemeeting/general/fullscreen_height", 
			      NULL);
    }

    dest.x = (int) (screen->w - zoomed_width) / 2;
    dest.y = (int) (screen->h - zoomed_height) / 2;
    dest.w = zoomed_width;
    dest.h = zoomed_height;

    SDL_LockYUVOverlay (overlay);
    SDL_DisplayYUVOverlay (overlay, &dest);    
    SDL_UnlockYUVOverlay (overlay);

    sdl_mutex.Signal ();
    gnomemeeting_threads_leave ();
  }
#endif


  /* we display both of them */
  if (display == 2) {

    /* What we transmit, in small */
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

    /* What we receive, in big */
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


