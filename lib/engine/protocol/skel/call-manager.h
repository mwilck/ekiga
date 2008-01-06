
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
 *                         call-manager.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras 
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : Declaration of the interface of a call manager
 *                          implementation backend. A call manager handles calls,
 *                          sometimes simultaneously.
 *
 */


#ifndef __CALL_MANAGER_H__
#define __CALL_MANAGER_H__

#include "call-core.h"
#include "codec-description.h"

namespace Ekiga {

  class CallManager
    {

  public:

      /* The constructor
       */
      CallManager () {};

      /* The destructor
       */
      ~CallManager () {}


      /*                 
       * CALL MANAGEMENT 
       */               

      /** Create a call based on the remote uri given as parameter
       * @param: An uri
       * @return: true if a Ekiga::Call could be created
       */
      virtual bool dial (const std::string uri) = 0; 

      /** This signal is emitted when a new call has been created,
       * it does not mean the call is setup, just that an Ekiga::Call object
       * has been created by the Ekiga::CallManager
       * @param the created call 
       */
      sigc::signal<void, Ekiga::Call *> new_call; 


      /*
       * Mandatory Settings
       */

      /** Return the list of available codecs
       * @return a set of the codecs and their descriptions
       */
      virtual CodecList get_codecs () = 0;

      /** Enable the given codecs
       * @param codecs is a set of the codecs and their descriptions
       */
      virtual void set_codecs (CodecList codecs) = 0; 


      /*
       * INSTANT MESSAGING 
       */

      /** Send a message to the given uri
       * @param: uri    : where to send the message
       *         message: what to send to the remote peer
       */
      virtual bool send_message (const std::string uri, 
                                 const std::string message) = 0;

      /** This signal is emitted when the transmission of a message failed
       * @param: uri    : where the message could not be sent
       *         error  : a string describing the error that occured
       */
      sigc::signal<void, std::string, std::string> im_failed;

      /** This signal is emitted when a message has been received
       * @param: display_name: the display name of the sender
       *         uri         : the uri of the sender
       *         message     : the message sent by the sender
       */
      sigc::signal<void, std::string, std::string, std::string> im_received;

      /** This signal is emitted when a message has been sent
       * @param: uri    : where the message has been sent
       *         message: the message that was sent
       */
      sigc::signal<void, std::string, std::string> im_sent;

      /** This signal is emitted when a chat conversation should be initiated
       * @param: uri            : the remote party
       *         display_name   : the display name
       */
      sigc::signal<void, std::string, std::string> new_chat;


      /*          
       * Accounts
       */        
      typedef enum { Processing, Registered, Unregistered, RegistrationFailed, UnregistrationFailed } RegistrationState;


      /** Register the provided account.
       * @param: account : the account to register.
       */
//      void register_account (Ekiga::Account & account);

      /** Unregister the provided account.
       * @param: account : the account to register.
       */
//      void register_account (Ekiga::Account & account);

      /** This signal is emitted when a registration status changes.
       * @param: aor            : the registered account
       *         state          : the current registration state
       *         error          : the associated error in case of failure 
       */
//      sigc::signal<void, std::string, RegistrationState, std::string> registration_event;


      /*
       * MISC
       */ 


  private :
      std::string uri_type;
    };
};

#endif
