
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
 *                         connection.cpp  -  description
 *                         ------------------------------
 *   begin                : Sat Dec 23 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains connection related functions.
 *   email                : dsandras@seconix.com
 *
 */


#include "../config.h"

#include "connection.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "config.h"
#include "endpoint.h"
#include "common.h"
#include "misc.h"

#include <gconf/gconf-client.h>

#define new PNEW


/* Declarations */

extern GnomeMeeting *MyApp;
extern GtkWidget *gm;


/* The functions */
GMH323Connection::GMH323Connection (GMH323EndPoint & ep, 
				    unsigned callReference)
  :H323Connection(ep, callReference)
{
  gw = gnomemeeting_get_main_window (gm);

  transmitted_audio = NULL;
  transmitted_video = NULL;
  opened_channels = 0;

  SetMaxAudioDelayJitter (gconf_client_get_int (gconf_client_get_default (), "/apps/gnomemeeting/audio_settings/jitter_buffer", NULL));
}


void GMH323Connection::OnClosedLogicalChannel(H323Channel & channel)
{
  opened_channels--;
  H323Connection::OnClosedLogicalChannel (channel);
}


BOOL GMH323Connection::OnStartLogicalChannel (H323Channel & channel)
{
  GConfClient *client = gconf_client_get_default ();
  PString name;
  gchar *msg = NULL;
  int sd = 0;
  int use_sd = 0;
  int re_vq = 2;
  int re_vq_ = 2;

  if (!H323Connection::OnStartLogicalChannel (channel))
    return FALSE;


  gnomemeeting_threads_enter ();
  gnomemeeting_log_insert (_("Started New Logical Channel..."));
  gnomemeeting_threads_leave ();

  switch (channel.GetDirection ()) {
    
  case H323Channel::IsTransmitter :
    name = channel.GetCapability().GetFormatName();
    msg = g_strdup_printf (_("Sending %s"), (const char *) name);

    if ((name == "H.261-CIF") || (name == "H.261-QCIF")) 
      transmitted_video = &channel;
    else
      transmitted_audio = &channel;

    gnomemeeting_threads_enter ();
    gnomemeeting_log_insert (msg);
    gnomemeeting_threads_leave ();
    
    g_free (msg);
    

    if ((name == "MS-GSM{sw}")||(name == "GSM-06.10{sw}")) {
  
      sd = gconf_client_get_bool (client, "/apps/gnomemeeting/audio_settings/gsm_sd", NULL);
      use_sd = 1;
    }

    if ((name == "G.711-ALaw-64k{sw}")||(name == "G.711-uLaw-64k{sw}"))	{

      sd = gconf_client_get_bool (client, "/apps/gnomemeeting/audio_settings/g711_sd", NULL);
      use_sd = 1;
    }
	
    if (use_sd == 1) {

      H323AudioCodec * codec = (H323AudioCodec *) channel.GetCodec ();
      codec->SetSilenceDetectionMode(!sd ?
				     H323AudioCodec::NoSilenceDetection :
				     H323AudioCodec::AdaptiveSilenceDetection);
      if (sd)
	msg = g_strdup_printf (_("Enabled silence detection for %s"), 
			       (const char *) name);
      else
	msg = g_strdup_printf (_("Disabled silence detection for %s"), 
			       (const char *) name);
      
      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (msg);
      gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_chan_button),
				TRUE);

      GTK_TOGGLE_BUTTON (gw->audio_chan_button)->active = TRUE;
      gtk_widget_draw (GTK_WIDGET (gw->audio_chan_button), NULL);
      gnomemeeting_threads_leave ();

      g_free (msg);
    }
    break;
    
  case H323Channel::IsReceiver :
    name = channel.GetCapability().GetFormatName();
    msg = g_strdup_printf (_("Receiving %s"), 
			   (const char *) name);
    
    gnomemeeting_threads_enter ();
    gnomemeeting_log_insert (msg);
    gnomemeeting_threads_leave ();

    g_free (msg);
    
    break;
      
  default :
    break;
  }


  /* Compute the received video quality */
  re_vq_ = gconf_client_get_int (GCONF_CLIENT (client), "/apps/gnomemeeting/video_settings/re_vq", NULL);
  re_vq = 32 - (int) ((double) re_vq_ / 100 * 31);

  if (channel.GetDirection() == H323Channel::IsReceiver) {

    if (channel.GetCodec ()->IsDescendant(H323VideoCodec::Class()) 
	&& (re_vq >= 0)) {

      msg = g_strdup_printf (_("Requesting remote to send video quality : %d%%"), 
			     re_vq_);

      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (msg);
      gnomemeeting_threads_leave ();

      g_free (msg);
				 
      /* kludge to wait for channel to ACK to be sent */
      PThread::Current()->Sleep(2000);
      
      H323ControlPDU pdu;
      H245_CommandMessage & command = 
	pdu.Build(H245_CommandMessage::e_miscellaneousCommand);
      
      H245_MiscellaneousCommand & miscCommand = command;
      miscCommand.m_logicalChannelNumber = (unsigned) channel.GetNumber();
      miscCommand.m_type.SetTag (H245_MiscellaneousCommand_type
				 ::e_videoTemporalSpatialTradeOff);
      PASN_Integer & value = miscCommand.m_type;
      value = re_vq;
      WriteControlPDU(pdu);
      
      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (_("Request ok"));
      gnomemeeting_threads_leave ();
    }  
  }
		
  opened_channels++;

  return TRUE;
}


void GMH323Connection::PauseChannel (int chan_num)
{
  if (chan_num == 0) {

    if (transmitted_audio != NULL) {

      if (transmitted_audio->IsPaused ()) {

	transmitted_audio->SetPause (FALSE);
	gnomemeeting_log_insert (_("Audio Channel:  Sending"));
	gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
			   _("Audio Channel:  Sending"));
      }
      else {

	transmitted_audio->SetPause (TRUE);
	gnomemeeting_log_insert (_("Audio Channel:  Paused"));
	gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
			   _("Audio Channel:  Paused"));
      }
    }
  }
  else {

    if (transmitted_video != NULL) {

      if (transmitted_video->IsPaused ()) {

	transmitted_video->SetPause (FALSE);
	gnomemeeting_log_insert (_("Video Channel:  Sending"));
	gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
			   _("Video Channel:  Sending"));
      }
      else {
	transmitted_video->SetPause (TRUE);
	gnomemeeting_log_insert (_("Video Channel:  Paused"));
	gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
			   _("Video Channel:  Paused"));
      }
    }
  }
}


void GMH323Connection::UnPauseChannels ()
{
  if (transmitted_audio != NULL)
    transmitted_audio->SetPause (FALSE);

  if (transmitted_video != NULL)
    transmitted_video->SetPause (FALSE);
}


H323Connection::AnswerCallResponse
GMH323Connection::OnAnswerCall (const PString & caller,
				const H323SignalPDU &,
				H323SignalPDU &)
{
  GConfClient *client = gconf_client_get_default ();
  MyApp -> Endpoint () -> SetCurrentCallToken (GetCallToken());

  /* We Make sure that the grabbing stops. We must do that here, 
     in OpenVideoChannel it is too late */
  GMVideoGrabber *vg = (GMVideoGrabber *) MyApp->Endpoint ()->GetVideoGrabber ();
  vg->Stop ();

  PThread::Current ()->Sleep (500);
  
  if (gconf_client_get_bool 
      (client, "/apps/gnomemeeting/general/do_not_disturb", 0)) {

    gnomemeeting_threads_enter ();
    gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
		       _("Auto Rejecting Incoming Call"));
    gnomemeeting_log_insert (_("Auto Rejecting Incoming Call"));
    gnomemeeting_threads_leave ();
    
    return AnswerCallDenied;
  }
  
  
  if (gconf_client_get_bool 
      (client, "/apps/gnomemeeting/general/auto_answer", 0)) {

    gnomemeeting_threads_enter ();
    gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
		       _("Auto Answering Incoming Call"));
    gnomemeeting_log_insert (_("Auto Answering Incoming Call"));
    gnomemeeting_threads_leave ();
    
    return AnswerCallNow;
  }
  
  return AnswerCallPending;
}


void GMH323Connection::OnUserInputString(const PString & value)
{
  gchar *msg = NULL;
  GdkColormap *cmap;
  GdkColor color;
  GdkFont *lucida_font;

  PString remote = GetRemotePartyName ();

  PINDEX bracket = remote.Find('[');
  if (bracket != P_MAX_INDEX)
    remote = remote.Left (bracket);

  bracket = remote.Find('(');
  if (bracket != P_MAX_INDEX)
    remote = remote.Left (bracket);

  gnomemeeting_threads_enter ();

  /* Get the system color map and allocate the color red */
  cmap = gdk_colormap_get_system();
  color.red = 0xffff;
  color.green = 0;
  color.blue = 0;
  gdk_color_alloc(cmap, &color);
  lucida_font = 
    gdk_font_load ("-b&h-lucida-bold-r-normal-*-*-100-*-*-p-*-iso8859-1");

  gtk_text_freeze (GTK_TEXT (gw->chat_text));
  msg = g_strdup_printf ("%s: ", (const char *) remote);
  gtk_text_insert (GTK_TEXT (gw->chat_text), lucida_font, &color, NULL, msg, -1);
  g_free (msg);

  msg = g_strdup_printf ("%s\n", (const char *) value);
  gtk_text_insert (GTK_TEXT (gw->chat_text), NULL, &gw->chat_text->style->black, 
		   NULL, msg, -1);
  g_free (msg);
  gtk_text_thaw (GTK_TEXT (gw->chat_text));

  gnomemeeting_threads_leave ();
}

