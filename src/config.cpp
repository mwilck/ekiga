
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


#include "config.h"
#include "common.h"
#include "audio.h"
#include "videograbber.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "pref_window.h"
#include "ils.h"

#include "../config.h"


/* Declarations */
extern GtkWidget *gm;
extern GnomeMeeting *MyApp;

static gboolean fps_limit_changed (gpointer);
static void fps_limit_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static gboolean vb_limit_changed (gpointer);
static void vb_limit_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static gboolean toggle_changed (gpointer);
static void toggle_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static gboolean entry_changed_ (gpointer);
static void entry_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static gboolean tr_vq_changed (gpointer);
static void tr_vq_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static gboolean re_vq_changed (gpointer);
static void re_vq_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static gboolean tr_ub_changed (gpointer);
static void tr_ub_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static gboolean jitter_buffer_changed (gpointer);
static void jitter_buffer_changed_nt (GConfClient*, guint, GConfEntry *, 
				      gpointer);
static gboolean register_changed (gpointer);
static void register_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static gboolean audio_mixer_changed (gpointer);
static void audio_mixer_changed_nt (GConfClient *, guint, GConfEntry *, gpointer);
static gboolean audio_device_changed (gpointer);
static void audio_device_changed_nt (GConfClient *, guint, GConfEntry *, 
				     gpointer);
static gboolean video_device_changed (gpointer);
static void video_device_changed_nt (GConfClient *, guint, GConfEntry *, 
				     gpointer);
static gboolean video_channel_changed (gpointer);
static void video_channel_changed_nt (GConfClient *, guint, GConfEntry *, 
				      gpointer);
static gboolean video_option_menu_changed (gpointer);
static void video_option_menu_changed_nt (GConfClient *, guint, GConfEntry *, 
					  gpointer);
static gboolean video_preview_changed (gpointer);
static void video_preview_changed_nt (GConfClient *, guint, GConfEntry *, 
				      gpointer);
static void enable_fps_changed_nt (GConfClient *, guint, GConfEntry *, 
				   gpointer);
static void enable_vb_changed_nt (GConfClient *, guint, GConfEntry *, 
				  gpointer);
static gboolean enable_vid_tr_changed (gpointer);
static void enable_vid_tr_changed_nt (GConfClient *, guint, GConfEntry *, 
				      gpointer);
static void audio_codecs_list_changed_nt (GConfClient *, guint, GConfEntry *, 
					  gpointer);
static gboolean view_widget_changed (gpointer);
static void view_widget_changed_nt (GConfClient *, guint, GConfEntry *, 
				    gpointer);
static gboolean notebook_info_changed (gpointer);
static void notebook_info_changed_nt (GConfClient *, guint, GConfEntry *, 
				      gpointer);
static void gnomemeeting_update_pref_window_sensitivity (void);


static gboolean fps_limit_changed (gpointer data)
{
  gdk_threads_enter ();

  GMVideoGrabber *vg = NULL;
  GConfClient *client = gconf_client_get_default ();
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  /* We set the new value for tr_fps_spin_adj */
  GTK_ADJUSTMENT (pw->tr_fps_spin_adj)->value = (int) data;
  gtk_widget_draw (pw->tr_fps, NULL);
  
  /* We update the current frame rate */
  vg = MyApp->Endpoint ()->GetVideoGrabber ();
  
  if ((vg != NULL)
      &&(gconf_client_get_bool (client, 
				"/apps/gnomemeeting/video_settings/enable_fps",
				NULL)))
    vg->SetFrameRate ((int) data);

  gdk_threads_leave ();
  
  return FALSE;
}


/* DESCRIPTION  :  This callback is called to update the fps limitation.
 * BEHAVIOR     :  Update it.
 * PRE          :  /
 */
static void fps_limit_changed_nt (GConfClient *client, guint cid, 
				  GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_INT) {
   
    g_idle_add (fps_limit_changed, 
		(gpointer) gconf_value_get_int (entry->value));
  }
}


static gboolean vb_limit_changed (gpointer data)
{
  gdk_threads_enter ();

  H323VideoCodec *vc = NULL;
  GConfClient *client = gconf_client_get_default ();
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  /* We set the new value for tr_fps_spin_adj */
  GTK_ADJUSTMENT (pw->video_bandwidth_spin_adj)->value = (int) data;
  gtk_widget_draw (pw->video_bandwidth, NULL);
  
  /* We update the current bitrate */
  vc = MyApp->Endpoint ()->GetCurrentVideoCodec ();

  if ((vc != NULL)
      &&(gconf_client_get_bool (client, "/apps/gnomemeeting/video_settings/enable_vb", NULL)))
    vc->SetAverageBitRate ((int) data * 1024 * 8);
  
  
  gdk_threads_leave ();
  
  return FALSE;
}


/* DESCRIPTION  :  This callback is called to update the vb limitation.
 * BEHAVIOR     :  Update it.
 * PRE          :  /
 */
static void vb_limit_changed_nt (GConfClient *client, guint cid, 
				  GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_INT) {
  
    g_idle_add (vb_limit_changed, 
		(gpointer) gconf_value_get_int (entry->value));
  }
}


static gboolean tr_vq_changed (gpointer data)
{
  gdk_threads_enter ();

  H323VideoCodec *vc = NULL;
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);
  int vq = 1;
   
  /* We set the new value for tr_fps_spin_adj */
  GTK_ADJUSTMENT (pw->tr_vq_spin_adj)->value = (int) data;
  gtk_widget_draw (pw->tr_vq, NULL);
  
  /* We update the video quality */
  vc = MyApp->Endpoint ()->GetCurrentVideoCodec ();
  
  vq = 32 - (int) ((double) (int) data / 100 * 31);
  
  if (vc != NULL)
    vc->SetTxQualityLevel (vq);
  
  gdk_threads_leave ();

  return FALSE;
}


/* DESCRIPTION  :  This callback is called the transmitted video quality.
 * BEHAVIOR     :  It updates the video quality.
 * PRE          :  data is a pointer to GM_prefs_window_widgets
 */
static void tr_vq_changed_nt (GConfClient *client, guint cid, 
			      GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_INT) {
    
    g_idle_add (tr_vq_changed,
		(gpointer) gconf_value_get_int (entry->value));
  }
}


static gboolean re_vq_changed (gpointer data)
{
  gdk_threads_enter ();

  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  /* We set the new value for re_vq_spin_adj */
  GTK_ADJUSTMENT (pw->re_vq_spin_adj)->value = (int) data;
  gtk_widget_draw (pw->re_vq, NULL);

  /* Display a popup if we are in a call */
  
  if (MyApp->Endpoint ()->GetCallingState ()) {
  
  gchar *msg = g_strdup (_("The new video quality hint for the received video stream will only apply to the next call."));
  
  gnomemeeting_warning_popup (pw->re_vq, msg);
  
  g_free (msg);
  }
  
  gdk_threads_leave ();
  
  return FALSE;
}


/* DESCRIPTION  :  This callback is called for the received video quality.
 * BEHAVIOR     :  Updates the widget. Displays a popup if we are in a call.
 * PRE          :  data is a pointer to GM_prefs_window_widgets
 */
static void re_vq_changed_nt (GConfClient *client, guint cid, 
			      GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_INT) {

    /* We put the real thing into an idle */
    g_idle_add (re_vq_changed, 
		(gpointer) gconf_value_get_int (entry->value));
  }
}


static gboolean tr_ub_changed (gpointer data)
{
  gdk_threads_enter ();

  H323VideoCodec *vc = NULL;
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  /* We set the new value for tr_fps_spin_adj */
  GTK_ADJUSTMENT (pw->tr_ub_spin_adj)->value = (int) data;
  gtk_widget_draw (pw->tr_ub, NULL);

  /* We update the current frame rate */
  vc = MyApp->Endpoint ()->GetCurrentVideoCodec ();
  
  if (vc != NULL)
    vc->SetBackgroundFill ((int) data);
  
  gdk_threads_leave ();

  return FALSE;
}


/* DESCRIPTION  :  This callback is called when the bg fill needs to be changed.
 * BEHAVIOR     :  It updates the background fill.
 * PRE          :  /
 */
static void tr_ub_changed_nt (GConfClient *client, guint cid, 
			      GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_INT) {
   
    g_idle_add (tr_ub_changed, 
		(gpointer) gconf_value_get_int (entry->value));
  }
}


static gboolean jitter_buffer_changed (gpointer data)
{
  gdk_threads_enter ();

  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);
  H323Connection *connection = NULL;

  /* We set the new value for tr_fps_spin_adj */
  GTK_ADJUSTMENT (pw->jitter_buffer_spin_adj)->value = (int) data;
  gtk_widget_draw (pw->jitter_buffer, NULL);

  /* We update the current value */
  connection = MyApp->Endpoint ()->GetCurrentConnection ();
    
  if (connection != NULL)
    connection->SetMaxAudioDelayJitter ((int) data);
  
  gdk_threads_leave ();

  return FALSE;
}


/* DESCRIPTION  :  This callback is called when the jitter buffer needs to be 
 *                 changed.
 * BEHAVIOR     :  It updates the widget and the value.
 * PRE          :  /
 */
static void jitter_buffer_changed_nt (GConfClient *client, guint cid, 
				      GConfEntry *entry, gpointer data)
{
  
  if (entry->value->type == GCONF_VALUE_INT) {
    
    g_idle_add (jitter_buffer_changed,
		(gpointer) gconf_value_get_int (entry->value));
  }
}


static gboolean toggle_changed (gpointer data)
{
  gdk_threads_enter ();

  GtkWidget *toggle = GTK_WIDGET (data);

  /* We set the new value for the widget
     This value is different from the previous one, or we are not called */
  GTK_TOGGLE_BUTTON (toggle)->active = !GTK_TOGGLE_BUTTON (toggle)->active; 
  gtk_widget_draw (GTK_WIDGET (toggle), NULL);

  gdk_threads_leave ();

  return FALSE;
}


/* DESCRIPTION  :  Generic notifiers for toggles.
 *                 This callback is called when a specific key of
 *                 the gconf database associated with a toggle changes.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  /
 */
static void toggle_changed_nt (GConfClient *client, guint cid, 
			       GConfEntry *entry, gpointer data)
{
  GtkWidget *toggle = GTK_WIDGET (data);

  if (entry->value->type == GCONF_VALUE_BOOL) {
   
    /* We set the new value for the widget if needed */
    if ((bool) GTK_TOGGLE_BUTTON (toggle)->active != gconf_value_get_bool (entry->value))
      g_idle_add (toggle_changed, (gpointer) toggle);
  }
}


static gboolean entry_changed_ (gpointer data)
{
  gdk_threads_enter ();
  
  GConfClient *client = gconf_client_get_default ();
  GtkWidget *e = GTK_WIDGET (data);
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  /* We set the new value for the widget */
  gtk_signal_handler_block_by_func (GTK_OBJECT (e),
				    GTK_SIGNAL_FUNC (entry_changed), 
				    (gpointer) gtk_object_get_data (GTK_OBJECT (e), "gconf_key")); 
  
  /* I can't set that value till Orbit2 and the new gconf are out! */

  /*  gtk_entry_set_text (GTK_ENTRY (e), gconf_value_get_string (entry->value));*/
  gtk_signal_handler_unblock_by_func (GTK_OBJECT (e),
				      GTK_SIGNAL_FUNC (entry_changed), 
				      (gpointer) gtk_object_get_data (GTK_OBJECT (e), "gconf_key")); 
  

  /* Local User Name */
  if ((e == pw->firstname)||(e == pw->surname)) {
    
    /* Set the local User name */
    gchar *firstname =
      gconf_client_get_string (client, 
			       "/apps/gnomemeeting/personal_data/firstname",
			       0);
    gchar *lastname =
      gconf_client_get_string (client, 
			       "/apps/gnomemeeting/personal_data/lastname", 
			       0);
      
    if ((firstname) && (lastname)) {
      
      gchar *local_name = g_strdup ("");
      local_name = g_strconcat (local_name, firstname, " ", lastname, NULL);
      
      MyApp->Endpoint ()->SetLocalUserName (local_name);
      g_free (local_name);
      g_free (firstname);
      g_free (lastname);
    }
  }
  
  gdk_threads_leave ();

  return FALSE;
}


/* DESCRIPTION  :  Generic notifiers for entries.
 *                 This callback is called when a specific key of
 *                 the gconf database associated with an entry changes.
 * BEHAVIOR     :  It updates the widget and the endpoint user name if the
 *                 entry was related to it!
 * PRE          :  /
 */
static void entry_changed_nt (GConfClient *client, guint cid, 
			      GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_STRING) {
   
    g_idle_add (entry_changed_, data);
  }
}


static gboolean video_option_menu_changed (gpointer data)
{
  gdk_threads_enter ();

  GMVideoGrabber *vg = NULL;
  GConfClient *client = gconf_client_get_default ();
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);
  GtkWidget *e = GTK_OPTION_MENU (data)->menu;
  
  /* We set the new value for the widget */
  gtk_signal_handler_block_by_func (GTK_OBJECT (e),
				    GTK_SIGNAL_FUNC (option_menu_changed), 
				    (gpointer) gtk_object_get_data (GTK_OBJECT (data), "gconf_key")); 
  /* Can't be done before Gnome 2 
  gtk_option_menu_set_history (GTK_OPTION_MENU (data),
			       gconf_value_get_int (entry->value));
  */
  gtk_signal_handler_unblock_by_func (GTK_OBJECT (e),
				      GTK_SIGNAL_FUNC (option_menu_changed), 
				      (gpointer) gtk_object_get_data (GTK_OBJECT (data), "gconf_key")); 


  /* We update the Endpoint */
  MyApp->Endpoint ()->RemoveAllCapabilities ();
  MyApp->Endpoint ()->AddAudioCapabilities ();
  
  if (GTK_WIDGET (data) == pw->opt1)
    MyApp->Endpoint ()
      ->AddVideoCapabilities (gconf_client_get_int (client, "/apps/gnomemeeting/devices/video_size", NULL));

  if (MyApp->Endpoint ()->GetCallingState () == 0) {
    
    vg = MyApp->Endpoint ()->GetVideoGrabber ();
    
    if (vg)
      vg->Reset ();
  }
  else {
    
    gnomemeeting_warning_popup (GTK_WIDGET (e),
				_("This change will only affect new calls."));
    
  }            


  gdk_threads_leave ();

  return FALSE;
}


/* DESCRIPTION  :  This notifier is called when a video option_menu is changed.
 * BEHAVIOR     :  It updates the widget and Reset the video if we are not
 *                 in a call. If we are in a call, display a popup.
 * PRE          :  /
 */
static void video_option_menu_changed_nt (GConfClient *client, guint cid, 
					  GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_INT) {

    g_idle_add (video_option_menu_changed, data);
  }
}


/* Is Not able to modify the widgets */
static gboolean audio_mixer_changed (gpointer data)
{
/*  GtkWidget *e = GTK_WIDGET (data);*/
  GM_pref_window_widgets *pw = NULL;
  GM_window_widgets *gw = NULL;
  int vol_play = 0, vol_rec = 0;
  gchar *player = NULL, *recorder = NULL;
  gchar *player_mixer = NULL, *recorder_mixer = NULL;
  gchar *text = NULL;
  gchar *entry_data = NULL;
  GConfClient *client = gconf_client_get_default ();

  gdk_threads_enter ();
  
  pw = gnomemeeting_get_pref_window (gm);
  gw = gnomemeeting_get_main_window (gm);

  entry_data = gtk_entry_get_text (GTK_ENTRY (data));
  
  /* Update the GUI */
  /* Not before Gnome 2 :-(
  gtk_signal_handler_block_by_func (GTK_OBJECT (e),
				    GTK_SIGNAL_FUNC (entry_changed), 
				    (gpointer) gtk_object_get_data (GTK_OBJECT (e), "gconf_key")); 
  gtk_entry_set_text (GTK_ENTRY (e), (gchar *) data);
  gtk_signal_handler_unblock_by_func (GTK_OBJECT (e),
				      GTK_SIGNAL_FUNC (entry_changed), 
				      (gpointer) gtk_object_get_data (GTK_OBJECT (e), "gconf_key")); 
  */

  /* We need to test if the string entry can be valid 
     FIX ME: We could find a way to not hardcode those values 
  */

  if ((!strcmp ((char *) entry_data, "/dev/mixer"))
      || (!strcmp ((char *) entry_data, "/dev/mixer3"))
      || (!strcmp ((char *) entry_data, "/dev/mixer2"))
      || (!strcmp ((char *) entry_data, "/dev/mixer1"))
      || (!strcmp ((char *) entry_data, "/dev/mixer0"))) {
    
    player_mixer = gconf_client_get_string (client, "/apps/gnomemeeting/devices/audio_player_mixer", NULL);
    recorder_mixer = gconf_client_get_string (client, "/apps/gnomemeeting/devices/audio_recorder_mixer", NULL);

    player = gconf_client_get_string (client, "/apps/gnomemeeting/devices/audio_player", NULL);
    recorder = gconf_client_get_string (client, "/apps/gnomemeeting/devices/audio_recorder", NULL);

    /* Get the volumes for the mixers */
    gnomemeeting_volume_get (player_mixer, 0, &vol_play);
    gnomemeeting_volume_get (recorder_mixer, 1, &vol_rec);
    
    gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_play),
			      (int) (vol_play / 257));
    gtk_adjustment_set_value (GTK_ADJUSTMENT (gw->adj_rec),
			      (int) (vol_rec / 257));
    
    /* Set recording source and set micro to record */
    /* Translators: This is shown in the history. */
    text = g_strdup_printf (_("Set Audio Mixer for player to %s"),
			    player_mixer);
    gnomemeeting_log_insert (text);
    g_free (text);
      
    /* Translators: This is shown in the history. */
    text = g_strdup_printf (_("Set Audio Mixer for recorder to %s"),
			      recorder_mixer);
    gnomemeeting_log_insert (text);
    g_free (text);
      
    gnomemeeting_set_recording_source (recorder_mixer, 0);
    
    g_free (player);
    g_free (recorder);
    g_free (player_mixer);
    g_free (recorder_mixer);
  }
  else {
    
    gchar *msg = g_strdup_printf (_("%s doesn't seem to be a valid mixer, please fix this!"), (char *) entry_data);
    GtkWidget *msg_box = gnome_message_box_new (msg, GNOME_MESSAGE_BOX_ERROR, 
						"OK", NULL);
    gtk_widget_show (msg_box);
    
    g_free (msg);
  }
  
  gdk_threads_leave ();

  return FALSE;
}


/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the mixers changes.
 * BEHAVIOR     :  It updates the widget, updates the sliders, if the user
 *                 erroneously used gconftool to put a wrong value in the entry
 *                 a popup will be displayed and the widget updated to the wrong
 *                 value, but there will be no other action.
 * PRE          :  /
 */
static void audio_mixer_changed_nt (GConfClient *client, guint cid, 
				    GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_STRING) {
  
    g_idle_add (audio_mixer_changed, 
		(gpointer) data);
  }
}


/* Not able to update the widgets*/
static gboolean audio_device_changed (gpointer data)
{
  gchar *text;
  gchar *player = NULL, *recorder = NULL;
  gchar *entry_data = NULL;
  int found = 0;
/*  GtkWidget *e = GTK_WIDGET (data);*/
  GM_window_widgets *gw = NULL;
  GM_pref_window_widgets *pw = NULL;
  GConfClient *client = gconf_client_get_default ();

  gdk_threads_enter ();
  
  gw = gnomemeeting_get_main_window (gm);
  pw = gnomemeeting_get_pref_window (gm);

  /* Update the GUI */
  /* Not possible before Gnome 2
  gtk_signal_handler_block_by_func (GTK_OBJECT (GTK_COMBO (e)->entry),
				      GTK_SIGNAL_FUNC (entry_changed), 
				      (gpointer) gtk_object_get_data (GTK_OBJECT (e), "gconf_key")); 
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (e)->entry), 
  gconf_value_get_string (entry->value));
  gtk_signal_handler_unblock_by_func (GTK_OBJECT (GTK_COMBO (e)->entry),
  GTK_SIGNAL_FUNC (entry_changed), 
  (gpointer) gtk_object_get_data (GTK_OBJECT (e), "gconf_key")); 
  */
  
  entry_data = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (data)->entry));

  /* Is the choosen device detected? */
  for (int i = gw->audio_player_devices.GetSize () - 1; i >= 0; i--) {

    if (data == pw->audio_player) {

      if (!strcmp (entry_data, gw->audio_player_devices [i]))
	found = 1;
    }
    else {

      if (!strcmp (entry_data, gw->audio_recorder_devices [i]))
	found = 1;
    }
  }

  if (found) {
    
    player = gconf_client_get_string (client, "/apps/gnomemeeting/devices/audio_player", NULL);
    recorder = gconf_client_get_string (client, "/apps/gnomemeeting/devices/audio_recorder", NULL);
      
    /* Set recording source and set micro to record */
    if (player != NULL)
      MyApp->Endpoint()->SetSoundChannelPlayDevice (player);
    if (recorder != NULL)
      MyApp->Endpoint()->SetSoundChannelRecordDevice (recorder);
      
    /* Translators: This is shown in the history. */
    text = g_strdup_printf (_("Set Audio Player to %s"),
			    player);
    gnomemeeting_log_insert (text);
    g_free (text);
    
    /* Translators: This is shown in the history. */
    text = g_strdup_printf (_("Set Audio Recorder to %s"),
			    recorder);
    gnomemeeting_log_insert (text);
    g_free (text);
    g_free (player);
    g_free (recorder);
  }
  else {
    
    gchar *msg = g_strdup_printf (_("%s doesn't seem to be a valid device, please fix this!"), entry_data);
    GtkWidget *msg_box = gnome_message_box_new (msg, GNOME_MESSAGE_BOX_ERROR, 
						"OK", NULL);
    gtk_widget_show (msg_box);
    
    g_free (msg);
  }
  
  gdk_threads_leave ();
  
  return FALSE;
}


/* DESCRIPTION  :  This notifier is called when the gconf database data
 *                 associated with the audio devices changes.
 * BEHAVIOR     :  It updates the widget, updates the endpoint and displays
 *                 a message in the history. If the device is not valid,
 *                 i.e. the user erroneously used gconftool, a message is
 *                 displayed, and the entry is updated, but no other action.
 * PRE          :  /
 */
static void audio_device_changed_nt (GConfClient *client, guint cid, 
				     GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_STRING) {

    g_idle_add (audio_device_changed, data);
  }
}


/* Not able to update widgets */
static gboolean video_device_changed (gpointer data)
{
  GMVideoGrabber *vg = NULL;
  GtkWidget *e = GTK_WIDGET (data);

  gdk_threads_enter ();
       
  /* We set the new value for the widget */
  /* No possible before Gnome2 
  gtk_signal_handler_block_by_func (GTK_OBJECT (GTK_COMBO (e)->entry),
				    GTK_SIGNAL_FUNC (entry_changed), 
				    (gpointer) gtk_object_get_data (GTK_OBJECT (e), "gconf_key")); 
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (e)->entry), 
		      gconf_value_get_string (entry->value));
  gtk_signal_handler_unblock_by_func (GTK_OBJECT (GTK_COMBO (e)->entry),
				      GTK_SIGNAL_FUNC (entry_changed), 
				      (gpointer) gtk_object_get_data (GTK_OBJECT (e), "gconf_key")); 
  */

  /* We reset the video device */
  if (MyApp->Endpoint ()->GetCallingState () == 0) {
    
    vg = MyApp->Endpoint ()->GetVideoGrabber ();

    if (vg)
      vg->Reset ();
  }
  else {
    
    gnomemeeting_warning_popup (e, 
				_("This change will only affect new calls"));
  }            
  
  gdk_threads_leave ();

  return FALSE;
}


/* DESCRIPTION  :  This callback is called when the video device changes in
 *                 the gconf database.
 * BEHAVIOR     :  It updates the widget and resets the video device.
 * PRE          :  /
 */
static void video_device_changed_nt (GConfClient *client, guint cid, 
				     GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_STRING) {
  
    g_idle_add (video_device_changed, data);
  }
}


static gboolean video_channel_changed (gpointer data)
{
  GMVideoGrabber *vg = NULL;
  GM_pref_window_widgets *pw = NULL;
  GtkWidget *e = GTK_WIDGET (data);

  gdk_threads_enter ();

  pw = gnomemeeting_get_pref_window (gm);

  /* We set the new value for the widget */
  /* Not able to do this with Gnome 1 :(
     gtk_signal_handler_block_by_func (GTK_OBJECT (pw->video_channel_spin_adj),
				      GTK_SIGNAL_FUNC (adjustment_changed), 
				      (gpointer) gtk_object_get_data (GTK_OBJECT (pw->video_channel_spin_adj), "gconf_key")); 
    gtk_adjustment_set_value (GTK_ADJUSTMENT (pw->video_channel_spin_adj),
			      (gfloat) gconf_value_get_int (entry->value));
    gtk_signal_handler_unblock_by_func (GTK_OBJECT (pw->video_channel_spin_adj),
					GTK_SIGNAL_FUNC (adjustment_changed), 
					(gpointer) gtk_object_get_data (GTK_OBJECT (pw->video_channel_spin_adj), "gconf_key")); 
   */

  /* We reset the video device */
  if (MyApp->Endpoint ()->GetCallingState () == 0) {
    
    vg = MyApp->Endpoint ()->GetVideoGrabber ();
    
    if (vg) 
      vg->Reset ();
  }
  else {

    gnomemeeting_warning_popup (e,
				_("This change will only affect new calls.")); 
  }
 
  gdk_threads_leave ();

  return FALSE;
}


/* DESCRIPTION  :  This callback is called when the video channel changes in
 *                 the gconf database.
 * BEHAVIOR     :  It updates the widget and the video device, if not in a 
 *                 all, or displays a popup.
 * PRE          :  /
 */
static void video_channel_changed_nt (GConfClient *client, guint cid, 
				      GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_INT) {
   
    g_idle_add (video_channel_changed, data);
  }
}


static gboolean video_preview_changed (gpointer data)
{
  gdk_threads_enter ();

  GMVideoGrabber *vg = NULL;
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);
     
  /* We set the new value for the widget */
  GTK_TOGGLE_BUTTON (pw->video_preview)->active = (bool) data;
  GTK_TOGGLE_BUTTON (gw->preview_button)->active = (bool) data;

  gtk_widget_draw (GTK_WIDGET (pw->video_preview), NULL);
  gtk_widget_draw (GTK_WIDGET (gw->preview_button), NULL);
    
  /* We reset the video device */
  if (MyApp->Endpoint ()->GetCallingState () == 0) {
    
    vg = MyApp->Endpoint ()->GetVideoGrabber ();
    
    if ((bool) data) {

      if (!vg->IsOpened ())
	vg->Open (TRUE);
    }
    else {
      
      if (vg->IsOpened ())
	vg->Close ();
    }
  }
  else {
    
    gchar *msg = g_strdup (_("Preview can't be changed during calls. Changes will take effect after this call."));
    GtkWidget *msg_box = gnome_message_box_new (msg, GNOME_MESSAGE_BOX_WARNING, 
						"OK", NULL);
    gtk_widget_show (msg_box);
    
    g_free (msg);
  }
 
  gdk_threads_leave ();
  return FALSE;
}


/* DESCRIPTION  :  This callback is called when the video channel changes in
 *                 the gconf database.
 * BEHAVIOR     :  It updates the widgets and enables preview, if not in a call,
 *                 or displays a popup.
 * PRE          :  /
 */
static void video_preview_changed_nt (GConfClient *client, guint cid, 
				      GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_BOOL) {
   
    g_idle_add (video_preview_changed, 
		(gpointer) gconf_value_get_bool (entry->value));
  }
}


/* DESCRIPTION  :  his callback is called when a specific key of
 *                 the gconf database associated with that toggle changes.
 * BEHAVIOR     :  It only updates the widget, and SetFrameRate to 0 to disable it
 * PRE          :  /
 */
static void enable_fps_changed_nt (GConfClient *client, guint cid, 
				   GConfEntry *entry, gpointer data)
{
  GtkWidget *toggle = GTK_WIDGET (data);
  GMVideoGrabber *vg = NULL;

  if (entry->value->type == GCONF_VALUE_BOOL) {
   
    /* We set the new value for the widget */
    GTK_TOGGLE_BUTTON (toggle)->active = gconf_value_get_bool (entry->value);
    gtk_widget_draw (GTK_WIDGET (toggle), NULL);

    /* Update the value */
    vg = MyApp->Endpoint ()->GetVideoGrabber ();

    if (vg) {
      
      /* Disable or enable tr fps limit */
      if (!gconf_value_get_bool (entry->value)) 
	vg->SetFrameRate (0);
      else
	vg->SetFrameRate (gconf_client_get_int (client, "/apps/gnomemeeting/video_settings/tr_fps", NULL));
    }
  }
}



/* DESCRIPTION  :  This callback is called when the specific key of
 *                 the gconf database associated with the video bw changes.
 * BEHAVIOR     :  It only updates the widget, and SetVB to 0 to disable it
 *                 or enable it.
 * PRE          :  /
 */
static void enable_vb_changed_nt (GConfClient *client, guint cid, 
				  GConfEntry *entry, gpointer data)
{
  GtkWidget *toggle = GTK_WIDGET (data);
  H323VideoCodec *vc = NULL;

  if (entry->value->type == GCONF_VALUE_BOOL) {
   
    /* We set the new value for the widget */
    GTK_TOGGLE_BUTTON (toggle)->active = gconf_value_get_bool (entry->value);
    gtk_widget_draw (GTK_WIDGET (toggle), NULL);

    /* Update the value */
    vc = MyApp->Endpoint ()->GetCurrentVideoCodec ();

    if (vc) {
      
      /* Disable or enable bandwidth limit */
      if (!gconf_value_get_bool (entry->value)) 
	vc->SetAverageBitRate (0);
      else
	vc->SetAverageBitRate (gconf_client_get_int (client, "/apps/gnomemeeting/video_settings/video_bandwidth", NULL) * 1024 * 8);
    }
  }
}


/* Able to update widgets */
static gboolean enable_vid_tr_changed (gpointer data)
{
  gdk_threads_enter ();

  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  /* We set the new value for the widget */
  GTK_TOGGLE_BUTTON (pw->vid_tr)->active = (bool) data;
  gtk_widget_draw (GTK_WIDGET (pw->vid_tr), NULL);
    
  /* We reset the video device */
  if (MyApp->Endpoint ()->GetCallingState () == 2) {
     
    gchar *msg = g_strdup (_("Video transmission can't be changed during calls. Changes will take effect after this call."));
    GtkWidget *msg_box = gnome_message_box_new (msg, GNOME_MESSAGE_BOX_WARNING, 
						"OK", NULL);
    gtk_widget_show (msg_box);
    
    g_free (msg);
  }
  
  gdk_threads_leave ();

  return FALSE;
}


/* DESCRIPTION  :  This callback is called when the video transmission toggle 
 *                 changes in the gconf database.
 * BEHAVIOR     :  It updates the widgets and enables vid tr, if not in a call,
 *                 or displays a popup.
 * PRE          :  /
 */
static void enable_vid_tr_changed_nt (GConfClient *client, guint cid, 
				      GConfEntry *entry, gpointer data)
{  
  if (entry->value->type == GCONF_VALUE_BOOL) {
   
    g_idle_add (enable_vid_tr_changed, 
		(gpointer) gconf_value_get_bool (entry->value));
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
  if (entry->value->type == GCONF_VALUE_STRING) {
   
    /* We set the new value for the widget */
    gchar **codecs;
    codecs = g_strsplit (gconf_value_get_string (entry->value), ":", 0);

    gtk_clist_freeze (GTK_CLIST (data));
    gtk_clist_clear (GTK_CLIST (data));
   
    for (int i = 0 ; codecs [i] != NULL ; i++) {

      gchar **couple = g_strsplit (codecs [i], "=", 0);
      gnomemeeting_codecs_list_add (GTK_WIDGET (data), couple [0], couple [1]);
      g_strfreev (couple);
    }

    gtk_clist_thaw (GTK_CLIST (data));
    g_strfreev (codecs);

    /* We update the capabilities */
    MyApp->Endpoint ()->RemoveAllCapabilities ();
    MyApp->Endpoint ()->AddAudioCapabilities ();
    MyApp->Endpoint ()->AddVideoCapabilities (gconf_client_get_int (client, "/apps/gnomemeeting/devices/video_size", NULL));
  }
}


/* Not able to change all widgets */
static gboolean view_widget_changed (gpointer data)
{
  gdk_threads_enter ();
  
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  GnomeUIInfo *view_menu_uiinfo =
    (GnomeUIInfo *) gtk_object_get_data (GTK_OBJECT (gm), "view_menu_uiinfo");
   
  /* We set the new value for the widget */
  /* We are Unable to do it before Gnome 2 :-(
  GTK_TOGGLE_BUTTON (data)->active = 
    gconf_value_get_bool (entry->value);
  gtk_widget_draw (GTK_WIDGET (data), NULL);
  */

  /* We show or hide the corresponding widget */
  if (data == pw->show_notebook) {

    /* Update the menu, 
       we are called only if the value has changed */
    GTK_CHECK_MENU_ITEM (view_menu_uiinfo [2].widget)->active = 
      !GTK_WIDGET_VISIBLE (gw->main_notebook);

    GTK_TOGGLE_BUTTON (pw->show_notebook)->active =
      !GTK_WIDGET_VISIBLE (gw->main_notebook);

    gtk_widget_draw (view_menu_uiinfo [2].widget, NULL);
    gtk_widget_draw (pw->show_notebook, NULL);

    if (!GTK_WIDGET_VISIBLE (gw->main_notebook))
      gtk_widget_show_all (gw->main_notebook);
    else
      gtk_widget_hide (gw->main_notebook);
  }
  
  if (data == pw->show_statusbar) {
    
    /* Update the menu,
       we are called only if the value has changed */
    GTK_CHECK_MENU_ITEM (view_menu_uiinfo [3].widget)->active = 
      !GTK_WIDGET_VISIBLE (gw->statusbar);

    GTK_TOGGLE_BUTTON (pw->show_statusbar)->active = 
      !GTK_WIDGET_VISIBLE (gw->statusbar);

    gtk_widget_draw (view_menu_uiinfo [3].widget, NULL);
    gtk_widget_draw (pw->show_statusbar, NULL);

    if (!GTK_WIDGET_VISIBLE (gw->statusbar))
      gtk_widget_show_all (gw->statusbar);
    else
      gtk_widget_hide (gw->statusbar);
  }

  if (data == pw->show_quickbar) {

    /* Update the menu,
       we are called only if the value has changed */
    GTK_CHECK_MENU_ITEM (view_menu_uiinfo [4].widget)->active = 
      !GTK_WIDGET_VISIBLE (gw->quickbar_frame);

    GTK_TOGGLE_BUTTON (pw->show_quickbar)->active =
      !GTK_WIDGET_VISIBLE (gw->quickbar_frame);
    
    gtk_widget_draw (pw->show_quickbar, NULL);
    gtk_widget_draw (view_menu_uiinfo [4].widget, NULL);

    if (!GTK_WIDGET_VISIBLE (gw->quickbar_frame))
      gtk_widget_show_all (gw->quickbar_frame);
    else
      gtk_widget_hide_all (gw->quickbar_frame);
  }

  if (data == pw->show_docklet) {
    
    /* Update the menu,
       we are called only if the value has changed */
    GTK_CHECK_MENU_ITEM (view_menu_uiinfo [5].widget)->active = 
      !GTK_WIDGET_VISIBLE (gw->docklet);

    GTK_TOGGLE_BUTTON (pw->show_docklet)->active =
      !GTK_WIDGET_VISIBLE (gw->docklet);

    gtk_widget_draw (pw->show_docklet, NULL);
    gtk_widget_draw (view_menu_uiinfo [5].widget, NULL);

    if (!GTK_WIDGET_VISIBLE (gw->docklet))
      gtk_widget_show (gw->docklet);
    else
      gtk_widget_hide (gw->docklet);
  }

  gdk_threads_leave ();

  return FALSE;
}


/* DESCRIPTION  :  This callback is called when something changes in the view
 *                 directory (either from the menu, either from the prefs).
 * BEHAVIOR     :  It updates the widget, menu and shows/hides the 
 *                 corresponding widget.
 * PRE          :  /
 */
static void view_widget_changed_nt (GConfClient *client, guint cid, 
				    GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_BOOL) {
   
    g_idle_add (view_widget_changed, data);
  }
}


/* Able to update widgets */
static gboolean register_changed (gpointer data)
{
  BOOL no_error = TRUE;
  gchar *gconf_string;
  GtkWidget *msg_box = NULL;
  GMH323EndPoint *endpoint = MyApp->Endpoint ();
  GConfClient *client = gconf_client_get_default ();

  gdk_threads_enter ();
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  /* Update the widgets */  
  GTK_TOGGLE_BUTTON (pw->ldap)->active = (bool) data;
  gtk_widget_set_sensitive (GTK_WIDGET (pw->directory_update_button), 
			    (bool) data);

  /* We check that all the needed information is available
     to update the LDAP directory */
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->ldap))) {
    
    /* Checks if the server name is ok */
    gconf_string =  gconf_client_get_string (GCONF_CLIENT (client),
					     "/apps/gnomemeeting/ldap/ldap_server", 
					     NULL);

    if ((gconf_string == NULL) || (!strcmp (gconf_string, ""))) {

      msg_box = gnome_message_box_new (_("Not registering because there is no LDAP server specified!"), GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
      no_error = FALSE;
    }
    g_free (gconf_string);


    /* Check if there is a first name */
    gconf_string =  gconf_client_get_string (GCONF_CLIENT (client),
					     "/apps/gnomemeeting/personal_data/firstname", NULL);

    if ((gconf_string == NULL) || (!strcmp (gconf_string, ""))) {
      
      msg_box = gnome_message_box_new (_("Not Registering: Please provide your first name!"), GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
      no_error = FALSE;
    }
    g_free (gconf_string);


    /* Check if there is a mail */
    gconf_string =  gconf_client_get_string (GCONF_CLIENT (client),
					     "/apps/gnomemeeting/personal_data/mail", NULL);

    if ((gconf_string == NULL) || (!strcmp (gconf_string, ""))) {
      
      msg_box = gnome_message_box_new (_("Not Registering: Please provide a valid e-mail!"), GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
      no_error = FALSE;
    }
    g_free (gconf_string);
    

    /* Display the popup in case of error, and update the widget, and 
       the gconf value for the register field */
    if (no_error == FALSE) {
      
      gtk_widget_show (msg_box);
    }
  }


  if (no_error) {
    
    int registering = gconf_client_get_bool (GCONF_CLIENT (client),
					     "/apps/gnomemeeting/ldap/register", 
					     NULL);
    GMILSClient *ils_client = (GMILSClient *) endpoint->GetILSClient ();
      
    if (registering) {
      
      ils_client->Register ();
    }
    else {
      
      ils_client->Unregister ();
    }
  }

  gdk_threads_leave ();

  return FALSE;
}


/* DESCRIPTION  :  This callback is called when the "register" gconf value 
 *                 changes.
 *                 The "register" value can change if the user plays with 
 *                 the button, or if he clicks on "Update" in Personal data, 
 *                 or if he uses gconftool.
 * BEHAVIOR     :  It registers to the LDAP server, IF all recquired fields are
 *                 available.
 * PRE          :  /
 */
static void register_changed_nt (GConfClient *client, guint cid, 
				 GConfEntry *entry, gpointer data)
{
  if (entry->value->type == GCONF_VALUE_BOOL) {

    g_idle_add (register_changed, (gpointer) gconf_value_get_bool (entry->value));
		
  }
}


/* Is able to update the widgets */
static gboolean notebook_info_changed (gpointer data)
{
 gdk_threads_enter ();
 
 GnomeUIInfo *notebook_view_uiinfo =
   (GnomeUIInfo *) gtk_object_get_data (GTK_OBJECT (gm), 
					"notebook_view_uiinfo");
 GM_window_widgets *gw = gnomemeeting_get_main_window (gm);
 
 int current_page = (int) data;

 if (current_page < 0 || current_page > 3)
   return FALSE;

 gtk_signal_handler_block_by_data (GTK_OBJECT (gw->main_notebook), 
				   gw->main_notebook);
 gtk_notebook_set_page (GTK_NOTEBOOK (gw->main_notebook),
			current_page);
 gtk_signal_handler_unblock_by_data (GTK_OBJECT (gw->main_notebook),
				     gw->main_notebook);

 gtk_widget_set_sensitive (GTK_WIDGET (gw->left_arrow), true);
 gtk_widget_set_sensitive (GTK_WIDGET (gw->right_arrow), true);
 
 if (current_page == 0)
   gtk_widget_set_sensitive (GTK_WIDGET (gw->left_arrow), false);
 else if (current_page == 3) 
   gtk_widget_set_sensitive (GTK_WIDGET (gw->right_arrow), false);

 for (int i = 0; i < 4; i++) {

   GTK_CHECK_MENU_ITEM (notebook_view_uiinfo[i].widget)->active =
     (current_page == i);
   gtk_widget_draw (GTK_WIDGET (GTK_CHECK_MENU_ITEM (notebook_view_uiinfo[i].widget)), NULL);
 }

 gdk_threads_leave ();
 
 return FALSE;
}


/* DESCRIPTION  :  This callback is called when something toggles the
 *                 corresponding option in gconf.
 * BEHAVIOR     :  Toggles the menu corresponding
 * PRE          :  gpointer is a valid pointer to the menu
 *                 structure.
 */
static void notebook_info_changed_nt (GConfClient *client, guint, 
				      GConfEntry *entry, 
				      gpointer user_data)
{
  if (entry->value->type == GCONF_VALUE_INT) {

    g_idle_add (notebook_info_changed, 
		(gpointer) gconf_value_get_int (entry->value));
  }
}

#if 0 /* Uncomment when we are under Gnome2 */

/* DESCRIPTION  :  This callback is called when something toggles the
 *                 corresponding option in gconf.
 * BEHAVIOR     :  Updated the combo strings
 * PRE          :  gpointer is a valid pointer to the combo
 */
static void history_changed_nt (GConfClient *client, guint, GConfEntry *entry, 
				gpointer user_data)
{
  GtkCombo *combo = GTK_COMBO (user_data);
  GList *hosts = 0;
  gchar **contacts;
  gchar *old_entry;

  if (entry->value->type != GCONF_VALUE_STRING)
    return;

  old_entry = gtk_editable_get_chars (GTK_EDITABLE (combo->entry), 0, -1);

  const gchar *new_hosts = gconf_value_get_string (entry->value);
  contacts = g_strsplit (new_hosts, ":", 0);
  for (int i = 0; contacts[i] != 0; i++)
    hosts = g_list_prepend (hosts, contacts[i]);

  gtk_object_remove_data (GTK_OBJECT (combo), "history");

  /* This is just needed if hosts in null */
  gtk_list_clear_items (GTK_LIST (combo->list), 0, -1);

  gtk_combo_set_popdown_strings (combo, hosts);
  if (hosts != 0)
    gtk_object_set_data_full (GTK_OBJECT (combo), "history", hosts,
			      gnomemeeting_freeg_list_data);

  /* Restore the previous value typed in the entry field */
  gtk_entry_set_text (GTK_ENTRY (combo->entry), old_entry);

  g_free (contacts);
  g_free (old_entry);
}
#endif

/* The functions  */
void gnomemeeting_init_gconf (GConfClient *client)
{
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_fps",
			   fps_limit_changed_nt, pw, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/video_bandwidth", vb_limit_changed_nt, pw, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_vq",
			   tr_vq_changed_nt, pw, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/re_vq",
			   re_vq_changed_nt, pw, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_settings/jitter_buffer", jitter_buffer_changed_nt, pw, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_ub",
			   tr_ub_changed_nt, pw, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/ldap/register",
			   register_changed_nt, pw, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/personal_data/firstname",
			   entry_changed_nt, pw->firstname, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/personal_data/mail",
			   entry_changed_nt, pw->mail, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/personal_data/lastname",
			   entry_changed_nt, pw->surname, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/personal_data/location",
			   entry_changed_nt, pw->location, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/personal_data/comment",
			   entry_changed_nt, pw->comment, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/general/auto_answer",
			   toggle_changed_nt, pw->aa, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/general/do_not_disturb",
			   toggle_changed_nt, pw->dnd, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/general/h245_tunneling",
			   toggle_changed_nt, pw->ht, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/general/fast_start",
			   toggle_changed_nt, pw->fs, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/general/incoming_call_sound", toggle_changed_nt, pw->incoming_call_sound, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/view/show_popup", toggle_changed_nt, pw->incoming_call_popup, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_player", audio_device_changed_nt, pw->audio_player, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_recorder", audio_device_changed_nt, pw->audio_recorder, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_player_mixer", audio_mixer_changed_nt, pw->audio_player_mixer, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/audio_recorder_mixer", audio_mixer_changed_nt, pw->audio_recorder_mixer, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_size", video_option_menu_changed_nt, pw->opt1, 0, 0);			   

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_format", video_option_menu_changed_nt, pw->opt2, 0, 0);			   

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_recorder", video_device_changed_nt, pw->video_device, 0, 0);			   

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_channel", video_channel_changed_nt, pw->video_channel, 0, 0);			   

  gconf_client_notify_add (client, "/apps/gnomemeeting/devices/video_preview", video_preview_changed_nt, pw, 0, 0);			   
  
  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/enable_fps", enable_fps_changed_nt, pw->fps, 0, 0);			     

  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/enable_vb", enable_vb_changed_nt, pw->vb, 0, 0);			     

  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/enable_video_transmission", enable_vid_tr_changed_nt, pw->vid_tr, 0, 0);	     

  gconf_client_notify_add (client, "/apps/gnomemeeting/audio_codecs/list", audio_codecs_list_changed_nt, pw->clist_avail, 0, 0);	     

  gconf_client_notify_add (client, "/apps/gnomemeeting/view/show_splash", 
			   toggle_changed_nt, pw->show_splash, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/view/show_control_panel", view_widget_changed_nt, pw->show_notebook, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/view/show_status_bar", view_widget_changed_nt, pw->show_statusbar, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/view/show_quick_bar", view_widget_changed_nt, pw->show_quickbar, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/view/show_docklet", view_widget_changed_nt, pw->show_docklet, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/view/notebook_info", notebook_info_changed_nt, NULL, 0, 0);

#if 0 /* FIXME: Uncomment when we are under GNOME2*/
  gconf_client_notify_add (client, "/apps/gnomemeeting/history/called_hosts", history_changed_nt, gw->combo, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/history/ldap_servers", history_changed_nt, lw->ils_server_combo, 0, 0);
#endif 

  gnomemeeting_update_pref_window_sensitivity ();
}


void entry_changed (GtkEditable  *e, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();
  gchar *key = (gchar *) data;

  gconf_client_set_string (GCONF_CLIENT (client),
                           key,
                           gtk_entry_get_text (GTK_ENTRY (e)),
                           NULL);

  gnomemeeting_update_pref_window_sensitivity ();
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


void option_menu_changed (GtkWidget *menu, gpointer data)
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


/* DESCRIPTION  :  /                                                          
 * BEHAVIOR     :  It updates the sensitivity of the pw->ldap toggle following
 *                 if all recquired values are present or not.                 
 * PRE          :  data is the gconf key                                      
 */
static void gnomemeeting_update_pref_window_sensitivity ()
{
  gchar *gconf_string = NULL;
  BOOL no_error = TRUE;

  /* Get interesting data */
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);
  GConfClient *client = gconf_client_get_default ();

  /* Checks if the server name is ok */
  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client),
                                           "/apps/gnomemeeting/ldap/ldap_server\
",
                                           NULL);

  if ((gconf_string == NULL) || (!strcmp (gconf_string, "")))
    no_error = FALSE;

  g_free (gconf_string);

  /* Check if there is a first name */
  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client),
                                           "/apps/gnomemeeting/personal_data/f\
irstname",
                                           NULL);

  if ((gconf_string == NULL) || (!strcmp (gconf_string, "")))
    no_error = FALSE;

  g_free (gconf_string);

  /* Check if there is a mail */
  gconf_string =  gconf_client_get_string (GCONF_CLIENT (client),
                                           "/apps/gnomemeeting/personal_data/m\
ail",
                                           NULL);

  if ((gconf_string == NULL) || (!strcmp (gconf_string, "")))
    no_error = FALSE;

  g_free (gconf_string);

  if (no_error) {

    gtk_widget_set_sensitive (GTK_WIDGET (pw->ldap), TRUE);
    /* Make the update button sensitive only if the register button is sensitiv\
       e too */
    if (gconf_client_get_bool (GCONF_CLIENT (client),
                               "/apps/gnomemeeting/ldap/register", 0))
      gtk_widget_set_sensitive (GTK_WIDGET (pw->directory_update_button), TRUE)\
	;
    else
      gtk_widget_set_sensitive (GTK_WIDGET (pw->directory_update_button), FALSE\
				);
  }
  else {

    gtk_widget_set_sensitive (GTK_WIDGET (pw->ldap), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (pw->directory_update_button), FALSE);
  }
}

