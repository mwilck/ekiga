
/* GnomeMeeting --  Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
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
 *                         druid.cpp  -  description
 *                         --------------------------
 *   begin                : Mon May 1 2002
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          build the druid.
 */


#include "../config.h"

#include "druid.h"
#include "gnomemeeting.h"
#include "pref_window.h"
#include "sound_handling.h"
#include "ils.h"
#include "misc.h"
#include "callbacks.h"

#include "dialog.h"
#include "stock-icons.h"
#include "gm_conf.h"

/* private data */

struct _GmDruidWindow
{
  GnomeDruid *druid;
  GtkWidget *ils_register;
  GtkWidget *audio_test_button;
  GtkWidget *video_test_button;
  GtkWidget *enable_microtelco;
  GtkWidget *kind_of_net;
  GtkWidget *progress;
  GtkWidget *audio_manager;
  GtkWidget *video_manager;
  GtkWidget *audio_player;
  GtkWidget *audio_recorder;
  GtkWidget *video_device;
  GtkWidget *gk_alias;
  GtkWidget *gk_password;
  GtkWidget *name;
  GtkWidget *use_callto;
  GtkWidget *mail;
  GnomeDruidPageEdge *page_edge;
};

/* Declarations */
static void gm_druid_destroy_internal_struct (gpointer);

static gint gm_druid_kind_of_net_hack (gpointer);

static void gm_druid_audio_test_button_clicked_cb (GtkWidget *,
						   gpointer);

static void gm_druid_video_test_button_clicked_cb (GtkWidget *,
						   gpointer);

static void gm_druid_cancel_cb (GtkWidget *, gpointer);
static void gm_druid_finish_cb (GnomeDruidPage *, GtkWidget *, gpointer);
static void gm_druid_delete_event_cb (GtkWidget *, 
				      GdkEventAny *, gpointer);

static void gm_druid_check_personal_data (GmDruidWindow *);
static void gm_druid_changed_cb (GtkWidget *, gpointer);
static void gm_druid_ils_register_toggle_cb (GtkToggleButton *b, gpointer data);
static void gm_druid_option_menu_update (GtkWidget *option_menu, gchar **options,
					 gchar *default_value);
static void gm_druid_get_all_data (GmDruidWindow *, gchar *&, gchar *&,
				   gchar *&, gchar *&, gchar *&,
				   gchar *&, gchar *&, gchar *&);
static void gm_druid_prepare_welcome_page_cb (GnomeDruidPage *,
					      GnomeDruid *, gpointer);
static void gm_druid_prepare_personal_data_page_cb (GnomeDruidPage *page,
						    GnomeDruid *druid,
						    gpointer data);
static void gm_druid_prepare_callto_page_cb (GnomeDruidPage *,
					     GnomeDruid *, gpointer);
static void gm_druid_prepare_audio_devices_page_cb (GnomeDruidPage *page,
						    GnomeDruid *druid,
						    gpointer data);
static void gm_druid_prepare_video_devices_page_cb (GnomeDruidPage *page,
						    GnomeDruid *druid, 
						    gpointer data);
static void gm_druid_prepare_final_page_cb (GnomeDruidPage *,
					    GnomeDruid *, gpointer);

static void gm_druid_init_welcome_page (GmDruidWindow *, int);
static void gm_druid_init_personal_data_page (GmDruidWindow *, int, int);
static void gm_druid_init_callto_page (GmDruidWindow *, int, int);
static void gm_druid_init_connection_type_page (GmDruidWindow *, int, int);
static void gm_druid_init_audio_manager_page (GmDruidWindow *, int, int);
static void gm_druid_init_audio_devices_page (GmDruidWindow *, int, int);
static void gm_druid_init_video_manager_page (GmDruidWindow *, int, int);
static void gm_druid_init_video_devices_page (GmDruidWindow *, int, int);
static void gm_druid_init_final_page (GmDruidWindow *, int);

extern GtkWidget *gm;

/* used to free the memory of the attached GmDruidWindow */
static void 
gm_druid_destroy_internal_struct (gpointer ptr)
{
  GmDruidWindow *dw = NULL;

  dw = (GmDruidWindow *)ptr;
 
  delete (dw);
}

/* GTK Callbacks */
static gint
gm_druid_kind_of_net_hack (gpointer data)
{
  gm_conf_set_int (GENERAL_KEY "kind_of_net", GPOINTER_TO_INT (data));

  return FALSE;
}


static void
gm_druid_audio_test_button_clicked_cb (GtkWidget *w,
				       gpointer data)
{
  GMH323EndPoint *ep = NULL;
  
  gchar *name = NULL;
  gchar *con_type = NULL;
  gchar *mail = NULL;
  gchar *audio_manager = NULL;
  gchar *player = NULL;
  gchar *recorder = NULL;
  gchar *video_manager = NULL;
  gchar *video_recorder = NULL;
  GmDruidWindow *dw = NULL;

  dw = (GmDruidWindow *)data;

  gm_druid_get_all_data (dw, name, mail, con_type, audio_manager, player,
			 recorder, video_manager, video_recorder);

  ep = GnomeMeeting::Process ()->Endpoint ();

  if (GTK_TOGGLE_BUTTON (w)->active) {

    /* Try to prevent a crossed mutex deadlock */
    gdk_threads_leave ();
    ep->StartAudioTester (audio_manager, player, recorder);
    gdk_threads_enter ();
  }
  else {

    gdk_threads_leave ();
    ep->StopAudioTester ();
    gdk_threads_enter ();
  }
}


static void
gm_druid_video_test_button_clicked_cb (GtkWidget *w,
				       gpointer data)
{
  GMVideoTester *t = NULL;

  gchar *name = NULL;
  gchar *con_type = NULL;
  gchar *mail = NULL;
  gchar *audio_manager = NULL;
  gchar *player = NULL;
  gchar *recorder = NULL;
  gchar *video_manager = NULL;
  gchar *video_recorder = NULL;
  GmDruidWindow *dw = NULL;

  dw = (GmDruidWindow *)dw;

  gm_druid_get_all_data (dw, name, mail, con_type, audio_manager, player,
			 recorder, video_manager, video_recorder);

  if (GTK_TOGGLE_BUTTON (w)->active)   
    t = new GMVideoTester (video_manager, video_recorder);
}


/* DESCRIPTION  :  This callback is called when the user clicks on Cancel.
 * BEHAVIOR     :  Hides the druid and shows GM.
 * PRE          :  /
 */
static void 
gm_druid_cancel_cb (GtkWidget *w, gpointer data)
{
  GmWindow *gw = NULL;
  GmDruidWindow *dw = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  dw = (GmDruidWindow *)data;
  
  gnome_druid_set_page (dw->druid, GNOME_DRUID_PAGE (dw->page_edge));
  gnomemeeting_window_hide (gw->druid_window);
  gnomemeeting_window_show (gm);
}


/* DESCRIPTION  :  This callback is called when the user clicks on finish.
 * BEHAVIOR     :  Destroys the druid, update config settings and update
 *                 the internal structures for devices and the corresponding
 *                 prefs window menus. Displays a welcome message.
 * PRE          :  /
 */
static void 
gm_druid_finish_cb (GnomeDruidPage *p, GtkWidget *w, gpointer data)
{
  GmWindow *gw = NULL;
  GmDruidWindow *dw = NULL;

  GMH323EndPoint *ep = NULL;
  
  GtkWidget *active_item = NULL;
  int item_index = 0;
  int version = 0;

  BOOL has_video_device = FALSE;
  
  gchar *name = NULL;
  gchar **couple = NULL;
  gchar *con_type = NULL;
  gchar *mail = NULL;
  gchar *audio_manager = NULL;
  gchar *player = NULL;
  gchar *recorder = NULL;
  gchar *video_manager = NULL;
  gchar *video_recorder = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  dw = (GmDruidWindow *)data;
  ep = GnomeMeeting::Process ()->Endpoint ();

  
  active_item =
    gtk_menu_get_active (GTK_MENU (GTK_OPTION_MENU (dw->kind_of_net)->menu));
  item_index =
    g_list_index (GTK_MENU_SHELL (GTK_MENU (GTK_OPTION_MENU (dw->kind_of_net)->menu))->children, active_item) + 1;

  gm_druid_get_all_data (dw, name, mail, con_type, audio_manager, player,
			 recorder, video_manager, video_recorder);

  
  /* Set the personal data: firstname, lastname and mail
     and ILS registering
  */
  if (name)
    couple = g_strsplit (name, " ", 2);

  if (couple && couple [0])
    gm_conf_set_string (PERSONAL_DATA_KEY "firstname", couple [0]);
  if (couple && couple [1])
    gm_conf_set_string (PERSONAL_DATA_KEY "lastname", couple [1]);

  gm_conf_set_string (PERSONAL_DATA_KEY "mail", mail);
  
  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dw->use_callto))
      && mail) {

    if (!gm_conf_get_bool (LDAP_KEY "enable_registering"))
      gm_conf_set_bool (LDAP_KEY "enable_registering", TRUE);
    else
      ep->ILSRegister ();
  }
  else {

    gm_conf_set_bool (LDAP_KEY "enable_registering", FALSE);
  }
  

  /* Set the right devices and managers */
  if (audio_manager)
    gm_conf_set_string (AUDIO_DEVICES_KEY "plugin", audio_manager);
  if (player)
    gm_conf_set_string (AUDIO_DEVICES_KEY "output_device", player);
  if (recorder)
    gm_conf_set_string (AUDIO_DEVICES_KEY "input_device", recorder);
  if (video_manager)
    gm_conf_set_string (VIDEO_DEVICES_KEY "plugin", video_manager);
  if (video_recorder) {
    
    gm_conf_set_string (VIDEO_DEVICES_KEY "input_device", video_recorder);
    if (strcmp (video_recorder, _("No device found")))
      has_video_device = TRUE;
  }
  

  /* Set the connection quality settings */
  /* Dialup */
  if (item_index == 1) {
    
    gm_conf_set_int (VIDEO_CODECS_KEY "transmitted_fps", 1);
    gm_conf_set_int (VIDEO_CODECS_KEY "transmitted_video_quality", 1);
    gm_conf_set_int (VIDEO_CODECS_KEY "maximum_video_bandwidth", 1);
    gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video_transmission", FALSE);
    gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video_reception", FALSE);
  }
  else if (item_index == 2) { /* ISDN */
    
    gm_conf_set_int (VIDEO_CODECS_KEY "transmitted_fps", 1);
    gm_conf_set_int (VIDEO_CODECS_KEY "transmitted_video_quality", 1);
    gm_conf_set_int (VIDEO_CODECS_KEY "maximum_video_bandwidth", 2);
    gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video_transmission", FALSE);
    gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video_reception", FALSE);
  }
  else if (item_index == 3) { /* DSL / CABLE */
    
    gm_conf_set_int (VIDEO_CODECS_KEY "transmitted_fps", 8);
    gm_conf_set_int (VIDEO_CODECS_KEY "transmitted_video_quality", 60);
    gm_conf_set_int (VIDEO_CODECS_KEY "maximum_video_bandwidth", 8);
    gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video_transmission",
		      has_video_device);
    gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video_reception", TRUE);
  }
  else if (item_index == 4) { /* LDAN */
    
    gm_conf_set_int (VIDEO_CODECS_KEY "transmitted_fps", 20);
    gm_conf_set_int (VIDEO_CODECS_KEY "transmitted_video_quality", 80);
    gm_conf_set_int (VIDEO_CODECS_KEY "maximum_video_bandwidth", 100);
    gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video_transmission",
		      has_video_device);
    gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video_reception", TRUE);
  }  

  g_timeout_add (2000, (GtkFunction) gm_druid_kind_of_net_hack,
		 GINT_TO_POINTER (item_index));

  
  /* Set User Name and Alias */
  ep->SetUserNameAndAlias ();
  
  
  /* Hide the druid and show GnomeMeeting */
  gnomemeeting_window_hide (GTK_WIDGET (gw->druid_window));
  gnome_druid_set_page (dw->druid, GNOME_DRUID_PAGE (dw->page_edge));
  gnomemeeting_window_show (gm);


  /* Will be done through config if the manager changes, but not
     if the manager doesn't change */
  GnomeMeeting::Process ()->DetectDevices ();  
  gnomemeeting_pref_window_update_devices_list ();
  

  /* Displays a welcome message */
  if (gm_conf_get_int (GENERAL_KEY "version") 
      < MAJOR_VERSION * 1000 + MINOR_VERSION * 10 + BUILD_NUMBER)
    gnomemeeting_message_dialog (GTK_WINDOW (gm), _("Welcome to GnomeMeeting 1.00!"), _("Congratulations, you have just successfully launched GnomeMeeting 1.00 for the first time.\nGnomeMeeting is the leading VoIP, videoconferencing and telephony software for Unix.\n\nThanks to all of you who have helped us along the road to our golden 1.00 release!\n\nThe GnomeMeeting Team."));

  
  /* Update the version number */
  version = MAJOR_VERSION*1000+MINOR_VERSION*10+BUILD_NUMBER;
    
  gm_conf_set_int (GENERAL_KEY "version", version);
}


/* DESCRIPTION  :  This callback is called when the user destroys the druid.
 * BEHAVIOR     :  Exits. 
 * PRE          :  /
 */
static void 
gm_druid_delete_event_cb (GtkWidget *w, GdkEventAny *ev, gpointer data)
{
  gm_druid_cancel_cb (w, data);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Checks if the "Next" button of the "Personal Information"
 *                 druid page can be sensitive or not. It will if all fields
 *                 are ok.
 * PRE          :  The druid and the page number.
 */
static void 
gm_druid_check_personal_data (GmDruidWindow *dw)
{
  GnomeDruid *druid = NULL;
  PString mail;
  gchar ** couple = NULL;

  BOOL error = TRUE;

  druid = dw -> druid;

      
  couple = g_strsplit (gtk_entry_get_text (GTK_ENTRY (dw->name)), " ", 2);
  
  /* for page 2 */
  if (couple && couple [0] && couple [1]
      && !PString (couple [0]).Trim ().IsEmpty ()
      && !PString (couple [1]).Trim ().IsEmpty ()) 
    error = FALSE;

  /* for page 3 */
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dw->use_callto)))
    error = FALSE;
  else {
    
    mail = PString (gtk_entry_get_text (GTK_ENTRY (dw->mail)));
    if (!mail.IsEmpty () && mail.Find ("@") != P_MAX_INDEX)
      error = FALSE;
  }
  
  
  if (!error)
    gnome_druid_set_buttons_sensitive (druid, TRUE, TRUE, TRUE, FALSE);
  else
    gnome_druid_set_buttons_sensitive (druid, TRUE, FALSE, TRUE, FALSE);
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
gm_druid_changed_cb (GtkWidget *w, gpointer data)
{
  GmDruidWindow *dw = NULL;

  dw = (GmDruidWindow *)data;

  gm_druid_check_personal_data (dw);
}


/* DESCRIPTION  :  Called when the user changes the registering toggle.
 * BEHAVIOR     :  Checks if the "Next" button of the "Personal Information"
 *                 druid page can be sensitive or not. It will if all fields
 *                 are ok, or if registering is disabled. (Calls the above
 *                 function).
 * PRE          :  /
 */
static void
gm_druid_ils_register_toggle_cb (GtkToggleButton *b, gpointer data)
{
  GmDruidWindow *dw = NULL;

  dw = (GmDruidWindow *)data;

  gm_druid_check_personal_data (dw);
}


static void
gm_druid_option_menu_update (GtkWidget *option_menu,
			     gchar **options,
			     gchar *default_value)
{
  GtkWidget *menu = NULL;
  GtkWidget *item = NULL;

  int history = -1;
  int cpt = 0;                                                   

  cpt = 0;

  g_return_if_fail (options != NULL);

  gtk_option_menu_remove_menu (GTK_OPTION_MENU (option_menu));
  menu = gtk_menu_new ();

  while (options [cpt]) {

    if (default_value && !strcmp (options [cpt], default_value)) 
      history = cpt;

    item = gtk_menu_item_new_with_label (options [cpt]);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    cpt++;
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);

  if (history != -1)
    gtk_option_menu_set_history (GTK_OPTION_MENU (option_menu), history);
}


static void
gm_druid_get_all_data (GmDruidWindow *dw,
		       gchar * &name,
		       gchar * &mail,
		       gchar * &connection_type,
		       gchar * &audio_manager,
		       gchar * &player,
		       gchar * &recorder,
		       gchar * &video_manager,
		       gchar * &video_recorder)
{
  GtkWidget *child = NULL;
  
  name = (gchar *) gtk_entry_get_text (GTK_ENTRY (dw->name));
  mail = (gchar *) gtk_entry_get_text (GTK_ENTRY (dw->mail));
  child = GTK_BIN (dw->kind_of_net)->child;
  if (child)
    connection_type = (gchar *) gtk_label_get_text (GTK_LABEL (child));
  else
    connection_type = "";

  child = GTK_BIN (dw->audio_manager)->child;
  if (child)
    audio_manager = (gchar *) gtk_label_get_text (GTK_LABEL (child));
  else
    audio_manager = "";

  child = GTK_BIN (dw->video_manager)->child;
  if (child)
    video_manager = (gchar *) gtk_label_get_text (GTK_LABEL (child));
  else
    video_manager = "";

  child = GTK_BIN (dw->audio_player)->child;
  if (child)
    player = (gchar *) gtk_label_get_text (GTK_LABEL (child));
  else
    player = "";

  child = GTK_BIN (dw->audio_recorder)->child;
  if (child)
    recorder = (gchar *) gtk_label_get_text (GTK_LABEL (child));
  else
    recorder = "";

  child = GTK_BIN (dw->video_device)->child;
  if (child)
    video_recorder = (gchar *) gtk_label_get_text (GTK_LABEL (child));
  else
    video_recorder = "";
}

static void 
gm_druid_prepare_welcome_page_cb (GnomeDruidPage *page,
				  GnomeDruid *druid, gpointer data)
{
  gnome_druid_set_buttons_sensitive (druid, FALSE, TRUE, TRUE, FALSE);
}

static void
gm_druid_prepare_personal_data_page_cb (GnomeDruidPage *page,
					GnomeDruid *druid, gpointer data)
{
  GmWindow *gw = NULL;
  GmDruidWindow *dw = NULL;
  gchar *firstname = NULL;
  gchar *lastname = NULL;
  gchar *mail = NULL;
  gchar *text = NULL;
  int kind_of_net = 0;
  gchar *audio_manager = NULL;
  gchar *video_manager = NULL;
  BOOL ils_register = FALSE;
  char **array = NULL;
  char *options [] =
    {_("56k Modem"),
     _("ISDN"),
     _("xDSL/Cable"),
     _("T1/LAN"),
     _("Keep current settings"), NULL};
 
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  dw = (GmDruidWindow *)data;
  
  firstname = gm_conf_get_string (PERSONAL_DATA_KEY "firstname");
  lastname = gm_conf_get_string (PERSONAL_DATA_KEY "lastname");
  mail = gm_conf_get_string (PERSONAL_DATA_KEY "mail");
  kind_of_net = gm_conf_get_int (GENERAL_KEY "kind_of_net");
  ils_register = gm_conf_get_bool (LDAP_KEY "enable_registering");  
  
  if (!strcmp (gtk_entry_get_text (GTK_ENTRY (dw->name)), "")) {
    
    if (firstname && lastname
	&& strcmp (firstname, "") && strcmp (lastname, "")) {
      
      text = g_strdup_printf ("%s %s", firstname, lastname);
      gtk_entry_set_text (GTK_ENTRY (dw->name), text);
      g_free (text);
    }
  }
  
  if (!strcmp (gtk_entry_get_text (GTK_ENTRY (dw->mail)), "")
      && mail)
    gtk_entry_set_text (GTK_ENTRY (dw->mail), mail);
  
  gm_druid_option_menu_update (dw->kind_of_net, options, NULL);
  gtk_option_menu_set_history (GTK_OPTION_MENU (dw->kind_of_net),
			       kind_of_net - 1);
  
  array = gw->audio_managers.ToCharArray ();
  audio_manager = gm_conf_get_string (AUDIO_DEVICES_KEY "plugin");
  gm_druid_option_menu_update (dw->audio_manager, array, audio_manager);
  free (array);
  
  array = gw->video_managers.ToCharArray ();
  video_manager = gm_conf_get_string (VIDEO_DEVICES_KEY "plugin");
  gm_druid_option_menu_update (dw->video_manager, array, video_manager);
  free (array);
  
  GTK_TOGGLE_BUTTON (dw->use_callto)->active = !ils_register;
  
  gm_druid_check_personal_data (dw);
  
  g_free (video_manager);
  g_free (audio_manager);    
  g_free (mail);
  g_free (firstname);
  g_free (lastname);

}

static void 
gm_druid_prepare_callto_page_cb (GnomeDruidPage *page, GnomeDruid *druid, 
				 gpointer data)
{
  GmDruidWindow *dw = NULL;

  dw = (GmDruidWindow *)data;

  gm_druid_check_personal_data (dw);
}

static void
gm_druid_prepare_audio_devices_page_cb (GnomeDruidPage *page, GnomeDruid *druid, 
					gpointer data)
{
  GmWindow *gw = NULL;
  GMH323EndPoint *ep = NULL;
  GmDruidWindow *dw = NULL;
  GtkWidget *child = NULL;
  gchar *audio_manager = NULL;
  gchar *player = NULL;
  gchar *recorder = NULL;
  GdkCursor *cursor = NULL;
  PStringArray devices;
  char **array = NULL;

  dw = (GmDruidWindow *)data;
  ep = GnomeMeeting::Process ()->Endpoint ();
  gw = GnomeMeeting::Process ()->GetMainWindow ();

  
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dw->audio_test_button),
				FALSE);
  
  cursor = gdk_cursor_new (GDK_WATCH);
  gdk_window_set_cursor (GTK_WIDGET (gw->druid_window)->window, cursor);
  gdk_cursor_unref (cursor);

  child = GTK_BIN (dw->audio_manager)->child;
  
  if (child)
    audio_manager = (gchar *) gtk_label_get_text (GTK_LABEL (child));
  else
    audio_manager = "";
	
  player = gm_conf_get_string (AUDIO_DEVICES_KEY "output_device");
  recorder = gm_conf_get_string (AUDIO_DEVICES_KEY "input_device");
    
  gnomemeeting_sound_daemons_suspend ();
  if (PString ("Quicknet") == audio_manager)
    devices = OpalIxJDevice::GetDeviceNames ();
  else
    devices = PSoundChannel::GetDeviceNames (audio_manager,
					     PSoundChannel::Player);
  if (devices.GetSize () == 0) {
    
    devices += PString (_("No device found"));
    gtk_widget_set_sensitive (GTK_WIDGET (dw->audio_test_button), FALSE);
  }
  else
    gtk_widget_set_sensitive (GTK_WIDGET (dw->audio_test_button), TRUE);
  
  array = devices.ToCharArray ();
  gm_druid_option_menu_update (dw->audio_player, array, player);
  free (array);

  if (PString ("Quicknet") == audio_manager)
    devices = OpalIxJDevice::GetDeviceNames ();
  else
    devices = PSoundChannel::GetDeviceNames (audio_manager,
					     PSoundChannel::Recorder);
  if (devices.GetSize () == 0) {
    
    devices += PString (_("No device found"));
    gtk_widget_set_sensitive (GTK_WIDGET (dw->audio_test_button), FALSE);
  }
  else 
    gtk_widget_set_sensitive (GTK_WIDGET (dw->audio_test_button), TRUE);
  
  array = devices.ToCharArray ();
  gm_druid_option_menu_update (dw->audio_recorder, array, recorder);
  free (array);
  gnomemeeting_sound_daemons_resume ();

  gdk_window_set_cursor (GTK_WIDGET (gw->druid_window)->window, NULL);
  
  g_free (player);
  g_free (recorder);
  
  if (ep->GetCallingState () != GMH323EndPoint::Standby)
    gtk_widget_set_sensitive (GTK_WIDGET (dw->audio_test_button), FALSE);
}

static void
gm_druid_prepare_video_devices_page_cb (GnomeDruidPage *page, GnomeDruid *druid, 
					gpointer data)
{
  GmWindow *gw = NULL;
  GmDruidWindow *dw = NULL;
  GMH323EndPoint *ep = NULL;
  GdkCursor *cursor = NULL;
  GtkWidget *child = NULL;
  gchar *video_manager = NULL;
  gchar *video_recorder = NULL;
  PStringArray devices;
  char **array = NULL;
 
  dw = (GmDruidWindow *)data;
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  ep = GnomeMeeting::Process ()->Endpoint ();

  cursor = gdk_cursor_new (GDK_WATCH);
  gdk_window_set_cursor (GTK_WIDGET (gw->druid_window)->window, cursor);
  gdk_cursor_unref (cursor);

  child = GTK_BIN (dw->video_manager)->child;

  if (child)
    video_manager = (gchar *) gtk_label_get_text (GTK_LABEL (child));
  else
    video_manager = "";
	
  video_recorder = gm_conf_get_string (VIDEO_DEVICES_KEY "input_device");
    
  devices = PVideoInputDevice::GetDriversDeviceNames (video_manager);

  if (devices.GetSize () == 0) {
    
    devices += PString (_("No device found"));
    gtk_widget_set_sensitive (GTK_WIDGET (dw->video_test_button), FALSE);
  }
  else 
    gtk_widget_set_sensitive (GTK_WIDGET (dw->video_test_button), TRUE);
  
  array = devices.ToCharArray ();
  gm_druid_option_menu_update (dw->video_device, array, video_recorder);
  free (array);
  
  gdk_window_set_cursor (GTK_WIDGET (gw->druid_window)->window, NULL);
  
  g_free (video_recorder);
  
  if (ep->GetCallingState () != GMH323EndPoint::Standby)
    gtk_widget_set_sensitive (GTK_WIDGET (dw->video_test_button), FALSE);

}

/* DESCRIPTION  :  Called when the user switches between the pages 1, 2, and 6.
 * BEHAVIOR     :  Update the Next/Back buttons following the fields and the
 *                 page. Updates the text of the last page.
 * PRE          :  GPOINTER_TO_INT (data) = page number to prepare.
 */
static void
gm_druid_prepare_final_page_cb (GnomeDruidPage *page,
				GnomeDruid *druid,
				gpointer data)
{
  GmWindow *gw = NULL;
  GmDruidWindow *dw = NULL;
  GMH323EndPoint *ep = NULL;
  
  gchar *name = NULL;
  gchar *mail = NULL;
  gchar *text = NULL;
  gchar *connection_type = NULL;
  gchar *player = NULL;
  gchar *recorder = NULL;
  gchar *video_recorder = NULL;
  gchar *video_manager = NULL;
  gchar *audio_manager = NULL;
  gchar *callto_url = NULL;
  
  PStringArray devices;
  
  dw = (GmDruidWindow *)data;
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  ep = GnomeMeeting::Process ()->Endpoint ();

  gm_druid_get_all_data (dw, name, mail, connection_type, audio_manager,
			 player, recorder, video_manager, video_recorder);
  callto_url = g_strdup_printf ("callto:ils.seconix.com/%s",
				mail ? mail : "");
    
  text = g_strdup_printf (_("You have now finished the GnomeMeeting configuration. All the settings can be changed in the GnomeMeeting preferences. Enjoy!\n\n\nConfiguration summary:\n\nUsername: %s\nConnection type: %s\nAudio manager: %s\nAudio player: %s\nAudio recorder: %s\nVideo manager: %s\nVideo input: %s\nCallto URL: %s\n"), name, connection_type, audio_manager, player, recorder, video_manager, video_recorder, !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dw->use_callto)) ? callto_url : _("None"));
  gnome_druid_page_edge_set_text (GNOME_DRUID_PAGE_EDGE (page), text);
  
  g_free (callto_url);
  g_free (text);
}

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the welcome druid page.
 * PRE          :  /
 */
static void
gm_druid_init_welcome_page (GmDruidWindow *dw, int t)
{

  gchar *title = NULL;

  static const gchar text[] =
    N_
    ("This is the GnomeMeeting general configuration druid. "
     "The following steps will set up GnomeMeeting by asking "
     "a few simple questions.\n\nOnce you have completed "
     "these steps, you can always change them later by "
     "selecting Preferences in the Edit menu.");

  title = g_strdup_printf (_("Configuration Druid - page 1/%d"), t);
  
  dw->page_edge =
    GNOME_DRUID_PAGE_EDGE (gnome_druid_page_edge_new_aa (GNOME_EDGE_START));
  gnome_druid_page_edge_set_title (dw->page_edge, title);
			   
  gnome_druid_page_edge_set_text (dw->page_edge, _(text));
  
  gnome_druid_append_page (dw->druid, GNOME_DRUID_PAGE (dw->page_edge));
  gnome_druid_set_page (dw->druid, GNOME_DRUID_PAGE (dw->page_edge));
  
  g_signal_connect_after (G_OBJECT (dw->page_edge), "prepare",
			  G_CALLBACK (gm_druid_prepare_welcome_page_cb), dw);

  g_free (title);
}

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the Personal Information.
 * PRE          :  /
 */
static void 
gm_druid_init_personal_data_page (GmDruidWindow *dw, int p, int t)
{
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GnomeDruid *druid = dw->druid;
  GtkWidget *page = NULL;


  page = gnome_druid_page_standard_new ();

  title = g_strdup_printf (_("Personal Information - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (GNOME_DRUID_PAGE_STANDARD (page),
				       title);
  g_free (title);
  
  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page));

  
  /* Start packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);

  
  /* The user fields */
  label = gtk_label_new (_("Please enter your first name and your surname:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  dw->name = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (vbox), dw->name, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("Your first name and surname will be used when connecting to other VoIP and videoconferencing software."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  g_signal_connect (G_OBJECT (dw->name), "changed",
		    G_CALLBACK (gm_druid_changed_cb), dw);

  g_signal_connect_after (G_OBJECT (page), "prepare",
			  G_CALLBACK (gm_druid_prepare_personal_data_page_cb), 
			  dw);
  
  gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page)->vbox),
		      GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for ILS registering and callto.
 * PRE          :  /
 */
static void 
gm_druid_init_callto_page (GmDruidWindow *dw, int p, int t)
{
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *align = NULL;
  
  gchar *title = NULL;
  gchar *text = NULL;
  
  GnomeDruid *druid = dw->druid;
  GtkWidget *page = NULL;


  page = gnome_druid_page_standard_new ();

  title = g_strdup_printf (_("Callto URL - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (GNOME_DRUID_PAGE_STANDARD (page),
				       title);
  g_free (title);
				
  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page));

  
  /* Start packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);

  label = gtk_label_new (_("Please enter your e-mail address:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  dw->mail = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (vbox), dw->mail, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("Your e-mail address is used when registering to the GnomeMeeting users directory. It is used to create a callto address permitting your contacts to easily call you wherever you are."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);


  dw->use_callto = gtk_check_button_new ();
  label = gtk_label_new (_("I don't want to register to the GnomeMeeting users directory and get a callto address"));
  gtk_container_add (GTK_CONTAINER (dw->use_callto), label);
  align = gtk_alignment_new (0, 1.0, 0, 0);
  gtk_container_add (GTK_CONTAINER (align), dw->use_callto);
  gtk_box_pack_start (GTK_BOX (vbox), align, TRUE, TRUE, 0);

  g_signal_connect (G_OBJECT (dw->mail), "changed",
		    G_CALLBACK (gm_druid_changed_cb), dw);

  g_signal_connect (G_OBJECT (dw->use_callto), "toggled",
		    G_CALLBACK (gm_druid_ils_register_toggle_cb), dw);

  g_signal_connect_after (G_OBJECT (page), "prepare",
			  G_CALLBACK (gm_druid_prepare_callto_page_cb), 
			  dw);
  
  gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page)->vbox),
		      GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the Connection page.
 * PRE          :  /
 */
static void 
gm_druid_init_connection_type_page (GmDruidWindow *dw, int p, int t)
{
  GtkWidget *vbox = NULL;
  GtkWidget *label = NULL;

  GnomeDruid *druid = dw->druid;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GnomeDruidPageStandard *page_standard = NULL;

  /* Get data */
  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  
  title = g_strdup_printf (_("Connection Type - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));


  /* Start packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);


  /* The connection type */
  label = gtk_label_new (_("Please choose your connection type:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  
  dw->kind_of_net = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (vbox), dw->kind_of_net, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The connection type will permit determining the best quality settings that GnomeMeeting will use during calls. You can later change the settings individually in the preferences window."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the audio manager configuration.
 * PRE          :  /
 */
static void 
gm_druid_init_audio_manager_page (GmDruidWindow *dw, int p, int t)
{
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;

  GnomeDruid *druid = dw->druid;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GnomeDruidPageStandard *page_standard = NULL;

  /* Get data */  
  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  
  title = g_strdup_printf (_("Audio Manager - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));


  /* Start packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);


  /* The Audio devices */
  label = gtk_label_new (_("Please choose your audio manager:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  
  dw->audio_manager = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (vbox), dw->audio_manager, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The audio manager is the plugin that will manage your audio devices, ALSA is probably the best choice when available."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);


  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the audio devices configuration.
 * PRE          :  /
 */
static void 
gm_druid_init_audio_devices_page (GmDruidWindow *dw, int p, int t)
{
  GtkWidget *align = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *label = NULL;

  GnomeDruid *druid = dw->druid;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GnomeDruidPageStandard *page_standard = NULL;

  /* Get data */
  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  
  title = g_strdup_printf (_("Audio Devices - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));


  /* Start packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);


  /* The Audio devices */
  label = gtk_label_new (_("Please choose the audio output device:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  
  dw->audio_player = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (vbox), dw->audio_player, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The audio output device is the device managed by the audio manager that will be used to play audio."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  label = gtk_label_new (" ");
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  label = gtk_label_new (_("Please choose the audio input device:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  
  dw->audio_recorder = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (vbox), dw->audio_recorder, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The audio input device is the device managed by the audio manager that will be used to record your voice."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);


  label = gtk_label_new (" ");
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  align = gtk_alignment_new (1.0, 0, 0, 0);
  dw->audio_test_button =
    gtk_toggle_button_new_with_label (_("Test Settings"));
  gtk_container_add (GTK_CONTAINER (align), dw->audio_test_button);
  gtk_box_pack_start (GTK_BOX (vbox), align, FALSE, FALSE, 0);
  
  g_signal_connect (G_OBJECT (dw->audio_test_button), "clicked",
		    GTK_SIGNAL_FUNC (gm_druid_audio_test_button_clicked_cb),
		    (gpointer) dw);

  g_signal_connect_after (G_OBJECT (page_standard), "prepare",
			  G_CALLBACK (gm_druid_prepare_audio_devices_page_cb), 
			  dw);


  /**/
  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the video manager configuration.
 * PRE          :  /
 */
static void 
gm_druid_init_video_manager_page (GmDruidWindow *dw, int p, int t)
{
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;

  GnomeDruid *druid = dw->druid;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GnomeDruidPageStandard *page_standard = NULL;

  /* Get data */
  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  
  title = g_strdup_printf (_("Video Manager - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));


  /* Start packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);


  /* The Audio devices */
  label = gtk_label_new (_("Please choose your video manager:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  
  dw->video_manager = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (vbox), dw->video_manager, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The video manager is the plugin that will manage your video devices, Video4Linux is the most common choice if you own a webcam."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the druid page for the video devices configuration.
 * PRE          :  /
 */
static void 
gm_druid_init_video_devices_page (GmDruidWindow *dw, int p, int t)
{
  GtkWidget *align = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *label = NULL;

  GnomeDruid *druid = dw->druid;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GnomeDruidPageStandard *page_standard = NULL;

  /* Get data */
  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  
  title = g_strdup_printf (_("Video Devices - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (druid, GNOME_DRUID_PAGE (page_standard));


  /* Start packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);


  /* The Video devices */
  label = gtk_label_new (_("Please choose the video input device:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  
  dw->video_device = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (vbox), dw->video_device, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The video input device is the device managed by the video manager that will be used to capture video."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  label = gtk_label_new (" ");
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  align = gtk_alignment_new (1.0, 0, 0, 0);
  dw->video_test_button =
    gtk_toggle_button_new_with_label (_("Test Settings"));
  gtk_container_add (GTK_CONTAINER (align), dw->video_test_button);
  gtk_box_pack_start (GTK_BOX (vbox), align, FALSE, FALSE, 0);
  
  g_signal_connect (G_OBJECT (dw->video_test_button), "clicked",
		    GTK_SIGNAL_FUNC (gm_druid_video_test_button_clicked_cb),
		    (gpointer) dw);

  g_signal_connect_after (G_OBJECT (page_standard), "prepare",
			  G_CALLBACK (gm_druid_prepare_video_devices_page_cb), 
			  dw);

  /**/
  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}

static void
gm_druid_init_final_page (GmDruidWindow *dw, int t)
{
  gchar *title = NULL;

  GnomeDruidPageEdge *page_final = NULL;

  page_final =
    GNOME_DRUID_PAGE_EDGE (gnome_druid_page_edge_new (GNOME_EDGE_FINISH));
  
  title = g_strdup_printf (_("Configuration complete - page %d/%d"), t, t);
  gnome_druid_page_edge_set_title (page_final, title);

  gnome_druid_append_page (dw->druid, GNOME_DRUID_PAGE (page_final));

  g_signal_connect_after (G_OBJECT (page_final), "prepare",
			  G_CALLBACK (gm_druid_prepare_final_page_cb), 
			  dw);  

  g_signal_connect (G_OBJECT (page_final), "finish",
		    G_CALLBACK (gm_druid_finish_cb), dw);

  g_free (title);
}

/* Functions */
GtkWidget *
gm_druid_window_new ()
{
  GtkWidget *window = NULL;
  GmDruidWindow *dw = NULL;
  
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("druid_window"), g_free); 
  
  gtk_window_set_title (GTK_WINDOW (window), 
			_("First Time Configuration Druid"));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

  dw = new GmDruidWindow;
  g_object_set_data_full (G_OBJECT (window), "GMObject",
			  (gpointer)dw, gm_druid_destroy_internal_struct);

  dw->druid = GNOME_DRUID (gnome_druid_new ());

  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (dw->druid));

  /* Create the different pages */
  gm_druid_init_welcome_page (dw, 9);
  gm_druid_init_personal_data_page (dw, 2, 9);
  gm_druid_init_callto_page (dw, 3, 9);
  gm_druid_init_connection_type_page (dw, 4, 9);
  gm_druid_init_audio_manager_page (dw, 5, 9);
  gm_druid_init_audio_devices_page (dw, 6, 9);
  gm_druid_init_video_manager_page (dw, 7, 9);
  gm_druid_init_video_devices_page (dw, 8, 9);
  gm_druid_init_final_page (dw, 9);

  g_signal_connect (G_OBJECT (dw->druid), "cancel",
		    G_CALLBACK (gm_druid_cancel_cb), dw);

  g_signal_connect (G_OBJECT (window), "delete_event",
		    G_CALLBACK (gm_druid_delete_event_cb), dw);

  gtk_widget_show_all (GTK_WIDGET (dw->druid));

  return window;
}

void 
gm_druid_set_test_buttons_sensitivity (GtkWidget *druid,
				       gboolean value)
{
  GmDruidWindow *dw = NULL;

  dw = (GmDruidWindow *)g_object_get_data (G_OBJECT (druid), "GMObject");

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dw->video_test_button),
				value);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dw->audio_test_button),
				value);

}
