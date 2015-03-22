
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
 *                         opal-endpoint.cpp  -  description
 *                         ---------------------------------
 *   begin                : Sat Dec 23 2000
 *   authors              : Damien Sandras
 *   description          : This file contains our OpalManager.
 *
 */


#include <algorithm>
#include <glib/gi18n.h>

#include "opal-call-manager.h"

#include "pcss-endpoint.h"

#include "call-core.h"
#include "opal-codec-description.h"
#include "videoinput-info.h"

#include "call-manager.h"

#include "sip-endpoint.h"
#ifdef HAVE_H323
#include "h323-endpoint.h"
#endif

#include <opal/transcoders.h>

#include <stdlib.h>

// opal manages its endpoints itself, so we must be wary
struct null_deleter
{
  void operator()(void const *) const
    { }
};


class StunDetector : public PThread
{
  PCLASSINFO(StunDetector, PThread);

public:

  StunDetector (const std::string & _server,
                Opal::EndPoint& _manager,
                GAsyncQueue* _queue)
    : PThread (1000, AutoDeleteThread),
    server (_server),
    manager (_manager),
    queue (_queue)
  {
    PTRACE (3, "Ekiga\tStarted STUN detector");
    g_async_queue_ref (queue);
    this->Resume ();
  };

  ~StunDetector ()
    {
      g_async_queue_unref (queue);
      PTRACE (3, "Ekiga\tStopped STUN detector");
    }

  void Main ()
    {
      PSTUNClient::NatTypes result = manager.SetSTUNServer (server);

      g_async_queue_push (queue, GUINT_TO_POINTER ((guint)result + 1));
    };

private:
  const std::string server;
  Opal::EndPoint & manager;
  GAsyncQueue* queue;
};


/* The class */
Opal::EndPoint::EndPoint (Ekiga::ServiceCore& core)
{
  call_core = core.get<Ekiga::CallCore> ("call-core");

  stun_thread = 0;

  /* Initialise the endpoint parameters */
#if P_HAS_IPV6
  char * ekiga_ipv6 = getenv("EKIGA_IPV6");
  // use IPv6 instead of IPv4 if EKIGA_IPV6 env var is set
  if (ekiga_ipv6 && PIPSocket::IsIpAddressFamilyV6Supported())
    PIPSocket::SetDefaultIpAddressFamilyV6();
  else
    PIPSocket::SetDefaultIpAddressFamilyV4();
#else
  PIPSocket::SetDefaultIpAddressFamilyV4();
#endif
  PIPSocket::SetSuppressCanonicalName (true);  // avoid long delays
  SetAutoStartTransmitVideo (true);
  SetAutoStartReceiveVideo (true);
  SetUDPPorts (5000, 5100);
  SetTCPPorts (30000, 30100);
  SetRtpIpPorts (5000, 5100);

  forward_on_no_answer = false;
  forward_on_busy = false;
  unconditional_forward = false;
  stun_enabled = false;
  auto_answer = false;

  // Create video devices
  PVideoDevice::OpenArgs video = GetVideoOutputDevice();
  video.deviceName = "EKIGAOUT";
  SetVideoOutputDevice (video);

  video = GetVideoOutputDevice();
  video.deviceName = "EKIGAIN";
  SetVideoPreviewDevice (video);

  video = GetVideoInputDevice();
  video.deviceName = "EKIGA";
  SetVideoInputDevice (video);

  // Media formats
  SetMediaFormatOrder (PStringArray ());
  SetMediaFormatMask (PStringArray ());

  // used to communicate with the StunDetector
  queue = g_async_queue_new ();

  PInterfaceMonitor::GetInstance().SetRefreshInterval (15000);

  // Create endpoints
  // Their destruction is controlled by Opal
  GMPCSSEndpoint *pcss_endpoint = new GMPCSSEndpoint (*this, core);
  pcss_endpoint->SetSoundChannelPlayDevice("EKIGA");
  pcss_endpoint->SetSoundChannelRecordDevice("EKIGA");
  sip_endpoint = new Sip::EndPoint (*this, core);
#ifdef HAVE_H323
  h323_endpoint= new H323::EndPoint (*this, core);
#endif
}


Opal::EndPoint::~EndPoint ()
{
  if (stun_thread)
    stun_thread->WaitForTermination ();

  g_async_queue_unref (queue);
}


void Opal::EndPoint::SetEchoCancellation (bool enabled)
{
  OpalEchoCanceler::Params ec;

  // General settings
  ec = GetEchoCancelParams ();
  ec.m_enabled = enabled;
  SetEchoCancelParams (ec);

  // Adjust setting for all connections of all calls
  for (PSafePtr<OpalCall> call = activeCalls;
       call != NULL;
       ++call) {

    for (int i = 0;
         i < 2;
         i++) {

      PSafePtr<OpalConnection> connection = call->GetConnection (i);
      if (connection) {

        OpalEchoCanceler *echo_canceler = connection->GetEchoCanceler ();

        if (echo_canceler)
          echo_canceler->SetParameters (ec);
      }
    }
  }

  PTRACE (4, "Opal::EndPoint\tEcho Cancellation: " << enabled);
}


bool Opal::EndPoint::GetEchoCancellation () const
{
  OpalEchoCanceler::Params ec = GetEchoCancelParams ();

  return ec.m_enabled;
}


void Opal::EndPoint::SetMaximumJitter (unsigned max_val)
{
  unsigned val = std::min (std::max (max_val, (unsigned) 20), (unsigned) 1000);

  SetAudioJitterDelay (20, val);

  // Adjust setting for all sessions of all connections of all calls
  for (PSafePtr<OpalCall> call = activeCalls;
       call != NULL;
       ++call) {

    for (int i = 0;
         i < 2;
         i++) {

      PSafePtr<OpalRTPConnection> connection = PSafePtrCast<OpalConnection, OpalRTPConnection> (call->GetConnection (i));
      if (connection) {

        OpalMediaStreamPtr stream = connection->GetMediaStream (OpalMediaType::Audio (), false);
        if (stream != NULL) {

          OpalRTPSession *session = (OpalRTPSession*)connection->GetMediaSession (stream->GetSessionID ());
          if (session != NULL) {

            unsigned units = session->GetJitterTimeUnits ();
            OpalJitterBuffer::Init init;
            init.m_minJitterDelay = 20 * units;
            init.m_maxJitterDelay = val * units;
            init.m_timeUnits = units;
            session->SetJitterBufferSize (init);
          }
        }
      }
    }
  }

  PTRACE (4, "Opal::EndPoint\tSet Maximum Jitter to " << val);
}


unsigned Opal::EndPoint::GetMaximumJitter () const
{
  return GetMaxAudioJitterDelay ();
}


void Opal::EndPoint::SetSilenceDetection (bool enabled)
{
  OpalSilenceDetector::Params sd;

  // General settings
  sd = GetSilenceDetectParams ();
  if (enabled)
    sd.m_mode = OpalSilenceDetector::AdaptiveSilenceDetection;
  else
    sd.m_mode = OpalSilenceDetector::NoSilenceDetection;
  SetSilenceDetectParams (sd);

  // Adjust setting for all connections of all calls
  for (PSafePtr<OpalCall> call = activeCalls;
       call != NULL;
       ++call) {

    for (int i = 0;
         i < 2;
         i++) {

      PSafePtr<OpalConnection> connection = call->GetConnection (i);
      if (connection) {

        OpalSilenceDetector *silence_detector = connection->GetSilenceDetector ();

        if (silence_detector)
          silence_detector->SetParameters (sd);
      }
    }
  }

  PTRACE (4, "Opal::EndPoint\tSilence Detection: " << enabled);
}


bool Opal::EndPoint::GetSilenceDetection () const
{
  OpalSilenceDetector::Params sd;

  sd = GetSilenceDetectParams ();

  return (sd.m_mode != OpalSilenceDetector::NoSilenceDetection);
}


void Opal::EndPoint::set_reject_delay (unsigned delay)
{
  reject_delay = std::max ((unsigned) 5, delay);
}


unsigned Opal::EndPoint::get_reject_delay () const
{
  return reject_delay;
}


void Opal::EndPoint::set_auto_answer (bool enabled)
{
  auto_answer = enabled;
}


bool Opal::EndPoint::get_auto_answer (void) const
{
  return auto_answer;
}


void Opal::EndPoint::set_forward_on_no_answer (bool enabled)
{
  forward_on_no_answer = enabled;
}

bool Opal::EndPoint::get_forward_on_no_answer ()
{
  return forward_on_no_answer;
}

void Opal::EndPoint::set_forward_on_busy (bool enabled)
{
  forward_on_busy = enabled;
}

bool Opal::EndPoint::get_forward_on_busy ()
{
  return forward_on_busy;
}

void Opal::EndPoint::set_unconditional_forward (bool enabled)
{
  unconditional_forward = enabled;
}

bool Opal::EndPoint::get_unconditional_forward ()
{
  return unconditional_forward;
}


void Opal::EndPoint::set_stun_server (const std::string & server)
{
  std::cout << "Set STUN SERVER TO " << server << std::endl << std::flush;
  if (server.empty ())
    stun_server = "stun.ekiga.net";

  stun_server = server;
  PTRACE (4, "Opal::EndPoint\tSet STUN Server to " << stun_server);
}


void Opal::EndPoint::set_stun_enabled (bool enabled)
{
  stun_enabled = enabled;
  if (stun_enabled && !stun_thread) {

    // Ready
    stun_thread = new StunDetector (stun_server, *this, queue);
    patience = 20;
    Ekiga::Runtime::run_in_main (boost::bind (&Opal::EndPoint::HandleSTUNResult, this), 1);
  }
  else {
    ready ();
  }

  PTRACE (4, "Opal::EndPoint\tSTUN Detection: " << enabled);
}


Opal::Sip::EndPoint& Opal::EndPoint::GetSipEndPoint ()
{
  return *sip_endpoint;
}


#ifdef HAVE_H323
Opal::H323::EndPoint& Opal::EndPoint::GetH323EndPoint ()
{
  return *h323_endpoint;
}
#endif


void Opal::EndPoint::SetVideoOptions (const Opal::EndPoint::VideoOptions & options)
{
  OpalMediaFormatList media_formats_list;
  OpalMediaFormat::GetAllRegisteredMediaFormats (media_formats_list);

  int maximum_frame_rate = std::min (std::max ((signed) options.maximum_frame_rate, 1), 30);
  int maximum_bitrate = (options.maximum_bitrate > 0 ? options.maximum_bitrate : 16384);
  int maximum_transmitted_bitrate = (options.maximum_transmitted_bitrate > 0 ? options.maximum_transmitted_bitrate : 256);
  int temporal_spatial_tradeoff = (options.temporal_spatial_tradeoff > 0 ? options.temporal_spatial_tradeoff : 12);
  // Configure all mediaOptions of all Video MediaFormats
  for (int i = 0 ; i < media_formats_list.GetSize () ; i++) {

    OpalMediaFormat media_format = media_formats_list [i];
    if (media_format.GetMediaType() == OpalMediaType::Video ()) {

      media_format.SetOptionInteger (OpalVideoFormat::FrameWidthOption (), Ekiga::VideoSizes [options.size].width);
      media_format.SetOptionInteger (OpalVideoFormat::FrameHeightOption (), Ekiga::VideoSizes [options.size].height);
      media_format.SetOptionInteger (OpalVideoFormat::FrameTimeOption (), (int) (media_format.GetClockRate () / maximum_frame_rate));
      media_format.SetOptionInteger (OpalVideoFormat::MaxBitRateOption (), maximum_bitrate * 1000);
      media_format.SetOptionInteger (OpalVideoFormat::TargetBitRateOption (), maximum_transmitted_bitrate * 1000);
      media_format.SetOptionInteger (OpalVideoFormat::MinRxFrameWidthOption(), GM_QSIF_WIDTH);
      media_format.SetOptionInteger (OpalVideoFormat::MinRxFrameHeightOption(), GM_QSIF_HEIGHT);
      media_format.SetOptionInteger (OpalVideoFormat::MaxRxFrameWidthOption(), GM_1080P_WIDTH);
      media_format.SetOptionInteger (OpalVideoFormat::MaxRxFrameHeightOption(), GM_1080P_HEIGHT);
      media_format.AddOption(new OpalMediaOptionUnsigned (OpalVideoFormat::TemporalSpatialTradeOffOption (), true, OpalMediaOption::NoMerge, temporal_spatial_tradeoff));
      media_format.SetOptionInteger (OpalVideoFormat::TemporalSpatialTradeOffOption(), temporal_spatial_tradeoff);

      if (media_format.GetName() != "YUV420P" &&
          media_format.GetName() != "RGB32" &&
          media_format.GetName() != "RGB24")
        media_format.SetOptionInteger (OpalVideoFormat::RateControlPeriodOption(), 300);

      switch (options.extended_video_roles) {
      case 0 :
        media_format.SetOptionInteger(OpalVideoFormat::ContentRoleMaskOption(), 0);
        break;

      case 2 : // Force Presentation (slides)
        media_format.SetOptionInteger(OpalVideoFormat::ContentRoleMaskOption(),
                                      OpalVideoFormat::ContentRoleBit(OpalVideoFormat::ePresentation));
        break;

      case 3 : // Force Live (main)
        media_format.SetOptionInteger(OpalVideoFormat::ContentRoleMaskOption(),
                                      OpalVideoFormat::ContentRoleBit(OpalVideoFormat::eMainRole));
        break;

        default :
          break;
      }

      OpalMediaFormat::SetRegisteredMediaFormat(media_format);
    }
  }

  // Adjust setting for all sessions of all connections of all calls
  for (PSafePtr<OpalCall> call = activeCalls;
       call != NULL;
       ++call) {

    for (int i = 0;
         i < 2;
         i++) {

      PSafePtr<OpalRTPConnection> connection = PSafePtrCast<OpalConnection, OpalRTPConnection> (call->GetConnection (i));
      if (connection) {

        OpalMediaStreamPtr stream = connection->GetMediaStream (OpalMediaType::Video (), false);
        if (stream != NULL) {

          OpalMediaFormat mediaFormat = stream->GetMediaFormat ();
          mediaFormat.SetOptionInteger (OpalVideoFormat::TemporalSpatialTradeOffOption(),
                                        temporal_spatial_tradeoff);
          mediaFormat.SetOptionInteger (OpalVideoFormat::TargetBitRateOption (),
                                        maximum_transmitted_bitrate * 1000);
          mediaFormat.ToNormalisedOptions();
          stream->UpdateMediaFormat (mediaFormat);
        }
      }
    }
  }

  PTRACE (4, "Opal::EndPoint\tVideo Max Bitrate: " << maximum_bitrate);
  PTRACE (4, "Opal::EndPoint\tVideo Max Tx Bitrate: " << maximum_transmitted_bitrate);
  PTRACE (4, "Opal::EndPoint\tVideo Temporal Spatial Tradeoff: " << temporal_spatial_tradeoff);
  PTRACE (4, "Opal::EndPoint\tVideo Size: " << options.size);
  PTRACE (4, "Opal::EndPoint\tVideo Max Frame Rate: " << maximum_frame_rate);
}


void Opal::EndPoint::GetVideoOptions (Opal::EndPoint::VideoOptions & options) const
{
  OpalMediaFormatList media_formats_list;
  OpalMediaFormat::GetAllRegisteredMediaFormats (media_formats_list);

  for (int i = 0 ; i < media_formats_list.GetSize () ; i++) {

    OpalMediaFormat media_format = media_formats_list [i];
    if (media_format.GetMediaType () == OpalMediaType::Video ()) {

      int j;
      for (j = 0; j < NB_VIDEO_SIZES; j++) {

        if (Ekiga::VideoSizes [j].width == media_format.GetOptionInteger (OpalVideoFormat::FrameWidthOption ())
            && Ekiga::VideoSizes [j].height == media_format.GetOptionInteger (OpalVideoFormat::FrameHeightOption ()))
          break;
      }
      if (j >= NB_VIDEO_SIZES)
        g_error ("Cannot find video size");
      options.size = j;

      options.maximum_frame_rate = (int) (media_format.GetClockRate () / media_format.GetFrameTime ());
      options.maximum_bitrate = (int) (media_format.GetOptionInteger (OpalVideoFormat::MaxBitRateOption ()) / 1000);
      options.maximum_transmitted_bitrate = (int) (media_format.GetOptionInteger (OpalVideoFormat::TargetBitRateOption ()) / 1000);
      options.temporal_spatial_tradeoff = media_format.GetOptionInteger (OpalVideoFormat::TemporalSpatialTradeOffOption ());

      int evr = media_format.GetOptionInteger (OpalVideoFormat::OpalVideoFormat::ContentRoleMaskOption ());
      switch (evr) {
      case 0: // eNoRole
        options.extended_video_roles = 0;
        break;
      case 1: // ePresentation
        options.extended_video_roles = 2;
        break;
      case 2: // eMainRole
        options.extended_video_roles = 3;
        break;
      default:
        options.extended_video_roles = 1;
        break;
      }

      break;
    }
  }
}


OpalCall *Opal::EndPoint::CreateCall (void *uri)
{
  Opal::Call* call = 0;

  if (uri != 0)
    call = new Opal::Call (*this, (const char *) uri);
  else
    call = new Opal::Call (*this, "");

  Ekiga::Runtime::run_in_main (boost::bind (boost::ref (created_call), call));

  return call;
}

void
Opal::EndPoint::DestroyCall (OpalCall* call)
{
  delete call;
}


void
Opal::EndPoint::HandleSTUNResult ()
{
  gboolean error = false;
  gboolean got_answer = false;

  if (g_async_queue_length (queue) > 0) {

    PSTUNClient::NatTypes result
      = (PSTUNClient::NatTypes)(GPOINTER_TO_UINT (g_async_queue_pop (queue))-1);
    got_answer = true;
    stun_thread = 0;

    if (result == PSTUNClient::SymmetricNat
	|| result == PSTUNClient::BlockedNat
	|| result == PSTUNClient::PartiallyBlocked) {

      error = true;
    }
    else {

      ready ();
    }
  }
  else if (patience == 0) {

    error = true;
  }

  if (error) {

    ReportSTUNError (_("Ekiga did not manage to configure your network settings automatically. You can"
		       " still use it, but you need to configure your network settings manually.\n\n"
		       "Please see http://wiki.ekiga.org/index.php/Enable_port_forwarding_manually for"
		       " instructions"));
    ready ();
  }
  else if (!got_answer) {

    patience--;
    Ekiga::Runtime::run_in_main (boost::bind (&Opal::EndPoint::HandleSTUNResult, this), 1);
  }
}


void
Opal::EndPoint::ReportSTUNError (const std::string error)
{
  boost::shared_ptr<Ekiga::CallCore> ccore = call_core.lock ();
  if (!ccore)
    return;

  // notice we're in for an infinite loop if nobody ever reports to the user!
  if ( !ccore->errors (error)) {

    Ekiga::Runtime::run_in_main (boost::bind (&Opal::EndPoint::ReportSTUNError, this, error),
				 10);
  }
}


PBoolean
Opal::EndPoint::CreateVideoOutputDevice (const OpalConnection & connection,
                                         const OpalMediaFormat & media_fmt,
                                         PBoolean preview,
                                         PVideoOutputDevice * & device,
                                         PBoolean & auto_delete)
{
  PVideoDevice::OpenArgs videoArgs;
  PString title;

  videoArgs = preview ?
    GetVideoPreviewDevice() : GetVideoOutputDevice();

  if (!preview) {
    unsigned openChannelCount = 0;
    OpalMediaStreamPtr mediaStream;

    while ((mediaStream = connection.GetMediaStream(OpalMediaType::Video(),
                                                    preview, mediaStream)) != NULL)
      ++openChannelCount;

    videoArgs.deviceName += psprintf(" ID=%u", openChannelCount);
  }

  media_fmt.AdjustVideoArgs(videoArgs);

  auto_delete = true;
  device = PVideoOutputDevice::CreateOpenedDevice (videoArgs, false);

  return device != NULL;
}

