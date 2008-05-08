
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
 *                         display-manager-common.h  -  description
 *                         ------------------------------
 *   begin                : Sat Feb 17 2001
 *   copyright            : (C) 2000-2008 by Damien Sandras
 *                        : (C) 2007-2008 by Matthias Schneider
 *   description          : GMVideoManager: Generic class that represents 
 *                            a thread that can display a video image and defines.
 *                            generic functions for local/remote/pip/pip external 
 *                            window/fullscreen video display. Provides interface 
 *                            to the GUI for an embedded window, display mode 
 *                            control and feedback of information like the status
 *                            of the video acceleration. Also provides the 
 *                            copying and local storage of the video frame.
 */


#ifndef _VIDEODISPLAY_H_
#define _VIDEODISPLAY_H_

#include "display-manager.h"
#include "runtime.h"
//FIXME
#include "ptbuildopts.h"
#include "ptlib.h"


typedef struct {
  DisplayMode display;
  unsigned int remote_width;
  unsigned int remote_height;

  unsigned int local_width;
  unsigned int local_height;
  
  unsigned int zoom;

  int embedded_x;
  int embedded_y;
} FrameInfo;

typedef struct {
  bool local;
  bool remote;  
} UpdateRequired;


class GMDisplayManager
   : public PThread,
     public Ekiga::DisplayManager
{
  PCLASSINFO(GMDisplayManager, PThread); 
public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Initialises the VideoDisplay_embedded.
   * PRE          :  /
   */
  GMDisplayManager (Ekiga::ServiceCore & core);


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
  */
  virtual ~GMDisplayManager (void);

  virtual void start ();

  virtual void stop ();

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Copy the frame data to a local structur and signal 
   *                 the thread to display it.
   * PRE          :  width and height (>=0),
   *                 the data
   *                 a boolean whether frame is a local or remote one
   *                 and the number of opened devices .
   */
   virtual void set_frame_data (unsigned width,
                                unsigned height,
                                const char* data,
                                bool local,
                                int devices_nbr);


   virtual void set_display_info (const DisplayInfo & _display_info)
   {
      PWaitAndSignal m(display_info_mutex);
      display_info = _display_info;
   };


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
  virtual bool frame_display_change_needed ();

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Setup the display following the display type,
   *                 picture dimensions and zoom value.
   *                 Returns FALSE in case of failure.
   * PRE          :  /
   */
  virtual void setup_frame_display () = 0;

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Closes the frame display and returns FALSE 
   *                 in case of failure.
   * PRE          :  /
   */
  virtual void close_frame_display () = 0;

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Display the given frame on the correct display.
   * PRE          :  The display needs to be initialized using 
   *                 SetupFrameDisplay. 
   */
  virtual void display_frame (const char *frame,
                              unsigned width,
                              unsigned height) = 0;


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Display the given frames as Picture in Picture.
   * PRE          :  The display needs to be initialized using 
   *                 SetupFrameDisplay. 
   */
  virtual void display_pip_frames (const char *local_frame,
                                   unsigned lf_width,
                                   unsigned lf_height,
                                   const char *remote_frame,
                                   unsigned rf_width,
                                   unsigned rf_height) = 0;

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Draw the frame via the Display* functions
   * PRE          :  /
   */
  virtual UpdateRequired redraw ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Sync the output of the frame to the display
   * PRE          :  /
   */
  virtual void sync(UpdateRequired sync_required) = 0;

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Initialises the display
   * PRE          :  /
   */
  virtual void init ();

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Uninitialises the frame containers
   * PRE          :  /
   */
  virtual void uninit ();

  virtual void get_display_info (DisplayInfo & _display_info) {
        PWaitAndSignal m(display_info_mutex);
        _display_info = display_info;
  }

  /* This variable has to be protected by display_info_mutex */
  DisplayInfo display_info;
  PMutex display_info_mutex; /* To protect the DisplayInfo object */

  PBYTEArray lframeStore;
  PBYTEArray rframeStore;

  FrameInfo last_frame;
  FrameInfo current_frame;
  
  bool first_frame_received;
  bool video_disabled;
  UpdateRequired update_required;

  PSyncPoint run_thread;                  /* To signal the thread shall execute its tasks */
  bool end_thread;
  bool init_thread;
  bool uninit_thread;

  PSyncPoint thread_created;              /* To signal that the thread has been created */
  PSyncPoint thread_initialised;          /* To signal that the thread has been initialised */
  PSyncPoint thread_uninitialised;        /* To signal that the thread has been uninitialised */
  PMutex     thread_ended;                /* To exit */
  
  PMutex var_mutex;      /* To protect variables that are read and written
			    from various threads */

  Ekiga::ServiceCore & core;
  Ekiga::Runtime & runtime;
};

#endif /* VIDEODISPLAY */
