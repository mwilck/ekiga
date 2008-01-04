
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

#include <sigc++/sigc++.h>


namespace Ekiga {

  /**
   * Everything is handled asynchronously and signaled through the
   * Ekiga::CallManager
   */
  class Call
    {

  public:

      Call () 
        { 
          outgoing = false; 
          re_a_bw = tr_a_bw = re_v_bw = tr_v_bw = 0.0;
          jitter = 0;
          lost_packets = late_packets = out_of_order_packets = 0.0;
        }

      virtual ~Call () {};

      enum StreamType { Audio, Video };

      /* 
       * Call Management
       */

      /** Hangup the call
       */
      virtual void hangup () = 0; 

      /** Answer an incoming call
       */
      virtual void answer () = 0; 

      /** Transfer the call to the specified uri
       * @param: uri: where to transfer the call
       */
      virtual void transfer (std::string /*uri*/) = 0;

      /** Put the call on hold or retrieve it
       */
      virtual void toggle_hold () = 0;

      /** Toggle the stream transmission (if any)
       * @param the stream type
       */
      virtual void toggle_stream_pause (StreamType type) = 0;

      /** Send the given DTMF
       * @param the dtmf (one char)
       */
      virtual void send_dtmf (const char dtmf) = 0;

      /* 
       * Call Information
       */

      /** Return the call id
       * @return: the call id 
       */
      virtual std::string get_id () = 0;

      /** Return the remote party name
       * @return: the remote party name
       */
      virtual std::string get_remote_party_name () = 0;

      /** Return the remote application
       * @return: the remote application
       */
      virtual std::string get_remote_application () = 0; 

      /** Return the remote callback uri
       * @return: the remote uri
       */
      virtual std::string get_remote_uri () = 0;

      /** Return the call duration
       * @return the current call duration
       */
      virtual std::string get_call_duration () = 0;

      /** Return information about call type
       * @return true if it is an outgoing call
       */
      bool is_outgoing () { return outgoing; }

      /** Return the received audio bandwidth
       * @return the received audio bandwidth in kbytes/s
       */
      double get_received_audio_bandwidth () { return re_a_bw; }

      /** Return the transmitted audio bandwidth
       * @return the transmitted audio bandwidth in kbytes/s
       */
      double get_transmitted_audio_bandwidth () { return tr_a_bw; }
      
      /** Return the received video bandwidth
       * @return the received video bandwidth in kbytes/s
       */ 
      double get_received_video_bandwidth () { return re_v_bw; }
      
      /** Return the transmitted video bandwidth
       * @return the transmitted video bandwidth in kbytes/s
       */
      double get_transmitted_video_bandwidth () { return tr_v_bw; }

      /** Return the transmitted video framerate
       * @return the transmitted video framerate  in frames/s
       */
      unsigned get_transmitted_video_framerate () { return tr_v_fps; }

      /** Return the received video framerate
       * @return the received video framerate in frames/s
       */
      unsigned get_received_video_framerate () { return re_v_fps; }

      /** Return the transmitted video resolution
       * @param: width: width in pixels
       * @param: height: height in pixels
       */
      void get_transmitted_video_resolution (unsigned & width, unsigned & height) { width = tr_width; height = tr_height; }

      /** Return the received video resolution
       * @param: width: width in pixels
       * @param: height: height in pixels
       */
      void get_received_video_resolution (unsigned & width, unsigned & height) { width = re_width; height = re_height; }

      /** Return the jitter size
       * @return the jitter size in ms
       */
      unsigned get_jitter_size () { return jitter; }

      /** Return the lost packets information 
       * @return the lost packets percentage
       */
      double get_lost_packets () { return lost_packets; }
      
      /** Return the late packets information 
       * @return the late packets percentage
       */
      double get_late_packets () { return late_packets; }

      /** Return the out of order packets information 
       * @return the out of order packets percentage
       */
      double get_out_of_order_packets () { return out_of_order_packets; }



      /*
       * Signals
       */
      
      /* Signal emitted when the call is established
       */
      sigc::signal<void> established;

      /* Signal emitted when an established call is cleared
       * @param: a string describing why the call was cleared
       */
      sigc::signal<void, std::string> cleared;

      /* Signal emitted when the call is missed, ie cleared
       * without having been established
       */
      sigc::signal<void> missed;

      /* Signal emitted when the call is forwarded
       */
      sigc::signal<void> forwarded;

      /* Signal emitted when the call is held
       */
      sigc::signal<void> held;
       
      /* Signal emitted when the call is retrieved
       */
      sigc::signal<void> retrieved;

      /* Signal emitted when the call has been setup
       */
      sigc::signal<void> setup;

      /* Signal emitted when a stream is opened
       * @param the stream name
       * @param the stream type
       * @param transmission or reception
       */
      sigc::signal<void, std::string, StreamType, bool> stream_opened;

      /* Signal emitted when a stream is closed
       * @param the stream name
       * @param the stream type
       * @param transmission or reception
       */
      sigc::signal<void, std::string, StreamType, bool> stream_closed;

      /* Signal emitted when a transmitted stream is paused
       * @param the stream name
       * @param the stream type
       * @param transmission or reception
       */
      sigc::signal<void, std::string, StreamType> stream_paused;

      /* Signal emitted when a transmitted stream is resumed
       * @param the stream name
       * @param the stream type
       * @param transmission or reception
       */
      sigc::signal<void, std::string, StreamType> stream_resumed;


  protected :
      
      std::string remote_party_name;
      std::string remote_uri;
      std::string remote_application;

      bool outgoing;

      double re_a_bw; 
      double tr_a_bw; 
      double re_v_bw; 
      double tr_v_bw; 
      unsigned re_v_fps;
      unsigned tr_v_fps;
      unsigned tr_width;
      unsigned tr_height;
      unsigned re_width;
      unsigned re_height;

      unsigned jitter; 

      double lost_packets; 
      double late_packets; 
      double out_of_order_packets; 
    };
};

#endif
