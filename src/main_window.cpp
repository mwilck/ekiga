
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
#include "chat_window.h"

#include <gconf/gconf-client.h>

#include "../pixmaps/speaker.xpm"
#include "../pixmaps/mic.xpm"
#include "../pixmaps/brightness.xpm"
#include "../pixmaps/whiteness.xpm"
#include "../pixmaps/contrast.xpm"
#include "../pixmaps/color.xpm"


/* Declarations */

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

static gint expose_event (GtkWidget *, GdkEventExpose *, gpointer);
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
			GM_rtp_data *rtp,
			int argc, char ** argv, char ** envp)
{
  GMH323EndPoint *endpoint = NULL;
  gchar *text = NULL;
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
  gnome_init_with_popt_table (PACKAGE, VERSION, argc, argv,
				    arguments, 0, NULL);

  gm = gnome_app_new ("gnomemeeting", _("GnomeMeeting"));
  gtk_window_set_policy (GTK_WINDOW (gm), FALSE, FALSE, TRUE);

  /* Some little gconf stuff */  
  client = gconf_client_get_default ();
  gconf_client_add_dir (client, "/apps/gnomemeeting",
			GCONF_CLIENT_PRELOAD_RECURSIVE,
			0);
  int gconf_test = 0;
  gconf_test = gconf_client_get_int (client, "/apps/gnomemeeting/general/gconf_test", NULL);

  if (gconf_test != 1234) {

    GtkWidget *dialog = gnome_message_box_new (_("Please check your gconf settings and permissions, it seems that gconf is not properly setup on your system"),
					       GNOME_MESSAGE_BOX_ERROR,
					       GNOME_STOCK_BUTTON_CLOSE,
					       NULL);

    int reply = gnome_dialog_run(GNOME_DIALOG(dialog));
    if ((reply == 0)||(reply == -1)) {

       delete (gw);
       delete (lw);
       delete (pw);
       delete (rtp);
       exit (-1);
    }
  }

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
    PTrace::Initialise (3);
 
  
  /* The LDAP part, if needed */
  if (gconf_client_get_bool (GCONF_CLIENT (client), "/apps/gnomemeeting/ldap/register", NULL)) {

    GMILSClient *gm_ils_client = (GMILSClient *) endpoint->GetILSClient ();
    gm_ils_client->Register ();
  }
  
  
  if (!endpoint->StartListener ()) {
    GtkWidget *msg_box = gnome_message_box_new (_("Could not start the listener thread. You will not be able to receive incoming calls."), GNOME_MESSAGE_BOX_ERROR, "OK", NULL);

    gtk_widget_show (msg_box);
  }

  /* Register to the Gatekeeper */
  int method = gconf_client_get_int (GCONF_CLIENT (client), "/apps/gnomemeeting/gatekeeper/registering_method", 0);

  /* We do that through the notifier */
  if (method)
    gconf_client_set_int (GCONF_CLIENT (client),
			  "/apps/gnomemeeting/gatekeeper/registering_method",
			  method, 0);

    
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
    (GTK_WINDOW (gm), GNOMEMEETING_IMAGES "/gnomemeeting-logo-icon.png");

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

  gtk_widget_set_usize (GTK_WIDGET (gw->main_notebook), 215, 0);
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (gw->main_notebook),
		    0, 2, 2, 3,
		    (GtkAttachOptions) (NULL),
		    (GtkAttachOptions) (NULL),
		    0, 0); 

  int selected_page = 
    gconf_client_get_int (client, "/apps/gnomemeeting/view/notebook_info", 0);

  if (gconf_client_get_bool 
      (client, "/apps/gnomemeeting/view/show_control_panel", 0))
    gtk_widget_show_all (GTK_WIDGET (gw->main_notebook));

  gtk_notebook_set_page (GTK_NOTEBOOK (gw->main_notebook), 
			 selected_page);

  gtk_signal_connect_after (GTK_OBJECT (gw->main_notebook), "switch-page",
			    GTK_SIGNAL_FUNC (notebook_page_changed_callback), 
			    gw->main_notebook);


  /* The drawing area that will display the webcam images */
  frame = gtk_frame_new (NULL);
  gw->video_frame = gtk_handle_box_new();

  gtk_container_add (GTK_CONTAINER (frame), gw->video_frame);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

  gw->drawing_area = gtk_drawing_area_new ();

  gtk_container_add (GTK_CONTAINER (gw->video_frame), gw->drawing_area);
  gtk_widget_set_usize (GTK_WIDGET (gw->video_frame), 
			GM_QCIF_WIDTH + GM_FRAME_SIZE, GM_QCIF_HEIGHT);

  gtk_drawing_area_size (GTK_DRAWING_AREA (gw->drawing_area), 
			 GM_QCIF_WIDTH, GM_QCIF_HEIGHT);

  gtk_signal_connect (GTK_OBJECT (gw->drawing_area), "expose_event",
		      (GtkSignalFunc) expose_event, gw);    	

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (frame), 
		    0, 2, 0, 1,
		    GTK_EXPAND,
		    (GtkAttachOptions) NULL,
		    10, 10);

  gtk_widget_show_all (GTK_WIDGET (frame));


  /* The Chat Window */
  gnomemeeting_init_chat_window ();
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (gw->chat_window), 
 		    2, 4, 0, 3,
 		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
 		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
 		    GNOME_PAD_SMALL, GNOME_PAD_SMALL);
 if (gconf_client_get_bool 
      (client, "/apps/gnomemeeting/view/show_chat_window", 0))
    gtk_widget_show_all (GTK_WIDGET (gw->chat_window));


  /* The remote name */
  gw->remote_name = gtk_entry_new ();
  gtk_entry_set_editable (GTK_ENTRY (gw->remote_name), FALSE);

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
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

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
			     GTK_SHADOW_NONE);

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

  label = gtk_label_new (_("Video"));  

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
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

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

  
  label = gtk_label_new (_("Audio"));

  gtk_notebook_append_page (GTK_NOTEBOOK (gw->main_notebook), frame, label);
}




