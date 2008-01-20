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

GMDisplayManager_dx::GMDisplayManager_dx (Ekiga::ServiceCore & _core)
: GMDisplayManager(_core)
{
  dxWindow = NULL;
}

GMDisplayManager_dx::~GMDisplayManager_dx ()
{
}

bool
GMDisplayManager_dx::frame_display_change_needed ()
{
  if (!dxWindow)
    return true;

  return GMDisplayManager::frame_display_change_needed ();
}

void
GMDisplayManager_dx::setup_frame_display ()
{
  Display local_display_info;

  get_display_info(local_display_info);

//  runtime.run_in_main (force_redraw.make_slot ()); //FIXME: check

  switch (current_frame.display) {
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
    PTRACE (1, "GMDisplayManager_DX\tDisplay variable not set");
    return;
    break; 
  }

  if ((!local_display_info.widget_info_set) || (!local_display_info.config_info_set)) 
      (local_display_info.display == UNSET) || (local_display_info.zoom == 0) || (current_frame.zoom == 0)) {
    PTRACE(4, "GMDisplayManager_DX\tWidget not yet realized or gconf info not yet set, not opening display");
    return;
  }

  close_frame_display ();

  runtime.run_in_main (sigc::bind (display_mode_changed.make_slot (), current_frame.display));

  HwAccelStatus hw_accel_stats = NONE;

  switch (current_frame.display) {
  case LOCAL_VIDEO:
    PTRACE(4, "GMDisplayManager_DX\tOpening LOCAL_VIDEO display with image of " << current_frame.local_width << "x" << current_frame.local_height);
    dxWindow = new DXWindow();
    hw_accel_stats = (HwAccelStatus) dxWindow->Init (local_display_info.hwnd,
                          local_display_info.x,
                          local_display_info.y,
                            (int) (current_frame.local_width * current_frame.zoom / 100), 
                            (int) (current_frame.local_height * current_frame.zoom / 100),
                            current_frame.local_width, 
                            current_frame.local_height);

    lastFrame.embeddedX = local_display_info.x;
    lastFrame.embeddedY = local_display_info.y;

    last_frame.display = LOCAL_VIDEO;
    last_frame.local_width = current_frame.local_width;
    last_frame.local_height = current_frame.local_height;
    last_frame.zoom = current_frame.zoom;
    break;

  case REMOTE_VIDEO:
    PTRACE(4, "GMDisplayManager_DX\tOpening REMOTE_VIDEO display with image of " << current_frame.remote_width << "x" << current_frame.remote_height);
    dxWindow = new DXWindow();
    hw_accel_stats = (HwAccelStatus) dxWindow->Init (local_display_info.hwnd,
                          local_display_info.x,
                          local_display_info.y,
                          (int) (current_frame.remote_width * current_frame.zoom / 100), 
                          (int) (current_frame.remote_height * current_frame.zoom / 100),
                          current_frame.remote_width, 
                          current_frame.remote_height); 

    lastFrame.embeddedX = local_display_info.x;
    lastFrame.embeddedY = local_display_info.y;

    last_frame.display = REMOTE_VIDEO;
    last_frame.remote_width = current_frame.remote_width;
    last_frame.remote_height = current_frame.remote_height;
    last_frame.zoom = current_frame.zoom;
    break;

  case FULLSCREEN:
  case PIP:
  case PIP_WINDOW:
    PTRACE(4, "GMDisplayManager_DX\tOpening display " << current_frame.display << " with images of " 
            << current_frame.local_width << "x" << current_frame.local_height << "(local) and " 
	    << current_frame.remote_width << "x" << current_frame.remote_height << "(remote)");
    dxWindow = new DXWindow();
    hw_accel_stats = (HwAccelStatus) dxWindow->Init ((current_frame.display == PIP) ? local_display_info.hwnd : NULL,
                          (current_frame.display == PIP) ? local_display_info.x : 0,
                          (current_frame.display == PIP) ? local_display_info.y : 0,
                          (int) (current_frame.remote_width * current_frame.zoom  / 100), 
                          (int) (current_frame.remote_height * current_frame.zoom  / 100),
                             current_frame.remote_width, 
                             current_frame.remote_height,
                             current_frame.local_width, 
                             current_frame.local_height); 

    if (dxWindow && current_frame.display == FULLSCREEN) 
      dxWindow->ToggleFullscreen ();

    lastFrame.embeddedX = local_display_info.x;
    lastFrame.embeddedY = local_display_info.y;

    last_frame.display = current_frame.display;
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
  PTRACE (4, "GMVideoDisplay_DX\tSetup display " << display << " with zoom value of " << zoom );

  if (local_display_info.on_top && dxWindow)
      dxWindow->ToggleOntop ();

//   if (!status)
//     close_frame_display ();

  runtime.run_in_main (sigc::bind (hw_accel_status_changed.make_slot (), hw_accel_status));
}

void
GMDisplayManager_dx::close_frame_display ()
{
  runtime.run_in_main (sigc::bind (hw_accel_status_changed.make_slot (), NO_VIDEO));

  if (dxWindow) {

    delete dxWindow;
    dxWindow = NULL;
  }
}

void
GMDisplayManager_dx::display_frame (const char *frame,
                             unsigned width,
                             unsigned height)
{
// FIXME processEvents
  if  (dxWindow)
    dxWindow->PutFrame ((uint8_t *) frame, width, height);
}

void
GMDisplayManager_dx::display_pip_frames (const char *local_frame,
                                 unsigned lf_width,
                                 unsigned lf_height,
                                 const char *remote_frame,
                                 unsigned rf_width,
                                 unsigned rf_height)
{
// FIXME processEvents
  if (currentFrame.display == FULLSCREEN && dxWindow && !dxWindow->IsFullScreen ())
    runtime.run_in_main (sigc::bind (fullscreen_mode_changed.make_slot (), OFF));

//  if (dxWindow && (update_required.remote || (!update_required.remote && !update_required.local)))

  if (dxWindow)
    dxWindow->PutFrame ((uint8_t *) remote_frame, rf_width, rf_height, 
                        (uint8_t *) local_frame, lf_width, lf_height);
}

void
GMDisplayManager_x::sync (UpdateRequired sync_required)
{
  // FIXME sync
}
