
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
 *                          urlhandler.h  -  description
 *                         ------------------------------
 *   begin                : Sat Jun 8 2002
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : Multithreaded class to call a given URL,
 *                          or to answer a call.
 *   email                : dsandras@seconix.com
 *
 */


#ifndef _URLHANDLER_H_
#define _URLHANDLER_H_

#include <ptlib.h>

#include "common.h"


class GMURLHandler : public PThread
{
  PCLASSINFO(GMURLHandler, PThread);


public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Initialise the parameters. GM will call the given URL
   *                 after having parsed it.
   * PRE          :  The URL.
   */
  GMURLHandler (PString);


  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Initialise the parameters. GM will answer the incoming
   *                 call (if any).
   * PRE          :  /
   */
  GMURLHandler ();

  
  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMURLHandler ();


  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Parses the URL and establish the call if URL ok or
   *                 user found in ILS directory.
   * PRE          :  /
   */
  void Main ();


protected:

  GmWindow *gw;
  PString url;
  BOOL answer_call;
  PMutex quit_mutex;
};
#endif
