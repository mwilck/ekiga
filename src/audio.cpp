
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
#include "gnomemeeting.h"

#include <ptlib.h>

#ifdef __FreeBSD__
#include <sys/types.h>
#include <signal.h>
#endif

#ifdef HAS_IXJ
#include <ixjlid.h>
#endif


extern GnomeMeeting *MyApp;

/* The functions */

int gnomemeeting_volume_set (char *mixer, int source, int *volume)
{
  int res, mixerfd;
#ifdef HAS_IXJ
  OpalLineInterfaceDevice *lid = NULL;

  if (!strcmp (mixer, "/dev/phone0")) {

    unsigned vol;
    if ((MyApp)&&(MyApp->Endpoint ()))
	lid = MyApp->Endpoint ()->GetLidDevice ();

    if (source == 0) 
      if (lid)
	lid->SetPlayVolume (0, vol);

    if (source == 0)
      if (lid)
	lid->SetRecordVolume (0, vol);

    *volume = (int) vol;
  }
  else {
#endif
    mixerfd = open(mixer, O_RDWR); 
      
    if (mixerfd == -1)
      return -1;

    if (source == 0)
      res = ioctl (mixerfd, MIXER_WRITE (SOUND_MIXER_VOLUME), volume);

    if (source == 1)
      res = ioctl (mixerfd, MIXER_WRITE (SOUND_MIXER_MIC), volume);

    close (mixerfd);

#ifdef HAS_IXJ
  }
#endif

  return 0;
}


int gnomemeeting_volume_get (char *mixer, int source, int *volume)
{
  int res, mixerfd, caps;

#ifdef HAS_IXJ
  OpalLineInterfaceDevice *lid = NULL;

  if (!strcmp (mixer, "/dev/phone0")) {

    unsigned vol;
    if ((MyApp)&&(MyApp->Endpoint ()))
      lid = MyApp->Endpoint ()->GetLidDevice ();

    if (source == 0) 
      if (lid)
	lid->GetPlayVolume (0, vol);

    if (source == 1)
      if (lid)
	lid->GetRecordVolume (0, vol);

    *volume = (int) vol;
  }
  else {
#endif

    mixerfd = open(mixer, O_RDWR);
      
    if (mixerfd == -1)
      return -1;

    if (source == 0)
      res = ioctl (mixerfd, MIXER_READ (SOUND_MIXER_READ_VOLUME), volume);

    if (source == 1)
      res = ioctl (mixerfd, MIXER_READ (SOUND_MIXER_MIC), volume);

    close (mixerfd);

#ifdef HAS_IXJ
  }
#endif  

  return TRUE;
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


int gnomemeeting_get_mixer_name (char *mixer, char **name)
{
#ifdef __FreeBSD__
  strcpy(name,"/dev/mixer");
#else
  int mixerfd, res;
  mixer_info info;

  mixerfd = open(mixer, O_RDWR);
      
  if (mixerfd == -1)
      return -1;
  
  res = ioctl(mixerfd, SOUND_MIXER_INFO, &info);
  *name = g_strdup (info.name);
  cout << "ici" << info.name << endl << flush;

  close (mixerfd);
#endif
  return 0;
}


PStringArray gnomemeeting_get_mixers ()
{
  
}


int kill_sound_daemons()
{
  char command[100];
  int err;
  FILE *out;
  pid_t pid;
  
  /* try to kill all artsd*/
  snprintf(command,100,"ps -u %s |grep artsd",getenv("LOGNAME"));
  out=popen(command,"r");
  if (out!=NULL)
    {
      do{
	err=fscanf(out,"%i",&pid);
	if (err==1) kill(pid,SIGINT);
      }while(err==1);
      pclose(out);
    }
  /* do the same with esd*/
  snprintf(command,100,"ps -u %s |grep esd",getenv("LOGNAME"));
  out=popen(command,"r");
  if (out!=NULL)
    {
      do{
	err=fscanf(out,"%i",&pid);
	if (err==1) kill(pid,SIGINT);
      }while(err==1);
      pclose(out);
    }
  return(0);
}
