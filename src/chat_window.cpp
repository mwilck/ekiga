
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

#include "common.h"
#include "endpoint.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "common.h"

#include "../config.h"


extern GtkWidget *gm;
extern GnomeMeeting *MyApp;


static void chat_entry_activate (GtkEditable *w, gpointer data)
{
  gchar *msg = NULL;
  /* Get the structs from the application */
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  GMH323EndPoint *endpoint = MyApp->Endpoint ();
  PString s;
  
  if (endpoint) {

    if (endpoint->GetCallingState () == 2) {

      H323Connection *connection = 
	endpoint->FindConnectionWithLock (endpoint->GetCurrentCallToken ());

      if (connection != NULL)  {
	
	s = PString (gtk_entry_get_text (GTK_ENTRY (w)));
 	connection->SendUserInput (s);
 	connection->Unlock();
	msg = g_strdup_printf ("%s: %s\n", "You", (const char *) s);
	gtk_text_insert (GTK_TEXT (gw->chat_text), NULL, NULL, NULL, msg, -1);
	g_free (msg);
	
	gtk_entry_set_text (GTK_ENTRY (w), "");
      }  
    }
  }
}


void gnomemeeting_init_chat_window ()
{
  GtkWidget *entry;
  GtkWidget *scr;
  GtkWidget *label;
  GtkWidget *table;


  /* Get the structs from the application */
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  /* a vbox to put the frames and the user list */
  gw->chat_window = gtk_frame_new (_("Text Chat"));

  table = gtk_table_new (1, 3, FALSE);

  gtk_container_set_border_width (GTK_CONTAINER (table), 
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (gw->chat_window), table);

  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);
  gtk_widget_set_usize (GTK_WIDGET (scr), 225, 0);

  gw->chat_text = gtk_text_new (NULL, NULL);
  gtk_text_set_line_wrap (GTK_TEXT (gw->chat_text), TRUE);
  gtk_text_set_word_wrap (GTK_TEXT (gw->chat_text), TRUE);
  gtk_text_set_editable (GTK_TEXT (gw->chat_text), FALSE);

  gtk_container_add (GTK_CONTAINER (scr), gw->chat_text);

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (scr), 
		    0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    0, 0);

  label = gtk_label_new (_("Send Message:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (label), 
		    0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    0, 0);

  entry = gtk_entry_new ();
  gtk_widget_set_usize (GTK_WIDGET (entry), 225, 0);
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (entry), 
		    0, 1, 2, 3,
		    (GtkAttachOptions) (NULL),
		    (GtkAttachOptions) (NULL),
		    0, 0);

  gtk_signal_connect (GTK_OBJECT (entry), "activate",
		      GTK_SIGNAL_FUNC (chat_entry_activate), gw->chat_text);

}
