
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
 *                         main.h  -  description
 *                         ----------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains the main class
 *   email                : dsandras@seconix.com
 *
 */

#ifndef _MAIN_H_
#define _MAIN_H_

#include <ptlib.h>
#include <gsmcodec.h>
#include <h323.h>

#include "common.h"
#include "endpoint.h"


/* COMMON NOTICE:  GM_window_widgets is a structure containing pointers
 *                 to all widgets created during the construction of the
 *                 main window (see common.h for the exact content)
 *                 that are needed for callbacks or other functions
 *                 This structure exists during till the end of 
 *                 the execution.
 */


/* The main gnomeMeeting class */

class GnomeMeeting : public PProcess
{
  PCLASSINFO(GnomeMeeting, PProcess);

 public:


  /* DESCRIPTION  :  Constructor.
   * BEHAVIOR     :  Init variables.
   * PRE          :  
   */
  GnomeMeeting ();


  /* DESCRIPTION  :  Destructor.
   * BEHAVIOR     :  
   * PRE          :  /
   */
  ~GnomeMeeting ();

  
  /* DESCRIPTION  :  To connect to a remote endpoint, or to answer a call.
   * BEHAVIOR     :  Answer a call, or call somebody.
   * PRE          :  /
   */
  void Connect ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  To refuse a call, or interrupt the current call.
   * PRE          :  /
   */
  void Disconnect ();
		

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Add the current IP to the history in the combo.
   * PRE          :  /
   */
  void AddContactIP (char *);

  
  /* Needed for PProcess */
  void Main();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the current endpoint.
   * PRE          :  /
   */
  GMH323EndPoint *Endpoint (void);


 private:

  GMH323EndPoint *endpoint;

  GM_window_widgets *gw;
  GM_ldap_window_widgets *lw;

  int call_number; 
};

#endif
