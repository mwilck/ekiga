/***************************************************************************
                          audio.cpp  -  description
                             -------------------
    begin                : Tue Mar 8 2001
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : This file contains all functions needed for
                           OSS
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

#ifndef _AUDIO_H_
#define _AUDIO_H_


#include <linux/soundcard.h>


#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

/******************************************************************************/
/* The functions                                                              */
/******************************************************************************/

// DESCRIPTION  :  /
// BEHAVIOR     :  Sets the given source of the given mixer to the given volume
// PRE          :  First param = mixer, Second = source (0 : audio,
//                 1 :mic), Third, the volume
int GM_volume_set (char *, int, int *);


// DESCRIPTION  :  /
// BEHAVIOR     :  Gets the volume for the given source of the given mixer
// PRE          :  First param = mixer, Second = source (0 : audio,
//                 1 :mic), Third, the volume
int GM_volume_get (char *, int, int *);


// DESCRIPTION  :  /
// BEHAVIOR     :  Sets the recording source
// PRE          :  First param = mixer, Second = source (0 : audio,
//                 1 :mic)
int GM_set_recording_source (char *, int);

/******************************************************************************/

#endif
