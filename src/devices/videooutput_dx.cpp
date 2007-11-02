
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise GetYUVHeight this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus GetYUVHeightd.
 */


/*
 *                         videooutput_dx.cpp -  description
 *                         ----------------------------------
 *   begin                : Sun Nov 19 2006
 *   copyright            : (C) 2006-2007 by Matthias Schneider
 *                          (C) 2000-2007 by Damien Sandras
 *   description          : Class to allow video output to a DirectX
 *                          accelerated window
 *
 */



#include "config.h"

#define P_FORCE_STATIC_PLUGIN 

#include "videooutput_dx.h"
#include "ekiga.h"
#include "misc.h"
#include "main.h"
#include "gmconf.h"

#include <ptlib/vconvert.h>

#include <gtk-2.0/gdk/gdkwin32.h>


/* The Methods */
GMVideoDisplay_DX::GMVideoDisplay_DX ()
{ 
  /* Internal stuff */
  dxWindow = NULL;

  fallback = FALSE;
}


GMVideoDisplay_DX::~GMVideoDisplay_DX()
{
}

BOOL 
GMVideoDisplay_DX::SetupFrameDisplay (int display, 
                                      guint lf_width, 
                                      guint lf_height, 
                                      guint rf_width, 
                                      guint rf_height, 
                                      double zoom)
{
  GtkWidget *main_window = NULL;
  GtkWidget *image = NULL;

  BOOL ret = FALSE;
  BOOL stay_on_top = FALSE;

  if (fallback)
    return GMVideoDisplay_GDK::SetupFrameDisplay (display, lf_width, lf_height,
                                                      rf_width, rf_height, zoom);

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  stay_on_top = gm_conf_get_bool (VIDEO_DISPLAY_KEY "stay_on_top");

  CloseFrameDisplay ();

  gm_main_window_force_redraw(main_window);

  if (gm_conf_get_bool (VIDEO_DISPLAY_KEY "disable_hw_accel")) {

    PTRACE (1, "GMVideoDisplay_DX\tDirectX Hardware acceleration disabled by configuration - falling back to GDK rendering");
    return FALSE;
  }

  switch (display) {
  case LOCAL_VIDEO:
    image = gm_main_window_get_resized_video_widget (main_window,
                                                     (int) (lf_width * zoom),
                                                     (int) (lf_height * zoom));
    if (!GTK_WIDGET_REALIZED (image)) 
      return TRUE;

    dxWindow = new DXWindow();
    ret = dxWindow->Init ((HWND)GDK_WINDOW_HWND (image->window),
                          image->allocation.x,
                          image->allocation.y,
                          (int) (lf_width * zoom), 
                          (int) (lf_height * zoom),
                          lf_width, 
                          lf_height);

    lastFrame.embeddedX = image->allocation.x;
    lastFrame.embeddedY = image->allocation.y;

    lastFrame.display = LOCAL_VIDEO;
    lastFrame.localWidth = lf_width;
    lastFrame.localHeight = lf_height;
    lastFrame.zoom = zoom;
    break;

  case REMOTE_VIDEO:
    image = gm_main_window_get_resized_video_widget (main_window,
                                                     (int) (rf_width * zoom),
                                                     (int) (rf_height * zoom));

    if (!GTK_WIDGET_REALIZED (image))  
      return TRUE;

    dxWindow = new DXWindow();
    ret = dxWindow->Init ((HWND)GDK_WINDOW_HWND (image->window),
                          image->allocation.x,
                          image->allocation.y,
                          (int) (rf_width * zoom), 
                          (int) (rf_height * zoom),
                          rf_width, 
                          rf_height); 

    lastFrame.embeddedX = image->allocation.x;
    lastFrame.embeddedY = image->allocation.y;

    lastFrame.display = REMOTE_VIDEO;
    lastFrame.remoteWidth = rf_width;
    lastFrame.remoteHeight = rf_height;
    lastFrame.zoom = zoom;
    break;

  case FULLSCREEN:
  case PIP:
  case PIP_WINDOW:
    image = gm_main_window_get_resized_video_widget (main_window,
                                                     (int) (rf_width * zoom),
                                                     (int) (rf_height * zoom));
    if (!GTK_WIDGET_REALIZED (image))  
      return TRUE;

    dxWindow = new DXWindow();
    ret = dxWindow->Init ((display == PIP) ? (HWND)GDK_WINDOW_HWND (image->window) : NULL,
                          (display == PIP) ? image->allocation.x : 0,
                          (display == PIP) ? image->allocation.y : 0,
                          (int) (rf_width * zoom), 
                          (int) (rf_height * zoom),
                          rf_width, 
                          rf_height,
                          lf_width, 
                          lf_height); 

    if (ret && dxWindow && display == FULLSCREEN) 
      dxWindow->ToggleFullscreen ();

    lastFrame.embeddedX = image->allocation.x;
    lastFrame.embeddedY = image->allocation.y;

    lastFrame.display = display;
    lastFrame.localWidth = lf_width;
    lastFrame.localHeight = lf_height;
    lastFrame.remoteWidth = rf_width;
    lastFrame.remoteHeight = rf_height;
    lastFrame.zoom = zoom;
    break;

  default:
    return FALSE;
    break;
  }
  PTRACE (4, "GMVideoDisplay_DX\tSetup display " << display << " with zoom value of " << zoom << " : " << (ret ? "Success" : "Failure"));

  if (stay_on_top) {

    if (ret && dxWindow)
      dxWindow->ToggleOntop ();
  }

  if (!ret)

    CloseFrameDisplay ();

  return ret;
}


BOOL 
GMVideoDisplay_DX::CloseFrameDisplay ()
{
  if (fallback)

    return GMVideoDisplay_GDK::CloseFrameDisplay ();

  if (dxWindow) {

    delete dxWindow;
    dxWindow = NULL;
  }

  return TRUE;
}

BOOL 
GMVideoDisplay_DX::FrameDisplayChangeNeeded (int display, 
                                             guint lf_width, 
                                             guint lf_height, 
                                             guint rf_width, 
                                             guint rf_height, 
                                             double zoom)
{
  if (!dxWindow) 
    return TRUE;

  return GMVideoDisplay_GDK::FrameDisplayChangeNeeded (display, lf_width, lf_height, rf_width, rf_height, zoom);
}


void 
GMVideoDisplay_DX::DisplayFrame (gpointer gtk_image,
                                      const guchar *frame,
                                      guint width,
                                      guint height,
                                      double zoom)
{
  if (fallback) {

     GMVideoDisplay_GDK::DisplayFrame (image, frame, width, height, zoom);
    return;
  }

  if  (dxWindow)

    dxWindow->PutFrame ((uint8_t *) frame, width, height);
}

void 
GMVideoDisplay_DX::DisplayPiPFrames (gpointer gtk_image,
                                          const guchar *lframe,
                                          guint lwidth,
                                          guint lheight,
                                          const guchar *rframe,
                                          guint rwidth,
                                          guint rheight,
                                          double zoom)
{
  GtkWidget *main_window = NULL;
  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  if (fallback) {

     GMVideoDisplay_GDK::DisplayPiPFrames (image, lframe, lwidth, rheight,
                                           rframe, rwidth, rheight, zoom);
  }

  if (currentFrame.display == FULLSCREEN && dxWindow && !dxWindow->IsFullScreen ()) {

    gm_main_window_toggle_fullscreen(main_window);
  }

  if (dxWindow)

    dxWindow->PutFrame ((uint8_t *) rframe, rwidth, rheight, 
                        (uint8_t *) lframe, lwidth, lheight);
}
