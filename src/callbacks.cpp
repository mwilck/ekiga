/***************************************************************************
                          callbacks.cpp  -  description
                             -------------------
    begin                : Sat Dec 23 2000
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : This file contains all the callbacks common to 
                           several files
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

#include "../config.h"

#include "main.h"
#include "endpoint.h"
#include "connection.h"
#include "callbacks.h"
#include "preferences.h"
#include "ldap_h.h"
#include "common.h"

#include <iostream.h> //

/******************************************************************************/
/* Global Variables                                                           */
/******************************************************************************/

extern GnomeMeeting *MyApp;	
extern GtkWidget *gm;

/******************************************************************************/


/******************************************************************************/
/* GTK Callbacks                                                              */
/******************************************************************************/

void toggle_window_callback (GtkWidget *widget, gpointer data)
{
  // if data == NULL, we hide data, else we hide the widget given as parameter
  if (GTK_IS_WIDGET (data))
      widget = GTK_WIDGET (data);

  if (GTK_WIDGET_VISIBLE (GTK_WIDGET (widget)))
    gtk_widget_hide (widget);
  else
    gtk_widget_show (widget);
}


void pause_audio_callback (GtkWidget *widget, gpointer data)
{
  GMH323Connection *connection = (GMH323Connection *)  MyApp->Endpoint ()
    ->Connection ();

  if (connection != NULL)
    connection->PauseChannel (0);
}


void pause_video_callback (GtkWidget *widget, gpointer data)
{
  GMH323Connection *connection = (GMH323Connection *)  MyApp->Endpoint ()
    ->Connection ();

  if (connection != NULL)
    connection->PauseChannel (1);
}


void pref_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;
  int call_state = MyApp->Endpoint ()->CallingState();

  if (gw->pref_window == NULL)
    GMPreferences (call_state, gw);
}


void ldap_callback (GtkButton *button, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;
  
  if (gw->ldap_window == NULL)
    GM_ldap_init (gw);
}


void connect_cb (GtkWidget *widget, gpointer data)
{	
  MyApp->Connect();
}


void disconnect_cb (GtkWidget *widget, gpointer data)
{	
  GMH323Connection *connection = (GMH323Connection *) MyApp->Endpoint ()
    ->Connection ();

  if (connection != NULL)
    connection->UnPauseChannels ();

  MyApp->Disconnect();
}


void about_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget *abox;
	
  const gchar *authors [] =
    {
      "Damien Sandras <dsandras@seconix.com>",
      "",
      N_("Contributors  :"),
      "Alex Larsson <alexl@redhat.com>",
      "Roger Hardiman <roger@freebsd.org>",
      NULL
    };
	
  authors [2] = gettext (authors [2]);
	
  abox = gnome_about_new (PACKAGE,
			  VERSION,
			  _("(c) 2000-2001 by Damien Sandras"),
			  authors,
			  _("GnomeMeeting is an H.323 compliant program for gnome\nThis program is not supported by Microsoft(c)."),
			  "/usr/share/pixmaps/gnomemeeting-logo.png");
	
  gtk_widget_show (abox);
  return;
}


void quit_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;
  
  MyApp -> Endpoint () -> ClearAllCalls ();
 
  usleep (1000);

  if (gw->applet == NULL)
    gtk_main_quit ();
  else
    {
      applet_widget_remove (APPLET_WIDGET (gw->applet));
      applet_widget_gtk_main_quit ();
    }
}


void view_statusbar_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;

  if (GTK_WIDGET_VISIBLE (GTK_WIDGET (gw->statusbar)))
    gtk_widget_hide (gw->statusbar);
  else
    gtk_widget_show (gw->statusbar);  
}


void view_notebook_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;

  if (GTK_WIDGET_VISIBLE (GTK_WIDGET (gw->main_notebook)))
    gtk_widget_hide (gw->main_notebook);
  else
    gtk_widget_show (gw->main_notebook);  
}


void view_remote_user_info_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;

  gtk_notebook_set_page (GTK_NOTEBOOK (gw->main_notebook), 0);
}


void view_log_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;

  gtk_notebook_set_page (GTK_NOTEBOOK (gw->main_notebook), 1);
}


void view_audio_settings_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;

  gtk_notebook_set_page (GTK_NOTEBOOK (gw->main_notebook), 2);
}


void view_video_settings_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;

  gtk_notebook_set_page (GTK_NOTEBOOK (gw->main_notebook), 3);
}


void popup_menu_local_callback (GtkWidget *widget, gpointer data)
{
  GMH323EndPoint *endpoint = MyApp->Endpoint ();
  endpoint->DisplayConfig (0);
}


void popup_menu_remote_callback (GtkWidget *widget, gpointer data)
{
  GMH323EndPoint *endpoint = MyApp->Endpoint ();
  endpoint->DisplayConfig (1);
}


void popup_menu_both_callback (GtkWidget *widget, gpointer data)
{
  GMH323EndPoint *endpoint = MyApp->Endpoint ();
  endpoint->DisplayConfig (2);
}

/******************************************************************************/
