
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         gdkvideoio.cpp  -  description
 *                         ------------------------------
 *   begin                : Sat Feb 17 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Class to permit to display in GDK Drawing Area or
 *                          SDL.
 *
 */


#include "../config.h"

#include "gdkvideoio.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "main_window.h"


#include "gm_conf.h"

#include <ptlib/vconvert.h>

PBYTEArray GDKVideoOutputDevice::lframeStore;
PBYTEArray GDKVideoOutputDevice::rframeStore;

int GDKVideoOutputDevice::rf_width;
int GDKVideoOutputDevice::lf_width;
int GDKVideoOutputDevice::rf_height;
int GDKVideoOutputDevice::lf_height;


/* The Methods */
GDKVideoOutputDevice::GDKVideoOutputDevice(int idno)
{ 
  /* Used to distinguish between input and output device. */
  device_id = idno; 

  start_in_fullscreen = FALSE;

  gnomemeeting_threads_enter ();
  /* Change the zoom value following we have to start in fullscreen
   * or not */
  start_in_fullscreen = 
    gm_conf_get_bool (VIDEO_DISPLAY_KEY "start_in_fullscreen");
  if (!start_in_fullscreen) {

    if (gm_conf_get_float (VIDEO_DISPLAY_KEY "zoom_factor") == -1.0)
      gm_conf_set_float (VIDEO_DISPLAY_KEY "zoom_factor", 1.00);
  } 
  else {

    gm_conf_set_float (VIDEO_DISPLAY_KEY "zoom_factor", -1.0);
  }
  gnomemeeting_threads_leave ();
}


GDKVideoOutputDevice::~GDKVideoOutputDevice()
{
  PWaitAndSignal m(redraw_mutex);

  lframeStore.SetSize (0);
  rframeStore.SetSize (0);
}


BOOL GDKVideoOutputDevice::Redraw ()
{
  GtkWidget *main_window = NULL;
  
  GMH323EndPoint *ep = NULL;

  double zoom = 1.0;
  double rzoom = 1.0;
  double lzoom = 1.0;
  int display = LOCAL_VIDEO;

  gboolean bilinear_filtering = FALSE;
  
  ep = GnomeMeeting::Process ()->Endpoint ();
  main_window = GnomeMeeting::Process ()->GetMainWindow (); 


  
  /* Take the mutexes before the redraw */
  redraw_mutex.Wait ();


  /* Updates the zoom value */
  gnomemeeting_threads_enter ();
  bilinear_filtering = 
    gm_conf_get_bool (VIDEO_DISPLAY_KEY "enable_bilinear_filtering");
  display = gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view");
  zoom = gm_conf_get_float (VIDEO_DISPLAY_KEY "zoom_factor");
  gnomemeeting_threads_leave ();

  

  /* If we are not in a call, then display the local video. If we
   * are in a call, then display what config tells us, except if
   * it requests to display both video streams and that there is only
   * one available 
   */
  

  if (!ep->CanAutoStartTransmitVideo () 
      || !ep->CanAutoStartReceiveVideo ()) {

    if (device_id == REMOTE)
      display = REMOTE_VIDEO;
    else if (device_id == LOCAL)
      display = LOCAL_VIDEO;
  }

  if (ep->GetCallingState () != GMH323EndPoint::Connected) 
    display = LOCAL_VIDEO;


  /* Display with the rigth zoom */
  gnomemeeting_threads_enter ();
  if (zoom == -1.0) {
  
    display = FULLSCREEN;
  }
  if (display == REMOTE_VIDEO || display == LOCAL_VIDEO) {
    
    rzoom = lzoom = zoom;
  }
  else if (display == BOTH) {
    
    lzoom = gm_conf_get_float (VIDEO_DISPLAY_KEY "local_zoom_factor");
    rzoom = gm_conf_get_float (VIDEO_DISPLAY_KEY "remote_zoom_factor");
  }
  else if (display == FULLSCREEN) {
  
    if (lf_width > 0)
      lzoom = (GM_QCIF_WIDTH / (double) lf_width);
    else
      lzoom = 1.0;

    if (rf_width > 0)
      rzoom = (GM_CIF_WIDTH / (double) rf_width);
    else
      rzoom = 1.0;
  }
  else if (display == BOTH_INCRUSTED) {
    
    rzoom = zoom;
    if (lf_height != 0)
      lzoom = (double) (rf_height / 3.00) / lf_height * rzoom;
    else
      lzoom = 0;
  }
  else if (display == BOTH_SIDE) {

    rzoom = zoom;
    if (lf_height != 0)
      lzoom = (double) rf_height / lf_height * rzoom;
    else
      lzoom = 0;
  }
  
  gm_main_window_update_video (main_window,
			       (const guchar *) lframeStore,
			       lf_width, lf_height, lzoom,
			       (const guchar *) rframeStore,
			       rf_width, rf_height, rzoom,
			       display, FALSE);
  gnomemeeting_threads_leave ();
  redraw_mutex.Signal ();

  return TRUE;
}


PStringList GDKVideoOutputDevice::GetDeviceNames() const
{
  PStringList  devlist;
  devlist.AppendString(GetDeviceName());

  return devlist;
}


BOOL GDKVideoOutputDevice::IsOpen ()
{
  return TRUE;
}


BOOL GDKVideoOutputDevice::SetFrameData (unsigned x,
					 unsigned y,
					 unsigned width,
					 unsigned height,
					 const BYTE * data,
					 BOOL endFrame)
{
  if (x+width > width || y+height > height)
    return FALSE;

  if (width != GM_CIF_WIDTH && width != GM_QCIF_WIDTH) 
    return FALSE;
  
  if (height != GM_CIF_HEIGHT && height != GM_QCIF_HEIGHT) 
    return FALSE;

  if (!endFrame)
    return FALSE;

  if (device_id == LOCAL) {

    lframeStore.SetSize (width * height * 3);
    lf_width = width;
    lf_height = height;

    if (converter)
      converter->Convert (data, lframeStore.GetPointer ());
  }
  else {

    rframeStore.SetSize (width * height * 3);
    rf_width = width;
    rf_height = height;

    if (converter)
      converter->Convert (data, rframeStore.GetPointer ());
  }
  
  EndFrame ();
  
  return TRUE;
}


BOOL GDKVideoOutputDevice::EndFrame()
{
  Redraw ();
  
  return TRUE;
}


BOOL GDKVideoOutputDevice::SetColourFormat (const PString & colour_format)
{
  if (colour_format == "BGR24")
    return PVideoOutputDevice::SetColourFormat (colour_format);

  return FALSE;  
}
