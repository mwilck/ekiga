
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2002 Damien Sandras
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
 *                          urlhandler.cpp  -  description
 *                         -------------------------------
 *   begin                : Sat Jun 8 2002
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : Multithreaded class to call a given URL.
 *   email                : dsandras@seconix.com
 *
 */


#include "../config.h" 


#include "urlhandler.h"
#include "gnomemeeting.h"
#include "main_window.h"
#include "misc.h"
#include "endpoint.h"
#include "toolbar.h"
#include "ils.h"


/* Declarations */
extern GnomeMeeting *MyApp;
extern GtkWidget *gm;


/* The class */
GMURLHandler::GMURLHandler (PString c)
  :PThread (1000, AutoDeleteThread)
{
  gw = gnomemeeting_get_main_window (gm);
  url = c;
 
  this->Resume ();
}


GMURLHandler::~GMURLHandler ()
{
  quit_mutex.Wait ();

  /* Nothing to do here except waiting that LDAP at worse
     cancels the search */

  quit_mutex.Signal ();
}


void GMURLHandler::Main ()
{
  PString call_address;
  GMILSClient *ils_client = NULL;
  PINDEX at;
  PINDEX slash;
  PINDEX callto;
  PString tmp_url;
  PString ils_server;
  PString ils_port = PString ("389");
  PString mail;
  PString current_call_token;
  gchar *ip = NULL;
  gchar *msg = NULL;
  GmWindow *gw = NULL;
  GMH323EndPoint *endpoint = NULL;
  H323Connection *con = NULL;
  GnomeUIInfo *call_menu_uiinfo = NULL;

  gnomemeeting_threads_enter ();
  gw = gnomemeeting_get_main_window (gm);
  call_menu_uiinfo = 
    (GnomeUIInfo *) g_object_get_data (G_OBJECT (gm), "call_menu_uiinfo");
  gnomemeeting_threads_leave ();

  endpoint = MyApp->Endpoint ();

  
  /* We first remove the callto: part if any */
  tmp_url = url.ToLower ();
  callto = tmp_url.FindLast ("//");
  if (callto != P_MAX_INDEX)
    url = url.Mid (callto + 2, P_MAX_INDEX);

  call_address = url;

  at = url.Find ('@');
  slash = url.Find ('/');

  
  /* We have a callto URL of the form : ils_server(:port)/mail */
  if ((at != P_MAX_INDEX) && (slash != P_MAX_INDEX)) {

    quit_mutex.Wait ();
    /* We disable the connect button and the connect menu 
       while searching */
    gnomemeeting_threads_enter ();
    gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [0].widget), 
			      FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [1].widget), 
			      FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (gw->connect_button), FALSE);
    gnomemeeting_threads_leave ();
  
    ils_server = url.Left (slash);
    PINDEX p = url.Find (':');

    gnomemeeting_threads_enter ();
    gnomemeeting_statusbar_flash (gm, _("Searching for user"));
    gnomemeeting_threads_leave ();
    /* There is a port */
    if (p != P_MAX_INDEX) {
    
      ils_port = ils_server.Mid (p + 1, P_MAX_INDEX);
      ils_server = ils_server.Left (p);
    }

    mail = url.Mid (slash + 1, P_MAX_INDEX);

    /* Search for the user on ILS */
    ils_client = (GMILSClient *) endpoint->GetILSClient ();

    ip =
      ils_client->Search ((gchar *) (const char *) ils_server, 
			  (gchar *) (const char *) ils_port, 
			  (gchar *) (const char *) mail);


    if (ip == NULL) {

      gnomemeeting_threads_enter ();
      gnomemeeting_statusbar_flash (gm, _("Error while connecting to ILS directory"));
      gnomemeeting_log_insert (gw->history_text_view, 
			       _("Error while connecting to ILS directory"));
      connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 0);
      gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [0].widget), 
				TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (gw->connect_button), TRUE);

      if (gw->progress_timeout) {

	gtk_timeout_remove (gw->progress_timeout);
	gw->progress_timeout = 0;
      }

      gnomemeeting_threads_leave ();

      quit_mutex.Signal ();
      return;
    }
     
    if ((ip)&&(!strcmp (ip, "0.0.0.0"))) {

      gnomemeeting_threads_enter ();
      gnomemeeting_statusbar_flash (gm, _("User not found"));
      gnomemeeting_log_insert (gw->history_text_view, _("User not found"));
      connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 0);
      gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [0].widget), 
				TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (gw->connect_button), TRUE);

      if (gw->progress_timeout) {

	gtk_timeout_remove (gw->progress_timeout);
	gw->progress_timeout = 0;
      }

      gnomemeeting_threads_leave ();

      quit_mutex.Signal ();
      return;
    }
    else {

      call_address = PString (ip);
    }

    gnomemeeting_threads_enter ();
    gtk_widget_set_sensitive (GTK_WIDGET (gw->connect_button), TRUE);
    gnomemeeting_threads_leave ();

    quit_mutex.Signal ();
  }
    

  /* Connect to the URL */
  con = 
    endpoint->MakeCallLocked (call_address, current_call_token);
  endpoint->SetCurrentConnection (con);
  endpoint->SetCurrentCallToken (current_call_token);
  endpoint->SetCallingState (1);
  con->Unlock ();
  
  gnomemeeting_threads_enter ();
  gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);


  /* Enable disconnect: we must be able to stop calling */
  gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [1].widget), 
			    TRUE);
  
  msg = g_strdup_printf (_("Calling %s"), 
			 (const char *) call_address);
  gnomemeeting_log_insert (gw->history_text_view, msg);
  gnomemeeting_log_insert (gw->calls_history_text_view, msg);
  gnome_appbar_push (GNOME_APPBAR (gw->statusbar), msg);
  connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 1);
  g_free (msg);		
  gnomemeeting_threads_leave ();		 
}
