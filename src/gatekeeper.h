
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
 *                         gatekeeper.cpp  -  description
 *                         ------------------------------
 *   begin                : Wed Sep 19 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : Multithreaded class to register to gatekeepers.
 *   email                : dsandras@seconix.com
 *
 */


#ifndef _GATEKEEPER_H_
#define _GATEKEEPER_H_

#include "common.h"

#include <gnome.h>
#include <ptlib.h>
#include <h323.h>

class GMH323Gatekeeper : public PThread
{
  PCLASSINFO(GMH323Gatekeeper, PThread);


public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Initialise the parameters.
   * PRE          :  /
   */
  GMH323Gatekeeper ();


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMH323Gatekeeper ();


  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Register to the gatekeeper using the method and the 
   *                 parameters in opts. This is done in a separate thread.
   *                 This class is auto-deleted on termination.
   * PRE          :  /
   */
  void Main ();

protected:

  GM_window_widgets *gw;
};

#endif
