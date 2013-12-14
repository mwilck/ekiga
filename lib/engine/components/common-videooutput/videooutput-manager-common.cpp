/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
 * Copyright (C) 2012, Xunta de Galicia <ocfloss@xunta.es>
 *
 * Authors: Matthias Schneider
 *          Victor Jaquez, Igalia S.L., AGASOL. <vjaquez@igalia.com>
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


#include "videooutput-manager-common.h"
#include "videoinput-info.h"

#include "runtime.h"

/* The functions */
GMVideoOutputManager::GMVideoOutputManager(Ekiga::ServiceCore & _core)
  : PThread (1000, AutoDeleteThread, HighestPriority, "GMVideoOutputManager"),
    core (_core)
{
}

GMVideoOutputManager::~GMVideoOutputManager ()
{
  PWaitAndSignal m(thread_ended);
}

void GMVideoOutputManager::open ()
{
  init_thread = true;
  thread_initialised.Wait();
}

void GMVideoOutputManager::close ()
{
  ready = false;
  uninit_thread = true;
  thread_uninitialised.Wait ();
}

void
GMVideoOutputManager::Main ()
{
  bool do_sync = false;
  bool initialised_thread = false;
  UpdateRequired sync_required;

  PWaitAndSignal m(thread_ended);
  thread_created.Signal ();

  while (!end_thread) {

    if (init_thread) {
      init();
      init_thread = false;
      initialised_thread = true;
      ready = true;
      thread_initialised.Signal();
    }
    else if (uninit_thread) {
      var_mutex.Wait ();
      close_frame_display ();
      uninit();
      uninit_thread = false;
      initialised_thread = false;
      lframeStore.SetSize (0);
      var_mutex.Signal ();
      thread_uninitialised.Signal();
    }
    else if (initialised_thread) {
      var_mutex.Wait ();
        do_sync = local_frame_received || remote_frame_received || ext_frame_received;
        if (do_sync)
          sync_required = redraw();
        local_frame_received = false;
        remote_frame_received = false;
        ext_frame_received = false;
      var_mutex.Signal ();
      if (do_sync)
        sync(sync_required);
    }

    PThread::Current()->Sleep(50);
  }

  var_mutex.Wait ();
  ready = false;
  close_frame_display ();
  var_mutex.Signal ();
}

void GMVideoOutputManager::set_frame_data (const char* data,
					   unsigned width,
					   unsigned height,
					   unsigned type,
					   int devices_nbr)
{
  Ekiga::DisplayInfo local_display_info;

  if (type < 2)
    get_display_info (local_display_info);
  else
    get_ext_display_info (local_display_info);

  bool local = (type == 0);

  var_mutex.Wait();
  if (!ready) {
    var_mutex.Signal();
    return;
  }

  if (local) {

    /* memcpy the frame */
    if ((unsigned) lframeStore.GetSize () != width * height * 3)
      lframeStore.SetSize (width * height *3);
    current_frame.local_width = width;
    current_frame.local_height= height;
    memcpy (lframeStore.GetPointer(), data, (width * height * 3) >> 1);
    local_frame_received = true;
  }
  else if (type == 1) { // REMOTE 1

    /* memcpy the frame */
    if ((unsigned) rframeStore.GetSize () != width * height * 3)
      rframeStore.SetSize (width * height * 3);
    current_frame.remote_width = width;
    current_frame.remote_height= height;
    memcpy (rframeStore.GetPointer(), data, (width * height * 3) >> 1);
    remote_frame_received = true;
  }
  else if (type == 2) { // REMOTE 2 (extended video)
    /* memcpy the frame */
    if ((unsigned) eframeStore.GetSize () != width * height * 3)
      eframeStore.SetSize (width * height * 3);
    current_frame.ext_width = width;
    current_frame.ext_height= height;
    memcpy (eframeStore.GetPointer(), data, (width * height * 3) >> 1);
    ext_frame_received = true;
  } else {
    var_mutex.Signal();
    return; // nothing happened
  }

  /* If there is only one device open, ignore the setting, and
   * display what we can actually display.
   */
  if (devices_nbr <= 1) {
    if (local) {
      local_display_info.mode = Ekiga::VO_MODE_LOCAL;
      remote_frame_received = false;
    }
    else {
      local_display_info.mode = Ekiga::VO_MODE_REMOTE;
      local_frame_received = false;
    }

    current_frame.both_streams_active = false;
    current_frame.ext_stream_active = false;
  } else {

    if (local_frame_received && !remote_frame_received)
        local_display_info.mode = Ekiga::VO_MODE_LOCAL;

    if (!local_frame_received && remote_frame_received)
        local_display_info.mode = Ekiga::VO_MODE_REMOTE;

    current_frame.both_streams_active = local_frame_received & remote_frame_received;
    current_frame.ext_stream_active = ext_frame_received;
  }
  current_frame.mode = local_display_info.mode;
  current_frame.zoom = local_display_info.zoom;

  if (local) {

    if (update_required.local) {
      PTRACE(3, "GMVideoOutputManager\tSkipped earlier local frame");
    }
    update_required.local = true;
  }
  else if (type == 1) {

    if (update_required.remote) {
      PTRACE(3, "GMVideoOutputManager\tSkipped earlier remote frame");
    }
    update_required.remote = true;
  }
  else if (type == 2) {

    if (update_required.extended) {
      PTRACE(3, "GMVideoOutputManager\tSkipped earlier extended frame");
    }
    update_required.extended = true;
  }

  var_mutex.Signal();

  if ((local_display_info.mode == Ekiga::VO_MODE_UNSET) ||
      (local_display_info.zoom == 0) ||
      (!local_display_info.config_info_set)) {
    PTRACE(4, "GMVideoOutputManager\tDisplay and zoom variable not set yet, not opening display");
    return;
  }

  if ((local_display_info.mode == Ekiga::VO_MODE_LOCAL) && !local)
    return;

  if (local_display_info.mode == Ekiga::VO_MODE_REMOTE && type != 1)
    return;

  if (local_display_info.mode == Ekiga::VO_MODE_REMOTE_EXT && type != 2)
    return;
}


void GMVideoOutputManager::init()
{
  /* State for last frame */
  last_frame.mode = Ekiga::VO_MODE_UNSET;
  last_frame.accel = Ekiga::VO_ACCEL_NO_VIDEO;
  last_frame.both_streams_active = false;
  last_frame.ext_stream_active = false;

  last_frame.local_width = 0;
  last_frame.local_height = 0;
  last_frame.remote_width = 0;
  last_frame.remote_height = 0;
  last_frame.ext_width = 0;
  last_frame.ext_height = 0;
  last_frame.zoom = 0;
  last_frame.embedded_x = 0;
  last_frame.embedded_y = 0;

  last_ext_frame.mode = Ekiga::VO_MODE_UNSET;
  last_ext_frame.accel = Ekiga::VO_ACCEL_NO_VIDEO;
  last_ext_frame.both_streams_active = false;
  last_ext_frame.ext_stream_active = false;

  last_ext_frame.local_width = 0;
  last_ext_frame.local_height = 0;
  last_ext_frame.remote_width = 0;
  last_ext_frame.remote_height = 0;
  last_ext_frame.ext_width = 0;
  last_ext_frame.ext_height = 0;
  last_ext_frame.zoom = 0;
  last_ext_frame.embedded_x = 0;
  last_ext_frame.embedded_y = 0;

  current_frame.ext_stream_active = false;
  current_frame.both_streams_active = false;
  current_frame.local_width = 0;
  current_frame.local_height = 0;
  current_frame.remote_width = 0;
  current_frame.remote_height = 0;
  current_frame.ext_width = 0;
  current_frame.ext_height = 0;

  /* Initialisation */
  video_disabled = false;
  local_frame_received = false;
  remote_frame_received = false;
  ext_frame_received = false;
  update_required.local = false;
  update_required.remote = false;
  update_required.extended = false;
  ready = false;
}

void GMVideoOutputManager::uninit ()
{
  /* This is common to all output classes */
  lframeStore.SetSize (0);
  rframeStore.SetSize (0);
  eframeStore.SetSize (0);
}

void GMVideoOutputManager::update_gui_device ()
{
  last_frame.both_streams_active = current_frame.both_streams_active;
  last_frame.ext_stream_active = current_frame.ext_stream_active;

  last_ext_frame.both_streams_active = current_frame.both_streams_active;
  last_ext_frame.ext_stream_active = current_frame.ext_stream_active;

  Ekiga::Runtime::run_in_main (boost::bind (&GMVideoOutputManager::device_closed_in_main, this));

  Ekiga::Runtime::run_in_main
    (boost::bind (&GMVideoOutputManager::device_opened_in_main, this,
                  current_frame.accel, current_frame.mode, current_frame.zoom,
                  current_frame.both_streams_active,
                  current_frame.ext_stream_active));
}

bool
GMVideoOutputManager::frame_display_change_needed ()
{
  Ekiga::DisplayInfo local_display_info;
  FrameInfo l_frame;

  l_frame = last_frame;
  if (current_frame.mode == Ekiga::VO_MODE_REMOTE_EXT)
    l_frame = last_ext_frame;

  if (l_frame.mode != current_frame.mode ||
      l_frame.zoom != current_frame.zoom) {
    return true;
  }

  if (current_frame.mode != Ekiga::VO_MODE_REMOTE_EXT)
    get_display_info (local_display_info);
  else
    get_ext_display_info (local_display_info);

  if ((!local_display_info.widget_info_set) ||
      (!local_display_info.config_info_set) ||
      (local_display_info.mode == Ekiga::VO_MODE_UNSET) ||
      (local_display_info.zoom == 0)) {
    PTRACE(4, "GMVideoOutputManager\tWidget not yet realized or gconf info not yet set, not opening display");
    return false;
  }

  bool local_changed = (l_frame.local_width  != current_frame.local_width ||
                        l_frame.local_height != current_frame.local_height);
  bool remote_changed = (l_frame.remote_width  != current_frame.remote_width ||
                         l_frame.remote_height != current_frame.remote_height);
  bool window_changed = (local_display_info.x != l_frame.embedded_x ||
                        local_display_info.y != l_frame.embedded_y);
  bool ext_changed = (l_frame.ext_width  != current_frame.ext_width ||
                      l_frame.ext_height != current_frame.ext_height);

  switch (current_frame.mode) {
  case Ekiga::VO_MODE_LOCAL:
    return ( local_changed || window_changed );
    break;

  case Ekiga::VO_MODE_REMOTE:
    return ( remote_changed || window_changed );
    break;

  case Ekiga::VO_MODE_PIP:
    return ( remote_changed || local_changed || window_changed );
    break;

  case Ekiga::VO_MODE_PIP_WINDOW:
  case Ekiga::VO_MODE_FULLSCREEN:
    return ( remote_changed || local_changed );
    break;

  case Ekiga::VO_MODE_REMOTE_EXT:
    return ( ext_changed || window_changed );
    break;

  case Ekiga::VO_MODE_UNSET:
  default:
    break;
  }

  return false;
}

GMVideoOutputManager::UpdateRequired
GMVideoOutputManager::redraw ()
{
  PWaitAndSignal m(var_mutex);

  UpdateRequired sync_required;
  sync_required = update_required;

  if (last_frame.both_streams_active != current_frame.both_streams_active ||
      last_frame.ext_stream_active != current_frame.ext_stream_active ||
      last_ext_frame.both_streams_active != current_frame.both_streams_active ||
      last_ext_frame.ext_stream_active != current_frame.ext_stream_active)
    update_gui_device ();
  else if (frame_display_change_needed ())
    setup_frame_display ();

  switch (current_frame.mode) {
  case Ekiga::VO_MODE_LOCAL:
    if ((unsigned) lframeStore.GetSize() == current_frame.local_width*current_frame.local_height*3)
      display_frame ((char *) lframeStore.GetPointer (),
                     current_frame.local_width, current_frame.local_height);
    break;

  case Ekiga::VO_MODE_REMOTE:
    if ((unsigned) rframeStore.GetSize() == current_frame.remote_width*current_frame.remote_height*3)
      display_frame ((char *) rframeStore.GetPointer (),
                     current_frame.remote_width, current_frame.remote_height);
    break;

  case Ekiga::VO_MODE_FULLSCREEN:
  case Ekiga::VO_MODE_PIP:
  case Ekiga::VO_MODE_PIP_WINDOW:
    if (((unsigned) lframeStore.GetSize() == current_frame.local_width*current_frame.local_height *3)
        && ((unsigned) rframeStore.GetSize() == current_frame.remote_width * current_frame.remote_height *3))
      display_pip_frames ((char *) lframeStore.GetPointer (),
                          current_frame.local_width, current_frame.local_height,
                          (char *) rframeStore.GetPointer (),
                          current_frame.remote_width, current_frame.remote_height);
    break;

  case Ekiga::VO_MODE_REMOTE_EXT:
    if ((unsigned) eframeStore.GetSize() == current_frame.ext_width*current_frame.ext_height*3)
      display_frame ((char *) eframeStore.GetPointer (),
                     current_frame.ext_width, current_frame.ext_height);
    break;

  case Ekiga::VO_MODE_UNSET:
  default:
    break;
  }

  update_required.local = false;
  update_required.remote = false;
  update_required.extended = false;

  return sync_required;
}


void
GMVideoOutputManager::device_opened_in_main (Ekiga::VideoOutputAccel accel,
					     Ekiga::VideoOutputMode mode,
					     unsigned zoom,
					     bool both, bool ext)
{
  device_opened (accel, mode, zoom, both, ext);
}

void
GMVideoOutputManager::device_closed_in_main ()
{
  device_closed ();
}
