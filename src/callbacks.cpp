
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
 *                         callbacks.cpp  -  description
 *                         -----------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains callbacks common to several
 *                          files.
 *   email                : dsandras@seconix.com
 *
 */


#include "../config.h"

#include "gnomemeeting.h"
#include "endpoint.h"
#include "connection.h"
#include "callbacks.h"
#include "pref_window.h"
#include "ldap_window.h"
#include "common.h"
#include "cleaner.h"


/* Declarations */
extern GnomeMeeting *MyApp;	
extern GtkWidget *gm;


/* The callbacks */
void toggle_window_callback (GtkWidget *widget, gpointer data)
{
  /* if data == NULL, we hide data, else we hide the widget 
     given as parameter */
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
    ->GetCurrentConnection ();

  if (connection != NULL)
    connection->PauseChannel (0);
}


void pause_video_callback (GtkWidget *widget, gpointer data)
{
  GMH323Connection *connection = (GMH323Connection *)  MyApp->Endpoint ()
    ->GetCurrentConnection ();

  if (connection != NULL)
    connection->PauseChannel (1);
}


void pref_callback (GtkWidget *widget, gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;
 
  if (!GTK_WIDGET_VISIBLE (pw->gw->pref_window)) {

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


void chat_callback (GtkButton *button, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;
  
  if (!GTK_WIDGET_VISIBLE (gw->chat_window))
    gtk_widget_show_all (gw->chat_window);
  else
    gtk_widget_hide_all (gw->chat_window);
}


void connect_cb (GtkWidget *widget, gpointer data)
{	
  if (MyApp->Endpoint ()->GetCallingState () == 0)
    MyApp->Connect();
}


void disconnect_cb (GtkWidget *widget, gpointer data)
{	
  GMH323Connection *connection = (GMH323Connection *) 
    MyApp->Endpoint ()->GetCurrentConnection ();

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

  if (gw->cleaner_thread_count == 0) {

      gw->cleaner_thread_count++;
      new GMThreadsCleaner ();
  }
}  


void popup_menu_local_callback (GtkWidget *widget, gpointer data)
{
  GMH323EndPoint *endpoint = MyApp->Endpoint ();
  endpoint->SetCurrentDisplay (0);
}


void popup_menu_remote_callback (GtkWidget *widget, gpointer data)
{
  GMH323EndPoint *endpoint = MyApp->Endpoint ();
  endpoint->SetCurrentDisplay (1);
}


void popup_menu_both_callback (GtkWidget *widget, gpointer data)
{
  GMH323EndPoint *endpoint = MyApp->Endpoint ();
  endpoint->SetCurrentDisplay (2);
}


void ldap_popup_menu_callback (GtkWidget *widget, gpointer data)
{
  GM_ldap_window_widgets *lw = (GM_ldap_window_widgets *) data;
  gchar *text;

  if (lw->last_selected_row [lw->current_page] != -1) {

    /* text doesn't need to be freed, it is a pointer to the data */
    gtk_clist_get_text (GTK_CLIST (lw->ldap_users_clist [lw->current_page]),
			lw->last_selected_row [lw->current_page],
			7, &text);
    
    /* if we are waiting for a call, add the IP
       to the history, and call that user       */
    if (MyApp->Endpoint ()->GetCallingState () == 0) {

      /* this function will store a copy of text */
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (lw->gw->combo)->entry), text);
      
      connect_cb (NULL, NULL);
    }
  }
}
