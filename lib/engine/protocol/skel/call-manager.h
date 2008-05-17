
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

namespace Ekiga
{

/**
 * @addtogroup calls
 * @{
 */

  class CallManager
    {

  public:

      class Interface
        {
      public:
          std::string voip_protocol;
          std::string protocol;
          std::string interface;
          bool publish;
          unsigned port;
        };
      typedef std::list<CallManager::Interface> InterfaceList;


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
      virtual bool dial (const std::string & uri) = 0; 


      /*
       * PROTOCOL INFORMATION
       */

      /**
       * @return the protocol name
       */
      virtual const std::list<std::string> & get_protocol_names () const = 0;

      /**
       * @return the interface on which we are accepting calls. Generally,
       * under the form protocol:IP:port.
       */
      virtual const CallManager::InterfaceList get_interfaces () const = 0;


      /*
       * Misc
       */

      /** Enable the given codecs
       * @param codecs is a set of the codecs and their descriptions
       *        when the function returns, the list also contains disabled
       *        codecs supported by the CallManager. Unsupported codecs 
       *        have been removed.
       */
      virtual void set_codecs (CodecList & codecs) = 0; 

      /** Return the list of available codecs
       * @return a set of the codecs and their descriptions
       */
      virtual const Ekiga::CodecList & get_codecs () const = 0;

      /** Set the display name used on outgoing calls
       * @param name is the display name to use.
       */
      virtual void set_display_name (const std::string & name) = 0;

      /** Return the display name used on outgoing calls
       */
      virtual const std::string & get_display_name () const = 0;

      /** Enable echo cancellation
       * @param enabled is true if echo cancellation should be enabled, false
       * otherwise.
       */
      virtual void set_echo_cancellation (bool enabled) = 0;

      /** Get echo cancellation setting
       * @return true if echo cancellation is enabled.
       */
      virtual bool get_echo_cancellation () const = 0;

      /** Enable silence detection
       * @param enabled is true if silence detection should be enabled, false
       * otherwise.
       */
      virtual void set_silence_detection (bool enabled) = 0;

      /** Get silence detection setting
       * @return true if silence detection is enabled.
       */
      virtual bool get_silence_detection () const = 0;

      /** Set maximum jitter 
       * @param max_val is the maximum jitter for calls in seconds.
       */
      virtual void set_maximum_jitter (unsigned max_val) = 0;

      /** Get maximum jitter 
       * @return the maximum jitter for calls in seconds.
       */
      virtual unsigned get_maximum_jitter () const = 0;

      /** Set delay before dropping an incoming call 
       * @param delay is the delay after which the call should be rejected
       * (or forwarded if supported by the CallManager).
       */
      virtual void set_reject_delay (unsigned delay) = 0;

      /** Get delay before dropping an incoming call
       * @return the delay in seconds after which a call should be rejected
       * (or forwarded if supported by the CallManager).
       */
      virtual unsigned get_reject_delay () const = 0;


      /*
       * INSTANT MESSAGING 
       */

      /** Send a message to the given uri
       * @param: uri    : where to send the message
       *         message: what to send to the remote peer
       */
      //virtual bool send_message (const std::string uri, 
                                 //const std::string message) = 0;

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
       * ACCOUNT INDICATIONS
       */

      /** This signal is emitted when there is a new message waiting indication
       * @param: account is the voicemail account
       *         mwi is the message waiting indication
       */
      sigc::signal<void, std::string, std::string> mwi_event;

      /** This signal is emitted when there is a new registration event
       * @param: account is the voicemail account
       *         state is the state
       *         info contains information about the registration status
       */
      sigc::signal<void, std::string, Ekiga::CallCore::RegistrationState, std::string> registration_event;
    };

/**
 * @}
 */

};

#endif
