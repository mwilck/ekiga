
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
#include "ils.h"
#include "videograbber.h"
#include "stock-icons.h"
#include "callbacks.h"


#ifndef DISABLE_GNOME
/* Declarations */
static void audio_test_button_clicked (GtkWidget *, gpointer);
static void video_test_button_clicked (GtkWidget *, gpointer);
static void gnomemeeting_druid_add_graphical_label (GtkWidget *, gchar *, 
						    gchar *);
static void gnomemeeting_druid_cancel (GtkWidget *, gpointer);
static void gnomemeeting_druid_quit (GtkWidget *, gpointer);
static void gnomemeeting_druid_destroy (GtkWidget *, GdkEventAny *, gpointer);
static void gnomemeeting_druid_user_page_check (GnomeDruid *);
static void gnomemeeting_druid_toggle_changed (GtkToggleButton *, gpointer);
static void gnomemeeting_druid_entry_changed (GtkWidget *, gpointer);
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
audio_test_button_clicked (GtkWidget *w,
			   gpointer data)
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

  if (GTK_TOGGLE_BUTTON (w)->active)   
    t = new GMVideoTester ();
}


static void
gnomemeeting_druid_add_graphical_label (GtkWidget *vbox, gchar *stock, 
					gchar *label_text)
{
  GtkWidget *hbox = NULL;
  GtkWidget *label = NULL;
  GtkWidget *image = NULL;

  /* Packing widgets */
  hbox = gtk_hbox_new (FALSE, 20);
  
  image = gtk_image_new_from_stock (stock, GTK_ICON_SIZE_DIALOG);

  label = gtk_label_new (label_text);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);

  gtk_box_pack_start (GTK_BOX (hbox), image,
                      TRUE, TRUE, 0);      
  gtk_box_pack_start (GTK_BOX (hbox), label,
                      TRUE, TRUE, 0);          
  gtk_box_pack_start (GTK_BOX (vbox), hbox,
                      TRUE, TRUE, 0);          
}


/* DESCRIPTION  :  This callback is called when the user clicks on Cancel.
 * BEHAVIOR     :  Hides the druid and shows GM.
 * PRE          :  /
 */
static void 
gnomemeeting_druid_cancel (GtkWidget *w, gpointer data)
{
  GmWindow *gw = NULL;
  GmDruidWindow *dw = NULL;

  gw = gnomemeeting_get_main_window (gm);
  dw = gnomemeeting_get_druid_window (gm);

  gnome_druid_set_page (dw->druid, GNOME_DRUID_PAGE (dw->page_edge));
  gtk_widget_hide (gw->druid_window);
  gtk_widget_show (gm);
}


/* DESCRIPTION  :  This callback is called when the user clicks on finish.
 * BEHAVIOR     :  Destroys the druid and update gconf settings.
 * PRE          :  /
 */
static void 
gnomemeeting_druid_quit (GtkWidget *w, gpointer data)
{
  GConfClient *client = NULL;
  gchar *gconf_string = NULL;
  gchar *gk_name = NULL;
  int cpt = 1;
  
  GSList *group = NULL;
  
  GmWindow *gw = NULL;
  GmDruidWindow *dw = NULL;
  
  client = gconf_client_get_default ();

  gw = gnomemeeting_get_main_window (gm);
  dw = gnomemeeting_get_druid_window (gm);


  /* Always register to make the callto available,the user can choose
     to be visible or not */
  gconf_client_set_bool (client, LDAP_KEY "register", true, NULL);
  (GM_ILS_CLIENT (MyApp->Endpoint ()->GetILSClientThread ()))->Modify ();


  /* Fix the toggles */
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dw->audio_test_button),
				FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dw->video_test_button),
				FALSE);


  /**/
  gconf_client_set_int (client, GENERAL_KEY "version", 
			MAJOR_VERSION * 100 + MINOR_VERSION, 0);


  /* Connection type */
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (dw->kind_of_net));
  while (group) {

    if (GTK_TOGGLE_BUTTON (group->data)->active)
      break;
    
    group = g_slist_next (group);
    cpt++;
  }
  gconf_client_set_int (client, GENERAL_KEY "kind_of_net", 6 - cpt, NULL);


  /* Did the user choose to use the MicroTelco service? */
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dw->enable_microtelco))) {

    gconf_string = 
      gconf_client_get_string (client, GATEKEEPER_KEY "gk_alias" , NULL);
      
    if (gconf_string) {

      gconf_client_set_string (client, GATEKEEPER_KEY "gk_host",
			       "gk.microtelco.com", 0);
      gconf_client_set_int (client, GATEKEEPER_KEY "registering_method", 1, 0);
      gconf_client_set_bool (client, SERVICES_KEY "microtelco", true, 0);

      MyApp->Endpoint ()->SetUserNameAndAlias ();
      MyApp->Endpoint ()->RemoveGatekeeper (0);
      MyApp->Endpoint ()->GatekeeperRegister ();
    }

    g_free (gconf_string);   
    g_free (gk_name);


    /* Enable Fast Start and Tunneling */
    gconf_client_set_bool (client, GENERAL_KEY "fast_start", true, NULL);
    gconf_client_set_bool (client, GENERAL_KEY "h245_tunneling", true, NULL);

    
    cpt = 0;
    /* Automatically select the quicknet device */
    while (cpt < gw->audio_player_devices.GetSize ()) {

      if (gw->audio_player_devices [cpt].Find ("phone") != P_MAX_INDEX) {

	gconf_client_set_string (client, DEVICES_KEY "audio_player",
				 gw->audio_player_devices [cpt], NULL);
	gconf_client_set_string (client, DEVICES_KEY "audio_recorder",
				 gw->audio_player_devices [cpt], NULL);
	break;
      }

      cpt++;
    }
  }
  else
    gconf_client_set_bool (client, SERVICES_KEY "microtelco", false, 0);

  gtk_widget_hide_all (GTK_WIDGET (gw->druid_window));
  gnome_druid_set_page (dw->druid, GNOME_DRUID_PAGE (dw->page_edge));
  gtk_widget_show (gm);
}


/* DESCRIPTION  :  This callback is called when the user destroys the druid.
 * BEHAVIOR     :  Exits. 
 * PRE          :  /
 */
static void 
gnomemeeting_druid_destroy (GtkWidget *w, GdkEventAny *ev, gpointer data)
{
  gnomemeeting_druid_quit (w, data);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Checks if the "Next" button of the "Personal Information"
 *                 druid page can be sensitive or not. It will if all fields
 *                 are ok.
 * PRE          :  /
 */
static void 
gnomemeeting_druid_user_page_check (GnomeDruid *druid)
{
  GmDruidWindow *dw = NULL;
  GConfClient *client = NULL;

  gchar *gconf_string = NULL;
  gboolean error = FALSE;

  
  /* Fetch data */
  client = gconf_client_get_default ();
  dw = gnomemeeting_get_druid_window (gm);
  

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

  if (!error)
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
  GmDruidWindow *dw = NULL;
  GmWindow *gw = NULL;

  dw = gnomemeeting_get_druid_window (gm);
  gw = gnomemeeting_get_main_window (gm);

  gnomemeeting_druid_user_page_check (dw->druid);

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)))
    gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (gw->druid_window), GTK_WIDGET (button), _("You chose to NOT use the GnomeMeeting ILS directory. Other users will not be able to contact you if you don't register to a directory service."));
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
  GmDruidWindow *dw = NULL;

  dw = gnomemeeting_get_druid_window (gm);

  gnomemeeting_druid_user_page_check (dw->druid);
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
  GmDruidWindow *dw = NULL;
  
  gw = gnomemeeting_get_main_window (gm);
  dw = gnomemeeting_get_druid_window (gm);
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
    gconf_client_set_int (client, VIDEO_SETTINGS_KEY "tr_vq", 40, NULL);
    gconf_client_set_int (client, VIDEO_SETTINGS_KEY "re_vq", 70, NULL);
    gconf_client_set_int (client, 
			  VIDEO_SETTINGS_KEY "maximum_video_bandwidth", 
			  6, NULL);
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
  GmDruidWindow *dw = gnomemeeting_get_druid_window (gm);
  GConfClient *client = gconf_client_get_default ();
  
  int kind_of_net = 1, cpt = 1;

  GSList *group = NULL;
  
  if (!strcmp ((char *) data, "0"))
    gnome_druid_set_buttons_sensitive (druid, FALSE, TRUE, TRUE, FALSE);

  if (!strcmp ((char *) data, "1")) {

    gnome_druid_set_buttons_sensitive (druid, TRUE, FALSE, TRUE, FALSE);
    gnomemeeting_druid_user_page_check (druid);
  }

  if (!strcmp ((char *) data, "7")) 
    gnomemeeting_druid_final_page_prepare (druid);

  
  /* Update the radio button corresponding to the kind of net without
     triggering the signal */
  if (!strcmp ((char *) data, "3")) {

    group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (dw->kind_of_net));
    kind_of_net =
      gconf_client_get_int (client, GENERAL_KEY "kind_of_net", NULL);

    while (kind_of_net && group) {

      GTK_TOGGLE_BUTTON (group->data)->active =
	(kind_of_net == 6 - cpt);

      gtk_widget_queue_draw (GTK_WIDGET (group->data));
      group = g_slist_next (group);
      cpt++;
    }
  }
}


static void
gnomemeeting_druid_final_page_prepare (GnomeDruid *druid)
{
  GmDruidWindow *dw = NULL;
  
  GnomeDruidPageEdge *page_final = 
    GNOME_DRUID_PAGE_EDGE (g_object_get_data (G_OBJECT (druid), "page_final"));

  gchar *text = NULL;
  gchar *kind_of_net_name = NULL;
  gchar *audio_recorder = NULL;
  gchar *audio_player = NULL;
  gchar *video_recorder = NULL;
  gchar *firstname = NULL;
  gchar *lastname = NULL;
  gchar *mail = NULL;
  gchar *callto = NULL;
  bool microtelco = false;
  int cpt = 1;
  
  GSList *group = NULL;

  /* Fetch the data */
  GConfClient *client = gconf_client_get_default ();
  dw = gnomemeeting_get_druid_window (gm);

  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (dw->kind_of_net));
  while (group) {

    if (GTK_TOGGLE_BUTTON (group->data)->active)
      break;
    
    group = g_slist_next (group);
    cpt++;
  }
  
  
  switch (6 - cpt) {
    
  case 1:
    kind_of_net_name = g_strdup (_("56K modem"));
    break;

  case 2:
    kind_of_net_name = g_strdup (_("ISDN"));
    break;

  case 3:
    kind_of_net_name = g_strdup (_("xDSL/Cable"));
    break;

  case 4:
    kind_of_net_name = g_strdup (_("T1/LAN"));
    break;

  case 5:
    kind_of_net_name = g_strdup (_("Other"));
    break;
  }

  firstname =
    gconf_client_get_string (client, PERSONAL_DATA_KEY "firstname", NULL);
  lastname =
    gconf_client_get_string (client, PERSONAL_DATA_KEY "lastname", NULL);
  mail =
    gconf_client_get_string (client, PERSONAL_DATA_KEY "mail", NULL);
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

  microtelco =
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dw->enable_microtelco));
  
  callto = g_strdup_printf ("callto://ils.seconix.com/%s", mail);
  
  text = g_strdup_printf (_("You have now finished the GnomeMeeting configuration. All the settings can be changed in the GnomeMeeting preferences. Enjoy!\n\n\nConfiguration Summary:\n\nUsername:  %s %s\nConnection type:  %s\nAudio player:  %s\nAudio recorder:  %s\nVideo player: %s\nMy Callto URL: %s\nPC-To-Phone calls: %s"), firstname, lastname, kind_of_net_name, audio_player, audio_recorder, video_recorder, callto, microtelco ? _("Enabled") : _("Disabled"));
  gnome_druid_page_edge_set_text (page_final, text);
    
  g_free (text);
  g_free (firstname);
  g_free (lastname);
  g_free (audio_player);
  g_free (audio_recorder);
  g_free (video_recorder);
  g_free (callto);
  g_free (mail);
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
  GtkWidget *entry = NULL;
  gchar *title = NULL;

  GmDruidWindow *dw = NULL;
  GnomeDruidPageStandard *page_standard = NULL;


  dw = gnomemeeting_get_druid_window (gm);
  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());

  title = g_strdup_printf (_("Personal Information - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);
				
  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));

  
  /* Packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);
  gnomemeeting_druid_add_graphical_label (vbox, GM_STOCK_DRUID_PERSONAL, _("Please enter your first name and surname, they will be used when connecting to other audio/video conferencing software.\n\nYour e-mail address is used to provide you with a callto address that your friends can use to call you without knowing your IP address.\n\nNo information is made public unless you allow it to be published on the directory of online GnomeMeeting users."));
					  

  /* The user fields */
  table = gnomemeeting_vbox_add_table (vbox, _("Personal Information"), 2, 2);
  
  entry = 
    gnomemeeting_table_add_entry (table, _("First _name:"), 
				  PERSONAL_DATA_KEY "firstname", NULL, 0);
  g_signal_connect (G_OBJECT (entry), "changed",
		    G_CALLBACK (gnomemeeting_druid_entry_changed), NULL);

  entry = 
    gnomemeeting_table_add_entry (table, _("_Surname:"), 
				  PERSONAL_DATA_KEY "lastname", NULL, 1);
  g_signal_connect (G_OBJECT (entry), "changed",
		    G_CALLBACK (gnomemeeting_druid_entry_changed), NULL);


  
  /* The callto url */
  table = gnomemeeting_vbox_add_table (vbox, _("Callto URL"), 1, 2);
  
  entry = 
    gnomemeeting_table_add_entry (table, _("E-_mail address:"), 
				  PERSONAL_DATA_KEY "mail", NULL, 2);
  g_signal_connect (G_OBJECT (entry), "changed",
		    G_CALLBACK (gnomemeeting_druid_entry_changed), NULL);


  /* The ILS registering */
  table = gnomemeeting_vbox_add_table (vbox, _("Directory of Online GnomeMeeting Users"), 1, 2);
  dw->ils_register =
    gnomemeeting_table_add_toggle (table, _("Publish my information on the directory of online GnomeMeeting users"), 
				  LDAP_KEY "visible", NULL, 2, 0);
    

  
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
  GtkWidget *radio2 = NULL;
  GtkWidget *radio3 = NULL;
  GtkWidget *radio4 = NULL;
  GtkWidget *radio5 = NULL;
  gchar *title = NULL;

  GConfClient *client = NULL;

  GmDruidWindow *dw = gnomemeeting_get_druid_window (gm);
  GnomeDruidPageStandard *page_standard = NULL;


  client = gconf_client_get_default ();

  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());

  title = g_strdup_printf (_("Network Connection - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));


  /* Packing widgets */                                                        
  vbox = gtk_vbox_new (FALSE, 2);
  box = gtk_vbox_new (FALSE, 2);

  gnomemeeting_druid_add_graphical_label (vbox, GM_STOCK_DRUID_CONNECTION, _("Please select your network connection type. This is used to set default video settings for your bandwidth. It is possible to change these defaults later."));

  
  /* Connection type */
  table = gnomemeeting_vbox_add_table (vbox, _("Connection Type"), 1, 1);
  dw->kind_of_net = gtk_radio_button_new_with_label (NULL, _("56K modem"));
  gtk_box_pack_start (GTK_BOX (box), dw->kind_of_net, TRUE, TRUE, 0);
  radio2 = 
    gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (dw->kind_of_net), 
						 _("ISDN"));
  gtk_box_pack_start (GTK_BOX (box), radio2, TRUE, TRUE, 0);
  radio3 = 
    gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (dw->kind_of_net), 
						 _("DSL/Cable"));
  gtk_box_pack_start (GTK_BOX (box), radio3, TRUE, TRUE, 0);
  radio4 = 
    gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (dw->kind_of_net), 
						 _("T1/LAN"));
  gtk_box_pack_start (GTK_BOX (box), radio4, TRUE, TRUE, 0);
  radio5= 
    gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (dw->kind_of_net), 
						 _("Other"));
  gtk_box_pack_start (GTK_BOX (box), radio5, TRUE, TRUE, 0);

  gtk_table_attach (GTK_TABLE (table), box, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dw->kind_of_net), true);
  
  /* Connect signals */
  g_signal_connect (G_OBJECT (dw->kind_of_net), "toggled",
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

  g_signal_connect_after (G_OBJECT (page_standard), "prepare",
			  G_CALLBACK (gnomemeeting_druid_page_prepare), 
			  (gpointer) "3");
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
  GtkWidget *label = NULL;

  GmDruidWindow *dw = gnomemeeting_get_druid_window (gm);
  
  gchar *title = NULL;

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

  gnomemeeting_druid_add_graphical_label (vbox, GM_STOCK_DRUID_AUDIO, _("Please choose the audio devices to use during the GnomeMeeting session. You can also choose to use a Quicknet device instead of the soundcard, but the \"Test Audio\" button will only work for soundcards. Notice that some webcams models have an internal microphone that can be used with GnomeMeeting."));


  /* The Audio devices */
  table = gnomemeeting_vbox_add_table (vbox, _("Audio Devices"), 4, 3);

  dw->audio_player = 
    gnomemeeting_table_add_pstring_option_menu (table, _("Audio player:"), gw->audio_player_devices, DEVICES_KEY "audio_player", _("Select the audio player device to use."), 0);

  dw->audio_player_mixer = 
    gnomemeeting_table_add_pstring_option_menu (table, _("Audio player mixer:"), gw->audio_mixers, DEVICES_KEY "audio_player_mixer", _("Select the mixer to use to change the volume of the audio player."), 1);
  
  /* The recorder */
  dw->audio_recorder = 
    gnomemeeting_table_add_pstring_option_menu (table, _("Audio recorder:"), gw->audio_recorder_devices, "/apps/gnomemeeting/devices/audio_recorder", _("Select the audio recorder device to use."), 2);

  dw->audio_recorder_mixer = 
    gnomemeeting_table_add_pstring_option_menu (table, _("Audio recorder mixer:"), gw->audio_mixers, DEVICES_KEY "audio_recorder_mixer", _("Select the mixer to use to change the volume of the audio recorder."), 3);
  

  /* Test button */
  label = gtk_label_new (_("Click here to test your audio devices:"));

  dw->audio_test_button = gtk_toggle_button_new_with_label (_("Test Audio"));
  gtk_table_attach (GTK_TABLE (table), dw->audio_test_button, 2, 3, 3, 4,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    20, 0);
  g_signal_connect (G_OBJECT (dw->audio_test_button), "clicked",
		    GTK_SIGNAL_FUNC (audio_test_button_clicked),
		    (gpointer) druid);


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

  gchar *title = NULL;

  GnomeDruidPageStandard *page_standard = NULL;

  GmWindow *gw = gnomemeeting_get_main_window (gm);
  GmDruidWindow *dw = gnomemeeting_get_druid_window (gm);

  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());

  title = g_strdup_printf (_("Video Devices - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));


  /* Packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);

  gnomemeeting_druid_add_graphical_label (vbox, GM_STOCK_DRUID_VIDEO, _("Please choose the video device to use during the GnomeMeeting session. Click on the \"Test Video\" button to check if your setup is correct and if your driver is supported by GnomeMeeting. You can only test the correctness of your driver if the selected device is not already in use."));


  /* The Video device */
  table = gnomemeeting_vbox_add_table (vbox, _("Video Devices"), 2, 3);
  
  dw->video_device = 
    gnomemeeting_table_add_pstring_option_menu (table, _("Video device:"), gw->video_devices, DEVICES_KEY "video_recorder", _("Enter the video device to use. Using an invalid video device for video transmission will transmit a test picture."), 0);
  

  /* Test button */
  dw->video_test_button = gtk_toggle_button_new_with_label (_("Test Video"));
  gtk_table_attach (GTK_TABLE (table), dw->video_test_button, 2, 3, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    0, 0);

  dw->progress = gtk_progress_bar_new ();
  gtk_table_attach (GTK_TABLE (table), dw->progress, 0, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    0, 0);

  g_signal_connect (G_OBJECT (dw->video_test_button), "clicked",
		    GTK_SIGNAL_FUNC (video_test_button_clicked), NULL);

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
  GtkWidget *table = NULL;
  GtkWidget *href = NULL;
  GtkWidget *entry = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *label = NULL;

  GmDruidWindow *dw = gnomemeeting_get_druid_window (gm);
    
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

  gnomemeeting_druid_add_graphical_label (vbox, GM_STOCK_DRUID_IXJ, _("You can make calls to regular phones and cell numbers worldwide using GnomeMeeting and the MicroTelco service from Quicknet Technologies. To enable this you need to enter your MicroTelco Account number and PIN below, then enable registering to the MicroTelco service. Please visit the GnomeMeeting website for more information."));

  
  /* The PC-To-Phone setup */
  table =
    gnomemeeting_vbox_add_table (vbox, _("PC-To-Phone Setup"), 3, 4);

  entry = 
    gnomemeeting_table_add_entry (table, _("Account number:"), 
				  GATEKEEPER_KEY "gk_alias", NULL, 0);
  entry = 
    gnomemeeting_table_add_entry (table, _("Password:"), 
				  GATEKEEPER_KEY "gk_password", NULL, 1);
  gtk_entry_set_visibility (GTK_ENTRY (entry), FALSE);


  /* The register toggle */
  dw->enable_microtelco = 
    gnomemeeting_table_add_toggle (table,
				   _("Register to the MicroTelco service"), 
				   SERVICES_KEY "enable_microtelco",
				   NULL, 2, 0);

  /* Account Info */
  gchar *gconf_url =
    gconf_client_get_string (client, GATEKEEPER_KEY "gk_alias", NULL);
  
  if (!gconf_url)
    gconf_url = g_strdup ("");

  label = 
    gtk_label_new (_("Click on one of the following links to get more information about your existing MicroTelco account, or to create a new account."));
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);

  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (label), FALSE, FALSE, 0);
  href = gnome_href_new ("http://www.microtelco.com", _("Get an account"));
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (href), FALSE, FALSE, 0);
  href = gnome_href_new ("http://www.linuxjack.com", _("Buy a card"));
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (href), FALSE, FALSE, 0);
  g_free (gconf_url);

  
  g_signal_connect_after (G_OBJECT (page_standard), "prepare",
			  G_CALLBACK (gnomemeeting_druid_page_prepare), 
			  (gpointer) "6");

  
  /**/
  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}
#endif


/* Functions */
void 
gnomemeeting_init_druid ()
{
#ifndef DISABLE_GNOME
  GmWindow *gw = NULL;
  GmDruidWindow *dw = NULL;

  gchar *title = NULL;

  GnomeDruidPageEdge *page_final = NULL;

  gw = gnomemeeting_get_main_window (gm);
  dw = gnomemeeting_get_druid_window (gm);
  
  gw->druid_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (gw->druid_window), 
			_("First Time Configuration Druid"));
  gtk_window_set_position (GTK_WINDOW (gw->druid_window), GTK_WIN_POS_CENTER);
  dw->druid = GNOME_DRUID (gnome_druid_new ());

  gtk_container_add (GTK_CONTAINER (gw->druid_window), GTK_WIDGET (dw->druid));


  title = g_strdup_printf (_("Configuration Druid - page %d/%d"), 1, 7);
  static const gchar text[] =
    N_
    ("This is the GnomeMeeting general configuration druid. "
     "The following steps will set up GnomeMeeting by asking "
     "a few simple questions.\n\nOnce you have completed "
     "these steps, you can always change them later by "
     "selecting Preferences in the Edit menu. ");


  /* Create the first page */
  dw->page_edge =
    GNOME_DRUID_PAGE_EDGE (gnome_druid_page_edge_new
			   (GNOME_EDGE_START));
  gnome_druid_page_edge_set_title (dw->page_edge, title);
  g_free (title);
			   
  gnome_druid_page_edge_set_text (dw->page_edge, _(text));

  gnome_druid_append_page (dw->druid, GNOME_DRUID_PAGE (dw->page_edge));
  gnome_druid_set_page (dw->druid, GNOME_DRUID_PAGE (dw->page_edge));
  
  g_signal_connect_after (G_OBJECT (dw->page_edge), "prepare",
			  G_CALLBACK (gnomemeeting_druid_page_prepare), 
			  (gpointer) "0");


  /* Create the user page */
  gnomemeeting_init_druid_user_page (dw->druid, 2, 7);
  
  /* Create connection type */
  gnomemeeting_init_druid_connection_type_page (dw->druid, 3, 7);
  
  /* Create the devices pages */
  gnomemeeting_init_druid_audio_devices_page (dw->druid, 4, 7);
  gnomemeeting_init_druid_video_devices_page (dw->druid, 5, 7);
  gnomemeeting_init_druid_ixj_device_page (dw->druid, 6, 7);

  /* Create final page */
  page_final =
    GNOME_DRUID_PAGE_EDGE (gnome_druid_page_edge_new (GNOME_EDGE_FINISH));
  
  title = g_strdup_printf (_("Configuration complete - page %d/%d"), 7, 7);
  gnome_druid_page_edge_set_title (page_final, title);
  g_free (title);

  gnome_druid_append_page (dw->druid, GNOME_DRUID_PAGE (page_final));

  g_signal_connect_after (G_OBJECT (page_final), "prepare",
			  G_CALLBACK (gnomemeeting_druid_page_prepare), 
			  (gpointer) "7");  
  g_object_set_data (G_OBJECT (dw->druid), "page_final", 
		     (gpointer) page_final);

  g_signal_connect (G_OBJECT (page_final), "finish",
		    G_CALLBACK (gnomemeeting_druid_quit), dw->druid);

  g_signal_connect (G_OBJECT (dw->druid), "cancel",
		    G_CALLBACK (gnomemeeting_druid_cancel), NULL);

  g_signal_connect (G_OBJECT (gw->druid_window), "delete_event",
		    G_CALLBACK (gnomemeeting_druid_destroy), NULL);
#endif
}

