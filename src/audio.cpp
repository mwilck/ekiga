
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

#include "audio.h" 
#include "common.h"


/******************************************************************************/
/* The functions                                                              */
/******************************************************************************/

int GM_volume_set (char *mixer, int source, int *volume)
{
  int res, mixerfd;

  mixerfd = open(mixer, O_RDWR); 
      
  if (mixerfd == -1)
      return -1;

  if (source == 0)
    res = ioctl (mixerfd, MIXER_WRITE (SOUND_MIXER_VOLUME), volume);

  if (source == 1)
    res = ioctl (mixerfd, MIXER_WRITE (SOUND_MIXER_MIC), volume);

  close (mixerfd);

  return 0;
}


int GM_volume_get (char *mixer, int source, int *volume)
{
  int res, mixerfd;
  
  mixerfd = open(mixer, O_RDWR);
      
  if (mixerfd == -1)
      return -1;

  if (source == 0)
    res = ioctl (mixerfd, MIXER_READ (SOUND_MIXER_READ_VOLUME), volume);

  if (source == 1)
    res = ioctl (mixerfd, MIXER_READ (SOUND_MIXER_MIC), volume);


  close (mixerfd);

  return 0;
}


int GM_set_recording_source (char *mixer, int source)
{
  int res, mixerfd;
  int rcsrc = SOUND_MASK_MIC;
  
  mixerfd = open(mixer, O_RDWR);
      
  if (mixerfd == -1)
      return -1;

  if (source == 0)
    ioctl (mixerfd, SOUND_MIXER_WRITE_RECSRC, &rcsrc);

  close (mixerfd);

  return 0;
}

/******************************************************************************/



