                                                                                
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
 *                         log_window.h  -  description
 *                         -----------------------
 *   begin                : Sun Sep 1 2002
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file declares functions to manage the log
 *
 */

#include <gtk/gtk.h>

#ifndef _LOG_WINDOW_H_
#define _LOG_WINDOW_H_


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Build the log window and returns it.
 * PRE          :  /
 */
GtkWidget *gnomemeeting_log_window_new ();

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Add text (gchar *) with timestamps into the log textview
 * PRE          :  The text to add (printf compatible)
 */
void
gnomemeeting_log_insert (GtkWidget *, const char *,
                         ...);

#endif /* _LOG_WINDOW_H_ */
