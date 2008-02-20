
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras

 * This program is free software; you can  redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ekiga is licensed under the GPL license and as a special exception, you
 * have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OPAL, OpenH323 and PWLIB
 * programs, as long as you do follow the requirements of the GNU GPL for all
 * the rest of the software thus combined.
 */


/*
 *                         vidinput-manager-mlogo.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the interface of a vidinput core.
 *                          A vidinput core manages VidInputManagers.
 *
 */


#ifndef __VIDINPUT_MANAGER_MLOGO_H__
#define __VIDINPUT_MANAGER_MLOGO_H__

#include "vidinput-core.h"
#include "vidinput-manager.h"
#include "runtime.h"

#include "ptbuildopts.h"
#include <ptclib/delaychan.h>

/**
 * @addtogroup vidinput
 * @{
 */

  class GMVidInputManager_mlogo
   : public Ekiga::VidInputManager
    {
  public:

      /* The constructor
       */
      GMVidInputManager_mlogo (Ekiga::ServiceCore & core);
      /* The destructor
       */
      ~GMVidInputManager_mlogo () {}


      /*                 
       * DISPLAY MANAGEMENT 
       */               

      /** Create a call based on the remote uri given as parameter
       * @param uri  an uri
       * @return     true if a Ekiga::Call could be created
       */
      virtual void get_vidinput_devices(std::vector <Ekiga::VidInputDevice> & vidinput_devices);
      
      virtual bool set_vidinput_device (const Ekiga::VidInputDevice & vidinput_device, int channel, Ekiga::VideoFormat format);

      virtual bool open (unsigned width, unsigned height, unsigned fps);

      virtual void close();

      virtual void get_frame_data (unsigned & width,
                           unsigned & height,
                           char *data);

  protected:  
      void CopyYUVArea (const char* srcFrame,
			unsigned srcWidth,
			unsigned srcHeight,
			char* dstFrame,
			unsigned dstX,
			unsigned dstY,
			unsigned dstWidth,
			unsigned dstHeight);

      char* background_frame;
      unsigned pos;
      unsigned increment;

      Ekiga::ServiceCore & core;
      Ekiga::Runtime & runtime;

      PAdaptiveDelay m_Pacing;
  };
/**
 * @}
 */


#endif
