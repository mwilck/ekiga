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
#include "cleaner.h"

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
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;
  int call_state = MyApp->Endpoint ()->CallingState ();
 
  if (!GTK_WIDGET_VISIBLE (pw->gw->pref_window))
    {
      gtk_widget_show_all (pw->gw->pref_window);

      /* update the preview button status in the pref window,
         this function will call the appropriate callback if needed */
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->video_preview),
				    gtk_toggle_button_get_active 
				    (GTK_TOGGLE_BUTTON 
				     (pw->gw->preview_button)));
    }
  else
    gtk_widget_hide_all (pw->gw->pref_window);
}


void ldap_callback (GtkButton *button, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;
  
  if (!GTK_WIDGET_VISIBLE (gw->ldap_window))
    gtk_widget_show_all (gw->ldap_window);
  else
    gtk_widget_hide_all (gw->ldap_window);
}


void connect_cb (GtkWidget *widget, gpointer data)
{	
  // data will be a widget if the callback is called from the popup window
  // that can be displayed on incoming calls
  if (GTK_IS_WIDGET (data))
    gtk_widget_destroy (GTK_WIDGET (data));

  MyApp->Connect();
}


void disconnect_cb (GtkWidget *widget, gpointer data)
{	
  GMH323Connection *connection = (GMH323Connection *) MyApp->Endpoint ()
    ->Connection ();

  // data will be a widget if the callback is called from the popup window
  // that can be displayed on incoming calls
  if (GTK_IS_WIDGET (data))
    gtk_widget_destroy (GTK_WIDGET (data));

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
      N_("Contributors:"),
      "Alex Larsson <alexl@redhat.com>",
      "Christopher R. Gabriel  <cgabriel@cgabriel.org>",
      "Kenneth Rohde Christiansen  <kenneth@gnu.org>",
      /* Translators: If your encoding allows it, use iacute (U00ED) for
         the 'i' of 'Rodriguez' and eacute (U00E9) for the first 'e' of
         'Perez'. */
      N_("Miguel Rodriguez Perez <migrax@terra.es>"),
      "Roger Hardiman <roger@freebsd.org>",
      "",
      N_("I18n Maintainer:"),
      "Christian Rose <menthos@gnu.org>",
      /* Translators: Replace "English" with the name of your language. */
      N_("English Translation:"),
      /* Translators: Replace my name with your name. */
      N_("Damien Sandras <dsandras@seconix.com>"),
      NULL
    };
	
  authors [2] = gettext (authors [2]);
  authors [9] = gettext (authors [9]);
  authors [11] = gettext (authors [11]);
  authors [12] = gettext (authors [12]);

  abox = gnome_about_new (PACKAGE,
			  VERSION,
			  /* Translators: Please change the (C) to a real
			     copyright character if your character set allows
			     it (Hint: iso-8859-1 is one of the character sets
			     that has this symbol). */
			  _("(C) 2000, 2001 Damien Sandras"),
			  authors,
			  _("GnomeMeeting is an H.323 compliant program for GNOME.\nThis program is not supported by Microsoft."),
			  "/usr/share/pixmaps/gnomemeeting-logo.png");
	
  gtk_widget_show (abox);
  return;
}


void quit_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;

  if (gw->cleaner_thread_count == 0)
    {
      gw->cleaner_thread_count++;
      new GMThreadsCleaner (gw);
    }
}  


void gtk_main_quit_callback (int res, gpointer data)
{
   GM_window_widgets *gw = (GM_window_widgets *) data;

   if (res == 0)
     {
       gtk_widget_hide (GTK_WIDGET (gm));
       gtk_main_quit ();
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


void ldap_popup_menu_callback (GtkWidget *widget, gpointer data)
{
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) data;
  gchar *text;

  if (lw->last_selected_row [lw->current_page] != -1)
    {
      /* text doesn't need to be freed, it is a pointer to the data */
      gtk_clist_get_text (GTK_CLIST (lw->ldap_users_clist [lw->current_page]),
			  lw->last_selected_row [lw->current_page],
			  7, &text);

      /* if we are waiting for a call, add the IP
       * to the history, and call that user       */
      if (MyApp->Endpoint ()->CallingState () == 0)
	{
	  /* this function will store a copy of text */
	  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (lw->gw->combo)->entry), text);
	  
	  connect_cb (NULL, NULL);
	}
    }
}

/******************************************************************************/
