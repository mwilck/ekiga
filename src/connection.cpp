
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
#include "callbacks.h"
#include "gnomemeeting.h"
#include "chat_window.h"
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
  int re_vq = 2;
  int re_vq_ = 2;

  if (opened_channels + 1 > 4)
    return FALSE;

  if (!H323Connection::OnStartLogicalChannel (channel))
    return FALSE;

  H323AudioCodec *codec = (H323AudioCodec *) channel.GetCodec ();

  gnomemeeting_threads_enter ();
  gnomemeeting_log_insert (gw->history_text_view,
			   _("Started New Logical Channel..."));
  gnomemeeting_threads_leave ();

  switch (channel.GetDirection ()) {
    
  case H323Channel::IsTransmitter :
    name = channel.GetCapability().GetFormatName();
    msg = g_strdup_printf (_("Sending %s"), (const char *) name);

    gnomemeeting_threads_enter ();
    gnomemeeting_log_insert (gw->history_text_view, msg);
    gnomemeeting_threads_leave ();
    
    g_free (msg);
    
    sd = 
      gconf_client_get_bool (client, "/apps/gnomemeeting/audio_settings/sd", 
			     NULL);
    	
    codec->SetSilenceDetectionMode(!sd ?
				   H323AudioCodec::NoSilenceDetection :
				   H323AudioCodec::AdaptiveSilenceDetection);

    if ((strcmp (name, "H.261-CIF"))&&(strcmp (name, "H.261-QCIF"))) {

      if (sd)
	msg = g_strdup_printf (_("Enabled silence detection for %s"), 
			       (const char *) name);
      else
	msg = g_strdup_printf (_("Disabled silence detection for %s"), 
			       (const char *) name);
      
      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (gw->history_text_view, msg);
      gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_chan_button),
				TRUE);
      gnomemeeting_threads_leave ();
    
      g_free (msg);
    }

    break;
  
  case H323Channel::IsReceiver :
    name = channel.GetCapability().GetFormatName();
    msg = g_strdup_printf (_("Receiving %s"), 
			   (const char *) name);
    
    gnomemeeting_threads_enter ();
    gnomemeeting_log_insert (gw->history_text_view, msg);
    gnomemeeting_threads_leave ();
    
    g_free (msg);
    
    break;
    
  default :
    break;
  }


  /* Compute the received video quality */
  re_vq_ = gconf_client_get_int (GCONF_CLIENT (client), 
				 "/apps/gnomemeeting/video_settings/re_vq", 
				 NULL);
  re_vq = 32 - (int) ((double) re_vq_ / 100 * 31);
    
  if (channel.GetDirection() == H323Channel::IsReceiver) {
      
    if (channel.GetCodec ()->IsDescendant(H323VideoCodec::Class()) 
	&& (re_vq >= 0)) {
	
      msg = 
	g_strdup_printf (_("Requesting remote to send video quality: %d%%"), 
			 re_vq_);
	
      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (gw->history_text_view, msg);
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
      gnomemeeting_log_insert (gw->history_text_view, _("Request ok"));
      gnomemeeting_threads_leave ();
    }  
  }

  opened_channels++;

  return TRUE;
}


void GMH323Connection::PauseChannel (int chan_num)
{
  H323Channel *audio_channel = NULL;
  H323Channel *video_channel = NULL;


  if (chan_num == 0) {

    audio_channel = MyApp->Endpoint ()->GetCurrentAudioChannel ();

    if (audio_channel != NULL) {

      g_signal_handlers_block_by_func (G_OBJECT (gw->audio_chan_button),
				       (gpointer) pause_audio_callback, gw);

      if (audio_channel->IsPaused ()) {

	audio_channel->SetPause (FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->audio_chan_button), FALSE);
	gnomemeeting_log_insert (gw->history_text_view,
				 _("Audio Channel:  Sending"));
	gnomemeeting_statusbar_flash (gm, 
				      _("Audio Channel:  Sending"));
      }
      else {

	audio_channel->SetPause (TRUE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->audio_chan_button), TRUE);
	gnomemeeting_log_insert (gw->history_text_view, 
				 _("Audio Channel:  Paused"));
	gnomemeeting_statusbar_flash (gm, 
				      _("Audio Channel:  Paused"));
      }

      g_signal_handlers_unblock_by_func (G_OBJECT (gw->audio_chan_button),
				       (gpointer) pause_audio_callback, gw);

    }
  }
  else {

    video_channel = MyApp->Endpoint ()->GetCurrentVideoChannel ();

    if (video_channel != NULL) {

      g_signal_handlers_block_by_func (G_OBJECT (gw->video_chan_button),
				       (gpointer) pause_video_callback, gw);

      if (video_channel->IsPaused ()) {

	video_channel->SetPause (FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->video_chan_button), FALSE);
	gnomemeeting_log_insert (gw->history_text_view,
				 _("Video Channel:  Sending"));
	gnomemeeting_statusbar_flash (gm,
				      _("Video Channel:  Sending"));
      }
      else {
	video_channel->SetPause (TRUE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->video_chan_button), TRUE);
	gnomemeeting_log_insert (gw->history_text_view,
				 _("Video Channel:  Paused"));
	gnomemeeting_statusbar_flash (gm,
				      _("Video Channel:  Paused"));
      }

      g_signal_handlers_unblock_by_func (G_OBJECT (gw->video_chan_button),
				       (gpointer) pause_video_callback, gw);
      
    }
  }
}


void GMH323Connection::UnPauseChannels ()
{
  H323Channel *audio_channel = NULL;
  H323Channel *video_channel = NULL;

  video_channel = MyApp->Endpoint ()->GetCurrentVideoChannel ();
  audio_channel = MyApp->Endpoint ()->GetCurrentAudioChannel ();

  if (audio_channel != NULL)
    audio_channel->SetPause (FALSE);

  if (video_channel != NULL)
    video_channel->SetPause (FALSE);
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
  GMVideoGrabber *vg = 
    (GMVideoGrabber *) MyApp->Endpoint ()->GetVideoGrabber ();
  vg->Stop ();


  PThread::Current ()->Sleep (500);
  
  if (gconf_client_get_bool 
      (client, "/apps/gnomemeeting/general/do_not_disturb", 0)) {

    gnomemeeting_threads_enter ();
    gnomemeeting_statusbar_flash (gm,
				  _("Auto Rejecting Incoming Call"));
    gnomemeeting_log_insert (gw->history_text_view,
			     _("Auto Rejecting Incoming Call"));
    gnomemeeting_log_insert (gw->calls_history_text_view, _("Auto Rejected"));
    gnomemeeting_threads_leave ();
    
    return AnswerCallDenied;
  }
  
  
  if (gconf_client_get_bool 
      (client, "/apps/gnomemeeting/general/auto_answer", 0)) {

    gnomemeeting_threads_enter ();
    gnomemeeting_statusbar_flash (gm,
				  _("Auto Answering Incoming Call"));
    gnomemeeting_log_insert (gw->history_text_view,
			     _("Auto Answering Incoming Call"));
    gnomemeeting_threads_leave ();
    
    return AnswerCallNow; 
  }
  
  return AnswerCallPending;
}


void GMH323Connection::OnUserInputString(const PString & value)
{
  PString val;
  PString remote = GetRemotePartyName ();
  GConfClient *client = gconf_client_get_default ();

  /* The remote party name has to be converted to UTF-8, but not
     the text */
  gchar *utf8_remote = NULL;

  PINDEX bracket = remote.Find(" [");
  if (bracket != P_MAX_INDEX)
    remote = remote.Left (bracket);

  bracket = remote.Find(" (");
  if (bracket != P_MAX_INDEX)
    remote = remote.Left (bracket);

  gnomemeeting_threads_enter ();

  
  /* The MCU sends MSG[remote] value as message, 
     check if we are not using the MCU */
  bracket = value.Find("[");

  if ((bracket != P_MAX_INDEX) && (bracket == 3)) {
    
    remote = value.Mid (bracket + 1, value.Find ("] ") - 4);
    bracket = value.Find ("] ");
    val = value.Mid (bracket + 1);
  }
  else
    val = value.Mid (3);
  
  utf8_remote = g_convert ((gchar *) (const unsigned char *)(remote), 
			   strlen ((const char*)(const unsigned char*)(remote)),
			   "UTF-8", "UCS-2", 0, 0, 0);
  gnomemeeting_text_chat_insert (utf8_remote, val, 1);
  g_free (utf8_remote);
  
  if (!GTK_WIDGET_VISIBLE (gw->chat_window))
    gconf_client_set_bool (client, "/apps/gnomemeeting/view/show_chat_window",
			   true, NULL);

  gnomemeeting_threads_leave ();
}

