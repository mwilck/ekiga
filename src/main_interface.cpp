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
#include "splash.h"
#include "common.h"
#include "menu.h"
#include "toolbar.h"
#include "callbacks.h"
#include "audio.h"
#include "webcam.h"

#include "../pixmaps/text_logo.xpm"
#include "../pixmaps/speaker.xpm"
#include "../pixmaps/mic.xpm"
#include "../pixmaps/brightness.xpm"
#include "../pixmaps/whiteness.xpm"
#include "../pixmaps/contrast.xpm"
#include "../pixmaps/color.xpm"


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
  char *audio_mixer;

  GM_window_widgets *gw = (GM_window_widgets *) data;
  
  vol_play =  (int) (GTK_ADJUSTMENT (gw->adj_play)->value) * 257;
  vol_rec =  (int) (GTK_ADJUSTMENT (gw->adj_rec)->value) * 257;

  audio_mixer = (gchar *) 
    gtk_object_get_data (GTK_OBJECT (gw->adj_play), "audio_mixer");

  GM_volume_set (audio_mixer, 0, &vol_play);
  GM_volume_set (audio_mixer, 1, &vol_rec);
}


void brightness_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GM_window_widgets *gw = (GM_window_widgets *) data;

  int brightness;

  brightness =  (int) (GTK_ADJUSTMENT (gw->adj_brightness)->value);


  if (GTK_WIDGET_IS_SENSITIVE (GTK_WIDGET (gw->video_settings_frame)))
    GM_cam_set_brightness (gw, brightness * 256);
}


void whiteness_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GM_window_widgets *gw = (GM_window_widgets *) data;

  int whiteness;

  whiteness =  (int) (GTK_ADJUSTMENT (gw->adj_whiteness)->value);


  if (GTK_WIDGET_IS_SENSITIVE (GTK_WIDGET (gw->video_settings_frame)))
    GM_cam_set_whiteness (gw, whiteness * 256);
}


void colour_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GM_window_widgets *gw = (GM_window_widgets *) data;

  int colour;

  colour =  (int) (GTK_ADJUSTMENT (gw->adj_colour)->value);


  if (GTK_WIDGET_IS_SENSITIVE (GTK_WIDGET (gw->video_settings_frame)))
    GM_cam_set_colour (gw, colour * 256);
}


void contrast_changed (GtkAdjustment *adjustment, gpointer data)
{ 
  GM_window_widgets *gw = (GM_window_widgets *) data;

  int contrast;

  contrast =  (int) (GTK_ADJUSTMENT (gw->adj_contrast)->value);


  if (GTK_WIDGET_IS_SENSITIVE (GTK_WIDGET (gw->video_settings_frame)))
    GM_cam_set_contrast (gw, contrast * 256);
}

/******************************************************************************/


/******************************************************************************/
/* The functions                                                              */
/******************************************************************************/

void GM_main_interface_init (GM_window_widgets *gw, options *opts)
{ 
  GtkWidget *table, *table_in;	
  GtkWidget *frame;


  // Init the splash screen
  if (opts->show_splash)
    {
      gw->splash_win = GM_splash_init ();     
      GM_splash_advance_progress (gw->splash_win, 
				  _("Building main interface"), 0.15);
    }

  
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


  // The drawing area that will display the webcam images 
  gw->video_frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (gw->video_frame), GTK_SHADOW_IN);

  gw->drawing_area = gtk_drawing_area_new ();

  gtk_container_add (GTK_CONTAINER (gw->video_frame), gw->drawing_area);
  gtk_widget_set_usize (GTK_WIDGET (gw->video_frame), 
			176 + 4, 144 + 4);

  gtk_drawing_area_size (GTK_DRAWING_AREA (gw->drawing_area), 176, 144);

  gtk_signal_connect (GTK_OBJECT (gw->drawing_area), "expose_event",
		      (GtkSignalFunc) expose_event, gw);    	

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (gw->video_frame), 
		    5, 30, 11, 37,
		    (GtkAttachOptions) NULL,
		    (GtkAttachOptions) NULL,
		    10, 10);


  // The statusbar
  gw->statusbar = gnome_appbar_new (FALSE, TRUE, GNOME_PREFERENCES_NEVER);	
  gnome_app_set_statusbar (GNOME_APP (gm), gw->statusbar);


  // The menu and toolbar
  GM_menu_init (gm,gw);
  GM_toolbar_init (gm, gw);	  
 

  // Init sockets
  static GnomeMeeting instance (gw, opts);

  if (opts->show_splash)
    {
      GM_splash_advance_progress (gw->splash_win, "Done  !", 1.00);
      gtk_widget_destroy (gw->splash_win);
    }

  gtk_widget_show_all (gm);
	
  // The logo
  gw->pixmap = gdk_pixmap_new(gw->drawing_area->window, 352, 288, -1);
  GM_init_main_interface_logo (gw);

  // Create a popup menu to attach it to the drawing area 
  GM_popup_menu_init (gw->drawing_area);
}


void GM_init_main_interface_remote_user_info (GtkWidget *notebook, 
					      GM_window_widgets *gw,
					      options *opts)
{
  GtkWidget *frame;
  GtkWidget *label;
  gchar * clist_titles [] = {N_("Info")};

  frame = gtk_frame_new (_("Remote User Info"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);

  gw->user_list = gtk_clist_new_with_titles (1, clist_titles);

  gtk_container_add (GTK_CONTAINER (frame), gw->user_list);
  gtk_container_set_border_width (GTK_CONTAINER (gw->user_list), 
				  GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
  
  gtk_clist_set_row_height (GTK_CLIST (gw->user_list), 15);
  gtk_clist_set_column_width (GTK_CLIST (gw->user_list), 0, 200);
  
  gtk_clist_set_shadow_type (GTK_CLIST (gw->user_list), GTK_SHADOW_IN);

  label = gtk_label_new (_("Remote User Info"));

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

  GtkTooltips *tip;

  int whiteness = 0, brightness = 0, colour = 0, contrast = 0;


  /* Webcam Control Frame */		
  gw->video_settings_frame = gtk_frame_new (_("Video Settings"));
  gtk_frame_set_shadow_type (GTK_FRAME (gw->video_settings_frame), 
			     GTK_SHADOW_OUT);

  /* Put a table in the first frame */
  table = gtk_table_new (4, 4, FALSE);
  gtk_container_add (GTK_CONTAINER (gw->video_settings_frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (gw->video_settings_frame), 
				  GNOME_PAD_SMALL);


  /* Read the values */
  GM_cam_get_params (opts, &whiteness, &brightness, &colour, &contrast);


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

  // disable
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

  GM_volume_get (opts->audio_mixer, 0, &vol);
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

  GM_volume_get (opts->audio_mixer, 1, &vol);
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
  gtk_object_set_data (GTK_OBJECT (gw->adj_play), "audio_mixer", 
		       (gpointer) opts->audio_mixer);

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
  update_rec.width = 176;
  update_rec.height = 144;

  if (gw->drawing_area->allocation.width != 176 &&
      gw->drawing_area->allocation.height != 144)
    {
      gtk_drawing_area_size (GTK_DRAWING_AREA (gw->drawing_area), 
			     176, 144);
      gtk_widget_set_usize (GTK_WIDGET (gw->video_frame),
			    176 + 4, 144 + 4);
    }

  gdk_draw_rectangle (gw->pixmap, gw->drawing_area->style->black_gc, TRUE,	
		      0, 0, 176, 144);

  text_logo = gdk_pixmap_create_from_xpm_d (gm->window, &text_logo_mask,
					    NULL,
					    (gchar **) text_logo_xpm);

  gdk_draw_pixmap (gw->pixmap, gw->drawing_area->style->black_gc, 
		   text_logo, 0, 0, 
		   (176 - 150) / 2,
		   (144 - 62) / 2, -1, -1);

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
