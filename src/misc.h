
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
 *                         misc.h  -  description
 *                         ----------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *                           and De Michele Cristiano
 *   description          : This file contains miscellaneous functions.
 *   email                : dsandras@seconix.comi, demichel@na.infn.it
 *
 */


#ifndef _MISC_H_
#define _MISC_H_


#include <gtk/gtkcombo.h>
#include <gtk/gtkwidget.h>
#include <ptclib/asner.h>

#include "common.h"


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Takes the GDK lock if we are not in the main thread.
 * PRE          :  Must not be called instead of gdk_threads_enter in timers
 *                 or idle functions, because they are executed in the main
 *                 thread.
 */
void 
gnomemeeting_threads_enter ();


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Releases the GDK lock if we are not in the main thread.
 * PRE          :  Must not be called instead of gdk_threads_leave in timers
 *                 or idle functions, because they are executed in the main
 *                 thread.
 */
void 
gnomemeeting_threads_leave ();


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Creates a button with the GtkWidget * as pixmap 
 *                 and the label as label.
 * PRE          :  /
 */
GtkWidget *
gnomemeeting_button (char *, GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Add text (gchar *) with timestamps into the given 
 *                 GtkTextView.
 * PRE          :  The text to add, and the text view to add the text into.
 */
void 
gnomemeeting_log_insert (GtkWidget *, gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Displays the gnomemeeting logo in the drawing area.
 * PRE          :  The GtkImage where to put the logo (pixbuf).
 */
void 
gnomemeeting_init_main_window_logo (GtkWidget *);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Creates a new incoming call popup and returns it.
 * PRE           : The name; and the app UTF-8 char *.
 */
GtkWidget * 
gnomemeeting_incoming_call_popup_new (gchar *, gchar *);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Flashes a message on the statusbar during a few seconds.
 *                 Removes the previous message.
 * PRE           : The GnomeApp, followed by printf syntax format.
 */
void 
gnomemeeting_statusbar_flash (GtkWidget *, const char *, ...);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Creates a video window.
 * PRE           : The title of the window, the drawing area, and the width and
 *                 height.
 */
GtkWidget *
gnomemeeting_video_window_new (gchar *, GtkWidget *&, int, int);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Takes a PString and returns the Left part before a [ or a (.
 * PRE           : An non-empty PString.
 */
PString 
gnomemeeting_pstring_cut (PString);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Takes an ISO-8859-1 encoded PString, and returns an UTF-8
 *                 encoded string.
 * PRE           : An ISO-8859-1 encoded PString.
 */
gchar *
gnomemeeting_from_iso88591_to_utf8 (PString);
#endif
