/***************************************************************************
                          config.cxx  -  description
                             -------------------
    begin                : Wed Feb 14 2001
    copyright            : (C) 2001 by Damien Sandras
    description          : Functions to store the config options
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

#include "config.h"
#include "common.h"
#include "audio.h"
#include "videograbber.h"
#include "main.h"

#include "../config.h"

#include <iostream.h> // 

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;

/******************************************************************************/
/* The functions                                                              */
/******************************************************************************/

void store_config (options *opts)
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
  gnome_config_set_int ("GeneralSettings/show_notebook", opts->show_notebook);
  gnome_config_set_int ("GeneralSettings/show_statusbar", 
			opts->show_statusbar);
  gnome_config_set_int ("GeneralSettings/show_quickbar", 
			opts->show_quickbar);
  gnome_config_set_int ("GeneralSettings/show_docklet", 
			opts->show_docklet);
  gnome_config_set_int ("GeneralSettings/incoming_call_sound", 
			opts->incoming_call_sound);
  gnome_config_set_int ("GeneralSettings/dnd", opts->dnd);
  gnome_config_set_int ("GeneralSettings/enable_auto_answer", opts->aa);
  gnome_config_set_int ("GeneralSettings/enable_popup", opts->popup);
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

  gnome_config_set_int ("GKSettings/gk", opts->gk);
  gnome_config_set_string ("GKSettings/gk_host", opts->gk_host);
  gnome_config_set_string ("GKSettings/gk_id", opts->gk_id);

  gnome_config_set_string ("Devices/audio_player", opts->audio_player);
  gnome_config_set_string ("Devices/audio_recorder", opts->audio_recorder);
  gnome_config_set_string ("Devices/audio_player_mixer", opts->audio_player_mixer);
  gnome_config_set_string ("Devices/audio_recorder_mixer", opts->audio_recorder_mixer);
  gnome_config_set_string ("Devices/video_device", opts->video_device);
  gnome_config_set_int ("Devices/video_channel", opts->video_channel);

  /* Save the audio codecs clist */
  /* First delete the values */
  for (cpt = 0 ; cpt < 5 ; cpt++)
    {
      strcpy (tosave, "EnabledAudio/");
      strcat (tosave, opts->audio_codecs [cpt] [0]);
      gnome_config_clean_key (tosave);
    }

  /* Then saves them in the order they appear */
  for (cpt = 4 ; cpt >= 0  ; cpt--)
    {
      strcpy (tosave, "EnabledAudio/");
      strcat (tosave, opts->audio_codecs [cpt] [0]);
      
      gnome_config_set_string(tosave, opts->audio_codecs [cpt] [1]);
    }

  g_free (tosave);

  gnome_config_sync();
  gnome_config_pop_prefix (); 		
}


// NB : this structure has to be freed
void read_config (options *opts)
{
  int cpt = 0;

  // to iterate through the audio codecs clist
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
  opts->show_docklet = gnome_config_get_int ("GeneralSettings/show_docklet");
  opts->show_notebook = gnome_config_get_int ("GeneralSettings/show_notebook");
  opts->show_statusbar = gnome_config_get_int ("GeneralSettings/show_statusbar");
  opts->show_quickbar = gnome_config_get_int ("GeneralSettings/show_quickbar");
  opts->incoming_call_sound = 
    gnome_config_get_int ("GeneralSettings/incoming_call_sound");
  opts->aa = gnome_config_get_int ("GeneralSettings/enable_auto_answer");
  opts->dnd = gnome_config_get_int ("GeneralSettings/dnd");
  opts->popup = gnome_config_get_int ("GeneralSettings/enable_popup");
  opts->video_preview = gnome_config_get_int ("GeneralSettings/video_preview");

  opts->firstname = gnome_config_get_string ("UserSettings/firstname=");
  opts->listen_port = gnome_config_get_string ("UserSettings/listen_port=");	
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
 
  while (gnome_config_iterator_next  (iterator, &key, &value))
    {
      opts->audio_codecs [cpt] [0] = key;
      opts->audio_codecs [cpt] [1] = value;
      // Do not free key and value as they are assigned 
      // as pointers to opts->audio_codecs
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


// NB: READ CONFIG FROM STRUCT : config in this structure should no be freed, 
//                               it contains pointers to the text fields of 
//                               the widgets, that will be destroyed with
//                               their text
gboolean check_config_from_struct (GM_pref_window_widgets *pw)
{
  GtkWidget *msg_box = NULL;
  int vol;
  gboolean no_error = TRUE;

  // ILS
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->ldap)))
    {
      // Checks if the server name is ok
      if (!strcmp (gtk_entry_get_text (GTK_ENTRY (pw->ldap_server)), ""))
	{
	  msg_box = gnome_message_box_new (_("Sorry, no ldap server specified!"), 
					   GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->ldap), FALSE);
	  no_error = FALSE;
	}

      if (!strcmp (gtk_entry_get_text (GTK_ENTRY (pw->ldap_port)), "")
	  || atoi (gtk_entry_get_text (GTK_ENTRY (pw->ldap_port))) < 1
	  || atoi (gtk_entry_get_text (GTK_ENTRY (pw->ldap_port))) > 2000)
	{
	  msg_box = gnome_message_box_new (_("Sorry, invalid ldap server port!"), 
					   GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->ldap), FALSE);
	  no_error = FALSE;
	}

      if (!strcmp (gtk_entry_get_text (GTK_ENTRY (pw->firstname)), ""))
	{
	  msg_box = gnome_message_box_new (_("Please provide your first name!"), 
					   GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->ldap), FALSE);
	  no_error = FALSE;
	}

      if (!strcmp (gtk_entry_get_text (GTK_ENTRY (pw->mail)), ""))
	{
	  msg_box = gnome_message_box_new (_("Please provide a valid e-mail!"), 
					   GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pw->ldap), FALSE);
	  no_error = FALSE;
	}
    }


  // Check Audio Mixer Settings for the Recorder and the Player device.
  if (pw->audio_mixer_changed)
    {
      if (GM_volume_get (gtk_entry_get_text (GTK_ENTRY (pw->audio_player_mixer)), 
			 0, &vol) == -1)
	{
	  msg_box = gnome_message_box_new (_("Could not open the player mixer."), 
					   GNOME_MESSAGE_BOX_ERROR, "OK", NULL);

	  no_error = FALSE;
	}

      if (GM_volume_get (gtk_entry_get_text (GTK_ENTRY (pw->audio_recorder_mixer)), 
			 0, &vol) == -1)
	{
	  msg_box = gnome_message_box_new (_("Could not open the player mixer."), 
					   GNOME_MESSAGE_BOX_ERROR, "OK", NULL);

	  no_error = FALSE;
	}    
    }

  // Check Gatekeeper Settings
  if (pw->gk_changed)
    {
      GtkWidget *active_item = gtk_menu_get_active (GTK_MENU 
						    (GTK_OPTION_MENU (pw->gk)->menu));
      int item_index = g_list_index (GTK_MENU_SHELL 
				     (GTK_OPTION_MENU (pw->gk)->menu)->children, 
				     active_item);
      if (item_index == 1)
	{
	  if (!strcmp (gtk_entry_get_text (GTK_ENTRY (pw->gk_host)), ""))
	    {
	      msg_box = gnome_message_box_new (_("Cannot register to an empty host. Please specify the host to contact to register with the gatekeeper."), GNOME_MESSAGE_BOX_ERROR, "OK", NULL);

	      no_error = FALSE;
	    }
	}


     if (item_index == 2)
	{
	  if (!strcmp (gtk_entry_get_text (GTK_ENTRY (pw->gk_id)), ""))
	    {
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


options * read_config_from_struct (GM_pref_window_widgets *pw)
{
  options *opts = NULL;
  GtkWidget *active_item;
  gint item_index;
  int cpt;

  opts = new (options);
  memset (opts, 0, sizeof (options));


  read_config (opts);
 
  /* General Settings */
  opts->show_splash = gtk_toggle_button_get_active 
    (GTK_TOGGLE_BUTTON (pw->show_splash));
  opts->show_notebook = gtk_toggle_button_get_active 
    (GTK_TOGGLE_BUTTON (pw->show_notebook));
  opts->show_statusbar = gtk_toggle_button_get_active 
    (GTK_TOGGLE_BUTTON (pw->show_statusbar));
  opts->show_quickbar = gtk_toggle_button_get_active 
    (GTK_TOGGLE_BUTTON (pw->show_quickbar));
  opts->show_docklet = gtk_toggle_button_get_active 
    (GTK_TOGGLE_BUTTON (pw->show_docklet));
  opts->incoming_call_sound = gtk_toggle_button_get_active 
    (GTK_TOGGLE_BUTTON (pw->incoming_call_sound));

  /* User Settings */
  opts->firstname = gtk_entry_get_text (GTK_ENTRY (pw->firstname));
  opts->surname = gtk_entry_get_text (GTK_ENTRY (pw->surname));
  opts->location = gtk_entry_get_text (GTK_ENTRY (pw->location));
  opts->mail = gtk_entry_get_text (GTK_ENTRY (pw->mail));
  opts->comment = gtk_entry_get_text (GTK_ENTRY (pw->comment));
  opts->listen_port = gtk_entry_get_text (GTK_ENTRY (pw->entry_port));
  opts->aa = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->aa));		
  opts->ht = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->ht));
  opts->fs = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->fs));
  opts->bps = (int) pw->bps_spin_adj->value;
  opts->g711_sd = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->g711_sd));
  opts->gsm_sd = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->gsm_sd));
  opts->dnd = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->dnd));
  opts->popup = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->popup));
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
      
  opts->tr_vq = (int) pw->tr_vq_spin_adj->value; // Transmitted Video Quality
  opts->tr_fps = (int) pw->tr_fps_spin_adj->value; // Transmitted FPS
  opts->tr_ub = (int) pw->tr_ub_spin_adj->value; // Number of Updated Blocks
  opts->re_vq = (int) pw->re_vq_spin_adj->value; // Received Video Quality
  opts->video_bandwidth = (int) pw->video_bandwidth_spin_adj->value;
  opts->jitter_buffer = (int) pw->jitter_buffer_spin_adj->value;
  opts->g711_frames = (int) pw->g711_frames_spin_adj->value;
  opts->gsm_frames = (int) pw->gsm_frames_spin_adj->value;

  opts->vid_tr = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->vid_tr));
  opts->vb = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->vb));
  opts->fps = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->fps));

  
  /* Audio codecs clist */
  for (cpt = 0 ; cpt < 5 ; cpt++)
    {
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


void read_config_from_gui (GM_window_widgets *gw, GM_ldap_window_widgets *lw, options *opts)
{
  int i = 0;
  gchar *old_pointer = NULL;
  gchar *text = NULL;

  /* Free the old values, we will read from the GUI */
  g_free (opts->ldap_servers_list);
  opts->ldap_servers_list = NULL;

  while ((text = (gchar *) g_list_nth_data (lw->ldap_servers_list, i)))
    {
      old_pointer = opts->ldap_servers_list;
      opts->ldap_servers_list = g_strconcat (text, ":", old_pointer, NULL);
      g_free (old_pointer);
      i++;
    }

  /* Check options from the menus, and update the options structure */
  GtkWidget *object = (GtkWidget *) 
    gtk_object_get_data (GTK_OBJECT (gm),
			 "view_menu_uiinfo");

  GnomeUIInfo *view_menu_uiinfo = (GnomeUIInfo *) object;

  opts->show_notebook =  
    GTK_CHECK_MENU_ITEM (view_menu_uiinfo [2].widget)->active;
  opts->show_quickbar =  
    GTK_CHECK_MENU_ITEM (view_menu_uiinfo [3].widget)->active;
  opts->show_statusbar =  
    GTK_CHECK_MENU_ITEM (view_menu_uiinfo [4].widget)->active;
  opts->show_docklet =  
    GTK_CHECK_MENU_ITEM (view_menu_uiinfo [5].widget)->active;
}


int config_first_time (void)
{
  gnome_config_push_prefix ("gnomemeeting/");

  int res = gnome_config_get_int ("UserSettings/notfirst");
  int version = gnome_config_get_int ("UserSettings/version");

  if (res == 0)
    init_config ();

  if (version < 12) { 
       
    gnome_config_set_int ("UserSettings/version", 12);

    gnome_config_set_int ("VideoSettings/transmitted_video_quality", 5);
    gnome_config_set_int ("VideoSettings/tr_fps", 15);
    gnome_config_set_int ("VideoSettings/fps", 0);
    gnome_config_set_int ("VideoSettings/vb", 0);

    gnome_config_set_int ("GeneralSettings/show_docklet", 1);
    gnome_config_set_int ("GeneralSettings/enable_popup", 1);
    
    gnome_config_set_int ("AdvancedSettings/enable_h245_tunneling", 1);
    
    gnome_config_set_int ("AdvancedSettings/gsm_silence_detection", 1);
    gnome_config_set_int ("AdvancedSettings/g711_silence_detection", 1);
    gnome_config_set_int ("AdvancedSettings/jitter_buffer", 50);
    gnome_config_set_int ("AdvancedSettings/gsm_frames", 4);
    gnome_config_set_int ("AdvancedSettings/g711_frames", 30);
    
    gnome_config_set_string ("LDAPSettings/ldap_servers_list", 
			     "ils.advalvas.be:ils.pi.be:ils.netmeetinghq.com:");

    gnome_config_set_int ("GKSettings/gk", 0);
    gnome_config_set_string ("GKSettings/gk_host", "");
    gnome_config_set_string ("GKSettings/gk_id", "");      
  }

  gnome_config_sync();
  gnome_config_pop_prefix ();

  return (version);
}


void init_config (void)
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

/******************************************************************************/
