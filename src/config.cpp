
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
#include "sound_handling.h"
#include "pref_window.h"
#include "ldap_window.h"
#include "tray.h"
#include "misc.h"
#include "menu.h"
#include "dialog.h"
#include "stock-icons.h"


/* Declarations */
extern GtkWidget *gm;
extern GnomeMeeting *MyApp;

static void entry_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void toggle_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void menu_radio_changed_nt (GConfClient *, guint, GConfEntry *, gpointer);
static void menu_toggle_changed_nt (GConfClient *, guint, GConfEntry *, gpointer);
static void string_option_menu_changed_nt (GConfClient *, guint, GConfEntry *, gpointer);
static void int_option_menu_changed_nt (GConfClient *, guint, GConfEntry *, gpointer);
static void adjustment_changed_nt (GConfClient *, guint, GConfEntry *, gpointer);

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
static void do_not_disturb_changed_nt (GConfClient*, guint, 
				       GConfEntry *, gpointer);
static void forward_toggle_changed_nt (GConfClient*, guint, GConfEntry *, 
				       gpointer);
static void audio_mixer_changed_nt (GConfClient *, guint, GConfEntry *, 
				    gpointer);
static void audio_device_changed_nt (GConfClient *, guint, GConfEntry *, 
				     gpointer);
static void video_device_setting_changed_nt (GConfClient *, guint, 
					     GConfEntry *, gpointer);
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
static void ht_fs_changed_nt (GConfClient *, guint, GConfEntry *, gpointer);
static void enable_vid_tr_changed_nt (GConfClient *, guint, GConfEntry *, 
				      gpointer);
static void silence_detection_changed_nt (GConfClient *, guint, 
					  GConfEntry *, gpointer);
static void network_settings_changed_nt (GConfClient *, guint, 
					 GConfEntry *, gpointer);
#ifdef HAS_IXJ
static void lid_device_changed_nt (GConfClient *, guint, GConfEntry *, 
				   gpointer);
static void lid_aec_changed_nt (GConfClient *, guint, GConfEntry *, gpointer);
static void lid_country_changed_nt (GConfClient *, guint, GConfEntry *, 
				    gpointer);
#endif


/* 
 * Generic notifiers that update specific widgets when a gconf key changes
 */


/* DESCRIPTION  :  Generic notifiers for entries.
 *                 This callback is called when a specific key of
 *                 the gconf database associated with an entry changes.
 * BEHAVIOR     :  It updates the widget.
 * PRE          :  /
 */
static void entry_changed_nt (GConfClient *client, guint cid, 
			      GConfEntry *entry, gpointer data)
{

  if (entry->value->type == GCONF_VALUE_STRING) {

    gdk_threads_enter ();
  
    GtkWidget *e = GTK_WIDGET (data);

    /* We set the new value for the widget */
    g_signal_handlers_block_by_func (G_OBJECT (e),
				     (gpointer) entry_changed, 
				     g_object_get_data (G_OBJECT (e), 
							"gconf_key")); 
  
    gtk_entry_set_text (GTK_ENTRY (e), gconf_value_get_string (entry->value));

    g_signal_handlers_unblock_by_func (G_OBJECT (e),
				       (gpointer) entry_changed, 
				       g_object_get_data (G_OBJECT (e), 
							  "gconf_key")); 

    
    gdk_threads_leave (); 
  }
}


/* DESCRIPTION  :  Generic notifiers for toggles.
 *                 This callback is called when a specific key of
 *                 the gconf database associated with a toggle changes, this
 *                 only updates the toggle.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  /
 */
static void toggle_changed_nt (GConfClient *client, guint cid, 
			       GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_BOOL) {
   
    gdk_threads_enter ();
  
    GtkWidget *e = GTK_WIDGET (data);

    /* We set the new value for the widget */
    g_signal_handlers_block_by_func (G_OBJECT (e),
				     (gpointer) toggle_changed, 
				     g_object_get_data (G_OBJECT (e), 
							"gconf_key")); 
  
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (e), 
				  (bool) gconf_value_get_bool (entry->value));

    g_signal_handlers_unblock_by_func (G_OBJECT (e),
				       (gpointer) toggle_changed, 
				       g_object_get_data (G_OBJECT (e), 
							  "gconf_key")); 

    
    gdk_threads_leave (); 
  }
}


/* DESCRIPTION  :  Notifiers for radios in the "Control Panel" menu.
 *                 This callback is called when a specific key of
 *                 the gconf database associated with a radio changes, this
 *                 only updates the radio in the menu.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  /
 */
static void menu_radio_changed_nt (GConfClient *client, guint cid, 
				   GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_INT) {
   
    gdk_threads_enter ();
  
    MenuEntry *e = (MenuEntry *) (data);

    /* We set the new value for the widget */
    for (int i = 0 ; i <= GM_MAIN_NOTEBOOK_HIDDEN ; i++) {

      if (gconf_value_get_int (entry->value) == i)
	GTK_CHECK_MENU_ITEM (e [CONTROL_PANEL_VIEW_MENU_INDICE+i].widget)->active = TRUE;
      else
	GTK_CHECK_MENU_ITEM (e [CONTROL_PANEL_VIEW_MENU_INDICE+i].widget)->active = FALSE;

      gtk_widget_queue_draw (GTK_WIDGET (e [CONTROL_PANEL_VIEW_MENU_INDICE+i].widget));
    }
    
    gdk_threads_leave (); 
  }
}


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
  if (entry->value->type == GCONF_VALUE_BOOL) {
   
    gdk_threads_enter ();
  
    GtkWidget *e = GTK_WIDGET (data);

    /* We set the new value for the widget */
    GTK_CHECK_MENU_ITEM (e)->active = 
      (bool) gconf_value_get_bool (entry->value);

    gtk_widget_queue_draw (GTK_WIDGET (e));

    gdk_threads_leave (); 
  }
}


/* DESCRIPTION  :  Generic notifiers for int-based option_menus.
 *                 This callback is called when a specific key of
 *                 the gconf database associated with an option menu changes,
 *                 it only updates the menu.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  /
 */
static void int_option_menu_changed_nt (GConfClient *client, guint cid, 
					GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_INT) {
   
    gdk_threads_enter ();
  
    /* We set the new value for the widget */
    g_signal_handlers_block_by_func (G_OBJECT (data),
				     (gpointer) int_option_menu_changed, 
				     (gpointer) g_object_get_data (G_OBJECT (data), 
								   "gconf_key")); 
    gtk_option_menu_set_history (GTK_OPTION_MENU (data),
				 gconf_value_get_int (entry->value));
  
    g_signal_handlers_unblock_by_func (G_OBJECT (data),
				       (gpointer) int_option_menu_changed, 
				       (gpointer) g_object_get_data (G_OBJECT (data), 
								     "gconf_key")); 

    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  Generic notifiers for adjustments.
 *                 This callback is called when a specific key of
 *                 the gconf database associated with an adjustment changes.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  /
 */
static void adjustment_changed_nt (GConfClient *client, guint cid, 
				   GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_INT) {
    
    gdk_threads_enter ();
    
    /* We set the new value for the widget */
    g_signal_handlers_block_by_func (G_OBJECT (data),
				     (gpointer) adjustment_changed, 
				     (gpointer) g_object_get_data (G_OBJECT (data), 
								   "gconf_key")); 
    gtk_adjustment_set_value (gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (data)),
			      gconf_value_get_int (entry->value));
  
    g_signal_handlers_unblock_by_func (G_OBJECT (data),
				       (gpointer) adjustment_changed, 
				       (gpointer) g_object_get_data (G_OBJECT (data), 
								     "gconf_key")); 

    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  Generic notifiers for string-based option_menus.
 *                 This callback is called when a specific key of
 *                 the gconf database associated with an option menu changes, this
 *                 only updates the menu.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  /
 */
static void string_option_menu_changed_nt (GConfClient *client, guint cid, 
					   GConfEntry *entry, gpointer data)
{
  int cpt = 0;
  GtkWidget *label = NULL;
  GList *glist = NULL;
  gpointer mydata;

  if (entry->value->type == GCONF_VALUE_STRING) {
   
    gdk_threads_enter ();
  
    /* We set the new value for the widget */
    g_signal_handlers_block_by_func (G_OBJECT (data),
				     (gpointer) string_option_menu_changed, 
				     (gpointer) g_object_get_data (G_OBJECT (data), 
								   "gconf_key")); 
    glist = 
      g_list_first (GTK_MENU_SHELL (GTK_MENU (GTK_OPTION_MENU (data)->menu))->children);
    
    while ((mydata = g_list_nth_data (glist, cpt)) != NULL) {

      label = GTK_BIN (mydata)->child;
      if ((label) && (!strcmp (gtk_label_get_text (GTK_LABEL (label)), 
			       gconf_value_get_string (entry->value))))
	break;
      cpt++; 
    } 

    gtk_option_menu_set_history (GTK_OPTION_MENU (data), cpt);
   
    g_signal_handlers_unblock_by_func (G_OBJECT (data),
				       (gpointer) string_option_menu_changed, 
				       (gpointer) g_object_get_data (G_OBJECT (data), 
								     "gconf_key")); 

    gdk_threads_leave ();
  }
}


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
      gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (gm), GTK_WIDGET (data), _("Changing this setting will only affect new calls"));
    
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

    gw = gnomemeeting_get_main_window (gm);

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
  MenuEntry *gnomemeeting_menu = NULL;
  GmDruidWindow *dw = NULL;
  
  if (entry->value->type == GCONF_VALUE_BOOL) {

    gdk_threads_enter ();

    gnomemeeting_menu = gnomemeeting_get_menu (gm);
    dw = gnomemeeting_get_druid_window (gm);
    
    if (gconf_value_get_bool (entry->value)) {

      gtk_widget_show (GTK_WIDGET (gnomemeeting_menu [MICROTELCO1_TOOLS_MENU_INDICE].widget));
      gtk_widget_show (GTK_WIDGET (gnomemeeting_menu [MICROTELCO2_TOOLS_MENU_INDICE].widget));
      GTK_TOGGLE_BUTTON (dw->enable_microtelco)->active = true;
    }
    else {

      gtk_widget_hide (GTK_WIDGET (gnomemeeting_menu [MICROTELCO1_TOOLS_MENU_INDICE].widget));
      gtk_widget_hide (GTK_WIDGET (gnomemeeting_menu [MICROTELCO2_TOOLS_MENU_INDICE].widget));
      GTK_TOGGLE_BUTTON (dw->enable_microtelco)->active = false;
    }
    
    gtk_widget_queue_draw (GTK_WIDGET (dw->enable_microtelco));
    gdk_threads_leave ();
  }
}
#endif


/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the H.245 Tunneling or 
 *                 the Fast Start changes.
 * BEHAVIOR     :  It updates the endpoint.
 * PRE          :  /
 */
static void ht_fs_changed_nt (GConfClient *client, guint cid, 
			      GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_BOOL) {

    gdk_threads_enter ();

    if (MyApp->Endpoint ()->GetCallingState () == 0)
      MyApp->Endpoint ()->UpdateConfig ();
    
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the enable_video_transmission key changes or
 *                 enable_video_reception.
 * BEHAVIOR     :  It updates the endpoint, and updates the registering on ILS.
 * PRE          :  /
 */
static void enable_vid_tr_changed_nt (GConfClient *client, guint cid, 
				      GConfEntry *entry, gpointer data)
{
  GMH323EndPoint *endpoint = MyApp->Endpoint ();

  if (entry->value->type == GCONF_VALUE_BOOL) {

    if (gconf_client_get_bool (client, "/apps/gnomemeeting/ldap/register", 0))
      (GM_ILS_CLIENT (endpoint->GetILSClientThread ()))->Modify ();

    gdk_threads_enter ();
    if (MyApp->Endpoint ()->GetCallingState () == 0)
      MyApp->Endpoint ()->UpdateConfig ();
    gdk_threads_leave ();
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
  
  PString current_call_token;
  
  GmWindow *gw = NULL;
  endpoint = MyApp->Endpoint ();
  
  if (entry->value->type == GCONF_VALUE_BOOL) {

    gdk_threads_enter ();

    current_call_token = endpoint->GetCurrentCallToken ();

    if (!current_call_token.IsEmpty ())
      connection =
	endpoint->FindConnectionWithLock (current_call_token);

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
	
	gw = gnomemeeting_get_main_window (gm);
	
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
	  
	  ac->SetSilenceDetectionMode(mode);
	}
      }

      connection->Unlock ();
    }

    gdk_threads_leave ();  
  }
}


/* DESCRIPTION  :  This callback is called to update capabilities.
 * BEHAVIOR     :  Updates them.
 * PRE          :  /
 */
static void capabilities_changed_nt (GConfClient *client, guint i, 
				     GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_INT
      || entry->value->type == GCONF_VALUE_STRING) {
   
    gdk_threads_enter ();

    /* We update the capabilities */
    if (MyApp->Endpoint ()->GetCallingState () == 0)
      MyApp->Endpoint ()->UpdateConfig ();

    gdk_threads_leave ();
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

  PString current_call_token;

  endpoint = MyApp->Endpoint ();
  
  int fps = 30;
  double frame_time = 0.0;

  if (entry->value->type == GCONF_VALUE_INT) {

    gdk_threads_enter ();
  
    current_call_token = endpoint->GetCurrentCallToken ();

    if (!current_call_token.IsEmpty ())
      connection =
	endpoint->FindConnectionWithLock (current_call_token);

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
    
    gdk_threads_leave ();
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

  PString current_call_token;

  int bitrate = 2;

  endpoint = MyApp->Endpoint ();
  

  if (entry->value->type == GCONF_VALUE_INT) {

    gdk_threads_enter ();

    current_call_token = endpoint->GetCurrentCallToken ();

    if (!current_call_token.IsEmpty ())
      connection =
	endpoint->FindConnectionWithLock (current_call_token);

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
    
    gdk_threads_leave ();
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

  PString current_call_token;
  
  int vq = 1;

  endpoint = MyApp->Endpoint ();

  if (entry->value->type == GCONF_VALUE_INT) {

    gdk_threads_enter ();

    current_call_token = endpoint->GetCurrentCallToken ();

    if (!current_call_token.IsEmpty ())
      connection =
	endpoint->FindConnectionWithLock (current_call_token);

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
      vq = 25 - (int) ((double) (int) gconf_value_get_int (entry->value) / 100 * 24);
  
      if (vc != NULL)
	vc->SetTxMaxQuality (vq);

      connection->Unlock ();
    }

    
    gdk_threads_leave ();
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

  PString current_call_token;
  
  endpoint = MyApp->Endpoint ();

  if (entry->value->type == GCONF_VALUE_INT) {

    gdk_threads_enter ();

        current_call_token = endpoint->GetCurrentCallToken ();

    if (!current_call_token.IsEmpty ())
      connection =
	endpoint->FindConnectionWithLock (current_call_token);

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
    
    gdk_threads_leave ();
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
  GmPrefWindow *pw = NULL;

  if (entry->value->type == GCONF_VALUE_INT) {

    gdk_threads_enter ();

    pw = gnomemeeting_get_pref_window (gm);

    min_val = 
      gconf_client_get_int (client, AUDIO_SETTINGS_KEY "min_jitter_buffer", 0);
    max_val = 
      gconf_client_get_int (client, AUDIO_SETTINGS_KEY "max_jitter_buffer", 0);
			    

    g_signal_handlers_block_by_func (G_OBJECT (data),
				     (gpointer) adjustment_changed, 
				       (gpointer) g_object_get_data (G_OBJECT (data), 
								     "gconf_key")); 

    if (data == pw->max_jitter_buffer)
      gtk_spin_button_set_range (GTK_SPIN_BUTTON (pw->min_jitter_buffer),
				 20.0, (gdouble) max_val+1);

    if (data == pw->min_jitter_buffer)
      gtk_spin_button_set_range (GTK_SPIN_BUTTON (pw->max_jitter_buffer),
				 (gdouble) min_val, 1000.0);

    g_signal_handlers_unblock_by_func (G_OBJECT (data),
				       (gpointer) adjustment_changed, 
				       (gpointer) g_object_get_data (G_OBJECT (data), 
								     "gconf_key")); 


    /* We update the current value */
    connection = ep->GetCurrentConnection ();

    if (connection != NULL)
      session =                                                                
        connection->GetSession (OpalMediaFormat::DefaultAudioSessionID);       
                                                                               
    if (session != NULL)                                                       
      session->SetJitterBufferSize ((int) min_val * 8, (int) max_val * 8); 

    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the audio mixer changes.
 * BEHAVIOR     :  It updates the sliders if a quicknet card is not used.
 * PRE          :  data = SOURCE_AUDIO or SOURCE_MIC
 */
static void
audio_mixer_changed_nt (GConfClient *client,
			guint cid, 
			GConfEntry *entry,
			gpointer data)
{
  int vol = 0;
  char *mixer = NULL;
  GmWindow *gw = NULL;
#ifdef HAS_IXJ
  GMH323EndPoint *endpoint = MyApp->Endpoint ();
  GMLid *lid = NULL;
#endif
  
  if (entry->value->type == GCONF_VALUE_STRING) {

#ifdef HAS_IXJ
    lid = endpoint->GetLidThread ();

    if (!lid) {
#endif
      gdk_threads_enter ();

      gw = gnomemeeting_get_main_window (gm);
      mixer = (char *) gconf_value_get_string (entry->value);

      vol = gnomemeeting_get_mixer_volume (mixer, GPOINTER_TO_INT (data));

      if (GPOINTER_TO_INT (data) == SOURCE_AUDIO)
	gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_play),
				  (int) (vol & 255));
      else 
	gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_rec),
				  (int) (vol & 255));

      gdk_threads_leave ();
#ifdef HAS_IXJ
    }
#endif
  }
}


/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the audio devices changes.
 * BEHAVIOR     :  It updates the endpoint and displays
 *                 a message in the history. If the device is not valid,
 *                 i.e. the user erroneously used gconftool, a message is
 *                 displayed. Notice that the code ensures that either no
 *                 no Quicknet device is used, or it is used for both devices.
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
    dw = gnomemeeting_get_druid_window (gm);
    pw = gnomemeeting_get_pref_window (gm);
    gw = gnomemeeting_get_main_window (gm);
      
    dev = PString (gconf_value_get_string (entry->value));

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


#ifdef HAS_IXJ
/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the lid device changes.
 * BEHAVIOR     :  It updates the endpoint and displays
 *                 a message in the history. If the device is not valid,
 *                 a message is displayed. Disable Speaker Phone mode, and
 *                 show/hide the toolbar button for speaker phone following
 *                 Quicknet is used or not.
 * PRE          :  /
 */
static void lid_device_changed_nt (GConfClient *client, guint cid, 
				   GConfEntry *entry, gpointer data)
{
  GmWindow *gw = NULL;

  if (entry->value->type == GCONF_VALUE_BOOL) {

    gdk_threads_enter ();
    gw = gnomemeeting_get_main_window (gm);
    
    if (MyApp->Endpoint ()->GetCallingState () == 0) {

      /* Update the configuration in order to update 
	 the local user name for calls */
      MyApp->Endpoint ()->UpdateConfig ();
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gw->speaker_phone_button),
				   FALSE);
    }

    /*
    if (gconf_value_get_bool (entry->value))
      gtk_widget_show_all (gw->speaker_phone_button);
    else
      gtk_widget_hide_all (gw->speaker_phone_button);
    */

    gdk_threads_leave ();
  }
}
#endif


/* DESCRIPTION  :  This callback is called when the video device changes in
 *                 the gconf database.
 * BEHAVIOR     :  It resets the video device.
 * PRE          :  /
 */
static void video_device_setting_changed_nt (GConfClient *client, guint cid, 
					     GConfEntry *entry, gpointer data)
{
  GMVideoGrabber *vg = NULL;

  if ((entry->value->type == GCONF_VALUE_STRING) ||
      (entry->value->type == GCONF_VALUE_INT)) {
  
    gdk_threads_enter ();

    /* We reset the video device */
    if (MyApp->Endpoint ()->GetCallingState () == 0) {
    
      vg = MyApp->GetVideoGrabber ();

      if (vg)
	vg->Reset ();
    }
  
    gdk_threads_leave ();
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
  if (entry->value->type == GCONF_VALUE_BOOL) {
   
    gdk_threads_enter ();
         
    /* We reset the video device */
    if (MyApp->Endpoint ()->GetCallingState () == 0) {
    
      if (gconf_value_get_bool (entry->value)) 
	MyApp->CreateVideoGrabber ();
      else 
	MyApp->RemoveVideoGrabber ();
    }
 
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This callback is called when something changes in the audio
 *                 codecs clist.
 * BEHAVIOR     :  It updates the widgets, and updates the capabilities of the
 *                 endpoint.
 * PRE          :  /
 */
static void audio_codecs_list_changed_nt (GConfClient *client, guint cid, 
					  GConfEntry *entry, gpointer data)
{ 
  GmPrefWindow *pw = NULL;
  
  if (entry->value->type == GCONF_VALUE_LIST) {
   
    gdk_threads_enter ();

    pw = gnomemeeting_get_pref_window (gm);

    /* We set the new value for the widget */
    gnomemeeting_codecs_list_build (pw->codecs_list_store);
    
    /* We update the capabilities */
    if (MyApp->Endpoint ()->GetCallingState () == 0)
      MyApp->Endpoint ()->UpdateConfig ();
    
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
  
  int cpt = 0;

  GtkWidget *page = NULL;
  GtkListStore *list_store = NULL;
  
  GmLdapWindow *lw = NULL;
  GmLdapWindowPage *lwp = NULL;
  
  if (e->value->type == GCONF_VALUE_LIST) {
  
    gdk_threads_enter ();

    lw = gnomemeeting_get_ldap_window (gm);
    
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

	  if (lwp && lwp->contact_section_name
	      && !strcasecmp (lwp->contact_section_name, group_name)) 
	    break;

	  cpt++;
	}

	if (lwp) {

	  list_store =
	    GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (lwp->tree_view)));
	  gnomemeeting_addressbook_group_populate (list_store, group_name);
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


/* DESCRIPTION  :  This callback is called when the a forward gconf value 
 *                 changes.
 * BEHAVIOR     :  It checks that there is a forwarding host specified, if
 *                 not, disable forwarding and displays a popup.
 * PRE          :  /
 */
static void forward_toggle_changed_nt (GConfClient *client, guint cid, 
				       GConfEntry *entry, gpointer data)
{
  GmWindow *gw = NULL;
  gchar *gconf_string = NULL;
  GtkWidget *msg_box = NULL;


  if ((entry->value->type == GCONF_VALUE_BOOL)&&
      (gconf_value_get_bool (entry->value))) {
 
    gdk_threads_enter ();

    gw = gnomemeeting_get_main_window (gm);

    /* Checks if the forward host name is ok */
    gconf_string =  gconf_client_get_string (GCONF_CLIENT (client), "/apps/gnomemeeting/call_forwarding/forward_host", NULL);
      
    if ((gconf_string == NULL) || (!strcmp (gconf_string, ""))) {
	
      msg_box = 
	gtk_message_dialog_new (GTK_WINDOW (gw->pref_window),
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				_("You need to specify an host to forward calls to!\nDisabling forwarding."));
      
      gtk_widget_show (msg_box);
      g_signal_connect_swapped (GTK_OBJECT (msg_box), "response",
				G_CALLBACK (gtk_widget_destroy),
				GTK_OBJECT (msg_box));
      
      gconf_client_set_bool (client, gconf_entry_get_key (entry), 0, NULL);   
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
  GMILSClient *ils_client = GM_ILS_CLIENT (endpoint->GetILSClientThread ());

  if (entry->value->type == GCONF_VALUE_BOOL) {

    gdk_threads_enter ();
    
    if (gconf_value_get_bool (entry->value))
      ils_client->Register ();
    else
      ils_client->Unregister ();

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
  GMILSClient *ils_client = GM_ILS_CLIENT (endpoint->GetILSClientThread ());

  if (entry->value->type == GCONF_VALUE_BOOL) {

    if (gconf_client_get_bool (client, "/apps/gnomemeeting/ldap/register", 0))
      ils_client->Modify ();
  }
}


/* DESCRIPTION  :  This callback is called when the "do_not_disturb" 
 *                 gconf value changes.
 * BEHAVIOR     :  Simply issued a modify request if we are regitered to an ILS
 *                 directory, and also modifies the tray icon.
 * PRE          :  /
 */
static void do_not_disturb_changed_nt (GConfClient *client, guint cid, 
				       GConfEntry *entry, gpointer data)
{
  GMH323EndPoint *endpoint = MyApp->Endpoint ();
  GMILSClient *ils_client = GM_ILS_CLIENT (endpoint->GetILSClientThread ());
  GmWindow *gw = NULL;

  if (entry->value->type == GCONF_VALUE_BOOL) {

    if (gconf_client_get_bool (client, "/apps/gnomemeeting/ldap/register", 0))
      ils_client->Modify ();

    gdk_threads_enter ();
    gw = gnomemeeting_get_main_window (gm);

    if (gconf_value_get_bool (entry->value))
      gnomemeeting_tray_set_content (G_OBJECT (gw->docklet), 2);
    else
      gnomemeeting_tray_set_content (G_OBJECT (gw->docklet), 0);
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
  gconf_client_set_int (client, "/apps/gnomemeeting/general/kind_of_net",
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
  if (entry->value->type == GCONF_VALUE_INT) {

    int lid_aec = gconf_value_get_int (entry->value);
    OpalLineInterfaceDevice *lid = NULL;
    GMLid *lid_thread = MyApp->Endpoint ()->GetLidThread ();

    if (lid_thread)
      lid = lid_thread->GetLidDevice ();

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
  if (entry->value->type == GCONF_VALUE_STRING) {
    
    GMLid *lid_thread = GM_LID (MyApp->Endpoint ()->GetLidThread ());
    OpalLineInterfaceDevice *lid = NULL;

    if (lid_thread)
      lid = lid_thread->GetLidDevice ();

    if ((gconf_value_get_string (entry->value))&&(lid))
      lid->SetCountryCodeName (gconf_value_get_string (entry->value));
  }
}
#endif


/* The functions  */
void gnomemeeting_init_gconf (GConfClient *client)
{
  GmDruidWindow *dw = gnomemeeting_get_druid_window (gm);
  GmPrefWindow *pw = gnomemeeting_get_pref_window (gm);
  GmWindow *gw = gnomemeeting_get_main_window (gm);
  MenuEntry *gnomemeeting_menu = gnomemeeting_get_menu (gm);
#ifndef WIN32
  MenuEntry *tray_menu = gnomemeeting_get_tray_menu (gm);
#endif
  
  /* There are in general 2 notifiers to attach to each widget :
     - the notifier that will update the widget itself to the new key
     - the notifier to take an appropriate action */
  
  /* gnomemeeting_init_pref_window_general */
  gconf_client_notify_add (client, "/apps/gnomemeeting/gatekeeper/gk_alias",
			   entry_changed_nt, pw->gk_alias, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/gatekeeper/gk_alias",
			   entry_changed_nt, dw->gk_alias, 0, 0);

  gconf_client_notify_add (client, 
			   "/apps/gnomemeeting/personal_data/firstname",
			   entry_changed_nt, pw->firstname, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/personal_data/mail",
			   entry_changed_nt, pw->mail, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/personal_data/lastname",
			   entry_changed_nt, pw->surname, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/personal_data/location",
			   entry_changed_nt, pw->location, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/personal_data/comment",
			   entry_changed_nt, pw->comment, 0, 0);


  /* gnomemeeting_init_pref_window_interface */
  gconf_client_notify_add (client, "/apps/gnomemeeting/view/show_popup", 
			   toggle_changed_nt, pw->incoming_call_popup, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/view/show_splash", 
			   toggle_changed_nt, pw->show_splash, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/view/control_panel_section", menu_radio_changed_nt, gnomemeeting_menu, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/view/control_panel_section", main_notebook_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/view/show_status_bar", menu_toggle_changed_nt, gnomemeeting_menu [STATUS_BAR_VIEW_MENU_INDICE].widget, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/view/show_status_bar", view_widget_changed_nt, gw->statusbar, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/view/show_chat_window", menu_toggle_changed_nt, gnomemeeting_menu [CHAT_WINDOW_VIEW_MENU_INDICE].widget, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/view/show_chat_window", view_widget_changed_nt, gw->chat_window, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/general/auto_answer", menu_toggle_changed_nt, gnomemeeting_menu [AA_CALL_MENU_INDICE].widget, 0, 0);
#ifndef WIN32
  gconf_client_notify_add (client, "/apps/gnomemeeting/general/auto_answer", 
			   menu_toggle_changed_nt,
			   tray_menu [4].widget,
			   0, 0);
#endif
  gconf_client_notify_add (client, "/apps/gnomemeeting/general/auto_answer", toggle_changed_nt, pw->aa, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/general/do_not_disturb", toggle_changed_nt, pw->dnd, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/general/do_not_disturb", menu_toggle_changed_nt, gnomemeeting_menu [DND_CALL_MENU_INDICE].widget, 0, 0);
#ifndef WIN32
  gconf_client_notify_add (client, "/apps/gnomemeeting/general/do_not_disturb",
			   menu_toggle_changed_nt, 
			   tray_menu [3].widget,
			   0, 0);
#endif
  gconf_client_notify_add (client, "/apps/gnomemeeting/general/do_not_disturb", do_not_disturb_changed_nt, pw->dnd, 0, 0);


#ifdef HAS_SDL
  gconf_client_notify_add (client, "/apps/gnomemeeting/general/fullscreen_width", adjustment_changed_nt, pw->fullscreen_width, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/general/fullscreen_height", adjustment_changed_nt, pw->fullscreen_height, 0, 0);
#endif

  gconf_client_notify_add (client, "/apps/gnomemeeting/general/incoming_call_sound", toggle_changed_nt, pw->incoming_call_sound, 0, 0);


  /* gnomemeeting_init_pref_window_h323_advanced */
  gconf_client_notify_add (client, "/apps/gnomemeeting/call_forwarding/always_forward", toggle_changed_nt, pw->always_forward, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/call_forwarding/always_forward", forward_toggle_changed_nt, pw->always_forward, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/call_forwarding/busy_forward", toggle_changed_nt, pw->busy_forward, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/call_forwarding/busy_forward", forward_toggle_changed_nt, pw->busy_forward, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/call_forwarding/no_answer_forward", toggle_changed_nt, pw->no_answer_forward, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/call_forwarding/no_answer_forward", forward_toggle_changed_nt, pw->no_answer_forward, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/call_forwarding/forward_host", entry_changed_nt, pw->forward_host, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/general/h245_tunneling", toggle_changed_nt, pw->ht, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/general/h245_tunneling", applicability_check_nt, pw->ht, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/general/h245_tunneling", ht_fs_changed_nt, pw->ht, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/general/fast_start", toggle_changed_nt, pw->fs, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/general/fast_start", applicability_check_nt, pw->fs, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/general/fast_start", ht_fs_changed_nt, pw->fs, 0, 0);

  gconf_client_notify_add (client, GENERAL_KEY "user_input_capability",
			   capabilities_changed_nt, NULL, 0, 0);
  gconf_client_notify_add (client, GENERAL_KEY "user_input_capability",
			   applicability_check_nt, pw->uic, 0, 0);
  gconf_client_notify_add (client, GENERAL_KEY "user_input_capability",
			   int_option_menu_changed_nt, pw->uic, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/general/ip_translation", toggle_changed_nt, pw->ip_translation, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/general/public_ip", entry_changed_nt, pw->public_ip, 0, 0);

  /* gnomemeeting_init_pref_window_directories */
  gconf_client_notify_add (client, "/apps/gnomemeeting/ldap/ldap_server",
			   entry_changed_nt, pw->ldap_server, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/ldap/register",
			   register_changed_nt, pw->ldap, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/ldap/register",
			   toggle_changed_nt, pw->ldap, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/ldap/visible",
			   toggle_changed_nt, pw->ldap_visible, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/ldap/visible",
			   ldap_visible_changed_nt, pw->ldap_visible, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/gatekeeper/gk_host",
			   entry_changed_nt, pw->gk_host, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/gatekeeper/gk_id",
			   entry_changed_nt, pw->gk_id, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/gatekeeper/gk_password",
			   entry_changed_nt, pw->gk_password, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/gatekeeper/gk_password",
			   entry_changed_nt, dw->gk_password, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/gatekeeper/registering_method", int_option_menu_changed_nt, pw->gk, 0, 0);


  /* gnomemeeting_init_pref_window_devices */
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_player", string_option_menu_changed_nt, pw->audio_player, 0, 0);
#ifndef DISABLE_GNOME
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_player", string_option_menu_changed_nt, dw->audio_player, 0, 0);
#endif
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_player", audio_device_changed_nt, pw->audio_player, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_player", applicability_check_nt, pw->audio_player, 0, 0);
  
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_player_mixer", string_option_menu_changed_nt, pw->audio_player_mixer, 0, 0);
#ifndef DISABLE_GNOME
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_player_mixer", string_option_menu_changed_nt, dw->audio_player_mixer, 0, 0);
#endif
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_player_mixer", audio_mixer_changed_nt, GINT_TO_POINTER (SOURCE_AUDIO), 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_recorder", string_option_menu_changed_nt, pw->audio_recorder, 0, 0);
#ifndef DISABLE_GNOME
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_recorder", string_option_menu_changed_nt, dw->audio_recorder, 0, 0);
#endif
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_recorder", audio_device_changed_nt, pw->audio_recorder, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_recorder", applicability_check_nt, pw->audio_recorder, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_recorder_mixer", string_option_menu_changed_nt, pw->audio_recorder_mixer, 0, 0);
#ifndef DISABLE_GNOME
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_recorder_mixer", string_option_menu_changed_nt, dw->audio_recorder_mixer, 0, 0);
#endif
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_recorder_mixer", audio_mixer_changed_nt, GINT_TO_POINTER (SOURCE_MIC), 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_recorder", string_option_menu_changed_nt, pw->video_device, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_recorder", video_device_setting_changed_nt, pw->video_device, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_recorder", applicability_check_nt, pw->video_device, 0, 0);			   

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_channel", video_device_setting_changed_nt, pw->video_channel, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_channel", adjustment_changed_nt, pw->video_channel, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_channel", applicability_check_nt, pw->video_channel, 0, 0);			   

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_size", int_option_menu_changed_nt, pw->opt1, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_size", video_device_setting_changed_nt, pw->opt1, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_size", capabilities_changed_nt, pw->opt1, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_size", applicability_check_nt, pw->opt1, 0, 0);			   

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_format", int_option_menu_changed_nt, pw->opt2, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_format", video_device_setting_changed_nt, pw->opt2, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_format", applicability_check_nt, pw->opt2, 0, 0);			   

 gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_image", entry_changed_nt, pw->video_image, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_image", video_device_setting_changed_nt, pw->video_image, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_image", applicability_check_nt, pw->video_image, 0, 0);		

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_preview", toggle_changed_nt, gw->preview_button, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_preview", video_preview_changed_nt, gw->preview_button, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_preview", applicability_check_nt, gw->preview_button, 0, 0);

#ifdef HAS_IXJ
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/lid_country", entry_changed_nt, pw->lid_country, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/lid_country", lid_country_changed_nt, pw->lid_country, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/lid_aec", int_option_menu_changed_nt, pw->lid_aec, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/lid_aec", lid_aec_changed_nt, pw->lid_aec, 0, 0);
#endif


  /* gnomemeeting_pref_window_audio_codecs */
  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_codecs/codecs_list", audio_codecs_list_changed_nt, pw->codecs_list_store, 0, 0);	     

  gconf_client_notify_add (client, AUDIO_SETTINGS_KEY "min_jitter_buffer", 
			   jitter_buffer_changed_nt, pw->min_jitter_buffer, 
			   0, 0);
  gconf_client_notify_add (client, AUDIO_SETTINGS_KEY "min_jitter_buffer", 
			   adjustment_changed_nt, pw->min_jitter_buffer, 
			   0, 0);

  gconf_client_notify_add (client, AUDIO_SETTINGS_KEY "max_jitter_buffer", 
			   jitter_buffer_changed_nt, pw->max_jitter_buffer, 
			   0, 0);
  gconf_client_notify_add (client, AUDIO_SETTINGS_KEY "max_jitter_buffer", 
			   adjustment_changed_nt, pw->max_jitter_buffer, 
			   0, 0);


  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_settings/gsm_frames", capabilities_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_settings/g711_frames", capabilities_changed_nt, NULL, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_settings/sd", silence_detection_changed_nt, pw->sd, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_settings/sd", toggle_changed_nt, pw->sd, 0, 0);


  /* gnomemeeting_pref_window_video_codecs */
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_fps", fps_limit_changed_nt, pw->tr_fps, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_fps", adjustment_changed_nt, pw->tr_fps, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_fps", network_settings_changed_nt, 0, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/enable_video_reception", applicability_check_nt, pw->vid_re, 0, 0);	     
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/enable_video_reception", toggle_changed_nt, pw->vid_re, 0, 0);	     
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/enable_video_reception", network_settings_changed_nt, 0, 0, 0);	     
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/enable_video_reception", enable_vid_tr_changed_nt, 0, 0, 0);	     

  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/enable_video_transmission", applicability_check_nt, pw->vid_tr, 0, 0);	     
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/enable_video_transmission", toggle_changed_nt, pw->vid_tr, 0, 0);	     
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/enable_video_transmission", network_settings_changed_nt, 0, 0, 0);	     
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/enable_video_transmission", enable_vid_tr_changed_nt, 0, 0, 0);	     

  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/maximum_video_bandwidth", maximum_video_bandwidth_changed_nt, pw->maximum_video_bandwidth, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/maximum_video_bandwidth", adjustment_changed_nt, pw->maximum_video_bandwidth, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/maximum_video_bandwidth", network_settings_changed_nt, 0, 0, 0);


  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_vq", tr_vq_changed_nt, pw->tr_vq, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_vq", adjustment_changed_nt, pw->tr_vq, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_vq", network_settings_changed_nt, 0, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/re_vq", network_settings_changed_nt, 0, 0, 0);


  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_ub", tr_ub_changed_nt, pw->tr_ub, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_ub", adjustment_changed_nt, pw->tr_ub, 0, 0);


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
}




void entry_changed (GtkEditable  *e, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();
  gchar *key = (gchar *) data;

  gconf_client_set_string (GCONF_CLIENT (client),
                           key,
                           gtk_entry_get_text (GTK_ENTRY (e)),
                           NULL);
}


void adjustment_changed (GtkAdjustment *adj, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();
  gchar *key = (gchar *) data;

  gconf_client_set_int (GCONF_CLIENT (client),
                        key,
                        (int) adj->value, NULL);
}


void toggle_changed (GtkCheckButton *but, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();
  gchar *key = (gchar *) data;

  gconf_client_set_bool (GCONF_CLIENT (client),
                         key,
                         gtk_toggle_button_get_active
                         (GTK_TOGGLE_BUTTON (but)),
                         NULL);
}


void int_option_menu_changed (GtkWidget *menu, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();
  gchar *key = (gchar *) data;
  guint item_index;
  GtkWidget *active_item;

  active_item = gtk_menu_get_active (GTK_MENU (menu));
  item_index = g_list_index (GTK_MENU_SHELL (GTK_MENU (menu))->children, 
			     active_item);
 
  gconf_client_set_int (GCONF_CLIENT (client),
			key, item_index, NULL);
}


void string_option_menu_changed (GtkWidget *menu, gpointer data)
{
  GtkWidget *active_item;
  const gchar *text;
  GConfClient *client = gconf_client_get_default ();

  gchar *key = (gchar *) data;


  active_item = gtk_menu_get_active (GTK_MENU (menu));
  if (active_item == NULL)
    text = "";
  else
    text = gtk_label_get_text (GTK_LABEL (GTK_BIN (active_item)->child));

  gconf_client_set_string (GCONF_CLIENT (client),
			   key, text, NULL);
}


