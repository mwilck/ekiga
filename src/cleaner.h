
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
 *                         cleaner.h -  description
 *                         ------------------------
 *   begin                : Mon Sep 26 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : Multithreaded class to end all threads when 
 *                          quitting.
 *   email                : dsandras@seconix.com
 *
 */


#ifndef _CLEANER_H_
#define _CLEANER_H_

#include "common.h"

#include <gnome.h>
#include <ptlib.h>
#include <h323.h>


/* The Cleaner class */
class GMThreadsCleaner : public PThread
{
  PCLASSINFO(GMThreadsCleaner, PThread);

 public:
  
  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Initialise the parameters.
   * PRE          :  /
   */
  GMThreadsCleaner ();


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMThreadsCleaner ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Unregister from the LDAP server, disconnect, shutdown the
   *                 devices... This is done in a separate thread.
   *                 This class is auto-deleted on termination.
   */
  void Main ();

protected:

  GmWindow *gw;
};

#endif
