
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
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
 *                         config.cpp  -  description
 *                         --------------------------
 *   begin                : Wed Feb 14 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras 
 *   description          : This file contains most of config stuff.
 *                          All notifiers are here.
 *                          Callbacks that updates the config cache 
 *                          are in their file, except some generic one that
 *                          are in this file.
 *   Additional code      : Miguel Rodríguez Pérez  <migrax@terra.es>
 *
 */


#include "../config.h"

#include "config.h"
#include "h323endpoint.h"
#include "endpoint.h"
#include "gnomemeeting.h"
#include "lid.h"
#include "pref_window.h"
#include "main_window.h"
#include "log_window.h"
#include "tray.h"
#include "misc.h"
#include "tools.h"
#include "urlhandler.h"

#include "dialog.h"
#include "stock-icons.h"
#include "gtk_menu_extensions.h"
#include "gm_conf-widgets_extensions.h"



/* Declarations */
static void applicability_check_nt (gpointer,
				    GmConfEntry *,
				    gpointer);


/* DESCRIPTION  :  This callback is called when the control panel 
 *                 section key changes.
 * BEHAVIOR     :  Sets the right page or hide it, and also sets 
 *                 the good value for the radio menu.
 * PRE          :  /
 */
static void control_panel_section_changed_nt (gpointer, 
                                              GmConfEntry *, 
                                              gpointer);


/* DESCRIPTION  :  This callback is called when the show chat window
 *                 key changes.
 * BEHAVIOR     :  Shows/hides it and updates the menu item.
 * PRE          :  The main window GMObject. 
 */
static void show_chat_window_changed_nt (gpointer, 
					 GmConfEntry *, 
					 gpointer);


static void maximum_video_bandwidth_changed_nt (gpointer, 
                                                GmConfEntry *, 
                                                gpointer);

static void tr_vq_changed_nt (gpointer, 
                              GmConfEntry *, 
                              gpointer);

static void tr_ub_changed_nt (gpointer, 
                              GmConfEntry *, 
                              gpointer);

static void jitter_buffer_changed_nt (gpointer, 
                                      GmConfEntry *, 
				      gpointer);

static void ils_option_changed_nt (gpointer, 
                                   GmConfEntry *, 
                                   gpointer);

static void stay_on_top_changed_nt (gpointer, 
				    GmConfEntry *, 
                                    gpointer);

static void incoming_call_mode_changed_nt (gpointer, 
					   GmConfEntry *,
					   gpointer);

static void call_forwarding_changed_nt (gpointer,
					GmConfEntry *, 
					gpointer);

static void manager_changed_nt (gpointer,
				GmConfEntry *, 
				gpointer);

static void audio_device_changed_nt (gpointer,
				     GmConfEntry *, 
				     gpointer);

static void video_device_changed_nt (gpointer, 
				     GmConfEntry *, 
				     gpointer);

static void video_device_setting_changed_nt (gpointer, 
					     GmConfEntry *, 
					     gpointer);

static void video_preview_changed_nt (gpointer, 
                                      GmConfEntry *, 
				      gpointer);

static void sound_events_list_changed_nt (gpointer,
					  GmConfEntry *, 
					  gpointer);

static void audio_codecs_list_changed_nt (gpointer, 
                                          GmConfEntry *, 
					  gpointer);


static void capabilities_changed_nt (gpointer, 
				     GmConfEntry *, 
                                     gpointer);

static void h245_tunneling_changed_nt (gpointer,
				       GmConfEntry *,
				       gpointer);

static void early_h245_changed_nt (gpointer,
				   GmConfEntry *,
				   gpointer);

static void fast_start_changed_nt (gpointer,
				   GmConfEntry *,
				   gpointer);

static void enable_video_transmission_changed_nt (gpointer, 
						  GmConfEntry *, 
						  gpointer);

static void enable_video_reception_changed_nt (gpointer, 
					       GmConfEntry *, 
					       gpointer);

static void silence_detection_changed_nt (gpointer, 
					  GmConfEntry *, 
                                          gpointer);

static void network_settings_changed_nt (gpointer, 
					 GmConfEntry *,
                                         gpointer);

#ifdef HAS_IXJ
static void lid_aec_changed_nt (gpointer,
				GmConfEntry *,
				gpointer);

static void lid_country_changed_nt (gpointer,
				    GmConfEntry *, 
				    gpointer);

static void lid_output_device_type_changed_nt (gpointer,
					       GmConfEntry *, 
					       gpointer);
#endif



/* DESCRIPTION  :  /
 * BEHAVIOR     :  Displays a popup if we are in a call.
 * PRE          :  A valid pointer to the preferences window GMObject.
 */
static void
applicability_check_nt (gpointer id, 
			GmConfEntry *entry,
			gpointer data)
{
  GMEndPoint *ep = NULL;
  
  g_return_if_fail (data != NULL);

  
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  if ((gm_conf_entry_get_type (entry) == GM_CONF_BOOL)
      ||(gm_conf_entry_get_type (entry) == GM_CONF_STRING)
      ||(gm_conf_entry_get_type (entry) == GM_CONF_INT)) {

    if (ep->GetCallingState () != GMEndPoint::Standby) {

      gdk_threads_enter ();
      gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (data),
					     gm_conf_entry_get_key (entry),
					     _("Changing this setting will only affect new calls"), 
					     _("You have changed a setting that doesn't permit to GnomeMeeting to apply the new change to the current call. Your new setting will only take effect for the next call."));
      gdk_threads_leave ();
    }
  }
}


static void 
control_panel_section_changed_nt (gpointer id, 
                                  GmConfEntry *entry, 
                                  gpointer data)
{
  gint section = 0;

  g_return_if_fail (data != NULL);
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    section = gm_conf_entry_get_int (entry);
    
    gdk_threads_enter ();
    gm_main_window_show_control_panel_section (GTK_WIDGET (data), 
					       section);
    gdk_threads_leave ();
  }
}


static void 
show_chat_window_changed_nt (gpointer id, 
			     GmConfEntry *entry, 
			     gpointer data)
{
  g_return_if_fail (data != NULL);
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    gdk_threads_enter ();
    gm_main_window_show_chat_window (GTK_WIDGET (data), 
				     gm_conf_entry_get_bool (entry));
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with the H.245 Tunneling changes.
 * BEHAVIOR     :  It updates the endpoint and displays a message.
 * PRE          :  /
 */
static void
h245_tunneling_changed_nt (gpointer id, 
			   GmConfEntry *entry,
			   gpointer data)
{
  GtkWidget *history_window = NULL;
  
  GMEndPoint *ep = NULL;
  GMH323EndPoint *h323EP = NULL;
  
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    ep = GnomeMeeting::Process ()->Endpoint ();
    history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
    h323EP = ep->GetH323EndPoint ();
    
    h323EP->DisableH245Tunneling (!gm_conf_entry_get_bool (entry));
    
    gdk_threads_enter ();
    gm_history_window_insert (history_window,
			      h323EP->IsH245TunnelingDisabled ()?
			      _("H.245 Tunneling disabled"):
			      _("H.245 Tunneling enabled"));
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with the early H.245 key changes.
 * BEHAVIOR     :  It updates the endpoint and displays a message.
 * PRE          :  /
 */
static void
early_h245_changed_nt (gpointer id, 
		       GmConfEntry *entry,
		       gpointer data)
{
  GtkWidget *history_window = NULL;
  
  GMEndPoint *ep = NULL;
  GMH323EndPoint *h323EP = NULL;  
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    ep = GnomeMeeting::Process ()->Endpoint ();
    history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
    h323EP = ep->GetH323EndPoint ();
    
    h323EP->DisableH245inSetup (!gm_conf_entry_get_bool (entry));
    
    gdk_threads_enter ();
    gm_history_window_insert (history_window,
			      h323EP->IsH245inSetupDisabled ()?
			      _("Early H.245 disabled"):
			      _("Early H.245 enabled"));
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with the Fast Start changes.
 * BEHAVIOR     :  It updates the endpoint and displays a message.
 * PRE          :  /
 */
static void
fast_start_changed_nt (gpointer id, 
		       GmConfEntry *entry,
		       gpointer data)
{
  GtkWidget *history_window = NULL;
  
  GMEndPoint *ep = NULL;
  GMH323EndPoint *h323EP = NULL;
  
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    ep = GnomeMeeting::Process ()->Endpoint ();
    h323EP = ep->GetH323EndPoint ();
    history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
    
    h323EP->DisableFastStart (!gm_conf_entry_get_bool (entry));
    
    gdk_threads_enter ();
    gm_history_window_insert (history_window,
			      h323EP->IsFastStartDisabled ()?
			      _("Fast Start disabled"):
			      _("Fast Start enabled"));
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with the enable_video_transmission key changes.
 * BEHAVIOR     :  It updates the endpoint.
 *                 If the user is in a call, the video channel will be started
 *                 and stopped on-the-fly.
 * PRE          :  /
 */
static void
enable_video_transmission_changed_nt (gpointer id, 
				      GmConfEntry *entry,
				      gpointer data)
{
  PString name;
  GMEndPoint *ep = NULL;

  ep = GnomeMeeting::Process ()->Endpoint ();

  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    ep->SetAutoStartTransmitVideo (gm_conf_entry_get_bool (entry));
    //FIXME
    //ep->AddAllCapabilities ();

    if (gm_conf_get_int (VIDEO_DEVICES_KEY "size") == 0)
      name = "H.261-QCIF";
    else
      name = "H.261-CIF";

    if (gm_conf_entry_get_bool (entry)) {
	
      //ep->StartLogicalChannel (name,
	//		       RTP_Session::DefaultVideoSessionID,
	//		       FALSE);
    }
    else {

      //ep->StopLogicalChannel (RTP_Session::DefaultVideoSessionID,
	//		      FALSE);
    }
  }
}


/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with the enable_video_transmission key changes.
 * BEHAVIOR     :  It updates the endpoint.
 *                 If the user is in a call, the video recpetion will be 
 *                 stopped on-the-fly if required.
 * PRE          :  /
 */
static void
enable_video_reception_changed_nt (gpointer id, 
				   GmConfEntry *entry,
				   gpointer data)
{
  PString name;
  GMEndPoint *ep = NULL;

  ep = GnomeMeeting::Process ()->Endpoint ();


  g_return_if_fail (data != NULL);


  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    ep->SetAutoStartReceiveVideo (gm_conf_entry_get_bool (entry));
    //FIXME ep->AddAllCapabilities ();

    if (gm_conf_get_int (VIDEO_DEVICES_KEY "size") == 0)
      name = "H.261-QCIF";
    else
      name = "H.261-CIF";

    if (!gm_conf_entry_get_bool (entry)) {
	
      //ep->StopLogicalChannel (RTP_Session::DefaultVideoSessionID,
	//		      TRUE);
    }
    else {

      if (ep->GetCallingState () != GMEndPoint::Standby) {

	gdk_threads_enter ();
	gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (data),
					       gm_conf_entry_get_key (entry),
					       _("Changing this setting will only affect new calls"), 
					       _("You have changed a setting that doesn't permit to GnomeMeeting to apply the new change to the current call. Your new setting will only take effect for the next call."));
	gdk_threads_leave ();
      }
    }
  }
}


/* DESCRIPTION  :  This callback is called when a silence detection key of
 *                 the config database associated with a toggle changes.
 * BEHAVIOR     :  It only updates the silence detection if we
 *                 are in a call. 
 * PRE          :  /
 */
static void 
silence_detection_changed_nt (gpointer id, 
                              GmConfEntry *entry, 
                              gpointer data)
{
  GtkWidget *history_window = NULL;
  OpalSilenceDetector::Params sd;
  
  GMEndPoint *ep = NULL;
  
  
  ep = GnomeMeeting::Process ()->Endpoint ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    /* We update the silence detection */
    sd = ep->GetSilenceDetectParams ();

    gdk_threads_enter ();
    if (sd.m_mode == OpalSilenceDetector::NoSilenceDetection) {

      sd.m_mode = OpalSilenceDetector::AdaptiveSilenceDetection;
      gm_history_window_insert (history_window,
				_("Enabled silence detection"));
    } 
    else {

      sd.m_mode = OpalSilenceDetector::NoSilenceDetection;
      gm_history_window_insert (history_window,
				_("Disabled silence detection"));
    }
    gdk_threads_leave ();  
    
    ep->SetSilenceDetectParams (sd);
  }
}


/* DESCRIPTION  :  This callback is called to update capabilities.
 * BEHAVIOR     :  Updates them.
 * PRE          :  /
 */
static void
capabilities_changed_nt (gpointer id, 
			 GmConfEntry *entry,
			 gpointer data)
{
  GMEndPoint *ep = NULL;

  if (gm_conf_entry_get_type (entry) == GM_CONF_INT
      || gm_conf_entry_get_type (entry) == GM_CONF_LIST
      || gm_conf_entry_get_type (entry) == GM_CONF_STRING) {
   
    ep = GnomeMeeting::Process ()->Endpoint ();

    // FIXME
    //ep->RemoveAllCapabilities ();
    //ep->AddAllCapabilities ();
  }
}


/* DESCRIPTION  :  This callback is called when the user changes the maximum
 *                 video bandwidth.
 * BEHAVIOR     :  It updates it.
 * PRE          :  /
 */
static void 
maximum_video_bandwidth_changed_nt (gpointer id, 
				    GmConfEntry *entry, 
                                    gpointer data)
{
  /* FIXME
  H323Channel *channel = NULL;
  H323Codec *raw_codec = NULL;
  H323VideoCodec *vc = NULL;
  H323Connection *connection = NULL;
  GMEndPoint *endpoint = NULL;

  int bitrate = 2;

  endpoint = GnomeMeeting::Process ()->Endpoint ();
  

  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    connection =
	endpoint->FindConnectionWithLock (endpoint->GetCurrentCallToken ());

    if (connection) {

      channel = 
	connection->FindChannel (RTP_Session::DefaultVideoSessionID, 
				 FALSE);

      if (channel)
	raw_codec = channel->GetCodec();
      
      if (raw_codec && PIsDescendant (raw_codec, H323VideoCodec)) 
	vc = (H323VideoCodec *) raw_codec;
     */
      /* We update the video quality */  
      /*bitrate = gm_conf_entry_get_int (entry) * 8 * 1024;
  
      if (vc != NULL)
	vc->SetMaxBitRate (bitrate);

      connection->Unlock ();
    }
  }*/
}


/* DESCRIPTION  :  This callback is called the transmitted video quality.
 * BEHAVIOR     :  It updates the video quality.
 * PRE          :  /
 */
static void 
tr_vq_changed_nt (gpointer id, 
                  GmConfEntry *entry, 
                  gpointer data)
{
  /*
  H323Connection *connection = NULL;
  H323Channel *channel = NULL;
  H323Codec *raw_codec = NULL;
  H323VideoCodec *vc = NULL;
  GMEndPoint *endpoint = NULL;

  int vq = 1;

  endpoint = GnomeMeeting::Process ()->Endpoint ();

  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    connection =
      endpoint->FindConnectionWithLock (endpoint->GetCurrentCallToken ());

    if (connection) {

      channel = 
	connection->FindChannel (RTP_Session::DefaultVideoSessionID, 
				 FALSE);

      if (channel)
	raw_codec = channel->GetCodec();
      
      if (raw_codec && PIsDescendant (raw_codec, H323VideoCodec)) 
	vc = (H323VideoCodec *) raw_codec;

      vq = 25 - (int) ((double) (int) gm_conf_entry_get_int (entry) / 100 * 24);

      if (vc)
	vc->SetTxQualityLevel (vq);

      connection->Unlock ();
    }
  }*/
}


/* DESCRIPTION  :  This callback is called when the bg fill needs to be changed.
 * BEHAVIOR     :  It updates the background fill.
 * PRE          :  /
 */
static void 
tr_ub_changed_nt (gpointer id, 
                  GmConfEntry *entry, 
                  gpointer data)
{
  /* FIXME
  H323Connection *connection = NULL;
  H323Channel *channel = NULL;
  H323Codec *raw_codec = NULL;
  H323VideoCodec *vc = NULL;
  GMEndPoint *endpoint = NULL;

  endpoint = GnomeMeeting::Process ()->Endpoint ();

  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    connection =
	endpoint->FindConnectionWithLock (endpoint->GetCurrentCallToken ());

    if (connection) {

      channel = 
	connection->FindChannel (RTP_Session::DefaultVideoSessionID, 
				 FALSE);

      if (channel)
	raw_codec = channel->GetCodec();
      
      if (raw_codec && PIsDescendant (raw_codec, H323VideoCodec)) 
	vc = (H323VideoCodec *) raw_codec;

      if (vc)
	vc->SetBackgroundFill ((int) gm_conf_entry_get_int (entry));
      
      connection->Unlock ();
    }
  }*/
}


/* DESCRIPTION  :  This callback is called when the jitter buffer needs to be 
 *                 changed.
 * BEHAVIOR     :  It updates the value.
 * PRE          :  /
 */
static void 
jitter_buffer_changed_nt (gpointer id, 
                          GmConfEntry *entry, 
                          gpointer data)
{
  GMEndPoint *ep = NULL;
  
  PSafePtr<OpalCall> call = NULL;
  PSafePtr<OpalConnection> connection = NULL;
  RTP_Session *session = NULL;

  int min_val = 20;
  int max_val = 500;

  ep = GnomeMeeting::Process ()->Endpoint ();  
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    gdk_threads_enter ();
    min_val = gm_conf_get_int (AUDIO_CODECS_KEY "minimum_jitter_buffer");
    max_val = gm_conf_get_int (AUDIO_CODECS_KEY "maximum_jitter_buffer");
    gdk_threads_leave ();

    call = ep->FindCallWithLock (ep->GetCurrentCallToken ());
    if (call != NULL) {

      //FIXME
      connection = call->GetConnection (0);

      if (connection != NULL) {

	session = 
	  connection->GetSession (OpalMediaFormat::DefaultAudioSessionID);
      }
    }
  }
}


/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with the audio or video manager changes.
 * BEHAVIOR     :  Updates the devices list for the new manager.
 * PRE          :  /
 */
static void
manager_changed_nt (gpointer id, 
		    GmConfEntry *entry,
		    gpointer data)
{
  g_return_if_fail (data != NULL);

  
  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING) {


    gdk_threads_enter ();
    GnomeMeeting::Process ()->DetectDevices ();
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with the audio devices changes.
 * BEHAVIOR     :  If a Quicknet device is used, then the Quicknet LID thread
 *                 is created. If not, it is removed provided we are not in
 *                 a call.
 *                 Notice that audio devices can not be changed during a call.
 * PRE          :  /
 */
static void
audio_device_changed_nt (gpointer id, 
			 GmConfEntry *entry,
			 gpointer data)
{
  GtkWidget *prefs_window = NULL;
  GMEndPoint *ep = NULL;

  OpalMediaFormatList capa;
  PString dev;

  ep = GnomeMeeting::Process ()->Endpoint ();
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING) {

    dev = gm_conf_entry_get_string (entry);

    if (ep->GetCallingState () == GMEndPoint::Standby
	&& gm_conf_entry_get_key (entry)
	&& !strcmp (gm_conf_entry_get_key (entry),
		    AUDIO_DEVICES_KEY "input_device")) {
      
      //if (dev.Find ("/dev/phone") != P_MAX_INDEX) 
	//ep->CreateLid (dev);
      //else 
	//ep->RemoveLid ();
      //FIXME
      capa = ep->GetAudioMediaFormats ();

      /* Update the codecs list and the capabilities */
      gnomemeeting_threads_enter ();
      gm_prefs_window_update_audio_codecs_list (prefs_window, capa);
      gnomemeeting_threads_leave ();

      //FIXME
      //ep->AddAllCapabilities ();
    }
  }
}



/* DESCRIPTION  :  This callback is called when the video device changes
 *                 in the config database.
 * BEHAVIOR     :  It creates a new video grabber if preview is active with
 *                 the selected video device.
 *                 If preview is not enabled, then the potentially existing
 *                 video grabber is deleted provided we are not in
 *                 a call.
 *                 Notice that the video device can't be changed during calls,
 *                 but its settings can be changed.
 * PRE          :  /
 */
static void 
video_device_changed_nt (gpointer id, 
			 GmConfEntry *entry, 
			 gpointer data)
{
  GMEndPoint *ep = NULL;
  BOOL preview = FALSE;
  
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  if ((gm_conf_entry_get_type (entry) == GM_CONF_STRING) ||
      (gm_conf_entry_get_type (entry) == GM_CONF_INT)) {

    if (ep && ep->GetCallingState () == GMEndPoint::Standby) {

      gdk_threads_enter ();
      preview = gm_conf_get_bool (VIDEO_DEVICES_KEY "enable_preview");
      gdk_threads_leave ();

      if (preview)
	ep->CreateVideoGrabber ();
      else 
	ep->RemoveVideoGrabber ();
    }
  }
}


/* DESCRIPTION  :  This callback is called when a video device setting changes
 *                 in the config database.
 * BEHAVIOR     :  It resets the video transmission if any, or resets the
 *                 video device if preview is enabled otherwise. Notice that
 *                 the video device can't be changed during calls, but its
 *                 settings can be changed. It also updates the capabilities.
 * PRE          :  /
 */
static void 
video_device_setting_changed_nt (gpointer id, 
				 GmConfEntry *entry, 
				 gpointer data)
{
  PString name;

  //int max_try = 0;
  BOOL preview = FALSE;
  //BOOL no_error = FALSE;

  GMEndPoint *ep = NULL;

  ep = GnomeMeeting::Process ()->Endpoint ();


  if ((gm_conf_entry_get_type (entry) == GM_CONF_STRING) ||
      (gm_conf_entry_get_type (entry) == GM_CONF_INT)) {
  
    /* Update the capabilities */
    //FIXME ep->AddAllCapabilities ();
    
    if (ep && ep->GetCallingState () == GMEndPoint::Standby) {

      gdk_threads_enter ();
      preview = gm_conf_get_bool (VIDEO_DEVICES_KEY "enable_preview");
      gdk_threads_leave ();

      if (preview)
	ep->CreateVideoGrabber ();
    }
    else if (ep->GetCallingState () == GMEndPoint::Connected) {

      gdk_threads_enter ();
      if (gm_conf_get_int (VIDEO_DEVICES_KEY "size") == 0)
	name = "H.261-QCIF";
      else
	name = "H.261-CIF";
      gdk_threads_leave ();

      if (gm_conf_get_bool (VIDEO_CODECS_KEY "enable_video_transmission")) {

	/*
	no_error =
	  ep->StopLogicalChannel (RTP_Session::DefaultVideoSessionID,
				  FALSE);

	while (no_error &&
	       !ep->StartLogicalChannel (name, 
					 RTP_Session::DefaultVideoSessionID,
					 FALSE)) {
    
	  max_try++;
	  PThread::Current ()->Sleep (300);
	  if (max_try >= 3) {
	    
	    no_error = FALSE;
	    break;
	  }
	}
*/
        /* if (!no_error) {

	  gdk_threads_enter ();
	  gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Failed to restart the video channel"), _("You have changed a video device related setting during a call. That requires to restart the video transmission channel, but it failed."));
	  gdk_threads_leave ();
	}
        */
      }
    }
  }
}


/* DESCRIPTION  :  This callback is called when the video preview changes in
 *                 the config database.
 * BEHAVIOR     :  It starts or stops the preview.
 * PRE          :  /
 */
static void video_preview_changed_nt (gpointer id, 
				      GmConfEntry *entry,
				      gpointer data)
{
  GMEndPoint *ep = NULL;
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {
   
    /* We reset the video device */
    ep = GnomeMeeting::Process ()->Endpoint ();
    
    if (ep && ep->GetCallingState () == GMEndPoint::Standby) {
    
      if (gm_conf_entry_get_bool (entry)) 
	ep->CreateVideoGrabber ();
      else 
	ep->RemoveVideoGrabber ();
    }
  }
}


/* DESCRIPTION  :  This callback is called when something changes in the sound
 *                 events list.
 * BEHAVIOR     :  It updates the events list widget.
 * PRE          :  A pointer to the prefs window GMObject.
 */
static void
sound_events_list_changed_nt (gpointer id, 
			      GmConfEntry *entry,
			      gpointer data)
{ 
  g_return_if_fail (data != NULL);

  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING
      || gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {
   
    gdk_threads_enter ();
    gm_prefs_window_sound_events_list_build (GTK_WIDGET (data));
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This callback is called when something changes in the audio
 *                 codecs clist.
 * BEHAVIOR     :  It updates the codecs list widget.
 * PRE          :  A valid pointer to the prefs window GMObject.
 */
static void
audio_codecs_list_changed_nt (gpointer id, 
			      GmConfEntry *entry,
			      gpointer data)
{
  GMEndPoint *ep = NULL;

  OpalMediaFormatList l;


  g_return_if_fail (data != NULL);
  
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_LIST) {
   
    ep = GnomeMeeting::Process ()->Endpoint ();
    l = ep->GetAudioMediaFormats ();
    
    /* Update the GUI */
    gdk_threads_enter ();
    gm_prefs_window_update_audio_codecs_list (GTK_WIDGET (data), l);
    gdk_threads_leave ();
  } 
}


/* DESCRIPTION  :  This callback is called when the forward config value 
 *                 changes.
 * BEHAVIOR     :  It checks that there is a forwarding host specified, if
 *                 not, disable forwarding and displays a popup.
 *                 It also modifies the "incoming_call_state" key if the
 *                 "always_forward" is modified, changing the corresponding
 *                 "incoming_call_mode" between AVAILABLE and FORWARD when
 *                 required.
 * PRE          :  A valid pointer to the prefs window GMObject.
 */
static void
call_forwarding_changed_nt (gpointer id, 
			    GmConfEntry *entry,
			    gpointer data)
{
  GtkWidget *main_window = NULL;

  gchar *conf_string = NULL;

  GMURL url;
    
  g_return_if_fail (data != NULL);
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    gdk_threads_enter ();

    /* If "always_forward" is not set, we can always change the
       "incoming_call_mode" to AVAILABLE if it was set to FORWARD */
    if (!gm_conf_get_bool (CALL_FORWARDING_KEY "always_forward")) {

      if (gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode") == FORWARD) 
	gm_conf_set_int (CALL_OPTIONS_KEY "incoming_call_mode", AVAILABLE);
    }


    /* Checks if the forward host name is ok */
    conf_string = gm_conf_get_string (CALL_FORWARDING_KEY "forward_host");
    
    if (conf_string)
      url = GMURL (conf_string);
    if (url.IsEmpty ()) {

      /* If the URL is empty, we display a message box indicating
	 to the user to put a valid hostname and we disable
	 "always_forward" if "always_forward" is enabled */
      if (gm_conf_entry_get_bool (entry)) {

	
	gnomemeeting_error_dialog (GTK_WIDGET_VISIBLE (data)?
				   GTK_WINDOW (data):
				   GTK_WINDOW (main_window),
				   _("Forward URL not specified"),
				   _("You need to specify an URL where to forward calls in the call forwarding section of the preferences!\n\nDisabling forwarding."));
            
	gm_conf_set_bool ((gchar *) gm_conf_entry_get_key (entry), FALSE);
      }
    }
    else {
      
      /* Change the "incoming_call_mode" to FORWARD if "always_forward"
	 is enabled and if the URL is not empty */
      if (gm_conf_get_bool (CALL_FORWARDING_KEY "always_forward")) {

	if (gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode") != FORWARD)
	  gm_conf_set_int (CALL_OPTIONS_KEY "incoming_call_mode", FORWARD);
      }
    }

    g_free (conf_string);

    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This callback is called when an ILS option is changed.
 * BEHAVIOR     :  It registers or unregisters with updated values. The ILS
 *                 thread will check that all required values are provided.
 * PRE          :  /
 */
static void 
ils_option_changed_nt (gpointer id, 
		       GmConfEntry *entry, 
		       gpointer data)
{
  GMEndPoint *endpoint = NULL;
  
  endpoint = GnomeMeeting::Process ()->Endpoint ();
 
  if (gm_conf_entry_get_type (entry) == GM_CONF_INT
      || gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    if (endpoint)
      endpoint->ILSRegister ();
  }
}


/* DESCRIPTION  :  This callback is called when the incoming_call_mode
 *                 config value changes.
 * BEHAVIOR     :  Modifies the tray icon, and the incoming call mode menu, the
 *                 always_forward key following the current mode is FORWARD or
 *                 not.
 * PRE          :  /
 */
static void
incoming_call_mode_changed_nt (gpointer id, 
			       GmConfEntry *entry,
			       gpointer data)
{
  GtkWidget *main_window = NULL;
  GtkWidget *tray = NULL;
  
  
  GMEndPoint::CallingState calling_state = GMEndPoint::Standby;
  GMEndPoint *ep = NULL;

  gboolean forward_on_busy = FALSE;
  IncomingCallMode i;

  ep = GnomeMeeting::Process ()->Endpoint ();
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  tray = GnomeMeeting::Process ()->GetTray ();
  

  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    calling_state = ep->GetCallingState ();
    
    gdk_threads_enter ();
    
    /* Update the call forwarding key if the status is changed */
    if (gm_conf_entry_get_int (entry) == FORWARD)
      gm_conf_set_bool (CALL_FORWARDING_KEY "always_forward", TRUE);
    else
      gm_conf_set_bool (CALL_FORWARDING_KEY "always_forward", FALSE);
   
    forward_on_busy = gm_conf_get_bool (CALL_FORWARDING_KEY "forward_on_busy");
    i = (IncomingCallMode) gm_conf_entry_get_int (entry);
    
    
    /* Update the tray icon and its menu */
    gm_tray_update (tray, calling_state, i, forward_on_busy);

    /* Update the main window and its menu */
    gm_main_window_set_incoming_call_mode (main_window, i);

#ifdef HAS_HOWL
    ep->ZeroconfUpdate ();
#endif
    
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This callback is called when the "stay_on_top" 
 *                 config value changes.
 * BEHAVIOR     :  Changes the hint for the video windows.
 * PRE          :  /
 */
static void 
stay_on_top_changed_nt (gpointer id, 
                        GmConfEntry *entry, 
                        gpointer data)
{
  bool val = false;
    
  g_return_if_fail (data != NULL);

  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    gdk_threads_enter ();

    val = gm_conf_entry_get_bool (entry);

    gm_main_window_set_stay_on_top (GTK_WIDGET (data), val);

    gdk_threads_leave ();
  }
}


/* DESCRIPTION    : This is called when any setting related to the druid 
 *                  network speed selecion changes.
 * BEHAVIOR       : Just writes an entry in the config database registering 
 *                  that fact.
 * PRE            : None
 */
static void 
network_settings_changed_nt (gpointer id, 
                             GmConfEntry *, 
                             gpointer)
{
  gdk_threads_enter ();
  gm_conf_set_int (GENERAL_KEY "kind_of_net", 5);
  gdk_threads_leave ();
}


#ifdef HAS_IXJ
/* DESCRIPTION    : This is called when any setting related to the 
 *                  lid AEC changes.
 * BEHAVIOR       : Updates it.
 * PRE            : None
 */
static void 
lid_aec_changed_nt (gpointer id, 
                    GmConfEntry *entry, 
                    gpointer)
{
  GMEndPoint *ep = NULL;
  GMLid *lid = NULL;
  
  int lid_aec = 0;
    
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    lid_aec = gm_conf_entry_get_int (entry);

    lid = (ep ? ep->GetLid () : NULL);

    if (lid) {

      lid->SetAEC (0, (OpalLineInterfaceDevice::AECLevels) lid_aec);
      lid->Unlock ();
    }
  }
}


/* DESCRIPTION    : This is called when any setting related to the 
 *                  country code changes.
 * BEHAVIOR       : Updates it.
 * PRE            : None
 */
static void 
lid_country_changed_nt (gpointer id, 
                        GmConfEntry *entry, 
			gpointer)
{
  GMEndPoint *ep = NULL;
  GMLid *lid = NULL;
  
  gchar *country_code = NULL;
    
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING) {
    
    lid = (ep ? ep->GetLid () : NULL);

    country_code = g_strdup (gm_conf_entry_get_string (entry));
    
    if (country_code && lid) {
      
      lid->SetCountryCodeName (country_code);
      lid->Unlock ();
  
      g_free (country_code);
    }
  }
}


/* DESCRIPTION    : This is called when any setting related to the 
 *                  lid output device type changes.
 * BEHAVIOR       : Updates it.
 * PRE            : None
 */
static void 
lid_output_device_type_changed_nt (gpointer id,
				   GmConfEntry *entry, 
				   gpointer)
{
  GMEndPoint *ep = NULL;
  GMLid *lid = NULL;
    
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {
    
    lid = (ep ? ep->GetLid () : NULL);

    if (lid) {

      if (gm_conf_entry_get_int (entry) == 0) // POTS
	  lid->EnableAudio (0, FALSE);
	else
	  lid->EnableAudio (0, TRUE);
      
      lid->Unlock ();
    }
  }
}
#endif

/* The functions */
gboolean 
gnomemeeting_conf_init ()
{
  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;
  GtkWidget *prefs_window = NULL;
  GtkWidget *tray = NULL;
  
  int conf_test = -1;
  
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  tray = GnomeMeeting::Process ()->GetTray ();
  main_window = GnomeMeeting::Process ()->GetMainWindow ();


  /* Start listeners */
  gm_conf_watch ();

    
  /* Check the config is ok */
  conf_test = gm_conf_get_int (GENERAL_KEY "gconf_test_age");
  
  if (conf_test != SCHEMA_AGE) 
    return FALSE;


  /* There are in general 2 notifiers to attach to each widget :
   * - the notifier that will update the widget itself to the new key,
   *   that one is attached when creating the widget.
   * - the notifier to take an appropriate action, that one is in this file.
   *   
   * Notice that there can be more than 2 notifiers for a key, some actions
   * like updating the ILS server are for example required for
   * several different key changes, they are thus in a separate notifier when
   * they can be reused at several places. If not, a same notifier can contain
   * several actions.
   */

  /* Notifiers for the USER_INTERFACE_KEY keys */
  gm_conf_notifier_add (USER_INTERFACE_KEY "main_window/control_panel_section",
			control_panel_section_changed_nt, main_window);
  
  gm_conf_notifier_add (USER_INTERFACE_KEY "main_window/show_chat_window",
			show_chat_window_changed_nt, main_window);

  
  /* Notifiers for the CALL_OPTIONS_KEY keys */
  gm_conf_notifier_add (CALL_OPTIONS_KEY "incoming_call_mode",
			incoming_call_mode_changed_nt, NULL);
  gm_conf_notifier_add (CALL_OPTIONS_KEY "incoming_call_mode",
			ils_option_changed_nt, NULL);
 

  /* Notifiers for the CALL_FORWARDING_KEY keys */
  gm_conf_notifier_add (CALL_FORWARDING_KEY "always_forward",
			call_forwarding_changed_nt, prefs_window);
  
  gm_conf_notifier_add (CALL_FORWARDING_KEY "forward_on_busy",
			call_forwarding_changed_nt, prefs_window);
  
  gm_conf_notifier_add (CALL_FORWARDING_KEY "forward_on_no_answer",
			call_forwarding_changed_nt, prefs_window);


  /* Notifiers related to the H323_ADVANCED_KEY */
  gm_conf_notifier_add (H323_KEY "enable_h245_tunneling",
			applicability_check_nt, prefs_window);
  gm_conf_notifier_add (H323_KEY "enable_h245_tunneling",
			h245_tunneling_changed_nt, NULL);

  gm_conf_notifier_add (H323_KEY "enable_early_h245",
			applicability_check_nt, prefs_window);
  gm_conf_notifier_add (H323_KEY "enable_early_h245",
			early_h245_changed_nt, NULL);

  gm_conf_notifier_add (H323_KEY "enable_fast_start",
			applicability_check_nt, prefs_window);
  gm_conf_notifier_add (H323_KEY "enable_fast_start",
			fast_start_changed_nt, NULL);

  gm_conf_notifier_add (H323_KEY "dtmf_sending",
			capabilities_changed_nt, NULL);
  gm_conf_notifier_add (H323_KEY "dtmf_sending",
			applicability_check_nt, prefs_window);

  
  /* Notifiers related to the H323_KEY */
  gm_conf_notifier_add (H323_KEY "default_gateway", 
			applicability_check_nt, prefs_window);
  
    
  /* Notifiers related the LDAP_KEY */
  gm_conf_notifier_add (LDAP_KEY "enable_registering",
			ils_option_changed_nt, NULL);

  gm_conf_notifier_add (LDAP_KEY "show_details", ils_option_changed_nt, NULL);
  
  
  /* Notifiers to AUDIO_DEVICES_KEY */
  gm_conf_notifier_add (AUDIO_DEVICES_KEY "plugin", 
			manager_changed_nt, prefs_window);

  gm_conf_notifier_add (AUDIO_DEVICES_KEY "output_device",
			audio_device_changed_nt, NULL);
  gm_conf_notifier_add (AUDIO_DEVICES_KEY "output_device",
			applicability_check_nt, prefs_window);
  
  gm_conf_notifier_add (AUDIO_DEVICES_KEY "input_device",
			audio_device_changed_nt, NULL);
  gm_conf_notifier_add (AUDIO_DEVICES_KEY "input_device",
			applicability_check_nt, prefs_window);

#ifdef HAS_IXJ
  gm_conf_notifier_add (AUDIO_DEVICES_KEY "lid_country_code",
			lid_country_changed_nt, NULL);

  gm_conf_notifier_add (AUDIO_DEVICES_KEY "lid_echo_cancellation_level",
			lid_aec_changed_nt, NULL);

  gm_conf_notifier_add (AUDIO_DEVICES_KEY "lid_output_device_type",
			lid_output_device_type_changed_nt, NULL);
#endif


  /* Notifiers to VIDEO_DEVICES_KEY */
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "plugin", 
			manager_changed_nt, prefs_window);
  
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "input_device", 
			video_device_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "input_device", 
			applicability_check_nt, prefs_window);

  gm_conf_notifier_add (VIDEO_DEVICES_KEY "channel", 
			video_device_setting_changed_nt, NULL);

  gm_conf_notifier_add (VIDEO_DEVICES_KEY "size", 
			video_device_setting_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "size", 
			capabilities_changed_nt, NULL);

  gm_conf_notifier_add (VIDEO_DEVICES_KEY "format", 
			video_device_setting_changed_nt, NULL);

  gm_conf_notifier_add (VIDEO_DEVICES_KEY "image", 
			video_device_setting_changed_nt, NULL);

  gm_conf_notifier_add (VIDEO_DEVICES_KEY "enable_preview", 
			video_preview_changed_nt, NULL);

  
  /* Notifiers for the VIDEO_DISPLAY_KEY keys */
  gm_conf_notifier_add (VIDEO_DISPLAY_KEY "stay_on_top", 
			stay_on_top_changed_nt, main_window);

  
  /* Notifiers for SOUND_EVENTS_KEY keys */
  gm_conf_notifier_add (SOUND_EVENTS_KEY "enable_incoming_call_sound", 
			sound_events_list_changed_nt, prefs_window);
  
  gm_conf_notifier_add (SOUND_EVENTS_KEY "incoming_call_sound",
			sound_events_list_changed_nt, prefs_window);

  gm_conf_notifier_add (SOUND_EVENTS_KEY "enable_ring_tone_sound", 
			sound_events_list_changed_nt, prefs_window);
  
  gm_conf_notifier_add (SOUND_EVENTS_KEY "ring_tone_sound", 
			sound_events_list_changed_nt, prefs_window);
  
  gm_conf_notifier_add (SOUND_EVENTS_KEY "enable_busy_tone_sound", 
			sound_events_list_changed_nt, prefs_window);
  
  gm_conf_notifier_add (SOUND_EVENTS_KEY "busy_tone_sound",
			sound_events_list_changed_nt, prefs_window);

 
  /* Notifiers for the AUDIO_CODECS_KEY keys */
  gm_conf_notifier_add (AUDIO_CODECS_KEY "list", 
			audio_codecs_list_changed_nt, 
			prefs_window);
  
  gm_conf_notifier_add (AUDIO_CODECS_KEY "list", capabilities_changed_nt, NULL);
  gm_conf_notifier_add (AUDIO_CODECS_KEY "minimum_jitter_buffer", 
			jitter_buffer_changed_nt, NULL);

  gm_conf_notifier_add (AUDIO_CODECS_KEY "maximum_jitter_buffer", 
			jitter_buffer_changed_nt, NULL);

  gm_conf_notifier_add (AUDIO_CODECS_KEY "gsm_frames", 
			capabilities_changed_nt, NULL);

  gm_conf_notifier_add (AUDIO_CODECS_KEY "g711_frames", 
			capabilities_changed_nt, NULL);

  gm_conf_notifier_add (AUDIO_CODECS_KEY "enable_silence_detection", 
			silence_detection_changed_nt, NULL);


  /* Notifiers for the VIDEO_CODECS_KEY keys */
  gm_conf_notifier_add (VIDEO_CODECS_KEY "enable_video_reception",
			network_settings_changed_nt, NULL);	     
  gm_conf_notifier_add (VIDEO_CODECS_KEY "enable_video_reception", 
			enable_video_reception_changed_nt, main_window);     

  gm_conf_notifier_add (VIDEO_CODECS_KEY "enable_video_transmission", 
			network_settings_changed_nt, NULL);	     
  gm_conf_notifier_add (VIDEO_CODECS_KEY "enable_video_transmission",
			enable_video_transmission_changed_nt, NULL);	     
  gm_conf_notifier_add (VIDEO_CODECS_KEY "enable_video_transmission", 
			ils_option_changed_nt, NULL);
  
  gm_conf_notifier_add (VIDEO_CODECS_KEY "maximum_video_bandwidth", 
			maximum_video_bandwidth_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_CODECS_KEY "maximum_video_bandwidth", 
			network_settings_changed_nt, NULL);

  gm_conf_notifier_add (VIDEO_CODECS_KEY "transmitted_video_quality",
			tr_vq_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_CODECS_KEY "transmitted_video_quality", 
			network_settings_changed_nt, NULL);


  gm_conf_notifier_add (VIDEO_CODECS_KEY "transmitted_background_blocks", 
			tr_ub_changed_nt, NULL);


  return TRUE;
}


void 
gnomemeeting_conf_upgrade ()
{
  gchar *conf_url = NULL;

  int version = 0;

  version = gm_conf_get_int (GENERAL_KEY "version");
  
  /* Install the h323: and callto: GNOME URL Handlers */
  conf_url = gm_conf_get_string ("/desktop/gnome/url-handlers/callto/command");
					       
  if (!conf_url) {
    
    gm_conf_set_string ("/desktop/gnome/url-handlers/callto/command", 
                      "gnomemeeting -c \"%s\"");

    gm_conf_set_bool ("/desktop/gnome/url-handlers/callto/need-terminal", 
		      false);
    
    gm_conf_set_bool ("/desktop/gnome/url-handlers/callto/enabled", true);
  }
  g_free (conf_url);

  conf_url = gm_conf_get_string ("/desktop/gnome/url-handlers/h323/command");
  if (!conf_url) {
    
    gm_conf_set_string ("/desktop/gnome/url-handlers/h323/command", 
                      "gnomemeeting -c \"%s\"");
    
    gm_conf_set_bool ("/desktop/gnome/url-handlers/h323/need-terminal", false);

    gm_conf_set_bool ("/desktop/gnome/url-handlers/h323/enabled", true);
  }
  g_free (conf_url);
}
