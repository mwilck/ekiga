
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

#include "callbacks.h"
#include "gnomemeeting.h"
#include "endpoint.h"
#include "connection.h"
#include "pref_window.h"
#include "ldap_window.h"
#include "common.h"
#include "misc.h"
#include "cleaner.h"


/* Declarations */
extern GnomeMeeting *MyApp;	
extern GtkWidget *gm;


/* The callbacks */
void pause_audio_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);
 
  GMH323Connection *connection = (GMH323Connection *)  MyApp->Endpoint ()
    ->GetCurrentConnection ();

  if (connection != NULL)
    connection->PauseChannel (0);

  GTK_TOGGLE_BUTTON (gw->audio_chan_button)->active = FALSE;
  gtk_widget_draw (GTK_WIDGET (gw->audio_chan_button), NULL);
}


void pause_video_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  GMH323Connection *connection = (GMH323Connection *)  MyApp->Endpoint ()
    ->GetCurrentConnection ();

  if (connection != NULL)
    connection->PauseChannel (1);

  GTK_TOGGLE_BUTTON (gw->video_chan_button)->active = FALSE;
  gtk_widget_draw (GTK_WIDGET (gw->video_chan_button), NULL);
}


void gnomemeeting_component_view (GtkWidget *w, gpointer data)
{
  if (!GTK_WIDGET_VISIBLE (GTK_WIDGET (data))) 
    gtk_widget_show_all (GTK_WIDGET (data));
  else
    gtk_widget_hide_all (GTK_WIDGET (data));
}


void connect_cb (GtkWidget *widget, gpointer data)
{	
  MyApp->Connect ();
}


void disconnect_cb (GtkWidget *widget, gpointer data)
{	
  GMH323Connection *connection = 
    (GMH323Connection *) MyApp->Endpoint ()->GetCurrentConnection ();
	
  if (connection != NULL)
    connection->UnPauseChannels ();

  MyApp->Disconnect ();
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
			  _("Copyright (C) 2000, 2001, 2002 Damien Sandras"),
			  authors,
			  _("GnomeMeeting is a full-featured H.323 videoconferencing application."),
			  GNOMEMEETING_IMAGES "/gnomemeeting-logo.png");
	
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
  int last_selected_row;
  GtkCList *ldap_users_clist;
  GtkWidget *curr_page;

  curr_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook),
					 gtk_notebook_get_current_page (GTK_NOTEBOOK
									(lw->notebook)));

  ldap_users_clist = GTK_CLIST (gtk_object_get_data (GTK_OBJECT (curr_page), 
						     "ldap_users_clist"));
  last_selected_row = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (ldap_users_clist),
							    "last_selected_row"));

  if (last_selected_row != -1) {

    /* text doesn't need to be freed, it is a pointer to the data */
    gtk_clist_get_text (GTK_CLIST (ldap_users_clist),
			last_selected_row,
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

/* DESCRIPTION  :  This callback is called when a gconf error happens
 * BEHAVIOR     :  Pop-up a message-box
 * PRE          :  /
 */
void gconf_error_callback (GConfClient *, GError *)
{
  GtkWidget *dialog = gnome_message_box_new (_("An error has happened in the configuration"
					       " backend\nMaybe some of your settings won't"
					       " be stored"), 
					     GNOME_MESSAGE_BOX_ERROR,
					     GNOME_STOCK_BUTTON_CLOSE,
					     NULL);
  gnome_dialog_run (GNOME_DIALOG (dialog));
}
