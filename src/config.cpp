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
#include "webcam.h"

#include <iostream.h> // 

extern GtkWidget *gm;

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
  gnome_config_set_int ("VideoSettings/transmitted_video_quality", opts->tr_vq);
  gnome_config_set_int ("VideoSettings/received_video_quality", opts->re_vq);
  gnome_config_set_int ("VideoSettings/transmitted_update_blocks", opts->tr_ub);
  gnome_config_set_int ("VideoSettings/video_transmission", opts->vid_tr);
  gnome_config_set_int ("VideoSettings/transmitted_fps", opts->tr_fps);

  gnome_config_set_int ("GeneralSettings/show_splash", opts->show_splash);
  gnome_config_set_int ("GeneralSettings/show_notebook", opts->show_notebook);
  gnome_config_set_int ("GeneralSettings/show_statusbar", opts->show_statusbar);
  gnome_config_set_int ("GeneralSettings/incoming_call_sound", 
			opts->incoming_call_sound);

  gnome_config_set_string ("UserSettings/firstname", opts->firstname);
  gnome_config_set_string ("UserSettings/surname", opts->surname);
  gnome_config_set_string ("UserSettings/mail", opts->mail);
  gnome_config_set_string ("UserSettings/comment", opts->comment);
  gnome_config_set_string ("UserSettings/location", opts->location);
  gnome_config_set_string ("UserSettings/listen_port", opts->listen_port);
  gnome_config_set_int ("UserSettings/notfirst", opts->notfirst);

  gnome_config_set_int ("AdvancedSettings/enable_fast_start", opts->fs);
  gnome_config_set_int ("AdvancedSettings/enable_h245_tunneling", opts->ht); 	
  gnome_config_set_int ("AdvancedSettings/enable_auto_answer", opts->aa);
  gnome_config_set_int ("AdvancedSettings/max_bps", opts->bps);
  gnome_config_set_int ("AdvancedSettings/silence_detection", opts->sd);
  gnome_config_set_int ("AdvancedSettings/dnd", opts->dnd);

  gnome_config_set_int ("LDAPSettings/ldap", opts->ldap);
  gnome_config_set_string ("LDAPSettings/ldap_server", opts->ldap_server);
  gnome_config_set_string ("LDAPSettings/ldap_port", opts->ldap_port);

  gnome_config_set_string ("Devices/audio_device", opts->audio_device);
  gnome_config_set_string ("Devices/audio_mixer", opts->audio_mixer);
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
  opts->tr_fps = gnome_config_get_int ("VideoSettings/transmitted_fps");
  opts->re_vq = gnome_config_get_int ("VideoSettings/received_video_quality");
  opts->tr_ub = gnome_config_get_int ("VideoSettings/transmitted_update_blocks");
  opts->vid_tr = gnome_config_get_int ("VideoSettings/video_transmission");

  opts->show_splash = gnome_config_get_int ("GeneralSettings/show_splash");
  opts->show_notebook = gnome_config_get_int ("GeneralSettings/show_notebook");
  opts->show_statusbar = gnome_config_get_int ("GeneralSettings/show_statusbar");
  opts->incoming_call_sound = 
    gnome_config_get_int ("GeneralSettings/incoming_call_sound");

  opts->firstname = gnome_config_get_string ("UserSettings/firstname");
  opts->listen_port = gnome_config_get_string ("UserSettings/listen_port");	
  opts->surname = gnome_config_get_string ("UserSettings/surname");
  opts->mail = gnome_config_get_string ("UserSettings/mail");
  opts->comment = gnome_config_get_string ("UserSettings/comment");
  opts->location =  gnome_config_get_string ("UserSettings/location");
  opts->notfirst = gnome_config_get_int ("UserSettings/notfirst");

  opts->fs = gnome_config_get_int ("AdvancedSettings/enable_fast_start");
  opts->ht = gnome_config_get_int ("AdvancedSettings/enable_h245_tunneling"); 	
  opts->aa = gnome_config_get_int ("AdvancedSettings/enable_auto_answer");
  opts->bps = gnome_config_get_int ("AdvancedSettings/max_bps");
  opts->sd = gnome_config_get_int ("AdvancedSettings/silence_detection");
  opts->dnd = gnome_config_get_int ("AdvancedSettings/dnd");

  opts->ldap = gnome_config_get_int ("LDAPSettings/ldap");
  opts->ldap_server = gnome_config_get_string ("LDAPSettings/ldap_server");
  opts->ldap_port = gnome_config_get_string ("LDAPSettings/ldap_port");

  opts->audio_device = gnome_config_get_string ("Devices/audio_device");
  opts->audio_mixer = gnome_config_get_string ("Devices/audio_mixer");
  opts->video_device = gnome_config_get_string ("Devices/video_device");
  opts->video_channel = gnome_config_get_int ("Devices/video_channel");

  gnome_config_sync();
  gnome_config_pop_prefix ();

  iterator = gnome_config_init_iterator("gnomemeeting/EnabledAudio");
 
  while (gnome_config_iterator_next  (iterator, &key, &value))
    {
      opts->audio_codecs [cpt] [0] = key;
      opts->audio_codecs [cpt] [1] = value;
      // Do not free key and value as they are assigned as pointers to opts->audio_codecs
      cpt++;
    }

  /* handle old config files which do not have a Devices section */
  if(opts->audio_device == NULL) opts->audio_device="/dev/dsp";
  if(opts->audio_mixer == NULL)  opts->audio_mixer="/dev/mixer";
#ifdef __linux__
  if(opts->video_device == NULL) opts->video_device="/dev/video";
#else
  if(opts->video_device == NULL) opts->video_device="/dev/bktr0";
#endif
}


void g_options_free (options *opts)
{
  g_free (opts->firstname);
  g_free (opts->listen_port); 
  g_free (opts->surname);
  g_free (opts->mail);
  g_free (opts->location);
  g_free (opts->comment);
  g_free (opts->audio_device);
  g_free (opts->audio_mixer);
  g_free (opts->video_device);

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

  // LDAP
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


  // Check Audio Settings
  if (pw->audio_mixer_changed)
    {
      if (GM_volume_get (gtk_entry_get_text (GTK_ENTRY (pw->audio_mixer)), 
			 0, &vol) == -1)
	{
	  msg_box = gnome_message_box_new (_("Could not open the mixer."), 
					   GNOME_MESSAGE_BOX_ERROR, "OK", NULL);

	  no_error = FALSE;
	}
      
    }


  // Check video settings if vid_tr is enables
  if (pw->vid_tr_changed)
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->vid_tr)))
	{
	  if (!GM_cam (gtk_entry_get_text (GTK_ENTRY (pw->video_device)),
		       (int) pw->video_channel_spin_adj->value))
	    
	    msg_box = gnome_message_box_new (_("Could not open the selected video device, but video transmission is enabled.\nGnomeMeeting will transmit a test image to the remote party during communications."), 
					     GNOME_MESSAGE_BOX_ERROR, "OK", NULL);
	}
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
  opts->sd = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->sd));

  /* Advanced Settings */
  opts->dnd = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->dnd));

  /* LDAP Settings */
  opts->ldap =  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->ldap));
  opts->ldap_server = gtk_entry_get_text (GTK_ENTRY (pw->ldap_server));
  opts->ldap_port = gtk_entry_get_text (GTK_ENTRY (pw->ldap_port));

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
	
  opts->vid_tr = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pw->vid_tr));

  
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
  opts->audio_device = gtk_entry_get_text (GTK_ENTRY (pw->audio_device));
  opts->audio_mixer = gtk_entry_get_text (GTK_ENTRY (pw->audio_mixer));
  opts->video_device = gtk_entry_get_text (GTK_ENTRY (pw->video_device));
  opts->video_channel = (int) pw->video_channel_spin_adj->value; 


  return opts;
}


int config_first_time (void)
{
  gnome_config_push_prefix ("gnomemeeting/");

  int res = gnome_config_get_int ("UserSettings/notfirst");

  gnome_config_sync();
  gnome_config_pop_prefix ();

  return (!res);
}


void init_config (void)
{
  
  gnome_config_push_prefix ("gnomemeeting/");
  gnome_config_set_int ("VideoSettings/video_size", 0);
  gnome_config_set_int ("VideoSettings/video_format", 2);
  gnome_config_set_int ("VideoSettings/transmitted_video_quality", 3);
  gnome_config_set_int ("VideoSettings/transmitted_fps", 15);
  gnome_config_set_int ("VideoSettings/received_video_quality", 3);
  gnome_config_set_int ("VideoSettings/transmitted_update_blocks", 2);
  gnome_config_set_int ("VideoSettings/video_transmission", 0);

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
  gnome_config_set_int ("GeneralSettings/incoming_call_sound", 1);

  gnome_config_set_int ("AdvancedSettings/enable_fast_start", 0);
  gnome_config_set_int ("AdvancedSettings/enable_h245_tunneling", 0); 	
  gnome_config_set_int ("AdvancedSettings/enable_auto_answer", 0);
  gnome_config_set_int ("AdvancedSettings/max_bps", 20000);
  gnome_config_set_int ("AdvancedSettings/silence_detection", 1);
  gnome_config_set_int ("AdvancedSettings/dnd", 0);

  gnome_config_set_int ("LDAPSettings/ldap", 0);
  gnome_config_set_string ("LDAPSettings/ldap_server", "");
  gnome_config_set_string ("LDAPSettings/ldap_port", "389");

  gnome_config_set_string ("EnabledAudio/LPC10", "0");
  gnome_config_set_string ("EnabledAudio/GSM-06.10", "0");
  gnome_config_set_string ("EnabledAudio/G.711-uLaw-64k", "1");
  gnome_config_set_string ("EnabledAudio/G.711-ALaw-64k", "1");
  gnome_config_set_string ("EnabledAudio/MS-GSM", "1");

  gnome_config_set_string ("Devices/audio_device", "/dev/dsp");
  gnome_config_set_string ("Devices/audio_mixer", "/dev/mixer");
#ifdef __linux__
  gnome_config_set_string ("Devices/video_device", "/dev/video");
#else
  gnome_config_set_string ("Devices/video_device", "/dev/bktr0");
#endif
  gnome_config_set_int ("Devices/video_channel", 0);

  gnome_config_set_string ("Placement/Dock", 
			   "Toolbar\\3,0,0,28\\Menubar\\0,0,0,0");
  gnome_config_sync();
  gnome_config_pop_prefix ();
}

/******************************************************************************/
