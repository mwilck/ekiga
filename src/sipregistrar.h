
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
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
 *                         sipregistrar.h  -  description
 *                         ------------------------------
 *   begin                : Wed Dec 8 2004
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Multithreaded class to register to registrars
 *                          given the options in config.
 *
 */


#ifndef _REGISTRAR_H_
#define _REGISTRAR_H_

#include "common.h"


class GMSIPRegistrar : public PThread
{
  PCLASSINFO(GMSIPRegistrar, PThread);


public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Initialise the parameters.
   * PRE          :  /
   */
  GMSIPRegistrar ();


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMSIPRegistrar ();


  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Register to the registrar using the method and the 
   *                 parameters in config. This is done in a separate thread.
   *                 This class is not auto-deleted on termination. A popup
   *                 is displayed if registration fails or if options are
   *                 missing.
   * PRE          :  /
   */
  void Main ();

protected:

  int registering_method;
  
  PString registrar_host;
  PString registrar_login;
  PString registrar_password;
  PString registrar_realm;

  PMutex quit_mutex;
};

#endif
