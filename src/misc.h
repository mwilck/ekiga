
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
 *                         misc.h  -  description
 *                         ----------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains miscellaneous functions.
 *   Additional Code      : De Michele Cristiano, Miguel Rodríguez 
 *
 */


#ifndef _MISC_H_
#define _MISC_H_

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
gnomemeeting_button_new (const char *, 
			 GtkWidget *);


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
 * BEHAVIOR      : Displays a message on the statusbar or clears it if msg = 0.
 *                 Removes the previous message.
 * PRE           : The GnomeApp, followed by printf syntax format.
 */
void 
gnomemeeting_statusbar_push (GtkWidget *, const char *, ...);


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


gchar *gnomemeeting_get_utf8 (PString);

GtkWidget *
gnomemeeting_table_add_entry (GtkWidget *, gchar *, gchar *, gchar *, int, gboolean = false);

                                                                               
GtkWidget *
gnomemeeting_table_add_toggle (GtkWidget *, gchar *, gchar *, 
			       gchar *, int);


GtkWidget *
gnomemeeting_table_add_spin (GtkWidget *, gchar *, gchar *, gchar *,
			     double,
			     double, double, int, gchar * = NULL, gboolean = false); /* Can be true only if gchar != NULL */
void gnomemeeting_table_add_spin_range (GtkWidget *table,
				   gchar *label1_txt,
				   GtkWidget **spin1,
				   gchar *label2_text,
				   GtkWidget **spin2,
				   gchar *label3_text,
				   gchar *spin1_gconf_key,
				   gchar *spin2_gconf_key,
				   gchar *spin1_tooltip,
				   gchar *spin2_tooltip,
				   double spin1_min,
				   double spin2_min,
				   double spin1_max,
				   double spin2_max,
				   double spins_step,
					int row);

GtkWidget *
gnomemeeting_table_add_int_option_menu (GtkWidget *, gchar *, gchar **, 
					gchar *, gchar *,int);

/* Do not add default, but 1st element in the array */
GtkWidget *
gnomemeeting_table_add_string_option_menu (GtkWidget *, gchar *, gchar **,
					   gchar *, gchar *, int);
GtkWidget *
gnomemeeting_table_add_pstring_option_menu (GtkWidget *, gchar *, PStringArray,
					    gchar *, gchar *, int);

void
gnomemeeting_update_pstring_option_menu (GtkWidget *,
					 PStringArray,
					 gchar *);


GtkWidget *
gnomemeeting_vbox_add_table (GtkWidget *, gchar *, int, int);
#endif
