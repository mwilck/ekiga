
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
 *                         config.cpp  -  description
 *                         --------------------------
 *   begin                : Wed Feb 14 2001
 *   copyright            : (C) 2000-2002 by Damien Sandras 
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
#include "common.h"
#include "callbacks.h"
#include "sound_handling.h"
#include "videograbber.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "pref_window.h"
#include "main_window.h"
#include "ils.h"
#include "dialog.h"
#include "tray.h"


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
static void gatekeeper_method_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void main_notebook_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void fps_limit_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void audio_mixer_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void tr_vq_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void tr_ub_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void jitter_buffer_changed_nt (GConfClient*, guint, GConfEntry *, 
				      gpointer);
static void register_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void do_not_disturb_changed_nt (GConfClient*, guint, 
				       GConfEntry *, gpointer);
static void forward_toggle_changed_nt (GConfClient*, guint, GConfEntry *, 
				       gpointer);
static void audio_device_changed_nt (GConfClient *, guint, GConfEntry *, 
				     gpointer);
static void video_device_setting_changed_nt (GConfClient *, guint, 
					     GConfEntry *, gpointer);
static void video_preview_changed_nt (GConfClient *, guint, GConfEntry *, 
				      gpointer);
static void enable_fps_changed_nt (GConfClient *, guint, GConfEntry *, 
				   gpointer);
static void audio_codecs_list_changed_nt (GConfClient *, guint, GConfEntry *, 
					  gpointer);
static void view_widget_changed_nt (GConfClient *, guint, GConfEntry *, 
				    gpointer);
static void audio_codec_setting_changed_nt (GConfClient *, guint, 
					    GConfEntry *, gpointer);
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


/* DESCRIPTION  :  Generic notifiers for radios in the menu.
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
  
    GnomeUIInfo *e = (GnomeUIInfo *) (data);

    /* We set the new value for the widget */
    for (int i = 0 ; i <= GM_MAIN_NOTEBOOK_HIDDEN ; i++) {

      if (gconf_value_get_int (entry->value) == i)
	GTK_CHECK_MENU_ITEM (e [i].widget)->active = TRUE;
      else
	GTK_CHECK_MENU_ITEM (e [i].widget)->active = FALSE;

      gtk_widget_queue_draw (GTK_WIDGET (e [i].widget));
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
 *                 the gconf database associated with an option menu changes, this
 *                 only updates the menu.
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


/* DESCRIPTION  :  This callback is called when something changes for the mixers.
 * BEHAVIOR     :  It updates the GUI to use the new values.
 * PRE          :  /
 */
static void audio_mixer_changed_nt (GConfClient *client, guint cid, 
				    GConfEntry *entry, gpointer data)
{
  gchar *player_mixer = NULL;
  gchar *recorder_mixer = NULL;
  gchar *text = NULL;
  int vol_play = 0, vol_rec = 0;

  GmWindow *gw = NULL;

  player_mixer = 
    gconf_client_get_string (client, "/apps/gnomemeeting/devices/audio_player_mixer", 0);
  recorder_mixer = 
    gconf_client_get_string (client, "/apps/gnomemeeting/devices/audio_recorder_mixer", 0);

  if (entry->value->type == GCONF_VALUE_STRING) {

    gdk_threads_enter ();

    gw = gnomemeeting_get_main_window (gm);

    /* Get the volumes for the mixers */
    gnomemeeting_volume_get (player_mixer, 0, &vol_play);
    gnomemeeting_volume_get (recorder_mixer, 1, &vol_rec);
    
    gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_play),
			      (int) (vol_play / 257));
    gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_rec),
			      (int) (vol_rec / 257));
    
    gnomemeeting_set_recording_source (recorder_mixer, 0); 

    /* Set recording source and set micro to record */
    /* Translators: This is shown in the history. */
    text = g_strdup_printf (_("Set Audio Mixer for player to %s"),
			    player_mixer);
    gnomemeeting_log_insert (gw->history_text_view, text);
    g_free (text);
    
    /* Translators: This is shown in the history. */
    text = g_strdup_printf (_("Set Audio Mixer for recorder to %s"),
			    recorder_mixer);
    gnomemeeting_log_insert (gw->history_text_view, text);
    g_free (text);

    gnomemeeting_set_recording_source (recorder_mixer, 0);

    g_free (player_mixer);
    g_free (recorder_mixer);
  
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


/* DESCRIPTION  :  This callback is called when the registering method to  the
 *                 gatekeeper options changes.
 * BEHAVIOR     :  It unregisters, registers to the gatekeeper using the class and with
 *                 the required method.
 * PRE          :  /
 */
static void gatekeeper_method_changed_nt (GConfClient *client, guint cid, 
					  GConfEntry *entry, 
					  gpointer data)
{
  if (entry->value->type == GCONF_VALUE_INT) {

    gdk_threads_enter ();

    /* We update the registering to the gatekeeper */
    /* Remove the current Gatekeeper */
    MyApp->Endpoint ()->RemoveGatekeeper(0);
    
    /* Register the current Endpoint to the Gatekeeper */
    MyApp->Endpoint ()->GatekeeperRegister ();

    gdk_threads_leave ();

  }
} 


/* DESCRIPTION  :  This callback is called when the control panel section changes.
 * BEHAVIOR     :  Sets the right page or hide it, and also sets the good value for
 *                 the toggle in the prefs.
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

    MyApp->Endpoint ()->UpdateConfig ();
    
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the enable_video_transmission key changes.
 * BEHAVIOR     :  It updates the endpoint, and updates the registering on ILS.
 * PRE          :  /
 */
static void enable_vid_tr_changed_nt (GConfClient *client, guint cid, 
				      GConfEntry *entry, gpointer data)
{
  GMH323EndPoint *endpoint = MyApp->Endpoint ();
  GMILSClient *ils_client = (GMILSClient *) endpoint->GetILSClient ();

  if (entry->value->type == GCONF_VALUE_BOOL) {

    if (gconf_client_get_bool (client, "/apps/gnomemeeting/ldap/register", 0))
      ils_client->Modify ();

    gdk_threads_enter ();
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
  H323AudioCodec *ac = NULL;
  GmWindow *gw = NULL;

  if (entry->value->type == GCONF_VALUE_BOOL) {

    gdk_threads_enter ();

    /* We update the silence detection */
    if (MyApp->Endpoint ()->GetCallingState () == 2) {

      ac = MyApp->Endpoint ()->GetCurrentAudioCodec ();
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

    gdk_threads_leave ();

  }
}


/* DESCRIPTION  :  This callback is called to update one audio codec related 
 *                 setting (# frames, but not silence detection)
 * BEHAVIOR     :  Update it, but display a popup if in a call.
 * PRE          :  /
 */
static void audio_codec_setting_changed_nt (GConfClient *client, guint i, 
					    GConfEntry *entry, gpointer data)
{
  int video_size = 0;

  if (entry->value->type == GCONF_VALUE_INT) {
   
    gdk_threads_enter ();

    /* We update the capabilities */
    MyApp->Endpoint ()->RemoveAllCapabilities ();
    MyApp->Endpoint ()->AddAudioCapabilities ();
    video_size = gconf_client_get_int (client, "/apps/gnomemeeting/devices/video_size", 0);
    MyApp->Endpoint ()->AddVideoCapabilities (video_size);

    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This callback is called to update the fps limitation.
 * BEHAVIOR     :  Update it.
 * PRE          :  /
 */
static void fps_limit_changed_nt (GConfClient *client, guint cid, 
				  GConfEntry *entry, gpointer data)
{
  GMVideoGrabber *vg = NULL;

  if (entry->value->type == GCONF_VALUE_INT) {

    gdk_threads_enter ();
  
    /* We update the current frame rate */
    vg = MyApp->Endpoint ()->GetVideoGrabber ();
  
    if ((vg != NULL)
	&&(gconf_client_get_bool (client, 
				  "/apps/gnomemeeting/video_settings/enable_fps",
				  NULL)))
      vg->SetFrameRate (gconf_value_get_int (entry->value));

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
  H323VideoCodec *vc = NULL;
  int vq = 1;


  if (entry->value->type == GCONF_VALUE_INT) {

    gdk_threads_enter ();
  
    /* We update the video quality */
    vc = MyApp->Endpoint ()->GetCurrentVideoCodec ();
  
    vq = 32 - (int) ((double) (int) gconf_value_get_int (entry->value) / 100 * 31);
  
    if (vc != NULL)
      vc->SetTxQualityLevel (vq);
  
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
  H323VideoCodec *vc = NULL;

  if (entry->value->type == GCONF_VALUE_INT) {

    gdk_threads_enter ();

    /* We update the current tr ub rate */
    vc = MyApp->Endpoint ()->GetCurrentVideoCodec ();
    
    if (vc != NULL)
      vc->SetBackgroundFill ((int) gconf_value_get_int (entry->value));
    
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
  H323EndPoint *ep = MyApp->Endpoint ();  
  gdouble val = 20.0;

  if (entry->value->type == GCONF_VALUE_INT) {

    gdk_threads_enter ();

    val = gconf_value_get_int (entry->value);

    /* We update the current value */
    connection = MyApp->Endpoint ()->GetCurrentConnection ();

    if (connection != NULL)
      session =                                                                
        connection->GetSession (OpalMediaFormat::DefaultAudioSessionID);       
                                                                               
    if (session != NULL)                                                       
      session->SetJitterBufferSize ((int) val * 8, 
				    ep->GetJitterThreadStackSize());

    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This notifier is called when the video size is changed.
 * BEHAVIOR     :  Updates the capabilities.
 * PRE          :  /
 */
static void video_size_changed_nt (GConfClient *client, guint cid, 
				   GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_INT) {

    gdk_threads_enter ();

    /* We update the Endpoint */
    MyApp->Endpoint ()->RemoveAllCapabilities ();
    MyApp->Endpoint ()->AddAudioCapabilities ();
    
    MyApp->Endpoint ()
      ->AddVideoCapabilities (gconf_value_get_int (entry->value));
    
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the audio devices changes.
 * BEHAVIOR     :  It updates the endpoint and displays
 *                 a message in the history. If the device is not valid,
 *                 i.e. the user erroneously used gconftool, a message is
 *                 displayed.
 * PRE          :  /
 */
static void audio_device_changed_nt (GConfClient *client, guint cid, 
				     GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_STRING) {

    gdk_threads_enter ();
  
    if (MyApp->Endpoint ()->GetCallingState () == 0)
      /* Update the configuration in order to update 
	 the local user name for calls */
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

    if (gconf_value_get_bool (entry->value))
      gtk_widget_show_all (gw->speaker_phone_button);
    else
      gtk_widget_hide_all (gw->speaker_phone_button);

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
    
      vg = MyApp->Endpoint ()->GetVideoGrabber ();

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

    GMVideoGrabber *vg = NULL;
         
    /* We reset the video device */
    if (MyApp->Endpoint ()->GetCallingState () == 0) {
    
      vg = MyApp->Endpoint ()->GetVideoGrabber ();
    
      if (gconf_value_get_bool (entry->value)) {

	if (!vg->IsOpened ())
	  vg->Open (TRUE);
      }
      else {
      
	if (vg->IsOpened ())
	  vg->Close ();
      }
    }
 
    gdk_threads_leave ();
  }
}


/* DESCRIPTION  :  his callback is called when a specific key of
 *                 the gconf database associated with that toggle changes.
 * BEHAVIOR     :  It does SetFrameRate to 0 to disable it.
 * PRE          :  /
 */
static void enable_fps_changed_nt (GConfClient *client, guint cid, 
				   GConfEntry *entry, gpointer data)
{
  GMVideoGrabber *vg = NULL;

  if (entry->value->type == GCONF_VALUE_BOOL) {

    gdk_threads_enter ();
  
    /* Update the value */
    vg = MyApp->Endpoint ()->GetVideoGrabber ();
  
    if (vg) {
    
      /* Disable or enable tr fps limit */
      if (!gconf_value_get_bool (entry->value)) 
	vg->SetFrameRate (0);
      else
	vg->SetFrameRate (gconf_client_get_int (client, 
						"/apps/gnomemeeting/video_settings/tr_fps", 
						NULL));
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
  int video_size = 0;
  
  if (entry->value->type == GCONF_VALUE_STRING) {
   
    gdk_threads_enter ();

    pw = gnomemeeting_get_pref_window (gm);

    /* We set the new value for the widget */
    gnomemeeting_codecs_list_build (pw->codecs_list_store, 
				    (gchar *) gconf_value_get_string (entry->value));

    /* We update the capabilities */
    MyApp->Endpoint ()->RemoveAllCapabilities ();
    MyApp->Endpoint ()->AddAudioCapabilities ();
    video_size = gconf_client_get_int (client, "/apps/gnomemeeting/devices/video_size", 0);
    MyApp->Endpoint ()->AddVideoCapabilities (video_size);
    
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
  GMILSClient *ils_client = (GMILSClient *) endpoint->GetILSClient ();

  if (entry->value->type == GCONF_VALUE_BOOL) {

    gdk_threads_enter ();
    
    if (gconf_value_get_bool (entry->value))
      ils_client->Register ();
    else
      ils_client->Unregister ();

    gdk_threads_leave ();
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
  GMILSClient *ils_client = (GMILSClient *) endpoint->GetILSClient ();
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
    OpalLineInterfaceDevice *lid = MyApp->Endpoint ()->GetLidDevice ();

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

    OpalLineInterfaceDevice *lid = MyApp->Endpoint ()->GetLidDevice ();
    if ((gconf_value_get_string (entry->value))&&(lid))
      lid->SetCountryCodeName (gconf_value_get_string (entry->value));
  }
}
#endif


/* The functions  */
void gnomemeeting_init_gconf (GConfClient *client)
{
  GmPrefWindow *pw = gnomemeeting_get_pref_window (gm);
  GmWindow *gw = gnomemeeting_get_main_window (gm);
  GnomeUIInfo *view_menu = 
    (GnomeUIInfo *) g_object_get_data (G_OBJECT (gm), 
				       "view_menu_uiinfo");
  GnomeUIInfo *notebook_view_uiinfo = 
    (GnomeUIInfo *) g_object_get_data (G_OBJECT (gm), 
				       "notebook_view_uiinfo");
  GnomeUIInfo *call_menu = 
    (GnomeUIInfo *) g_object_get_data (G_OBJECT (gm), 
				       "call_menu_uiinfo");

  /* There are in general 2 notifiers to attach to each widget :
     - the notifier that will update the widget itself to the new key
     - the notifier to take an appropriate action */

  /* gnomemeeting_init_pref_window_general */
  gconf_client_notify_add (client, "/apps/gnomemeeting/gatekeeper/gk_alias",
			   entry_changed_nt, pw->gk_alias, 0, 0);

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

  gconf_client_notify_add (client, "/apps/gnomemeeting/view/control_panel_section", menu_radio_changed_nt, notebook_view_uiinfo, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/view/control_panel_section", main_notebook_changed_nt, notebook_view_uiinfo, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/view/show_status_bar", menu_toggle_changed_nt, view_menu [3].widget, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/view/show_status_bar", view_widget_changed_nt, gw->statusbar, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/view/show_docklet", menu_toggle_changed_nt, view_menu [4].widget, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/view/show_docklet", view_widget_changed_nt, gw->docklet, 0, 0);


  gconf_client_notify_add (client, "/apps/gnomemeeting/view/show_chat_window", menu_toggle_changed_nt, view_menu [1].widget, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/view/show_chat_window", view_widget_changed_nt, gw->chat_window, 0, 0);


  gconf_client_notify_add (client, "/apps/gnomemeeting/view/left_toolbar", menu_toggle_changed_nt, view_menu [0].widget, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/view/left_toolbar", view_widget_changed_nt, GTK_WIDGET (gnome_app_get_dock_item_by_name(GNOME_APP (gm), "left_toolbar")), 0, 0);


  gconf_client_notify_add (client, "/apps/gnomemeeting/general/auto_answer", menu_toggle_changed_nt, call_menu [4].widget, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/general/auto_answer", 
			   menu_toggle_changed_nt,
			   gnomemeeting_tray_get_uiinfo (G_OBJECT (gw->docklet), 4),
			   0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/general/auto_answer", toggle_changed_nt, pw->aa, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/general/do_not_disturb", toggle_changed_nt, pw->dnd, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/general/do_not_disturb", menu_toggle_changed_nt, call_menu [3].widget, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/general/do_not_disturb",
			   menu_toggle_changed_nt, 
			   gnomemeeting_tray_get_uiinfo (G_OBJECT (gw->docklet), 3),
			   0, 0);
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


  /* gnomemeeting_init_pref_window_directories */
  gconf_client_notify_add (client, "/apps/gnomemeeting/ldap/ldap_server",
			   entry_changed_nt, pw->ldap_server, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/ldap/register",
			   register_changed_nt, pw->ldap, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/ldap/register",
			   toggle_changed_nt, pw->ldap, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/gatekeeper/gk_host",
			   entry_changed_nt, pw->gk_host, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/gatekeeper/gk_id",
			   entry_changed_nt, pw->gk_id, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/gatekeeper/registering_method", gatekeeper_method_changed_nt, pw->gk, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/gatekeeper/registering_method", int_option_menu_changed_nt, pw->gk, 0, 0);


  /* gnomemeeting_init_pref_window_devices */
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_player", string_option_menu_changed_nt, pw->audio_player, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_player", audio_device_changed_nt, pw->audio_player, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_player", applicability_check_nt, pw->audio_player, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_recorder", string_option_menu_changed_nt, pw->audio_recorder, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_recorder", audio_device_changed_nt, pw->audio_recorder, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_recorder", applicability_check_nt, pw->audio_recorder, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_recorder", string_option_menu_changed_nt, pw->video_device, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_recorder", video_device_setting_changed_nt, pw->video_device, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_recorder", applicability_check_nt, pw->video_device, 0, 0);			   

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_channel", video_device_setting_changed_nt, pw->video_channel, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_channel", adjustment_changed_nt, pw->video_channel, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_channel", applicability_check_nt, pw->video_channel, 0, 0);			   

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_size", int_option_menu_changed_nt, pw->opt1, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_size", video_device_setting_changed_nt, pw->opt1, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_size", video_size_changed_nt, pw->opt1, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_size", applicability_check_nt, pw->opt1, 0, 0);			   

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_format", int_option_menu_changed_nt, pw->opt2, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_format", video_device_setting_changed_nt, pw->opt2, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_format", applicability_check_nt, pw->opt2, 0, 0);			   

 gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_image", entry_changed_nt, pw->video_image, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_image", video_device_setting_changed_nt, pw->video_image, 0, 0);			   
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_image", applicability_check_nt, pw->video_image, 0, 0);		

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_preview", video_preview_changed_nt, pw->video_preview, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_preview", toggle_changed_nt, gw->preview_button, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_preview", toggle_changed_nt, pw->video_preview, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_preview", applicability_check_nt, pw->video_preview, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_player_mixer", entry_changed_nt, pw->audio_player_mixer, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_player_mixer", audio_mixer_changed_nt, pw->audio_player_mixer, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_recorder_mixer", entry_changed_nt, pw->audio_recorder_mixer, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_recorder_mixer", audio_mixer_changed_nt, pw->audio_recorder_mixer, 0, 0);

#ifdef HAS_IXJ
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/lid_country", entry_changed_nt, pw->lid_country, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/lid_country", lid_country_changed_nt, pw->lid_country, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/lid_aec", int_option_menu_changed_nt, pw->lid_aec, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/lid_aec", lid_aec_changed_nt, pw->lid_aec, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/lid", toggle_changed_nt, pw->lid, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/lid", lid_device_changed_nt, pw->lid_country, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/lid", applicability_check_nt, pw->lid_country, 0, 0);
#endif


  /* gnomemeeting_pref_window_audio_codecs */
  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_codecs/list", audio_codecs_list_changed_nt, pw->codecs_list_store, 0, 0);	     

  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_settings/jitter_buffer", jitter_buffer_changed_nt, pw->jitter_buffer, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_settings/jitter_buffer", adjustment_changed_nt, pw->jitter_buffer, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_settings/jitter_buffer", network_settings_changed_nt, 0, 0, 0);


  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_settings/gsm_frames", audio_codec_setting_changed_nt, pw->gsm_frames, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_settings/gsm_frames", applicability_check_nt, pw->gsm_frames, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_settings/gsm_frames", adjustment_changed_nt, pw->gsm_frames, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_settings/g711_frames", audio_codec_setting_changed_nt, pw->g711_frames, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_settings/g711_frames", applicability_check_nt, pw->g711_frames, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_settings/g711_frames", adjustment_changed_nt, pw->g711_frames, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_settings/sd", silence_detection_changed_nt, pw->sd, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_settings/sd", toggle_changed_nt, pw->sd, 0, 0);


  /* gnomemeeting_pref_window_video_codecs */
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/enable_fps", enable_fps_changed_nt, pw->fps, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/enable_fps", toggle_changed_nt, pw->fps, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/enable_fps", network_settings_changed_nt, 0, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_fps", fps_limit_changed_nt, pw->tr_fps, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_fps", adjustment_changed_nt, pw->tr_fps, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_fps", network_settings_changed_nt, 0, 0, 0);


  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/enable_video_transmission", applicability_check_nt, pw->vid_tr, 0, 0);	     
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/enable_video_transmission", toggle_changed_nt, pw->vid_tr, 0, 0);	     
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/enable_video_transmission", network_settings_changed_nt, 0, 0, 0);	     
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/enable_video_transmission", enable_vid_tr_changed_nt, 0, 0, 0);	     

  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_vq", tr_vq_changed_nt, pw->tr_vq, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_vq", adjustment_changed_nt, pw->tr_vq, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_vq", network_settings_changed_nt, 0, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/re_vq", adjustment_changed_nt, pw->re_vq, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/re_vq", applicability_check_nt, pw->re_vq, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/re_vq", network_settings_changed_nt, 0, 0, 0);


  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_ub", tr_ub_changed_nt, pw->tr_ub, 0, 0);
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_ub", adjustment_changed_nt, pw->tr_ub, 0, 0);
}



