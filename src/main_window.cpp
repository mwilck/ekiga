
/* GnomeMeeting -- A Video-Conferencing application
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
 * GnomeMeeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         main_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Mon Mar 26 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          build the main window.
 */


#include "../config.h"

#include "main_window.h"
#include "gnomemeeting.h"
#include "chat_window.h"
#include "config.h"
#include "misc.h"
#include "callbacks.h"
#include "tray.h"
#include "lid.h"
#include "sound_handling.h"
#include "urlhandler.h"

#include <dialog.h>
#include <gmentrydialog.h>
#include <stock-icons.h>
#include <gm_conf.h>
#include <contacts/gm_contacts.h>
#include <gtk_menu_extensions.h>
#include <stats_drawing_area.h>
#include <widgets/history-combo.h>


#include "../pixmaps/text_logo.xpm"


#ifndef DISABLE_GNOME
#include <libgnomeui/gnome-window-icon.h>
#include "bonobo_component.h"
#endif

#ifndef WIN32
#include <gdk/gdkx.h>
#endif

#if defined(P_FREEBSD) || defined (P_MACOSX)
#include <libintl.h>
#endif

#include <libxml/parser.h>


#define GM_MAIN_WINDOW(x) (GmWindow *) (x)


/* Declarations */
extern GtkWidget *gm;


/* GUI Functions */


/* DESCRIPTION  : / 
 * BEHAVIOR     : Frees a GmMainWindow and its content.
 * PRE          : A non-NULL pointer to a GmMainWindow.
 */
static void gm_mw_destroy (gpointer);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to the private GmMainWindow
 * 		  used by the main book GMObject.
 * PRE          : The given GtkWidget pointer must be the main window GMObject. 
 */
static GmWindow *gm_mw_get_mw (GtkWidget *);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Updates the connect button in toggled mode or not.
 * PRE          : The given GtkWidget pointer must be the main window GMObject,
 * 		  BOOL is TRUE if the button should be in calling state.
 */
static void gm_mw_update_connect_button (GtkWidget *,
					 BOOL);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Create the toolbars of the main window.
 *                 The toolbars are created in their initial state, with
 *                 required items being unsensitive.
 * PRE          :  The main window GMObject.
 */
static void gm_mw_init_toolbars (GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Create the menu of the main window.
 *                 The menu is created in its initial state, with
 *                 required items being unsensitive.
 * PRE          :  The main window GMObject.
 */
static void gm_mw_init_menu (GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the stats part of the main window.
 * PRE          : The given GtkWidget pointer must be the main window GMObject. 
 */
static void gm_mw_init_stats (GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the dialpad part of the main window.
 * PRE          : The given GtkWidget pointer must be the main window GMObject. 
 */
static void gm_mw_init_dialpad (GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the video settings part of the main window.
 * PRE          : The given GtkWidget pointer must be the main window GMObject. 
 */
static void gm_mw_init_video_settings (GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the audio settings part of the main window.
 * PRE          : The given GtkWidget pointer must be the main window GMObject. 
 */
static void gm_mw_init_audio_settings (GtkWidget *);


/* Callbacks */

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Set the current active call on hold and updates the GUI
 * 		   accordingly.
 * PRE          :  The main window GMObject as data.
 */
static void hold_current_call_cb (GtkWidget *,
				  gpointer);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Set the current active call audio or video channel on pause
 * 		   or not and update the GUI accordingly.
 * PRE          :  GPOINTER_TO_INT (data) = 0 if audio, 1 if video.
 */
static void pause_current_call_channel_cb (GtkWidget *,
					   gpointer);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a dialog to transfer the current call and transfer
 * 		   it if required.
 * PRE          :  The main window GMObject as data.
 */
static void transfer_current_call_cb (GtkWidget *,
				      gpointer);


/* DESCRIPTION  :  This callback is called when a video window is shown.
 * BEHAVIOR     :  Set the WM HINTS to stay-on-top if the config key is set
 *                 to true.
 * PRE          :  /
 */
static void video_window_shown_cb (GtkWidget *,
				   gpointer);


/* DESCRIPTION  :  This callback is called when the user changes the
 *                 audio settings sliders in the main notebook.
 * BEHAVIOR     :  Update the volume of the choosen mixers or of the lid.
 * PRE          :  The main window GMObject.
 */
static void audio_volume_changed_cb (GtkAdjustment *, 
				     gpointer);


/* DESCRIPTION  :  This callback is called when the user changes one of the 
 *                 video settings sliders in the main notebook.
 * BEHAVIOR     :  Updates the value in real time.
 * PRE          :  gpointer is a valid pointer to the main window GmObject.
 */
static void video_settings_changed_cb (GtkAdjustment *, 
				       gpointer);


/* DESCRIPTION  :  This callback is called when the user drops a contact.
 * BEHAVIOR     :  Calls the user corresponding to the contact.
 * PRE          :  Assumes data hides a GmWindow*
 */
static void dnd_call_contact_cb (GtkWidget *widget, 
				 GmContact *contact,
				 gint x, 
				 gint y, 
				 gpointer data);


/* DESCRIPTION  :  This callback is called when the user changes the
 *                 page in the main notebook.
 * BEHAVIOR     :  Update the config key accordingly.
 * PRE          :  A valid pointer to the main window GmObject.
 */
static void control_panel_section_changed_cb (GtkNotebook *, 
					      GtkNotebookPage *,
					      gint, 
					      gpointer);


/* DESCRIPTION  :  This callback is called when the user 
 *                 clicks on the dialpad button.
 * BEHAVIOR     :  Generates a dialpad event.
 * PRE          :  A valid pointer to the main window GMObject.
 */
static void dialpad_button_clicked_cb (GtkButton *, 
				       gpointer);


/* DESCRIPTION  :  This callback is called when the user tries to close
 *                 the application using the window manager.
 * BEHAVIOR     :  Calls the real callback if the notification icon is 
 *                 not shown else hide GM.
 * PRE          :  A valid pointer to the main window GMObject.
 */
static gint window_closed_cb (GtkWidget *, 
			      GdkEvent *, 
			      gpointer);


/* DESCRIPTION  :  This callback is called when the user clicks on the
 *                 clear text chat menu entry
 * BEHAVIOR     :  clears text chat
 * PRE          :  The main window GMObject.
 */
static void text_chat_clear_cb (GtkWidget *,
				gpointer);


/* DESCRIPTION  :  This callback is called when the user changes the zoom
 *                 factor in the menu.
 * BEHAVIOR     :  Sets zoom to 1:2 if data == 0, 1:1 if data == 1, 
 *                 2:1 if data == 2. (Updates the config key).
 * PRE          :  /
 */
static void zoom_changed_cb (GtkWidget *,
			     gpointer);


/* DESCRIPTION  :  This callback is called when the user toggles fullscreen
 *                 factor in the popup menu.
 * BEHAVIOR     :  Toggles the fullscreen configuration key. 
 * PRE          :  / 
 */
static void fullscreen_changed_cb (GtkWidget *,
				   gpointer);


/* DESCRIPTION  :  This callback is called when the user toggles an 
 *                 item in the speed dials menu.
 * BEHAVIOR     :  Calls the given speed dial.
 * PRE          :  data is the speed dial as a gchar *
 */
static void speed_dial_menu_item_selected_cb (GtkWidget *,
					      gpointer);


/* DESCRIPTION  :  This callback is called when the user changes the URL
 * 		   in the URL bar.
 * BEHAVIOR     :  It udpates the tooltip with the new URL.
 * PRE          :  data is a valid pointer to the main window GMObject.
 */
static void combo_url_changed_cb (GtkEditable *, 
				  gpointer);

/* DESCRIPTION  :  This callback is called when the user presses the control 
 *                 panel button in the toolbar. 
 * BEHAVIOR     :  Updates the config cache : 0 or 3 (off) for the control
 * 		   panel section.
 * PRE          :  /
 */
static void control_panel_button_clicked_cb (GtkWidget *, 
					     gpointer);


/* DESCRIPTION  :  This callback is called when the user presses a
 *                 button in the toolbar. 
 *                 (See menu_toggle_changed)
 * BEHAVIOR     :  Updates the config cache.
 * PRE          :  data is the key.
 */
static void toolbar_toggle_button_changed_cb (GtkWidget *, 
					      gpointer);


/* DESCRIPTION  :  This callback is called when the user toggles the 
 *                 connect button.
 * BEHAVIOR     :  Connect or disconnect.
 * PRE          :  /
 */
static void toolbar_connect_button_clicked_cb (GtkToggleButton *, 
					       gpointer);



/* Implementation */
static void
gm_mw_destroy (gpointer mw)
{
  g_return_if_fail (mw != NULL);

  delete ((GmWindow *) mw);
}


static GmWindow *
gm_mw_get_mw (GtkWidget *main_window)
{
  g_return_val_if_fail (main_window != NULL, NULL);

  return GM_MAIN_WINDOW (g_object_get_data (G_OBJECT (main_window), 
					    "GMObject"));
}


static void 
gm_mw_update_connect_button (GtkWidget *main_window,
			     BOOL is_calling)
{
  GmWindow *mw = NULL;
  
  GtkWidget *image = NULL;
  

  g_return_if_fail (main_window != NULL);
  
  mw = gm_mw_get_mw (main_window);
  g_return_if_fail (mw != NULL);
  
  
  image = (GtkWidget *) g_object_get_data (G_OBJECT (mw->connect_button), 
					   "image");
  
  if (image != NULL) {
    
    if (is_calling == 1) {
      
        gtk_image_set_from_stock (GTK_IMAGE (image),
                                  GM_STOCK_CONNECT, 
                                  GTK_ICON_SIZE_LARGE_TOOLBAR);
        
        /* Block the signal */
        g_signal_handlers_block_by_func (G_OBJECT (mw->connect_button), (void *) toolbar_connect_button_clicked_cb, NULL);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mw->connect_button), 
				      TRUE);
        g_signal_handlers_unblock_by_func (G_OBJECT (mw->connect_button), (void *) toolbar_connect_button_clicked_cb, NULL);
	
      } else {
        
        gtk_image_set_from_stock (GTK_IMAGE (image),
                                  GM_STOCK_DISCONNECT, 
                                  GTK_ICON_SIZE_LARGE_TOOLBAR);
        
        g_signal_handlers_block_by_func (G_OBJECT (mw->connect_button), (void *) toolbar_connect_button_clicked_cb, NULL);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mw->connect_button), 
				      FALSE);
        g_signal_handlers_unblock_by_func (G_OBJECT (mw->connect_button), (void *) toolbar_connect_button_clicked_cb, NULL);
      }   
    
    gtk_widget_queue_draw (GTK_WIDGET (image));
    gtk_widget_queue_draw (GTK_WIDGET (mw->connect_button));
  }
}


static void
gm_mw_init_toolbars (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  
  GtkWidget *toolbar = NULL;
  
  GtkWidget *hbox = NULL;
  GtkWidget *image = NULL;

  GtkWidget *addressbook_window = NULL;
  
  addressbook_window = GnomeMeeting::Process ()->GetAddressbookWindow ();

  
  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  
  /* The main horizontal toolbar */
  toolbar = gtk_toolbar_new ();


  /* Combo */
  mw->combo =
    gm_history_combo_new (USER_INTERFACE_KEY "main_window/urls_history");

  gtk_combo_set_use_arrows_always (GTK_COMBO(mw->combo), TRUE);
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (mw->combo)->entry), "h323:");

  gtk_tooltips_set_tip (mw->tips, GTK_WIDGET (GTK_COMBO (mw->combo)->entry), 
			"h323:", NULL);

  gtk_combo_disable_activate (GTK_COMBO (mw->combo));
  g_signal_connect (G_OBJECT (GTK_COMBO (mw->combo)->entry), "activate",
  		    G_CALLBACK (connect_cb), NULL);

  hbox = gtk_hbox_new (FALSE, 2);

  gtk_box_pack_start (GTK_BOX (hbox), mw->combo, TRUE, TRUE, 1);
  gtk_box_pack_start (GTK_BOX (hbox), toolbar, FALSE, FALSE, 1);
 
  gtk_container_set_border_width (GTK_CONTAINER (toolbar), 2);

  g_signal_connect (G_OBJECT (GTK_WIDGET (GTK_COMBO(mw->combo)->entry)),
		    "changed", G_CALLBACK (combo_url_changed_cb), 
		    (gpointer) main_window);


  /* The connect button */
  mw->connect_button = gtk_toggle_button_new ();
  gtk_tooltips_set_tip (mw->tips, GTK_WIDGET (mw->connect_button), 
			_("Enter an URL to call on the left, and click on this button to connect to the given URL"), NULL);
  
  image = gtk_image_new_from_stock (GM_STOCK_DISCONNECT, 
                                    GTK_ICON_SIZE_LARGE_TOOLBAR);

  gtk_container_add (GTK_CONTAINER (mw->connect_button), GTK_WIDGET (image));
  g_object_set_data (G_OBJECT (mw->connect_button), "image", image);

  gtk_widget_set_size_request (GTK_WIDGET (mw->connect_button), 28, 28);

  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), mw->connect_button,
			     NULL, NULL);

  g_signal_connect (G_OBJECT (mw->connect_button), "clicked",
                    G_CALLBACK (toolbar_connect_button_clicked_cb), 
		    NULL);

  gtk_widget_show_all (GTK_WIDGET (hbox));
  
#ifndef DISABLE_GNOME
  gnome_app_add_docked (GNOME_APP (main_window), hbox, "main_toolbar",
  			BONOBO_DOCK_ITEM_BEH_EXCLUSIVE,
  			BONOBO_DOCK_TOP, 1, 0, 0);
#else
  gtk_box_pack_start (GTK_BOX (mw->window_vbox), hbox, 
		      FALSE, FALSE, 0);
#endif


  /* The left toolbar */
  toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_orientation (GTK_TOOLBAR (toolbar), 
			       GTK_ORIENTATION_VERTICAL);

  image =
    gtk_image_new_from_stock (GM_STOCK_TEXT_CHAT, 
			      GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show (image);
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
			   NULL,
			   _("Open text chat"), 
			   NULL,
			   image,
			   GTK_SIGNAL_FUNC (toolbar_toggle_button_changed_cb),
			   (gpointer) USER_INTERFACE_KEY "main_window/show_chat_window");
  
  image = gtk_image_new_from_stock (GM_STOCK_CONTROL_PANEL, 
				    GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show (image);
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
			   NULL,
			   _("Open control panel"),
			   NULL,
			   image,
			   GTK_SIGNAL_FUNC (control_panel_button_clicked_cb),
			   NULL);

  
  image = gtk_image_new_from_stock (GM_STOCK_ADDRESSBOOK_24,
				    GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show (image);
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
			   NULL,
			   _("Open address book"),
			   NULL,
			   image,
			   GTK_SIGNAL_FUNC (show_window_cb),
			   (gpointer) addressbook_window); 

  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);

  
  /* Video Preview Button */
  mw->preview_button = gtk_toggle_button_new ();

  image = gtk_image_new_from_stock (GM_STOCK_VIDEO_PREVIEW, 
                                    GTK_ICON_SIZE_MENU);

  gtk_container_add (GTK_CONTAINER (mw->preview_button), GTK_WIDGET (image));
  GTK_TOGGLE_BUTTON (mw->preview_button)->active =
    gm_conf_get_bool (VIDEO_DEVICES_KEY "enable_preview");

  g_signal_connect (G_OBJECT (mw->preview_button), "toggled",
		    G_CALLBACK (toolbar_toggle_button_changed_cb),
		    (gpointer) VIDEO_DEVICES_KEY "enable_preview");

  gtk_tooltips_set_tip (mw->tips, mw->preview_button,
			_("Display images from your camera device"),
			NULL);

  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), 
			     mw->preview_button, NULL, NULL);


  /* Audio Channel Button */
  mw->audio_chan_button = gtk_toggle_button_new ();
 
  image = gtk_image_new_from_stock (GM_STOCK_AUDIO_MUTE, 
                                    GTK_ICON_SIZE_MENU);

  gtk_container_add (GTK_CONTAINER (mw->audio_chan_button), 
		     GTK_WIDGET (image));

  gtk_widget_set_sensitive (GTK_WIDGET (mw->audio_chan_button), FALSE);

  g_signal_connect (G_OBJECT (mw->audio_chan_button), "clicked",
		    G_CALLBACK (pause_current_call_channel_cb), 
		    GINT_TO_POINTER (0));

  gtk_tooltips_set_tip (mw->tips, mw->audio_chan_button,
			_("Audio transmission status. During a call, click here to suspend or resume the audio transmission."), NULL);

  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), 
			     mw->audio_chan_button, NULL, NULL);


  /* Video Channel Button */
  mw->video_chan_button = gtk_toggle_button_new ();

  image = gtk_image_new_from_stock (GM_STOCK_VIDEO_MUTE,
				    GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (mw->video_chan_button), 
		     GTK_WIDGET (image));

  gtk_widget_set_sensitive (GTK_WIDGET (mw->video_chan_button), FALSE);

  g_signal_connect (G_OBJECT (mw->video_chan_button), "clicked",
		    G_CALLBACK (pause_current_call_channel_cb), 
		    GINT_TO_POINTER (1));

  gtk_tooltips_set_tip (mw->tips, mw->video_chan_button,
			_("Video transmission status. During a call, click here to suspend or resume the video transmission."), NULL);

  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), 
			     mw->video_chan_button, NULL, NULL);

#ifndef DISABLE_GNOME
  gnome_app_add_toolbar (GNOME_APP (main_window), GTK_TOOLBAR (toolbar),
 			 "left_toolbar", BONOBO_DOCK_ITEM_BEH_EXCLUSIVE,
 			 BONOBO_DOCK_LEFT, 2, 0, 0);
#else
  gtk_box_pack_start (GTK_BOX (mw->window_hbox), left_toolbar, 
		      FALSE, FALSE, 0);
#endif
  
  gtk_widget_show_all (GTK_WIDGET (toolbar));
}

	
static void
gm_mw_init_menu (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  
  GtkWidget *addressbook_window = NULL;
  GtkWidget *chat_window = NULL;
  GtkWidget *druid_window = NULL;
  GtkWidget *calls_history_window = NULL;
  GtkWidget *history_window = NULL;
  GtkWidget *prefs_window = NULL;
  
  IncomingCallMode icm = AVAILABLE;
  ControlPanelSection cps = CLOSED;
  bool show_status_bar = false;
  bool show_chat_window = false;

  GSList *glist = NULL;

  g_return_if_fail (main_window != NULL);
  mw = gm_mw_get_mw (main_window);
  
  addressbook_window = GnomeMeeting::Process ()->GetAddressbookWindow ();
  calls_history_window = GnomeMeeting::Process ()->GetCallsHistoryWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  druid_window = GnomeMeeting::Process ()->GetDruidWindow ();
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();

  mw->main_menu = gtk_menu_bar_new ();


  /* Default values */
  icm = (IncomingCallMode) 
    gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode"); 
  cps = (ControlPanelSection)
    gm_conf_get_int (USER_INTERFACE_KEY "main_window/control_panel_section"); 
  show_status_bar =
    gm_conf_get_bool (USER_INTERFACE_KEY "main_window/show_status_bar"); 
  show_chat_window =
    gm_conf_get_bool (USER_INTERFACE_KEY "main_window/show_chat_window"); 

  
  static MenuEntry gnomemeeting_menu [] =
    {
      GTK_MENU_NEW (_("C_all")),

      GTK_MENU_ENTRY("connect", _("C_onnect"), _("Create a new connection"), 
		     GM_STOCK_CONNECT_16, 'o',
		     GTK_SIGNAL_FUNC (connect_cb), NULL, TRUE),
      GTK_MENU_ENTRY("disconnect", _("_Disconnect"),
		     _("Close the current connection"), 
		     GM_STOCK_DISCONNECT_16, 'd',
		     GTK_SIGNAL_FUNC (disconnect_cb), NULL, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_RADIO_ENTRY("available", _("_Available"),
			   _("Display a popup to accept the call"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == AVAILABLE), TRUE),
      GTK_MENU_RADIO_ENTRY("free_for_chat", _("Free for Cha_t"),
			   _("Auto answer calls"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == FREE_FOR_CHAT), TRUE),
      GTK_MENU_RADIO_ENTRY("busy", _("_Busy"), _("Reject calls"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == BUSY), TRUE),
      GTK_MENU_RADIO_ENTRY("forward", _("_Forward"), _("Forward calls"),
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == FORWARD), TRUE),

      GTK_MENU_SEPARATOR,

      GTK_SUBMENU_NEW("speed_dials", _("Speed dials")),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("hold_call", _("_Hold Call"), _("Hold the current call"),
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (hold_current_call_cb), main_window, 
		     FALSE),
      GTK_MENU_ENTRY("transfer_call", _("_Transfer Call"),
		     _("Transfer the current call"),
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (transfer_current_call_cb), main_window, 
		     FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("suspend_audio", _("Suspend _Audio"),
		     _("Suspend or resume the audio transmission"),
		     NULL, 0,
		     GTK_SIGNAL_FUNC (pause_current_call_channel_cb),
		     GINT_TO_POINTER (0), FALSE),
      GTK_MENU_ENTRY("suspend_video", _("Suspend _Video"),
		     _("Suspend or resume the video transmission"),
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (pause_current_call_channel_cb),
		     GINT_TO_POINTER (1), FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("save_picture", _("_Save Current Picture"), 
		     _("Save a snapshot of the current video"),
		     GTK_STOCK_SAVE, 'S',
		     GTK_SIGNAL_FUNC (save_callback), NULL, FALSE),

      GTK_MENU_SEPARATOR,
      
      GTK_MENU_ENTRY("close", _("_Close"), _("Close the GnomeMeeting window"),
		     GTK_STOCK_CLOSE, 'W', 
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) gm, TRUE),

      GTK_MENU_SEPARATOR,
      
      GTK_MENU_ENTRY("quit", _("_Quit"), _("Quit GnomeMeeting"),
		     GTK_STOCK_QUIT, 'Q', 
		     GTK_SIGNAL_FUNC (quit_callback), NULL, TRUE),

      GTK_MENU_NEW (_("_Edit")),

      GTK_MENU_ENTRY("configuration_druid", _("Configuration Druid"),
		     _("Run the configuration druid"),
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) druid_window, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("preferences", _("_Preferences"),
		     _("Change your preferences"), 
		     GTK_STOCK_PREFERENCES, 'P',
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) prefs_window, TRUE),

      GTK_MENU_NEW(_("_View")),

      GTK_MENU_TOGGLE_ENTRY("text_chat", _("Text Chat"),
			    _("View/Hide the text chat window"), 
			    NULL, 0,
			    GTK_SIGNAL_FUNC (toggle_menu_changed_cb),
			    (gpointer) USER_INTERFACE_KEY "main_window/show_chat_window",
			    show_chat_window, TRUE),
      GTK_MENU_TOGGLE_ENTRY("status_bar", _("Status Bar"),
			    _("View/Hide the status bar"), 
			    NULL, 0, 
			    GTK_SIGNAL_FUNC (toggle_menu_changed_cb),
			    (gpointer) USER_INTERFACE_KEY "main_window/show_status_bar",
			    show_status_bar, TRUE),

      GTK_SUBMENU_NEW("control_panel", _("Control Panel")),

      GTK_MENU_RADIO_ENTRY("statistics", _("Statistics"), 
			   _("View audio/video transmission and reception statistics"),
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb), 
			   (gpointer) USER_INTERFACE_KEY "main_window/control_panel_section",
			   (cps == 0), TRUE),
      GTK_MENU_RADIO_ENTRY("dialpad", _("_Dialpad"), _("View the dialpad"),
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb), 
			   (gpointer) USER_INTERFACE_KEY "main_window/control_panel_section",
			   (cps == 1), TRUE),
      GTK_MENU_RADIO_ENTRY("audio_settings", _("_Audio Settings"),
			   _("View audio settings"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb), 
			   (gpointer) USER_INTERFACE_KEY "main_window/control_panel_section",
			   (cps == 2), TRUE),
      GTK_MENU_RADIO_ENTRY("video_settings", _("_Video Settings"),
			   _("View video settings"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb), 
			   (gpointer) USER_INTERFACE_KEY "main_window/control_panel_section",
			   (cps == 3), TRUE),
      GTK_MENU_RADIO_ENTRY("off", _("Off"), _("Hide the control panel"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb), 
			   (gpointer) USER_INTERFACE_KEY "main_window/control_panel_section",
			   (cps == 4), TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("clear_text_chat", _("_Clear Text Chat"),
		     _("Clear the text chat"), 
		     GTK_STOCK_CLEAR, 'L',
		     GTK_SIGNAL_FUNC (text_chat_clear_cb),
		     (gpointer) chat_window, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_RADIO_ENTRY("local_video", _("Local Video"),
			   _("Local video image"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   TRUE, FALSE),
      GTK_MENU_RADIO_ENTRY("remote_video", _("Remote Video"),
			   _("Remote video image"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb), 
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   FALSE, FALSE),
      GTK_MENU_RADIO_ENTRY("both_incrusted", _("Both (Local Video Incrusted)"),
			   _("Both video images"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb), 
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   FALSE, FALSE),
      GTK_MENU_RADIO_ENTRY("both_new_window",
			   _("Both (Local Video in New Window)"),
			   _("Both video images"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb), 
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   FALSE, FALSE),
      GTK_MENU_RADIO_ENTRY("both_new_windows",
			   _("Both (Both in New Windows)"), 
			   _("Both video images"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb), 
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   FALSE, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("zoom_in", _("Zoom In"), _("Zoom in"), 
		     GTK_STOCK_ZOOM_IN, '+', 
		     GTK_SIGNAL_FUNC (zoom_changed_cb),
		     GINT_TO_POINTER (2), FALSE),
      GTK_MENU_ENTRY("zoom_out", _("Zoom Out"), _("Zoom out"), 
		     GTK_STOCK_ZOOM_OUT, '-', 
		     GTK_SIGNAL_FUNC (zoom_changed_cb),
		     GINT_TO_POINTER (0), FALSE),
      GTK_MENU_ENTRY("normal_size", _("Normal Size"), _("Normal size"), 
		     GTK_STOCK_ZOOM_100, '=',
		     GTK_SIGNAL_FUNC (zoom_changed_cb),
		     GINT_TO_POINTER (1), FALSE),

      GTK_MENU_ENTRY("fullscreen", _("Fullscreen"), _("Switch to fullscreen"), 
		     GTK_STOCK_ZOOM_IN, 'f', 
		     GTK_SIGNAL_FUNC (fullscreen_changed_cb),
		     NULL, FALSE),

      GTK_MENU_NEW(_("_Tools")),
      
      GTK_MENU_ENTRY("address_book", _("Address _Book"),
		     _("Open the address book"),
		     GM_STOCK_ADDRESSBOOK_16, 0,
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) addressbook_window, TRUE),
      
      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("log", _("General History"),
		     _("View the operations history"),
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) history_window, TRUE),
      GTK_MENU_ENTRY("calls_history", _("Calls History"),
		     _("View the calls history"),
		     GM_STOCK_CALLS_HISTORY, 'h',
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) calls_history_window, TRUE),

      GTK_MENU_SEPARATOR,

#ifndef DISABLE_GNOME      
      GTK_MENU_ENTRY("pc-to-phone", _("PC-To-Phone Account"),
		     _("Manage your PC-To-Phone account"),
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) mw->pc_to_phone_window, TRUE),
#else
      GTK_MENU_ENTRY("pc-to-phone", _("PC-To-Phone Account"),
		     _("Manage your PC-To-Phone account"),
		     NULL, 0, 
                     NULL, NULL, FALSE),
#endif
      
      GTK_MENU_NEW(_("_Help")),

#ifndef DISABLE_GNOME
       GTK_MENU_ENTRY("help", _("_Contents"),
                     _("Get help by reading the GnomeMeeting manual"),
                     GTK_STOCK_HELP, GDK_F1, 
                     GTK_SIGNAL_FUNC (help_cb), NULL, TRUE),
#else
       GTK_MENU_ENTRY("help", _("_Contents"),
                     _("Get help by reading the GnomeMeeting manual"),
                     GTK_STOCK_HELP, GDK_F1, 
                     NULL, NULL, FALSE),
#endif
       
      GTK_MENU_ENTRY("about", _("_About"),
		     _("View information about GnomeMeeting"),
		     NULL, 'a', 
		     GTK_SIGNAL_FUNC (about_callback), (gpointer) gm,
		     TRUE),

      GTK_MENU_END
    };


  gtk_build_menu (mw->main_menu, 
		  gnomemeeting_menu, 
		  mw->accel, 
		  mw->statusbar);

  glist = 
    gnomemeeting_addressbook_get_contacts (NULL, FALSE, NULL, NULL, NULL, "*"); 
  gm_main_window_speed_dials_menu_update (main_window, glist);
  g_slist_foreach (glist, (GFunc) gm_contact_delete, NULL);
  g_slist_free (glist);

  gtk_widget_show_all (GTK_WIDGET (mw->main_menu));
}


static void 
gm_mw_init_stats (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  
  GtkWidget *frame2 = NULL;
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;

  
  g_return_if_fail (main_window != NULL);
  mw = gm_mw_get_mw (main_window);

  
  /* The first frame with statistics display */
  frame2 = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_IN);

  vbox = gtk_vbox_new (FALSE, 6);
  mw->stats_drawing_area = stats_drawing_area_new ();

  gtk_box_pack_start (GTK_BOX (vbox), frame2, FALSE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (frame2), mw->stats_drawing_area);

  gtk_widget_set_size_request (GTK_WIDGET (frame2), GM_QCIF_WIDTH, 47);

  gtk_widget_queue_draw (mw->stats_drawing_area);


  /* The second one with some labels */
  mw->stats_label =
    gtk_label_new (_("Lost packets:\nLate packets:\nRound-trip delay:\nJitter buffer:"));
  gtk_misc_set_alignment (GTK_MISC (mw->stats_label), 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), mw->stats_label, FALSE, TRUE,
		      GNOMEMEETING_PAD_SMALL);
  
  label = gtk_label_new (_("Statistics"));

  gtk_notebook_append_page (GTK_NOTEBOOK (mw->main_notebook), vbox, label);
}


static void 
gm_mw_init_dialpad (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  
  GtkWidget *label = NULL;
  GtkWidget *table = NULL;
  GtkWidget *button = NULL;

  int i = 0;

  char *key_n [] = { "1", "2", "3", "4", "5", "6", "7", "8", "9",
		     "*", "0", "#"};
  char *key_a []= { "  ", "abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv",
		   "wxyz", "  ", "  ", "  "};

  gchar *text_label = NULL;
  

  g_return_if_fail (main_window != NULL);
  mw = gm_mw_get_mw (main_window);

  table = gtk_table_new (4, 3, TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 6);
  
  for (i = 0 ; i < 12 ; i++) {

    label = gtk_label_new (NULL);
    text_label =
      g_strdup_printf ("%s<sub><span size=\"small\">%s</span></sub>",
		       key_n [i], key_a [i]);
    gtk_label_set_markup (GTK_LABEL (label), text_label); 
    button = gtk_button_new ();
    gtk_container_set_border_width (GTK_CONTAINER (button), 0);
    gtk_container_add (GTK_CONTAINER (button), label);
    
    gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (button), 
		      i%3, i%3+1, i/3, i/3+1,
		      (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL),
		      1, 1);
    
    g_signal_connect (G_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialpad_button_clicked_cb), 
		      main_window);

    g_free (text_label);
  }
  
  label = gtk_label_new (_("Dialpad"));

  gtk_notebook_append_page (GTK_NOTEBOOK (mw->main_notebook),
			    table, label);
}


static void 
gm_mw_init_video_settings (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  
  GtkWidget *label = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *image = NULL;

  GtkWidget *hscale_brightness = NULL;
  GtkWidget *hscale_colour = NULL;
  GtkWidget *hscale_contrast = NULL;
  GtkWidget *hscale_whiteness = NULL;

  int brightness = 0, colour = 0, contrast = 0, whiteness = 0;
  

  g_return_if_fail (main_window != NULL);
  mw = gm_mw_get_mw (main_window);

  
  /* Webcam Control Frame, we need it to disable controls */		
  mw->video_settings_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (mw->video_settings_frame), 
			     GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (mw->video_settings_frame), 0);
  
  /* Category */
  vbox = gtk_vbox_new (0, FALSE);
  gtk_container_add (GTK_CONTAINER (mw->video_settings_frame), vbox);

  
  /* Brightness */
  hbox = gtk_hbox_new (0, FALSE);
  image = gtk_image_new_from_stock (GM_STOCK_BRIGHTNESS, GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  mw->adj_brightness = gtk_adjustment_new (brightness, 0.0, 
					   255.0, 1.0, 5.0, 1.0);
  hscale_brightness = gtk_hscale_new (GTK_ADJUSTMENT (mw->adj_brightness));
  gtk_range_set_update_policy (GTK_RANGE (hscale_brightness),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_brightness), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_brightness), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_brightness, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  gtk_tooltips_set_tip (mw->tips, hscale_brightness,
			_("Adjust brightness"), NULL);

  g_signal_connect (G_OBJECT (mw->adj_brightness), "value-changed",
		    G_CALLBACK (video_settings_changed_cb), 
		    (gpointer) gm);


  /* Whiteness */
  hbox = gtk_hbox_new (0, FALSE);
  image = gtk_image_new_from_stock (GM_STOCK_WHITENESS, GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  mw->adj_whiteness = gtk_adjustment_new (whiteness, 0.0, 
					  255.0, 1.0, 5.0, 1.0);
  hscale_whiteness = gtk_hscale_new (GTK_ADJUSTMENT (mw->adj_whiteness));
  gtk_range_set_update_policy (GTK_RANGE (hscale_whiteness),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_whiteness), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_whiteness), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_whiteness, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  gtk_tooltips_set_tip (mw->tips, hscale_whiteness,
			_("Adjust whiteness"), NULL);

  g_signal_connect (G_OBJECT (mw->adj_whiteness), "value-changed",
		    G_CALLBACK (video_settings_changed_cb), 
		    (gpointer) gm);


  /* Colour */
  hbox = gtk_hbox_new (0, FALSE);
  image = gtk_image_new_from_stock (GM_STOCK_COLOURNESS, GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  mw->adj_colour = gtk_adjustment_new (colour, 0.0, 
				       255.0, 1.0, 5.0, 1.0);
  hscale_colour = gtk_hscale_new (GTK_ADJUSTMENT (mw->adj_colour));
  gtk_range_set_update_policy (GTK_RANGE (hscale_colour),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_colour), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_colour), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_colour, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  gtk_tooltips_set_tip (mw->tips, hscale_colour,
			_("Adjust color"), NULL);

  g_signal_connect (G_OBJECT (mw->adj_colour), "value-changed",
		    G_CALLBACK (video_settings_changed_cb), 
		    (gpointer) gm);


  /* Contrast */
  hbox = gtk_hbox_new (0, FALSE);
  image = gtk_image_new_from_stock (GM_STOCK_CONTRAST, GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
  
  mw->adj_contrast = gtk_adjustment_new (contrast, 0.0, 
					 255.0, 1.0, 5.0, 1.0);
  hscale_contrast = gtk_hscale_new (GTK_ADJUSTMENT (mw->adj_contrast));
  gtk_range_set_update_policy (GTK_RANGE (hscale_contrast),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_contrast), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_contrast), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_contrast, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  gtk_tooltips_set_tip (mw->tips, hscale_contrast,
			_("Adjust contrast"), NULL);

  g_signal_connect (G_OBJECT (mw->adj_contrast), "value-changed",
		    G_CALLBACK (video_settings_changed_cb), (gpointer) gm);
  

  gtk_widget_set_sensitive (GTK_WIDGET (mw->video_settings_frame), FALSE);

  label = gtk_label_new (_("Video"));  

  gtk_notebook_append_page (GTK_NOTEBOOK(mw->main_notebook), 
			    mw->video_settings_frame, label);
}



static void 
gm_mw_init_audio_settings (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  
  GtkWidget *hscale_play = NULL; 
  GtkWidget *hscale_rec = NULL;
  GtkWidget *label = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  

  /* Get the data from the GMObject */
  mw = gm_mw_get_mw (main_window);
  

  /* Audio control frame, we need it to disable controls */		
  mw->audio_volume_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (mw->audio_volume_frame), 
			     GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (mw->audio_volume_frame), 0);


  /* The vbox */
  vbox = gtk_vbox_new (0, FALSE);
  gtk_container_add (GTK_CONTAINER (mw->audio_volume_frame), vbox);
  gtk_widget_set_sensitive (GTK_WIDGET (mw->audio_volume_frame), FALSE);
  

  /* Output volume */
  hbox = gtk_hbox_new (0, FALSE);
  gtk_box_pack_start (GTK_BOX (hbox),
		      gtk_image_new_from_stock (GM_STOCK_VOLUME, 
						GTK_ICON_SIZE_SMALL_TOOLBAR),
		      FALSE, FALSE, 0);
  
  mw->adj_output_volume = gtk_adjustment_new (0, 0.0, 100.0, 1.0, 5.0, 1.0);
  hscale_play = gtk_hscale_new (GTK_ADJUSTMENT (mw->adj_output_volume));
  gtk_range_set_update_policy (GTK_RANGE (hscale_play),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_play), GTK_POS_RIGHT); 
  gtk_scale_set_draw_value (GTK_SCALE (hscale_play), FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_play, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);


  /* Input volume */
  hbox = gtk_hbox_new (0, FALSE);
  gtk_box_pack_start (GTK_BOX (hbox),
		      gtk_image_new_from_stock (GM_STOCK_MICROPHONE, 
						GTK_ICON_SIZE_SMALL_TOOLBAR),
		      FALSE, FALSE, 0);

  mw->adj_input_volume = gtk_adjustment_new (0, 0.0, 100.0, 1.0, 5.0, 1.0);
  hscale_rec = gtk_hscale_new (GTK_ADJUSTMENT (mw->adj_input_volume));
  gtk_range_set_update_policy (GTK_RANGE (hscale_rec),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_rec), GTK_POS_RIGHT); 
  gtk_scale_set_draw_value (GTK_SCALE (hscale_rec), FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_rec, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);


  g_signal_connect (G_OBJECT (mw->adj_output_volume), "value-changed",
		    G_CALLBACK (audio_volume_changed_cb), main_window);

  g_signal_connect (G_OBJECT (mw->adj_input_volume), "value-changed",
		    G_CALLBACK (audio_volume_changed_cb), main_window);

		    
  label = gtk_label_new (_("Audio"));

  gtk_notebook_append_page (GTK_NOTEBOOK (mw->main_notebook),
			    mw->audio_volume_frame, label);
}


/* The functions */
void 
gm_main_window_update_logo (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  
  GdkPixbuf *tmp = NULL;
  GdkPixbuf *text_logo_pix = NULL;
  GtkRequisition size_request;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);
  

  gtk_widget_size_request (GTK_WIDGET (mw->video_frame), &size_request);

  if ((size_request.width != GM_QCIF_WIDTH) || 
      (size_request.height != GM_QCIF_HEIGHT)) {

     gtk_widget_set_size_request (GTK_WIDGET (mw->video_frame),
				  176, 144);
  }

  text_logo_pix = gdk_pixbuf_new_from_xpm_data ((const char **) text_logo_xpm);
  tmp = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, 176, 144);
  gdk_pixbuf_fill (tmp, 0x000000FF);  /* Opaque black */

  gdk_pixbuf_copy_area (text_logo_pix, 0, 0, 176, 60, 
			tmp, 0, 42);
  gtk_image_set_from_pixbuf (GTK_IMAGE (mw->main_video_image),
			     GDK_PIXBUF (tmp));

  g_object_unref (text_logo_pix);
  g_object_unref (tmp);
}


void 
gm_main_window_dialpad_event (GtkWidget *main_window,
			      const char d)
{
  GmWindow *mw = NULL;
  
  GMH323EndPoint *endpoint = NULL;
  H323Connection *connection = NULL;

#ifdef HAS_IXJ
  GMLid *lid = NULL;
#endif
  
  PString url;
  PString new_url;

  char dtmf = d;
  gchar *msg = NULL;
  
  
  g_return_if_fail (main_window != NULL);
  mw = gm_mw_get_mw (main_window);
  
  endpoint = GnomeMeeting::Process ()->Endpoint ();

  if (mw->transfer_call_popup)
    url = gm_entry_dialog_get_text (GM_ENTRY_DIALOG (mw->transfer_call_popup));
  else
    url = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (mw->combo)->entry)); 

  
  if (endpoint->GetCallingState () == GMH323EndPoint::Standby) {

    /* Replace the * by a . */
    if (dtmf == '*') 
      dtmf = '.';
  }
      
  new_url = PString (url) + dtmf;

  if (mw->transfer_call_popup)
    gm_entry_dialog_set_text (GM_ENTRY_DIALOG (mw->transfer_call_popup),
			      new_url);
  else if (endpoint->GetCallingState () == GMH323EndPoint::Standby)
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (mw->combo)->entry), new_url);

  if (dtmf == '#' && mw->transfer_call_popup)
    gtk_dialog_response (GTK_DIALOG (mw->transfer_call_popup),
			 GTK_RESPONSE_ACCEPT);
  
  if (endpoint->GetCallingState () == GMH323EndPoint::Connected
      && !mw->transfer_call_popup) {

    gdk_threads_leave ();
    connection = 
      endpoint->FindConnectionWithLock (endpoint->GetCurrentCallToken ());
            
    if (connection) {

      msg = g_strdup_printf (_("Sent dtmf %c"), dtmf);
      
      connection->SendUserInput (dtmf);
      connection->Unlock ();
    }
    gdk_threads_enter ();

    if (msg) {

      gnomemeeting_statusbar_flash (mw->statusbar, msg);
      g_free (msg);
    }
  }

#ifdef HAS_IXJ
  lid = endpoint->GetLid ();
  if (lid) {

    lid->StopTone (0);
    lid->Unlock ();
  }
#endif
}


void 
gm_main_window_set_call_hold (GtkWidget *main_window,
			      gboolean is_on_hold)
{
  GmWindow *mw = NULL;
  
  GtkWidget *child = NULL;
  
  
  g_return_if_fail (main_window != NULL);
  
  mw = gm_mw_get_mw (main_window);
  
  g_return_if_fail (mw != NULL);
  
  
  child = GTK_BIN (gtk_menu_get_widget (mw->main_menu, "hold_call"))->child;

  if (is_on_hold) {

    if (GTK_IS_LABEL (child))
      gtk_label_set_text_with_mnemonic (GTK_LABEL (child),
					_("_Retrieve Call"));

    /* Set the audio and video buttons/menu to unsensitive */
    gtk_widget_set_sensitive (GTK_WIDGET (mw->audio_chan_button), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (mw->video_chan_button), FALSE);
    
    GTK_TOGGLE_BUTTON (mw->audio_chan_button)->active = TRUE;
    GTK_TOGGLE_BUTTON (mw->video_chan_button)->active = TRUE;
    
    gtk_menu_set_sensitive (mw->main_menu, "suspend_audio", FALSE);
    gtk_menu_set_sensitive (mw->main_menu, "suspend_video", FALSE);
  }
  else {

    if (GTK_IS_LABEL (child))
      gtk_label_set_text_with_mnemonic (GTK_LABEL (child),
					_("_Hold Call"));

    gtk_widget_set_sensitive (GTK_WIDGET (mw->audio_chan_button), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (mw->video_chan_button), TRUE);
    
    GTK_TOGGLE_BUTTON (mw->audio_chan_button)->active = FALSE;
    GTK_TOGGLE_BUTTON (mw->video_chan_button)->active = FALSE;
    
    gtk_menu_set_sensitive (mw->main_menu, "suspend_audio", TRUE);
    gtk_menu_set_sensitive (mw->main_menu, "suspend_video", TRUE);
  }
}


void 
gm_main_window_set_channel_pause (GtkWidget *main_window,
				  gboolean pause,
				  gboolean is_video)
{
  GmWindow *mw = NULL;
  
  GtkWidget *child = NULL;
  GtkToggleButton *b = NULL;

  gchar *msg = NULL;
  
  g_return_if_fail (main_window != NULL);
  
  mw = gm_mw_get_mw (main_window);
  
  g_return_if_fail (mw != NULL);
  

  if (!pause && !is_video)
    msg = g_strdup (_("Suspend _Audio"));
  else if (!pause && is_video)
    msg = g_strdup (_("Suspend _Video"));
  else if (pause && !is_video)
    msg = g_strdup (_("Resume _Audio"));
  else if (pause && is_video)
    msg = g_strdup (_("Resume _Video"));

  
  if (is_video) {
    
    b = GTK_TOGGLE_BUTTON (mw->video_chan_button);
    child =
      GTK_BIN (gtk_menu_get_widget (mw->main_menu, "suspend_video"))->child;
  }
  else {
    
    b = GTK_TOGGLE_BUTTON (mw->audio_chan_button);
    child =
      GTK_BIN (gtk_menu_get_widget (mw->main_menu, "suspend_audio"))->child;
  }
	

  if (GTK_IS_LABEL (child)) 
    gtk_label_set_text_with_mnemonic (GTK_LABEL (child),
				      msg);

  g_signal_handlers_block_by_func (G_OBJECT (b),
				   (gpointer) pause_current_call_channel_cb,
				   GINT_TO_POINTER (is_video));
  gtk_toggle_button_set_active (b, pause);
  g_signal_handlers_unblock_by_func (G_OBJECT (b),
				     (gpointer) pause_current_call_channel_cb,
				     GINT_TO_POINTER (is_video));

  g_free (msg);
}


void
gm_main_window_update_sensitivity (//GtkWidget *main_window,
				   unsigned calling_state)
{
  GmWindow *mw = NULL;
  
  GtkWidget *main_window = NULL;

  mw = GnomeMeeting::Process ()->GetMainWindow ();
  main_window = gm;
 
  switch (calling_state)
    {
    case GMH323EndPoint::Standby:

      gtk_menu_set_sensitive (mw->main_menu, "connect", TRUE);
      gtk_menu_set_sensitive (mw->main_menu, "disconnect", FALSE);
      gtk_menu_section_set_sensitive (mw->main_menu, "hold_call", FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (mw->preview_button), TRUE);
      gm_mw_update_connect_button (main_window, FALSE);
      break;


    case GMH323EndPoint::Calling:

      gtk_menu_set_sensitive (mw->main_menu, "connect", FALSE);
      gtk_menu_set_sensitive (mw->main_menu, "disconnect", TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (mw->preview_button), FALSE);
      gm_mw_update_connect_button (main_window, TRUE);
      break;


    case GMH323EndPoint::Connected:

      gtk_menu_set_sensitive (mw->main_menu, "connect", FALSE);
      gtk_menu_set_sensitive (mw->main_menu, "disconnect", TRUE);
      gtk_menu_section_set_sensitive (mw->main_menu, "hold_call", TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (mw->preview_button), FALSE);
      gm_mw_update_connect_button (main_window, TRUE);
      break;


    case GMH323EndPoint::Called:

      gtk_menu_set_sensitive (mw->main_menu, "disconnect", TRUE);
      gm_mw_update_connect_button (main_window, FALSE);
      break;
    }
}


void
gm_main_window_update_sensitivity (//GtkWidget *,
				   BOOL is_video,
				   BOOL is_receiving,
				   BOOL is_transmitting)
{
  GmWindow *mw = NULL;
  GtkWidget *button = NULL;
  GtkWidget *frame = NULL;

  mw = GnomeMeeting::Process ()->GetMainWindow ();

  
  /* We are updating video related items */
  if (is_video) {

  
    /* Receiving and sending => Everything sensitive in the section control */
    if (is_receiving && is_transmitting) {

      gtk_menu_section_set_sensitive (mw->main_menu,
				      "local_video", TRUE);
    }
    else {


      /* Default to nothing being sensitive */
      gtk_menu_section_set_sensitive (mw->main_menu,
				      "local_video", FALSE);
      
      
      /* We are sending video, but not receiving 
       * => local must be sensitive */
      if (is_transmitting) {

	gtk_menu_set_sensitive (mw->main_menu,
				"local_video", TRUE);
      }
      /* We are receiving video, but not transmitting,
       * => remote must be sensitive */
      else if (is_receiving) {

	gtk_menu_set_sensitive (mw->main_menu,
				"remote_video", TRUE);
      }
      
      
      /* We are not transmitting, and not receiving anything,
       * => Disable the zoom completely */
      if (!is_receiving && !is_transmitting) {

	gtk_menu_section_set_sensitive (mw->main_menu,
					"zoom_in", FALSE);
	gtk_menu_set_sensitive (mw->main_menu, "save_picture", FALSE);
      }
      else {
	/* Or activate it as at least something is transmitted or 
	 * received */
	gtk_menu_section_set_sensitive (mw->main_menu,
					"zoom_in", TRUE);
	gtk_menu_set_sensitive (mw->main_menu, "save_picture", TRUE);
      }
    }

    frame = mw->video_settings_frame;
    button = mw->video_chan_button;
  }
  else {

    frame = mw->audio_volume_frame;
    button = mw->audio_chan_button;
  }
  

  if (is_transmitting) {

    if (!is_video) 
      gtk_menu_set_sensitive (mw->main_menu, "suspend_audio", TRUE);
    else
      gtk_menu_set_sensitive (mw->main_menu, "suspend_video", TRUE);
  }	
  else {

    if (!is_video)
      gtk_menu_set_sensitive (mw->main_menu, "suspend_audio", FALSE);
    else
      gtk_menu_set_sensitive (mw->main_menu, "suspend_video", FALSE);

  }


  gtk_widget_set_sensitive (GTK_WIDGET (button), is_transmitting);
  gtk_widget_set_sensitive (GTK_WIDGET (frame), is_transmitting);
}


void
gm_main_window_set_volume_sliders_values (GtkWidget *main_window,
					  int output_volume, 
					  int input_volume)
{
  GmWindow *mw = NULL;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  if (output_volume != -1)
    GTK_ADJUSTMENT (mw->adj_output_volume)->value = output_volume;

  if (input_volume != -1)
    GTK_ADJUSTMENT (mw->adj_input_volume)->value = input_volume;
  
  gtk_widget_queue_draw (GTK_WIDGET (mw->audio_volume_frame));
}


void
gm_main_window_get_volume_sliders_values (GtkWidget *main_window,
					  int &output_volume, 
					  int &input_volume)
{
  GmWindow *mw = NULL;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  output_volume = (int) GTK_ADJUSTMENT (mw->adj_output_volume)->value;
  input_volume = (int) GTK_ADJUSTMENT (mw->adj_input_volume)->value;
}


void
gm_main_window_set_video_sliders_values (GtkWidget *main_window,
					 int whiteness,
					 int brightness,
					 int colour,
					 int contrast)
{
  GmWindow *mw = NULL;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  if (whiteness != -1)
    GTK_ADJUSTMENT (mw->adj_whiteness)->value = whiteness;

  if (brightness != -1)
    GTK_ADJUSTMENT (mw->adj_brightness)->value = brightness;
  
  if (colour != -1)
    GTK_ADJUSTMENT (mw->adj_colour)->value = colour;
  
  if (contrast != -1)
    GTK_ADJUSTMENT (mw->adj_contrast)->value = contrast;
  
  gtk_widget_queue_draw (GTK_WIDGET (mw->video_settings_frame));
}


void
gm_main_window_get_video_sliders_values (GtkWidget *main_window,
					 int &whiteness, 
					 int &brightness,
					 int &colour,
					 int &contrast)
{
  GmWindow *mw = NULL;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  whiteness = (int) GTK_ADJUSTMENT (mw->adj_whiteness)->value;
  brightness = (int) GTK_ADJUSTMENT (mw->adj_brightness)->value;
  colour = (int) GTK_ADJUSTMENT (mw->adj_colour)->value;
  contrast = (int) GTK_ADJUSTMENT (mw->adj_contrast)->value;
}


void gm_main_window_select_control_panel_section (GtkWidget *main_window,
						  int section)
{
  GmWindow *mw = NULL;
  
  g_return_if_fail (main_window != NULL);
  
  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  if (section == GM_MAIN_NOTEBOOK_HIDDEN)
    gtk_widget_hide_all (mw->main_notebook);
  else {

    gtk_widget_show_all (mw->main_notebook);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (mw->main_notebook), section);
  }
}


void 
gm_main_window_control_panel_section_menu_update (GtkWidget *main_window,
						  int section)
{
  GmWindow *mw = NULL;
  
  GtkWidget *menu = NULL;
  
  g_return_if_fail (main_window != NULL);
  
  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);
  
  menu = gtk_menu_get_widget (mw->main_menu, "statistics");
  
  gtk_radio_menu_select_with_widget (GTK_WIDGET (menu), section);
}


void
gm_main_window_speed_dials_menu_update (GtkWidget *main_window,
					GSList *glist)
{
  GmWindow *mw = NULL;
  GmContact *contact = NULL;
  
  GtkWidget *item = NULL;
  GtkWidget *menu = NULL;

  GSList *glist_iter = NULL;
  GList *old_glist_iter = NULL;

  gchar *ml = NULL;  


  g_return_if_fail (main_window != NULL);
  
  mw = gm_mw_get_mw (main_window);

  menu = gtk_menu_get_widget (mw->main_menu, "speed_dials");

  while ((old_glist_iter = GTK_MENU_SHELL (menu)->children)) 
    gtk_container_remove (GTK_CONTAINER (menu),
			  GTK_WIDGET (old_glist_iter->data));

  glist_iter = glist;
  while (glist_iter && glist_iter->data) {

    contact = GM_CONTACT (glist_iter->data);

    ml = g_strdup_printf ("<b>%s#</b>   <i>%s</i>", 
			  contact->speeddial, 
			  contact->fullname);

    item = gtk_menu_item_new_with_label (ml);
    gtk_label_set_markup (GTK_LABEL (gtk_bin_get_child (GTK_BIN (item))),
			  ml);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    gtk_widget_show (item);

    g_signal_connect_data (G_OBJECT (item), "activate",
			   GTK_SIGNAL_FUNC (speed_dial_menu_item_selected_cb),
			   (gpointer) g_strdup (contact->speeddial),
			   (GClosureNotify) g_free, (GConnectFlags) 0);


    glist_iter = g_slist_next (glist_iter);

    g_free (ml);
  }
}


void 
gm_main_window_transfer_dialog_run (GtkWidget *main_window,
				    gchar *u)
{
  GMH323EndPoint *endpoint = NULL;
  GmWindow *mw = NULL;
  
  GMURL url;
 
  gchar *conf_forward_value = NULL;
  gint answer = 0;
  

  g_return_if_fail (main_window != NULL);
  
  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);
  
  endpoint = GnomeMeeting::Process ()->Endpoint ();
  
  mw->transfer_call_popup = 
    gm_entry_dialog_new (_("Transfer call to:"),
			 _("Transfer"));

  gtk_window_set_transient_for (GTK_WINDOW (mw->transfer_call_popup),
				GTK_WINDOW (main_window));
  
  if (u)
    conf_forward_value = g_strdup (u);
  else
    conf_forward_value =
      gm_conf_get_string (CALL_FORWARDING_KEY "forward_host");
  
  gtk_dialog_set_default_response (GTK_DIALOG (mw->transfer_call_popup),
				   GTK_RESPONSE_ACCEPT);
  
  if (conf_forward_value && strcmp (conf_forward_value, ""))
    gm_entry_dialog_set_text (GM_ENTRY_DIALOG (mw->transfer_call_popup),
			      conf_forward_value);
  else
    gm_entry_dialog_set_text (GM_ENTRY_DIALOG (mw->transfer_call_popup),
			      (const char *) url.GetDefaultURL ());

  g_free (conf_forward_value);
  conf_forward_value = NULL;
  
  gnomemeeting_threads_dialog_show (mw->transfer_call_popup);

  answer = gtk_dialog_run (GTK_DIALOG (mw->transfer_call_popup));
  switch (answer) {

  case GTK_RESPONSE_ACCEPT:

    conf_forward_value =
      (gchar *) gm_entry_dialog_get_text (GM_ENTRY_DIALOG (mw->transfer_call_popup));
    new GMURLHandler (conf_forward_value, TRUE);
      
    break;

  default:
    break;
  }

  gtk_widget_destroy (mw->transfer_call_popup);
  mw->transfer_call_popup = NULL;
}


/* GTK callbacks */

static void 
hold_current_call_cb (GtkWidget *widget,
		      gpointer data)
{
  PString call_token;
  GMH323EndPoint *endpoint = NULL;

  BOOL is_on_hold = FALSE;
  
  g_return_if_fail (data != NULL);
  endpoint = GnomeMeeting::Process ()->Endpoint ();


  /* Release the GDK thread to prevent deadlocks, change
   * the hold state at the endpoint level.
   */
  gdk_threads_leave ();
  call_token = endpoint->GetCurrentCallToken ();
  is_on_hold = endpoint->IsCallOnHold (call_token);
  if (endpoint->SetCallOnHold (call_token, !is_on_hold))
    is_on_hold = !is_on_hold; /* It worked */
  gdk_threads_enter ();

  
  /* Update the GUI */
  gm_main_window_set_call_hold (GTK_WIDGET (data), is_on_hold);
}


static void
pause_current_call_channel_cb (GtkWidget *widget,
			       gpointer data)
{
  GMH323EndPoint *endpoint = NULL;
  GMVideoGrabber *vg = NULL;

  GtkWidget *main_window = NULL;
 
  PString current_call_token;
  BOOL is_paused = FALSE;
  
  endpoint = GnomeMeeting::Process ()->Endpoint ();
  current_call_token = endpoint->GetCurrentCallToken ();

  main_window = gm; 

  cout << "ici" << endl << flush;
  
  if (!current_call_token.IsEmpty ()
      && endpoint->GetCallingState () == GMH323EndPoint::Standby) {

    gdk_threads_leave ();
    vg = endpoint->GetVideoGrabber ();
    if (vg && vg->IsGrabbing ())
      vg->StopGrabbing ();
    else
      vg->StartGrabbing ();
    gdk_threads_enter ();
  }
  else {

    if (GPOINTER_TO_INT (data) == 0) {
      
      gdk_threads_leave ();
      is_paused = endpoint->IsCallAudioPaused (current_call_token);
      if (endpoint->SetCallAudioPause (current_call_token, !is_paused))
	is_paused = !is_paused; /* It worked */
      gdk_threads_enter ();

      gm_main_window_set_channel_pause (main_window, is_paused, FALSE);
    }
    else {

      gdk_threads_leave ();
      is_paused = endpoint->IsCallVideoPaused (current_call_token);
      if (endpoint->SetCallVideoPause (current_call_token, !is_paused))
	is_paused = !is_paused; /* It worked */
      gdk_threads_enter ();
      
      gm_main_window_set_channel_pause (main_window, is_paused, TRUE);
    }
  }
}


static void 
transfer_current_call_cb (GtkWidget *widget,
			  gpointer data)
{
  g_return_if_fail (data != NULL);
  
  gm_main_window_transfer_dialog_run (GTK_WIDGET (data), NULL);  
}


static void
video_window_shown_cb (GtkWidget *w, 
		       gpointer data)
{
  GMH323EndPoint *endpoint = NULL;

  endpoint = GnomeMeeting::Process ()->Endpoint ();

  if (endpoint 
      && gm_conf_get_bool (VIDEO_DISPLAY_KEY "stay_on_top")
      && endpoint->GetCallingState () == GMH323EndPoint::Connected)
    gdk_window_set_always_on_top (GDK_WINDOW (w->window), TRUE);
}


static void
dnd_call_contact_cb (GtkWidget *widget, 
		     GmContact *contact,
		     gint x, 
		     gint y, 
		     gpointer data)
{
  GmWindow *mw = NULL;
  
  g_return_if_fail (data != NULL);
  
  if (contact && contact->url) {
    mw = (GmWindow *)data;
     if (GnomeMeeting::Process ()->Endpoint ()->GetCallingState () == GMH323EndPoint::Standby) {
       
       /* this function will store a copy of text */
       gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (mw->combo)->entry),
			   PString (contact->url));
       
       gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mw->connect_button),
				     true);
     }
     gm_contact_delete (contact);
  }
}


static void 
audio_volume_changed_cb (GtkAdjustment *adjustment, 
			 gpointer data)
{
  GMH323EndPoint *ep = NULL;
  
  H323Connection *con = NULL;
  H323Codec *raw_codec = NULL;
  H323Channel *channel = NULL;

  PSoundChannel *sound_channel = NULL;

  int play_vol =  0, rec_vol = 0;


  g_return_if_fail (data != NULL);
  ep = GnomeMeeting::Process ()->Endpoint ();

  gm_main_window_get_volume_sliders_values (GTK_WIDGET (data), 
					    play_vol, rec_vol);

  gdk_threads_leave ();
 
  con = ep->FindConnectionWithLock (ep->GetCurrentCallToken ());

  if (con) {

    for (int cpt = 0 ; cpt < 2 ; cpt++) {

      channel = 
        con->FindChannel (RTP_Session::DefaultAudioSessionID, (cpt == 0));         
      if (channel) {

        raw_codec = channel->GetCodec();

        if (raw_codec) {

          sound_channel = (PSoundChannel *) raw_codec->GetRawDataChannel ();

          if (sound_channel)
            ep->SetDeviceVolume (sound_channel, 
                                 (cpt == 1), 
                                 (cpt == 1) ? rec_vol : play_vol);
        }
      }
    }
    con->Unlock ();
  }

  gdk_threads_enter ();
}


static void 
video_settings_changed_cb (GtkAdjustment *adjustment, 
			   gpointer data)
{ 
  GMH323EndPoint *ep = NULL;
  GMVideoGrabber *video_grabber = NULL;

  int brightness = -1;
  int whiteness = -1;
  int colour = -1;
  int contrast = -1;

  g_return_if_fail (data != NULL);
  
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  gm_main_window_get_video_sliders_values (GTK_WIDGET (data),
					   brightness,
					   whiteness,
					   colour,
					   contrast);

  /* Notice about mutexes:
     The GDK lock is taken in the callback. We need to release it, because
     if CreateVideoGrabber is called in another thread, it will only
     release its internal mutex (also used by GetVideoGrabber) after it 
     returns, but it will return only if it is opened, and it can't open 
     if the GDK lock is held as it will wait on the GDK lock before 
     updating the GUI */
  gdk_threads_leave ();
  if (ep && (video_grabber = ep->GetVideoGrabber ())) {
    
    video_grabber->SetWhiteness (whiteness << 8);
    video_grabber->SetBrightness (brightness << 8);
    video_grabber->SetColour (colour << 8);
    video_grabber->SetContrast (contrast << 8);
    video_grabber->Unlock ();
  }
  gdk_threads_enter ();
}


static void 
control_panel_section_changed_cb (GtkNotebook *notebook, 
				  GtkNotebookPage *page,
				  gint page_num, 
				  gpointer data) 
{
  GmWindow *mw = NULL;
  
  gint current_page = 0;

  
  g_return_if_fail (data != NULL);
  mw = gm_mw_get_mw (GTK_WIDGET (data));

  
  current_page = 
    gtk_notebook_get_current_page (GTK_NOTEBOOK (mw->main_notebook));
  gm_conf_set_int (USER_INTERFACE_KEY "main_window/control_panel_section",
		   current_page);
}


static void 
dialpad_button_clicked_cb (GtkButton *button, 
			   gpointer data)
{
  GtkWidget *label = NULL;
  const char *button_text = NULL;

  g_return_if_fail (data != NULL);

  
  /* FIXME: separation dans dialpad event du code du endpoint */
  label = gtk_bin_get_child (GTK_BIN (button));
  button_text = gtk_label_get_text (GTK_LABEL (label));

  if (button_text
      && strcmp (button_text, "")
      && strlen (button_text) > 1
      && button_text [0])
    gm_main_window_dialpad_event (GTK_WIDGET (data),
				  button_text [0]);
}


static gint 
window_closed_cb (GtkWidget *widget, 
		  GdkEvent *event,
		  gpointer data)
{
  GtkWidget *tray = NULL;
  
  GmWindow *mw = NULL;
  
  gboolean b = FALSE;

  g_return_val_if_fail (data != NULL, FALSE);
  mw = gm_mw_get_mw (GTK_WIDGET (data));
  tray = GnomeMeeting::Process ()->GetTray ();
  

  b = gm_tray_is_embedded (tray);

  if (!b)
    quit_callback (NULL, data);
  else 
    gnomemeeting_window_hide (GTK_WIDGET (gm));

  return (TRUE);
}  


static void 
text_chat_clear_cb (GtkWidget *widget,
		    gpointer data)
{
  g_return_if_fail (data != NULL);
  
  gnomemeeting_text_chat_clear (GTK_WIDGET (data));
}


static void 
zoom_changed_cb (GtkWidget *widget,
		 gpointer data)
{
  double zoom = 0.0;
  
  zoom = gm_conf_get_float (VIDEO_DISPLAY_KEY "zoom_factor");

  switch (GPOINTER_TO_INT (data)) {

  case 0:
    if (zoom > 0.5)
      zoom = zoom / 2.0;
    break;

  case 1:
    zoom = 1.0;
    break;

  case 2:
    if (zoom < 2.00)
      zoom = zoom * 2.0;
  }

  gm_conf_set_float (VIDEO_DISPLAY_KEY "zoom_factor", zoom);
}


static void 
fullscreen_changed_cb (GtkWidget *widget,
		       gpointer data)
{
  gm_conf_set_float (VIDEO_DISPLAY_KEY "zoom_factor", -1.0);
}


static void
speed_dial_menu_item_selected_cb (GtkWidget *w,
				  gpointer data)
{
  GmWindow *mw = NULL;
  GMH323EndPoint *ep = NULL;
  
  gchar *url = NULL;
    
  mw = gm_mw_get_mw (gm); 
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  g_return_if_fail (data != NULL);

  url = g_strdup_printf ("%s#", (gchar *) data);
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (mw->combo)->entry),
		      (gchar *) url);
    
  /* FIXME */
  if (ep->GetCallingState () == GMH323EndPoint::Connected)
    gm_main_window_transfer_dialog_run (gm, url);
  else
    connect_cb (NULL, NULL);

  g_free (url);
}


static void
combo_url_changed_cb (GtkEditable  *e, 
		      gpointer data)
{
  GmWindow *mw = NULL;

  gchar *tip_text = NULL;
  
  g_return_if_fail (data != NULL);
  mw = gm_mw_get_mw (gm); 

  g_return_if_fail (mw != NULL);
  
  tip_text = (gchar *)
    gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (mw->combo)->entry));

  gtk_tooltips_set_tip (mw->tips, 
			GTK_WIDGET (GTK_COMBO (mw->combo)->entry), 
			tip_text, NULL);
}


static void 
control_panel_button_clicked_cb (GtkWidget *w, 
				 gpointer data)
{
  if (gm_conf_get_int (USER_INTERFACE_KEY "main_window/control_panel_section") 
      == GM_MAIN_NOTEBOOK_HIDDEN) { 
    
    gm_conf_set_int (USER_INTERFACE_KEY "main_window/control_panel_section", 
		     0);
  } 
  else {   
    
    gm_conf_set_int (USER_INTERFACE_KEY "main_window/control_panel_section", 
		     GM_MAIN_NOTEBOOK_HIDDEN);
  }
}


static void 
toolbar_toggle_button_changed_cb (GtkWidget *widget, 
				  gpointer data)
{
  bool shown = gm_conf_get_bool ((gchar *) data);

  gm_conf_set_bool ((gchar *) data, !shown);
}


static void 
toolbar_connect_button_clicked_cb (GtkToggleButton *w, 
				   gpointer data)
{
  if (gtk_toggle_button_get_active (w))  
    connect_cb (NULL, NULL);  
  else
    disconnect_cb (NULL, NULL);
}


/* Public functions */
GtkWidget *
gm_main_window_new (GmWindow *mw)
{
  GtkWidget *window = NULL;
  GtkWidget *table = NULL;	
  GtkWidget *frame = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GdkPixbuf *pixbuf = NULL;
  GtkWidget *event_box = NULL;
  GtkWidget *chat_window = NULL;

  int main_notebook_section = 0;
  
  /* The Top-level window */
#ifndef DISABLE_GNOME
  window = gnome_app_new ("gnomemeeting", NULL);
#else
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
#endif
  //FIXME
  gm = window;
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("main_window"), g_free);

  gtk_window_set_title (GTK_WINDOW (window), 
			_("GnomeMeeting"));
  gtk_window_set_position (GTK_WINDOW (window), 
			   GTK_WIN_POS_CENTER);


  /* The GMObject data */
  //mw = new GmWindow ();
  g_object_set_data_full (G_OBJECT (window), "GMObject", 
			  mw, (GDestroyNotify) gm_mw_destroy);

  
  mw->accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (window), mw->accel);

#ifdef DISABLE_GNOME
  mw->window_vbox = gtk_vbox_new (0, FALSE);
  gtk_container_add (GTK_CONTAINER (window), mw->window_vbox);
  gtk_widget_show (mw->window_vbox);
  mw->window_hbox = gtk_hbox_new (0, FALSE);
  gtk_box_pack_start (GTK_BOX (mw->window_vbox), mw->window_hbox, 
		      FALSE, FALSE, 0);
#endif

  
  /* The main menu and the toolbars */
  gm_mw_init_menu (window);
  gm_mw_init_toolbars (window);

    
  /* The statusbar */
  mw->statusbar = gtk_statusbar_new ();
#ifndef DISABLE_GNOME
  gnome_app_add_docked (GNOME_APP (window), 
			mw->main_menu,
			"menubar",
			BONOBO_DOCK_ITEM_BEH_EXCLUSIVE,
  			BONOBO_DOCK_TOP, 0, 0, 0);
#else
  gtk_box_pack_start (GTK_BOX (mw->window_vbox), mw->main_menu,
		      FALSE, FALSE, 0);
#endif

  
  /* Create a table in the main window to attach things like buttons */
  table = gtk_table_new (3, 4, FALSE);
#ifdef DISABLE_GNOME
  gtk_box_pack_start (GTK_BOX (mw->window_hbox), table, FALSE, FALSE, 0);
#else
  gnome_app_set_contents (GNOME_APP (window), table);
#endif
  gtk_widget_show (table);

  /* The Notebook */
  mw->main_notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (mw->main_notebook), GTK_POS_BOTTOM);
  gtk_notebook_popup_enable (GTK_NOTEBOOK (mw->main_notebook));
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (mw->main_notebook), TRUE);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (mw->main_notebook), TRUE);

  gm_mw_init_stats (window);
  gm_mw_init_dialpad (window);
  gm_mw_init_audio_settings (window);
  gm_mw_init_video_settings (window);

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (mw->main_notebook),
		    0, 2, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    6, 6); 

  main_notebook_section = 
    gm_conf_get_int (USER_INTERFACE_KEY "main_window/control_panel_section");

  if (main_notebook_section != GM_MAIN_NOTEBOOK_HIDDEN) {

    gtk_widget_show_all (GTK_WIDGET (mw->main_notebook));
    gtk_notebook_set_current_page (GTK_NOTEBOOK ((mw->main_notebook)), 
				   main_notebook_section);
  }


  /* The frame that contains video and remote name display */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

  /* The frame that contains the video */
  mw->video_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (mw->video_frame), GTK_SHADOW_IN);
  
  event_box = gtk_event_box_new ();

  vbox = gtk_vbox_new (FALSE, 0);

  gtk_container_add (GTK_CONTAINER (frame), event_box);
  gtk_container_add (GTK_CONTAINER (event_box), vbox);
  gtk_box_pack_start (GTK_BOX (vbox), mw->video_frame, TRUE, TRUE, 0);

  mw->main_video_image = gtk_image_new ();
  gtk_container_set_border_width (GTK_CONTAINER (mw->video_frame), 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
  gtk_container_add (GTK_CONTAINER (mw->video_frame), mw->main_video_image);

  gtk_widget_set_size_request (GTK_WIDGET (mw->video_frame), 
			       GM_QCIF_WIDTH + GM_FRAME_SIZE, 
			       GM_QCIF_HEIGHT + GM_FRAME_SIZE); 

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (frame), 
		    0, 2, 0, 1,
		    (GtkAttachOptions) GTK_EXPAND,
		    (GtkAttachOptions) GTK_EXPAND,
		    6, 6);

  gtk_widget_show_all (GTK_WIDGET (frame));

  
  /* The statusbar and the progressbar */
  hbox = gtk_hbox_new (0, FALSE);
#ifdef DISABLE_GNOME
  gtk_box_pack_start (GTK_BOX (mw->window_vbox), hbox, 
		      FALSE, FALSE, 0);
#else
  gnome_app_add_docked (GNOME_APP (window), hbox, "statusbar",
  			BONOBO_DOCK_ITEM_BEH_EXCLUSIVE,
  			BONOBO_DOCK_BOTTOM, 3, 0, 0);
#endif
  gtk_widget_show (hbox);

  
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (mw->statusbar), FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), mw->statusbar, 
		      TRUE, TRUE, 0);

  if (gm_conf_get_bool (USER_INTERFACE_KEY "main_window/show_status_bar"))
    gtk_widget_show (GTK_WIDGET (mw->statusbar));
  else
    gtk_widget_hide (GTK_WIDGET (mw->statusbar));
  
  
  /* The 2 video window popups */
  mw->local_video_window =
    gnomemeeting_video_window_new (_("Local Video"),
				   mw->local_video_image,
				   "local_video_window");
  mw->remote_video_window =
    gnomemeeting_video_window_new (_("Remote Video"),
				   mw->remote_video_image,
				   "remote_video_window");
  
  gm_main_window_update_logo (window);

  g_signal_connect (G_OBJECT (mw->local_video_window), "show", 
		    GTK_SIGNAL_FUNC (video_window_shown_cb), NULL);
  g_signal_connect (G_OBJECT (mw->remote_video_window), "show", 
		    GTK_SIGNAL_FUNC (video_window_shown_cb), NULL);


  /* The remote name */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

  mw->remote_name = gtk_label_new (NULL);
  gtk_widget_set_size_request (GTK_WIDGET (mw->remote_name), 
			       GM_QCIF_WIDTH, -1);

  gtk_container_add (GTK_CONTAINER (frame), mw->remote_name);
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
  gtk_widget_show_all (GTK_WIDGET (frame));


  /* The Chat Window */
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  /* FIXME */
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (chat_window), 
 		    2, 4, 0, 3,
 		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
 		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
 		    6, 6);
  if (gm_conf_get_bool (USER_INTERFACE_KEY "main_window/show_chat_window"))
    gtk_widget_show_all (GTK_WIDGET (chat_window));
  
  gtk_widget_set_size_request (GTK_WIDGET (mw->main_notebook),
			       GM_QCIF_WIDTH + GM_FRAME_SIZE, -1);
  gtk_widget_set_size_request (GTK_WIDGET (window), -1, -1);

  
  /* Add the window icon and title */
  gtk_window_set_title (GTK_WINDOW (window), _("GnomeMeeting"));
  pixbuf = 
    gdk_pixbuf_new_from_file (GNOMEMEETING_IMAGES
			      "gnomemeeting-logo-icon.png", NULL);
  gtk_window_set_icon (GTK_WINDOW (window), pixbuf);
  gtk_widget_realize (window);
  g_object_unref (G_OBJECT (pixbuf));
  gtk_window_set_resizable (GTK_WINDOW (window), false);

  g_signal_connect_after (G_OBJECT (mw->main_notebook), "switch-page",
			  G_CALLBACK (control_panel_section_changed_cb), 
			  window);


  /* Init the Drag and drop features */
  gm_contacts_dnd_set_dest (GTK_WIDGET (window), dnd_call_contact_cb, mw);

  /* if the user tries to close the window : delete_event */
  g_signal_connect (G_OBJECT (window), "delete_event",
		    G_CALLBACK (window_closed_cb), 
		    (gpointer) window);

  g_signal_connect (G_OBJECT (window), "show", 
		    GTK_SIGNAL_FUNC (video_window_shown_cb), NULL);

  
  return window;
}


/* The main () */

int main (int argc, char ** argv, char ** envp)
{
  PProcess::PreInitialise (argc, argv, envp);

  GtkWidget *dialog = NULL;
  
  GmWindow *mw = NULL;

  gchar *url = NULL;
  gchar *key_name = NULL;
  gchar *msg = NULL;

  int debug_level = 0;

  
#ifndef WIN32
  setenv ("ESD_NO_SPAWN", "1", 1);
#endif
  

  /* Threads + Locale Init + config */
  g_thread_init (NULL);
  gdk_threads_init ();
  
#ifndef WIN32
  gtk_init (&argc, &argv);
#else
  gtk_init (NULL, NULL);
#endif

  xmlInitParser ();

  gm_conf_init (argc, argv);

  
  /* Upgrade the preferences */
  gnomemeeting_conf_upgrade ();

  /* Initialize gettext */
  textdomain (GETTEXT_PACKAGE);
  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");

  
  /* Select the Mic as default source for OSS. Will be removed when
   * ALSA will be everywhere 
   */
  gnomemeeting_mixers_mic_select ();
  
#ifndef DISABLE_GNOME
  /* Cope with command line options */
  struct poptOption arguments [] =
    {
      {"debug", 'd', POPT_ARG_INT, &debug_level, 
       1, N_("Prints debug messages in the console (level between 1 and 6)"), 
       NULL},
      {"call", 'c', POPT_ARG_STRING, &url,
       1, N_("Makes GnomeMeeting call the given URL"), NULL},
      {NULL, '\0', 0, NULL, 0, NULL, NULL}
    };
  
  /* GnomeMeeting Initialisation */
  gnome_program_init ("gnomemeeting", VERSION,
		      LIBGNOMEUI_MODULE, argc, argv,
		      GNOME_PARAM_POPT_TABLE, arguments,
		      GNOME_PARAM_HUMAN_READABLE_NAME,
		      "gnomemeeting",
		      GNOME_PARAM_APP_DATADIR, GNOMEMEETING_DATADIR,
		      (void *) NULL);
#endif
  
  gdk_threads_enter ();
 
  /* The factory */
#ifndef DISABLE_GNOME
  if (bonobo_component_init (argc, argv))
    exit (1);
#endif

  /* GnomeMeeting main initialisation */
  static GnomeMeeting instance;

  /* Debug */
  if (debug_level != 0)
    PTrace::Initialise (PMAX (PMIN (4, debug_level), 0), NULL,
			PTrace::Timestamp | PTrace::Thread
			| PTrace::Blocks | PTrace::DateAndTime);

  
  /* Detect the devices, exit if it fails */
  if (!GnomeMeeting::Process ()->DetectDevices ()) {

    dialog = gnomemeeting_error_dialog (NULL, _("No usable audio manager detected"), _("GnomeMeeting didn't find any usable sound manager. Make sure that your installation is correct."));
    
    g_signal_handlers_disconnect_by_func (G_OBJECT (dialog),
					  (gpointer) gtk_widget_destroy,
					  G_OBJECT (dialog));

    gtk_dialog_run (GTK_DIALOG (dialog));
    exit (-1);
  }


  /* Init the process and build the GUI */
  GnomeMeeting::Process ()->BuildGUI ();
  GnomeMeeting::Process ()->Init ();


  /* Init the config DB, exit if it fails */
  if (!gnomemeeting_conf_init ()) {

    key_name = g_strdup ("\"/apps/gnomemeeting/general/gconf_test_age\"");
    msg = g_strdup_printf (_("GnomeMeeting got an invalid value for the GConf key %s.\n\nIt probably means that your GConf schemas have not been correctly installed or the that permissions are not correct.\n\nPlease check the FAQ (http://www.gnomemeeting.org/faq.php), the throubleshoot section of the GConf site (http://www.gnome.org/projects/gconf/) or the mailing list archives for more information (http://mail.gnome.org) about this problem."), key_name);
    
    dialog = gnomemeeting_error_dialog (GTK_WINDOW (gm),
					_("Gconf key error"), msg);

    g_signal_handlers_disconnect_by_func (G_OBJECT (dialog),
					  (gpointer) gtk_widget_destroy,
					  G_OBJECT (dialog));


    g_free (msg);
    g_free (key_name);
    
    gtk_dialog_run (GTK_DIALOG (dialog));
    exit (-1);
  }

  
  /* Call the given host if needed */
  if (url) {

    mw = GnomeMeeting::Process ()->GetMainWindow ();
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (mw->combo)->entry), url);
    connect_cb (NULL, NULL);
  }

  
  /* The GTK loop */
  gtk_main ();
  gdk_threads_leave ();

  gm_conf_save ();

  return 0;
}


#ifdef WIN32
int APIENTRY WinMain(HINSTANCE hInstance,
		     HINSTANCE hPrevInstance,
		     LPSTR     lpCmdLine,
		     int       nCmdShow)
{
  return main (0, NULL, NULL);
}
#endif
