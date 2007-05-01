
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
 *                         gdkvideoio.cpp  -  description
 *                         ------------------------------
 *   begin                : Sat Feb 17 2001
 *   copyright            : (C) 2000-2007 by Damien Sandras
 *   description          : Class to permit to display in GDK Drawing Area or
 *                          SDL.
 *
 */


#include "../../config.h"

#define P_FORCE_STATIC_PLUGIN 

#include "videooutput_gdk.h"
#include "ekiga.h"
#include "misc.h"
#include "main.h"


#include "gmconf.h"

#include <ptlib/vconvert.h>


PBYTEArray PVideoOutputDevice_GDK::lframeStore;
PBYTEArray PVideoOutputDevice_GDK::rframeStore;

int PVideoOutputDevice_GDK::devices_nbr = 0;

int PVideoOutputDevice_GDK::rf_width = 0;
int PVideoOutputDevice_GDK::lf_width = 0;
int PVideoOutputDevice_GDK::rf_height = 0;
int PVideoOutputDevice_GDK::lf_height = 0;


/* Plugin definition */
class PVideoOutputDevice_GDK_PluginServiceDescriptor 
: public PDevicePluginServiceDescriptor
{
  public:
    virtual PObject *CreateInstance (int) const 
      {
	return new PVideoOutputDevice_GDK (); 
      }
    
    
    virtual PStringList GetDeviceNames(int) const 
      { 
	return PStringList("GDK"); 
      }
    
    virtual bool ValidateDeviceName (const PString & deviceName, 
				     int) const 
      { 
	return deviceName.Find("GDK") == 0; 
      }
} PVideoOutputDevice_GDK_descriptor;

PCREATE_PLUGIN(GDK, PVideoOutputDevice, &PVideoOutputDevice_GDK_descriptor);


/* The Methods */
PVideoOutputDevice_GDK::PVideoOutputDevice_GDK ()
{ 
  is_active = FALSE;
  
  /* Used to distinguish between input and output device. */
  device_id = 0; 

  start_in_fullscreen = FALSE;

  /* State for last frame */
  lastFrame.display = 99;
  lastFrame.localWidth = 0;
  lastFrame.localHeight = 0;
  lastFrame.remoteWidth = 0;
  lastFrame.remoteHeight = 0;  
  lastFrame.zoom = 99;
  lastFrame.embeddedX = 0;
  lastFrame.embeddedY = 0;

  gnomemeeting_threads_enter ();
  start_in_fullscreen = gm_conf_get_bool (VIDEO_DISPLAY_KEY "start_in_fullscreen");
  if (!start_in_fullscreen && gm_conf_get_float (VIDEO_DISPLAY_KEY "zoom_factor") == -1.0)
    gm_conf_set_float (VIDEO_DISPLAY_KEY "zoom_factor", 1.00);
  if (start_in_fullscreen)
    gm_conf_set_float (VIDEO_DISPLAY_KEY "zoom_factor", -1.0);
  gnomemeeting_threads_leave ();

  /* Internal stuff */
  window = NULL;
  image = NULL;
}


PVideoOutputDevice_GDK::~PVideoOutputDevice_GDK()
{
  PWaitAndSignal m(redraw_mutex);

  CloseFrameDisplay ();

  lframeStore.SetSize (0);
  rframeStore.SetSize (0);

  if (is_active)
    devices_nbr = PMAX (0, devices_nbr-1);
}


BOOL 
PVideoOutputDevice_GDK::Open (const PString &name,
			      BOOL unused)
{ 
  if (name == "GDKIN") 
    device_id = 1; 

  return TRUE; 
}


BOOL 
PVideoOutputDevice_GDK::FrameDisplayChangeNeeded (int display, 
                                                  guint lf_width, 
                                                  guint lf_height, 
                                                  guint rf_width, 
                                                  guint rf_height, 
                                                  double zoom)
{
  GtkWidget *main_window = NULL;
  GtkWidget *image = NULL;

  main_window = GnomeMeeting::Process()->GetMainWindow ();
  image = gm_main_window_get_video_widget (main_window);
  PTRACE(4, "PVideoOutputDevice_GDK\tOriginal Settings: dp:" << lastFrame.display << " zoom:" << lastFrame.zoom << " rW:" << lastFrame.remoteWidth << " rH:" << lastFrame.remoteHeight << " lW:" << lastFrame.localWidth << " lH:" << lastFrame.localHeight << " x:" << lastFrame.embeddedX << " y:" << lastFrame.embeddedY);
  PTRACE(4, "PVideoOutputDevice_GDK\tOriginal Settings: dp:" << display << " lzoom:" << zoom << " rW:" << rf_width << " rH:" << rf_height << " lW:" << lf_width << " lH:" << lf_height << " x:" << image->allocation.x << " y:" << image->allocation.y);

  switch (display) {
  case LOCAL_VIDEO:
    return (lastFrame.display != LOCAL_VIDEO 
            || lastFrame.zoom != zoom || lastFrame.localWidth != lf_width || lastFrame.localHeight != lf_height 
            || image->allocation.x != lastFrame.embeddedX || image->allocation.y != lastFrame.embeddedY);
    break;

  case REMOTE_VIDEO:
    return (lastFrame.display != REMOTE_VIDEO
            || lastFrame.zoom != zoom || lastFrame.remoteWidth != rf_width || lastFrame.remoteHeight != rf_height
            || image->allocation.x != lastFrame.embeddedX || image->allocation.y != lastFrame.embeddedY);
    break;

  case PIP:
    return (lastFrame.display != display || lastFrame.zoom != zoom 
            || lastFrame.remoteWidth != rf_width || lastFrame.remoteHeight != rf_height
            || lastFrame.localWidth != lf_width || lastFrame.localHeight != lf_height
            || image->allocation.x != lastFrame.embeddedX || image->allocation.y != lastFrame.embeddedY);
    break;
  case PIP_WINDOW:
  case FULLSCREEN:
    return (lastFrame.display != display || lastFrame.zoom != zoom 
            || lastFrame.remoteWidth != rf_width || lastFrame.remoteHeight != rf_height
            || lastFrame.localWidth != lf_width || lastFrame.localHeight != lf_height);
    break;
  }

  return FALSE;
}


BOOL 
PVideoOutputDevice_GDK::SetupFrameDisplay (int display, 
                                           guint lf_width, 
                                           guint lf_height, 
                                           guint rf_width, 
                                           guint rf_height, 
                                           double zoom)
{
  GtkWidget *main_window = NULL;
  GtkWidget *vbox = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  if (display != PIP_WINDOW) 
    CloseFrameDisplay ();

  switch (display) {
  case LOCAL_VIDEO:
    image = gm_main_window_get_resized_video_widget (main_window,
                                                     (int) (lf_width * zoom),
                                                     (int) (lf_height * zoom));
    lastFrame.display = LOCAL_VIDEO;
    lastFrame.localWidth = lf_width;
    lastFrame.localHeight = lf_height;
    lastFrame.zoom = zoom;
    lastFrame.embeddedX = image->allocation.x;
    lastFrame.embeddedY = image->allocation.y;

    break;

  case REMOTE_VIDEO:
    image = gm_main_window_get_resized_video_widget (main_window,
                                                     (int) (rf_width * zoom),
                                                     (int) (rf_height * zoom));
    lastFrame.display = REMOTE_VIDEO;
    lastFrame.remoteWidth = rf_width;
    lastFrame.remoteHeight = rf_height;
    lastFrame.zoom = zoom;
    lastFrame.embeddedX = image->allocation.x;
    lastFrame.embeddedY = image->allocation.y;

    break;

  case PIP:
    image = gm_main_window_get_resized_video_widget (main_window,
                                                     (int) (rf_width * zoom),
                                                     (int) (rf_height * zoom));
    lastFrame.display = PIP;
    lastFrame.localWidth = lf_width;
    lastFrame.localHeight = lf_height;
    lastFrame.remoteWidth = rf_width;
    lastFrame.remoteHeight = rf_height;
    lastFrame.zoom = zoom;
    lastFrame.embeddedX = image->allocation.x;
    lastFrame.embeddedY = image->allocation.y;

    break;

  case PIP_WINDOW:

    if (window == NULL) {

      window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

      vbox = gtk_vbox_new (FALSE, 0);
      image = gtk_image_new ();

      gtk_box_pack_start (GTK_BOX (vbox), image, TRUE, TRUE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (window), 0);

      gtk_container_add (GTK_CONTAINER (window), vbox);
      gtk_widget_realize (image);

      gtk_window_set_resizable (GTK_WINDOW (window), FALSE);

      gtk_widget_show_all (window);
    }

      gtk_widget_set_size_request (GTK_WIDGET (image), 
                                   (int) (rf_width * zoom), 
                                   (int) (rf_height * zoom));
      lastFrame.remoteWidth = rf_width;
      lastFrame.remoteHeight = rf_height;
      lastFrame.localWidth = lf_width;
      lastFrame.localHeight = lf_height;
      lastFrame.zoom = zoom;

    gm_main_window_update_logo (main_window);

    lastFrame.display = PIP_WINDOW; 
    break;

  default:
    return FALSE;
    break;
  }
  PTRACE (4, "PVideoOutputDevice_GDK\tSetup display " << display << " with zoom value of " << zoom);

  return TRUE;
}


BOOL 
PVideoOutputDevice_GDK::CloseFrameDisplay ()
{
  if (window != NULL) {
    gtk_widget_destroy (window);
    window = NULL;
    image = NULL;
  }

  return TRUE;
}


void 
PVideoOutputDevice_GDK::DisplayFrame (gpointer gtk_image,
                                      const guchar *frame,
                                      guint width,
                                      guint height,
                                      double zoom)
{
  GdkPixbuf *pic = NULL;
  GdkPixbuf *scaled_pic = NULL;
  GtkWidget *image = GTK_WIDGET (gtk_image);

  if (zoom != 1.00 && zoom != 2.00 && zoom != 0.5)
    zoom = 1.00;

  pic = gdk_pixbuf_new_from_data (frame, GDK_COLORSPACE_RGB, 
                                  FALSE, 8, 
                                  width, 
                                  height, 
                                  width * 3, 
                                  NULL, NULL);
  if (zoom != 1.0) {

    scaled_pic = gdk_pixbuf_scale_simple (pic,
                                          (int) (width * zoom),
                                          (int) (height * zoom),
                                          GDK_INTERP_NEAREST);
    gtk_image_set_from_pixbuf (GTK_IMAGE (image), 
                               GDK_PIXBUF (scaled_pic));
    g_object_unref (pic);
    g_object_unref (scaled_pic);
  }
  else {

    gtk_image_set_from_pixbuf (GTK_IMAGE (image), 
                               GDK_PIXBUF (pic));
    g_object_unref (pic);
  }

}


void 
PVideoOutputDevice_GDK::DisplayPiPFrames (gpointer gtk_image,
                                          const guchar *lframe,
                                          guint lwidth,
                                          guint lheight,
                                          const guchar *rframe,
                                          guint rwidth,
                                          guint rheight,
                                          double zoom)
{
  GtkWidget *main_window = NULL;

  GdkPixbuf *pic = NULL;
  GdkPixbuf *local_pic = NULL;
  GdkPixbuf *inside_pic = NULL;
  GdkPixbuf *scaled_pic = NULL;

  GtkWidget *image = GTK_WIDGET (gtk_image);

  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  if (zoom != 1.00 && zoom != 2.00 && zoom != 0.5)
    zoom = 1.00;

  pic = gdk_pixbuf_new_from_data (rframe, GDK_COLORSPACE_RGB, 
                                  FALSE, 8, 
                                  rwidth, 
                                  rheight, 
                                  rwidth * 3, 
                                  NULL, NULL);
  local_pic = gdk_pixbuf_new_from_data (lframe, GDK_COLORSPACE_RGB, 
                                        FALSE, 8, 
                                        lwidth, 
                                        lheight, 
                                        lwidth * 3, 
                                        NULL, NULL);

  inside_pic = gdk_pixbuf_scale_simple (local_pic,
                                        (int) (rwidth / 3),
                                        (int) (rheight / 3),
                                        GDK_INTERP_NEAREST);
  gdk_pixbuf_copy_area  (inside_pic, 
                         0, 0,
                         gdk_pixbuf_get_width (inside_pic), 
                         gdk_pixbuf_get_height (inside_pic),
                         pic,
                         (int) (rf_width * 2 / 3), 
                         (int) (rf_height * 2 / 3));
  if (zoom != 1.0) {

    scaled_pic = gdk_pixbuf_scale_simple (pic,
                                          (int) (rwidth * zoom),
                                          (int) (rheight * zoom),
                                          GDK_INTERP_NEAREST);

  }
  else {

    scaled_pic = gdk_pixbuf_copy (pic);
  }

  gtk_image_set_from_pixbuf (GTK_IMAGE (image), 
                             GDK_PIXBUF (scaled_pic));
  g_object_unref (pic);
  g_object_unref (scaled_pic);
  g_object_unref (local_pic);
  g_object_unref (inside_pic);

}


BOOL 
PVideoOutputDevice_GDK::Redraw (int display, 
                                double zoom)
{
  BOOL ret = TRUE;

  if (device_id == LOCAL) {

    gnomemeeting_threads_enter ();
    switch (display) 
      {
      case LOCAL_VIDEO:
          PTRACE(4,"GDK\tLOCAL output");
          if (FrameDisplayChangeNeeded (display, lf_width, lf_height, rf_width, rf_height, zoom)) 
            ret = SetupFrameDisplay (display, lf_width, lf_height, rf_width, rf_height, zoom); 

          if (ret && image)
            DisplayFrame (image, lframeStore.GetPointer (), lf_width, lf_height, zoom);
        break;

      case REMOTE_VIDEO:
          PTRACE(4,"GDK\tREMOTE output");
          if (FrameDisplayChangeNeeded (display, lf_width, lf_height, rf_width, rf_height, zoom)) 
            ret = SetupFrameDisplay (display, lf_width, lf_height, rf_width, rf_height, zoom);

          if (ret && image)
            DisplayFrame (image, rframeStore.GetPointer (), rf_width, rf_height, zoom);
        break;

      case FULLSCREEN:
        display = PIP;
        gm_conf_set_float (VIDEO_DISPLAY_KEY "zoom_factor", 1.0);
      case PIP:
      case PIP_WINDOW:
          PTRACE(4,"GDK\tPIP output");
          if (FrameDisplayChangeNeeded (display, lf_width, lf_height, rf_width, rf_height, zoom)) 
            ret = SetupFrameDisplay (display, lf_width, lf_height, rf_width, rf_height, zoom);

          if (ret && image)
            DisplayPiPFrames (image, lframeStore.GetPointer (), lf_width, lf_height,
                                    rframeStore.GetPointer (), rf_width, rf_height, zoom);
        break;
      }

    gnomemeeting_threads_leave ();
  }
  return TRUE;
}


PStringList PVideoOutputDevice_GDK::GetDeviceNames() const
{
  PStringList  devlist;
  devlist.AppendString(GetDeviceName());

  return devlist;
}


BOOL PVideoOutputDevice_GDK::IsOpen ()
{
  return TRUE;
}


BOOL PVideoOutputDevice_GDK::SetFrameData (unsigned x,
					   unsigned y,
					   unsigned width,
					   unsigned height,
					   const BYTE * data,
					   BOOL endFrame)
{
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

  return EndFrame ();
}


BOOL PVideoOutputDevice_GDK::EndFrame ()
{
  GtkWidget *main_window = NULL;

  int display = 0;
  double zoom = 0.0;
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  gnomemeeting_threads_enter ();
  display = gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view");
  zoom = gm_conf_get_float (VIDEO_DISPLAY_KEY "zoom_factor");
  gnomemeeting_threads_leave ();

  /* Take the mutexes before the redraw */
  PWaitAndSignal m(redraw_mutex);

  /* Device is now open */
  if (!is_active) {
    is_active = TRUE;
    devices_nbr = PMIN (2, devices_nbr+1);
  }

  /* If there is only one device open, ignore the setting, and 
   * display what we can actually display.
   */
  if (devices_nbr <= 1) {
    if (device_id == REMOTE)
      display = REMOTE_VIDEO;
    else if (device_id == LOCAL)
      display = LOCAL_VIDEO;
  }

  if (zoom == -1.00)
    display = FULLSCREEN;

  if (zoom != 0.5 && zoom != 2.00 && zoom != 1.00)
    zoom = 1.00;

  gnomemeeting_threads_enter ();
  gm_main_window_set_display_type (main_window, display);
  gnomemeeting_threads_leave ();

  return Redraw (display, zoom);
}


BOOL PVideoOutputDevice_GDK::SetColourFormat (const PString & colour_format)
{
  if (colour_format == "RGB24") {
    return PVideoOutputDevice::SetColourFormat (colour_format);
  }

  return FALSE;  
}
