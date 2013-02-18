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
 *                         videooutput-manager-x.cpp -  description
 *                         ----------------------------------
 *   begin                : Sun Nov 19 2006
 *   copyright            : (C) 2006-2008 by Matthias Schneider
 *                          (C) 2000-2008 by Damien Sandras
 *   description          : Class to allow video output to a X/XVideo
 *                          accelerated window
 */

#include "videooutput-manager-x.h"

#include "xwindow.h"

#ifdef HAVE_XV
#include "xvwindow.h"
#endif

GMVideoOutputManager_x::GMVideoOutputManager_x (Ekiga::ServiceCore & _core)
: GMVideoOutputManager(_core)
{
  /* Internal stuff */
  lxWindow = NULL;
  rxWindow = NULL;
  exWindow = NULL;

  rDisplay = XOpenDisplay (NULL);
  lDisplay = XOpenDisplay (NULL);
  eDisplay = XOpenDisplay (NULL);

  pip_window_available = true;

  end_thread = false;
  init_thread = false;
  uninit_thread = false;

  this->Resume ();
  thread_created.Wait ();
}

GMVideoOutputManager_x::~GMVideoOutputManager_x ()
{
}

void GMVideoOutputManager_x::quit ()
{
  end_thread = true;
  run_thread.Signal();
  thread_ended.Wait();

  if (lDisplay)
    XCloseDisplay (lDisplay);
  if (rDisplay)
    XCloseDisplay (rDisplay);
  if (eDisplay)
    XCloseDisplay (eDisplay);
}

bool
GMVideoOutputManager_x::frame_display_change_needed ()
{
    switch (current_frame.mode)
    {
      case Ekiga::VO_MODE_LOCAL:
          if (!lxWindow)
            return true;
          break;
      case Ekiga::VO_MODE_REMOTE:
          if (!rxWindow)
            return true;
          break;
      case Ekiga::VO_MODE_FULLSCREEN:
      case Ekiga::VO_MODE_PIP:
      case Ekiga::VO_MODE_PIP_WINDOW:
          if (!rxWindow || (pip_window_available && (!lxWindow)) )
              return true;
          break;
      case Ekiga::VO_MODE_REMOTE_EXT:
          if (!exWindow)
            return true;
          break;
     case Ekiga::VO_MODE_UNSET:
     default:
          break;
    }
  return GMVideoOutputManager::frame_display_change_needed ();

}

XWindow *
GMVideoOutputManager_x::create_window (Ekiga::DisplayInfo &info,
                                       const struct WinitContinuation &contXV,
                                       const struct WinitContinuation &contX,
                                       bool pip)
{
  Ekiga::VideoOutputAccel accel = Ekiga::VO_ACCEL_NONE;
  XWindow *win = NULL;
  struct WinitContinuation cont = contXV;
  Ekiga::VideoOutputMode mode = current_frame.mode;

#ifdef HAVE_XV
  if (!info.disable_hw_accel) {
    win = new XVWindow ();
    accel = Ekiga::VO_ACCEL_ALL;
    cont = contXV;
  }
#endif

  do { // let's test the xvwindow; if it fails, try xwindow
    if (!win) {
      if (pip && !info.allow_pip_sw_scaling) {
        pip_window_available = false;
        current_frame.accel = Ekiga::VO_ACCEL_ALL;
        return NULL;
      }

      win = new XWindow ();
      accel = Ekiga::VO_ACCEL_NONE;
      cont = contX;
    }

    if (win->Init (cont.display, cont.window, cont.gc,
                   cont.x, cont.y,
                   cont.wWidth, cont.wHeight,
                   cont.iWidth, cont.iHeight)) {

      current_frame.accel = accel;

      if (accel == Ekiga::VO_ACCEL_NONE)
        win->SetSwScalingAlgo(info.sw_scaling_algorithm);

      if (pip)
        pip_window_available = true;

      break;
    }
    else { // Couldn't init window

      delete win;
      win = NULL;

      PString strmode = "PIP";
      if (mode == Ekiga::VO_MODE_LOCAL)
        strmode = "LOCAL";
      else if (mode == Ekiga::VO_MODE_REMOTE)
        strmode = "REMOTE";

      if (accel == Ekiga::VO_ACCEL_NONE) { // bailout
        PTRACE(1, "GMVideoOutputManager_X\t:"<< strmode
               << ": Could not open X Window - no video");

        if (!pip) {
          current_frame.accel = Ekiga::VO_ACCEL_NO_VIDEO;
          video_disabled = true;
        }
        else {
          pip_window_available = false;
        }

        return NULL;

      }
      else { // 2nd try
        PTRACE(1, "GMVideoOutputManager_X\t:" << strmode
               << ": Could not open XV Window");
      }
    }
  } while (!win);

  return win;
}

void
GMVideoOutputManager_x::setup_frame_display ()
{
  Ekiga::DisplayInfo local_display_info;

  if (video_disabled)
    return;

  get_display_info(local_display_info);

  switch (current_frame.mode) {
  case Ekiga::VO_MODE_LOCAL:
    Ekiga::Runtime::run_in_main (boost::bind (&GMVideoOutputManager_x::size_changed_in_main, this, (unsigned) (current_frame.local_width * current_frame.zoom / 100), (unsigned) (current_frame.local_height * current_frame.zoom / 100)));
    break;
  case Ekiga::VO_MODE_REMOTE:
  case Ekiga::VO_MODE_PIP:
    Ekiga::Runtime::run_in_main (boost::bind (&GMVideoOutputManager_x::size_changed_in_main, this, (unsigned) (current_frame.remote_width * current_frame.zoom / 100), (unsigned) (current_frame.remote_height * current_frame.zoom / 100)));
    break;
  case Ekiga::VO_MODE_FULLSCREEN:
    Ekiga::Runtime::run_in_main (boost::bind (&GMVideoOutputManager_x::size_changed_in_main, this, 176, 144));
    break;
  case Ekiga::VO_MODE_PIP_WINDOW:
    Ekiga::Runtime::run_in_main (boost::bind (&GMVideoOutputManager_x::size_changed_in_main, this, 176, 144));
    break;
  case Ekiga::VO_MODE_REMOTE_EXT:
    Ekiga::Runtime::run_in_main (boost::bind (&GMVideoOutputManager_x::size_changed_in_main, this, (unsigned) (current_frame.ext_width * current_frame.zoom / 100), (unsigned) (current_frame.ext_height * current_frame.zoom / 100)));
    break;
  case Ekiga::VO_MODE_UNSET:
  default:
    PTRACE (1, "GMVideoOutputManager_X\tDisplay variable not set");
    return;
    break;
  }

  if ((!local_display_info.widget_info_set) || (!local_display_info.config_info_set) ||
      (local_display_info.mode == Ekiga::VO_MODE_UNSET) || (local_display_info.zoom == 0) || (current_frame.zoom == 0)) {
    PTRACE(4, "GMVideoOutputManager_X\tWidget not yet realized or gconf info not yet set, not opening display");
    return;
  }

  close_frame_display ();

  pip_window_available = false;

  current_frame.accel = Ekiga::VO_ACCEL_NONE;

  switch (current_frame.mode) {
  // VO_MODE_LOCAL ------------------------------------------------------------------
  case Ekiga::VO_MODE_LOCAL: {
    PTRACE(4, "GMVideoOutputManager_X\tOpening VO_MODE_LOCAL display with image of " <<
           current_frame.local_width << "x" << current_frame.local_height);

    struct WinitContinuation cont = {
      lDisplay,
      local_display_info.window,
      local_display_info.gc,
      local_display_info.x,
      local_display_info.y,
      (int) (current_frame.local_width * current_frame.zoom / 100),
      (int) (current_frame.local_height * current_frame.zoom / 100),
      (int) current_frame.local_width,
      (int) current_frame.local_height,
    };

    lxWindow = create_window (local_display_info, cont, cont);

    last_frame.embedded_x = local_display_info.x;
    last_frame.embedded_y = local_display_info.y;

    last_frame.mode = Ekiga::VO_MODE_LOCAL;
    last_frame.local_width = current_frame.local_width;
    last_frame.local_height = current_frame.local_height;
    last_frame.zoom = current_frame.zoom;
    break;
  }

  // VO_MODE_REMOTE ----------------------------------------------------------------
  case Ekiga::VO_MODE_REMOTE: {
    PTRACE(4, "GMVideoOutputManager_X\tOpening VO_MODE_REMOTE display with image of "
           << current_frame.remote_width << "x" << current_frame.remote_height);

    struct WinitContinuation cont = {
      rDisplay,
      local_display_info.window,
      local_display_info.gc,
      local_display_info.x,
      local_display_info.y,
      (int) (current_frame.remote_width * current_frame.zoom / 100),
      (int) (current_frame.remote_height * current_frame.zoom / 100),
      (int) current_frame.remote_width,
      (int) current_frame.remote_height,
    };

    rxWindow = create_window (local_display_info, cont, cont);

    last_frame.embedded_x = local_display_info.x;
    last_frame.embedded_y = local_display_info.y;

    last_frame.mode = Ekiga::VO_MODE_REMOTE;
    last_frame.remote_width = current_frame.remote_width;
    last_frame.remote_height = current_frame.remote_height;
    last_frame.zoom = current_frame.zoom;
    break;
  }

  // VO_MODE_REMOTE ----------------------------------------------------------------
  case Ekiga::VO_MODE_REMOTE_EXT: {
    PTRACE(4, "GMVideoOutputManager_X\tOpening VO_MODE_REMOTE_EXT display with image of "
           << current_frame.ext_width << "x" << current_frame.ext_height);

    struct WinitContinuation cont = {
      eDisplay,
      local_display_info.window,
      local_display_info.gc,
      local_display_info.x,
      local_display_info.y,
      (int) (current_frame.ext_width * current_frame.zoom / 100),
      (int) (current_frame.ext_height * current_frame.zoom / 100),
      (int) current_frame.ext_width,
      (int) current_frame.ext_height,
    };

    exWindow = create_window (local_display_info, cont, cont);

    last_frame.embedded_x = local_display_info.x;
    last_frame.embedded_y = local_display_info.y;

    last_frame.mode = Ekiga::VO_MODE_REMOTE_EXT;
    last_frame.ext_width = current_frame.ext_width;
    last_frame.ext_height = current_frame.ext_height;
    last_frame.zoom = current_frame.zoom;
    break;
  }

  // PIP_VIDEO ------------------------------------------------------------------
  case Ekiga::VO_MODE_FULLSCREEN:
  case Ekiga::VO_MODE_PIP:
  case Ekiga::VO_MODE_PIP_WINDOW: {
    PTRACE(4, "GMVideoOutputManager_X\tOpening display " << current_frame.mode << " with images of "
            << current_frame.local_width << "x" << current_frame.local_height << "(local) and "
	    << current_frame.remote_width << "x" << current_frame.remote_height << "(remote)");

    struct WinitContinuation rcont = {
      rDisplay,
      (current_frame.mode == Ekiga::VO_MODE_PIP) ?
      local_display_info.window : DefaultRootWindow (rDisplay),
      (current_frame.mode == Ekiga::VO_MODE_PIP) ?
      local_display_info.gc : NULL,
      (current_frame.mode == Ekiga::VO_MODE_PIP) ?
      local_display_info.x : 0,
      (current_frame.mode == Ekiga::VO_MODE_PIP) ?
      local_display_info.y : 0,
      (int) (current_frame.remote_width * current_frame.zoom / 100),
      (int) (current_frame.remote_height * current_frame.zoom / 100),
      (int) current_frame.remote_width,
      (int) current_frame.remote_height,
    };

    rxWindow = create_window (local_display_info, rcont, rcont);

    struct WinitContinuation lcontxv = {
      rxWindow->GetDisplay (),
      rxWindow->GetWindowHandle (),
      rxWindow->GetGC (),
      (int) (current_frame.remote_width * current_frame.zoom  / 100 * 2 / 3),
      (int) (current_frame.remote_height * current_frame.zoom  / 100 * 2 / 3),
      (int) (current_frame.remote_width * current_frame.zoom  / 100 / 3),
      (int) (current_frame.remote_height * current_frame.zoom  / 100 / 3),
      (int) current_frame.local_width,
      (int) current_frame.local_height,
    };

    struct WinitContinuation lcontx = {
       lDisplay,
       rxWindow->GetWindowHandle (),
       (current_frame.mode == Ekiga::VO_MODE_PIP) ? local_display_info.gc : NULL,
       (int) (current_frame.remote_width * current_frame.zoom  / 100 * 2 / 3),
       (int) (current_frame.remote_height * current_frame.zoom  / 100 * 2 / 3),
       (int) (current_frame.remote_width * current_frame.zoom  / 100 / 3),
       (int) (current_frame.remote_height * current_frame.zoom  / 100 / 3),
       (int) current_frame.local_width,
       (int) current_frame.local_height,
    };

    lxWindow = create_window (local_display_info, lcontxv, lcontx, true);

    if (rxWindow && lxWindow) {
      rxWindow->RegisterSlave (lxWindow);
      lxWindow->RegisterMaster (rxWindow);
    }

    if (rxWindow && current_frame.mode == Ekiga::VO_MODE_FULLSCREEN)
      rxWindow->ToggleFullscreen ();

    if ((current_frame.mode != Ekiga::VO_MODE_PIP_WINDOW) &&
        (current_frame.mode != Ekiga::VO_MODE_FULLSCREEN)) {
      last_frame.embedded_x = local_display_info.x;
      last_frame.embedded_y = local_display_info.y;
    }

    last_frame.mode = current_frame.mode;
    last_frame.local_width = current_frame.local_width;
    last_frame.local_height = current_frame.local_height;
    last_frame.remote_width = current_frame.remote_width;
    last_frame.remote_height = current_frame.remote_height;
    last_frame.zoom = current_frame.zoom;
    break;
  }

  case Ekiga::VO_MODE_UNSET:
  default:
    return;
    break;
  }

  if (local_display_info.on_top) {

    if (lxWindow)
      lxWindow->ToggleOntop ();
    if (rxWindow)
      rxWindow->ToggleOntop ();
    if (exWindow)
      exWindow->ToggleOntop ();
  }

  last_frame.both_streams_active = current_frame.both_streams_active;
  last_frame.ext_stream_active = current_frame.ext_stream_active;

  if (video_disabled) {
    Ekiga::Runtime::run_in_main (boost::bind (&GMVideoOutputManager_x::device_error_in_main, this, Ekiga::VO_ERROR));
  }
  else {
    Ekiga::Runtime::run_in_main
      (boost::bind (&GMVideoOutputManager_x::device_opened_in_main, this,
                    current_frame.accel, current_frame.mode, current_frame.zoom,
                    current_frame.both_streams_active,
                    current_frame.ext_stream_active));
  }
}

void
GMVideoOutputManager_x::close_frame_display ()
{
  Ekiga::Runtime::run_in_main (boost::bind (&GMVideoOutputManager_x::device_closed_in_main, this));

  if (rxWindow)
    rxWindow->RegisterSlave (NULL);
  if (exWindow)
    exWindow->RegisterSlave (NULL);
  if (lxWindow)
    lxWindow->RegisterMaster (NULL);

  if (lxWindow) {
    delete lxWindow;
    lxWindow = NULL;
  }

  if (rxWindow) {
    delete rxWindow;
    rxWindow = NULL;
  }

  if (exWindow) {
    delete exWindow;
    exWindow = NULL;
  }
}

void
GMVideoOutputManager_x::display_frame (const char *frame,
                             unsigned width,
                             unsigned height)
{
  if (rxWindow)
    rxWindow->ProcessEvents();

  if (lxWindow)
    lxWindow->ProcessEvents();

  if (exWindow)
    exWindow->ProcessEvents();

  if  ((current_frame.mode == Ekiga::VO_MODE_LOCAL) && (lxWindow))
    lxWindow->PutFrame ((uint8_t *) frame, width, height);

  if  ((current_frame.mode == Ekiga::VO_MODE_REMOTE) && (rxWindow))
    rxWindow->PutFrame ((uint8_t *) frame, width, height);

  if  ((current_frame.mode == Ekiga::VO_MODE_REMOTE_EXT) && (exWindow))
    exWindow->PutFrame ((uint8_t *) frame, width, height);
}

void
GMVideoOutputManager_x::display_pip_frames (const char *local_frame,
                                 unsigned lf_width,
                                 unsigned lf_height,
                                 const char *remote_frame,
                                 unsigned rf_width,
                                 unsigned rf_height)
{
  if (rxWindow)
    rxWindow->ProcessEvents();

  if (lxWindow)
    lxWindow->ProcessEvents();

  if (current_frame.mode == Ekiga::VO_MODE_FULLSCREEN && rxWindow && !rxWindow->IsFullScreen ())
    Ekiga::Runtime::run_in_main (boost::bind (&GMVideoOutputManager_x::fullscreen_mode_changed_in_main, this, Ekiga::VO_FS_OFF));

  if (rxWindow && (update_required.remote || (!update_required.remote && !update_required.local)))
    rxWindow->PutFrame ((uint8_t *) remote_frame, rf_width, rf_height);

  if (lxWindow && (update_required.local  || (!update_required.remote && !update_required.local)))
    lxWindow->PutFrame ((uint8_t *) local_frame, lf_width, lf_height);
}

void
GMVideoOutputManager_x::sync (UpdateRequired sync_required)
{
  bool none_required = ( !sync_required.remote &&
                         !sync_required.local &&
                         !sync_required.extended );

  if (rxWindow && (sync_required.remote || none_required)) {
    rxWindow->Sync();
  }

  if (lxWindow && (sync_required.local || none_required)) {
    lxWindow->Sync();
  }

  if (exWindow && (sync_required.extended || none_required)) {
    exWindow->Sync();
  }
}

void
GMVideoOutputManager_x::size_changed_in_main (unsigned width,
					      unsigned height)
{
  size_changed (width, height);
}

void
GMVideoOutputManager_x::device_opened_in_main (Ekiga::VideoOutputAccel accel,
					       Ekiga::VideoOutputMode mode,
					       unsigned zoom,
					       bool both, bool ext)
{
  device_opened (accel, mode, zoom, both, ext);
}

void
GMVideoOutputManager_x::device_closed_in_main ()
{
  device_closed ();
}

void
GMVideoOutputManager_x::device_error_in_main (Ekiga::VideoOutputErrorCodes code)
{
  device_error (code);
}

void
GMVideoOutputManager_x::fullscreen_mode_changed_in_main (Ekiga::VideoOutputFSToggle val)
{
  fullscreen_mode_changed (val);
}
