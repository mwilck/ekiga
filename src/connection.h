/***************************************************************************
                          connection.h  -  description
                             -------------------
    begin                : Sat Dec 23 2000
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : Connection functions
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

#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <ptlib.h>
#include <h323.h>
#include <string.h>
#include "h323pdu.h"

#include "endpoint.h"
#include "common.h"


/******************************************************************************/
/*   GMH323Connection : manages the connections                               */
/******************************************************************************/

class GMH323Connection : public H323Connection
{
  PCLASSINFO(GMH323Connection, H323Connection);

  
  public:

    // DESCRIPTION  :  The constructor
    // BEHAVIOR     :  Setups the connection parameters
    // PRE          :  GM_window_widgets is a valid pointer to a valid
    //                 struct containing all the widgets needed to manage
    //                 and update the main GUI, GMH323EndPoint the endpoint
    //                 initiating the connection, unsigned the call reference
    //                 and valid options
    GMH323Connection (GMH323EndPoint &, unsigned, 
		      GM_window_widgets *, options *);


    // DESCRIPTION  :  Callback called when OpenH323 opens a new logical channel
    // BEHAVIOR     :  Updates the log window with information about it, returns
    //                 FALSE if error, TRUE if OK
    // PRE          :  
    virtual BOOL OnStartLogicalChannel (H323Channel &);


    // DESCRIPTION  :  Callback called when OpenH323 has closed a logical channel
    // BEHAVIOR     :  Calls the corresponding OpenH323 callback
    // PRE          :  /
    virtual void OnClosedLogicalChannel (H323Channel &);


    // DESCRIPTION  :  Callback called when OpenH323 closes a new logical channel
    // BEHAVIOR     :  returns TRUE
    // PRE          :  /
    virtual BOOL OnClosingLogicalChannel (H323Channel &);


    // DESCRIPTION  :  /
    // BEHAVIOR     :  if int = 0, pauses or unpauses the transmitted
    //                 audio channel (if any)
    //                 if int = 1, pauses or unpauses the transmitted
    //                 video channel (if any)
    // PRE          :  /
    void PauseChannel (int);


    // DESCRIPTION  :  /
    // BEHAVIOR     :  Unpause video and audio channels
    // PRE          :  /
    void UnPauseChannels (void);


    // DESCRIPTION  :  This callback is called to give the opportunity
    //                 to take an action on an incoming call
    // BEHAVIOR     :  Behavior is the following :
    //                 - returns AnswerCallDenied if user is in DND mode
    //                   connection aborted and a Release Complete PDU is sent
    //                 - returns AnswerCallNow if user is in Auto Answer mode
    //                   H323 protocol proceeds and the call is answered
    //                 - return AnswerCallPending (default)
    //                   pause until AnsweringCall is called (if the user
    //                   clicks on connect or disconnect)
    // PRE          :  /
    virtual H323Connection::AnswerCallResponse OnAnswerCall (const PString &,
							     const H323SignalPDU &,
							     H323SignalPDU &);

    
  protected:
    GM_window_widgets *gw;          // GM_window_widgets
    H323Channel *transmitted_audio; // transmitted audio channel
    H323Channel *transmitted_video; // transmitted video channel
    options *opts;                  // current options
    int opened_channels;            // opened channels number

};

/******************************************************************************/

#endif
