 
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
  device_id = REMOTE; 

#ifdef HAS_SDL
  screen = NULL;
  overlay = NULL;
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
#endif

  /* Hide the 2 popup windows */
  gnomemeeting_threads_enter ();

  gtk_widget_hide_all (GTK_WIDGET (gw->local_video_window));
  gtk_widget_hide_all (GTK_WIDGET (gw->remote_video_window));
  gnomemeeting_threads_leave ();
}


BOOL GDKVideoOutputDevice::Redraw (const void * frame)
{
  GtkWidget *image = NULL;
  GdkPixbuf *src_pic = NULL;
  GdkPixbuf *zoomed_pic = NULL;
  GdkPixbuf *tmp = NULL;
  int image_width = 0, image_height = 0;
  PMutex both_mutex;
  GConfClient *client = NULL;

  gboolean unref = true;

  enum GdkInterpType interp_type = GDK_INTERP_NEAREST;
  int zoomed_width = GM_QCIF_WIDTH; 
  int zoomed_height = GM_QCIF_HEIGHT;

  int display = 0;


  /* Common to the 2 instances of gdkvideoio */
  static GdkPixbuf *local_pic = NULL;
  static gboolean logo_displayed = false;
  static double zoom = 1.0;

#ifdef HAS_SDL
  static gboolean fullscreen = false;
  static gboolean has_to_fs = false;
  static gboolean has_to_switch_fs = false;
  static gboolean old_fullscreen = false;

  static int fs_device = 0;
#endif

  client = gconf_client_get_default ();

  /* Take the mutexes before the redraw */
  redraw_mutex.Wait ();


  /* Updates the zoom value */
  zoom = 
    gconf_client_get_float (client, 
			    "/apps/gnomemeeting/video_display/zoom_factor", 
			    NULL);
  if (zoom != 0.5 && zoom != 1 && zoom != 2)
    zoom = 1.0;

#ifdef HAS_SDL 
  fullscreen =
    gconf_client_get_bool (client, 
			  "/apps/gnomemeeting/video_display/fullscreen", 0);
#endif


  /* Let's go for the GTK stuff */
  gnomemeeting_threads_enter ();
  
  /* What do we display: what gconf tells us except if we are not in 
     a call, or if it is not a valid choice */
  if (MyApp->Endpoint ()->GetCallingState () != 2) {

    display = 0;
  }
  else {

    display = 
      gconf_client_get_int (client, 
			    "/apps/gnomemeeting/video_display/video_view", 
			    NULL);

    if (MyApp->Endpoint ()->GetVideoChannelsNumber () <= 1)
      if (device_id == REMOTE)
	display = REMOTE_VIDEO;
      else if (device_id == LOCAL)
	display = LOCAL_VIDEO;
  }

  gnomemeeting_video_submenu_select (display);

  /* Show or hide the different windows if needed */
  if (display == BOTH) { /* display == BOTH */

    /* Display the GnomeMeeting logo in the main window */
    if (!logo_displayed) {

      gnomemeeting_init_main_window_logo (gw->main_video_image);
      logo_displayed = true;
    }

    if (!GTK_WIDGET_VISIBLE (gw->local_video_window))
      gtk_widget_show_all (GTK_WIDGET (gw->local_video_window));
    
    if (!GTK_WIDGET_VISIBLE (gw->remote_video_window))
      gtk_widget_show_all (GTK_WIDGET (gw->remote_video_window));

    if (device_id == REMOTE) {

      gtk_window_get_size (GTK_WINDOW (gw->remote_video_window), 
			   &zoomed_width, &zoomed_height);
      zoomed_width = zoomed_width - GM_FRAME_SIZE;
      zoomed_height = zoomed_height - GM_FRAME_SIZE;
      image = gw->remote_video_image;
    }
    else {

      gtk_window_get_size (GTK_WINDOW (gw->local_video_window), 
			   &zoomed_width, &zoomed_height);
      zoomed_width = zoomed_width - GM_FRAME_SIZE;
      zoomed_height = zoomed_height - GM_FRAME_SIZE;
      image = gw->local_video_image;
    } 
  }
  else {

    logo_displayed = false;
    zoomed_width = (int) (frameWidth * zoom);
    zoomed_height = (int) (frameHeight * zoom);

    image = gw->main_video_image;

    if (display == BOTH_LOCAL) { /* display != BOTH && display == BOTH_LOCAL */

      /* Overwrite the zoom values and the GtkImage in which we will 
	 display if display == BOTH_LOCAL and that we receive
	 the data for the LOCAL window */
      if (device_id == LOCAL) {

	image = gw->local_video_image;

	gtk_window_get_size (GTK_WINDOW (gw->local_video_window), 
			     &zoomed_width, &zoomed_height);
	zoomed_width = zoomed_width - GM_FRAME_SIZE;
	zoomed_height = zoomed_height - GM_FRAME_SIZE;
      }

      if (!GTK_WIDGET_VISIBLE (gw->local_video_window))
	gtk_widget_show_all (GTK_WIDGET (gw->local_video_window));
      
      if (GTK_WIDGET_VISIBLE (gw->remote_video_window))
	gtk_widget_hide_all (GTK_WIDGET (gw->remote_video_window));
    }
    else { /* display != BOTH && display != BOTH_LOCAL */
      
      if (GTK_WIDGET_VISIBLE (gw->local_video_window))
	gtk_widget_hide_all (GTK_WIDGET (gw->local_video_window));
      
      if (GTK_WIDGET_VISIBLE (gw->remote_video_window))
	gtk_widget_hide_all (GTK_WIDGET (gw->remote_video_window));
    }
  }


#ifdef HAS_SDL
  /* If it is needed, delete the SDL window */
  sdl_mutex.Wait ();

  /* If we are in fullscreen, but that the option is not to be in
     fullscreen, delete the fullscreen SDL window */
  if ((screen) && (!fullscreen)) {
    
    SDL_FreeSurface (screen);
    if (overlay)
      SDL_FreeYUVOverlay (overlay);
    screen = NULL;
    overlay = NULL;
    SDL_Quit ();
  }
  else
    /* If fullscreen is selected but that we are not in fullscreen, 
       call SDL init */
    if ((fullscreen) && (!screen))
      SDL_Init (SDL_INIT_VIDEO);

  sdl_mutex.Signal ();



  if (has_to_fs) {
      
    zoomed_width = 
      gconf_client_get_int (client, 
			    "/apps/gnomemeeting/video_display/fullscreen_width", 
			    NULL);
    zoomed_height = 
      gconf_client_get_int (client, 
			    "/apps/gnomemeeting/video_display/fullscreen_height", 
			    NULL);
    }
#endif

  
  if ((int) (buffer.GetSize ()) != (int) (frameWidth * frameHeight * 3))
    buffer.SetSize(frameWidth * frameHeight * 3);

  if (frame)
    H323VideoDevice::Redraw(frame);


  /* The real size picture */
  tmp =  
    gdk_pixbuf_new_from_data ((const guchar *) buffer,
			      GDK_COLORSPACE_RGB, FALSE, 8, frameWidth,
			      frameHeight, frameWidth * 3, NULL, NULL);
  src_pic = gdk_pixbuf_copy (tmp);
  g_object_unref (tmp);


  if (gconf_client_get_bool (client, "/apps/gnomemeeting/video_display/bilinear_filtering", NULL)) 
    interp_type = GDK_INTERP_BILINEAR;


  /* The zoomed picture */
  if ((zoomed_width != (int)frameWidth)||
      (zoomed_height != (int)frameHeight) ||
      (interp_type != GDK_INTERP_NEAREST)) {

    zoomed_pic = 
      gdk_pixbuf_scale_simple (src_pic, zoomed_width, 
			       zoomed_height, interp_type);
  }
  else {

    zoomed_pic = src_pic;
    unref = false;
  }

  /* Need to redefine screen size in main window (popups are already
     of the good size) ? */
  gtk_widget_get_size_request (GTK_WIDGET (gw->video_frame), 
			       &image_width, &image_height);

  if ( ((image_width != zoomed_width) || (image_height != zoomed_height)) 
       &&
      ( ((device_id == LOCAL) && (display == LOCAL_VIDEO)) ||
	((device_id == REMOTE) && (display == REMOTE_VIDEO)) ||
	((device_id == REMOTE) && (display == BOTH_LOCAL)) ||
	((display == BOTH_INCRUSTED) && (device_id == REMOTE)) ) ){

    gtk_widget_set_size_request (GTK_WIDGET (gw->video_frame),
				 zoomed_width, 
				 zoomed_height);
  }
  gnomemeeting_threads_leave ();


#ifdef HAS_SDL  
  /* Need to go full screen or to close the SDL window ? */
  gnomemeeting_threads_enter ();
  sdl_mutex.Wait ();
  has_to_fs = fullscreen;

  /* Need to toggle fullscreen */
  if (old_fullscreen == !fullscreen) {

    has_to_switch_fs = true;
    old_fullscreen = fullscreen;
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
	  fullscreen = has_to_fs;
	  has_to_switch_fs = true;
	  gconf_client_set_bool (client, 
				 "/apps/gnomemeeting/video_display/fullscreen", fullscreen, 0);
	}
      }

    }
  

  if (has_to_switch_fs) {

    if (display == 0)
      fs_device = 1;
    else
      fs_device = 0;
  }
    
  sdl_mutex.Signal ();
  gnomemeeting_threads_leave ();
#endif


  /* We display what we transmit, or what we receive */
  if ( (((device_id == 0 && display == 1) ||
	(device_id == 1 && display == 0)) && (display != 2)) 
       || ((display == 3) && (device_id == 0)) ) {
    
    gnomemeeting_threads_enter ();
    gtk_image_set_from_pixbuf (GTK_IMAGE (gw->main_video_image), 
 			       GDK_PIXBUF (zoomed_pic));
    gnomemeeting_threads_leave ();
  }
   
    
  if ( (display == BOTH) || 
       ((display == BOTH_LOCAL) && (device_id == LOCAL)) ) {
    
    gnomemeeting_threads_enter ();
    if (device_id == LOCAL)
      gtk_image_set_from_pixbuf (GTK_IMAGE (gw->local_video_image), 
				 GDK_PIXBUF (zoomed_pic));
    else
      gtk_image_set_from_pixbuf (GTK_IMAGE (gw->remote_video_image), 
				 GDK_PIXBUF (zoomed_pic));

    gnomemeeting_threads_leave ();
  }


#ifdef HAS_SDL
  /* Display local in a SDL window, or the selected device in fullscreen */
  if ( (has_to_fs) && (device_id == fs_device) ) {
  
    unsigned char *base;

    gnomemeeting_threads_enter ();
    sdl_mutex.Wait ();
   
    /* Go fullscreen */
    if (has_to_switch_fs) {
      
	screen = SDL_SetVideoMode (640, 480, 0, 
				   SDL_SWSURFACE | SDL_HWSURFACE | 
				   SDL_ANYFORMAT);
      
      SDL_WM_ToggleFullScreen (screen);
      SDL_ShowCursor (SDL_DISABLE);
      has_to_switch_fs = false;
    }

    base = (unsigned char *) frame;
    

    if (overlay)
      SDL_FreeYUVOverlay (overlay);
    overlay = SDL_CreateYUVOverlay (frameWidth, frameHeight, 
				    SDL_IYUV_OVERLAY, screen);
    
    SDL_LockYUVOverlay (overlay);
    overlay->pixels [0] = base;
    overlay->pixels [1] = base + (frameWidth * frameHeight);
    overlay->pixels [2] = base + (frameWidth * frameHeight * 5/4);
    SDL_UnlockYUVOverlay (overlay);

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
  
      if (local_pic) {

	g_object_unref (local_pic);
	local_pic = NULL;
      }

      local_pic = 
 	gdk_pixbuf_scale_simple (zoomed_pic, GM_QCIF_WIDTH / 3, 
 				 GM_QCIF_HEIGHT / 3, 
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
			       GM_QCIF_WIDTH / 3, GM_QCIF_HEIGHT / 3, 
			       zoomed_pic,
			       zoomed_width - GM_QCIF_WIDTH / 3, 
			       zoomed_height - GM_QCIF_HEIGHT / 3);
	
	gtk_image_set_from_pixbuf (GTK_IMAGE (gw->main_video_image), 
				   GDK_PIXBUF (zoomed_pic));

	g_object_unref (local_pic);
	local_pic = NULL;
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
  if (unref) 
    g_object_unref (G_OBJECT (zoomed_pic));
  

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


