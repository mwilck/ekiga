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

#include "ldap_h.h"
#include "toolbar.h"
#include "endpoint.h"
#include "main.h"
#include "main_interface.h"
#include "config.h"
#include "common.h"
#include "connection.h"
#include "applet.h"
#include "splash.h"
#include "audio.h"
#include "webcam.h"

#include <iostream.h> //

#define new PNEW

/******************************************************************************/
/* GTK Callbacks                                                              */
/******************************************************************************/


gint PlaySound (GtkWidget *applet)
{
  GtkWidget *object = NULL;

  if (applet != NULL)
  {
    // First we check if it is the phone or the globe that is displayed
    gdk_threads_enter ();
    object = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (applet),
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

  // GM_window_widgets
  // as the pointer given as parameter will not be freed until the end
  // of the execution, we make a simple copy of it...
  gw = w;

  SetCurrentConnection (NULL);
  SetCallingState (0);
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


BOOL GMH323EndPoint::Initialise ()
{
  GtkWidget *msg_box;
  char *ip = NULL;
  char *name = NULL;

  // Set the various options
  SetCallingState (0);
  applet_timeout = 0;
  sound_timeout = 0;


  RemoveAllCapabilities ();

  if (strcmp (opts->firstname, ""))
    {
      name = (char *) malloc (50);
      strcpy (name, "");
      strncat (name, opts->firstname, 24);
      strcat (name, " ");
      if (strcmp (opts->surname, ""))
	strncat (name, opts->surname, 24);

      SetLocalUserName (name);
      AddAliasName (name); // for the gatekeeper
      free (name);
    }

  
  ip = IP ();

  if (opts->show_splash)
    GM_splash_advance_progress (gw->splash_win, 
				_("Registering to ILS directory"), 
				0.45);

  if (opts->ldap)
    GM_ldap_register ((char *) ip, gw);

  // Default codecs
  RemoveAllCapabilities ();

  if (opts->show_splash)
    GM_splash_advance_progress (gw->splash_win, _("Adding Audio Capabilities"), 
				0.60);
  AddAudioCapabilities ();

  if (opts->show_splash)
    GM_splash_advance_progress (gw->splash_win, _("Adding Video Capabilities"), 
				0.75);
  AddVideoCapabilities (opts->video_size);

	
  //Start the listener thread for incoming calls
  H323ListenerTCP *listener;
  if (opts->show_splash)
    GM_splash_advance_progress (gw->splash_win, _("Starting Listener"), 0.90);

  listener = new H323ListenerTCP (*this, INADDR_ANY, atoi (opts->listen_port));

	
  if (!StartListener (listener))
    {
      msg_box = gnome_message_box_new (_("Could not start listener."), 
				       GNOME_MESSAGE_BOX_ERROR, "OK", NULL);

      gnome_dialog_set_parent (GNOME_DIALOG (msg_box), GTK_WINDOW (gm));

      gtk_widget_show (msg_box);


      return FALSE;
    }	
  
  GM_log_insert (gw->log_text, _("Listener started"));

  // Set recording source and set micro to record
  SetSoundChannelPlayDevice(opts->audio_device);
  SetSoundChannelRecordDevice(opts->audio_device);
  GM_set_recording_source (opts->audio_mixer, 0); 
  
  received_video_device = NULL;
  transmitted_video_device = NULL;

  free (ip);
  return TRUE;
}


void GMH323EndPoint::ReInitialise ()
{
  char *name = NULL;

  // Free the old options
  g_options_free (opts);

  // Set the various options
  read_config (opts);

  if (strcmp (opts->firstname, ""))
    {
      name = (char *) malloc (50);
      strcpy (name, "");
      strncat (name, opts->firstname, 24);
      strcat (name, " ");
      if (strcmp (opts->surname, ""))
	strncat (name, opts->surname, 24);

      SetLocalUserName (name);
      free (name);
    }
 
  // Default codecs
  RemoveAllCapabilities ();
  AddAudioCapabilities ();
  AddVideoCapabilities (opts->video_size);
  
  received_video_device = NULL;
  transmitted_video_device = NULL;
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


BOOL GMH323EndPoint::OnIncomingCall (H323Connection & connection, 
				     const H323SignalPDU &, H323SignalPDU &)
{
  char *msg = (char *) calloc (200, 1);
  PString name = connection.GetRemotePartyName();
  const char * remotePartyName = (const char *)name;  
  // only a pointer => destroyed with the PString

  strncpy ((char *) msg, _("Call from: "), 200);
  strcat ((char *) msg, (char *) remotePartyName);
  

  gdk_threads_enter ();
  gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
		     (gchar *) msg);

  if (gw->applet != NULL)
    applet_widget_set_tooltip (APPLET_WIDGET (gw->applet),  
			       (gchar *) msg);

  GM_log_insert (gw->log_text, msg);

  if ((applet_timeout == 0) && (gw->applet != NULL))
    {
      applet_timeout = gtk_timeout_add (1000, 
					(GtkFunction) AppletFlash, 
					gw->applet);
    }

  if ((sound_timeout == 0) && (opts->incoming_call_sound))
    {
      sound_timeout = gtk_timeout_add (1000, 
				       (GtkFunction) PlaySound,
				       gw->applet);
    }

  gdk_threads_leave ();

  free (msg);  

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
  const char * remotePartyName = (const char *)name;
  const char * remoteApp = (const char *)app;
  char *msg;

  char cname [40];
  char capp[80];
  gchar *data [1];

  msg = (char *) malloc (300);

  strncpy ((char *) cname, (char *) remotePartyName, 39);
  strncpy ((char *) capp, (char *) remoteApp, 79);
	
  strcpy (msg, "");
  strcat (msg, _("Connected with "));
  strcat (msg, cname);
  strcat (msg, _(" using "));
  strcat (msg, capp);

  SetCurrentCallToken (token);
  SetCurrentConnection (FindConnectionWithoutLocks (CallToken ()));


  gdk_threads_enter ();
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

  data [0] = (gchar *) cname;
  gtk_clist_append (GTK_CLIST (gw->user_list), (gchar **) data);	

  data [0] = (gchar *) capp;
  gtk_clist_append (GTK_CLIST (gw->user_list), (gchar **) data);	

  if (applet_timeout != 0)
    gtk_timeout_remove (applet_timeout);

  if (sound_timeout != 0)
    gtk_timeout_remove (sound_timeout);

  applet_timeout = 0;
  sound_timeout = 0;

  if (gw->applet != NULL)
    {
      GM_applet_set_content (gw->applet, 0);
      applet_widget_set_tooltip (APPLET_WIDGET (gw->applet), NULL);
    }

  gdk_threads_leave ();

  calling_state = 2;

  free (msg);
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


  /* Remove the timers if needed */
  if (applet_timeout != 0)
    gtk_timeout_remove (applet_timeout);

  applet_timeout = 0;

  if (sound_timeout != 0)
    gtk_timeout_remove (sound_timeout);

  sound_timeout = 0;

  if (gw->applet != NULL)
    {
      GM_applet_set_content (gw->applet, 0);
      applet_widget_set_tooltip (APPLET_WIDGET (gw->applet), NULL);
    }


  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame), FALSE);

  enable_disconnect ();
  enable_connect ();

  GM_init_main_interface_logo (gw);
  
  gdk_threads_leave ();
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
  gdk_threads_enter ();

  /* If needed , delete the timers */
  if (applet_timeout != 0)
    gtk_timeout_remove (applet_timeout);
  
  applet_timeout = 0;

  if (sound_timeout != 0)
    gtk_timeout_remove (sound_timeout);

  sound_timeout = 0;

  if (gw->applet != NULL)
    {
      GM_applet_set_content (gw->applet, 0);
      applet_widget_set_tooltip (APPLET_WIDGET (gw->applet), NULL);
    }

  gdk_threads_leave ();

  /* */
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
  GtkWidget *msg_box = NULL;
  int height, width, error = 0;

  /* If it is possible to transmit and
     if the user enabled transmission and
     if OpenVideoDevice is called for the encoding */
 if ((opts->vid_tr) && (isEncoding)) 
   {
     PVideoChannel      * channel = new PVideoChannel;
    
     if (opts->video_size == 0)
       {
	 height = 176;
	 width = 144;
       }
     else
       {
	 height = 352;
	 width = 288;
       }
     
     transmitted_video_device = new GDKVideoOutputDevice (isEncoding, gw);
   
     transmitted_video_device->SetFrameSize (height, width);

     codec.SetTxQualityLevel (opts->tr_vq);
     codec.SetBackgroundFill (opts->tr_ub);
    
     // Try and open the default grabber. If that fails, open
     // the Fake grabber
     gw->grabber = new PVideoInputDevice();
   
     if (gw->grabber->Open (opts->video_device, FALSE) &&
	 gw->grabber->SetVideoFormat 
       (opts->video_format ? PVideoDevice::NTSC : PVideoDevice::PAL) &&
         gw->grabber->SetChannel (opts->video_channel) &&
         gw->grabber->SetFrameRate (opts->tr_fps)  &&
         gw->grabber->SetFrameSize (height, width)) 
       {
	 // Quick hack for Philips Webcams (which should not be needed)
	 if (!gw->grabber->SetColourFormatConverter ("YUV411P"))
	     error = !gw->grabber->SetColourFormatConverter ("YUV420P");
       }
     else
       error = 1;

     if (error == 1)
       {


	 msg_box = gnome_message_box_new (_("Could not open the video device."), GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
  
	 gtk_widget_show (msg_box);

	 error = 1;

	 // delete the failed grabber and open the fake grabber
	 delete gw->grabber;

	 gw->grabber = new PFakeVideoInputDevice();
	 gw->grabber->SetColourFormatConverter ("YUV411P");
	 gw->grabber->SetVideoFormat (PVideoDevice::PAL);
	 gw->grabber->SetChannel (100);     //NTSC static image.
	 gw->grabber->SetFrameRate (10);
	 gw->grabber->SetFrameSize (height, width);
       }


     gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame), TRUE);
     gw->grabber->Start ();
     
     channel->AttachVideoReader (gw->grabber);
     channel->AttachVideoPlayer (transmitted_video_device);
     
     return codec.AttachChannel (channel, TRUE);
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
