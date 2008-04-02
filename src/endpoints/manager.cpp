
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
 *                         endpoint.cpp  -  description
 *                         ----------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the Endpoint class.
 *
 */


#include "config.h"

#include "manager.h"
#include "pcss.h"
#include "h323.h"
#include "sip.h"

#include "urlhandler.h"

#include "ekiga.h"
#include "misc.h"
#include "main.h"

#include "gmconf.h"

#include "call-core.h"
#include "opal-gmconf-bridge.h"
#include "opal-call.h"
#include "opal-codec-description.h"

#include "vidinput-info.h"

#include <opal/transcoders.h>
#include <ptclib/http.h>
#include <ptclib/html.h>
#include <ptclib/pstun.h>

#include <math.h>

#define new PNEW


extern "C" {
  unsigned char linear2ulaw(int pcm_val);
  int ulaw2linear(unsigned char u_val);
};



/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with the listening interface changes.
 * BEHAVIOR     :  Updates the interface.
 * PRE          :  data is a pointer to the GMManager.
 */
static void network_interface_changed_nt (G_GNUC_UNUSED gpointer id,
                                          GmConfEntry *entry, 
                                          gpointer data);



/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with the public ip changes.
 * BEHAVIOR     :  Updates the IP Translation address.
 * PRE          :  data is a pointer to the GMManager.
 */
static void public_ip_changed_nt (G_GNUC_UNUSED gpointer id,
                                  GmConfEntry *entry, 
                                  gpointer data);



/* DESCRIPTION  :  This callback is called when the status config value changes.
 * BEHAVIOR     :  Updates the presence for the endpoints.
 * PRE          :  /
 */
static void status_changed_nt (G_GNUC_UNUSED gpointer id,
                               GmConfEntry *entry,
                               gpointer data);


static  bool same_codec_desc (Ekiga::CodecDescription a, Ekiga::CodecDescription b)
{ 
  return (a.name == b.name && a.rate == b.rate); 
}


static void 
network_interface_changed_nt (G_GNUC_UNUSED gpointer id,
                              GmConfEntry *entry, 
                              gpointer data)
{
  GMManager *ep = (GMManager *) data;
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING) {

    gdk_threads_enter ();
    ep->ResetListeners ();
    gdk_threads_leave ();
  }
}


static void 
public_ip_changed_nt (G_GNUC_UNUSED gpointer id,
		      GmConfEntry *entry, 
		      gpointer data)
{
  GMManager *ep = (GMManager *) data;

  const char *public_ip = NULL;
  int nat_method = 0;
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING) {
    
    gdk_threads_enter ();
    public_ip = gm_conf_entry_get_string (entry);
    nat_method = gm_conf_get_int (NAT_KEY "method");
    gdk_threads_leave ();

    if (nat_method == 2 && public_ip)
      ep->SetTranslationAddress (PString (public_ip));
    else
      ep->SetTranslationAddress (PString ("0.0.0.0"));
  }
}


/* DESCRIPTION  :  This callback is called when the status config value changes.
 * BEHAVIOR     :  Updates the presence for the endpoints.
 * PRE          :  /
 */
static void
status_changed_nt (G_GNUC_UNUSED gpointer id,
                   GmConfEntry *entry,
                   gpointer data)
{
  GMManager *ep = (GMManager *) data;

  guint status = CONTACT_ONLINE;

  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    status = gm_conf_entry_get_int (entry);
    
    ep->UpdatePublishers ();
  }
}


/* The class */
GMManager::GMManager (Ekiga::ServiceCore & _core)
: core (_core), runtime (*(dynamic_cast<Ekiga::Runtime *> (core.get ("runtime"))))
{
  /* Initialise the endpoint paramaters */
  SetCallingState (GMManager::Standby);

#ifdef HAVE_AVAHI
  zcp = NULL;
#endif

  gk = NULL;
  sc = NULL;

  PIPSocket::SetDefaultIpAddressFamilyV4();
  
  manager = NULL;

  GatewayIPTimer.SetNotifier (PCREATE_NOTIFIER (OnGatewayIPTimeout));
  GatewayIPTimer.RunContinuous (PTimeInterval (5));

  IPChangedTimer.SetNotifier (PCREATE_NOTIFIER (OnIPChanged));
  IPChangedTimer.RunContinuous (120000);

  h323EP = NULL;
  sipEP = NULL;
  pcssEP = NULL;

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
  h323EP = new GMH323Endpoint (*this);
  AddRouteEntry("pc:.* = h323:<da>");
	
  sipEP = new GMSIPEndpoint (*this, core);
  AddRouteEntry("pc:.* = sip:<da>");
  
  pcssEP = new GMPCSSEndpoint (*this);
  AddRouteEntry("h323:.* = pc:<db>");
  AddRouteEntry("sip:.* = pc:<db>");
  
  autoStartTransmitVideo = autoStartReceiveVideo = true;

  // Keep a pointer to the runtime

  Init ();

  // Set ports
  unsigned a, b, c, d, e, f = 0;
  get_port_ranges (a, b, c, d, e, f);
  set_port_ranges (a, b, c, d, e, f);

  // Config
  bridge = new Opal::ConfBridge (*this);

  //
  call_core = dynamic_cast<Ekiga::CallCore *> (core.get ("call-core"));
}


GMManager::~GMManager ()
{
  Exit ();

  delete bridge;
}


bool GMManager::dial (const std::string uri)
{
  if (uri.find ("sip:") == 0 || uri.find ("h323:") == 0 || uri.find (":") == string::npos) {

    new GMURLHandler (core, uri.c_str ());
    return true;
  }

  return false;
}


bool GMManager::send_message (const std::string uri, const std::string message)
{
  if (uri.find ("sip:") == 0 || uri.find (":") == string::npos) {

    if (!uri.empty () && !message.empty ())
      sipEP->Message (uri.c_str (), message.c_str ());

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


bool
GMManager::populate_menu (Ekiga::Contact &contact,
                          Ekiga::MenuBuilder &builder)
{
  std::string name = contact.get_name ();
  std::map<std::string, std::string> uris = contact.get_uris ();

  return menu_builder_add_actions (name, uris, builder);
}


bool 
GMManager::populate_menu (const std::string uri,
                          Ekiga::MenuBuilder & builder)
{
  std::map<std::string, std::string> uris; 
  uris [""] = uri;

  return menu_builder_add_actions ("", uris, builder);
}


bool 
GMManager::menu_builder_add_actions (const std::string & fullname,
                                     std::map<std::string,std::string> & uris,
                                     Ekiga::MenuBuilder & builder)
{
  bool populated = false;

  /* Add actions of type "call" for all uris */
  for (std::map<std::string, std::string>::const_iterator iter = uris.begin ();
       iter != uris.end ();
       iter++) {

    std::string action = _("Call");

    if (!iter->first.empty ())
      action = action + " [" + iter->first + "]";

    builder.add_action ("call", action, sigc::bind (sigc::mem_fun (this, &GMManager::on_dial), iter->second));

    populated = true;
  }

  /* Add actions of type "message" for all uris */
  for (std::map<std::string, std::string>::const_iterator iter = uris.begin ();
       iter != uris.end ();
       iter++) {

    std::string action = _("Message");

    if (!iter->first.empty ())
      action = action + " [" + iter->first + "]";

    builder.add_action ("message", action, sigc::bind (sigc::mem_fun (this, &GMManager::on_message), fullname, iter->second));

    populated = true;
  }

  return populated;
}


void GMManager::get_jitter_buffer_size (unsigned & min_val,
                                        unsigned & max_val)
{
  min_val = gm_conf_get_int (AUDIO_CODECS_KEY "minimum_jitter_buffer");
  max_val = gm_conf_get_int (AUDIO_CODECS_KEY "maximum_jitter_buffer");
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


bool GMManager::get_echo_cancelation ()
{
  return gm_conf_get_bool (AUDIO_CODECS_KEY "enable_silence_detection");
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


bool GMManager::get_silence_detection ()
{
  OpalSilenceDetector::Params sd;

  sd = GetSilenceDetectParams ();

  return (sd.m_mode != OpalSilenceDetector::NoSilenceDetection);
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


void GMManager::get_port_ranges (unsigned & min_udp_port, 
                                 unsigned & max_udp_port,
                                 unsigned & min_tcp_port, 
                                 unsigned & max_tcp_port,
                                 unsigned & min_rtp_port, 
                                 unsigned & max_rtp_port)
{
  std::string key [3] = { 
    PORTS_KEY "udp_port_range",
    PORTS_KEY "rtp_port_range",
    PORTS_KEY "tcp_port_range"
  };

  gchar *port_range = NULL;
  gchar **couple = NULL;

  for (int i = 0;
       i < 3;
       i++) {

    port_range = gm_conf_get_string (key [i].c_str ());
    if (port_range) {

      couple = g_strsplit (port_range, ":", 2);

      if (i == 0) {

        min_udp_port = atoi (couple [0]);
        max_udp_port = atoi (couple [1]);
      }
      else if (i == 1) {

        min_tcp_port = atoi (couple [0]);
        max_tcp_port = atoi (couple [1]);
      }
      else {

        min_rtp_port = atoi (couple [0]);
        max_rtp_port = atoi (couple [1]);
      }
    }

    g_free (port_range);
    g_strfreev (couple);
    port_range = NULL;
    couple = NULL;
  }
}


void GMManager::set_port_ranges (unsigned min_udp_port, 
                                 unsigned max_udp_port,
                                 unsigned min_tcp_port, 
                                 unsigned max_tcp_port,
                                 unsigned min_rtp_port, 
                                 unsigned max_rtp_port)
{
  SetTCPPorts (min_tcp_port, max_tcp_port);
  SetRtpIpPorts (min_rtp_port, max_rtp_port);
  SetUDPPorts (min_udp_port, max_udp_port);
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


void
GMManager::Exit ()
{
  ClearAllCalls (OpalConnection::EndedByLocalUser, TRUE);

#ifdef HAVE_AVAHI
  RemoveZeroconfClient ();
#endif

  RemoveAccountsEndpoint ();

  RemoveSTUNClient ();
}


bool
GMManager::AcceptCurrentIncomingCall ()
{
  if (pcssEP) {

    pcssEP->AcceptCurrentIncomingCall ();
    return TRUE;
  }

  return FALSE;
}


void 
GMManager::SetCallingState (CallingState i)
{
  PWaitAndSignal m(cs_access_mutex);

  calling_state = i;
}


GMManager::CallingState
GMManager::GetCallingState ()
{
  PWaitAndSignal m(cs_access_mutex);

  return calling_state;
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


void
GMManager::SetUserInputMode ()
{
  h323EP->SetUserInputMode ();
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


GMPCSSEndpoint *
GMManager::GetPCSSEndpoint ()
{
  return pcssEP;
}


PString
GMManager::GetCurrentAddress (PString protocol)
{
  OpalEndPoint *ep = NULL;

  PIPSocket::Address ip(PIPSocket::GetDefaultIpAny());
  WORD port = 0;

  ep = FindEndPoint (protocol.IsEmpty () ? "sip" : protocol);
  
  if (!ep)
    return PString ();
  
  if (!ep->GetListeners ().IsEmpty ())
    ep->GetListeners()[0].GetLocalAddress().GetIpAndPort (ip, port);
      
  return ip.AsString () + ":" + PString (port);
}


PString
GMManager::GetURL (PString protocol)
{
  GmAccount *account = NULL;
  PString url;
  gchar *account_url = NULL;

  if (protocol.IsEmpty ())
    return PString ();

  account = gnomemeeting_get_default_account ((gchar *)(const char *) protocol);

  if (account) {

    if (account->enabled)
      account_url = g_strdup_printf ("%s:%s@%s", (const char *) protocol, 
				     account->username, account->host);
    gm_account_delete (account);
  }

  if (!account_url)
    account_url = g_strdup_printf ("%s:%s", (const char *) protocol,
				   (const char *) GetCurrentAddress (protocol));

  url = account_url;
  g_free (account_url);

  return url;
}


void
GMManager::UpdatePublishers (void)
{
#ifdef HAVE_AVAHI
  PWaitAndSignal m(zcp_access_mutex);
  if (zcp)  
    zcp->Publish ();
#endif
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
GMManager::SetCurrentCallToken (PString s)
{
  PWaitAndSignal m(ct_access_mutex);

  current_call_token = s;
}


PString 
GMManager::GetCurrentCallToken ()
{
  PWaitAndSignal m(ct_access_mutex);

  return current_call_token;
}


#ifdef HAVE_AVAHI
void 
GMManager::CreateZeroconfClient ()
{
  PWaitAndSignal m(zcp_access_mutex);

  if (zcp)
    delete zcp;

  zcp = new GMZeroconfPublisher (*this);
}


void 
GMManager::RemoveZeroconfClient ()
{
  PWaitAndSignal m(zcp_access_mutex);

  if (zcp) {

    delete zcp;
    zcp = NULL;
  }
}
#endif

			     
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

  ResetListeners ();
}



PSafePtr<OpalConnection> GMManager::GetConnection (PSafePtr<OpalCall> call, 
						    bool is_remote)
{
  PSafePtr<OpalConnection> connection = NULL;

  if (call == NULL)
    return connection;

  connection = call->GetConnection (is_remote ? 1:0);
  /* is_remote => SIP or H.323 connection
   * !is_remote => PCSS Connection
   */
  if (!connection
      || (is_remote && PIsDescendant(&(*connection), OpalPCSSConnection))
      || (!is_remote && !PIsDescendant(&(*connection), OpalPCSSConnection))) 
    connection = call->GetConnection (is_remote ? 0:1);

  return connection;
}


OpalCall *GMManager::CreateCall ()
{
  Ekiga::Call *call = NULL;

  call = new Opal::Call (*this, core);

  runtime.run_in_main (sigc::bind (new_call, call));

  return dynamic_cast<OpalCall *> (call);
}


bool
GMManager::OnIncomingConnection (OpalConnection &connection,
                                 unsigned reason,
                                 const std::string & forward_uri)
{
  bool res = FALSE;

  /* Act on the connection */
  switch (reason) {

  case 1:
    connection.ClearCall (OpalConnection::EndedByLocalBusy);
    res = FALSE;
    break;
    
  case 2:
    connection.ForwardCall (forward_uri);
    res = FALSE;
    break;
    
  case 4:
    res = TRUE;
  default:

  case 0:
    res = OpalManager::OnIncomingConnection (connection, 0, NULL);
    break;
  }
  
  /* Update the current state if action is 0 or 4.
   * Show popup if action is 1 (show popup)
   */
  if (reason == 0 || reason == 4) {
    
    SetCallingState (GMManager::Called);
    SetCurrentCallToken (connection.GetCall ().GetToken ());
  }

  return res;
}


void 
GMManager::OnEstablishedCall (OpalCall &call)
{
  /* Update internal state */
  SetCallingState (GMManager::Connected);
  SetCurrentCallToken (call.GetToken ());
}


void 
GMManager::OnEstablished (OpalConnection &connection)
{
  RTP_Session *audio_session = NULL;
  RTP_Session *video_session = NULL;

  /* Do nothing for the PCSS connection */
  if (PIsDescendant(&connection, OpalPCSSConnection)) {
    
    PTRACE (3, "GMManager\t Will establish the connection");
    OpalManager::OnEstablished (connection);
    return;
  }

  if (PIsDescendant(&connection, OpalRTPConnection)) {

    audio_session = PDownCast (OpalRTPConnection, &connection)->GetSession (OpalMediaFormat::DefaultAudioSessionID);
    video_session = PDownCast (OpalRTPConnection, &connection)->GetSession (OpalMediaFormat::DefaultVideoSessionID);
    if (audio_session) {
      audio_session->SetIgnorePayloadTypeChanges (TRUE);
    }

    if (video_session) {
      video_session->SetIgnorePayloadTypeChanges (TRUE);
    }
  }
  
  PTRACE (3, "GMManager\t Will establish the connection");
  OpalManager::OnEstablished (connection);
}


void 
GMManager::OnClearedCall (OpalCall & call)
{
  if (GetCurrentCallToken() != PString::Empty() 
      && GetCurrentCallToken () != call.GetToken())
    return;
  
  /* Play busy tone if we were connected */
  if (GetCallingState () == GMManager::Connected)
    pcssEP->PlaySoundEvent ("busy_tone_sound"); 

  /* Update internal state */
  SetCallingState (GMManager::Standby);
  SetCurrentCallToken ("");

  /* Reinitialize codecs */
  re_audio_codec = tr_audio_codec = re_video_codec = tr_video_codec = "";
}


void 
GMManager::OnMessageReceived (const SIPURL & _from,
                              const PString & _body)
{
  SIPURL from = _from;
  std::string display_name = (const char *) from.GetDisplayName ();
  from.AdjustForRequestURI ();
  std::string uri = (const char *) from.AsString ();
  std::string message = (const char *) _body;

  pcssEP->PlaySoundEvent ("new_message_sound"); // FIXME use signals here too

  runtime.run_in_main (sigc::bind (im_received.make_slot (), display_name, uri, message));
}


void 
GMManager::OnMessageFailed (const SIPURL & _to,
                            G_GNUC_UNUSED SIP_PDU::StatusCodes reason)
{
  SIPURL to = _to;
  to.AdjustForRequestURI ();
  std::string uri = (const char *) to.AsString ();
  runtime.run_in_main (sigc::bind (im_failed.make_slot (), uri, 
                                   _("Could not send message")));
}


void 
GMManager::OnMessageSent (const PString & _to,
                          const PString & body)
{
  SIPURL to = _to;
  to.AdjustForRequestURI ();
  std::string uri = (const char *) to.AsString ();
  std::string message = (const char *) body;
  runtime.run_in_main (sigc::bind (im_sent.make_slot (), uri, message));
}


void
GMManager::Init ()
{
  OpalMediaFormatList list;
  
  int nat_method = 0;
  
  gchar *ip = NULL;
  
  /* GmConf cache */
  gnomemeeting_threads_enter ();
  nat_method = gm_conf_get_int (NAT_KEY "method");
  ip = gm_conf_get_string (NAT_KEY "public_ip");
  gnomemeeting_threads_leave ();

  /* Set Up IP translation */
  if (nat_method == 2 && ip)
    SetTranslationAddress (PString (ip));
  else
    SetTranslationAddress (PString ("0.0.0.0")); 
  
  /* Create a Zeroconf client */
#ifdef HAVE_AVAHI
  CreateZeroconfClient ();
#endif

  /* Update publishers */
  UpdatePublishers ();

  /* Set initial codecs */
  SetMediaFormatOrder (PStringArray ());
  SetMediaFormatMask (PStringArray ());

  /* Reset the listeners */
  ResetListeners ();

  g_free (ip);
  
  /* GMConf notifiers for what we manager */
  gm_conf_notifier_add (PROTOCOLS_KEY "interface",
			network_interface_changed_nt, this);
  gm_conf_notifier_add (NAT_KEY "public_ip",
			public_ip_changed_nt, this);
  gm_conf_notifier_add (PERSONAL_DATA_KEY "status",
			status_changed_nt, this);
}


void
GMManager::ResetListeners ()
{
  gchar *iface = NULL;
  gchar *ports = NULL;
  gchar **couple = NULL;

  WORD port = 0;
  WORD min_port = 5060;
  WORD max_port = 5080;

  gnomemeeting_threads_enter ();
  iface = gm_conf_get_string (PROTOCOLS_KEY "interface");
  gnomemeeting_threads_leave ();
  
  /* Create the H.323 and SIP listeners */
  if (h323EP) {
  
    gnomemeeting_threads_enter ();
    port = gm_conf_get_int (H323_KEY "listen_port");
    gnomemeeting_threads_leave ();
    
    h323EP->RemoveListener (NULL);
    if (!h323EP->StartListener (iface, port)) 
      PTRACE (1, "Manager\tCould not start H.323 listener on " << iface << ":" << port);
  }

  if (sipEP) {
    
    gnomemeeting_threads_enter ();
    port = gm_conf_get_int (SIP_KEY "listen_port");
    ports = gm_conf_get_string (PORTS_KEY "udp_port_range");
    if (ports)
      couple = g_strsplit (ports, ":", 2);
    if (couple && couple [0]) {
      min_port = atoi (couple [0]);
    }
    if (couple && couple [1]) {
      max_port = atoi (couple [1]);
    }
    gnomemeeting_threads_leave ();
    
    sipEP->RemoveListener (NULL);
    if (!sipEP->StartListener (iface, port)) {
      
      bool success = false;
      port = min_port;
      while (port <= max_port && !success) {
       
        success = sipEP->StartListener (iface, port);
        port++;
      }

      if (!success) 
        PTRACE (1, "Manager\tCould not start SIP listener on " << iface << ":" << min_port << "-" << max_port);
    }

    g_strfreev (couple);
    g_free (ports);
  }

  g_free (iface);
}


void
GMManager::OnIPChanged (PTimer &,
			INT)
{
  PIPSocket::InterfaceTable ifaces;
  OpalTransportAddress current_address;
  PIPSocket::Address current_ip;
  
  PString ip;
  PINDEX i = 0;
  
  bool found_ip = FALSE;
  
  gchar *iface = NULL;

  gnomemeeting_threads_enter ();
  iface = gm_conf_get_string (PROTOCOLS_KEY "interface");
  gnomemeeting_threads_leave ();

  /* Detect the valid interfaces */
  PIPSocket::GetInterfaceTable (ifaces);
  if (ifaces.GetSize () <= 0) {
  
    g_free (iface);
    return;
  }
  
  /* Is there a listener? */
  if (sipEP->GetListeners ().GetSize () > 0) 
    current_address = sipEP->GetListeners ()[0].GetLocalAddress ();
  else if (h323EP->GetListeners ().GetSize () > 0) 
    current_address = h323EP->GetListeners ()[0].GetLocalAddress ();
  else {

    g_free (iface);
    return;
  }
    
  /* Are we listening on the correct interface? */
  if (current_address.GetIpAddress (current_ip)) {

    while (i < ifaces.GetSize () && !found_ip) {

      ip = ifaces [i].GetAddress ().AsString ();

      if (ip == current_ip.AsString ())
	found_ip = TRUE;

      i++;
    }
  }

  if (!found_ip) {

    PTRACE (4, "Ekiga\tIP Address changed, updating listeners.");

    if (ifaces.GetSize () > 1) {
      
      /* This will update the UI and the listeners through a GConf
       * Notifier.
       */
      gnomemeeting_threads_enter ();
      GnomeMeeting::Process ()->DetectInterfaces ();
      gnomemeeting_threads_leave ();
    }
  }

  g_free (iface);
}


bool
GMManager::SetDeviceVolume (PSoundChannel *sound_channel,
			     bool is_encoding,
			     unsigned int vol)
{
  return DeviceVolume (sound_channel, is_encoding, TRUE, vol);
}


bool
GMManager::GetDeviceVolume (PSoundChannel *sound_channel,
                                 bool is_encoding,
                                 unsigned int &vol)
{
  return DeviceVolume (sound_channel, is_encoding, FALSE, vol);
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


void GMManager::OnHold (OpalConnection & connection)
{
  dynamic_cast <Opal::Call &> (connection.GetCall ()).OnHold (connection.IsConnectionOnHold ());
}


void 
GMManager::OnGatewayIPTimeout (PTimer &,
				    INT)
{
  PHTTPClient web_client ("Ekiga");
  PString html, ip_address;
  gboolean ip_checking = false;

  web_client.SetReadTimeout (PTimeInterval (0, 5));
  gdk_threads_enter ();
  ip_checking = gm_conf_get_bool (NAT_KEY "enable_ip_checking");
  gchar *ip_detector = gm_conf_get_string (NAT_KEY "public_ip_detector");
  gdk_threads_leave ();

  if (ip_detector != NULL
      && ip_checking
      && web_client.GetTextDocument (ip_detector, html)) {

    if (!html.IsEmpty ()) {

      PRegularExpression regex ("[0-9]*[.][0-9]*[.][0-9]*[.][0-9]*");
      PINDEX pos, len;

      if (html.FindRegEx (regex, pos, len)) 
	ip_address = html.Mid (pos,len);

    }
  }
  g_free (ip_detector);
  if (!ip_address.IsEmpty () && ip_checking) {

    gdk_threads_enter ();
    gm_conf_set_string (NAT_KEY "public_ip",
			(gchar *) (const char *) ip_address);
    gdk_threads_leave ();
  }

  GatewayIPTimer.RunContinuous (PTimeInterval (0, 0, 15));
}


bool 
GMManager::DeviceVolume (PSoundChannel *sound_channel,
			 G_GNUC_UNUSED bool is_encoding,
			 bool set,
			 unsigned int &vol)
{
  bool err = TRUE;

  if (sound_channel && GetCallingState () == GMManager::Connected) {

    
    if (set) {

      err = 
	sound_channel->SetVolume (vol)
	&& err;
    }
    else {

      err = 
	sound_channel->GetVolume (vol)
	&& err;
    }
  }

  return err;
}

void
GMManager::on_dial (std::string uri)
{
  dial (uri);
}


void
GMManager::on_message (std::string name,
                       std::string uri)
{
  runtime.run_in_main (sigc::bind (new_chat.make_slot (), name, uri));
}


void
GMManager::OnMWIReceived (const PString & account,
                          const PString & mwi)
{
  PINDEX i = 0;
  PINDEX j = 0;
  int total = 0;

  PString key;
  PString val;
  PString *value = NULL;
  
  PWaitAndSignal m(mwi_access_mutex);

  /* Add MWI information for given account */
  value = mwiData.GetAt (key);

  /* Something changed for that account */
  if (value && *value != mwi) {
    mwiData.SetAt (account, new PString (mwi));

    /* Compute the total number of new voice mails */
    while (i < mwiData.GetSize ()) {

      value = NULL;
      key = mwiData.GetKeyAt (i);
      value = mwiData.GetAt (key);
      if (value) {

        val = *value;
        j = val.Find ("/");
        if (j != P_MAX_INDEX)
          val = value->Left (j);
        else 
          val = *value;

        total += val.AsInteger ();
      }
      i++;
    }

    runtime.run_in_main (sigc::bind (mwi_event.make_slot (), 
                                      (const char *) account, 
                                      (const char *) mwi,
                                      total));

    /* Sound event if new voice mail */
    pcssEP->PlaySoundEvent ("new_voicemail_sound");
  }
}


void
GMManager::OnRegistered (const PString & aor,
                         bool wasRegistering)
{
  if (call_core)
    runtime.run_in_main (sigc::bind (call_core->registration_event.make_slot (), 
                                     std::string ((const char *) aor), 
                                     wasRegistering ? Ekiga::CallCore::Registered : Ekiga::CallCore::Unregistered,
                                     std::string ()));
}


void
GMManager::OnRegistering (const PString & aor,
                         G_GNUC_UNUSED bool isRegistering)
{
  if (call_core)
    runtime.run_in_main (sigc::bind (call_core->registration_event.make_slot (), 
                                     std::string ((const char *) aor), 
                                     Ekiga::CallCore::Processing,
                                     std::string ()));
}


void
GMManager::OnRegistrationFailed (const PString & aor,
                                 bool wasRegistering,
                                 std::string info)
{
  if (call_core)
    runtime.run_in_main (sigc::bind (call_core->registration_event.make_slot (), 
                                     std::string ((const char *) aor), 
                                     wasRegistering ? Ekiga::CallCore::RegistrationFailed : Ekiga::CallCore::UnregistrationFailed,
                                     info));
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
