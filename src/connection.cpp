/***************************************************************************
                          connection.h  -  description
                             -------------------
    begin                : Sat Dec 23 2000
    copyright            : (C) 2000-2001 by Damien Sandras
    description          : Connection functions
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
#include "connection.h"
#include "main.h"
#include "main_interface.h"
#include "config.h"
#include "endpoint.h"
#include "common.h"
#include "toolbar.h"

#define new PNEW


extern GnomeMeeting *MyApp;

/******************************************************************************/
/* The functions                                                              */
/******************************************************************************/

GMH323Connection::GMH323Connection (GMH323EndPoint & ep, unsigned callReference,
				    GM_window_widgets *w, options *o)
  :H323Connection(ep, callReference, 1, !o->ht)
{
  // Just assign pointers, no need to create a copy
  gw = w;
  opts = o;
  transmitted_audio = NULL;
  transmitted_video = NULL;
  opened_channels = 0;
}


void GMH323Connection::OnClosedLogicalChannel(H323Channel & channel)
{
  opened_channels--;
  H323Connection::OnClosedLogicalChannel(channel);
}


BOOL GMH323Connection::OnClosingLogicalChannel (H323Channel & channel)
{
  return TRUE;
}


BOOL GMH323Connection::OnStartLogicalChannel (H323Channel & channel)
{
  PString name;
  char *msg;
  char *quality;

  msg = (char *) malloc (200);
  quality = (char *) malloc (50);

  if (!H323Connection::OnStartLogicalChannel (channel))
    return FALSE;
  
  gdk_threads_enter ();
  GM_log_insert (gw->log_text,
			_("Started New Logical Channel..."));
  gdk_threads_leave ();
  
  switch (channel.GetDirection ())
    {
    case H323Channel::IsTransmitter :
      name = channel.GetCapability().GetFormatName();
      strcpy (msg, _("Sending "));
      strncat (msg, name, 190);

      if ((name == "H.261-CIF") || (name == "H.261-QCIF"))
	transmitted_video = &channel;
      else
	transmitted_audio = &channel;

      gdk_threads_enter ();
      GM_log_insert (gw->log_text, msg);
      gdk_threads_leave ();
      break;
      
    case H323Channel::IsReceiver :
      name = channel.GetCapability().GetFormatName();
      strcpy (msg, _("Receiving "));
      strncat (msg, name, 180);
      gdk_threads_enter ();
      GM_log_insert (gw->log_text, msg);
      gdk_threads_leave ();
      break;
      
    default :
      break;
    }

  if (channel.GetDirection() == H323Channel::IsReceiver) 
    {
      if (channel.GetCodec ()->IsDescendant(H323VideoCodec::Class()) 
	  && (opts->re_vq >= 0)) 
	{
	  gdk_threads_enter ();
	  strcpy (msg, "Requesting remote to send video quality : ");
	  sprintf (quality, "%d", opts->re_vq);
	  strcat (msg, quality);
	  strcat (msg, "/31");
	  GM_log_insert (gw->log_text, msg);
	  gdk_threads_leave ();

	  // kludge to wait for channel to ACK to be sent
	  PThread::Current()->Sleep(2000);
	  
	  H323ControlPDU pdu;
	  H245_CommandMessage & command = 
	    pdu.Build(H245_CommandMessage::e_miscellaneousCommand);
	  
	  H245_MiscellaneousCommand & miscCommand = command;
	  miscCommand.m_logicalChannelNumber = (unsigned) channel.GetNumber();
	  miscCommand.m_type.SetTag (H245_MiscellaneousCommand_type
				     ::e_videoTemporalSpatialTradeOff);
	  PASN_Integer & value = miscCommand.m_type;
	  value = opts->re_vq;
	  WriteControlPDU(pdu);
	  
	  gdk_threads_enter ();
	  GM_log_insert (gw->log_text, _("Request ok"));
	  gdk_threads_leave ();
	}  
    }
		

  free (msg);
  free (quality);

  opened_channels++;

  return TRUE;
}


void GMH323Connection::PauseChannel (int chan_num)
{
  if (chan_num == 0)
    {
      if (transmitted_audio != NULL)
	{
	  if (transmitted_audio->IsPaused ())
	    {
	      transmitted_audio->SetPause (FALSE);
	      GM_log_insert (gw->log_text, 
				    _("Audio Channel:  Sending"));
	      gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
				 _("Audio Channel:  Sending"));
	    }
	  else
	    {
	      transmitted_audio->SetPause (TRUE);
	      GM_log_insert (gw->log_text, 
				    _("Audio Channel:  Paused"));
	      gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
				 _("Audio Channel:  Paused"));
	    }
	}
    }
  else
    {
      if (transmitted_video != NULL)
	{
	  if (transmitted_video->IsPaused ())
	    {
	      transmitted_video->SetPause (FALSE);
	      GM_log_insert (gw->log_text, 
				    _("Video Channel:  Sending"));
	      gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
				 _("Video Channel:  Sending"));
	    }
	  else
	    {
	      transmitted_video->SetPause (TRUE);
	      GM_log_insert (gw->log_text, 
				    _("Video Channel:  Paused"));
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
  MyApp -> Endpoint () -> SetCurrentCallToken (GetCallToken());
  
  if (opts->dnd)
    {
      gdk_threads_enter ();
      gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
			 _("Auto Rejecting Incoming Call"));
      GM_log_insert (gw->log_text, _("Auto Rejecting Incoming Call"));
      enable_disconnect ();
      disable_connect ();
      gdk_threads_leave ();

      return AnswerCallDenied;
    }


  if (opts->aa)
    {
      gdk_threads_enter ();
      gnome_appbar_push (GNOME_APPBAR (gw->statusbar), 
			 _("Auto Answering Incoming Call"));
      GM_log_insert (gw->log_text, _("Auto Answering Incoming Call"));

      enable_disconnect ();
      disable_connect ();
      gdk_threads_leave ();

      return AnswerCallNow;
    }

  return AnswerCallPending;
}

/******************************************************************************/
