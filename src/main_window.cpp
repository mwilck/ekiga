
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
 *                         main_interface.cpp  -  description
 *                         ----------------------------------
 *   begin                : Mon Mar 26 2001
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          build the main window.
 *   email                : dsandras@seconix.com
 */

#include "../config.h"

#include <orb/orbit.h>
extern "C" {
#include <libgnorba/gnorba.h>
#include <libgnomeui/gnome-window-icon.h>
}

#include "main_window.h"
#include "gnomemeeting.h"
#include "callbacks.h"
#include "docklet.h"
#include "ils.h"
#include "common.h"
#include "menu.h"
#include "toolbar.h"
#include "callbacks.h"
#include "audio.h"
#include "videograbber.h"
#include "endpoint.h"
#include "pref_window.h"
#include "config.h"
#include "misc.h"
#include "e-splash.h"

#include <gconf/gconf-client.h>

#include "../pixmaps/speaker.xpm"
#include "../pixmaps/mic.xpm"
#include "../pixmaps/brightness.xpm"
#include "../pixmaps/whiteness.xpm"
#include "../pixmaps/contrast.xpm"
#include "../pixmaps/color.xpm"
#include "../pixmaps/eye.xpm"
#include "../pixmaps/quickcam.xpm"
#include "../pixmaps/left_arrow.xpm"
#include "../pixmaps/right_arrow.xpm"
#include "../pixmaps/sample.xpm"


/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

static gint expose_event (GtkWidget *, GdkEventExpose *, gpointer);
static void audio_volume_changed (GtkAdjustment *, gpointer);
static void brightness_changed (GtkAdjustment *, gpointer);
static void whiteness_changed (GtkAdjustment *, gpointer);
static void colour_changed (GtkAdjustment *, gpointer);
static void contrast_changed (GtkAdjustment *, gpointer);
static void left_arrow_clicked (GtkWidget *, gpointer);
static void right_arrow_clicked (GtkWidget *, gpointer);
static void silence_detection_button_clicked (GtkWidget *, gpointer);

static void gnomemeeting_init_main_window ();
static gint gm_quit_callback (GtkWidget *, GdkEvent *, gpointer);
static void gnomemeeting_init_main_window_video_settings ();
static void gnomemeeting_init_main_window_audio_settings ();
static void gnomemeeting_init_main_window_log  ();
static void gnomemeeting_init_main_window_remote_user_info ();
static void notebook_page_changed_callback (GtkNotebook *, gpointer);


/* GTK Callbacks */

/* DESCRIPTION  :  This callback is called when something the
*                  page selected in the notebook changes
 * BEHAVIOR     :  Updated the gconf entry
 * PRE          :  /
 */
static void notebook_page_changed_callback (GtkNotebook *notebook, gpointer)
{
  GConfClient *client = gconf_client_get_default ();

  gconf_client_set_int (client, "/apps/gnomemeeting/view/notebook_info",
			gtk_notebook_get_current_page (notebook), 0);
}


/* DESCRIPTION  :  This callback is called when the main window is covered by
 *                 another window or updated.
 * BEHAVIOR     :  Update it
 * PRE          :  gpointer is a valid pointer to GM_window_widgets
 */
gint expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;

  gdk_draw_pixmap(widget->window,
		  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		  gw->pixmap,
		  event->area.x, event->area.y,
		  event->area.x, event->area.y,
		  event->area.width, event->area.height);

  return FALSE;
}


/* DESCRIPTION  :  This callback is called when the user changes the
 *                 audio settings sliders in the main notebook.
 * BEHAVIOR     :  Update the volume.
 * PRE          :  gpointer is a valid pointer to GM_pref_window_widgets
 */
void audio_volume_changed (GtkAdjustment *adjustment, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();
  int vol_play, vol_rec;
  char *audio_recorder_mixer;
  char *audio_player_mixer;

  GM_window_widgets *gw = (GM_window_widgets *) data;
  
  vol_play =  (int) (GTK_ADJUSTMENT (gw->adj_play)->value) * 257;
  vol_rec =  (int) (GTK_ADJUSTMENT (gw->adj_rec)->value) * 257;

  /* return a pointer to the data, not a copy => no need to free */
  audio_player_mixer = gconf_client_get_string (client, "/apps/gnomemeeting/devices/audio_player_mixer", NULL);
  audio_recorder_mixer = gconf_client_get_string (client, "/apps/gnomemeeting/devices/audio_recorder_mixer", NULL);
  
  gnomemeeting_volume_set (audio_player_mixer, 0, &vol_play);
  gnomemeeting_volume_set (audio_recorder_mixer, 1, &vol_rec);

  g_free (audio_player_mixer);
  g_free (audio_recorder_mixer);
}


/* DESCRIPTION  :  This callback is called when the user changes the 
 *                 video brightness slider in the main notebook.
 * BEHAVIOR     :  Update the value in real time.
 * PRE          :  gpointer is a valid pointer to GM_window_widgets
 */
void brightness_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GM_window_widgets *gw = (GM_window_widgets *) data;
  GMVideoGrabber *video_grabber = MyApp->Endpoint ()->GetVideoGrabber ();

  int brightness;

  brightness =  (int) (GTK_ADJUSTMENT (gw->adj_brightness)->value);

  video_grabber->SetBrightness (brightness * 256);
}


/* DESCRIPTION  :  This callback is called when the user changes the 
 *                 video whiteness slider in the main notebook.
 * BEHAVIOR     :  Update the value in real time.
 * PRE          :  gpointer is a valid pointer to GM_window_widgets
 */
void whiteness_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GM_window_widgets *gw = (GM_window_widgets *) data;
  GMVideoGrabber *video_grabber = MyApp->Endpoint ()->GetVideoGrabber ();

  int whiteness;

  whiteness =  (int) (GTK_ADJUSTMENT (gw->adj_whiteness)->value);

  video_grabber->SetWhiteness (whiteness * 256);
}


/* DESCRIPTION  :  This callback is called when the user changes the 
 *                 video colour slider in the main notebook.
 * BEHAVIOR     :  Update the value in real time.
 * PRE          :  gpointer is a valid pointer to GM_window_widgets
 */
void colour_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GM_window_widgets *gw = (GM_window_widgets *) data;
  GMVideoGrabber *video_grabber = MyApp->Endpoint ()->GetVideoGrabber ();

  int colour;

  colour =  (int) (GTK_ADJUSTMENT (gw->adj_colour)->value);

  video_grabber->SetColour (colour * 256);
}


/* DESCRIPTION  :  This callback is called when the user changes the 
 *                 video contrast slider in the main notebook.
 * BEHAVIOR     :  Update the value in real time.
 * PRE          :  gpointer is a valid pointer to GM_window_widgets
 */
void contrast_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GM_window_widgets *gw = (GM_window_widgets *) data;
  GMVideoGrabber *video_grabber = MyApp->Endpoint ()->GetVideoGrabber ();

  int contrast;

  contrast =  (int) (GTK_ADJUSTMENT (gw->adj_contrast)->value);

  video_grabber->SetContrast (contrast * 256);
}


/* DESCRIPTION  :  This callback is called when the user clicks 
 *                 on the left arrow.
 * BEHAVIOR     :  Writes the new selected tab to the gconf db
 * PRE          :  /
 */
void left_arrow_clicked (GtkWidget *w, gpointer data)
{
  GConfClient *client = GCONF_CLIENT (data);
  
  int current = gconf_client_get_int (client,
				      "/apps/gnomemeeting/view/notebook_info", 0);
  gconf_client_set_int (client, "/apps/gnomemeeting/view/notebook_info",
			current - 1, 0);
}


/* DESCRIPTION  :  This callback is called when the user clicks 
 *                 on the right arrow.
 * BEHAVIOR     :  Writes the new selected tab to the gconf db
 * PRE          :  /
 */
void right_arrow_clicked (GtkWidget *w, gpointer data)
{
  GConfClient *client = GCONF_CLIENT (data);
  
  int current = gconf_client_get_int (client,
				      "/apps/gnomemeeting/view/notebook_info", 0);
  gconf_client_set_int (client, "/apps/gnomemeeting/view/notebook_info",
			current + 1, 0);
}


/* DESCRIPTION  :  This callback is called when the user clicks 
 *                 on the silence detection button during a call.
 * BEHAVIOR     :  Enable/Disable silence detection.
 * PRE          :  /
 */
void silence_detection_button_clicked (GtkWidget *w, gpointer data)
{
  MyApp->Endpoint ()->ChangeSilenceDetection ();
}


/* DESCRIPTION  :  This callback is called when the user tries to close
 *                 the application using the window manager.
 * BEHAVIOR     :  Calls the real callback.
 * PRE          :  /
 */
static gint gm_quit_callback (GtkWidget *widget, GdkEvent *event, 
			      gpointer data)
{
  quit_callback (NULL, data);

  return (TRUE);
}  


/* The functions */

void gnomemeeting_init (GM_window_widgets *gw, 
			GM_pref_window_widgets *pw,
			GM_ldap_window_widgets *lw, 
			int argc, char ** argv, char ** envp)
{
  GMH323EndPoint *endpoint = NULL;
  gchar *text = NULL;
  int debug = 0;

  /* Cope with command line options */
  static struct poptOption arguments[] =
    {
      {"debug", 'd', POPT_ARG_NONE, 
       0, 0, N_("Prints debug messages in the console"), NULL},
      {NULL, 0, 0, NULL, 0, NULL, NULL}
    };

  for (int i = 0; i < argc; i++) {
    if (!strcmp (argv[i], "-d") || !strcmp (argv[i], "--debug"))
      debug = 1;
  } 

  /* Gnome Initialisation */
  gnome_init_with_popt_table (PACKAGE, VERSION, argc, argv,
				    arguments, 0, NULL);

  gm = gnome_app_new ("gnomemeeting", _("GnomeMeeting"));
  gtk_window_set_policy (GTK_WINDOW (gm), FALSE, FALSE, TRUE);

  /* Some little gconf stuff */  
  GConfClient *client = gconf_client_get_default ();
  gconf_client_add_dir (client, "/apps/gnomemeeting",
			GCONF_CLIENT_PRELOAD_RECURSIVE,
			0);

  /* We store all the pointers to the structure as data of gm */
  gtk_object_set_data (GTK_OBJECT (gm), "gw", gw);
  gtk_object_set_data (GTK_OBJECT (gm), "lw", lw);
  gtk_object_set_data (GTK_OBJECT (gm), "pw", pw);

  /* Startup Process */
  gw->docklet = gnomemeeting_init_docklet ();

  /* Init the splash screen */
  gw->splash_win = e_splash_new ();
  gtk_signal_connect (GTK_OBJECT (gw->splash_win), "delete_event",
		      GTK_SIGNAL_FUNC (gtk_widget_hide_on_delete), 0);
  int icon_index = 0;

  cout << "FIX ME: Show Splash" << endl << flush;
  if (1) {

    /* We show the splash screen */
    gtk_widget_show (gw->splash_win);
    while (gtk_events_pending ())
      gtk_main_iteration ();
    // Now we load all the icons
    // FIXME: Icon for audio/video devices
    GdkPixbuf *icon_pixbuf = gdk_pixbuf_new_from_file (GNOMEMEETING_IMAGES
						       "/gnomemeeting-logo-icon.png");
    e_splash_add_icon (E_SPLASH (gw->splash_win), icon_pixbuf);
    gdk_pixbuf_unref (icon_pixbuf);
    // FIXME: Icon for main-interface
    icon_pixbuf = gdk_pixbuf_new_from_file (GNOMEMEETING_IMAGES
					     "/gnomemeeting-logo-icon.png");
    e_splash_add_icon (E_SPLASH (gw->splash_win), icon_pixbuf);
    gdk_pixbuf_unref (icon_pixbuf);
    // FIXME: Icon for Audio capabilities
    icon_pixbuf = gdk_pixbuf_new_from_file (GNOMEMEETING_IMAGES
					    "/gnomemeeting-logo-icon.png");
    e_splash_add_icon (E_SPLASH (gw->splash_win), icon_pixbuf);
    gdk_pixbuf_unref (icon_pixbuf);
    // FIXME: Icon for Video capabilities
    icon_pixbuf = gdk_pixbuf_new_from_file (GNOMEMEETING_IMAGES
					    "/gnomemeeting-logo-icon.png");
    e_splash_add_icon (E_SPLASH (gw->splash_win), icon_pixbuf);
    gdk_pixbuf_unref (icon_pixbuf);
    // FIXME: Icon for ILS directory
    if (gconf_client_get_bool (GCONF_CLIENT (client), "/apps/gnomemeeting/ldap/register", NULL)) {
      icon_pixbuf = gdk_pixbuf_new_from_file (GNOMEMEETING_IMAGES
					      "/gnomemeeting-logo-icon.png");
      e_splash_add_icon (E_SPLASH (gw->splash_win), icon_pixbuf);
      gdk_pixbuf_unref (icon_pixbuf); 
    }
    // FIXME: Icon for listener thread
    icon_pixbuf = gdk_pixbuf_new_from_file (GNOMEMEETING_IMAGES
					    "/gnomemeeting-logo-icon.png");
    e_splash_add_icon (E_SPLASH (gw->splash_win), icon_pixbuf);
    gdk_pixbuf_unref (icon_pixbuf);
    // FIXME: Icon for Gatekeeper
    icon_pixbuf = gdk_pixbuf_new_from_file (GNOMEMEETING_IMAGES
					    "/gnomemeeting-logo-icon.png");
    e_splash_add_icon (E_SPLASH (gw->splash_win), icon_pixbuf);
    gdk_pixbuf_unref (icon_pixbuf);
    while (gtk_events_pending ())
      gtk_main_iteration ();
  }
  
  /* Search for devices */
  cout << "FIX ME: Show Splash" << endl << flush;
  if (1) {
    e_splash_set_icon_highlight (E_SPLASH (gw->splash_win), icon_index++, true);
    while (gtk_events_pending ())
      gtk_main_iteration ();
  }
  
  gw->audio_player_devices = 
    PSoundChannel::GetDeviceNames (PSoundChannel::Player);
  gw->audio_recorder_devices = 
    PSoundChannel::GetDeviceNames (PSoundChannel::Recorder);
  gw->video_devices = PVideoInputDevice::GetInputDeviceNames ();

  /* Main interface creation */
  cout << "FIX ME: Show Splash" << endl << flush;
  if (1) {
    e_splash_set_icon_highlight (E_SPLASH (gw->splash_win), icon_index++, true);
    while (gtk_events_pending ())
      gtk_main_iteration ();
  }
  
  /* Build the interface */
  gnomemeeting_init_main_window ();
  gnomemeeting_init_ldap_window ();
  gnomemeeting_init_pref_window ();
  gnomemeeting_init_menu ();
  gnomemeeting_init_toolbar ();	
  
  /* Launch the GnomeMeeting H.323 part */
  static GnomeMeeting instance;
  endpoint = MyApp->Endpoint ();
  
  if (debug)
    PTrace::Initialise (4);
  
  cout << "FIX ME: Show Splash" << endl << flush;
  if (1) {
    e_splash_set_icon_highlight (E_SPLASH (gw->splash_win), icon_index++, true);
    while (gtk_events_pending ())
      gtk_main_iteration ();
  }
  endpoint->AddAudioCapabilities ();
  
  if (1) {
    e_splash_set_icon_highlight (E_SPLASH (gw->splash_win), icon_index++, true);
    while (gtk_events_pending ())
      gtk_main_iteration ();
  }
  endpoint->AddVideoCapabilities (gconf_client_get_int (GCONF_CLIENT (client), "/apps/gnomemeeting/video_settings/video_size", NULL));
  
  /* The LDAP part, if needed */
  if (gconf_client_get_bool (GCONF_CLIENT (client), "/apps/gnomemeeting/ldap/register", NULL)) {
    if (1) {
      e_splash_set_icon_highlight (E_SPLASH (gw->splash_win), icon_index++, true);
      while (gtk_events_pending ())
	gtk_main_iteration ();
    }

    GMILSClient *gm_ils_client = (GMILSClient *) endpoint->GetILSClient ();
    gm_ils_client->Register ();
  }
  
  /* Run the listener thread */
  cout << "FIX ME: Show Splash" << endl << flush;
  if (1) {
      e_splash_set_icon_highlight (E_SPLASH (gw->splash_win), icon_index++, true);
      while (gtk_events_pending ())
	gtk_main_iteration ();
  }
  
  if (!endpoint->StartListener ()) {
    GtkWidget *msg_box = gnome_message_box_new (_("Could not start the listener thread. You will not be able to receive incoming calls."), GNOME_MESSAGE_BOX_ERROR, "OK", NULL);

    gtk_widget_show (msg_box);
  }

  /* Register to the Gatekeeper */
  cout << "FIX ME: Show Splash" << endl << flush;
  cout << "FIX ME: GateKeeper" << endl << flush;
  if (0) {
    if (1) {
	e_splash_set_icon_highlight (E_SPLASH (gw->splash_win), icon_index++, true);
	while (gtk_events_pending ())
	  gtk_main_iteration ();
    }
    
    endpoint->GatekeeperRegister ();
  }

  
  /* Set recording source and set micro to record */  
  MyApp->Endpoint()->
    SetSoundChannelPlayDevice (gw->audio_player_devices [0]);
  /* Translators: This is shown in the history. */
  text = g_strdup_printf (_("Set Audio player device to %s"), 
			  (const char *) gw->audio_player_devices [0]);
  gnomemeeting_log_insert (text);
  g_free (text);

  MyApp->Endpoint()->
    SetSoundChannelRecordDevice (gw->audio_recorder_devices [0]);
  /* Translators: This is shown in the history. */
  text = g_strdup_printf (_("Set Audio recorder device to %s"), 
			  (const char *) gw->audio_recorder_devices [0]);
  gnomemeeting_log_insert (text);
  g_free (text);

  /* Set the right recording source : mic */
/*  char *recorder = (const char *) gw->audio_recorder_devices [0];
  gnomemeeting_set_recording_source (recorder, 0); 
*/
  cout << "FIX ME: The recording source" << endl << flush;

  /* Set the local User name */
  gchar *firstname =
    gconf_client_get_string (client, 
			     "/apps/gnomemeeting/personal_data/firstname", 0);
  gchar *lastname =
    gconf_client_get_string (client, 
			     "/apps/gnomemeeting/personal_data/lastname", 0);

  if ((firstname) && (lastname)) {
    
    gchar *local_name = g_strdup ("");
    local_name = g_strconcat (local_name, firstname, " ", lastname, NULL);
    
    endpoint->SetLocalUserName (local_name);
    g_free (local_name);
    g_free (firstname);
    g_free (lastname);
  }
    
  /* Show the main window */
  gtk_widget_show (GTK_WIDGET (gm));
  
  /* The logo */
  gw->pixmap = gdk_pixmap_new (gw->drawing_area->window, 
			       GM_CIF_WIDTH * 2, GM_CIF_HEIGHT * 2, -1);
  gnomemeeting_init_main_window_logo ();

  /* Create a popup menu to attach it to the drawing area  */
  gnomemeeting_popup_menu_init (gw->drawing_area, gw);

  /* Start the video preview */
  if (gconf_client_get_bool (client, "/apps/gnomemeeting/devices/video_preview", NULL)) {
    GMVideoGrabber *vg = NULL;
    vg = MyApp->Endpoint ()->GetVideoGrabber ();
    
    if (vg)
      vg->Open (TRUE);
  }

  /* Set icon */
  gtk_widget_push_visual(gdk_rgb_get_visual());
  gtk_widget_push_colormap(gdk_rgb_get_cmap());
  gnome_window_icon_set_from_file 
    (GTK_WINDOW (gm), GNOMEMEETING_IMAGES "gnomemeeting-logo-icon.png");

  if (gw->splash_win)
    gtk_widget_destroy (gw->splash_win);

  gnomemeeting_init_gconf (client);

  /* if the user tries to close the window : delete_event */
  gtk_signal_connect (GTK_OBJECT (gm), "delete_event",
		      GTK_SIGNAL_FUNC (gm_quit_callback), (gpointer) gw);

}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the main window.
 * PRE          :  Valid options.
 */
void gnomemeeting_init_main_window ()
{ 
  GConfClient *client = gconf_client_get_default ();
  GtkWidget *table, *table_in;	
  GtkWidget *frame;
  GtkWidget *pixmap;
  GtkWidget *handle_box;

  GtkTooltips *tip;

  
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  /* Create a table in the main window to attach things like buttons */
  table = gtk_table_new (58, 35, FALSE);
  
  gnome_app_set_contents (GNOME_APP (gm), GTK_WIDGET (table));


  /* The remote IP field */
  frame = gtk_frame_new(_("Remote host to contact"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);

  gtk_widget_set_usize (GTK_WIDGET (frame), 246, 52);
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (frame), 0, 35, 1, 11,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    10, 10);
	
  table_in = gtk_table_new (4, 10, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table_in);

  gtk_table_attach (GTK_TABLE (table_in), 
		    GTK_WIDGET (gtk_label_new (_("Host:"))), 0, 2, 0, 4,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    0, 0);

  cout << "FIX ME: History Combo Box" << endl << flush;
  gw->combo = gnomemeeting_history_combo_box_new(gw);

  gtk_combo_set_use_arrows_always (GTK_COMBO(gw->combo), TRUE);

  gtk_combo_disable_activate (GTK_COMBO (gw->combo));
  gtk_signal_connect (GTK_OBJECT (GTK_COMBO (gw->combo)->entry), "activate",
		      GTK_SIGNAL_FUNC (connect_cb), NULL);

  gtk_table_attach (GTK_TABLE (table_in), GTK_WIDGET (gw->combo), 2, 8, 0, 4,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    10, 10);

  gtk_widget_show_all (GTK_WIDGET (frame));


  /* The Notebook */
  gw->main_notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gw->main_notebook), GTK_POS_TOP);
  gtk_notebook_popup_enable (GTK_NOTEBOOK (gw->main_notebook));
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (gw->main_notebook), FALSE);

  gnomemeeting_init_main_window_remote_user_info ();
  gnomemeeting_init_main_window_log ();
  gnomemeeting_init_main_window_audio_settings ();
  gnomemeeting_init_main_window_video_settings ();

  gtk_widget_set_usize (GTK_WIDGET (gw->main_notebook), 246, 134);
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (gw->main_notebook),
		    5, 30, 38, 58,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    10, 10); 

  int selected_page = 
    gconf_client_get_int (client, "/apps/gnomemeeting/view/notebook_info", 0);

  if (gconf_client_get_bool 
      (client, "/apps/gnomemeeting/view/show_control_panel", 0))
    gtk_widget_show_all (GTK_WIDGET (gw->main_notebook));

  gtk_notebook_set_page (GTK_NOTEBOOK (gw->main_notebook), 
			 selected_page);

  gtk_signal_connect_after (GTK_OBJECT (gw->main_notebook), "switch-page",
			    GTK_SIGNAL_FUNC (notebook_page_changed_callback), 
			    0);

  /* The drawing area that will display the webcam images */
  gw->video_frame = gtk_handle_box_new();

  gw->drawing_area = gtk_drawing_area_new ();

  gtk_container_add (GTK_CONTAINER (gw->video_frame), gw->drawing_area);
  gtk_widget_set_usize (GTK_WIDGET (gw->video_frame), 
			GM_QCIF_WIDTH + GM_FRAME_SIZE, GM_QCIF_HEIGHT);

  gtk_drawing_area_size (GTK_DRAWING_AREA (gw->drawing_area), 
			 GM_QCIF_WIDTH, GM_QCIF_HEIGHT);

  gtk_signal_connect (GTK_OBJECT (gw->drawing_area), "expose_event",
		      (GtkSignalFunc) expose_event, gw);    	

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (gw->video_frame), 
		    5, 30, 11, 35,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    10, 10);

  gtk_widget_show (GTK_WIDGET (gw->video_frame));
  gtk_widget_show (GTK_WIDGET (gw->drawing_area));


  /* The control buttons */
  gw->quickbar_frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (gw->quickbar_frame), GTK_SHADOW_OUT);

  gtk_widget_set_usize (GTK_WIDGET (gw->quickbar_frame), GM_QCIF_WIDTH, 32);
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (gw->quickbar_frame), 
		    5, 30, 35, 37,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    10, 0);
	

  table_in = gtk_table_new (6, 1, FALSE);
  gtk_container_add (GTK_CONTAINER (gw->quickbar_frame), table_in);

  /* Video Preview Button */
  gw->preview_button = gtk_toggle_button_new ();

  pixmap = gnome_pixmap_new_from_xpm_d ((char **) eye_xpm);
  gtk_container_add (GTK_CONTAINER (gw->preview_button), pixmap);

  gtk_widget_set_usize (GTK_WIDGET (gw->preview_button), 24, 24);
  gtk_table_attach (GTK_TABLE (table_in), 
		    gw->preview_button, 0, 1, 0, 1,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    2, 2);

  GTK_TOGGLE_BUTTON (gw->preview_button)->active = 
    gconf_client_get_bool (client, "/apps/gnomemeeting/devices/video_preview", NULL);

  gtk_signal_connect (GTK_OBJECT (gw->preview_button), "clicked",
                      GTK_SIGNAL_FUNC (toggle_changed), 
		      (gpointer) "/apps/gnomemeeting/devices/video_preview");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, gw->preview_button,
			_("Click here to begin to display images from your camera device."), NULL);

  /* Audio Channel Button */
  gw->audio_chan_button = gtk_toggle_button_new ();

  pixmap = gnome_pixmap_new_from_xpm_d ((char **) speaker_xpm);
  gtk_container_add (GTK_CONTAINER (gw->audio_chan_button), pixmap);

  gtk_widget_set_usize (GTK_WIDGET (gw->audio_chan_button), 24, 24);
  gtk_table_attach (GTK_TABLE (table_in), 
		    gw->audio_chan_button, 1, 2, 0, 1,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    2, 2);

  gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_chan_button), FALSE);

  gtk_signal_connect (GTK_OBJECT (gw->audio_chan_button), "clicked",
                      GTK_SIGNAL_FUNC (pause_audio_callback), gw);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, gw->audio_chan_button,
			_("Audio Transmission Status. During a call, click here to pause the audio transmission."), NULL);

  /* Video Channel Button */
  gw->video_chan_button = gtk_toggle_button_new ();

  pixmap = gnome_pixmap_new_from_xpm_d ((char **) quickcam_xpm);
  gtk_container_add (GTK_CONTAINER (gw->video_chan_button), pixmap);

  gtk_widget_set_usize (GTK_WIDGET (gw->video_chan_button), 24, 24);
  gtk_table_attach (GTK_TABLE (table_in), 
		    gw->video_chan_button, 2, 3, 0, 1,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    2, 2);

  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_chan_button), FALSE);

  gtk_signal_connect (GTK_OBJECT (gw->video_chan_button), "clicked",
                      GTK_SIGNAL_FUNC (pause_video_callback), gw);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, gw->video_chan_button,
			_("Video Transmission Status. During a call, click here to pause the video transmission."), NULL);

  /* Silence Detection Button */
  gw->silence_detection_button = gtk_toggle_button_new ();

  pixmap = gnome_pixmap_new_from_xpm_d ((char **) sample_xpm);
  gtk_container_add (GTK_CONTAINER (gw->silence_detection_button), pixmap);

  gtk_widget_set_usize (GTK_WIDGET (gw->silence_detection_button), 24, 24);
  gtk_table_attach (GTK_TABLE (table_in), 
		    gw->silence_detection_button, 3, 4, 0, 1,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    2, 2);

  gtk_widget_set_sensitive (GTK_WIDGET (gw->silence_detection_button), FALSE);

  gtk_signal_connect (GTK_OBJECT (gw->silence_detection_button), "clicked",
                      GTK_SIGNAL_FUNC (silence_detection_button_clicked), gw);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, gw->silence_detection_button,
			_("This button permits enabling/disabling of silence detection during calls."), NULL);

  /* Left arrow */
  gw->left_arrow = gtk_button_new ();

  pixmap = gnome_pixmap_new_from_xpm_d ((char **) left_arrow_xpm);
  gtk_container_add (GTK_CONTAINER (gw->left_arrow), pixmap);

  gtk_widget_set_usize (GTK_WIDGET (gw->left_arrow), 24, 24);

  gtk_table_attach (GTK_TABLE (table_in), 
		    gw->left_arrow, 4, 5, 0, 1,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    2, 2);

  gtk_signal_connect (GTK_OBJECT (gw->left_arrow), "clicked",
		      GTK_SIGNAL_FUNC (left_arrow_clicked), client);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, gw->left_arrow,
			_("Click here to display the previous section of the Control Panel."), NULL);

  /* Right arrow */
  gw->right_arrow = gtk_button_new ();

  pixmap = gnome_pixmap_new_from_xpm_d ((char **) right_arrow_xpm);
  gtk_container_add (GTK_CONTAINER (gw->right_arrow), pixmap);

  gtk_widget_set_usize (GTK_WIDGET (gw->right_arrow), 24, 24);

  gtk_table_attach (GTK_TABLE (table_in), 
		    gw->right_arrow, 5, 6, 0, 1,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    2, 2);

  gtk_signal_connect (GTK_OBJECT (gw->right_arrow), "clicked",
		      GTK_SIGNAL_FUNC (right_arrow_clicked), client);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, gw->right_arrow,
			_("Click here to display the next section of the Control Panel."), NULL);

  if (selected_page == 0)
    gtk_widget_set_sensitive (gw->left_arrow, false);
  else if (selected_page == 3)
    gtk_widget_set_sensitive (gw->right_arrow, false);

  /* The statusbar */
  gw->statusbar = gnome_appbar_new (FALSE, TRUE, GNOME_PREFERENCES_NEVER);	
  gnome_app_set_statusbar (GNOME_APP (gm), gw->statusbar);

  if (gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_status_bar", 0))
    gtk_widget_show (GTK_WIDGET (gw->statusbar));
  else
    gtk_widget_hide (GTK_WIDGET (gw->statusbar));

  if (gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_quick_bar", 0))
    gtk_widget_show_all (GTK_WIDGET (gw->quickbar_frame));  
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the remote user info part of the main window.
 * PRE          :  /
 */
void gnomemeeting_init_main_window_remote_user_info ()
{
  GtkWidget *frame;
  GtkWidget *label;
  GtkWidget *scr;
  gchar * clist_titles [] = {" ", N_("Info")};

  clist_titles [1] = gettext (clist_titles [1]);

  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  frame = gtk_frame_new (_("Remote User"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);

  scr = gtk_scrolled_window_new (NULL, NULL);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);

  gtk_container_add (GTK_CONTAINER (frame), scr);

  gw->user_list = gtk_clist_new_with_titles (2, clist_titles);

  gtk_container_add (GTK_CONTAINER (scr), gw->user_list);
  gtk_container_set_border_width (GTK_CONTAINER (gw->user_list), 
				  GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
  
  gtk_clist_set_row_height (GTK_CLIST (gw->user_list), 17);
  gtk_clist_set_column_width (GTK_CLIST (gw->user_list), 0, 20);
  gtk_clist_set_column_width (GTK_CLIST (gw->user_list), 1, 180);
  gtk_clist_set_column_auto_resize (GTK_CLIST (gw->user_list), 1, TRUE);

  gtk_clist_set_shadow_type (GTK_CLIST (gw->user_list), GTK_SHADOW_IN);

  label = gtk_label_new (_("Remote User"));

  gtk_notebook_append_page (GTK_NOTEBOOK (gw->main_notebook), frame, 
			    label);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the history log part of the main window.
 * PRE          :  /
 */
void gnomemeeting_init_main_window_log ()
{
  GtkWidget *frame;
  GtkWidget *label;
  GtkWidget *scr;

  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  frame = gtk_frame_new (_("History Log"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);

  gw->log_text = gtk_text_new (NULL, NULL);
  gtk_text_set_line_wrap (GTK_TEXT (gw->log_text), TRUE);
  gtk_text_set_word_wrap (GTK_TEXT (gw->log_text), TRUE);

  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (frame), scr);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (scr), GNOME_PAD_SMALL);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_ALWAYS);

  gtk_text_set_editable (GTK_TEXT (gw->log_text), FALSE);

  gtk_container_add (GTK_CONTAINER (scr), gw->log_text);

  label = gtk_label_new (_("History"));

  gtk_notebook_append_page (GTK_NOTEBOOK (gw->main_notebook), frame, label);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the video settings part of the main window. This
 *                 part is made unsensitive while the grabber is not enabled.
 * PRE          :  /
 */
void gnomemeeting_init_main_window_video_settings ()
{
  GtkWidget *label;
  
  GtkWidget *table;

  GtkWidget *pixmap;
  GtkWidget *hscale_brightness, *hscale_colour, 
    *hscale_contrast, *hscale_whiteness;

  int brightness = 0, colour = 0, contrast = 0, whiteness = 0;
  
  GtkTooltips *tip;

  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  /* Webcam Control Frame */		
  gw->video_settings_frame = gtk_frame_new (_("Video Settings"));
  gtk_frame_set_shadow_type (GTK_FRAME (gw->video_settings_frame), 
			     GTK_SHADOW_OUT);

  /* Put a table in the first frame */
  table = gtk_table_new (4, 4, FALSE);
  gtk_container_add (GTK_CONTAINER (gw->video_settings_frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (gw->video_settings_frame), 
				  GNOME_PAD_SMALL);


  /* Brightness */
  pixmap =  gnome_pixmap_new_from_xpm_d ((char **) brightness_xpm);
  gtk_table_attach (GTK_TABLE (table), pixmap, 0, 1, 0, 1,
		    (GtkAttachOptions) (NULL),
		    (GtkAttachOptions) (NULL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gw->adj_brightness = gtk_adjustment_new (brightness, 0.0, 
					   255.0, 1.0, 5.0, 1.0);
  hscale_brightness = gtk_hscale_new (GTK_ADJUSTMENT (gw->adj_brightness));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_brightness), TRUE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_brightness), GTK_POS_RIGHT);
  gtk_table_attach (GTK_TABLE (table), hscale_brightness, 1, 4, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, 0);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, hscale_brightness,
			_("Adjust brightness"), NULL);

  gtk_signal_connect (GTK_OBJECT (gw->adj_brightness), "value-changed",
		      GTK_SIGNAL_FUNC (brightness_changed), (gpointer) gw);


  /* Whiteness */
  pixmap =  gnome_pixmap_new_from_xpm_d ((char **) whiteness_xpm);
  gtk_table_attach (GTK_TABLE (table), pixmap, 0, 1, 1, 2,
 (GtkAttachOptions) (NULL),
		    (GtkAttachOptions) (NULL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gw->adj_whiteness = gtk_adjustment_new (whiteness, 0.0, 
					  255.0, 1.0, 5.0, 1.0);
  hscale_whiteness = gtk_hscale_new (GTK_ADJUSTMENT (gw->adj_whiteness));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_whiteness), TRUE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_whiteness), GTK_POS_RIGHT);
  gtk_table_attach (GTK_TABLE (table), hscale_whiteness, 1, 4, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, 0);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, hscale_whiteness,
			_("Adjust whiteness"), NULL);

  gtk_signal_connect (GTK_OBJECT (gw->adj_whiteness), "value-changed",
		      GTK_SIGNAL_FUNC (whiteness_changed), (gpointer) gw);


  /* Colour */
  pixmap =  gnome_pixmap_new_from_xpm_d ((char **) color_xpm);
  gtk_table_attach (GTK_TABLE (table), pixmap, 0, 1, 2, 3,
		    (GtkAttachOptions) (NULL),
		    (GtkAttachOptions) (NULL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gw->adj_colour = gtk_adjustment_new (colour, 0.0, 
				       255.0, 1.0, 5.0, 1.0);
  hscale_colour = gtk_hscale_new (GTK_ADJUSTMENT (gw->adj_colour));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_colour), TRUE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_colour), GTK_POS_RIGHT);
  gtk_table_attach (GTK_TABLE (table), hscale_colour, 1, 4, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, 0);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, hscale_colour,
			_("Adjust color"), NULL);

  gtk_signal_connect (GTK_OBJECT (gw->adj_colour), "value-changed",
		      GTK_SIGNAL_FUNC (colour_changed), (gpointer) gw);


  /* Contrast */
  pixmap =  gnome_pixmap_new_from_xpm_d ((char **) contrast_xpm);
  gtk_table_attach (GTK_TABLE (table), pixmap, 0, 1, 3, 4,
		    (GtkAttachOptions) (NULL),
		    (GtkAttachOptions) (NULL),
		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);

  gw->adj_contrast = gtk_adjustment_new (contrast, 0.0, 
					 255.0, 1.0, 5.0, 1.0);
  hscale_contrast = gtk_hscale_new (GTK_ADJUSTMENT (gw->adj_contrast));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_contrast), TRUE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_contrast), GTK_POS_RIGHT);
  gtk_table_attach (GTK_TABLE (table), hscale_contrast, 1, 4, 3, 4,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOME_PAD_SMALL, 0);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, hscale_contrast,
			_("Adjust contrast"), NULL);

  gtk_signal_connect (GTK_OBJECT (gw->adj_contrast), "value-changed",
		      GTK_SIGNAL_FUNC (contrast_changed), (gpointer) gw);

  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame), FALSE);

  label = gtk_label_new (_("Video Settings"));  

  gtk_notebook_append_page (GTK_NOTEBOOK(gw->main_notebook), 
			    gw->video_settings_frame, 
			    label);

}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the audio setting part of the main window.
 * PRE          :  /
 */
void gnomemeeting_init_main_window_audio_settings ()
{
  GtkWidget *label;
  GtkWidget *hscale_play, *hscale_rec;
  GtkWidget *audio_table;
  GtkWidget *pixmap;

  int vol = 0;

  GtkWidget *frame;
  GConfClient *client = gconf_client_get_default ();

  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  frame = gtk_frame_new (_("Audio Settings"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);

  audio_table = gtk_table_new (4, 4, TRUE);
  gtk_container_add (GTK_CONTAINER (frame), audio_table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

  pixmap = gnome_pixmap_new_from_xpm_d ((char **) speaker_xpm);

  gtk_table_attach (GTK_TABLE (audio_table), pixmap, 0, 1, 0, 1,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    GNOME_PAD_SMALL, 0);

  gchar *player_mixer = gconf_client_get_string (client, "/apps/gnomemeeting/devices/audio_player_mixer", NULL);
  gnomemeeting_volume_get (player_mixer, 0, &vol);
  g_free (player_mixer);
  gw->adj_play = gtk_adjustment_new (vol / 257, 0.0, 100.0, 1.0, 5.0, 1.0);
  hscale_play = gtk_hscale_new (GTK_ADJUSTMENT (gw->adj_play));
  gtk_scale_set_value_pos (GTK_SCALE (hscale_play),GTK_POS_RIGHT); 
  gtk_scale_set_draw_value (GTK_SCALE (hscale_play), TRUE);

  gtk_table_attach (GTK_TABLE (audio_table), hscale_play, 1, 4, 0, 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    0, 0);

  pixmap = gnome_pixmap_new_from_xpm_d ((char **) mic_xpm);

  gtk_table_attach (GTK_TABLE (audio_table), pixmap, 0, 1, 1, 2,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    GNOME_PAD_SMALL, 0);

  gchar *recorder_mixer = gconf_client_get_string (client, "/apps/gnomemeeting/devices/audio_recorder_mixer", NULL);
  gnomemeeting_volume_get (recorder_mixer, 1, &vol);
  g_free (recorder_mixer);
  gw->adj_rec = gtk_adjustment_new (vol / 257, 0.0, 100.0, 1.0, 5.0, 1.0);
  hscale_rec = gtk_hscale_new (GTK_ADJUSTMENT (gw->adj_rec));
  gtk_scale_set_value_pos (GTK_SCALE (hscale_rec),GTK_POS_RIGHT); 
  gtk_scale_set_draw_value (GTK_SCALE (hscale_rec), TRUE);

  gtk_table_attach (GTK_TABLE (audio_table), hscale_rec, 1, 4, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    0, 0);

  gtk_signal_connect (GTK_OBJECT (gw->adj_play), "value-changed",
		      GTK_SIGNAL_FUNC (audio_volume_changed), (gpointer) gw);

  gtk_signal_connect (GTK_OBJECT (gw->adj_rec), "value-changed",
		      GTK_SIGNAL_FUNC (audio_volume_changed), (gpointer) gw);

  
  label = gtk_label_new (_("Audio Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK (gw->main_notebook), frame, label);
}




