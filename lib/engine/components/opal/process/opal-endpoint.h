
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         opal-endpoint.h  -  description
 *                         -------------------------------
 *   begin                : Sat Dec 23 2000
 *   authors              : Damien Sandras
 *   description          : This file contains our OpalManager.
 *
 */


#ifndef __OPAL_ENDPOINT_H_
#define __OPAL_ENDPOINT_H_

#include "config.h"

#include <ptlib.h>

#ifdef HAVE_H323
#include <h323/h323.h>
#include "h323-endpoint.h"
#endif

#include <sip/sip.h>

#include "opal-call.h"

#include "call-manager.h"
#include "contact-core.h"

#include "actor.h"

class GMPCSSEndpoint;

namespace Opal {

  class Account;
  class CallManager;
  namespace SIP {
    class EndPoint;
  }
#ifdef HAVE_H323
  namespace H323 {
    class EndPoint;
  }
#endif

  /* This is the OPAL endpoint. We do not want it to directly
   * use the CallCore, CallManager's and other engine implementations.
   * We want it to provide those services to the relevant engine objects
   * and trigger signals when appropriate.
   */
  class EndPoint : public OpalManager
  {
    PCLASSINFO(EndPoint, OpalManager);

public:

    EndPoint (Ekiga::ServiceCore& _core);

    ~EndPoint ();

    void SetEchoCancellation (bool enabled);
    bool GetEchoCancellation () const;

    void SetMaximumJitter (unsigned max_val);
    unsigned GetMaximumJitter () const;

    void SetSilenceDetection (bool enabled);
    bool GetSilenceDetection () const;

    void SetNoAnswerDelay (unsigned delay);
    unsigned GetNoAnswerDelay () const;

    void SetAutoAnswer (bool enabled);
    bool GetAutoAnswer () const;

    void set_stun_server (const std::string & server);
    void set_stun_enabled (bool);

    Sip::EndPoint& GetSipEndPoint ();
#ifdef HAVE_H323
    H323::EndPoint& GetH323EndPoint ();
#endif


    /**/
    struct VideoOptions
      {
        VideoOptions ()
          : size (0),
          maximum_frame_rate (0),
          temporal_spatial_tradeoff (0),
          maximum_bitrate (0),
          maximum_transmitted_bitrate (0),
          extended_video_roles (0) {};

        unsigned size;
        unsigned maximum_frame_rate;
        unsigned temporal_spatial_tradeoff;
        unsigned maximum_bitrate;
        unsigned maximum_transmitted_bitrate;
        unsigned extended_video_roles;
      };

    void SetVideoOptions (const VideoOptions & options);
    void GetVideoOptions (VideoOptions & options) const;

    boost::signals2::signal<void(void)> ready;

private:
    OpalCall *CreateCall (void *uri);

    void DestroyCall (OpalCall * call);

    void DestroyCall (boost::shared_ptr<Ekiga::Call> call);

    void HandleSTUNResult ();

    void ReportSTUNError (const std::string error);

    PBoolean CreateVideoOutputDevice (const OpalConnection & connection,
                                      const OpalMediaFormat & media_fmt,
                                      PBoolean preview,
                                      PVideoOutputDevice * & device,
                                      PBoolean & auto_delete);

    OpalConnection::AnswerCallResponse OnAnswerCall (OpalConnection & connection,
                                                     const PString & caller);


    /* used to get the STUNDetector results */
    PThread* stun_thread;
    GAsyncQueue* queue;
    unsigned int patience;

    std::string stun_server;
    unsigned noAnswerDelay;
    bool autoAnswer;
    bool stun_enabled;

    /* The various related endpoints */
    Sip::EndPoint *sip_endpoint;
#ifdef HAVE_H323
    H323::EndPoint *h323_endpoint;
#endif
    Ekiga::ServiceCore& core;
  };
};
#endif
