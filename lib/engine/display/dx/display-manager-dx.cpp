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
 *                         display-manager-dx.cpp -  description
 *                         ----------------------------------
 *   begin                : Sun Nov 19 2006
 *   copyright            : (C) 2006-2008 by Matthias Schneider
 *                          (C) 2000-2008 by Damien Sandras
 *   description          : Class to allow video output to a DirectX
 *                          accelerated window
 */

#include "display-manager-dx.h"

GMVideoOutputManager_dx::GMVideoOutputManager_dx (Ekiga::ServiceCore & _core)
: GMVideoOutputManager(_core)
{
  dxWindow = NULL;

  end_thread = false;
  init_thread = false;
  uninit_thread = false;

  this->Resume ();
  thread_created.Wait ();
}

GMVideoOutputManager_dx::~GMVideoOutputManager_dx ()
{
  end_thread = true;
  run_thread.Signal();
  thread_ended.Wait();
}

bool
GMVideoOutputManager_dx::frame_display_change_needed ()
{
  if (!dxWindow)
    return true;

  return GMVideoOutputManager::frame_display_change_needed ();
}

void
GMVideoOutputManager_dx::setup_frame_display ()
{
  DisplayInfo local_display_info;

  get_display_info(local_display_info);

//  runtime.run_in_main (force_redraw.make_slot ()); //FIXME: check

  switch (current_frame.mode) {
  case LOCAL_VIDEO:
    runtime.run_in_main (sigc::bind (display_size_changed.make_slot (), (unsigned) (current_frame.local_width * current_frame.zoom / 100), (unsigned) (current_frame.local_height * current_frame.zoom / 100)));
    break;
  case REMOTE_VIDEO:
  case PIP:
    runtime.run_in_main (sigc::bind (display_size_changed.make_slot (), (unsigned) (current_frame.remote_width * current_frame.zoom / 100), (unsigned) (current_frame.remote_height * current_frame.zoom / 100)));
    break;
  case FULLSCREEN:
    runtime.run_in_main (sigc::bind (display_size_changed.make_slot (), 176, 144));
    break;
  case PIP_WINDOW:
    runtime.run_in_main (sigc::bind (display_size_changed.make_slot (), 176, 144));
    break;
  case UNSET:
  default:
    PTRACE (1, "GMVideoOutputManager_DX\tDisplay variable not set");
    return;
    break; 
  }

  if (   (!local_display_info.widget_info_set) || (!local_display_info.config_info_set) 
      || (local_display_info.mode == UNSET) || (local_display_info.zoom == 0) || (current_frame.zoom == 0)) {
    PTRACE(4, "GMVideoOutputManager_DX\tWidget not yet realized or gconf info not yet set, not opening display");
    return;
  }

  close_frame_display ();

  runtime.run_in_main (sigc::bind (videooutput_mode_changed.make_slot (), current_frame.mode));

  VideoOutputAccel videooutput_accel = VO_ACCEL_NONE;

  switch (current_frame.mode) {
  case LOCAL_VIDEO:
    PTRACE(4, "GMVideoOutputManager_DX\tOpening LOCAL_VIDEO display with image of " << current_frame.local_width << "x" << current_frame.local_height);
    dxWindow = new DXWindow();
    videooutput_accel = (VideoOutputAccel) dxWindow->Init (local_display_info.hwnd,
                          local_display_info.x,
                          local_display_info.y,
                            (int) (current_frame.local_width * current_frame.zoom / 100), 
                            (int) (current_frame.local_height * current_frame.zoom / 100),
                            current_frame.local_width, 
                            current_frame.local_height);

    last_frame.embedded_x = local_display_info.x;
    last_frame.embedded_y = local_display_info.y;

    last_frame.mode = LOCAL_VIDEO;
    last_frame.local_width = current_frame.local_width;
    last_frame.local_height = current_frame.local_height;
    last_frame.zoom = current_frame.zoom;
    break;

  case REMOTE_VIDEO:
    PTRACE(4, "GMVideoOutputManager_DX\tOpening REMOTE_VIDEO display with image of " << current_frame.remote_width << "x" << current_frame.remote_height);
    dxWindow = new DXWindow();
    videooutput_accel = (VideoOutputAccel) dxWindow->Init (local_display_info.hwnd,
                          local_display_info.x,
                          local_display_info.y,
                          (int) (current_frame.remote_width * current_frame.zoom / 100), 
                          (int) (current_frame.remote_height * current_frame.zoom / 100),
                          current_frame.remote_width, 
                          current_frame.remote_height); 

    last_frame.embedded_x = local_display_info.x;
    last_frame.embedded_y = local_display_info.y;

    last_frame.mode = REMOTE_VIDEO;
    last_frame.remote_width = current_frame.remote_width;
    last_frame.remote_height = current_frame.remote_height;
    last_frame.zoom = current_frame.zoom;
    break;

  case FULLSCREEN:
  case PIP:
  case PIP_WINDOW:
    PTRACE(4, "GMVideoOutputManager_DX\tOpening display " << current_frame.mode << " with images of " 
            << current_frame.local_width << "x" << current_frame.local_height << "(local) and " 
	    << current_frame.remote_width << "x" << current_frame.remote_height << "(remote)");
    dxWindow = new DXWindow();
    videooutput_accel = (VideoOutputAccel) dxWindow->Init ((current_frame.mode == PIP) ? local_display_info.hwnd : NULL,
                          (current_frame.mode == PIP) ? local_display_info.x : 0,
                          (current_frame.mode == PIP) ? local_display_info.y : 0,
                          (int) (current_frame.remote_width * current_frame.zoom  / 100), 
                          (int) (current_frame.remote_height * current_frame.zoom  / 100),
                             current_frame.remote_width, 
                             current_frame.remote_height,
                             current_frame.local_width, 
                             current_frame.local_height); 

    if (dxWindow && current_frame.mode == FULLSCREEN) 
      dxWindow->ToggleFullscreen ();

    last_frame.embedded_x = local_display_info.x;
    last_frame.embedded_y = local_display_info.y;

    last_frame.mode = current_frame.mode;
    last_frame.local_width = current_frame.local_width;
    last_frame.local_height = current_frame.local_height;
    last_frame.remote_width = current_frame.remote_width;
    last_frame.remote_height = current_frame.remote_height;
    last_frame.zoom = current_frame.zoom;
    break;

  case UNSET:
  default:
    return;
    break;
  }
  PTRACE (4, "GMVideoDisplay_DX\tSetup display " << current_frame.mode << " with zoom value of " << current_frame.zoom );

  if (local_display_info.on_top && dxWindow)
      dxWindow->ToggleOntop ();

//   if (!status)
//     close_frame_display ();

  runtime.run_in_main (sigc::bind (device_opened.make_slot (), videooutput_accel));
}

void
GMVideoOutputManager_dx::close_frame_display ()
{
  runtime.run_in_main (device_closed.make_slot ());

  if (dxWindow) {

    delete dxWindow;
    dxWindow = NULL;
  }
}

void
GMVideoOutputManager_dx::display_frame (const char *frame,
                             unsigned width,
                             unsigned height)
{
  if  (dxWindow) {
    dxWindow->ProcessEvents();
    dxWindow->PutFrame ((uint8_t *) frame, width, height, false);
  }
}

void
GMVideoOutputManager_dx::display_pip_frames (const char *local_frame,
                                 unsigned lf_width,
                                 unsigned lf_height,
                                 const char *remote_frame,
                                 unsigned rf_width,
                                 unsigned rf_height)
{
  if (dxWindow)
    dxWindow->ProcessEvents(); 

  if (current_frame.mode == FULLSCREEN && dxWindow && !dxWindow->IsFullScreen ())
    runtime.run_in_main (sigc::bind (fullscreen_mode_changed.make_slot (), OFF));

  if (dxWindow) {
    if (update_required.remote || (!update_required.remote && !update_required.local))
      dxWindow->PutFrame ((uint8_t *) remote_frame, rf_width, rf_height, false);
      
    if (update_required.local  || (!update_required.remote && !update_required.local))
      dxWindow->PutFrame ((uint8_t *) local_frame, lf_width, lf_height, true);      
  }
}

void
GMVideoOutputManager_dx::sync (UpdateRequired sync_required)
{
  if (dxWindow)
    dxWindow->Sync(); 
}
