
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2001 Damien Sandras
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
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains miscellaneous functions.
 *   email                : dsandras@seconix.com
 *
 */

#include "../config.h"


#include "toolbar.h"
#include "endpoint.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "config.h"
#include "common.h"
#include "connection.h"
#include "docklet.h"
#include "audio.h"
#include "videograbber.h"
#include "gatekeeper.h"
#include "callbacks.h"
#include "ils.h"
#include "misc.h"

#include <gconf/gconf-client.h>


#include "../pixmaps/computer.xpm"

#define new PNEW


/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;


/* The class */
GMH323EndPoint::GMH323EndPoint ()
{
  gw = gnomemeeting_get_main_window (gm);
  lw = gnomemeeting_get_ldap_window (gm);

  SetCurrentConnection (NULL);
  SetCallingState (0);
  
  video_grabber = NULL;
  listener = NULL;
  codecs_count = 1;

  docklet_timeout = 0;
  sound_timeout = 0;

  /* Start the ILSClient PThread, do not register to it */
  ils_client = new GMILSClient ();

  /* Start the video grabber thread */
  video_grabber = new GMVideoGrabber ();

  /* The gconf client */
  client = gconf_client_get_default ();

  clearCallOnRoundTripFail = TRUE;
  disableFastStart = 1;
  disableH245Tunneling = 
    !gconf_client_get_bool (client, 
			    "/apps/gnomemeeting/general/h245_tunneling", 0);
  
  received_video_device = NULL;
  transmitted_video_device = NULL;
}


GMH323EndPoint::~GMH323EndPoint ()
{
  /* We do not delete the webcam and the ils_client 
     threads here, but in the Cleaner thread that is
     called when the user chooses to quit... */
}


H323Capabilities GMH323EndPoint::RemoveCapability (PString name)
{
  capabilities.Remove (name);
  return capabilities;
}


void GMH323EndPoint::RemoveAllCapabilities ()
{
  codecs_count = 1;
  capabilities.RemoveAll ();
}


void GMH323EndPoint::SetCallingState (int i)
{
  calling_state = i;
}


int GMH323EndPoint::GetCallingState (void)
{
  return calling_state;
}


void GMH323EndPoint::AddVideoCapabilities (int video_size)
{
  gchar *msg = NULL;

  if (video_size == 1) {

    /* CIF Capability in first position */
    SetCapability(0, 1, 
		  new H323_H261Capability (0, 2, FALSE, FALSE, 6217));

    msg = g_strdup_printf (_("%d. H.261\n (CIF size)"), codecs_count);
    gnomemeeting_log_insert (msg);
    g_free (msg);

    codecs_count++;

    SetCapability(0, 1, 
		  new H323_H261Capability (4, 0, FALSE, FALSE, 6217));

    msg = g_strdup_printf (_("%d. H.261\n (QCIF size)"), codecs_count);
    gnomemeeting_log_insert (msg);
    g_free (msg);

    codecs_count++;
  }
  else {
    
    SetCapability(0, 1, 
		  new H323_H261Capability (4, 0, FALSE, FALSE, 6217));
    
    msg = g_strdup_printf (_("%d. H.261\n (QCIF size)"), codecs_count);
    gnomemeeting_log_insert (msg);
    g_free (msg);

    codecs_count++;
    
    SetCapability(0, 1, 
		  new H323_H261Capability (0, 2, FALSE, FALSE, 6217));

    msg = g_strdup_printf (_("%d. H.261\n (CIF size)"), codecs_count);
    gnomemeeting_log_insert (msg);
    g_free (msg);

    codecs_count++;
  }

  if (gconf_client_get_bool (client, "/apps/gnomemeeting/video_settings/enable_video_transmission", 0)) {
    
    autoStartTransmitVideo = TRUE;
  }
  else {
    
    autoStartTransmitVideo = FALSE;
  }
}

void GMH323EndPoint::AddAudioCapabilities ()
{
  gchar **codecs;
  gchar *clist_data = NULL;
  gchar *msg = NULL;

  gnomemeeting_log_insert (_("Usable codecs table:"));
	
  
  cout << "FIX ME: # of Frames in audio capabilities" << endl << flush;
  /* Add or not the audio capabilities */ 
  clist_data = gconf_client_get_string (client, "/apps/gnomemeeting/audio_codecs/list", NULL);
  
  codecs = g_strsplit (clist_data, ":", 0);

  for (int i = 0 ; codecs [i] != NULL ; i++) {
    
    gchar **couple = g_strsplit (codecs [i], "=", 0);

    if ((!strcmp (couple [0], "MS-GSM")) && (!strcmp (couple [1], "1"))) {
      
      MicrosoftGSMAudioCapability* gsm_capa; 
      
      SetCapability (0, 0, gsm_capa = new MicrosoftGSMAudioCapability);
      msg = g_strdup_printf (_("%d. MS-GSM\n(%d frames/packet)"), 
			     codecs_count, 5);
      gnomemeeting_log_insert (msg);
      codecs_count++;  
//      gsm_capa->SetTxFramesInPacket (5);
      
      g_free (msg);
    }

    if ((!strcmp (couple [0], "G.711-uLaw-64k"))&&(!strcmp (couple [1], "1"))) {
      
      H323_G711Capability *g711_capa; 
      
      SetCapability (0, 0, g711_capa = new H323_G711Capability 
		     (H323_G711Capability::muLaw));
      msg = g_strdup_printf (_("%d. G.711-uLaw\n (%d frames/packet)"), 
			     codecs_count, 5);
      gnomemeeting_log_insert (msg);
      codecs_count++;
  //    g711_capa->SetTxFramesInPacket (5);
	  
      g_free (msg);
    }

    if ((!strcmp (couple [0], "G.711-ALaw-64k"))&&(!strcmp (couple [1], "1"))) {
      
      H323_G711Capability *g711_capa; 
      
      SetCapability (0, 0, g711_capa = new H323_G711Capability 
		     (H323_G711Capability::ALaw));
      msg = g_strdup_printf (_("%d. G.711-ALaw\n (%d frames/packet)"), 
			     codecs_count, 10);
      gnomemeeting_log_insert (msg);
      codecs_count++;
    //  g711_capa->SetTxFramesInPacket (opts->g711_frames);
      
      g_free (msg);
    }
    
    if ((!strcmp (couple [0], "GSM-06.10"))&&(!strcmp (couple [1], "1"))) {
      
      H323_GSM0610Capability * gsm_capa; 
      
      SetCapability (0, 0, gsm_capa = new H323_GSM0610Capability);	
      msg = g_strdup_printf (_("%d. GSM-06.10\n (%d frames/packet)"), 
			     codecs_count, 10);
      gnomemeeting_log_insert (msg);
      codecs_count++;

      //gsm_capa->SetTxFramesInPacket (opts->gsm_frames);
      
      g_free (msg);
    }

    if ((!strcmp (couple [0], "LPC10"))&&(!strcmp (couple [1], "1"))) {
      
      SetCapability(0, 0, new H323_LPC10Capability (*this));
      msg = g_strdup_printf (_("%d. LPC10"), 
			     codecs_count);
      gnomemeeting_log_insert (msg);
      codecs_count++;
    
      g_free (msg);
    }
    
    g_strfreev (couple);
  }	
  
  g_strfreev (codecs);
  g_free (clist_data);
}


gchar *GMH323EndPoint::GetCurrentIP ()
{
  PIPSocket::InterfaceTable interfaces;
  PIPSocket::Address ip_addr;

  gchar *ip = NULL;

  if (!PIPSocket::GetInterfaceTable (interfaces)) 

    PIPSocket::GetHostAddress (ip_addr);
  else {

    for (unsigned int i = 0; i < interfaces.GetSize(); i++) {

      ip_addr = interfaces[i].GetAddress();

      if (ip_addr != 0  && 
	  ip_addr != PIPSocket::Address()) /* Ignore 127.0.0.1 */
	
	break;  	      
    }
  }

  ip = g_strdup ((const char *) ip_addr.AsString ());

  return ip;
}


BOOL GMH323EndPoint::StartListener ()
{
  /* Start the listener thread for incoming calls */
  listener = new H323ListenerTCP (*this, INADDR_ANY, 
				  1720);

  /* Succesfull ? */
  if (!H323EndPoint::StartListener (listener)) {

    delete listener;
    listener = NULL;

    return FALSE;
  }
   
  return TRUE;
}


H323Connection *GMH323EndPoint::CreateConnection (unsigned callReference)
{
  return new GMH323Connection (*this, callReference);
}


H323Connection *GMH323EndPoint::GetCurrentConnection ()
{
  return current_connection;
}


GMVideoGrabber *GMH323EndPoint::GetVideoGrabber (void)
{
  return (GMVideoGrabber *) video_grabber;
}


H323VideoCodec *GMH323EndPoint::GetCurrentVideoCodec (void)
{
  H323VideoCodec *video_codec = NULL;

  if (!GetCurrentCallToken ().IsEmpty()) {
    
    H323Connection * connection = GetCurrentConnection ();
    
    if (connection != NULL) {

      H323Channel * channel = 
	connection->FindChannel (RTP_Session::DefaultVideoSessionID, 
				 FALSE);

      if (channel != NULL) {
	
	H323Codec * raw_codec  = channel->GetCodec();

	if (raw_codec->IsDescendant (H323VideoCodec::Class())) {
	  
	  video_codec = (H323VideoCodec *) raw_codec;
	}
      }
    }
  } 
  
  return video_codec;
}


void GMH323EndPoint::ChangeSilenceDetection (void)
{
  if (!GetCurrentCallToken ().IsEmpty()) {
    
    H323Connection * connection = GetCurrentConnection ();
    
    if (connection != NULL) {

      H323Channel * chan = 
	connection->FindChannel (RTP_Session::DefaultAudioSessionID, 
				 FALSE);

      if (chan == NULL)

	gnomemeeting_log_insert (_("Could not find audio channel"));
      else {

	H323Codec * rawCodec  = chan->GetCodec();
	if (!rawCodec->IsDescendant (H323AudioCodec::Class()))
	  gnomemeeting_log_insert (_("Could not find audio channel"));
	else {
	  H323AudioCodec * codec = (H323AudioCodec *) rawCodec;
	  H323AudioCodec::SilenceDetectionMode mode = 
	    codec->GetSilenceDetectionMode();
	  
	  if (mode == H323AudioCodec::AdaptiveSilenceDetection) {

	    mode = H323AudioCodec::NoSilenceDetection;
	    gnomemeeting_log_insert (_("Disabled Silence Detection"));
	  } 
	  else {
	    mode = H323AudioCodec::AdaptiveSilenceDetection;
	    gnomemeeting_log_insert (_("Enabled Silence Detection"));
	  }

	  codec->SetSilenceDetectionMode(mode);
	}
      }
    } 
  }
}


PThread *GMH323EndPoint::GetILSClient (void)
{
  return ils_client;
}


void GMH323EndPoint::SetCurrentConnection (H323Connection *c)
{
  current_connection = c;
}


void GMH323EndPoint::SetCurrentCallToken (PString s)
{
  current_call_token = s;
}


PString GMH323EndPoint::GetCurrentCallToken ()
{
  return current_call_token;
}


H323Gatekeeper *GMH323EndPoint::GetGatekeeper ()
{
  return gatekeeper;
}


void GMH323EndPoint::GatekeeperRegister ()
{
  new GMH323Gatekeeper ();
}


BOOL GMH323EndPoint::OnIncomingCall (H323Connection & connection, 
				     const H323SignalPDU &, H323SignalPDU &)
{
  char *msg = NULL;
  PString name = connection.GetRemotePartyName();
  const char * remotePartyName = (const char *)name;  
  /* only a pointer => destroyed with the PString */

  msg = g_strdup_printf (_("Call from %s"), remotePartyName);

  gnomemeeting_threads_enter ();
  gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
		     (gchar *) msg);
			 
  gnomemeeting_log_insert (msg);
  gnomemeeting_threads_leave ();

  /* if we are already in a call */
  if (!(GetCurrentCallToken ().IsEmpty ())) {

    connection.ClearCall(H323Connection::EndedByLocalBusy);   
    return FALSE;
  }
  
  current_connection = FindConnectionWithLock
    (connection.GetCallToken ());
  current_connection->Unlock ();

  gnomemeeting_threads_enter ();

  if ((docklet_timeout == 0)) {

    docklet_timeout = gtk_timeout_add (1000, 
				       (GtkFunction) gnomemeeting_docklet_flash, 
				       gw->docklet);
  }

  if ((sound_timeout == 0) && (gconf_client_get_bool (client, "/apps/gnomemeeting/general/incoming_call_sound", 0))) {

    sound_timeout = gtk_timeout_add (1000, 
				     (GtkFunction) PlaySound,
				     gw->docklet);
  }

  if (gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_popup", 0) 
      && (!gconf_client_get_bool (client, "/apps/gnomemeeting/general/do_not_disturb", 0)) 
      && (!gconf_client_get_bool (client, "/apps/gnomemeeting/general/auto_answer", 0)) 
      && (GetCurrentCallToken ().IsEmpty ())) {

    GtkWidget *label = NULL;
    
    gw->incoming_call_popup = gnome_dialog_new (_("Incoming call"),
						_("Connect"), 
						_("Disconnect"),
						NULL);

    label = gtk_label_new (msg);
    gtk_box_pack_start (GTK_BOX 
			(GNOME_DIALOG (gw->incoming_call_popup)->vbox), 
			label, TRUE, TRUE, 0);
    
    gnome_dialog_button_connect (GNOME_DIALOG (gw->incoming_call_popup),
				 0, GTK_SIGNAL_FUNC (connect_cb), 
				 gw);
    
    gnome_dialog_button_connect (GNOME_DIALOG (gw->incoming_call_popup),
				 1, GTK_SIGNAL_FUNC (disconnect_cb), 
				 gw);
    
    gnome_dialog_set_close (GNOME_DIALOG (gw->incoming_call_popup), TRUE);
    gnome_dialog_set_default (GNOME_DIALOG (gw->incoming_call_popup), 0);
      

    /* Disable the possibility to connect/disconnect from menu and/or
       toolbar */
    gnomemeeting_disable_connect ();
    gnomemeeting_disable_disconnect ();

    gtk_widget_show (label);
    gtk_widget_show (gw->incoming_call_popup);
  }
  
  g_free (msg);  

  gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), FALSE);

  gnomemeeting_threads_leave ();


  SetCurrentCallToken (connection.GetCallToken());
  return TRUE;
}


void GMH323EndPoint::OnConnectionEstablished (H323Connection & connection, 
						const PString & token)
{
  PString name = connection.GetRemotePartyName();
  PString app = connection.GetRemoteApplication ();
  const char * remotePartyName = (const char *) name;
  const char * remoteApp = (const char *) app;
  char *msg;
  GdkPixmap *computer;
  GdkBitmap *computer_mask;

  gchar *data [2];

  gnomemeeting_threads_enter ();
  computer = gdk_pixmap_create_from_xpm_d (gm->window, &computer_mask,
					   NULL,
					   (gchar **) computer_xpm); 

  msg = g_strdup_printf (_("Connected with %s using %s"), 
			 remotePartyName, remoteApp);

  SetCurrentCallToken (token);
  SetCurrentConnection (FindConnectionWithoutLocks (token));

  gnome_appbar_push (GNOME_APPBAR (gw->statusbar), _("Connected"));

  if (gconf_client_get_bool (client, "/apps/gnomemeeting/general/fast_start", 0))    
    gnomemeeting_log_insert (_("Fast start enabled"));
  else
    gnomemeeting_log_insert (_("Fast start disabled"));

  if (disableH245Tunneling == 0)    
    gnomemeeting_log_insert (_("H.245 Tunnelling enabled"));
  else
    gnomemeeting_log_insert (_("H.245 Tunnelling disabled"));

  gnomemeeting_log_insert (msg);

  data [0] = "";
  data [1] = g_strdup ((gchar *) remotePartyName);
  gtk_clist_append (GTK_CLIST (gw->user_list), (gchar **) data);	
  g_free (data [1]);

  gtk_clist_set_pixmap (GTK_CLIST (gw->user_list), 
			0, 0, 
			computer, computer_mask);

  data [1] = g_strdup ((gchar *) remoteApp);
  gtk_clist_append (GTK_CLIST (gw->user_list), (gchar **) data);	
  g_free (data [1]);

  if (docklet_timeout != 0)
    gtk_timeout_remove (docklet_timeout);

  if (sound_timeout != 0)
    gtk_timeout_remove (sound_timeout);

  docklet_timeout = 0;
  sound_timeout = 0;

  gnomemeeting_docklet_set_content (gw->docklet, 0);

  gnomemeeting_threads_leave ();

  calling_state = 2;

  g_free (msg);
}


void GMH323EndPoint::OnConnectionCleared (H323Connection & connection, 
					  const PString & clearedCallToken)
{
  int exit = 0; /* do not exit */

  /* If we are called because the current call has ended and not another
     call, do nothing */
  if (GetCurrentCallToken () == clearedCallToken) 
    SetCurrentCallToken (PString ());
  else
    exit = 1;

  gnomemeeting_threads_enter ();

  switch (connection.GetCallEndReason ()) {

  case H323Connection::EndedByRemoteUser :
    gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
		       _("Remote party has cleared the call"));
    break;
    
  case H323Connection::EndedByCallerAbort :
    gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
		       _("Remote party has stopped calling"));
    break;

  case H323Connection::EndedByRefusal :
    gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
		       _("Remote party did not accept your call"));
    break;

  case H323Connection::EndedByNoAnswer :
    gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
		       _("Remote party did not answer your call"));
    break;
    
  case H323Connection::EndedByTransportFail :
    gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
		       _("This call ended abnormally"));
    break;
    
  case H323Connection::EndedByCapabilityExchange :
    gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
		       _("Could not find common codec with remote party"));
    break;

  case H323Connection::EndedByNoAccept :
    gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
			 _("Remote party did not accept your call"));
    break;

  case H323Connection::EndedByAnswerDenied :
    gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
		       _("Refused incoming call"));
    break;

  case H323Connection::EndedByNoUser :
    gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
		       _("User not found"));
    break;
    
  case H323Connection::EndedByNoBandwidth :
    gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
		       _("Call ended: insufficient bandwidth"));
    break;
    
  case H323Connection::EndedByConnectFail :
    switch (connection.GetSignallingChannel ()->GetErrorNumber ()) {

    case ENETUNREACH :
      gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
			 _("Remote party could not be reached"));
      break;
      
    case ETIMEDOUT :
      gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
			 _("Remote party is not online"));
      break;
      
    case ECONNREFUSED :
      gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
			 _("No phone running for remote party"));
      break;
      
    default :
      gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
			 _("Transport error calling"));
    }
    break;
    
  default :
    gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
		       _("Call completed"));
  }
  
  gnomemeeting_log_insert (_("Call completed"));

  gnomemeeting_threads_leave ();


 /* If we are called because the current call has ended and not another
     call, return */
  if (exit == 1) 
    return;

  gnomemeeting_threads_enter ();
  gtk_clist_clear (GTK_CLIST (gw->user_list));
  
  SetCurrentConnection (NULL);
  SetCallingState (0);
  
  /* Remove the timers if needed and clear the docklet */
  if (docklet_timeout != 0)
    gtk_timeout_remove (docklet_timeout);
  
  docklet_timeout = 0;
  
  if (sound_timeout != 0)
    gtk_timeout_remove (sound_timeout);
  
  sound_timeout = 0;

  gnomemeeting_docklet_set_content (gw->docklet, 0);
  
  gnomemeeting_threads_leave ();
  
  gnomemeeting_threads_enter ();

  /* Disable / enable buttons */
  if (!gconf_client_get_bool (client, "/apps/gnomemeeting/devices/video_preview", 0))
    gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_chan_button), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->silence_detection_button), 
			    FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_chan_button), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), TRUE);

  GTK_TOGGLE_BUTTON (gw->audio_chan_button)->active = FALSE;
  GTK_TOGGLE_BUTTON (gw->video_chan_button)->active = FALSE;
  GTK_TOGGLE_BUTTON (gw->silence_detection_button)->active = FALSE;

  gnomemeeting_enable_disconnect ();
  gnomemeeting_enable_connect ();

  SetCurrentDisplay (0);

  GtkWidget *object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (gm),
							 "display_uiinfo");

  GnomeUIInfo *display_uiinfo = (GnomeUIInfo *) object;
  
  GTK_CHECK_MENU_ITEM (display_uiinfo [0].widget)->active = TRUE;
  GTK_CHECK_MENU_ITEM (display_uiinfo [1].widget)->active = FALSE;
  GTK_CHECK_MENU_ITEM (display_uiinfo [2].widget)->active = FALSE;
  
  gnomemeeting_threads_leave ();

  /* Reset the Video Grabber, if preview, else close it */
  GMVideoGrabber *vg = (GMVideoGrabber *) video_grabber;

  if (gconf_client_get_bool (client, "/apps/gnomemeeting/devices/video_preview", 0)){
   
    vg->Reset ();
  }
  else {

    if (vg->IsOpened ())
      vg->Close ();
  }
}


void GMH323EndPoint::SetCurrentDisplay (int choice)
{ 
  display_config = choice;

  if (transmitted_video_device != NULL)
    transmitted_video_device->SetCurrentDisplay (choice);
  
  if (received_video_device != NULL)		
    received_video_device->SetCurrentDisplay (choice);
 
}


BOOL GMH323EndPoint::OpenAudioChannel(H323Connection & connection,
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

  /* Clear the docklet */
  gnomemeeting_docklet_set_content (gw->docklet, 0);

  gnomemeeting_threads_leave ();

  if (H323EndPoint::OpenAudioChannel(connection, isEncoding, 
				     bufferSize, codec))
    return TRUE;

  cerr << "Could not open sound device ";
  if (isEncoding)
    cerr << GetSoundChannelRecordDevice();
  else
    cerr << GetSoundChannelPlayDevice();
  cerr << " - Check permissions or full duplex capability." << endl;

  return FALSE;
}


BOOL GMH323EndPoint::OpenVideoChannel (H323Connection & connection,
				       BOOL isEncoding, 
				       H323VideoCodec & codec)
{
  GMVideoGrabber *vg = (GMVideoGrabber *) video_grabber;

  /* If it is possible to transmit and
     if the user enabled transmission and
     if OpenVideoDevice is called for the encoding */
  if ((gconf_client_get_bool (client, "/apps/gnomemeeting/video_settings/enable_video_transmission", 0))&&(isEncoding)) {

     if (!vg->IsOpened ())
       vg->Open (FALSE, TRUE); /* Do not grab, synchronous opening */

     /* Here, the grabber is opened */
     PVideoChannel *channel = vg->GetVideoChannel ();
     transmitted_video_device = vg->GetEncodingDevice ();

     vg->Stop ();
     
     gnomemeeting_threads_enter ();
     SetCurrentDisplay (0);
     
     GtkWidget *object = (GtkWidget *) 
       gtk_object_get_data (GTK_OBJECT (gm),
			    "display_uiinfo");

     GnomeUIInfo *display_uiinfo = (GnomeUIInfo *) object;
     
     GTK_CHECK_MENU_ITEM (display_uiinfo [0].widget)->active = TRUE;
     GTK_CHECK_MENU_ITEM (display_uiinfo [1].widget)->active = FALSE;
     GTK_CHECK_MENU_ITEM (display_uiinfo [2].widget)->active = FALSE;
     
     gnomemeeting_threads_leave ();
     
     /* Codecs Settings */
     if (gconf_client_get_bool (client, "/apps/gnomemeeting/video_settings/enable_vb", NULL))
       codec.SetAverageBitRate (1024 * gconf_client_get_int (client, "/apps/gnomemeeting/video_settings/video_bandwidth", NULL) * 8);
     else {
       
       codec.SetAverageBitRate (0); // Disable
       
       int vq = 32 - (int) ((double) gconf_client_get_int (client, "/apps/gnomemeeting/video_settings/tr_vq", NULL) / 100 * 31);

       codec.SetTxQualityLevel (vq);
       codec.SetBackgroundFill (gconf_client_get_int (client, "/apps/gnomemeeting/video_settings/tr_ub", 0));   
     }
     
     gnomemeeting_threads_enter ();
     gtk_widget_set_sensitive (GTK_WIDGET (gw->video_chan_button),
			       TRUE);
     
     GTK_TOGGLE_BUTTON (gw->video_chan_button)->active = TRUE;
     gnomemeeting_threads_leave ();

     return codec.AttachChannel (channel, FALSE); 
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
      SetCurrentDisplay (0);
      
      GtkWidget *object = (GtkWidget *) 
	gtk_object_get_data (GTK_OBJECT (gm),
			     "display_uiinfo");
      
      GnomeUIInfo *display_uiinfo = (GnomeUIInfo *) object;
      
      GTK_CHECK_MENU_ITEM (display_uiinfo [0].widget)->active = FALSE;
      GTK_CHECK_MENU_ITEM (display_uiinfo [1].widget)->active = TRUE;
      GTK_CHECK_MENU_ITEM (display_uiinfo [2].widget)->active = FALSE;
      
      SetCurrentDisplay (1); 
      gnomemeeting_threads_leave ();
      
      return codec.AttachChannel (channel);
    }
    else
      return FALSE;    
  }
}
