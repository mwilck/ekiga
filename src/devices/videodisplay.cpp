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
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         videodisplay.cpp  -  description
 *                         ------------------------------
 *   begin                : Sat Feb 17 2001
 *   copyright            : (C) 2000-2007 by Damien Sandras
 *                        : (C)      2007 by Matthias Schneider
 *   description          : GMVideoDisplay: Generic class that represents 
 *                            a thread that can display a video image.
 *                          GMVideoDisplay_embedded: Class that supports generic
 *                            functions for local/remote/pip/pip external 
 *                            window/fullscreen video display. Provides interface 
 *                            to the GUI for an embedded window, display mode 
 *                            control and feedback of information like the status
 *                            of the video acceleration. Also provides the 
 *                            interface to the PVideoOutputDevice_EKIGA and 
 *                            implements copying and local storage of the video 
 *                            frame.
 */


#include "config.h"

#define P_FORCE_STATIC_PLUGIN 

#include "videodisplay.h"
#include "ekiga.h"
#include "misc.h"
#include "main.h"

GMVideoDisplay::GMVideoDisplay ()
  : PThread (1000, NoAutoDeleteThread, HighPriority, "GMVideoDisplay") // to be verified
{
}

GMVideoDisplay::~GMVideoDisplay ()
{
}

/* The functions */
GMVideoDisplay_embedded::GMVideoDisplay_embedded (Ekiga::ServiceCore & _core)
: core (_core)
{
  /* Variables */

  /* State for last frame */
  lastFrame.display = UNSET;
  lastFrame.localWidth = 0;
  lastFrame.localHeight = 0;
  lastFrame.remoteWidth = 0;
  lastFrame.remoteHeight = 0;  
  lastFrame.zoom = 0;
  lastFrame.embeddedX = 0;
  lastFrame.embeddedY = 0;  

  currentFrame.localWidth = 0;
  currentFrame.localHeight = 0;
  currentFrame.remoteWidth = 0;
  currentFrame.remoteHeight = 0;

  /* Initialisation */
  stop = FALSE;
  video_disabled = FALSE;
  first_frame_received = FALSE;
  update_required.local = FALSE;
  update_required.remote = FALSE;

  runtime = dynamic_cast<Ekiga::Runtime *> (core.get ("runtime")); 
  
  sigc::connection conn;
  conn = set_display_type.connect (sigc::ptr_fun (gm_main_window_set_display_type));
  connections.push_back (conn);

  conn = fullscreen_menu_update_sensitivity.connect (sigc::ptr_fun (gm_main_window_fullscreen_menu_update_sensitivity));
  connections.push_back (conn);

  conn = toggle_fullscreen.connect (sigc::ptr_fun (gm_main_window_toggle_fullscreen));
  connections.push_back (conn);

  conn = update_logo.connect (sigc::ptr_fun (gm_main_window_update_logo));
  connections.push_back (conn);

  conn = force_redraw.connect (sigc::ptr_fun (gm_main_window_force_redraw));
  connections.push_back (conn);

  conn = set_resized_video_widget.connect (sigc::ptr_fun (gm_main_window_set_resized_video_widget));
  connections.push_back (conn);

  conn = update_zoom_display.connect (sigc::ptr_fun (gm_main_window_update_zoom_display));
  connections.push_back (conn);

  conn = update_video_accel_status.connect (sigc::ptr_fun (gm_main_window_update_video_accel_status));
  connections.push_back (conn);

  conn = GnomeMeeting::Process ()->set_video_info.connect (sigc::mem_fun (this, &GMVideoDisplay_embedded::SetVideoInfo));
    connections.push_back (conn);

  this->Resume ();
  thread_sync_point.Wait ();
}

GMVideoDisplay_embedded::~GMVideoDisplay_embedded ()
{
  /* This is common to all output classes */
  lframeStore.SetSize (0);
  rframeStore.SetSize (0);

  for (std::vector<sigc::connection>::iterator iter = connections.begin ();
       iter != connections.end ();
       iter++)
     iter->disconnect ();
}

void
GMVideoDisplay_embedded::Main ()
{
  bool do_sync;
  UpdateRequired sync_required;

  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();

  while (!stop) {
    frame_available_sync_point.Wait(250);
    var_mutex.Wait ();
      do_sync = first_frame_received;
      if (first_frame_received)
        sync_required = Redraw();
    var_mutex.Signal ();
    if (do_sync)
      Sync(sync_required);
  }

  var_mutex.Wait ();
  CloseFrameDisplay ();
  var_mutex.Signal ();
}

void GMVideoDisplay_embedded::SetFrameData (unsigned width,
				       unsigned height,
				       const BYTE * data,
				       bool local,
				       int devices_nbr
)
{ 
  VideoInfo localVideoInfo;

  GetVideoInfo(&localVideoInfo);


  var_mutex.Wait();

  /* If there is only one device open, ignore the setting, and 
   * display what we can actually display.
   */
  if (devices_nbr <= 1) {

    if (!local)
      localVideoInfo.display = REMOTE_VIDEO;
    else 
      localVideoInfo.display = LOCAL_VIDEO;

    runtime->run_in_main (sigc::bind (set_display_type.make_slot (), localVideoInfo.display));
  }

  currentFrame.display = localVideoInfo.display;
  currentFrame.zoom = localVideoInfo.zoom; 
  first_frame_received = TRUE;

  if (local) {

    /* memcpy the frame */
    lframeStore.SetSize (width * height * 3);
    currentFrame.localWidth = width;
    currentFrame.localHeight= height;

    memcpy (lframeStore.GetPointer(), data, (width * height * 3) >> 1);
    if (update_required.local) 
      PTRACE(3, "Skipped earlier local frame");
    update_required.local = TRUE;
  }
  else {

    /* memcpy the frame */
    rframeStore.SetSize (width * height * 3);
    currentFrame.remoteWidth = width;
    currentFrame.remoteHeight= height;

    memcpy (rframeStore.GetPointer(), data, (width * height * 3) >> 1);
    if (update_required.remote) 
      PTRACE(3, "Skipped earlier remote frame");
    update_required.remote = TRUE;
  }
  var_mutex.Signal();

  if ((localVideoInfo.display == UNSET) || (localVideoInfo.zoom == 0) || (!localVideoInfo.gconfInfoSet)) {
    runtime->run_in_main (update_zoom_display.make_slot ());
    PTRACE(4, "GMVideoDisplay_embedded\tDisplay and zoom variable not set yet, not opening display");
     return;
  }

  if ((localVideoInfo.display == LOCAL_VIDEO) && !local)
      return;

  if ((localVideoInfo.display == REMOTE_VIDEO) && local)
      return;

  frame_available_sync_point.Signal();
}

bool 
GMVideoDisplay_embedded::FrameDisplayChangeNeeded (VideoMode display, 
                                              guint lf_width, 
                                              guint lf_height, 
                                              guint rf_width, 
                                              guint rf_height, 
                                              unsigned int zoom)
{
  VideoInfo localVideoInfo;

  GetVideoInfo(&localVideoInfo);

  if ((!localVideoInfo.widgetInfoSet) || (!localVideoInfo.gconfInfoSet) ||
      (localVideoInfo.display == UNSET) || (localVideoInfo.zoom == 0)) {
    PTRACE(4, "GMVideoDisplay_X\tWidget not yet realized or gconf info not yet set, not opening display");
    return FALSE;
  }
  switch (display) {
  case LOCAL_VIDEO:
    return (lastFrame.display != LOCAL_VIDEO 
            || lastFrame.zoom != zoom || lastFrame.localWidth != lf_width || lastFrame.localHeight != lf_height 
            || localVideoInfo.x != lastFrame.embeddedX || localVideoInfo.y != lastFrame.embeddedY);
    break;

  case REMOTE_VIDEO:
    return (lastFrame.display != REMOTE_VIDEO
            || lastFrame.zoom != zoom || lastFrame.remoteWidth != rf_width || lastFrame.remoteHeight != rf_height
            || localVideoInfo.x != lastFrame.embeddedX || localVideoInfo.y != lastFrame.embeddedY);
    break;

  case PIP:
    return (lastFrame.display != display || lastFrame.zoom != zoom 
            || lastFrame.remoteWidth != rf_width || lastFrame.remoteHeight != rf_height
            || lastFrame.localWidth != lf_width || lastFrame.localHeight != lf_height
            || localVideoInfo.x != lastFrame.embeddedX || localVideoInfo.y != lastFrame.embeddedY);
    break;
  case PIP_WINDOW:
  case FULLSCREEN:
    return (lastFrame.display != display || lastFrame.zoom != zoom 
            || lastFrame.remoteWidth != rf_width || lastFrame.remoteHeight != rf_height
            || lastFrame.localWidth != lf_width || lastFrame.localHeight != lf_height);
    break;
  case UNSET:
  default:
    break;
  }
  return FALSE;
}

UpdateRequired
GMVideoDisplay_embedded::Redraw ()
{
  UpdateRequired sync_required;
  sync_required = update_required;
  
    if (FrameDisplayChangeNeeded (currentFrame.display, currentFrame.localWidth, currentFrame.localHeight, 
                                  currentFrame.remoteWidth, currentFrame.remoteHeight, currentFrame.zoom)) 
      SetupFrameDisplay (currentFrame.display, currentFrame.localWidth, currentFrame.localHeight, 
                         currentFrame.remoteWidth, currentFrame.remoteHeight, currentFrame.zoom); 

    switch (currentFrame.display) 
      {
      case LOCAL_VIDEO:
          if (lframeStore.GetSize() > 0)
            DisplayFrame (lframeStore.GetPointer (), currentFrame.localWidth, currentFrame.localHeight);
        break;

      case REMOTE_VIDEO:
          if (rframeStore.GetSize() > 0)
            DisplayFrame (rframeStore.GetPointer (), currentFrame.remoteWidth, currentFrame.remoteHeight);
        break;

     case FULLSCREEN:
     case PIP:
     case PIP_WINDOW:
          if ((lframeStore.GetSize() > 0) &&  (rframeStore.GetSize() > 0))
            DisplayPiPFrames (lframeStore.GetPointer (), currentFrame.localWidth, currentFrame.localHeight,
                              rframeStore.GetPointer (), currentFrame.remoteWidth, currentFrame.remoteHeight);
       break;
    case UNSET:
    default:
       break;
    }

  update_required.local = FALSE;
  update_required.remote = FALSE;

  return sync_required;
}

void GMVideoDisplay_embedded::SetVideoInfo (VideoInfo* newVideoInfo) 
{
  PWaitAndSignal m(video_info_mutex);
  videoInfo = *newVideoInfo;
}

void GMVideoDisplay_embedded::GetVideoInfo (VideoInfo* getVideoInfo) 
{
  PWaitAndSignal m(video_info_mutex);
  *getVideoInfo = videoInfo;
}


