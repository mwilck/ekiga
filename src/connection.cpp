
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
 *                         connection.cpp  -  description
 *                         ------------------------------
 *   begin                : Sat Dec 23 2001
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains connection related functions.
 *
 */


#include "../config.h"

#include "connection.h"
#include "endpoint.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "chat_window.h"
#include "dialog.h"

#include <h323pdu.h>

#define new PNEW


/* Declarations */

extern GnomeMeeting *MyApp;
extern GtkWidget *gm;


/* The functions */
GMH323Connection::GMH323Connection (GMH323EndPoint & ep, 
				    unsigned callReference)
  :H323Connection(ep, callReference)
{
  int min_jitter = 20;
  int max_jitter = 1000;

  gnomemeeting_threads_enter ();
  gw = MyApp->GetMainWindow ();

  opened_channels = 0;

  min_jitter = 
    gconf_client_get_int (gconf_client_get_default (), 
			  AUDIO_SETTINGS_KEY "min_jitter_buffer", NULL);

  max_jitter = 
    gconf_client_get_int (gconf_client_get_default (), 
			  AUDIO_SETTINGS_KEY "max_jitter_buffer", NULL);
  gnomemeeting_threads_leave ();

  SetAudioJitterDelay (min_jitter, max_jitter);
}


void GMH323Connection::OnClosedLogicalChannel(H323Channel & channel)
{
  opened_channels--;
  H323Connection::OnClosedLogicalChannel (channel);
}


BOOL GMH323Connection::OnStartLogicalChannel (H323Channel & channel)
{
  GConfClient *client = NULL;
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
  client = gconf_client_get_default ();
  gnomemeeting_log_insert (gw->history_text_view,
			   _("Started New Logical Channel..."));
  gnomemeeting_threads_leave ();

  switch (channel.GetDirection ()) {
    
  case H323Channel::IsTransmitter :
    name = channel.GetCapability().GetFormatName();
    msg = g_strdup_printf (_("Sending %s"), (const char *) name);

    gnomemeeting_threads_enter ();
    gnomemeeting_log_insert (gw->history_text_view, msg);
   
    g_free (msg);
    
    sd = 
      gconf_client_get_bool (client, "/apps/gnomemeeting/audio_settings/sd", 
			     NULL);
    gnomemeeting_threads_leave ();
    	
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
  gnomemeeting_threads_enter ();
  re_vq_ = gconf_client_get_int (GCONF_CLIENT (client), 
				 "/apps/gnomemeeting/video_settings/re_vq", 
				 NULL);
  gnomemeeting_threads_leave ();
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


H323Connection::AnswerCallResponse
GMH323Connection::OnAnswerCall (const PString & caller,
				const H323SignalPDU &,
				H323SignalPDU &)
{
  GConfClient *client = NULL;
  BOOL aa = FALSE;

  MyApp -> Endpoint () -> SetCurrentCallToken (GetCallToken());

  gnomemeeting_threads_enter ();  
  client = gconf_client_get_default ();
  aa = 
    gconf_client_get_bool (client, "/apps/gnomemeeting/general/auto_answer",
			   NULL); 
  gnomemeeting_threads_leave ();

  if (aa) {

    gnomemeeting_threads_enter ();  
    gnomemeeting_statusbar_flash (gw->statusbar,
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
  PINDEX bracket;
  GConfClient *client = gconf_client_get_default ();

  /* The remote party name has to be converted to UTF-8, but not
     the text */
  gchar *utf8_remote = NULL;

  /* The MCU sends MSG[remote] value as message, 
     check if we are not using the MCU */
  bracket = value.Find("[");

  if ((bracket != P_MAX_INDEX) && (bracket == 3)) {
    
    remote = value.Mid (bracket + 1, value.Find ("] ") - 4);
    bracket = value.Find ("] ");
    val = value.Mid (bracket + 1);
  }
  else {

    if (value.Find ("MSG") != P_MAX_INDEX)
      val = value.Mid (3);
    else
      return;
  }

  /* If the remote name can be converted, use the conversion,
     else (Netmeeting), suppose it is ISO-8859-1 */  
  remote = gnomemeeting_pstring_cut (remote);
  if (g_utf8_validate ((gchar *) (const unsigned char*) remote, -1, NULL))
    utf8_remote = g_strdup ((char *) (const unsigned char *) (remote));
  else
    utf8_remote = gnomemeeting_from_iso88591_to_utf8 (remote);

  gnomemeeting_threads_enter ();
  if (utf8_remote && strcmp (utf8_remote, "")) 
    gnomemeeting_text_chat_insert (utf8_remote, val, 1);
  
  if (!GTK_WIDGET_VISIBLE (gw->chat_window))
    gconf_client_set_bool (client, "/apps/gnomemeeting/view/show_chat_window",
			   true, NULL);

  g_free (utf8_remote);
  gnomemeeting_threads_leave ();
}


BOOL GMH323Connection::OnReceivedFacility(const H323SignalPDU & pdu)
{
  if (pdu.m_h323_uu_pdu.m_h323_message_body.GetTag() == H225_H323_UU_PDU_h323_message_body::e_empty)
    return TRUE;

  if (pdu.m_h323_uu_pdu.m_h323_message_body.GetTag() != H225_H323_UU_PDU_h323_message_body::e_facility)
    return FALSE;
  const H225_Facility_UUIE & fac = pdu.m_h323_uu_pdu.m_h323_message_body;

  // Check that it has the H.245 channel connection info
  if (fac.m_reason.GetTag() == H225_FacilityReason::e_startH245 &&
		  fac.HasOptionalField(H225_Facility_UUIE::e_h245Address) &&
			fac.m_protocolIdentifier.GetValue().IsEmpty()) {
    if (controlChannel != NULL && !controlChannel->IsOpen()) {
        PTRACE(2, "H225\tSimultaneous start of H.245 channel, connecting to remote.");
        controlChannel->CleanUpOnTermination();
        delete controlChannel;
        controlChannel = NULL;
    }
  }
  return H323Connection::OnReceivedFacility(pdu);
}


void 
GMH323Connection::HandleCallTransferFailure (const int returnError)
{
  GmWindow *gw = NULL;
  
  gnomemeeting_threads_enter ();
  gw = MyApp->GetMainWindow ();
  gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Call transfer failed"), _("The remote user tried to transfer your call to another user, but it failed."));
  gnomemeeting_log_insert (gw->history_text_view, _("Call transfer failed"));
  gnomemeeting_threads_leave ();
}
