
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
 *                         chat_window.h  -  description
 *                         -----------------------------
 *   begin                : Wed Jan 23 2002
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains functions to build the chat
 *                          window. It uses DTMF tones.
 *   Additional code      : Kenneth Christiansen  <kenneth@gnu.org>
 *
 */


#ifndef __CHAT_WINDOW_H
#define __CHAT_WINDOW_H

#include "common.h"


G_BEGIN_DECLS


/**
 * gnomemeeting_text_chat_init:
 *
 * Initializes the text chat view.
 **/
GtkWidget *gnomemeeting_text_chat_init ();


/**
 * gnomemeeting_text_chat_insert:
 *
 * @local is name of user.
 * @str is the string to insert. Most well known smilies are 
 * converted to beautiful icons.
 * @user is 1 for local user or 2 for remote (check this - guess)
 *
 * Inserts a text into the text chat. If the text contains smilies
 * it will try to show graphical emoticons instead.
 **/ 
void gnomemeeting_text_chat_insert (PString local, PString str, int user);

G_END_DECLS

#endif /* __CHAT_WINDOW_H */
