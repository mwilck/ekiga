
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
 *                         chat_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Wed Jan 23 2002
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains functions to build the chat
 *                          window. It uses DTMF tones.
 *   email                : dsandras@seconix.com
 *
 */


#include <gnome.h>
#include <ptlib.h>
#include <h323.h>

#include "endpoint.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "common.h"


extern GtkWidget *gm;
extern GnomeMeeting *MyApp;

/* DESCRIPTION  :  This callback is called when the user clicks
 *                 closes the window (destroy or delete_event signals).
 * BEHAVIOR     :  Hide the window.
 * PRE          :  gpointer is a valid pointer to a GM_ldap_window_widgets.
 */
static void chat_window_clicked (GnomeDialog *widget, int button, gpointer data)
{
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  if (GTK_WIDGET_VISIBLE (gw->chat_window))
    gtk_widget_hide_all (gw->chat_window);
}


static void chat_entry_activate (GtkWidget *w, gpointer data)
{
  gchar *msg = NULL;
  GMH323EndPoint *endpoint = MyApp->Endpoint ();
  PString s;
  
  /* Get the structs from the application */
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  if (endpoint) {

    if (endpoint->GetCallingState () == 2) {

      H323Connection *connection = 
	endpoint->FindConnectionWithLock (endpoint->GetCurrentCallToken ());
    
      if (connection != NULL)  {
	
	s = PString (gtk_entry_get_text (GTK_ENTRY (w)));
	connection->SendUserInput (s);
	connection->Unlock();
	
	msg = g_strdup_printf ("%s: %s\n", "You:", (const char *) s);
	gtk_text_insert (GTK_TEXT (gw->chat_text), NULL, NULL, NULL, msg, -1);
	g_free (msg);
	
	gtk_entry_set_text (GTK_ENTRY (w), "");
      }  
    }
  }
}


void gnomemeeting_init_chat_window ()
{
  GtkWidget *vbox;
  GtkWidget *entry;
  GtkWidget *scr;

  /* Get the structs from the application */
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  gw->chat_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_policy (GTK_WINDOW (gw->chat_window), 
			 FALSE, FALSE, TRUE);
  gtk_window_set_title (GTK_WINDOW (gw->chat_window), 
			_("Chat Window"));
  gtk_window_set_position (GTK_WINDOW (gw->chat_window), 
			   GTK_WIN_POS_CENTER);

  /* a vbox to put the frames and the user list */
  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (gw->chat_window), vbox);

 
  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);

  gw->chat_text = gtk_text_new (NULL, NULL);
  gtk_text_set_line_wrap (GTK_TEXT (gw->chat_text), TRUE);
  gtk_text_set_word_wrap (GTK_TEXT (gw->chat_text), TRUE);
  gtk_text_set_editable (GTK_TEXT (gw->chat_text), FALSE);
  gtk_widget_set_usize (GTK_WIDGET (gw->chat_text), 283, 130);

  gtk_box_pack_start (GTK_BOX (vbox), scr, FALSE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (scr), gw->chat_text);

  entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);

  gtk_signal_connect (GTK_OBJECT (entry), "activate",
		      GTK_SIGNAL_FUNC (chat_entry_activate), NULL);

  gtk_signal_connect (GTK_OBJECT(gw->chat_window), "delete_event",
		      GTK_SIGNAL_FUNC(chat_window_clicked), NULL);
  
  gtk_signal_connect (GTK_OBJECT (gw->chat_window), "destroy",
		      GTK_SIGNAL_FUNC (chat_window_clicked), NULL);  
}
