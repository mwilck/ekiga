 
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2002 Damien Sandras
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
 *                         vfakeio.h  -  description
 *                         -------------------------
 *   begin                : Tue Jul 30 2003
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains a descendant of a Fake Input 
 *                          Device that display the GM logo when connected to
 *                          a remote party without using a camera.
 *   email                : dsandras@seconix.com
 *
 */


#ifndef _VFAKEIO_H_
#define _VFAKEIO_H_


#include <ptlib.h>
#include <h323.h>
#include <gnome.h>

 
class GMH323FakeVideoInputDevice : public PFakeVideoInputDevice 
{
  PCLASSINFO(GMH323FakeVideoInputDevice, PFakeVideoInputDevice);

  
 public:
  
  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Creates the Fake Input Device.
   * PRE          :  A name representing the path to the image to display. If
   *                 NULL or wrong, the GM logo will be transmitted.
   */
  GMH323FakeVideoInputDevice (gchar *);


  /* DESCRIPTION  :  The destructor
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMH323FakeVideoInputDevice ();


  /* DESCRIPTION  :  The destructor
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  virtual BOOL GetFrameDataNoDelay (BYTE *, PINDEX *);


  BYTE *data;
  GdkPixbuf *data_pix;
  int rgb_increment;
};


#endif
