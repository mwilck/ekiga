
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
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : The ldap thread.
 *   email                : dsandras@seconix.com
 *
 */


#ifndef _ILS_H_
#define _ILS_H_

#include <iostream>
#include <gtk/gtk.h>
#include <glib.h>
#include <ptlib.h>
#include <ldap.h>

#include <gconf/gconf-client.h>

#include "common.h"
#include "xdap.h"

#define GM_ILS_CLIENT(x) (GMILSClient *)(x)


class GMILSClient : public PThread
{
  PCLASSINFO(GMILSClient, PThread);

public:
  GMILSClient ();
  ~GMILSClient ();


  void Register ();
  void Unregister ();
  void Modify ();
  gchar *Search (gchar *, gchar *, gchar *);
  BOOL CheckFieldsConfig (void);
  BOOL CheckServerConfig (void);

protected:
  void Main ();
  BOOL Register (int);

  GmWindow *gw;
  GmLdapWindow *lw;

  int running;
  int has_to_register;
  int has_to_unregister;
  int has_to_modify;
  int registered;

  PTime starttime;
  LDAP *ldap_connection;
  LDAP *ldap_search_connection;
  int rc_search_connection;
  PMutex quit_mutex;

  GConfClient *client;
};


/* The Browser class */
class GMILSBrowser : public PThread
{
  PCLASSINFO(GMILSBrowser, PThread);

 public:
  
  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Initialise the parameters.
   * PRE          :  The LDAP Server to browse (non-empty string).
   *                 The search filter, can be NULL (no filter).
   */
  GMILSBrowser (gchar *, gchar * = NULL);


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMILSBrowser ();


 protected:


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Browses the server and fills in the list.
   * PRE          :  /
   */
  void Main ();

  
  GtkWidget *page;
  GmLdapWindow *lw;
  LDAP *ldap_connection;
  gchar *ldap_server;
  gchar *search_filter;
  int rc;
};
#endif
