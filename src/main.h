/***************************************************************************
                          main.h  -  description
                             -------------------
    begin                : Sat Dec 23 2000
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : This file contains all central functions to cope
                           with OpenH323
    email                : dsandras@acm.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MAIN_H_
#define _MAIN_H_

#include <ptlib.h>
#include <gsmcodec.h>
#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <glib.h>
#include <h323.h>

#include "common.h"
#include "endpoint.h"


/******************************************************************************/
/* The GTK callbacks                                                          */
/******************************************************************************/

// COMMON NOTICE:  GM_window_widgets is a structure containing pointers
//                 to all widgets created during the construction of the
//                 main window (see common.h for the exact content)
//                 that are needed for callbacks or other functions
//                 This structure exists during till the end of the execution.


// DESCRIPTION  :  This Timer is called when Gnome erroneously thinks that
//                 it has nothing to do.
// BEHAVIOR     :  It treats signals if needed.
// PRE          :
gint gnome_idle_timer (void);

// DESCRIPTION  :  This Timer is called evry second.
// BEHAVIOR     :  Elapsed time since the beginning of the connection is displayed.
// PRE          :
gint AppbarUpdate (GtkWidget *);

/******************************************************************************/


/******************************************************************************/
/*   This class is the main GnomeMeeting program to manage OpenH323           */
/******************************************************************************/

class GnomeMeeting : public PProcess
{
  PCLASSINFO(GnomeMeeting, PProcess);

 public:


  // DESCRIPTION  :  Constructor.
  // BEHAVIOR     :  Init variables.
  // PRE          :  GM_window_widgets is a valid pointer to the struct
  //                 containing all the widgets needed to manage and
  //                 update the main GUI, options * is valid too
  GnomeMeeting (GM_window_widgets *, options *);


  // DESCRIPTION  :  Destructor.
  // BEHAVIOR     :  
  // PRE          :  /
  ~GnomeMeeting ();

  
  // DESCRIPTION  :  To connect to a remote endpoint, or to answer a call.
  // BEHAVIOR     :  Answer a call, or call somebody, disable video transmission
  //                 if enabled but not available.
  // PRE          :  /
  void Connect ();


  // DESCRIPTION  :  /
  // BEHAVIOR     :  To refuse a call, or interrupt the current call
  // PRE          :  /
  void Disconnect ();
		

  // DESCRIPTION  :  /
  // BEHAVIOR     :  Add the current IP to the history in the combo
  // PRE          :  /
  void AddContactIP (char *);

  
  // Needed for PProcess
  void Main();

  
  // DESCRIPTION  :  /
  // BEHAVIOR     :  Returns the current endpoint
  // PRE          :  /
  GMH323EndPoint *Endpoint (void);


 private:

  // a pointer to the user options
  options *opts;
  
  // a pointer to the endpoint of the GnomeMeeting application
  GMH323EndPoint * endpoint;

  // a pointer that will be a pointer to the struct containing all
  // the widgets of the main window that will need to be modified
  // during the execution
  GM_window_widgets *gw;

  // the call number
  int call_number; 
};
/******************************************************************************/

#endif
