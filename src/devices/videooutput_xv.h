
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
 *                         videooutput_xv.h -  description
 *                         ----------------------------------
 *   begin                : Sun Nov 17 2006
 *   copyright            : (C) 2006-2007 by Matthias Schneider
 *                          (C) 2000-2007 by Damien Sandras
 *   description          : Class to allow video output to a XVideo
 *                          accelerated window
 *
 */


#ifndef _XVVIDEOIO_H_
#define _XVVIDEOIO_H_

#include "common.h"
#include "videooutput_gdk.h"

#include <xwindow.h>
#include <xvwindow.h>

class GMManager;

class GMVideoDisplay_XV : public GMVideoDisplay_GDK
{
public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  GMVideoDisplay_XV ();


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  virtual ~GMVideoDisplay_XV ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Setup the display following the display type,
   *                 picture dimensions and zoom value.
   *                 Returns FALSE in case of failure.
   * PRE          :  /
   */
  virtual bool SetupFrameDisplay (int display, 
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
  virtual bool FrameDisplayChangeNeeded (int display, 
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
  virtual bool CloseFrameDisplay ();

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

protected:

  XVWindow *lxvWindow;
  XVWindow *rxvWindow;

  Display *lDisplay;
  Display *rDisplay;

  GdkGC *embGC;

  bool pipWindowAvailable;
};
#endif
