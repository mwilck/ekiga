
/*
 * GnomeMeeting -- A Video-Conferencing application
 *
 * Copyright (C) 2000-2002 Damien Sandras
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

#include <libgnomeui/gnome-window-icon.h>

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
#include "druid.h"
#include "chat_window.h"

#include <gconf/gconf-client.h>

#include "../pixmaps/inlines.h"

#include "../pixmaps/brightness.xpm"
#include "../pixmaps/whiteness.xpm"
#include "../pixmaps/contrast.xpm"
#include "../pixmaps/color.xpm"


/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

static void main_notebook_page_changed (GtkNotebook *, GtkNotebookPage *,
					gint, gpointer);
static void audio_volume_changed (GtkAdjustment *, gpointer);
static void brightness_changed (GtkAdjustment *, gpointer);
static void whiteness_changed (GtkAdjustment *, gpointer);
static void colour_changed (GtkAdjustment *, gpointer);
static void contrast_changed (GtkAdjustment *, gpointer);

static void gnomemeeting_init_main_window ();
static gint gm_quit_callback (GtkWidget *, GdkEvent *, gpointer);
static void gnomemeeting_init_main_window_video_settings ();
static void gnomemeeting_init_main_window_audio_settings ();
static void gnomemeeting_init_main_window_log  ();


/* GTK Callbacks */

/**
 * DESCRIPTION  :  This callback is called when the user changes the
 *                 page in the main notebook.
 * BEHAVIOR     :  Update the menu accordingly.
 * PRE          :  /
 **/
static void 
main_notebook_page_changed (GtkNotebook *notebook, GtkNotebookPage *page,
			    gint page_num, gpointer user_data) {

  GnomeUIInfo *notebook_view_uiinfo =
    (GnomeUIInfo *) g_object_get_data (G_OBJECT (gm),
				       "notebook_view_uiinfo");
  GmWindow *gw = gnomemeeting_get_main_window (gm);

  for (int i = 0; i < 3; i++) 
    GTK_CHECK_MENU_ITEM (notebook_view_uiinfo[i].widget)->active =
      (gtk_notebook_get_current_page (GTK_NOTEBOOK (gw->main_notebook)) == i);
  
}


/**
 * DESCRIPTION  :  This callback is called when the user changes the
 *                 audio settings sliders in the main notebook.
 * BEHAVIOR     :  Update the volume.
 * PRE          :  gpointer is a valid pointer to GM_pref_window_widgets
 **/
void 
audio_volume_changed (GtkAdjustment *adjustment, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();
  int vol_play, vol_rec;
  char *audio_recorder_mixer;
  char *audio_player_mixer;

  GmWindow *gw = GM_WINDOW (data);
  
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


/**
 * DESCRIPTION  :  This callback is called when the user changes the 
 *                 video brightness slider in the main notebook.
 * BEHAVIOR     :  Update the value in real time.
 * PRE          :  gpointer is a valid pointer to GM_window_widgets
 */
void 
brightness_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GmWindow *gw = GM_WINDOW (data);
  GMVideoGrabber *video_grabber = MyApp->Endpoint ()->GetVideoGrabber ();

  int brightness;

  brightness =  (int) (GTK_ADJUSTMENT (gw->adj_brightness)->value);

  video_grabber->SetBrightness (brightness * 256);
}


/**
 * DESCRIPTION  :  This callback is called when the user changes the 
 *                 video whiteness slider in the main notebook.
 * BEHAVIOR     :  Update the value in real time.
 * PRE          :  gpointer is a valid pointer to GM_window_widgets
 */
void 
whiteness_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GmWindow *gw = GM_WINDOW (data);
  GMVideoGrabber *video_grabber = MyApp->Endpoint ()->GetVideoGrabber ();

  int whiteness;

  whiteness =  (int) (GTK_ADJUSTMENT (gw->adj_whiteness)->value);

  video_grabber->SetWhiteness (whiteness * 256);
}


/**
 * DESCRIPTION  :  This callback is called when the user changes the 
 *                 video colour slider in the main notebook.
 * BEHAVIOR     :  Update the value in real time.
 * PRE          :  gpointer is a valid pointer to GM_window_widgets
 */
void 
colour_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GmWindow *gw = GM_WINDOW (data);
  GMVideoGrabber *video_grabber = MyApp->Endpoint ()->GetVideoGrabber ();

  int colour;

  colour =  (int) (GTK_ADJUSTMENT (gw->adj_colour)->value);

  video_grabber->SetColour (colour * 256);
}


/**
 * DESCRIPTION  :  This callback is called when the user changes the 
 *                 video contrast slider in the main notebook.
 * BEHAVIOR     :  Update the value in real time.
 * PRE          :  gpointer is a valid pointer to GM_window_widgets
 **/
void 
contrast_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GmWindow *gw = GM_WINDOW (data);
  GMVideoGrabber *video_grabber = MyApp->Endpoint ()->GetVideoGrabber ();

  int contrast;

  contrast =  (int) (GTK_ADJUSTMENT (gw->adj_contrast)->value);

  video_grabber->SetContrast (contrast * 256);
}


/**
 * DESCRIPTION  :  This callback is called when the user tries to close
 *                 the application using the window manager.
 * BEHAVIOR     :  Calls the real callback.
 * PRE          :  /
 **/
static gint 
gm_quit_callback (GtkWidget *widget, GdkEvent *event, 
			      gpointer data)
{
  quit_callback (NULL, data);

  return (TRUE);
}  


/* The functions */

void 
gnomemeeting_init (GmWindow *gw, 
                   GM_pref_window_widgets *pw,
                   GM_ldap_window_widgets *lw, 
                   GM_rtp_data *rtp,
                   int argc, char ** argv, char ** envp)
{
  GMH323EndPoint *endpoint = NULL;
  gchar *firstname = NULL;
  gchar *lastname = NULL;
  gchar *alias = NULL;
  bool show_splash;
  GConfClient *client;
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
  GnomeProgram *p = gnome_program_init ("GnomeMeeting", VERSION,
					LIBGNOMEUI_MODULE, argc, argv,
					GNOME_PARAM_POPT_TABLE, arguments,
					GNOME_PARAM_HUMAN_READABLE_NAME,
					_("GnomeMeeting"),
					GNOME_PARAM_APP_DATADIR, DATADIR,
					NULL);

  gm = gnome_app_new ("gnomemeeting", _("GnomeMeeting"));

  /* Some little gconf stuff */  
  client = gconf_client_get_default ();
  gconf_client_add_dir (client, "/apps/gnomemeeting",
			GCONF_CLIENT_PRELOAD_RECURSIVE,
			0);
  gchar *gconf_test = NULL;
  gconf_test = gconf_client_get_string (client, "/apps/gnomemeeting/general/gconf_test", NULL);

   if (gconf_test == NULL || strcmp (gconf_test, BUILD_ID)) {

     int reply = 0;
     GtkWidget *dialog = 
       gtk_message_dialog_new (GTK_WINDOW (gm),
			       GTK_DIALOG_DESTROY_WITH_PARENT,
			       GTK_MESSAGE_ERROR,
			       GTK_BUTTONS_CLOSE,
			       _("Please check your gconf settings and permissions, it seems that gconf is not properly setup on your system"));

     gtk_dialog_run (GTK_DIALOG (dialog));
     gtk_widget_destroy (dialog);

     delete (gw);
     delete (lw);
     delete (pw);
     delete (rtp);
     exit (-1);
   }
   g_free (gconf_test);

  /* We store all the pointers to the structure as data of gm */
  g_object_set_data (G_OBJECT (gm), "gw", gw);
  g_object_set_data (G_OBJECT (gm), "lw", lw);
  g_object_set_data (G_OBJECT (gm), "pw", pw);

  /* Startup Process */
  gw->docklet = gnomemeeting_init_docklet ();

  /* Init the splash screen */
  gw->splash_win = e_splash_new ();
  g_signal_connect (G_OBJECT (gw->splash_win), "delete_event",
		    G_CALLBACK (gtk_widget_hide_on_delete), 0);

  show_splash = gconf_client_get_bool (client, "/apps/gnomemeeting/"
				      "view/show_splash", 0);  

  if (show_splash) {

    /* We show the splash screen */
    gtk_widget_show (gw->splash_win);

    while (gtk_events_pending ())
      gtk_main_iteration ();
  }
  

  /* Search for devices */
  gw->audio_player_devices = 
    PSoundChannel::GetDeviceNames (PSoundChannel::Player);
  gw->audio_recorder_devices = 
    PSoundChannel::GetDeviceNames (PSoundChannel::Recorder);
  gw->video_devices = PVideoInputDevice::GetInputDeviceNames ();


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
    PTrace::Initialise (6);

 
  /* Start the video preview */
  if (gconf_client_get_bool (client, "/apps/gnomemeeting/devices/video_preview", NULL)) {
    GMVideoGrabber *vg = NULL;
    vg = MyApp->Endpoint ()->GetVideoGrabber ();
    
    if (vg)
      vg->Open (TRUE);
  }


  /* Set the local User name */
  firstname =
    gconf_client_get_string (client, 
			     "/apps/gnomemeeting/personal_data/firstname", 
			     0);
  lastname =
    gconf_client_get_string (client, 
			     "/apps/gnomemeeting/personal_data/lastname", 0);
  
  if ((firstname) && (lastname)) {
    
    gchar *local_name = g_strdup ("");
    local_name = g_strconcat (local_name, firstname, " ", lastname, NULL);
    
    /* It is the first alias for the gatekeeper */
    endpoint->SetLocalUserName (local_name);
  }

  alias =
    gconf_client_get_string (client, 
			     "/apps/gnomemeeting/gatekeeper/gk_alias", 
			     0);
  
  if (alias != NULL) {
    
    PString Alias = PString (alias);
    
    if (!Alias.IsEmpty ())
      endpoint->AddAliasName (Alias);
  }

  
  /* The LDAP part, if needed */
  if (gconf_client_get_bool (GCONF_CLIENT (client), "/apps/gnomemeeting/ldap/register", NULL)) {

    GMILSClient *gm_ils_client = (GMILSClient *) endpoint->GetILSClient ();
    gm_ils_client->Register ();
  }
  
  
  if (!endpoint->StartListener ()) {

    GtkWidget *dialog = 
       gtk_message_dialog_new (GTK_WINDOW (gm),
			       GTK_DIALOG_MODAL,
			       GTK_MESSAGE_ERROR,
			       GTK_BUTTONS_OK,
			       _("Could not start the listener thread. You will not be able to receive incoming calls."));

     gtk_dialog_run (GTK_DIALOG (dialog));
     gtk_widget_destroy (dialog);
  }

  
  /* Start the Gconf notifiers */
  gnomemeeting_init_gconf (client);


  /* Register to the Gatekeeper */
  int method = gconf_client_get_int (GCONF_CLIENT (client), "/apps/gnomemeeting/gatekeeper/registering_method", 0);

  /* We do that through the notifier */
  if (method)
    gconf_client_set_int (GCONF_CLIENT (client),
			  "/apps/gnomemeeting/gatekeeper/registering_method",
			  method, 0);


  /* Init the druid */
  if (gconf_client_get_int (client, 
			    "/apps/gnomemeeting/general/version", NULL) < 92)

    gnomemeeting_init_druid ((gpointer) "first");

  else {

  /* Show the main window */
  if (!gconf_client_get_bool (GCONF_CLIENT (client), 
			     "/apps/gnomemeeting/view/show_docklet", 0) ||
      !gconf_client_get_bool (GCONF_CLIENT (client),
			     "/apps/gnomemeeting/view/start_docked", 0))

    gtk_widget_show (GTK_WIDGET (gm));
  }


  /* Create a popup menu to attach it to the drawing area  */
  gnomemeeting_popup_menu_init (gw->video_frame, gw);

  /* Set icon */
  GdkPixbuf *pixbuf_icon = 
    gdk_pixbuf_new_from_file (GNOMEMEETING_IMAGES "/gnomemeeting-logo-icon.png", NULL); 

  gtk_window_set_icon (GTK_WINDOW (gm), pixbuf_icon);
  g_object_unref (G_OBJECT (pixbuf_icon));
  gtk_window_set_resizable (GTK_WINDOW (gm), false);

  /* Hide the splash */
  if (gw->splash_win)
    gtk_widget_hide (gw->splash_win);


  /* if the user tries to close the window : delete_event */
  g_signal_connect (G_OBJECT (gm), "delete_event",
		    G_CALLBACK (gm_quit_callback), (gpointer) gw);

  gnomemeeting_init_main_window_logo ();


  /* The gtk_widget_show (gm) will show the toolbar, hide it if needed */
  if (!gconf_client_get_bool (client, "/apps/gnomemeeting/view/left_toolbar", 0)) 
    gtk_widget_hide (GTK_WIDGET (gnome_app_get_dock_item_by_name(GNOME_APP (gm), "left_toolbar")));
}


/**
 * DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the main window.
 * PRE          :  Valid options.
 **/
void gnomemeeting_init_main_window ()
{ 
  GConfClient *client = gconf_client_get_default ();
  GtkWidget *table;	
  GtkWidget *frame;
  
  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  /* Create a table in the main window to attach things like buttons */
  table = gtk_table_new (3, 4, FALSE);
  
  gnome_app_set_contents (GNOME_APP (gm), GTK_WIDGET (table));


  /* The Notebook */
  gw->main_notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gw->main_notebook), GTK_POS_BOTTOM);
  gtk_notebook_popup_enable (GTK_NOTEBOOK (gw->main_notebook));
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (gw->main_notebook), TRUE);

  gnomemeeting_init_main_window_log ();
  gnomemeeting_init_main_window_audio_settings ();
  gnomemeeting_init_main_window_video_settings ();

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (gw->main_notebook),
		    0, 2, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    0, 0); 

  if (gconf_client_get_bool 
      (client, "/apps/gnomemeeting/view/show_control_panel", 0))
    gtk_widget_show_all (GTK_WIDGET (gw->main_notebook));


  /* The drawing area that will display the webcam images */
  frame = gtk_frame_new (NULL);
  gw->video_frame = gtk_handle_box_new();

  gtk_container_add (GTK_CONTAINER (frame), gw->video_frame);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

  gtk_widget_set_size_request (GTK_WIDGET (gw->video_frame), 
			       GM_QCIF_WIDTH + GM_FRAME_SIZE, GM_QCIF_HEIGHT);

  gw->video_image = gtk_image_new ();
  gtk_container_add (GTK_CONTAINER (gw->video_frame), gw->video_image);

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (frame), 
		    0, 2, 0, 1,
		    GTK_EXPAND,
		    (GtkAttachOptions) NULL,
		    10, 10);

  gtk_widget_show_all (GTK_WIDGET (frame));


  /* The Chat Window */
  gnomemeeting_init_text_chat_window ();
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (gw->chat->window), 
 		    2, 4, 0, 3,
 		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
 		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
 		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
  if (gconf_client_get_bool 
      (client, "/apps/gnomemeeting/view/show_chat_window", 0))
          gtk_widget_show_all (GTK_WIDGET (gw->chat->window));


  /* The remote name */
  gw->remote_name = gtk_entry_new ();
  gtk_editable_set_editable (GTK_EDITABLE (gw->remote_name), FALSE);

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (gw->remote_name), 
		    0, 2, 1, 2,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    10, 10);

  gtk_widget_show (GTK_WIDGET (gw->remote_name));


  /* The statusbar */
  gw->statusbar = gnome_appbar_new (FALSE, TRUE, GNOME_PREFERENCES_NEVER);	
  gnome_app_set_statusbar (GNOME_APP (gm), gw->statusbar);

  if (gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_status_bar", 0))
    gtk_widget_show (GTK_WIDGET (gw->statusbar));
  else
    gtk_widget_hide (GTK_WIDGET (gw->statusbar));

  g_signal_connect_after (G_OBJECT (gw->main_notebook), "switch-page",
			  G_CALLBACK (main_notebook_page_changed), NULL);
}


/**
 * DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the history log part of the main window.
 * PRE          :  /
 **/
void gnomemeeting_init_main_window_log ()
{
  GtkWidget *view;
  GtkWidget *frame;
  GtkWidget *label;
  GtkWidget *scr;

  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  frame = gtk_frame_new (_("History Log"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);

  view = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);

  gw->log_text = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
  
  scr = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (frame), scr);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (scr), GNOME_PAD_SMALL);
  
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);

  gtk_container_add (GTK_CONTAINER (scr), view);    

  label = gtk_label_new (_("History"));

  gtk_notebook_append_page (GTK_NOTEBOOK (gw->main_notebook), frame, label);
}


/**
 * DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the video settings part of the main window. This
 *                 part is made unsensitive while the grabber is not enabled.
 * PRE          :  /
 **/
void gnomemeeting_init_main_window_video_settings ()
{
  GtkWidget *label;
  
  GtkWidget *table;

  GtkWidget *pixmap;
  GdkPixbuf *pixbuf;
  GtkWidget *hscale_brightness, *hscale_colour, 
    *hscale_contrast, *hscale_whiteness;

  int brightness = 0, colour = 0, contrast = 0, whiteness = 0;
  
  GtkTooltips *tip;

  GmWindow *gw = gnomemeeting_get_main_window (gm);

  /* Webcam Control Frame */		
  gw->video_settings_frame = gtk_frame_new (_("Video Settings"));
  gtk_frame_set_shadow_type (GTK_FRAME (gw->video_settings_frame), 
			     GTK_SHADOW_ETCHED_OUT);

  /* Put a table in the first frame */
  table = gtk_table_new (4, 4, FALSE);
  gtk_container_add (GTK_CONTAINER (gw->video_settings_frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (gw->video_settings_frame), 
				  GNOME_PAD_SMALL);


  /* Brightness */
  pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) brightness_xpm);
  pixmap =  gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref (G_OBJECT (pixbuf));
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

  g_signal_connect (G_OBJECT (gw->adj_brightness), "value-changed",
		    G_CALLBACK (brightness_changed), (gpointer) gw);


  /* Whiteness */
  pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) whiteness_xpm);
  pixmap =  gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref (G_OBJECT (pixbuf));
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

  g_signal_connect (G_OBJECT (gw->adj_whiteness), "value-changed",
		    G_CALLBACK (whiteness_changed), (gpointer) gw);


  /* Colour */
  pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) color_xpm);
  pixmap =  gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref (G_OBJECT (pixbuf));
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

  g_signal_connect (G_OBJECT (gw->adj_colour), "value-changed",
		    G_CALLBACK (colour_changed), (gpointer) gw);


  /* Contrast */
  pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) contrast_xpm);
  pixmap =  gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref (G_OBJECT (pixbuf));
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

  g_signal_connect (G_OBJECT (gw->adj_contrast), "value-changed",
		    G_CALLBACK (contrast_changed), (gpointer) gw);
  
  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame), FALSE);

  label = gtk_label_new (_("Video"));  

  gtk_notebook_append_page (GTK_NOTEBOOK(gw->main_notebook), 
			    gw->video_settings_frame, 
			    label);

}


/**
 * DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the audio setting part of the main window.
 * PRE          :  /
 **/
void gnomemeeting_init_main_window_audio_settings ()
{
  GtkWidget *label;
  GtkWidget *hscale_play, *hscale_rec;
  GtkWidget *audio_table;
  GdkPixbuf *mic, *volume;
  GtkWidget *pixmap;

  int vol = 0;

  GtkWidget *frame;
  GConfClient *client = gconf_client_get_default ();

  GmWindow *gw = gnomemeeting_get_main_window (gm);

  volume = gdk_pixbuf_new_from_inline (-1, inline_volume, FALSE, NULL);
  mic = gdk_pixbuf_new_from_inline (-1, inline_mic, FALSE, NULL);

  frame = gtk_frame_new (_("Audio Settings"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);

  audio_table = gtk_table_new (4, 4, TRUE);
  gtk_container_add (GTK_CONTAINER (frame), audio_table);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

  pixmap = gtk_image_new_from_pixbuf (volume);

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


  pixmap = gtk_image_new_from_pixbuf (mic);

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

  g_signal_connect (G_OBJECT (gw->adj_play), "value-changed",
		    G_CALLBACK (audio_volume_changed), (gpointer) gw);

  g_signal_connect (G_OBJECT (gw->adj_rec), "value-changed",
		    G_CALLBACK (audio_volume_changed), (gpointer) gw);

  
  label = gtk_label_new (_("Audio"));

  gtk_notebook_append_page (GTK_NOTEBOOK (gw->main_notebook), frame, label);
}

