
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
 *                         call.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras 
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : declaration of the interface of a call handled by 
 *                          the Ekiga::CallManager.
 *
 */

#ifndef __CALL_H__
#define __CALL_H__

namespace Ekiga {

  /**
   * Everything is handled asynchronously and signaled through the
   * Ekiga::CallManager
   */
  class Call
    {

  public:

      /* The destructor
       */
      virtual ~Call () {}


      /* 
       * Call Management
       */

      /** Hangup the call
       */
      void hangup (); 

      /** Transfer the call to the specified uri
       * @param: uri: where to transfer the call
       */
      void transfer (std::string uri);

      /** Put the call on hold
       */
      void hold ();

      /** Retrieve the call if it is on hold
       */
      void retrieve ();

      /** Pause the audio stream (if any)
       */
      void pause_audio ();

      /** Pause the video stream (if any)
       */
      void pause_video ();


      /* 
       * Call Information
       */

      /** Return the remote party name
       * @return: the remote party name
       */
      std::string get_remote_party_name ();

      /** Return the remote application
       * @return: the remote application
       */
      std::string get_remote_application ();

      /** Return the remote callback uri
       * @return: the remote uri
       */
      std::string get_remote_uri ();

      /** Return a string describing the reason why the call was ended
       * @return: a string describing the reason of the end of the call
       *          provided the call is effectively over.
       */
      std::string get_call_end_reason ();

      /** Return the call duration
       * @return: the current call duration
       */
      std::string get_call_duration ();
    };
};

#endif
