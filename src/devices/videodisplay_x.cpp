  
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
 *                         videodisplay_x.cpp -  description
 *                         ----------------------------------
 *   begin                : Sun Nov 19 2006
 *   copyright            : (C) 2006-2007 by Matthias Schneider
 *                          (C) 2000-2007 by Damien Sandras
 *   description          : Class to allow video output to a X/XVideo
 *                          accelerated window
 *
 */



#include "config.h"

#define P_FORCE_STATIC_PLUGIN 

#include "videodisplay_x.h"
#include "ekiga.h"
#include "misc.h"
#include "main.h"

#include <gdk/gdkx.h>

#ifdef HAVE_XV
#include "xvwindow.h"
#endif

/* The Methods */
GMVideoDisplay_X::GMVideoDisplay_X (Ekiga::ServiceCore & _core)
: GMVideoDisplay_embedded (_core)
{ 
  /* Internal stuff */
  lxWindow = NULL;
  rxWindow = NULL;

  rDisplay = XOpenDisplay (NULL);
  lDisplay = XOpenDisplay (NULL);
  embGC = NULL;

  pipWindowAvailable = TRUE;
}


GMVideoDisplay_X::~GMVideoDisplay_X()
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

void 
GMVideoDisplay_X::SetupFrameDisplay (VideoMode display, 
                                     guint lf_width, 
                                     guint lf_height, 
                                     guint rf_width, 
                                     guint rf_height, 
                                     unsigned int zoom)
{
  VideoInfo localVideoInfo;
  VideoAccelStatus status = NONE;

  if (video_disabled)
    return;

  GetVideoInfo(&localVideoInfo);

  switch (display) {
  case LOCAL_VIDEO:
    runtime->run_in_main (sigc::bind (set_resized_video_widget.make_slot (), (int) (lf_width * zoom / 100), (int) (lf_height * zoom / 100)));
    break;
  case REMOTE_VIDEO:
  case PIP:
    runtime->run_in_main (sigc::bind (set_resized_video_widget.make_slot (), (int) (rf_width * zoom / 100), (int) (rf_height * zoom / 100)));
    break;
  case FULLSCREEN:
    runtime->run_in_main (sigc::bind (set_resized_video_widget.make_slot (), (int) (GM_QCIF_WIDTH), (int) (GM_QCIF_HEIGHT)));
    break;
  case PIP_WINDOW:
    runtime->run_in_main (sigc::bind (set_resized_video_widget.make_slot (), (int) (GM_QCIF_WIDTH), (int) (GM_QCIF_HEIGHT)));
    break;
  case UNSET:
  default:
    PTRACE (1, "GMVideoDisplay_X\tDisplay variable not set");
    return;
    break;
  }

  if ((!localVideoInfo.widgetInfoSet) || (!localVideoInfo.gconfInfoSet) ||
      (localVideoInfo.display == UNSET) || (localVideoInfo.zoom == 0) || (zoom == 0)) {
    PTRACE(4, "GMVideoDisplay_X\tWidget not yet realized or gconf info not yet set, not opening display");
    return;
  }

  CloseFrameDisplay ();

  runtime->run_in_main (sigc::bind (set_display_type.make_slot (), display));

  pipWindowAvailable = FALSE;

  switch (display) {
// LOCAL_VIDEO ------------------------------------------------------------------
  case LOCAL_VIDEO:
    PTRACE(4, "GMVideoDisplay_X\tOpening LOCAL_VIDEO display with image of " << lf_width << "x" << lf_height);
#ifdef HAVE_XV
    if (!localVideoInfo.disableHwAccel) {
      lxWindow = new XVWindow ();
      if (lxWindow->Init (localVideoInfo.xdisplay, 
                            localVideoInfo.window, 
                            localVideoInfo.gc,
                            localVideoInfo.x,
                            localVideoInfo.y,
                            (int) (lf_width * zoom / 100), 
                            (int) (lf_height * zoom / 100),
                            lf_width, 
                            lf_height)) {
	status = ALL;
        PTRACE(4, "GMVideoDisplay_X\tLOCAL_VIDEO: Successfully opened XV Window");
      }
      else {
	delete lxWindow;
	lxWindow = NULL;
	status = NONE;
        PTRACE(4, "GMVideoDisplay_X\tLOCAL_VIDEO: Could not open XV Window");
      }
    }
#endif			   
    if (status==NONE) {
      PTRACE(3, "GMVideoDisplay_X\tFalling back to SW" << ((localVideoInfo.disableHwAccel) 
                                      ? " since HW acceleration was deactivated by configuration" 
                                      : " since HW acceleration failed to initalize"));
      lxWindow = new XWindow ();
      if (lxWindow->Init (localVideoInfo.xdisplay, 
                            localVideoInfo.window, 
                            localVideoInfo.gc,
                            localVideoInfo.x,
                            localVideoInfo.y,
                           (int) (lf_width * zoom / 100), 
                           (int) (lf_height * zoom / 100),
                           lf_width, 
                           lf_height)) {
       lxWindow->SetSwScalingAlgo(localVideoInfo.swScalingAlgorithm);
       PTRACE(4, "GMVideoDisplay_X\tLOCAL_VIDEO: Successfully opened X Window");
      }
      else {
        delete lxWindow;
        lxWindow = NULL;
        video_disabled = TRUE;
        status = NO_VIDEO;
        PTRACE(1, "GMVideoDisplay_X\tLOCAL_VIDEO: Could not open X Window - no video");
      }
    }
    
    lastFrame.embeddedX = localVideoInfo.x;
    lastFrame.embeddedY = localVideoInfo.y;

    lastFrame.display = LOCAL_VIDEO;
    lastFrame.localWidth = lf_width;
    lastFrame.localHeight = lf_height;
    lastFrame.zoom = zoom;
    break;

// REMOTE_VIDEO ----------------------------------------------------------------
  case REMOTE_VIDEO:
    PTRACE(4, "GMVideoDisplay_X\tOpening REMOTE_VIDEO display with image of " << rf_width << "x" << rf_height);
#ifdef HAVE_XV
    if (!localVideoInfo.disableHwAccel) {
      rxWindow = new XVWindow ();
      if (rxWindow->Init (localVideoInfo.xdisplay, 
                          localVideoInfo.window, 
                          localVideoInfo.gc,
                          localVideoInfo.x,
                          localVideoInfo.y,
                          (int) (rf_width * zoom / 100), 
                          (int) (rf_height * zoom / 100),
                          rf_width, 
                          rf_height)) {
       status = ALL;
       PTRACE(4, "GMVideoDisplay_X\tREMOTE_VIDEO: Successfully opened XV Window");
     }
     else {
       delete rxWindow;
       rxWindow = NULL;
       status = NONE;
       PTRACE(1, "GMVideoDisplay_X\tLOCAL_VIDEO: Could not open XV Window");

     }
    }
#endif			   
    if (status==NONE) {
      PTRACE(3, "GMVideoDisplay_X\tFalling back to SW" << ((localVideoInfo.disableHwAccel) 
                                      ? " since HW acceleration was deactivated by configuration" 
                                      : " since HW acceleration failed to initalize"));
      rxWindow = new XWindow ();
      if ( rxWindow->Init (localVideoInfo.xdisplay, 
                             localVideoInfo.window, 
                             localVideoInfo.gc,
                             localVideoInfo.x,
                             localVideoInfo.y,
                             (int) (rf_width * zoom / 100), 
                             (int) (rf_height * zoom / 100),
                             rf_width, 
                             rf_height)) {
        rxWindow->SetSwScalingAlgo(localVideoInfo.swScalingAlgorithm);
        PTRACE(4, "GMVideoDisplay_X\tREMOTE_VIDEO: Successfully opened X Window");
      }
      else {
        delete rxWindow;
        rxWindow = NULL;
        video_disabled = TRUE;
        status = NO_VIDEO;
        PTRACE(1, "GMVideoDisplay_X\tREMOTE_VIDEO: Could not open X Window - no video");
      }
    }

    lastFrame.embeddedX = localVideoInfo.x;
    lastFrame.embeddedY = localVideoInfo.y;

    lastFrame.display = REMOTE_VIDEO;
    lastFrame.remoteWidth = rf_width;
    lastFrame.remoteHeight = rf_height;
    lastFrame.zoom = zoom;
    break;

// PIP_VIDEO ------------------------------------------------------------------
  case FULLSCREEN:
  case PIP:
  case PIP_WINDOW:
    PTRACE(4, "GMVideoDisplay_X\tOpening display " << display << " with images of " 
            << lf_width << "x" << lf_height << "(local) and " 
	    << rf_width << "x" << rf_height << "(remote)");
#ifdef HAVE_XV
    if (!localVideoInfo.disableHwAccel) {
      rxWindow = new XVWindow ();
      if (rxWindow->Init ((display == PIP) ? localVideoInfo.xdisplay : rDisplay, 
                             (display == PIP) ? localVideoInfo.window : DefaultRootWindow (rDisplay), 
                             (display == PIP) ? localVideoInfo.gc : NULL,
                             (display == PIP) ? localVideoInfo.x : 0,
                             (display == PIP) ? localVideoInfo.y : 0,
                             (int) (rf_width * zoom / 100), 
                             (int) (rf_height * zoom / 100),
                             rf_width, 
                             rf_height)) {
        status = REMOTE_ONLY;
        PTRACE(4, "GMVideoDisplay_X\tPIP: Successfully opened remote XV Window");
      }
      else 
      {
        delete rxWindow;
	rxWindow = NULL;
	status = NONE;
        PTRACE(1, "GMVideoDisplay_X\tPIP: Could not open remote XV Window");
      }
    }
#endif			   
    if (status == NONE) {
      PTRACE(3, "GMVideoDisplay_X\tFalling back to SW" << ((localVideoInfo.disableHwAccel) 
                                      ? " since HW acceleration was deactivated by configuration" 
                                      : " since HW acceleration failed to initalize"));
      rxWindow = new XWindow ();
      if (rxWindow->Init ((display == PIP) ? localVideoInfo.xdisplay : rDisplay, 
                             (display == PIP) ? localVideoInfo.window : DefaultRootWindow (rDisplay), 
                             (display == PIP) ? localVideoInfo.gc : NULL,
                             (display == PIP) ? localVideoInfo.x : 0,
                             (display == PIP) ? localVideoInfo.y : 0,
                             (int) (rf_width * zoom / 100), 
                             (int) (rf_height * zoom / 100),
                             rf_width, 
                             rf_height)) {
        rxWindow->SetSwScalingAlgo(localVideoInfo.swScalingAlgorithm);
        PTRACE(4, "GMVideoDisplay_X\tPIP: Successfully opened remote X Window");
      }
      else {
        delete rxWindow;
        rxWindow = NULL;
        video_disabled = TRUE;
        status = NO_VIDEO;
        PTRACE(1, "GMVideoDisplay_X\tPIP: Could not open remote X Window - no video");
      }
    }


#ifdef HAVE_XV
    if (status == REMOTE_ONLY) {
      lxWindow = new XVWindow();
      if (lxWindow->Init (   rxWindow->GetDisplay (), 
                             rxWindow->GetWindowHandle (),
                             rxWindow->GetGC (),
                             (int) (rf_width * zoom  / 100 * 2 / 3), 
                             (int) (rf_height * zoom  / 100 * 2 / 3), 
                             (int) (rf_width * zoom  / 100 / 3), 
                             (int) (rf_height * zoom  / 100 / 3),
                             lf_width, 
                             lf_height)) {
        status = ALL;
        pipWindowAvailable = TRUE;
        PTRACE(4, "GMVideoDisplay_X\tPIP: Successfully opened local XV Window");
      }
      else {
        delete lxWindow;
	lxWindow = NULL;
        pipWindowAvailable = FALSE;
        PTRACE(1, "GMVideoDisplay_X\tPIP: Could not open local XV Window");
      }
    }
#endif
    if ((status != ALL) && (localVideoInfo.allowPipSwScaling)) {
      PTRACE(3, "GMVideoDisplay_X\tFalling back to SW" << ((localVideoInfo.disableHwAccel) 
                                      ? " since HW acceleration was deactivated by configuration" 
                                      : " since HW acceleration failed to initalize"));
      lxWindow = new XWindow ();
      if (lxWindow->Init ((display == PIP) ? localVideoInfo.xdisplay : lDisplay, 
                             rxWindow->GetWindowHandle (),
                             (display == PIP) ? localVideoInfo.gc : NULL,
                             (int) (rf_width * zoom  / 100 * 2 / 3), 
                             (int) (rf_height * zoom  / 100 * 2 / 3), 
                             (int) (rf_width * zoom  / 100 / 3), 
                             (int) (rf_height * zoom  / 100 / 3),
                             lf_width, 
                             lf_height)) {
       lxWindow->SetSwScalingAlgo(localVideoInfo.swScalingAlgorithm);
       pipWindowAvailable = TRUE;
       PTRACE(4, "GMVideoDisplay_X\tPIP: Successfully opened local X Window");
     }
     else {
       delete lxWindow;
       lxWindow = NULL;
       pipWindowAvailable = FALSE;
       PTRACE(4, "GMVideoDisplay_X\tPIP: Could not open local X Window - picture-in-picture disabled");
      }
    }

    if ((status != ALL) && (!localVideoInfo.allowPipSwScaling)) {
      PTRACE(3, "GMVideoDisplay_X\tNot opening PIP window since HW acceleration is not available and SW fallback is disabled by configuration");
      status = ALL;
    }

    if (rxWindow && lxWindow) {

      rxWindow->RegisterSlave (lxWindow);
      lxWindow->RegisterMaster (rxWindow);
    }	  

    if (rxWindow && display == FULLSCREEN) 
      rxWindow->ToggleFullscreen ();
    
    if ((display != PIP_WINDOW) && (display != FULLSCREEN)) {
      lastFrame.embeddedX = localVideoInfo.x;
      lastFrame.embeddedY = localVideoInfo.y;
    }

    lastFrame.display = display;
    lastFrame.localWidth = lf_width;
    lastFrame.localHeight = lf_height;
    lastFrame.remoteWidth = rf_width;
    lastFrame.remoteHeight = rf_height;
    lastFrame.zoom = zoom;
    break;

  case UNSET:
  default:
    return;
    break;
  }

  if (localVideoInfo.onTop) {

    if (lxWindow)
      lxWindow->ToggleOntop ();
    if (rxWindow)
      rxWindow->ToggleOntop ();
  }

  runtime->run_in_main (sigc::bind (update_video_accel_status.make_slot (), status));
}


bool 
GMVideoDisplay_X::CloseFrameDisplay ()
{
  if (runtime) //FIXME
    runtime->run_in_main (sigc::bind (update_video_accel_status.make_slot (), NO_VIDEO));

  if (rxWindow) 
    rxWindow->RegisterSlave (NULL);
  if (lxWindow) 
    lxWindow->RegisterMaster (NULL);

  if (rxWindow) {
    delete rxWindow;
    rxWindow = NULL;
  }

  if (lxWindow) {
    delete lxWindow;
    lxWindow = NULL;
  }

  return TRUE;
}

bool 
GMVideoDisplay_X::FrameDisplayChangeNeeded (VideoMode display, 
                                                  guint lf_width, 
                                                  guint lf_height, 
                                                  guint rf_width, 
                                                  guint rf_height, 
                                                  unsigned int zoom)
{
    switch (display) 
    {
      case LOCAL_VIDEO:
          if (!lxWindow) 
            return TRUE;
          break;
      case REMOTE_VIDEO:
          if (!rxWindow) 
            return TRUE;
          break;
      case FULLSCREEN:
      case PIP:
      case PIP_WINDOW:
          if (!rxWindow || (pipWindowAvailable && (!lxWindow)) )
              return TRUE;
          break;
     case UNSET:
     default:
          break;
    }
  return GMVideoDisplay_embedded::FrameDisplayChangeNeeded (display, lf_width, lf_height, rf_width, rf_height, zoom);
}

void 
GMVideoDisplay_X::DisplayFrame (     const guchar *frame,
                                      guint width,
                                      guint height)
{
  if (rxWindow)
    rxWindow->ProcessEvents();

  if (lxWindow)
    lxWindow->ProcessEvents();

  if  ((currentFrame.display == LOCAL_VIDEO) && (lxWindow))
    lxWindow->PutFrame ((uint8_t *) frame, width, height);

  if  ((currentFrame.display == REMOTE_VIDEO) && (rxWindow))
    rxWindow->PutFrame ((uint8_t *) frame, width, height);
}

void 
GMVideoDisplay_X::DisplayPiPFrames (     const guchar *lframe,
                                          guint lwidth,
                                          guint lheight,
                                          const guchar *rframe,
                                          guint rwidth,
                                          guint rheight)
{
  if (rxWindow)
    rxWindow->ProcessEvents();

  if (lxWindow)
    lxWindow->ProcessEvents();

  if (currentFrame.display == FULLSCREEN && rxWindow && !rxWindow->IsFullScreen ())
    runtime->run_in_main (sigc::bind (toggle_fullscreen.make_slot (), OFF));

  if (rxWindow && (update_required.remote || (!update_required.remote && !update_required.local)))
    rxWindow->PutFrame ((uint8_t *) rframe, rwidth, rheight);

  if (lxWindow && (update_required.local  || (!update_required.remote && !update_required.local)))
    lxWindow->PutFrame ((uint8_t *) lframe, lwidth, lheight);

}

void 
GMVideoDisplay_X::Sync(UpdateRequired sync_required)
{

  if (rxWindow && (sync_required.remote || (!sync_required.remote && !sync_required.local))) {
    rxWindow->Sync();
  }

  if (lxWindow && (sync_required.local  || (!sync_required.remote && !sync_required.local))) {
    lxWindow->Sync();
  }
}

