
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
 *                         lid.h  -  description
 *                         --------------------------
 *   begin                : Sun Dec 1 2002
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains the LID methods.
 *   email                : dsandras@seconix.com
 *
 */


#ifndef _LID_H_
#define _LID_H_

#include "common.h"
#include "gdkvideoio.h"

#include <ptlib.h>
#include <h323.h>
#include <gtk/gtk.h>

#ifdef HAS_IXJ
#include <ixjlid.h>
#endif

#define GM_LID(x) (GMLid *)(x)

class GMLid : public PThread
{
  PCLASSINFO(GMLid, PThread);

 public:

  GMLid ();
  ~GMLid ();

  void Main ();

  void Open ();

  void Close ();
 
  void Stop ();

  OpalLineInterfaceDevice *GetLidDevice ();

 private:

  OpalLineInterfaceDevice *lid;
  PMutex quit_mutex;
  int stop;
};

#endif
