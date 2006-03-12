
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
 *                         calls_history_window.h  -  description
 *                         -----------------------
 *   begin                : Sun Sep 1 2002
 *   copyright            : (C) 2000-2006 by Damien Sandras 
 *   description          : This file declares functions to manage the
 *                          calls history
 */

#ifndef _CALLS_HISTORY_WINDOW_H_
#define _CALLS_HISTORY_WINDOW_H_

enum {
  RECEIVED_CALL,
  PLACED_CALL,
  MISSED_CALL,
  MAX_VALUE_CALL
};


/* The functions  */

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Build the calls history window and returns a pointer to it.
 * PRE          :  /
 */
GtkWidget *gm_calls_history_window_new ();


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Add a call to the calls history. This will update the 
 * 		   window through a notifier.
 * PRE          :  /
 */
void gm_calls_history_add_call (int,
				const char *, 
				const char *,
				const char *,
				const char *,
				const char *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Clear the calls history. It will update the window
 * 		   through a notifier.
 * PRE          :  /
 */
void gm_calls_history_clear (int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns the list of the last n contacts for all calls, 
 * 		   unique or not. 
 * PRE          :  /
 */
GSList *gm_calls_history_get_calls (int,
				    int,
				    gboolean,
				    gboolean);



#endif /* _CALLS_HISTORY_WINDOW_H_ */
