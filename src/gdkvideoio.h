
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2001 Damien Sandras
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
 */

/*
 *                         gdkvideoio.cxx  -  description
 *                        -------------------
 *   begin                : Sat Feb 17 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : Class to permit to display in GDK Drawing Area
 *   email                : dsandras@seconix.com
 *
 */


#ifndef _GDKVIDEOIO_H_
#define _GDKVIDEOIO_H_

#include <ptlib.h>
#include <h323.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <videoio.h>
#include <gnome.h>
#include <pthread.h>

#include "common.h"


class GDKVideoOutputDevice : public H323VideoDevice
{
  PCLASSINFO(GDKVideoOutputDevice, H323VideoDevice);


  public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Setup parameters.
   * PRE          :  GM_window_widgets is a valid pointer to a valid
   *                 GM_window_widgets structure.
   */
  GDKVideoOutputDevice (GM_window_widgets *);
    
    
  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Setups the parameters, 
   *                 int = 0 if we do not transmit,
   *                 1 otherwise, if we do not transmit, 
   *                 default display = local
   *                 else default display = remote.
   * PRE          :  GM_window_widgets is a valid pointer to a valid
   *                 GM_window_widgets structure.
   */
  GDKVideoOutputDevice (int, GM_window_widgets *);


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GDKVideoOutputDevice ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Changes current buffer to display.
   * PRE          :  * 0 (local) or 1 (remote) or 2 (both)
   *                 Must be protected if called from threads.
   */
  void SetCurrentDisplay (int);


  /* Same as in H323VideoDevice.
   */
  virtual BOOL Redraw(const void * frame);


  protected:

  /* Same as in H323VideoDevice */
  BOOL WriteLineSegment(int x, int y, unsigned len, const BYTE * data);

  int device_id; /* The current device : encoding or not */
  int transmitted_frame_number;
  int received_frame_number;

  PBYTEArray buffer; /* The RGB24 buffer; contains the images */
  int display_config; /* Current display : local or remote or both */
  PMutex redraw_mutex;
  PMutex display_config_mutex;
    
  GM_window_widgets *gw;
};

#endif
