
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
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
 *                         calls_history_window.h  -  description
 *                         -----------------------
 *   begin                : Sun Sep 1 2002
 *   copyright            : (C) 2000-2004 by Damien Sandras 
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
 * BEHAVIOR     :  Returns the list of all contacts for all calls. 
 * PRE          :  /
 */
GSList *gnomemeeting_calls_history_window_get_calls (GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Build the calls history window and returns a pointer to it.
 * PRE          :  /
 */
GtkWidget *gnomemeeting_calls_history_window_new ();


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Populate the calls history window.
 * PRE          :  /
 */
void gnomemeeting_calls_history_window_populate (GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Add a call to the calls history window.
 * PRE          :  /
 */
void gnomemeeting_calls_history_window_add_call (GtkWidget *,
						 int,
						 const char *, 
						 const char *,
						 const char *,
						 const char *,
						 const char *);

#endif /* _CALLS_HISTORY_WINDOW_H_ */
