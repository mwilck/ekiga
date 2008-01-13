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
 *                         display-manager-common.cpp  -  description
 *                         ------------------------------
 *   begin                : Sat Feb 17 2001
 *   copyright            : (C) 2000-2008 by Damien Sandras
 *                        : (C) 2007-2008 by Matthias Schneider
 *   description          : GMVideoManager: Generic class that represents 
 *                            a thread that can display a video image and defines.
 *                            generic functions for local/remote/pip/pip external 
 *                            window/fullscreen video display. Provides interface 
 *                            to the GUI for an embedded window, display mode 
 *                            control and feedback of information like the status
 *                            of the video acceleration. Also provides the 
 *                            copying and local storage of the video frame.
 *
 */


#include "display-manager-common.h"

/* The functions */
GMDisplayManager::GMDisplayManager(Ekiga::ServiceCore & _core)
  : PThread (1000, NoAutoDeleteThread, HighPriority, "GMDisplayManager"),
    core (_core), runtime (*(dynamic_cast<Ekiga::Runtime *> (_core.get ("runtime"))))
{
}

GMDisplayManager::~GMDisplayManager ()
{
}

void GMDisplayManager::start ()
{
  Ekiga::DisplayManager::start();

  /* State for last frame */
  last_frame.display = UNSET_new;
  last_frame.local_width = 0;
  last_frame.local_height = 0;
  last_frame.remote_width = 0;
  last_frame.remote_height = 0;  
  last_frame.zoom = 0;
  last_frame.embedded_x = 0;
  last_frame.embedded_y = 0;  

  current_frame.local_width = 0;
  current_frame.local_height = 0;
  current_frame.remote_width = 0;
  current_frame.remote_height = 0;

  /* Initialisation */
  end_thread = false;
  video_disabled = false;
  first_frame_received = false;
  update_required.local = false;
  update_required.remote = false;

  rx_frames = 0;
  tx_frames = 0;

  last_stats = PTime ();

//  this->Resume ();
  this->Restart ();
  thread_sync_point.Wait ();
printf ("Start\n");
}

void GMDisplayManager::stop () {
  end_thread = true;
  /* Wait for the Main () method to be terminated */
  frame_available_sync_point.Signal();
  PWaitAndSignal m(quit_mutex);

  /* This is common to all output classes */
  lframeStore.SetSize (0);
  rframeStore.SetSize (0);

  Ekiga::DisplayManager::stop();
printf ("Stop\n");
}

void
GMDisplayManager::Main ()
{
  bool do_sync;
  UpdateRequired sync_required;

  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();

  while (!end_thread) {
    frame_available_sync_point.Wait(250);
printf ("Thread\n");
    var_mutex.Wait ();
      do_sync = first_frame_received;
      if (first_frame_received)
        sync_required = redraw();
    var_mutex.Signal ();
    if (do_sync)
      sync(sync_required);
  }

  var_mutex.Wait ();
  close_frame_display ();
  var_mutex.Signal ();
}

void GMDisplayManager::set_frame_data (unsigned width,
				       unsigned height,
				       const char* data,
				       bool local,
				       int devices_nbr
)
{ 
  DisplayInfo local_display_info;

  get_display_info(local_display_info);

printf ("SetFrameData\n");

  var_mutex.Wait();

  /* If there is only one device open, ignore the setting, and 
   * display what we can actually display.
   */
  if (devices_nbr <= 1) {

    if (!local)
      local_display_info.display = REMOTE_VIDEO_new;
    else 
      local_display_info.display = LOCAL_VIDEO_new;

    runtime.run_in_main (sigc::bind (display_type_changed.make_slot (), local_display_info.display));
  }

  current_frame.display = local_display_info.display;
  current_frame.zoom = local_display_info.zoom; 
  first_frame_received = true;

  if (local) {

    /* memcpy the frame */
    lframeStore.SetSize (width * height * 3);
    current_frame.local_width = width;
    current_frame.local_height= height;

    tx_frames++;
    
    if ((width != display_stats.tx_width) || (height != display_stats.tx_height)) {
      display_stats.tx_width = width;
      display_stats.tx_height = height;
      //FIXME
    }

    memcpy (lframeStore.GetPointer(), data, (width * height * 3) >> 1);
    if (update_required.local) 
      PTRACE(3, "Skipped earlier local frame");
    update_required.local = true;
  }
  else {

    /* memcpy the frame */
    rframeStore.SetSize (width * height * 3);
    current_frame.remote_width = width;
    current_frame.remote_height= height;
    
    rx_frames++;

    if ((width != display_stats.rx_width) || (height != display_stats.rx_height)) {
      display_stats.rx_width = width;
      display_stats.rx_height = height;
      //FIXME
    }

    memcpy (rframeStore.GetPointer(), data, (width * height * 3) >> 1);
    if (update_required.remote) 
      PTRACE(3, "Skipped earlier remote frame");
    update_required.remote = true;
  }

  PTimeInterval t = PTime () - last_stats;
  if (t.GetMilliSeconds() > 2000) {
    display_stats.tx_fps = round ((rx_frames * 1000) / t.GetMilliSeconds());
    display_stats.rx_fps = round ((tx_frames * 1000) / t.GetMilliSeconds());
    rx_frames = 0;
    tx_frames = 0;
    last_stats = PTime();
    //FIXME
  }

  var_mutex.Signal();

  if ((local_display_info.display == UNSET_new) || (local_display_info.zoom == 0) || (!local_display_info.gconfInfoSet)) {
    runtime.run_in_main (display_info_update_required.make_slot ());
    PTRACE(4, "GMVideoDisplay_embedded\tDisplay and zoom variable not set yet, not opening display");
     return;
  }

  if ((local_display_info.display == LOCAL_VIDEO_new) && !local)
      return;

  if ((local_display_info.display == REMOTE_VIDEO_new) && local)
      return;
printf ("Signaling\n");
  frame_available_sync_point.Signal();
}

bool 
GMDisplayManager::frame_display_change_needed (DisplayMode display, 
                                              unsigned lf_width, 
                                              unsigned lf_height, 
                                              unsigned rf_width, 
                                              unsigned rf_height, 
                                              unsigned int zoom)
{
  DisplayInfo local_display_info;

  get_display_info(local_display_info);

  if ((!local_display_info.widgetInfoSet) || (!local_display_info.gconfInfoSet) ||
      (local_display_info.display == UNSET_new) || (local_display_info.zoom == 0)) {
    PTRACE(4, "GMVideoDisplay_X\tWidget not yet realized or gconf info not yet set, not opening display");
    return false;
  }
  switch (display) {
  case LOCAL_VIDEO_new:
    return (last_frame.display != LOCAL_VIDEO_new 
            || last_frame.zoom != zoom || last_frame.local_width != lf_width || last_frame.local_height != lf_height 
            || local_display_info.x != last_frame.embedded_x || local_display_info.y != last_frame.embedded_y);
    break;

  case REMOTE_VIDEO_new:
    return (last_frame.display != REMOTE_VIDEO_new
            || last_frame.zoom != zoom || last_frame.remote_width != rf_width || last_frame.remote_height != rf_height
            || local_display_info.x != last_frame.embedded_x || local_display_info.y != last_frame.embedded_y);
    break;

  case PIP_new:
    return (last_frame.display != display || last_frame.zoom != zoom 
            || last_frame.remote_width != rf_width || last_frame.remote_height != rf_height
            || last_frame.local_width != lf_width || last_frame.local_height != lf_height
            || local_display_info.x != last_frame.embedded_x || local_display_info.y != last_frame.embedded_y);
    break;
  case PIP_WINDOW_new:
  case FULLSCREEN_new:
    return (last_frame.display != display || last_frame.zoom != zoom 
            || last_frame.remote_width != rf_width || last_frame.remote_height != rf_height
            || last_frame.local_width != lf_width || last_frame.local_height != lf_height);
    break;
  case UNSET_new:
  default:
    break;
  }
  return false;
}

UpdateRequired
GMDisplayManager::redraw ()
{
  UpdateRequired sync_required;
  sync_required = update_required;
  
    if (frame_display_change_needed (current_frame.display, current_frame.local_width, current_frame.local_height, 
                                  current_frame.remote_width, current_frame.remote_height, current_frame.zoom)) 
      setup_frame_display (current_frame.display, current_frame.local_width, current_frame.local_height, 
                         current_frame.remote_width, current_frame.remote_height, current_frame.zoom); 

    switch (current_frame.display) 
      {
      case LOCAL_VIDEO_new:
          if (lframeStore.GetSize() > 0)
            display_frame ((char*)lframeStore.GetPointer (), current_frame.local_width, current_frame.local_height);
        break;

      case REMOTE_VIDEO_new:
          if (rframeStore.GetSize() > 0)
            display_frame ((char*)rframeStore.GetPointer (), current_frame.remote_width, current_frame.remote_height);
        break;

     case FULLSCREEN_new:
     case PIP_new:
     case PIP_WINDOW_new:
          if ((lframeStore.GetSize() > 0) &&  (rframeStore.GetSize() > 0))
            display_pip_frames ((char*)lframeStore.GetPointer (), current_frame.local_width, current_frame.local_height,
                              (char*)rframeStore.GetPointer (), current_frame.remote_width, current_frame.remote_height);
       break;
    case UNSET_new:
    default:
       break;
    }

  update_required.local = false;
  update_required.remote = false;

  return sync_required;
}
