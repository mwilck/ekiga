
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
#include "common.h"


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Takes the GDK lock if we are not in the main thread.
 * PRE          :  Must not be called instead of gdk_threads_enter in timers
 *                 or idle functions, because they are executed in the main
 *                 thread.
 */
void gnomemeeting_threads_enter ();


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Releases the GDK lock if we are not in the main thread.
 * PRE          :  Must not be called instead of gdk_threads_leave in timers
 *                 or idle functions, because they are executed in the main
 *                 thread.
 */
void gnomemeeting_threads_leave ();


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Creates a button with the GtkWidget * as pixmap and the label
 *                 as label.
 * PRE          :  /
 */
GtkWidget *gnomemeeting_button (char *, GtkWidget *);


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Returns the structure of widgets of the main window.
 * PRE          :  The GtkWidget must be a pointer to the Main gnomeMeeting 
 *                 window.
 */
GM_window_widgets *gnomemeeting_get_main_window (GtkWidget *);


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Returns the structure of widgets of the prefs window.
 * PRE          :  The GtkWidget must be a pointer to the Main gnomeMeeting 
 *                 window.
 */
GM_pref_window_widgets *gnomemeeting_get_pref_window (GtkWidget *);


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Returns the structure of widgets of the ldap window.
 * PRE          :  The GtkWidget must be a pointer to the Main gnomeMeeting 
 *                 window.
 */
GM_ldap_window_widgets *gnomemeeting_get_ldap_window (GtkWidget *);


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Returns the structure of widgets of the chat window.
 * PRE          :  The GtkWidget must be a pointer to the Main gnomeMeeting 
 *                 window.
 */
GmTextChat *gnomemeeting_get_chat_window (GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Add some texts to the log part of the main window.
 * PRE          :  /
 */
void gnomemeeting_log_insert (gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Displays the gnomemeeting logo in the drawing area.
 * PRE          :  /
 */
void gnomemeeting_init_main_window_logo ();


/* DESCRIPTION  :  This callback is called by a timer function.
 * BEHAVIOR     :  Plays the sound choosen in the gnome control center.
 * PRE          :  The pointer to the docklet must be valid.
 */
gint PlaySound (GtkWidget *);


extern GtkWidget* 
gnomemeeting_history_combo_box_new (const gchar *key);

/* DESCRIPTION   :  /
 * BEHAVIOR      : Add a new entry to the history combo and saves it
 *                 in the gconf db.
 * PRE           : /
 */
void 
gnomemeeting_history_combo_box_add_entry (GtkCombo *, const gchar *, const gchar *);

void gnomemeeting_warning_popup (GtkWidget *, gchar *);

/* DESCRIPTION   :  /
 * BEHAVIOR      : Frees data in a double linked list
 * PRE           : the list must have dinamically alocated data
 */
void gnomemeeting_free_glist_data (gpointer user_data);

#endif
