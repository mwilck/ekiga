
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
 *   copyright            : (C) 2006 by Matthias Schneider
 *                          (C) 2000-2007 by Damien Sandras
 *   description          : Class to allow video output to a DirectX
 *                          accelerated window
 *
 */



#include "../../config.h"

#define P_FORCE_STATIC_PLUGIN 

#include "videooutput_dx.h"
#include "ekiga.h"
#include "misc.h"
#include "main.h"
#include "history.h"


#include "gmconf.h"

#include <ptlib/vconvert.h>

#include <gtk-2.0/gdk/gdkwin32.h>

BOOL PVideoOutputDevice_DX::fallback = FALSE;

/* Plugin definition */
class PVideoOutputDevice_DX_PluginServiceDescriptor 
: public PDevicePluginServiceDescriptor
{
  public:
    virtual PObject *CreateInstance (int) const 
      {
	return new PVideoOutputDevice_DX (); 
      }
    
    
    virtual PStringList GetDeviceNames (int) const 
      { 
	return PStringList ("DX"); 
      }
    
    virtual bool ValidateDeviceName (const PString & deviceName, 
				     int) const 
      { 
	return deviceName.Find ("DX") == 0; 
      }
} PVideoOutputDevice_DX_descriptor;

PCREATE_PLUGIN(DX, PVideoOutputDevice, &PVideoOutputDevice_DX_descriptor);


/* The Methods */
PVideoOutputDevice_DX::PVideoOutputDevice_DX ()
{ 
  /* Internal stuff */
  dxWindow = NULL;

  fallback = FALSE;
}


PVideoOutputDevice_DX::~PVideoOutputDevice_DX()
{
  PWaitAndSignal m(redraw_mutex);

  CloseFrameDisplay ();
}


BOOL 
PVideoOutputDevice_DX::Open (const PString &name, 
                             BOOL unused)
{
  if (name == "DXIN") 
    device_id = 1; 

  return TRUE; 
}


BOOL 
PVideoOutputDevice_DX::SetupFrameDisplay (int display, 
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
    return PVideoOutputDevice_GDK::SetupFrameDisplay (display, lf_width, lf_height,
                                                      rf_width, rf_height, zoom);

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  stay_on_top = gm_conf_get_bool (VIDEO_DISPLAY_KEY "stay_on_top");

  CloseFrameDisplay ();

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
  PTRACE (4, "PVideoOutputDevice_DX\tSetup display " << display << " with zoom value of " << zoom << " : " << (ret ? "Success" : "Failure"));

  if (stay_on_top) {

    if (ret && dxWindow)
      dxWindow->ToggleOntop ();
  }

  if (!ret)
    CloseFrameDisplay ();

  return ret;
}


BOOL 
PVideoOutputDevice_DX::CloseFrameDisplay ()
{
  if (fallback)
    return PVideoOutputDevice_GDK::CloseFrameDisplay ();

  if (dxWindow) {
    delete dxWindow;
    dxWindow = NULL;
  }

  return TRUE;
}


BOOL 
PVideoOutputDevice_DX::Redraw (int display,
                                    double zoom)
{
  BOOL ret = TRUE; 

  if (fallback)
    return PVideoOutputDevice_GDK::Redraw (display, zoom);

  if (device_id == LOCAL) {

    gnomemeeting_threads_enter ();
    switch (display) 
      {
      case LOCAL_VIDEO:
        if (FrameDisplayChangeNeeded (display, lf_width, lf_height, rf_width, rf_height, zoom) || !dxWindow) 
          ret = SetupFrameDisplay (display, lf_width, lf_height, rf_width, rf_height, zoom); 

        if (ret && dxWindow)
          dxWindow->PutFrame ((uint8_t *) lframeStore.GetPointer (), lf_width, lf_height);
        break;

      case REMOTE_VIDEO:
        if (FrameDisplayChangeNeeded (display, lf_width, lf_height, rf_width, rf_height, zoom) || !dxWindow) 
          ret = SetupFrameDisplay (display, lf_width, lf_height, rf_width, rf_height, zoom);

        if (ret && dxWindow)
          dxWindow->PutFrame ((uint8_t *) rframeStore.GetPointer (), rf_width, rf_height);
        break;

      case FULLSCREEN:
      case PIP:
      case PIP_WINDOW:
        if (FrameDisplayChangeNeeded (display, lf_width, lf_height, rf_width, rf_height, zoom) || !dxWindow ) 
          ret = SetupFrameDisplay (display, lf_width, lf_height, rf_width, rf_height, zoom);

        if (display == FULLSCREEN && dxWindow && !dxWindow->IsFullscreen ())
          gm_conf_set_float (VIDEO_DISPLAY_KEY "zoom_factor", 1.00);

        if (ret && dxWindow)
          dxWindow->PutFrame ((uint8_t *) rframeStore.GetPointer (), rf_width, rf_height, (uint8_t *) lframeStore.GetPointer (), lf_width, lf_height);
        break;
      }
    gnomemeeting_threads_leave ();
  }

  if (!ret) {

    PTRACE (4, "PVideoOutputDevice_DX\tFalling back to GDK Rendering");
    fallback = TRUE;
    return PVideoOutputDevice_GDK::Redraw (display, zoom);
  }

  return TRUE;
}


BOOL 
PVideoOutputDevice_DX::SetFrameData (unsigned x,
                                     unsigned y,
                                     unsigned width,
                                     unsigned height,
                                     const BYTE *data,
                                     BOOL endFrame)
{
  if (fallback)
    return PVideoOutputDevice_GDK::SetFrameData (x, y, width, height, data, endFrame);

  if (x+width > width || y+height > height)
    return FALSE;

  if (width != GM_CIF_WIDTH && width != GM_QCIF_WIDTH && width != GM_SIF_WIDTH) 
    return FALSE;
  
  if (height != GM_CIF_HEIGHT && height != GM_QCIF_HEIGHT && height != GM_SIF_HEIGHT) 
    return FALSE;

  if (!endFrame)
    return FALSE;

  if (device_id == LOCAL) {

    lframeStore.SetSize (width * height * 3);
    lf_width = width;
    lf_height = height;

    memcpy (lframeStore.GetPointer(), data,( int) (width * height * 3 / 2)); 
  }
  else {

    rframeStore.SetSize (width * height * 3);
    rf_width = width;
    rf_height = height;

    memcpy (rframeStore.GetPointer(), data, (int) (width * height * 3 / 2)); 
  }
  
  return EndFrame ();
}

