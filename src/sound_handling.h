
/*  sound_handling.cpp
 *
 *  GnomeMeeting -- A Video-Conferencing application
 *  Copyright (C) 2000-2002 Damien Sandras
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
/*
 *                         sound_handling.h  -  description
 *                         --------------------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains sound handling functions.
 *   email                : dsandras@seconix.com
 *
 */

#ifndef __GM_SOUND_HANDLING_H
#define __GM_SOUND_HANDLING_H

#include "common.h"
#include <gtk/gtk.h>
#include <esd.h>

#ifdef __linux__
#include <linux/soundcard.h>
#endif
#ifdef __FreeBSD__
#if (__FreeBSD__ >= 5)
#include <sys/soundcard.h>
#else
#include <machine/soundcard.h>
#endif
#endif

#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <ptlib.h>

enum { SOURCE_AUDIO, SOURCE_MIC };

/* The functions */

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Set the given source of the given mixer to the given volume
 * PRE          :  First param = mixer, Second = source (SOURCE_AUDIO or
 *                 SOURCE_MIC), Third, the volume
 */
int gnomemeeting_volume_set (char *, int, int *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Get the volume for the given source of the given mixer, returns
 *                 -1 if error.
 * PRE          :  First param = mixer, Second = source (SOURCE_AUDIO or
 *                 SOURCE_MIC), Third, the volume
 */
int gnomemeeting_volume_get (char *, int, int *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Set the recording source
 * PRE          :  First param = mixer, Second = source (SOURCE_AUDIO or
 *                 SOURCE_MIC)
 */
int gnomemeeting_set_recording_source (char *, int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Get the device name
 * PRE          :  First param = mixer, Second = char ** to contain the name
 */
int gnomemeeting_get_mixer_name (char *, char **);

/* DESCRIPTION   :  /
 * BEHAVIOR      : Puts ESD (and Artsd if support compiled in) into standby 
 *                 mode. An error message is displayed in the gnomemeeting
 *                 history if it failed. No message is displayed if it is
 *                 succesful.
 * PRE           : /
 */
void gnomemeeting_sound_daemons_suspend ();


/* DESCRIPTION   :  /
 * BEHAVIOR      : Puts ESD (and Artsd if support compiled in) into normal
 *                 mode. An error message is displayed in the gnomemeeting
 *                 history if it failed. No message is displayed if it is
 *                 succesful.
 * PRE           : /
 */
void gnomemeeting_sound_daemons_resume ();


/* DESCRIPTION  :  This callback is called by a timer function.
 * BEHAVIOR     :  Plays the sound choosen in the gnome control center.
 * PRE          :  The pointer to the docklet must be valid.
 */
int gnomemeeting_sound_play_ringtone (GtkWidget *widget);

#endif
