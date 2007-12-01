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

  videoWidgetInfo.wasSet = FALSE;
  videoWidgetInfo.x = 0;
  videoWidgetInfo.y = 0;
  videoWidgetInfo.onTop = FALSE;
  videoWidgetInfo.disableHwAccel = FALSE;

#ifdef WIN32
  videoWidgetInfo.hwnd = NULL;
#else
  videoWidgetInfo.gc = NULL;
  videoWidgetInfo.window = 0;
  videoWidgetInfo.display = NULL;
#endif

  displayInfo = -1;
  zoomInfo = -1;

  runtime = GnomeMeeting::Process ()->GetRuntime (); // FIXME
  
  sigc::connection conn;
  conn = set_display_type.connect (sigc::ptr_fun (gm_main_window_set_display_type));
  connections.push_back (conn);

  conn = fullscreen_menu_update_sensitivity.connect (sigc::ptr_fun (gm_main_window_fullscreen_menu_update_sensitivity));
  connections.push_back (conn);

  conn = toggle_fullscreen.connect (sigc::ptr_fun (gm_main_window_toggle_fullscreen));
  connections.push_back (conn);

  conn = update_logo.connect (sigc::ptr_fun (gm_main_window_update_logo));
  connections.push_back (conn);

  conn = force_redraw.connect (sigc::ptr_fun (gm_main_window_force_redraw));
  connections.push_back (conn);

  conn = set_resized_video_widget.connect (sigc::ptr_fun (gm_main_window_set_resized_video_widget));
  connections.push_back (conn);

  conn = update_zoom_display.connect (sigc::ptr_fun (gm_main_window_update_zoom_display));
  connections.push_back (conn);

  conn = GnomeMeeting::Process ()->set_video_window.connect (sigc::mem_fun (this, &GMVideoDisplay_GDK::SetWidget));
    connections.push_back (conn);

  conn = GnomeMeeting::Process ()->set_zoom_display.connect (sigc::mem_fun (this, &GMVideoDisplay_GDK::SetDisplay));
    connections.push_back (conn);

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

  for (std::vector<sigc::connection>::iterator iter = connections.begin ();
       iter != connections.end ();
       iter++)
     iter->disconnect ();
}

void
GMVideoDisplay_GDK::Main ()
{
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
  var_mutex.Signal ();
}

void GMVideoDisplay_GDK::SetFrameData (G_GNUC_UNUSED unsigned x,
				       unsigned y,
				       unsigned width,
				       unsigned height,
				       const BYTE * data,
				       PColourConverter* setConverter,
				       bool local,
				       int devices_nbr)
{ 
  int display = 0;
  double zoom = 0.0;

  GetDisplay (&display, &zoom);

  if ((display == -1) || (zoom == -1)) {

    runtime->run_in_main (update_zoom_display.make_slot ());
    PTRACE(4, "GMVideoDisplay_GDK\tDisplay and zoom variable not set yet, not opening display");
     return;
  }

  var_mutex.Wait();

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

    runtime->run_in_main (sigc::bind (set_display_type.make_slot (), display));
  }

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
      PTRACE(4, "Updating Interval Timers: LOCAL: " << localInterval << " REMOTE: " << remoteInterval);
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
      PTRACE(4, "Updating Interval Timers: LOCAL: " << localInterval << " REMOTE: " << remoteInterval);
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
  default:
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
    if ((!local) && ((remoteInterval + 20) >= localInterval )) 
      return;
    if ((local) && ((remoteInterval + 20) < localInterval ))
      return;
    break;
  }

  frame_available_sync_point.Signal();
}

void GMVideoDisplay_GDK::SetFallback (bool newFallback)
{
  PWaitAndSignal m(var_mutex);

  fallback = newFallback;
}

bool 
GMVideoDisplay_GDK::FrameDisplayChangeNeeded (int display, 
                                              guint lf_width, 
                                              guint lf_height, 
                                              guint rf_width, 
                                              guint rf_height, 
                                              double zoom)
{
  WidgetInfo currentWidgetInfo;
  bool wasSet;

#ifdef WIN32
  wasSet = GetWidget(&currentWidgetInfo.x, &currentWidgetInfo.y, &currentWidgetInfo.hwnd, &currentWidgetInfo.onTop, &currentWidgetInfo.disableHwAccel);
#else
  wasSet = GetWidget(&currentWidgetInfo.x, &currentWidgetInfo.y, &currentWidgetInfo.gc, &currentWidgetInfo.window, &currentWidgetInfo.display, &currentWidgetInfo.onTop, &currentWidgetInfo.disableHwAccel);
#endif

  if (!wasSet) {
    PTRACE(4, "GMVideoDisplay_GDK\tWidget not yet realized, not opening display");
    return FALSE;
  }

  switch (display) {
  default:
  case LOCAL_VIDEO:
    return (lastFrame.display != LOCAL_VIDEO 
            || lastFrame.zoom != zoom || lastFrame.localWidth != lf_width || lastFrame.localHeight != lf_height 
            || currentWidgetInfo.x != lastFrame.embeddedX || currentWidgetInfo.y != lastFrame.embeddedY);
    break;

  case REMOTE_VIDEO:
    return (lastFrame.display != REMOTE_VIDEO
            || lastFrame.zoom != zoom || lastFrame.remoteWidth != rf_width || lastFrame.remoteHeight != rf_height
            || currentWidgetInfo.x != lastFrame.embeddedX || currentWidgetInfo.y != lastFrame.embeddedY);
    break;

  case PIP:
    return (lastFrame.display != display || lastFrame.zoom != zoom 
            || lastFrame.remoteWidth != rf_width || lastFrame.remoteHeight != rf_height
            || lastFrame.localWidth != lf_width || lastFrame.localHeight != lf_height
            || currentWidgetInfo.x != lastFrame.embeddedX || currentWidgetInfo.y != lastFrame.embeddedY);
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

bool 
GMVideoDisplay_GDK::SetupFrameDisplay (int display, 
                                           guint lf_width, 
                                           guint lf_height, 
                                           guint rf_width, 
                                           guint rf_height, 
                                           double zoom)
{
  WidgetInfo currentWidgetInfo;
  bool wasSet;

  if (display != PIP_WINDOW) 
    CloseFrameDisplay ();

#ifdef WIN32
  wasSet = GetWidget(&currentWidgetInfo.x, &currentWidgetInfo.y, &currentWidgetInfo.hwnd, &currentWidgetInfo.onTop, &currentWidgetInfo.disableHwAccel);
#else
  wasSet = GetWidget(&currentWidgetInfo.x, &currentWidgetInfo.y, &currentWidgetInfo.gc, &currentWidgetInfo.window, &currentWidgetInfo.display, &currentWidgetInfo.onTop, &currentWidgetInfo.disableHwAccel);
#endif

  switch (display) {
  case LOCAL_VIDEO:
    runtime->run_in_main (sigc::bind (set_resized_video_widget.make_slot (), (int) (lf_width * zoom), (int) (lf_height * zoom)));
    break;
  case REMOTE_VIDEO:
  case PIP:
    runtime->run_in_main (sigc::bind (set_resized_video_widget.make_slot (), (int) (rf_width * zoom), (int) (rf_height * zoom)));
    break;
  case FULLSCREEN:
  case PIP_WINDOW:
    runtime->run_in_main (sigc::bind (set_resized_video_widget.make_slot (), (int) (GM_QCIF_WIDTH), (int) (GM_QCIF_HEIGHT)));
    break;
  }

  if (!wasSet) {
    PTRACE(4, "GMVideoDisplay_GDK\tWidget not yet realized, not opening display");
    return TRUE;
  }

  runtime->run_in_main (sigc::bind (set_display_type.make_slot (), display));

  GtkWidget *main_window = NULL;
  main_window = GnomeMeeting::Process()->GetMainWindow ();
  if (!main_window) return TRUE;

gnomemeeting_threads_enter();
  image = gm_main_window_get_video_widget (main_window);
gnomemeeting_threads_leave();

  switch (display) {
  case LOCAL_VIDEO:
    lastFrame.display = LOCAL_VIDEO;
    lastFrame.localWidth = lf_width;
    lastFrame.localHeight = lf_height;
    lastFrame.zoom = zoom;
    lastFrame.embeddedX = currentWidgetInfo.x;
    lastFrame.embeddedY = currentWidgetInfo.y;
    break;

  case REMOTE_VIDEO:
    lastFrame.display = REMOTE_VIDEO;
    lastFrame.remoteWidth = rf_width;
    lastFrame.remoteHeight = rf_height;
    lastFrame.zoom = zoom;
    lastFrame.embeddedX = currentWidgetInfo.x;
    lastFrame.embeddedY = currentWidgetInfo.y;
    break;

  case PIP:
    lastFrame.display = PIP;
    lastFrame.localWidth = lf_width;
    lastFrame.localHeight = lf_height;
    lastFrame.remoteWidth = rf_width;
    lastFrame.remoteHeight = rf_height;
    lastFrame.zoom = zoom;
    lastFrame.embeddedX = currentWidgetInfo.x;
    lastFrame.embeddedY = currentWidgetInfo.y;
    break;

  case PIP_WINDOW:

    if (window == NULL) {
gnomemeeting_threads_enter();
      GtkWidget *vbox = NULL;

      window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

      vbox = gtk_vbox_new (FALSE, 0);
      image = gtk_image_new ();

      gtk_box_pack_start (GTK_BOX (vbox), image, TRUE, TRUE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (window), 0);

      gtk_container_add (GTK_CONTAINER (window), vbox);
      gtk_widget_realize (image);

      gtk_window_set_resizable (GTK_WINDOW (window), FALSE);

      gtk_widget_show_all (window);
      gtk_widget_set_size_request (GTK_WIDGET (image), 
                                   (int) (rf_width * zoom), 
                                   (int) (rf_height * zoom));
gnomemeeting_threads_leave();
    }

    lastFrame.remoteWidth = rf_width;
    lastFrame.remoteHeight = rf_height;
    lastFrame.localWidth = lf_width;
    lastFrame.localHeight = lf_height;
    lastFrame.zoom = zoom;

    runtime->run_in_main (update_logo.make_slot ());

    lastFrame.display = PIP_WINDOW; 
    break;

  default:
    return FALSE;
    break;
  }

  return TRUE;
}

bool 
GMVideoDisplay_GDK::CloseFrameDisplay ()
{
  if (window != NULL) {
gnomemeeting_threads_enter();
    gtk_widget_destroy (window);
gnomemeeting_threads_leave();
    window = NULL;
  }

  return TRUE;
}


void 
GMVideoDisplay_GDK::DisplayFrame ( const guchar *frame,
                                   guint width,
                                   guint height,
                                   double zoom)
{
  GdkPixbuf *pic = NULL;
  GdkPixbuf *scaled_pic = NULL;

  if (zoom != 1.00 && zoom != 2.00 && zoom != 0.5)
    zoom = 1.00;

gnomemeeting_threads_enter();
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
  gtk_image_set_from_pixbuf (GTK_IMAGE (image), 
                             GDK_PIXBUF (scaled_pic));
gnomemeeting_threads_leave();
  g_object_unref (pic);
  g_object_unref (scaled_pic);

}


void 
GMVideoDisplay_GDK::DisplayPiPFrames (    const guchar *lframe,
                                          guint lwidth,
                                          guint lheight,
                                          const guchar *rframe,
                                          guint rwidth,
                                          guint rheight,
                                          double zoom)
{
  GdkPixbuf *pic = NULL;
  GdkPixbuf *local_pic = NULL;
  GdkPixbuf *inside_pic = NULL;
  GdkPixbuf *scaled_pic = NULL;

  if (zoom != 1.00 && zoom != 2.00 && zoom != 0.5)
    zoom = 1.00;

gnomemeeting_threads_enter();
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

  gtk_image_set_from_pixbuf (GTK_IMAGE (image), 
                             GDK_PIXBUF (scaled_pic));
gnomemeeting_threads_leave();
  g_object_unref (pic);
  g_object_unref (scaled_pic);
  g_object_unref (local_pic);
  g_object_unref (inside_pic);

}

void GMVideoDisplay_GDK::Redraw ()
{
  bool ret = TRUE;
    if (FrameDisplayChangeNeeded (currentFrame.display, currentFrame.localWidth, currentFrame.localHeight, 
                                  currentFrame.remoteWidth, currentFrame.remoteHeight, currentFrame.zoom)) 
      ret = SetupFrameDisplay (currentFrame.display, currentFrame.localWidth, currentFrame.localHeight, 
                                  currentFrame.remoteWidth, currentFrame.remoteHeight, currentFrame.zoom); 

    switch (currentFrame.display) 
      {
      case LOCAL_VIDEO:
          if (ret && (lframeStore.GetSize() > 0 ))
            DisplayFrame (lframeStore.GetPointer (), currentFrame.localWidth, currentFrame.localHeight, currentFrame.zoom);
        break;

      case REMOTE_VIDEO:
          if (ret && (rframeStore.GetSize () > 0))
            DisplayFrame (rframeStore.GetPointer (), currentFrame.remoteWidth, currentFrame.remoteHeight, currentFrame.zoom);
        break;

      case FULLSCREEN:
      case PIP:
      case PIP_WINDOW:
          if (ret && (lframeStore.GetSize() > 0) && (rframeStore.GetSize () > 0))
            DisplayPiPFrames (lframeStore.GetPointer (), currentFrame.localWidth, currentFrame.localHeight,
                              rframeStore.GetPointer (), currentFrame.remoteWidth, currentFrame.remoteHeight, currentFrame.zoom);
        break;
      }

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
  PBYTEArray tempBuffer; // cannot do conversion in place

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

void GMVideoDisplay_GDK::SetDisplay (int display, double zoom) 
{
  PWaitAndSignal m(widget_data_mutex);

  if (display != -1)
    displayInfo = display;
    
  if (zoom != -1)
    zoomInfo = zoom;
}

void GMVideoDisplay_GDK::GetDisplay (int* display, double* zoom) 
{
  PWaitAndSignal m(widget_data_mutex);
  *display = displayInfo;
  *zoom = zoomInfo;
}

void GMVideoDisplay_GDK::SetWidget (GtkWidget* video_image, bool on_top, bool disable_hw_accel) 
{
  PWaitAndSignal m(widget_data_mutex);

  if (!GTK_WIDGET_REALIZED (video_image))
    return;

  videoWidgetInfo.wasSet = TRUE;

  videoWidgetInfo.x = video_image->allocation.x;
  videoWidgetInfo.y = video_image->allocation.y;
  videoWidgetInfo.onTop = on_top;
  videoWidgetInfo.disableHwAccel = disable_hw_accel;

#ifdef WIN32
  videoWidgetInfo.hwnd = ((HWND)GDK_WINDOW_HWND (video_image->window));
#else
  if (!videoWidgetInfo.gc)
    videoWidgetInfo.gc = GDK_GC_XGC(gdk_gc_new(video_image->window));
  videoWidgetInfo.window = GDK_WINDOW_XWINDOW (video_image->window);
  videoWidgetInfo.display = GDK_DISPLAY ();
#endif

}

#ifdef WIN32
bool GMVideoDisplay_GDK::GetWidget(int* x, int* y, HWND* hwnd, bool* on_top, bool* disable_hw_accel)
{
  PWaitAndSignal m(widget_data_mutex);
  *x = videoWidgetInfo.x;
  *y = videoWidgetInfo.y;
  *hwnd = videoWidgetInfo.hwnd;
  *on_top = videoWidgetInfo.onTop;
  *disable_hw_accel = videoWidgetInfo.disableHwAccel;
  return videoWidgetInfo.wasSet;
}
#else
bool GMVideoDisplay_GDK::GetWidget(int* x, int* y, GC* gc, Window* window, Display** display, bool* on_top, bool* disable_hw_accel)
{
  PWaitAndSignal m(widget_data_mutex);
  *x = videoWidgetInfo.x;
  *y = videoWidgetInfo.y;
  *gc = videoWidgetInfo.gc;
  *window = videoWidgetInfo.window;
  *display = videoWidgetInfo.display;
  *on_top = videoWidgetInfo.onTop;
  *disable_hw_accel = videoWidgetInfo.disableHwAccel;
  return videoWidgetInfo.wasSet;
}
#endif

