
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
 *                         ils.h  -  description
 *                         ---------------------
 *   begin                : Sun Sep 23 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : The ldap thread.
 *   email                : dsandras@seconix.com
 *
 */

#ifndef _ILS_H_
#define _ILS_H_

#include <lber.h>
#include <ldap.h>
#include <iostream.h>
#include <gnome.h>
#include <glib.h>
#include <sys/socket.h>
#include <ptlib.h>

#include <gconf/gconf-client.h>

#include "common.h"

/* This calls will change in 0.13 */

class GMILSClient : public PThread
{
  PCLASSINFO(GMILSClient, PThread);

public:
  GMILSClient (options *);
  ~GMILSClient ();

  void Main ();
  void stop ();
  void Register ();
  void Unregister ();
  void ils_browse (int);

protected:
  BOOL Register (BOOL);
  void ils_browse (void);
  void UpdateConfig (void);

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

  /* The options that will be updated from the gconf cache */
  gchar *ldap_server;
  gchar *ldap_port;
  gchar *firstname;
  gchar *surname;
  gchar *mail;
  gchar *comment;
  gchar *location;

  GConfClient *client;
};

#endif
