
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
 *                         ils.h  -  description
 *                         ---------------------
 *   begin                : Sun Sep 23 2001
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : The ILS registering thread, and the ILS browser
 *                          threads.
 *
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


/* This class is the ILS registering thread. It will run the whole life of
   the application, and execute inside the thread the required operations
   (Register/Modify/Unregister). It is using XDAP for those operations.
*/
class GMILSClient : public PThread
{
  PCLASSINFO(GMILSClient, PThread);

  
 public:
  
  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Creates the ILS registering thread and starts it. It will
   *                 automatically register to the ILS server given in the
   *                 gconf preferences if the user chose to register.
   * PRE          :  /
   */
  GMILSClient ();

  
  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  Only returns when the thread has ended all operations.
   * PRE          :  /
   */
  ~GMILSClient ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the flag to register to 1. When all operations will
   *                 be terminated, the thread will try to register to the
   *                 ILS server. Options are stored in the GConf database.
   * PRE          :  /
   */
  void Register ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the flag to unregister to 1. When all operations will
   *                 be terminated, the thread will try to unregister from the
   *                 ILS server. 
   * PRE          :  /
   */
  void Unregister ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Set the flag to modify to 1. When all operations will
   *                 be terminated, the thread will try to update the
   *                 current registered options on the ILS server. Options
   *                 are stored in the GConf database.
   * PRE          :  /
   */
  void Modify ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Searches the IP registered on the ILS server for the given
   *                 e-mail address. Returns NULL if there is no IP registered
   *                 for the given ILS server, port and e-mail address.
   * PRE          :  non-NULL ILS server, ILS port and e-mail address.
   */
  gchar *Search (gchar *, gchar *, gchar *);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Checks that all fields needed to register to ILS are
   *                 present in the GConf database (firstname, email). If not
   *                 returns FALSE and displays a popup to warn the user.
   * PRE          :  /
   */
  BOOL CheckFieldsConfig (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Checks that all fields needed to register to ILS are
   *                 present in the GConf database (server). If not
   *                 returns FALSE and displays a popup to warn the user.
   * PRE          :  /
   */
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
