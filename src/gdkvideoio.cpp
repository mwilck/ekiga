
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



/* The Methods */
GDKVideoOutputDevice::GDKVideoOutputDevice(int idno)
{ 
  /* Used to distinguish between input and output device. */
  device_id = idno; 
}


GDKVideoOutputDevice::~GDKVideoOutputDevice()
{
  PWaitAndSignal m(redraw_mutex);
}


BOOL GDKVideoOutputDevice::Redraw ()
{
  GtkWidget *main_window = NULL;
  
  GMH323EndPoint *ep = NULL;
  
  PMutex both_mutex;


  gchar *string = NULL;
  gchar **couple = NULL;

  static double zoom = 1.0;
  int zoomed_width = 0;
  int zoomed_height = 0;
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
  if (display == BOTH) {

    if (device_id == LOCAL) 
      string = 
	gm_conf_get_string (USER_INTERFACE_KEY "local_video_window/size");
    else
      string = 
	gm_conf_get_string (USER_INTERFACE_KEY "remote_video_window/size");
      
    if (string)
      couple = g_strsplit (string, ",", 0);

    if (couple && couple [0])
      zoomed_width = atoi (couple [0]);

    if (couple && couple [1])
      zoomed_height = atoi (couple [1]);
  }
  else {

    zoomed_width = (int) (frameWidth * zoom);
    zoomed_height = (int) (frameHeight * zoom);
  }
  gnomemeeting_threads_leave ();

  
  if (zoom != 0.5 && zoom != 1 && zoom != 2)
    zoom = 1.0;

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


  if ( (((device_id == REMOTE && display == REMOTE_VIDEO) ||
	 (device_id == LOCAL && display == LOCAL_VIDEO)) 
	&& (display != BOTH_INCRUSTED))
       || (display == BOTH)) { 

    gnomemeeting_threads_enter ();
    gm_main_window_update_video (main_window,
				 (const guchar *) frameStore,
				 frameWidth, frameHeight,
				 zoomed_width, zoomed_height,
				 display,
				 (device_id == REMOTE),
				 TRUE);
    gnomemeeting_threads_leave ();
  }
  
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


BOOL GDKVideoOutputDevice::SetFrameData(
					unsigned x,
					unsigned y,
					unsigned width,
					unsigned height,
					const BYTE * data,
					BOOL endFrame)
{
  if (x+width > frameWidth || y+height > frameHeight)
    return FALSE;

  if (!endFrame)
    return FALSE;

  frameStore.SetSize (width * height * 3);
  
  if (converter)
    converter->Convert (data, frameStore.GetPointer ());
  
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
