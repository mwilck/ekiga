
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2003 Damien Sandras
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
 *                         sound_handling.h  -  description
 *                         --------------------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains sound handling functions.
 *
 */


#ifndef __GM_SOUND_HANDLING_H
#define __GM_SOUND_HANDLING_H

#include "common.h"
#include "endpoint.h"

#ifndef WIN32
#include <esd.h>
#endif


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

  GMH323EndPoint *ep;
};
#endif
