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
 *                         videooutput_gdk.cpp  -  description
 *                         ------------------------------
 *   begin                : Sat Feb 17 2001
 *   copyright            : (C) 2000-2007 by Damien Sandras
 *                        : (C)      2007 by Matthias Schneider
 *   description          : Class to permit to display in GDK Drawing Area
 *
 */


#include "config.h"

#define P_FORCE_STATIC_PLUGIN 

#include "videooutput_gdk.h"
#include "ekiga.h"
#include "misc.h"
#include "main.h"

#include "gmconf.h"

#include <ptlib/vconvert.h>

/* The functions */
GMVideoDisplay_GDK::GMVideoDisplay_GDK ()
  : PThread (1000, NoAutoDeleteThread)
{
  /* Variables */

  /* State for last frame */
  lastFrame.display = 99;
  lastFrame.localWidth = 0;
  lastFrame.localHeight = 0;
  lastFrame.remoteWidth = 0;
  lastFrame.remoteHeight = 0;  
  lastFrame.zoom = 99;
  lastFrame.embeddedX = 0;
  lastFrame.embeddedY = 0;  

  currentFrame.localWidth = 0;
  currentFrame.localHeight = 0;
  currentFrame.remoteWidth = 0;
  currentFrame.remoteHeight = 0;

  localInterval = -21;
  remoteInterval = -21;
  lastLocalIntervalTime = PTime();
  lastRemoteIntervalTime = PTime();
  numberOfLocalFrames = 0;
  numberOfRemoteFrames = 0;

  fallback = FALSE;

  window = NULL;
  image = NULL;

  /* Initialisation */
  stop = FALSE;
  first_frame_received = FALSE;

  this->Resume ();
  thread_sync_point.Wait ();
}

GMVideoDisplay_GDK::~GMVideoDisplay_GDK ()
{
  /* Check if it hasnt been stopped already by a derived class */
  if (!stop) {

    stop = TRUE;

    /* Wait for the Main () method to be terminated */
    frame_available_sync_point.Signal();
    PWaitAndSignal m(quit_mutex);
  }

  /* This is common to all output classes */
  lframeStore.SetSize (0);
  rframeStore.SetSize (0);
}

void
GMVideoDisplay_GDK::Main ()
{
  GtkWidget *main_window = NULL;

  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();

  while (!stop) {
    frame_available_sync_point.Wait(250);
    var_mutex.Wait ();
    if (first_frame_received)
      Redraw();
    var_mutex.Signal ();
  }

  var_mutex.Wait ();
  CloseFrameDisplay ();
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  var_mutex.Signal ();
}

void GMVideoDisplay_GDK::SetFrameData (G_GNUC_UNUSED unsigned x,
				       G_GNUC_UNUSED unsigned y,
				       unsigned width,
				       unsigned height,
				       const BYTE * data,
				       PColourConverter* setConverter,
				       BOOL local,
				       int devices_nbr
)
{ 
  GtkWidget *main_window = NULL;

  int display = 0;
  double zoom = 0.0;

  var_mutex.Wait();

  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  gnomemeeting_threads_enter ();
  display = gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view");
  zoom = gm_conf_get_float (VIDEO_DISPLAY_KEY "zoom_factor");
  gnomemeeting_threads_leave ();

  if (zoom != 0.5 && zoom != 2.00 && zoom != 1.00)
    zoom = 1.00;

  /* If there is only one device open, ignore the setting, and 
   * display what we can actually display.
   */
  if (devices_nbr <= 1) {
    if (!local)
      display = REMOTE_VIDEO;
    else 
      display = LOCAL_VIDEO;
  }

  gnomemeeting_threads_enter ();
  gm_main_window_set_display_type (main_window, display);
  gnomemeeting_threads_leave ();

  converter = setConverter;
  currentFrame.display = display;
  currentFrame.zoom = zoom;
  first_frame_received = TRUE;

  if (local) {

    /* statistics to find if remote or local stream has higher fps */
    numberOfLocalFrames++;
    if ((numberOfLocalFrames % 5) == 0) {

      localInterval = (PTime() - lastLocalIntervalTime).GetMilliSeconds();
      lastLocalIntervalTime = PTime();
      PTRACE(1, "Updating Interval Timers: LOCAL: " << localInterval << " REMOTE: " << remoteInterval);
    }

    /* convert or memcpy the frame */
    lframeStore.SetSize (width * height * 3);
    currentFrame.localWidth = width;
    currentFrame.localHeight= height;

    if (fallback) {

      if (converter)
        converter->Convert (data, lframeStore.GetPointer ());
    }
    else {

      memcpy (lframeStore.GetPointer(), data, (width * height * 3) >> 1);
    }
  }
  else {

    /* statistics to find if remote or local stream has higher fps */
    numberOfRemoteFrames++;
    if ((numberOfRemoteFrames % 5) == 0) {

      remoteInterval = (PTime() - lastRemoteIntervalTime).GetMilliSeconds();
      lastRemoteIntervalTime = PTime();
      PTRACE(1, "Updating Interval Timers: LOCAL: " << localInterval << " REMOTE: " << remoteInterval);
    }

    /* convert or memcpy the frame */
    rframeStore.SetSize (width * height * 3);
    currentFrame.remoteWidth = width;
    currentFrame.remoteHeight= height;

    if (fallback) {

      if (converter)
        converter->Convert (data, rframeStore.GetPointer ());
    }
    else {

      memcpy (rframeStore.GetPointer(), data, (width * height * 3) >> 1);
    }
  }
  var_mutex.Signal();

  switch (display) {
  case LOCAL_VIDEO:
    if (!local)
      return;
    break;
  case REMOTE_VIDEO:
    if (local)
      return;
    break;
  case PIP:
  case PIP_WINDOW:
  case FULLSCREEN:
  default:
    if ((!local) && ((remoteInterval + 20) >= localInterval )) 
      return;
    if ((local) && ((remoteInterval + 20) < localInterval ))
      return;
    break;
  }

  frame_available_sync_point.Signal();
}

void GMVideoDisplay_GDK::SetFallback (BOOL newFallback)
{
  PWaitAndSignal m(var_mutex);

  fallback = newFallback;
}

BOOL 
GMVideoDisplay_GDK::FrameDisplayChangeNeeded (int display,
                                              guint lf_width,
                                              guint lf_height,
                                              guint rf_width,
                                              guint rf_height,
                                              double zoom)
{
  GtkWidget *main_window = NULL;
  GtkWidget *_image = NULL;

  main_window = GnomeMeeting::Process()->GetMainWindow ();
  _image = gm_main_window_get_video_widget (main_window);

  switch (display) {
  case LOCAL_VIDEO:
    return (lastFrame.display != LOCAL_VIDEO 
            || lastFrame.zoom != zoom || lastFrame.localWidth != lf_width || lastFrame.localHeight != lf_height 
            || _image->allocation.x != lastFrame.embeddedX || _image->allocation.y != lastFrame.embeddedY);
    break;

  case REMOTE_VIDEO:
    return (lastFrame.display != REMOTE_VIDEO
            || lastFrame.zoom != zoom || lastFrame.remoteWidth != rf_width || lastFrame.remoteHeight != rf_height
            || _image->allocation.x != lastFrame.embeddedX || _image->allocation.y != lastFrame.embeddedY);
    break;

  case PIP:
    return (lastFrame.display != display || lastFrame.zoom != zoom 
            || lastFrame.remoteWidth != rf_width || lastFrame.remoteHeight != rf_height
            || lastFrame.localWidth != lf_width || lastFrame.localHeight != lf_height
            || _image->allocation.x != lastFrame.embeddedX || _image->allocation.y != lastFrame.embeddedY);
    break;
  case PIP_WINDOW:
  case FULLSCREEN:
  default:
    return (lastFrame.display != display || lastFrame.zoom != zoom 
            || lastFrame.remoteWidth != rf_width || lastFrame.remoteHeight != rf_height
            || lastFrame.localWidth != lf_width || lastFrame.localHeight != lf_height);
    break;
  }

  return FALSE;
}

BOOL 
GMVideoDisplay_GDK::SetupFrameDisplay (int display, 
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
  PTRACE (4, "GMVideoDisplay\tSetup display " << display << " with zoom value of " << zoom);

  return TRUE;
}

BOOL 
GMVideoDisplay_GDK::CloseFrameDisplay ()
{
  if (window != NULL) {
    gtk_widget_destroy (window);
    window = NULL;
    image = NULL;
  }

  return TRUE;
}


void 
GMVideoDisplay_GDK::DisplayFrame (gpointer gtk_image,
				  const guchar *frame,
				  guint width,
				  guint height,
				  double zoom)
{
  GdkPixbuf *pic = NULL;
  GdkPixbuf *scaled_pic = NULL;

  if (!gtk_image)
    return;

  if (zoom != 1.00 && zoom != 2.00 && zoom != 0.5)
    zoom = 1.00;

  pic = gdk_pixbuf_new_from_data (frame, GDK_COLORSPACE_RGB, 
                                  FALSE, 8, 
                                  width, 
                                  height, 
                                  width * 3, 
                                  NULL, NULL);

  scaled_pic = gdk_pixbuf_scale_simple (pic,
                                        (int) (width * zoom),
                                        (int) (height * zoom),
                                        GDK_INTERP_NEAREST);
  gtk_image_set_from_pixbuf (GTK_IMAGE (gtk_image), 
                             GDK_PIXBUF (scaled_pic));
  g_object_unref (pic);
  g_object_unref (scaled_pic);

}


void 
GMVideoDisplay_GDK::DisplayPiPFrames (gpointer gtk_image,
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

  if (!gtk_image)
    return;

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
                         (int) (rwidth * 2 / 3), 
                         (int) (rheight * 2 / 3));
  if (zoom != 1.0) {

    scaled_pic = gdk_pixbuf_scale_simple (pic,
                                          (int) (rwidth * zoom),
                                          (int) (rheight * zoom),
                                          GDK_INTERP_NEAREST);

  }
  else {

    scaled_pic = gdk_pixbuf_copy (pic);
  }

  gtk_image_set_from_pixbuf (GTK_IMAGE (gtk_image), 
                             GDK_PIXBUF (scaled_pic));
  g_object_unref (pic);
  g_object_unref (scaled_pic);
  g_object_unref (local_pic);
  g_object_unref (inside_pic);

}

void GMVideoDisplay_GDK::Redraw ()
{
  BOOL ret = TRUE;
    gnomemeeting_threads_enter ();
    if (FrameDisplayChangeNeeded (currentFrame.display, currentFrame.localWidth, currentFrame.localHeight, 
                                  currentFrame.remoteWidth, currentFrame.remoteHeight, currentFrame.zoom)) 
      ret = SetupFrameDisplay (currentFrame.display, currentFrame.localWidth, currentFrame.localHeight, 
                                  currentFrame.remoteWidth, currentFrame.remoteHeight, currentFrame.zoom); 

    switch (currentFrame.display) 
      {
      case LOCAL_VIDEO:
          if (ret && (lframeStore.GetSize() > 0 ))
            DisplayFrame (image, lframeStore.GetPointer (), currentFrame.localWidth, currentFrame.localHeight, currentFrame.zoom);
        break;

      case REMOTE_VIDEO:
          if (ret && (rframeStore.GetSize () > 0))
            DisplayFrame (image, rframeStore.GetPointer (), currentFrame.remoteWidth, currentFrame.remoteHeight, currentFrame.zoom);
        break;

      case FULLSCREEN:
      case PIP:
      case PIP_WINDOW:
      default:
          if (ret && (lframeStore.GetSize() > 0) && (rframeStore.GetSize () > 0))
            DisplayPiPFrames (image, lframeStore.GetPointer (), currentFrame.localWidth, currentFrame.localHeight,
                                     rframeStore.GetPointer (), currentFrame.remoteWidth, currentFrame.remoteHeight, currentFrame.zoom);
        break;
      }

    gnomemeeting_threads_leave ();

  if (!ret) {

    PTRACE (4, "GMVideoDisplay\tFalling back to GDK Rendering");
    doFallback();
    Redraw();
  }

  return;
}

void
GMVideoDisplay_GDK::doFallback ()
{
  GtkWidget *main_window = NULL;
  PBYTEArray tempBuffer; // cannot do conversion in place

  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  gnomemeeting_threads_enter ();
  gm_main_window_fullscreen_menu_update_sensitivity (main_window, FALSE); 
  gnomemeeting_threads_leave ();

  fallback = TRUE;
  if (lframeStore.GetSize() > 0 ) {

    tempBuffer.SetSize (currentFrame.localWidth * currentFrame.localHeight * 3);
    memcpy (tempBuffer.GetPointer(), lframeStore.GetPointer(), (currentFrame.localWidth * currentFrame.localHeight * 3) >> 1); 
    if (converter)

      converter->Convert (tempBuffer.GetPointer(), lframeStore.GetPointer ());
  }

  if (rframeStore.GetSize() > 0 ) {

    tempBuffer.SetSize (currentFrame.remoteWidth * currentFrame.remoteHeight * 3);
    memcpy (tempBuffer.GetPointer(), rframeStore.GetPointer(), (currentFrame.remoteWidth * currentFrame.remoteHeight * 3) >> 1); 
    if (converter)

      converter->Convert (tempBuffer.GetPointer(), rframeStore.GetPointer ());
  }
}
