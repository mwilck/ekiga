
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


/* Private data for the GmObject */
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

  GMStunClient *stun_client;
};


typedef struct _GmDruidWindow GmDruidWindow;


#define GM_DRUID_WINDOW(x) (GmDruidWindow *) (x)


/* Declarations */


/* GUI functions */

/* DESCRIPTION  : /
 * BEHAVIOR     : Frees a GmDruidWindow and its content.
 * PRE          : A non-NULL pointer to a GmDruidWindow.
 */
static void gm_dw_destroy (gpointer);


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns a pointer to the private GmDruidWindow
 *                used by the druid window GMObject.
 * PRE          : The given GtkWidget pointer must be a druid window GMObject.
 */
static GmDruidWindow *gm_dw_get_dw (GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Checks if the "Next" button of the "Personal Information"
 *                 druid page can be sensitive or not. It will if the name
 *                 field is ok.
 * PRE          :  The druid window GMObject.
 */
static void gm_dw_check_name (GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Checks if the "Next" button of the "Callto URL"
 *                 druid page can be sensitive or not. It will if the email
 *                 field is ok, or if registering is disabled.
 * PRE          :  The druid window GMObject.
 */
static void gm_dw_check_callto (GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Updates the given GtkOptionMenu with the given array and
 * 		   sets the default value.
 * PRE          :  /
 */
static void gm_dw_option_menu_update (GtkWidget *, 
				      gchar **,
				      gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Retrieves the data from the various fields of the druid 
 * 		   window GMObject and fills the parameters in.
 * PRE          :  A valid pointer to the druid window GMObject.
 */
static void gm_dw_get_all_data (GtkWidget *,
				gchar *&, 
				gchar *&,
				gchar *&, 
				gchar *&, 
				gchar *&,
				gchar *&, 
				gchar *&, 
				gchar *&);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Init the welcome page.
 * PRE          :  A valid pointer to the druid window GMObject. Followed by
 * 		   the current page number.
 */
static void gm_dw_init_welcome_page (GtkWidget *, 
				     int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Init the personal data page.
 * PRE          :  A valid pointer to the druid window GMObject. Followed by
 * 		   the current page number and the total pages number.
 */
static void gm_dw_init_personal_data_page (GtkWidget *, 
					   int, 
					   int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Init the callto url page.
 * PRE          :  A valid pointer to the druid window GMObject. Followed by
 * 		   the current page number and the total pages number.
 */
static void gm_dw_init_callto_page (GtkWidget *, 
				    int, 
				    int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Init the connection type page.
 * PRE          :  A valid pointer to the druid window GMObject. Followed by
 * 		   the current page number and the total pages number.
 */
static void gm_dw_init_connection_type_page (GtkWidget *, 
					     int, 
					     int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Init the NAT type page.
 * PRE          :  A valid pointer to the druid window GMObject. Followed by
 * 		   the current page number and the total pages number.
 */
static void gm_dw_init_nat_type_page (GtkWidget *, 
				      int, 
				      int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Init the audio plugin page.
 * PRE          :  A valid pointer to the druid window GMObject. Followed by
 * 		   the current page number and the total pages number.
 */
static void gm_dw_init_audio_manager_page (GtkWidget *, 
					   int, 
					   int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Init the audio devices page.
 * PRE          :  A valid pointer to the druid window GMObject. Followed by
 * 		   the current page number and the total page number.
 */
static void gm_dw_init_audio_devices_page (GtkWidget *, 
					   int, 
					   int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Init the video manager page.
 * PRE          :  A valid pointer to the druid window GMObject. Followed by
 * 		   the current page number and the total pages number.
 */
static void gm_dw_init_video_manager_page (GtkWidget *, 
					   int, 
					   int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Init the video devices page.
 * PRE          :  A valid pointer to the druid window GMObject. Followed by
 * 		   the current page number and the total pages number.
 */
static void gm_dw_init_video_devices_page (GtkWidget *, 
					   int, 
					   int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Init the summary page.
 * PRE          :  A valid pointer to the druid window GMObject. Followed by
 * 		   the total pages number.
 */
static void gm_dw_init_final_page (GtkWidget *, 
				   int);


/* GTK Callbacks */

/* DESCRIPTION  : Timeout function called with delay of 2 seconds to update
 * 		  the kind_of_net GConf key to avoid collision with
 * 		  other notifiers triggered when video gconf related keys
 * 		  are updated (leading to kind_of_net being set to 5). 
 * 		  Ugly hack.
 * BEHAVIOR     : Updates the key.
 * PRE          : GPOINTER_TO_INT (data) = 1 = PSTN, 2 = ISDN, 
 * 		3 = DSL/CABLE, 4 = LAN, 5 = Custom
 */
static gint kind_of_net_hack_cb (gpointer);


/* DESCRIPTION  : Callback called when the audio test button is clicked in the
 * 		  druid window GMObject.
 * BEHAVIOR     : Start or stop the GMH323EndPoint Audio Tester following
 * 		  the toggle is active or not.
 * PRE          : A valid pointer to a valid druid window GMObject.
 */
static void audio_test_button_clicked_cb (GtkWidget *,
					  gpointer);


/* DESCRIPTION  : Callback called when the video test button is clicked in the
 * 		  druid window GMObject.
 * BEHAVIOR     : Start a Video Tester thread.
 * PRE          : A valid pointer to a valid druid window GMObject.
 */
static void video_test_button_clicked_cb (GtkWidget *,
					  gpointer);


/* DESCRIPTION  :  This callback is called when the user clicks on Cancel.
 * BEHAVIOR     :  Hides the druid and shows GM.
 * PRE          :  A valid pointer to a valid druid window GMObject.
 */
static void cancel_cb (GtkWidget *, 
		       gpointer);


/* DESCRIPTION  :  This callback is called when the user clicks on finish.
 * BEHAVIOR     :  Destroys the druid, update config settings and update
 *                 the internal structures for devices and the corresponding
 *                 prefs window menus. Displays a welcome message.
 * PRE          :  A valid pointer to a valid druid window GMObject.
 */
static void finish_cb (GnomeDruidPage *, 
		       GtkWidget *, 
		       gpointer);


/* DESCRIPTION  :  This callback is called when the user destroys the druid.
 * BEHAVIOR     :  Exits. 
 * PRE          :  A Valid pointer to a valid druid window GMObject. 
 */
static void delete_event_cb (GtkWidget *, 
			     GdkEventAny *, 
			     gpointer);


/* DESCRIPTION  :  Called when the user changes an info in the Personal
 *                 Information page.
 * BEHAVIOR     :  Checks if the "Next" button of the "Personal Information"
 *                 druid page can be sensitive or not. It will if the name
 *                 field is ok.
 * PRE          :  The druid window GMObject.
 */
static void name_changed_cb (GtkWidget *, 
			     gpointer);


/* DESCRIPTION  :  Called when the user changes an info in the Callto URL page.
 * BEHAVIOR     :  Checks if the "Next" button of the "Callto URL"
 *                 druid page can be sensitive or not. It will if email field
 *                 is ok, or if registering is disabled.
 * PRE          :  The druid window GMObject.
 */
static void email_changed_cb (GtkWidget *, 
			      gpointer);


/* DESCRIPTION  :  Called when the user changes the registering toggle.
 * BEHAVIOR     :  Checks if the "Next" button of the "Personal Information"
 *                 druid page can be sensitive or not. It will if all fields
 *                 are ok, or if registering is disabled. (Calls the above
 *                 function).
 * PRE          :  The druid window GMObject.
 */
static void ils_register_toggled_cb (GtkToggleButton *, 
				     gpointer);


/* DESCRIPTION  :  Called when the user switches from one page to another.
 * BEHAVIOR     :  Updates the Back/Next buttons accordingly following
 * 		   if all fields are correct or not.
 * PRE          :  /
 */
static void prepare_welcome_page_cb (GnomeDruidPage *,
				     GnomeDruid *, 
				     gpointer);


/* DESCRIPTION  :  Called when the user switches from one page to another.
 * BEHAVIOR     :  Updates the Back/Next buttons accordingly following
 * 		   if all fields are correct or not. Updates the fields
 * 		   contents for all pages of the druid following the current
 * 		   preferences.
 * PRE          :  The druid window GMObject.
 */
static void prepare_personal_data_page_cb (GnomeDruidPage *,
					   GnomeDruid *,
					   gpointer);


/* DESCRIPTION  :  Called when the user switches from one page to another.
 * BEHAVIOR     :  Updates the Back/Next buttons accordingly following
 * 		   if all fields are correct (not register and no email, or
 * 		   register to ILS and an email specified).
 * PRE          :  The druid window GMObject.
 */
static void prepare_callto_page_cb (GnomeDruidPage *,
				    GnomeDruid *, 
				    gpointer);


/* DESCRIPTION  :  Called when the user switches from one page to another.
 * BEHAVIOR     :  Updates the Back/Next buttons accordingly following
 * 		   if all fields are correct or not. Updates the audio
 * 		   devices list following the audio manager choosen at the
 * 		   previous page.
 * PRE          :  The druid window GMObject.
 */
static void prepare_audio_devices_page_cb (GnomeDruidPage *,
					   GnomeDruid *,
					   gpointer);


/* DESCRIPTION  :  Called when the user switches from one page to another.
 * BEHAVIOR     :  Updates the Back/Next buttons accordingly following
 * 		   if all fields are correct or not. Updates the video devices
 * 		   list following the video manager chosen at the previous 
 * 		   page.
 * PRE          :  The druid window GMObject.
 */
static void prepare_video_devices_page_cb (GnomeDruidPage *,
					   GnomeDruid *, 
					   gpointer);


/* DESCRIPTION  :  Called when the user switches from one page to another.
 * BEHAVIOR     :  Updates the Back/Next buttons accordingly following
 * 		   if all fields are correct or not. Prepares the summary
 * 		   of preferences.
 * PRE          :  The druid window GMObject.
 */
static void prepare_final_page_cb (GnomeDruidPage *,
				   GnomeDruid *, 
				   gpointer);


/* DESCRIPTION  :  Called when the user clicks on the NAT detect button.
 * BEHAVIOR     :  Detects the NAT type and displays an help dialog.
 * PRE          :  The druid window GMObject.
 */
static void nat_detect_button_clicked_cb (GtkWidget *,
					  gpointer);


static void 
gm_dw_destroy (gpointer d)
{
  GmDruidWindow *dw = NULL;
  
  g_return_if_fail (d != NULL);
 
  dw = GM_DRUID_WINDOW (d);
  
  if (dw->stun_client)
    delete (dw->stun_client);
  
  delete (dw);
}


GmDruidWindow *
gm_dw_get_dw (GtkWidget *druid_window)
{
    g_return_val_if_fail (druid_window != NULL, NULL);

      return GM_DRUID_WINDOW (g_object_get_data (G_OBJECT (druid_window), "GMObject"));
}


static void 
gm_dw_check_name (GtkWidget *druid_window)
{
  GmDruidWindow *dw = NULL;
  
  gchar ** couple = NULL;

  g_return_if_fail (druid_window != NULL);

  dw = gm_dw_get_dw (druid_window);
  
  couple = g_strsplit (gtk_entry_get_text (GTK_ENTRY (dw->name)), " ", 2);
  
  if (couple && couple [0] && couple [1]
      && !PString (couple [0]).Trim ().IsEmpty ()
      && !PString (couple [1]).Trim ().IsEmpty ()) 
    gnome_druid_set_buttons_sensitive (dw->druid, TRUE, TRUE, TRUE, FALSE);
  else
    gnome_druid_set_buttons_sensitive (dw->druid, TRUE, FALSE, TRUE, FALSE);
}


static void 
gm_dw_check_callto (GtkWidget *druid_window)
{
  GmDruidWindow *dw = NULL;
  
  PString mail;
  BOOL correct = FALSE;


  g_return_if_fail (druid_window != NULL);

  dw = gm_dw_get_dw (druid_window); 

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dw->use_callto)))
    correct = TRUE;
  else {    
    mail = PString (gtk_entry_get_text (GTK_ENTRY (dw->mail)));
    if (!mail.IsEmpty () && mail.Find ("@") != P_MAX_INDEX)
      correct = TRUE;
  }
   
  if (correct)
    gnome_druid_set_buttons_sensitive (dw->druid, TRUE, TRUE, TRUE, FALSE);
  else
    gnome_druid_set_buttons_sensitive (dw->druid, TRUE, FALSE, TRUE, FALSE);
}


static void
gm_dw_option_menu_update (GtkWidget *option_menu,
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

  if (!options)
    return;


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
gm_dw_get_all_data (GtkWidget *druid_window,
		    gchar * &name,
		    gchar * &mail,
		    gchar * &connection_type,
		    gchar * &audio_manager,
		    gchar * &player,
		    gchar * &recorder,
		    gchar * &video_manager,
		    gchar * &video_recorder)
{
  GmDruidWindow *dw = NULL;
  
  GtkWidget *child = NULL;
  
  g_return_if_fail (druid_window != NULL);
  
  dw = gm_dw_get_dw (druid_window);
  
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
gm_dw_init_welcome_page (GtkWidget *druid_window,
			 int t)
{
  GmDruidWindow *dw = NULL;

  gchar *title = NULL;

  static const gchar text[] =
    N_
    ("This is the GnomeMeeting general configuration druid. "
     "The following steps will set up GnomeMeeting by asking "
     "a few simple questions.\n\nOnce you have completed "
     "these steps, you can always change them later by "
     "selecting Preferences in the Edit menu.");


  g_return_if_fail (druid_window != NULL);

  dw = gm_dw_get_dw (druid_window);

  
  title = g_strdup_printf (_("Configuration Druid - page 1/%d"), t);
  
  dw->page_edge =
    GNOME_DRUID_PAGE_EDGE (gnome_druid_page_edge_new_aa (GNOME_EDGE_START));
  gnome_druid_page_edge_set_title (dw->page_edge, title);
			   
  gnome_druid_page_edge_set_text (dw->page_edge, _(text));
  
  gnome_druid_append_page (dw->druid, GNOME_DRUID_PAGE (dw->page_edge));
  gnome_druid_set_page (dw->druid, GNOME_DRUID_PAGE (dw->page_edge));
  
  g_signal_connect_after (G_OBJECT (dw->page_edge), "prepare",
			  G_CALLBACK (prepare_welcome_page_cb), 
			  druid_window);

  g_free (title);
}


static void 
gm_dw_init_personal_data_page (GtkWidget *druid_window,
			       int p, 
			       int t)
{
  GmDruidWindow *dw = NULL;
  
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GtkWidget *page = NULL;


  g_return_if_fail (druid_window != NULL);

  dw = gm_dw_get_dw (druid_window);

  
  page = gnome_druid_page_standard_new ();

  title = g_strdup_printf (_("Personal Information - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (GNOME_DRUID_PAGE_STANDARD (page),
				       title);
  g_free (title);
  
  gnome_druid_append_page (dw->druid, GNOME_DRUID_PAGE (page));

  
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
		    G_CALLBACK (name_changed_cb), 
		    druid_window);

  g_signal_connect_after (G_OBJECT (page), "prepare",
			  G_CALLBACK (prepare_personal_data_page_cb), 
			  druid_window);
  
  gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page)->vbox),
		      GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}


static void 
gm_dw_init_callto_page (GtkWidget *druid_window,
			int p,
			int t)
{
  GmDruidWindow *dw = NULL;
  
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *align = NULL;
  
  gchar *title = NULL;
  gchar *text = NULL;
  
  GtkWidget *page = NULL;


  g_return_if_fail (druid_window != NULL);

  dw = gm_dw_get_dw (druid_window);


  page = gnome_druid_page_standard_new ();

  title = g_strdup_printf (_("Callto URL - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (GNOME_DRUID_PAGE_STANDARD (page),
				       title);
  g_free (title);
				
  gnome_druid_append_page (dw->druid, GNOME_DRUID_PAGE (page));

  
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
		    G_CALLBACK (email_changed_cb), 
		    druid_window);

  g_signal_connect (G_OBJECT (dw->use_callto), "toggled",
		    G_CALLBACK (ils_register_toggled_cb), 
		    druid_window);

  g_signal_connect_after (G_OBJECT (page), "prepare",
			  G_CALLBACK (prepare_callto_page_cb), 
			  druid_window);
  
  gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page)->vbox),
		      GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}


static void 
gm_dw_init_connection_type_page (GtkWidget *druid_window,
				 int p, 
				 int t)
{
  GmDruidWindow *dw = NULL;
  
  GtkWidget *vbox = NULL;
  GtkWidget *label = NULL;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GnomeDruidPageStandard *page_standard = NULL;


  g_return_if_fail (druid_window != NULL);
  
  dw = gm_dw_get_dw (druid_window);

  
  /* Get data */
  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  
  title = g_strdup_printf (_("Connection Type - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (dw->druid, GNOME_DRUID_PAGE (page_standard));


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


static void 
gm_dw_init_nat_type_page (GtkWidget *druid_window,
			  int p, 
			  int t)
{
  GmDruidWindow *dw = NULL;
  
  GtkWidget *vbox = NULL;
  GtkWidget *label = NULL;
  GtkWidget *button = NULL;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GnomeDruidPageStandard *page_standard = NULL;


  g_return_if_fail (druid_window != NULL);
  
  dw = gm_dw_get_dw (druid_window);

  
  /* Get data */
  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  
  title = g_strdup_printf (_("NAT Type - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (dw->druid, GNOME_DRUID_PAGE (page_standard));


  /* Start packing widgets */
  vbox = gtk_vbox_new (FALSE, 2);

  label = gtk_label_new (_("Click here to detect your NAT Type:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  button = gtk_button_new_with_label (_("Detect NAT Type"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  
  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The NAT type detection will permit to assist you in configuring your NAT router to be able to do calls with GnomeMeeting."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  g_signal_connect (G_OBJECT (button), "clicked",
		    GTK_SIGNAL_FUNC (nat_detect_button_clicked_cb), 
		    (gpointer) druid_window);

  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}


static void 
gm_dw_init_audio_manager_page (GtkWidget *druid_window,
			       int p,
			       int t)
{
  GmDruidWindow *dw = NULL;
  
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GnomeDruidPageStandard *page_standard = NULL;


  g_return_if_fail (druid_window != NULL);

  dw = gm_dw_get_dw (druid_window);
  

  /* Get data */  
  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  
  title = g_strdup_printf (_("Audio Manager - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (dw->druid, GNOME_DRUID_PAGE (page_standard));


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


static void 
gm_dw_init_audio_devices_page (GtkWidget *druid_window, 
			       int p, 
			       int t)
{
  GmDruidWindow *dw = NULL;
  
  GtkWidget *align = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *label = NULL;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GnomeDruidPageStandard *page_standard = NULL;


  g_return_if_fail (druid_window);

  dw = gm_dw_get_dw (druid_window);

  
  /* Get data */
  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  
  title = g_strdup_printf (_("Audio Devices - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (dw->druid, GNOME_DRUID_PAGE (page_standard));


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
		    GTK_SIGNAL_FUNC (audio_test_button_clicked_cb),
		    (gpointer) druid_window);

  g_signal_connect_after (G_OBJECT (page_standard), "prepare",
			  G_CALLBACK (prepare_audio_devices_page_cb), 
			  druid_window);


  /**/
  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}


static void 
gm_dw_init_video_manager_page (GtkWidget *druid_window, 
			       int p, 
			       int t)
{
  GmDruidWindow *dw = NULL;
  
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GnomeDruidPageStandard *page_standard = NULL;


  g_return_if_fail (druid_window != NULL);

  dw = gm_dw_get_dw (druid_window);

  
  /* Get data */
  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  
  title = g_strdup_printf (_("Video Manager - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (dw->druid, GNOME_DRUID_PAGE (page_standard));


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


static void 
gm_dw_init_video_devices_page (GtkWidget *druid_window,
			       int p, 
			       int t)
{
  GmDruidWindow *dw = NULL;
  
  GtkWidget *align = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *label = NULL;

  gchar *title = NULL;
  gchar *text = NULL;
  
  GnomeDruidPageStandard *page_standard = NULL;


  g_return_if_fail (druid_window != NULL);

  dw = gm_dw_get_dw (druid_window);


  /* Get data */
  page_standard = 
    GNOME_DRUID_PAGE_STANDARD (gnome_druid_page_standard_new ());
  
  title = g_strdup_printf (_("Video Devices - page %d/%d"), p, t);
  gnome_druid_page_standard_set_title (page_standard, title);
  g_free (title);

  gnome_druid_append_page (dw->druid, GNOME_DRUID_PAGE (page_standard));


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
		    GTK_SIGNAL_FUNC (video_test_button_clicked_cb),
		    (gpointer) druid_window);

  g_signal_connect_after (G_OBJECT (page_standard), "prepare",
			  G_CALLBACK (prepare_video_devices_page_cb), 
			  druid_window);

  /**/
  gtk_box_pack_start (GTK_BOX (page_standard->vbox), GTK_WIDGET (vbox), 
		      TRUE, TRUE, 8);
}


static void
gm_dw_init_final_page (GtkWidget *druid_window, 
		       int t)
{
  GmDruidWindow *dw = NULL;
  
  gchar *title = NULL;

  GnomeDruidPageEdge *page_final = NULL;

  g_return_if_fail (druid_window != NULL);

  dw = gm_dw_get_dw (druid_window);


  page_final =
    GNOME_DRUID_PAGE_EDGE (gnome_druid_page_edge_new (GNOME_EDGE_FINISH));
  
  title = g_strdup_printf (_("Configuration complete - page %d/%d"), t, t);
  gnome_druid_page_edge_set_title (page_final, title);

  gnome_druid_append_page (dw->druid, GNOME_DRUID_PAGE (page_final));

  g_signal_connect_after (G_OBJECT (page_final), "prepare",
			  G_CALLBACK (prepare_final_page_cb), 
			  druid_window);  

  g_signal_connect (G_OBJECT (page_final), "finish",
		    G_CALLBACK (finish_cb), 
		    druid_window);

  g_free (title);
}


/* GTK Callbacks */
static gint
kind_of_net_hack_cb (gpointer data)
{
  gm_conf_set_int (GENERAL_KEY "kind_of_net", 
		   GPOINTER_TO_INT (data));

  return FALSE;
}


static void
audio_test_button_clicked_cb (GtkWidget *w,
			      gpointer data)
{ 
  GMH323EndPoint *ep = NULL;

  GtkWidget *druid_window = NULL;
  
  gchar *name = NULL;
  gchar *con_type = NULL;
  gchar *mail = NULL;
  gchar *audio_manager = NULL;
  gchar *player = NULL;
  gchar *recorder = NULL;
  gchar *video_manager = NULL;
  gchar *video_recorder = NULL;
  

  g_return_if_fail (data != NULL);
  
  ep = GnomeMeeting::Process ()->Endpoint ();
  druid_window = GTK_WIDGET (data);

  gm_dw_get_all_data (druid_window, 
		      name, 
		      mail, 
		      con_type, 
		      audio_manager, 
		      player,
		      recorder, 
		      video_manager, 
		      video_recorder);


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
video_test_button_clicked_cb (GtkWidget *w,
			      gpointer data)
{
  GMVideoTester *t = NULL;

  GtkWidget *druid_window = NULL;
  
  gchar *name = NULL;
  gchar *con_type = NULL;
  gchar *mail = NULL;
  gchar *audio_manager = NULL;
  gchar *player = NULL;
  gchar *recorder = NULL;
  gchar *video_manager = NULL;
  gchar *video_recorder = NULL;

  g_return_if_fail (data != NULL);

  druid_window = GTK_WIDGET (data);
  
  gm_dw_get_all_data (druid_window, 
		      name, 
		      mail, 
		      con_type, 
		      audio_manager, 
		      player,
		      recorder, 
		      video_manager, 
		      video_recorder);

  if (GTK_TOGGLE_BUTTON (w)->active)   
    t = new GMVideoTester (video_manager, video_recorder);
}


static void 
cancel_cb (GtkWidget *w, 
	   gpointer data)
{
  GtkWidget *main_window = NULL;
  
  GmDruidWindow *dw = NULL;

  g_return_if_fail (data != NULL);

  dw = gm_dw_get_dw (GTK_WIDGET (data));
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  gnome_druid_set_page (dw->druid, GNOME_DRUID_PAGE (dw->page_edge));
  gnomemeeting_window_hide (GTK_WIDGET (data));
  
  gnomemeeting_window_show (main_window);
}


static void 
finish_cb (GnomeDruidPage *p, 
	   GtkWidget *w, 
	   gpointer data)
{
  GmDruidWindow *dw = NULL;

  GMH323EndPoint *ep = NULL;
  
  GtkWidget *druid_window = NULL;
  GtkWidget *main_window = NULL;
  GtkWidget *prefs_window = NULL;
  
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

  PStringArray audio_input_devices;
  PStringArray audio_output_devices;
  PStringArray video_input_devices;

  
  g_return_if_fail (data != NULL);
  
  druid_window = GTK_WIDGET (data);

  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();

  dw = gm_dw_get_dw (druid_window);
  ep = GnomeMeeting::Process ()->Endpoint ();

  
  active_item =
    gtk_menu_get_active (GTK_MENU (GTK_OPTION_MENU (dw->kind_of_net)->menu));
  item_index =
    g_list_index (GTK_MENU_SHELL (GTK_MENU (GTK_OPTION_MENU (dw->kind_of_net)->menu))->children, active_item) + 1;


  gm_dw_get_all_data (druid_window, 
		      name, 
		      mail, 
		      con_type, 
		      audio_manager, 
		      player,
		      recorder, 
		      video_manager, 
		      video_recorder);

  
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
    
    gm_conf_set_int (VIDEO_CODECS_KEY "transmitted_video_quality", 10);
    gm_conf_set_int (VIDEO_CODECS_KEY "maximum_video_bandwidth", 1);
    gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video_transmission", FALSE);
    gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video_reception", FALSE);
  }
  else if (item_index == 2) { /* ISDN */
    
    gm_conf_set_int (VIDEO_CODECS_KEY "transmitted_video_quality", 20);
    gm_conf_set_int (VIDEO_CODECS_KEY "maximum_video_bandwidth", 2);
    gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video_transmission", FALSE);
    gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video_reception", FALSE);
  }
  else if (item_index == 3) { /* DSL / CABLE */
    
    gm_conf_set_int (VIDEO_CODECS_KEY "transmitted_video_quality", 80);
    gm_conf_set_int (VIDEO_CODECS_KEY "maximum_video_bandwidth", 8);
    gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video_transmission",
		      has_video_device);
    gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video_reception", TRUE);
  }
  else if (item_index == 4) { /* LAN */
    
    gm_conf_set_int (VIDEO_CODECS_KEY "transmitted_video_quality", 100);
    gm_conf_set_int (VIDEO_CODECS_KEY "maximum_video_bandwidth", 100);
    gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video_transmission",
		      has_video_device);
    gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video_reception", TRUE);
  }  

  g_timeout_add (2000, 
		 (GtkFunction) kind_of_net_hack_cb,
		 GINT_TO_POINTER (item_index));

  
  /* Set User Name and Alias */
  ep->SetUserNameAndAlias ();
  
  
  /* Hide the druid and show GnomeMeeting */
  gnomemeeting_window_hide (druid_window);
  gnome_druid_set_page (dw->druid, GNOME_DRUID_PAGE (dw->page_edge));
  gnomemeeting_window_show (main_window);


  /* Will be done through the config if the manager changes, but not
     if the manager doesn't change */
  GnomeMeeting::Process ()->DetectDevices ();  
  

  /* Update the version number */
  version = MAJOR_VERSION*1000+MINOR_VERSION*10+BUILD_NUMBER;
    
  gm_conf_set_int (GENERAL_KEY "version", version);
}


static void 
delete_event_cb (GtkWidget *w, GdkEventAny *ev, gpointer data)
{
  cancel_cb (w, data);
}


static void
name_changed_cb (GtkWidget *w, 
		 gpointer data)
{
  g_return_if_fail (data != NULL);

  gm_dw_check_name (GTK_WIDGET (data));
}


static void
email_changed_cb (GtkWidget *w, 
		  gpointer data)
{
  g_return_if_fail (data != NULL);

  gm_dw_check_callto (GTK_WIDGET (data));
}


static void
ils_register_toggled_cb (GtkToggleButton *b, 
			 gpointer data)
{
  g_return_if_fail (data != NULL);

  gm_dw_check_callto (GTK_WIDGET (data));
}


static void 
prepare_welcome_page_cb (GnomeDruidPage *page,
			 GnomeDruid *druid, 
			 gpointer data)
{
  gnome_druid_set_buttons_sensitive (druid, FALSE, TRUE, TRUE, FALSE);
}


static void
prepare_personal_data_page_cb (GnomeDruidPage *page,
			       GnomeDruid *druid, 
			       gpointer data)
{
  GmDruidWindow *dw = NULL;
  
  PStringArray devs;
  
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
 

  /* Get the data */
  g_return_if_fail (page != NULL && druid != NULL && data != NULL);

  dw = gm_dw_get_dw (GTK_WIDGET (data));

  
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
  
  gm_dw_option_menu_update (dw->kind_of_net, options, NULL);
  gtk_option_menu_set_history (GTK_OPTION_MENU (dw->kind_of_net),
			       kind_of_net - 1);
  
  devs = GnomeMeeting::Process ()->GetAudioPlugins ();
  array = devs.ToCharArray ();
  audio_manager = gm_conf_get_string (AUDIO_DEVICES_KEY "plugin");
  gm_dw_option_menu_update (dw->audio_manager, array, audio_manager);
  free (array);
  
  devs = GnomeMeeting::Process ()->GetVideoPlugins ();
  array = devs.ToCharArray ();
  video_manager = gm_conf_get_string (VIDEO_DEVICES_KEY "plugin");
  gm_dw_option_menu_update (dw->video_manager, array, video_manager);
  free (array);
  
  GTK_TOGGLE_BUTTON (dw->use_callto)->active = !ils_register;
  
  gm_dw_check_name (GTK_WIDGET (data));
  
  g_free (video_manager);
  g_free (audio_manager);    
  g_free (mail);
  g_free (firstname);
  g_free (lastname);

}


static void 
prepare_callto_page_cb (GnomeDruidPage *page, 
			GnomeDruid *druid, 
			gpointer data)
{
  GmDruidWindow *dw = NULL;
  
  
  /* Get the data */
  g_return_if_fail (page != NULL && druid != NULL && data != NULL);

  dw = gm_dw_get_dw (GTK_WIDGET (data));

  gm_dw_check_callto (GTK_WIDGET (data));
}


static void
prepare_audio_devices_page_cb (GnomeDruidPage *page, 
			       GnomeDruid *druid,
			       gpointer data)
{
  GmDruidWindow *dw = NULL;

  GMH323EndPoint *ep = NULL;

  GtkWidget *child = NULL;
  gchar *audio_manager = NULL;
  gchar *player = NULL;
  gchar *recorder = NULL;
  PStringArray devices;
  char **array = NULL;


  /* Get the data */
  g_return_if_fail (page != NULL && druid != NULL && data != NULL);

  dw = gm_dw_get_dw (GTK_WIDGET (data));

  ep = GnomeMeeting::Process ()->Endpoint ();
  
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dw->audio_test_button),
				FALSE);
  
  child = GTK_BIN (dw->audio_manager)->child;
  
  if (child)
    audio_manager = (gchar *) gtk_label_get_text (GTK_LABEL (child));
  else
    audio_manager = "";
	
  player = gm_conf_get_string (AUDIO_DEVICES_KEY "output_device");
  recorder = gm_conf_get_string (AUDIO_DEVICES_KEY "input_device");
    

  /* FIXME: We should use DetectDevices, however DetectDevices
   * works only for the currently selected audio and video plugins,
   * not for a random one.
   */
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
  gm_dw_option_menu_update (dw->audio_player, array, player);
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
  gm_dw_option_menu_update (dw->audio_recorder, array, recorder);
  free (array);
  gnomemeeting_sound_daemons_resume ();

  
  g_free (player);
  g_free (recorder);
  
  if (ep->GetCallingState () != GMH323EndPoint::Standby)
    gtk_widget_set_sensitive (GTK_WIDGET (dw->audio_test_button), FALSE);
}


static void
prepare_video_devices_page_cb (GnomeDruidPage *page, 
			       GnomeDruid *druid, 
			       gpointer data)
{
  GmDruidWindow *dw = NULL;
  
  GMH323EndPoint *ep = NULL;

  GdkCursor *cursor = NULL;
  GtkWidget *child = NULL;
  GtkWidget *druid_window = NULL;
  
  gchar *video_manager = NULL;
  gchar *video_recorder = NULL;
  PStringArray devices;
  char **array = NULL;
 

  /* Get the data */
  g_return_if_fail (page != NULL && druid != NULL && data != NULL);

  dw = gm_dw_get_dw (GTK_WIDGET (data));

  ep = GnomeMeeting::Process ()->Endpoint ();

  druid_window = GTK_WIDGET (data);


  cursor = gdk_cursor_new (GDK_WATCH);
  gdk_window_set_cursor (GTK_WIDGET (druid_window)->window, cursor);
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
  gm_dw_option_menu_update (dw->video_device, array, video_recorder);
  free (array);
  
  gdk_window_set_cursor (GTK_WIDGET (data)->window, NULL);
  
  g_free (video_recorder);
  
  if (ep->GetCallingState () != GMH323EndPoint::Standby)
    gtk_widget_set_sensitive (GTK_WIDGET (dw->video_test_button), FALSE);

}


static void
prepare_final_page_cb (GnomeDruidPage *page,
		       GnomeDruid *druid,
		       gpointer data)
{
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
  

  /* Get the data */
  g_return_if_fail (page != NULL && druid != NULL && data != NULL);

  dw = gm_dw_get_dw (GTK_WIDGET (data));

  ep = GnomeMeeting::Process ()->Endpoint ();

  gm_dw_get_all_data (GTK_WIDGET (data), 
		      name, 
		      mail, 
		      connection_type, 
		      audio_manager,
		      player, 
		      recorder, 
		      video_manager, 
		      video_recorder);

  callto_url = g_strdup_printf ("callto:ils.seconix.com/%s",
				mail ? mail : "");
    
  text = g_strdup_printf (_("You have now finished the GnomeMeeting configuration. All the settings can be changed in the GnomeMeeting preferences. Enjoy!\n\n\nConfiguration summary:\n\nUsername: %s\nConnection type: %s\nAudio manager: %s\nAudio player: %s\nAudio recorder: %s\nVideo manager: %s\nVideo input: %s\nCallto URL: %s\n"), name, connection_type, audio_manager, player, recorder, video_manager, video_recorder, !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dw->use_callto)) ? callto_url : _("None"));
  gnome_druid_page_edge_set_text (GNOME_DRUID_PAGE_EDGE (page), text);
  
  g_free (callto_url);
  g_free (text);
}


static void
nat_detect_button_clicked_cb (GtkWidget *button,
			      gpointer data)
{
  GmDruidWindow *dw = NULL;

  PString nat_type;

  g_return_if_fail (data != NULL);

  dw = gm_dw_get_dw (GTK_WIDGET (data));

  g_return_if_fail (dw != NULL);

  gdk_threads_leave ();
  if (dw->stun_client) 
    delete (dw->stun_client);
  dw->stun_client = new GMStunClient (FALSE);
  gdk_threads_enter ();
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
  dw->stun_client = NULL;
  g_object_set_data_full (G_OBJECT (window), "GMObject",
			  (gpointer) dw, 
			  gm_dw_destroy);

  dw->druid = GNOME_DRUID (gnome_druid_new ());

  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (dw->druid));

  /* Create the different pages */
  gm_dw_init_welcome_page (window, 10);
  gm_dw_init_personal_data_page (window, 2, 10);
  gm_dw_init_callto_page (window, 3, 10);
  gm_dw_init_connection_type_page (window, 4, 10);
  gm_dw_init_nat_type_page (window, 5, 10);
  gm_dw_init_audio_manager_page (window, 6, 10);
  gm_dw_init_audio_devices_page (window, 7, 10);
  gm_dw_init_video_manager_page (window, 8, 10);
  gm_dw_init_video_devices_page (window, 9, 10);
  gm_dw_init_final_page (window, 10);

  g_signal_connect (G_OBJECT (dw->druid), "cancel",
		    G_CALLBACK (cancel_cb), 
		    window);

  g_signal_connect (G_OBJECT (window), "delete_event",
		    G_CALLBACK (delete_event_cb), 
		    window);

  gtk_widget_show_all (GTK_WIDGET (dw->druid));

  return window;
}

void 
gm_druid_window_set_test_buttons_sensitivity (GtkWidget *druid,
					      gboolean value)
{
  GmDruidWindow *dw = NULL;

  dw = gm_dw_get_dw (druid);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dw->video_test_button),
				value);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dw->audio_test_button),
				value);

}
