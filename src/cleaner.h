/***************************************************************************
                          garabage.h  -  description
                             -------------------
    begin                : Wed Sep 19 2001
    copyright            : (C) 2001 by Damien Sandras
    description          : Multithreaded class to end the threads when
                           quitting
    email                : dsandras@seconix.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GARBAGE_H_
#define _GARBAGE_H_

#include "common.h"

#include <gnome.h>
#include <ptlib.h>
#include <h323.h>

class GMThreadsCleaner : public PThread
{
  PCLASSINFO(GMThreadsCleaner, PThread);

public:
  GMThreadsCleaner (GM_window_widgets *);
  ~GMThreadsCleaner ();

  void Main ();

protected:
  options *opts;
  GM_window_widgets *gw;
};

/******************************************************************************/
#endif
