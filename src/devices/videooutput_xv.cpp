
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
 *                         videooutput_xv.cpp -  description
 *                         ----------------------------------
 *   begin                : Sun Nov 19 2006
 *   copyright            : (C) 2006-2007 by Matthias Schneider
 *                          (C) 2000-2007 by Damien Sandras
 *   description          : Class to allow video output to a XVideo
 *                          accelerated window
 *
 */



#include "config.h"

#define P_FORCE_STATIC_PLUGIN 

#include "videooutput_xv.h"
#include "ekiga.h"
#include "misc.h"
#include "main.h"

#include "gmconf.h"

#include <ptlib/vconvert.h>

#include <gdk/gdkx.h>


/* The Methods */
GMVideoDisplay_XV::GMVideoDisplay_XV ()
{ 
  /* Internal stuff */
  lxvWindow = NULL;
  rxvWindow = NULL;

  rDisplay = XOpenDisplay (NULL);
  lDisplay = XOpenDisplay (NULL);
  embGC = NULL;

  pipWindowAvailable = TRUE;
  fallback = FALSE;
}


GMVideoDisplay_XV::~GMVideoDisplay_XV()
{
  stop = TRUE;
  /* Wait for the Main () method to be terminated */
  frame_available_sync_point.Signal();
  PWaitAndSignal m(quit_mutex);

  if (embGC)
    gdk_gc_destroy(embGC);

  if (lDisplay) 
    XCloseDisplay (lDisplay);
  if (rDisplay)
    XCloseDisplay (rDisplay);
}

BOOL 
GMVideoDisplay_XV::SetupFrameDisplay (int display, 
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

  wasSet = GetWidget(&currentWidgetInfo.x, &currentWidgetInfo.y, &currentWidgetInfo.gc, &currentWidgetInfo.window, &currentWidgetInfo.display, &currentWidgetInfo.onTop, &currentWidgetInfo.disableHwAccel);

  if (wasSet && currentWidgetInfo.disableHwAccel) {

    PTRACE (1, "GMVideoDisplay_XV\tXVideo Hardware acceleration disabled by configuration - falling back to GDK rendering");
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
    runtime->run_in_main (sigc::bind (set_resized_video_widget.make_slot (), (int) (GM_QCIF_WIDTH), (int) (GM_QCIF_HEIGHT)));
    break;
  case PIP_WINDOW:
    runtime->run_in_main (sigc::bind (set_resized_video_widget.make_slot (), (int) (GM_QCIF_WIDTH), (int) (GM_QCIF_HEIGHT)));
    break;
  }

  if (!wasSet) {
    PTRACE(4, "GMVideoDisplay_XV\tWidget not yet realized, not opening display");
    return TRUE;
  }

  runtime->run_in_main (sigc::bind (set_display_type.make_slot (), display));

  switch (display) {
  case LOCAL_VIDEO:
    lxvWindow = new XVWindow ();
    ret = lxvWindow->Init (currentWidgetInfo.display, 
                           currentWidgetInfo.window, 
                           currentWidgetInfo.gc,
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
    rxvWindow = new XVWindow ();
    ret = rxvWindow->Init (currentWidgetInfo.display, 
                           currentWidgetInfo.window, 
                           currentWidgetInfo.gc,
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
    rxvWindow = new XVWindow ();
    ret = rxvWindow->Init ((display == PIP) ? currentWidgetInfo.display : rDisplay, 
                           (display == PIP) ? currentWidgetInfo.window : DefaultRootWindow (rDisplay), 
                           (display == PIP) ? currentWidgetInfo.gc : NULL,
                           (display == PIP) ? currentWidgetInfo.x : 0,
                           (display == PIP) ? currentWidgetInfo.y : 0,
                           (int) (rf_width * zoom), 
                           (int) (rf_height * zoom),
                           rf_width, 
                           rf_height);

    if (ret) {
      lxvWindow = new XVWindow();
      if (lxvWindow->Init ((display == PIP) ? currentWidgetInfo.display : lDisplay, 
                             rxvWindow->GetWindowHandle (),
                             (display == PIP) ? currentWidgetInfo.gc : NULL,
                             (int) (rf_width * zoom * 2/3), 
                             (int) (rf_height * zoom * 2/3), 
                             (int) (rf_width * zoom / 3), 
                             (int) (rf_height * zoom / 3),
                             lf_width, 
                             lf_height))
      {
        if (rxvWindow) 
          rxvWindow->RegisterSlave (lxvWindow);
        if (lxvWindow) 
          lxvWindow->RegisterMaster (rxvWindow);
      }
      else {
        delete lxvWindow;
        lxvWindow = NULL;
        pipWindowAvailable = FALSE;
        PTRACE (1, "GMVideoDisplay_XV\tPIP creation failed");
      }
    }

    if (ret && rxvWindow && display == FULLSCREEN) 
      rxvWindow->ToggleFullscreen ();
    
    if ((display != PIP_WINDOW) || (display != FULLSCREEN)) {
      lastFrame.embeddedX = currentWidgetInfo.x;
      lastFrame.embeddedY = currentWidgetInfo.y;
    }

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
  PTRACE (4, "GMVideoDisplay_XV\tSetup display " << display << " with zoom value of " << zoom << " : " << (ret ? "Success" : "Failure"));

  if (currentWidgetInfo.onTop) {

    if (ret && lxvWindow)
      lxvWindow->ToggleOntop ();
    if (ret && rxvWindow)
      rxvWindow->ToggleOntop ();
  }

  if (!ret)
    CloseFrameDisplay ();

  return ret;
}


BOOL 
GMVideoDisplay_XV::CloseFrameDisplay ()
{
  if (fallback)
    return GMVideoDisplay_GDK::CloseFrameDisplay ();

  if (rxvWindow) 
    rxvWindow->RegisterSlave (NULL);
  if (lxvWindow) 
    lxvWindow->RegisterMaster (NULL);

  if (rxvWindow) {
    delete rxvWindow;
    rxvWindow = NULL;
  }

  if (lxvWindow) {
    delete lxvWindow;
    lxvWindow = NULL;
  }

  return TRUE;
}

BOOL 
GMVideoDisplay_XV::FrameDisplayChangeNeeded (int display, 
                                                  guint lf_width, 
                                                  guint lf_height, 
                                                  guint rf_width, 
                                                  guint rf_height, 
                                                  double zoom)
{
  if (!fallback) {
    switch (display) 
    {
      case LOCAL_VIDEO:
          if (!lxvWindow) 
            return TRUE;
          break;
      case REMOTE_VIDEO:
          if (!rxvWindow) 
            return TRUE;
          break;
      case FULLSCREEN:
      case PIP:
      case PIP_WINDOW:
          if (!rxvWindow || (pipWindowAvailable && (!lxvWindow)) )
              return TRUE;
          break;
    }
  }
  return GMVideoDisplay_GDK::FrameDisplayChangeNeeded (display, lf_width, lf_height, rf_width, rf_height, zoom);
}

void 
GMVideoDisplay_XV::DisplayFrame (     const guchar *frame,
                                      guint width,
                                      guint height,
                                      double zoom)
{
  if (fallback) {
     GMVideoDisplay_GDK::DisplayFrame (frame, width, height, zoom);
    return;
  }

  if  ((currentFrame.display == LOCAL_VIDEO) && (lxvWindow)) {
    lxvWindow->ProcessEvents();
    lxvWindow->PutFrame ((uint8_t *) frame, width, height);
    lxvWindow->Sync();
  }
 
  if  ((currentFrame.display == REMOTE_VIDEO) && (rxvWindow)) {
    rxvWindow->ProcessEvents();
    rxvWindow->PutFrame ((uint8_t *) frame, width, height);
    rxvWindow->Sync();
  }
}

void 
GMVideoDisplay_XV::DisplayPiPFrames (     const guchar *lframe,
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

  if (currentFrame.display == FULLSCREEN && rxvWindow && !rxvWindow->IsFullScreen ()) {
    runtime->run_in_main (sigc::bind (toggle_fullscreen.make_slot (), 0));
  }
  if (rxvWindow ) {
    rxvWindow->ProcessEvents();
    rxvWindow->PutFrame ((uint8_t *) rframe, rwidth, rheight);
    rxvWindow->Sync();
  }

  if (lxvWindow) {
    lxvWindow->ProcessEvents();
    lxvWindow->PutFrame ((uint8_t *) lframe, lwidth, lheight);
    lxvWindow->Sync();
  }
}
