
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
 *                         pcssendpoint.h  -  description
 *                         ------------------------------
 *   begin                : Sun Oct 24 2004
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains the PCSS EndPoint class.
 *
 */


#ifndef _PCSS_ENDPOINT_H_
#define _PCSS_ENDPOINT_H_

#include "common.h"
#include "endpoint.h"


class GMPCSSEndPoint : public OpalPCSSEndPoint
{
  PCLASSINFO (GMPCSSEndPoint, OpalPCSSEndPoint);

public:
  GMPCSSEndPoint (GMEndPoint &);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Accept the current incoming call.
   * PRE          :  /
   */
  void AcceptCurrentIncomingCall ();
  

  /* DESCRIPTION  :  This callback is called when there is an 
   * 		     incoming PCSS connection. This only happens
   * 		     when the SIP/H.323 connection is not rejected
   * 		     or forwarded.
   * BEHAVIOR     :  Auto-Answer the call or not. Call the OnIncomingConnection
   * 		     function of the GMEndPoint, and triggers the appropriate
   * 		     timeouts (no answer, ringing). Display a popup if
   * 		     required.
   * PRE          :  /
   */
  virtual void OnShowIncoming (const OpalPCSSConnection &);

  virtual BOOL OnShowOutgoing (const OpalPCSSConnection &);
  virtual PString OnGetDestination (const OpalPCSSConnection &);  

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Plays a sound event on its device.
   * PRE          :  /
   */
  void PlaySoundEvent (PString);
  
  
  /* DESCRIPTION  :  This callback is called when an audio channel has to
   *                 be opened.
   * BEHAVIOR     :  Open the Audio Channel or warn the user if it was
   *                 impossible.
   * PRE          :  /
   */
  PSoundChannel *CreateSoundChannel (const OpalPCSSConnection &, 
				     BOOL);

  
  /* DESCRIPTION  :  This callback is called when the connection is 
   *                 established and everything is ok.
   * BEHAVIOR     :  Stops the timers.
   * PRE          :  /
   */
  void OnEstablished (OpalConnection &);

  
  /* DESCRIPTION  :  This callback is called when a connection to a remote
   *                 endpoint is cleared.
   * BEHAVIOR     :  Stops the timers.
   * PRE          :  /
   */
  void OnReleased (OpalConnection &);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the playing and recording volume levels.
   * PRE          :  /
   */
  void GetDeviceVolume (unsigned int &,
			unsigned int &);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Updates the playing and recording volume levels.
   * PRE          :  /
   */
  void SetDeviceVolume (unsigned int,
			unsigned int);

private:
  
  /* DESCRIPTION  :  Notifier called when an incoming call
   *                 has not been answered after 15 seconds.
   * BEHAVIOR     :  Reject the call, or forward if forward on no answer is
   *                 enabled in the config database.
   * PRE          :  /
   */
  PDECLARE_NOTIFIER(PTimer, GMPCSSEndPoint, OnNoAnswerTimeout);


  /* DESCRIPTION  :  Notifier called every second while waiting for an answer
   *                 for an incoming call.
   * BEHAVIOR     :  Display an animation in the docklet and play a ring
   *                 sound.
   * PRE          :  /
   */
  PDECLARE_NOTIFIER(PTimer, GMPCSSEndPoint, OnCallPending);
  
  PTimer NoAnswerTimer;
  PTimer CallPendingTimer;

  
  GMEndPoint & endpoint;
  PString incomingConnectionToken; 

  PMutex sound_event_mutex;
};

#endif
