
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
 *                         lid.h  -  description
 *                         ---------------------
 *   begin                : Sun Dec 1 2002
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains LID functions.
 *
 */


#ifndef _LID_H_
#define _LID_H_

#include "../config.h"

#include "common.h"
#include "endpoint.h"


#ifdef HAS_IXJ
#include <ixjlid.h>



class GMLid : public PThread
{
  PCLASSINFO(GMLid, PThread);

 public:

  GMLid (PString);
  
  ~GMLid ();

  void Main ();

  void Open ();

  void Close ();
 
  void Stop ();

  void UpdateState (GMH323EndPoint::CallingState);

  void SetAEC (unsigned, OpalLineInterfaceDevice::AECLevels);

  void SetCountryCodeName (const PString &);

  void SetVolume (int, int);
  
  BOOL areSoftwareCodecsSupported ();

  OpalLineInterfaceDevice *GetLidDevice ();
  
  void Lock ();

  void Unlock ();
  
 private:

  OpalLineInterfaceDevice *lid;
  PMutex device_access_mutex;
  PMutex quit_mutex;
  PSyncPoint thread_sync_point;

  PString dev_name;
  int stop;
};
#endif

#endif
