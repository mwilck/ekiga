
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
 *                         gdkvideoio.h  -  description
 *                         ----------------------------
 *   begin                : Sat Feb 17 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Class to permit to display in GDK Drawing Area or
 *                          SDL.
 *
 */


#ifndef _GDKVIDEOIO_H_
#define _GDKVIDEOIO_H_

#include "common.h"

class GMManager;


class PVideoOutputDevice_GDK : public PVideoOutputDevice
{
  PCLASSINFO(PVideoOutputDevice_GDK, PVideoOutputDevice);

  public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  PVideoOutputDevice_GDK ();


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~PVideoOutputDevice_GDK ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Open the device given the device name.
   * PRE          :  Device name to open, immediately start device.
   */
  BOOL Open (const PString &, 
	     BOOL); 

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return a list of all of the drivers available.
   * PRE          :  /
   */
  PStringList GetDeviceNames() const;


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Get the maximum frame size in bytes.
   * PRE          :  /
   */
  PINDEX GetMaxFrameBytes() { return 352 * 288 * 3 * 2; }

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns TRUE if the output device is open.
   * PRE          :  /
   */
  BOOL IsOpen ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  If data for the end frame is received, then we convert
   *                 it from to RGB32 and we display it.
   * PRE          :  x and y positions in the picture (>=0),
   *                 width and height (>=0),
   *                 the data, and a boolean indicating if it is the end
   *                 frame or not.
   */
  BOOL SetFrameData (unsigned, unsigned, unsigned, unsigned,
		     const BYTE *, BOOL);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns TRUE if the colour format is supported (ie RGB24).
   * PRE          :  /
   */
  BOOL SetColourFormat (const PString &);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Displays the current frame.
   * PRE          :  /
   */
  BOOL EndFrame();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Start displaying.
   * PRE          :  /
   */
  BOOL Start () { return TRUE; };
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Stop displaying.
   * PRE          :  /
   */
  BOOL Stop () { return TRUE; };
    
 protected:


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Redraw the frame given as parameter.
   * PRE          :  /
   */
  BOOL Redraw ();

  static int devices_nbr; /* The number of devices opened */
  int device_id;          /* The current device : encoding or not */
  int display_config;     /* Current display : local or remote or both */

  PMutex redraw_mutex;

  static PBYTEArray lframeStore;
  static PBYTEArray rframeStore;
  static int lf_width;
  static int lf_height;
  static int rf_width;
  static int rf_height;
  
  BOOL start_in_fullscreen;
  BOOL is_open;
  BOOL is_active;

  enum {REMOTE, LOCAL};
};
#endif
