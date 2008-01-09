
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
 *                         videodisplay.cpp  -  description
 *                         ------------------------------
 *   begin                : Sat Feb 17 2001
 *   copyright            : (C) 2000-2007 by Damien Sandras
 *                        : (C)      2007 by Matthias Schneider
 *   description          : GMVideoDisplay: Generic class that represents 
 *                            a thread that can display a video image.
 *                          GMVideoDisplay_embedded: Class that supports generic
 *                            functions for local/remote/pip/pip external 
 *                            window/fullscreen video display. Provides interface 
 *                            to the GUI for an embedded window, display mode 
 *                            control and feedback of information like the status
 *                            of the video acceleration. Also provides the 
 *                            interface to the PVideoOutputDevice_EKIGA and 
 *                            implements copying and local storage of the video 
 *                            frame.
 */


#ifndef _VIDEODISPLAY_H_
#define _VIDEODISPLAY_H_

#include "common.h"
#include "ekiga.h"
#include <sigc++/sigc++.h>

class GMManager;

typedef struct _FrameInfo
{
  VideoMode display;
  unsigned int remoteWidth;
  unsigned int remoteHeight;

  unsigned int localWidth;
  unsigned int localHeight;
  
  unsigned int zoom;

  int embeddedX;
  int embeddedY;
} FrameInfo;

typedef struct _UpdateRequired
{
  bool local;
  bool remote;  
} UpdateRequired;

class GMVideoDisplay : public PThread
{
  PCLASSINFO(GMVideoDisplay, PThread);

 public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Initialises the VideoDisplay.
   * PRE          :  /
   */
  GMVideoDisplay (Ekiga::ServiceCore & core);


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  virtual ~GMVideoDisplay (void);

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Copy the frame data to a local structur and signal 
   *                 the thread to display it.
   * PRE          :  width and height (>=0),
   *                 the data
   *                 a boolean whether frame is a local or remote one
   *                 and the number of opened devices .
   */
  virtual void SetFrameData (unsigned width,
                             unsigned height,
                             const BYTE *data,
                             bool local,
                             int devices_nbr) = 0;
  /* DESCRIPTION  :  Update all information about the video_image widget,
                     gconf-settings and display type and zoom settings..
   *                 Needed for embedded window 
   * BEHAVIOR     :  Called when the video_image widget receives an expose event,
   *                 when a parameter like zoom or display type is change and
   *                 once before the display i opened to initialize all settings
   *                 obtained from gconf.
   * PRE          :  A VideoInfo object with all the updated information
   */
  virtual void SetVideoInfo (VideoInfo* newVideoInfo);

 protected:

  /* Callbacks for functions that have to be
     executed in the main thread  */
  sigc::signal<void,
               VideoMode> set_display_type;                 /* gm_main_window_set_display_type */

  sigc::signal<void,
               int> fullscreen_menu_update_sensitivity;     /* gm_main_window_fullscreen_menu_update_sensitivity */

  sigc::signal<void,
               FSToggle> toggle_fullscreen;                 /* gm_main_window_toggle_fullscreen */

  sigc::signal<void,
               int,
               int> set_resized_video_widget;               /* gm_main_window_set_resized_video_widget */
	       
  sigc::signal<void> update_logo;                           /* gm_main_window_update_logo  */
  sigc::signal<void> force_redraw;                          /* gm_main_window_force_redraw */
  sigc::signal<void> update_zoom_display;                   /* gm_main_window_update_zoom_display */
  sigc::signal<void,
               VideoAccelStatus> update_video_accel_status; /* gm_main_window_update_video_accel_status */

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Copy the VideoInfo information to a local variable 
                     in a protected manner
   * PRE          :  The protected VideoInfo has been set via SetVideoInfo.
   */
  virtual void GetVideoInfo (VideoInfo* getVideoInfo);

  VideoStats videoStats;

  PMutex video_info_mutex; /* To protect the VideoInfo object*/

  /* This variable has to be protected by video_info_mutex */
  VideoInfo videoInfo;

  Ekiga::Runtime* runtime;
  Ekiga::ServiceCore & core;
};


class GMVideoDisplay_embedded : public GMVideoDisplay
{
 public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Initialises the VideoDisplay_embedded.
   * PRE          :  /
   */
  GMVideoDisplay_embedded (Ekiga::ServiceCore & core);


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
    ==============================================================================
    ATTENTION: When deriving from this class, ALWAYS make sure that the 
    destructor of the derived class waits for the thread to terminate -
    if the thread is still running after the derived class' destructor finishes, 
    all virtual functions will be switched back to this base class where they are 
    purely virtual, leading to severe runtime issues.
    ==============================================================================
  */
  virtual ~GMVideoDisplay_embedded (void);

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Copy the frame data to a local structur and signal 
   *                 the thread to display it.
   * PRE          :  width and height (>=0),
   *                 the data
   *                 a boolean whether frame is a local or remote one
   *                 and the number of opened devices .
   */
   virtual void SetFrameData (unsigned width,
                             unsigned height,
                             const BYTE *data,
                             bool local,
                             int devices_nbr);


 protected:

  /* DESCRIPTION  :  The video thread loop that waits for a frame and
   *                 then displays it
   * BEHAVIOR     :  Loops and displays
   * PRE          :  /
   */
  virtual void Main (void);

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns TRUE if the given settings require a
   *                 reinitialization of the display, FALSE 
   *                 otherwise.
   * PRE          :  /
   */
  virtual bool FrameDisplayChangeNeeded (VideoMode display, 
                                         guint lf_width, 
                                         guint lf_height, 
                                         guint rf_width, 
                                         guint rf_height, 
                                         unsigned int zoom);

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Setup the display following the display type,
   *                 picture dimensions and zoom value.
   *                 Returns FALSE in case of failure.
   * PRE          :  /
   */
  virtual void SetupFrameDisplay (VideoMode display, 
                                  guint lf_width, 
                                  guint lf_height, 
                                  guint rf_width, 
                                  guint rf_height, 
                                  unsigned int zoom) = 0;

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Closes the frame display and returns FALSE 
   *                 in case of failure.
   * PRE          :  /
   */
  virtual bool CloseFrameDisplay () = 0;

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Display the given frame on the correct display.
   * PRE          :  The display needs to be initialized using 
   *                 SetupFrameDisplay. 
   */
  virtual void DisplayFrame (const guchar *frame,
                             guint width,
                             guint height) = 0;


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
                                 guint rheight) = 0;

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Draw the frame via the Display* functions
   * PRE          :  /
   */
  virtual UpdateRequired Redraw ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Sync the output of the frame to the display
   * PRE          :  /
   */
  virtual void Sync(UpdateRequired sync_required) = 0;

  PBYTEArray lframeStore;
  PBYTEArray rframeStore;

  FrameInfo lastFrame;
  FrameInfo currentFrame;

  unsigned rxFrames;
  unsigned txFrames;
  
  bool stop;
  bool first_frame_received;
  bool video_disabled;
  UpdateRequired update_required;

  PMutex var_mutex;      /* To protect variables that are read and written
			    from various threads */
  PMutex quit_mutex;     /* To exit */

  PSyncPoint frame_available_sync_point;     /* To signal a new frame has to be displayed  */
  PSyncPoint thread_sync_point;              /* To signal that the thread has been created */

  std::vector<sigc::connection> connections; //FIXME
  
  PTime lastStats;

};

#endif /* VIDEODISPLAY */
