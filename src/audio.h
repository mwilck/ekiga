
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
 *                         audio.h  -  description
 *                         -----------------------
 *   begin                : Tue Mar 8 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains OSS functions.
 *   email                : dsandras@seconix.com
 *
 */

#ifndef _AUDIO_H_
#define _AUDIO_H_


#ifdef __linux__
#include <linux/soundcard.h>
#endif
#ifdef __FreeBSD__
#include <machine/soundcard.h>
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

PStringArray gnomemeeting_get_mixers ();

int kill_sound_daemons ();
#endif
