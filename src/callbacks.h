
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
 *                         callbacks.h  -  description
 *                         ---------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains callbacks common to several
 *                          files.
 *   email                : dsandras@seconix.com
 *
 */

#ifndef _CALLBACKS_H
#define _CALLBACKS_H

#include <gtk/gtkwidget.h>
#include <gconf/gconf-client.h>


/* DESCRIPTION  :  This callback is called when the user chooses to pause
 *                 the audio transmission.
 * BEHAVIOR     :  Pause the audio transmission.
 * PRE          :  /
 */
void pause_audio_callback (GtkWidget *, gpointer);


/* DESCRIPTION  :  This callback is called when the user chooses to pause
 *                 the video transmission.
 * BEHAVIOR     :  Pause the video transmission
 * PRE          :  /
 */
void pause_video_callback (GtkWidget *, gpointer);


/* DESCRIPTION  :  This callback is called when the user choose to hide
 *                 or show the main window.
 * BEHAVIOR     :  Hide or show the main window.
 * PRE          :  /
 */
void toggle_window_callback (GtkWidget *, gpointer);


/* DESCRIPTION  :  This callback is called when the user chooses to open
 *                 the about window.
 * BEHAVIOR     :  Open the about window.
 * PRE          :  /
 */
void about_callback (GtkWidget *, gpointer);


/* DESCRIPTION  :  This callback is called when the user choose to establish
 *                 a connection.
 * BEHAVIOR     :  Call the remote endpoint or accept the incoming call.
 * PRE          :  /
 */
void connect_cb (GtkWidget *, gpointer);


/* DESCRIPTION  :  This callback is called when the user choose to stop
 *                 a connection.
 * BEHAVIOR     :  Do not accept the incoming call or stops the current call.
 * PRE          :  /
 */
void disconnect_cb (GtkWidget *, gpointer);


/* DESCRIPTION  :  This callback is called when the user chooses to open
 *                 or close some component.
 * BEHAVIOR     :  Shows or hide it.
 * PRE          :  gpointer is a valid pointer to the widget.
 */
void gnomemeeting_component_view (GtkWidget *, gpointer);


/* DESCRIPTION  :  This callback is called when the user chooses to quit.
 * BEHAVIOR     :  Quit.
 * PRE          :  /
 */
void quit_callback (GtkWidget *, gpointer);


/* DESCRIPTION  :  Quit callback.
 * BEHAVIOR     :  Quit  or not.
 * PRE          :  /
 */
void gtk_main_quit_callback (int, gpointer);


/* DESCRIPTION  :  This callback is called when the user chooses to display
 *                 the local webcam image.
 * BEHAVIOR     :  Set default to local.
 * PRE          :  /
 */
void popup_menu_local_callback (GtkWidget *, gpointer);


/* DESCRIPTION  :  This callback is called when the user chooses to display
 *                 the remote webcam image
 * BEHAVIOR     :  Set default to remote.
 * PRE          :  /
 */
void popup_menu_remote_callback (GtkWidget *, gpointer);


/* DESCRIPTION  :  This callback is called when the user chooses to display
 *                 the both webcam images.
 * BEHAVIOR     :  Set default to both.
 * PRE          :  /
 */
void popup_menu_both_callback (GtkWidget *, gpointer);


/* DESCRIPTION  :  This callback is called when the user clicks on "Call
 *                 this user" in the ldap window.
 * BEHAVIOR     :  Add the user name in the combo box and call him.
 * PRE          :  /
 */
void ldap_popup_menu_callback (GtkWidget *, gpointer);

/* DESCRIPTION  :  This callback is called when a gconf error happens
 * BEHAVIOR     :  Pop-up a message-box
 * PRE          :  /
 */
void gconf_error_callback (GConfClient *, GError *);


#endif
