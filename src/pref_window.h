
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
 *                         pref_window.h  -  description
 *                         -----------------------------
 *   begin                : Tue Dec 26 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          create the preferences window and all its callbacks
 *   Additional code      : Miguel Rodríguez Pérez  <migrax@terra.es> 
 */


#ifndef _PREFERENCES_H_
#define _PREFERENCES_H_

#include "common.h"


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Refreshes the devices list in the GUI to update them from
 *                 the GnomeMeeting available devices list.
 * PRE          :  The main window GMObject, the audio input devices list,
 * 		   the audio output devices list, the video input devices list.
 */
void 
gm_prefs_window_update_devices_list (GtkWidget *,
				     PStringArray,
				     PStringArray,
				     PStringArray);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Refreshes the codecs list in the GUI to update them from
 *                 the detected codecs following the user configuration.
 * PRE          :  /
 */
void
gm_prefs_window_update_audio_codecs_list (GtkWidget *,
					  OpalMediaFormatList);


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Builds the sound events list of the preferences window. 
 * PRE          :  /
 */
void
gm_prefs_window_sound_events_list_build (GtkWidget *); 


/* DESCRIPTION  :  /
 * BEHAVIOR     :  It builds the preferences window
 *                 (sections' ctree / Notebook pages) and connect GTK signals
 *                 to appropriate callbacks, then returns it.
 * PRE          :  /
 */
GtkWidget *
gm_prefs_window_new ();



#endif
     
