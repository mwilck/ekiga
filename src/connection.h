
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
 *                         connection.h  -  description
 *                         ----------------------------
 *   begin                : Sat Dec 23 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains connection related functions.
 *   email                : dsandras@seconix.com
 *
 */


#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <ptlib.h>
#include <h323.h>
#include <string.h>
#include "h323pdu.h"

#include "endpoint.h"
#include "common.h"


/* GMH323Connection */

class GMH323Connection : public H323Connection
{
  PCLASSINFO(GMH323Connection, H323Connection);

  
  public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Setups the connection parameters.
   * PRE          :  /
   */
  GMH323Connection (GMH323EndPoint &, unsigned);


  /* DESCRIPTION  :  Callback called when OpenH323 opens a new logical channel
   * BEHAVIOR     :  Updates the log window with information about it, returns
   *                 FALSE if error, TRUE if OK
   * PRE          :  /
   */
  virtual BOOL OnStartLogicalChannel (H323Channel &);


  /* DESCRIPTION  :  Callback called when OpenH323 closes a new logical channel
   * BEHAVIOR     :  Close the channel.
   * PRE          :  /
   */
  virtual void OnClosedLogicalChannel (H323Channel &);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  if int = 0, pauses or unpauses the transmitted
   *                 audio channel (if any)
   *                 if int = 1, pauses or unpauses the transmitted
   *                 video channel (if any)
   * PRE          :  /
   */
  void PauseChannel (int);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Unpause video and audio channels
   * PRE          :  /
   */
  void UnPauseChannels (void);


  /* DESCRIPTION  :  This callback is called to give the opportunity
   *                 to take an action on an incoming call
   * BEHAVIOR     :  Behavior is the following :
   *                 - returns AnswerCallDenied if user is in DND mode
   *                   connection aborted and a Release Complete PDU is sent
   *                 - returns AnswerCallNow if user is in Auto Answer mode
   *                   H323 protocol proceeds and the call is answered
   *                 - return AnswerCallPending (default)
   *                   pause until AnsweringCall is called (if the user
   *                   clicks on connect or disconnect)
   * PRE          :  /
   */
  virtual H323Connection::AnswerCallResponse 
    OnAnswerCall (const PString &, const H323SignalPDU &, H323SignalPDU &);

  
  protected:
    GM_window_widgets *gw;
    H323Channel *transmitted_audio; 
    H323Channel *transmitted_video; 
    int opened_channels; 
};


#endif
