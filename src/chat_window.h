
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
 *                         chat_window.h  -  description
 *                         -----------------------------
 *   begin                : Wed Jan 23 2002
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains functions to build the chat
 *                          window. It uses DTMF tones.
 *   email                : dsandras@seconix.com
 *
 */


#include <ptlib.h>


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Build the chat window.
 * PRE          :  /
 */
void gnomemeeting_init_text_chat_window ();


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Add text to the chat window, first PString is user name, second
 *                 is the text to add. If int = 0 then the user name will be in blue,
 *                 otherwise it is in red.
 * PRE          :  /
 */
void gnomemeeting_text_chat_insert (PString, PString, int);
