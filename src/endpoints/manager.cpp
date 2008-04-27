
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 *                         manager.cpp  -  description
 *                         ---------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the Endpoint class.
 *
 */


#include "config.h"

#include "manager.h"

#include "h323.h"
#include "sip.h"
#include "pcss.h"

#include "call-core.h"
#include "opal-gmconf-bridge.h"
#include "opal-call.h"
#include "opal-codec-description.h"
#include "vidinput-info.h"


static  bool same_codec_desc (Ekiga::CodecDescription a, Ekiga::CodecDescription b)
{ 
  return (a.name == b.name && a.rate == b.rate); 
}


class dialer : public PThread
{
  PCLASSINFO(dialer, PThread);

public:

  dialer (const std::string & uri, GMManager & ep) 
    : PThread (1000, AutoDeleteThread), 
      dial_uri (uri),
      endpoint (ep) 
  {
    this->Resume ();
  };
  
  void Main () 
  {
    PString token;
    endpoint.SetUpCall ("pc:*", dial_uri, token);
  };

private:
  const std::string dial_uri;
  GMManager & endpoint;
};


// FIXME: we shouldnt call sound events here but signal to the frontend which then triggers them

/* The class */
GMManager::GMManager (Ekiga::ServiceCore & _core)
: core (_core), 
  runtime (*(dynamic_cast<Ekiga::Runtime *> (core.get ("runtime")))),
  audiooutput_core (*(dynamic_cast<Ekiga::AudioOutputCore *> (_core.get ("audiooutput-core")))) 
{
  /* Initialise the endpoint paramaters */
  PIPSocket::SetDefaultIpAddressFamilyV4();
  autoStartTransmitVideo = autoStartReceiveVideo = true;
  
  manager = NULL;

  h323EP = NULL;
  sipEP = NULL;
  pcssEP = NULL;
  sc = NULL;

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
  h323EP = new GMH323Endpoint (*this, core);
  AddRouteEntry("pc:.* = h323:<da>");
	
  sipEP = new GMSIPEndpoint (*this, core);
  AddRouteEntry("pc:.* = sip:<da>");

  pcssEP = new GMPCSSEndpoint (*this, core);
  pcssEP->SetSoundChannelPlayDevice("EKIGA");
  pcssEP->SetSoundChannelRecordDevice("EKIGA");
  AddRouteEntry("h323:.* = pc:<db>");
  AddRouteEntry("sip:.* = pc:<db>");

  // Media formats
  SetMediaFormatOrder (PStringArray ());
  SetMediaFormatMask (PStringArray ());

  // Config
  bridge = new Opal::ConfBridge (*this);

  //
  call_core = dynamic_cast<Ekiga::CallCore *> (core.get ("call-core"));
}


GMManager::~GMManager ()
{
  ClearAllCalls (OpalConnection::EndedByLocalUser, TRUE);
  RemoveAccountsEndpoint ();
  RemoveSTUNClient ();

  delete bridge;
}


bool GMManager::dial (const std::string uri)
{
  PString token;
  std::stringstream ustr;

  if (uri.find ("sip:") == 0 
      || uri.find ("h323:") == 0 
      || uri.find (":") == string::npos) {

    if (uri.find (":") == string::npos)
      ustr << "sip:" << uri;
    else
      ustr << uri;

    new dialer (ustr.str (), *this);

    return true;
  }

  return false;
}

void GMManager::set_fullname (const std::string name)
{
  SetDefaultDisplayName (name.c_str ());

  sipEP->SetDefaultDisplayName (name.c_str ());
  h323EP->SetDefaultDisplayName (name.c_str ());
  h323EP->SetLocalUserName (name.c_str ());
}

const std::string GMManager::get_fullname () const
{
  return (const char*) GetDefaultDisplayName ();
}

void GMManager::set_jitter_buffer_size (unsigned min_val,
                                        unsigned max_val)
{
  // Adjust general settings
  SetAudioJitterDelay (PMAX (min_val, 20), PMIN (max_val, 1000));
  
  // Adjust setting for all sessions of all connections of all calls
  for (PSafePtr<OpalCall> call = activeCalls;
       call != NULL;
       ++call) {

    for (int i = 0; 
         i < 2;
         i++) {

      PSafePtr<OpalRTPConnection> connection = PSafePtrCast<OpalConnection, OpalRTPConnection> (call->GetConnection (i));
      if (connection) {

        RTP_Session *session = 
          connection->GetSession (OpalMediaFormat::DefaultAudioSessionID);

        if (session != NULL) {

          unsigned units = session->GetJitterTimeUnits ();
          session->SetJitterBufferSize (min_val * units, 
                                        max_val * units, 
                                        units);
        }
      }
    }
  }
}

void GMManager::get_jitter_buffer_size (unsigned & min_val,
                                        unsigned & max_val)
{
  min_val = GetMinAudioJitterDelay (); 
  max_val = GetMaxAudioJitterDelay (); 
}

void GMManager::set_echo_cancelation (bool enabled)
{
  OpalEchoCanceler::Params ec;
  
  // General settings
  ec = GetEchoCancelParams ();
  if (enabled)
    ec.m_mode = OpalEchoCanceler::Cancelation;
  else
    ec.m_mode = OpalEchoCanceler::NoCancelation;
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
}

bool GMManager::get_echo_cancelation ()
{
  OpalEchoCanceler::Params ec = GetEchoCancelParams ();

  return (ec.m_mode == OpalEchoCanceler::Cancelation); 
}

void GMManager::set_silence_detection (bool enabled)
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
}

bool GMManager::get_silence_detection ()
{
  OpalSilenceDetector::Params sd;

  sd = GetSilenceDetectParams ();

  return (sd.m_mode != OpalSilenceDetector::NoSilenceDetection);
}

void GMManager::set_port_ranges (unsigned min_udp_port, 
                                 unsigned max_udp_port,
                                 unsigned min_tcp_port, 
                                 unsigned max_tcp_port)
{
  SetTCPPorts (min_tcp_port, max_tcp_port);
  SetRtpIpPorts (min_udp_port, max_udp_port);
  SetUDPPorts (min_udp_port, max_udp_port);
}

void GMManager::get_port_ranges (unsigned & min_udp_port, 
                                 unsigned & max_udp_port,
                                 unsigned & min_tcp_port, 
                                 unsigned & max_tcp_port)
{
  min_udp_port = GetUDPPortBase ();
  max_udp_port = GetUDPPortMax ();

  min_tcp_port = GetTCPPortBase ();
  max_tcp_port = GetTCPPortMax ();
}

void GMManager::set_video_options (const GMManager::VideoOptions & options)
{
  OpalMediaFormatList media_formats_list;
  OpalMediaFormat::GetAllRegisteredMediaFormats (media_formats_list);

  // Configure all mediaOptions of all Video MediaFormats
  for (int i = 0 ; i < media_formats_list.GetSize () ; i++) {

    OpalMediaFormat media_format = media_formats_list [i];
    if (media_format.GetDefaultSessionID () == OpalMediaFormat::DefaultVideoSessionID) {

      media_format.SetOptionInteger (OpalVideoFormat::FrameWidthOption (), 
                                     Ekiga::VideoSizes [options.size].width);  
      media_format.SetOptionInteger (OpalVideoFormat::FrameHeightOption (), 
                                     Ekiga::VideoSizes [options.size].height);  
      media_format.SetOptionInteger (OpalVideoFormat::FrameTimeOption (),
                                     (int) (90000 / options.maximum_frame_rate));
      media_format.SetOptionInteger (OpalVideoFormat::MaxBitRateOption (), 
                                     options.maximum_received_bitrate * 1000);
      media_format.SetOptionInteger (OpalVideoFormat::TargetBitRateOption (), 
                                     options.maximum_transmitted_bitrate * 1000);
      media_format.SetOptionInteger (OpalVideoFormat::MinRxFrameWidthOption(), 
                                     160);
      media_format.SetOptionInteger (OpalVideoFormat::MinRxFrameHeightOption(), 
                                     120);
      media_format.SetOptionInteger (OpalVideoFormat::MaxRxFrameWidthOption(), 
                                     1920);
      media_format.SetOptionInteger (OpalVideoFormat::MaxRxFrameHeightOption(), 
                                     1088);
      media_format.AddOption(new OpalMediaOptionUnsigned (OpalVideoFormat::TemporalSpatialTradeOffOption (), 
                                                          true, OpalMediaOption::NoMerge, 
                                                          options.temporal_spatial_tradeoff));  
      media_format.SetOptionInteger (OpalVideoFormat::TemporalSpatialTradeOffOption(), 
                                     options.temporal_spatial_tradeoff);  
      media_format.AddOption(new OpalMediaOptionUnsigned (OpalVideoFormat::MaxFrameSizeOption (), 
                                                          true, OpalMediaOption::NoMerge, 1400));
      media_format.SetOptionInteger (OpalVideoFormat::MaxFrameSizeOption (), 
                                     1400);  

      if ( media_format.GetName() != "YUV420P" &&
           media_format.GetName() != "RGB32" &&
           media_format.GetName() != "RGB24") {

        media_format.SetOptionBoolean (OpalVideoFormat::RateControlEnableOption(),
                                      true);
        media_format.SetOptionInteger (OpalVideoFormat::RateControlWindowSizeOption(),
                                      500);
        media_format.SetOptionInteger (OpalVideoFormat::RateControlMaxFramesSkipOption(),
                                      1);
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

      PSafePtr<OpalConnection> connection = call->GetConnection (i);
      if (connection) {

        OpalMediaStream *stream = 
          connection->GetMediaStream (OpalMediaFormat::DefaultVideoSessionID, false); 

        if (stream != NULL) {

          OpalMediaFormat mediaFormat = stream->GetMediaFormat ();
          mediaFormat.SetOptionInteger (OpalVideoFormat::TemporalSpatialTradeOffOption(), 
                                        options.temporal_spatial_tradeoff);  
          mediaFormat.SetOptionInteger (OpalVideoFormat::TargetBitRateOption (), 
                                        options.maximum_transmitted_bitrate * 1000);
          stream->UpdateMediaFormat (mediaFormat);
        }
      }
    }
  }
}


void GMManager::get_video_options (GMManager::VideoOptions & options)
{
  OpalMediaFormatList media_formats_list;
  OpalMediaFormat::GetAllRegisteredMediaFormats (media_formats_list);

  for (int i = 0 ; i < media_formats_list.GetSize () ; i++) {

    OpalMediaFormat media_format = media_formats_list [i];
    if (media_format.GetDefaultSessionID () == OpalMediaFormat::DefaultVideoSessionID) {

      int j = 0;
      for (j = 0; j < NB_VIDEO_SIZES; j++) {

        if (Ekiga::VideoSizes [j].width == media_format.GetOptionInteger (OpalVideoFormat::FrameWidthOption ())
            && Ekiga::VideoSizes [j].width == media_format.GetOptionInteger (OpalVideoFormat::FrameWidthOption ()))
          break;
      }
      options.size = j;

      options.maximum_frame_rate = 
        (int) (90000 / media_format.GetOptionInteger (OpalVideoFormat::FrameTimeOption ()));
      options.maximum_received_bitrate = 
        (int) (media_format.GetOptionInteger (OpalVideoFormat::MaxBitRateOption ()) / 1000);
      options.maximum_transmitted_bitrate = 
        (int) (media_format.GetOptionInteger (OpalVideoFormat::TargetBitRateOption ()) / 1000);
      options.temporal_spatial_tradeoff = 
        media_format.GetOptionInteger (OpalVideoFormat::TemporalSpatialTradeOffOption ());

      break;
    }
  }
}


Ekiga::CodecList GMManager::get_codecs ()
{
  return codecs;
}


void GMManager::set_codecs (Ekiga::CodecList & _codecs)
{
  PStringArray initial_order;
  PStringArray initial_mask;

  OpalMediaFormatList all_media_formats;
  OpalMediaFormatList media_formats;

  PStringArray order;
  PStringArray mask;

  // What do we support
  GetAllowedFormats (all_media_formats);
  Ekiga::CodecList all_codecs = Opal::CodecList (all_media_formats);

  // 
  // Clean the CodecList given as paramenter : remove unsupported codecs and 
  // add missing codecs at the end of the list
  //

  // Build the Ekiga::CodecList taken into account by the GMManager
  // It contains codecs given as argument to set_codecs, and other codecs
  // supported by the manager
  for (Ekiga::CodecList::iterator it = all_codecs.begin ();
       it != all_codecs.end ();
       it++) {

    Ekiga::CodecList::iterator i  = 
      search_n (_codecs.begin (), _codecs.end (), 1, *it, same_codec_desc);
    if (i == _codecs.end ()) {
      _codecs.push_back (*it);
    }
  }

  // Remove unsupported codecs
  for (Ekiga::CodecList::iterator it = _codecs.begin ();
       it != _codecs.end ();
       it++) {

    Ekiga::CodecList::iterator i  = 
      search_n (all_codecs.begin (), all_codecs.end (), 1, *it, same_codec_desc);
    if (i == _codecs.end ())
      _codecs.erase (it);
  }
  codecs = _codecs;


  // 
  // Update OPAL
  //
  Ekiga::CodecList::iterator codecs_it;
  for (codecs_it = codecs.begin () ;
       codecs_it != codecs.end () ;
       codecs_it++) {

    bool active = (*codecs_it).active;
    std::string name = (*codecs_it).name;
    unsigned rate = (*codecs_it).rate;
    int j = 0;

    // Find the OpalMediaFormat corresponding to the Ekiga::CodecDescription
    if (active) {
      for (j = 0 ; 
           j < all_media_formats.GetSize () ;
           j++) {

        if (name == (const char *) all_media_formats [j].GetEncodingName ()
            && rate == all_media_formats [j].GetClockRate ()) {

          // Found something
          order += all_media_formats [j];
        }
      }
    }
  }

  // Build the mask
  all_media_formats = OpalTranscoder::GetPossibleFormats (pcssEP->GetMediaFormats ());
  all_media_formats.Remove (order);

  for (int i = 0 ; 
       i < all_media_formats.GetSize () ; 
       i++)
    mask += all_media_formats [i];

  // Update the OpalManager
  SetMediaFormatMask (mask);
  SetMediaFormatOrder (order);
}


GMH323Endpoint *
GMManager::GetH323Endpoint ()
{
  return h323EP;
}


GMSIPEndpoint *
GMManager::GetSIPEndpoint ()
{
  return sipEP;
}


void
GMManager::Register (GmAccount *account)
{
  PWaitAndSignal m(manager_access_mutex);

  if (manager == NULL)
    manager = new GMAccountsEndpoint (*this);

  if (account != NULL)
    manager->RegisterAccount (account);
}


void
GMManager::RemoveAccountsEndpoint ()
{
  PWaitAndSignal m(manager_access_mutex);

  if (manager)
    delete manager;
  manager = NULL;
}


void 
GMManager::CreateSTUNClient (bool display_progress,
                             bool display_config_dialog,
                             bool wait,
                             GtkWidget *parent)
{
  PWaitAndSignal m(sc_mutex);

  if (sc) 
    delete (sc);

  SetSTUNServer (PString ());

  /* Be a client for the specified STUN Server */
  sc = new GMStunClient (display_progress, 
                         display_config_dialog, 
                         wait,
                         parent,
                         *this);
}


void 
GMManager::RemoveSTUNClient ()
{
  PWaitAndSignal m(sc_mutex);

  if (sc) 
    delete (sc);

  SetSTUNServer (PString ());

  sc = NULL;
}


OpalCall *GMManager::CreateCall ()
{
  Ekiga::Call *call = NULL;

  call = new Opal::Call (*this, core);

  return dynamic_cast<OpalCall *> (call);
}


void
GMManager::OnClosedMediaStream (const OpalMediaStream & stream)
{
  OpalMediaFormatList list = pcssEP->GetMediaFormats ();
  OpalManager::OnClosedMediaStream (stream);

  if (list.FindFormat(stream.GetMediaFormat()) != list.end ())
    dynamic_cast <Opal::Call &> (stream.GetConnection ().GetCall ()).OnClosedMediaStream ((OpalMediaStream &) stream);
}


bool 
GMManager::OnOpenMediaStream (OpalConnection & connection,
                              OpalMediaStream & stream)
{
  OpalMediaFormatList list = pcssEP->GetMediaFormats ();
  if (!OpalManager::OnOpenMediaStream (connection, stream))
    return FALSE;

  if (list.FindFormat(stream.GetMediaFormat()) == list.end ()) 
    dynamic_cast <Opal::Call &> (connection.GetCall ()).OnOpenMediaStream (stream);

  return TRUE;
}


void GMManager::GetAllowedFormats (OpalMediaFormatList & full_list)
{
  OpalMediaFormatList list = OpalTranscoder::GetPossibleFormats (pcssEP->GetMediaFormats ());
  std::list<std::string> black_list;
   
  black_list.push_back ("GSM-AMR");
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

  // Purge blacklisted codecs
  for (PINDEX i = 0 ; i < list.GetSize () ; i++) {

    std::list<std::string>::iterator it = find (black_list.begin (), black_list.end (), (const char *) list [i]);
    if (it == black_list.end ()) 
      full_list += list [i];
  }
}

