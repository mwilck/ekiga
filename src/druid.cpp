
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

#include "gnomemeeting.h"
#include "dialog.h"
#include "misc.h"
#include "videograbber.h"
#include "stock-icons.h"
#include "callbacks.h"


#ifndef DISABLE_GNOME
/* Declarations */
static void audio_test_button_clicked (GtkWidget *, gpointer);
static void video_test_button_clicked (GtkWidget *, gpointer);
static void gnomemeeting_druid_add_graphical_label (GtkWidget *, gchar *, 
						    gchar *);
static void gnomemeeting_druid_quit (GtkWidget *, gpointer);
static void gnomemeeting_druid_destroy (GtkWidget *, GdkEventAny *, gpointer);
static void gnomemeeting_druid_cancel (GtkWidget *, gpointer);
static void gnomemeeting_druid_user_page_check (GnomeDruid *);
static void gnomemeeting_druid_toggle_changed (GtkToggleButton *, gpointer);
static void gnomemeeting_druid_entry_changed (GtkWidget *, gpointer);
static void gnomemeeting_druid_ixj_account_changed (GtkWidget *, gpointer);
static void gnomemeeting_druid_radio_changed (GtkToggleButton *, gpointer);
static void gnomemeeting_druid_page_prepare (GnomeDruidPage *, GnomeDruid *,
					     gpointer);
static void gnomemeeting_druid_final_page_prepare (GnomeDruid *);

static void gnomemeeting_init_druid_user_page (GnomeDruid *, int, int);
static void gnomemeeting_init_druid_audio_devices_page (GnomeDruid *, 
							int, int); 
static void gnomemeeting_init_druid_ixj_device_page (GnomeDruid *, int, int);
static void gnomemeeting_init_druid_connection_type_page (GnomeDruid *, 
							  int, int);

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;


/* GTK Callbacks */

static void
audio_test_button_clicked (GtkWidget *w, gpointer data)
{
  if (GTK_TOGGLE_BUTTON (w)->active) {
    
    MyApp->Endpoint ()->StartAudioTester ();
  }
  else {

    MyApp->Endpoint ()->StopAudioTester ();
  }
}


static void
video_test_button_clicked (GtkWidget *w, gpointer data)
{
  GMVideoTester *t = NULL;
  GmWindow *gw = 
    gnomemeeting_get_main_window (gm);
  GtkWindow *window = 
    (GtkWindow *) g_object_get_data (G_OBJECT (gw->druid), "window");

  if (GTK_TOGGLE_BUTTON (w)->active)   
    t = new GMVideoTester ((GtkWidget *) data, w, window);
}


static void
gnomemeeting_druid_add_graphical_label (GtkWidget *vbox, gchar *stock, 
					gchar *label_text)
{
  GtkWidget *hbox = NULL;
  GtkWidget *label = NULL;
  GtkWidget *image = NULL;

  PangoAttrList     *attrs = NULL; 
  PangoAttribute    *attr = NULL; 

  /* Packing widgets */                                                        
  hbox = gtk_hbox_new (FALSE, 20);

  image = gtk_image_new_from_stock (stock, GTK_ICON_SIZE_DIALOG);

  label = gtk_label_new (label_text);
  attrs = pango_attr_list_new ();
  attr = pango_attr_style_new (PANGO_STYLE_ITALIC);
  attr->start_index = 0;
  attr->end_index = strlen (gtk_label_get_text (GTK_LABEL (label)));
  pango_attr_list_insert (attrs, attr);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_label_set_attributes (GTK_LABEL (label), attrs);
  pango_attr_list_unref (attrs);

  gtk_box_pack_start (GTK_BOX (hbox), image,                                   
                      TRUE, TRUE, 0);      
  gtk_box_pack_start (GTK_BOX (hbox), label,                                   
                      TRUE, TRUE, 0);          
  gtk_box_pack_start (GTK_BOX (vbox), hbox,                                   
                      TRUE, TRUE, 0);          
}



/* DESCRIPTION  :  This callback is called when the user clicks on finish.
 * BEHAVIOR     :  Destroys the druid and update gconf settings.
 * PRE          :  /
 */
static void 
gnomemeeting_druid_quit (GtkWidget *w, gpointer data)
{
  GtkWindow *window = NULL;
  GConfClient *client = NULL;
  GtkToggleButton *b = NULL; 
  gchar *gconf_string = NULL;
  gchar *gk_name = NULL;

  GmWindow *gw = NULL;

  client = gconf_client_get_default ();
  gw = gnomemeeting_get_main_window (gm);

  window = GTK_WINDOW (g_object_get_data (G_OBJECT (gw->druid), "window"));
  b = GTK_TOGGLE_BUTTON (g_object_get_data (G_OBJECT (gw->druid), "toggle"));

  if (gtk_toggle_button_get_active (b))
    gconf_client_set_bool (client, LDAP_KEY "register", false, NULL);
  else 
    gconf_client_set_bool (client, LDAP_KEY "register", true, NULL);

  b = GTK_TOGGLE_BUTTON (g_object_get_data (G_OBJECT (gw->druid), 
					    "audio_test"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b), FALSE);

  b = GTK_TOGGLE_BUTTON (g_object_get_data (G_OBJECT (gw->druid), 
					    "video_test"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b), FALSE);

  gconf_client_set_int (client, GENERAL_KEY "version", 
			MAJOR_VERSION * 100 + MINOR_VERSION, 0);

  gconf_client_set_int (client, GENERAL_KEY "kind_of_net",
			GPOINTER_TO_INT (g_object_get_data (G_OBJECT (gw->druid),
							    "kind_of_net")),
			NULL);  

  b = GTK_TOGGLE_BUTTON (g_object_get_data (G_OBJECT (gw->druid), 
					    "ixj_toggle"));
  if (gtk_toggle_button_get_active (b)) {

    gconf_string = 
      gconf_client_get_string (client, GATEKEEPER_KEY "gk_alias" , NULL);
    
    if (gconf_string)
      gk_name = g_strdup_printf ("%s.cce.microtelco.com", gconf_string);
      
    if (gk_name) {

      gconf_client_set_string (client, GATEKEEPER_KEY "gk_host", gk_name, 0);
      gconf_client_set_int (client, GATEKEEPER_KEY "registering_method", 1, 0);

      MyApp->Endpoint ()->GatekeeperRegister ();
    }

    g_free (gconf_string);   
    g_free (gk_name);
  }


  gw->druid = NULL;

  gtk_widget_destroy (GTK_WIDGET (window));
  gtk_widget_show (gm);
}


/* DESCRIPTION  :  This callback is called when the user destroys the druid.
 * BEHAVIOR     :  Exits. 
 * PRE          :  /
 */
static void 
gnomemeeting_druid_destroy (GtkWidget *w, GdkEventAny *ev, gpointer data)
{
  gnomemeeting_druid_cancel (w, data);
}


/* DESCRIPTION  :  This callback is called when the user clicks on cancel.
 * BEHAVIOR     :  Exits. 
 * PRE          :  /
 */
static void 
gnomemeeting_druid_cancel (GtkWidget *w, gpointer data)
{
  GtkWidget *window = NULL;
  GmWindow *gw = gnomemeeting_get_main_window (gm);

  window = (GtkWidget *) g_object_get_data (G_OBJECT (gw->druid), "window");

  gtk_widget_destroy (GTK_WIDGET (window));
  gw->druid = NULL;

  /* Do not quit if we started the druid from the menu */
  if (strcmp ((gchar *) data, "menu"))
    quit_callback (NULL, gw);
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
  gconf_string = 
    gconf_client_get_string (client, PERSONAL_DATA_KEY "firstname", NULL);
  if ((gconf_string == NULL)||(!strcmp (gconf_string, "")))
    error = TRUE;
  g_free (gconf_string);

  gconf_string = 
    gconf_client_get_string (client, PERSONAL_DATA_KEY "lastname", NULL);
  if ((gconf_string == NULL)||(!strcmp (gconf_string, "")))
    error = TRUE;
  g_free (gconf_string);

  gconf_string = 
    gconf_client_get_string (client, PERSONAL_DATA_KEY "mail", NULL);
  if ((gconf_string == NULL)||(!strcmp (gconf_string, "")))
    error = TRUE;
  g_free (gconf_string);

  gconf_string = 
    gconf_client_get_string (client, PERSONAL_DATA_KEY "comment", NULL);
  if ((gconf_string == NULL)||(!strcmp (gconf_string, "")))
    error = TRUE;
  g_free (gconf_string);

  gconf_string = 
    gconf_client_get_string (client, PERSONAL_DATA_KEY "location", NULL);
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
  GmWindow *gw = NULL;

  gw = gnomemeeting_get_main_window (gm);

  gnomemeeting_druid_user_page_check (gw->druid);

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)))
    gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (gm), GTK_WIDGET (button), _("You chose to NOT use the GnomeMeeting ILS directory. Other users will not be able to contact you if you don't register to a directory service."));
}


/* DESCRIPTION  :  Called when the user changes an info in the Personal
 *                 Information page.
 * BEHAVIOR     :  Checks if the "Next" button of the "Personal Information"
 *                 druid page can be sensitive or not. It will if all fields
 *                 are ok, or if registering is disabled. (Calls the above
 *                 function).
 * PRE          :  /
 */
static void
gnomemeeting_druid_entry_changed (GtkWidget *w, gpointer data)
{
  GmWindow *gw = NULL;

  gw = gnomemeeting_get_main_window (gm);

  gnomemeeting_druid_user_page_check (gw->druid);
}


/* DESCRIPTION  :  Called when the user changes his account info in the 
 *                 PC-To-Phone setup.
 * BEHAVIOR     :  Updates the URL.
 * PRE          :  /
 */
static void
gnomemeeting_druid_ixj_account_changed (GtkWidget *w, gpointer data)
{
  GmWindow *gw = NULL;
  GtkWidget *href = NULL;
  
  const gchar *entry_text = NULL;
  gchar *url = NULL;

  gw = gnomemeeting_get_main_window (gm);

  href = 
    GTK_WIDGET (g_object_get_data (G_OBJECT (gw->druid), "ixj_href"));

  entry_text = gtk_entry_get_text (GTK_ENTRY (w));
  
  url = g_strdup_printf ("http://%s.an.microtelco.com", entry_text);

  gnome_href_set_url (GNOME_HREF (href), url);
  g_free (url);
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

  GmWindow *gw = NULL;

  gw = gnomemeeting_get_main_window (gm);
  client = gconf_client_get_default ();

  /* Dialup */
  if (selection == 1) {
    
    gconf_client_set_int (client, VIDEO_SETTINGS_KEY "tr_fps", 1, NULL);
    gconf_client_set_int (client, VIDEO_SETTINGS_KEY "tr_vq", 1, NULL);
    gconf_client_set_int (client, 
			  VIDEO_SETTINGS_KEY "maximum_video_bandwidth", 
			  1, NULL);
    gconf_client_set_int (client, VIDEO_SETTINGS_KEY "re_vq", 10, NULL);
    gconf_client_set_bool (client, 
			   VIDEO_SETTINGS_KEY "enable_video_transmission", 
			   0, NULL);
    gconf_client_set_bool (client, 
			   VIDEO_SETTINGS_KEY "enable_video_reception", 
			   0, NULL);
 }

  /* ISDN */
  if (selection == 2) {
    
    gconf_client_set_int (client, VIDEO_SETTINGS_KEY "tr_fps", 1, NULL);
    gconf_client_set_int (client, VIDEO_SETTINGS_KEY "tr_vq", 10, NULL);
    gconf_client_set_int (client, 
			  VIDEO_SETTINGS_KEY "maximum_video_bandwidth", 
			  2, NULL);
    gconf_client_set_int (client, VIDEO_SETTINGS_KEY "re_vq", 30, NULL);
    gconf_client_set_bool (client, 
			   VIDEO_SETTINGS_KEY "enable_video_transmission", 
			   1, NULL);
    gconf_client_set_bool (client, 
			   VIDEO_SETTINGS_KEY "enable_video_reception", 
			   1, NULL);
  }

  /* DSL / CABLE */
  if (selection == 3) {
    
    gconf_client_set_int (client, VIDEO_SETTINGS_KEY "tr_fps", 6, NULL);
    gconf_client_set_int (client, VIDEO_SETTINGS_KEY "tr_vq", 50, NULL);
    gconf_client_set_int (client, VIDEO_SETTINGS_KEY "re_vq", 70, NULL);
    gconf_client_set_int (client, 
			  VIDEO_SETTINGS_KEY "maximum_video_bandwidth", 
			  7, NULL);
    gconf_client_set_bool (client, 
			   VIDEO_SETTINGS_KEY "enable_video_transmission", 
			   1, NULL);
    gconf_client_set_bool (client, 
			   VIDEO_SETTINGS_KEY "enable_video_reception", 
			   1, NULL);
}

  /* LAN */
  if (selection == 4) {
    
    gconf_client_set_int (client, VIDEO_SETTINGS_KEY "tr_fps", 10, NULL);
    gconf_client_set_int (client, VIDEO_SETTINGS_KEY "tr_vq", 70, NULL);
    gconf_client_set_int (client, VIDEO_SETTINGS_KEY "re_vq", 70, NULL);
    gconf_client_set_int (client, 
			  VIDEO_SETTINGS_KEY "maximum_video_bandwidth", 
			  80, NULL);
    gconf_client_set_bool (client, 
			   VIDEO_SETTINGS_KEY "enable_video_transmission", 
			   1, NULL);
    gconf_client_set_bool (client, 
			   VIDEO_SETTINGS_KEY "enable_video_reception", 
			   1, NULL);
  }

  /* Custom */
  /* We store the selected network in the druid. Storing right now in gconf 
     doesn't work, as it would be overwritten. We just write it when the user 
     closes the druid */
  g_object_set_data (G_OBJECT (gw->druid), "kind_of_net", 
		     GINT_TO_POINTER (selection));
}


/* DESCRIPTION  :  Called when the user switches between the pages 1, 2, and 6.
 * BEHAVIOR     :  Update the Next/Back buttons following the fields and the
 *                 page. Updates the text of the last page.
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

  if (!strcmp ((char *) data, "7")) 
    gnomemeeting_druid_final_page_prepare (druid);
}


static void
gnomemeeting_druid_final_page_prepare (GnomeDruid *druid)
{
  GnomeDruidPageEdge *page_final = 
    GNOME_DRUID_PAGE_EDGE (g_object_get_data (G_OBJECT (druid), "page_final"));

  gchar *text = NULL;
  gchar *kind_of_net_name = NULL;
  gchar *audio_recorder = NULL;
  gchar *audio_player = NULL;
  gchar *video_recorder = NULL;
  gchar *firstname = NULL;
  gchar *lastname = NULL;
  BOOL reg = TRUE;
  
  GConfClient *client = gconf_client_get_default ();

  switch (GPOINTER_TO_INT (g_object_get_data (G_OBJECT (druid), 
					      "kind_of_net"))) {
  case 1:
      kind_of_net_name = g_strdup (_("56K modem"));
      break;

  case 2:
    kind_of_net_name = g_strdup (_("ISDN"));
    break;

  case 3:
    kind_of_net_name = g_strdup (_("DSL/Cable"));
    break;

  case 4:
    kind_of_net_name = g_strdup (_("T1/LAN"));
    break;

  case 5:
    kind_of_net_name = g_strdup (_("Custom"));
    break;
  }

  firstname =
    gconf_client_get_string (client, PERSONAL_DATA_KEY "firstname", NULL);
  lastname =
    gconf_client_get_string (client, PERSONAL_DATA_KEY "lastname", NULL);
  audio_player =
    gconf_client_get_string (client, DEVICES_KEY "audio_player", NULL);
  audio_recorder =
    gconf_client_get_string (client, DEVICES_KEY "audio_recorder", NULL);
  video_recorder =
    gconf_client_get_string (client, DEVICES_KEY "video_recorder", NULL);

  if (!firstname)
    firstname = g_strdup ("");
  if (!lastname)
    lastname = g_strdup ("");
  if (!audio_player)
    audio_player = g_strdup ("");
  if (!audio_recorder)
    audio_recorder = g_strdup ("");
  if (!video_recorder)
    video_recorder = g_strdup ("");
	
  reg = 
    !GTK_TOGGLE_BUTTON (g_object_get_data (G_OBJECT (druid), "toggle"))->active;

  text = g_strdup_printf ("You have now finished the GnomeMeeting configuration. All the settings can be changed in the GnomeMeeting preferences. Enjoy!\n\n\nChanges Summary:\n\nUsername:  %s %s\nConnection Type:  %s\nAudio Player:  %s\nAudio Recorder:  %s\nVideo Player: %s\nRegistering to ILS:  %s", firstname, lastname, kind_of_net_name, audio_player, audio_recorder, video_recorder, reg?"Enabled":"Disabled");
  gnome_druid_page_edge_set_text (page_final, text);
    
  g_free (text);
  g_free (firstname);
  g_free (lastname);
  g_free (audio_player);
  g_free (audio_recorder);
  g_free (video_recorder);
}

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the Personal Information.
 * PRE          :  /
 */
static void 
gnomemeeting_init_druid_user_page (GnomeDruid *druid, int p, int t)
{
  GtkWidget *vbox = NULL;
  GtkWidget *table = NULL;
  GtkWidget *toggle = NULL;
  GtkWidget *entry = NULL;
  gchar *title = NULL;

  GnomeDruidPageStandard *page_standard = NULL;

  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());

  title = g_strdup_printf (_("Personal Data - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);
				
  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));

  
  /* Packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);
  gnomemeeting_druid_add_graphical_label (vbox, GM_STOCK_DRUID_PERSONAL, _("Please enter information about yourself. This information will be used when connecting to remote H.323 software, and to register in the directory of GnomeMeeting users. The directory is used to register online users and is an easy way to find your friends."));
					  
  table = gnomemeeting_vbox_add_table (vbox, _("Personal Information"), 6, 2);


  /* The user fields */
  entry = 
    gnomemeeting_table_add_entry (table, _("First Name:"), 
				  PERSONAL_DATA_KEY "firstname", NULL, 0);

  entry = 
    gnomemeeting_table_add_entry (table, _("Last Name:"), 
				  PERSONAL_DATA_KEY "lastname", NULL, 1);

  entry = 
    gnomemeeting_table_add_entry (table, _("E-mail Address:"), 
				  PERSONAL_DATA_KEY "mail", NULL, 2);

  entry = 
    gnomemeeting_table_add_entry (table, _("Comment:"), 
				  PERSONAL_DATA_KEY "comment", NULL, 3);

  entry = 
    gnomemeeting_table_add_entry (table, _("Location:"), 
				  PERSONAL_DATA_KEY "location", NULL, 4);

  g_signal_connect (G_OBJECT (entry), "changed",
		    G_CALLBACK (gnomemeeting_druid_entry_changed), 
		    NULL);
  

  /* The register toggle */
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
static void 
gnomemeeting_init_druid_connection_type_page (GnomeDruid *druid, int p, int t)
{
  GtkWidget *vbox = NULL;
  GtkWidget *box = NULL;
  GtkWidget *table = NULL;
  GtkWidget *radio1 = NULL;
  GtkWidget *radio2 = NULL;
  GtkWidget *radio3 = NULL;
  GtkWidget *radio4 = NULL;
  GtkWidget *radio5 = NULL;
  gchar *title = NULL;

  GConfClient *client = NULL;

  GnomeDruidPageStandard *page_standard = NULL;


  client = gconf_client_get_default ();

  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());

  title = g_strdup_printf (_("Connection Type - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));


  /* Packing widgets */                                                        
  vbox = gtk_vbox_new (FALSE, 2);
  box = gtk_vbox_new (FALSE, 2);

  gnomemeeting_druid_add_graphical_label (vbox, GM_STOCK_DRUID_CONNECTION, _("Please enter your connection type. This setting is used to set default settings following your bandwidth. It will set good global settings but it is however possible to tweak and change them later to obtain a better result in each particular case."));

  
  /* Connection type */
  table = gnomemeeting_vbox_add_table (vbox, _("Connection Type"), 1, 1);
  radio1 = gtk_radio_button_new_with_label (NULL, _("56K modem"));
  gtk_box_pack_start (GTK_BOX (box), radio1, TRUE, TRUE, 0);
  radio2 = 
    gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio1), 
						 _("ISDN"));
  gtk_box_pack_start (GTK_BOX (box), radio2, TRUE, TRUE, 0);
  radio3 = 
    gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio1), 
						 _("DSL/Cable"));
  gtk_box_pack_start (GTK_BOX (box), radio3, TRUE, TRUE, 0);
  radio4 = 
    gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio1), 
						 _("T1/LAN"));
  gtk_box_pack_start (GTK_BOX (box), radio4, TRUE, TRUE, 0);
  radio5= 
    gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio1), 
						 _("Custom"));
  gtk_box_pack_start (GTK_BOX (box), radio5, TRUE, TRUE, 0);

  gtk_table_attach (GTK_TABLE (table), box, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  
  /* Type of network */
  int net_selected = 
    gconf_client_get_int (client, GENERAL_KEY "kind_of_net", NULL);
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


  /* Update the buttons to the right value and connect signals */
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (selected_button), true); 
  g_signal_connect (G_OBJECT (radio1), "toggled",
		    G_CALLBACK (gnomemeeting_druid_radio_changed), 
		    GINT_TO_POINTER (1));
  g_signal_connect (G_OBJECT (radio2), "toggled",
		    G_CALLBACK (gnomemeeting_druid_radio_changed), 
		    GINT_TO_POINTER (2));
  g_signal_connect (G_OBJECT (radio3), "toggled",
		    G_CALLBACK (gnomemeeting_druid_radio_changed), 
		    GINT_TO_POINTER (3));
  g_signal_connect (G_OBJECT (radio4), "toggled",
		    G_CALLBACK (gnomemeeting_druid_radio_changed), 
		    GINT_TO_POINTER (4));
  g_signal_connect (G_OBJECT (radio5), "toggled",
		    G_CALLBACK (gnomemeeting_druid_radio_changed), 
		    GINT_TO_POINTER (5));

  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the audio devices configuration.
 * PRE          :  /
 */
static void 
gnomemeeting_init_druid_audio_devices_page (GnomeDruid *druid, int p, int t)
{
  GtkWidget *vbox = NULL;
  GtkWidget *table = NULL;
  GtkWidget *button = NULL;
  GtkWidget *label = NULL;
  GtkWidget *audio_recorder = NULL;
  GtkWidget *audio_player = NULL;

  gchar *title = NULL;
  gchar *audio_player_devices_list [20];
  gchar *audio_recorder_devices_list [20];
  int i = 0;

  GnomeDruidPageStandard *page_standard = NULL;

  GmWindow *gw = gnomemeeting_get_main_window (gm);

  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  
  title = g_strdup_printf (_("Audio Devices - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));


  /* Packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);

  gnomemeeting_druid_add_graphical_label (vbox, GM_STOCK_DRUID_AUDIO, _("Please choose the audio devices to use during the GnomeMeeting session. You can also choose to use a Quicknet device instead of the soundcard. Some webcams models have an internal microphone that can be used with GnomeMeeting."));


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
    gnomemeeting_table_add_string_option_menu (table, _("Audio Player:"), audio_player_devices_list, DEVICES_KEY "audio_player", _("Enter the audio player device to use."), 0);

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
    gnomemeeting_table_add_string_option_menu (table, _("Audio Recorder:"), audio_recorder_devices_list, DEVICES_KEY "audio_recorder", _("Enter the audio recorder device to use."), 1);
  
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
gnomemeeting_init_druid_video_devices_page (GnomeDruid *druid, int p, int t)
{
  GtkWidget *vbox = NULL;
  GtkWidget *table = NULL;
  GtkWidget *button = NULL;
  GtkWidget *video_device = NULL;
  GtkWidget *progress = NULL;
  GtkWidget *label = NULL;

  gchar *title = NULL;
  gchar *video_devices [20];
  int i = 0;

  GnomeDruidPageStandard *page_standard = NULL;

  GmWindow *gw = gnomemeeting_get_main_window (gm);

  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());

  title = g_strdup_printf (_("Video Devices - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));


  /* Packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);

  gnomemeeting_druid_add_graphical_label (vbox, GM_STOCK_DRUID_VIDEO, _("Please choose the video device to use during the GnomeMeeting session. Click on the Video Test button to check if your setup is correct and if your driver is supported by GnomeMeeting."));


  /* The Video device */
  table = gnomemeeting_vbox_add_table (vbox, _("Video Devices"), 3, 3);
  i = gw->video_devices.GetSize () - 1;
  if (i >= 20) i = 19;

  for (int j = i ; j >= 0; j--) 
    video_devices [j] = g_strdup (gw->video_devices [j]);
  
  video_devices [i+1] = NULL;
  
  video_device = 
    gnomemeeting_table_add_string_option_menu (table, _("Video Device:"), video_devices, DEVICES_KEY "video_recorder", _("Enter the video device to use. Using an invalid video device for video transmission will transmit a test picture."), 0);
  
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


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the ixj device configuration.
 * PRE          :  /
 */
static void 
gnomemeeting_init_druid_ixj_device_page (GnomeDruid *druid, int p, int t)
{
  GtkWidget *href = NULL;
  GtkWidget *toggle = NULL;
  GtkWidget *entry = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *table = NULL;
  GtkWidget *label = NULL;

  GConfClient *client = gconf_client_get_default ();

  gchar *title = NULL;

  GnomeDruidPageStandard *page_standard = NULL;

  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());

  title = g_strdup_printf (_("PC-To-Phone Setup - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));


  /* Packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);

  gnomemeeting_druid_add_graphical_label (vbox, GM_STOCK_DRUID_IXJ, _("You can make calls to regular telephones and cell numbers worldwide using GnomeMeeting and the MicroTelco service from Quicknet Technologies. To enable this feature you need a compatible card from Quicknet Technologies and to enter a MicroTelco Account number and PIN below."));


  /* The PC-To-Phone setup */
  table = gnomemeeting_vbox_add_table (vbox, _("PC-To-Phone Setup"), 2, 4);

  entry = 
    gnomemeeting_table_add_entry (table, _("Account Number:"), 
				  GATEKEEPER_KEY "gk_alias", NULL, 0);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 12);
  g_signal_connect (G_OBJECT (entry), "changed",
		    GTK_SIGNAL_FUNC (gnomemeeting_druid_ixj_account_changed), 
		    NULL);

  entry = 
    gnomemeeting_table_add_entry (table, _("Password:"), 
				  GATEKEEPER_KEY "gk_password", NULL, 1);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 4);
  gtk_entry_set_visibility (GTK_ENTRY (entry), FALSE);


  /* The register toggle */
  toggle = 
    gtk_check_button_new_with_label (_("Register to the MicroTelco service"));

  gtk_table_attach (GTK_TABLE (table), toggle, 0, 2, 3, 4,
		    (GtkAttachOptions) NULL, 
		    (GtkAttachOptions) NULL,
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  g_object_set_data (G_OBJECT (druid), "ixj_toggle", (gpointer) toggle);


  /* Account Info */
  gchar *gconf_url =
    gconf_client_get_string (client, GATEKEEPER_KEY "gk_alias", NULL);
  
  if (!gconf_url)
    gconf_url = g_strdup ("");

  gchar *url = g_strdup_printf ("http://%s.an.microtelco.com", gconf_url);
  label = 
    gtk_label_new (_("Click on one of the following links to get more information about your existing MicroTelco account, or to create a new account."));
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);

  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (label), FALSE, FALSE, 0);
  href = gnome_href_new (url, _("My Account Information"));
  g_object_set_data (G_OBJECT (druid), "ixj_href", (gpointer) href);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (href), FALSE, FALSE, 0);
  href = gnome_href_new ("http://www.microtelco.com", "Get An Account");
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (href), FALSE, FALSE, 0);
  href = gnome_href_new ("http://www.linuxjack.com", "Buy a card");
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (href), FALSE, FALSE, 0);
  g_free (gconf_url);
  g_free (url);


  /**/
  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}
#endif


/* Functions */
void 
gnomemeeting_init_druid (gpointer data)
{
#ifndef DISABLE_GNOME
  GmWindow *gw = NULL;
  GtkWidget *window = NULL;
  gchar *title = NULL;

  GnomeDruidPageEdge *page_edge = NULL;
  GnomeDruidPageEdge *page_final = NULL;

  gw = gnomemeeting_get_main_window (gm);
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), 
			_("First Time Configuration Druid"));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gw->druid = GNOME_DRUID (gnome_druid_new ());

  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (gw->druid));

  g_object_set_data (G_OBJECT (gw->druid), "window", window);

  title = g_strdup_printf (_("Configuration Assistant - page %d/%d"), 1, 7);
  static const gchar text[] =
    N_
    ("Welcome to the GnomeMeeting general configuration druid. "
     "The following steps will set up GnomeMeeting by asking "
     "a few simple questions. Once you have completed "
     "these steps, you can always change them later in "
     "the preferences. ");


  /* Create the first page */
  page_edge =
    GNOME_DRUID_PAGE_EDGE (gnome_druid_page_edge_new
			   (GNOME_EDGE_START));
  gnome_druid_page_edge_set_title (page_edge, title);
  g_free (title);
			   
  gnome_druid_page_edge_set_text (page_edge, _(text));

  gnome_druid_append_page (gw->druid, GNOME_DRUID_PAGE (page_edge));
  gnome_druid_set_page (gw->druid, GNOME_DRUID_PAGE (page_edge));
  
  g_signal_connect_after (G_OBJECT (page_edge), "prepare",
			  G_CALLBACK (gnomemeeting_druid_page_prepare), 
			  (gpointer) "0");


  /* Create the user page */
  gnomemeeting_init_druid_user_page (gw->druid, 2, 7);
  
  /* Create connection type */
  gnomemeeting_init_druid_connection_type_page (gw->druid, 3, 7);
  
  /* Create the devices pages */
  gnomemeeting_init_druid_audio_devices_page (gw->druid, 4, 7);
  gnomemeeting_init_druid_video_devices_page (gw->druid, 5, 7);
  gnomemeeting_init_druid_ixj_device_page (gw->druid, 6, 7);

  /* Create final page */
  page_final =
    GNOME_DRUID_PAGE_EDGE (gnome_druid_page_edge_new (GNOME_EDGE_FINISH));
  
  title = g_strdup_printf (_("Done! - page %d/%d"), 7, 7);
  gnome_druid_page_edge_set_title (page_final, title);
  g_free (title);

  gnome_druid_append_page (gw->druid, GNOME_DRUID_PAGE (page_final));

  g_signal_connect_after (G_OBJECT (page_final), "prepare",
			  G_CALLBACK (gnomemeeting_druid_page_prepare), 
			  (gpointer) "7");  
  g_object_set_data (G_OBJECT (gw->druid), "page_final", 
		     (gpointer) page_final);

  g_signal_connect (G_OBJECT (page_final), "finish",
		    G_CALLBACK (gnomemeeting_druid_quit), gw->druid);

  g_signal_connect (G_OBJECT (gw->druid), "cancel",
		    G_CALLBACK (gnomemeeting_druid_cancel), data);

  g_signal_connect (G_OBJECT (window), "delete_event",
		    G_CALLBACK (gnomemeeting_druid_destroy), data);

  gtk_widget_show_all (window);
#endif
}

