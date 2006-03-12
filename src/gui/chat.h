
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         chat_window.h  -  description
 *                         -----------------------------
 *   begin                : Wed Jan 23 2002
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains functions to build the chat
 *                          window. It uses DTMF tones.
 *   Additional code      : Kenneth Christiansen  <kenneth@gnu.org>
 *
 */


#ifndef __CHAT_WINDOW_H
#define __CHAT_WINDOW_H

#include "common.h"

G_BEGIN_DECLS

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Initializes the text chat view.
 * PRE          :  /
 */
GtkWidget *gm_text_chat_window_new ();


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Adds a page in the text chat window.
 * PRE          :  The text chat window, the contact URL and name.
 */
GtkWidget *gm_text_chat_window_add_tab (GtkWidget *, 
					const char *,
					const char *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns TRUE if the chat window already has a tab
 * 		   for the given URL.
 * PRE          :  The text chat window, the contact URL and name.
 */
gboolean  gm_text_chat_window_has_tab (GtkWidget *, 
				       const char *);

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Displays the colored text chat message,
 *		   with some enhancements (context menu
 *		   for uris, graphics for smileys, etc)
 * PRE          :  The name of the (local or remote) user, the remote url,
 * 		   the message and 0 / 1 / 2 for local / remote user / error.
 */
void gm_text_chat_window_insert (GtkWidget *, 
				 const char *,
				 const char *,
				 const char *,
				 int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the urls in the cache of the url entry. It is done
 * 		   using the list of the last 100 given/received/missed calls,
 * 		   but also using the address book contacts.
 * PRE          :  The chat window GMObject.
 */
void gm_text_chat_window_urls_history_update (GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the chat window sensitivity and state following 
 * 		   the given calling state for the given url (if NULL, update
 * 		   all tabs).
 * PRE          :  The main window GMObject.
 * 		   A valid GMManager calling state.
 */
void gm_chat_window_update_calling_state (GtkWidget *,
					  const char *,
					  const char *,
					  unsigned);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Displays info message on the statusbar during a few seconds.
 *                 Removes the previous message. The message is only displayed
 *                 if the current tab is the one for the given URL (or if the
 *                 given URL is NULL).
 * PRE           : The main window GMObject, followed by printf syntax format.
 */
void gm_chat_window_push_info_message (GtkWidget *, 
				       const char *,
				       const char *, 
				       ...);

G_END_DECLS

#endif /* __CHAT_WINDOW_H */
