/***************************************************************************
                          main_interface.cpp  -  description
                             -------------------
    begin                : Mon Mar 26 2001
    copyright            : (C) 2001 by Damien Sandras
    description          : Contains the functions to display the main interface
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

#include "../config.h"

#include "main_interface.h"
#include "main.h"
#include "callbacks.h"
#include "docklet.h"
#include "splash.h"
#include "ils.h"
#include "common.h"
#include "menu.h"
#include "toolbar.h"
#include "callbacks.h"
#include "audio.h"
#include "videograbber.h"
#include "endpoint.h"
#include "preferences.h"

#include "../pixmaps/text_logo.xpm"
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


/******************************************************************************/
/* Global Variables                                                           */
/******************************************************************************/

extern GtkWidget *gm;
extern GnomeMeeting *MyApp;	

/******************************************************************************/


/******************************************************************************/
/* GTK Callbacks                                                              */
/******************************************************************************/

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


void audio_volume_changed (GtkAdjustment *adjustment, gpointer data)
{
  int vol_play, vol_rec;
  char *audio_recorder_mixer;
  char *audio_player_mixer;

  GM_window_widgets *gw = (GM_window_widgets *) data;
  
  vol_play =  (int) (GTK_ADJUSTMENT (gw->adj_play)->value) * 257;
  vol_rec =  (int) (GTK_ADJUSTMENT (gw->adj_rec)->value) * 257;

  // returns a pointer to the data, not a copy => no need to free
  audio_player_mixer = (gchar *) 
    gtk_object_get_data (GTK_OBJECT (gw->adj_play), "audio_player_mixer");

  audio_recorder_mixer = (gchar *) 
    gtk_object_get_data (GTK_OBJECT (gw->adj_rec), "audio_recorder_mixer");

  GM_volume_set (audio_player_mixer, 0, &vol_play);
  GM_volume_set (audio_recorder_mixer, 1, &vol_rec);
}


void brightness_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GM_window_widgets *gw = (GM_window_widgets *) data;
  GMVideoGrabber *video_grabber = MyApp->Endpoint ()->GetVideoGrabber ();

  int brightness;

  brightness =  (int) (GTK_ADJUSTMENT (gw->adj_brightness)->value);

  video_grabber->SetBrightness (brightness * 256);
}


void whiteness_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GM_window_widgets *gw = (GM_window_widgets *) data;
  GMVideoGrabber *video_grabber = MyApp->Endpoint ()->GetVideoGrabber ();

  int whiteness;

  whiteness =  (int) (GTK_ADJUSTMENT (gw->adj_whiteness)->value);

  video_grabber->SetWhiteness (whiteness * 256);
}


void colour_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GM_window_widgets *gw = (GM_window_widgets *) data;
  GMVideoGrabber *video_grabber = MyApp->Endpoint ()->GetVideoGrabber ();

  int colour;

  colour =  (int) (GTK_ADJUSTMENT (gw->adj_colour)->value);

  video_grabber->SetColour (colour * 256);
}


void contrast_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GM_window_widgets *gw = (GM_window_widgets *) data;
  GMVideoGrabber *video_grabber = MyApp->Endpoint ()->GetVideoGrabber ();

  int contrast;

  contrast =  (int) (GTK_ADJUSTMENT (gw->adj_contrast)->value);

  video_grabber->SetContrast (contrast * 256);
}


void preview_button_clicked (GtkButton *button, gpointer data)
{
  options *opts = (options *) data;
  GMVideoGrabber *video_grabber = (GMVideoGrabber *) MyApp->Endpoint ()->GetVideoGrabber ();

  if (!video_grabber->IsOpened ())
    {
      // Start the video preview
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
      video_grabber->Open (1);
      opts->video_preview = 1;
    }
  else
    {
      // Stop the video preview
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
      video_grabber->Close ();
      opts->video_preview = 0;
    }
}


void left_arrow_clicked (GtkWidget *w, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;

  gtk_notebook_prev_page (GTK_NOTEBOOK (gw->main_notebook));

  GtkWidget *object = (GtkWidget *) 
    gtk_object_get_data (GTK_OBJECT (gm),
			 "notebook_view_uiinfo");

  GnomeUIInfo *notebook_view_uiinfo = (GnomeUIInfo *) object;

  GTK_CHECK_MENU_ITEM (notebook_view_uiinfo [0].widget)->active =
    (gtk_notebook_get_current_page (GTK_NOTEBOOK (gw->main_notebook)) == 0);
  GTK_CHECK_MENU_ITEM (notebook_view_uiinfo [1].widget)->active = 
    (gtk_notebook_get_current_page (GTK_NOTEBOOK (gw->main_notebook)) == 1);
  GTK_CHECK_MENU_ITEM (notebook_view_uiinfo [2].widget)->active = 
    (gtk_notebook_get_current_page (GTK_NOTEBOOK (gw->main_notebook)) == 2);
  GTK_CHECK_MENU_ITEM (notebook_view_uiinfo [3].widget)->active =
    (gtk_notebook_get_current_page (GTK_NOTEBOOK (gw->main_notebook)) == 3);
}


void right_arrow_clicked (GtkWidget *w, gpointer data)
{
  GM_window_widgets *gw = (GM_window_widgets *) data;

  gtk_notebook_next_page (GTK_NOTEBOOK (gw->main_notebook));

  GtkWidget *object = (GtkWidget *) 
    gtk_object_get_data (GTK_OBJECT (gm),
			 "notebook_view_uiinfo");

  GnomeUIInfo *notebook_view_uiinfo = (GnomeUIInfo *) object;

  GTK_CHECK_MENU_ITEM (notebook_view_uiinfo [0].widget)->active =
    (gtk_notebook_get_current_page (GTK_NOTEBOOK (gw->main_notebook)) == 0);
  GTK_CHECK_MENU_ITEM (notebook_view_uiinfo [1].widget)->active = 
    (gtk_notebook_get_current_page (GTK_NOTEBOOK (gw->main_notebook)) == 1);
  GTK_CHECK_MENU_ITEM (notebook_view_uiinfo [2].widget)->active = 
    (gtk_notebook_get_current_page (GTK_NOTEBOOK (gw->main_notebook)) == 2);
  GTK_CHECK_MENU_ITEM (notebook_view_uiinfo [3].widget)->active =
    (gtk_notebook_get_current_page (GTK_NOTEBOOK (gw->main_notebook)) == 3);
}


void silence_detection_button_clicked (GtkWidget *w, gpointer data)
{
  MyApp->Endpoint ()->ChangeSilenceDetection ();
}


static void gm_quit_callback (GtkWidget *widget, GdkEvent *event, 
			      gpointer data)
{
  quit_callback (NULL, data);
}  

/******************************************************************************/


/******************************************************************************/
/* The functions                                                              */
/******************************************************************************/

void GM_init (GM_window_widgets *gw, GM_pref_window_widgets *pw,
	      GM_ldap_window_widgets *lw, options *opts, 
	      int argc, char ** argv, char ** envp)
{
  GMH323EndPoint *endpoint = NULL;
  gchar *text = NULL;
  int debug = 0;
  
  if (config_first_time ())
    init_config ();

  read_config (opts);

  // Cope with command line options
  static struct poptOption arguments[] =
    {
      {"debug", 'd', POPT_ARG_NONE, 
       0, 0, N_("Prints debug messages in the console"), NULL},
      {NULL, 0, 0, NULL, 0, NULL, NULL}
    };

  for (int i = 0; i < argc; i++) 
    {
      if (!strcmp (argv[i], "-d") || !strcmp (argv[i], "--debug"))
	debug = 1;
    } 

  // Gnome Initialisation
  // CORBA is needed for the docklet stuff
  CORBA_Environment ev;
  CORBA_exception_init (&ev);
  gnome_CORBA_init_with_popt_table (PACKAGE, VERSION, &argc, argv,
				    arguments, 0, NULL, 
				    static_cast<GnorbaInitFlags> (0), &ev);


  gm = gnome_app_new ("gnomemeeting", _("GnomeMeeting"));
  gtk_window_set_policy (GTK_WINDOW (gm), FALSE, FALSE, TRUE);

  // Startup Process
  // Main interface creation
  if (opts->show_splash)
    {
      // Init the splash screen
      gw->splash_win = GM_splash_init ();

      GM_splash_advance_progress (gw->splash_win, 
				  _("Building main interface"), 0.15);
    }

  gw->docklet = GM_docklet_init ();

  /* Build the interface */
  GM_main_interface_init (gw, opts);
  GM_ldap_init (gw, lw, opts);
  gnomemeeting_preferences_init (0, gw, pw, opts);
  gnomemeeting_menu_init (gm, gw, pw);
  GM_toolbar_init (gm, gw, pw);	

  // Launch the GnomeMeeting H.323 part
  static GnomeMeeting instance (gw, lw, opts);
  endpoint = MyApp->Endpoint ();
  endpoint->Initialise ();

  if (debug)
    PTrace::Initialise (3);

  endpoint->RemoveAllCapabilities ();

  if (opts->show_splash)
    GM_splash_advance_progress (gw->splash_win, _("Adding Audio Capabilities"), 
				0.30);
  endpoint->AddAudioCapabilities ();

  if (opts->show_splash)
    GM_splash_advance_progress (gw->splash_win, _("Adding Video Capabilities"), 
				0.45);
  endpoint->AddVideoCapabilities (opts->video_size);

  if (opts->ldap)
    {
      if (opts->show_splash)
        GM_splash_advance_progress (gw->splash_win, 
				    _("Registering to ILS directory"), 
				    0.60);
      GMILSClient *gm_ils_client = (GMILSClient *) endpoint->GetILSClient ();
      gm_ils_client->Register ();
    }

  
  // Run the listener thread
  if (opts->show_splash)
    GM_splash_advance_progress (gw->splash_win, 
				_("Starting the listener thread"), 
				0.70);

  if (!endpoint->StartListener ())
    {
      GtkWidget *msg_box = gnome_message_box_new (_("Could not start the listener thread. You will not be able to receive incoming calls."), GNOME_MESSAGE_BOX_ERROR, "OK", NULL);

      gtk_widget_show (msg_box);
    }


  // Register to the Gatekeeper
  if (opts->gk)
    {
      if (opts->show_splash)
        GM_splash_advance_progress (gw->splash_win, 
				    _("Registering to the Gatekeeper"), 
				    0.80);

      endpoint->GatekeeperRegister ();
    }

  
  // Initialise the devices
  if (opts->show_splash)
    GM_splash_advance_progress (gw->splash_win, 
				_("Detecting available audio and video devices"), 0.90);
  
  /* Search for devices */
  gw->audio_player_devices = PSoundChannel::GetDeviceNames (PSoundChannel::Player);
  gw->audio_recorder_devices = PSoundChannel::GetDeviceNames (PSoundChannel::Recorder);
  gw->video_devices = PVideoInputDevice::GetInputDeviceNames ();

  /* Set recording source and set micro to record */
  MyApp->Endpoint()->SetSoundChannelPlayDevice(opts->audio_player);
  MyApp->Endpoint()->SetSoundChannelRecordDevice(opts->audio_recorder);
  
  /* Translators: This is shown in the history. */
  text = g_strdup_printf (_("Set Audio player device to %s"), 
			  opts->audio_player);
  GM_log_insert (gw->log_text, text);
  g_free (text);
  
  /* Translators: This is shown in the history. */
  text = g_strdup_printf (_("Set Audio recorder device to %s"), 
			  opts->audio_recorder);
  GM_log_insert (gw->log_text, text);
  g_free (text);
  
  GM_set_recording_source (opts->audio_recorder_mixer, 0); 

  if (opts->show_splash)
    GM_splash_advance_progress (gw->splash_win, 
				_("Done!"), 0.9999);

  gtk_widget_show (GTK_WIDGET (gm));
	
  /* Start to grab? */
  if (opts->video_preview)
    {
      GMVideoGrabber *video_grabber = 
	(GMVideoGrabber *) endpoint->GetVideoGrabber ();

      video_grabber->Open (1);
    }

  // The logo
  gw->pixmap = gdk_pixmap_new (gw->drawing_area->window, 
			       GM_CIF_WIDTH * 2, GM_CIF_HEIGHT * 2, -1);
  GM_init_main_interface_logo (gw);

  /* Create a popup menu to attach it to the drawing area  */
  gnomemeeting_popup_menu_init (gw->drawing_area, gw);

  /* Set icon */
  gtk_widget_push_visual(gdk_rgb_get_visual());
  gtk_widget_push_colormap(gdk_rgb_get_cmap());
  gnome_window_icon_set_from_file 
    (GTK_WINDOW (gm), "/usr/share/pixmaps/gnomemeeting-logo-icon.png");

  if (gw->splash_win)
    gtk_widget_destroy (gw->splash_win);

  /* if the user tries to close the window : delete_event */
  gtk_signal_connect (GTK_OBJECT (gm), "delete_event",
		      GTK_SIGNAL_FUNC (gm_quit_callback), (gpointer) gw);
}


void GM_main_interface_init (GM_window_widgets *gw, options *opts)
{ 
  GtkWidget *table, *table_in;	
  GtkWidget *frame;
  GtkWidget *pixmap;
  GtkWidget *left_arrow, *right_arrow;
  GtkWidget *handle_box;

  GtkTooltips *tip;

  // Create a table in the main window to attach things like buttons
  table = gtk_table_new (58, 35, FALSE);
  
  gnome_app_set_contents (GNOME_APP (gm), GTK_WIDGET (table));


  // The remote IP field
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

  gw->combo = gtk_combo_new ();
  gtk_combo_disable_activate(GTK_COMBO(gw->combo));

  gtk_table_attach (GTK_TABLE (table_in), GTK_WIDGET (gw->combo), 2, 8, 0, 4,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    10, 10);

  gtk_widget_show_all (GTK_WIDGET (frame));

  // The Notebook 
  gw->main_notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gw->main_notebook), GTK_POS_TOP);
  gtk_notebook_popup_enable (GTK_NOTEBOOK (gw->main_notebook));
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (gw->main_notebook), FALSE);

  GM_init_main_interface_remote_user_info (gw->main_notebook, gw, opts);
  GM_init_main_interface_log (gw->main_notebook, gw, opts);
  GM_init_main_interface_audio_settings (gw->main_notebook, gw, opts);
  GM_init_main_interface_video_settings (gw->main_notebook, gw, opts);

  gtk_widget_set_usize (GTK_WIDGET (gw->main_notebook), 246, 134);
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (gw->main_notebook),
		    5, 30, 38, 58,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    10, 10); 

  if (opts->show_notebook)
    gtk_widget_show_all (GTK_WIDGET (gw->main_notebook));

  // The drawing area that will display the webcam images 
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

  // The control buttons
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

  GTK_TOGGLE_BUTTON (gw->preview_button)->active = opts->video_preview;

  gtk_signal_connect (GTK_OBJECT (gw->preview_button), "clicked",
                      GTK_SIGNAL_FUNC (preview_button_clicked), opts);

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
  left_arrow = gtk_button_new ();

  pixmap = gnome_pixmap_new_from_xpm_d ((char **) left_arrow_xpm);
  gtk_container_add (GTK_CONTAINER (left_arrow), pixmap);

  gtk_widget_set_usize (GTK_WIDGET (left_arrow), 24, 24);

  gtk_table_attach (GTK_TABLE (table_in), 
		    left_arrow, 4, 5, 0, 1,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    2, 2);

  gtk_signal_connect (GTK_OBJECT (left_arrow), "clicked",
		      GTK_SIGNAL_FUNC (left_arrow_clicked), gw);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, left_arrow,
			_("Click here to display the previous section of the Control Panel."), NULL);

  /* Right arrow */
  right_arrow = gtk_button_new ();

  pixmap = gnome_pixmap_new_from_xpm_d ((char **) right_arrow_xpm);
  gtk_container_add (GTK_CONTAINER (right_arrow), pixmap);

  gtk_widget_set_usize (GTK_WIDGET (right_arrow), 24, 24);

  gtk_table_attach (GTK_TABLE (table_in), 
		    right_arrow, 5, 6, 0, 1,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    2, 2);

  gtk_signal_connect (GTK_OBJECT (right_arrow), "clicked",
		      GTK_SIGNAL_FUNC (right_arrow_clicked), gw);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, right_arrow,
			_("Click here to display the next section of the Control Panel."), NULL);


  // The statusbar
  gw->statusbar = gnome_appbar_new (FALSE, TRUE, GNOME_PREFERENCES_NEVER);	
  gnome_app_set_statusbar (GNOME_APP (gm), gw->statusbar);

  if (opts->show_statusbar)
    gtk_widget_show (GTK_WIDGET (gw->statusbar));
  else
    gtk_widget_hide (GTK_WIDGET (gw->statusbar));

  if (opts->show_quickbar)
    gtk_widget_show_all (GTK_WIDGET (gw->quickbar_frame));
  
}


void GM_init_main_interface_remote_user_info (GtkWidget *notebook, 
					      GM_window_widgets *gw,
					      options *opts)
{
  GtkWidget *frame;
  GtkWidget *label;
  GtkWidget *scr;
  gchar * clist_titles [] = {" ", N_("Info")};

  clist_titles [1] = gettext (clist_titles [1]);

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


void GM_init_main_interface_log (GtkWidget *notebook, 
				 GM_window_widgets *gw,
				 options *opts)
{
  GtkWidget *frame;
  GtkWidget *label;
  GtkWidget *scr;

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


void GM_init_main_interface_video_settings (GtkWidget *notebook, 
					    GM_window_widgets *gw,
					    options *opts)
{
  GtkWidget *label;
  
  GtkWidget *table;

  GtkWidget *pixmap;
  GtkWidget *hscale_brightness, *hscale_colour, 
    *hscale_contrast, *hscale_whiteness;

  int brightness = 0, colour = 0, contrast = 0, whiteness = 0;
  
  GtkTooltips *tip;

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

  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), gw->video_settings_frame, 
			    label);

}


void GM_init_main_interface_audio_settings (GtkWidget *notebook, 
					    GM_window_widgets *gw,
					    options *opts)
{
  GtkWidget *label;
  GtkWidget *hscale_play, *hscale_rec;
  GtkWidget *audio_table;
  GtkWidget *pixmap;

  int vol = 0;

  GtkWidget *frame;

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

  GM_volume_get (opts->audio_player_mixer, 0, &vol);
  vol = vol * 100 / 25700;
  gw->adj_play = gtk_adjustment_new (vol, 0.0, 100.0, 1.0, 5.0, 1.0);
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

  GM_volume_get (opts->audio_recorder_mixer, 1, &vol);
  vol = vol * 100 / 25700;
  gw->adj_rec = gtk_adjustment_new (vol, 0.0, 100.0, 1.0, 5.0, 1.0);
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

  /* To prevent opts to become global, add data to adj_play */
  gtk_object_set_data (GTK_OBJECT (gw->adj_play), "audio_player_mixer", 
		       g_strdup (opts->audio_player_mixer));

  gtk_object_set_data (GTK_OBJECT (gw->adj_rec), "audio_recorder_mixer", 
		       g_strdup (opts->audio_recorder_mixer));

  label = gtk_label_new (_("Audio Settings"));

  gtk_notebook_append_page (GTK_NOTEBOOK (gw->main_notebook), frame, label);
}


void GM_init_main_interface_logo (GM_window_widgets *gw)
{
  GdkPixmap *text_logo;
  GdkBitmap *text_logo_mask;
  GdkRectangle update_rec;
  
  update_rec.x = 0;
  update_rec.y = 0;
  update_rec.width = GM_QCIF_WIDTH;
  update_rec.height = GM_QCIF_HEIGHT;

  if (gw->drawing_area->allocation.width != GM_QCIF_WIDTH &&
      gw->drawing_area->allocation.height != GM_QCIF_HEIGHT)
    {
      gtk_drawing_area_size (GTK_DRAWING_AREA (gw->drawing_area), 
			     GM_QCIF_WIDTH, GM_QCIF_HEIGHT);
      gtk_widget_set_usize (GTK_WIDGET (gw->video_frame),
			    GM_QCIF_WIDTH + GM_FRAME_SIZE, GM_QCIF_HEIGHT);
    }

  gdk_draw_rectangle (gw->pixmap, gw->drawing_area->style->black_gc, TRUE,	
		      0, 0, GM_QCIF_WIDTH, GM_QCIF_HEIGHT);

  text_logo = gdk_pixmap_create_from_xpm_d (gm->window, &text_logo_mask,
					    NULL,
					    (gchar **) text_logo_xpm);

  gdk_draw_pixmap (gw->pixmap, gw->drawing_area->style->black_gc, 
		   text_logo, 0, 0, 
		   (GM_QCIF_WIDTH - 150) / 2,
		   (GM_QCIF_HEIGHT - 62) / 2, -1, -1);

  gtk_widget_draw (gw->drawing_area, &update_rec);   
}


void GM_log_insert (GtkWidget *w, char *text)
{
  time_t *timeptr;
  char *time_str;

  time_str = (char *) malloc (21);
  timeptr = new (time_t);
  
  time (timeptr);
  strftime(time_str, 20, "%H:%M:%S :  ", localtime (timeptr));

  gtk_text_insert (GTK_TEXT (w), NULL, NULL, NULL, time_str, -1);
  gtk_text_insert (GTK_TEXT (w), NULL, NULL, NULL, text, -1);
  gtk_text_insert (GTK_TEXT (w), NULL, NULL, NULL, "\n", -1);

  free (time_str);
  delete (timeptr);
}

/******************************************************************************/
