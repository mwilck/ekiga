/***************************************************************************
                           ils.h  -  description
                             -------------------
    begin                : Sun Sep 23 2001
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : This file contains all the functions needed 
                           for ILS support
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


#ifndef _ILS_H_
#define _ILS_H_

#include <lber.h>
#include <ldap.h>
#include <iostream.h>
#include <gnome.h>
#include <glib.h>
#include <sys/socket.h>
#include <ptlib.h>

#include "common.h"


class GMILSClient : public PThread
{
  PCLASSINFO(GMILSClient, PThread);

public:
  GMILSClient (GM_window_widgets *, GM_ldap_window_widgets *, 
	       options *);
  ~GMILSClient ();

  void Main ();
  void stop ();
  void Register ();
  void Unregister ();
  void ils_browse (int);

protected:
  BOOL Register (BOOL);
  void ils_browse (void);

  options *opts;
  GM_window_widgets *gw;
  GM_ldap_window_widgets *lw;

  int running;
  int msgid;
  int has_to_register;
  int has_to_unregister;
  int has_to_browse;
  int in_the_loop;
  int page_num;
  int registered;

  PTime starttime;
  LDAP *ldap_connection;
  PMutex quit_mutex;
};

#endif
