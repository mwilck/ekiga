
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

#include "endpoint.h"
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


#define GM_AUDIO_TESTER(x) (GMAudioTester *)(x)

enum { SOURCE_AUDIO, SOURCE_MIC };


/* The functions */

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

void gnomemeeting_mixers_mic_select (void);
PStringArray gnomemeeting_get_mixers (void);
PStringArray gnomemeeting_get_audio_player_devices (void);
PStringArray gnomemeeting_get_audio_recorder_devices (void);
int gnomemeeting_get_mixer_volume (char *mixer, int source);
void gnomemeeting_set_mixer_volume (char *mixer, int source, int vol);
     
class GMAudioTester : public PThread
{
  PCLASSINFO(GMAudioTester, PThread);


public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  
   * PRE          :  
   */
  GMAudioTester (GMH323EndPoint *);


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMAudioTester ();


  void Main ();


  void Stop ();


protected:

  BOOL stop;
  PMutex quit_mutex;
  PSoundChannel *player;
  PSoundChannel *recorder;

  GmWindow *gw;
  GtkWidget *dialog;
  GtkWidget *label;

  gchar *msg;

  GMH323EndPoint *ep;
};
#endif
