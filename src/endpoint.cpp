 
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2002 Damien Sandras
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 *                         endpoint.cpp  -  description
 *                         ----------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains miscellaneous functions.
 *   email                : dsandras@seconix.com
 *
 */

#include "../config.h"

#include "toolbar.h"
#include "endpoint.h"
#include "gnomemeeting.h"
#include "dialog.h"
#include "common.h"
#include "connection.h"
#include "tray.h"
#include "sound_handling.h"
#include "videograbber.h"
#include "gatekeeper.h"
#include "ils.h"
#include "misc.h"
#include "menu.h"
#include "main_window.h"
#include "lid.h"

#include <gconf/gconf-client.h>
#include <g726codec.h>
#include <gsmcodec.h>
#include <mscodecs.h>
#include <h261codec.h>
#include <videoio.h>
#include <gtk/gtk.h>
#include <lpc10codec.h>

#ifdef SPEEX_CODEC
#include <speexcodec.h>
#endif
#ifndef DISABLE_GNOME
#include <gnome.h>
#endif


#define new PNEW


/* Declarations */
static gint IncomingCallTimeout (gpointer);

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;


/* The Timer */
static gint 
IncomingCallTimeout (gpointer data) 
{
  GmWindow *gw = NULL;
  H323Connection *connection = NULL;
  gchar *msg = NULL;
  gchar *forward_host_gconf = NULL;
  PString forward_host;
  gboolean no_answer_forward = FALSE;
  GConfClient *client = NULL;

  client = gconf_client_get_default ();

  gdk_threads_enter ();

  /* Forwarding on no answer */
  no_answer_forward = 
    gconf_client_get_bool (client, CALL_FORWARDING_KEY "no_answer_forward", 0);
  forward_host_gconf = 
    gconf_client_get_string (client, CALL_FORWARDING_KEY "forward_host", 0);

  if (forward_host_gconf)
    forward_host = PString (forward_host_gconf);
  else
    forward_host = PString ("");

  gw = gnomemeeting_get_main_window (gm);

  /* Destroy the incoming call popup */
  if (gw->incoming_call_popup) {

    gtk_widget_destroy (gw->incoming_call_popup);
    gw->incoming_call_popup = NULL;
  }

  /* If forward host specified and forward requested */
  if ((!forward_host.IsEmpty ())&&(no_answer_forward)) {

    connection = MyApp->Endpoint ()->GetCurrentConnection ();
    if (connection) {

      connection->ForwardCall (PString ((const char *) forward_host));
      msg = g_strdup_printf (_("Forwarding Call to %s (No Answer)"), 
			     (const char *) forward_host);
      gnomemeeting_log_insert (gw->history_text_view, msg);

      gnomemeeting_statusbar_push (gw->statusbar, _("Call forwarded"));
      gnomemeeting_log_insert (gw->calls_history_text_view, 
 			       _("Call forwarded"));
      g_free (msg);
    }
  }
  else {

    if (MyApp->Endpoint ()->GetCallingState () == 3) {

      MyApp->Disconnect (H323Connection::EndedByNoAnswer);
    }
  }
  g_free (forward_host_gconf);

  gdk_threads_leave ();

  return FALSE;
}


/* The class */
GMH323EndPoint::GMH323EndPoint ()
{
  gchar *msg = NULL;
  gchar *udp_port_range = NULL;
  gchar *tcp_port_range = NULL;
  gchar **udp_couple = NULL;
  gchar **tcp_couple = NULL;

  /* Get the GTK structures and GConf client */
  gw = gnomemeeting_get_main_window (gm);
  lw = gnomemeeting_get_ldap_window (gm);
  chat = gnomemeeting_get_chat_window (gm);
  client = gconf_client_get_default ();

  /* Initialise the endpoint paramaters */
  SetCurrentConnection (NULL);
  SetCallingState (0);
  
#ifdef HAS_IXJ
  lid_thread = NULL;
#endif

  video_grabber = NULL;
  listener = NULL;
  snapshot_number = 0;

  docklet_timeout = 0;
  sound_timeout = 0;

  opened_audio_channels = 0;
  opened_video_channels = 0;

  /* Start the ILSClient PThread */
  ils_client = new GMILSClient ();

  /* Start the video grabber thread */
  video_grabber = new GMVideoGrabber ();

  udp_port_range = 
    gconf_client_get_string (client, GENERAL_KEY "udp_port_range", NULL);
  tcp_port_range = 
    gconf_client_get_string (client, GENERAL_KEY "tcp_port_range", NULL);
  
  if (udp_port_range)
    udp_couple = g_strsplit (udp_port_range, ":", 0);
  if (tcp_port_range)
    tcp_couple = g_strsplit (tcp_port_range, ":", 0);
  
  if (tcp_couple [0] && tcp_couple [1]) {

    SetTCPPorts (atoi (tcp_couple [0]), atoi (tcp_couple [1]));
  }

  if (udp_couple [0] && udp_couple [1]) {

    SetRtpIpPorts (atoi (udp_couple [0]), atoi (udp_couple [1]));
  }

  g_free (tcp_port_range);
  g_free (udp_port_range);
  g_strfreev (tcp_couple);
  g_strfreev (udp_couple);

  clearCallOnRoundTripFail = TRUE;  

  received_video_device = NULL;
  transmitted_video_device = NULL;
  player_channel = NULL;
  recorder_channel = NULL;
  audio_tester = NULL;


  /* We can add these capabilities here as they will exist
     the whole life of the EP */
  H323_UserInputCapability::AddAllCapabilities(capabilities, 0, P_MAX_INDEX);

  /* Update general configuration */
  UpdateConfig ();

  /* GM is started */
  msg = g_strdup_printf (_("Started GnomeMeeting V%d.%d for %s\n"), 
			 MAJOR_VERSION, MINOR_VERSION, g_get_user_name ());
  gnomemeeting_log_insert (gw->history_text_view, msg);
  g_free (msg);
}


GMH323EndPoint::~GMH323EndPoint ()
{
  quit_mutex.Wait ();

  delete (ils_client);

  var_access_mutex.Wait ();
  ils_client = NULL;
  var_access_mutex.Signal ();

  delete (video_grabber);

  var_access_mutex.Wait ();
  video_grabber = NULL;
  var_access_mutex.Signal ();

#ifdef HAS_IXJ  
  if (lid_thread) 
    delete (lid_thread);
  var_access_mutex.Wait ();
  lid_thread = NULL;
  var_access_mutex.Signal ();
#endif

  quit_mutex.Signal ();
}


void GMH323EndPoint::UpdateConfig ()
{
  /* FIX ME : séparer en plusieurs morceau, virer updateconfig, bouger un 
     max de choses dans le constructeur. Seuls les devices doivent rester
     ici */
  gchar *text = NULL;
  gchar *player = NULL;
  gchar *recorder = NULL;

  int video_size = 0;

  BOOL use_lid = FALSE;
  BOOL h245_tunneling = FALSE;
  BOOL fast_start = FALSE;

  GmPrefWindow *pw = NULL;
  GmWindow *gw = NULL;

  gnomemeeting_threads_enter ();
  pw = gnomemeeting_get_pref_window (gm);
  gw = gnomemeeting_get_main_window (gm);


  /* Get the gconf settings */
  use_lid = 
    gconf_client_get_bool (client, DEVICES_KEY "lid", NULL);
  player = 
    gconf_client_get_string (client, DEVICES_KEY "audio_player", NULL);
  recorder = 
    gconf_client_get_string (client, DEVICES_KEY "audio_recorder", NULL);
  h245_tunneling = 
    gconf_client_get_bool (client, GENERAL_KEY "h245_tunneling", NULL);
  fast_start =
    gconf_client_get_bool (client, GENERAL_KEY "fast_start", NULL);
  video_size = 
    gconf_client_get_int (client, VIDEO_SETTINGS_KEY "video_size", NULL);


  gnomemeeting_sound_daemons_suspend ();

  /* Do not change these values during calls */
  if (GetCallingState () == 0) {

    /**/
    /* Set recording source and set micro to record if no LID is used */
    if (!use_lid) {

      /* Change that setting only if needed */
      if (player && (GetSoundChannelPlayDevice () != PString (player))) { 
	
	/* Player is always a correct sound device, thanks to 
	   gnomemeeting_add_string_option_menu */
	SetSoundChannelPlayDevice (player);
	text = g_strdup_printf (_("Set Audio player device to %s"), 
				(const char *) player);
	gnomemeeting_log_insert (gw->history_text_view, text);
	g_free (text);
      } 


      /* Change that setting only if needed */
      if (recorder && (GetSoundChannelRecordDevice ()!= PString (recorder))) { 

	  SetSoundChannelRecordDevice (recorder);
	  text = g_strdup_printf (_("Set Audio recorder device to %s"), 
				  (const char *) recorder);
	  gnomemeeting_log_insert (gw->history_text_view, text);
	  g_free (text);

      }
    }
    

    /**/
    /* Update the H.245 Tunnelling and Fast Start Settings if needed */
    if (disableH245Tunneling != !h245_tunneling) {

      gnomemeeting_log_insert (gw->history_text_view, 
			       h245_tunneling?
			       _("Enabling H.245 Tunnelling"):
			       _("Disabling H.245 Tunnelling"));
      disableH245Tunneling = !h245_tunneling;
    }

    if (disableFastStart != !fast_start) {

      gnomemeeting_log_insert (gw->history_text_view, 
			       fast_start?
			       _("Enabling Fast Start"):
			       _("Disabling Fast Start"));
      disableFastStart = !fast_start;
    }


#ifdef HAS_IXJ
    /* Use the quicknet card if needed */
    if (use_lid) {

      if (!lid_thread)
	lid_thread = new GMLid ();
    }
    else {
      
      delete (lid_thread);
      lid_thread = NULL;
    }
#endif


    /**/
    /* Update the capabilities */
    RemoveAllCapabilities ();
    AddAudioCapabilities ();
    AddVideoCapabilities (video_size);
  }

  gnomemeeting_sound_daemons_resume ();

  gnomemeeting_threads_leave ();

  g_free (player);
  g_free (recorder);
}


H323Capabilities 
GMH323EndPoint::RemoveCapability (PString name)
{
  capabilities.Remove (name);
  return capabilities;
}


void 
GMH323EndPoint::RemoveAllCapabilities ()
{
  capabilities.RemoveAll ();
}


void 
GMH323EndPoint::SetCallingState (int i)
{
  var_access_mutex.Wait ();
  calling_state = i;
  var_access_mutex.Signal ();
}


int 
GMH323EndPoint::GetCallingState (void)
{
  int cstate;

  var_access_mutex.Wait ();
  cstate = calling_state;
  var_access_mutex.Signal ();

  return cstate;
}


void 
GMH323EndPoint::AddVideoCapabilities (int video_size)
{
  BOOL enable_video_transmission = FALSE;


  /* Add video capabilities */
  if (video_size == 1) {

    /* CIF Capability in first position */
    SetCapability(0, 1, 
		  new H323_H261Capability (0, 2, FALSE, FALSE, 6217));

    SetCapability(0, 1, 
		  new H323_H261Capability (4, 0, FALSE, FALSE, 6217));
  }
  else {
    
    SetCapability(0, 1, 
		  new H323_H261Capability (4, 0, FALSE, FALSE, 6217)); 
    
    SetCapability(0, 1, 
		  new H323_H261Capability (0, 2, FALSE, FALSE, 6217));
  }


  /* Enable video transmission */
  enable_video_transmission = 
    gconf_client_get_bool (client, 
			   VIDEO_SETTINGS_KEY "enable_video_transmission", 0);
  autoStartTransmitVideo = enable_video_transmission;
}


void 
GMH323EndPoint::AddAudioCapabilities ()
{
  gchar **couple;
  GSList *codecs_data = NULL;
  BOOL use_pcm16_codecs = TRUE;
  int g711_frames = 0;
  int gsm_frames = 0;
  MicrosoftGSMAudioCapability* gsm_capa = NULL; 
  H323_G711Capability *g711_capa = NULL; 
  H323_G726_Capability * g72616_capa = NULL; 
  H323_GSM0610Capability *gsm2_capa = NULL; 


  /* Read GConf settings */ 
  codecs_data = 
    gconf_client_get_list (client, AUDIO_CODECS_KEY "codecs_list", 
			   GCONF_VALUE_STRING, NULL);
  g711_frames = 
    gconf_client_get_int (client, AUDIO_SETTINGS_KEY "g711_frames", NULL);
  gsm_frames = 
    gconf_client_get_int (client, AUDIO_SETTINGS_KEY "gsm_frames", NULL);


#ifdef HAS_IXJ
  /* Add the audio capabilities provided by the LID Hardware */
  if (GetLidThread ()) {

    OpalLineInterfaceDevice *lid = NULL;

    lid = lid_thread->GetLidDevice ();

    if (lid && lid->IsOpen ()) {

      H323_LIDCapability::AddAllCapabilities (*lid, capabilities, 0, 0);

      /* If the LID can do PCM16 we can use the software codecs like GSM too */
      use_pcm16_codecs = 
	lid->GetMediaFormats ().GetValuesIndex (OpalMediaFormat(OPAL_PCM16)) 
	!= P_MAX_INDEX;    

      /* If the LID can do PCM16, we remove the G.711 hw codec for now */
      if (use_pcm16_codecs)
	capabilities.Remove ("G.711");
    }
  }
#endif


  /* Let's go */
  while (use_pcm16_codecs && codecs_data) {
    
    couple = g_strsplit ((gchar *) codecs_data->data, "=", 0);

    if (couple [0] && couple [1] != NULL) {

      if (!strcmp (couple [1], "1")) {

#ifdef SPEEX_CODEC
	if (!strcmp (couple [0], "Speex-15k")) 
	    SetCapability (0, 0, new SpeexNarrow5AudioCapability ());
	else if (!strcmp (couple [0], "Speex-8k")) 
	  SetCapability (0, 0, new SpeexNarrow3AudioCapability ());
	else	 
#endif
	if (!strcmp (couple [0], "MS-GSM")) {
	  
	  SetCapability (0, 0, gsm_capa = new MicrosoftGSMAudioCapability);
	  gsm_capa->SetTxFramesInPacket (gsm_frames);
	}
	else if (!strcmp (couple [0], "G.711-uLaw-64k")) {

	  g711_capa = new H323_G711Capability (H323_G711Capability::muLaw);
	  SetCapability (0, 0, g711_capa);
	  g711_capa->SetTxFramesInPacket (g711_frames);
	}
	else if (!strcmp (couple [0], "G.711-ALaw-64k")) {

	  g711_capa = new H323_G711Capability (H323_G711Capability::ALaw);
	  SetCapability (0, 0, g711_capa);
	  g711_capa->SetTxFramesInPacket (g711_frames);
	}
	else if (!strcmp (couple [0], "GSM-06.10")) {
      
	  SetCapability (0, 0, gsm2_capa = new H323_GSM0610Capability);	
	  gsm2_capa->SetTxFramesInPacket (gsm_frames);
	}
	else if (!strcmp (couple [0], "G.726-32k")) {

	  g72616_capa = 
	    new H323_G726_Capability (*this, H323_G726_Capability::e_32k);
	  SetCapability (0, 0, g72616_capa);
	}
	else if (!strcmp (couple [0], "LPC10")) {

	  SetCapability(0, 0, new H323_LPC10Capability (*this));
	}
      }	
    }

    g_strfreev (couple);
    codecs_data = codecs_data->next;
  }

  g_slist_free (codecs_data);
}


char *
GMH323EndPoint::GetCurrentIP ()
{
  PIPSocket::InterfaceTable interfaces;
  PIPSocket::Address ip_addr;

  gchar *ip = NULL;

  if (!PIPSocket::GetInterfaceTable (interfaces)) 

    PIPSocket::GetHostAddress (ip_addr);
  else {

    for (unsigned int i = 0; i < (unsigned int) (interfaces.GetSize()); i++) {

      ip_addr = interfaces[i].GetAddress();

      if (ip_addr != 0  && 
	  ip_addr != PIPSocket::Address()) /* Ignore 127.0.0.1 */
	
	break;  	      
    }
  }

  ip = g_strdup ((const char *) ip_addr.AsString ());

  return ip;
}


void 
GMH323EndPoint::TranslateTCPAddress(PIPSocket::Address &localAddr, 
				    const PIPSocket::Address &remoteAddr)
{
  PIPSocket::Address addr;
  BOOL ip_translation = FALSE;
  gchar *ip = NULL;

  ip_translation = 
    gconf_client_get_bool (client, GENERAL_KEY "ip_translation", NULL);

  if (ip_translation) {

    /* Ignore Ip translation for local networks */
    if ( !((remoteAddr.Byte1() == 192) && (remoteAddr.Byte2() == 168))
	 
	 && !((remoteAddr.Byte1() == 172) 
	      && ((remoteAddr.Byte2() >= 16)&&(remoteAddr.Byte2()<=31)))
	 
	 && !(remoteAddr.Byte1() == 10)) {

      ip = 
	gconf_client_get_string (client, GENERAL_KEY "public_ip", NULL);

      if (ip) {

	addr = PIPSocket::Address(ip);

	if (addr != PIPSocket::Address ("0.0.0.0"))
	  localAddr = addr;
      }
    }
  }
}


BOOL 
GMH323EndPoint::StartListener ()
{
  int listen_port = 1720;


  listen_port = 
    gconf_client_get_int (client, GENERAL_KEY "listen_port", NULL);

  /* Start the listener thread for incoming calls */
  listener = new H323ListenerTCP (*this, INADDR_ANY, listen_port);

  /* unsuccesfull */
  if (!H323EndPoint::StartListener (listener)) {

    delete listener;
    listener = NULL;

    return FALSE;
  }
   
  return TRUE;
}


int 
GMH323EndPoint::GetRecorderVolume ()
{
  unsigned int vol = 0;
  
#ifdef HAS_IXJ
  GMLid *ld = NULL;
  ld = GetLidThread ();

  if (ld)
    ld->GetLidDevice ()->GetRecordVolume (0, vol);
  else
#endif
  if (audio_tester) 
    return ((GM_AUDIO_TESTER (audio_tester))->GetRecorderVolume ());
  else if (recorder_channel) 
    recorder_channel->GetVolume (vol);

  return vol;
}


BOOL 
GMH323EndPoint::SetRecorderVolume (int vol)
{
#ifdef HAS_IXJ
  GMLid *ld = NULL;
  ld = GetLidThread ();

  if (ld)
    ld->GetLidDevice ()->SetRecordVolume (0, vol);
  else
#endif
  if (audio_tester) {

    return ((GM_AUDIO_TESTER (audio_tester))->SetRecorderVolume (vol));
  }
  else if (recorder_channel) {

    recorder_channel->SetVolume (vol);
    return TRUE;
  }
  
  return FALSE;
}


int 
GMH323EndPoint::GetPlayerVolume ()
{
  unsigned int vol = 0;

#ifdef HAS_IXJ
  GMLid *ld = NULL;
  ld = GetLidThread ();

  if (ld)
    ld->GetLidDevice ()->GetPlayVolume (0, vol);
  else
#endif
  if (audio_tester) 
    return ((GM_AUDIO_TESTER (audio_tester))->GetPlayerVolume ());
  else if (player_channel) 
    player_channel->GetVolume (vol);

  return vol;
}


BOOL 
GMH323EndPoint::SetPlayerVolume (int vol)
{
#ifdef HAS_IXJ
  GMLid *ld = NULL;
  ld = GetLidThread ();

  if (ld)
    ld->GetLidDevice ()->SetPlayVolume (0, vol);
  else
#endif
  if (audio_tester) {

    return ((GM_AUDIO_TESTER (audio_tester))->SetPlayerVolume (vol));
  }
  else if (player_channel) {

    player_channel->SetVolume (vol);
    return TRUE;
  }

  return FALSE;
}


void 
GMH323EndPoint::StartAudioTester ()
{
  if (!audio_tester)
    audio_tester = new GMAudioTester (this, GTK_WINDOW (gm));
}


void 
GMH323EndPoint::StopAudioTester ()
{
  if (audio_tester) {
   
    (GM_AUDIO_TESTER (audio_tester))->Stop ();
    audio_tester = NULL;
  }
}


H323Connection *
GMH323EndPoint::CreateConnection (unsigned callReference)
{
  return new GMH323Connection (*this, callReference);
}


H323Connection *
GMH323EndPoint::GetCurrentConnection ()
{
  H323Connection *con = NULL;

  var_access_mutex.Wait ();
  con = current_connection;
  var_access_mutex.Signal ();

  return con;
}


PThread *
GMH323EndPoint::GetVideoGrabberThread (void)
{
  PThread *vg = NULL;

  var_access_mutex.Wait ();
  vg = video_grabber;
  var_access_mutex.Signal ();

  return vg;
}


H323VideoCodec *
GMH323EndPoint::GetCurrentVideoCodec (void)
{
  H323VideoCodec *video_codec = NULL;
  H323Channel *channel = NULL;

  channel = GetCurrentVideoChannel ();

  if (channel != NULL) {
	
    H323Codec *raw_codec  = channel->GetCodec();

    if (raw_codec && raw_codec->IsDescendant (H323VideoCodec::Class())) {
      
      video_codec = (H323VideoCodec *) raw_codec;
    }
  }
    
  return video_codec;
}


H323AudioCodec *
GMH323EndPoint::GetCurrentAudioCodec (void)
{
  H323AudioCodec *audio_codec = NULL;
  H323Channel *channel = NULL;

  channel = GetCurrentAudioChannel ();
  if (channel != NULL) {
	
    H323Codec * raw_codec  = channel->GetCodec();
    
    if (raw_codec && raw_codec->IsDescendant (H323AudioCodec::Class())) {
      
      audio_codec = (H323AudioCodec *) raw_codec;
    }
  }
    
  return audio_codec;
}


H323Channel *
GMH323EndPoint::GetCurrentAudioChannel (void)
{
  H323Channel *channel = NULL;

  if (!GetCurrentCallToken ().IsEmpty()) {
    
    H323Connection *connection = 
      FindConnectionWithLock (GetCurrentCallToken ());
    
    if (connection != NULL) {

      channel = 
	connection->FindChannel (RTP_Session::DefaultAudioSessionID, 
				 FALSE);

      connection->Unlock ();
    }
  } 
  
  return channel;
}


H323Channel *
GMH323EndPoint::GetCurrentVideoChannel (void)
{
  H323Channel *channel = NULL;

  if (!GetCurrentCallToken ().IsEmpty()) {
    
    H323Connection *connection = 
      FindConnectionWithLock (GetCurrentCallToken ());
    
    if (connection != NULL) {

      channel = 
	connection->FindChannel (RTP_Session::DefaultVideoSessionID, 
				 FALSE);

      connection->Unlock ();
    }
  } 
  
  return channel;
}


int
GMH323EndPoint::GetVideoChannelsNumber (void)
{
  return opened_video_channels;
}


PThread *
GMH323EndPoint::GetILSClientThread (void)
{
  PThread *ils = NULL;

  var_access_mutex.Wait ();
  ils = ils_client;
  var_access_mutex.Signal ();

  return ils;
}


void 
GMH323EndPoint::SetCurrentConnection (H323Connection *c)
{
  var_access_mutex.Wait ();
  current_connection = c;
  var_access_mutex.Signal ();
}


void 
GMH323EndPoint::SetCurrentCallToken (PString s)
{
  var_access_mutex.Wait ();
  current_call_token = s;
  var_access_mutex.Signal ();
}


PString 
GMH323EndPoint::GetCurrentCallToken ()
{
  PString c;

  var_access_mutex.Wait ();
  c = current_call_token;
  var_access_mutex.Signal ();

  return c;
}


H323Gatekeeper *
GMH323EndPoint::GetGatekeeper ()
{
  return gatekeeper;
}


void 
GMH323EndPoint::GatekeeperRegister ()
{
  new GMH323Gatekeeper ();
}


BOOL 
GMH323EndPoint::OnConnectionForwarded (H323Connection &,
                                       const PString &forward_party,
                                       const H323SignalPDU &)
{  
  gchar *msg = NULL;
  PString call_token = GetCurrentCallToken ();

  if (MakeCall (forward_party, call_token)) {

    gnomemeeting_threads_enter ();
    msg = g_strdup_printf (_("Forwarding Call to %s"), 
			   (const char*) forward_party);
    gnomemeeting_statusbar_push (gw->statusbar, msg);
    gnomemeeting_log_insert (gw->history_text_view, msg);
    gnomemeeting_log_insert (gw->calls_history_text_view, _("Call forwarded"));
    gnomemeeting_threads_leave ();

    g_free (msg);

    return TRUE;
  }
  else {

    msg = g_strdup_printf (_("Error forwarding call to %s"), 
			   (const char*) forward_party);
    
    gnomemeeting_threads_enter ();
    gnomemeeting_warning_dialog (GTK_WINDOW (gm), msg);
    gnomemeeting_threads_leave ();

    g_free (msg);

    return FALSE;
  }

  return FALSE;
}


BOOL 
GMH323EndPoint::OnIncomingCall (H323Connection & connection, 
                                const H323SignalPDU &, H323SignalPDU &)
{
  char *msg = NULL;
  PString forward_host;
  PString name = connection.GetRemotePartyName ();
  PString app = connection.GetRemoteApplication ();
  gchar *utf8_name = NULL;
  gchar *utf8_app = NULL;

  gchar *forward_host_gconf = NULL;
  BOOL always_forward = FALSE;
  BOOL busy_forward = FALSE;
  BOOL aa = FALSE;
  BOOL dnd = FALSE;
  BOOL play_sound = FALSE;
  BOOL show_popup = FALSE;

  BOOL do_forward = FALSE; /* TRUE if all conditions are satisfied to forward
			      the call */
  BOOL do_reject = FALSE; /* TRUE if we reject the call */

  int no_answer_timeout = 0;


  /* Check the gconf keys */
  forward_host_gconf = 
    gconf_client_get_string (client, CALL_FORWARDING_KEY "forward_host", NULL);
  always_forward = 
    gconf_client_get_bool (client, CALL_FORWARDING_KEY "always_forward", NULL);
  busy_forward = 
    gconf_client_get_bool (client, CALL_FORWARDING_KEY "busy_forward", NULL);
  dnd = 
    gconf_client_get_bool (client, GENERAL_KEY "do_not_disturb", NULL);
  aa = 
    gconf_client_get_bool (client, GENERAL_KEY "auto_answer", NULL);
  play_sound = 
    gconf_client_get_bool (client, GENERAL_KEY "incoming_call_sound", NULL);
  show_popup =
    gconf_client_get_bool (client, VIEW_KEY "show_popup", NULL);

  if (forward_host_gconf)
    forward_host = PString (forward_host_gconf);
  else
    forward_host = PString ("");

    
  /* Remote Name and application */
  utf8_app = gnomemeeting_get_utf8 (gnomemeeting_pstring_cut (app));
  utf8_name = gnomemeeting_get_utf8 (gnomemeeting_pstring_cut (name));


  /* Update the history and status bar */
  msg = g_strdup_printf (_("Call from %s using %s"), 
			 (const char *) utf8_name,
			 (const char *) utf8_app);

  gnomemeeting_threads_enter ();
  gnomemeeting_statusbar_push (gw->statusbar, msg);
  gnomemeeting_log_insert (gw->history_text_view, msg);
  gnomemeeting_log_insert (gw->calls_history_text_view, msg);
  gnomemeeting_threads_leave ();
  g_free (msg);


  /* if we have enabled call forwarding for all calls, do the forward */
  if (!forward_host.IsEmpty() && always_forward) {

    msg = 
      g_strdup_printf (_("Forwarding Call from %s to %s (Forward All Calls)"),
		       (const char *) utf8_name, (const char *) forward_host);
    do_forward = TRUE;
  } 
  /* if we are already in a call: forward or reject */
  else if (!GetCurrentCallToken ().IsEmpty () || GetCallingState () != 0) {

    /* if we have enabled forward when busy, do the forward */
    if (!forward_host.IsEmpty() && busy_forward) {

      msg = 
	g_strdup_printf (_("Forwarding Call from %s to %s (Busy)"),
			 (const char *) utf8_name, 
			 (const char *) forward_host);

      do_forward = TRUE;
    } 
    else {

      /* there is no forwarding, so reject the call */
      msg = g_strdup_printf (_("Auto Rejecting Incoming Call from %s (Busy)"),
			     (const char *) utf8_name);
     
      do_reject = TRUE;
    }
  }


  if (do_reject || do_forward) {

    /* Add the full message in the history */
    gnomemeeting_threads_enter ();
    gnomemeeting_log_insert (gw->history_text_view, msg);
    gnomemeeting_threads_leave ();

    /* Free things, we will return */
    g_free (forward_host_gconf);
    g_free (utf8_name);
    g_free (utf8_app);
    g_free (msg);

    if (do_reject) {
      
      gnomemeeting_threads_enter ();
      gnomemeeting_statusbar_push (gw->statusbar, _("Auto Rejected"));
      gnomemeeting_log_insert (gw->calls_history_text_view, 
			       _("Auto Rejected"));
      gnomemeeting_threads_leave ();

      connection.ClearCall (H323Connection::EndedByLocalBusy); 
      return FALSE;
    }
    else {

      gnomemeeting_threads_enter ();
      gnomemeeting_statusbar_push (gw->statusbar, _("Call Forwarded"));
      gnomemeeting_log_insert (gw->calls_history_text_view, 
			       _("Call Forwarded"));
      gnomemeeting_threads_leave ();

      return !connection.ForwardCall (forward_host);
    }
  }
   

  /* If we are here, the call doesn't need to be rejected or forwarded */
  gnomemeeting_threads_enter ();
  gnomemeeting_call_menu_connect_set_sensitive (1, TRUE);
  gnomemeeting_threads_leave ();

 
  /* Do things only if no auto answer and no do not disturb */
  if (!aa && !dnd) {

#ifdef HAS_IXJ
    if (lid_thread) {

      OpalLineInterfaceDevice *lid = NULL;
      lid = lid_thread->GetLidDevice ();

      /* If we have a LID, make it ring */
      if (lid && lid->IsOpen()) {
	
	lid->RingLine (OpalIxJDevice::POTSLine, 0x33);
      }
    }
#endif


    /* The timers */
    gnomemeeting_threads_enter ();
    if ((docklet_timeout == 0)) {
      
      docklet_timeout = 
	gtk_timeout_add (1000, (GtkFunction) gnomemeeting_tray_flash, 
			 gw->docklet);
    }
    
    if (sound_timeout == 0 && play_sound) {
      
      sound_timeout = 
	gtk_timeout_add (1000, (GtkFunction) gnomemeeting_sound_play_ringtone,
			 gw->docklet);
    }
    
    if (no_answer_timeout == 0) {
      
      no_answer_timeout = 
	gtk_timeout_add (25000, (GtkFunction) IncomingCallTimeout, NULL);
    }
    gnomemeeting_threads_leave ();
			
			
    /* Incoming Call Popup, if needed */
    if (show_popup) {
    
      gnomemeeting_threads_enter ();
      gw->incoming_call_popup = 
	gnomemeeting_incoming_call_popup_new (utf8_name, utf8_app);
      gnomemeeting_threads_leave ();
    }
  

    gnomemeeting_threads_enter ();
    gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (gw->video_test_button), FALSE);
    gnomemeeting_threads_leave ();
  }
	

  /* If no forward or reject, update the internal state */
  if (GetCurrentCallToken ().IsEmpty ())
    SetCurrentCallToken (connection.GetCallToken ());
  SetCallingState (3);
  SetCurrentConnection (FindConnectionWithLock (GetCurrentCallToken ()));
  GetCurrentConnection ()->Unlock ();

  g_free (forward_host_gconf);
  g_free (utf8_name);
  g_free (utf8_app);

  return TRUE;
}


void 
GMH323EndPoint::OnConnectionEstablished (H323Connection & connection, 
                                         const PString & token)
{
  H323VideoCodec *video_codec = NULL;
  PString name = connection.GetRemotePartyName();
  PString app = connection.GetRemoteApplication ();
  gchar *utf8_app = NULL;
  gchar *utf8_name = NULL;
  char *msg = NULL;
  BOOL reg = FALSE;
  int vq = 0;
  int bf = 0;


  /* Remote Name and application */
  utf8_app = gnomemeeting_get_utf8 (gnomemeeting_pstring_cut (app));
  utf8_name = gnomemeeting_get_utf8 (gnomemeeting_pstring_cut (name));

  
  /* Get the gconf settings */
  vq = gconf_client_get_int (client, VIDEO_SETTINGS_KEY "tr_vq", NULL);
  bf = gconf_client_get_int (client, VIDEO_SETTINGS_KEY "tr_ub", NULL);
  reg = gconf_client_get_bool (client, LDAP_KEY "register", NULL);

  /* Remove the progress timeout */
  if (gw->progress_timeout) {

    gnomemeeting_threads_enter ();
    gtk_timeout_remove (gw->progress_timeout);
    gw->progress_timeout = 0;
    gtk_widget_hide (gw->progressbar);
    gnomemeeting_threads_leave ();
  }
  
  
  /* Set Video Codecs Settings */
  vq = 32 - (int) ((double) vq / 100 * 31);
  video_codec = GetCurrentVideoCodec ();
  if (video_codec) {

    video_codec->SetTxQualityLevel (vq);
    video_codec->SetBackgroundFill (bf);   
  }


  /* Connected */
  gnomemeeting_threads_enter ();
  msg = g_strdup_printf (_("Connected with %s using %s"), 
			 utf8_name, utf8_app);
  gnomemeeting_statusbar_push (gw->statusbar, _("Connected"));
  gnomemeeting_log_insert (gw->history_text_view, 
			   disableFastStart ?
			   _("Fast start disabled") :
			   _("Fast start enabled"));
  gnomemeeting_log_insert (gw->history_text_view,
			   disableH245Tunneling ?
			   _("H.245 Tunnelling disabled") :
			   _("H.245 Tunnelling enabled"));
  gnomemeeting_log_insert (gw->history_text_view, msg);
  gnomemeeting_log_insert (gw->calls_history_text_view, msg);

  gtk_entry_set_text (GTK_ENTRY (gw->remote_name), (const char *) utf8_name);

  if (docklet_timeout != 0) {

    gtk_timeout_remove (docklet_timeout);
    docklet_timeout = 0;
  }

  if (sound_timeout != 0) {

    gtk_timeout_remove (sound_timeout);
    sound_timeout = 0;
  }

  if (gw->incoming_call_popup) {
    
    gtk_widget_destroy (gw->incoming_call_popup);
    gw->incoming_call_popup = NULL;
  }
  gnomemeeting_threads_leave ();


#ifdef HAS_IXJ
  if (lid_thread) {

    OpalLineInterfaceDevice *lid = NULL;
    lid = lid_thread->GetLidDevice ();
    
    /* If we have a LID, make sure it is no longer ringing */
    if (lid && lid->IsOpen()) {

      lid->StopTone(0);
      lid->SetRemoveDTMF(0, TRUE);
    }
  }
#endif

  gnomemeeting_threads_enter ();
  connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 1);
  gnomemeeting_call_menu_pause_set_sensitive (TRUE);
  gnomemeeting_tray_set_content (G_OBJECT (gw->docklet), 2);
  gnomemeeting_threads_leave ();


  /* Update ILS if needed */
  if (reg)
    (GM_ILS_CLIENT (GetILSClientThread ()))->Modify ();


  /* Update internal state */
  SetCurrentCallToken (token);
  SetCurrentConnection (FindConnectionWithoutLocks (token));
  SetCallingState (2);

  g_free (msg);
  g_free (utf8_name);
  g_free (utf8_app);
}


void 
GMH323EndPoint::OnConnectionCleared (H323Connection & connection, 
                                     const PString & clearedCallToken)
{
  gchar *msg_reason = NULL;
  gchar *msg = NULL;
  PTimeInterval t;
  GtkTextIter start_iter, end_iter;
  BOOL dnd = FALSE;
  BOOL reg = FALSE;
  BOOL preview = FALSE;
  GMVideoGrabber *vg = GM_VIDEO_GRABBER (GetVideoGrabberThread ());

  /* Always wait to return from this function before quitting */
  quit_mutex.Wait ();

  gnomemeeting_threads_enter ();
  gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_settings_frame), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_test_button), TRUE);
  gnomemeeting_threads_leave ();

  var_access_mutex.Wait ();
  player_channel = NULL;
  recorder_channel = NULL;
  var_access_mutex.Signal ();


  /* Get GConf settings */
  dnd = gconf_client_get_bool (client, GENERAL_KEY "do_not_disturb", NULL);
  reg = gconf_client_get_bool (client, LDAP_KEY "register", NULL);
  preview = gconf_client_get_bool (client, DEVICES_KEY "video_preview", NULL);
 

  /* If we are called because the current incoming call has ended and 
     not another call, ok, else do nothing */
  if (GetCurrentCallToken () == clearedCallToken) {

    SetCurrentCallToken (PString ());
  }
  else {

    quit_mutex.Signal ();

    return;
  }


  opened_video_channels = 0;
  opened_audio_channels = 0;

  gnomemeeting_threads_enter ();

  switch (connection.GetCallEndReason ()) {

  case H323Connection::EndedByRemoteUser :
    msg_reason = g_strdup (_("Remote party has cleared the call"));
    break;
    
  case H323Connection::EndedByCallerAbort :
    msg_reason = g_strdup (_("Remote party has stopped calling"));
    break;

  case H323Connection::EndedByRefusal :
    msg_reason = g_strdup (_("Remote party did not accept your call"));
    break;

  case H323Connection::EndedByRemoteBusy :
    msg_reason = g_strdup (_("Remote party was busy"));
    break;

  case H323Connection::EndedByRemoteCongestion :
    msg_reason = g_strdup (_("Congested link to remote party"));
    break;

  case H323Connection::EndedByNoAnswer :
    msg_reason = g_strdup (_("The call was not answered in the required time"));
    break;
    
  case H323Connection::EndedByTransportFail :
    msg_reason = g_strdup (_("This call ended abnormally"));
    break;
    
  case H323Connection::EndedByCapabilityExchange :
    msg_reason = g_strdup (_("Could not find common codec with remote party"));
    break;

  case H323Connection::EndedByNoAccept :
    msg_reason = g_strdup (_("Remote party did not accept your call"));
    break;

  case H323Connection::EndedByAnswerDenied :
    msg_reason = g_strdup (_("Refused incoming call"));
    break;

  case H323Connection::EndedByNoUser :
    msg_reason = g_strdup (_("User not found"));
    break;
    
  case H323Connection::EndedByNoBandwidth :
    msg_reason = g_strdup (_("Call ended: insufficient bandwidth"));
    break;

  case H323Connection::EndedByUnreachable :
    msg_reason = g_strdup (_("Remote party could not be reached"));
    break;

  case H323Connection::EndedByHostOffline :
    msg_reason = g_strdup (_("Remote party is offline"));
    break;

  case H323Connection::EndedByConnectFail :
    msg_reason = g_strdup (_("Transport Error calling"));
    break;

  default :
    msg_reason = g_strdup (_("Call completed"));
  }

  gnomemeeting_log_insert (gw->history_text_view, msg_reason);
  gnomemeeting_log_insert (gw->calls_history_text_view, msg_reason);
  
 
  if (connection.GetConnectionStartTime ().GetTimeInSeconds () > 0) {

    t = PTime () - connection.GetConnectionStartTime();
    msg = g_strdup_printf (_("Call duration: %.2ld:%.2ld:%.2ld"),
			   (long) t.GetHours (), (long) t.GetMinutes () % 60,
			   (long) t.GetSeconds () % 60);
    gnomemeeting_log_insert (gw->history_text_view, msg);
    gnomemeeting_log_insert (gw->calls_history_text_view, msg);
    g_free (msg);
  }

  if (gw->progress_timeout) {

    gtk_timeout_remove (gw->progress_timeout);
    gw->progress_timeout = 0;
    gtk_widget_hide (gw->progressbar);
  }

  if (gw->incoming_call_popup) {

    gtk_widget_destroy (gw->incoming_call_popup);
    gw->incoming_call_popup = NULL;
  }

  if (docklet_timeout != 0) {

    gtk_timeout_remove (docklet_timeout);
    docklet_timeout = 0;
  }
  
  if (sound_timeout != 0) {

    gtk_timeout_remove (sound_timeout);
    sound_timeout = 0;
  }

  gnomemeeting_threads_leave ();


  /* Update the tray icon */
  gnomemeeting_threads_enter ();
  if (dnd)
    gnomemeeting_tray_set_content (G_OBJECT (gw->docklet), 2);
  else
    gnomemeeting_tray_set_content (G_OBJECT (gw->docklet), 0);
  gnomemeeting_threads_leave ();


  /* No need to do all that if we are simply receiving an incoming call
     that was rejected in connection.cpp because of(DND) */
  if (GetCallingState () != 3 && GetCallingState () != 1) {

    SetCallingState (0);

    /* Update ILS if needed */
    if (reg)
      (GM_ILS_CLIENT (GetILSClientThread ()))->Modify ();
  
    /* Reset the Video Grabber, if preview, else close it */
    if (preview) {

      vg->Close (TRUE);
      vg->Open (TRUE, TRUE); /* Grab and do a synchronous opening
				in this thread */
      /* Display grabbed images */
      vg->Start ();
    }
    else {
    
      if (vg->IsOpened ())
	vg->Close ();

      gnomemeeting_threads_enter ();
      gnomemeeting_zoom_submenu_set_sensitive (FALSE);
#ifdef HAS_SDL
      gnomemeeting_fullscreen_option_set_sensitive (FALSE);
#endif
      gnomemeeting_init_main_window_logo (gw->main_video_image);
      gnomemeeting_threads_leave ();
    }
  }


  /* Play Busy Tone */
#ifdef HAS_IXJ
  if (lid_thread) {

    OpalLineInterfaceDevice *lid = NULL;
    lid = lid_thread->GetLidDevice ();
    
    if (lid) {
      
      if (GTK_TOGGLE_BUTTON (gw->speaker_phone_button)->active)
	lid->EnableAudio (0, FALSE);
      
      lid->PlayTone (0, OpalLineInterfaceDevice::BusyTone);
      lid->RingLine (0, 0);
    }
  }
#endif

 
  /* We update the stats part */
  gnomemeeting_threads_enter ();
  gtk_entry_set_text (GTK_ENTRY (gw->remote_name), "");

  gtk_widget_queue_draw_area (gw->stats_drawing_area, 0, 0, GTK_WIDGET (gw->stats_drawing_area)->allocation.width, GTK_WIDGET (gw->stats_drawing_area)->allocation.height);
  gtk_label_set_text (GTK_LABEL (gw->stats_label), _("Sent/Received:\nLost/Late Packets:\nRound trip delay:\nJitter Buffer:"));

  
  /* We empty the text chat buffer */ 
  gtk_text_buffer_get_start_iter (chat->text_buffer, &start_iter);
  gtk_text_buffer_get_end_iter (chat->text_buffer, &end_iter);

  gtk_text_buffer_delete (chat->text_buffer, &start_iter, &end_iter);
  chat->buffer_is_empty = TRUE;


  /* Disable disconnect, and the mute functions in the call menu */
  gnomemeeting_call_menu_connect_set_sensitive (1, FALSE);
  gnomemeeting_call_menu_pause_set_sensitive (FALSE);
  connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 0);


  /* Disable Remote Video (Local video is disabled elsewhere) 
     and select the good section */
  gnomemeeting_video_submenu_set_sensitive (false, LOCAL_VIDEO, true);
  gnomemeeting_video_submenu_set_sensitive (false, REMOTE_VIDEO, true);
  gnomemeeting_video_submenu_select (0);


  /* Disable / enable buttons */
  if (!preview)
    gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_chan_button), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_chan_button), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), TRUE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_test_button), TRUE);

  GTK_TOGGLE_BUTTON (gw->audio_chan_button)->active = FALSE;
  GTK_TOGGLE_BUTTON (gw->video_chan_button)->active = FALSE;
  gtk_widget_queue_draw (gw->audio_chan_button);
  gtk_widget_queue_draw (gw->video_chan_button);


  /* Resume sound daemons */
  gnomemeeting_sound_daemons_resume ();
  gnomemeeting_threads_leave ();


  /* Try to update the config if some settings were changed 
     during the call */
  UpdateConfig ();


  /* Update internal state */
  SetCurrentConnection (NULL);
  SetCallingState (0);


  /* Display the call end reason in the statusbar */
  gdk_threads_enter ();
  gnomemeeting_statusbar_flash (gw->statusbar, msg_reason);
  g_free (msg_reason);
  gdk_threads_leave ();

  quit_mutex.Signal ();
}


void 
GMH323EndPoint::SavePicture (void)
{ 
  GdkPixbuf *pic = 
    gtk_image_get_pixbuf (GTK_IMAGE (gw->main_video_image));
  gchar *prefix = 
    gconf_client_get_string (client, GENERAL_KEY "save_prefix", NULL);
  gchar *dirname = (gchar *) g_get_home_dir ();
  gchar *filename = g_strdup_printf ("%s/%s%d.png", dirname, prefix, 
				     snapshot_number);

  snapshot_number++;

  gdk_pixbuf_save (pic, filename, "png", NULL, NULL);
  g_free (prefix);
  g_free (filename);
}


void GMH323EndPoint::SetUserNameAndAlias ()
{
  gchar *firstname = NULL;
  gchar *lastname = NULL;
  gchar *local_name = NULL;
  gchar *alias = NULL;
  
  /* Set the local User name */
  firstname = 
    gconf_client_get_string (client,PERSONAL_DATA_KEY "firstname", NULL);
  lastname  = 
    gconf_client_get_string (client, PERSONAL_DATA_KEY "lastname", NULL);
  alias = 
    gconf_client_get_string (client, GATEKEEPER_KEY "gk_alias", NULL);  

  if (firstname && lastname && strcmp (firstname, ""))  { 

    local_name = g_strconcat (firstname, " ", lastname, NULL);

    SetLocalUserName (local_name);

    g_free (firstname);
    g_free (lastname);
    g_free (local_name);
  }

  if (alias && strcmp (alias, "")) {

    AddAliasName (alias);
    g_free (alias);
  }
}


BOOL 
GMH323EndPoint::OpenAudioChannel(H323Connection & connection,
                                 BOOL isEncoding,
                                 unsigned bufferSize,
                                 H323AudioCodec & codec)
{
  gnomemeeting_threads_enter ();

  /* If needed , delete the timers */
  if (docklet_timeout != 0) {

    gtk_timeout_remove (docklet_timeout);
    docklet_timeout = 0;
  }

  if (sound_timeout != 0) {

    gtk_timeout_remove (sound_timeout);
    sound_timeout = 0;
  }


  /* Suspend the daemons */
  gnomemeeting_sound_daemons_suspend ();
  gnomemeeting_threads_leave ();

  opened_audio_channels++;
#ifdef HAS_IXJ
  OpalLineInterfaceDevice *lid = NULL;

  if (lid_thread) 
    lid = lid_thread->GetLidDevice ();
  
  /* If we are using a hardware LID, connect the audio stream to the LID */
  if (lid && lid->IsOpen()) {
    
    gnomemeeting_threads_enter ();
    gchar *msg = g_strdup_printf (_("Attaching lid hardware to codec"));
    gnomemeeting_log_insert (gw->history_text_view, msg);
    g_free (msg);
    
    gnomemeeting_threads_leave ();
    
      
    if (!codec.AttachChannel (new OpalLineChannel (*lid,
						   OpalIxJDevice::POTSLine, codec))) {
      return FALSE;
    }
  }
  else
#endif
  if (!H323EndPoint::OpenAudioChannel(connection, isEncoding, 
				      bufferSize, codec)) {

    gnomemeeting_threads_enter ();

    if (isEncoding)
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Could not open audio channel for audio transmission (soundcard busy?).\nDisabling audio transmission."));
    else
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Could not open audio channel for audio reception (soundcard busy?).\nDisabling audio reception."));
    gnomemeeting_threads_leave ();

    return FALSE;
  }

  if (isEncoding) {

    recorder_channel = (PSoundChannel *) codec.GetRawDataChannel ();
    gnomemeeting_threads_enter ();
    gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_rec), 
			      GetRecorderVolume ());
    gnomemeeting_threads_leave ();
  }
  else {

    player_channel = (PSoundChannel *) codec.GetRawDataChannel ();
    gnomemeeting_threads_enter ();
    gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_play), 
			      GetPlayerVolume ());
    gnomemeeting_threads_leave ();
  }

  gnomemeeting_threads_enter ();
  gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_settings_frame), TRUE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_test_button), FALSE);
  gnomemeeting_threads_leave ();

  return TRUE;
}


BOOL 
GMH323EndPoint::OpenVideoChannel (H323Connection & connection,
                                  BOOL isEncoding, 
                                  H323VideoCodec & codec)
{
  GMVideoGrabber *vg = GM_VIDEO_GRABBER (GetVideoGrabberThread ());
  BOOL vid_tr = FALSE;
  
  if (opened_video_channels >= 2)
    return FALSE;


  /* Get the gconf config */
  vid_tr = 
    gconf_client_get_bool (client, 
			   VIDEO_SETTINGS_KEY "enable_video_transmission", 
			   NULL);

  /* If it is possible to transmit and
     if the user enabled transmission and
     if OpenVideoDevice is called for the encoding */
  if (vid_tr && isEncoding) {

     if (vg->IsOpened ())
       vg->Stop ();
     else 
       vg->Open (FALSE, TRUE); /* Do not grab, synchronous opening */
     
     gnomemeeting_threads_enter ();
     
     /* Here, the grabber is opened */
     var_access_mutex.Wait ();
     PVideoChannel *channel = vg->GetVideoChannel ();
     transmitted_video_device = vg->GetEncodingDevice ();
     opened_video_channels++;
     var_access_mutex.Signal ();

     /* Updates the view menu */
     gnomemeeting_zoom_submenu_set_sensitive (TRUE);

#ifdef HAS_SDL
     gnomemeeting_fullscreen_option_set_sensitive (TRUE);
#endif

     if (opened_video_channels >= 2) 
	gnomemeeting_video_submenu_set_sensitive (TRUE, LOCAL_VIDEO, TRUE);
      else
	gnomemeeting_video_submenu_set_sensitive (TRUE, LOCAL_VIDEO, FALSE);


     /* Default Codecs Settings */
     codec.SetTxQualityLevel (-1);
     codec.SetAverageBitRate (0); // Disable

     gtk_widget_set_sensitive (GTK_WIDGET (gw->video_chan_button),
			       TRUE);
     
     gnomemeeting_threads_leave ();

     bool result = codec.AttachChannel (channel, FALSE); 

     return result;
  }
  else {

    /* If we only receive */
    if (!isEncoding) {
       
      var_access_mutex.Wait ();
      PVideoChannel *channel = new PVideoChannel;
      received_video_device = new GDKVideoOutputDevice (isEncoding, gw);
      opened_video_channels++;
      var_access_mutex.Signal ();

      channel->AttachVideoPlayer (received_video_device);

      /* Stop to grab */
      if (vg->IsOpened ())
	vg->Stop ();
      
      gnomemeeting_threads_enter ();

      /* Update menus */
      gnomemeeting_zoom_submenu_set_sensitive (TRUE);
#ifdef HAS_SDL
      gnomemeeting_fullscreen_option_set_sensitive (TRUE);
#endif

      if (opened_video_channels >= 2) 
	gnomemeeting_video_submenu_set_sensitive (TRUE, REMOTE_VIDEO, TRUE);
      else
	gnomemeeting_video_submenu_set_sensitive (TRUE, REMOTE_VIDEO, FALSE);

      gnomemeeting_video_submenu_select (1);
      
      gnomemeeting_threads_leave ();
      
      bool result = codec.AttachChannel (channel);

      return result;
    }
    else
      return FALSE;    
  }
}


#ifdef HAS_IXJ
GMLid *
GMH323EndPoint::GetLidThread (void)
{
  GMLid *l = NULL;

  var_access_mutex.Wait ();
  l = lid_thread;
  var_access_mutex.Signal ();

  return l;
}
#endif



