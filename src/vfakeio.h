
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2003 Damien Sandras
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
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         vfakeio.h  -  description
 *                         -------------------------
 *   begin                : Tue Jul 30 2003
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains a descendant of a Fake Input 
 *                          Device that display the GM logo when connected to
 *                          a remote party without using a camera.
 *
 */


#ifndef _VFAKEIO_H_
#define _VFAKEIO_H_


#include <ptlib.h>
#include <h323.h>

#include <gtk/gtk.h>

 
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
  GdkPixbuf *logo_pix;
  gchar *video_image;
  bool picture;
  int rgb_increment;
  int pos;
  int increment;
};


#endif
