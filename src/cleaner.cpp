
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
 *                         cleaner.cpp -  description
 *                         --------------------------
 *   begin                : Mon Sep 26 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : Multithreaded class to end all threads when 
 *                          quitting.
 *   email                : dsandras@seconix.com
 *
 */


#include "../config.h"

#include "common.h"
#include "cleaner.h"
#include "ils.h"
#include "gnomemeeting.h"
#include "callbacks.h"
#include "misc.h"


/* Declarations */
extern GnomeMeeting *MyApp;
extern GtkWidget *gm;


/* The methods */

GMThreadsCleaner::GMThreadsCleaner ()
  :PThread (1000, AutoDeleteThread)
{
  gw = gnomemeeting_get_main_window (gm);

  this->Resume ();
}


GMThreadsCleaner::~GMThreadsCleaner ()
{
  /* Nothing to do here */
}


void GMThreadsCleaner::Main ()
{
  int counter = 0;

  GMILSClient *ils_client = (GMILSClient *) 
    MyApp->Endpoint ()->GetILSClient();

  GMVideoGrabber *video_grabber = (GMVideoGrabber *) 
    MyApp->Endpoint ()->GetVideoGrabber ();

  GMH323EndPoint *endpoint = MyApp->Endpoint ();

  gnomemeeting_threads_enter ();

  gnome_appbar_push (GNOME_APPBAR (gw->statusbar), _("Quit in progress..."));
  gnomemeeting_log_insert (_("Quit in progress..."));

  /* Synchronous End of Call */
  endpoint->ClearAllCalls (H323Connection::EndedByLocalUser, FALSE);

  gnomemeeting_threads_leave ();

  while (MyApp->Endpoint ()->GetCallingState () != 0) {
   
    /* if OpenH323 doesn't disconnect, we force the exit */
    if (counter > 50) {

      cout << "Warning: We have forced the exit" << endl << flush;
      gtk_main_quit ();      
    }

    Current ()->Sleep (100);
    counter++;  
  }
  
  delete (ils_client);
  delete (video_grabber);

  gnomemeeting_threads_enter ();
  gtk_main_quit ();
  gnomemeeting_threads_leave ();
}
