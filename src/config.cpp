
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
 *   description          : This file contains most of gconf stuff.
 *                          All notifiers are here.
 *                          Callbacks that updates the gconf cache 
 *                          are in their file, except some generic one that
 *                          are in this file.
 *   Additional code      : Miguel Rodríguez Pérez  <migrax@terra.es>
 *
 */


#include "../config.h"

#include "config.h"
#include "connection.h"
#include "endpoint.h"
#include "gnomemeeting.h"
#include "videograbber.h"
#include "ils.h"
#include "lid.h"
#include "urlhandler.h"
#include "sound_handling.h"
#include "pref_window.h"
#include "ldap_window.h"
#include "tray.h"
#include "misc.h"
#include "tools.h"

#include "dialog.h"
#include "stock-icons.h"
#include "gtk_menu_extensions.h"
#include "gconf_widgets_extensions.h"



/* Declarations */
extern GtkWidget *gm;


static void applicability_check_nt (GConfClient *,
				    guint,
				    GConfEntry *,
				    gpointer);

static void main_notebook_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void fps_limit_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void maximum_video_bandwidth_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void tr_vq_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void tr_ub_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void jitter_buffer_changed_nt (GConfClient*, guint, GConfEntry *, 
				      gpointer);
static void register_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void ldap_visible_changed_nt (GConfClient*, guint, 
				     GConfEntry *, gpointer);
static void stay_on_top_changed_nt (GConfClient*, guint, 
				    GConfEntry *, gpointer);

static void calls_history_changed_nt (GConfClient*,
				      guint, 
				      GConfEntry *,
				      gpointer);

static void incoming_call_mode_changed_nt (GConfClient*,
					   guint, 
					   GConfEntry *,
					   gpointer);

static void call_forwarding_changed_nt (GConfClient*,
					guint,
					GConfEntry *, 
					gpointer);

static void manager_changed_nt (GConfClient *,
				guint,
				GConfEntry *, 
				gpointer);

static void audio_device_changed_nt (GConfClient *,
				     guint,
				     GConfEntry *, 
				     gpointer);

static void video_device_changed_nt (GConfClient *, 
				     guint, 
				     GConfEntry *, 
				     gpointer);

static void video_device_setting_changed_nt (GConfClient *, 
					     guint, 
					     GConfEntry *, 
					     gpointer);

static void video_preview_changed_nt (GConfClient *, guint, GConfEntry *, 
				      gpointer);

static void sound_events_list_changed_nt (GConfClient *,
					  guint,
					  GConfEntry *, 
					  gpointer);

static void audio_codecs_list_changed_nt (GConfClient *, guint, GConfEntry *, 
					  gpointer);
static void contacts_sections_list_group_content_changed_nt (GConfClient *,
							     guint, 
							     GConfEntry *, 
							     gpointer);
static void contacts_sections_list_changed_nt (GConfClient *, guint, 
					       GConfEntry *, gpointer);
static void view_widget_changed_nt (GConfClient *, guint, GConfEntry *, 
				    gpointer);
static void capabilities_changed_nt (GConfClient *, guint, 
				     GConfEntry *, gpointer);

static void h245_tunneling_changed_nt (GConfClient *,
				       guint,
				       GConfEntry *,
				       gpointer);

static void early_h245_changed_nt (GConfClient *,
				   guint,
				   GConfEntry *,
				   gpointer);

static void fast_start_changed_nt (GConfClient *,
				   guint,
				   GConfEntry *,
				   gpointer);

static void use_gateway_changed_nt (GConfClient *,
				    guint, 
				    GConfEntry *,
				    gpointer);

static void enable_video_transmission_changed_nt (GConfClient *, 
						  guint, 
						  GConfEntry *, 
						  gpointer);

static void enable_video_reception_changed_nt (GConfClient *, 
					       guint, 
					       GConfEntry *, 
					       gpointer);

static void silence_detection_changed_nt (GConfClient *, guint, 
					  GConfEntry *, gpointer);
static void network_settings_changed_nt (GConfClient *, guint, 
					 GConfEntry *, gpointer);

#ifdef HAS_IXJ
static void lid_aec_changed_nt (GConfClient *,
				guint,
				GConfEntry *,
				gpointer);

static void lid_country_changed_nt (GConfClient *,
				    guint,
				    GConfEntry *, 
				    gpointer);

static void lid_output_device_type_changed_nt (GConfClient *,
					       guint,
					       GConfEntry *, 
					       gpointer);
#endif


/* 
 * Generic notifiers that update specific widgets when a gconf key changes
 */
/* DESCRIPTION  :  Generic notifiers for toggles in the menu.
 *                 This callback is called when a specific key of
 *                 the gconf database associated with a toggle changes, this
 *                 only updates the toggle in the menu.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  /
 */
static void menu_toggle_changed_nt (GConfClient *client, guint cid, 
				    GConfEntry *entry, gpointer data)
{
  GtkWidget *e = NULL;
  
  if (entry->value->type == GCONF_VALUE_BOOL) {
   
    gdk_threads_enter ();
  
    e = GTK_WIDGET (data);

    /* We set the new value for the widget */
    GTK_CHECK_MENU_ITEM (e)->active = 
      gconf_value_get_bool (entry->value);

    gtk_widget_queue_draw (GTK_WIDGET (e));

    gdk_threads_leave (); 
  }
}

/* DESCRIPTION  :  Notifiers for radios menu.
 *                 This callback is called when a specific key of
 *                 the gconf database associated with a radio menu changes, 
 *                 this only updates the radio in the menu.
 * BEHAVIOR     :  It updates the widget.
 * PRE          :  One of the GtkCheckMenuItem of the radio menu.
 */
static void 
radio_menu_changed_nt (GConfClient *client, 
		       guint cid, 
		       GConfEntry *entry, 
		       gpointer data)
{
  if (entry->value->type == GCONF_VALUE_INT) {
   
    gdk_threads_enter ();
  
    gtk_radio_menu_select_with_widget (GTK_WIDGET (data),
				       gconf_value_get_int (entry->value));
    
    gdk_threads_leave (); 
  }
}

/* FIX ME: should be moved to the menu */


/* DESCRIPTION  :  This callback is called when something changes in the view
 *                 directory (either from the menu, either from the prefs).
 * BEHAVIOR     :  It shows/hides the corresponding widget.
 * PRE          :  /
 */
static void view_widget_changed_nt (GConfClient *client, guint cid, 
				    GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_BOOL) {

    gdk_threads_enter ();
  
    if (gconf_value_get_bool (entry->value))
      gtk_widget_show_all (GTK_WIDGET (data));
    else
      gtk_widget_hide_all (GTK_WIDGET (data));
    
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Displays a popup if we are in a call.
 * PRE          :  /
 */
static void
applicability_check_nt (GConfClient *client,
			guint cid, 
			GConfEntry *entry,
			gpointer data)
{
  GmWindow *gw = NULL;
  GMH323EndPoint *ep = NULL;
  
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  if ((entry->value->type == GCONF_VALUE_BOOL)
      ||(entry->value->type == GCONF_VALUE_STRING)
      ||(entry->value->type == GCONF_VALUE_INT)) {

    if (ep->GetCallingState () != GMH323EndPoint::Standby) {

      gdk_threads_enter ();
      gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (gw->pref_window), gconf_entry_get_key (entry), _("Changing this setting will only affect new calls"), _("You have changed a setting that doesn't permit to GnomeMeeting to apply the new change to the current call. Your new setting will only take effect for the next call."));
      gdk_threads_leave ();
    }
  }
}


/* DESCRIPTION  :  This callback is called when the control panel 
 *                 section changes.
 * BEHAVIOR     :  Sets the right page or hide it, and also sets 
 *                 the good value for the toggle in the prefs.
 * PRE          :  /
 */
static void main_notebook_changed_nt (GConfClient *client, guint cid, 
				      GConfEntry *entry, gpointer data)
{
  GmWindow *gw = NULL;

  if (entry->value->type == GCONF_VALUE_INT) {

    gdk_threads_enter ();

    gw = GnomeMeeting::Process ()->GetMainWindow ();

    if (gconf_value_get_int (entry->value) == GM_MAIN_NOTEBOOK_HIDDEN)
      gtk_widget_hide_all (gw->main_notebook);
    else {

      gtk_widget_show_all (gw->main_notebook);
      gtk_notebook_set_current_page (GTK_NOTEBOOK (gw->main_notebook),
				     gconf_value_get_int (entry->value));
    }

    gdk_threads_leave ();

  }
}


/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the H.245 Tunneling changes.
 * BEHAVIOR     :  It updates the endpoint and displays a message.
 * PRE          :  /
 */
static void
h245_tunneling_changed_nt (GConfClient *client,
			   guint cid, 
			   GConfEntry *entry,
			   gpointer data)
{
  GMH323EndPoint *ep = NULL;
  GmWindow *gw = NULL;
  
  if (entry->value->type == GCONF_VALUE_BOOL) {

    ep = GnomeMeeting::Process ()->Endpoint ();
    gw = GnomeMeeting::Process ()->GetMainWindow ();
    
    ep->DisableH245Tunneling (!gconf_value_get_bool (entry->value));
    
    gdk_threads_enter ();
    gnomemeeting_log_insert (gw->history_text_view,
			     ep->IsH245TunnelingDisabled ()?
			     _("H.245 Tunneling disabled"):
			     _("H.245 Tunneling enabled"));
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the early H.245 key changes.
 * BEHAVIOR     :  It updates the endpoint and displays a message.
 * PRE          :  /
 */
static void
early_h245_changed_nt (GConfClient *client,
		       guint cid, 
		       GConfEntry *entry,
		       gpointer data)
{
  GMH323EndPoint *ep = NULL;
  GmWindow *gw = NULL;
  
  if (entry->value->type == GCONF_VALUE_BOOL) {

    ep = GnomeMeeting::Process ()->Endpoint ();
    gw = GnomeMeeting::Process ()->GetMainWindow ();
    
    ep->DisableH245inSetup (!gconf_value_get_bool (entry->value));
    
    gdk_threads_enter ();
    gnomemeeting_log_insert (gw->history_text_view,
			     ep->IsH245inSetupDisabled ()?
			     _("Early H.245 disabled"):
			     _("Early H.245 enabled"));
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the Fast Start changes.
 * BEHAVIOR     :  It updates the endpoint and displays a message.
 * PRE          :  /
 */
static void
fast_start_changed_nt (GConfClient *client,
		       guint cid, 
		       GConfEntry *entry,
		       gpointer data)
{
  GMH323EndPoint *ep = NULL;
  GmWindow *gw = NULL;
  
  if (entry->value->type == GCONF_VALUE_BOOL) {

    ep = GnomeMeeting::Process ()->Endpoint ();
    gw = GnomeMeeting::Process ()->GetMainWindow ();
    
    ep->DisableFastStart (!gconf_value_get_bool (entry->value));
    
    gdk_threads_enter ();
    gnomemeeting_log_insert (gw->history_text_view,
			     ep->IsFastStartDisabled ()?
			     _("Fast Start disabled"):
			     _("Fast Start enabled"));
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the use_gateway changes.
 * BEHAVIOR     :  It checks if the key can be enabled or not following
 *                 if a gateway has been specified or not.
 * PRE          :  /
 */
static void
use_gateway_changed_nt (GConfClient *client,
			guint cid, 
			GConfEntry *entry,
			gpointer data)
{
  GmWindow *gw = NULL;
  PString gateway;
  gchar *gateway_string;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  if (entry->value->type == GCONF_VALUE_BOOL) {

    gdk_threads_enter ();
    gateway_string = gconf_get_string (H323_GATEWAY_KEY "host");
    gateway = gateway_string;

    if (gateway.IsEmpty () && gconf_value_get_bool (entry->value)) {
      
      gnomemeeting_warning_dialog (GTK_WINDOW (gw->pref_window), _("No gateway or proxy specified"), _("You need to specify a host to use as gateway or proxy."));
      gconf_set_bool (H323_GATEWAY_KEY "use_gateway", FALSE);
    }

    g_free (gateway_string);
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the enable_video_transmission key changes.
 * BEHAVIOR     :  It updates the endpoint, and updates the registering on ILS.
 *                 If the user is in a call, the video channel will be started
 *                 and stopped on-the-fly.
 * PRE          :  /
 */
static void
enable_video_transmission_changed_nt (GConfClient *client,
				      guint cid, 
				      GConfEntry *entry,
				      gpointer data)
{
  PString name;
  GMH323EndPoint *ep = NULL;

  ep = GnomeMeeting::Process ()->Endpoint ();

  if (entry->value->type == GCONF_VALUE_BOOL) {

    ep->SetAutoStartTransmitVideo (gconf_value_get_bool (entry->value));

    if (gconf_get_int (VIDEO_DEVICES_KEY "size") == 0)
      name = "H.261-QCIF";
    else
      name = "H.261-CIF";

    if (gconf_value_get_bool (entry->value)) {
	
      ep->StartLogicalChannel (name,
			       RTP_Session::DefaultVideoSessionID,
			       FALSE);
    }
    else {

      ep->StopLogicalChannel (RTP_Session::DefaultVideoSessionID,
			      FALSE);
    }

    gdk_threads_enter ();
    if (gconf_client_get_bool (client, LDAP_KEY "enable_registering", 0))
      ep->ILSRegister ();
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the enable_video_transmission key changes.
 * BEHAVIOR     :  It updates the endpoint.
 *                 If the user is in a call, the video recpetion will be 
 *                 stopped on-the-fly if required.
 * PRE          :  /
 */
static void
enable_video_reception_changed_nt (GConfClient *client,
				   guint cid, 
				   GConfEntry *entry,
				   gpointer data)
{
  PString name;
  GMH323EndPoint *ep = NULL;

  ep = GnomeMeeting::Process ()->Endpoint ();

  if (entry->value->type == GCONF_VALUE_BOOL) {

    ep->SetAutoStartReceiveVideo (gconf_value_get_bool (entry->value));

    if (gconf_get_int (VIDEO_DEVICES_KEY "size") == 0)
      name = "H.261-QCIF";
    else
      name = "H.261-CIF";

    if (!gconf_value_get_bool (entry->value)) {
	
      ep->StopLogicalChannel (RTP_Session::DefaultVideoSessionID,
			      TRUE);
    }
    else {

      if (ep->GetCallingState () != GMH323EndPoint::Standby) {

	gdk_threads_enter ();
	gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (gm), gconf_entry_get_key (entry), _("Changing this setting will only affect new calls"), _("You have changed a setting that doesn't permit to GnomeMeeting to apply the new change to the current call. Your new setting will only take effect for the next call."));
	gdk_threads_leave ();
      }
    }
  }
}


/* DESCRIPTION  :  This callback is called when a silence detection key of
 *                 the gconf database associated with a toggle changes.
 * BEHAVIOR     :  It only updates the silence detection if we
 *                 are in a call. 
 * PRE          :  /
 */
static void silence_detection_changed_nt (GConfClient *client, guint cid, 
					  GConfEntry *entry, gpointer data)
{
  H323Codec *raw_codec = NULL;
  H323Connection *connection = NULL;
  H323Channel *channel = NULL;
  H323AudioCodec *ac = NULL;
  GMH323EndPoint *endpoint = NULL;
  
  GmWindow *gw = NULL;
  endpoint = GnomeMeeting::Process ()->Endpoint ();
  
  if (entry->value->type == GCONF_VALUE_BOOL) {

    connection = 
      endpoint->FindConnectionWithLock (endpoint->GetCurrentCallToken ());

    if (connection) {

      channel = 
	connection->FindChannel (RTP_Session::DefaultAudioSessionID, 
				 FALSE);

      if (channel)
	raw_codec = channel->GetCodec();
      
      if (raw_codec && raw_codec->IsDescendant (H323AudioCodec::Class())) {

	ac = (H323AudioCodec *) raw_codec;
      }
   
      /* We update the silence detection */
      if (ac && GnomeMeeting::Process ()->Endpoint ()->GetCallingState () == GMH323EndPoint::Connected) {

	gdk_threads_enter ();
	gw = GnomeMeeting::Process ()->GetMainWindow ();
	
	if (ac != NULL) {
	  
	  H323AudioCodec::SilenceDetectionMode mode = 
	    ac->GetSilenceDetectionMode();
	  
	  if (mode == H323AudioCodec::AdaptiveSilenceDetection) {
	    
	    mode = H323AudioCodec::NoSilenceDetection;
	    gnomemeeting_log_insert (gw->history_text_view,
				     _("Disabled Silence Detection"));
	  } 
	  else {
	    
	    mode = H323AudioCodec::AdaptiveSilenceDetection;
	    gnomemeeting_log_insert (gw->history_text_view,
				     _("Enabled Silence Detection"));
	  }
	  gdk_threads_leave ();  
	  
	  ac->SetSilenceDetectionMode(mode);
	}
      }

      connection->Unlock ();
    }
  }
}


/* DESCRIPTION  :  This callback is called to update capabilities.
 * BEHAVIOR     :  Updates them.
 * PRE          :  /
 */
static void
capabilities_changed_nt (GConfClient *client,
			 guint i, 
			 GConfEntry *entry,
			 gpointer data)
{
  GMH323EndPoint *ep = NULL;

  if (entry->value->type == GCONF_VALUE_INT
      || entry->value->type == GCONF_VALUE_LIST
      || entry->value->type == GCONF_VALUE_STRING) {
   
    ep = GnomeMeeting::Process ()->Endpoint ();

    ep->RemoveAllCapabilities ();
    ep->AddAllCapabilities ();
  }
}


/* DESCRIPTION  :  This callback is called to update the min fps limitation.
 * BEHAVIOR     :  Update it.
 * PRE          :  /
 */
static void fps_limit_changed_nt (GConfClient *client, guint cid, 
				  GConfEntry *entry, gpointer data)
{
  H323Connection *connection = NULL;
  H323Channel *channel = NULL;
  H323Codec *raw_codec = NULL;
  H323VideoCodec *vc = NULL;
  GMH323EndPoint *endpoint = NULL;

  endpoint = GnomeMeeting::Process ()->Endpoint ();
  
  int fps = 30;
  double frame_time = 0.0;

  if (entry->value->type == GCONF_VALUE_INT) {

    connection =
      endpoint->FindConnectionWithLock (endpoint->GetCurrentCallToken ());

    if (connection) {

      channel = 
	connection->FindChannel (RTP_Session::DefaultVideoSessionID, 
				 FALSE);

      if (channel)
	raw_codec = channel->GetCodec();
      
      if (raw_codec && raw_codec->IsDescendant (H323VideoCodec::Class())) {

	vc = (H323VideoCodec *) raw_codec;
      }
   

      /* We update the minimum fps limit */
      fps = gconf_value_get_int (entry->value);
      frame_time = (unsigned) (1000.0 / fps);
      frame_time = PMAX (33, PMIN(1000000, frame_time));

      if (vc != NULL)
	vc->SetTargetFrameTimeMs ((unsigned int) frame_time);

      connection->Unlock ();
    }
  }
}


/* DESCRIPTION  :  This callback is called when the user changes the maximum
 *                 video bandwidth.
 * BEHAVIOR     :  It updates it.
 * PRE          :  /
 */
static void 
maximum_video_bandwidth_changed_nt (GConfClient *client, guint cid, 
				    GConfEntry *entry, gpointer data)
{
  H323Channel *channel = NULL;
  H323Codec *raw_codec = NULL;
  H323VideoCodec *vc = NULL;
  H323Connection *connection = NULL;
  GMH323EndPoint *endpoint = NULL;

  int bitrate = 2;

  endpoint = GnomeMeeting::Process ()->Endpoint ();
  

  if (entry->value->type == GCONF_VALUE_INT) {

    connection =
	endpoint->FindConnectionWithLock (endpoint->GetCurrentCallToken ());

    if (connection) {

      channel = 
	connection->FindChannel (RTP_Session::DefaultVideoSessionID, 
				 FALSE);

      if (channel)
	raw_codec = channel->GetCodec();
      
      if (raw_codec && raw_codec->IsDescendant (H323VideoCodec::Class())) {

	vc = (H323VideoCodec *) raw_codec;
      }

      /* We update the video quality */  
      bitrate = gconf_value_get_int (entry->value) * 8 * 1024;
  
      if (vc != NULL)
	vc->SetMaxBitRate (bitrate);

      connection->Unlock ();
    }
  }
}


/* DESCRIPTION  :  This callback is called the transmitted video quality.
 * BEHAVIOR     :  It updates the video quality.
 * PRE          :  /
 */
static void tr_vq_changed_nt (GConfClient *client, guint cid, 
			      GConfEntry *entry, gpointer data)
{
  H323Connection *connection = NULL;
  H323Channel *channel = NULL;
  H323Codec *raw_codec = NULL;
  H323VideoCodec *vc = NULL;
  GMH323EndPoint *endpoint = NULL;

  int vq = 1;

  endpoint = GnomeMeeting::Process ()->Endpoint ();

  if (entry->value->type == GCONF_VALUE_INT) {

    connection =
      endpoint->FindConnectionWithLock (endpoint->GetCurrentCallToken ());

    if (connection) {

      channel = 
	connection->FindChannel (RTP_Session::DefaultVideoSessionID, 
				 FALSE);

      if (channel)
	raw_codec = channel->GetCodec();
      
      if (raw_codec && raw_codec->IsDescendant (H323VideoCodec::Class())) 
	vc = (H323VideoCodec *) raw_codec;

      /* We update the video quality */
      vq = 25 - (int) ((double) (int) gconf_value_get_int (entry->value) / 100 * 24);
  
      if (vc != NULL)
	vc->SetTxMaxQuality (vq);

      connection->Unlock ();
    }
  }
}


/* DESCRIPTION  :  This callback is called when the bg fill needs to be changed.
 * BEHAVIOR     :  It updates the background fill.
 * PRE          :  /
 */
static void tr_ub_changed_nt (GConfClient *client, guint cid, 
			      GConfEntry *entry, gpointer data)
{
  H323Connection *connection = NULL;
  H323Channel *channel = NULL;
  H323Codec *raw_codec = NULL;
  H323VideoCodec *vc = NULL;
  GMH323EndPoint *endpoint = NULL;

  endpoint = GnomeMeeting::Process ()->Endpoint ();

  if (entry->value->type == GCONF_VALUE_INT) {

    connection =
	endpoint->FindConnectionWithLock (endpoint->GetCurrentCallToken ());

    if (connection) {

      channel = 
	connection->FindChannel (RTP_Session::DefaultVideoSessionID, 
				 FALSE);

      if (channel)
	raw_codec = channel->GetCodec();
      
      if (raw_codec && raw_codec->IsDescendant (H323VideoCodec::Class())) {

	vc = (H323VideoCodec *) raw_codec;
      }

      /* We update the current tr ub rate */
      if (vc != NULL)
	vc->SetBackgroundFill ((int) gconf_value_get_int (entry->value));
      
      connection->Unlock ();
    }
  }
}


/* DESCRIPTION  :  This callback is called when the jitter buffer needs to be 
 *                 changed.
 * BEHAVIOR     :  It updates the widget and the value.
 * PRE          :  /
 */
static void jitter_buffer_changed_nt (GConfClient *client, guint cid, 
				      GConfEntry *entry, gpointer data)
{
  RTP_Session *session = NULL;  
  H323Connection *connection = NULL;
  GMH323EndPoint *ep = GnomeMeeting::Process ()->Endpoint ();  
  gdouble min_val = 20.0;
  gdouble max_val = 500.0;

  if (entry->value->type == GCONF_VALUE_INT) {

    min_val = 
      gconf_client_get_int (client, AUDIO_CODECS_KEY "minimum_jitter_buffer", 0);
    max_val = 
      gconf_client_get_int (client, AUDIO_CODECS_KEY "maximum_jitter_buffer", 0);
			    
    /* We update the current value */
    connection = 
      ep->FindConnectionWithLock (ep->GetCurrentCallToken ());

    if (connection) {

      session =                                                                
        connection->GetSession (OpalMediaFormat::DefaultAudioSessionID);       
      connection->Unlock ();
    }

                                                                               
    if (session != NULL)                                                       
      session->SetJitterBufferSize ((int) min_val * 8, (int) max_val * 8); 
  }
}


/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the audio or video manager changes.
 * BEHAVIOR     :  Updates the devices list for the new manager.
 * PRE          :  /
 */
static void
manager_changed_nt (GConfClient *client,
		    guint cid, 
		    GConfEntry *entry,
		    gpointer data)
{
  if (entry->value->type == GCONF_VALUE_STRING) {

    GnomeMeeting::Process ()->DetectDevices ();

    gdk_threads_enter ();
    gnomemeeting_pref_window_update_devices_list ();
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the audio devices changes.
 * BEHAVIOR     :  If a Quicknet device is used, then the Quicknet LID thread
 *                 is created. If not, it is removed provided we are not in
 *                 a call.
 *                 Notice that audio devices can not be changed during a call.
 * PRE          :  /
 */
static void
audio_device_changed_nt (GConfClient *client,
			 guint cid, 
			 GConfEntry *entry,
			 gpointer data)
{
  GMH323EndPoint *ep = NULL;
  GmPrefWindow *pw = NULL;

  PString dev;

  pw = GnomeMeeting::Process ()->GetPrefWindow ();
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  if (entry->value->type == GCONF_VALUE_STRING) {

    dev = gconf_value_get_string (entry->value);

    if (ep->GetCallingState () == GMH323EndPoint::Standby
	&& gconf_entry_get_key (entry)
	&& !strcmp (gconf_entry_get_key (entry),
		    AUDIO_DEVICES_KEY "input_device")) {
      
      if (dev.Find ("phone") != P_MAX_INDEX) 
	ep->CreateLid (dev);
      else 
	ep->RemoveLid ();
    }
  }
}



/* DESCRIPTION  :  This callback is called when the video device changes
 *                 in the gconf database.
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
video_device_changed_nt (GConfClient *client, 
			 guint cid, 
			 GConfEntry *entry, 
			 gpointer data)
{
  GMH323EndPoint *ep = NULL;
  BOOL preview = FALSE;
  
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  if ((entry->value->type == GCONF_VALUE_STRING) ||
      (entry->value->type == GCONF_VALUE_INT)) {

    if (ep && ep->GetCallingState () == GMH323EndPoint::Standby) {

      gdk_threads_enter ();
      preview = gconf_get_bool (VIDEO_DEVICES_KEY "enable_preview");
      gdk_threads_leave ();

      if (preview)
	ep->CreateVideoGrabber ();
      else 
	ep->RemoveVideoGrabber ();
    }
  }
}


/* DESCRIPTION  :  This callback is called when a video device setting changes
 *                 in the gconf database.
 * BEHAVIOR     :  It resets the video transmission if any, or resets the
 *                 video device if preview is enabled otherwise. Notice that
 *                 the video device can't be changed during calls, but its
 *                 settings can be changed. It also updates the capabilities.
 * PRE          :  /
 */
static void 
video_device_setting_changed_nt (GConfClient *client, 
				 guint cid, 
				 GConfEntry *entry, 
				 gpointer data)
{
  PString name;

  int max_try = 0;
  BOOL preview = FALSE;
  BOOL no_error = FALSE;

  GMH323EndPoint *ep = NULL;

  ep = GnomeMeeting::Process ()->Endpoint ();


  if ((entry->value->type == GCONF_VALUE_STRING) ||
      (entry->value->type == GCONF_VALUE_INT)) {
  
    ep->AddAllCapabilities ();
    
    if (ep && ep->GetCallingState () == GMH323EndPoint::Standby) {

      gdk_threads_enter ();
      preview = gconf_get_bool (VIDEO_DEVICES_KEY "enable_preview");
      gdk_threads_leave ();

      if (preview)
	ep->CreateVideoGrabber ();
    }
    else if (ep->GetCallingState () == GMH323EndPoint::Connected) {

      gdk_threads_enter ();
      if (gconf_get_int (VIDEO_DEVICES_KEY "size") == 0)
	name = "H.261-QCIF";
      else
	name = "H.261-CIF";
      gdk_threads_leave ();

      if (gconf_get_bool (VIDEO_CODECS_KEY "enable_video_transmission")) {

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

	if (!no_error) {

	  gdk_threads_enter ();
	  gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Failed to restart the video channel"), _("You have changed a video device related setting during a call. That requires to restart the video transmission channel, but it failed."));
	  gdk_threads_leave ();
	}
      }
    }
  }
}


/* DESCRIPTION  :  This callback is called when the video preview changes in
 *                 the gconf database.
 * BEHAVIOR     :  It starts or stops the preview.
 * PRE          :  /
 */
static void video_preview_changed_nt (GConfClient *client, guint cid, 
				      GConfEntry *entry, gpointer data)
{
  GMH323EndPoint *ep = NULL;
  
  if (entry->value->type == GCONF_VALUE_BOOL) {
   
    /* We reset the video device */
    ep = GnomeMeeting::Process ()->Endpoint ();
    
    if (ep && ep->GetCallingState () == GMH323EndPoint::Standby) {
    
      if (gconf_value_get_bool (entry->value)) 
	ep->CreateVideoGrabber ();
      else 
	ep->RemoveVideoGrabber ();
    }
  }
}


/* DESCRIPTION  :  This callback is called when something changes in the sound
 *                 events list.
 * BEHAVIOR     :  It updates the events list widget.
 * PRE          :  /
 */
static void
sound_events_list_changed_nt (GConfClient *client,
			      guint cid, 
			      GConfEntry *entry,
			      gpointer data)
{ 
  GmPrefWindow *pw = NULL;

  pw = GnomeMeeting::Process ()->GetPrefWindow ();

  if (entry->value->type == GCONF_VALUE_STRING
      || entry->value->type == GCONF_VALUE_BOOL) {
   
    gdk_threads_enter ();
    gnomemeeting_prefs_window_sound_events_list_build (GTK_TREE_VIEW (pw->sound_events_list));
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This callback is called when something changes in the audio
 *                 codecs clist.
 * BEHAVIOR     :  It updates the codecs list widget.
 *                 endpoint.
 * PRE          :  /
 */
static void
audio_codecs_list_changed_nt (GConfClient *client,
			      guint cid, 
			      GConfEntry *entry,
			      gpointer data)
{
#ifdef HAS_IXJ
  GMLid *lid = NULL;
#endif

  GMH323EndPoint *ep = NULL;
  GmPrefWindow *pw = NULL;

  BOOL use_quicknet = FALSE;
  BOOL soft_codecs_supported = FALSE;
  
  if (entry->value->type == GCONF_VALUE_LIST) {
   
    pw = GnomeMeeting::Process ()->GetPrefWindow ();
    ep = GnomeMeeting::Process ()->Endpoint ();

#ifdef HAS_IXJ
    lid = ep->GetLid ();
    if (lid) {
      
      use_quicknet = TRUE;
      soft_codecs_supported = lid->areSoftwareCodecsSupported ();
      lid->Unlock ();
    }
#endif

    
    /* Update the GUI */
    gdk_threads_enter ();
    gnomemeeting_codecs_list_build (pw->codecs_list_store,
				    use_quicknet,
				    soft_codecs_supported);
    gdk_threads_leave ();
  } 
}


static void
contacts_sections_list_group_content_changed_nt (GConfClient *client, 
						 guint cid,
						 GConfEntry *e, gpointer data)
{
  const char *gconf_key = NULL;
  gchar **group_split = NULL;
  gchar *group_name = NULL;
  gchar *group_name_unescaped = NULL;
  
  int cpt = 0;

  GtkWidget *page = NULL;
  GtkListStore *list_store = NULL;

  GmWindow *gw = NULL;
  GmLdapWindow *lw = NULL;
  GmLdapWindowPage *lwp = NULL;
  
  if (e->value->type == GCONF_VALUE_LIST) {
  
    gdk_threads_enter ();

    lw = GnomeMeeting::Process ()->GetLdapWindow ();
    gw = GnomeMeeting::Process ()->GetMainWindow ();
    
    gconf_key = gconf_entry_get_key (e);

    if (gconf_key) {
      
      group_split = g_strsplit (gconf_key, CONTACTS_GROUPS_KEY, 2);

      if (group_split [1])
	group_name = g_utf8_strdown (group_split [1], -1);

      if (group_name) {

	while ((page =
		gtk_notebook_get_nth_page (GTK_NOTEBOOK (lw->notebook),
					   cpt)) ){

	  lwp = gnomemeeting_get_ldap_window_page (page);

	  if (lwp
	      && lwp->contact_section_name
	      && !strcasecmp (lwp->contact_section_name, group_name)) 
	    break;

	  cpt++;
	}

	if (lwp) {

	  list_store =
	    GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (lwp->tree_view)));
	  group_name_unescaped =
	    gconf_unescape_key (group_name, -1);
	  gnomemeeting_addressbook_group_populate (list_store,
						   group_name_unescaped);
	  g_free (group_name_unescaped);
	}
	g_free (group_name);
      }

      g_strfreev (group_split);
    }

    /* Update the speed dials menu */
    gnomemeeting_speed_dials_menu_update (gw->main_menu);

    gdk_threads_leave ();
  }  
}

  
/* DESCRIPTION  :  This callback is called when something changes in the 
 * 		   servers or groups contacts list. 
 * BEHAVIOR     :  It updates the tree_view widget and the notebook pages,
 *                 but also the speed dials menu.
 * PRE          :  data is the page type (CONTACTS_SERVERS or CONTACTS_GROUPS)
 */
static void contacts_sections_list_changed_nt (GConfClient *client, guint cid,
					       GConfEntry *e, gpointer data)
{ 
  GmWindow *gw = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  if (e->value->type == GCONF_VALUE_LIST) {
  
    gdk_threads_enter ();
    gnomemeeting_addressbook_sections_populate ();
    gnomemeeting_speed_dials_menu_update (gw->main_menu);
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This callback is called when the forward gconf value 
 *                 changes.
 * BEHAVIOR     :  It checks that there is a forwarding host specified, if
 *                 not, disable forwarding and displays a popup.
 *                 It also modifies the "incoming_call_state" key if the
 *                 "always_forward" is modified, changing the corresponding
 *                 "incoming_call_mode" between AVAILABLE and FORWARD when
 *                 required.
 * PRE          :  /
 */
static void
call_forwarding_changed_nt (GConfClient *client,
			    guint cid, 
			    GConfEntry *entry,
			    gpointer data)
{
  GmWindow *gw = NULL;
  gchar *gconf_string = NULL;

  GMURL url;

  if (entry->value->type == GCONF_VALUE_BOOL) {

    gdk_threads_enter ();

    gw = GnomeMeeting::Process ()->GetMainWindow ();

  
    /* If "always_forward" is not set, we can always change the
       "incoming_call_mode" to AVAILABLE if it was set to FORWARD */
    if (!gconf_client_get_bool (client, CALL_FORWARDING_KEY "always_forward",
				NULL)) {

      if (gconf_client_get_int (client,
				CALL_OPTIONS_KEY "incoming_call_mode",
				NULL) == FORWARD) {
	gconf_client_set_int (client, CALL_OPTIONS_KEY "incoming_call_mode",
			      AVAILABLE, NULL);
      }
    }


    /* Checks if the forward host name is ok */
    gconf_string =
      gconf_client_get_string (client, CALL_FORWARDING_KEY "forward_host", 0);

    
    if (gconf_string)
      url = GMURL (gconf_string);
    if (url.IsEmpty ()) {

      /* If the URL is empty, we display a message box indicating
	 to the user to put a valid hostname and we disable
	 "always_forward" if "always_forward" is enabled */
      if (gconf_value_get_bool (entry->value)) {

	
	gnomemeeting_error_dialog (GTK_WIDGET_VISIBLE (gw->pref_window)?
				   GTK_WINDOW (gw->pref_window):
				   GTK_WINDOW (gm),
				   _("Forward URL not specified"),
				   _("You need to specify an URL where to forward calls in the call forwarding section of the preferences!\n\nDisabling forwarding."));
            
	gconf_client_set_bool (client, gconf_entry_get_key (entry), 0, NULL);
      }
    }
    else {

      /* Change the "incoming_call_mode" to FORWARD if "always_forward"
	 is enabled and if the URL is not empty */
      if (gconf_client_get_bool (client, CALL_FORWARDING_KEY "always_forward",
				 NULL)) {

	if (gconf_client_get_int (client,
				  CALL_OPTIONS_KEY "incoming_call_mode",
				  NULL) != FORWARD)
	  gconf_client_set_int (client, CALL_OPTIONS_KEY "incoming_call_mode",
				FORWARD, NULL);
      }
    }

    g_free (gconf_string);

    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This callback is called when the "register" gconf value 
 *                 changes.
 * BEHAVIOR     :  It registers or unregisters. The ILS
 *                 thread will check that all required values are provided.
 * PRE          :  /
 */
static void register_changed_nt (GConfClient *client, guint cid, 
				 GConfEntry *entry, gpointer data)
{
  GMH323EndPoint *endpoint = GnomeMeeting::Process ()->Endpoint ();
 
  if (entry->value->type == GCONF_VALUE_BOOL) {

    gdk_threads_enter ();
    endpoint->ILSRegister ();
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This callback is called when the ldap_visible
 *                 gconf value changes.
 * BEHAVIOR     :  Simply issued a modify request if we are regitered to an ILS
 *                 directory.
 * PRE          :  /
 */
static void ldap_visible_changed_nt (GConfClient *client, guint cid, 
				     GConfEntry *entry, gpointer data)
{
  GMH323EndPoint *endpoint = GnomeMeeting::Process ()->Endpoint ();

  if (entry->value->type == GCONF_VALUE_BOOL) {

    gdk_threads_enter ();
    if (gconf_client_get_bool (client, LDAP_KEY "enable_registering", 0))
      endpoint->ILSRegister ();
    gdk_threads_leave ();
  }

}


/* DESCRIPTION  :  This callback is called when the incoming_call_mode
 *                 gconf value changes.
 * BEHAVIOR     :  Simply issued a modify request if we are regitered to an ILS
 *                 directory, but also modifies the tray icon, and the
 *                 always_forward key following the current mode is FORWARD or
 *                 not.
 * PRE          :  /
 */
static void
incoming_call_mode_changed_nt (GConfClient *client,
			       guint cid, 
			       GConfEntry *entry,
			       gpointer data)
{
  GMH323EndPoint *endpoint = GnomeMeeting::Process ()->Endpoint ();
  GmWindow *gw = NULL;

  if (entry->value->type == GCONF_VALUE_INT) {

    gdk_threads_enter ();
    if (gconf_client_get_bool (client, LDAP_KEY "enable_registering", 0))
      endpoint->ILSRegister ();

    gw = GnomeMeeting::Process ()->GetMainWindow ();

    if (gconf_value_get_int (entry->value) == FORWARD)
      gconf_client_set_bool (client, CALL_FORWARDING_KEY "always_forward",
			     true, NULL);
    else
      gconf_client_set_bool (client, CALL_FORWARDING_KEY "always_forward",
			     false, NULL);
    /*    if (gconf_value_get_bool (entry->value))
      gnomemeeting_tray_set_content (gw->docklet, 2);
    else
    gnomemeeting_tray_set_content (gw->docklet, 0);*/
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This callback is called when the "stay_on_top" 
 *                 gconf value changes.
 * BEHAVIOR     :  Changes the hint for the video windows.
 * PRE          :  /
 */
static void stay_on_top_changed_nt (GConfClient *client, guint cid, 
				    GConfEntry *entry, gpointer data)
{
  GmWindow *gw = NULL;
  bool val = false;

  if (entry->value->type == GCONF_VALUE_BOOL) {

    gdk_threads_enter ();

    gw = GnomeMeeting::Process ()->GetMainWindow ();

    val = gconf_value_get_bool (entry->value);

    gdk_window_set_always_on_top (GDK_WINDOW (gm->window), val);
    gdk_window_set_always_on_top (GDK_WINDOW (gw->local_video_window->window), 
				  val);
    gdk_window_set_always_on_top (GDK_WINDOW (gw->remote_video_window->window), 
				  val);

    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This callback is called when one of the calls history
 *                 gconf value changes.
 * BEHAVIOR     :  Rebuild its content.
 * PRE          :  /
 */
static void calls_history_changed_nt (GConfClient *client,
				      guint cid, 
				      GConfEntry *entry,
				      gpointer data)
{
  if (entry->value->type == GCONF_VALUE_LIST) {

    gdk_threads_enter ();
    gnomemeeting_calls_history_window_populate ();
    gdk_threads_leave ();
  }
}


/* DESCRIPTION    : This is called when any setting related to the druid 
 *                  network speep selecion changes.
 * BEHAVIOR       : Just writes an entry in the gconf database registering 
 *                  that fact.
 * PRE            : None
 */
static void network_settings_changed_nt (GConfClient *client, guint, 
					 GConfEntry *, gpointer)
{
  gconf_client_set_int (client, GENERAL_KEY "kind_of_net",
			5, NULL);
}


#ifdef HAS_IXJ
/* DESCRIPTION    : This is called when any setting related to the 
 *                  lid AEC changes.
 * BEHAVIOR       : Updates it.
 * PRE            : None
 */
static void 
lid_aec_changed_nt (GConfClient *client, guint, GConfEntry *entry, gpointer)
{
  GMH323EndPoint *ep = NULL;
  GMLid *lid = NULL;
  int lid_aec = 0;
  
  if (entry->value->type == GCONF_VALUE_INT) {

    lid_aec = gconf_value_get_int (entry->value);

    ep = GnomeMeeting::Process ()->Endpoint ();
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
lid_country_changed_nt (GConfClient *client, guint, GConfEntry *entry, 
			gpointer)
{
  GMH323EndPoint *ep = NULL;
  GMLid *lid = NULL;
  gchar *country_code = NULL;
  
  if (entry->value->type == GCONF_VALUE_STRING) {
    
    ep = GnomeMeeting::Process ()->Endpoint ();
    lid = (ep ? ep->GetLid () : NULL);

    country_code = g_strdup (gconf_value_get_string (entry->value));
    
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
lid_output_device_type_changed_nt (GConfClient *client,
				   guint,
				   GConfEntry *entry, 
				   gpointer)
{
  GMH323EndPoint *ep = NULL;

  GMLid *lid = NULL;
  
  if (entry->value->type == GCONF_VALUE_INT) {
    
    ep = GnomeMeeting::Process ()->Endpoint ();
    lid = (ep ? ep->GetLid () : NULL);

    if (lid) {

      if (gconf_value_get_int (entry->value) == 0) // POTS
	  lid->EnableAudio (0, FALSE);
	else
	  lid->EnableAudio (0, TRUE);
      
      lid->Unlock ();
    }
  }
}
#endif


/* DESCRIPTION  :  This callback is called when a gconf error happens
 * BEHAVIOR     :  Pop-up a message-box
 * PRE          :  /
 */
static void
gconf_error_callback (GConfClient *,
		      GError *)
{
  GtkWidget *dialog = 
    gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
			    GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
			    _("An error has happened in the configuration"
			      " backend.\nMaybe some of your settings won't "
			      "be saved."));

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}


/* The functions  */
gboolean gnomemeeting_init_gconf (GConfClient *client)
{
  GmPrefWindow *pw = GnomeMeeting::Process ()->GetPrefWindow ();
  GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();
  int gconf_test = -1;
  
#ifndef DISABLE_GCONF
  gconf_client_add_dir (client, "/apps/gnomemeeting",
			GCONF_CLIENT_PRELOAD_NONE, 0);
#endif

    
#ifndef WIN32
  gconf_test =
    gconf_client_get_int (client, GENERAL_KEY "gconf_test_age", NULL);
  
  if (gconf_test != SCHEMA_AGE) 
    return FALSE;
#endif

  
  /* Set a default gconf error handler */
  gconf_client_set_error_handling (gconf_client_get_default (),
				   GCONF_CLIENT_HANDLE_UNRETURNED);
  gconf_client_set_global_default_error_handler (gconf_error_callback);


  /* There are in general 2 notifiers to attach to each widget :
     - the notifier that will update the widget itself to the new key,
     that one is attached when creating the widget.
     - the notifier to take an appropriate action, that one is in this file
  */
  
  gconf_client_notify_add (client, USER_INTERFACE_KEY "main_window/control_panel_section", main_notebook_changed_nt, NULL, 0, 0);
  gconf_client_notify_add (client, USER_INTERFACE_KEY "main_window/show_status_bar", menu_toggle_changed_nt, gtk_menu_get_widget (gw->main_menu, "status_bar"), 0, 0);

  gconf_client_notify_add (client, USER_INTERFACE_KEY "main_window/show_status_bar", view_widget_changed_nt, gw->statusbar, 0, 0);

  gconf_client_notify_add (client, USER_INTERFACE_KEY "main_window/show_chat_window", menu_toggle_changed_nt, gtk_menu_get_widget (gw->main_menu, "text_chat"), 0, 0);
  gconf_client_notify_add (client, USER_INTERFACE_KEY "main_window/show_chat_window", view_widget_changed_nt, gw->chat_window, 0, 0);


  /* Calls History Window */
  gconf_client_notify_add (client, USER_INTERFACE_KEY "calls_history_window/placed_calls_history", calls_history_changed_nt, NULL, 0, 0);
  gconf_client_notify_add (client, USER_INTERFACE_KEY "calls_history_window/missed_calls_history", calls_history_changed_nt, NULL, 0, 0);
  gconf_client_notify_add (client, USER_INTERFACE_KEY "calls_history_window/received_calls_history", calls_history_changed_nt, NULL, 0, 0);
  
  
  /* Call Control */
  gconf_client_notify_add (client, CALL_OPTIONS_KEY "incoming_call_mode", 
			   radio_menu_changed_nt,
			   gtk_menu_get_widget (gw->main_menu, "available"),
			   NULL, NULL);
  gconf_client_notify_add (client, CALL_OPTIONS_KEY "incoming_call_mode", 
			   radio_menu_changed_nt,
			   gtk_menu_get_widget (gw->tray_popup_menu,
						"available"),
			   NULL, NULL);
  gconf_client_notify_add (client, CALL_OPTIONS_KEY "incoming_call_mode", 
			   incoming_call_mode_changed_nt, NULL,
			   NULL, NULL);



  gconf_client_notify_add (client, VIDEO_DISPLAY_KEY "stay_on_top", 
			   stay_on_top_changed_nt, NULL, 0, 0);


  /* gnomemeeting_init_pref_window_h323_advanced */
  gconf_client_notify_add (client, CALL_FORWARDING_KEY "always_forward", call_forwarding_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, CALL_FORWARDING_KEY "forward_on_busy", call_forwarding_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, CALL_FORWARDING_KEY "forward_on_no_answer", call_forwarding_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, H323_ADVANCED_KEY "enable_h245_tunneling",
			   applicability_check_nt, NULL, 0, 0);
  gconf_client_notify_add (client, H323_ADVANCED_KEY "enable_h245_tunneling",
			   h245_tunneling_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, H323_ADVANCED_KEY "enable_early_h245",
			   applicability_check_nt, NULL, 0, 0);
  gconf_client_notify_add (client, H323_ADVANCED_KEY "enable_early_h245",
			   early_h245_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, H323_ADVANCED_KEY "enable_fast_start",
			   applicability_check_nt, NULL, 0, 0);
  gconf_client_notify_add (client, H323_ADVANCED_KEY "enable_fast_start",
			   fast_start_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, H323_ADVANCED_KEY "dtmf_sending",
			   capabilities_changed_nt, NULL, 0, 0);
  gconf_client_notify_add (client, H323_ADVANCED_KEY "dtmf_sending",
			   applicability_check_nt, NULL, 0, 0);

  
  /* gnomemeeting_init_pref_window_gateway */
  gconf_client_notify_add (client, H323_GATEWAY_KEY "host",
			   applicability_check_nt, NULL, 0, 0);
  
  gconf_client_notify_add (client, H323_GATEWAY_KEY "use_gateway",
			   applicability_check_nt, NULL, 0, 0);
  gconf_client_notify_add (client, H323_GATEWAY_KEY "use_gateway",
			   use_gateway_changed_nt, NULL, 0, 0);

    
  /* gnomemeeting_init_pref_window_directories */
  gconf_client_notify_add (client, LDAP_KEY "enable_registering",
			   register_changed_nt, NULL, 0, 0);
  gconf_client_notify_add (client, LDAP_KEY "show_details",
			   ldap_visible_changed_nt, NULL, 0, 0);


  /* gnomemeeting_init_pref_window_devices */
  gconf_client_notify_add (client, VIDEO_DEVICES_KEY "plugin", 
			   manager_changed_nt, 
			   NULL, 0, 0);
  gconf_client_notify_add (client, AUDIO_DEVICES_KEY "plugin", 
			   manager_changed_nt, 
			   NULL, 0, 0);


  gconf_client_notify_add (client, AUDIO_DEVICES_KEY "output_device",
			   audio_device_changed_nt,
			   NULL, 0, 0);
  gconf_client_notify_add (client, AUDIO_DEVICES_KEY "output_device",
			   applicability_check_nt,
			   NULL, 0, 0);
  

  gconf_client_notify_add (client, AUDIO_DEVICES_KEY "input_device",
			   audio_device_changed_nt,
			   NULL, 0, 0);
  gconf_client_notify_add (client, AUDIO_DEVICES_KEY "input_device",
			   applicability_check_nt,
			   NULL, 0, 0);

  gconf_client_notify_add (client, VIDEO_DEVICES_KEY "input_device", 
			   video_device_changed_nt, 
			   NULL, NULL, NULL);
  gconf_client_notify_add (client, VIDEO_DEVICES_KEY "input_device",
			   applicability_check_nt,
			   NULL, 0, 0);

  gconf_client_notify_add (client, VIDEO_DEVICES_KEY "channel", 
			   video_device_setting_changed_nt, 
			   NULL, NULL, NULL);

  gconf_client_notify_add (client, VIDEO_DEVICES_KEY "size", 
			   video_device_setting_changed_nt, 
			   NULL, NULL, NULL);
  gconf_client_notify_add (client, VIDEO_DEVICES_KEY "size", 
			   capabilities_changed_nt,
			   NULL, NULL, NULL);

  gconf_client_notify_add (client, VIDEO_DEVICES_KEY "format", 
			   video_device_setting_changed_nt, 
			   NULL, NULL, NULL);

  gconf_client_notify_add (client, VIDEO_DEVICES_KEY "image", 
			   video_device_setting_changed_nt, 
			   NULL, NULL, NULL);


  gconf_client_notify_add (client, VIDEO_DEVICES_KEY "enable_preview",
			   video_preview_changed_nt,
			   NULL, 0, 0);

  gconf_client_notify_add (client, VIDEO_DEVICES_KEY "enable_preview",
			   toggle_changed_nt,
			   gw->preview_button, 0, 0);

#ifdef HAS_IXJ
  gconf_client_notify_add (client, AUDIO_DEVICES_KEY "lid_country_code",
			   lid_country_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client,
			   AUDIO_DEVICES_KEY "lid_echo_cancellation_level",
			   lid_aec_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client,
			   AUDIO_DEVICES_KEY "lid_output_device_type",
			   lid_output_device_type_changed_nt, NULL, 0, 0);
#endif

  
  /* gnomemeeting_pref_window_sound_events */
  gconf_client_notify_add (client,
			   SOUND_EVENTS_KEY "enable_incoming_call_sound",
			   sound_events_list_changed_nt,
			   NULL, NULL, NULL);
  gconf_client_notify_add (client,
			   SOUND_EVENTS_KEY "incoming_call_sound",
			   sound_events_list_changed_nt,
			   NULL, NULL, NULL);

  gconf_client_notify_add (client,
			   SOUND_EVENTS_KEY "enable_ring_tone_sound",
			   sound_events_list_changed_nt,
			   NULL, NULL, NULL);
  gconf_client_notify_add (client,
			   SOUND_EVENTS_KEY "ring_tone_sound",
			   sound_events_list_changed_nt,
			   NULL, NULL, NULL);
  
  gconf_client_notify_add (client,
			   SOUND_EVENTS_KEY "enable_busy_tone_sound",
			   sound_events_list_changed_nt,
			   NULL, NULL, NULL);
  gconf_client_notify_add (client,
			   SOUND_EVENTS_KEY "busy_tone_sound",
			   sound_events_list_changed_nt,
			   NULL, NULL, NULL);

 
  /* gnomemeeting_pref_window_audio_codecs */
  gconf_client_notify_add (client, AUDIO_CODECS_KEY "list", audio_codecs_list_changed_nt, pw->codecs_list_store, 0, 0);	     
  gconf_client_notify_add (client, AUDIO_CODECS_KEY "list", 
			   capabilities_changed_nt,
			   NULL, NULL, NULL);

  gconf_client_notify_add (client, AUDIO_CODECS_KEY "minimum_jitter_buffer", 
			   jitter_buffer_changed_nt,
			   NULL, 0, 0);

  gconf_client_notify_add (client, AUDIO_CODECS_KEY "maximum_jitter_buffer", 
			   jitter_buffer_changed_nt,
			   NULL, 0, 0);

  gconf_client_notify_add (client, AUDIO_CODECS_KEY "gsm_frames",
			   capabilities_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, AUDIO_CODECS_KEY "g711_frames",
			   capabilities_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, AUDIO_CODECS_KEY "enable_silence_detection",
			   silence_detection_changed_nt, NULL, 0, 0);


  /* gnomemeeting_pref_window_video_codecs */
  gconf_client_notify_add (client, VIDEO_CODECS_KEY "transmitted_fps",
			   fps_limit_changed_nt, NULL, 0, 0);
  gconf_client_notify_add (client, VIDEO_CODECS_KEY "transmitted_fps",
			   network_settings_changed_nt, 0, 0, 0);

  gconf_client_notify_add (client, VIDEO_CODECS_KEY "enable_video_reception", network_settings_changed_nt, 0, 0, 0);	     
  gconf_client_notify_add (client, VIDEO_CODECS_KEY "enable_video_reception", enable_video_reception_changed_nt, NULL, 0, 0);	     

  gconf_client_notify_add (client, VIDEO_CODECS_KEY "enable_video_transmission", network_settings_changed_nt, 0, 0, 0);	     
  gconf_client_notify_add (client, VIDEO_CODECS_KEY "enable_video_transmission", enable_video_transmission_changed_nt, 0, 0, 0);	     

  gconf_client_notify_add (client, VIDEO_CODECS_KEY "maximum_video_bandwidth", maximum_video_bandwidth_changed_nt, NULL, 0, 0);
  gconf_client_notify_add (client, VIDEO_CODECS_KEY "maximum_video_bandwidth", network_settings_changed_nt, 0, 0, 0);


  gconf_client_notify_add (client, VIDEO_CODECS_KEY "transmitted_video_quality", tr_vq_changed_nt, NULL, 0, 0);
  gconf_client_notify_add (client, VIDEO_CODECS_KEY "transmitted_video_quality", network_settings_changed_nt, NULL, 0, 0);


  gconf_client_notify_add (client, VIDEO_CODECS_KEY "transmitted_background_blocks", tr_ub_changed_nt, NULL, 0, 0);


  /* LDAP Window */
  gconf_client_notify_add (client, CONTACTS_KEY "ldap_servers_list",
			   contacts_sections_list_changed_nt, 
			   GINT_TO_POINTER (CONTACTS_SERVERS), 0, 0);	    

  gconf_client_notify_add (client, CONTACTS_KEY "groups_list",
			   contacts_sections_list_changed_nt, 
			   GINT_TO_POINTER (CONTACTS_GROUPS), 0, 0);	     

  gconf_client_notify_add (client, CONTACTS_KEY "groups",
			   contacts_sections_list_group_content_changed_nt, 
			   NULL, 0, 0);

  return TRUE;
}


void gnomemeeting_gconf_upgrade ()
{
  gchar *gconf_url = NULL;

  int version = 0;
  GConfClient *client = NULL;

  client = gconf_client_get_default ();
  version = gconf_client_get_int (client, GENERAL_KEY "version", NULL);
  

  /* Disable bilinear filtering */
  if (version < 99) {

  }  


  /* Install the URL Handlers */
  gconf_url = 
    gconf_client_get_string (client, 
			     "/desktop/gnome/url-handlers/callto/command", 0);
					       
  if (!gconf_url) {
    
    gconf_client_set_string (client,
			     "/desktop/gnome/url-handlers/callto/command", 
			     "gnomemeeting -c \"%s\"", NULL);
    gconf_client_set_bool (client,
			   "/desktop/gnome/url-handlers/callto/need-terminal", 
			   false, NULL);
    gconf_client_set_bool (client,
			   "/desktop/gnome/url-handlers/callto/enabled", 
			   true, NULL);
  }
  g_free (gconf_url);

  gconf_url = 
    gconf_client_get_string (client, 
			     "/desktop/gnome/url-handlers/h323/command", 0);
  if (!gconf_url) {
    
    gconf_client_set_string (client,
			     "/desktop/gnome/url-handlers/h323/command", 
			     "gnomemeeting -c \"%s\"", NULL);
    gconf_client_set_bool (client,
			   "/desktop/gnome/url-handlers/h323/need-terminal", 
			   false, NULL);
    gconf_client_set_bool (client,
			   "/desktop/gnome/url-handlers/h323/enabled", 
			   true, NULL);
  }
  g_free (gconf_url);
}


