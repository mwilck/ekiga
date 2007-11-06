
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 *                         videooutput_gdk.h  -  description
 *                         ----------------------------
 *   begin                : Sat Feb 17 2001
 *   copyright            : (C) 2000-2007 by Damien Sandras
 *                          (C)      2007 by Matthias Schneider
 *   description          : Class to permit to display in GDK Drawing Area
 *
 */


#ifndef _GDKVIDEOIO_H_
#define _GDKVIDEOIO_H_

#include "common.h"
#include "ekiga.h"
#include <sigc++/sigc++.h>

#ifdef WIN32
#include <gdk/gdkwin32.h>
#else
#include <gdk/gdkx.h>
#endif

class GMManager;

typedef struct _FrameInfo
{
  int display;
  unsigned int remoteWidth;
  unsigned int remoteHeight;

  unsigned int localWidth;
  unsigned int localHeight;
  
  double zoom;

  int embeddedX;
  int embeddedY;
} FrameInfo;

typedef struct _WidgetInfo
{
  BOOL wasSet;
  int x;
  int y;
  BOOL onTop;
  BOOL disableHwAccel;

#ifdef WIN32
  HWND hwnd;
#else
  GC gc;
  Window window;
  Display* display;
#endif

} WidgetInfo;

class GMVideoDisplay_GDK : public PThread
{
  PCLASSINFO(GMVideoDisplay_GDK, PThread);

 public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Initialises the VideoGrabber, the VideoGrabber is opened
   *                 asynchronously given the config parameters. If the opening
   *                 fails, an error popup is displayed.
   * PRE          :  First parameter is TRUE if the VideoGrabber must grab
   *                 once opened. The second one is TRUE if the VideoGrabber
   *                 must be opened synchronously. The last one is a 
   *                 reference to the GMManager.
   */
  GMVideoDisplay_GDK ();


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  virtual ~GMVideoDisplay_GDK (void);

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  If data for the end frame is received, then we convert
   *                 it to the correct colour format and we display it.
   * PRE          :  x and y positions in the picture (>=0),
   *                 width and height (>=0),
   *                 the data, and a boolean indicating if it is the end
   *                 frame or not.
   */
  virtual void SetFrameData (unsigned x,
                             unsigned y,
                             unsigned width,
                             unsigned height,
                             const BYTE *data,
                             PColourConverter* setConverter,
                             BOOL local,
                             int devices_nbr);

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  If data for the end frame is received, then we convert
   *                 it to the correct colour format and we display it.
   * PRE          :  x and y positions in the picture (>=0),
   *                 width and height (>=0),
   *                 the data, and a boolean indicating if it is the end
   *                 frame or not.
   */
  virtual void SetFallback  (BOOL newFallback);

  /* DESCRIPTION  :  Update all information about the video_image widget.
   *                 Needed for embedded window 
   * BEHAVIOR     :  Called when the video_image widget receives an expose event
   * PRE          :  Pointer to the widget, and variables on_top and disable_hw_accel
   */
  virtual void SetWidget (GtkWidget* video_image, BOOL on_top, BOOL disable_hw_accel);

  /* DESCRIPTION  :  Callback to update the display and zoom variables.
   * BEHAVIOR     :  Called if the display variable or zoom level is changed.
   * PRE          :  Display ID and zoom level.
   */
  virtual void SetDisplay (int display, double zoom);

 protected:
  void Main (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Setup the display following the display type,
   *                 picture dimensions and zoom value.
   *                 Returns FALSE in case of failure.
   * PRE          :  /
   */
  virtual BOOL SetupFrameDisplay (int display, 
                                  guint lf_width, 
                                  guint lf_height, 
                                  guint rf_width, 
                                  guint rf_height, 
                                  double zoom);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns TRUE if the given settings require a
   *                 reinitialization of the display, FALSE 
   *                 otherwise.
   * PRE          :  /
   */
  virtual BOOL FrameDisplayChangeNeeded (int display, 
                                         guint lf_width, 
                                         guint lf_height, 
                                         guint rf_width, 
                                         guint rf_height, 
                                         double zoom);

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Closes the frame display and returns FALSE 
   *                 in case of failure.
   * PRE          :  /
   */
  virtual BOOL CloseFrameDisplay ();

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Display the given frame on the correct display.
   * PRE          :  The display needs to be initialized using 
   *                 SetupFrameDisplay. 
   */
  virtual void DisplayFrame (const guchar *frame,
                             guint width,
                             guint height,
                             double zoom);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Display the given frames as Picture in Picture.
   * PRE          :  The display needs to be initialized using 
   *                 SetupFrameDisplay. 
   */
  virtual void DisplayPiPFrames (const guchar *lframe,
                                 guint lwidth,
                                 guint lheight,
                                 const guchar *rframe,
                                 guint rwidth,
                                 guint rheight,
                                 double zoom);

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Redraw the frame.
   * PRE          :  /
   */
  virtual void Redraw ();

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Used by derived functions to convert current
   *                 frame to RGB
   * PRE          :  /
   */
  virtual void doFallback ();

  PBYTEArray lframeStore;
  PBYTEArray rframeStore;

  PColourConverter* converter;

  FrameInfo lastFrame;
  FrameInfo currentFrame;

  /* Only for GDK output (depreciated) */
  GtkWidget* window;
  GtkWidget* image;

  BOOL fallback;
  BOOL stop;
  BOOL first_frame_received;

  PMutex var_mutex;      /* To protect variables that are read and written
			    from various threads */
  PMutex quit_mutex;     /* To exit */
  PMutex widget_data_mutex;

  PSyncPoint frame_available_sync_point;     /* To signal a new frame has to be displayed  */
  PSyncPoint thread_sync_point;              /* To signal that the thread has been created */

  /* For fps statistics */
  PTime lastLocalIntervalTime;
  PTime lastRemoteIntervalTime;
  unsigned localInterval;
  unsigned remoteInterval;
  unsigned numberOfLocalFrames;
  unsigned numberOfRemoteFrames;

  /* Callbacks for functions that have to be
     executed in the main thread  */
  sigc::signal<void,
               int> set_display_type;                   /* gm_main_window_set_display_type */

  sigc::signal<void,
               int> fullscreen_menu_update_sensitivity; /* gm_main_window_fullscreen_menu_update_sensitivity */

  sigc::signal<void,
               int> toggle_fullscreen;                  /* gm_main_window_toggle_fullscreen */

  sigc::signal<void,
               int,
               int> set_resized_video_widget;          /* gm_main_window_set_resized_video_widget */
	       
  sigc::signal<void> update_logo;                      /* gm_main_window_update_logo  */
  sigc::signal<void> force_redraw;                     /* gm_main_window_force_redraw */
  sigc::signal<void> update_zoom_display;              /* gm_main_window_update_zoom_display */

  std::vector<sigc::connection> connections;

#ifdef WIN32
  virtual BOOL GetWidget(int* x, int* y, HWND* hwnd, BOOL* on_top, BOOL* disable_hw_accel);
#else
  virtual BOOL GetWidget(int* x, int* y, GC* gc, Window* window, Display** display, BOOL* on_top, BOOL* disable_hw_accel);
#endif

  void GetDisplay (int* display, double* zoom);
  
  /* These variables have to be protected by widget_data_mutex */
  WidgetInfo videoWidgetInfo;
  int displayInfo;
  double zoomInfo;
  
  
  Ekiga::Runtime* runtime;
};

#endif
