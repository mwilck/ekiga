
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
 *                         opal-call-manager.cpp  -  description
 *                         ---------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the Endpoint class.
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
                Opal::CallManager& _manager,
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
  Opal::CallManager & manager;
  GAsyncQueue* queue;
};


using namespace Opal;


/* The class */
CallManager::CallManager (Ekiga::ServiceCore& core)
{
  call_core = core.get<Ekiga::CallCore> ("call-core");
  notification_core = core.get<Ekiga::NotificationCore> ("notification-core");

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
  SetAutoStartTransmitVideo (true);
  SetAutoStartReceiveVideo (true);
  SetUDPPorts (5000, 5100);
  SetTCPPorts (30000, 30100);
  SetRtpIpPorts (5000, 5100);

  pcssEP = NULL;

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

  // Create endpoints
  pcssEP = new GMPCSSEndpoint (*this, core);
  pcssEP->SetSoundChannelPlayDevice("EKIGA");
  pcssEP->SetSoundChannelRecordDevice("EKIGA");

  // Media formats
  SetMediaFormatOrder (PStringArray ());
  SetMediaFormatMask (PStringArray ());

  // used to communicate with the StunDetector
  queue = g_async_queue_new ();

  PInterfaceMonitor::GetInstance().SetRefreshInterval (15000);

  nat_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (NAT_SCHEMA));
  nat_settings->changed.connect (boost::bind (&CallManager::setup, this, _1));

  audio_codecs_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (AUDIO_CODECS_SCHEMA));
  audio_codecs_settings->changed.connect (boost::bind (&CallManager::setup, this, _1));

  video_codecs_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (VIDEO_CODECS_SCHEMA));
  video_codecs_settings->changed.connect (boost::bind (&CallManager::setup, this, _1));

  video_devices_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (VIDEO_DEVICES_SCHEMA));
  video_devices_settings->changed.connect (boost::bind (&CallManager::setup, this, _1));

  ports_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (PORTS_SCHEMA));
  ports_settings->changed.connect (boost::bind (&CallManager::setup, this, _1));

  protocols_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (PROTOCOLS_SCHEMA));
  protocols_settings->changed.connect (boost::bind (&CallManager::setup, this, _1));

  call_options_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (CALL_OPTIONS_SCHEMA));
  call_options_settings->changed.connect (boost::bind (&CallManager::setup, this, _1));

  call_forwarding_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (CALL_FORWARDING_SCHEMA));
  call_forwarding_settings->changed.connect (boost::bind (&CallManager::setup, this, _1));

  personal_data_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (PERSONAL_DATA_SCHEMA));
  personal_data_settings->changed.connect (boost::bind (&CallManager::setup, this, _1));

  /* ContactActor & PresenceCore registration */
  // FIXME could move to CallCore
  boost::shared_ptr<Ekiga::ContactCore> contact_core = core.get< Ekiga::ContactCore > ("contact-core");
  contact_core->push_back (Ekiga::URIActionProviderPtr (this));

  boost::shared_ptr<Ekiga::PresenceCore> presence_core = core.get< Ekiga::PresenceCore > ("presence-core");
  presence_core->push_back (Ekiga::URIActionProviderPtr (this));
}


CallManager::~CallManager ()
{
  if (stun_thread)
    stun_thread->WaitForTermination ();
  ClearAllCalls (OpalConnection::EndedByLocalUser, true);
  ShutDownEndpoints ();

  g_async_queue_unref (queue);
}


void CallManager::pull_actions (Ekiga::Actor & actor,
                                const std::string & name,
                                const std::string & uri)
{
  if (is_supported_uri (uri)) {

    add_action (actor,
                Ekiga::ActionPtr (new Ekiga::Action ("call", _("Call"), boost::bind (&CallManager::dial, this, uri))));
  }
}


void CallManager::set_display_name (const std::string & name)
{
  display_name = name;

  SetDefaultDisplayName (display_name);
}


const std::string & CallManager::get_display_name () const
{
  return display_name;
}


void CallManager::set_echo_cancellation (bool enabled)
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

  PTRACE (4, "Opal::CallManager\tEcho Cancellation: " << enabled);
}


bool CallManager::get_echo_cancellation () const
{
  OpalEchoCanceler::Params ec = GetEchoCancelParams ();

  return ec.m_enabled;
}


void CallManager::set_maximum_jitter (unsigned max_val)
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

  PTRACE (4, "Opal::CallManager\tSet Maximum Jitter to " << val);
}


unsigned CallManager::get_maximum_jitter () const
{
  return GetMaxAudioJitterDelay ();
}


void CallManager::set_silence_detection (bool enabled)
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

  PTRACE (4, "Opal::CallManager\tSilence Detection: " << enabled);
}


bool CallManager::get_silence_detection () const
{
  OpalSilenceDetector::Params sd;

  sd = GetSilenceDetectParams ();

  return (sd.m_mode != OpalSilenceDetector::NoSilenceDetection);
}


void CallManager::set_reject_delay (unsigned delay)
{
  reject_delay = std::max ((unsigned) 5, delay);
}


unsigned CallManager::get_reject_delay () const
{
  return reject_delay;
}


void CallManager::set_auto_answer (bool enabled)
{
  auto_answer = enabled;
}


bool CallManager::get_auto_answer (void) const
{
  return auto_answer;
}


const Ekiga::CodecList & CallManager::get_codecs () const
{
  return codecs;
}


void CallManager::set_codecs (Ekiga::CodecList & _codecs)
{
  PStringArray mask, order;
  OpalMediaFormatList formats;
  OpalMediaFormat::GetAllRegisteredMediaFormats (formats);

  codecs = _codecs;

  for (Ekiga::CodecList::const_iterator iter = codecs.begin ();
       iter != codecs.end ();
       iter++)
    if ((*iter).active)
      order += (*iter).name;

  formats.Remove (order);

  for (int i = 0 ; i < formats.GetSize () ; i++)
    mask += (const char *) formats[i];

  SetMediaFormatOrder (order);
  SetMediaFormatMask (mask);
  PTRACE (4, "Ekiga\tSet codecs: " << setfill(';') << GetMediaFormatOrder ());
  PTRACE (4, "Ekiga\tDisabled codecs: " << setfill(';') << GetMediaFormatMask ());
}


void CallManager::set_forward_on_no_answer (bool enabled)
{
  forward_on_no_answer = enabled;
}

bool CallManager::get_forward_on_no_answer ()
{
  return forward_on_no_answer;
}

void CallManager::set_forward_on_busy (bool enabled)
{
  forward_on_busy = enabled;
}

bool CallManager::get_forward_on_busy ()
{
  return forward_on_busy;
}

void CallManager::set_unconditional_forward (bool enabled)
{
  unconditional_forward = enabled;
}

bool CallManager::get_unconditional_forward ()
{
  return unconditional_forward;
}

void CallManager::set_udp_ports (unsigned min_port,
                                 unsigned max_port)
{
  if (min_port < max_port) {

    SetUDPPorts (min_port, max_port);
    SetRtpIpPorts (min_port, max_port);
  }
}


void CallManager::get_udp_ports (unsigned & min_port,
                                 unsigned & max_port) const
{
  min_port = GetUDPPortBase ();
  max_port = GetUDPPortMax ();
}

void CallManager::set_tcp_ports (unsigned min_port,
                                 unsigned max_port)
{
  if (min_port < max_port)
    SetTCPPorts (min_port, max_port);
}


void CallManager::get_tcp_ports (unsigned & min_port,
                                 unsigned & max_port) const
{
  min_port = GetTCPPortBase ();
  max_port = GetTCPPortMax ();
}

void
CallManager::get_rtp_tos (unsigned &tos) const
{
  tos = GetMediaTypeOfService ();
}

void
CallManager::set_rtp_tos (unsigned tos)
{
  SetMediaTypeOfService (tos);
}

void CallManager::set_stun_server (const std::string & server)
{
  if (server.empty ())
    stun_server = "stun.ekiga.net";

  stun_server = server;
  PTRACE (4, "Opal::CallManager\tSet STUN Server to " << stun_server);
}


void CallManager::set_stun_enabled (bool enabled)
{
  stun_enabled = enabled;
  if (stun_enabled && !stun_thread) {

    // Ready
    stun_thread = new StunDetector (stun_server, *this, queue);
    patience = 20;
    Ekiga::Runtime::run_in_main (boost::bind (&CallManager::HandleSTUNResult, this), 1);
  } else
    ready ();

  PTRACE (4, "Opal::CallManager\tSTUN Detection: " << enabled);
}


bool CallManager::dial (const std::string & uri)
{
  for (Ekiga::CallManager::iterator iter = Ekiga::CallManager::begin ();
       iter != Ekiga::CallManager::end ();
       iter++)
    if ((*iter)->dial (uri))
      return true;

  return false;
}


void CallManager::hang_up ()
{
  ClearAllCalls (OpalConnection::EndedByLocalUser, true);
}


bool CallManager::transfer (const std::string & uri,
                            bool attended)
{
  for (Ekiga::CallManager::iterator iter = Ekiga::CallManager::begin ();
       iter != Ekiga::CallManager::end ();
       iter++)
    if ((*iter)->transfer (uri, attended))
      return true;

  return false;
}


bool CallManager::is_supported_uri (const std::string & uri)
{
  for (Ekiga::CallManager::iterator iter = Ekiga::CallManager::begin ();
       iter != Ekiga::CallManager::end ();
       iter++)
    if ((*iter)->is_supported_uri (uri))
      return true;

  return false;
}


void CallManager::set_video_options (const CallManager::VideoOptions & options)
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

  PTRACE (4, "Opal::CallManager\tVideo Max Bitrate: " << maximum_bitrate);
  PTRACE (4, "Opal::CallManager\tVideo Max Tx Bitrate: " << maximum_transmitted_bitrate);
  PTRACE (4, "Opal::CallManager\tVideo Temporal Spatial Tradeoff: " << temporal_spatial_tradeoff);
  PTRACE (4, "Opal::CallManager\tVideo Size: " << options.size);
  PTRACE (4, "Opal::CallManager\tVideo Max Frame Rate: " << maximum_frame_rate);
}


void CallManager::get_video_options (CallManager::VideoOptions & options) const
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

void
CallManager::create_call_in_main (Opal::Call* _call)
{
  // beware that opal puts calls in PSafePtr objects,
  // and hence we must not delete them ourselves
  boost::shared_ptr<Opal::Call> call(_call, null_deleter ());

  // FIXME: we should check it worked, but what do we do if it doesn't?
  boost::shared_ptr<Ekiga::CallCore> ccore = call_core.lock ();

  call->set_engine_services (notification_core, ccore);

  ccore->add_call (call, boost::dynamic_pointer_cast<CallManager>(shared_from_this ()));
}

OpalCall *CallManager::CreateCall (void *uri)
{
  Opal::Call* call = 0;

  if (uri != 0)
    call = new Opal::Call (*this, (const char *) uri);
  else
    call = new Opal::Call (*this, "");

  Ekiga::Runtime::run_in_main (boost::bind (&CallManager::create_call_in_main, this, call));

  return call;
}

void
CallManager::DestroyCall (OpalCall* call)
{
  delete call;
}


void
CallManager::OnClosedMediaStream (const OpalMediaStream & stream)
{
  OpalMediaFormatList list = pcssEP->GetMediaFormats ();
  OpalManager::OnClosedMediaStream (stream);

  if (list.FindFormat(stream.GetMediaFormat()) != list.end ())
    dynamic_cast <Opal::Call &> (stream.GetConnection ().GetCall ()).OnClosedMediaStream ((OpalMediaStream &) stream);
}


bool
CallManager::OnOpenMediaStream (OpalConnection & connection,
				OpalMediaStream & stream)
{
  OpalMediaFormatList list = pcssEP->GetMediaFormats ();
  if (!OpalManager::OnOpenMediaStream (connection, stream))
    return FALSE;

  if (list.FindFormat(stream.GetMediaFormat()) == list.end ())
    dynamic_cast <Opal::Call &> (connection.GetCall ()).OnOpenMediaStream (stream);

  return TRUE;
}


void CallManager::GetAllowedFormats (OpalMediaFormatList & full_list)
{
  OpalMediaFormatList list = OpalTranscoder::GetPossibleFormats (pcssEP->GetMediaFormats ());
  list.RemoveNonTransportable ();
  std::list<std::string> black_list;

  black_list.push_back ("GSM-AMR");
  black_list.push_back ("Linear-16-Stereo-48kHz");
  black_list.push_back ("LPC-10");
  black_list.push_back ("SpeexIETFNarrow-11k");
  black_list.push_back ("SpeexIETFNarrow-15k");
  black_list.push_back ("SpeexIETFNarrow-18.2k");
  black_list.push_back ("SpeexIETFNarrow-24.6k");
  black_list.push_back ("SpeexIETFNarrow-5.95k");
  black_list.push_back ("iLBC-13k3");
  black_list.push_back ("iLBC-15k2");
  black_list.push_back ("RFC4175_YCbCr-4:2:0");
  black_list.push_back ("RFC4175_RGB");

  // Disable T.140 chat for now
  black_list.push_back ("T.140");

  // Disable CISCO NSE
  black_list.push_back ("NamedSignalEvent");

  // Disable Far-End Camera Control for now.
  black_list.push_back ("FECC-RTP");
  black_list.push_back ("FECC-HDLC");


  // Purge blacklisted codecs
  for (PINDEX i = 0 ; i < list.GetSize () ; i++) {

    std::list<std::string>::iterator it = find (black_list.begin (), black_list.end (), (const char *) list [i]);
    if (it == black_list.end ()) {
      if (list [i].GetMediaType () == OpalMediaType::Audio () || list [i].GetMediaType () == OpalMediaType::Video ())
        full_list += list [i];
    }
  }

  PTRACE(4, "Ekiga\tAll available formats: " << setfill (',') << full_list);
}

void
CallManager::HandleSTUNResult ()
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

      if (sip_endpoint)
        sip_endpoint->setup ("listen-port");
      if (h323_endpoint)
        h323_endpoint->setup ("listen-port");

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
    Ekiga::Runtime::run_in_main (boost::bind (&CallManager::HandleSTUNResult, this), 1);
  }
}

void
CallManager::ReportSTUNError (const std::string error)
{
  boost::shared_ptr<Ekiga::CallCore> ccore = call_core.lock ();
  if (!ccore)
    return;

  // notice we're in for an infinite loop if nobody ever reports to the user!
  if ( !ccore->errors (error)) {

    Ekiga::Runtime::run_in_main (boost::bind (&CallManager::ReportSTUNError, this, error),
				 10);
  }
}

PBoolean
CallManager::CreateVideoOutputDevice(const OpalConnection & connection,
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
  device = PVideoOutputDevice::CreateOpenedDevice(videoArgs, false);
  return device != NULL;
}


void
CallManager::set_sip_endpoint (boost::shared_ptr<Opal::Sip::EndPoint> _sip_endpoint)
{
  sip_endpoint = _sip_endpoint;
  add_protocol_manager (sip_endpoint);
}


#ifdef HAVE_H323
void
CallManager::set_h323_endpoint (boost::shared_ptr<Opal::H323::EndPoint> _h323_endpoint)
{
  h323_endpoint = _h323_endpoint;
  add_protocol_manager (h323_endpoint);
}
#endif

void
CallManager::setup (const std::string & setting)
{
  if (setting.empty () || setting == "stun-server") {

    set_stun_server (nat_settings->get_string ("stun-server"));
  }
  if (setting.empty () || setting == "enable-stun") {

    set_stun_enabled (nat_settings->get_bool ("enable-stun"));
  }
  if (setting.empty () || setting == "maximum-jitter-buffer") {

    set_maximum_jitter (audio_codecs_settings->get_int ("maximum-jitter-buffer"));
  }
  if (setting.empty () || setting == "enable-silence-detection") {

    set_silence_detection (audio_codecs_settings->get_bool ("enable-silence-detection"));
  }
  if (setting.empty () || setting == "enable-echo-cancellation") {

    set_echo_cancellation (audio_codecs_settings->get_bool ("enable-echo-cancellation"));
  }
  if (setting.empty () || setting == "rtp-tos-field") {
    set_rtp_tos (protocols_settings->get_int ("rtp-tos-field"));
  }
  if (setting.empty () || setting == "no-answer-timeout") {

    set_reject_delay (call_options_settings->get_int ("no-answer-timeout"));
  }
  if (setting.empty () || setting == "auto-answer") {
    set_auto_answer (call_options_settings->get_bool ("auto-answer"));
  }
  if (setting.empty () || setting == "forward-on-no-anwer") {
    set_forward_on_no_answer (call_forwarding_settings->get_bool ("forward-on-no-answer"));
  }
  if (setting.empty () || setting == "forward-on-busy") {
    set_forward_on_busy (call_forwarding_settings->get_bool ("forward-on-busy"));
  }
  if (setting.empty () || setting == "always-forward") {
    set_unconditional_forward (call_forwarding_settings->get_bool ("always-forward"));
  }
  if (setting.empty () || setting == "full-name") {
    std::string full_name = personal_data_settings->get_string ("full-name");
    if (!full_name.empty ())
      set_display_name (full_name);
  }
  if (setting.empty () || setting == "maximum-video-tx-bitrate") {

    CallManager::VideoOptions options;
    get_video_options (options);
    options.maximum_transmitted_bitrate = video_codecs_settings->get_int ("maximum-video-tx-bitrate");
    set_video_options (options);
  }
  if (setting.empty () || setting == "temporal-spatial-tradeoff") {

    CallManager::VideoOptions options;
    get_video_options (options);
    options.temporal_spatial_tradeoff = video_codecs_settings->get_int ("temporal-spatial-tradeoff");
    set_video_options (options);
  }
  if (setting.empty () || setting == "size") {

    CallManager::VideoOptions options;
    get_video_options (options);
    options.size = video_devices_settings->get_enum ("size");
    set_video_options (options);
  }
  if (setting.empty () || setting == "max-frame-rate") {

    CallManager::VideoOptions options;
    get_video_options (options);
    options.maximum_frame_rate = video_codecs_settings->get_int ("max-frame-rate");
    set_video_options (options);
  }
  if (setting.empty () || setting == "maximum-video-bitrate") {

    CallManager::VideoOptions options;
    get_video_options (options);
    options.maximum_bitrate = video_codecs_settings->get_int ("maximum-video-bitrate");
    set_video_options (options);
  }
  if (setting.empty () || setting == "media-list") {

    std::list<std::string> config_codecs = audio_codecs_settings->get_string_list ("media-list");
    std::list<std::string> video_codecs = video_codecs_settings->get_string_list ("media-list");

    config_codecs.insert (config_codecs.end(), video_codecs.begin(), video_codecs.end());
    // This will add all supported codecs that are not present in the configuration
    // at the end of the list.
    Opal::CodecList all_codecs;
    all_codecs.load (config_codecs);

    // Update the manager codecs
    set_codecs (all_codecs);
  }
  if (setting.empty () || setting == "udp-port-range" || setting == "tcp-port-range") {

    std::string ports;
    gchar **couple = NULL;
    unsigned min_port = 0;
    unsigned max_port = 0;
    const char *key[2] = { "udp-port-range", "tcp-port-range" };

    for (int i = 0 ; i < 2 ; i++) {
      ports = ports_settings->get_string (key[i]);
      if (!ports.empty ())
        couple = g_strsplit (ports.c_str (), ":", 2);
      if (couple && couple [0])
        min_port = atoi (couple [0]);
      if (couple && couple [1])
        max_port = atoi (couple [1]);

      if (i == 0)
        set_udp_ports (min_port, max_port);
      else
        set_tcp_ports (min_port, max_port);

      g_strfreev (couple);
    }
  }
}
