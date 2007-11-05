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
 *                         videooutput.cpp  -  description
 *                         ------------------------------
 *   begin                : Sat Feb 17 2001
 *   copyright            : (C) 2000-2007 by Damien Sandras
 *   description          : PVideoOutputDevice class to permit to display via
 *                          GMVideoDisplay class
 *
 */


#include "config.h"

#define P_FORCE_STATIC_PLUGIN 

#include "videooutput.h"

#include "ekiga.h"
#include "misc.h"
#include "main.h"


#include "gmconf.h"

#include <ptlib/vconvert.h>

int PVideoOutputDevice_EKIGA::devices_nbr = 0;

#if defined HAVE_XV
GMVideoDisplay_XV* PVideoOutputDevice_EKIGA::videoDisplay = NULL;
#elif defined HAVE_DX
GMVideoDisplay_DX* PVideoOutputDevice_EKIGA::videoDisplay = NULL;
#else
GMVideoDisplay_GDK* PVideoOutputDevice_EKIGA::videoDisplay = NULL;
#endif

PMutex PVideoOutputDevice_EKIGA::videoDisplay_mutex;
/* Plugin definition */
class PVideoOutputDevice_EKIGA_PluginServiceDescriptor 
: public PDevicePluginServiceDescriptor
{
  public:
    virtual PObject *CreateInstance (int) const 
      {
	return new PVideoOutputDevice_EKIGA (); 
      }
    
    
    virtual PStringList GetDeviceNames(int) const 
      { 
	return PStringList("EKIGA"); 
      }
    
    virtual bool ValidateDeviceName (const PString & deviceName, 
				     int) const 
      { 
	return deviceName.Find("EKIGA") == 0; 
      }
} PVideoOutputDevice_EKIGA_descriptor;

PCREATE_PLUGIN(EKIGA, PVideoOutputDevice, &PVideoOutputDevice_EKIGA_descriptor);


/* The Methods */
PVideoOutputDevice_EKIGA::PVideoOutputDevice_EKIGA ()
{ 
 PWaitAndSignal m(videoDisplay_mutex);

  is_active = FALSE;
  
  /* Used to distinguish between input and output device. */
  device_id = 0; 

  /* Internal stuff */
  numberOfFrames = 0;

  if (!videoDisplay) 
#if defined HAVE_XV
     videoDisplay = new GMVideoDisplay_XV();
#elif defined HAVE_DX
     videoDisplay = new GMVideoDisplay_DX();
#else
     videoDisplay = new GMVideoDisplay_GDK();
#endif
}


PVideoOutputDevice_EKIGA::~PVideoOutputDevice_EKIGA()
{
  PWaitAndSignal m(videoDisplay_mutex);

  if (is_active)
    devices_nbr = PMAX (0, devices_nbr-1);
  if (devices_nbr == 0) {
     if (videoDisplay)
       delete videoDisplay;
     videoDisplay = NULL;
  }
}


BOOL 
PVideoOutputDevice_EKIGA::Open (const PString &name,
				G_GNUC_UNUSED BOOL unused)
{ 
  if (name == "EKIGAIN") 
    device_id = 1; 

#if not defined HAVE_XV && not defined HAVE_DX
  if (videoDisplay)
    videoDisplay->SetFallback(TRUE);
#endif

  return TRUE; 

}

PStringList PVideoOutputDevice_EKIGA::GetDeviceNames() const
{
  PStringList  devlist;
  devlist.AppendString(GetDeviceName());

  return devlist;
}


BOOL PVideoOutputDevice_EKIGA::IsOpen ()
{
  return TRUE;
}


BOOL PVideoOutputDevice_EKIGA::SetFrameData (unsigned x,
					   unsigned y,
					   unsigned width,
					   unsigned height,
					   const BYTE * data,
					   BOOL endFrame)
{
 PWaitAndSignal m(videoDisplay_mutex);

  numberOfFrames++;

  if (x > 0 || y > 0)
    return FALSE;

  if (width < 160 || width > 2048) 
    return FALSE;
  
  if (height <120 || height > 2048) 
    return FALSE;

  if (!endFrame)
    return FALSE;

  /* Device is now open */
  if (!is_active) {
    is_active = TRUE;
    devices_nbr = PMIN (2, devices_nbr+1);
  }

  videoDisplay->SetFrameData ( x, y, width, height, data, converter, (device_id == LOCAL), devices_nbr);

  return TRUE;
}

BOOL PVideoOutputDevice_EKIGA::SetColourFormat (const PString & colour_format)
{
  if (colour_format == "RGB24") {
    return PVideoOutputDevice::SetColourFormat (colour_format);
  }

  return FALSE;  
}
