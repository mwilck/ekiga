
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
 *   Additional code      : Miguel Rodríguez Pérez  <miguelrp@gmail.com>
 *
 */


#include "../config.h"

#include "config.h"

#include "h323endpoint.h"
#include "sipendpoint.h"
#include "endpoint.h"

#include "gnomemeeting.h"
#include "lid.h"
#include "pref_window.h"
#include "accounts.h"
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
 * BEHAVIOR     :  Sets the right page, and also sets 
 *                 the good value for the radio menu. If the control
 *                 panel is hidden (VIDEOPHONE view), then switch
 *                 to the FULLVIEW view.
 * PRE          :  /
 */
static void control_panel_section_changed_nt (gpointer, 
                                              GmConfEntry *, 
                                              gpointer);


/* DESCRIPTION  :  This callback is called when the view mode 
 *                 key changes.
 * BEHAVIOR     :  Shows/hides components and updates the UI.
 * PRE          :  The main window GMObject. 
 */
static void view_mode_changed_nt (gpointer, 
					   GmConfEntry *, 
					   gpointer);


/* DESCRIPTION  :  This callback is called when the firstname or last name
 *                 keys changes.
 * BEHAVIOR     :  Updates the ILS and ZeroConf registrations and the 
 * 		   main endpoint configuration.
 * PRE          :  /
 */
static void fullname_changed_nt (gpointer, 
				 GmConfEntry *, 
				 gpointer);

static void video_media_format_changed_nt (gpointer, 
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

static void accounts_list_changed_nt (gpointer,
				      GmConfEntry *, 
				      gpointer);

static void interface_changed_nt (gpointer,
				  GmConfEntry *, 
				  gpointer);

static void public_ip_changed_nt (gpointer,
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

static void outbound_proxy_changed_nt (gpointer,
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

static void echo_cancelation_changed_nt (gpointer, 
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
  ViewMode m = SOFTPHONE;

  g_return_if_fail (data != NULL);
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    gdk_threads_enter ();
    section = gm_conf_entry_get_int (entry);
    gm_main_window_set_control_panel_section (GTK_WIDGET (data), 
					      section);
    m = (ViewMode) gm_conf_get_int (USER_INTERFACE_KEY "main_window/view_mode");
    if (m == VIDEOPHONE)
      gm_conf_set_int (USER_INTERFACE_KEY "main_window/view_mode", FULLVIEW);
    gdk_threads_leave ();
  }
}


static void 
view_mode_changed_nt (gpointer id, 
		      GmConfEntry *entry, 
		      gpointer data)
{
  ViewMode m = SOFTPHONE;

  g_return_if_fail (data != NULL);
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    gdk_threads_enter ();
    m = (ViewMode) gm_conf_entry_get_int (entry);
    gm_main_window_set_view_mode (GTK_WIDGET (data), m);
    gdk_threads_leave ();
  }
}


static void 
fullname_changed_nt (gpointer id, 
		     GmConfEntry *entry, 
		     gpointer data)
{
  GMEndPoint *endpoint = NULL;

  endpoint = GnomeMeeting::Process ()->Endpoint ();
  
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
  
  GMEndPoint *ep = NULL;
  GMSIPEndPoint *sipEP = NULL;

  gchar *outbound_proxy_login = NULL;
  gchar *outbound_proxy_password = NULL;
  gchar *outbound_proxy_host = NULL;
  
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING) {

    ep = GnomeMeeting::Process ()->Endpoint ();
    sipEP = ep->GetSIPEndPoint ();
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
 *                 associated with the enable_video_transmission key changes.
 * BEHAVIOR     :  It updates the endpoint.
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
  }
}


/* DESCRIPTION  :  This notifier is called when the config database data
 *                 associated with the enable_video_transmission key changes.
 * BEHAVIOR     :  It updates the endpoint.
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
  
  GMEndPoint *ep = NULL;
  

  ep = GnomeMeeting::Process ()->Endpoint ();
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
  
  GMEndPoint *ep = NULL;
  

  ep = GnomeMeeting::Process ()->Endpoint ();
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

    ep->SetAllMediaFormats ();
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
  GMEndPoint *ep = NULL;
  OpalMediaStream *stream = NULL;
  PSafePtr<OpalCall> call = NULL;
  PSafePtr<OpalConnection> connection = NULL;
  
  int vq = 1;
  int bitrate = 2;

  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    ep = GnomeMeeting::Process ()->Endpoint ();

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
	  OpalMediaFormat mediaFormat (OPAL_H261_QCIF);
	  mediaFormat.SetOptionInteger (OpalVideoFormat::EncodingQualityOption,
					vq);
	  mediaFormat.SetOptionInteger (OpalVideoFormat::TargetBitRateOption, 
					bitrate * 8 * 1024);
	  stream->UpdateMediaFormat (mediaFormat);
	}
      }
    }
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

      connection = ep->GetConnection (call, TRUE);

      if (connection != NULL) {

	session = 
	  connection->GetSession (OpalMediaFormat::DefaultAudioSessionID);

	if (session != NULL) 
	  session->SetJitterBufferSize (min_val * 8, max_val * 8); 
      }
    }
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
  GMEndPoint *ep = GnomeMeeting::Process ()->Endpoint ();
  
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
  GMEndPoint *ep = NULL;
  
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_STRING) {
    
    ep = GnomeMeeting::Process ()->Endpoint ();
    
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
      
#ifdef HAS_IXJ
      //if (dev.Find ("/dev/phone") != P_MAX_INDEX) 
	//ep->CreateLid (dev);
      //else 
	//ep->RemoveLid ();
      //FIXME
      capa = ep->GetAvailableAudioMediaFormats ();
#endif

      /* Update the codecs list and the capabilities */
      gnomemeeting_threads_enter ();
      gm_prefs_window_update_audio_codecs_list (prefs_window, capa);
      gnomemeeting_threads_leave ();

      ep->SetAllMediaFormats ();
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
  
  ep = GnomeMeeting::Process ()->Endpoint ();
  
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

  GMEndPoint *ep = NULL;

  ep = GnomeMeeting::Process ()->Endpoint ();


  if ((gm_conf_entry_get_type (entry) == GM_CONF_STRING) ||
      (gm_conf_entry_get_type (entry) == GM_CONF_INT)) {
    
    if (ep && ep->GetCallingState () == GMEndPoint::Standby) {

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
    l = ep->GetAvailableAudioMediaFormats ();
    
    /* Update the GUI */
    gdk_threads_enter ();
    gm_prefs_window_update_audio_codecs_list (GTK_WIDGET (data), l);
    gdk_threads_leave ();

    ep->SetAllMediaFormats ();
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
      endpoint->UpdatePublishers ();
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
    if (tray)
      gm_tray_update (tray, calling_state, i, forward_on_busy);

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
  GtkWidget *prefs_window = NULL;
  GtkWidget *tray = NULL;
  
  int conf_test = -1;
  
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();
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

  
  /* Notifiers for the PERSONAL_DATA_KEY keys */
  gm_conf_notifier_add (PERSONAL_DATA_KEY "firstname",
			fullname_changed_nt, NULL);
  
  gm_conf_notifier_add (PERSONAL_DATA_KEY "lastname",
			fullname_changed_nt, NULL);

  
  /* Notifiers for the USER_INTERFACE_KEY keys */
  gm_conf_notifier_add (USER_INTERFACE_KEY "main_window/control_panel_section",
			control_panel_section_changed_nt, main_window);
  
  gm_conf_notifier_add (USER_INTERFACE_KEY "main_window/view_mode",
			view_mode_changed_nt, main_window);
  
  
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
			capabilities_changed_nt, NULL);
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

  gm_conf_notifier_add (H323_KEY "dtmf_mode",
			capabilities_changed_nt, NULL);
  gm_conf_notifier_add (H323_KEY "dtmf_mode",
			applicability_check_nt, prefs_window);

  
  /* Notifiers related the LDAP_KEY */
  gm_conf_notifier_add (LDAP_KEY "enable_registering",
			ils_option_changed_nt, NULL);

  gm_conf_notifier_add (LDAP_KEY "show_details", ils_option_changed_nt, NULL);
  
  
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
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "channel", 
			applicability_check_nt, prefs_window);

  gm_conf_notifier_add (VIDEO_DEVICES_KEY "size", 
			video_device_setting_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_DEVICES_KEY "size", 
			capabilities_changed_nt, NULL);
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
  
  gm_conf_notifier_add (AUDIO_CODECS_KEY "enable_echo_cancelation", 
			echo_cancelation_changed_nt, NULL);

  
  /* Notifiers for the VIDEO_CODECS_KEY keys */
  gm_conf_notifier_add (VIDEO_CODECS_KEY "enable_video_reception",
			network_settings_changed_nt, NULL);	     
  gm_conf_notifier_add (VIDEO_CODECS_KEY "enable_video_reception", 
			enable_video_reception_changed_nt, main_window);     
  gm_conf_notifier_add (VIDEO_CODECS_KEY "enable_video_reception", 
			applicability_check_nt, prefs_window);

  gm_conf_notifier_add (VIDEO_CODECS_KEY "enable_video_transmission", 
			network_settings_changed_nt, NULL);	     
  gm_conf_notifier_add (VIDEO_CODECS_KEY "enable_video_transmission",
			enable_video_transmission_changed_nt, NULL);	     
  gm_conf_notifier_add (VIDEO_CODECS_KEY "enable_video_transmission", 
			ils_option_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_CODECS_KEY "enable_video_transmission", 
			applicability_check_nt, prefs_window);
  
  gm_conf_notifier_add (VIDEO_CODECS_KEY "maximum_video_bandwidth", 
			video_media_format_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_CODECS_KEY "maximum_video_bandwidth",
			capabilities_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_CODECS_KEY "maximum_video_bandwidth", 
			network_settings_changed_nt, NULL);

  gm_conf_notifier_add (VIDEO_CODECS_KEY "transmitted_video_quality",
			video_media_format_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_CODECS_KEY "transmitted_video_quality",
			capabilities_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_CODECS_KEY "transmitted_video_quality", 
			network_settings_changed_nt, NULL);


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

  conf_url = gm_conf_get_string (NAT_KEY "public_ip_detector");
  if (conf_url && !strcmp (conf_url, "http://213.193.144.104/ip/"))
    gm_conf_set_string (NAT_KEY "public_ip_detector", 
			"http://gnomemeeting.net/ip/");
  g_free (conf_url);
}
