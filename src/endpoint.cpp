 
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
#include "misc.h"
#include "config.h"
#include "common.h"
#include "connection.h"
#include "tray.h"
#include "sound_handling.h"
#include "videograbber.h"
#include "gatekeeper.h"
#include "callbacks.h"
#include "ils.h"
#include "misc.h"
#include "menu.h"

#include <gconf/gconf-client.h>
#include <g726codec.h>
#include <gsmcodec.h>
#include <mscodecs.h>
#include <h261codec.h>
#include <videoio.h>
#include <gnome.h>
#include <lpc10codec.h>

#ifdef SPEEX_CODEC
#include <speexcodec.h>
#endif


#define new PNEW

#ifndef IPTOS_LOWDELAY
#define IPTOS_LOWDELAY 0x10
#endif

#ifndef IPTOS_PREC_PRIORITY
#define IPTOS_PREC_PRIORITY 0x20
#endif


/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;


/* The Timer */
static gint IncomingCallTimeout (gpointer data) 
{
  GmWindow       *gw = NULL;
  H323Connection *connection = NULL;
  gchar          *msg = NULL;
  gchar          *forward_host_gconf = NULL;
  PString         forward_host;
  gboolean        no_answer_forward = FALSE;
  GConfClient    *client = NULL;

  client = gconf_client_get_default ();

  gdk_threads_enter ();

  no_answer_forward = 
    gconf_client_get_bool (client, "/apps/gnomemeeting/call_forwarding/no_answer_forward", 0);
  forward_host_gconf = gconf_client_get_string (client, "/apps/gnomemeeting/call_forwarding/forward_host", 0);

  if (forward_host_gconf)
    forward_host = PString (forward_host_gconf);
  else
    forward_host = PString ("");


  gw = gnomemeeting_get_main_window (gm);

  if (gw->incoming_call_popup)
    gtk_widget_destroy (gw->incoming_call_popup);

  gw->incoming_call_popup = NULL;

  /* If forward host specified */
  if ((!forward_host.IsEmpty ())&&(no_answer_forward)) {

    connection = MyApp->Endpoint ()->GetCurrentConnection ();
    if (connection) {

      connection->ForwardCall (PString ((const char *) forward_host));
      msg = g_strdup_printf (_("Forwarding Call to %s (No Answer)"), 
			     (const char*) forward_host);
      gnome_appbar_push (GNOME_APPBAR (gw->statusbar), msg);
      gnomemeeting_log_insert (gw->history_text_view, msg);
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

  gw = gnomemeeting_get_main_window (gm);
  lw = gnomemeeting_get_ldap_window (gm);
  chat = gnomemeeting_get_chat_window (gm);

  SetCurrentConnection (NULL);
  SetCallingState (0);
  
#ifdef HAS_IXJ
  lid = NULL;
  lid_thread = NULL;
#endif

  video_grabber = NULL;
  listener = NULL;
  codecs_count = 1;
  snapshot_number = 0;

  docklet_timeout = 0;
  sound_timeout = 0;

  opened_audio_channels = 0;
  opened_video_channels = 0;

  /* Start the ILSClient PThread, do not register to it */
  ils_client = new GMILSClient ();

  /* Start the video grabber thread */
  video_grabber = new GMVideoGrabber ();

  /* The gconf client */
  client = gconf_client_get_default ();

  udp_port_range = 
    gconf_client_get_string (client, 
			     "/apps/gnomemeeting/general/udp_port_range", 
			     NULL);
  tcp_port_range = 
    gconf_client_get_string (client, 
			     "/apps/gnomemeeting/general/tcp_port_range", 
			     NULL);
  
  if (udp_port_range)
    udp_couple = g_strsplit (udp_port_range, ":", 0);
  if (tcp_port_range)
    tcp_couple = g_strsplit (tcp_port_range, ":", 0);

  

  clearCallOnRoundTripFail = TRUE;
  
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

  SetRtpIpTypeofService (IPTOS_LOWDELAY|IPTOS_PREC_PRIORITY);
  soundChannelBuffers = 4;

  received_video_device = NULL;
  transmitted_video_device = NULL;

  /* We can add this capability here as it will remain 
     the whole life of the EP */
  H323_UserInputCapability::AddAllCapabilities(capabilities, 0, P_MAX_INDEX);

  /* General Configuration */
  UpdateConfig ();

  /* GM is started */
  msg = g_strdup_printf (_("Started GnomeMeeting V%d.%d for %s\n"), 
			 MAJOR_VERSION, MINOR_VERSION, g_get_user_name ());
  gnomemeeting_log_insert (gw->history_text_view, msg);
  g_free (msg);
}


GMH323EndPoint::~GMH323EndPoint ()
{
  /* grabber and ILS threads are deleted in the
     cleaner */

#ifdef HAS_IXJ  
  if (lid) {
 
    lid->Close();
  }
#endif
}


void GMH323EndPoint::UpdateConfig ()
{
  int    found_player = 0;
  int    found_recorder = 0;
  gchar *text = NULL;
  gchar *player = NULL;
  gchar *recorder = NULL;
  gchar *recorder_mixer = NULL;
  gchar *lid_device = NULL;
  gchar *lid_country = NULL;
  int    lid_aec = 0;
  bool   use_lid = FALSE;
  GmPrefWindow *pw = NULL;
  GmWindow     *gw = NULL;

  gnomemeeting_threads_enter ();
  pw = gnomemeeting_get_pref_window (gm);
  gw = gnomemeeting_get_main_window (gm);

  /* Do not change these values during calls */
  if (GetCallingState () == 0) {

    use_lid = 
      gconf_client_get_bool (client, "/apps/gnomemeeting/devices/lid", 0);
  

    /* Set recording source and set micro to record if no LID is used */
    if (!use_lid) {

      player = 
	gconf_client_get_string (client, 
				 "/apps/gnomemeeting/devices/audio_player", 
				 NULL);
      recorder = 
	gconf_client_get_string (client, 
				 "/apps/gnomemeeting/devices/audio_recorder", 
				 NULL);
      recorder_mixer = 
	gconf_client_get_string (client, 
				 "/apps/gnomemeeting/devices/audio_recorder", 
				 NULL);
 

      /* Is the choosen device detected? */
      for (int i = gw->audio_player_devices.GetSize () - 1; i >= 0; i--) {
      
	if (player != NULL) {

	  if (!strcmp (player, gw->audio_player_devices [i]))
	    found_player = 1;
	}
      
	if (recorder != NULL) {
	  
	  if (!strcmp (recorder, gw->audio_recorder_devices [i]))
	    found_recorder = 1;
	}
      }
    

      /* Change that setting only if needed */
      if (GetSoundChannelPlayDevice () != PString (player)) { 

	if (found_player) {
     
	  SetSoundChannelPlayDevice (player);
	  text = g_strdup_printf (_("Set Audio player device to %s"), 
				  (const char *) player);
	}
	else {
      
	  SetSoundChannelPlayDevice (gw->audio_player_devices [0]);
	  text = g_strdup_printf (_("Set Audio player device to %s"), 
				  (const char *) gw->audio_player_devices [0]);
	}

	gnomemeeting_log_insert (gw->history_text_view, text);
	g_free (text);
      } 


      /* Change that setting only if needed */
      if (GetSoundChannelRecordDevice () != PString (recorder)) { 

	gnomemeeting_sound_daemons_suspend ();
	if (found_recorder) {
	  
	  SetSoundChannelRecordDevice (recorder);
	  gnomemeeting_set_recording_source (recorder_mixer, 0); 
	  text = g_strdup_printf (_("Set Audio recorder device to %s"), 
				  (const char *) recorder);
	}
	else {

	  SetSoundChannelRecordDevice (gw->audio_recorder_devices [0]);
	  
	  gnomemeeting_set_recording_source (recorder_mixer, 0); 

	  /* Translators: This is shown in the history. */
	  text = g_strdup_printf (_("Set Audio recorder device to %s"), 
				  (const char *) 
				  gw->audio_recorder_devices [0]);
	}
    
	gnomemeeting_log_insert (gw->history_text_view, text);
	g_free (text);
	gnomemeeting_sound_daemons_resume ();
      }
    }
    

    /**/
    /* Update the H.245 Tunnelling and Fast Start Settings if needed */
    if (disableH245Tunneling != !gconf_client_get_bool (client, "/apps/gnomemeeting/general/h245_tunneling", 0)) {

      if (!disableH245Tunneling)
	text = g_strdup (_("Disabling H.245 Tunnelling"));
      else
	text = g_strdup (_("Enabling H.245 Tunnelling"));

      gnomemeeting_log_insert (gw->history_text_view, text);
      g_free (text);
    }

    disableH245Tunneling = 
      !gconf_client_get_bool (client, 
			      "/apps/gnomemeeting/general/h245_tunneling", 0);


    if (disableFastStart != !gconf_client_get_bool (client, "/apps/gnomemeeting/general/fast_start", 0)) {

      if (!disableFastStart)
	text = g_strdup (_("Disabling Fast Start"));
      else
	text = g_strdup (_("Enabling Fast Start"));

      gnomemeeting_log_insert (gw->history_text_view, text);
      g_free (text);
    }

    disableFastStart = 
      !gconf_client_get_bool (client, 
			      "/apps/gnomemeeting/general/fast_start", 0);

#ifdef HAS_IXJ
    /* Use the quicknet card if needed */
    if (use_lid) {

      if (!lid) {
	
	lid_device =  
	  gconf_client_get_string (client, 
				   "/apps/gnomemeeting/devices/lid_device", 0);

	if (lid_device == NULL)
	  lid_device = g_strdup ("/dev/phone0");

	lid = new OpalIxJDevice;
	if (lid->Open ("/dev/phone0")) {
	
	  gchar *msg = NULL;
	  msg = g_strdup_printf (_("Using Quicknet device %s"), 
				 (const char *) lid->GetName ());
	  gnomemeeting_log_insert (gw->history_text_view, msg);
	  g_free (msg);
	  
	  lid->SetLineToLineDirect(0, 1, FALSE);
	  lid->EnableAudio(0, TRUE); /* Enable the POTS Telephone handset */
	  	 
	  lid_country =
	    gconf_client_get_string (client, 
				     "/apps/gnomemeeting/devices/lid_country",
				     NULL);
	  if (lid_country)
	    lid->SetCountryCodeName(lid_country);
	  g_free (lid_country);

	  lid_aec =
	    gconf_client_get_int (client, 
				  "/apps/gnomemeeting/devices/lid_aec", NULL);
	  switch (lid_aec) {
	    
	  case 0:
	    lid->SetAEC (0, OpalLineInterfaceDevice::AECOff);
	    break;

	  case 1:
	    lid->SetAEC (0, OpalLineInterfaceDevice::AECLow);
	    break;

	  case 2:
	    lid->SetAEC (0, OpalLineInterfaceDevice::AECMedium);
	    break;

	  case 3:
	    lid->SetAEC (0, OpalLineInterfaceDevice::AECHigh);
	    break;

	  case 5:
	    lid->SetAEC (0, OpalLineInterfaceDevice::AECAGC);
	    break;
	  }

	  lid_thread = PThread::Create (PCREATE_NOTIFIER(LidThread), 0,
					PThread::NoAutoDeleteThread,
					PThread::NormalPriority,
					"LidHookMonitor");

	/* Get the volumes for the mixers */
	  /* Should be fixed: changing the mixers must not changed the sliders
	     if LID is used and not using LID anymore must restore normal
	     mixers levels. Moreover it would be great to add a label */
	//   if (lid_aec != 5) {

// 	    gnomemeeting_volume_get (player_mixer, 0, &vol_play);
// 	    gnomemeeting_volume_get (recorder_mixer, 1, &vol_rec);
// 	  }

// 	  gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_play),
// 				    (int) (vol_play / 257));
// 	  gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_rec),
// 				    (int) (vol_rec / 257));
	  
	}
	else {

	  gconf_client_set_bool (client, "/apps/gnomemeeting/devices/lid", 
				 0, 0);
	  gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (gm), gw->speaker_phone_button, _("Error while opening the Quicknet device. Disabling Quicknet device."));
	}


	g_free (lid_device);
      }
    }
    else { /* If the user chose to not use anymore the Quicknet device */
      
      if (lid) {

	delete (lid);
	lid = NULL;
      }
    }
#endif


    /**/
    /* Update the capabilities */
    RemoveAllCapabilities ();
    AddAudioCapabilities ();
    AddVideoCapabilities (gconf_client_get_int (GCONF_CLIENT (client), "/apps/gnomemeeting/video_settings/video_size", NULL));


    g_free (player);
    g_free (recorder);
    g_free (recorder_mixer);    
  }

  gnomemeeting_threads_leave ();
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
  codecs_count = 1;
  capabilities.RemoveAll ();
}


void 
GMH323EndPoint::SetCallingState (int i)
{
  var_mutex.Wait ();
  calling_state = i;
  var_mutex.Signal ();
}


int 
GMH323EndPoint::GetCallingState (void)
{
  int cstate;

  var_mutex.Wait ();
  cstate = calling_state;
  var_mutex.Signal ();

  return cstate;
}


void 
GMH323EndPoint::AddVideoCapabilities (int video_size)
{
  if (video_size == 1) {

    /* CIF Capability in first position */
    SetCapability(0, 1, 
		  new H323_H261Capability (0, 2, FALSE, FALSE, 6217));

    codecs_count++;

    SetCapability(0, 1, 
		  new H323_H261Capability (4, 0, FALSE, FALSE, 6217));

    codecs_count++;
  }
  else {
    
    SetCapability(0, 1, 
		  new H323_H261Capability (4, 0, FALSE, FALSE, 6217));
    
    codecs_count++;
    
    SetCapability(0, 1, 
		  new H323_H261Capability (0, 2, FALSE, FALSE, 6217));

    codecs_count++;
  }

  if (gconf_client_get_bool (client, "/apps/gnomemeeting/video_settings/enable_video_transmission", 0)) {
    
    autoStartTransmitVideo = TRUE;
  }
  else {
    
    autoStartTransmitVideo = FALSE;
  }
}


void 
GMH323EndPoint::AddAudioCapabilities ()
{
  gchar **couple;
  GSList *codecs_data = NULL;
  BOOL use_pcm16_codecs = TRUE;

#ifdef HAS_IXJ
  /* Add the audio capabilities provided by the LID Hardware */
  if ((lid != NULL) && lid->IsOpen()) {

    H323_LIDCapability::AddAllCapabilities (*lid, capabilities, 0, 0);

    /* If the LID can do PCM16 we can use the software codecs like GSM too */
    use_pcm16_codecs = lid->GetMediaFormats ().GetValuesIndex (OpalMediaFormat(OPAL_PCM16)) != P_MAX_INDEX;
    
    /* Not well supported with my Quicknet Card => do not use them */
    use_pcm16_codecs = FALSE;
  }
#endif


  /* Add or not the audio capabilities */ 
  codecs_data = 
    gconf_client_get_list (client, 
			   "/apps/gnomemeeting/audio_codecs/codecs_list", 
			   GCONF_VALUE_STRING, NULL);
  
  int g711_frames = 
    gconf_client_get_int (client, 
			  "/apps/gnomemeeting/audio_settings/g711_frames", 
			  NULL);
  int gsm_frames = 
    gconf_client_get_int (client, 
			  "/apps/gnomemeeting/audio_settings/gsm_frames", 
			  NULL);


  /* Let's go */
  while (codecs_data) {
    
    couple = g_strsplit ((gchar *) codecs_data->data, "=", 0);

    if ((couple [0] != NULL)&&(couple [1] != NULL)) {


#ifdef SPEEX_CODEC
      if ((!strcmp (couple [0], "Speex-15k")) && (!strcmp (couple [1], "1"))) {
      
	SetCapability (0, 0, new SpeexNarrow5AudioCapability ());
	codecs_count++;  
      }

      if ((!strcmp (couple [0], "Speex-8.4k")) && (!strcmp (couple [1], "1"))) {
      
	SetCapability (0, 0, new SpeexNarrow3AudioCapability ());
	codecs_count++;  
      }
#endif

      if ((!strcmp (couple [0], "MS-GSM")) && (!strcmp (couple [1], "1"))) {
      
	MicrosoftGSMAudioCapability* gsm_capa; 
      
	SetCapability (0, 0, gsm_capa = new MicrosoftGSMAudioCapability);
	codecs_count++;  
	gsm_capa->SetTxFramesInPacket (gsm_frames);
      }

      if ((!strcmp (couple [0], "G.711-uLaw-64k"))&&(!strcmp (couple [1], "1"))) {
	
	H323_G711Capability *g711_capa; 
      
	SetCapability (0, 0, g711_capa = new H323_G711Capability 
		       (H323_G711Capability::muLaw));
	codecs_count++;
	g711_capa->SetTxFramesInPacket (g711_frames);
      }

      if ((!strcmp (couple [0], "G.711-ALaw-64k"))&&(!strcmp (couple [1], "1"))) {
	
	H323_G711Capability *g711_capa; 
	
	SetCapability (0, 0, g711_capa = new H323_G711Capability 
		       (H323_G711Capability::ALaw));
	codecs_count++;
	g711_capa->SetTxFramesInPacket (g711_frames);
      }
      
      if ((!strcmp (couple [0], "GSM-06.10"))&&(!strcmp (couple [1], "1"))) {
	
	H323_GSM0610Capability * gsm_capa; 
	
	SetCapability (0, 0, gsm_capa = new H323_GSM0610Capability);	
	codecs_count++;
	
	gsm_capa->SetTxFramesInPacket (gsm_frames);
      }
      
      if ((!strcmp (couple [0], "G.726-32k"))&&(!strcmp (couple [1], "1"))) {
	
	H323_G726_Capability * g72616_capa; 
	
	SetCapability (0, 0, g72616_capa = 
		       new H323_G726_Capability (*this, H323_G726_Capability::e_32k));
	codecs_count++;
      }
      
      if ((!strcmp (couple [0], "LPC10"))&&(!strcmp (couple [1], "1"))) {
	
	SetCapability(0, 0, new H323_LPC10Capability (*this));
	codecs_count++;
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

  /* If enabled */
  if (gconf_client_get_bool (client, "/apps/gnomemeeting/general/ip_translation", NULL)) {

    if ( !((remoteAddr.Byte1() == 192) && (remoteAddr.Byte2() == 168))
	 
	 && !((remoteAddr.Byte1() == 172) 
	      && ((remoteAddr.Byte2() >= 16)&&(remoteAddr.Byte2()<=31)))
	 
	 && !(remoteAddr.Byte1() == 10)) {

      gchar *ip = gconf_client_get_string (client, "/apps/gnomemeeting/general/public_ip", NULL);
      if (ip)
	addr = PIPSocket::Address(ip);

      if (addr != PIPSocket::Address ("0.0.0.0"))
	localAddr = addr;
    }
  }

  return;
}


BOOL 
GMH323EndPoint::StartListener ()
{
  int listen_port = gconf_client_get_int (client, "/apps/gnomemeeting/general/listen_port", NULL);

  /* Start the listener thread for incoming calls */
  listener = new H323ListenerTCP (*this, INADDR_ANY, 
				  listen_port);

  /* Succesfull ? */
  if (!H323EndPoint::StartListener (listener)) {

    delete listener;
    listener = NULL;

    return FALSE;
  }
   
  return TRUE;
}


H323Connection *
GMH323EndPoint::CreateConnection (unsigned callReference)
{
  return new GMH323Connection (*this, callReference);
}


H323Connection *
GMH323EndPoint::GetCurrentConnection ()
{
  H323Connection *con;

  var_mutex.Wait ();
  con = current_connection;
  var_mutex.Signal ();

  return con;
}


GMVideoGrabber *
GMH323EndPoint::GetVideoGrabber (void)
{
  return (GMVideoGrabber *) video_grabber;
}


void 
GMH323EndPoint::EnableVideoTransmission (bool i)
{
  autoStartTransmitVideo = i;
}


H323VideoCodec *
GMH323EndPoint::GetCurrentVideoCodec (void)
{
  H323VideoCodec *video_codec = NULL;
  H323Channel *channel = NULL;

  channel = GetCurrentVideoChannel ();

  if (channel != NULL) {
	
    H323Codec * raw_codec  = channel->GetCodec();

    if (raw_codec->IsDescendant (H323VideoCodec::Class())) {
      
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
    
    if (raw_codec->IsDescendant (H323AudioCodec::Class())) {
      
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


PThread *
GMH323EndPoint::GetILSClient (void)
{
  return ils_client;
}


void 
GMH323EndPoint::SetCurrentConnection (H323Connection *c)
{
  current_connection = c;
}


void 
GMH323EndPoint::SetCurrentCallToken (PString s)
{
  var_mutex.Wait ();
  current_call_token = s;
  var_mutex.Signal ();
}


PString 
GMH323EndPoint::GetCurrentCallToken ()
{
  PString c;

  var_mutex.Wait ();
  c = current_call_token;
  var_mutex.Signal ();

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

  if (MakeCall (forward_party, current_call_token)) {

    gnomemeeting_threads_enter ();
    msg = g_strdup_printf (_("Forwarding Call to %s"), 
			   (const char*) forward_party);
    gnome_appbar_push (GNOME_APPBAR (gw->statusbar), msg);
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
  PString name = connection.GetRemotePartyName ();
  PString app = connection.GetRemoteApplication ();
  PString forward_host;
  gchar *utf8_name = NULL;
  gchar *utf8_app = NULL;

  gchar *forward_host_gconf = NULL;
  gboolean always_forward = FALSE;
  gboolean busy_forward = FALSE;
  gboolean aa = FALSE;
  gboolean dnd = FALSE;

  int no_answer_timeout = 0;

  /* Check the forward host if any */
  forward_host_gconf = 
    gconf_client_get_string (client, 
			     "/apps/gnomemeeting/call_forwarding/forward_host", 0);
  always_forward = 
    gconf_client_get_bool (client, 
			   "/apps/gnomemeeting/call_forwarding/always_forward", 0);
  busy_forward = 
    gconf_client_get_bool (client, 
			   "/apps/gnomemeeting/call_forwarding/busy_forward", 0);
 
  
  /* Auto Answer / Do Not Disturb */
  dnd = gconf_client_get_bool 
    (client, "/apps/gnomemeeting/general/do_not_disturb", 0);

  aa = gconf_client_get_bool 
    (client, "/apps/gnomemeeting/general/auto_answer", 0);


  /* Convert the remote party name and app to UTF8 */
  utf8_name = g_convert ((const char *) name, strlen ((const char *) name),
			 "UTF-8", "ISO-8859-1", NULL, NULL, NULL);
  utf8_app = g_convert ((const char *) app, strlen ((const char *) app),
			"UTF-8", "ISO-8859-1", NULL, NULL, NULL);


  if (forward_host_gconf)
    forward_host = PString (forward_host_gconf);
  else
    forward_host = PString ("");


  /* Update the history and status bar */
  msg = g_strdup_printf (_("Call from %s using %s"), 
			 (const char *) utf8_name,
			 (const char *) utf8_app);

  gnomemeeting_threads_enter ();
  gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
		     (gchar *) msg);
			 
  gnomemeeting_log_insert (gw->history_text_view, msg);
  gnomemeeting_log_insert (gw->calls_history_text_view, msg);
  gnomemeeting_threads_leave ();
  g_free (msg);


  /* if we have enabled call forwarding for all calls, do the forward */
  if ((!forward_host.IsEmpty())&&(always_forward)) {

    gnomemeeting_threads_enter ();
    msg = 
      g_strdup_printf (_("Forwarding Call from %s to %s (Forward All Calls)"),
		       (const char *) utf8_name, (const char *) forward_host);
    gnome_appbar_push (GNOME_APPBAR (gw->statusbar), msg);
    gnomemeeting_log_insert (gw->history_text_view, msg);
    gnomemeeting_log_insert (gw->calls_history_text_view, _("Call forwarded"));
    gnomemeeting_threads_leave ();

    g_free (forward_host_gconf);
    g_free (msg);
    g_free (utf8_name);

    return !connection.ForwardCall (forward_host);
  }


  /* if we are already in a call */
  if (!(GetCurrentCallToken ().IsEmpty ())||(GetCallingState () != 0)) {

    /* if we have enabled forward when busy, do the forward */
    if ((!forward_host.IsEmpty())&&(busy_forward)) {

      gnomemeeting_threads_enter ();
      msg = g_strdup_printf (_("Forwarding Call from %s to %s (Busy)"),
			     (const char *) utf8_name, 
			     (const char *) forward_host);
      gnome_appbar_push (GNOME_APPBAR (gw->statusbar), msg);
      gnomemeeting_log_insert (gw->history_text_view, msg);
      gnomemeeting_log_insert (gw->calls_history_text_view, 
			       _("Call forwarded"));
      gnomemeeting_threads_leave ();

      g_free (forward_host_gconf);
      g_free (msg);
      g_free (utf8_name);

      return !connection.ForwardCall (forward_host);
    } 
    else {

      /* there is no forwarding, so reject the call */
      gnomemeeting_threads_enter ();
      msg = g_strdup_printf (_("Auto Rejecting Incoming Call from %s (Busy)"),
			     (const char *) utf8_name);
      gnome_appbar_push (GNOME_APPBAR (gw->statusbar), msg);
      gnomemeeting_log_insert (gw->history_text_view, msg);
      gnomemeeting_log_insert (gw->calls_history_text_view, 
			       _("Auto Rejected"));
      g_free (msg);

      gnomemeeting_threads_leave ();

      connection.ClearCall(H323Connection::EndedByLocalBusy);   

      g_free (forward_host_gconf);
      g_free (utf8_name);

      return FALSE;
    }
  }
  
   
  /* Enable disconnect: we must be able to refuse the incoming call */
  gnomemeeting_threads_enter ();
  GnomeUIInfo *call_menu_uiinfo =
    (GnomeUIInfo *) g_object_get_data (G_OBJECT (gm), "call_menu_uiinfo");
  gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [1].widget), TRUE);
  gnomemeeting_threads_leave ();

  current_connection = FindConnectionWithLock (connection.GetCallToken ());
  current_connection->Unlock ();

  SetCallingState (3);


  /* Do things with the docklet, the ring, and the lid 
     only if no aa or dnd */
  if ((!aa) && (!dnd)) {

#ifdef HAS_IXJ
    /* If we have a LID, make it ring */
    if ((lid != NULL) && (lid->IsOpen())) {
      
      lid->RingLine (OpalIxJDevice::POTSLine, 0x33);
    }
#endif


    /* The timers */
    gnomemeeting_threads_enter ();
    if ((docklet_timeout == 0)) {

      docklet_timeout = 
	gtk_timeout_add (1000, 
			 (GtkFunction) gnomemeeting_tray_flash, 
			 gw->docklet);
    }

    if ((sound_timeout == 0) && (gconf_client_get_bool (client, "/apps/gnomemeeting/general/incoming_call_sound", 0))) {

      sound_timeout = gtk_timeout_add (1000, 
				       (GtkFunction) gnomemeeting_sound_play_ringtone,
				       gw->docklet);
    }

    if (no_answer_timeout == 0) {

      no_answer_timeout = gtk_timeout_add (25000, 
					   (GtkFunction) IncomingCallTimeout,
					   NULL);
    }
    gnomemeeting_threads_leave ();
  }


  /* Incoming Call Popup, if needed */
  if (gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_popup", 0) 
      && (!aa) && (!dnd)) {

    gnomemeeting_threads_enter ();
    gw->incoming_call_popup = 
      gnomemeeting_incoming_call_popup_new (utf8_name, utf8_app);
    gnomemeeting_threads_leave ();
  }
  

  gnomemeeting_threads_enter ();
  gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
  gnomemeeting_threads_leave ();


  SetCurrentCallToken (connection.GetCallToken());

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
  int vq;


  /* Convert remote app and remote name */
  utf8_app = g_convert ((const char *) app, strlen ((const char *) app),
			"UTF-8", "ISO-8859-1", NULL, NULL, NULL);
  utf8_name = g_convert ((const char *) name, strlen ((const char *) name),
			 "UTF-8", "ISO-8859-1", NULL, NULL, NULL);


  gnomemeeting_threads_enter ();

  if (gw->progress_timeout) {

    gtk_timeout_remove (gw->progress_timeout);
    gw->progress_timeout = 0;
    gtk_widget_hide (GTK_WIDGET (gnome_appbar_get_progress (GNOME_APPBAR (gw->statusbar))));
  }
  
  /* Set Video Codecs Settings */
  vq = 32 - (int) ((double) gconf_client_get_int (client, "/apps/gnomemeeting/video_settings/tr_vq", NULL) / 100 * 31);

  video_codec = GetCurrentVideoCodec ();

  if (video_codec) {

    video_codec->SetTxQualityLevel (vq);
    video_codec->SetBackgroundFill (gconf_client_get_int (client, "/apps/gnomemeeting/video_settings/tr_ub", 0));   
  }


  /* Connected */
  msg = g_strdup_printf (_("Connected with %s using %s"), 
			 utf8_name, utf8_app);

  SetCurrentCallToken (token);
  SetCurrentConnection (FindConnectionWithoutLocks (token));

  gnome_appbar_push (GNOME_APPBAR (gw->statusbar), _("Connected"));

  if (gconf_client_get_bool (client, "/apps/gnomemeeting/general/fast_start", 0))    
    gnomemeeting_log_insert (gw->history_text_view, _("Fast start enabled"));
  else
    gnomemeeting_log_insert (gw->history_text_view, _("Fast start disabled"));

  if (disableH245Tunneling == 0)    
    gnomemeeting_log_insert (gw->history_text_view, 
			     _("H.245 Tunnelling enabled"));
  else
    gnomemeeting_log_insert (gw->history_text_view,
			     _("H.245 Tunnelling disabled"));

  gnomemeeting_log_insert (gw->history_text_view, msg);

  PINDEX bracket = name.Find('[');
  if (bracket != P_MAX_INDEX)
    name = name.Left (bracket);

  bracket = name.Find('(');
  if (bracket != P_MAX_INDEX)
    name = name.Left (bracket);

  gtk_entry_set_text (GTK_ENTRY (gw->remote_name), (const char *) utf8_name);

  if (docklet_timeout != 0)
    gtk_timeout_remove (docklet_timeout);

  if (sound_timeout != 0)
    gtk_timeout_remove (sound_timeout);

  docklet_timeout = 0;
  sound_timeout = 0;


#ifdef HAS_IXJ
  /* If we have a LID, make sure it is no longer ringing */
  if ((lid != NULL) && (lid->IsOpen())) {
    lid->RingLine (OpalIxJDevice::POTSLine, 0);
  }
#endif

  connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 1);

  /* Enable the mute functions in the call menu */
  GnomeUIInfo *call_menu_uiinfo =
    (GnomeUIInfo *) g_object_get_data (G_OBJECT (gm), "call_menu_uiinfo");
  gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [6].widget), TRUE);
  gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [7].widget), TRUE);

  gnomemeeting_threads_leave ();


  /* Update ILS if needed */
  if (gconf_client_get_bool (client, "/apps/gnomemeeting/ldap/register", 0))
    ((GMILSClient *) (ils_client))->Modify ();

  gnomemeeting_threads_enter ();

  /* Update the icon in the tray */
  gnomemeeting_tray_set_content (G_OBJECT (gw->docklet), 2);


  /* If popup, destroy it */
  if (gw->incoming_call_popup) {
    
    gtk_widget_destroy (gw->incoming_call_popup);
    gw->incoming_call_popup = NULL;
  }

  gnomemeeting_threads_leave ();

  SetCallingState (2);

  g_free (msg);
  g_free (utf8_name);
  g_free (utf8_app);
}


void 
GMH323EndPoint::OnConnectionCleared (H323Connection & connection, 
                                     const PString & clearedCallToken)
{
  GMVideoGrabber *vg = (GMVideoGrabber *) video_grabber;
  gchar *msg = NULL;
  PTimeInterval t;
  GtkTextIter start_iter, end_iter;

  /* If we are called because the current incoming call has ended and 
     not another call, ok, else do nothing */
  if (GetCurrentCallToken () == clearedCallToken) {

    SetCurrentCallToken (PString ());
  }
  else {

    gnomemeeting_threads_enter ();
    gnome_appbar_clear_stack (GNOME_APPBAR (gw->statusbar));
    gnomemeeting_threads_leave ();

    return;
  }


  opened_video_channels = 0;
  opened_audio_channels = 0;

  gnomemeeting_threads_enter ();

  switch (connection.GetCallEndReason ()) {

  case H323Connection::EndedByRemoteUser :
    msg = g_strdup (_("Remote party has cleared the call"));
    break;
    
  case H323Connection::EndedByCallerAbort :
    msg = g_strdup (_("Remote party has stopped calling"));
    break;

  case H323Connection::EndedByRefusal :
    msg = g_strdup (_("Remote party did not accept your call"));
    break;

  case H323Connection::EndedByRemoteBusy :
    msg = g_strdup (_("Remote party was busy"));
    break;

  case H323Connection::EndedByRemoteCongestion :
    msg = g_strdup (_("Congested link to remote party"));
    break;

  case H323Connection::EndedByNoAnswer :
    msg = g_strdup (_("The call was not answered in the required time"));
    break;
    
  case H323Connection::EndedByTransportFail :
    msg = g_strdup (_("This call ended abnormally"));
    break;
    
  case H323Connection::EndedByCapabilityExchange :
    msg = g_strdup (_("Could not find common codec with remote party"));
    break;

  case H323Connection::EndedByNoAccept :
    msg = g_strdup (_("Remote party did not accept your call"));
    break;

  case H323Connection::EndedByAnswerDenied :
    msg = g_strdup (_("Refused incoming call"));
    break;

  case H323Connection::EndedByNoUser :
    msg = g_strdup (_("User not found"));
    break;
    
  case H323Connection::EndedByNoBandwidth :
    msg = g_strdup (_("Call ended: insufficient bandwidth"));
    break;

  case H323Connection::EndedByUnreachable :
    msg = g_strdup (_("Remote party could not be reached"));
    break;

  case H323Connection::EndedByHostOffline :
    msg = g_strdup (_("Remote party is offline"));
    break;

  case H323Connection::EndedByConnectFail :
    msg = g_strdup (_("Transport Error calling"));
    break;

  default :
    msg = g_strdup (_("Call completed"));
  }
  
  gnomemeeting_statusbar_flash (gm, msg);
  gnomemeeting_log_insert (gw->history_text_view, msg);
  gnomemeeting_log_insert (gw->calls_history_text_view, msg);
  g_free (msg);
 
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
    gtk_widget_hide (GTK_WIDGET (gnome_appbar_get_progress (GNOME_APPBAR (gw->statusbar))));
  }

  gnomemeeting_threads_leave ();


  /* Update the tray icon */
  gnomemeeting_threads_enter ();
  if (gconf_client_get_bool 
      (client, "/apps/gnomemeeting/general/do_not_disturb", 0)) 
    gnomemeeting_tray_set_content (G_OBJECT (gw->docklet), 2);
  else
    gnomemeeting_tray_set_content (G_OBJECT (gw->docklet), 0);
  gnomemeeting_threads_leave ();


  /* No need to do all that if we are simply receiving an incoming call
     that was rejected in connection.cpp because of(DND) */
  if (GetCallingState () != 3) {

    SetCallingState (0);

    /* Update ILS if needed */
    if (gconf_client_get_bool (client, "/apps/gnomemeeting/ldap/register", 0))
      ((GMILSClient *) (ils_client))->Modify ();
  
    /* Reset the Video Grabber, if preview, else close it */
    if (gconf_client_get_bool (client, 
			       "/apps/gnomemeeting/devices/video_preview", 0)) {

      vg->Close (TRUE);
      vg->Open (TRUE, TRUE); /* Grab and do a synchronous opening
				in this thread */
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


  /* Display grabbed images */
  vg->Start ();


  /* Play Busy Tone */
#ifdef HAS_IXJ
  if (lid) {

    if (GTK_TOGGLE_BUTTON (gw->speaker_phone_button)->active)
      lid->EnableAudio (0, FALSE);

    lid->PlayTone (0, OpalLineInterfaceDevice::BusyTone);
    lid->RingLine (OpalIxJDevice::POTSLine, 0);
  }
#endif

  /* We destroy the incoming call popup if any */
  gnomemeeting_threads_enter ();
  if (gw->incoming_call_popup) {

    gtk_widget_destroy (gw->incoming_call_popup);
    gw->incoming_call_popup = NULL;
  }
  gnomemeeting_threads_leave ();



  /* We update the stats part */
  gnomemeeting_threads_enter ();
  gtk_entry_set_text (GTK_ENTRY (gw->remote_name), "");

  gtk_widget_queue_draw_area (gw->stats_drawing_area, 0, 0, GTK_WIDGET (gw->stats_drawing_area)->allocation.width, GTK_WIDGET (gw->stats_drawing_area)->allocation.height);
  gtk_label_set_text (GTK_LABEL (gw->stats_label), _("Sent/Received:\nLost/Late Packets:\nRound trip delay:"));

  
  /* We empty the text chat buffer */ 
  gtk_text_buffer_get_start_iter (chat->text_buffer, &start_iter);
  gtk_text_buffer_get_end_iter (chat->text_buffer, &end_iter);

  gtk_text_buffer_delete (chat->text_buffer, &start_iter, &end_iter);
  chat->buffer_is_empty = TRUE;

  connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 0);


  /* Disable disconnect, and the mute functions in the call menu */
  GnomeUIInfo *call_menu_uiinfo =
    (GnomeUIInfo *) g_object_get_data (G_OBJECT (gm), "call_menu_uiinfo");
  gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [0].widget), TRUE);
  gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [1].widget), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [6].widget), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (call_menu_uiinfo [7].widget), FALSE);


  /* Disable Remote Video (Local video is disabled elsewhere) 
     and select the good section */
  gnomemeeting_video_submenu_set_sensitive (FALSE);
  gnomemeeting_video_submenu_select (0);


  /* Remove the timers if needed and clear the docklet */
  if (docklet_timeout != 0)
    gtk_timeout_remove (docklet_timeout);
  
  docklet_timeout = 0;
  
  if (sound_timeout != 0)
    gtk_timeout_remove (sound_timeout);
  
  sound_timeout = 0;
  
  gnomemeeting_threads_leave ();
  
  gnomemeeting_threads_enter ();


  /* Disable / enable buttons */
  if (!gconf_client_get_bool (client, "/apps/gnomemeeting/devices/video_preview", 0))
    gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_chan_button), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_chan_button), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), TRUE);

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

  SetCurrentConnection (NULL);
  SetCallingState (0);
}


void 
GMH323EndPoint::SavePicture (void)
{ 
  GdkPixbuf *pic = gtk_image_get_pixbuf (GTK_IMAGE (gw->main_video_image));
  gchar *prefix = 
    gconf_client_get_string (client, "/apps/gnomemeeting/general/save_prefix",
			     NULL);
  gchar *dirname = (gchar *) g_get_home_dir ();
  gchar *filename = g_strdup_printf ("%s/%s%d.png", dirname, prefix, 
				     snapshot_number);

  snapshot_number++;

  gdk_pixbuf_save (pic, filename, "png", NULL, NULL);
  g_free (prefix);
  g_free (filename);
}


BOOL 
GMH323EndPoint::OpenAudioChannel(H323Connection & connection,
                                 BOOL isEncoding,
                                 unsigned bufferSize,
                                 H323AudioCodec & codec)
{
  gnomemeeting_threads_enter ();

  /* If needed , delete the timers */
  if (docklet_timeout != 0)
    gtk_timeout_remove (docklet_timeout);
  docklet_timeout = 0;

  if (sound_timeout != 0)
    gtk_timeout_remove (sound_timeout);
  sound_timeout = 0;


  /* Suspend the daemons */
  gnomemeeting_sound_daemons_suspend ();

  gnomemeeting_threads_leave ();

  opened_audio_channels++;
#ifdef HAS_IXJ
  /* If we are using a hardware LID, connect the audio stream to the LID */
  if ((lid != NULL) && lid->IsOpen()) {

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

  return TRUE;
}


BOOL 
GMH323EndPoint::OpenVideoChannel (H323Connection & connection,
                                  BOOL isEncoding, 
                                  H323VideoCodec & codec)
{
  GMVideoGrabber *vg = (GMVideoGrabber *) video_grabber;
  
  if (opened_video_channels >= 2)
    return FALSE;

  /* If it is possible to transmit and
     if the user enabled transmission and
     if OpenVideoDevice is called for the encoding */
  if ((gconf_client_get_bool (client, "/apps/gnomemeeting/video_settings/enable_video_transmission", 0))&&(isEncoding)) {

     if (vg->IsOpened ())
       vg->Stop ();
     
     if (!vg->IsOpened ())
       vg->Open (FALSE, TRUE); /* Do not grab, synchronous opening */
     
     gnomemeeting_threads_enter ();
     
     /* Here, the grabber is opened */
     PVideoChannel *channel = vg->GetVideoChannel ();
     transmitted_video_device = vg->GetEncodingDevice ();

     /* Updates the view menu */
     gnomemeeting_zoom_submenu_set_sensitive (TRUE);

#ifdef HAS_SDL
     gnomemeeting_fullscreen_option_set_sensitive (TRUE);
#endif

     /* Default Codecs Settings */
     codec.SetTxQualityLevel (-1);
     codec.SetAverageBitRate (0); // Disable

     gtk_widget_set_sensitive (GTK_WIDGET (gw->video_chan_button),
			       TRUE);
     
     gnomemeeting_threads_leave ();

     bool result = codec.AttachChannel (channel, FALSE); 
     
     opened_video_channels++;

     return result;
  }
  else {

    /* If we only receive */
    if (!isEncoding) {
       
      PVideoChannel *channel = new PVideoChannel;
      
      received_video_device = new GDKVideoOutputDevice (isEncoding, gw);
      
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

      gnomemeeting_video_submenu_set_sensitive (TRUE);
      gnomemeeting_video_submenu_select (1);
      
      gnomemeeting_threads_leave ();
      
      bool result = codec.AttachChannel (channel);

      opened_video_channels++;

      return result;
    }
    else
      return FALSE;    
  }
}


#ifdef HAS_IXJ
PThread *
GMH323EndPoint::GetLidThread (void)
{
  return lid_thread;
}


OpalLineInterfaceDevice *
GMH323EndPoint::GetLidDevice ()
{
  return lid;
}
#endif


#ifdef HAS_IXJ
void 
GMH323EndPoint::LidThread (PThread &, INT)
{
  BOOL OffHook, lastOffHook;


  /* Check the initial hook status. */
  OffHook = lastOffHook = lid->IsLineOffHook (OpalIxJDevice::POTSLine);

  /* OffHook can take a few cycles to settle, so on the first pass */
  /* assume we are off-hook and play a dial tone. */
  lid->PlayTone(0, OpalLineInterfaceDevice::DialTone);

  while ((lid != NULL) && (lid->IsOpen()) )
  {
    OffHook = 
      (lid->IsLineOffHook (OpalIxJDevice::POTSLine));


    /* If there is a state change */
    if ((OffHook == TRUE) && (lastOffHook == FALSE)) {

      if (GetCallingState() == 3) { /* 3 = incoming call */

	lid->StopTone (0);
        lid->RingLine(OpalIxJDevice::POTSLine, 0);
	
        gnomemeeting_threads_enter ();
	if (gw->incoming_call_popup) {

	  gtk_widget_destroy (gw->incoming_call_popup);
	  gw->incoming_call_popup = NULL;
	}
	gnomemeeting_threads_leave ();

	MyApp->Connect ();
      }


      if (GetCallingState() == 0) { /* not connected */

        lid->PlayTone (0, OpalLineInterfaceDevice::DialTone);
      }
    }


    /* if phone is on hook */
    if ((OffHook == FALSE) && (lastOffHook == TRUE)) {

      if (GetCallingState() == 2) { /* 2 = currently in a call */

	MyApp->Disconnect ();
      }
    }

    lastOffHook = OffHook;

    /* We must poll to read the hook state */
    PThread::Sleep(50);

  }
}
#endif
