/*  toolbar.cpp - This file contains all the functions needed to
 *                create the toolbar.
 *
 *  GnomeMeeting -- A Video-Conferencing application
 *  Copyright (C) 2000-2002 Damien Sandras
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Started: Sat Dec 23 2000
 *
 *  Authors: Damien Sandras <dsandras@seconix.com>
 *           Kenneth Christiansen <kenneth@gnu.org>
 */

#include "../config.h"


#include "toolbar.h"
#include "callbacks.h"
#include "gnomemeeting.h"
#include "connection.h"
#include "common.h"
#include "misc.h" 
#include "stock-icons.h"
#include "history-combo.h"


/* Declarations */

extern GnomeMeeting *MyApp;
extern GtkWidget *gm;


static void url_combo_changed            (GtkEditable  *, gpointer);

#ifdef HAS_IXJ
static void speaker_phone_toggle_changed (GtkToggleButton *, gpointer);
#endif

static void connect_button_clicked       (GtkToggleButton *, gpointer);
static void toolbar_toggle_changed       (GtkWidget *, gpointer);
static void toolbar_button_changed       (GtkWidget *, gpointer);
static void toolbar_cp_button_changed    (GtkWidget *, gpointer);


/* Static functions */
static void url_combo_changed (GtkEditable  *e, gpointer data)
{
  GtkTooltips *tip = NULL;
  gchar *tip_text = (gchar *)
    gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (data)->entry));

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, GTK_WIDGET (GTK_COMBO (data)->entry), 
			tip_text, NULL);
}


#ifdef HAS_IXJ
static void speaker_phone_toggle_changed (GtkToggleButton *w, gpointer data)
{
  OpalLineInterfaceDevice *lid = NULL;

  lid = MyApp->Endpoint ()->GetLidDevice ();

  if (lid) {

    if (gtk_toggle_button_get_active (w)) {
      
      gnomemeeting_log_insert (_("Speakerphone enabled"));
      if (MyApp->Endpoint()->GetCallingState () == 0) /* Not in a call */
	lid->PlayTone (0, OpalLineInterfaceDevice::DialTone);

      if (MyApp->Endpoint()->GetCallingState () == 1) /* Calling */
	lid->PlayTone (0, OpalLineInterfaceDevice::RingTone);

      lid->EnableAudio(0, FALSE);
    }
    else {

      gnomemeeting_log_insert (_("Speakerphone disabled"));
      lid->EnableAudio(0, TRUE);
      lid->StopTone (0);
    }
  }
}
#endif


/* DESCRIPTION  :  This callback is called when the user toggles the 
 *                 connect button.
 * BEHAVIOR     :  Connect or disconnect.
 * PRE          :  /
 */
static void connect_button_clicked (GtkToggleButton *w, gpointer data)
{

  if (gtk_toggle_button_get_active (w))
    if (MyApp->Endpoint ()->GetCallingState () == 0) 
      MyApp->Connect ();
    else {
      
      connect_button_update_pixmap (w, 0);
      gnomemeeting_log_insert (_("Trying to stop calling"));
    }
  else
    MyApp->Disconnect ();
}


/* DESCRIPTION  :  This callback is called when the user toggles the 
 *                 corresponding component in the toolbar. 
 *                 (See menu_toggle_changed)
 * BEHAVIOR     :  Updates the gconf cache.
 * PRE          :  data is the key.
 */
static void toolbar_toggle_changed (GtkWidget *w, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();

  gconf_client_set_bool (client,
			 (gchar *) data,
			 gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w)), NULL);
}


/* DESCRIPTION  :  This callback is called when the user presses the control panel
 *                 button in the toolbar. 
 *                 (See menu_toggle_changed)
 * BEHAVIOR     :  Updates the gconf cache : 0 or 3 (off) for the cp section.
 * PRE          :  data is the key.
 */
static void toolbar_cp_button_changed (GtkWidget *w, gpointer data)
{
    GConfClient *client = gconf_client_get_default ();

    if (gconf_client_get_int (client, (gchar *) data, 0) == 3) 
    { 
        gconf_client_set_int (client, (gchar *) data, 0, NULL);
    } 
    else 
    {    
        gconf_client_set_int (client, (gchar *) data, 3, NULL);
    }
}


/* DESCRIPTION  :  This callback is called when the user presses a
 *                 button in the toolbar. 
 *                 (See menu_toggle_changed)
 * BEHAVIOR     :  Updates the gconf cache.
 * PRE          :  data is the key.
 */
static void toolbar_button_changed (GtkWidget *widget, gpointer data)
{
    GConfClient *client = gconf_client_get_default ();
  
    bool shown = gconf_client_get_bool (client, (gchar *) data, NULL);

    gconf_client_set_bool (client, (gchar *) data, !shown, NULL);
}



/* The functions */

void gnomemeeting_init_toolbar ()
{
  GtkWidget *pixmap;
  GdkPixmap *Pixmap;
  GdkBitmap *mask;
  GtkTooltips *tip;
  GdkPixbuf *pixbuf;
  GtkWidget *hbox;
  GtkWidget *image;
  GtkWidget *left_toolbar;

  GConfClient *client = gconf_client_get_default ();

  GmWindow *gw = gnomemeeting_get_main_window (gm);

  left_toolbar = gtk_toolbar_new ();

  gtk_toolbar_append_item  (GTK_TOOLBAR (left_toolbar),
                            _("ILS Directory"),
                            _("Find friends on ILS"),
                            NULL,
                            gtk_image_new_from_stock (GM_STOCK_ILS_DIRECTORY, 
                                                      GTK_ICON_SIZE_LARGE_TOOLBAR),
                            GTK_SIGNAL_FUNC (gnomemeeting_component_view),
                            (gpointer) gw->ldap_window); 

  gtk_toolbar_append_item  (GTK_TOOLBAR (left_toolbar),
                            _("Chat"),
                            _("Make a text chat with your friend"), 
                            NULL,
                            gtk_image_new_from_stock (GM_STOCK_TEXT_CHAT, 
                                                      GTK_ICON_SIZE_LARGE_TOOLBAR),
                            GTK_SIGNAL_FUNC (toolbar_button_changed),
                            (gpointer) "/apps/gnomemeeting/view/show_chat_window");
  
  gtk_toolbar_append_item  (GTK_TOOLBAR (left_toolbar),
                            _("Control Panel"),
                            _("Display the control panel"),
                            NULL,
                            gtk_image_new_from_stock (GM_STOCK_CONTROL_PANEL, 
                                                      GTK_ICON_SIZE_LARGE_TOOLBAR),
                            GTK_SIGNAL_FUNC (toolbar_cp_button_changed),
                            (gpointer) "/apps/gnomemeeting/view/control_panel_section");

  gtk_toolbar_append_space (GTK_TOOLBAR (left_toolbar));


  /* Both toolbars */

  /* The main horizontal toolbar */
  GtkWidget *toolbar = gtk_toolbar_new ();


  /* Combo */
  gw->combo = gm_history_combo_new ("/apps/gnomemeeting/"
                                    "history/called_urls_list");

  gtk_combo_set_use_arrows_always (GTK_COMBO(gw->combo), TRUE);
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry), "callto://");
  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, GTK_WIDGET (GTK_COMBO (gw->combo)->entry), 
			"callto://", NULL);

  gtk_combo_disable_activate (GTK_COMBO (gw->combo));
  g_signal_connect (G_OBJECT (GTK_COMBO (gw->combo)->entry), "activate",
  		    G_CALLBACK (connect_cb), NULL);

  hbox = gtk_hbox_new (FALSE, 2);

  gtk_box_pack_start (GTK_BOX (hbox), gw->combo, TRUE, TRUE, 1);
  gtk_box_pack_start (GTK_BOX (hbox), toolbar, FALSE, FALSE, 1);
 
  gtk_container_set_border_width (GTK_CONTAINER (toolbar), 2);

  g_signal_connect (G_OBJECT (GTK_WIDGET (GTK_COMBO(gw->combo)->entry)),
		    "changed", G_CALLBACK (url_combo_changed), 
		    (gpointer)  (gw->combo));


  /* The connect button */
  gw->connect_button = gtk_toggle_button_new ();
  
  image = gtk_image_new_from_stock (GM_STOCK_DISCONNECT, 
                                    GTK_ICON_SIZE_LARGE_TOOLBAR);

  gtk_container_add (GTK_CONTAINER (gw->connect_button), GTK_WIDGET (image));
  g_object_set_data (G_OBJECT (gw->connect_button), "image", image);

  gtk_widget_set_size_request (GTK_WIDGET (gw->connect_button), 28, 28);

  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), gw->connect_button,
			     NULL, NULL);

  g_signal_connect (G_OBJECT (gw->connect_button), "clicked",
                    G_CALLBACK (connect_button_clicked), 
		    gw->connect_button);

  gnome_app_add_docked (GNOME_APP (gm), hbox, "main_toolbar",
  			BONOBO_DOCK_ITEM_BEH_EXCLUSIVE,
  			BONOBO_DOCK_TOP, 1, 0, 0);

  gnome_app_add_toolbar (GNOME_APP (gm), GTK_TOOLBAR (left_toolbar),
 			 "left_toolbar", BONOBO_DOCK_ITEM_BEH_EXCLUSIVE,
 			 BONOBO_DOCK_LEFT, 3, 0, 0);

  gtk_toolbar_set_style (GTK_TOOLBAR (left_toolbar), GTK_TOOLBAR_ICONS);
 

  /* Video Preview Button */
  gw->preview_button = gtk_toggle_button_new ();

  image = gtk_image_new_from_stock (GM_STOCK_VIDEO_PREVIEW, 
                                    GTK_ICON_SIZE_MENU);

  gtk_container_add (GTK_CONTAINER (gw->preview_button), GTK_WIDGET (image));

  /* We set the key as data to be able to get the data in order to block       
     the signal in the gconf notifier */                             
  g_object_set_data (G_OBJECT (gw->preview_button), "gconf_key", 
		     (void *) "/apps/gnomemeeting/devices/video_preview");

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gw->preview_button), 
				gconf_client_get_bool (client, "/apps/gnomemeeting/devices/video_preview", NULL));

  g_signal_connect (G_OBJECT (gw->preview_button), "clicked",
		    G_CALLBACK (toolbar_toggle_changed), 
		    (gpointer) "/apps/gnomemeeting/devices/video_preview");

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, gw->preview_button,
			_("Click here to begin to display images from your camera device."),
			NULL);

  gtk_toolbar_append_widget (GTK_TOOLBAR (left_toolbar), 
			     gw->preview_button, NULL, NULL);


#ifdef HAS_IXJ
  /* Quicknet LID button speakerphone */
  gw->speaker_phone_button = gtk_toggle_button_new ();

  image = gtk_image_new_from_stock (GM_STOCK_SPEAKER_PHONE, 
                                    GTK_ICON_SIZE_MENU);

  gtk_container_add (GTK_CONTAINER (gw->speaker_phone_button), 
		     GTK_WIDGET (image));

  g_signal_connect (G_OBJECT (gw->speaker_phone_button), "clicked",
		    G_CALLBACK (speaker_phone_toggle_changed), 
		    NULL);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, gw->speaker_phone_button,
			_("Click here to activate the speaker phone device of your Quicknet card."), NULL);


  gtk_toolbar_append_widget (GTK_TOOLBAR (left_toolbar), 
			     gw->speaker_phone_button, NULL, NULL);
#endif


  /* Audio Channel Button */
  gw->audio_chan_button = gtk_toggle_button_new ();
 
  image = gtk_image_new_from_stock (GM_STOCK_AUDIO_MUTE, 
                                    GTK_ICON_SIZE_MENU);

  gtk_container_add (GTK_CONTAINER (gw->audio_chan_button), 
		     GTK_WIDGET (image));

  gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_chan_button), FALSE);

  g_signal_connect (G_OBJECT (gw->audio_chan_button), "clicked",
		    G_CALLBACK (pause_audio_callback), gw);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, gw->audio_chan_button,
			_("Audio Transmission Status. During a call, click here to pause the audio transmission."), NULL);

  gtk_toolbar_append_widget (GTK_TOOLBAR (left_toolbar), 
			     gw->audio_chan_button, NULL, NULL);


  /* Video Channel Button */
  gw->video_chan_button = gtk_toggle_button_new ();

  image = gtk_image_new_from_stock (GM_STOCK_QUICKCAM,
				    GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (gw->video_chan_button), 
		     GTK_WIDGET (image));

  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_chan_button), FALSE);

  g_signal_connect (G_OBJECT (gw->video_chan_button), "clicked",
		    G_CALLBACK (pause_video_callback), gw);

  tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tip, gw->video_chan_button,
			_("Video Transmission Status. During a call, click here to pause the video transmission."), NULL);

  gtk_toolbar_append_widget (GTK_TOOLBAR (left_toolbar), 
			     gw->video_chan_button, NULL, NULL);


  gtk_widget_show_all (GTK_WIDGET (hbox));

  gtk_widget_show (GTK_WIDGET (gw->combo));
  gtk_widget_show_all (GTK_WIDGET (gw->connect_button));
  gtk_widget_show_all (GTK_WIDGET (gw->preview_button));

#ifdef HAS_IXJ
  gtk_widget_show_all (GTK_WIDGET (gw->speaker_phone_button));
#endif

  gtk_widget_show_all (GTK_WIDGET (gw->audio_chan_button));
  gtk_widget_show_all (GTK_WIDGET (gw->video_chan_button));

  if (gconf_client_get_bool (client, "/apps/gnomemeeting/view/left_toolbar", 0)) 
    gtk_widget_show (GTK_WIDGET (gnome_app_get_dock_item_by_name(GNOME_APP (gm), "left_toolbar")));
  else
    gtk_widget_hide (GTK_WIDGET (gnome_app_get_dock_item_by_name(GNOME_APP (gm), "left_toolbar")));
}


void connect_button_update_pixmap (GtkToggleButton *button, int pressed)
{
  GtkWidget *image = NULL;
  
  GmWindow *gw = gnomemeeting_get_main_window (gm);
  
  image = (GtkWidget *) g_object_get_data (G_OBJECT (button), "image");
  
  if (image != NULL)	{
    
    if (pressed == 1) 
      {
        gtk_image_set_from_stock (GTK_IMAGE (image),
                                  GM_STOCK_CONNECT, 
                                  GTK_ICON_SIZE_LARGE_TOOLBAR);
        
        /* Block the signal */
        g_signal_handlers_block_by_func (G_OBJECT (button), 
                                         (void *) connect_button_clicked, 
                                         (gpointer) gw->connect_button);
        gtk_toggle_button_set_active (button, TRUE);
        g_signal_handlers_unblock_by_func (G_OBJECT (button), 
                                           (void *) connect_button_clicked, 
                                           (gpointer) gw->connect_button);
      } else {
        
        gtk_image_set_from_stock (GTK_IMAGE (image),
                                  GM_STOCK_DISCONNECT, 
                                  GTK_ICON_SIZE_LARGE_TOOLBAR);
        
        g_signal_handlers_block_by_func (G_OBJECT (button), 
                                         (void *) connect_button_clicked, 
                                         (gpointer) gw->connect_button);
        gtk_toggle_button_set_active (button, FALSE);
        g_signal_handlers_unblock_by_func (G_OBJECT (button), 
                                           (void *) connect_button_clicked, 
                                           (gpointer) gw->connect_button);
      }   
    
    gtk_widget_queue_draw (GTK_WIDGET (image));
  }
}
