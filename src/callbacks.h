/***************************************************************************
                          callbacks.h  -  description
                             -------------------
    begin                : Sat Dec 23 2000
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : This file contains all the callbacks common to 
                           several files
    email                : dsandras@acm.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CALLBACK_H
#define _CALLBACK_H

#include <gnome.h>


/******************************************************************************/
/* The GTK callbacks                                                          */
/******************************************************************************/
// DESCRIPTION  :  This callback is called when the user chooses to pause
//                 the audio transmission
// BEHAVIOR     :  Pauses the audio transmission
// PRE          :  /
void pause_audio_callback (GtkWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user chooses to pause
//                 the video transmission
// BEHAVIOR     :  Pauses the video transmission
// PRE          :  /
void pause_video_callback (GtkWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user chooses to open
//                 the ILS window
// BEHAVIOR     :  If the ILS window is not already open, open it
// PRE          :  gpointer is a valid pointer to a valid GM_window_widgets
void ldap_callback (GtkButton *, gpointer);


// DESCRIPTION  :  This callback is called when the user choose to hide
//                 or show the main window
// BEHAVIOR     :  Hide or show the main window
// PRE          :  /
void toggle_window_callback (GtkWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user chooses to open
//                 the about window
// BEHAVIOR     :  opens the about window
// PRE          :  /
void about_callback (GtkWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user choose to establish
//                 a connection
// BEHAVIOR     :  Calls the remote endpoint or accept the incoming call
// PRE          :  /
void connect_cb (GtkWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user choose to stop
//                 a connection
// BEHAVIOR     :  doesn't accept the incoming call or stops the current call
// PRE          :  /
void disconnect_cb (GtkWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user chooses to open
//                 the preferences window
// BEHAVIOR     :  If the preferenced window is not already open, open it
// PRE          :  gpointer is a valid pointer to a valid GM_window_widgets
void pref_callback (GtkWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user chooses to quit
// BEHAVIOR     :  Asks the Are you sure to quit question ...
// PRE          :  /
void quit_callback (GtkWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the clicks on yes or no
//                 in the Are you sure to quit popup
// BEHAVIOR     :  Quit  or not
// PRE          :  /
void gtk_main_quit_callback (int, gpointer);



// DESCRIPTION  :  This callback is called when the user chooses to display
//                 the local webcam image
// BEHAVIOR     :  Sets default to local
// PRE          :  /
void popup_menu_local_callback (GtkWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user chooses to display
//                 the remote webcam image
// BEHAVIOR     :  Sets default to remote
void popup_menu_remote_callback (GtkWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user chooses to display
//                 the both webcam images
// BEHAVIOR     :  Sets default to both
// PRE          :  /
void popup_menu_both_callback (GtkWidget *, gpointer);

void ldap_popup_menu_callback (GtkWidget *, gpointer);
/******************************************************************************/

#endif
