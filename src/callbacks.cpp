
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
void save_callback (GtkWidget *widget, gpointer data)
{
  MyApp->Endpoint ()->SavePicture ();
}


void pause_audio_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);
 
  GMH323Connection *connection = (GMH323Connection *)  MyApp->Endpoint ()
    ->GetCurrentConnection ();

  if (connection != NULL)
    connection->PauseChannel (0);

}


void pause_video_callback (GtkWidget *widget, gpointer data)
{
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  GMH323Connection *connection = (GMH323Connection *)  MyApp->Endpoint ()
    ->GetCurrentConnection ();

  if (connection != NULL)
    connection->PauseChannel (1);

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
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  if (gw->incoming_call_popup)
    gtk_widget_destroy (gw->incoming_call_popup);

  gw->incoming_call_popup = NULL;

  if (MyApp->Endpoint ()->GetCallingState () == 0)
    MyApp->Connect ();
}


void disconnect_cb (GtkWidget *widget, gpointer data)
{	
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  GMH323Connection *connection = 
    (GMH323Connection *) MyApp->Endpoint ()->GetCurrentConnection ();

  if (gw->incoming_call_popup)
    gtk_widget_destroy (gw->incoming_call_popup);

  gw->incoming_call_popup = NULL;
	
  if (connection != NULL)
    connection->UnPauseChannels ();
  
  MyApp->Disconnect ();
}


void about_callback (GtkWidget *widget, gpointer parent_window)
{
  GtkWidget *abox;
  GdkPixbuf *pixbuf = NULL;
  gchar *file;
	
  const gchar *authors [] = {
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
      "Fabrice Alphonso <fabrice.alphonso@wanadoo.fr>",
      "Alp Toker <alp@atoker.com>",
      "Paul <paul666@mailandnews.com>",
      NULL
  };

  const gchar *translators [] = {
      N_("Internationalisation Maintainer:"),
      "Christian Rose <menthos@gnu.org>",
      NULL
  };
	
  authors [2] = gettext (authors [2]);
  authors [6] = gettext (authors [6]);
  translators [0] = gettext (translators [0]);
  
  const char *documenters [] = {
      NULL
  };

  /* Translators: Please write translator credits here, and
   * seperate names with \n */
  const char *translator_credits = _("translator_credits");

  file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "gnomemeeting-logo-icon.png", TRUE, NULL);
  pixbuf = gdk_pixbuf_new_from_file (file, NULL);
  g_free(file);

  abox = gnome_about_new ("GnomeMeeting",
			  VERSION,
			  /* Translators: Please change the (C) to a real
			     copyright character if your character set allows
			     it (Hint: iso-8859-1 is one of the character sets
			     that has this symbol). */
			  _("Copyright (C) 2000, 2002 Damien Sandras"),
                          /* Translators: Please test to see if your translation
                           * looks OK and fits within the box */
			  _("GnomeMeeting is a full-featured H.323\nvideo conferencing application."),
			  (const char **) authors,
                          (const char **) documenters,
                          strcmp (translator_credits, "translator_credits") != 0 ? 
                          translator_credits : "No translators, English by\n"
                          "Damien Sandras <dsandras@seconix.com>",
			  pixbuf);

  gtk_window_set_transient_for (GTK_WINDOW (abox), GTK_WINDOW (parent_window));
  gtk_window_present (GTK_WINDOW (abox));
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


/* DESCRIPTION  :  This callback is called when a gconf error happens
 * BEHAVIOR     :  Pop-up a message-box
 * PRE          :  /
 */
void gconf_error_callback (GConfClient *, GError *)
{
  GtkWidget *dialog = 
    gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
			    GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
			    _("An error has happened in the configuration"
			    " backend\nMaybe some of your settings won't"
			      " be stored"));

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}
