/***************************************************************************
                          endpoint.cpp  -  description
                             -------------------
    begin                : Sat Dec 23 2000
    copyright            : (C) 2000-2001 by Damien Sandras
    email                : dsandras@acm.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../config.h"

#include "toolbar.h"
#include "endpoint.h"
#include "main.h"
#include "main_interface.h"
#include "config.h"
#include "common.h"
#include "connection.h"
#include "docklet.h"
#include "audio.h"
#include "videograbber.h"
#include "gatekeeper.h"
#include "callbacks.h"
#include "ils.h"
#include <status-docklet.h>

#include "../pixmaps/computer.xpm"

#include <iostream.h> //

#define new PNEW

/******************************************************************************/
/* GTK Callbacks                                                              */
/******************************************************************************/


gint PlaySound (GtkWidget *widget)
{
  GtkWidget *object = NULL;

  if (widget != NULL)
  {
    // First we check if it is the phone or the globe that is displayed
    gdk_threads_enter ();
    object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (widget),
						"pixmapg");
    gdk_threads_leave ();
  }

  // If the applet contents the phone pixmap
  if (object == NULL)
    gnome_triggers_do ("", "program", "GnomeMeeting", 
		       "incoming_call", NULL);
  
  return TRUE;
}

/******************************************************************************/


/******************************************************************************/
/* Global Variables                                                           */
/******************************************************************************/

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;

/******************************************************************************/


/******************************************************************************/
/* The functions                                                              */
/******************************************************************************/

GMH323EndPoint::GMH323EndPoint (GM_window_widgets *w, options *o)
{
  opts = o;
  gw = w;

  SetCurrentConnection (NULL);
  SetCallingState (0);

  video_grabber = NULL;
  listener = NULL;
  grabber = NULL;

  // Start the ILSClient PThread, do not register to it
  ils_client = new GMILSClient (gw, opts);
  video_grabber = new GMVideoGrabber (gw, opts);
}


GMH323EndPoint::~GMH323EndPoint ()
{
  // We do not delete the webcam and the ils_client 
  // threads here, but in the Cleaner thread that is
  // called when the user chooses to quit...
}


H323Capabilities GMH323EndPoint::RemoveCapability (PString name)
{
  capabilities.Remove(name);
  return capabilities;
}


void GMH323EndPoint::SetCallingState (int i)
{
  calling_state = i;
}


int GMH323EndPoint::CallingState (void)
{
  return calling_state;
}


void GMH323EndPoint::RemoveAllCapabilities ()
{
  capabilities.RemoveAll ();
}


void GMH323EndPoint::AddVideoCapabilities (int video_size)
{
  if (video_size == 1)
    {
      /* CIF Capability in first position */
      SetCapability(0, 1, 
		    new H323_H261Capability (0, 2, FALSE, FALSE, 6217));
      GM_log_insert (gw->log_text, _("Added H.261 CIF capability"));
      
      SetCapability(0, 1, 
		    new H323_H261Capability (4, 0, FALSE, FALSE, 6217));
      GM_log_insert (gw->log_text, 
			    _("Added H.261 QCIF capability"));
    }
  else
    {
      SetCapability(0, 1, 
		    new H323_H261Capability (4, 0, FALSE, FALSE, 6217));
      GM_log_insert (gw->log_text, 
			    _("Added H.261 QCIF capability"));

      SetCapability(0, 1, 
		    new H323_H261Capability (0, 2, FALSE, FALSE, 6217));
      GM_log_insert (gw->log_text, _("Added H.261 CIF capability"));
    }
}


void GMH323EndPoint::AddAudioCapabilities ()
{
  char *key, *value;
  void *iterator;
  
  GM_log_insert (gw->log_text, _("Reinitialize the capabilities"));
	
  iterator = gnome_config_init_iterator("gnomemeeting/EnabledAudio");
  
  while (gnome_config_iterator_next  (iterator, &key, &value))
    {
      if ((!strcmp (key, "MS-GSM")) && (!strcmp (value, "1")))
	{
	  SetCapability (0, 0, new MicrosoftGSMAudioCapability);
	  GM_log_insert (gw->log_text, _("Added MS-GSM capability"));
	}

      if ((!strcmp (key, "G.711-uLaw-64k"))&&(!strcmp (value, "1")))
	{
	  SetCapability (0, 0, new H323_G711Capability 
			 (H323_G711Capability::muLaw));
	  GM_log_insert (gw->log_text, 
				_("Added G.711-uLaw capability"));
	}

      if ((!strcmp (key, "G.711-ALaw-64k"))&&(!strcmp (value, "1")))
	{
	  SetCapability (0, 0, new H323_G711Capability 
			 (H323_G711Capability::ALaw));
	  GM_log_insert (gw->log_text, 
				_("Added G.711-ALaw capability"));
	}

      if ((!strcmp (key, "GSM-06.10"))&&(!strcmp (value, "1")))
	{
	  SetCapability (0, 0, new H323_GSM0610Capability);	
	  GM_log_insert (gw->log_text, 
				_("Added GSM-06.10 capability"));
	}

      if ((!strcmp (key, "LPC10"))&&(!strcmp (value, "1")))
	{
	  SetCapability(0, 0, new H323_LPC10Capability(*this));
	  GM_log_insert (gw->log_text, _("Added LPC10 capability"));
	}

      g_free (key);
      g_free (value);
    }	
}


char *GMH323EndPoint::IP ()
{
  PIPSocket::InterfaceTable interfaces;
  PIPSocket::Address ipAddr;

  char *ip;

  ip = (char *) malloc (50);

  if (!PIPSocket::GetInterfaceTable(interfaces)) 
    PIPSocket::GetHostAddress(ipAddr);
  else
    {
      for (unsigned int i = 0; i < interfaces.GetSize(); i++) 
	{
	  ipAddr = interfaces[i].GetAddress();
	  if (ipAddr != 0  && 
	      ipAddr != PIPSocket::Address()) // Ignore 127.0.0.1
	    
	    break;  	      
	}
    }

  strcpy (ip, (const char *) ipAddr.AsString ());

  return ip;
}


BOOL GMH323EndPoint::StartListener ()
{
  //Start the listener thread for incoming calls
  listener = new H323ListenerTCP (*this, INADDR_ANY, atoi (opts->listen_port));

  if (!H323EndPoint::StartListener (listener))
    {
      delete listener;
      listener = NULL;
      return FALSE;
    }
   
  return TRUE;
}


BOOL GMH323EndPoint::Initialise ()
{
  gchar *name = NULL;

  // Set the various options
  SetCallingState (0);
  docklet_timeout = 0;
  sound_timeout = 0;

  if (strcmp (opts->firstname, ""))
    {
      // if firstname and surname
      if (strcmp (opts->surname, ""))
	name = g_strdup_printf ("%s %s", opts->firstname, opts->surname);
      else  // if only firstname
	name = g_strdup_printf ("%s", opts->firstname);

      SetLocalUserName (name);
      g_free (name);
    }
  
  received_video_device = NULL;
  transmitted_video_device = NULL;

  return TRUE;
}


void GMH323EndPoint::ReInitialise ()
{
  // Free the old options
  g_options_free (opts);

  // Set the various options
  read_config (opts);

  if (CallingState () == 0)
    Initialise ();
}


H323Connection *GMH323EndPoint::CreateConnection (unsigned callReference)
{
  return new GMH323Connection (*this, callReference, 
			       gw, opts);
}


H323Connection *GMH323EndPoint::Connection ()
{
  return current_connection;
}


GMVideoGrabber *GMH323EndPoint::GetVideoGrabber (void)
{
  return (GMVideoGrabber *) video_grabber;
}


void GMH323EndPoint::ChangeSilenceDetection (void)
{
  if (!CallToken ().IsEmpty())
    {
      H323Connection * connection = Connection ();
      
      if (connection != NULL) 
	{
	  H323Channel * chan = 
	    connection->FindChannel (RTP_Session::DefaultAudioSessionID, FALSE);

	  if (chan == NULL)
	    GM_log_insert (gw->log_text, _("Could not find audio channel"));
	  else 
	    {
	      H323Codec * rawCodec  = chan->GetCodec();
	      if (!rawCodec->IsDescendant (H323AudioCodec::Class()))
		GM_log_insert (gw->log_text, _("Could not find audio channel"));
	      else 
		{
                  H323AudioCodec * codec = (H323AudioCodec *) rawCodec;
		  H323AudioCodec::SilenceDetectionMode mode = codec->GetSilenceDetectionMode();

                  if (mode == H323AudioCodec::AdaptiveSilenceDetection) 
		    {
		      mode = H323AudioCodec::NoSilenceDetection;
		      GM_log_insert (gw->log_text, _("Disabled Silence Detection"));
		    } 
		  else 
		    {
		      mode = H323AudioCodec::AdaptiveSilenceDetection;
		      GM_log_insert (gw->log_text, _("Enabled Silence Detection"));
		    }
                  codec->SetSilenceDetectionMode(mode);
                }
	    }
	} 
    }
}


PVideoInputDevice *GMH323EndPoint::Grabber (void)
{
  return grabber;
}


PThread *GMH323EndPoint::get_ils_client (void)
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


PString GMH323EndPoint::CallToken ()
{
  return current_call_token;
}


H323Gatekeeper * GMH323EndPoint::Gatekeeper ()
{
  return gatekeeper;
}


void GMH323EndPoint::GatekeeperRegister ()
{
  new GMH323Gatekeeper (gw, opts);
}


BOOL GMH323EndPoint::OnIncomingCall (H323Connection & connection, 
				     const H323SignalPDU &, H323SignalPDU &)
{
  char *msg = NULL;
  PString name = connection.GetRemotePartyName();
  const char * remotePartyName = (const char *)name;  
  // only a pointer => destroyed with the PString

  msg = g_strdup_printf (_("Call from %s"), remotePartyName);

  gdk_threads_enter ();
  gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
		     (gchar *) msg);
			 
  GM_log_insert (gw->log_text, msg);

  if ((docklet_timeout == 0))
    {
      docklet_timeout = gtk_timeout_add (1000, 
					 (GtkFunction) docklet_flash, 
					 gw->docklet);
    }

  if ((sound_timeout == 0) && (opts->incoming_call_sound))
    {
      sound_timeout = gtk_timeout_add (2000, 
				       (GtkFunction) PlaySound,
				       STATUS_DOCKLET (gw->docklet)->plug);
    }

  if ((opts->popup) && (!opts->aa) && (!opts->dnd))
    {
      GtkWidget *msg_box = NULL;
      GtkWidget *label = NULL;

      msg_box = gnome_dialog_new (_("Incoming call"),
				  _("Connect"), _("Disconnect"),
				  NULL);

      label = gtk_label_new (msg);
      gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (msg_box)->vbox), 
			  label, TRUE, TRUE, 0);

      gnome_dialog_button_connect (GNOME_DIALOG (msg_box),
				   0, GTK_SIGNAL_FUNC (connect_cb), msg_box);

      gnome_dialog_button_connect (GNOME_DIALOG (msg_box),
				   1, GTK_SIGNAL_FUNC (disconnect_cb), msg_box);

      gtk_widget_show (label);
      gtk_widget_show (msg_box);
    }
  
  g_free (msg);  

  gdk_threads_leave ();

  if (CallToken ().IsEmpty ())
    {
      SetCurrentCallToken (connection.GetCallToken());
      return TRUE;
    }
  else
    return FALSE;	
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

  gdk_threads_enter ();
  computer = gdk_pixmap_create_from_xpm_d (gm->window, &computer_mask,
					   NULL,
					   (gchar **) computer_xpm); 

  msg = g_strdup_printf (_("Connected with %s using %s"), 
			 remotePartyName, remoteApp);

  SetCurrentCallToken (token);
  SetCurrentConnection (FindConnectionWithoutLocks (token));

  gnome_appbar_push (GNOME_APPBAR (gw->statusbar), _("Connected"));

  if (opts->fs == 1)    
    GM_log_insert (gw->log_text, _("Fast start enabled"));
  else
    GM_log_insert (gw->log_text, _("Fast start disabled"));

  if (opts->ht == 1)    
    GM_log_insert (gw->log_text, _("H.245 Tunnelling enabled"));
  else
    GM_log_insert (gw->log_text, _("H.245 Tunnelling disabled"));

  GM_log_insert (gw->log_text, msg);

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

  GM_docklet_set_content (gw->docklet, 0);

  gdk_threads_leave ();

  calling_state = 2;

  g_free (msg);
}


void GMH323EndPoint::OnConnectionCleared (H323Connection & connection, 
					  const PString & clearedCallToken)
{
  
  if (CallToken () == clearedCallToken)  SetCurrentCallToken (PString ());

  gdk_threads_enter ();
  switch (connection.GetCallEndReason ())
    {
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
      switch (connection.GetSignallingChannel ()->GetErrorNumber ())
	{
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

  GM_log_insert (gw->log_text, _("Call completed"));

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

  GM_docklet_set_content (gw->docklet, 0);

  gdk_threads_leave ();
  
  gdk_threads_enter ();

  /* Disable / enable buttons */
  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_chan_button), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->silence_detection_button), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_chan_button), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (gw->preview_button), TRUE);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->audio_chan_button), FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->video_chan_button), FALSE);

  enable_disconnect ();
  enable_connect ();

  GM_init_main_interface_logo (gw);
  
  gdk_threads_leave ();

  // Start the Video Grabber if video preview
  // else close the grabber
  GMVideoGrabber *vg = (GMVideoGrabber *) video_grabber;

  if (opts->vid_tr)
    {
      if (opts->video_preview)
	vg->Start ();
      else
	vg->Close ();
    }
}


void GMH323EndPoint::DisplayConfig (int choice)
{

  display_config = choice;
  if (transmitted_video_device != NULL)
    transmitted_video_device->DisplayConfig (choice);
  
  if (received_video_device != NULL)		
    received_video_device->DisplayConfig (choice);
 
}


BOOL GMH323EndPoint::OpenAudioChannel(H323Connection & connection,
				      BOOL isEncoding,
				      unsigned bufferSize,
				      H323AudioCodec & codec)
{
  GMH323Connection *c = (GMH323Connection *) Connection ();
  H323Channel *channel = (H323Channel *) c->GetTransmittedAudioChannel();

  gdk_threads_enter ();

  /* If needed , delete the timers */
  if (docklet_timeout != 0)
    gtk_timeout_remove (docklet_timeout);
  docklet_timeout = 0;

  if (sound_timeout != 0)
    gtk_timeout_remove (sound_timeout);
  sound_timeout = 0;

  /* Clear the docklet */
  GM_docklet_set_content (gw->docklet, 0);

  /* Enable the possibility to pause the transmission audio channel and to */
  /* change the silence detection mode                                     */
  if (isEncoding)
    {
      gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_chan_button),
				TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (gw->silence_detection_button),
				TRUE);

      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->audio_chan_button),
				    TRUE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->silence_detection_button),
				    opts->sd);
    }

  gdk_threads_leave ();

  /* */
  cout << channel->GetCapability().GetFormatName();

  codec.SetSilenceDetectionMode(!opts->sd ?
				H323AudioCodec::NoSilenceDetection :
				H323AudioCodec::AdaptiveSilenceDetection);

  if (H323EndPoint::OpenAudioChannel(connection, isEncoding, bufferSize, codec))
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
  /* If it is possible to transmit and
     if the user enabled transmission and
     if OpenVideoDevice is called for the encoding */
 if ((opts->vid_tr) && (isEncoding)) 
   {
     GMVideoGrabber *vg = (GMVideoGrabber *) video_grabber;

     if (!vg->IsOpened ())
	 vg->Open ();

     while (!vg->IsOpened ())
       usleep (100);

     PVideoChannel *channel = vg->GetVideoChannel ();
     transmitted_video_device = vg->GetEncodingDevice ();
     vg->Stop ();

     DisplayConfig (0);

     /* Codecs Settings */
     codec.SetTxQualityLevel (opts->tr_vq);
     codec.SetBackgroundFill (opts->tr_ub);   

     if (opts->video_bandwidth != 0)
       codec.SetAverageBitRate (1024 * PMAX (16, PMIN (2048, opts->video_bandwidth * 8)));

     gdk_threads_enter ();
     gtk_widget_set_sensitive (GTK_WIDGET (gw->video_chan_button),
			       TRUE);

     gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->video_chan_button),
				   TRUE);
     gdk_threads_leave ();
     
     return codec.AttachChannel (channel, FALSE); // do not close the channel at the end
   }
 else
   {
     /* If we only receive */
     if (!isEncoding)
       {       
	 PVideoChannel *channel = new PVideoChannel;
	 
	 received_video_device = new GDKVideoOutputDevice (isEncoding, gw);
	  
	 channel->AttachVideoPlayer (received_video_device);
	 
	 DisplayConfig (1); 
	 
         return codec.AttachChannel (channel);
       }
   }
 
 return FALSE;
}
/******************************************************************************/
