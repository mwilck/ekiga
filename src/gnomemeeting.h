
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
 *                         gnomemeeting.h  -  description
 *                         ------------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains the main class
 *
 */


#ifndef _GNOMEMEETING_H_
#define _GNOMEMEETING_H_

#include "common.h"
#include "endpoint.h"
#include "videograbber.h"


/* COMMON NOTICE:  GmWindow is a structure containing pointers
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
   * PRE          :  The reason why the call was not disconnected.
   */
  void Disconnect (H323Connection::CallEndReason = H323Connection::EndedByLocalUser);

		
  /* Needed for PProcess */
  void Main();

  void CreateVideoGrabber (BOOL = true, BOOL = false);
  
  void RemoveVideoGrabber (BOOL = false);
  GMVideoGrabber *GetVideoGrabber ();
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the current endpoint.
   * PRE          :  /
   */
  GMH323EndPoint *Endpoint (void);


 private:

  GMH323EndPoint *endpoint;
  PThread *url_handler;
  GMVideoGrabber *video_grabber;
  
  GmWindow *gw;
  GmLdapWindow *lw;

  PMutex var_mutex;
  PIntCondMutex *vg;
  int call_number; 
};

#endif
