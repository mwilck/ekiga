/***************************************************************************
                          gdkvideoio.h  -  description
                             -------------------
    begin                : Sat Feb 17 2001
    copyright            : (C) 2001 by Damien Sandras
    description          : Class needed to display in a GDK video outpur device
    email                : dsandras@acm.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GDKVIDEOIO_H_
#define _GDKVIDEOIO_H_

#include <ptlib.h>
#include <h323.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <videoio.h>
#include <gnome.h>

#include "common.h"


/******************************************************************************/
/*   GDKVideoOutputDevice : to manage the GDK output device (webcam)          */
/******************************************************************************/

class GDKVideoOutputDevice : public H323VideoDevice
{
  PCLASSINFO(GDKVideoOutputDevice, H323VideoDevice);

  public:

    // DESCRIPTION  :  The constructor
    // BEHAVIOR     :  Setups the parameters
    // PRE          :  GM_window_widgets is a valid pointer to a valid
    //                 GM_window_widgets
    GDKVideoOutputDevice (GM_window_widgets *);

    
    // DESCRIPTION  :  The constructor
    // BEHAVIOR     :  Setups the parameters, int = 0 if we do not transmit,
    //                 1 otherwise, if we do not transmit, default display = local
    //                 else default display = remote
    // PRE          :  GM_window_widgets is a valid pointer to a valid
    //                 GM_window_widgets
    GDKVideoOutputDevice (int, GM_window_widgets *);


    // DESCRIPTION  :  /
    // BEHAVIOR     :  scale of 1/9 if input buffer is 176x144 and returns the
    //                 corresponding buffer, scale 1/18 if input buffer is 352x288
    // PRE          :  a non-empty buffer
    void Resize (void *, void *, double);


    // DESCRIPTION  :  /
    // BEHAVIOR     :  changes current buffer to display (1 or 0)
    // PRE          :  1 or 0
    void DisplayConfig (int);


    // Same as in H323VideoDevice
    virtual BOOL Redraw(const void * frame);

  protected:

    // Same as in H323VideoDevice
    BOOL WriteLineSegment(int x, int y, unsigned len, const BYTE * data);

    int device_id;
    int transmitted_frame_number;
    int received_frame_number;

    PBYTEArray buffer;
    int display_config;
    
    GM_window_widgets *gw;
};

/******************************************************************************/

#endif
