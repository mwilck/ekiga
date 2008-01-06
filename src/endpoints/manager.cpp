
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
#include "opal-call.h"
#include "opal-codec-description.h"

#include <opal/transcoders.h>
#include <ptclib/http.h>
#include <ptclib/html.h>
#include <ptclib/pstun.h>

#define new PNEW


extern "C" {
  unsigned char linear2ulaw(int pcm_val);
  int ulaw2linear(unsigned char u_val);
};



/* DESCRIPTION  :  This notifier is called when the firstname or last name
 *                 keys changes.
 * BEHAVIOR     :  Updates the ZeroConf registrations and the internal i
 *                 configuration. 
 * PRE          :  data is a pointer to the GMManager.
 */
static void fullname_changed_nt (gpointer id,
				 GmConfEntry *entry,
				 gpointer data);


/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with the enable_video key changes.
 * BEHAVIOR     :  It updates the endpoint.
 * PRE          :  data is a pointer to the GMManager.
 */
static void enable_video_changed_nt (G_GNUC_UNUSED gpointer id,
                                     GmConfEntry *entry,
                                     gpointer data);


/* DESCRIPTION  :  This callback is called when a silence detection key of
 *                 the config database associated with a toggle changes.
 * BEHAVIOR     :  Update silence detection.
 * PRE          :  data is a pointer to the GMManager.
 */
static void silence_detection_changed_nt (G_GNUC_UNUSED gpointer id,
                                          GmConfEntry *entry, 
                                          gpointer data);


/* DESCRIPTION  :  This callback is called when the echo cancelation key of
 *                 the config database associated with a toggle changes.
 * BEHAVIOR     :  Update echo cancelation.
 * PRE          :  data is a pointer to the GMManager.
 */
static void echo_cancelation_changed_nt (G_GNUC_UNUSED gpointer id,
                                         GmConfEntry *entry, 
                                         gpointer data);


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


/* DESCRIPTION  :  This callback is called when the jitter buffer needs to be 
 *                 changed.
 * BEHAVIOR     :  Update the jitter.
 * PRE          :  data is a pointer to the GMManager.
 */
static void jitter_buffer_changed_nt (G_GNUC_UNUSED gpointer id,
                                      GmConfEntry *entry, 
                                      gpointer data);


/* DESCRIPTION  :  This callback is called when the video device changes
 *                 in the config database.
 * BEHAVIOR     :  It creates a new video grabber if preview is active with
 *                 the selected video device.
 *                 If preview is not enabled, then the potentially existing
 *                 video grabber is deleted provided we are not in
 *                 a call.
 *                 Notice that the video device can't be changed during calls,
 *                 but its setting can be changed.
 * PRE          :  data is a pointer to the GMManager.
 */
static void video_device_changed_nt (G_GNUC_UNUSED gpointer id,
                                     GmConfEntry *entry, 
                                     gpointer data);


/* DESCRIPTION  :  This callback is called when one of the video media format
 * 		   settings changes.
 * BEHAVIOR     :  It updates the media format settings.
 * PRE          :  data is a pointer to the GMManager.
 */
static void video_option_changed_nt (G_GNUC_UNUSED gpointer id,
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


static void from_gslist_to_codec_list (const GSList *codecs_config, 
                                       Ekiga::CodecList & config_codecs)
{
  GSList *codecs_config_it = NULL;
  
  codecs_config_it = (GSList *) codecs_config;
  while (codecs_config_it) {


    Ekiga::CodecDescription d = Ekiga::CodecDescription ((char *) codecs_config_it->data);
    if (!d.name.empty ())
      config_codecs.push_back (d);

    codecs_config_it = g_slist_next (codecs_config_it);
  }
}


static void from_media_formats_to_codec_list (OpalMediaFormatList & full_list, Ekiga::CodecList & codecs)
{
  for (PINDEX i = 0 ; i < full_list.GetSize () ; i++) {

    if (full_list [i].IsTransportable ()) {

      Ekiga::CodecDescription desc = Opal::CodecDescription (full_list [i]);

      Ekiga::CodecList::iterator it = 
        search_n (codecs.begin (), codecs.end (), 1, desc, same_codec_desc);
      if (it == codecs.end ()) 
        codecs.push_back (desc);
      else {
        it->protocols.sort ();
        it->protocols.merge (desc.protocols);
        it->protocols.unique ();
      }
    }
  }
}


static void 
fullname_changed_nt (G_GNUC_UNUSED gpointer id,
		     GmConfEntry *entry, 
		     gpointer data)
{
  GMManager *endpoint = (GMManager *) data;
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING) {

    endpoint->SetUserNameAndAlias ();
    endpoint->UpdatePublishers ();
  }
}


static void
enable_video_changed_nt (G_GNUC_UNUSED gpointer id,
			 GmConfEntry *entry,
			 gpointer data)
{
  PString name;
  GMManager *ep = (GMManager *) data;

  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    ep->SetAutoStartTransmitVideo (gm_conf_entry_get_bool (entry));
    ep->SetAutoStartReceiveVideo (gm_conf_entry_get_bool (entry));
  }
}


static void 
silence_detection_changed_nt (G_GNUC_UNUSED gpointer id,
                              GmConfEntry *entry, 
                              gpointer data)
{
  GMManager *ep = (GMManager *) data;

  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    ep->set_silence_detection (gm_conf_entry_get_bool (entry));
  }
}


static void 
echo_cancelation_changed_nt (G_GNUC_UNUSED gpointer id,
			     GmConfEntry *entry, 
			     gpointer data)
{
  GMManager *ep = (GMManager *) data;

  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    ep->set_echo_cancelation (gm_conf_entry_get_bool (entry));
  }
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


static void 
audio_codecs_list_changed_nt (G_GNUC_UNUSED gpointer id,
                        GmConfEntry *entry, 
                        gpointer data)
{
  GMManager *ep = (GMManager *) data;
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_LIST) {

    Ekiga::CodecList list; 
    GSList *audio_codecs_config = gm_conf_entry_get_list (entry);
    GSList *video_codecs_config = gm_conf_get_string_list (VIDEO_CODECS_KEY "list");

    from_gslist_to_codec_list (audio_codecs_config, list);
    from_gslist_to_codec_list (video_codecs_config, list);
    
    ep->set_codecs (list);

    g_slist_free (audio_codecs_config);

    g_slist_foreach (video_codecs_config, (GFunc) g_free, NULL);
    g_slist_free (video_codecs_config);
  }
}


static void 
video_codecs_list_changed_nt (G_GNUC_UNUSED gpointer id,
                        GmConfEntry *entry, 
                        gpointer data)
{
  GMManager *ep = (GMManager *) data;
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_LIST) {

    Ekiga::CodecList list; 
    GSList *video_codecs_config = gm_conf_entry_get_list (entry);
    GSList *audio_codecs_config = gm_conf_get_string_list (AUDIO_CODECS_KEY "list");

    from_gslist_to_codec_list (audio_codecs_config, list);
    from_gslist_to_codec_list (video_codecs_config, list);
    
    ep->set_codecs (list);

    g_slist_free (video_codecs_config);

    g_slist_foreach (audio_codecs_config, (GFunc) g_free, NULL);
    g_slist_free (audio_codecs_config);
  }
}


static void 
jitter_buffer_changed_nt (G_GNUC_UNUSED gpointer id,
                          GmConfEntry *entry, 
                          gpointer data)
{
  GMManager *ep = (GMManager *) data;
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    unsigned min_val, max_val = 0;
    ep->get_jitter_buffer_size (min_val, max_val);
    ep->set_jitter_buffer_size (min_val, max_val);
  }
}


static void 
video_device_changed_nt (G_GNUC_UNUSED gpointer id,
			 GmConfEntry *entry, 
			 gpointer data)
{
  GMManager *ep = (GMManager *) data;
  
  if ((gm_conf_entry_get_type (entry) == GM_CONF_BOOL) ||
      (gm_conf_entry_get_type (entry) == GM_CONF_STRING) ||
      (gm_conf_entry_get_type (entry) == GM_CONF_INT)) {

    ep->UpdateDevices ();
  }
}

// TODO use entry values in all notifiers
static void 
video_option_changed_nt (G_GNUC_UNUSED gpointer id,
                         GmConfEntry *entry, 
                         gpointer data)
{
  GMManager *ep = (GMManager *) data;

  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    unsigned a, b, c, d, e = 0;
    ep->get_video_options (a, b, c, d, e);
    ep->set_video_options (a, b, c, d, e);
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
    ep->PublishPresence (status);
  }
}


/* The class */
GMManager::GMManager (Ekiga::ServiceCore & _core)
: core (_core), runtime (*(dynamic_cast<Ekiga::Runtime *> (core.get ("runtime"))))
{
  /* Initialise the endpoint paramaters */
  video_grabber = NULL;
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
  video.deviceName = "Moving logo";
  SetVideoInputDevice (video);

  // Create endpoints
  h323EP = new GMH323Endpoint (*this);
  h323EP->Init ();
  AddRouteEntry("pc:.* = h323:<da>");
	
  sipEP = new GMSIPEndpoint (*this, core);
  AddRouteEntry("pc:.* = sip:<da>");
  
  pcssEP = new GMPCSSEndpoint (*this);
  AddRouteEntry("h323:.* = pc:<da>");
  AddRouteEntry("sip:.* = pc:<da>");
  
  autoStartTransmitVideo = autoStartReceiveVideo = true;

  // Keep a pointer to the runtime

  Init ();

  // Set ports
  unsigned a, b, c, d, e, f = 0;
  get_port_ranges (a, b, c, d, e, f);
  set_port_ranges (a, b, c, d, e, f);

  // Video options
  gm_conf_notifier_add (VIDEO_CODECS_KEY "maximum_video_tx_bitrate", 
			video_option_changed_nt, this);
  gm_conf_notifier_add (VIDEO_CODECS_KEY "temporal_spatial_tradeoff",
			video_option_changed_nt, this);
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "size", 
                        video_option_changed_nt, this);
  gm_conf_notifier_add (VIDEO_CODECS_KEY "max_frame_rate",
                        video_option_changed_nt, this);
  gm_conf_notifier_add (VIDEO_CODECS_KEY "maximum_video_rx_bitrate",
                        video_option_changed_nt, this);
  gm_conf_notifier_trigger (VIDEO_CODECS_KEY "maximum_video_rx_bitrate");
  
  // Set Codecs from the Configuration
  detect_codecs ();
  gm_conf_notifier_add (AUDIO_CODECS_KEY "list",
			audio_codecs_list_changed_nt, this);
  gm_conf_notifier_trigger (AUDIO_CODECS_KEY "list"); 

  gm_conf_notifier_add (VIDEO_CODECS_KEY "list",
			video_codecs_list_changed_nt, this);
  gm_conf_notifier_trigger (VIDEO_CODECS_KEY "list"); 

  // The jitter
  gm_conf_notifier_add (AUDIO_CODECS_KEY "minimum_jitter_buffer", 
			jitter_buffer_changed_nt, this);
  gm_conf_notifier_add (AUDIO_CODECS_KEY "maximum_jitter_buffer", 
			jitter_buffer_changed_nt, this);
  gm_conf_notifier_trigger (AUDIO_CODECS_KEY "maximum_jitter_buffer"); 

  // Audio codecs settings
  gm_conf_notifier_add (AUDIO_CODECS_KEY "enable_silence_detection", 
			silence_detection_changed_nt, this);
  gm_conf_notifier_trigger (AUDIO_CODECS_KEY "enable_silence_detection");

  gm_conf_notifier_add (AUDIO_CODECS_KEY "enable_echo_cancelation", 
			echo_cancelation_changed_nt, this);
  gm_conf_notifier_trigger (AUDIO_CODECS_KEY "enable_silence_detection");
}


GMManager::~GMManager ()
{
  Exit ();
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

      PSafePtr<OpalConnection> connection = call->GetConnection (i);
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
  return gm_conf_get_bool (AUDIO_CODECS_KEY "enable_echo_cancelation");
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


void GMManager::set_video_options (unsigned size,
                                   unsigned max_frame_rate,
                                   unsigned temporal_spatial_tradeoff,
                                   unsigned maximum_video_rx_bitrate,
                                   unsigned maximum_video_tx_bitrate)
{
  OpalMediaFormatList media_formats_list;
  OpalMediaFormat::GetAllRegisteredMediaFormats (media_formats_list);

  // Configure all mediaOptions of all Video MediaFormats
  for (int i = 0 ; i < media_formats_list.GetSize () ; i++) {

    OpalMediaFormat media_format = media_formats_list [i];
    if (media_format.GetDefaultSessionID () == OpalMediaFormat::DefaultVideoSessionID) {

      media_format.SetOptionInteger (OpalVideoFormat::FrameWidthOption (), 
                                     video_sizes [size].width);  
      media_format.SetOptionInteger (OpalVideoFormat::FrameHeightOption (), 
                                     video_sizes[ size].height);  
      media_format.SetOptionInteger (OpalVideoFormat::FrameTimeOption (),
                                     (int) (90000 / max_frame_rate));
      media_format.SetOptionInteger (OpalVideoFormat::MaxBitRateOption (), 
                                     maximum_video_rx_bitrate * 1000);
      media_format.SetOptionInteger (OpalVideoFormat::TargetBitRateOption (), 
                                     maximum_video_tx_bitrate * 1000);
      media_format.SetOptionInteger (OpalVideoFormat::MinRxFrameWidthOption(), 
                                     160);
      media_format.SetOptionInteger (OpalVideoFormat::MinRxFrameHeightOption(), 
                                     120);
      media_format.SetOptionInteger (OpalVideoFormat::MaxRxFrameWidthOption(), 
                                     1920);
      media_format.SetOptionInteger (OpalVideoFormat::MaxRxFrameHeightOption(), 
                                     1088);
      media_format.AddOption(new OpalMediaOptionUnsigned (OpalVideoFormat::TemporalSpatialTradeOffOption (), 
                                                          true, OpalMediaOption::NoMerge, temporal_spatial_tradeoff));  
      media_format.AddOption(new OpalMediaOptionUnsigned (OpalVideoFormat::MaxFrameSizeOption (), 
                                                          true, OpalMediaOption::NoMerge, 1400));

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
          mediaFormat.SetOptionInteger (OpalVideoFormat::TemporalSpatialTradeOffOption() , temporal_spatial_tradeoff);  
          mediaFormat.SetOptionInteger (OpalVideoFormat::TargetBitRateOption (), maximum_video_tx_bitrate * 1000);
          stream->UpdateMediaFormat (mediaFormat);
        }
      }
    }
  }
}


void GMManager::get_video_options (unsigned & size,
                                   unsigned & max_frame_rate,
                                   unsigned & temporal_spatial_tradeoff,
                                   unsigned & maximum_video_rx_bitrate,
                                   unsigned & maximum_video_tx_bitrate)
{
  max_frame_rate = gm_conf_get_int (VIDEO_CODECS_KEY "max_frame_rate");
  temporal_spatial_tradeoff = gm_conf_get_int (VIDEO_CODECS_KEY "temporal_spatial_tradeoff");
  maximum_video_rx_bitrate = gm_conf_get_int (VIDEO_CODECS_KEY "maximum_video_rx_bitrate");
  maximum_video_tx_bitrate = gm_conf_get_int (VIDEO_CODECS_KEY "maximum_video_tx_bitrate");
  size = gm_conf_get_int (VIDEO_DEVICES_KEY "size");
}


void
GMManager::Exit ()
{
  ClearAllCalls (OpalConnection::EndedByLocalUser, TRUE);

#ifdef HAVE_AVAHI
  RemoveZeroconfClient ();
#endif

  RemoveAccountsEndpoint ();

  RemoveVideoGrabber ();

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


void GMManager::UpdateDevices ()
{
  bool preview = FALSE;
  gchar *device_name = NULL;
  unsigned size = 0;
  unsigned max_frame_rate = 15;

  /* Get the config settings */
  gnomemeeting_threads_enter ();
  preview = gm_conf_get_bool (VIDEO_DEVICES_KEY "enable_preview");
  device_name = gm_conf_get_string (VIDEO_DEVICES_KEY "input_device");
  size = gm_conf_get_int (VIDEO_DEVICES_KEY "size");
  max_frame_rate = gm_conf_get_int (VIDEO_CODECS_KEY "max_frame_rate");
  gnomemeeting_threads_leave ();

  /* Do not change these values during calls */
  if (GetCallingState () == GMManager::Standby) {

    /* Video preview */
    if (preview) {
      GMVideoGrabber *vg = GetVideoGrabber ();
      if (vg) {
        vg->StopGrabbing (); 
        vg->Unlock ();
      }
      PThread::Sleep (1000);
      CreateVideoGrabber (TRUE, TRUE, video_sizes[size].width, video_sizes[size].height, max_frame_rate);
    }
    else 
      RemoveVideoGrabber ();

    /* Update the video input device */
    PVideoDevice::OpenArgs video = GetVideoInputDevice();
    video.deviceName = device_name;
    SetVideoInputDevice (video);
  }

  g_free (device_name);
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
  GSList *codecs_config = NULL;

  std::map<std::string, Ekiga::CodecDescription> codecs;
  Ekiga::CodecList all_codecs;
  Ekiga::CodecList config_codecs;

  OpalMediaFormatList full_list;

  // Build the CodecList from the available OpalMediaFormats
  GetAllowedFormats (full_list);
  from_media_formats_to_codec_list (full_list, all_codecs);

  // Build the CodecList from the configuration
  codecs_config = gm_conf_get_string_list (AUDIO_CODECS_KEY "list");
  from_gslist_to_codec_list (codecs_config, config_codecs);
  g_slist_foreach (codecs_config, (GFunc) g_free, NULL);
  g_slist_free (codecs_config);
  codecs_config = gm_conf_get_string_list (VIDEO_CODECS_KEY "list");
  from_gslist_to_codec_list (codecs_config, config_codecs);
  g_slist_foreach (codecs_config, (GFunc) g_free, NULL);
  g_slist_free (codecs_config);

  // Finally build the CodecList taken into account by the GMManager
  // It contains codecs from the configuration and other disabled codecs
  for (Ekiga::CodecList::iterator it = all_codecs.begin ();
       it != all_codecs.end ();
       it++) {

    Ekiga::CodecList::iterator i  = search_n (config_codecs.begin (), config_codecs.end (), 1, *it, same_codec_desc);
    if (i == config_codecs.end ()) {
      config_codecs.push_back (*it);
    }
  }

  // Remove unsupported codecs
  for (Ekiga::CodecList::iterator it = config_codecs.begin ();
       it != config_codecs.end ();
       it++) {

    Ekiga::CodecList::iterator i  = search_n (all_codecs.begin (), all_codecs.end (), 1, *it, same_codec_desc);
    if (i == config_codecs.end ())
      config_codecs.erase (it);
  }

  return config_codecs;
}


void GMManager::set_codecs (Ekiga::CodecList codecs)
{
  PStringArray initial_order;
  PStringArray initial_mask;

  OpalMediaFormatList all_media_formats;
  OpalMediaFormatList media_formats;

  PStringArray order;
  PStringArray mask;

  GetAllowedFormats (all_media_formats);

  // Build order
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


GMVideoGrabber *
GMManager::CreateVideoGrabber (bool start_grabbing,
                               bool synchronous,
			       unsigned width,
			       unsigned height,
			       unsigned rate)
{
  PWaitAndSignal m(vg_access_mutex);

  if (video_grabber)
    delete (video_grabber);

  video_grabber = new GMVideoGrabber (start_grabbing, synchronous, width, height, rate, *this);

  return video_grabber;
}


void
GMManager::RemoveVideoGrabber ()
{
  PWaitAndSignal m(vg_access_mutex);

  PTRACE(4, "GMManager\t Deleting grabber device");  //FIXME: There seems to be a problem in win32 since 2.0.x here
  if (video_grabber) {

    delete (video_grabber);
  }      
  PTRACE(4, "GMManager\t Sucessfully deleted grabber device");  //FIXME: There seems to be a problem in win32 since 2.0.x here
  video_grabber = NULL;
}


GMVideoGrabber *
GMManager::GetVideoGrabber ()
{
  PWaitAndSignal m(vg_access_mutex);

  if (video_grabber)
    video_grabber->Lock ();
  
  return video_grabber;
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
GMManager::PublishPresence (guint status)
{
  PWaitAndSignal m(manager_access_mutex);

  if (manager == NULL)
    manager = new GMAccountsEndpoint (*this);

  manager->PublishPresence (status);
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
                                 PString extra)
{
  bool res = FALSE;

  /* Act on the connection */
  switch (reason) {

  case 1:
    connection.ClearCall (OpalConnection::EndedByLocalBusy);
    res = FALSE;
    break;
    
  case 2:
    connection.ForwardCall (extra);
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
  
  /* Asterisk sometimes forgets to send an INVITE, HACK */
  audio_session = connection.GetSession (OpalMediaFormat::DefaultAudioSessionID);
  video_session = connection.GetSession (OpalMediaFormat::DefaultVideoSessionID);
  if (audio_session) {
    audio_session->SetIgnoreOtherSources (TRUE);
    audio_session->SetIgnorePayloadTypeChanges (TRUE);
  }
  
  if (video_session) {
    video_session->SetIgnoreOtherSources (TRUE);
    video_session->SetIgnorePayloadTypeChanges (TRUE);
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

  /* Try to update the devices use if some settings were changed 
     during the call */
  UpdateDevices ();
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
GMManager::SetUserNameAndAlias ()
{
  gchar *firstname = NULL;
  gchar *lastname = NULL;
  gchar *local_name = NULL;
  
  
  gnomemeeting_threads_enter ();
  firstname = gm_conf_get_string (PERSONAL_DATA_KEY "firstname");
  lastname = gm_conf_get_string (PERSONAL_DATA_KEY "lastname");
  gnomemeeting_threads_leave ();

  
  local_name = gnomemeeting_create_fullname (firstname, lastname);
  if (local_name)
    SetDefaultDisplayName (local_name);

  
  /* Update the H.323 endpoint user name and alias */
  h323EP->SetUserNameAndAlias ();

  
  /* Update the SIP endpoint user name and alias */
  sipEP->SetUserNameAndAlias ();


  g_free (local_name);
  g_free (firstname);
  g_free (lastname);
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
  
  /* Update general devices configuration */
  UpdateDevices ();
  
  /* Set the User Name and Alias */  
  SetUserNameAndAlias ();

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
  gm_conf_notifier_add (PERSONAL_DATA_KEY "firstname",
			fullname_changed_nt, this);
  gm_conf_notifier_add (PERSONAL_DATA_KEY "lastname",
			fullname_changed_nt, this);
  gm_conf_notifier_add (VIDEO_CODECS_KEY "enable_video", 
			enable_video_changed_nt, this);

  gm_conf_notifier_add (PROTOCOLS_KEY "interface",
			network_interface_changed_nt, this);
  gm_conf_notifier_add (NAT_KEY "public_ip",
			public_ip_changed_nt, this);
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "input_device", 
			video_device_changed_nt, this);
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "channel", 
			video_device_changed_nt, this);
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "size", 
			video_device_changed_nt, this);
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "format", 
			video_device_changed_nt, this);
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "image", 
			video_device_changed_nt, this);
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "enable_preview", 
			video_device_changed_nt, this);

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

  bool success = FALSE;

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
  OpalManager::OnClosedMediaStream (stream);

  if (pcssEP->GetMediaFormats ().FindFormat(stream.GetMediaFormat()) == P_MAX_INDEX)
    dynamic_cast <Opal::Call &> (stream.GetConnection ().GetCall ()).OnClosedMediaStream ((OpalMediaStream &) stream);
}


bool 
GMManager::OnOpenMediaStream (OpalConnection & connection,
                              OpalMediaStream & stream)
{
  if (!OpalManager::OnOpenMediaStream (connection, stream))
    return FALSE;

  if (pcssEP->GetMediaFormats ().FindFormat(stream.GetMediaFormat()) == P_MAX_INDEX) 
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


bool
GMManager::CreateVideoInputDevice (G_GNUC_UNUSED const OpalConnection &con,
				   const OpalMediaFormat &format,
				   PVideoInputDevice * & device,
				   bool & auto_delete)
{
  GMVideoGrabber *vg = NULL;
  auto_delete = FALSE;

  unsigned width  = format.GetOptionInteger(OpalVideoFormat::FrameWidthOption (),  PVideoFrameInfo::QCIFWidth);
  unsigned height = format.GetOptionInteger(OpalVideoFormat::FrameHeightOption (), PVideoFrameInfo::QCIFHeight);
  unsigned rate   = round ((double)format.GetClockRate() / (double)format.GetFrameTime());

  PTRACE(4, "GMManager\tCreating grabber with " << width << "x" << height << "/" << rate);
  vg = GetVideoGrabber ();
  if (!vg) {
    CreateVideoGrabber (FALSE, TRUE, width, height, rate);
    vg = GetVideoGrabber ();
  } 
  else {
    if ((vg->GetWidth()     != width)  ||
        (vg->GetHeight()    != height) ||
        (vg->GetFrameRate() != rate)) {
      // We have negotiated a different resolution than the 
      // preview device was configure with
      CreateVideoGrabber (FALSE, TRUE, width, height, rate);
      vg = GetVideoGrabber ();
    }
  }

  vg->StopGrabbing ();
  device = vg->GetInputDevice ();
  vg->Unlock ();
  
  return (device != NULL);
}


bool 
GMManager::CreateVideoOutputDevice(G_GNUC_UNUSED const OpalConnection & connection,
				   const OpalMediaFormat & format,
				   bool preview,
				   PVideoOutputDevice * & device,
				   bool & auto_delete)
{
  const PVideoDevice::OpenArgs & args = videoOutputDevice;

  /* Display of the input video, the display already
   * has the same size as the grabber 
   */
  if (preview) {

    GMVideoGrabber *vg = NULL;
    auto_delete = FALSE;

    vg = GetVideoGrabber ();

    if (vg) {

      device = vg->GetOutputDevice ();
      vg->Unlock ();
    }
    return (device != NULL);
  }
  else { /* Display of the video we are receiving */
    
    auto_delete = TRUE;
    device = PVideoOutputDevice::CreateDeviceByName (args.deviceName);
    
    if (device != NULL) {
      
      videoOutputDevice.width = 
	format.GetOptionInteger(OpalVideoFormat::FrameWidthOption (),  PVideoFrameInfo::QCIFWidth);
      videoOutputDevice.height = 
	format.GetOptionInteger(OpalVideoFormat::FrameHeightOption (), PVideoFrameInfo::QCIFHeight);
      videoOutputDevice.rate = format.GetClockRate() / format.GetFrameTime();

      if (device->OpenFull (args, FALSE))
	return TRUE;

      delete device;
    }
  }

  return FALSE;
}


void
GMManager::SendDTMF (PString callToken,
		      PString dtmf)
{
  PSafePtr <OpalCall> call = NULL;
  PSafePtr <OpalConnection> connection = NULL;

  call = FindCallWithLock (callToken);

  if (call != NULL) {
    
    connection = GetConnection (call, TRUE);

    if (connection != NULL) 
      connection->SendUserInputTone(dtmf [0], 180);
  }
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


void GMManager::detect_codecs ()
{
  Ekiga::CodecList codecs = get_codecs ();
  GSList *audio_codecs_list = NULL;
  GSList *video_codecs_list = NULL;

  for (Ekiga::CodecList::iterator it = codecs.begin ();
       it != codecs.end ();
       it++) {

    if ((*it).audio)
      audio_codecs_list = g_slist_append (audio_codecs_list, g_strdup ((*it).str ().c_str ()));
    else
      video_codecs_list = g_slist_append (video_codecs_list, g_strdup ((*it).str ().c_str ()));
  }

  gm_conf_set_string_list (AUDIO_CODECS_KEY "list", audio_codecs_list);
  g_slist_foreach (audio_codecs_list, (GFunc) g_free, NULL);
  g_slist_free (audio_codecs_list);

  gm_conf_set_string_list (VIDEO_CODECS_KEY "list", video_codecs_list);
  g_slist_foreach (video_codecs_list, (GFunc) g_free, NULL);
  g_slist_free (video_codecs_list);
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
  runtime.run_in_main (sigc::bind (registration_event.make_slot (), 
                                    std::string ((const char *) aor), 
                                    wasRegistering ? Registered : Unregistered,
                                    std::string ()));
}


void
GMManager::OnRegistering (const PString & aor,
                         G_GNUC_UNUSED bool isRegistering)
{
  runtime.run_in_main (sigc::bind (registration_event.make_slot (), 
                                    std::string ((const char *) aor), 
                                    Processing,
                                    std::string ()));
}


void
GMManager::OnRegistrationFailed (const PString & aor,
                                 bool wasRegistering,
                                 std::string info)
{
  runtime.run_in_main (sigc::bind (registration_event.make_slot (), 
                                   std::string ((const char *) aor), 
                                   wasRegistering ? RegistrationFailed : UnregistrationFailed,
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
