/***************************************************************************
                          preferences.h
                             -------------------
    begin                : Tue Dec 26 2000
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : This file contains all functions needed to create
                           the preferences window and all its callbacks
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


#ifndef _PREFERENCES_H_
#define _PREFERENCES_H_

#include <gnome.h>
#include <stdlib.h>
#include <pthread.h>

#include "common.h"
#include "config.h"


/******************************************************************************/
/* The GTK callbacks                                                          */
/******************************************************************************/

// COMMON NOTICE:  GM_pref_window_widgets is a structure containing pointers
//                 to all widgets created during the construction of the
//                 preferences window (see common.h for the exact content)
//                 that are needed for callbacks or other functions
//                 This structure is recreated / destroyed with each call.
//                 GM_window_widgets is a structure containing pointers
//                 to all widgets created during the construction of the
//                 main window that will be needed later.
//                 They are created at the startup of GnomeMeeting and destroyed
//                 at the end of the execution.


// DESCRIPTION  :  This callback is called when the user clicks
//                 on a button : OK, CANCEL, APPLY, or closed by the WM
// BEHAVIOR     :  OK, CANCEL and WM free things and destroy the window
//                 APPLY saves the options and apply them for future calls
// PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets
void pref_window_clicked (GnomeDialog *, int, gpointer);


// DESCRIPTION  :  This callback is called when the pref window is destroyed
// BEHAVIOR     :  prevents the destroy
// PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets
gint pref_window_destroyed (GtkWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user clicks
//                 on a button in the Audio Codecs Settings (Add, Del, Up, Down)
// BEHAVIOR     :  It updates the clist order or the clist data following the
//                 operation (Up => up, Add => changes row pixmap and set
//                 row data to 1)
// PRE          :  gpointer is a valid pointer to a char containing
//                 the operation (Add / Del / Up / Down)
void button_clicked (GtkWidget *, gpointer);


// DESCRIPTION  :  This callback is called when the user clicks
//                 on a row of the codecs clist in the Audio Codecs Settings
// BEHAVIOR     :  It updates the GM_pref_window_widgets * content (row_avail
//                 field is set to the last selected row)
// PRE          :  gpointer is a valid pointer to the GM_pref_window_widgets
void clist_row_selected (GtkWidget *, gint, gint,
			 GdkEventButton *, gpointer);


// DESCRIPTION  :  This callback is called when the user clicks
//                 on a row of the ctree containing all the sections
// BEHAVIOR     :  It changes the notebook page to the selected one 
// PRE          :
void ctree_row_selected (GtkWidget *, gint, gint,
			 GdkEventButton *, gpointer);


// DESCRIPTION  :  This callback is called when the user changes
//                 a ldap related option
// BEHAVIOR     :  It sets a flag to say that LDAP has to be reactivated 
// PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets
void ldap_option_changed (GtkEditable *, gpointer);


// DESCRIPTION  :  This callback is called when the user changes
//                 the audio device
// BEHAVIOR     :  It sets a flag to say that the new audio mixers have to
//                 be tested
// PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets
void audio_mixer_changed (GtkEditable *, gpointer);


// DESCRIPTION  :  This callback is called when the user clicks
//                 on the video transmission toggle button, or change the video
//                 device, or the video channel
// BEHAVIOR     :  It sets a flag 
// PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets
void vid_tr_changed (GtkToggleButton *, gpointer);


// DESCRIPTION  :  This callback is called when the user changes
//                 any gatekeeper option
// BEHAVIOR     :  It sets a flag 
// PRE          :  gpointer is a valid pointer to a GM_pref_window_widgets
void gk_option_changed (GtkWidget *, gpointer);
/******************************************************************************/


/******************************************************************************/
/* The different functions to build the preferences window                    */
/******************************************************************************/

// DESCRIPTION  :  /
// BEHAVIOR     :  It builds the preferences window
//                 (sections' ctree / Notebook pages) and connect GTK signals
//                 to appropriate callbacks
// PRE          :  The first parameter is the calling_state when the window
//                 is created.  The second one a pointer
//                 to a valid GM_window_widgets
void GMPreferences (int, GM_window_widgets *);


// DESCRIPTION  :  /
// BEHAVIOR     :  It builds the notebook page for audio codecs settings and
//                 add it to the notebook, default values are set from the
//                 options struct given as parameter
// PRE          :  parameters have to be valid
//                 * 1 : pointer to the notebook
//                 * 2 : pointer to valid GM_pref_window_widgets
//                 * 3 : calling_state such as when creating the pref window
//                 * 4 : pointer to valid options read in the config file
 void init_pref_audio_codecs (GtkWidget *, GM_pref_window_widgets *,
			      int, options *);


// DESCRIPTION  :  
// BEHAVIOR     :  It builds the notebook page for the codecs settings and
//                 add it to the notebook, default values are set from the
//                 options struct given as parameter
// PRE          :  See init_pref_audio_codecs
 void init_pref_codecs_settings (GtkWidget *, GM_pref_window_widgets *,
			         int, options *);


// DESCRIPTION  :  
// BEHAVIOR     :  It builds the notebook page for interface settings
//                 add it to the notebook, default values are set from the
//                 options struct given as parameter
// PRE          :  See init_pref_audio_codecs
 void init_pref_interface (GtkWidget *, GM_pref_window_widgets *,
			   int, options *);


// DESCRIPTION  :  /
// BEHAVIOR     :  It builds the notebook page for general settings and
//                 add it to the notebook, default values are set from the
//                 options struct given as parameter
// PRE          :  See init_pref_audio_codecs
 void init_pref_general (GtkWidget *, GM_pref_window_widgets *,
			 int, options *);


// DESCRIPTION  :  /
// BEHAVIOR     :  It builds the notebook page for advanced settings and
//                 add it to the notebook, default values are set from the
//                 options struct given as parameter
// PRE          :  See init_pref_audio_codecs
 void init_pref_advanced (GtkWidget *, GM_pref_window_widgets *,
			  int, options *);


// DESCRIPTION  :  /
// BEHAVIOR     :  It builds the notebook page for ILS settings and
//                 add it to the notebook, default values are set from the
//                 options struct given as parameter
// PRE          :  See init_pref_audio_codecs
 void init_pref_ldap (GtkWidget *, GM_pref_window_widgets *,
		      int, options *);


// DESCRIPTION  :  /
// BEHAVIOR     :  It builds the notebook page for GateKeeper settings and
//                 add it to the notebook, default values are set from the
//                 options struct given as parameter
// PRE          :  See init_pref_audio_codecs
 void init_pref_gatekeeper (GtkWidget *, GM_pref_window_widgets *,
			    int, options *);


// DESCRIPTION  :  /
// BEHAVIOR     :  It builds the notebook page for the device settings and
//                 add it to the notebook, default values are set from the
//                 options struct given as parameter
// PRE          :  See init_pref_audio_codecs
 void init_pref_devices (GtkWidget *, GM_pref_window_widgets *,
			 int, options *);

/******************************************************************************/


/******************************************************************************/
/* Miscellaneous functions                                                    */
/******************************************************************************/

// DESCRIPTION  :  /
// BEHAVIOR     :  Apply the options given as parameter to the endpoint
//                 and soundcard and video4linux device and LDAP
// PRE          :  A valid pointer to valid options and a valid pointer
//                 to GM_pref_window_widgets 
 void apply_options (options *opts, GM_pref_window_widgets *);


// DESCRIPTION  :  / 
// BEHAVIOR     :  Create a button with the GtkWidget * as pixmap and the label
//                 as label
// PRE          :  /
 GtkWidget * add_button (char *, GtkWidget *);

// DESCRIPTION  :  / 
// BEHAVIOR     :  Add the codec (second parameter) to the codecs clist (first)
//                 and the right pixmap (Enabled/Disabled) following the third
//                 parameter. Also sets row data (1 for Enabled, O for not).
// PRE          :  First parameter should be a valid clist
 void add_codec (GtkWidget *, gchar *, gchar *);

/******************************************************************************/

#endif
