
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
 *                         audio.cpp  -  description
 *                         -------------------------
 *   begin                : Tue Mar 8 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains OSS functions.
 *   email                : dsandras@seconix.com
 *
 */


#include "audio.h" 
#include "common.h"


/* The functions */

int gnomemeeting_volume_set (char *mixer, int source, int *volume)
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


int gnomemeeting_volume_get (char *mixer, int source, int *volume)
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


int gnomemeeting_set_recording_source (char *mixer, int source)
{
  int res, mixerfd;
  int rcsrc;
  
  mixerfd = open(mixer, O_RDWR);
      
  if (mixerfd == -1)
      return -1;

  if (ioctl (mixerfd, SOUND_MIXER_READ_RECSRC, &rcsrc))
    rcsrc = 0;

  if (source == 0) {
    rcsrc |= SOUND_MASK_MIC;
    ioctl (mixerfd, SOUND_MIXER_WRITE_RECSRC, &rcsrc);
  }

  close (mixerfd);

  return 0;
}


int gnomemeeting_get_mixer_name (char *mixer, char *name)
{
  int mixerfd, res;
  mixer_info info;

  mixerfd = open(mixer, O_RDWR);
      
  if (mixerfd == -1)
      return -1;
  
  res = ioctl(mixerfd, SOUND_MIXER_INFO, &info);
  strcpy (name, info.name);

  close (mixerfd);

  return 0;
}



