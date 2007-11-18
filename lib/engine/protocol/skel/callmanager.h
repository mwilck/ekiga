
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras

 * This program is free software; you can  redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ekiga is licensed under the GPL license and as a special exception, you
 * have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OPAL, OpenH323 and PWLIB
 * programs, as long as you do follow the requirements of the GNU GPL for all
 * the rest of the software thus combined.
 */


/*
 *                         callmanager.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras 
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : declaration of the interface of a call manager
 *                          implementation backend. A call manager handle calls,
 *                          sometimes simultaneously.
 *
 */

#ifndef __CALL_MANAGER_H__
#define __CALL_MANAGER_H__

namespace Ekiga {

  class CallManager
    {

  public:

      /* The destructor
       */
      virtual ~CallManager () {}


      /*                 
       * CALL MANAGEMENT 
       */               

      /** Create a call based on the remote uri given as parameter.
       * @return A reference to he created Ekiga::Call is returned by the method.
       */
      Ekiga::Call create_call (std::string uri); 

      /** This signal is emitted when the call status changes.
       * There are different statuses described in the Ekiga::Call interface, but
       * we can summarize them :
       *   - Outgoing     : an outgoing call is being initiated
       *   - Incoming     : an incoming call is being initiated
       *   - Established  : the call has been established
       *   - Cleared      : the call is terminated
       *   - Missed       : the call is terminated without having been established
       *   - Forwarded    : the call is terminated because it has been forwarded
       *   - Held         : the call is on hold
       * @param: A reference to the Ekiga::Call on which the event occurs.
       */
      sigc::signal<void, Ekiga::Call &> call_event;


      /*
       * INSTANT MESSAGING 
       */

      /** Send a message to the given uri
       * @param: uri    : where to send the message
       *         message: what to send to the remote peer
       */
      void send_message (std::string uri, 
                         std::string message);

      /** This signal is emitted when the transmission of a message failed
       * @param: uri    : where the message could not be sent
       *         error  : a string describing the error that occured
       */
      sigc::signal<void, std::string uri, std::string error> im_failed;

      /** This signal is emitted when a message has been received
       * @param: display_name: the display name of the sender
       *         uri         : the uri of the sender
       *         message     : the message sent by the sender
       */
      sigc::signal<void, std::string display_name, std::string uri, std::string message> im_received;

      /** This signal is emitted when a message has been sent
       * @param: uri    : where the message has been sent
       *         message: the message that was sent
       */
      sigc::signal<void, std::string uri, std::string message> im_sent;

      /** This signal is emitted when a chat conversation should be initiated
       * @param: uri            : the remote party
       *         display_name   : the display name
       */
      sigc::signal<void, std::string uri, std::string display_name> new_chat;


      /*          
       * Accounts
       */        

      /** Register the provided account.
       * @param: account : the account to register.
       */
      void register_account (Ekiga::Account & account);

      /** Unregister the provided account.
       * @param: account : the account to register.
       */
      void register_account (Ekiga::Account & account);

      /** This signal is emitted when a registration status changes.
       * @param: aor            : the registered account
       *         state          : the current registration state
       *         error          : the associated error in case of failure 
       */
      sigc::signal<void, std::string aor, CallManager::RegistrationState state, std::string error> registration_event;


      /*
       * MISC
       */ 

      /** This signal is emitted when a message waiting indication is received.
       * @param: aor            : the account
       *         mwi_string     : the received indication as a string
       *         total          : the total number of unread messages for all accounts
       */
      sigc::signal<void, std::string aor, std::string mwi_string, unsigned int total> message_waiting_event;

      /** This signal is emitted when a media stream is open or closed.
       * @param: codec          : the codec name
       *         video          : true if it is a video codec
       *         transmitting   : true if the media stream is open for transmission
       *         closing        : true of the media stream is closed
       */
      sigc::signal<void, std::string codec, bool video, bool transmitting, bool closing> media_stream_event;

      /** This signal is emitted regularly to give the level of the input/output signals for the currently active call.
       * @param: input          : the input level
       *         output         : the output level 
       */
      sigc::signal<void, float, float> audio_signal_event;

      /** This signal is emitted regularly to give statistics about the currently active call.
       * @param: stats          : the statistics
       */
      sigc::signal<void, Ekiga::CallStatistics & stats> stats_event;
    };
};

#endif
