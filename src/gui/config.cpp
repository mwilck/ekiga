
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         config.cpp  -  description
 *                         --------------------------
 *   begin                : Wed Feb 14 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras 
 *   description          : This file contains most of config stuff.
 *                          All notifiers are here.
 *                          Callbacks that updates the config cache 
 *                          are in their file, except some generic one that
 *                          are in this file.
 *   Additional code      : Miguel Rodríguez Pérez  <miguelrp@gmail.com>
 *
 */


#include "../../config.h"

#include "config.h"

#include "h323.h"
#include "sip.h"
#include "manager.h"

#include "ekiga.h"
#include "preferences.h"
#include "druid.h"
#include "accounts.h"
#include "main.h"
#include "history.h"
#include "statusicon.h"
#include "misc.h"
#include "tools.h"
#include "urlhandler.h"

#include "gmdialog.h"
#include "gmstockicons.h"
#include "gmmenuaddon.h"
#include "gmconfwidgets.h"



/* Declarations */
static void applicability_check_nt (gpointer id,
				    GmConfEntry *entry,
				    gpointer data);


/* DESCRIPTION  :  This callback is called when the control panel 
 *                 section key changes.
 * BEHAVIOR     :  Sets the right page, and also sets 
 *                 the good value for the radio menu. 
 * PRE          :  /
 */
static void panel_section_changed_nt (gpointer id,
                                      GmConfEntry *entry,
                                      gpointer data);


/* DESCRIPTION  :  This callback is called when the firstname or last name
 *                 keys changes.
 * BEHAVIOR     :  Updates the ILS and ZeroConf registrations and the 
 * 		   main endpoint configuration.
 * PRE          :  /
 */
static void fullname_changed_nt (gpointer id,
				 GmConfEntry *entry,
				 gpointer data);

static void h245_tunneling_changed_nt (gpointer id,
				       GmConfEntry *entry,
				       gpointer data);

static void early_h245_changed_nt (gpointer id,
				   GmConfEntry *entry,
				   gpointer data);

static void fast_start_changed_nt (gpointer id,
				   GmConfEntry *enty,
				   gpointer data);

static void outbound_proxy_changed_nt (gpointer id,
				       GmConfEntry *entry,
				       gpointer data);

static void enable_video_changed_nt (gpointer id,
				     GmConfEntry *entry,
				     gpointer data);

static void silence_detection_changed_nt (gpointer id,
					  GmConfEntry *entry,
                                          gpointer data);

static void echo_cancelation_changed_nt (gpointer id,
					 GmConfEntry *entry,
					 gpointer data);

static void dtmf_mode_changed_nt (gpointer id,
                                  GmConfEntry *entry,
                                  gpointer data);

static void video_media_format_changed_nt (gpointer id,
					   GmConfEntry *entry,
					   gpointer data);

static void jitter_buffer_changed_nt (gpointer id,
                                      GmConfEntry *entry,
				      gpointer data);

static void accounts_list_changed_nt (gpointer id,
				      GmConfEntry *entry,
				      gpointer data);

static void interface_changed_nt (gpointer id,
				  GmConfEntry *entry,
				  gpointer data);

static void public_ip_changed_nt (gpointer id,
				  GmConfEntry *entry,
				  gpointer data);

static void manager_changed_nt (gpointer id,
				GmConfEntry *entry,
				gpointer data);

static void video_device_changed_nt (gpointer id,
				     GmConfEntry *entry,
				     gpointer data);

static void video_device_setting_changed_nt (gpointer id,
					     GmConfEntry *entry,
					     gpointer data);

static void video_preview_changed_nt (gpointer id,
                                      GmConfEntry *entry,
				      gpointer data);

static void sound_events_list_changed_nt (gpointer id,
					  GmConfEntry *entry,
					  gpointer data);

static void call_forwarding_changed_nt (gpointer id,
					GmConfEntry *entry,
					gpointer data);

#if 0
static void ils_option_changed_nt (gpointer id,
                                   GmConfEntry *entry,
                                   gpointer data);
#endif 

static void incoming_call_mode_changed_nt (gpointer id,
					   GmConfEntry *entry,
					   gpointer data);

static void stay_on_top_changed_nt (gpointer id,
				    GmConfEntry *entry,
                                    gpointer data);

static void network_settings_changed_nt (gpointer id,
					 GmConfEntry *entry,
                                         gpointer data);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Displays a popup if we are in a call.
 * PRE          :  A valid pointer to the preferences window GMObject.
 */
static void
applicability_check_nt (gpointer id, 
			GmConfEntry *entry,
			gpointer data)
{
  GMManager *ep = NULL;
  
  g_return_if_fail (data != NULL);

  
  ep = GnomeMeeting::Process ()->GetManager ();
  
  if ((gm_conf_entry_get_type (entry) == GM_CONF_BOOL)
      ||(gm_conf_entry_get_type (entry) == GM_CONF_STRING)
      ||(gm_conf_entry_get_type (entry) == GM_CONF_INT)) {

    if (ep->GetCallingState () != GMManager::Standby) {

      gdk_threads_enter ();
      gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (data),
					     gm_conf_entry_get_key (entry),
					     _("Changing this setting will only affect new calls"), 
					     _("Ekiga cannot apply one or more changes to the current call. Your new settings will take effect for the next call."));
      gdk_threads_leave ();
    }
  }
}


static void 
panel_section_changed_nt (gpointer id, 
                          GmConfEntry *entry, 
                          gpointer data)
{
  gint section = 0;

  g_return_if_fail (data != NULL);
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    gdk_threads_enter ();
    section = gm_conf_entry_get_int (entry);
    gm_main_window_set_panel_section (GTK_WIDGET (data), 
                                      section);
    gdk_threads_leave ();
  }
}


static void 
fullname_changed_nt (gpointer id, 
		     GmConfEntry *entry, 
		     gpointer data)
{
  GMManager *endpoint = NULL;

  endpoint = GnomeMeeting::Process ()->GetManager ();
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING) {

    endpoint->SetUserNameAndAlias ();
    endpoint->UpdatePublishers ();
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
  
  GMManager *ep = NULL;
  GMH323Endpoint *h323EP = NULL;
  
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    ep = GnomeMeeting::Process ()->GetManager ();
    history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
    h323EP = ep->GetH323Endpoint ();
    
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
  
  GMManager *ep = NULL;
  GMH323Endpoint *h323EP = NULL;  
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    ep = GnomeMeeting::Process ()->GetManager ();
    history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
    h323EP = ep->GetH323Endpoint ();
    
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
  
  GMManager *ep = NULL;
  GMH323Endpoint *h323EP = NULL;
  
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    ep = GnomeMeeting::Process ()->GetManager ();
    h323EP = ep->GetH323Endpoint ();
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
 *                 associated with the SIP Outbound Proxy changes.
 * BEHAVIOR     :  It updates the endpoint.
 * PRE          :  /
 */
static void
outbound_proxy_changed_nt (gpointer id, 
			   GmConfEntry *entry,
			   gpointer data)
{
  GtkWidget *history_window = NULL;
  
  GMManager *ep = NULL;
  GMSIPEndpoint *sipEP = NULL;

  gchar *outbound_proxy_login = NULL;
  gchar *outbound_proxy_password = NULL;
  gchar *outbound_proxy_host = NULL;
  
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING) {

    ep = GnomeMeeting::Process ()->GetManager ();
    sipEP = ep->GetSIPEndpoint ();
    history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
    
    gdk_threads_enter ();
    outbound_proxy_host = gm_conf_get_string (SIP_KEY "outbound_proxy_host");
    outbound_proxy_login = gm_conf_get_string (SIP_KEY "outbound_proxy_login");
    outbound_proxy_password = 
      gm_conf_get_string (SIP_KEY "outbound_proxy_password");
    gdk_threads_leave ();
  
    sipEP->SetProxy (outbound_proxy_host, 
		     outbound_proxy_login, 
		     outbound_proxy_password);
    
    g_free (outbound_proxy_host);
    g_free (outbound_proxy_login);
    g_free (outbound_proxy_password);
  }
}


/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with the enable_video key changes.
 * BEHAVIOR     :  It updates the endpoint.
 * PRE          :  /
 */
static void
enable_video_changed_nt (gpointer id, 
			 GmConfEntry *entry,
			 gpointer data)
{
  PString name;
  GMManager *ep = NULL;

  ep = GnomeMeeting::Process ()->GetManager ();

  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    ep->SetAutoStartTransmitVideo (gm_conf_entry_get_bool (entry));
    ep->SetAutoStartReceiveVideo (gm_conf_entry_get_bool (entry));
  }
}


/* DESCRIPTION  :  This callback is called when a silence detection key of
 *                 the config database associated with a toggle changes.
 * BEHAVIOR     :  Updates the silence detection.
 * PRE          :  /
 */
static void 
silence_detection_changed_nt (gpointer id, 
                              GmConfEntry *entry, 
                              gpointer data)
{
  PSafePtr <OpalCall> call = NULL;
  PSafePtr <OpalConnection> connection = NULL;
  
  GtkWidget *history_window = NULL;
  
  OpalSilenceDetector *silence_detector = NULL;
  OpalSilenceDetector::Params sd;
  
  GMManager *ep = NULL;
  

  ep = GnomeMeeting::Process ()->GetManager ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    /* We update the silence detection endpoint parameter */
    sd = ep->GetSilenceDetectParams ();

    gdk_threads_enter ();
    if (gm_conf_entry_get_bool (entry)) {

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


    /* If we are in a call update it in real time */
    call = ep->FindCallWithLock (ep->GetCurrentCallToken ());
    if (call != NULL) {

      connection = ep->GetConnection (call, FALSE);

      if (connection != NULL) {

	silence_detector = connection->GetSilenceDetector ();

	if (silence_detector)
	  silence_detector->SetParameters (sd);
      }
    }
  }
}


/* DESCRIPTION  :  This callback is called when the echo cancelation key of
 *                 the config database associated with a toggle changes.
 * BEHAVIOR     :  Updates the echo cancelation.
 * PRE          :  /
 */
static void 
echo_cancelation_changed_nt (gpointer id, 
			     GmConfEntry *entry, 
			     gpointer data)
{
  PSafePtr <OpalCall> call = NULL;
  PSafePtr <OpalConnection> connection = NULL;
  
  GtkWidget *history_window = NULL;
  
  OpalEchoCanceler *echo_canceler = NULL;
  OpalEchoCanceler::Params ec;
  
  GMManager *ep = NULL;
  

  ep = GnomeMeeting::Process ()->GetManager ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    /* We update the echo cancelation endpoint parameter */
    ec = ep->GetEchoCancelParams ();

    gdk_threads_enter ();
    if (gm_conf_entry_get_bool (entry)) {

      ec.m_mode = OpalEchoCanceler::Cancelation;
      gm_history_window_insert (history_window,
				_("Enabled echo cancelation"));
    } 
    else {

      ec.m_mode = OpalEchoCanceler::NoCancelation;
      gm_history_window_insert (history_window,
				_("Disabled echo cancelation"));
    }
    gdk_threads_leave ();  
    
    ep->SetEchoCancelParams (ec);


    /* If we are in a call update it in real time */
    call = ep->FindCallWithLock (ep->GetCurrentCallToken ());
    if (call != NULL) {

      connection = ep->GetConnection (call, FALSE);

      if (connection != NULL) {

	echo_canceler = connection->GetEchoCanceler ();

	if (echo_canceler)
	  echo_canceler->SetParameters (ec);
      }
    }
  }
}


/* DESCRIPTION  :  This callback is called to update capabilities when the
 *                 DTMF mode is changed.
 * BEHAVIOR     :  Updates them.
 * PRE          :  /
 */
static void
dtmf_mode_changed_nt (gpointer id, 
                      GmConfEntry *entry,
                      gpointer data)
{
  GMManager *ep = NULL;

  if (gm_conf_entry_get_type (entry) == GM_CONF_INT
      || gm_conf_entry_get_type (entry) == GM_CONF_STRING) {
   
    ep = GnomeMeeting::Process ()->GetManager ();

    ep->SetUserInputMode ();
  }
}


/* DESCRIPTION  :  This callback is called when one of the video media format
 * 		   settings changes.
 * BEHAVIOR     :  It updates the media format settings.
 * PRE          :  /
 */
static void 
video_media_format_changed_nt (gpointer id, 
			       GmConfEntry *entry, 
			       gpointer data)
{
  GMManager *ep = NULL;
  OpalMediaStream *stream = NULL;
  PSafePtr<OpalCall> call = NULL;
  PSafePtr<OpalConnection> connection = NULL;
  
  int vq = 1;
  int bitrate = 2;

  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    ep = GnomeMeeting::Process ()->GetManager ();

    call = ep->FindCallWithLock (ep->GetCurrentCallToken ());
    if (call != NULL) {

      connection = ep->GetConnection (call, TRUE);

      if (connection != NULL) {

	stream = 
	  connection->GetMediaStream (OpalMediaFormat::DefaultVideoSessionID, 
				      FALSE);

	if (stream != NULL) {
	  

	  gdk_threads_enter ();
	  vq = 
	    gm_conf_get_int (VIDEO_CODECS_KEY "transmitted_video_quality");
	  bitrate = 
	    gm_conf_get_int (VIDEO_CODECS_KEY "maximum_video_bandwidth");
	  gdk_threads_leave ();
	  
	  vq = 25 - (int) ((double) (int) vq / 100 * 24);
	  OpalMediaFormat mediaFormat = stream->GetMediaFormat ();
          mediaFormat.SetOptionInteger (OpalVideoFormat::EncodingQualityOption,
                                        vq);  
          mediaFormat.SetOptionInteger (OpalVideoFormat::TargetBitRateOption, 
                                        bitrate * 8 * 1024);

	  stream->UpdateMediaFormat (mediaFormat);
	}
      }
    }

    PStringArray *order = new PStringArray ();
    PStringArray current_order = ep->GetMediaFormatOrder ();
    for (int i = 0 ; i < current_order.GetSize () ; i++)
      if (OpalMediaFormat (current_order [i]).GetDefaultSessionID () == 2)
        *order += current_order [i];
    ep->SetVideoMediaFormats (order);
    delete order;
  }
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
  GMManager *ep = NULL;
  
  PSafePtr<OpalCall> call = NULL;
  PSafePtr<OpalConnection> connection = NULL;
  RTP_Session *session = NULL;

  int min_val = 20;
  int max_val = 500;
  unsigned units = 8;

  ep = GnomeMeeting::Process ()->GetManager ();  
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    gdk_threads_enter ();
    min_val = gm_conf_get_int (AUDIO_CODECS_KEY "minimum_jitter_buffer");
    max_val = gm_conf_get_int (AUDIO_CODECS_KEY "maximum_jitter_buffer");
    gdk_threads_leave ();

    call = ep->FindCallWithLock (ep->GetCurrentCallToken ());
    if (call != NULL) {

      connection = ep->GetConnection (call, TRUE);

      if (connection != NULL) {

	session = 
	  connection->GetSession (OpalMediaFormat::DefaultAudioSessionID);

	if (session != NULL) {

	  units = session->GetJitterTimeUnits ();
	  session->SetJitterBufferSize (min_val * units, 
					max_val * units, 
					units);
	}
      }
    }
    else
      ep->SetAudioJitterDelay (PMAX (min_val, 20), PMIN (max_val, 1000));
  }
}


/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with an account changes.
 * BEHAVIOR     :  Updates the GUI and the registrations.
 * PRE          :  /
 */
static void 
accounts_list_changed_nt (gpointer id,
			  GmConfEntry *entry, 
			  gpointer data)
{
  GtkWidget *accounts_window = NULL;

  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();

  if (gm_conf_entry_get_type (entry) == GM_CONF_LIST) {

    gdk_threads_enter ();
    gm_accounts_window_update_accounts_list (accounts_window);
    gdk_threads_leave ();
  }

}


/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with the listening interface changes.
 * BEHAVIOR     :  Updates the interface.
 * PRE          :  /
 */
static void 
interface_changed_nt (gpointer id,
		      GmConfEntry *entry, 
		      gpointer data)
{
  GMManager *ep = GnomeMeeting::Process ()->GetManager ();
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING) {

    gdk_threads_enter ();
    ep->StartListeners ();
    gdk_threads_leave ();
  }

}


/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with the public ip changes.
 * BEHAVIOR     :  Updates the IP Translation address.
 * PRE          :  /
 */
static void 
public_ip_changed_nt (gpointer id,
		      GmConfEntry *entry, 
		      gpointer data)
{
  const char *public_ip = NULL;
  int nat_method = 0;
  GMManager *ep = NULL;
  
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING) {
    
    ep = GnomeMeeting::Process ()->GetManager ();
    
    gdk_threads_enter ();
    public_ip = gm_conf_entry_get_string (entry);
    nat_method = gm_conf_get_int (NAT_KEY "method");
    gdk_threads_leave ();

    if (nat_method == 2 && public_ip)
      ep->SetTranslationAddress (PString (public_ip));
    else
      ep->SetTranslationAddress (PString ("0.0.0.0"));
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
  GMManager *ep = NULL;
  
  ep = GnomeMeeting::Process ()->GetManager ();
  
  if ((gm_conf_entry_get_type (entry) == GM_CONF_STRING) ||
      (gm_conf_entry_get_type (entry) == GM_CONF_INT)) {

    ep->UpdateDevices ();
  }
}


/* DESCRIPTION  :  This callback is called when a video device setting changes
 *                 in the config database.
 * BEHAVIOR     :  It resets the video device if preview is enabled.
 * PRE          :  /
 */
static void 
video_device_setting_changed_nt (gpointer id, 
				 GmConfEntry *entry, 
				 gpointer data)
{
  PString name;

  BOOL preview = FALSE;

  GMManager *ep = NULL;

  ep = GnomeMeeting::Process ()->GetManager ();

  if ((gm_conf_entry_get_type (entry) == GM_CONF_STRING) ||
      (gm_conf_entry_get_type (entry) == GM_CONF_INT)) {
    
    if (ep && ep->GetCallingState () == GMManager::Standby) {

      gdk_threads_enter ();
      preview = gm_conf_get_bool (VIDEO_DEVICES_KEY "enable_preview");
      gdk_threads_leave ();

      if (preview)
	ep->CreateVideoGrabber ();
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
  GMManager *ep = NULL;
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {
   
    /* We reset the video device */
    ep = GnomeMeeting::Process ()->GetManager ();
    
    if (ep && ep->GetCallingState () == GMManager::Standby) {
    
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

	
/*	gnomemeeting_error_dialog (GTK_WIDGET_VISIBLE (data)?
				   GTK_WINDOW (data):
				   GTK_WINDOW (main_window),
				   _("Forward URL not specified"),
				   _("You need to specify an URL where to forward calls in the call forwarding section of the preferences!\n\nDisabling forwarding."));
            
	gm_conf_set_bool ((gchar *) gm_conf_entry_get_key (entry), FALSE);*/
	//FIXME
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


#if 0
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
  GMManager *endpoint = NULL;
  
  endpoint = GnomeMeeting::Process ()->GetManager ();
 
  if (gm_conf_entry_get_type (entry) == GM_CONF_INT
      || gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    if (endpoint)
      endpoint->UpdatePublishers ();
  }
}
#endif


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
  GtkWidget *statusicon = NULL;
  
  
  GMManager::CallingState calling_state = GMManager::Standby;
  GMManager *ep = NULL;

  gboolean forward_on_busy = FALSE;
  IncomingCallMode i;

  ep = GnomeMeeting::Process ()->GetManager ();
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  statusicon = GnomeMeeting::Process ()->GetStatusicon ();
  

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
    gm_statusicon_update_full (statusicon, calling_state, i, forward_on_busy);

    /* Update the main window and its menu */
    gm_main_window_set_incoming_call_mode (main_window, i);

    ep->UpdatePublishers ();
    
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
  gm_conf_set_int (GENERAL_KEY "kind_of_net", NET_CUSTOM);
  gdk_threads_leave ();
}


/* The functions */
gboolean 
gnomemeeting_conf_check ()
{
  int conf_test = -1;
  
  /* Check the config is ok */
  conf_test = gm_conf_get_int (GENERAL_KEY "gconf_test_age");
  
  if (conf_test != SCHEMA_AGE) 
    return FALSE;

  return TRUE;
}


void
gnomemeeting_conf_init ()
{
  GtkWidget *main_window = NULL;
  GtkWidget *prefs_window = NULL;
  GtkWidget *statusicon = NULL;
  
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();
  statusicon = GnomeMeeting::Process ()->GetStatusicon ();
  main_window = GnomeMeeting::Process ()->GetMainWindow ();


  /* Init gm_conf */
  gm_conf_watch ();

    
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

  
  /* Notifiers for the PERSONAL_DATA_KEY keys */
  gm_conf_notifier_add (PERSONAL_DATA_KEY "firstname",
			fullname_changed_nt, NULL);
  
  gm_conf_notifier_add (PERSONAL_DATA_KEY "lastname",
			fullname_changed_nt, NULL);

  
  /* Notifiers for the USER_INTERFACE_KEY keys */
  gm_conf_notifier_add (USER_INTERFACE_KEY "main_window/panel_section",
			panel_section_changed_nt, main_window);
  
  
  /* Notifiers for the CALL_OPTIONS_KEY keys */
  gm_conf_notifier_add (CALL_OPTIONS_KEY "incoming_call_mode",
			incoming_call_mode_changed_nt, NULL);
#if 0
  gm_conf_notifier_add (CALL_OPTIONS_KEY "incoming_call_mode",
			ils_option_changed_nt, NULL);
#endif
 

  /* Notifiers for the CALL_FORWARDING_KEY keys */
  gm_conf_notifier_add (CALL_FORWARDING_KEY "always_forward",
			call_forwarding_changed_nt, prefs_window);
  
  gm_conf_notifier_add (CALL_FORWARDING_KEY "forward_on_busy",
			call_forwarding_changed_nt, prefs_window);
  
  gm_conf_notifier_add (CALL_FORWARDING_KEY "forward_on_no_answer",
			call_forwarding_changed_nt, prefs_window);


  /* Notifiers related to the H323_KEY */
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

  gm_conf_notifier_add (H323_KEY "dtmf_mode",
			dtmf_mode_changed_nt, NULL);
  gm_conf_notifier_add (H323_KEY "dtmf_mode",
			applicability_check_nt, prefs_window);
  
  gm_conf_notifier_add (H323_KEY "default_gateway", 
			applicability_check_nt, prefs_window);
  
  
  /* Notifiers related to the SIP_KEY */
  gm_conf_notifier_add (SIP_KEY "outbound_proxy_host",
			applicability_check_nt, prefs_window);
  gm_conf_notifier_add (SIP_KEY "outbound_proxy_host",
			outbound_proxy_changed_nt, NULL);
  
  gm_conf_notifier_add (SIP_KEY "outbound_proxy_login",
			applicability_check_nt, prefs_window);
  gm_conf_notifier_add (SIP_KEY "outbound_proxy_login",
			outbound_proxy_changed_nt, NULL);
			
  gm_conf_notifier_add (SIP_KEY "outbound_proxy_password",
			applicability_check_nt, prefs_window);
  gm_conf_notifier_add (SIP_KEY "outbound_proxy_password",
			outbound_proxy_changed_nt, NULL);
  
  gm_conf_notifier_add (SIP_KEY "default_proxy",
			applicability_check_nt, prefs_window);

  gm_conf_notifier_add (SIP_KEY "dtmf_mode",
			dtmf_mode_changed_nt, NULL);
  gm_conf_notifier_add (SIP_KEY "dtmf_mode",
			applicability_check_nt, prefs_window);

  
#if 0 
  /* Notifiers related the LDAP_KEY */
  gm_conf_notifier_add (LDAP_KEY "enable_registering",
			ils_option_changed_nt, NULL);

  gm_conf_notifier_add (LDAP_KEY "show_details", ils_option_changed_nt, NULL);
#endif

  
  /* Notifiers for the PROTOCOLS_KEY */
  gm_conf_notifier_add (PROTOCOLS_KEY "accounts_list",
			accounts_list_changed_nt, NULL);
  
  gm_conf_notifier_add (PROTOCOLS_KEY "interface",
			interface_changed_nt, NULL);


  /* Notifier for the NAT_KEY */
  gm_conf_notifier_add (NAT_KEY "public_ip",
			public_ip_changed_nt, NULL);


  /* Notifiers to AUDIO_DEVICES_KEY */
  gm_conf_notifier_add (AUDIO_DEVICES_KEY "plugin", 
			manager_changed_nt, prefs_window);

  gm_conf_notifier_add (AUDIO_DEVICES_KEY "output_device",
			applicability_check_nt, prefs_window);
  
  gm_conf_notifier_add (AUDIO_DEVICES_KEY "input_device",
			applicability_check_nt, prefs_window);


  /* Notifiers to VIDEO_DEVICES_KEY */
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "plugin", 
			manager_changed_nt, prefs_window);
  
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "input_device", 
			video_device_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "input_device", 
			applicability_check_nt, prefs_window);

  gm_conf_notifier_add (VIDEO_DEVICES_KEY "channel", 
			video_device_setting_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "channel", 
			applicability_check_nt, prefs_window);

  gm_conf_notifier_add (VIDEO_DEVICES_KEY "size", 
			video_device_setting_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "size", 
			video_media_format_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "size", 
			applicability_check_nt, prefs_window);

  gm_conf_notifier_add (VIDEO_DEVICES_KEY "format", 
			video_device_setting_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "format", 
			applicability_check_nt, prefs_window);

  gm_conf_notifier_add (VIDEO_DEVICES_KEY "image", 
			video_device_setting_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "image", 
			applicability_check_nt, prefs_window);

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
  
  gm_conf_notifier_add (SOUND_EVENTS_KEY "enable_new_voicemail_sound", 
			sound_events_list_changed_nt, prefs_window);
  
  gm_conf_notifier_add (SOUND_EVENTS_KEY "new_voicemail_sound",
			sound_events_list_changed_nt, prefs_window);

  gm_conf_notifier_add (SOUND_EVENTS_KEY "enable_new_message_sound",
			sound_events_list_changed_nt, prefs_window);

  gm_conf_notifier_add (SOUND_EVENTS_KEY "new_message_sound",
			sound_events_list_changed_nt, prefs_window);

 
  /* Notifiers for the AUDIO_CODECS_KEY keys */
  gm_conf_notifier_add (AUDIO_CODECS_KEY "minimum_jitter_buffer", 
			jitter_buffer_changed_nt, NULL);

  gm_conf_notifier_add (AUDIO_CODECS_KEY "maximum_jitter_buffer", 
			jitter_buffer_changed_nt, NULL);

  gm_conf_notifier_add (AUDIO_CODECS_KEY "enable_silence_detection", 
			silence_detection_changed_nt, NULL);
  
  gm_conf_notifier_add (AUDIO_CODECS_KEY "enable_echo_cancelation", 
			echo_cancelation_changed_nt, NULL);

  
  /* Notifiers for the VIDEO_CODECS_KEY keys */
  gm_conf_notifier_add (VIDEO_CODECS_KEY "enable_video",
			network_settings_changed_nt, NULL);	     
  gm_conf_notifier_add (VIDEO_CODECS_KEY "enable_video", 
			enable_video_changed_nt, main_window);     
#if 0
  gm_conf_notifier_add (VIDEO_CODECS_KEY "enable_video", 
			ils_option_changed_nt, NULL);
#endif
  gm_conf_notifier_add (VIDEO_CODECS_KEY "enable_video", 
			applicability_check_nt, prefs_window);

  gm_conf_notifier_add (VIDEO_CODECS_KEY "maximum_video_bandwidth", 
			video_media_format_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_CODECS_KEY "maximum_video_bandwidth", 
			network_settings_changed_nt, NULL);

  gm_conf_notifier_add (VIDEO_CODECS_KEY "transmitted_video_quality",
			video_media_format_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_CODECS_KEY "transmitted_video_quality", 
			network_settings_changed_nt, NULL);
}


void 
gnomemeeting_conf_upgrade ()
{
  GSList *codecs = NULL;
  gchar *conf_url = NULL;

  int version = 0;

  version = gm_conf_get_int (GENERAL_KEY "version");
  
  /* Install the sip:, h323: and callto: GNOME URL Handlers */
  conf_url = gm_conf_get_string ("/desktop/gnome/url-handlers/callto/command");
					       
  if (!conf_url
      || !strcmp (conf_url, "gnomemeeting -c \"%s\"")) {

    
    gm_conf_set_string ("/desktop/gnome/url-handlers/callto/command", 
			"ekiga -c \"%s\"");

    gm_conf_set_bool ("/desktop/gnome/url-handlers/callto/needs_terminal", 
		      false);
    
    gm_conf_set_bool ("/desktop/gnome/url-handlers/callto/enabled", true);
  }
  g_free (conf_url);

  conf_url = gm_conf_get_string ("/desktop/gnome/url-handlers/h323/command");
  if (!conf_url 
      || !strcmp (conf_url, "gnomemeeting -c \"%s\"")) {

    gm_conf_set_string ("/desktop/gnome/url-handlers/h323/command", 
                        "ekiga -c \"%s\"");

    gm_conf_set_bool ("/desktop/gnome/url-handlers/h323/needs_terminal", false);

    gm_conf_set_bool ("/desktop/gnome/url-handlers/h323/enabled", true);
  }
  g_free (conf_url);

  conf_url = gm_conf_get_string ("/desktop/gnome/url-handlers/sip/command");
  if (!conf_url 
      || !strcmp (conf_url, "gnomemeeting -c \"%s\"")) {

    gm_conf_set_string ("/desktop/gnome/url-handlers/sip/command", 
                        "ekiga -c \"%s\"");

    gm_conf_set_bool ("/desktop/gnome/url-handlers/sip/needs_terminal", false);

    gm_conf_set_bool ("/desktop/gnome/url-handlers/sip/enabled", true);
  }
  g_free (conf_url);

  /* Upgrade IP detector IP address */
  conf_url = gm_conf_get_string (NAT_KEY "public_ip_detector");
  if (conf_url && !strcmp (conf_url, "http://213.193.144.104/ip/"))
    gm_conf_set_string (NAT_KEY "public_ip_detector", 
			"http://ekiga.net/ip/");
  g_free (conf_url);

  /* Upgrade the audio codecs list */
  codecs = g_slist_append (codecs, g_strdup ("106|16000|20800=1"));
  codecs = g_slist_append (codecs, g_strdup ("107|8000|13333=1"));
  codecs = g_slist_append (codecs, g_strdup ("3|8000|13200=1"));
  codecs = g_slist_append (codecs, g_strdup ("96|8000|12800=1"));
  codecs = g_slist_append (codecs, g_strdup ("105|8000|8000=1"));
  codecs = g_slist_append (codecs, g_strdup ("0|8000|64000=1"));
  codecs = g_slist_append (codecs, g_strdup ("8|8000|64000=1"));
  codecs = g_slist_append (codecs, g_strdup ("112|8000|16000=1"));
  codecs = g_slist_append (codecs, g_strdup ("110|8000|32000=1"));
  gm_conf_set_string_list (AUDIO_CODECS_KEY "list", codecs);
  g_slist_foreach (codecs, (GFunc) g_free, NULL);
  g_slist_free (codecs);
}
