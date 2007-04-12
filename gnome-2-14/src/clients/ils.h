
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 *                         ils.h  -  description
 *                         ---------------------
 *   begin                : Sun Sep 23 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : The ILS class thread.
 *
 */


#ifndef _ILS_H_
#define _ILS_H_

#define LDAP_DEPRECATED 1

#include "common.h"
#include "xdap.h"

#include <iostream>
#include <ldap.h>


#define GM_ILS_CLIENT(x) (GMILSClient *)(x)


/* This class is the ILS registering thread. It will run the whole life of
   the application, and execute inside the thread the required operations
   (Register/Modify/Unregister). It is using XDAP for those operations.
*/
class GMILSClient : public PObject
{
  PCLASSINFO(GMILSClient, PObject);

  
 public:
  
  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Creates the ILS registering thread and starts it. It will
   *                 automatically register to the ILS server given in the
   *                 config preferences if the user chose to register.
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
   *                 ILS server. Options are stored in the config database.
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
   *                 are stored in the config database.
   * PRE          :  /
   */
  void Modify ();


protected:


  /* Different possible operations */
  enum Operation {
    
    ILS_REGISTER, 
    ILS_UPDATE, 
    ILS_UNREGISTER,
    ILS_NONE
  };


  /* Checks if the fields required for registering are present, if not
     displays a popup and returns FALSE if the user wanted to register */
  BOOL CheckFieldsConfig (BOOL);

  /* Checks if the server fields required for registering are present, if not
     displays a popup and returns FALSE */
  BOOL CheckServerConfig (void);

  /* Executes an XDAP operation, displays popups and do nothing if
     it fails */
  void ILSOperation (Operation operation);

  /* Processes an XDAP operation on an XML tree, returns FALSE when it fails */
  BOOL XDAPProcess (LDAP *ldap, xmlDocPtr xp, xmlNodePtr *curp);


  int operation;

  PTime starttime;
  LDAP *ldap_connection;
  PMutex quit_mutex;
};
#endif
