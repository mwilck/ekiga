
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2003 Damien Sandras
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
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         endpoint.cpp  -  description
 *                         ----------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains the Endpoint class.
 *
 */


#include "../config.h"

#include "endpoint.h"
#include "connection.h"
#include "gatekeeper.h"
#include "urlhandler.h"
#include "ils.h"
#include "gnomemeeting.h"
#include "sound_handling.h"
#include "tray.h"
#include "misc.h"
#include "menu.h"
#include "toolbar.h"
#include "chat_window.h"
#include "ldap_window.h"
#include "pref_window.h"
#include "main_window.h"
#include "tools.h"
#include "dialog.h"

#include <g726codec.h>
#include <gsmcodec.h>
#include <mscodecs.h>
#include <h261codec.h>
#include <lpc10codec.h>
#include <speexcodec.h>

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

  gdk_threads_enter ();
  client = gconf_client_get_default ();
  
  /* Forwarding on no answer */
  no_answer_forward = 
    gconf_client_get_bool (client, CALL_FORWARDING_KEY "no_answer_forward", 0);
  forward_host_gconf = 
    gconf_client_get_string (client, CALL_FORWARDING_KEY "forward_host", 0);

  if (forward_host_gconf)
    forward_host = PString (forward_host_gconf);
  else
    forward_host = PString ("");

  gw = MyApp->GetMainWindow ();

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
  gchar *rtp_port_range = NULL;
  gchar *udp_port_range = NULL;
  gchar *tcp_port_range = NULL;
  gchar **rtp_couple = NULL;
  gchar **udp_couple = NULL;
  gchar **tcp_couple = NULL;

  /* Get the GTK structures and GConf client */
  gw = MyApp->GetMainWindow ();
  lw = MyApp->GetLdapWindow ();
  chat = MyApp->GetTextChat ();
  client = gconf_client_get_default ();
  vg = new PIntCondMutex (0, 0);
  
  /* Initialise the endpoint paramaters */
  video_grabber = NULL;
  SetCurrentConnection (NULL);
  SetCallingState (0);
  
#ifdef HAS_IXJ
  lid = NULL;
#endif
  ils_client = NULL;
  listener = NULL;

  docklet_timeout = 0;
  sound_timeout = 0;

  opened_audio_channels = 0;
  opened_video_channels = 0;


  /* Use IPv6 address family by default if available. */
#ifdef P_HAS_IPV6
  if (PIPSocket::IsIpAddressFamilyV6Supported())
    PIPSocket::SetDefaultIpAddressFamilyV6();
#endif
  
  rtp_port_range = 
    gconf_client_get_string (client, PORTS_KEY "rtp_port_range", NULL);
  udp_port_range = 
    gconf_client_get_string (client, PORTS_KEY "udp_port_range", NULL);
  tcp_port_range = 
    gconf_client_get_string (client, PORTS_KEY "tcp_port_range", NULL);

  if (rtp_port_range)
    rtp_couple = g_strsplit (rtp_port_range, ":", 0);
  if (udp_port_range)
    udp_couple = g_strsplit (udp_port_range, ":", 0);
  if (tcp_port_range)
    tcp_couple = g_strsplit (tcp_port_range, ":", 0);
  
  if (tcp_couple [0] && tcp_couple [1]) {

    SetTCPPorts (atoi (tcp_couple [0]), atoi (tcp_couple [1]));
    PTRACE (1, "Set TCP port range to " << atoi (tcp_couple [0])
	    << atoi (tcp_couple [1]));
  }

  if (rtp_couple [0] && rtp_couple [1]) {

    SetRtpIpPorts (atoi (rtp_couple [0]), atoi (rtp_couple [1]));
    PTRACE (1, "Set RTP port range to " << atoi (rtp_couple [0])
	    << atoi (rtp_couple [1]));
  }

  if (udp_couple [0] && udp_couple [1]) {

    SetUDPPorts (atoi (udp_couple [0]), atoi (udp_couple [1]));
    PTRACE (1, "Set UDP port range to " << atoi (udp_couple [0])
	    << atoi (udp_couple [1]));
  }

  g_free (tcp_port_range);
  g_free (udp_port_range);
  g_free (rtp_port_range);
  g_strfreev (tcp_couple);
  g_strfreev (udp_couple);
  g_strfreev (rtp_couple);

  received_video_device = NULL;
  transmitted_video_device = NULL;
  player_channel = NULL;
  recorder_channel = NULL;
  audio_tester = NULL;

  SetNoMediaTimeout (PTimeInterval (0, 15, 0));
  ILSTimer.SetNotifier (PCREATE_NOTIFIER(OnILSTimeout));

  ils_registered = false;

  /* Update general configuration */
  UpdateConfig ();
}


GMH323EndPoint::~GMH323EndPoint ()
{
  if (listener)
    RemoveListener (listener);

  PWaitAndSignal m(ils_access_mutex);
  /* Delete any ILS client which could be running */
  if (ils_client)
    delete (ils_client);

  /* Create a new one to unregister */
  ils_client = new GMILSClient ();
  ils_client->Unregister ();
  delete (ils_client);
}


H323Connection *
GMH323EndPoint::MakeCallLocked (const PString &call_addr, PString &call_token)
{
  called_address = call_addr;

  return H323EndPoint::MakeCallLocked (call_addr, call_token);
}


void GMH323EndPoint::UpdateConfig ()
{
  /* FIX ME : séparer en plusieurs morceau, virer updateconfig, bouger un 
     max de choses dans le constructeur. Seuls les devices doivent rester
     ici */
  gchar *player = NULL;
  gchar *recorder = NULL;
  gchar *manager = NULL;

  int video_size = 0;

  BOOL use_lid = FALSE;
  BOOL h245_tunneling = FALSE;
  BOOL fast_start = FALSE;

  GmPrefWindow *pw = NULL;
  GmWindow *gw = NULL;

  gnomemeeting_threads_enter ();
  pw = MyApp->GetPrefWindow ();
  gw = MyApp->GetMainWindow ();

  /* Get the gconf settings */
  manager =
    gconf_client_get_string (client, DEVICES_KEY "audio_manager", NULL);
  player = 
    gconf_client_get_string (client, DEVICES_KEY "audio_player", NULL);
  recorder = 
    gconf_client_get_string (client, DEVICES_KEY "audio_recorder", NULL);
  h245_tunneling = 
    gconf_client_get_bool (client, GENERAL_KEY "h245_tunneling", NULL);
  fast_start =
    gconf_client_get_bool (client, GENERAL_KEY "fast_start", NULL);
  video_size = 
    gconf_client_get_int (client, DEVICES_KEY "video_size", NULL);
  gnomemeeting_threads_leave ();

  gnomemeeting_sound_daemons_suspend ();

  /* Do not change these values during calls */
  if (GetCallingState () == 0) {

    if (PString (player).Find ("/dev/phone") != P_MAX_INDEX
	|| PString (recorder).Find ("/dev/phone") != P_MAX_INDEX) {
      
      use_lid = true;
    }
    

    /* Refreshes the prefs window */
#ifdef TRY_PLUGINS
    gnomemeeting_threads_enter ();
    if (manager && (GetSoundChannelManager () != PString (manager))) 
      gnomemeeting_pref_window_refresh_devices_list (NULL, NULL);
    gnomemeeting_threads_leave ();
#endif


    /**/
    /* Set recording source and set micro to record if no LID is used */
    if (!use_lid) {

      /* Change that setting only if needed */
      if (player && (GetSoundChannelPlayDevice () != PString (player))) 
	SetSoundChannelPlayDevice (player);

      /* Change that setting only if needed */
      if (recorder && (GetSoundChannelRecordDevice ()!= PString (recorder))) 
	SetSoundChannelRecordDevice (recorder);
    }
    

    /**/
    /* Update the H.245 Tunnelling and Fast Start Settings if needed */
    if (disableH245Tunneling != !h245_tunneling) {

      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (gw->history_text_view, 
			       h245_tunneling?
			       (gchar *)_("Enabling H.245 Tunnelling"):
			       (gchar *)_("Disabling H.245 Tunnelling"));
      gnomemeeting_threads_leave ();
      disableH245Tunneling = !h245_tunneling;
    }

    if (disableFastStart != !fast_start) {

      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (gw->history_text_view, 
			       fast_start?
			       (gchar *)_("Enabling Fast Start"):
			       (gchar *)_("Disabling Fast Start"));
      disableFastStart = !fast_start;
      gnomemeeting_threads_leave ();
    }


#ifdef HAS_IXJ
    /* Use the quicknet card if needed */
    if (use_lid) 
      CreateLid ();
    else
      RemoveLid ();
#endif


    /**/
    /* Update the capabilities */
    RemoveAllCapabilities ();
    AddAudioCapabilities ();
    AddVideoCapabilities (video_size);
    AddUserInputCapabilities ();
  }

  gnomemeeting_sound_daemons_resume ();

  g_free (manager);
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
  cs_access_mutex.Wait ();
  calling_state = i;
  cs_access_mutex.Signal ();
}


int 
GMH323EndPoint::GetCallingState (void)
{
  int cstate;

  cs_access_mutex.Wait ();
  cstate = calling_state;
  cs_access_mutex.Signal ();

  return cstate;
}


H323Connection * 
GMH323EndPoint::SetupTransfer (const PString & token,
			       const PString & call_identity,
			       const PString & remote_party,
			       PString & new_token,
			       void *)
{
  H323Connection *conn = NULL;

  conn =
    H323EndPoint::SetupTransfer (token, call_identity,
				 remote_party, new_token);

  SetTransferCallToken (new_token);
  
  return conn;
}


void 
GMH323EndPoint::AddVideoCapabilities (int video_size)
{
  BOOL enable_video_transmission = FALSE;
  BOOL enable_video_reception = TRUE;

  /* Enable video transmission / reception */
  gnomemeeting_threads_enter ();
  enable_video_transmission = 
    gconf_client_get_bool (client, 
			   VIDEO_SETTINGS_KEY "enable_video_transmission", 0);
  enable_video_reception = 
    gconf_client_get_bool (client, 
			   VIDEO_SETTINGS_KEY "enable_video_reception", 0);
  gnomemeeting_threads_leave ();

  /* Capabilities we can send and receive */
  if (enable_video_reception) {
    
    /* Add video capabilities */
    if (video_size == 1) {

      /* CIF Capability in first position */
      SetCapability (0, 1, new H323_H261Capability (0, 2, FALSE, FALSE, 6217));
      SetCapability (0, 1, new H323_H261Capability (4, 0, FALSE, FALSE, 6217));
    }
    else {

      SetCapability (0, 1, new H323_H261Capability (4, 0, FALSE, FALSE, 6217)); 
      SetCapability (0, 1, new H323_H261Capability (0, 2, FALSE, FALSE, 6217));
    }
  }
  else {

    if (enable_video_transmission) {
      
      /* Capabilities we can send (if enabled), but not receive */
      /* Add video capabilities */
      if (video_size == 1) {

	/* CIF Capability in first position */
	AddCapability (new H323_H261Capability (0, 2, FALSE, FALSE, 6217));
	AddCapability (new H323_H261Capability (4, 0, FALSE, FALSE, 6217));  
      }
      else {

	AddCapability (new H323_H261Capability (4, 0, FALSE, FALSE, 6217)); 
	AddCapability (new H323_H261Capability (0, 2, FALSE, FALSE, 6217));  
      }
    }
  }


  /* Enable video transmission */
  autoStartTransmitVideo = enable_video_transmission;
}


void
GMH323EndPoint::AddUserInputCapabilities ()
{
  int cap = 0;

  gnomemeeting_threads_enter ();
  cap =
    gconf_client_get_int (client, GENERAL_KEY "user_input_capability", 0);
  gnomemeeting_threads_leave ();
    
  if (cap == 3)
    capabilities.SetCapability (0, P_MAX_INDEX, new H323_UserInputCapability(H323_UserInputCapability::SignalToneH245));
  else if (cap == 2)
    capabilities.SetCapability(0, P_MAX_INDEX, new H323_UserInputCapability(H323_UserInputCapability::SignalToneRFC2833));
  else if (cap == 4) {
      
    PINDEX num = capabilities.SetCapability(0, P_MAX_INDEX, new H323_UserInputCapability(H323_UserInputCapability::HookFlashH245));
    capabilities.SetCapability(0, num+1, new H323_UserInputCapability(H323_UserInputCapability::BasicString));
      
  } else if (cap != 1)
    AddAllUserInputCapabilities(0, P_MAX_INDEX);
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

  PStringArray to_remove;
  PStringArray to_reorder;
  
  /* Read GConf settings */ 
  gnomemeeting_threads_enter ();
  codecs_data = 
    gconf_client_get_list (client, AUDIO_CODECS_KEY "codecs_list", 
			   GCONF_VALUE_STRING, NULL);
  g711_frames = 
    gconf_client_get_int (client, AUDIO_SETTINGS_KEY "g711_frames", NULL);
  gsm_frames = 
    gconf_client_get_int (client, AUDIO_SETTINGS_KEY "gsm_frames", NULL);
  gnomemeeting_threads_leave ();

  
#ifdef HAS_IXJ
  /* Add the audio capabilities provided by the LID Hardware */
  GMLid *l = NULL;
  if ((l = GetLid ())) {

    /* If the LID can do PCM16 we can use the software
       codecs like GSM too */
    use_pcm16_codecs = l->areSoftwareCodecsSupported ();

    if (use_pcm16_codecs)
      capabilities.Remove ("G.711");

    l->Unlock ();
  }
#endif


  if (use_pcm16_codecs && codecs_data) {


    SetCapability (0, 0, new SpeexNarrow5AudioCapability ());
    SetCapability (0, 0, new SpeexNarrow3AudioCapability ());

    SetCapability (0, 0, gsm_capa = new MicrosoftGSMAudioCapability);
    gsm_capa->SetTxFramesInPacket (gsm_frames);

    g711_capa = new H323_G711Capability (H323_G711Capability::muLaw);
    SetCapability (0, 0, g711_capa);
    g711_capa->SetTxFramesInPacket (g711_frames);

    g711_capa = new H323_G711Capability (H323_G711Capability::ALaw);
    SetCapability (0, 0, g711_capa);
    g711_capa->SetTxFramesInPacket (g711_frames);

    SetCapability (0, 0, gsm2_capa = new H323_GSM0610Capability);	
    gsm2_capa->SetTxFramesInPacket (gsm_frames);

    g72616_capa = 
      new H323_G726_Capability (*this, H323_G726_Capability::e_32k);
    SetCapability (0, 0, g72616_capa);

    SetCapability(0, 0, new H323_LPC10Capability (*this));
  }

  
#ifdef HAS_IXJ
  if ((l = GetLid ())) {

    OpalLineInterfaceDevice *lid_device = NULL;

    lid_device = l->GetLidDevice ();

    if (lid_device && lid_device->IsOpen ())
      H323_LIDCapability::AddAllCapabilities (*lid_device, capabilities, 0, 0);

    l->Unlock ();
  }
#endif
  
  
  /* Let's go */
  while (use_pcm16_codecs && codecs_data) {
    
    couple = g_strsplit ((gchar *) codecs_data->data, "=", 0);

    if (couple [0] && couple [1] != NULL) {

      if (!strcmp (couple [1], "0")) 
	to_remove.AppendString (couple [0]);
      else
	to_reorder.AppendString (couple [0]);
    }

    g_strfreev (couple);
    codecs_data = codecs_data->next;
  }


  capabilities.Remove (to_remove);
  capabilities.Reorder (to_reorder);

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

  gnomemeeting_threads_enter ();
  ip_translation = 
    gconf_client_get_bool (client, GENERAL_KEY "ip_translation", NULL);
  gnomemeeting_threads_leave ();

  if (ip_translation) {

    /* Ignore Ip translation for local networks */
    if ( !((remoteAddr.Byte1() == 192) && (remoteAddr.Byte2() == 168))
	 
	 && !((remoteAddr.Byte1() == 172) 
	      && ((remoteAddr.Byte2() >= 16)&&(remoteAddr.Byte2()<=31)))
	 
	 && !(remoteAddr.Byte1() == 10)) {

      gnomemeeting_threads_enter ();
      ip = 
	gconf_client_get_string (client, GENERAL_KEY "public_ip", NULL);
      gnomemeeting_threads_leave ();

      if (ip) {

	addr = PIPSocket::Address(ip);

	if (addr != PIPSocket::Address ("0.0.0.0"))
	  localAddr = addr;
      }

      g_free (ip);
    }
  }
}


BOOL 
GMH323EndPoint::StartListener ()
{
  int listen_port = 1720;


  listen_port = 
    gconf_client_get_int (client, PORTS_KEY "listen_port", NULL);

  /* Start the listener thread for incoming calls */
  listener =
    new H323ListenerTCP (*this, PIPSocket::GetDefaultIpAny(), listen_port);
   

  /* unsuccesfull */
  if (!H323EndPoint::StartListener (listener)) {

    delete listener;
    listener = NULL;

    return FALSE;
  }
   
  return TRUE;
}


void 
GMH323EndPoint::StartAudioTester ()
{
  if (!audio_tester)
    audio_tester = new GMAudioTester (this);
}


void 
GMH323EndPoint::StopAudioTester ()
{
  if (audio_tester) {
   
    (GM_AUDIO_TESTER (audio_tester))->Stop ();
    audio_tester = NULL;
  }
}


void
GMH323EndPoint::CreateVideoGrabber (BOOL start_grabbing,
				    BOOL synchronous)
{
  PWaitAndSignal m(vg_access_mutex);
  
  if (!video_grabber)
    video_grabber =
      new GMVideoGrabber (vg, start_grabbing, synchronous);
}


void
GMH323EndPoint::RemoveVideoGrabber (BOOL synchronous)
{
  PWaitAndSignal m(vg_access_mutex);

  if (video_grabber) {

    video_grabber->Close ();
  }      
  video_grabber = NULL;

  if (synchronous)
    vg->WaitCondition ();
}


GMVideoGrabber *
GMH323EndPoint::GetVideoGrabber ()
{
  GMVideoGrabber *vg = NULL;
  PWaitAndSignal m(vg_access_mutex);

  vg = video_grabber;

  if (vg)
    vg->Lock ();
  
  return vg;
}


H323Gatekeeper *
GMH323EndPoint::CreateGatekeeper(H323Transport * transport)
{
  return new H323GatekeeperWithNAT(*this, transport);
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

  cc_access_mutex.Wait ();
  con = current_connection;
  cc_access_mutex.Signal ();

  return con;
}


int
GMH323EndPoint::GetVideoChannelsNumber (void)
{
  return opened_video_channels;
}


void
GMH323EndPoint::ILSRegister (void)
{
  /* Force the Update */
  ILSTimer.RunContinuous (PTimeInterval (1));
}


void 
GMH323EndPoint::SetCurrentConnection (H323Connection *c)
{
  cc_access_mutex.Wait ();
  current_connection = c;
  cc_access_mutex.Signal ();
}


void 
GMH323EndPoint::SetCurrentCallToken (PString s)
{
  ct_access_mutex.Wait ();
  current_call_token = s;
  ct_access_mutex.Signal ();
}


PString 
GMH323EndPoint::GetCurrentCallToken ()
{
  PString c;

  ct_access_mutex.Wait ();
  c = current_call_token;
  ct_access_mutex.Signal ();

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
  gnomemeeting_threads_enter ();
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
  gnomemeeting_threads_leave ();

  if (forward_host_gconf)
    forward_host = PString (GMURL (forward_host_gconf).GetValidURL ());
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
  gnomemeeting_threads_leave ();
  g_free (msg);


  /* if we have enabled call forwarding for all calls, do the forward */
  if (!forward_host.IsEmpty() && always_forward) {

    msg = 
      g_strdup_printf (_("Forwarding call from %s to %s (Forward all calls)"),
		       (const char *) utf8_name, (const char *) forward_host);
    do_forward = TRUE;
  }
  else if (dnd) {

    /* dnd, so reject the call */
    msg =
      g_strdup_printf (_("Auto rejecting incoming call from %s (Do not disturb)"),
		       (const char *) utf8_name);
    
    do_reject = TRUE;
  }
  /* if we are already in a call: forward or reject */
  else if (GetCallingState () != 0) {

    /* if we have enabled forward when busy, do the forward */
    if (!forward_host.IsEmpty() && busy_forward) {

      msg = 
	g_strdup_printf (_("Forwarding call from %s to %s (Busy)"),
			 (const char *) utf8_name, 
			 (const char *) forward_host);

      do_forward = TRUE;
    } 
    else {

      /* there is no forwarding, so reject the call */
      msg = g_strdup_printf (_("Auto rejecting incoming call from %s (Busy)"),
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
      gnomemeeting_threads_leave ();

      connection.ClearCall (H323Connection::EndedByLocalBusy); 
      return FALSE;
    }
    else {

      gnomemeeting_threads_enter ();
      gnomemeeting_statusbar_push (gw->statusbar, _("Call Forwarded"));
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
    GMLid *l = NULL;
    
    if ((l = GetLid ())) {

      l->RingLine (1);
      l->Unlock ();
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
    gnomemeeting_threads_leave ();
  }
	

  /* If no forward or reject, update the internal state */
  SetCurrentCallToken (connection.GetCallToken ());
  SetCallingState (3);
  SetCurrentConnection (FindConnectionWithLock (GetCurrentCallToken ()));
  GetCurrentConnection ()->Unlock ();

  g_free (forward_host_gconf);
  g_free (utf8_name);
  g_free (utf8_app);

  return TRUE;
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
    msg = g_strdup_printf (_("Forwarding call to %s"),
			   (const char*) forward_party);
    gnomemeeting_statusbar_push (gw->statusbar, msg);
    gnomemeeting_log_insert (gw->history_text_view, msg);
    gnomemeeting_threads_leave ();
    g_free (msg);

    return TRUE;
  }
  else {

    msg = g_strdup_printf (_("Error while forwarding call to %s"),
			   (const char*) forward_party);
    gnomemeeting_threads_enter ();
    gnomemeeting_warning_dialog (GTK_WINDOW (gm), msg, _("There was an error when forwarding the call to the given host."));
    gnomemeeting_threads_leave ();

    g_free (msg);

    return FALSE;
  }

  
  return FALSE;
}


void 
GMH323EndPoint::OnConnectionEstablished (H323Connection & connection, 
                                         const PString & token)
{
  H323Codec *raw_codec = NULL;
  H323VideoCodec *video_codec = NULL;
  H323Channel *channel = NULL;
  
  PString local_name = GetLocalUserName ();
  PString name = connection.GetRemotePartyName();
  PString app = connection.GetRemoteApplication ();
  gchar *utf8_app = NULL;
  gchar *utf8_name = NULL;
  gchar *utf8_local_name = NULL;
  char *msg = NULL;
  BOOL reg = FALSE;
  int vq = 0;
  int bf = 0;
  int tr_fps = 0;
  int bitrate = 2;
  double frame_time = 0.0;

  
  /* Remote Name and application */
  utf8_app = gnomemeeting_get_utf8 (gnomemeeting_pstring_cut (app));
  utf8_name = gnomemeeting_get_utf8 (gnomemeeting_pstring_cut (name));
  utf8_local_name = 
    gnomemeeting_get_utf8 (gnomemeeting_pstring_cut (local_name));

  
  /* Get the gconf settings */
  gnomemeeting_threads_enter ();
  vq = gconf_client_get_int (client, VIDEO_SETTINGS_KEY "tr_vq", NULL);
  bf = gconf_client_get_int (client, VIDEO_SETTINGS_KEY "tr_ub", NULL);
  bitrate = gconf_client_get_int (client, VIDEO_SETTINGS_KEY "maximum_video_bandwidth", NULL);
  tr_fps = gconf_client_get_int (client, VIDEO_SETTINGS_KEY "tr_fps", NULL);
  reg = gconf_client_get_bool (client, LDAP_KEY "register", NULL);
  gnomemeeting_threads_leave ();


  /* Remove the progress timeout */
  if (gw->progress_timeout) {

    gnomemeeting_threads_enter ();
    gtk_timeout_remove (gw->progress_timeout);
    gw->progress_timeout = 0;
    gtk_widget_hide (gw->progressbar);
    gnomemeeting_threads_leave ();
  }
  
  
  /* Set Video Codecs Settings */
  vq = 25 - (int) ((double) vq / 100 * 24);
  frame_time = (unsigned) (1000.0/tr_fps);
  frame_time = PMAX (33, PMIN(1000000, frame_time));
  channel = connection.FindChannel (RTP_Session::DefaultVideoSessionID,
				    FALSE);

  if (channel)
    raw_codec = channel->GetCodec();

  if (raw_codec && raw_codec->IsDescendant (H323VideoCodec::Class())) {

    video_codec = (H323VideoCodec *) raw_codec;
  }
  
  if (video_codec) {

    /* The maximum quality corresponds to the lowest quality indice, 1
     * and the lowest quality corresponds to 24 */
    video_codec->SetTxMinQuality (1);
    video_codec->SetTxMaxQuality (vq);
    video_codec->SetBackgroundFill (bf);   
    video_codec->SetMaxBitRate (bitrate * 8 * 1024);
    video_codec->SetTargetFrameTimeMs ((unsigned int) frame_time);
    video_codec->SetVideoMode (H323VideoCodec::DynamicVideoQuality | 
			       H323VideoCodec::AdaptivePacketDelay |
			       video_codec->GetVideoMode());
  }


  /* Connected */
  gnomemeeting_threads_enter ();

  msg = g_strdup_printf (_("Connected with %s using %s"), 
			 utf8_name, utf8_app);
  gnomemeeting_statusbar_push (gw->statusbar, _("Connected"));
  gnomemeeting_log_insert (gw->history_text_view, 
			   disableFastStart ?
			   (gchar *)_("Fast start disabled"):
			   (gchar *)_("Fast start enabled"));
  gnomemeeting_log_insert (gw->history_text_view,
			   disableH245Tunneling ?
			   (gchar *)_("H.245 Tunnelling disabled"):
			   (gchar *)_("H.245 Tunnelling enabled"));
  gnomemeeting_log_insert (gw->history_text_view, msg);

  gtk_label_set_text (GTK_LABEL (gw->remote_name), (const char *) utf8_name);
  gtk_window_set_title (GTK_WINDOW (gw->remote_video_window), 
			(const char *) utf8_name);
  gtk_window_set_title (GTK_WINDOW (gw->local_video_window), 
			(const char *) utf8_local_name);


  /* set-on-top to True */
  if (gconf_client_get_bool (GCONF_CLIENT (client), 
			     VIDEO_DISPLAY_KEY "stay_on_top", NULL)) {
    gdk_window_set_always_on_top (GDK_WINDOW (gm->window), TRUE);
    gdk_window_set_always_on_top (GDK_WINDOW (gw->local_video_window->window), 
				  TRUE);
    gdk_window_set_always_on_top (GDK_WINDOW (gw->remote_video_window->window), 
				  TRUE);
  }


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
  GMLid *l = NULL;

  if ((l = GetLid ())) {

    l->RingLine (3);
    l->Unlock ();
  }
#endif

  
  /* Update internal state */
  SetCurrentCallToken (token);
  SetCurrentConnection (FindConnectionWithoutLocks (token));
  SetCallingState (2);

  gnomemeeting_threads_enter ();
  gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);
  connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 1);
  gnomemeeting_addressbook_update_menu_sensitivity ();
  gnomemeeting_call_menu_connect_set_sensitive (0, FALSE);
  gnomemeeting_call_menu_connect_set_sensitive (1, TRUE);
  gnomemeeting_call_menu_functions_set_sensitive (TRUE);
  gnomemeeting_tray_set_content (gw->docklet, 2);
  gnomemeeting_threads_leave ();

  /* Update ILS if needed */
  if (reg)
    ILSRegister ();

  g_free (msg);
  g_free (utf8_name);
  g_free (utf8_local_name);
  g_free (utf8_app);
}


void
GMH323EndPoint::GetRemoteConnectionInfo (H323Connection & connection,
					 gchar * & utf8_name,
					 gchar * & utf8_app,
					 gchar * & utf8_url)
{
  const H323Transport *transport = NULL;
  H323TransportAddress address;

  PINDEX idx;
  
  PString remote_ip;
  PString remote_name;
  PString remote_app;
  PString remote_alias;

    /* Get information about the remote user */
  remote_name = connection.GetRemotePartyName ();
  idx = remote_name.Find ("(");
  if (idx != P_MAX_INDEX) {
    
    remote_alias = remote_name.Mid (idx + 1);
    remote_alias =
      remote_alias.Mid (0, (remote_alias.Find (",") != P_MAX_INDEX) ?
			remote_alias.Find (",") : remote_alias.Find (")"));
  }
  remote_app = connection.GetRemoteApplication ();
  gnomemeeting_threads_enter ();
  if (gconf_client_get_int (client,
			    GATEKEEPER_KEY "registering_method", NULL) > 0
      && !remote_alias.IsEmpty ()) {

    if (!connection.GetRemotePartyNumber ().IsEmpty ())
      remote_ip = connection.GetRemotePartyNumber ();
    else
      remote_ip = remote_alias;
  }
  else {

    /* Get the remote IP to display in the calls history */
    transport = connection.GetSignallingChannel ();
    if (transport) 
      remote_ip = transport->GetRemoteAddress ().GetHostName ();
  }
  gnomemeeting_threads_leave ();

  remote_ip = GMURL ().GetDefaultURL () + remote_ip;
  utf8_app = gnomemeeting_get_utf8 (remote_app);
  utf8_name = gnomemeeting_get_utf8 (gnomemeeting_pstring_cut (remote_name));
  utf8_url = gnomemeeting_get_utf8 (remote_ip);
}


void 
GMH323EndPoint::OnConnectionCleared (H323Connection & connection, 
                                     const PString & clearedCallToken)
{
  gchar *msg_reason = NULL;
  
  gchar *utf8_url = NULL;
  gchar *utf8_name = NULL;
  gchar *utf8_app = NULL;

  PTimeInterval t;
  BOOL auto_clear_text_chat = FALSE;
  BOOL dnd = FALSE;
  BOOL reg = FALSE;
  BOOL preview = FALSE;
  
  ch_access_mutex.Wait ();
  player_channel = NULL;
  recorder_channel = NULL;
  ch_access_mutex.Signal ();
  
  if (connection.GetConnectionStartTime ().IsValid ())
    t = PTime () - connection.GetConnectionStartTime();

  GetRemoteConnectionInfo (connection, utf8_name, utf8_app, utf8_url);
  gnomemeeting_threads_enter ();
  if (t.GetSeconds () == 0 && connection.HadAnsweredCall ())
    gnomemeeting_calls_history_window_add_call (2, utf8_name,
						(const char *) utf8_url,
						"0", utf8_app);
  else
    if (connection.HadAnsweredCall ())
      gnomemeeting_calls_history_window_add_call (0, utf8_name,
						  (const char *) utf8_url,
						  t.AsString (2), utf8_app);
    else
      gnomemeeting_calls_history_window_add_call (1, utf8_name,
						  (const char*) called_address,
						  t.AsString (2), utf8_app);
  g_free (utf8_app);
  g_free (utf8_name);
  g_free (utf8_url);
  gnomemeeting_threads_leave ();

  /* Get GConf settings */
  gnomemeeting_threads_enter ();
  auto_clear_text_chat =
    gconf_client_get_bool (client, GENERAL_KEY "auto_clear_text_chat", NULL);
  dnd = gconf_client_get_bool (client, GENERAL_KEY "do_not_disturb", NULL);
  reg = gconf_client_get_bool (client, LDAP_KEY "register", NULL);
  preview = gconf_client_get_bool (client, DEVICES_KEY "video_preview", NULL);
  gnomemeeting_threads_leave ();


  /* If we are called because the current incoming call has ended and 
     not another call, ok, else do nothing */
  if (GetCurrentCallToken () == clearedCallToken) {

    if (!GetTransferCallToken ().IsEmpty ()) {

      SetCurrentCallToken (GetTransferCallToken ());
      SetTransferCallToken (PString ());
    }
    else {

      SetCurrentCallToken (PString ());
      SetTransferCallToken (PString ());
    }
  }
  else {
  
    return;
  }
  
  opened_video_channels = 0;
  opened_audio_channels = 0;

  gnomemeeting_threads_enter ();

  switch (connection.GetCallEndReason ()) {

  case H323Connection::EndedByLocalUser :
    msg_reason = g_strdup (_("Local user cleared the call"));
    break;
  case H323Connection::EndedByRemoteUser :
    msg_reason = g_strdup (_("Remote user cleared the call"));
    break;
  case H323Connection::EndedByRefusal :
    msg_reason = g_strdup (_("Remote user did not accept the call"));
    break;
  case H323Connection::EndedByNoAccept :
    msg_reason = g_strdup (_("Remote user did not accept the call"));
    break;
  case H323Connection::EndedByAnswerDenied :
    msg_reason = g_strdup (_("Local user did not accept the call"));
    break;
  case H323Connection::EndedByGatekeeper :
    msg_reason = g_strdup (_("The Gatekeeper cleared the call"));
    break;
  case H323Connection::EndedByNoAnswer :
    msg_reason = g_strdup (_("Call not answered in the required time"));
    break;
  case H323Connection::EndedByCallerAbort :
    msg_reason = g_strdup (_("Remote user has stopped calling"));
    break;
  case H323Connection::EndedByTransportFail :
    msg_reason = g_strdup (_("Abnormal call termination"));
    break;
  case H323Connection::EndedByConnectFail :
    msg_reason = g_strdup (_("Could not connect to remote host"));
    break;
  case H323Connection::EndedByNoBandwidth :
    msg_reason = g_strdup (_("Insufficient bandwidth"));
    break;
  case H323Connection::EndedByCapabilityExchange :
    msg_reason = g_strdup (_("No common codec"));
    break;
  case H323Connection::EndedByCallForwarded :
    msg_reason = g_strdup (_("Call forwarded"));
    break;
  case H323Connection::EndedBySecurityDenial :
    msg_reason = g_strdup (_("Security check Failed"));
    break;
  case H323Connection::EndedByLocalBusy :
    msg_reason = g_strdup (_("Local user is busy"));
    break;
  case H323Connection::EndedByRemoteBusy :
    msg_reason = g_strdup (_("Remote user is busy"));
    break;
  case H323Connection::EndedByRemoteCongestion :
    msg_reason = g_strdup (_("Congested link to remote party"));
    break;
  case H323Connection::EndedByLocalCongestion :
    msg_reason = g_strdup (_("Congested link to remote party"));
    break;
  case H323Connection::EndedByUnreachable :
    msg_reason = g_strdup (_("Remote user is unreachable"));
    break;
  case H323Connection::EndedByNoEndPoint :
    msg_reason = g_strdup (_("Remote user is not running GnomeMeeting"));
    break;
  case H323Connection::EndedByHostOffline :
    msg_reason = g_strdup (_("Remote host is offline"));
    break;
  case H323Connection::EndedByTemporaryFailure :
    msg_reason = g_strdup (_("Temporary failure"));
    break;
  case H323Connection::EndedByNoUser :
    msg_reason = g_strdup (_("User not found"));
    break;

  default :
    msg_reason = g_strdup (_("Call completed"));
  }

  gnomemeeting_log_insert (gw->history_text_view, msg_reason);

  gnomemeeting_main_window_enable_statusbar_progress (false);

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
    gnomemeeting_tray_set_content (gw->docklet, 2);
  else
    gnomemeeting_tray_set_content (gw->docklet, 0);
  gnomemeeting_threads_leave ();


  /* No need to do all that if we are simply receiving an incoming call
     that was rejected in connection.cpp because of(DND) */
  if (GetCallingState () != 3 && GetCallingState () != 1) {

    SetCallingState (0);

    /* Update ILS if needed */
    if (reg)
      ILSRegister ();

    /* Reset the Video Grabber, if preview, else close it */
    if (preview) {

      vg_access_mutex.Wait ();
      if (video_grabber) {
	
	video_grabber->Reset ();
	vg_access_mutex.Signal ();
      }
      else {
	
	vg_access_mutex.Signal ();
	CreateVideoGrabber ();
      }
    }
    else {
      
      RemoveVideoGrabber ();
      gnomemeeting_threads_enter ();
      gnomemeeting_init_main_window_logo (gw->main_video_image);
      gnomemeeting_zoom_submenu_set_sensitive (FALSE);
#ifdef HAS_SDL
      gnomemeeting_fullscreen_option_set_sensitive (FALSE);
#endif
      gnomemeeting_threads_leave ();
    }
  }


  /* Play Busy Tone */
#ifdef HAS_IXJ
  GMLid *l = NULL;
  if ((l = GetLid ())) {

    l->RingLine (2);
    l->Unlock ();
  }
#endif

 
  /* We update the stats part */
  gnomemeeting_threads_enter ();
  gtk_label_set_text (GTK_LABEL (gw->remote_name), "");

  gtk_widget_queue_draw_area (gw->stats_drawing_area, 0, 0, GTK_WIDGET (gw->stats_drawing_area)->allocation.width, GTK_WIDGET (gw->stats_drawing_area)->allocation.height);
  gtk_label_set_text (GTK_LABEL (gw->stats_label), _("Lost packets:\nLate packets:\nRound-trip delay:\nJitter buffer:"));

  
  /* We empty the text chat buffer */
  if (auto_clear_text_chat)
    gnomemeeting_text_chat_clear (NULL, chat);


  /* set-on-top to False */
  if (gconf_client_get_bool (GCONF_CLIENT (client), 
			     VIDEO_DISPLAY_KEY "stay_on_top", NULL)) {
   
    gdk_window_set_always_on_top (GDK_WINDOW (gm->window), FALSE);
    gdk_window_set_always_on_top (GDK_WINDOW (gw->local_video_window->window), 
				  FALSE);
    gdk_window_set_always_on_top (GDK_WINDOW (gw->remote_video_window->window), 
				  FALSE);
  }


  /* Disable disconnect, and the mute functions in the call menu */
  gnomemeeting_call_menu_connect_set_sensitive (0, TRUE);
  gnomemeeting_call_menu_connect_set_sensitive (1, FALSE);
  gnomemeeting_call_menu_functions_set_sensitive (FALSE);
  gnomemeeting_addressbook_update_menu_sensitivity ();
  connect_button_update_pixmap (GTK_TOGGLE_BUTTON (gw->connect_button), 0);


  /* Disable Remote Video and select the good section */
  gnomemeeting_video_submenu_set_sensitive (false, LOCAL_VIDEO, true);
  gnomemeeting_video_submenu_set_sensitive (false, REMOTE_VIDEO, true);
  gnomemeeting_video_submenu_select (0);


  /* Disable / enable buttons */
  if (!preview)
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


  /* Update internal state */
  SetCurrentConnection (NULL);
  SetCallingState (0);

  /* Display the call end reason in the statusbar */
  gdk_threads_enter ();
  gnomemeeting_statusbar_flash (gw->statusbar, msg_reason);
  g_free (msg_reason);
  gdk_threads_leave ();
}


void 
GMH323EndPoint::SavePicture (void)
{ 
  PTime ts = PTime ();
  GdkPixbuf *pic = NULL;
  gchar *prefix = NULL;
  gchar *dirname = NULL;
  gchar *filename = NULL;
  
  prefix = gconf_client_get_string (client, GENERAL_KEY "save_prefix", NULL);
  dirname = (gchar *) g_get_home_dir ();
  pic = gtk_image_get_pixbuf (GTK_IMAGE (gw->main_video_image));

  if (pic && prefix && dirname) {
    
    filename = g_strdup_printf ("%s/%s%.2d_%.2d_%.2d-%.2d%.2d%.2d.png",
				dirname, prefix,
				ts.GetYear(), ts.GetMonth(), ts.GetDay(),
				ts.GetHour(), ts.GetMinute(), ts.GetSecond());
	
    gdk_pixbuf_save (pic, filename, "png", NULL, NULL);
    g_free (prefix);
    g_free (filename);
  }
}


void GMH323EndPoint::SetUserNameAndAlias ()
{
  gchar *firstname = NULL;
  gchar *lastname = NULL;
  gchar *local_name = NULL;
  gchar *alias = NULL;
  
  /* Set the local User name */
  gnomemeeting_threads_enter ();
  firstname = 
    gconf_client_get_string (client,PERSONAL_DATA_KEY "firstname", NULL);
  lastname  = 
    gconf_client_get_string (client, PERSONAL_DATA_KEY "lastname", NULL);
  alias = 
    gconf_client_get_string (client, GATEKEEPER_KEY "gk_alias", NULL);  
  gnomemeeting_threads_leave ();

  if (firstname && lastname && strcmp (firstname, ""))  { 

    if (strcmp (lastname, ""))
      local_name = g_strconcat (firstname, " ", lastname, NULL);
    else
      local_name = g_strdup (firstname);

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


void 
GMH323EndPoint::OnILSTimeout (PTimer &, INT)
{
  PWaitAndSignal m(ils_access_mutex);

  if (!ils_client) {
    
    ils_client = new GMILSClient ();

    if (gconf_client_get_bool (GCONF_CLIENT (client), LDAP_KEY "register", 0)){

      if (!ils_registered) {

	ils_client->Register ();
	ils_registered = true;
      }
      else {

	if (ILSTimer.GetResetTime ().GetMinutes () == 20) {

	  ils_client->Unregister ();
	  ils_client->Register ();
	  ils_registered = true;
	}
	else
	  ils_client->Modify ();
      }
    }
    else if (ils_registered) {

      ils_client->Unregister ();
      ils_registered = false;
    }

    delete (ils_client);
    ils_client = NULL;
  }  


  ILSTimer.RunContinuous (PTimeInterval (0, 0, 20));
}


BOOL 
GMH323EndPoint::OpenAudioChannel(H323Connection & connection,
                                 BOOL isEncoding,
                                 unsigned bufferSize,
                                 H323AudioCodec & codec)
{
  BOOL no_error = TRUE;

  /* Wait that the primary call has terminated (in case of transfer)
     before opening the channels for the second call */
  TransferCallWait ();

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
  GMLid *l = NULL;
  OpalLineInterfaceDevice *lid_device = NULL;

  if ((l = GetLid ())) 
    lid_device = l->GetLidDevice ();

  /* If we are using a hardware LID, connect the audio stream to the LID */
  if (lid_device && lid_device->IsOpen()) {

    l->RingLine (4);
    
    if (!codec.AttachChannel (new OpalLineChannel (*lid_device,
						   OpalIxJDevice::POTSLine,
						   codec))) {

      l->Unlock ();

      return FALSE;
    }
    else
      if (l)
	l->Unlock ();

    gnomemeeting_threads_enter ();
    gchar *msg = g_strdup_printf (_("Attaching lid hardware to codec"));
    gnomemeeting_log_insert (gw->history_text_view, msg);
    g_free (msg);    
    gnomemeeting_threads_leave ();
  }
  else
#endif
#ifdef TRY_PLUGINS
   {
     PSoundChannel *sound_channel = NULL;
     gchar *audio_manager = NULL;
     PString device_name;

     codec.SetSilenceDetectionMode (GetSilenceDetectionMode ());

     gnomemeeting_threads_enter ();
     audio_manager =
       gconf_client_get_string (client, DEVICES_KEY "audio_manager", 0);
     if (audio_manager)
       device_name = PString (audio_manager) + " ";
     gnomemeeting_threads_leave ();
     
     if (isEncoding) 
       device_name += GetSoundChannelRecordDevice ();
     else
       device_name += GetSoundChannelPlayDevice ();

     sound_channel = 
       PDeviceManager::GetOpenedSoundDevice (device_name, 
					     isEncoding? PDeviceManager::Input 
					     : PDeviceManager::Output, 
					     1, 8000, 16); 

     if (sound_channel) {

       sound_channel->SetBuffers (bufferSize, soundChannelBuffers);
       return codec.AttachChannel (sound_channel);
     }
     else
       no_error = FALSE;

   }
#else // not TRY_PLUGINS
  if (!H323EndPoint::OpenAudioChannel (connection,
				       isEncoding, 
				       bufferSize,
				       codec)) 
    no_error = FALSE;
#endif

  if (!no_error) {

    gnomemeeting_threads_enter ();

    if (isEncoding)
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Could not open audio channel for audio transmission"), _("An error occured while trying to record from the soundcard for the audio transmission. Please check that your soundcard is not busy and that your driver supports full-duplex.\nThe audio transmission has been disabled."));
    else
      gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Could not open audio channel for audio reception"), _("An error occured while trying to play audio to the soundcard for the audio reception. Please check that your soundcard is not busy and that your driver supports full-duplex.\nThe audio reception has been disabled."));
    gnomemeeting_threads_leave ();
  }

  
  return no_error;
}


BOOL
GMH323EndPoint::SetSoundChannelPlayDevice(const PString &name)
{
  gchar *text = NULL;

#ifdef TRY_PLUGINS
  gchar *audio_manager = NULL;

  gnomemeeting_threads_enter ();
  audio_manager =
    gconf_client_get_string (client, DEVICES_KEY "audio_manager", 0);
  gnomemeeting_threads_leave ();

  if (gw->audio_player_devices.GetSize () == 0) 
    gw->audio_player_devices = 
      PDeviceManager::GetDeviceNames (audio_manager, PDeviceManager::SoundOut);

  if (!audio_manager 
      || gw->audio_player_devices.GetValuesIndex (name) == P_MAX_INDEX) {

    g_free (audio_manager);
    return FALSE;
  }

  soundChannelManager = PString (audio_manager);

  g_free (audio_manager);
#else
  if (PSoundChannel::GetDeviceNames(PSoundChannel::Player).GetValuesIndex(name) == P_MAX_INDEX)
    return FALSE;
#endif

  soundChannelPlayDevice = name;

  text = g_strdup_printf (_("Set Audio player device to %s"), 
			  (const char *) soundChannelPlayDevice);
  gnomemeeting_threads_enter ();
  gnomemeeting_log_insert (gw->history_text_view, text);
  gnomemeeting_threads_leave ();

  g_free (text);

  return TRUE;   
}
 
 
BOOL
GMH323EndPoint::SetSoundChannelRecordDevice (const PString &name)
{
  gchar *text = NULL;

#ifdef TRY_PLUGINS
  gchar *audio_manager = NULL;

  gnomemeeting_threads_enter ();
  audio_manager =
    gconf_client_get_string (client, DEVICES_KEY "audio_manager", 0);
  gnomemeeting_threads_leave ();

  if (gw->audio_recorder_devices.GetSize () == 0) 
    gw->audio_recorder_devices = 
      PDeviceManager::GetDeviceNames (audio_manager, PDeviceManager::SoundIn);


  if (!audio_manager 
      || gw->audio_recorder_devices.GetValuesIndex (name) == P_MAX_INDEX) {

    g_free (audio_manager);
    return FALSE;
  }

  soundChannelManager = PString (audio_manager);

  g_free (audio_manager);
#else
  if (PSoundChannel::GetDeviceNames(PSoundChannel::Recorder).GetValuesIndex(name) == P_MAX_INDEX)
    return FALSE;
#endif

  soundChannelRecordDevice = name;

  text = g_strdup_printf (_("Set Audio player device to %s"), 
			  (const char *) soundChannelPlayDevice);
  gnomemeeting_threads_enter ();
  gnomemeeting_log_insert (gw->history_text_view, text);
  gnomemeeting_threads_leave ();

  g_free (text);

  return TRUE;
}


BOOL 
GMH323EndPoint::OpenVideoChannel (H323Connection & connection,
                                  BOOL isEncoding, 
                                  H323VideoCodec & codec)
{
  bool vid_tr = FALSE;
  bool result = FALSE;

  /* Wait that the primary call has terminated (in case of transfer)
     before opening the channels for the second call */
  TransferCallWait ();

 
  if (opened_video_channels >= 2)
    return FALSE;

  /* Get the gconf config */
  gnomemeeting_threads_enter ();
  vid_tr = 
    gconf_client_get_bool (client, 
			   VIDEO_SETTINGS_KEY "enable_video_transmission", 
			   NULL);
  gnomemeeting_threads_leave ();

  /* If it is possible to transmit and
     if the user enabled transmission and
     if OpenVideoDevice is called for the encoding */
  if (vid_tr && isEncoding) {

    vg_access_mutex.Wait ();
    if (!video_grabber) {

      vg_access_mutex.Signal ();
      CreateVideoGrabber (FALSE, TRUE); /* Do not grab and
					   start synchronously */
    }
    else {
      
      video_grabber->StopGrabbing ();
      vg_access_mutex.Signal ();
    }
    
    gnomemeeting_threads_enter ();
    /* Here, the grabber is opened */
    ch_access_mutex.Wait ();
    vg_access_mutex.Wait ();
    PVideoChannel *channel = video_grabber->GetVideoChannel ();
    transmitted_video_device = video_grabber->GetEncodingDevice ();
    vg_access_mutex.Signal ();
    opened_video_channels++;
    ch_access_mutex.Signal ();

    /* Updates the view menu */
    gnomemeeting_zoom_submenu_set_sensitive (TRUE);

#ifdef HAS_SDL
    gnomemeeting_fullscreen_option_set_sensitive (TRUE);
#endif

    if (opened_video_channels >= 2) 
      gnomemeeting_video_submenu_set_sensitive (TRUE, LOCAL_VIDEO, TRUE);
    else
      gnomemeeting_video_submenu_set_sensitive (TRUE, LOCAL_VIDEO, FALSE);


    gtk_widget_set_sensitive (GTK_WIDGET (gw->video_chan_button),
			      TRUE);     
    gnomemeeting_threads_leave ();

    ch_access_mutex.Wait ();
    if (channel)
      result = codec.AttachChannel (channel, FALSE);
    ch_access_mutex.Signal ();

    return result;
  }
  else {

    /* If we only receive */
    if (!isEncoding) {
       
      ch_access_mutex.Wait ();
      PVideoChannel *channel = new PVideoChannel;
      received_video_device = new GDKVideoOutputDevice (isEncoding, gw);
      received_video_device->SetColourFormatConverter ("YUV420P");      
      opened_video_channels++;
      channel->AttachVideoPlayer (received_video_device);
      ch_access_mutex.Signal ();


      /* Stop to grab */
      if (video_grabber)
	video_grabber->StopGrabbing ();
      
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

      ch_access_mutex.Wait ();
      if (channel)
	result = codec.AttachChannel (channel);
      ch_access_mutex.Signal ();
      
      return result;
    }
    else
      return FALSE;    
  }
}


#ifdef HAS_IXJ
GMLid *
GMH323EndPoint::GetLid (void)
{
  GMLid *l = NULL;

  lid_access_mutex.Wait ();
  l = lid;
  if (lid) 
    lid->Lock ();

  lid_access_mutex.Signal ();

  return l;
}


void
GMH323EndPoint::CreateLid (void)
{
  lid_access_mutex.Wait ();
  if (!lid)
    lid = new GMLid ();
  lid_access_mutex.Signal ();
}


void
GMH323EndPoint::RemoveLid (void)
{
  lid_access_mutex.Wait ();
  if (lid) {
    
    delete (lid);
    lid = NULL;
  }
  lid_access_mutex.Signal ();
}
#endif


void 
GMH323EndPoint::SetTransferCallToken (PString s)
{
  tct_access_mutex.Wait ();
  transfer_call_token = s;
  tct_access_mutex.Signal ();
}


PString 
GMH323EndPoint::GetTransferCallToken ()
{
  PString c;

  tct_access_mutex.Wait ();
  c = transfer_call_token;
  tct_access_mutex.Signal ();

  return c;
}


void
GMH323EndPoint::TransferCallWait ()
{

  while (!GetTransferCallToken ().IsEmpty ())
    PThread::Current ()->Sleep (100);
}
