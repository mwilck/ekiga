
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

#include "../config.h"


#include <gnome.h>
#include <ptlib.h>
#include <h323.h>

#include "common.h"
#include "chat_window.h"
#include "endpoint.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "common.h"


extern GtkWidget *gm;
extern GnomeMeeting *MyApp;


static void chat_entry_activate (GtkEditable *w, gpointer data)
{
  /* Get the structs from the application */
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  GMH323EndPoint *endpoint = MyApp->Endpoint ();
  PString s;

  // For testing purposes.
  // s = PString (gtk_entry_get_text (GTK_ENTRY (w)));
  // gnomemeeting_chat_window_text_insert ("Kenneth", s, 0);
  // gtk_entry_set_text (GTK_ENTRY (w), "");

  if (endpoint) {

    PString local = endpoint->GetLocalUserName ();

    PINDEX bracket = local.Find('[');
    if (bracket != P_MAX_INDEX)
      local = local.Left (bracket);
    
    bracket = local.Find('(');
    if (bracket != P_MAX_INDEX)
      local = local.Left (bracket);
    
    if (endpoint->GetCallingState () == 2) {

      H323Connection *connection = endpoint->GetCurrentConnection ();

      if (connection != NULL)  {
	
	s = PString (gtk_entry_get_text (GTK_ENTRY (w)));
 	connection->SendUserInput ("MSG"+s);

	gnomemeeting_chat_window_text_insert (local, s, 0);

	gtk_entry_set_text (GTK_ENTRY (w), "");
      }
    }
  }
}


void gnomemeeting_chat_window_text_insert (PString local, PString str, int user)
{
  gchar *msg = NULL;
  GtkTextIter iter;
  GtkTextMark *mark;
  static gboolean first = TRUE;

  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  gtk_text_buffer_get_end_iter (gw->chat_buffer, &iter);

  if (first) 
  {
          msg = g_strdup_printf ("%s: ", (const char *) local);
          first = FALSE;
  }
  else
  msg = g_strdup_printf ("\n%s: ", (const char *) local);

  if (user == 1)
  {
          gtk_text_buffer_insert_with_tags_by_name (gw->chat_buffer, &iter, 
                                                    msg, -1,                         
                                                    "primary-user", NULL);
  } else {
          gtk_text_buffer_insert_with_tags_by_name (gw->chat_buffer, &iter, 
                                                    msg, -1,                         
                                                    "secondary-user", NULL);
  }
  
  g_free (msg);

  msg = g_strdup_printf ("%s", (const char *) str);

  gtk_text_buffer_insert (gw->chat_buffer, &iter, msg, -1);

  g_free (msg);

  mark = gtk_text_buffer_get_mark (gw->chat_buffer, "current-position");

  gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (gw->chat_view), mark, 
                                0.0, FALSE, 0,0);
}


void gnomemeeting_init_chat_window ()
{
  GtkWidget *entry;
  GtkWidget *scr;
  GtkWidget *label;
  GtkWidget *table;

  GtkTextTag *primary_user;
  GtkTextTag *secondary_user;

  GtkTextIter  iter;
  GtkTextMark *mark;

  /* Get the structs from the application */
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  gw->chat_window = gtk_frame_new (_("Text Chat"));

  table = gtk_table_new (1, 3, FALSE);

  gtk_container_set_border_width (GTK_CONTAINER (table), 
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (gw->chat_window), table);

  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);
  gtk_widget_set_size_request (GTK_WIDGET (scr), 225, -1);

  gw->chat_view = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (gw->chat_view), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (gw->chat_view), GTK_WRAP_WORD);

  gw->chat_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (gw->chat_view));

  gtk_text_buffer_get_end_iter (gw->chat_buffer, &iter);
  
  mark = gtk_text_buffer_create_mark (gw->chat_buffer, "current-position", &iter, FALSE);

  gtk_text_buffer_create_tag (gw->chat_buffer, "primary-user",
			      "foreground", "red", 
                              "weight", 900, NULL); 

  gtk_text_buffer_create_tag (gw->chat_buffer, "secondary-user",
			      "foreground", "blue", 
                              "weight", 900, NULL);

  gtk_container_add (GTK_CONTAINER (scr), gw->chat_view);
  
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
  gtk_widget_set_size_request (GTK_WIDGET (entry), 225, -1);
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (entry), 
		    0, 1, 2, 3,
		    (GtkAttachOptions) (NULL),
		    (GtkAttachOptions) (NULL),
		    0, 0);

  g_signal_connect (GTK_OBJECT (entry), "activate",
                    G_CALLBACK (chat_entry_activate), gw->chat_view);

}
