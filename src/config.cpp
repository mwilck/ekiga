
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
 *                         config.cpp  -  description
 *                         --------------------------
 *   begin                : Wed Feb 14 2001
 *   copyright            : (C) 2000-2003 by Damien Sandras 
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
#include "gnomemeeting.h"
#include "videograbber.h"
#include "ils.h"
#include "urlhandler.h"
#include "sound_handling.h"
#include "pref_window.h"
#include "ldap_window.h"
#include "tray.h"
#include "misc.h"

#include "dialog.h"
#include "stock-icons.h"
#include "gtk_menu_extensions.h"
#include "gconf_widgets_extensions.h"


/* Declarations */
extern GtkWidget *gm;
extern GnomeMeeting *MyApp;


static void applicability_check_nt (GConfClient *, guint, GConfEntry *, gpointer);
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

static void incoming_call_mode_changed_nt (GConfClient*,
					   guint, 
					   GConfEntry *,
					   gpointer);

static void
call_forwarding_changed_nt (GConfClient*,
			    guint,
			    GConfEntry *, 
			    gpointer);

static void manager_changed_nt (GConfClient *, guint, GConfEntry *, 
				gpointer);
static void audio_device_changed_nt (GConfClient *, guint, GConfEntry *, 
				     gpointer);

static void video_device_setting_changed_nt (GConfClient *, 
					     guint, 
					     GConfEntry *, 
					     gpointer);

static void video_preview_changed_nt (GConfClient *, guint, GConfEntry *, 
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
#ifndef DISABLE_GNOME
static void microtelco_enabled_nt (GConfClient *, guint, GConfEntry *,
				   gpointer);
#endif
static void h245_tunneling_changed_nt (GConfClient *,
				       guint,
				       GConfEntry *,
				       gpointer);

static void fast_start_changed_nt (GConfClient *,
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
static void lid_aec_changed_nt (GConfClient *, guint, GConfEntry *, gpointer);
static void lid_country_changed_nt (GConfClient *, guint, GConfEntry *, 
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
static void applicability_check_nt (GConfClient *client, guint cid, 
				    GConfEntry *entry, gpointer data)
{
  if ((entry->value->type == GCONF_VALUE_BOOL)
      ||(entry->value->type == GCONF_VALUE_STRING)
      ||(entry->value->type == GCONF_VALUE_INT)) {

    gdk_threads_enter ();
  
    if (MyApp->Endpoint ()->GetCallingState () != 0)
      gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (gm), GTK_WIDGET (data), _("Changing this setting will only affect new calls"), _("You have changed a setting that doesn't permit to GnomeMeeting to apply the new change to the current call. Your new setting will only take effect for the next call."));
    
    gdk_threads_leave ();
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

    gw = MyApp->GetMainWindow ();

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
 *                 associated with the microtelco service changes.
 * BEHAVIOR     :  It shows or hides the account option in the tools menu and
 *                 also updates the ixj druid page.
 * PRE          :  /
 */
#ifndef DISABLE_GNOME
static void microtelco_enabled_nt (GConfClient *client, guint cid, 
				   GConfEntry *entry, gpointer data)
{
  GmWindow *gw = NULL;
  GmDruidWindow *dw = NULL;
  
  if (entry->value->type == GCONF_VALUE_BOOL) {

    gdk_threads_enter ();

    dw = MyApp->GetDruidWindow ();
    gw = MyApp->GetMainWindow ();
    
    if (gconf_value_get_bool (entry->value)) {

      gtk_widget_show (gtk_menu_get_widget (gw->main_menu, "microtelco"));
      GTK_TOGGLE_BUTTON (dw->enable_microtelco)->active = true;
    }
    else {
      
      gtk_widget_hide (gtk_menu_get_widget (gw->main_menu, "microtelco"));
      GTK_TOGGLE_BUTTON (dw->enable_microtelco)->active = false;
    }
    
    gtk_widget_queue_draw (GTK_WIDGET (dw->enable_microtelco));
    gdk_threads_leave ();
  }
}
#endif


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

    ep = MyApp->Endpoint ();
    gw = MyApp->GetMainWindow ();
    
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

    ep = MyApp->Endpoint ();
    gw = MyApp->GetMainWindow ();
    
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

  ep = MyApp->Endpoint ();

  if (entry->value->type == GCONF_VALUE_BOOL) {

    ep->SetAutoStartTransmitVideo (gconf_value_get_bool (entry->value));

    if (gconf_client_get_int (client, DEVICES_KEY "video_size", NULL) == 0)
      name = "H.261-QCIF";
    else
      name = "H.261-CIF";

    if (gconf_value_get_bool (entry->value)) {
	
      bool res = ep->StartLogicalChannel (name,
			       RTP_Session::DefaultVideoSessionID,
			       FALSE);
      cout << "ici " << res << endl << flush;
    }
    else {

      ep->StopLogicalChannel (RTP_Session::DefaultVideoSessionID,
			      FALSE);
    }

    gdk_threads_enter ();
    if (gconf_client_get_bool (client, LDAP_KEY "register", 0))
      ep->ILSRegister ();
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the enable_video_transmission key changes.
 * BEHAVIOR     :  It updates the endpoint.
 *                 If the user is in a call, the video channel will be started
 *                 and stopped on-the-fly as long as the remote has enabled
 *                 video reception.
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

  ep = MyApp->Endpoint ();

  if (entry->value->type == GCONF_VALUE_BOOL) {

    ep->SetAutoStartReceiveVideo (gconf_value_get_bool (entry->value));

    if (gconf_client_get_int (client, DEVICES_KEY "video_size", NULL) == 0)
      name = "H.261-QCIF";
    else
      name = "H.261-CIF";

    if (!gconf_value_get_bool (entry->value)) {
	
      ep->StopLogicalChannel (RTP_Session::DefaultVideoSessionID,
			      TRUE);
    }
    else {

      ep->StartLogicalChannel (name,
			       RTP_Session::DefaultVideoSessionID,
			       TRUE);
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
  endpoint = MyApp->Endpoint ();
  
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
      if (ac && MyApp->Endpoint ()->GetCallingState () == 2) {

	gdk_threads_enter ();
	gw = MyApp->GetMainWindow ();
	
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
   
    ep = MyApp->Endpoint ();

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

  endpoint = MyApp->Endpoint ();
  
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

  endpoint = MyApp->Endpoint ();
  

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

  endpoint = MyApp->Endpoint ();

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

  endpoint = MyApp->Endpoint ();

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
  GMH323EndPoint *ep = MyApp->Endpoint ();  
  gdouble min_val = 20.0;
  gdouble max_val = 500.0;

  if (entry->value->type == GCONF_VALUE_INT) {

    min_val = 
      gconf_client_get_int (client, AUDIO_SETTINGS_KEY "min_jitter_buffer", 0);
    max_val = 
      gconf_client_get_int (client, AUDIO_SETTINGS_KEY "max_jitter_buffer", 0);
			    
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
 * BEHAVIOR     :  Updates the devices list for the new manager if 
 *                 we are not in a call. If we are in a call, it will be
 *                 done after that call.
 * PRE          :  /
 */
static void manager_changed_nt (GConfClient *client, guint cid, 
				GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_STRING) {

    gdk_threads_enter ();
    if (MyApp->Endpoint ()->GetCallingState () == 0)
      gnomemeeting_pref_window_refresh_devices_list (NULL, NULL);
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the audio devices changes.
 * BEHAVIOR     :  It updates the endpoint and displays
 *                 a message in the history. If the device is not valid,
 *                 i.e. the user erroneously used gconftool, a message is
 *                 displayed. Notice that the code ensures that either no
 *                 no Quicknet device is used, or it is used for both devices.
 *                 It also disables the druid test buttons for cases where
 *                 a test is not possible (no device found or quicknet).
 * PRE          :  /
 */
static void audio_device_changed_nt (GConfClient *client, guint cid, 
				     GConfEntry *entry, gpointer data)
{
  GmWindow *gw = NULL;
  GmDruidWindow *dw = NULL;
  GmPrefWindow *pw = NULL;
  
  PString dev, dev1, dev2;
  gchar *player = NULL;
  gchar *recorder = NULL;
  
  if (entry->value->type == GCONF_VALUE_STRING) {

    gdk_threads_enter ();
    dw = MyApp->GetDruidWindow ();
    pw = MyApp->GetPrefWindow ();
    gw = MyApp->GetMainWindow ();
      
    dev = PString (gconf_value_get_string (entry->value));


    /* Disable the druid button if no device is found */
    if (dev.Find (_("No device found")) == P_MAX_INDEX) 
      gtk_widget_set_sensitive (GTK_WIDGET (dw->audio_test_button), TRUE);
    else
      gtk_widget_set_sensitive (GTK_WIDGET (dw->audio_test_button), FALSE);
	

    /* If one of the devices that we are using is a quicknet device,
       we update the other devices too */
    if (dev.Find ("phone") != P_MAX_INDEX) {

      gconf_client_set_string (client, DEVICES_KEY "audio_recorder",
			       (const char *) dev, NULL);
      gconf_client_set_string (client, DEVICES_KEY "audio_player",
			       (const char *) dev, NULL);

      gnomemeeting_codecs_list_build (pw->codecs_list_store);
#ifndef DISABLE_GNOME
      gtk_widget_set_sensitive (GTK_WIDGET (dw->audio_test_button), false);
#endif
    }
    else {

      /* If what we changed right now has now a non quicknet value,
	 and that the other device value is a quicknet device, we change
	 it, because Quicknet can't be used for one device and not for
	 the other */
      player =
	gconf_client_get_string (client, DEVICES_KEY "audio_player", NULL);
      recorder =
	gconf_client_get_string (client, DEVICES_KEY "audio_recorder",
				 NULL);
      dev1 = PString (player);
      dev2 = PString (recorder);
      
      if (dev1.Find ("phone") != P_MAX_INDEX
	  || dev2.Find ("phone") != P_MAX_INDEX) {

	gconf_client_set_string (client, DEVICES_KEY "audio_recorder",
			       (const char *) dev, NULL);
	gconf_client_set_string (client, DEVICES_KEY "audio_player",
				 (const char *) dev, NULL);

	gnomemeeting_codecs_list_build (pw->codecs_list_store);
#ifndef DISABLE_GNOME
	gtk_widget_set_sensitive (GTK_WIDGET (dw->audio_test_button), true);
#endif
      }
    }
    
    if (MyApp->Endpoint ()->GetCallingState () == 0)
      /* Update the configuration in order to update 
	 the capabilities for calls */
      MyApp->Endpoint ()->UpdateConfig ();

    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This callback is called when the video device changes in
 *                 the gconf database.
 * BEHAVIOR     :  It resets the video transmission.
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
  BOOL no_error = FALSE;

  GMH323EndPoint *ep = NULL;

  if ((entry->value->type == GCONF_VALUE_STRING) ||
      (entry->value->type == GCONF_VALUE_INT)) {
    
    /* We reset the video device, no need to gdk_threads_enter here */
    ep = MyApp->Endpoint ();

    ep->AddAllCapabilities ();
    
    if (ep && ep->GetCallingState () == 0) {
      
      if (gconf_get_bool (DEVICES_KEY "video_preview")) {
    
	ep->RemoveVideoGrabber ();
	ep->CreateVideoGrabber ();
      }
    }
    else if (ep->GetCallingState () == 2) {

      gdk_threads_enter ();
      if (gconf_get_int (DEVICES_KEY "video_size") == 0)
	name = "H.261-QCIF";
      else
	name = "H.261-CIF";
      gdk_threads_leave ();

      if (gconf_get_bool (VIDEO_SETTINGS_KEY "enable_video_transmission")) {

	no_error=
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
    ep = MyApp->Endpoint ();
    
    if (ep && ep->GetCallingState () == 0) {
    
      if (gconf_value_get_bool (entry->value)) 
	ep->CreateVideoGrabber ();
      else 
	ep->RemoveVideoGrabber ();
    }
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
  GmPrefWindow *pw = NULL;
  
  if (entry->value->type == GCONF_VALUE_LIST) {
   
    gdk_threads_enter ();

    pw = MyApp->GetPrefWindow ();

    gnomemeeting_codecs_list_build (pw->codecs_list_store);

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
  
  GmLdapWindow *lw = NULL;
  GmLdapWindowPage *lwp = NULL;
  
  if (e->value->type == GCONF_VALUE_LIST) {
  
    gdk_threads_enter ();

    lw = MyApp->GetLdapWindow ();
    
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
    
    gdk_threads_leave ();
  }  
}

  
/* DESCRIPTION  :  This callback is called when something changes in the 
 * 		   servers or groups contacts list. 
 * BEHAVIOR     :  It updates the tree_view widget and the notebook pages.
 * PRE          :  data is the page type (CONTACTS_SERVERS or CONTACTS_GROUPS)
 */
static void contacts_sections_list_changed_nt (GConfClient *client, guint cid,
					       GConfEntry *e, gpointer data)
{ 
  if (e->value->type == GCONF_VALUE_LIST) {
  
    gdk_threads_enter ();
    gnomemeeting_addressbook_sections_populate ();
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

    gw = MyApp->GetMainWindow ();

  
    /* If "always_forward" is not set, we can always change the
       "incoming_call_mode" to AVAILABLE if it was set to FORWARD */
    if (!gconf_client_get_bool (client, CALL_FORWARDING_KEY "always_forward",
				NULL)) {

      if (gconf_client_get_int (client,
				CALL_CONTROL_KEY "incoming_call_mode",
				NULL) == FORWARD) {
	gconf_client_set_int (client, CALL_CONTROL_KEY "incoming_call_mode",
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
				  CALL_CONTROL_KEY "incoming_call_mode",
				  NULL) != FORWARD)
	  gconf_client_set_int (client, CALL_CONTROL_KEY "incoming_call_mode",
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
  GMH323EndPoint *endpoint = MyApp->Endpoint ();
 
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
  GMH323EndPoint *endpoint = MyApp->Endpoint ();

  if (entry->value->type == GCONF_VALUE_BOOL) {

    gdk_threads_enter ();
    if (gconf_client_get_bool (client, LDAP_KEY "register", 0))
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
  GMH323EndPoint *endpoint = MyApp->Endpoint ();
  GmWindow *gw = NULL;

  if (entry->value->type == GCONF_VALUE_INT) {

    gdk_threads_enter ();
    if (gconf_client_get_bool (client, LDAP_KEY "register", 0))
      endpoint->ILSRegister ();

    gw = MyApp->GetMainWindow ();

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

    gw = MyApp->GetMainWindow ();

    val = gconf_value_get_bool (entry->value);

    gdk_window_set_always_on_top (GDK_WINDOW (gm->window), val);
    gdk_window_set_always_on_top (GDK_WINDOW (gw->local_video_window->window), 
				  val);
    gdk_window_set_always_on_top (GDK_WINDOW (gw->remote_video_window->window), 
				  val);

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

    ep = MyApp->Endpoint ();
    lid = (ep ? ep->GetLid () : NULL);

    if (lid) {

      switch (lid_aec) {
	
      case 0:
	lid->SetAEC (0, OpalLineInterfaceDevice::AECOff);
	break;
	
      case 1:
	lid->SetAEC (0, OpalLineInterfaceDevice::AECLow);
	break;
	
      case 2:
	lid->SetAEC (0, OpalLineInterfaceDevice::AECMedium);
	break;
	
      case 3:
	lid->SetAEC (0, OpalLineInterfaceDevice::AECHigh);
	break;
	
      case 5:
	lid->SetAEC (0, OpalLineInterfaceDevice::AECAGC);
	break;
      }

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
    
    ep = MyApp->Endpoint ();
    lid = (ep ? ep->GetLid () : NULL);

    country_code = g_strdup (gconf_value_get_string (entry->value));
    
    if (country_code && lid) {
      
      lid->SetCountryCodeName (country_code);
      lid->Unlock ();
      g_free (country_code);
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
  GmPrefWindow *pw = MyApp->GetPrefWindow ();
  GmWindow *gw = MyApp->GetMainWindow ();
  int gconf_test = -1;
  
#ifndef DISABLE_GCONF
  gconf_client_add_dir (client, "/apps/gnomemeeting",
			GCONF_CLIENT_PRELOAD_RECURSIVE, 0);
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
  
  gconf_client_notify_add (client, VIEW_KEY "control_panel_section", main_notebook_changed_nt, NULL, 0, 0);
  gconf_client_notify_add (client, VIEW_KEY "show_status_bar", menu_toggle_changed_nt, gtk_menu_get_widget (gw->main_menu, "status_bar"), 0, 0);

  gconf_client_notify_add (client, VIEW_KEY "show_status_bar", view_widget_changed_nt, gw->statusbar, 0, 0);

  gconf_client_notify_add (client, VIEW_KEY "show_chat_window", menu_toggle_changed_nt, gtk_menu_get_widget (gw->main_menu, "text_chat"), 0, 0);
  gconf_client_notify_add (client, VIEW_KEY "show_chat_window", view_widget_changed_nt, gw->chat_window, 0, 0);

  gconf_client_notify_add (client, CALL_CONTROL_KEY "incoming_call_mode", 
			   radio_menu_changed_nt,
			   gtk_menu_get_widget (gw->main_menu, "available"),
			   NULL, NULL);
  gconf_client_notify_add (client, CALL_CONTROL_KEY "incoming_call_mode", 
			   radio_menu_changed_nt,
			   gtk_menu_get_widget (gw->tray_popup_menu,
						"available"),
			   NULL, NULL);
  gconf_client_notify_add (client, CALL_CONTROL_KEY "incoming_call_mode", 
			   incoming_call_mode_changed_nt, NULL,
			   NULL, NULL);



  gconf_client_notify_add (client, VIDEO_DISPLAY_KEY "stay_on_top", 
			   stay_on_top_changed_nt, NULL, 0, 0);


  /* gnomemeeting_init_pref_window_h323_advanced */
  gconf_client_notify_add (client, CALL_FORWARDING_KEY "always_forward", call_forwarding_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, CALL_FORWARDING_KEY "busy_forward", call_forwarding_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, CALL_FORWARDING_KEY "no_answer_forward", call_forwarding_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, GENERAL_KEY "h245_tunneling",
			   applicability_check_nt, pw->ht, 0, 0);
  gconf_client_notify_add (client, GENERAL_KEY "h245_tunneling",
			   h245_tunneling_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, GENERAL_KEY "fast_start",
			   applicability_check_nt, pw->fs, 0, 0);
  gconf_client_notify_add (client, GENERAL_KEY "fast_start",
			   fast_start_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, GENERAL_KEY "user_input_capability",
			   capabilities_changed_nt, NULL, 0, 0);
  gconf_client_notify_add (client, GENERAL_KEY "user_input_capability",
			   applicability_check_nt, pw->uic, 0, 0);

  /* gnomemeeting_init_pref_window_directories */
  gconf_client_notify_add (client, LDAP_KEY "register",
			   register_changed_nt, NULL, 0, 0);
  gconf_client_notify_add (client, LDAP_KEY "visible",
			   ldap_visible_changed_nt, NULL, 0, 0);


  /* gnomemeeting_init_pref_window_devices */
#ifdef TRY_PLUGINS
  /* Audio Manager */
  gconf_client_notify_add (client, DEVICES_KEY "audio_manager", 
			   manager_changed_nt, 
			   NULL, 0, 0);
  /* Video Manager */
  gconf_client_notify_add (client, DEVICES_KEY "video_manager", 
			   manager_changed_nt, 
			   NULL, 0, 0);
#endif

  gconf_client_notify_add (client, DEVICES_KEY "audio_player", audio_device_changed_nt, pw->audio_player, 0, 0);
  gconf_client_notify_add (client, DEVICES_KEY "audio_player", applicability_check_nt, pw->audio_player, 0, 0);
  

  gconf_client_notify_add (client, DEVICES_KEY "audio_recorder", audio_device_changed_nt, pw->audio_recorder, 0, 0);
  gconf_client_notify_add (client, DEVICES_KEY "audio_recorder", applicability_check_nt, pw->audio_recorder, 0, 0);

  gconf_client_notify_add (client, DEVICES_KEY "video_recorder", 
			   video_device_setting_changed_nt, 
			   NULL, NULL, NULL);

  gconf_client_notify_add (client, DEVICES_KEY "video_channel", 
			   video_device_setting_changed_nt, 
			   NULL, NULL, NULL);

  gconf_client_notify_add (client, DEVICES_KEY "video_size", 
			   video_device_setting_changed_nt, 
			   NULL, NULL, NULL);
  gconf_client_notify_add (client, DEVICES_KEY "video_size", 
			   capabilities_changed_nt,
			   NULL, NULL, NULL);

  gconf_client_notify_add (client, DEVICES_KEY "video_format", 
			   video_device_setting_changed_nt, 
			   NULL, NULL, NULL);

  gconf_client_notify_add (client, DEVICES_KEY "video_image", 
			   video_device_setting_changed_nt, 
			   NULL, NULL, NULL);


  gconf_client_notify_add (client, DEVICES_KEY "video_preview",
			   video_preview_changed_nt,
			   NULL, 0, 0);
  gconf_client_notify_add (client, DEVICES_KEY "video_preview",
			   toggle_changed_nt,
			   gw->preview_button, 0, 0);

#ifdef HAS_IXJ
  gconf_client_notify_add (client, DEVICES_KEY "lid_country", lid_country_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, DEVICES_KEY "lid_aec", lid_aec_changed_nt, NULL, 0, 0);
#endif


  /* gnomemeeting_pref_window_audio_codecs */
  gconf_client_notify_add (client, AUDIO_CODECS_KEY "codecs_list", audio_codecs_list_changed_nt, pw->codecs_list_store, 0, 0);	     
  gconf_client_notify_add (client, AUDIO_CODECS_KEY "codecs_list", 
			   capabilities_changed_nt,
			   NULL, NULL, NULL);

  gconf_client_notify_add (client, AUDIO_SETTINGS_KEY "min_jitter_buffer", 
			   jitter_buffer_changed_nt,
			   NULL, 0, 0);

  gconf_client_notify_add (client, AUDIO_SETTINGS_KEY "max_jitter_buffer", 
			   jitter_buffer_changed_nt,
			   NULL, 0, 0);

  gconf_client_notify_add (client, AUDIO_SETTINGS_KEY "gsm_frames",
			   capabilities_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, AUDIO_SETTINGS_KEY "g711_frames",
			   capabilities_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, AUDIO_SETTINGS_KEY "sd",
			   silence_detection_changed_nt, NULL, 0, 0);


  /* gnomemeeting_pref_window_video_codecs */
  gconf_client_notify_add (client, VIDEO_SETTINGS_KEY "tr_fps",
			   fps_limit_changed_nt, NULL, 0, 0);
  gconf_client_notify_add (client, VIDEO_SETTINGS_KEY "tr_fps",
			   network_settings_changed_nt, 0, 0, 0);

  gconf_client_notify_add (client, VIDEO_SETTINGS_KEY "enable_video_reception", network_settings_changed_nt, 0, 0, 0);	     
  gconf_client_notify_add (client, VIDEO_SETTINGS_KEY "enable_video_reception", enable_video_reception_changed_nt, 0, 0, 0);	     

  gconf_client_notify_add (client, VIDEO_SETTINGS_KEY "enable_video_transmission", network_settings_changed_nt, 0, 0, 0);	     
  gconf_client_notify_add (client, VIDEO_SETTINGS_KEY "enable_video_transmission", enable_video_transmission_changed_nt, 0, 0, 0);	     

  gconf_client_notify_add (client, VIDEO_SETTINGS_KEY "maximum_video_bandwidth", maximum_video_bandwidth_changed_nt, NULL, 0, 0);
  gconf_client_notify_add (client, VIDEO_SETTINGS_KEY "maximum_video_bandwidth", network_settings_changed_nt, 0, 0, 0);


  gconf_client_notify_add (client, VIDEO_SETTINGS_KEY "tr_vq", tr_vq_changed_nt, NULL, 0, 0);
  gconf_client_notify_add (client, VIDEO_SETTINGS_KEY "tr_vq", network_settings_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, VIDEO_SETTINGS_KEY "re_vq", network_settings_changed_nt, 0, 0, 0);


  gconf_client_notify_add (client, VIDEO_SETTINGS_KEY "tr_ub", tr_ub_changed_nt, NULL, 0, 0);


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

  
  /* Microtelco */
#ifndef DISABLE_GNOME
  gconf_client_notify_add (client, SERVICES_KEY "enable_microtelco",
			   microtelco_enabled_nt, NULL, 0, 0);
#endif

  return TRUE;
}


void gnomemeeting_gconf_upgrade ()
{
  int gconf_value_int = 0;
  gchar *gconf_value = NULL;
  gchar *gconf_url = NULL;
  gchar *group_name = NULL;
  gchar *group_content_gconf_key = NULL;
  gchar *new_group_content_gconf_key = NULL;
  GSList *group_content = NULL;
  GSList *group_content_iter = NULL;
  GSList *new_group_content = NULL;
  GSList *groups = NULL;
  GSList *groups_iter = NULL;
  GSList *list = NULL;

  int version = 0;
  GConfClient *client = NULL;

  client = gconf_client_get_default ();
  version = gconf_client_get_int (client, GENERAL_KEY "version", NULL);
  
  /* New Speex Audio codec in 0.95 (all Unix versions of 0.95 will have it)
     Also enable Fast Start and enable Tunneling */
  if (version < 95) {

    list = g_slist_append (list, (void *) "SpeexNarrow-8k=1");
    list = g_slist_append (list, (void *) "MS-GSM=1");
    list = g_slist_append (list, (void *) "SpeexNarrow-15k=1");
    list = g_slist_append (list, (void *) "GSM-06.10=1");
    list = g_slist_append (list, (void *) "G.726-32k=1");
    list = g_slist_append (list, (void *) "G.711-uLaw-64k=1");
    list = g_slist_append (list, (void *) "G.711-ALaw-64k=1");
    list = g_slist_append (list, (void *) "LPC-10=1");
    list = g_slist_append (list, (void *) "G.723.1=1");
    gconf_client_set_list (client, AUDIO_CODECS_KEY "codecs_list", 
			   GCONF_VALUE_STRING, list, NULL);

    g_slist_free (list);

    gconf_client_set_bool (client, GENERAL_KEY "fast_start", false, NULL);
    gconf_client_set_bool (client, GENERAL_KEY "h245_tunneling", true, NULL);
  }


  /* With 0.97, we convert the old addressbook to the new format,
     same for the port ranges */
  if (version < 97) {

    groups =
      gconf_client_get_list (client, CONTACTS_KEY "groups_list",
			     GCONF_VALUE_STRING, NULL);
    groups_iter = groups;
  
    while (groups_iter && groups_iter->data) {
    
      group_name = g_utf8_strdown ((char *) groups_iter->data, -1);
      group_content_gconf_key =
	g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY,
			 (char *) groups_iter->data);
      new_group_content_gconf_key =
	g_strdup_printf ("%s%s", CONTACTS_GROUPS_KEY, group_name);
	
      group_content =
	gconf_client_get_list (client, group_content_gconf_key,
			       GCONF_VALUE_STRING, NULL);
      group_content_iter = group_content;
      
      new_group_content =
	gconf_client_get_list (client, new_group_content_gconf_key,
			       GCONF_VALUE_STRING, NULL);
	
      while (group_content_iter && group_content_iter->data) {

	new_group_content =
	  g_slist_append (new_group_content, group_content_iter->data);
	  
	group_content_iter = g_slist_next (group_content_iter);
      }

      gconf_client_set_list (client, new_group_content_gconf_key,
			     GCONF_VALUE_STRING, new_group_content, NULL);
      gconf_client_unset (client, group_content_gconf_key, NULL);
      g_free (group_content_gconf_key);
      g_free (new_group_content_gconf_key);
      g_free (group_name);
      g_slist_free (group_content);
      g_slist_free (new_group_content);
      new_group_content = NULL;
      groups_iter = g_slist_next (groups_iter);
    }
      
    g_slist_free (groups);


    /* Convert the old ports keys */
    gconf_value_int =
      gconf_client_get_int (client, GENERAL_KEY "listen_port", 0);
    if (gconf_value_int != 0)
      gconf_client_set_int (client, PORTS_KEY "listen_port",
			    gconf_value_int, 0);
    
    gconf_value =
      gconf_client_get_string (client, GENERAL_KEY "tcp_port_range", 0);
    if (gconf_value)
      gconf_client_set_string (client, PORTS_KEY "tcp_port_range",
			       gconf_value, 0);
    g_free (gconf_value);
    
    gconf_value =
      gconf_client_get_string (client, GENERAL_KEY "udp_port_range", 0);
    if (gconf_value)
      gconf_client_set_string (client, PORTS_KEY "rtp_port_range",
			       gconf_value, 0);
    g_free (gconf_value);

    gconf_client_remove_dir (client, "/apps/gnomemeeting", 0);
    gconf_client_unset (client, GENERAL_KEY "listen_port", NULL);
    gconf_client_unset (client, GENERAL_KEY "tcp_port_range", NULL);
    gconf_client_unset (client, GENERAL_KEY "udp_port_range", NULL);
    gconf_client_add_dir (client, "/apps/gnomemeeting",
			  GCONF_CLIENT_PRELOAD_RECURSIVE, 0);
  }


  /* Disable bilinear filtering */
  if (version < 99) {

    /* New iLBC codec, remove LPC-10 from the GUI 
       because people shouldn't use it except for fun */
    list = NULL;
    list = g_slist_append (list, (void *) "iLBC-13k3=1");
    list = g_slist_append (list, (void *) "MS-GSM=1");
    list = g_slist_append (list, (void *) "iLBC-15k2=1");
    list = g_slist_append (list, (void *) "SpeexNarrow-15k=1");
    list = g_slist_append (list, (void *) "GSM-06.10=1");
    list = g_slist_append (list, (void *) "SpeexNarrow-8k=1");
    list = g_slist_append (list, (void *) "G.726-32k=1");
    list = g_slist_append (list, (void *) "G.711-uLaw-64k=1");
    list = g_slist_append (list, (void *) "G.711-ALaw-64k=1");
    list = g_slist_append (list, (void *) "G.723.1=1");
    list = g_slist_append (list, (void *) "LPC-10=0");
    gconf_client_set_list (client, AUDIO_CODECS_KEY "codecs_list", 
			   GCONF_VALUE_STRING, list, NULL);

    g_slist_free (list);

    /* Disable bilinear filtering */
    gconf_client_set_bool (client, VIDEO_DISPLAY_KEY "bilinear_filtering", 
			   false, NULL);

    gconf_client_remove_dir (client, "/apps/gnomemeeting", 0);

    /* Remove the color_format key as we are not using it anymore */
    gconf_client_unset (client, VIDEO_DISPLAY_KEY "color_format", NULL);

    /* Move the old keys for NAT to the new ones and unset the old ones */
    gconf_client_set_bool (client, NAT_KEY "ip_translation",
			   gconf_client_get_bool (client, 
						  GENERAL_KEY "ip_translation"
						  , 0), 0);
    gconf_client_unset (client, GENERAL_KEY "ip_translation", NULL);
    gconf_client_unset (client, GENERAL_KEY "public_ip", NULL);

    /* Remove the deprecated auto_answer and do_not_disturb keys */
    gconf_client_unset (client, GENERAL_KEY "auto_answer", NULL);
    gconf_client_unset (client, GENERAL_KEY "do_not_disturb", NULL);

    gconf_client_add_dir (client, "/apps/gnomemeeting",
			  GCONF_CLIENT_PRELOAD_RECURSIVE, 0);
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


