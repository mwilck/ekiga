
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
 *                        toolbar.cpp  -  description
 *                        ---------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2001 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          create the toolbar.
 *   email                : dsandras@seconix.com
 *
 */

#include "../config.h"


#include "toolbar.h"
#include "callbacks.h"
#include "gnomemeeting.h"
#include "connection.h"
#include "common.h"
#include "misc.h" 

#include "../pixmaps/connect.xpm"
#include "../pixmaps/disconnect.xpm"
#include "../pixmaps/ils.xpm"
#include "../pixmaps/settings.xpm"
#include "../pixmaps/gnome-chat.xpm"
#include "../pixmaps/eye.xpm"
#include "../pixmaps/speaker.xpm"
#include "../pixmaps/quickcam.xpm"

/* Declarations */

extern GnomeMeeting *MyApp;
extern GtkWidget *gm;

/* Here due to a bug in GLIB 2.00 */
#define	g_signal_handlers_block_by_func(instance, func, data) \
    g_signal_handlers_block_matched ((instance), (GSignalMatchType) (G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA), \
				     0, 0, NULL, (func), (data))
#define	g_signal_handlers_unblock_by_func(instance, func, data) \
    g_signal_handlers_unblock_matched ((instance), (GSignalMatchType) (G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA), \
				       0, 0, NULL, (func), (data))


/* Static functions */
static void connect_button_clicked (GtkToggleButton *w, gpointer data)
{
  if (gtk_toggle_button_get_active (w))
    MyApp->Connect ();
  else {

    GMH323Connection *connection = 
      (GMH323Connection *) MyApp->Endpoint ()->GetCurrentConnection ();
	
    if (connection != NULL)
      connection->UnPauseChannels ();
    
    MyApp->Disconnect ();
  }
}


void connect_button_update_pixmap (GtkToggleButton *w, int pressed)
{
  GtkWidget *pixmap = NULL;
  GdkPixmap *Pixmap = NULL;
  GdkBitmap *mask = NULL;
  GdkPixbuf *pixbuf = NULL;

  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);

  pixmap = (GtkWidget *) 
    gtk_object_get_data (GTK_OBJECT (w), "pixmap");
  
  if (pixmap != NULL)	{

    if (pressed == 1) {

      pixbuf = 
	gdk_pixbuf_new_from_xpm_data ((const char **) connect_xpm);

      /* Block the signal */
      g_signal_handlers_block_by_func (G_OBJECT (w), 
				       connect_button_clicked, 
				       (gpointer) gw->connect_button);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), TRUE);
      g_signal_handlers_unblock_by_func (G_OBJECT (w), 
				       connect_button_clicked, 
				       (gpointer) gw->connect_button);
    }
    else {

      pixbuf = 
	gdk_pixbuf_new_from_xpm_data ((const char **) disconnect_xpm);
      
      g_signal_handlers_block_by_func (G_OBJECT (w), 
				       connect_button_clicked, 
				       (gpointer) gw->connect_button);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), FALSE);
      g_signal_handlers_unblock_by_func (G_OBJECT (w), 
					 connect_button_clicked, 
					 (gpointer) gw->connect_button);
    }

    if (pixbuf) {

      gdk_pixbuf_render_pixmap_and_mask (pixbuf, &Pixmap, &mask, 127);
    
      gtk_pixmap_set (GTK_PIXMAP (pixmap), Pixmap, mask);
      gtk_object_remove_data (GTK_OBJECT (w), "pixmap");
      gtk_object_set_data (GTK_OBJECT (w), "pixmap", pixmap);
    }
    else {

      GTK_TOGGLE_BUTTON (w)->active = !GTK_TOGGLE_BUTTON (w)->active; 
      gtk_widget_draw (GTK_WIDGET (w), NULL);
    }
  }
}


/* DESCRIPTION  :  This callback is called when the user toggles the 
 *                 corresponding component in the toolbar. 
 *                 (See menu_toggle_changed)
 * BEHAVIOR     :  Updates the gconf cache.
 * PRE          :  data is the key.
 */
static void toolbar_toggle_changed (GtkWidget *widget, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();
  bool shown = gconf_client_get_bool (client, (gchar *) data, NULL);

  gconf_client_set_bool (client,
			 (gchar *) data,
			 !shown, NULL);
}


/* The functions */

void gnomemeeting_init_toolbar ()
{
  GtkWidget *pixmap;
  GdkPixmap *Pixmap;
  GdkBitmap *mask;
  GdkPixbuf *pixbuf;
  GtkWidget *hbox;

  GtkTooltips *tip;
  GConfClient *client = gconf_client_get_default ();

  GM_window_widgets *gw = gnomemeeting_get_main_window (gm);
  
  static GnomeUIInfo left_toolbar [] =
    {
      {
	GNOME_APP_UI_ITEM,
	N_("ILS Directory"), N_("Find friends on ILS"),
	(void *) gnomemeeting_component_view, gw->ldap_window, NULL,
	GNOME_APP_PIXMAP_DATA, ils_xpm,
	NULL, GDK_CONTROL_MASK, NULL
	},
        {
	GNOME_APP_UI_ITEM,
	N_("Chat"), N_("Make a text chat with your friend"),
	(void *) toolbar_toggle_changed, 
	(gpointer) "/apps/gnomemeeting/view/show_chat_window",
	NULL, GNOME_APP_PIXMAP_DATA, gnome_chat_xpm,
	NULL, GDK_CONTROL_MASK, NULL
	},
	{
	GNOME_APP_UI_ITEM,
	N_("Control Panel"), N_("Display the control panel"),
	(void *) toolbar_toggle_changed, 
	(gpointer) "/apps/gnomemeeting/view/show_control_panel",
	NULL, GNOME_APP_PIXMAP_DATA, settings_xpm,
	NULL, GDK_CONTROL_MASK, NULL
	},
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_END
    };


  /* Combo */
  gw->combo = gnomemeeting_history_combo_box_new("/apps/gnomemeeting/"
 						 "history/called_hosts");

  gtk_combo_set_use_arrows_always (GTK_COMBO(gw->combo), TRUE);

  gtk_combo_disable_activate (GTK_COMBO (gw->combo));
  gtk_signal_connect (GTK_OBJECT (GTK_COMBO (gw->combo)->entry), "activate",
  		      GTK_SIGNAL_FUNC (connect_cb), NULL);


  /* Both toolbars */
  GtkWidget *toolbar = gtk_toolbar_new ();

  hbox = gtk_hbox_new (FALSE, 2);

  gtk_box_pack_start (GTK_BOX (hbox), gw->combo, TRUE, TRUE, 1);
  gtk_box_pack_start (GTK_BOX (hbox), toolbar, FALSE, FALSE, 1);

  gnome_app_add_docked (GNOME_APP (gm), hbox, "main_toolbar",
 			BONOBO_DOCK_ITEM_BEH_EXCLUSIVE,
 			BONOBO_DOCK_TOP, 1, 0, 0);

  gtk_container_set_border_width (GTK_CONTAINER (toolbar), 2);

  gw->connect_button = gtk_toggle_button_new ();
  pixbuf =  gdk_pixbuf_new_from_xpm_data ((const char **) disconnect_xpm);
  gdk_pixbuf_render_pixmap_and_mask (pixbuf, &Pixmap, &mask, 127);
  pixmap = gtk_pixmap_new (Pixmap, mask);
  gtk_container_add (GTK_CONTAINER (gw->connect_button), pixmap);

  gtk_pixmap_set (GTK_PIXMAP (pixmap), Pixmap, mask);
  gtk_object_set_data (GTK_OBJECT (gw->connect_button), "pixmap", pixmap);

  gtk_widget_set_usize (GTK_WIDGET (gw->connect_button), 28, 28);
  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), gw->connect_button, NULL, NULL);

  gtk_signal_connect (GTK_OBJECT (gw->connect_button), "clicked",
                      GTK_SIGNAL_FUNC (connect_button_clicked), 
		      gw->connect_button);

  GtkWidget *toolbar2 = gtk_toolbar_new ();

  gnome_app_fill_toolbar(GTK_TOOLBAR (toolbar2), left_toolbar, NULL);
  gnome_app_add_toolbar (GNOME_APP (gm), GTK_TOOLBAR(toolbar2),
 			 "left_toolbar", BONOBO_DOCK_ITEM_BEH_EXCLUSIVE,
 			 BONOBO_DOCK_LEFT, 3, 0, 0);


  /* Video Preview Button */
  gw->preview_button = gtk_toggle_button_new ();

  pixmap = gnome_pixmap_new_from_xpm_d ((const gchar **) eye_xpm);
  gtk_container_add (GTK_CONTAINER (gw->preview_button), pixmap);

  gtk_widget_set_usize (GTK_WIDGET (gw->preview_button), 24, 24);

  GTK_TOGGLE_BUTTON (gw->preview_button)->active = 
    gconf_client_get_bool (client, "/apps/gnomemeeting/devices/video_preview", 
			   NULL);

  gtk_signal_connect (GTK_OBJECT (gw->preview_button), "clicked",
                      GTK_SIGNAL_FUNC (toggle_changed), 
		      (gpointer) "/apps/gnomemeeting/devices/video_preview");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, gw->preview_button,
			_("Click here to begin to display images from your camera device."), NULL);

  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar2), 
			     gw->preview_button, NULL, NULL);


  /* Audio Channel Button */
  gw->audio_chan_button = gtk_toggle_button_new ();

  pixmap = gnome_pixmap_new_from_xpm_d ((const gchar **) speaker_xpm);
  gtk_container_add (GTK_CONTAINER (gw->audio_chan_button), pixmap);

  gtk_widget_set_usize (GTK_WIDGET (gw->audio_chan_button), 24, 24);

  gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_chan_button), FALSE);

  gtk_signal_connect (GTK_OBJECT (gw->audio_chan_button), "clicked",
                      GTK_SIGNAL_FUNC (pause_audio_callback), gw);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, gw->audio_chan_button,
			_("Audio Transmission Status. During a call, click here to pause the audio transmission."), NULL);

  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar2), 
			     gw->audio_chan_button, NULL, NULL);


  /* Video Channel Button */
  gw->video_chan_button = gtk_toggle_button_new ();

  pixmap = gnome_pixmap_new_from_xpm_d ((const gchar **) quickcam_xpm);
  gtk_container_add (GTK_CONTAINER (gw->video_chan_button), pixmap);

  gtk_widget_set_usize (GTK_WIDGET (gw->video_chan_button), 24, 24);

  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_chan_button), FALSE);

  gtk_signal_connect (GTK_OBJECT (gw->video_chan_button), "clicked",
                      GTK_SIGNAL_FUNC (pause_video_callback), gw);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, gw->video_chan_button,
			_("Video Transmission Status. During a call, click here to pause the video transmission."), NULL);

  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar2), 
			     gw->video_chan_button, NULL, NULL);


  gtk_widget_show_all (GTK_WIDGET (hbox));

  gtk_widget_show (GTK_WIDGET (gw->combo));
  gtk_widget_show_all (GTK_WIDGET (gw->connect_button));
  gtk_widget_show_all (GTK_WIDGET (gw->preview_button));
  gtk_widget_show_all (GTK_WIDGET (gw->audio_chan_button));
  gtk_widget_show_all (GTK_WIDGET (gw->video_chan_button));

  if (gconf_client_get_bool (client, "/apps/gnomemeeting/view/left_toolbar", 0)) 
    gtk_widget_show (GTK_WIDGET (gnome_app_get_dock_item_by_name(GNOME_APP (gm), "left_toolbar")));
  else
    gtk_widget_hide (GTK_WIDGET (gnome_app_get_dock_item_by_name(GNOME_APP (gm), "left_toolbar")));
}
