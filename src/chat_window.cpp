
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2003 Damien Sandras
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
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         chat_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Wed Jan 23 2002
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains functions to build the chat
 *                          window. It uses DTMF tones.
 *   Additional code      : Kenneth Christiansen  <kenneth@gnu.org>
 *
 */


#include "../config.h"

#include "chat_window.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "gtk-text-buffer-extentions.h"


extern GtkWidget *gm;
extern GnomeMeeting *MyApp;


static void chat_entry_activate (GtkEditable *w, gpointer data)
{
  GMH323EndPoint *endpoint = MyApp->Endpoint ();
  PString s;
    
  if (endpoint) {
        
    PString local = endpoint->GetLocalUserName ();
    /* The local party name has to be converted to UTF-8, but not
       the text */
    gchar *utf8_local = NULL;


    if (endpoint->GetCallingState () == 2) {
            
      H323Connection *connection = endpoint->GetCurrentConnection ();
            
      if (connection != NULL)  {
                
	s = PString (gtk_entry_get_text (GTK_ENTRY (w)));
	connection->SendUserInput ("MSG"+s);

	if (g_utf8_validate ((gchar *) (const unsigned char*) local, -1, NULL))
	  utf8_local = g_strdup ((char *) (const char *) (local));
	else
	  utf8_local = gnomemeeting_from_iso88591_to_utf8 (local);

	if (utf8_local)
	  gnomemeeting_text_chat_insert (utf8_local, s, 0);
	g_free (utf8_local);
                
	gtk_entry_set_text (GTK_ENTRY (w), "");
      }
    }
  }
}


void 
gnomemeeting_text_chat_insert (PString local, PString str, int user)
{
  gchar *msg = NULL;
  GtkTextIter iter;
  GtkTextMark *mark;
  
  GmTextChat *chat = gnomemeeting_get_chat_window (gm);

  gtk_text_buffer_get_end_iter (chat->text_buffer, &iter);

  if (chat->buffer_is_empty)
  {
    msg = g_strdup_printf ("%s: ", (const char *) local);
    chat->buffer_is_empty = FALSE;
  }
  else
    msg = g_strdup_printf ("\n%s: ", (const char *) local);

  if (user == 1)
    gtk_text_buffer_insert_with_tags_by_name (chat->text_buffer, &iter, msg, 
					      -1, "primary-user", NULL);
  else
    gtk_text_buffer_insert_with_tags_by_name (chat->text_buffer, &iter, msg, 
					      -1, "secondary-user", NULL);
  
  g_free (msg);
  
  gtk_text_buffer_insert_with_emoticons (chat->text_buffer, &iter, 
					 (const char *) str);

  mark = gtk_text_buffer_get_mark (chat->text_buffer, "current-position");

  gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (chat->text_view), mark, 
				0.0, FALSE, 0,0);
}


GtkWidget *gnomemeeting_text_chat_init ()
{
  GtkWidget *entry;
  GtkWidget *scr;
  GtkWidget *label;
  GtkWidget *table;
  GtkWidget *frame;
  GtkWidget *chat_window;

  GtkTextIter  iter;
  GtkTextMark *mark;

  /* Get the structs from the application */
  GmTextChat *chat = gnomemeeting_get_chat_window (gm);

  chat_window = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (chat_window), GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (chat_window), 7);
  table = gtk_table_new (1, 3, FALSE);

  gtk_container_set_border_width (GTK_CONTAINER (table), 
				  2);
  gtk_container_add (GTK_CONTAINER (chat_window), table);

  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);
  gtk_widget_set_size_request (GTK_WIDGET (scr), 245, 133);

  chat->text_view = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (chat->text_view), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (chat->text_view),
			       GTK_WRAP_WORD);

  chat->text_buffer = 
    gtk_text_view_get_buffer (GTK_TEXT_VIEW (chat->text_view));

  gtk_text_buffer_get_end_iter (chat->text_buffer, &iter);
  gtk_text_view_set_cursor_visible  (GTK_TEXT_VIEW (chat->text_view), false);

  mark = gtk_text_buffer_create_mark (chat->text_buffer, 
				      "current-position", &iter, FALSE);

  gtk_text_buffer_create_tag (chat->text_buffer, "primary-user",
			      "foreground", "red", 
			      "weight", 900, NULL);

  gtk_text_buffer_create_tag (chat->text_buffer, "secondary-user",
			      "foreground", "blue", 
			      "weight", 900, NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (scr), chat->text_view);
  gtk_container_add (GTK_CONTAINER (frame), scr);
  
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (frame), 
		    0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    0, 0);

  label = gtk_label_new (_("Send Message:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (label), 
		    0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    0, 0);

  entry = gtk_entry_new ();
  gtk_widget_set_size_request (GTK_WIDGET (entry), 245, -1);
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (entry), 
		    0, 1, 2, 3,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    0, 0);

  g_signal_connect (GTK_OBJECT (entry), "activate",
		    G_CALLBACK (chat_entry_activate), chat->text_view);

  chat->buffer_is_empty = TRUE;

  return chat_window;
}
