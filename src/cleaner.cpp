/***************************************************************************
                          garbage.cpp  -  description
                             -------------------
    begin                : Mon Sep 26 2001
    copyright            : (C) 2001 by Damien Sandras
    description          : Multithreaded class to end all threads when 
                           quitting.
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


#include "common.h"
#include "cleaner.h"
#include "ils.h"
#include "main.h"
#include "callbacks.h"
#include "main_interface.h"


extern GnomeMeeting *MyApp;

GMThreadsCleaner::GMThreadsCleaner (GM_window_widgets *g)
  :PThread (1000, AutoDeleteThread)
{
  gw = g;

  this->Resume ();
}


GMThreadsCleaner::~GMThreadsCleaner ()
{
  // Nothing to do here
}


void GMThreadsCleaner::Main ()
{
  GMILSClient *ils_client = (GMILSClient *) 
    MyApp->Endpoint ()->get_ils_client();
  GMVideoGrabber *video_grabber = (GMVideoGrabber *) 
    MyApp->Endpoint ()->GetVideoGrabber ();

  gdk_threads_enter ();
  disconnect_cb (NULL, NULL);
  gnome_appbar_push (GNOME_APPBAR (gw->statusbar), _("Quit in progress..."));
  GM_log_insert (gw->log_text, _("Quit in progress..."));
  gdk_threads_leave ();

  while (MyApp->Endpoint ()->CallingState () != 0)
    Current ()->Sleep (100);

  delete (ils_client);
  delete (video_grabber);

  gdk_threads_enter ();

  gtk_main_quit ();

  gdk_threads_leave ();
}
/******************************************************************************/
