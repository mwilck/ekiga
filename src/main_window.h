
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
 *                         main_window.h  -  description
 *                         -----------------------------
 *   begin                : Mon Mar 26 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          build the main window.
 */


#ifndef _MAIN_INTERFACE_H
#define _MAIN_INTERFACE_H

#include "common.h"


void gm_main_window_dialpad_event (GtkWidget *,
				   const char);

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Displays the gnomemeeting logo in the video window.
 * PRE          :  The main window GMObject.
 */
void gm_main_window_update_logo (GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the main window hold call menu and toolbar items
 * 		   following the call is on hold (TRUE) or not (FALSE).
 * PRE          :  The main window GMObject.
 */
void gm_main_window_set_call_hold (GtkWidget *,
				   gboolean);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the main window pause channel menu and toolbar items
 * 		   following the channel is paused (TRUE) or not (FALSE). The
 * 		   last argument is true if we are modifying a video channel
 * 		   item.
 * PRE          :  The main window GMObject.
 */
void gm_main_window_set_channel_pause (GtkWidget *,
				       gboolean,
				       gboolean);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the main window sensitivity following the calling
 *                 state.
 * PRE          :  The main window GMObject.
 * 		   A valid GMH323EndPoint calling state.
 */
void gm_main_window_update_sensitivity (//GtkWidget *,
					unsigned);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the main window sensitivity following the opened
 *                 and closed audio and video channels.
 * PRE          :  The main window GMObject.
 * 		   The first parameter is TRUE if we are updating video
 *                 channels related items, FALSE if we are updating audio
 *                 channels related items. The second parameter is TRUE
 *                 if we are transmitting audio (or video), the third is TRUE
 *                 if we are receiving audio (or video).
 */
void gm_main_window_update_sensitivity (//GtkWidget *,
					BOOL,
					BOOL,
					BOOL);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the main window video sliders to the given values,
 * 		   notice it only updates the GUI.
 * PRE          :  A valid pointer to the main window GMObject, followed
 * 		   by the whiteness, brightness, colourness and contrast.
 * 		   Their values must be comprised between -1 (no change) and 
 * 		   255.
 */
void gm_main_window_set_video_sliders_values (GtkWidget *,
					      int,
					      int,
					      int, 
					      int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Gets the values for the main window video sliders and
 * 		   updates the parameters accordingly.
 * 		   Notice it only reads the values from the GUI, not from
 * 		   the video grabber.
 * PRE          :  A valid pointer to the main window GMObject, followed
 * 		   by the whiteness, brightness, colourness and contrast.
 * 		   Their values will be comprised between 0 and 255 when
 * 		   the function returns.
 */
void gm_main_window_get_video_sliders_values (GtkWidget *,
					      int &,
					      int &,
					      int &, 
					      int &);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the main window audio sliders to the given values,
 * 		   notice it only updates the GUI.
 * PRE          :  A valid pointer to the main window GMObject, followed
 * 		   by the output and input volumes.
 * 		   Their values must be comprised between -1 (no change) and 
 * 		   255.
 */
void gm_main_window_set_volume_sliders_values (GtkWidget *,
					       int, 
					       int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Gets the values of the main window audio sliders.
 * PRE          :  A valid pointer to the main window GMObject, followed
 * 		   by the output and input volumes.
 * 		   Their values will be comprised between 0 and 255 when 
 * 		   the function returns.
 */
void gm_main_window_get_volume_sliders_values (GtkWidget *,
					       int &, 
					       int &);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Select the correct control panel section in the menus
 * 		   and in the main window.
 * PRE          :  The main window GMObject and a valid section.
 */
void gm_main_window_select_control_panel_section (GtkWidget *,
						  int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the speed dials menu in the call menu given the
 *                 main window GMObject and using the address book.
 * PRE          :  The main window GMObject and the GSList of GmContacts.
 */
void gm_main_window_speed_dials_menu_update (GtkWidget *,
					     GSList *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Runs a dialog to transfer a call. The default transfer
 * 		   destination is the specified forward host if any, if not
 * 		   it will be the forward host from the configuration, if any.
 * PRE          :  The main window GMObject.
 */
void gm_main_window_transfer_dialog_run (GtkWidget *,
					 gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the main window and adds the popup to the image.
 * PRE          :  Accels.
 **/
GtkWidget *gm_main_window_new (GmWindow *);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Flashes a message on the statusbar during a few seconds.
 *                 Removes the previous message.
 * PRE           : The main window GMObject, followed by printf syntax format.
 */
void 
gm_main_window_flash_message (GtkWidget *, 
			      const char *, 
			      ...);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Displays a message on the statusbar or clears it if msg = 0.
 *                 Removes the previous message.
 * PRE           : The main window GMObject, followed by printf syntax format.
 */
void 
gm_main_window_push_message (GtkWidget *, 
			     const char *, 
			     ...);



#endif

