
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
 *                         druid.cpp  -  description
 *                         --------------------------
 *   begin                : Mon May 1 2002
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          build the druid.
 *   email                : dsandras@seconix.com
 */


#include "../config.h"

#include <iostream>
#include <gtk/gtk.h>
#include <ptlib.h>

#ifndef DISABLE_GNOME
#include <gnome.h>
#endif

#include <gconf/gconf-client.h>

#include "main_window.h"
#include "config.h"
#include "dialog.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "sound_handling.h"


#ifndef DISABLE_GNOME
/* Declarations */
static void audio_test_button_clicked (GtkWidget *, gpointer);
static void video_test_button_clicked (GtkWidget *, gpointer);
static void gnomemeeting_druid_quit (GtkWidget *, gpointer);
static void gnomemeeting_druid_cancel (GtkWidget *, gpointer);
static void gnomemeeting_druid_user_page_check (GnomeDruid *);
static void gnomemeeting_druid_toggle_changed (GtkToggleButton *, gpointer);
static void gnomemeeting_druid_radio_changed (GtkToggleButton *, gpointer);
static void gnomemeeting_druid_page_prepare (GnomeDruidPage *, GnomeDruid *,
					     gpointer);

static void gnomemeeting_init_druid_user_page (GnomeDruid *);
static void gnomemeeting_init_druid_audio_devices_page (GnomeDruid *); 
static void gnomemeeting_init_druid_video_devices_page (GnomeDruid *);
static void gnomemeeting_init_druid_connection_type_page (GnomeDruid *);


static GnomeDruid *druid;
extern GtkWidget *gm;
extern GnomeMeeting *MyApp;


/* GTK Callbacks */

static void
audio_test_button_clicked (GtkWidget *w, gpointer data)
{
  static GMAudioTester *t = NULL; /* Keep it static */
  GtkWindow *window = 
    (GtkWindow *) g_object_get_data (G_OBJECT (druid), "window");
  
  if (GTK_TOGGLE_BUTTON (w)->active) {
    
    if (t)
      delete (t);
    t = new GMAudioTester (MyApp->Endpoint (), window);
  }
  else {

    t->Stop ();
    t = NULL;
  }
}


static void
video_test_button_clicked (GtkWidget *w, gpointer data)
{
  GMVideoTester *t = NULL;
  GtkWindow *window = 
    (GtkWindow *) g_object_get_data (G_OBJECT (druid), "window");

  if (GTK_TOGGLE_BUTTON (w)->active)   
    t = new GMVideoTester ((GtkWidget *) data, window);
}


/* DESCRIPTION  :  This callback is called when the user clicks on finish.
 * BEHAVIOR     :  Shows GM, destroy the druid, and register to the ILS
 *                 directory if it works.
 * PRE          :  /
 */
static void 
gnomemeeting_druid_quit (GtkWidget *w, gpointer data)
{
  GtkWindow *window = NULL;
  GConfClient *client = NULL;
  GtkToggleButton *b = NULL; 

  client = gconf_client_get_default ();

  window = GTK_WINDOW (g_object_get_data (G_OBJECT (druid), "window"));
  b = GTK_TOGGLE_BUTTON (g_object_get_data (G_OBJECT (druid), "toggle"));

  if (gtk_toggle_button_get_active (b))
    gconf_client_set_bool (client, "/apps/gnomemeeting/ldap/register", 
			   false, NULL);
  else 
    gconf_client_set_bool (client, "/apps/gnomemeeting/ldap/register", 
			   true, NULL);

  b = GTK_TOGGLE_BUTTON (g_object_get_data (G_OBJECT (druid), "audio_test"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b), FALSE);

  b = GTK_TOGGLE_BUTTON (g_object_get_data (G_OBJECT (druid), "video_test"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b), FALSE);

  gconf_client_set_int (client, "/apps/gnomemeeting/general/version", 
			MAJOR_VERSION * 100 + MINOR_VERSION, 0);

  gconf_client_set_int (client, "/apps/gnomemeeting/general/kind_of_net",
			GPOINTER_TO_INT (g_object_get_data (G_OBJECT (druid),
							    "kind_of_net")),
			NULL);  

  gtk_widget_destroy (GTK_WIDGET (window));
  gtk_widget_show (gm);
}


/* DESCRIPTION  :  This callback is called when the user clicks on cancel.
 * BEHAVIOR     :  Exits. (Possible memory leak here with the structs).
 * PRE          :  /
 */
static void 
gnomemeeting_druid_cancel (GtkWidget *w, gpointer data)
{
  GtkWidget *window = NULL;

  window = (GtkWidget *) g_object_get_data (G_OBJECT (druid), "window");

  gtk_widget_destroy (GTK_WIDGET (window));

  /* Do not quit if we started the druid from the menu */
  if (strcmp ((gchar *) data, "menu"))
    gtk_main_quit ();
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Checks if the "Next" button of the "Personal Information"
 *                 druid page can be sensitive or not. It will if all fields
 *                 are ok, or if registering is disabled.
 * PRE          :  /
 */
static void 
gnomemeeting_druid_user_page_check (GnomeDruid *druid)
{
  GConfClient *client = NULL;
  gchar *gconf_string = NULL;
  GtkWidget *toggle = NULL;
  gboolean error = FALSE;

  /* Fetch data */
  toggle = (GtkWidget *) g_object_get_data (G_OBJECT (druid), "toggle");
  client = gconf_client_get_default ();


  /* We check that all fields are present, or the toggle desactivated */
  gconf_string = gconf_client_get_string (client, "/apps/gnomemeeting/personal_data/firstname", NULL);
  if ((gconf_string == NULL)||(!strcmp (gconf_string, "")))
    error = TRUE;
  g_free (gconf_string);

  gconf_string = gconf_client_get_string (client, "/apps/gnomemeeting/personal_data/lastname", NULL);
  if ((gconf_string == NULL)||(!strcmp (gconf_string, "")))
    error = TRUE;
  g_free (gconf_string);

  gconf_string = gconf_client_get_string (client, "/apps/gnomemeeting/personal_data/mail", NULL);
  if ((gconf_string == NULL)||(!strcmp (gconf_string, "")))
    error = TRUE;
  g_free (gconf_string);

  gconf_string = gconf_client_get_string (client, "/apps/gnomemeeting/personal_data/comment", NULL);
  if ((gconf_string == NULL)||(!strcmp (gconf_string, "")))
    error = TRUE;
  g_free (gconf_string);

  gconf_string = gconf_client_get_string (client, "/apps/gnomemeeting/personal_data/location", NULL);
  if ((gconf_string == NULL)||(!strcmp (gconf_string, "")))
    error = TRUE;
  g_free (gconf_string);


  if ((!error)||(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle))))
    gnome_druid_set_buttons_sensitive (druid, TRUE, TRUE, TRUE, FALSE);
  else
    gnome_druid_set_buttons_sensitive (druid, TRUE, FALSE, TRUE, FALSE);
}


/* DESCRIPTION  :  Called when the registering toggle changes in the Personal
 *                 Information page.
 * BEHAVIOR     :  Checks if the "Next" button of the "Personal Information"
 *                 druid page can be sensitive or not. It will if all fields
 *                 are ok, or if registering is disabled. (Calls the above
 *                 function).
 * PRE          :  /
 */
static void
gnomemeeting_druid_toggle_changed (GtkToggleButton *button, gpointer data)
{
  gnomemeeting_druid_user_page_check (druid);

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)))
    gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (gm), GTK_WIDGET (button), _("You chose to NOT use the GnomeMeeting ILS directory. Other users will not be able to contact you if you don't register to a directory service."));
}

					   
/* DESCRIPTION  :  Called when the radio button of the Connection page changes.
 * BEHAVIOR     :  Set default keys to good default settings.
 * PRE          :  /
 */
static void 
gnomemeeting_druid_radio_changed (GtkToggleButton *b, gpointer data)
{
  GConfClient *client = NULL;
  int selection = GPOINTER_TO_INT (data);

  client = gconf_client_get_default ();

  /* Dialup */
  if (selection == 1) {
    
    gconf_client_set_int (client, "/apps/gnomemeeting/video_settings/tr_fps",
			  1, NULL);
    gconf_client_set_int (client, "/apps/gnomemeeting/video_settings/tr_vq",
			  10, NULL);
    gconf_client_set_int (client, "/apps/gnomemeeting/video_settings/re_vq",
			  10, NULL);
    gconf_client_set_int (client, 
			  "/apps/gnomemeeting/audio_settings/jitter_buffer",
			  200, NULL);
    gconf_client_set_bool (client, 
			   "/apps/gnomemeeting/video_settings/enable_fps",
			   0, NULL);
    gconf_client_set_bool (client, 
			   "/apps/gnomemeeting/video_settings/enable_video_transmission", 0, NULL);
  }

  /* ISDN */
  if (selection == 2) {
    
    gconf_client_set_int (client, "/apps/gnomemeeting/video_settings/tr_fps",
			  1, NULL);
    gconf_client_set_int (client, "/apps/gnomemeeting/video_settings/tr_vq",
			  30, NULL);
    gconf_client_set_int (client, "/apps/gnomemeeting/video_settings/re_vq",
			  30, NULL);
    gconf_client_set_int (client, 
			  "/apps/gnomemeeting/audio_settings/jitter_buffer",
			  200, NULL);
    gconf_client_set_bool (client, 
			   "/apps/gnomemeeting/video_settings/enable_fps",
			   1, NULL);
    gconf_client_set_bool (client, 
			   "/apps/gnomemeeting/video_settings/enable_video_transmission", 1, NULL);
  }

  /* DSL / CABLE */
  if (selection == 3) {
    
    gconf_client_set_int (client, "/apps/gnomemeeting/video_settings/tr_fps",
			  6, NULL);
    gconf_client_set_int (client, "/apps/gnomemeeting/video_settings/tr_vq",
			  70, NULL);
    gconf_client_set_int (client, "/apps/gnomemeeting/video_settings/re_vq",
			  70, NULL);
    gconf_client_set_int (client, 
			  "/apps/gnomemeeting/audio_settings/jitter_buffer",
			  100, NULL);
    gconf_client_set_bool (client, 
			   "/apps/gnomemeeting/video_settings/enable_fps",
			   1, NULL);
    gconf_client_set_bool (client, 
			   "/apps/gnomemeeting/video_settings/enable_video_transmission", 1, NULL);
  }

  /* LAN */
  if (selection == 4) {
    
    gconf_client_set_int (client, "/apps/gnomemeeting/video_settings/tr_fps",
			  30, NULL);
    gconf_client_set_int (client, "/apps/gnomemeeting/video_settings/tr_vq",
			  100, NULL);
    gconf_client_set_int (client, "/apps/gnomemeeting/video_settings/re_vq",
			  100, NULL);
    gconf_client_set_int (client, 
			  "/apps/gnomemeeting/audio_settings/jitter_buffer",
			  50, NULL);
    gconf_client_set_bool (client, 
			   "/apps/gnomemeeting/video_settings/enable_fps",
			   0, NULL);
    gconf_client_set_bool (client, 
			   "/apps/gnomemeeting/video_settings/enable_video_transmission", 1, NULL);
  }

  /* Custom */
  /* We store the selected network in the druid. Storing right now in gconf 
     doesn't work, as it would be overwritten. We just write it when the user 
     closes the druid */
  g_object_set_data (G_OBJECT (druid), "kind_of_net", 
		     GINT_TO_POINTER (selection));
}


/* DESCRIPTION  :  Called when the user switches between the first two pages.
 * BEHAVIOR     :  Update the Next/Back buttons following the fields and the
 *                 page.
 * PRE          :  /
 */
static void
gnomemeeting_druid_page_prepare (GnomeDruidPage *page, GnomeDruid *druid,
				 gpointer data)
{
  if (!strcmp ((char *) data, "0"))
    gnome_druid_set_buttons_sensitive (druid, FALSE, TRUE, TRUE, FALSE);

  if (!strcmp ((char *) data, "1")) {

    gnome_druid_set_buttons_sensitive (druid, TRUE, FALSE, TRUE, FALSE);
    gnomemeeting_druid_user_page_check (druid);
  }
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the Personal Information.
 * PRE          :  /
 */
static void gnomemeeting_init_druid_user_page (GnomeDruid *druid)
{
  GtkWidget *vbox = NULL;
  GtkWidget *table = NULL;
  GtkWidget *label = NULL;
  GtkWidget *toggle = NULL;
  PangoAttrList     *attrs = NULL; 
  PangoAttribute    *attr = NULL; 
  GnomeDruidPageStandard *page_standard = NULL;

  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  gnome_druid_page_standard_set_title (page_standard, 
				       _("Personal Data - page 2/6"));
  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));

  /* Packing widgets */                                                        
  vbox = gtk_vbox_new (FALSE, 4);

  label = gtk_label_new (_("Please enter information about yourself. This information will be used when connecting to remote H.323 software, and to register in the directory of GnomeMeeting users. The directory is used to register users when they are using GnomeMeeting so that you can call them, and they can call you."));
  attrs = pango_attr_list_new ();
  attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
  attr->start_index = 0;
  attr->end_index = strlen (gtk_label_get_text (GTK_LABEL (label)));
  pango_attr_list_insert (attrs, attr);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_label_set_attributes (GTK_LABEL (label), attrs);
  pango_attr_list_unref (attrs);
  gtk_box_pack_start (GTK_BOX (vbox), label,                                   
                      TRUE, TRUE, 0);          
  
  table = gnomemeeting_vbox_add_table (vbox, _("Personal Information"), 6, 2);
  gnomemeeting_table_add_entry (table, _("First Name:"), "/apps/gnomemeeting/personal_data/firstname", NULL, 0);

  gnomemeeting_table_add_entry (table, _("Last Name:"), "/apps/gnomemeeting/personal_data/lastname", NULL, 1);
  gnomemeeting_table_add_entry (table, _("E-mail Address:"), "/apps/gnomemeeting/personal_data/mail", NULL, 2);
  gnomemeeting_table_add_entry (table, _("Comment:"), "/apps/gnomemeeting/personal_data/comment", NULL, 3);
  gnomemeeting_table_add_entry (table, _("Location:"), "/apps/gnomemeeting/personal_data/location", NULL, 4);

  toggle = gtk_check_button_new_with_label (_("Do not register me to ILS"));   
  gtk_table_attach (GTK_TABLE (table), toggle, 0, 2, 5, 6,
		    (GtkAttachOptions) NULL, 
		    (GtkAttachOptions) NULL,
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  g_object_set_data (G_OBJECT (druid), "toggle", (gpointer) toggle);

  g_signal_connect (G_OBJECT (toggle), "toggled",
		    G_CALLBACK (gnomemeeting_druid_toggle_changed), 
		    NULL);

  g_signal_connect_after (G_OBJECT (page_standard), "prepare",
			  G_CALLBACK (gnomemeeting_druid_page_prepare), 
			  (gpointer) "1");
  
  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the Connection page.
 * PRE          :  /
 */
static void gnomemeeting_init_druid_connection_type_page (GnomeDruid *druid)
{
  GtkWidget *vbox = NULL;
  GtkWidget *box = NULL;
  GtkWidget *table = NULL;
  GtkWidget *label = NULL;
  GtkWidget *radio1 = NULL;
  GtkWidget *radio2 = NULL;
  GtkWidget *radio3 = NULL;
  GtkWidget *radio4 = NULL;
  GtkWidget *radio5 = NULL;
  GConfClient *client = NULL;

  PangoAttrList     *attrs = NULL; 
  PangoAttribute    *attr = NULL; 

  GnomeDruidPageStandard *page_standard = NULL;


  client = gconf_client_get_default ();

  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  gnome_druid_page_standard_set_title (page_standard, 
				       _("Connection Type - page 3/6"));
  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));


  /* Packing widgets */                                                        
  vbox = gtk_vbox_new (FALSE, 4);
  box = gtk_vbox_new (FALSE, 2);

  label = gtk_label_new (_("Please enter your connection type. This setting is used to set default settings following your bandwidth. It will set good global settings but it is however possible to tweak and change some of them to obtain a better result in each particular case."));
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  attrs = pango_attr_list_new ();
  attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
  attr->start_index = 0;
  attr->end_index = strlen (gtk_label_get_text (GTK_LABEL (label)));
  pango_attr_list_insert (attrs, attr);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_label_set_attributes (GTK_LABEL (label), attrs);
  pango_attr_list_unref (attrs);
  gtk_box_pack_start (GTK_BOX (vbox), label,                                   
                      TRUE, TRUE, 0);
  
  /* Connection type */
  table = gnomemeeting_vbox_add_table (vbox, _("Connection Type"), 1, 1);
  radio1 = gtk_radio_button_new_with_label (NULL, _("56K modem"));
  gtk_box_pack_start (GTK_BOX (box), radio1, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (radio1), "toggled",
		    G_CALLBACK (gnomemeeting_druid_radio_changed), 
		    GINT_TO_POINTER (1));

  radio2 = 
    gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio1), 
						 _("ISDN"));
  gtk_box_pack_start (GTK_BOX (box), radio2, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (radio2), "toggled",
		    G_CALLBACK (gnomemeeting_druid_radio_changed), 
		    GINT_TO_POINTER (2));

  radio3 = 
    gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio1), 
						 _("DSL/Cable"));
  gtk_box_pack_start (GTK_BOX (box), radio3, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (radio3), "toggled",
		    G_CALLBACK (gnomemeeting_druid_radio_changed), 
		    GINT_TO_POINTER (3));

  radio4 = 
    gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio1), 
						 _("T1/LAN"));
  gtk_box_pack_start (GTK_BOX (box), radio4, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (radio4), "toggled",
		    G_CALLBACK (gnomemeeting_druid_radio_changed), 
		    GINT_TO_POINTER (4));

  radio5= 
    gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio1), 
						 _("Custom"));
  gtk_box_pack_start (GTK_BOX (box), radio5, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (radio5), "toggled",
		    G_CALLBACK (gnomemeeting_druid_radio_changed), 
		    GINT_TO_POINTER (5));

  gtk_table_attach (GTK_TABLE (table), box, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);


  int net_selected = gconf_client_get_int (client, 
					   "/apps/gnomemeeting/general/kind_of_net", NULL);
  GtkWidget *selected_button = radio1;
  
  switch (net_selected) {

  case 1:
    selected_button = radio1; break;
  case 2:
    selected_button = radio2; break;
  case 3:
    selected_button = radio3; break;
  case 4:
    selected_button = radio4; break;
  case 5:
    selected_button = radio5; break;
  default: net_selected = 1;
  
  }

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (selected_button), true); 

  /* Write the appropiate defaults */
  gnomemeeting_druid_radio_changed (GTK_TOGGLE_BUTTON (selected_button), 
				    GINT_TO_POINTER (net_selected));
  
  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the audio devices configuration.
 * PRE          :  /
 */
static void 
gnomemeeting_init_druid_audio_devices_page (GnomeDruid *druid)
{
  GtkWidget *vbox = NULL;
  GtkWidget *table = NULL;
  GtkWidget *button = NULL;
  GtkWidget *audio_recorder = NULL;
  GtkWidget *audio_player = NULL;
  GtkWidget *label = NULL;

  PangoAttrList     *attrs = NULL; 
  PangoAttribute    *attr = NULL; 

  gchar *audio_player_devices_list [20];
  gchar *audio_recorder_devices_list [20];
  int i = 0;

  GnomeDruidPageStandard *page_standard = NULL;

  GmWindow *gw = gnomemeeting_get_main_window (gm);

  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  gnome_druid_page_standard_set_title (page_standard, _("Audio Devices - page 4/6"));
  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));

  /* Packing widgets */
  vbox = gtk_vbox_new (FALSE, 4);

  label = gtk_label_new (_("Please choose the audio devices to use during the GnomeMeeting session. You can also choose to use a Quicknet device instead of the soundcard(s). Some webcams models have an internal microphone that can be used with GnomeMeeting."));
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  attrs = pango_attr_list_new ();
  attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
  attr->start_index = 0;
  attr->end_index = strlen (gtk_label_get_text (GTK_LABEL (label)));
  pango_attr_list_insert (attrs, attr);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_label_set_attributes (GTK_LABEL (label), attrs);
  pango_attr_list_unref (attrs);
  gtk_box_pack_start (GTK_BOX (vbox), label,                                   
                      TRUE, TRUE, 0);


  /* The Audio player */
  table = gnomemeeting_vbox_add_table (vbox, _("Audio Devices"), 3, 3);
  i = gw->audio_player_devices.GetSize () - 1;
  if (i >= 20) i = 19;

  for (int j = i ; j >= 0; j--) 
    if (strcmp (gw->audio_player_devices [j], "loopback"))
      audio_player_devices_list [j] = g_strdup (gw->audio_player_devices [j]);
    else
      audio_player_devices_list [j] = NULL;

  
  audio_player_devices_list [i+1] = NULL;
  
  audio_player = 
    gnomemeeting_table_add_string_option_menu (table, _("Audio Player:"), audio_player_devices_list, "/apps/gnomemeeting/devices/audio_player", _("Enter the audio player device to use."), 0);

  for (int j = i ; j >= 0; j--) 
    g_free (audio_player_devices_list [j]);


  /* The audio recorder */
  i = gw->audio_recorder_devices.GetSize () - 1;
  if (i >= 20) i = 19;

  for (int j = i ; j >= 0; j--) 
    if (strcmp (gw->audio_recorder_devices [j], "loopback"))
      audio_recorder_devices_list [j] = 
	g_strdup (gw->audio_recorder_devices [j]);
    else
      audio_recorder_devices_list [j] = NULL;
  
  audio_recorder_devices_list [i + 1] = NULL;

  audio_recorder = 
    gnomemeeting_table_add_string_option_menu (table, _("Audio Recorder:"), audio_recorder_devices_list, "/apps/gnomemeeting/devices/audio_recorder", _("Enter the audio recorder device to use."), 1);
  
  for (int j = i ; j >= 0; j--) 
    g_free (audio_recorder_devices_list [j]);


  /* Test button */
  label = gtk_label_new (_("Click here to test your audio devices:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 2, 2, 3,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    0, 0);

  button = gtk_toggle_button_new_with_label (_("Voice echo"));
  gtk_table_attach (GTK_TABLE (table), button, 2, 3, 2, 3,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    0, 0);
  g_signal_connect (G_OBJECT (button), "clicked",
		    GTK_SIGNAL_FUNC (audio_test_button_clicked),
		    (gpointer) druid);
  g_object_set_data (G_OBJECT (druid), "audio_test", button);

  /**/
  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the video devices configuration.
 * PRE          :  /
 */
static void 
gnomemeeting_init_druid_video_devices_page (GnomeDruid *druid)
{
  GtkWidget *vbox = NULL;
  GtkWidget *table = NULL;
  GtkWidget *button = NULL;
  GtkWidget *video_device = NULL;
  GtkWidget *label = NULL;
  GtkWidget *progress = NULL;

  PangoAttrList     *attrs = NULL; 
  PangoAttribute    *attr = NULL; 

  gchar *video_devices [20];
  int i = 0;

  GnomeDruidPageStandard *page_standard = NULL;

  GmWindow *gw = gnomemeeting_get_main_window (gm);

  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  gnome_druid_page_standard_set_title (page_standard, _("Video Devices - page 5/6"));
  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));

  /* Packing widgets */
  vbox = gtk_vbox_new (FALSE, 4);

  label = gtk_label_new (_("Please choose the video device to use during the GnomeMeeting session. Click on the Video Test button to check if your setup is correct and if your driver is supported by GnomeMeeting."));
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  attrs = pango_attr_list_new ();
  attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
  attr->start_index = 0;
  attr->end_index = strlen (gtk_label_get_text (GTK_LABEL (label)));
  pango_attr_list_insert (attrs, attr);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_label_set_attributes (GTK_LABEL (label), attrs);
  pango_attr_list_unref (attrs);
  gtk_box_pack_start (GTK_BOX (vbox), label,                                   
                      TRUE, TRUE, 0);


  /* The Video device */
  table = gnomemeeting_vbox_add_table (vbox, _("Video Devices"), 3, 3);
  i = gw->video_devices.GetSize () - 1;
  if (i >= 20) i = 19;

  for (int j = i ; j >= 0; j--) 
    video_devices [j] = g_strdup (gw->video_devices [j]);
  
  video_devices [i+1] = NULL;
  
  video_device = 
    gnomemeeting_table_add_string_option_menu (table, _("Video Device:"), video_devices, "/apps/gnomemeeting/devices/video_recorder", _("Enter the video device to use. Using an invalid video device for video transmission will transmit a test picture."), 0);
  
  for (int j = i ; j >= 0; j--) 
    g_free (video_devices [j]);

  /* Test button */
  label = 
    gtk_label_new (_("Click here to test your video device conformity:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 2, 1, 2,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    0, 0);

  button = gtk_toggle_button_new_with_label (_("Video Test"));
  gtk_table_attach (GTK_TABLE (table), button, 2, 3, 1, 2,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    0, 0);

  /* Progress bar */
  progress = gtk_progress_bar_new ();
  gtk_table_attach (GTK_TABLE (table), progress, 0, 3, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    10, 10);

  g_signal_connect (G_OBJECT (button), "clicked",
		    GTK_SIGNAL_FUNC (video_test_button_clicked),
		    (gpointer) progress);
  g_object_set_data (G_OBJECT (druid), "video_test", button);

  /**/
  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}
#endif


/* Functions */
void gnomemeeting_init_druid (gpointer data)
{
#ifndef DISABLE_GNOME
  GtkWidget *window = NULL;
  GnomeDruidPageEdge *page_edge = NULL;
  GnomeDruidPageEdge *page_final = NULL;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), 
			_("First Time Configuration Druid"));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  druid = GNOME_DRUID (gnome_druid_new ());

  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (druid));

  g_object_set_data (G_OBJECT (druid), "window", window);

  static const gchar title[] = 
    N_("Configuration Assistant - page 1/6");
  static const gchar text[] =
    N_
    ("Welcome to the GnomeMeeting general configuration druid. "
     "The following steps will set up GnomeMeeting by asking "
     "a few simple questions. Once you have completed "
     "these steps, you can always change them later in "
     "the preferences. ");
  static const gchar bye[] =
    N_("You have successfully set up GnomeMeeting. Enjoy!\n"
       "   -- The GnomeMeeting development team");

  /* Create the first page */
  page_edge =
    GNOME_DRUID_PAGE_EDGE (gnome_druid_page_edge_new
			   (GNOME_EDGE_START));
  gnome_druid_page_edge_set_title (page_edge, _(title));
  gnome_druid_page_edge_set_text (page_edge, _(text));

  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_edge));
  gnome_druid_set_page (druid, GNOME_DRUID_PAGE (page_edge));
  
  g_signal_connect_after (G_OBJECT (page_edge), "prepare",
			  G_CALLBACK (gnomemeeting_druid_page_prepare), 
			  (gpointer) "0");

  /* Create the user page */
  gnomemeeting_init_druid_user_page (druid);
  
  /* Create connection type */
  gnomemeeting_init_druid_connection_type_page (druid);
  
  /* Create the devices pages */
  gnomemeeting_init_druid_audio_devices_page (druid);
  gnomemeeting_init_druid_video_devices_page (druid);

  /* Create final page */
  page_final =
    GNOME_DRUID_PAGE_EDGE (gnome_druid_page_edge_new (GNOME_EDGE_FINISH));
  
  gnome_druid_page_edge_set_title (page_final, _("Done! - page 6/6"));
  gnome_druid_page_edge_set_text (page_final, _(bye));
  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_final));
  
  g_signal_connect (G_OBJECT (page_final), "finish",
		    G_CALLBACK (gnomemeeting_druid_quit), druid);

  g_signal_connect (G_OBJECT (druid), "cancel",
		    G_CALLBACK (gnomemeeting_druid_cancel), data);

  gtk_widget_show_all (window);
#endif
}

