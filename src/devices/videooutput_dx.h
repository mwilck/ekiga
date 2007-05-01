
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
 *                         videooutput_dx.h -  description
 *                         ----------------------------------
 *   begin                : Sun Nov 17 2006
 *   copyright            : (C) 2006 by Matthias Schneider
 *                          (C) 2000-2007 by Damien Sandras
 *   description          : Class to allow video output to a DirectX
 *                          accelerated window
 *
 */


#ifndef _DXVIDEOIO_H_
#define _DXVIDEOIO_H_

#include "common.h"
#include "videooutput_gdk.h"

#include <dxwindow.h>

class GMManager;


class PVideoOutputDevice_DX : public PVideoOutputDevice_GDK
{
  PCLASSINFO(PVideoOutputDevice_DX, PVideoOutputDevice_GDK);

public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  PVideoOutputDevice_DX ();


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~PVideoOutputDevice_DX ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Open the device given the device name.
   * PRE          :  Device name to open, immediately start device.
   */
  virtual BOOL Open (const PString &name,
                     BOOL unused);


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
   * BEHAVIOR     :  Closes the frame display and returns FALSE 
   *                 in case of failure.
   * PRE          :  /
   */
  virtual BOOL CloseFrameDisplay ();

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  If data for the end frame is received, then we convert
   *                 it to the correct colour format and we display it.
   * PRE          :  x and y positions in the picture (>=0),
   *                 width and height (>=0),
   *                 the data, and a boolean indicating if it is the end
   *                 frame or not.
   */
  virtual BOOL SetFrameData (unsigned x,
                             unsigned y,
                             unsigned width,
                             unsigned height,
                             const BYTE *data,
                             BOOL endFrame);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Start displaying.
   * PRE          :  /
   */
  virtual BOOL Start () { return TRUE; };


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Stop displaying.
   * PRE          :  /
   */
  virtual BOOL Stop () { return TRUE; };


protected:

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Redraw the frame given as parameter.
   * PRE          :  /
   */
  virtual BOOL Redraw (int display, 
                       double zoom);

  DXWindow *dxWindow;

  static BOOL fallback;
};
#endif
