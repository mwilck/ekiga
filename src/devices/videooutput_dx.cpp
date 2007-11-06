
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
  BOOL ret = FALSE;

  WidgetInfo currentWidgetInfo;
  BOOL wasSet;

  if (fallback)
    return GMVideoDisplay_GDK::SetupFrameDisplay (display, lf_width, lf_height,
                                                      rf_width, rf_height, zoom);

  CloseFrameDisplay ();

  runtime->run_in_main (force_redraw.make_slot ());

  wasSet = GetWidget(&currentWidgetInfo.x, &currentWidgetInfo.y, &currentWidgetInfo.hwnd, &currentWidgetInfo.onTop, &currentWidgetInfo.disableHwAccel);

  if (wasSet && currentWidgetInfo.disableHwAccel) {

    PTRACE (1, "GMVideoDisplay_DX\tDirectX Hardware acceleration disabled by configuration - falling back to GDK rendering");
    return FALSE;
  }

  switch (display) {
  case LOCAL_VIDEO:
    runtime->run_in_main (sigc::bind (set_resized_video_widget.make_slot (), (int) (lf_width * zoom), (int) (lf_height * zoom)));
    break;
  case REMOTE_VIDEO:
  case PIP:
    runtime->run_in_main (sigc::bind (set_resized_video_widget.make_slot (), (int) (rf_width * zoom), (int) (rf_height * zoom)));
    break;
  case FULLSCREEN:
  case PIP_WINDOW:
    runtime->run_in_main (sigc::bind (set_resized_video_widget.make_slot (), (int) (GM_QCIF_WIDTH), (int) (GM_QCIF_HEIGHT)));
    break;
  }

  if (!wasSet) {
    PTRACE(4, "GMVideoDisplay_DX\tWidget not yet realized, not opening display");
    return TRUE;
  }

  runtime->run_in_main (sigc::bind (set_display_type.make_slot (), display));

  switch (display) {
  case LOCAL_VIDEO:
    dxWindow = new DXWindow();
    ret = dxWindow->Init (currentWidgetInfo.hwnd,
                          currentWidgetInfo.x,
                          currentWidgetInfo.y,
                          (int) (lf_width * zoom), 
                          (int) (lf_height * zoom),
                          lf_width, 
                          lf_height);

    lastFrame.embeddedX = currentWidgetInfo.x;
    lastFrame.embeddedY = currentWidgetInfo.y;

    lastFrame.display = LOCAL_VIDEO;
    lastFrame.localWidth = lf_width;
    lastFrame.localHeight = lf_height;
    lastFrame.zoom = zoom;
    break;

  case REMOTE_VIDEO:
    dxWindow = new DXWindow();
    ret = dxWindow->Init (currentWidgetInfo.hwnd,
                          currentWidgetInfo.x,
                          currentWidgetInfo.y,
                          (int) (rf_width * zoom), 
                          (int) (rf_height * zoom),
                          rf_width, 
                          rf_height); 

    lastFrame.embeddedX = currentWidgetInfo.x;
    lastFrame.embeddedY = currentWidgetInfo.y;

    lastFrame.display = REMOTE_VIDEO;
    lastFrame.remoteWidth = rf_width;
    lastFrame.remoteHeight = rf_height;
    lastFrame.zoom = zoom;
    break;

  case FULLSCREEN:
  case PIP:
  case PIP_WINDOW:
    dxWindow = new DXWindow();
    ret = dxWindow->Init ((display == PIP) ? currentWidgetInfo.hwnd : NULL,
                          (display == PIP) ? currentWidgetInfo.x : 0,
                          (display == PIP) ? currentWidgetInfo.y : 0,
                          (int) (rf_width * zoom), 
                          (int) (rf_height * zoom),
                          rf_width, 
                          rf_height,
                          lf_width, 
                          lf_height); 

    if (ret && dxWindow && display == FULLSCREEN) 
      dxWindow->ToggleFullscreen ();

    lastFrame.embeddedX = currentWidgetInfo.x;
    lastFrame.embeddedY = currentWidgetInfo.y;

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

  if (currentWidgetInfo.onTop) {

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
  if ((!fallback) && (!dxWindow))
    return TRUE;

  return GMVideoDisplay_GDK::FrameDisplayChangeNeeded (display, lf_width, lf_height, rf_width, rf_height, zoom);
}


void 
GMVideoDisplay_DX::DisplayFrame (     const guchar *frame,
                                      guint width,
                                      guint height,
                                      double zoom)
{
  if (fallback) {

    GMVideoDisplay_GDK::DisplayFrame (frame, width, height, zoom);
    return;
  }

  if  (dxWindow)

    dxWindow->PutFrame ((uint8_t *) frame, width, height);
}

void 
GMVideoDisplay_DX::DisplayPiPFrames (     const guchar *lframe,
                                          guint lwidth,
                                          guint lheight,
                                          const guchar *rframe,
                                          guint rwidth,
                                          guint rheight,
                                          double zoom)
{
  if (fallback) {

     GMVideoDisplay_GDK::DisplayPiPFrames (lframe, lwidth, lheight,
                                           rframe, rwidth, rheight, zoom);
     return;
  }

  if (currentFrame.display == FULLSCREEN && dxWindow && !dxWindow->IsFullScreen ()) {
    runtime->run_in_main (sigc::bind (toggle_fullscreen.make_slot (), 0));
  }

  if (dxWindow)

    dxWindow->PutFrame ((uint8_t *) rframe, rwidth, rheight, 
                        (uint8_t *) lframe, lwidth, lheight);
}
