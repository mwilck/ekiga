/***************************************************************************
                          main_interface.h  -  description
                             -------------------
    begin                : Mon Mar 26 2001
    copyright            : (C) 2001 by Damien Sandras
    description          : Contains the functions to display the main interface
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


#ifndef _MAIN_INTERFACE_H
#define _MAIN_INTERFACE_H

#include <gnome.h>
#include <orb/orbit.h>
extern "C" {
#include <libgnorba/gnorba.h>
}

#include "common.h"
#include "config.h"

/******************************************************************************/
/* The GTK callbacks                                                          */
/******************************************************************************/

// DESCRIPTION  :  This callback is called when the main window is covered by
//                 another window or updated
// BEHAVIOR     :  update it
// PRE          :  gpointer is a valid pointer to GM_window_widgets
gint expose_event (GtkWidget *, GdkEventExpose *, gpointer);


// DESCRIPTION  :  This callback is called when the user change the
//                 audio settings sliders in the main notebook
// BEHAVIOR     :  update the volume
// PRE          :  gpointer is a valid pointer to GM_pref_window_widgets
void audio_volume_changed (GtkAdjustment *, gpointer);


// DESCRIPTION  :  This callback is called when the user change the 
//                 video brightness slider in the main notebook
// BEHAVIOR     :  update the value in real time
// PRE          :  gpointer is a valid pointer to GM_window_widgets
void brightness_changed (GtkAdjustment *, gpointer);


// DESCRIPTION  :  This callback is called when the user change the 
//                 video whiteness slider in the main notebook
// BEHAVIOR     :  update the value in real time
// PRE          :  gpointer is a valid pointer to GM_window_widgets
void whiteness_changed (GtkAdjustment *, gpointer);


// DESCRIPTION  :  This callback is called when the user change the 
//                 video colour slider in the main notebook
// BEHAVIOR     :  update the value in real time
// PRE          :  gpointer is a valid pointer to GM_window_widgets
void colour_changed (GtkAdjustment *, gpointer);


// DESCRIPTION  :  This callback is called when the user change the 
//                 video contrast slider in the main notebook
// BEHAVIOR     :  update the value in real time
// PRE          :  gpointer is a valid pointer to GM_window_widgets
void contrast_changed (GtkAdjustment *, gpointer);


// DESCRIPTION  :  This callback is called when the user clicks 
//                 on the preview button
// BEHAVIOR     :  Displays the webcam images
// PRE          :  /
void preview_button_clicked (GtkButton *, gpointer);


// DESCRIPTION  :  This callback is called when the user clicks 
//                 on the left arrow
// BEHAVIOR     :  If the Notebook page is not the first, switches to
//                 the previous page.
// PRE          :  /
void left_arrow_clicked (GtkWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user clicks 
//                 on the right arrow
// BEHAVIOR     :  If the Notebook page is not the last, switches to
//                 the next page.
// PRE          :  /
void right_arrow_clicked (GtkWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user clicks 
//                 on the silence detection button during a call
// BEHAVIOR     :  Enables/Disables silence detection
// PRE          :  /
void silence_detection_button_clicked (GtkWidget *, gpointer);

/******************************************************************************/
/* The functions to build the main window                                     */
/******************************************************************************/

// COMMON NOTICE : GM_window_widgets, GM_ldap_window_widgets and options 
//                 pointers must be valid,
//                 options must have been read before calling these functions

// BEHAVIOR     :  init things, build the initial gm window
void GM_init (GM_window_widgets *, GM_ldap_window_widgets *, options *, int, 
	      char **, char **);

// BEHAVIOR     :  builds the main interface
void GM_main_interface_init (GM_window_widgets *, options *);


// BEHAVIOR     :  builds the video settings part of the main notebook
void GM_init_main_interface_video_settings (GtkWidget *, GM_window_widgets *,
					    options *);


// BEHAVIOR     :  builds the audio settings part of the main notebook
void GM_init_main_interface_audio_settings (GtkWidget *, GM_window_widgets *,
					    options *);


// BEHAVIOR     :  builds the history log part of the main notebook
void GM_init_main_interface_log  (GtkWidget *, GM_window_widgets *,
				  options *);


// BEHAVIOR     :  builds the remote user info part of the main notebook
void GM_init_main_interface_remote_user_info (GtkWidget *, GM_window_widgets *,
					      options *);


// BEHAVIOR     :  Draw the logo
void GM_init_main_interface_logo (GM_window_widgets *);


// BEHAVIOR : add the text (char *) into the GtkWidget *
// PRE : GtkWidget * must be a pointer to a GtkText
void GM_log_insert (GtkWidget *, char *);
/******************************************************************************/

#endif
