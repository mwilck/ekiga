
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
#include "dialog.h"
#include "misc.h"


#ifndef DISABLE_GNOME
#include <gnome.h>
#endif

#include <gconf/gconf-client.h>
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
  int x = 0, y = 0;
  GMH323EndPoint *endpoint = MyApp->Endpoint ();

  gnomemeeting_threads_enter ();

  gint timeout = 
    GPOINTER_TO_INT (g_object_get_data (G_OBJECT (gm), "timeout"));
  gtk_timeout_remove (timeout);

  gnomemeeting_statusbar_push (gw->statusbar, _("Quit in progress..."));

  /* Synchronous End of Call */
  if (endpoint->GetCallingState () != 0)
    endpoint->ClearCall (endpoint->GetCurrentCallToken (), 
			 H323Connection::EndedByLocalUser);

  gnomemeeting_threads_leave ();

  while (endpoint->GetCallingState ())
    Current ()->Sleep (50);

  delete (endpoint);

  gnomemeeting_threads_enter ();

  gtk_window_get_size (GTK_WINDOW (gw->local_video_window), &x, &y);
  gconf_client_set_int (gconf_client_get_default (), 
			"/apps/gnomemeeting/video_display/local_video_width",
			x, NULL);
  gconf_client_set_int (gconf_client_get_default (), 
			"/apps/gnomemeeting/video_display/local_video_height", 
			y, NULL);

  gtk_window_get_size (GTK_WINDOW (gw->remote_video_window), &x, &y);
  gconf_client_set_int (gconf_client_get_default (), 
			"/apps/gnomemeeting/video_display/remote_video_width",			x, NULL);
  gconf_client_set_int (gconf_client_get_default (), 
			"/apps/gnomemeeting/video_display/remote_video_height",
			y, NULL);
	    
  gtk_main_quit ();

  gnomemeeting_threads_leave ();
}
