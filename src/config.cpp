
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
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : Functions to store the config options.
 *   email                : dsandras@seconix.com
 *
 */


#include "config.h"
#include "common.h"
#include "audio.h"
#include "videograbber.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "ils.h"

#include "../config.h"


/* Declarations */
extern GtkWidget *gm;
extern GnomeMeeting *MyApp;

static void fps_limit_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void toggle_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void entry_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void tr_vq_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void tr_ub_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);
static void register_changed_nt (GConfClient*, guint, GConfEntry *, gpointer);


/* DESCRIPTION  :  This callback is called to update the fps limitation.
 * BEHAVIOR     :  Update it.
 * PRE          :  /
 */
static void fps_limit_changed_nt (GConfClient *client, guint cid, 
				  GConfEntry *entry, gpointer data)
{
  GMVideoGrabber *vg = NULL;
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  if (entry->value->type == GCONF_VALUE_INT) {
   
    /* We set the new value for tr_fps_spin_adj */
    GTK_ADJUSTMENT (pw->tr_fps_spin_adj)->value = 
      gconf_value_get_int (entry->value);

    /* We update the current frame rate */
    vg = MyApp->Endpoint ()->GetVideoGrabber ();

    if (vg != NULL)
      vg->SetFrameRate ((int) gconf_value_get_int (entry->value));
  }
}


/* DESCRIPTION  :  This callback is called the transmitted video quality.
 * BEHAVIOR     :  It updates the video quality.
 * PRE          :  data is a pointer to the tr_fps_spin_adj widget
 */
static void tr_vq_changed_nt (GConfClient *client, guint cid, 
			      GConfEntry *entry, gpointer data)
{
  H323VideoCodec *vc = NULL;
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  if (entry->value->type == GCONF_VALUE_INT) {
   
    /* We set the new value for tr_fps_spin_adj */
    GTK_ADJUSTMENT (pw->tr_vq_spin_adj)->value = 
      gconf_value_get_int (entry->value);

    /* We update the video quality */
    vc = MyApp->Endpoint ()->GetCurrentVideoCodec ();

    if (vc != NULL)
      vc->SetTxQualityLevel ((int) gconf_value_get_int (entry->value));
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
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;

  if (entry->value->type == GCONF_VALUE_INT) {
   
    /* We set the new value for tr_fps_spin_adj */
    GTK_ADJUSTMENT (pw->tr_ub_spin_adj)->value = 
      gconf_value_get_int (entry->value);

    /* We update the current frame rate */
    vc = MyApp->Endpoint ()->GetCurrentVideoCodec ();

    if (vc != NULL)
      vc->SetBackgroundFill ((int) gconf_value_get_int (entry->value));
  }
}


/* DESCRIPTION  :  Generic notifiers for entries.
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
   
    /* We set the new value for the widget */
    GTK_TOGGLE_BUTTON (toggle)->active = gconf_value_get_bool (entry->value);
    gtk_widget_draw (GTK_WIDGET (toggle), NULL);
   
  }
}


/* DESCRIPTION  :  Generic notifiers for entries.
 *                 This callback is called when a specific key of
 *                 the gconf database associated with an entry changes.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  /
 */
static void entry_changed_nt (GConfClient *client, guint cid, 
			      GConfEntry *entry, gpointer data)
{
  GtkWidget *e = GTK_WIDGET (data);
  guint signal_id;

  cout << "ici" << endl << flush;

  if (entry->value->type == GCONF_VALUE_STRING) {
   
    /* We set the new value for the widget */
    signal_id = gtk_signal_lookup ("changed", GTK_TYPE_ENTRY);
    gtk_signal_handler_block (GTK_OBJECT (e), signal_id);
    gtk_entry_set_text (GTK_ENTRY (e), gconf_value_get_string (entry->value));
    gtk_signal_handler_unblock (GTK_OBJECT (e), signal_id);
  }
}


/* DESCRIPTION  :  This callback is called when the "register" gconf value changes.
 *                 The "register" value can change if the user plays with the button,
 *                 or if he clicks on "Update" in Personnal data, or if he uses
 *                 gconftool.
 * BEHAVIOR     :  It registers to the LDAP server, IF all recquired fields are
 *                 available.
 * PRE          :  /
 */
static void register_changed_nt (GConfClient *client, guint cid, 
				 GConfEntry *entry, gpointer data)
{
  GM_pref_window_widgets *pw = (GM_pref_window_widgets *) data;
  BOOL no_error = TRUE;
  gchar *gconf_string;
  GtkWidget *msg_box;
  GMH323EndPoint *endpoint = MyApp->Endpoint ();
  GConfChangeSet* revert_cs; /* To revert the changes */


  if (entry->value->type == GCONF_VALUE_BOOL) {
    
    /* Update the widgets */
    GTK_TOGGLE_BUTTON (pw->ldap)->active = gconf_value_get_bool (entry->value);
    gtk_widget_set_sensitive (GTK_WIDGET (pw->directory_update_button), 
			      gconf_value_get_bool (entry->value));

    /* We check that all the needed information is available
       to update the LDAP directory */
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->ldap))) {

      /* Checks if the server name is ok */
      gconf_string =  gconf_client_get_string (GCONF_CLIENT (client),
					       "/apps/gnomemeeting/ldap/ldap_server", 
					       NULL);

      if ((gconf_string == NULL) || (!strcmp (gconf_string, ""))) {

	msg_box = gnome_message_box_new (_("Not registering because there is no LDAP server specified!"), 
					 GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
	no_error = FALSE;
      }
      g_free (gconf_string);


      /* Check if there is a first name */
      gconf_string =  gconf_client_get_string (GCONF_CLIENT (client),
					       "/apps/gnomemeeting/personnal_data/firstname", 
					       NULL);

      if ((gconf_string == NULL) || (!strcmp (gconf_string, ""))) {

	msg_box = gnome_message_box_new (_("Not Registering: Please provide your first name!"), 
					 GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
	no_error = FALSE;
      }
      g_free (gconf_string);


      /* Check if there is a mail */
      gconf_string =  gconf_client_get_string (GCONF_CLIENT (client),
					       "/apps/gnomemeeting/personnal_data/mail", 
					       NULL);

      if ((gconf_string == NULL) || (!strcmp (gconf_string, ""))) {
      
	msg_box = gnome_message_box_new (_("Not Registering: Please provide a valid e-mail!"), 
					 GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
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
  }
}


/* The functions  */
void gnomemeeting_init_gconf (GConfClient *client)
{
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_fps",
			   fps_limit_changed_nt, pw, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_vq",
			   tr_vq_changed_nt, pw, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/video_settings/tr_ub",
			   tr_ub_changed_nt, pw, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/ldap/register",
			   register_changed_nt, pw, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/personnal_data/firstname",
			   entry_changed_nt, pw->firstname, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/general/auto_answer",
			   toggle_changed_nt, pw->aa, 0, 0);

  gconf_client_notify_add (client, "/apps/gnomemeeting/general/do_not_disturb",
			   toggle_changed_nt, pw->dnd, 0, 0);
}


void gnomemeeting_store_config (options *opts)
{
  char *tosave = NULL;
  int cpt = 0;
  
  tosave = (gchar *) g_malloc (100);
  
  gnome_config_push_prefix ("gnomemeeting/");

  gnome_config_set_int ("VideoSettings/video_size", opts->video_size);
  gnome_config_set_int ("VideoSettings/video_format", opts->video_format);
  gnome_config_set_int ("VideoSettings/transmitted_video_quality", 
			opts->tr_vq);
  gnome_config_set_int ("VideoSettings/received_video_quality", opts->re_vq);
  gnome_config_set_int ("VideoSettings/transmitted_update_blocks", 
			opts->tr_ub);
  gnome_config_set_int ("VideoSettings/video_transmission", opts->vid_tr);
  gnome_config_set_int ("VideoSettings/tr_fps", opts->tr_fps);
  gnome_config_set_int ("VideoSettings/fps", opts->fps);
  gnome_config_set_int ("VideoSettings/video_bandwidth", 
			opts->video_bandwidth);
  gnome_config_set_int ("VideoSettings/vb", opts->vb);

  gnome_config_set_int ("GeneralSettings/show_splash", opts->show_splash);
  gnome_config_set_int ("GeneralSettings/incoming_call_sound", 
			opts->incoming_call_sound);
  gnome_config_set_int ("GeneralSettings/dnd", opts->dnd);
  gnome_config_set_int ("GeneralSettings/enable_auto_answer", opts->aa);
  gnome_config_set_int ("GeneralSettings/video_preview", opts->video_preview);

  gnome_config_set_string ("UserSettings/firstname", opts->firstname);
  gnome_config_set_string ("UserSettings/surname", opts->surname);
  gnome_config_set_string ("UserSettings/mail", opts->mail);
  gnome_config_set_string ("UserSettings/comment", opts->comment);
  gnome_config_set_string ("UserSettings/location", opts->location);
  gnome_config_set_string ("UserSettings/listen_port", opts->listen_port);
  gnome_config_set_int ("UserSettings/notfirst", opts->notfirst);

  gnome_config_set_int ("AdvancedSettings/enable_fast_start", opts->fs);
  gnome_config_set_int ("AdvancedSettings/gsm_silence_detection", 
			opts->gsm_sd);
  gnome_config_set_int ("AdvancedSettings/g711_silence_detection", 
			opts->g711_sd);
  gnome_config_set_int ("AdvancedSettings/enable_h245_tunneling", opts->ht); 
  gnome_config_set_int ("AdvancedSettings/max_bps", opts->bps);
  gnome_config_set_int ("AdvancedSettings/jitter_buffer", opts->jitter_buffer);
  gnome_config_set_int ("AdvancedSettings/g711_frames", opts->g711_frames);
  gnome_config_set_int ("AdvancedSettings/gsm_frames", opts->gsm_frames);

  gnome_config_set_int ("LDAPSettings/ldap", opts->ldap);
  gnome_config_set_string ("LDAPSettings/ldap_server", opts->ldap_server);
  gnome_config_set_string ("LDAPSettings/ldap_port", opts->ldap_port);
  gnome_config_set_string ("LDAPSettings/ldap_servers_list", 
			   opts->ldap_servers_list);
  gnome_config_set_string ("MiscSettings/contacts_list", opts->old_contacts_list);

  gnome_config_set_int ("GKSettings/gk", opts->gk);
  gnome_config_set_string ("GKSettings/gk_host", opts->gk_host);
  gnome_config_set_string ("GKSettings/gk_id", opts->gk_id);

  gnome_config_set_string ("Devices/audio_player", opts->audio_player);
  gnome_config_set_string ("Devices/audio_recorder", opts->audio_recorder);
  gnome_config_set_string ("Devices/audio_player_mixer", opts->audio_player_mixer);
  gnome_config_set_string ("Devices/audio_recorder_mixer", opts->audio_recorder_mixer);
  gnome_config_set_string ("Devices/video_device", opts->video_device);
  gnome_config_set_int ("Devices/video_channel", opts->video_channel);

  /* Save the audio codecs clist 
     First delete the values */
  for (cpt = 0 ; cpt < 5 ; cpt++) {
    
    strcpy (tosave, "EnabledAudio/");
    strcat (tosave, opts->audio_codecs [cpt] [0]);
    gnome_config_clean_key (tosave);
  }

  /* Then saves them in the order they appear */
  for (cpt = 4 ; cpt >= 0  ; cpt--) {

    strcpy (tosave, "EnabledAudio/");
    strcat (tosave, opts->audio_codecs [cpt] [0]);
    
    gnome_config_set_string(tosave, opts->audio_codecs [cpt] [1]);
  }
  
  g_free (tosave);
  
  gnome_config_sync();
  gnome_config_pop_prefix (); 		
}


/* NB : this structure has to be freed */
void gnomemeeting_read_config (options *opts)
{
  int cpt = 0;

  void *iterator; 
  char *key, *value;
  
  gnome_config_push_prefix ("gnomemeeting/");
  opts->video_size = gnome_config_get_int ("VideoSettings/video_size");
  opts->video_format = gnome_config_get_int ("VideoSettings/video_format");
  opts->tr_vq = gnome_config_get_int ("VideoSettings/transmitted_video_quality");
  opts->tr_fps = gnome_config_get_int ("VideoSettings/tr_fps");
  opts->fps = gnome_config_get_int ("VideoSettings/fps");
  opts->re_vq = gnome_config_get_int ("VideoSettings/received_video_quality");
  opts->tr_ub = gnome_config_get_int ("VideoSettings/transmitted_update_blocks");
  opts->vid_tr = gnome_config_get_int ("VideoSettings/video_transmission");
  opts->video_bandwidth = gnome_config_get_int ("VideoSettings/video_bandwidth");
  opts->vb = gnome_config_get_int ("VideoSettings/vb");

  opts->show_splash = gnome_config_get_int ("GeneralSettings/show_splash");
  opts->incoming_call_sound = 
    gnome_config_get_int ("GeneralSettings/incoming_call_sound");
  opts->aa = gnome_config_get_int ("GeneralSettings/enable_auto_answer");
  opts->dnd = gnome_config_get_int ("GeneralSettings/dnd");
  opts->video_preview = gnome_config_get_int ("GeneralSettings/video_preview");

  opts->firstname = gnome_config_get_string ("UserSettings/firstname=");
  opts->listen_port = g_strdup ("");
  opts->surname = gnome_config_get_string ("UserSettings/surname=");
  opts->mail = gnome_config_get_string ("UserSettings/mail=");
  opts->comment = gnome_config_get_string ("UserSettings/comment=");
  opts->location =  gnome_config_get_string ("UserSettings/location=");
  opts->notfirst = gnome_config_get_int ("UserSettings/notfirst");

  opts->fs = gnome_config_get_int ("AdvancedSettings/enable_fast_start");
  opts->ht = gnome_config_get_int ("AdvancedSettings/enable_h245_tunneling"); 	
  opts->bps = gnome_config_get_int ("AdvancedSettings/max_bps");
  opts->g711_sd = 
    gnome_config_get_int ("AdvancedSettings/g711_silence_detection");
  opts->gsm_sd = 
    gnome_config_get_int ("AdvancedSettings/gsm_silence_detection");
  opts->jitter_buffer = 
    gnome_config_get_int ("AdvancedSettings/jitter_buffer");
  opts->g711_frames = 
    gnome_config_get_int ("AdvancedSettings/g711_frames");
  opts->gsm_frames = 
    gnome_config_get_int ("AdvancedSettings/gsm_frames");

  opts->ldap = gnome_config_get_int ("LDAPSettings/ldap");
  opts->ldap_server = gnome_config_get_string ("LDAPSettings/ldap_server=");
  opts->ldap_port = gnome_config_get_string ("LDAPSettings/ldap_port=");
  opts->ldap_servers_list = 
    gnome_config_get_string ("LDAPSettings/ldap_servers_list=");
  opts->old_contacts_list =
    gnome_config_get_string ("MiscSettings/contacts_list=");


  opts->gk = gnome_config_get_int ("GKSettings/gk");
  opts->gk_host = gnome_config_get_string ("GKSettings/gk_host=");
  opts->gk_id = gnome_config_get_string ("GKSettings/gk_id=");

  opts->audio_player = gnome_config_get_string ("Devices/audio_player=");
  opts->audio_recorder = gnome_config_get_string ("Devices/audio_recorder=");
  opts->audio_player_mixer = 
    gnome_config_get_string ("Devices/audio_player_mixer=");
  opts->audio_recorder_mixer = 
    gnome_config_get_string ("Devices/audio_recorder_mixer=");
  opts->video_device = gnome_config_get_string ("Devices/video_device=");
  opts->video_channel = gnome_config_get_int ("Devices/video_channel");

  gnome_config_sync();
  gnome_config_pop_prefix ();

  iterator = gnome_config_init_iterator("gnomemeeting/EnabledAudio");
 
  while (gnome_config_iterator_next  (iterator, &key, &value)) {

    opts->audio_codecs [cpt] [0] = key;
    opts->audio_codecs [cpt] [1] = value;
    /* Do not free key and value as they are assigned 
       as pointers to opts->audio_codecs */
    cpt++;
  }

  /* Handle old config files format */
  if(opts->audio_player == NULL) 
    opts->audio_player =  
      g_strdup (PSoundChannel::GetDefaultDevice (PSoundChannel::Player));

  if(opts->audio_recorder == NULL) 
    opts->audio_recorder =  
      g_strdup (PSoundChannel::GetDefaultDevice (PSoundChannel::Recorder));

  if(opts->audio_player_mixer == NULL)  
    opts->audio_player_mixer = g_strdup ("/dev/mixer");

  if(opts->audio_recorder_mixer == NULL)  
    opts->audio_recorder_mixer = g_strdup ("/dev/mixer");

  if (opts->video_device == NULL)
    opts->video_device = 
    g_strdup (PVideoChannel::GetDefaultDevice (PVideoChannel::Player));

  if (opts->gk_host == NULL)
    opts->gk_host = g_strdup ("");

  if (opts->gk_id == NULL)
    opts->gk_id = g_strdup ("");

  if (opts->ldap_servers_list == NULL)
    opts->ldap_servers_list = g_strdup ("");
}


void g_options_free (options *opts)
{
  g_free (opts->firstname);
  g_free (opts->listen_port); 
  g_free (opts->surname);
  g_free (opts->mail);
  g_free (opts->location);
  g_free (opts->comment);
  g_free (opts->gk_host);
  g_free (opts->gk_id);
  g_free (opts->audio_player);
  g_free (opts->audio_recorder);
  g_free (opts->audio_player_mixer);
  g_free (opts->audio_recorder_mixer);
  g_free (opts->video_device);
  g_free (opts->ldap_servers_list);

  for (int i = 0 ; i < 5 ; i++)
    for (int j = 0 ; j < 2 ; j++)
      g_free (opts->audio_codecs [i] [j]);

  g_free (opts->ldap_server);
  g_free (opts->ldap_port);
}


/* NB: READ CONFIG FROM STRUCT : config in this structure should no be freed, 
                                 it contains pointers to the text fields of 
                                 the widgets, that will be destroyed with
                                 their text. */
gboolean gnomemeeting_check_config_from_struct ()
{
  GtkWidget *msg_box = NULL;
  int vol;
  gboolean no_error = TRUE;

  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);


  /* Check Audio Mixer Settings for the Recorder and the Player device. */
  if (pw->audio_mixer_changed) {

    if (gnomemeeting_volume_get (gtk_entry_get_text(GTK_ENTRY 
						    (pw->audio_player_mixer)), 
				 0, &vol) == -1) {

      msg_box = gnome_message_box_new (_("Could not open the player mixer."), 
				       GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
      
      no_error = FALSE;
    }

    if (gnomemeeting_volume_get (gtk_entry_get_text(GTK_ENTRY 
						    (pw->audio_recorder_mixer)), 
				 0, &vol) == -1) {

      msg_box = gnome_message_box_new (_("Could not open the player mixer."), 
				       GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
      
      no_error = FALSE;
    }    
  }

  /* Check Gatekeeper Settings */
  if (pw->gk_changed) {

    GtkWidget *active_item = gtk_menu_get_active (GTK_MENU 
						  (GTK_OPTION_MENU (pw->gk)->menu));
    int item_index = g_list_index (GTK_MENU_SHELL 
				   (GTK_OPTION_MENU (pw->gk)->menu)->children, 
				   active_item);
    if (item_index == 1) {

      if (!strcmp (gtk_entry_get_text (GTK_ENTRY (pw->gk_host)), "")) {
	
	msg_box = gnome_message_box_new (_("Cannot register to an empty host. Please specify the host to contact to register with the gatekeeper."), GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
	
	no_error = FALSE;
      }
    }


    if (item_index == 2) {
      
      if (!strcmp (gtk_entry_get_text (GTK_ENTRY (pw->gk_id)), "")) {

	msg_box = gnome_message_box_new (_("Please specify a Gatekeeper ID to contact to register."), GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
	
	no_error = FALSE;
      }
    }
    
    if (!no_error)
      gtk_option_menu_set_history (GTK_OPTION_MENU (pw->gk), 0);	
  }
 
  
  if (msg_box != NULL)
    gtk_widget_show (msg_box);
  
  return no_error;
}


options *gnomemeeting_read_config_from_struct ()
{
  options *opts = NULL;
  GtkWidget *active_item;
  gint item_index;
  int cpt;

  opts = new (options);
  memset (opts, 0, sizeof (options));

  gnomemeeting_read_config (opts);
 
  GM_pref_window_widgets *pw = gnomemeeting_get_pref_window (gm);

  /* General Settings */
  opts->show_splash = gtk_toggle_button_get_active 
    (GTK_TOGGLE_BUTTON (pw->show_splash));
  opts->incoming_call_sound = gtk_toggle_button_get_active 
    (GTK_TOGGLE_BUTTON (pw->incoming_call_sound));

  /* User Settings */
  opts->firstname = gtk_entry_get_text (GTK_ENTRY (pw->firstname));
  opts->surname = gtk_entry_get_text (GTK_ENTRY (pw->surname));
  opts->location = gtk_entry_get_text (GTK_ENTRY (pw->location));
  opts->mail = gtk_entry_get_text (GTK_ENTRY (pw->mail));
  opts->comment = gtk_entry_get_text (GTK_ENTRY (pw->comment));
  //opts->listen_port = gtk_entry_get_text (GTK_ENTRY (pw->entry_port));
  opts->aa = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->aa));
  opts->ht = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->ht));
  opts->fs = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->fs));
  opts->bps = (int) pw->bps_spin_adj->value;
  opts->g711_sd = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->g711_sd));
  opts->gsm_sd = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->gsm_sd));
  opts->dnd = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->dnd));
  opts->video_preview = 
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->video_preview));

  /* LDAP Settings */
  opts->ldap =  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->ldap));
  opts->ldap_server = gtk_entry_get_text (GTK_ENTRY (pw->ldap_server));
  opts->ldap_port = gtk_entry_get_text (GTK_ENTRY (pw->ldap_port));

  /* Gatekeeper Settings */
  active_item = gtk_menu_get_active (GTK_MENU 
				     (GTK_OPTION_MENU (pw->gk)->menu));
  item_index = g_list_index (GTK_MENU_SHELL 
			     (GTK_OPTION_MENU (pw->gk)->menu)->children, 
			      active_item);
  opts->gk = item_index;
  opts->gk_host = gtk_entry_get_text (GTK_ENTRY (pw->gk_host));
  opts->gk_id = gtk_entry_get_text (GTK_ENTRY (pw->gk_id));

  /* Video Codec Settings */
  active_item = gtk_menu_get_active (GTK_MENU 
				     (GTK_OPTION_MENU (pw->opt1)->menu));
  item_index = g_list_index (GTK_MENU_SHELL 
			     (GTK_OPTION_MENU (pw->opt1)->menu)->children, 
			      active_item);
  opts->video_size = item_index;
      
  active_item = gtk_menu_get_active (GTK_MENU (GTK_OPTION_MENU 
					       (pw->opt2)->menu));
  item_index = g_list_index (GTK_MENU_SHELL (GTK_OPTION_MENU (pw->opt2)->menu)
			     ->children, 
			      active_item);
  opts->video_format = item_index;
      
  opts->tr_vq = (int) pw->tr_vq_spin_adj->value;
  opts->tr_fps = (int) pw->tr_fps_spin_adj->value;
  opts->tr_ub = (int) pw->tr_ub_spin_adj->value;
  opts->re_vq = (int) pw->re_vq_spin_adj->value;
  opts->video_bandwidth = (int) pw->video_bandwidth_spin_adj->value;
  opts->jitter_buffer = (int) pw->jitter_buffer_spin_adj->value;
  opts->g711_frames = (int) pw->g711_frames_spin_adj->value;
  opts->gsm_frames = (int) pw->gsm_frames_spin_adj->value;

  opts->vid_tr = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->vid_tr));
  opts->vb = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->vb));
  opts->fps = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->fps));

  
  /* Audio codecs clist */
  for (cpt = 0 ; cpt < 5 ; cpt++) {

    gtk_clist_get_text (GTK_CLIST (pw->clist_avail), 
			cpt, 1, &opts->audio_codecs [cpt] [0]);
    opts->audio_codecs [cpt] [1] = (char *) 
      gtk_clist_get_row_data (GTK_CLIST 
			      (pw->clist_avail),
			      cpt);
  }

  /* Devices */
  opts->audio_player = gtk_entry_get_text 
    (GTK_ENTRY (GTK_COMBO (pw->audio_player)->entry));
  opts->audio_recorder = gtk_entry_get_text 
    (GTK_ENTRY (GTK_COMBO (pw->audio_recorder)->entry));
  opts->audio_player_mixer = 
    gtk_entry_get_text (GTK_ENTRY (pw->audio_player_mixer));
  opts->audio_recorder_mixer = 
    gtk_entry_get_text (GTK_ENTRY (pw->audio_recorder_mixer));
  opts->video_device = gtk_entry_get_text 
    (GTK_ENTRY (GTK_COMBO (pw->video_device)->entry));
  opts->video_channel = (int) pw->video_channel_spin_adj->value; 


  return opts;
}


void gnomemeeting_read_config_from_gui (options *opts)
{
  int i = 0;
  gchar *old_pointer = NULL;
  gchar *text = NULL;

  /* Get the data */
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);
  GM_ldap_window_widgets *lw = gnomemeeting_get_ldap_window (gm);

  /* Free the old values, we will read from the GUI */
  g_free (opts->ldap_servers_list);
  opts->ldap_servers_list = NULL;

  while ((text = (gchar *) g_list_nth_data (lw->ldap_servers_list, i))) {
    old_pointer = opts->ldap_servers_list;
    opts->ldap_servers_list = g_strconcat (text, ":", old_pointer, NULL);
    g_free (old_pointer);
    i++;
    }

  /* Free the old values, we will read from the GUI */
  g_free (opts->old_contacts_list);
  opts->old_contacts_list = NULL;
  i = 0;
  while (text = (gchar *) g_list_nth_data (gw->old_contacts_list, i))
    {
      old_pointer = opts->old_contacts_list;
      opts->old_contacts_list = g_strconcat (text, ":", old_pointer, NULL);
      g_free (old_pointer);
      i++;
    }

  /* Check options from the menus, and update the options structure */
  GtkWidget *object = (GtkWidget *) 
    gtk_object_get_data (GTK_OBJECT (gm), "view_menu_uiinfo");

  GnomeUIInfo *view_menu_uiinfo = (GnomeUIInfo *) object;
}


int gnomemeeting_config_first_time (void)
{
  gnome_config_push_prefix ("gnomemeeting/");

  int res = gnome_config_get_int ("UserSettings/notfirst");
  int version = gnome_config_get_int ("UserSettings/version");

  if (res == 0)
    gnomemeeting_init_config ();

  if (version < 122) { 
       
    gnome_config_set_int ("UserSettings/version", 122);

    gnome_config_set_int ("VideoSettings/transmitted_video_quality", 5);
    gnome_config_set_int ("VideoSettings/tr_fps", 15);
    gnome_config_set_int ("VideoSettings/fps", 0);
    gnome_config_set_int ("VideoSettings/vb", 0);

    gnome_config_set_int ("GeneralSettings/show_docklet", 1);
    gnome_config_set_int ("GeneralSettings/enable_popup", 1);
    
    gnome_config_set_int ("AdvancedSettings/enable_h245_tunneling", 0);
    
    gnome_config_set_int ("AdvancedSettings/gsm_silence_detection", 1);
    gnome_config_set_int ("AdvancedSettings/g711_silence_detection", 1);
    gnome_config_set_int ("AdvancedSettings/jitter_buffer", 50);
    gnome_config_set_int ("AdvancedSettings/gsm_frames", 4);
    gnome_config_set_int ("AdvancedSettings/g711_frames", 30);
    
    gnome_config_set_string ("LDAPSettings/ldap_servers_list", 
			     "ils.advalvas.be:ils.netmeetinghq.com:argo.dyndns.org:");
    gnome_config_set_string ("LDAPSettings/ldap_server", 
			     "argo.dyndns.org");

    gnome_config_set_int ("GKSettings/gk", 0);
    gnome_config_set_string ("GKSettings/gk_host", "");
    gnome_config_set_string ("GKSettings/gk_id", "");      
  }

  gnome_config_sync();
  gnome_config_pop_prefix ();

  return (version);
}


void gnomemeeting_init_config (void)
{
  
  gnome_config_push_prefix ("gnomemeeting/");
  gnome_config_set_int ("VideoSettings/video_size", 0);
  gnome_config_set_int ("VideoSettings/video_format", 2);
  gnome_config_set_int ("VideoSettings/transmitted_video_quality", 3);
  gnome_config_set_int ("VideoSettings/tr_fps", 15);
  gnome_config_set_int ("VideoSettings/fps", 1);
  gnome_config_set_int ("VideoSettings/received_video_quality", 3);
  gnome_config_set_int ("VideoSettings/transmitted_update_blocks", 2);
  gnome_config_set_int ("VideoSettings/video_transmission", 0);
  gnome_config_set_int ("VideoSettings/video_bandwidth", 32);
  gnome_config_set_int ("VideoSettings/vb", 0);

  gnome_config_set_string ("UserSettings/firstname", "");
  gnome_config_set_string ("UserSettings/surname", "");
  gnome_config_set_string ("UserSettings/mail", "");
  gnome_config_set_string ("UserSettings/comment", "");
  gnome_config_set_string ("UserSettings/location", "");
  gnome_config_set_string ("UserSettings/listen_port", "1720");
  gnome_config_set_int ("UserSettings/notfirst", 1);

  gnome_config_set_int ("GeneralSettings/show_splash", 1);
  gnome_config_set_int ("GeneralSettings/show_notebook", 1);
  gnome_config_set_int ("GeneralSettings/show_statusbar", 1);
  gnome_config_set_int ("GeneralSettings/show_quickbar", 1);
  gnome_config_set_int ("GeneralSettings/show_docklet", 0);
  gnome_config_set_int ("GeneralSettings/incoming_call_sound", 1);
  gnome_config_set_int ("GeneralSettings/enable_auto_answer", 0);
  gnome_config_set_int ("GeneralSettings/dnd", 0);
  gnome_config_set_int ("GeneralSettings/enable_popup", 0);
  gnome_config_set_int ("GeneralSettings/video_preview", 0);

  gnome_config_set_int ("AdvancedSettings/enable_fast_start", 0);
  gnome_config_set_int ("AdvancedSettings/enable_h245_tunneling", 1); 	
  gnome_config_set_int ("AdvancedSettings/max_bps", 20000);
  gnome_config_set_int ("AdvancedSettings/gsm_silence_detection", 1);
  gnome_config_set_int ("AdvancedSettings/g711_silence_detection", 1);
  gnome_config_set_int ("AdvancedSettings/jitter_buffer", 50);
  gnome_config_set_int ("AdvancedSettings/gsm_frames", 4);
  gnome_config_set_int ("AdvancedSettings/g711_frames", 30);

  gnome_config_set_int ("LDAPSettings/ldap", 0);
  gnome_config_set_string ("LDAPSettings/ldap_server", "");
  gnome_config_set_string ("LDAPSettings/ldap_port", "389");
  gnome_config_set_string ("LDAPSettings/ldap_servers_list", 
			   "ils.advalvas.be:ils.pi.be:ils.netmeetinghq.com:");

  gnome_config_set_int ("GKSettings/gk", 0);
  gnome_config_set_string ("GKSettings/gk_host", "");
  gnome_config_set_string ("GKSettings/gk_id", "");

  gnome_config_set_string ("EnabledAudio/LPC10", "0");
  gnome_config_set_string ("EnabledAudio/GSM-06.10", "0");
  gnome_config_set_string ("EnabledAudio/G.711-uLaw-64k", "1");
  gnome_config_set_string ("EnabledAudio/G.711-ALaw-64k", "1");
  gnome_config_set_string ("EnabledAudio/MS-GSM", "1");

  gnome_config_set_string ("Devices/audio_player", 
			   PSoundChannel::GetDefaultDevice (PSoundChannel::Player));
  gnome_config_set_string ("Devices/audio_recorder", 
			   PSoundChannel::GetDefaultDevice (PSoundChannel::Recorder));
  gnome_config_set_string ("Devices/audio_player_mixer", "/dev/mixer");
  gnome_config_set_string ("Devices/audio_recorder_mixer", "/dev/mixer");
  gnome_config_set_string ("Devices/video_device", 
			   PVideoChannel::GetDefaultDevice (PVideoChannel::Player));
  gnome_config_set_int ("Devices/video_channel", 0);

  gnome_config_set_string ("Placement/Dock", 
			   "Toolbar\\3,0,0,28\\Menubar\\0,0,0,0");
  gnome_config_sync();
  gnome_config_pop_prefix ();
}

