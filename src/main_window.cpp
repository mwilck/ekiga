
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
#include "calls_history_window.h"
#include "pcssendpoint.h"
#include "gnomemeeting.h"
#include "chat_window.h"
#include "config.h"
#include "misc.h"
#include "callbacks.h"
#include "tray.h"
#include "lid.h"
#include "sound_handling.h"
#include "urlhandler.h"
#include "gm_events.h"

#include <dialog.h>
#include <gmentrydialog.h>
#include <stock-icons.h>
#include <gm_conf.h>
#include <contacts/gm_contacts.h>
#include <gtk_menu_extensions.h>
#include <stats_drawing_area.h>
#include <gm_events.h>


#include "../pixmaps/text_logo.xpm"

#include <gdk/gdkkeysyms.h>

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

#ifdef HAS_SDL
#include <SDL.h>
#endif


#define GM_MAIN_WINDOW(x) (GmWindow *) (x)


/* Declarations */
struct _GmWindow
{
  GtkObject *adj_input_volume;
  GtkObject *adj_output_volume;
  GtkWidget *audio_volume_frame;

  GtkObject *adj_whiteness;
  GtkObject *adj_brightness;
  GtkObject *adj_colour;
  GtkObject *adj_contrast;
  GtkWidget *video_settings_frame;
  
  GtkTooltips *tips;
  GtkAccelGroup *accel;

  GtkWidget *main_menu;
  
#ifdef DISABLE_GNOME
  GtkWidget *window_vbox;
  GtkWidget *window_hbox;
#endif

  GtkWidget *statusbar;
  GtkWidget *remote_name;
  GtkWidget *combo;
  GtkWidget *main_notebook;
  GtkWidget *main_video_image;
  GtkWidget *local_video_image;
  GtkWidget *local_video_window;
  GtkWidget *remote_video_image;
  GtkWidget *remote_video_window;
  GtkWidget *video_frame;
  GtkWidget *preview_button;
  GtkWidget *connect_button;
  GtkWidget *video_chan_button;
  GtkWidget *audio_chan_button;
  GtkWidget *incoming_call_popup;
  GtkWidget *transfer_call_popup;
  GtkWidget *stats_label;
  GtkWidget *stats_drawing_area;

#ifdef HAS_SDL
  SDL_Surface *screen;
#endif
};


typedef struct _GmWindow GmWindow;


#define GM_WINDOW(x) (GmWindow *) (x)


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
 * PRE          :  The main window GMObject. The statusbar must have been
 * 		   created.
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


/* DESCRIPTION  : /
 * BEHAVIOR     : Push a message on the main window status bar. That message
 * 		  will only appear 4 seconds if the second parameter is TRUE,
 * 		  and will stay until it is cleared if it is FALSE. The third
 * 		  parameter indicates if it is an info message or a normal 
 * 		  message. Info messages will stay until another info message
 * 		  is displayed or until they are cleared when the user clicks
 * 		  in the statusbar. A normal message will stay until another one
 * 		  is displayed, even if that other one is only flashing.
 * PRE          : /
 */
static void gm_mw_push_message (GtkWidget *main_window, 
				gboolean flash_message,
				gboolean info_message,
				const char *msg, 
				...);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Creates a video window.
 * PRE           : The title of the window, the drawing area and the window
 *                 name that will be used by gnomemeeting_window_show/hide.
 */
GtkWidget *gm_mw_video_window_new (GtkWidget *,
				   gboolean,
				   gchar *,
				   GtkWidget *&,
				   gchar *);


#ifdef HAS_SDL
/* DESCRIPTION   : /
 * BEHAVIOR      : Creates a video window.
 * PRE           : /
 */
void gm_mw_init_fullscreen_video_window (GtkWidget *);


/* DESCRIPTION   : /
 * BEHAVIOR      : Toggle the fullscreen state of the main window.
 * PRE           : /
 */
void gm_mw_toggle_fullscreen (GtkWidget *);
	

/* DESCRIPTION   : /
 * BEHAVIOR      : Return TRUE if the Esc key is pressed.
 * PRE           : /
 */
gboolean gm_mw_poll_fullscreen_video_window (GtkWidget *);
	

/* DESCRIPTION   : /
 * BEHAVIOR      : Creates a video window.
 * PRE           : /
 */
void gm_mw_destroy_fullscreen_video_window (GtkWidget *);

#endif


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
 *                 factor in the menu, and chooses to zoom in.
 * BEHAVIOR     :  zoom *= 2.
 * PRE          :  The GConf key to update with the new zoom.
 */
static void zoom_in_changed_cb (GtkWidget *,
				gpointer);


/* DESCRIPTION  :  This callback is called when the user changes the zoom
 *                 factor in the menu, and chooses to zoom in.
 * BEHAVIOR     :  zoom /= 2.
 * PRE          :  The GConf key to update with the new zoom.
 */
static void zoom_out_changed_cb (GtkWidget *,
				 gpointer);


/* DESCRIPTION  :  This callback is called when the user changes the zoom
 *                 factor in the menu, and chooses to zoom in.
 * BEHAVIOR     :  zoom = 1.
 * PRE          :  The GConf key to update with the new zoom.
 */
static void zoom_normal_changed_cb (GtkWidget *,
				    gpointer);


#ifdef HAS_SDL
/* DESCRIPTION  :  This callback is called when the user toggles fullscreen
 *                 factor in the popup menu.
 * BEHAVIOR     :  Toggles the fullscreen configuration key. 
 * PRE          :  / 
 */
static void fullscreen_changed_cb (GtkWidget *,
				   gpointer);
#endif


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
 * PRE          :  A valid pointer to the main window GMObject. 
 */
static void url_changed_cb (GtkEditable *, 
			    gpointer);


/* DESCRIPTION  :  This callback is called when the user selects a match in the
 * 		   possible URLs list.
 * BEHAVIOR     :  It udpates the URL bar and calls it.
 * PRE          :  /
 */
static gboolean completion_url_selected_cb (GtkEntryCompletion *,
					    GtkTreeModel *,
					    GtkTreeIter *,
					    gpointer);

/* DESCRIPTION  :  This callback is called when the user selects an url in the
 * 		   possible URLs list of the combo.
 * BEHAVIOR     :  Calls it.
 * PRE          :  /
 */
static void combo_url_selected_cb (GtkComboBox *,
				   gpointer);


/* DESCRIPTION  :  This callback is called when the user clicks on enter
 * 		   with a non-empty URL bar.
 * BEHAVIOR     :  It calls the URL.
 * PRE          :  /
 */
static void url_activated_cb (GtkWidget *, 
			      gpointer);


/* DESCRIPTION  :  This callback is called to compare urls and see if they
 * 		   match.
 * BEHAVIOR     :  It returns TRUE if the given key matches an URL OR a last
 * 		   name or first name in the list store of the completion 
 * 		   entry AND if the matched URL was not already returned
 * 		   previously.
 * 		   2 H323 URLs match if they begin by
 * 		   the same chars, and 2 CALLTO URLs with a valid email
 * 		   address on an ILS server match if the key matches an email
 * 		   address or the begin of a server. 
 * PRE          :  data is a valid pointer to the list store.
 */
static gboolean found_url_cb (GtkEntryCompletion *,
			      const gchar *,
			      GtkTreeIter *,
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
 * BEHAVIOR     :  Connect if there is a connect URL in the URL bar and if the
 * 		   button is toggled, the button is untoggled if there is no 
 * 		   url, disconnect if the button is untoggled.
 * PRE          :  data is a valid pointer to the main window GMObject.
 */
static void toolbar_connect_button_clicked_cb (GtkToggleButton *, 
					       gpointer);


/* DESCRIPTION  :  This callback is called after a few seconds to clear
 * 		   a message in the statusbar.
 * BEHAVIOR     :  Clears the message.
 * PRE          :  A valid msg id.
 */
static int statusbar_clear_msg_cb (gpointer);


/* DESCRIPTION  :  This callback is called when the status bar is clicked.
 * BEHAVIOR     :  Clear all info message, not normal messages. Reset the
 * 		   endpoint missed calls number.
 * PRE          :  The main window GMObject.
 */
static gboolean statusbar_clicked_cb (GtkWidget *,
				      GdkEventButton *,
				      gpointer);


/* DESCRIPTION  :  This callback is called on delete event for the incoming
 * 		   call dialog.
 * BEHAVIOR     :  Disconnects and set the pointer to NULL, the destroy signal
 * 		   will destroy the dialog by itself.
 * PRE          :  A valid main window GMObject.
 */
static gboolean delete_incoming_call_dialog_cb (GtkWidget *,
						GdkEvent *,
						gpointer);


/* Implementation */
static void
gm_mw_destroy (gpointer m)
{
  GmWindow *mw = GM_WINDOW (m);
  
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
        g_signal_handlers_block_by_func (G_OBJECT (mw->connect_button), (void *) toolbar_connect_button_clicked_cb, main_window);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mw->connect_button), 
				      TRUE);
        g_signal_handlers_unblock_by_func (G_OBJECT (mw->connect_button), (void *) toolbar_connect_button_clicked_cb, main_window);
	
      } else {
        
        gtk_image_set_from_stock (GTK_IMAGE (image),
                                  GM_STOCK_DISCONNECT, 
                                  GTK_ICON_SIZE_LARGE_TOOLBAR);
        
        g_signal_handlers_block_by_func (G_OBJECT (mw->connect_button), (void *) toolbar_connect_button_clicked_cb, main_window);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mw->connect_button), 
				      FALSE);
        g_signal_handlers_unblock_by_func (G_OBJECT (mw->connect_button), (void *) toolbar_connect_button_clicked_cb, main_window);
      }   
    
    gtk_widget_queue_draw (GTK_WIDGET (image));
    gtk_widget_queue_draw (GTK_WIDGET (mw->connect_button));
  }
}


static void
gm_mw_init_toolbars (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  
  GtkWidget *button = NULL;
  GtkToolItem *item = NULL;

  GtkListStore *list_store = NULL;
  GtkWidget *toolbar = NULL;
  
  GtkEntryCompletion *completion = NULL;
  
  GtkWidget *image = NULL;

  GtkWidget *addressbook_window = NULL;
  
#ifndef DISABLE_GNOME
  int behavior = 0;
  BOOL toolbar_detachable = TRUE;
#endif

  addressbook_window = GnomeMeeting::Process ()->GetAddressbookWindow ();

  
  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);


#ifndef DISABLE_GNOME
  toolbar_detachable = 
    gm_conf_get_bool ("/desktop/gnome/interface/toolbar_detachable");
#endif

  
  /* The main horizontal toolbar */
  toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (toolbar), FALSE);


  /* URL bar */
  /* Entry */
  item = gtk_tool_item_new ();
  mw->combo = gtk_combo_box_entry_new_text ();

  gtk_container_add (GTK_CONTAINER (item), mw->combo);
  gtk_container_set_border_width (GTK_CONTAINER (item), 4);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), TRUE);
  
  completion = gtk_entry_completion_new ();
  
  list_store = gtk_list_store_new (3, 
				   G_TYPE_STRING,
				   G_TYPE_STRING,
				   G_TYPE_STRING);
  
  gtk_entry_completion_set_model (GTK_ENTRY_COMPLETION (completion),
				  GTK_TREE_MODEL (list_store));
  gtk_entry_completion_set_text_column (GTK_ENTRY_COMPLETION (completion), 2);
  gtk_entry_set_completion (GTK_ENTRY (GTK_BIN (mw->combo)->child), completion);
  
  gtk_entry_set_text (GTK_ENTRY (GTK_BIN (mw->combo)->child),
		      GMURL ().GetDefaultURL ());

  gtk_entry_completion_set_match_func (GTK_ENTRY_COMPLETION (completion),
				       found_url_cb, 
				       (gpointer) list_store,
				       NULL);
  
  gm_main_window_urls_history_update (main_window);

  g_signal_connect (G_OBJECT (GTK_BIN (mw->combo)->child), "changed", 
		    GTK_SIGNAL_FUNC (url_changed_cb), (gpointer) main_window);
  g_signal_connect (G_OBJECT (GTK_BIN (mw->combo)->child), "activate", 
		    GTK_SIGNAL_FUNC (url_activated_cb), NULL);
  g_signal_connect (G_OBJECT (mw->combo), "changed", 
		    GTK_SIGNAL_FUNC (combo_url_selected_cb), NULL);  
  g_signal_connect (G_OBJECT (completion), "match-selected", 
		    GTK_SIGNAL_FUNC (completion_url_selected_cb), NULL);

  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, 0);

  
  /* The connect button */
  item = gtk_tool_item_new ();
  mw->connect_button = gtk_toggle_button_new ();
  gtk_container_add (GTK_CONTAINER (item), mw->connect_button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);

  gtk_tooltips_set_tip (mw->tips, GTK_WIDGET (mw->connect_button), 
			_("Enter an URL to call on the left, and click on this button to connect to the given URL"), NULL);
  
  image = gtk_image_new_from_stock (GM_STOCK_DISCONNECT, 
                                    GTK_ICON_SIZE_LARGE_TOOLBAR);

  gtk_container_add (GTK_CONTAINER (mw->connect_button), GTK_WIDGET (image));
  g_object_set_data (G_OBJECT (mw->connect_button), "image", image);

  gtk_widget_set_size_request (GTK_WIDGET (mw->connect_button), 32, 32);

  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

  g_signal_connect (G_OBJECT (mw->connect_button), "clicked",
                    G_CALLBACK (toolbar_connect_button_clicked_cb), 
		    main_window);

  gtk_widget_show_all (GTK_WIDGET (toolbar));
  
#ifndef DISABLE_GNOME
  behavior = (BONOBO_DOCK_ITEM_BEH_EXCLUSIVE
	      | BONOBO_DOCK_ITEM_BEH_NEVER_VERTICAL);

  if (!toolbar_detachable)
    behavior |= BONOBO_DOCK_ITEM_BEH_LOCKED;

  gnome_app_add_docked (GNOME_APP (main_window), toolbar, "main_toolbar",
			BonoboDockItemBehavior (behavior),
  			BONOBO_DOCK_TOP, 1, 0, 0);
#else
  gtk_box_pack_start (GTK_BOX (mw->window_vbox), toolbar, 
		      FALSE, FALSE, 0);
#endif


  /* The left toolbar */
  toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_orientation (GTK_TOOLBAR (toolbar), 
			       GTK_ORIENTATION_VERTICAL);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (toolbar), FALSE);
  

  /* The text chat */
  item = gtk_tool_item_new ();
  button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  image = gtk_image_new_from_stock (GM_STOCK_TEXT_CHAT,
				    GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_container_add (GTK_CONTAINER (button), image);
  gtk_container_add (GTK_CONTAINER (item), button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (item), 
			     mw->tips, _("Open text chat"), NULL);
  
  gtk_widget_show (button);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), 
		      GTK_TOOL_ITEM (item), -1);

  g_signal_connect (G_OBJECT (button), "clicked",
		    GTK_SIGNAL_FUNC (toolbar_toggle_button_changed_cb),
		    (gpointer) USER_INTERFACE_KEY "main_window/show_chat_window");
  

  /* The control panel */
  item = gtk_tool_item_new ();
  button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  image = gtk_image_new_from_stock (GM_STOCK_CONTROL_PANEL,
				    GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_container_add (GTK_CONTAINER (button), image);
  gtk_container_add (GTK_CONTAINER (item), button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);
  
  gtk_widget_show (GTK_WIDGET (item));
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (item), 
			     mw->tips, _("Open control panel"), NULL);

  g_signal_connect (G_OBJECT (button), "clicked",
		    GTK_SIGNAL_FUNC (control_panel_button_clicked_cb), NULL);
  

  /* The address book */
  item = gtk_tool_item_new ();
  button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  image = gtk_image_new_from_stock (GM_STOCK_ADDRESSBOOK_24,
				    GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_container_add (GTK_CONTAINER (button), image);
  gtk_container_add (GTK_CONTAINER (item), button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);
  
  gtk_widget_show (GTK_WIDGET (item));
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (item), 
			     mw->tips, _("Open address book"), NULL);

  g_signal_connect (G_OBJECT (button), "clicked",
		    GTK_SIGNAL_FUNC (show_window_cb), 
		    (gpointer) addressbook_window);
  

  
  /* Video Preview Button */
  item = gtk_tool_item_new ();
  mw->preview_button = gtk_toggle_button_new ();
  image = gtk_image_new_from_stock (GM_STOCK_VIDEO_PREVIEW,
				    GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_container_add (GTK_CONTAINER (mw->preview_button), image);
  gtk_container_add (GTK_CONTAINER (item), mw->preview_button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);
  
  gtk_widget_show (mw->preview_button);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), 
		      GTK_TOOL_ITEM (item), -1);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (item), mw->tips,
			     _("Display images from your camera device"), 
			     NULL);

  gtk_widget_set_size_request (GTK_WIDGET (mw->preview_button), 28, 28);

  GTK_TOGGLE_BUTTON (mw->preview_button)->active =
    gm_conf_get_bool (VIDEO_DEVICES_KEY "enable_preview");

  g_signal_connect (G_OBJECT (mw->preview_button), "toggled",
		    G_CALLBACK (toolbar_toggle_button_changed_cb),
		    (gpointer) VIDEO_DEVICES_KEY "enable_preview");


  /* Audio Channel Button */
  item = gtk_tool_item_new ();
  mw->audio_chan_button = gtk_toggle_button_new ();
  image = gtk_image_new_from_stock (GM_STOCK_AUDIO_MUTE,
				    GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_container_add (GTK_CONTAINER (mw->audio_chan_button), image);
  gtk_container_add (GTK_CONTAINER (item), mw->audio_chan_button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);
  
  gtk_widget_show (mw->audio_chan_button);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), 
		      GTK_TOOL_ITEM (item), -1);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (item), mw->tips,
			     _("Audio transmission status. During a call, click here to suspend or resume the audio transmission."), NULL);

  gtk_widget_set_size_request (GTK_WIDGET (mw->audio_chan_button), 28, 28);
  gtk_widget_set_sensitive (GTK_WIDGET (mw->audio_chan_button), FALSE);

  g_signal_connect (G_OBJECT (mw->audio_chan_button), "clicked",
		    G_CALLBACK (pause_current_call_channel_cb), 
		    GINT_TO_POINTER (0));


  /* Video Channel Button */
  item = gtk_tool_item_new ();
  mw->video_chan_button = gtk_toggle_button_new ();
  image = gtk_image_new_from_stock (GM_STOCK_VIDEO_MUTE,
				    GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_container_add (GTK_CONTAINER (mw->video_chan_button), image);
  gtk_container_add (GTK_CONTAINER (item), mw->video_chan_button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);
  
  gtk_widget_show (mw->video_chan_button);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), 
		      GTK_TOOL_ITEM (item), -1);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (item), mw->tips,
			     _("Video transmission status. During a call, click here to suspend or resume the video transmission."), NULL);

  gtk_widget_set_size_request (GTK_WIDGET (mw->video_chan_button), 28, 28);
  gtk_widget_set_sensitive (GTK_WIDGET (mw->video_chan_button), FALSE);

  g_signal_connect (G_OBJECT (mw->video_chan_button), "clicked",
		    G_CALLBACK (pause_current_call_channel_cb), 
		    GINT_TO_POINTER (1));


  /* Add the toolbar to the UI */
#ifndef DISABLE_GNOME
  behavior = BONOBO_DOCK_ITEM_BEH_EXCLUSIVE;

  if (!toolbar_detachable)
    behavior |= BONOBO_DOCK_ITEM_BEH_LOCKED;

  gnome_app_add_toolbar (GNOME_APP (main_window), GTK_TOOLBAR (toolbar),
 			 "left_toolbar", 
			 BonoboDockItemBehavior (behavior),
 			 BONOBO_DOCK_LEFT, 2, 0, 0);
#else
  gtk_box_pack_start (GTK_BOX (mw->window_hbox), toolbar, 
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
  GtkWidget *pc2phone_window = NULL;
  
  IncomingCallMode icm = AVAILABLE;
  ControlPanelSection cps = CLOSED;
  bool show_chat_window = false;
  int nbr = 0;

  GSList *glist = NULL;

  g_return_if_fail (main_window != NULL);
  mw = gm_mw_get_mw (main_window);
  
  addressbook_window = GnomeMeeting::Process ()->GetAddressbookWindow ();
  calls_history_window = GnomeMeeting::Process ()->GetCallsHistoryWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  druid_window = GnomeMeeting::Process ()->GetDruidWindow ();
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();
  pc2phone_window = GnomeMeeting::Process ()->GetPC2PhoneWindow ();

  mw->main_menu = gtk_menu_bar_new ();


  /* Default values */
  icm = (IncomingCallMode) 
    gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode"); 
  cps = (ControlPanelSection)
    gm_conf_get_int (USER_INTERFACE_KEY "main_window/control_panel_section"); 
  show_chat_window =
    gm_conf_get_bool (USER_INTERFACE_KEY "main_window/show_chat_window"); 

  
  static MenuEntry gnomemeeting_menu [] =
    {
      GTK_MENU_NEW (_("C_all")),

      GTK_MENU_ENTRY("connect", _("C_onnect"), _("Create a new connection"), 
		     GM_STOCK_CONNECT_16, 'o',
		     GTK_SIGNAL_FUNC (connect_cb), main_window, TRUE),
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
      GTK_MENU_RADIO_ENTRY("auto_answer", _("Aut_o Answer"),
			   _("Auto answer calls"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == AUTO_ANSWER), TRUE),
      GTK_MENU_RADIO_ENTRY("do_not_disturb", _("_Do Not Disturb"), 
			   _("Reject calls"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == DO_NOT_DISTURB), TRUE),
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
		     NULL, 't', 
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
		     (gpointer) main_window, TRUE),

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
      GTK_MENU_RADIO_ENTRY("both_incrusted", _("Both (Picture-in-Picture)"),
			   _("Both video images"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb), 
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   FALSE, FALSE),
      GTK_MENU_RADIO_ENTRY("both_side_by_side", _("Both (Side-by-Side)"),
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
		     GTK_SIGNAL_FUNC (zoom_in_changed_cb),
		     (gpointer) VIDEO_DISPLAY_KEY "zoom_factor", FALSE),
      GTK_MENU_ENTRY("zoom_out", _("Zoom Out"), _("Zoom out"), 
		     GTK_STOCK_ZOOM_OUT, '-', 
		     GTK_SIGNAL_FUNC (zoom_out_changed_cb),
		     (gpointer) VIDEO_DISPLAY_KEY "zoom_factor", FALSE),
      GTK_MENU_ENTRY("normal_size", _("Normal Size"), _("Normal size"), 
		     GTK_STOCK_ZOOM_100, '=',
		     GTK_SIGNAL_FUNC (zoom_normal_changed_cb),
		     (gpointer) VIDEO_DISPLAY_KEY "zoom_factor", FALSE),

#ifdef HAS_SDL
      GTK_MENU_ENTRY("fullscreen", _("Fullscreen"), _("Switch to fullscreen"), 
		     GTK_STOCK_ZOOM_IN, 'f', 
		     GTK_SIGNAL_FUNC (fullscreen_changed_cb),
		     (gpointer) main_window, FALSE),
#endif

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

      GTK_MENU_ENTRY("pc-to-phone", _("PC-To-Phone Account"),
		     _("Manage your PC-To-Phone account"),
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) pc2phone_window, TRUE),
      
      GTK_MENU_NEW(_("_Help")),

#ifndef DISABLE_GNOME
       GTK_MENU_ENTRY("help", _("_Contents"),
                     _("Get help by reading the GnomeMeeting manual"),
                     GTK_STOCK_HELP, GDK_F1, 
                     GTK_SIGNAL_FUNC (help_cb), NULL, TRUE),
       
      GTK_MENU_ENTRY("about", _("_About"),
		     _("View information about GnomeMeeting"),
		     GNOME_STOCK_ABOUT, 'a', 
		     GTK_SIGNAL_FUNC (about_callback), (gpointer) main_window,
		     TRUE),
#else
      GTK_MENU_ENTRY("help", _("_Contents"),
                     _("Get help by reading the GnomeMeeting manual"),
                     GTK_STOCK_HELP, GDK_F1, 
                     NULL, NULL, FALSE),
       
      GTK_MENU_ENTRY("about", _("_About"),
		     _("View information about GnomeMeeting"),
		     NULL, 'a', 
		     GTK_SIGNAL_FUNC (about_callback), (gpointer) main_window,
		     TRUE),
#endif
       
      GTK_MENU_END
    };


  gtk_build_menu (mw->main_menu, 
		  gnomemeeting_menu, 
		  mw->accel, 
		  mw->statusbar);

  glist = 
    gnomemeeting_addressbook_get_contacts (NULL, nbr, 
					   FALSE, NULL, NULL, NULL, "*"); 
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
  
  GtkWidget *button = NULL;
  GtkWidget *label = NULL;
  GtkWidget *table = NULL;

  int i = 0;

  char *key_n [] = { "1", "2", "3", "4", "5", "6", "7", "8", "9",
		     "*", "0", "#"};
  guint key_kp [] = { GDK_KP_1, GDK_KP_2, GDK_KP_3, GDK_KP_4, GDK_KP_5, 
    		      GDK_KP_6, GDK_KP_7, GDK_KP_8, GDK_KP_9, GDK_KP_Multiply,
		      GDK_KP_0, GDK_numbersign};

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
   
    gtk_widget_add_accelerator (button, "activate", 
				mw->accel, key_kp [i], 
				(GdkModifierType) 0, (GtkAccelFlags) 0);
    
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
		    (gpointer) main_window);


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
		    (gpointer) main_window);


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
		    (gpointer) main_window);


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
		    G_CALLBACK (video_settings_changed_cb), 
		    (gpointer) main_window);
  

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


static void 
gm_mw_push_message (GtkWidget *main_window, 
		    gboolean flash_message,
		    gboolean info_message,
		    const char *msg, 
		    ...)
{
  GmWindow *mw = NULL;
  
  gint id = 0;
  gint msg_id = 0;
  int len = 0;
  
  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);
  
  
  len = g_slist_length ((GSList *) (GTK_STATUSBAR (mw->statusbar)->messages));
  if (info_message)
    id = gtk_statusbar_get_context_id (GTK_STATUSBAR (mw->statusbar), 
				       "info");
  else
    id = gtk_statusbar_get_context_id (GTK_STATUSBAR (mw->statusbar), 
				       "statusbar");
  
  for (int i = 0 ; i < len ; i++)
    gtk_statusbar_pop (GTK_STATUSBAR (mw->statusbar), id);

  if (msg) {

    va_list args;
    char buffer [1025];

    va_start (args, msg);
    vsnprintf (buffer, 1024, msg, args);

    msg_id = gtk_statusbar_push (GTK_STATUSBAR (mw->statusbar), id, buffer);

    va_end (args);

    if (flash_message)
      gtk_timeout_add (4000, statusbar_clear_msg_cb, 
		       GINT_TO_POINTER (msg_id));
  }
}


GtkWidget *
gm_mw_video_window_new (GtkWidget *main_window,
			gboolean is_local,
			gchar *title, 
			GtkWidget *&image,
			gchar *window_name)
{
  GmWindow *mw = NULL;
  
  GtkWidget *event_box = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *window = NULL;


  g_return_val_if_fail (main_window != NULL, NULL);
  mw = gm_mw_get_mw (main_window);
  g_return_val_if_fail (mw != NULL, NULL);
  

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), title);
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup (window_name), g_free);
  
  event_box = gtk_event_box_new ();
  vbox = gtk_vbox_new (0, FALSE);
  image = gtk_image_new ();

  gtk_box_pack_start (GTK_BOX (vbox), image, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (window), 0);

  gtk_container_add (GTK_CONTAINER (window), event_box);
  gtk_container_add (GTK_CONTAINER (event_box), vbox);
  gtk_widget_realize (image);

  gtk_widget_set_size_request (GTK_WIDGET (image), 176, 144);
  gtk_window_set_resizable (GTK_WINDOW (window), FALSE);

  gtk_widget_show_all (event_box);

  static MenuEntry local_popup_menu [] = {
    
      GTK_MENU_ENTRY("zoom_in", _("Zoom In"), _("Zoom in"), 
		     GTK_STOCK_ZOOM_IN, '+', 
		     GTK_SIGNAL_FUNC (zoom_in_changed_cb),
		     (gpointer) VIDEO_DISPLAY_KEY "local_zoom_factor", TRUE),
      GTK_MENU_ENTRY("zoom_out", _("Zoom Out"), _("Zoom out"), 
		     GTK_STOCK_ZOOM_OUT, '-', 
		     GTK_SIGNAL_FUNC (zoom_out_changed_cb),
		     (gpointer) VIDEO_DISPLAY_KEY "local_zoom_factor", TRUE),
      GTK_MENU_ENTRY("normal_size", _("Normal Size"), _("Normal size"), 
		     GTK_STOCK_ZOOM_100, '=',
		     GTK_SIGNAL_FUNC (zoom_normal_changed_cb),
		     (gpointer) VIDEO_DISPLAY_KEY "local_zoom_factor", TRUE),

#ifdef HAS_SDL
      GTK_MENU_ENTRY("fullscreen", _("Fullscreen"), _("Switch to fullscreen"), 
		     GTK_STOCK_ZOOM_IN, 'f', 
		     GTK_SIGNAL_FUNC (fullscreen_changed_cb),
		     (gpointer) main_window, TRUE),
#endif

      GTK_MENU_END
  };
  
  static MenuEntry remote_popup_menu [] = {
    
      GTK_MENU_ENTRY("zoom_in", _("Zoom In"), _("Zoom in"), 
		     GTK_STOCK_ZOOM_IN, '+', 
		     GTK_SIGNAL_FUNC (zoom_in_changed_cb),
		     (gpointer) VIDEO_DISPLAY_KEY "remote_zoom_factor", TRUE),
      GTK_MENU_ENTRY("zoom_out", _("Zoom Out"), _("Zoom out"), 
		     GTK_STOCK_ZOOM_OUT, '-', 
		     GTK_SIGNAL_FUNC (zoom_out_changed_cb),
		     (gpointer) VIDEO_DISPLAY_KEY "remote_zoom_factor", TRUE),
      GTK_MENU_ENTRY("normal_size", _("Normal Size"), _("Normal size"), 
		     GTK_STOCK_ZOOM_100, '=',
		     GTK_SIGNAL_FUNC (zoom_normal_changed_cb),
		     (gpointer) VIDEO_DISPLAY_KEY "remote_zoom_factor", TRUE),

#ifdef HAS_SDL
      GTK_MENU_ENTRY("fullscreen", _("Fullscreen"), _("Switch to fullscreen"), 
		     GTK_STOCK_ZOOM_IN, 'f', 
		     GTK_SIGNAL_FUNC (fullscreen_changed_cb),
		     (gpointer) main_window, TRUE),
#endif

      GTK_MENU_END
  };

  if (is_local)
    gtk_build_popup_menu (event_box, local_popup_menu, mw->accel);
  else
    gtk_build_popup_menu (event_box, remote_popup_menu, mw->accel);
  
  g_signal_connect (G_OBJECT (window), "delete_event",
		    G_CALLBACK (gtk_widget_hide_on_delete), 0);

  return window;
}


#ifdef HAS_SDL
void
gm_mw_init_fullscreen_video_window (GtkWidget *main_window)
{
  GmWindow *mw = NULL;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  mw->screen = SDL_GetVideoSurface ();
  if (!mw->screen) {

    SDL_Init (SDL_INIT_VIDEO);
    mw->screen = SDL_SetVideoMode (640, 480, 0, 
				   SDL_SWSURFACE | SDL_HWSURFACE | 
				   SDL_ANYFORMAT);
    SDL_WM_ToggleFullScreen (mw->screen);
    SDL_ShowCursor (SDL_DISABLE);
  }
}


gboolean
gm_mw_poll_fullscreen_video_window (GtkWidget *main_window)
{
  SDL_Event event;

  
  while (SDL_PollEvent (&event)) {

    if (event.type == SDL_KEYDOWN) {

      /* Exit Full Screen */
      if ((event.key.keysym.sym == SDLK_ESCAPE) ||
	  (event.key.keysym.sym == SDLK_f)) {

	return TRUE;
      }
    }
  }

  return FALSE;
}


void
gm_mw_toggle_fullscreen (GtkWidget *main_window)
{
  if (gm_conf_get_float (VIDEO_DISPLAY_KEY "zoom_factor") == -1.0)
    gm_conf_set_float (VIDEO_DISPLAY_KEY "zoom_factor", 1.0);
  else
    gm_conf_set_float (VIDEO_DISPLAY_KEY "zoom_factor", -1.0);
}


void
gm_mw_destroy_fullscreen_video_window (GtkWidget *main_window)
{
  GmWindow *mw = NULL;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  SDL_Quit ();
  mw->screen = NULL;
  
}
#endif


/* GTK callbacks */
static void 
hold_current_call_cb (GtkWidget *widget,
		      gpointer data)
{
  PString call_token;
  GMEndPoint *endpoint = NULL;

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
  GMEndPoint *endpoint = NULL;
  GMVideoGrabber *vg = NULL;

  GtkWidget *main_window = NULL;
 
  PString current_call_token;
  BOOL is_paused = FALSE;
  
  endpoint = GnomeMeeting::Process ()->Endpoint ();
  current_call_token = endpoint->GetCurrentCallToken ();

  main_window = GnomeMeeting::Process ()->GetMainWindow (); 

  if (current_call_token.IsEmpty ()
      && (GPOINTER_TO_INT (data) == 1)
      && endpoint->GetCallingState () == GMEndPoint::Standby) {

    gdk_threads_leave ();
    vg = endpoint->GetVideoGrabber ();
    if (vg && vg->IsGrabbing ()) {
      
      vg->StopGrabbing ();
      gm_main_window_set_channel_pause (main_window, TRUE, TRUE);
    }
    else {
      
      vg->StartGrabbing ();
      gm_main_window_set_channel_pause (main_window, FALSE, TRUE);
    }
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
  GMEndPoint *endpoint = NULL;

  endpoint = GnomeMeeting::Process ()->Endpoint ();

  if (endpoint 
      && gm_conf_get_bool (VIDEO_DISPLAY_KEY "stay_on_top")
      && endpoint->GetCallingState () == GMEndPoint::Connected)
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
     if (GnomeMeeting::Process ()->Endpoint ()->GetCallingState () == GMEndPoint::Standby) {
       
       /* this function will store a copy of text */
       //gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (mw->combo)->entry),
	//		   PString (contact->url));
       
       //gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mw->connect_button),
	//			     true);
//FIXME
     }
     gm_contact_delete (contact);
  }
}


static void 
audio_volume_changed_cb (GtkAdjustment *adjustment, 
			 gpointer data)
{
  GMEndPoint *ep = NULL;
  GMPCSSEndPoint *pcssEP = NULL;
  
  int play_vol = 0; 
  int rec_vol = 0;

  g_return_if_fail (data != NULL);

  ep = GnomeMeeting::Process ()->Endpoint ();
  pcssEP = ep->GetPCSSEndPoint ();
  
  gm_main_window_get_volume_sliders_values (GTK_WIDGET (data), 
					    play_vol, rec_vol);

  gdk_threads_leave ();
  pcssEP->SetDeviceVolume (play_vol, rec_vol);
  gdk_threads_enter ();
}


static void 
video_settings_changed_cb (GtkAdjustment *adjustment, 
			   gpointer data)
{ 
  GMEndPoint *ep = NULL;
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

  BOOL sent = FALSE;
  PString call_token;
  PString url;

  GMEndPoint *endpoint = NULL;

  g_return_if_fail (data != NULL);


  endpoint = GnomeMeeting::Process ()->Endpoint ();

  label = gtk_bin_get_child (GTK_BIN (button));
  button_text = gtk_label_get_text (GTK_LABEL (label));

  if (button_text
      && strcmp (button_text, "")
      && strlen (button_text) > 1
      && button_text [0]) {

    /* Release the GDK thread to prevent deadlocks */
    gdk_threads_leave ();
    call_token = endpoint->GetCurrentCallToken ();

    /* Send the DTMF if there is a current call */
    if (!call_token.IsEmpty ()) {
      
      endpoint->SendDTMF (call_token, button_text [0]);
      sent = TRUE;
    }
    gdk_threads_enter ();


    /* Update the GUI, ie the URL bar if we are not in a call,
     * and a button press in all cases */
    if (!sent) {

      url = gm_main_window_get_call_url (GTK_WIDGET (data));
      if (button_text [0] == '*')
	url += '.';
      else
	url += button_text [0];
      
      gm_main_window_set_call_url (GTK_WIDGET (data), url);
    }
    else
      gm_main_window_flash_message (GTK_WIDGET (data),
				    _("Sent DTMF %s"), button_text [0]);
  }
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
    gnomemeeting_window_hide (GTK_WIDGET (data));

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
zoom_in_changed_cb (GtkWidget *widget,
		    gpointer data)
{
  double zoom = 0.0;

  g_return_if_fail (data != NULL);

  zoom = gm_conf_get_float ((char *) data);

  if (zoom < 2.00)
    zoom = zoom * 2.0;

  gm_conf_set_float ((char *) data, zoom);
}


static void 
zoom_out_changed_cb (GtkWidget *widget,
		     gpointer data)
{
  double zoom = 0.0;

  g_return_if_fail (data != NULL);

  zoom = gm_conf_get_float ((char *) data);

  if (zoom > 0.5)
    zoom = zoom / 2.0;
  
  gm_conf_set_float ((char *) data, zoom);
}


static void 
zoom_normal_changed_cb (GtkWidget *widget,
			gpointer data)
{
  double zoom = 1.0;

  g_return_if_fail (data != NULL);

  gm_conf_set_float ((char *) data, zoom);
}


#ifdef HAS_SDL
static void 
fullscreen_changed_cb (GtkWidget *widget,
		       gpointer data)
{
  gm_mw_toggle_fullscreen (GTK_WIDGET (data));
}
#endif


static void
speed_dial_menu_item_selected_cb (GtkWidget *w,
				  gpointer data)
{
  GtkWidget *main_window = NULL;
  
  GmWindow *mw = NULL;
  GMEndPoint *ep = NULL;
  
  gchar *url = NULL;
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  
  mw = gm_mw_get_mw (main_window); 
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  g_return_if_fail (data != NULL);

  url = g_strdup_printf ("%s#", (gchar *) data);
  gm_main_window_set_call_url (main_window, url);
    

  /* Directly Connect or run the transfer dialog */
  if (ep->GetCallingState () == GMEndPoint::Connected)
    gm_main_window_transfer_dialog_run (main_window, url);
  else
    GnomeMeeting::Process ()->Connect (url);

  g_free (url);
}


static void
url_changed_cb (GtkEditable  *e, 
		gpointer data)
{
  GmWindow *mw = NULL;
  
  const char *tip_text = NULL;
  
  g_return_if_fail (data != NULL);
  mw = gm_mw_get_mw (GTK_WIDGET (data));

  tip_text = gtk_entry_get_text (GTK_ENTRY (e));

  gtk_tooltips_set_tip (mw->tips, GTK_WIDGET (mw->combo), tip_text, NULL);
}


static gboolean
completion_url_selected_cb (GtkEntryCompletion *completion,
			    GtkTreeModel *model,
			    GtkTreeIter *iter,
			    gpointer data)
{
  gchar *url = NULL;
  
  gtk_tree_model_get (GTK_TREE_MODEL (model), iter, 1, &url, -1);

  GnomeMeeting::Process ()->Connect (url);

  g_free (url);

  return TRUE;
}


static void
combo_url_selected_cb (GtkComboBox *widget,
		       gpointer data)
{
  const char *url = NULL;
  
  url = gtk_entry_get_text (GTK_ENTRY (GTK_BIN (widget)->child));

  GnomeMeeting::Process ()->Connect (url);  
}


static void 
url_activated_cb (GtkWidget *w,
		  gpointer data)
{
  const char *url = NULL;
  
  url = gtk_entry_get_text (GTK_ENTRY (w));
  
  GnomeMeeting::Process ()->Connect (url);
}


static gboolean 
found_url_cb (GtkEntryCompletion *completion,
	      const gchar *key,
	      GtkTreeIter *iter,
	      gpointer data)
{
  GtkListStore *list_store = NULL;
  GtkTreeIter tree_iter;
  
  GtkTreePath *current_path = NULL;
  GtkTreePath *path = NULL;
    
  gchar *val = NULL;
  gchar *entry = NULL;
  gchar *tmp_entry = NULL;
  
  PCaselessString s;

  PINDEX j = 0;
  BOOL found = FALSE;
  
  g_return_val_if_fail (data != NULL, FALSE);
  
  list_store = GTK_LIST_STORE (data);

  if (!key || GMURL (key).GetCanonicalURL ().GetLength () < 2)
    return FALSE;

  for (int i = 0 ; (i < 2 && !found) ; i++) {
    
    gtk_tree_model_get (GTK_TREE_MODEL (list_store), iter, i, &val, -1);
    s = val;
    /* Check if one of the names matches the canonical form of the URL */
    if (i == 0) {
      
      j = s.Find (GMURL (key).GetCanonicalURL ());

      if (j != P_MAX_INDEX && j > 0) {

	char c = s [j - 1];
	
	found = (c == 32);
      }
      else if (j == 0)
	found = TRUE;
      else
	found = FALSE;
    }
    /* Check if both GMURLs match */
    else if (i == 1 && GMURL(s).Find (GMURL (key))) 
      found = TRUE;

    g_free (val);
  }
  
  if (!found)
    return FALSE;
  
  /* We have found something, but is it the first item ? */
  gtk_tree_model_get (GTK_TREE_MODEL (list_store), iter, 2, &entry, -1);

  if (found) {

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list_store),
				       &tree_iter)) {

      do {

	gtk_tree_model_get (GTK_TREE_MODEL (list_store), &tree_iter, 
			    2, &tmp_entry, -1);

	if (tmp_entry && !strcmp (tmp_entry, entry)) {

	  current_path = 
	    gtk_tree_model_get_path (GTK_TREE_MODEL (list_store),
				     iter);
	  path = 
	    gtk_tree_model_get_path (GTK_TREE_MODEL (list_store), 
				     &tree_iter);

	  if (gtk_tree_path_compare (path, current_path) < 0) 
	    found = FALSE;

	  gtk_tree_path_free (path);
	  gtk_tree_path_free (current_path);
	}
	
      } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list_store), 
					 &tree_iter) && found);

    }
  }
  
  g_free (entry);

  return found;
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
  PString url;

  g_return_if_fail (data != NULL);

  url = gm_main_window_get_call_url (GTK_WIDGET (data));
  
  if (gtk_toggle_button_get_active (w)) {
  
    if (!GMURL (url).IsEmpty ())
      GnomeMeeting::Process ()->Connect (url);
    else
      gm_mw_update_connect_button (GTK_WIDGET (data), FALSE);
  }
  else
    GnomeMeeting::Process ()->Disconnect ();
}


static int 
statusbar_clear_msg_cb (gpointer data)
{
  GtkWidget *main_window = NULL;
  
  GmWindow *mw = NULL;
  int id = 0;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  mw = gm_mw_get_mw (GTK_WIDGET (main_window));
  
  g_return_val_if_fail (mw != NULL, FALSE);
  
  gdk_threads_enter ();

  id = gtk_statusbar_get_context_id (GTK_STATUSBAR (mw->statusbar),
				     "statusbar");

  gtk_statusbar_remove (GTK_STATUSBAR (mw->statusbar), id, 
			GPOINTER_TO_INT (data));

  gdk_threads_leave ();

  return FALSE;
}


static gboolean 
statusbar_clicked_cb (GtkWidget *widget,
		      GdkEventButton *event,
		      gpointer data)
{
  GmWindow *mw = NULL;

  GMEndPoint *ep = NULL;
  
  gint len = 0;
  gint id = 0;
  
  g_return_val_if_fail (data != NULL, FALSE);

  mw = gm_mw_get_mw (GTK_WIDGET (data));

  g_return_val_if_fail (mw != NULL, FALSE);
  
  ep = GnomeMeeting::Process ()->Endpoint ();

  len = g_slist_length ((GSList *) (GTK_STATUSBAR (mw->statusbar)->messages));
  id = gtk_statusbar_get_context_id (GTK_STATUSBAR (mw->statusbar), 
				     "info");
  
  for (int i = 0 ; i < len ; i++)
    gtk_statusbar_pop (GTK_STATUSBAR (mw->statusbar), id);

  ep->ResetMissedCallsNumber ();

  return FALSE;
}


static gboolean
delete_incoming_call_dialog_cb (GtkWidget *w,
				GdkEvent *ev,
				gpointer data)
{
  GmWindow *mw = NULL;

  g_return_val_if_fail (data != NULL, TRUE);

  mw = gm_mw_get_mw (GTK_WIDGET (data));

  g_return_val_if_fail (GTK_WIDGET (data), TRUE);

  mw->incoming_call_popup = NULL;

  GnomeMeeting::Process ()->Disconnect ();
  
  return FALSE;
}


/* Public functions */
void 
gm_main_window_press_dialpad (GtkWidget *main_window,
			      const char c)
{
  guint key = 0;

  if (c == '*')
    key = GDK_KP_Multiply;
  else if (c == '#')
    key = GDK_numbersign;
  else
    key = GDK_KP_0 + atoi (&c);

  gtk_accel_groups_activate (G_OBJECT (main_window), key, (GdkModifierType) 0);
}


void 
gm_main_window_update_video (GtkWidget *main_window,
			     const guchar *lbuffer,
			     int lf_width,
			     int lf_height,
			     double lzoom,
			     const guchar *rbuffer,
			     int rf_width,
			     int rf_height,
			     double rzoom,
			     int display_type,
			     gboolean bilinear_filtering)
{
  GmWindow *mw = NULL;

  GdkPixbuf *lsrc_pic = NULL;
  GdkPixbuf *zlsrc_pic = NULL;
  GdkPixbuf *rsrc_pic = NULL;
  GdkPixbuf *zrsrc_pic = NULL;

#ifdef HAS_SDL
  Uint32 rmask, gmask, bmask, amask = 0;
  SDL_Surface *lsurface = NULL;
  SDL_Surface *rsurface = NULL;
  SDL_Surface *lblit_conf = NULL;
  SDL_Surface *rblit_conf = NULL;
  SDL_Rect dest;
#endif
  
  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);
  
  /* Update the display selection in the main and in the video popup menus */
  gtk_radio_menu_select_with_id (mw->main_menu, "local_video", display_type);


#ifdef HAS_SDL
  if (display_type != FULLSCREEN)
      gm_mw_destroy_fullscreen_video_window (main_window);
#endif

  /* Select and show the correct windows */
  if (display_type == BOTH) { /* display == BOTH */

    /* Display the GnomeMeeting logo in the main window */
    gm_main_window_update_logo (main_window);

    if (!GTK_WIDGET_VISIBLE (mw->local_video_window))
      gnomemeeting_window_show (GTK_WIDGET (mw->local_video_window));
    if (!GTK_WIDGET_VISIBLE (mw->remote_video_window))
      gnomemeeting_window_show (GTK_WIDGET (mw->remote_video_window));
  }
#ifdef HAS_SDL
  else if (display_type == FULLSCREEN) {

    gm_mw_init_fullscreen_video_window (main_window);
    if (gm_mw_poll_fullscreen_video_window (main_window)) 
      gm_mw_toggle_fullscreen (main_window);
  }
#endif
  else {

    /* display_type != BOTH && display_type != BOTH_LOCAL */

    if (GTK_WIDGET_VISIBLE (mw->local_video_window))
      gnomemeeting_window_hide (GTK_WIDGET (mw->local_video_window));
    if (GTK_WIDGET_VISIBLE (mw->remote_video_window))
      gnomemeeting_window_hide (GTK_WIDGET (mw->remote_video_window));

  }

  
  /* The real size picture, if required */
  if (display_type != REMOTE_VIDEO && lbuffer) {
    
    if (lf_width > 0 && lf_height > 0) {

      lsrc_pic =  
	gdk_pixbuf_new_from_data (lbuffer, GDK_COLORSPACE_RGB, 
				  FALSE, 8, lf_width, lf_height, 
				  lf_width * 3, 
				  NULL, NULL);
      if (lzoom != 1.0 && lzoom > 0)
	zlsrc_pic = 
	  gdk_pixbuf_scale_simple (lsrc_pic, 
				   (int) (lf_width * lzoom),
				   (int) (lf_height * lzoom),
				   bilinear_filtering?
				   GDK_INTERP_BILINEAR:GDK_INTERP_NEAREST);
      else
	zlsrc_pic = gdk_pixbuf_copy (lsrc_pic);
    }
  }
  
  if (display_type != LOCAL_VIDEO && rbuffer) {
   
    if (rf_width > 0 && rf_height > 0) {

      rsrc_pic =  
	gdk_pixbuf_new_from_data (rbuffer, GDK_COLORSPACE_RGB, 
				  FALSE, 8, rf_width, rf_height, 
				  rf_width * 3, 
				  NULL, NULL);
      if (rzoom != 1.0 && rzoom > 0) 
	zrsrc_pic = 
	  gdk_pixbuf_scale_simple (rsrc_pic, 
				   (int) (rf_width * rzoom),
				   (int) (rf_height * rzoom),
				   bilinear_filtering?
				   GDK_INTERP_BILINEAR:GDK_INTERP_NEAREST);
      else
	zrsrc_pic = gdk_pixbuf_copy (rsrc_pic);

      g_object_unref (rsrc_pic);
    }
  }

  
  switch (display_type) {

  case LOCAL_VIDEO:
    if (zlsrc_pic) {
      
      gtk_widget_set_size_request (GTK_WIDGET (mw->video_frame),
				   (int) (lf_width * lzoom),
				   (int) (lf_height * lzoom));
      gtk_image_set_from_pixbuf (GTK_IMAGE (mw->main_video_image), 
				 GDK_PIXBUF (zlsrc_pic));
      g_object_unref (zlsrc_pic);
    }
    break;

  case REMOTE_VIDEO:
    if (zrsrc_pic) {
      
      gtk_widget_set_size_request (GTK_WIDGET (mw->video_frame),
				   (int) (rf_width * rzoom),
				   (int) (rf_height * rzoom));
      gtk_image_set_from_pixbuf (GTK_IMAGE (mw->main_video_image), 
				 GDK_PIXBUF (zrsrc_pic));
      g_object_unref (zrsrc_pic);
    }
    break;

  case BOTH_INCRUSTED:

    if (zlsrc_pic && zrsrc_pic) {

      gdk_pixbuf_copy_area  (zlsrc_pic, 
			     0 , 0,
			     (int) (lf_width * lzoom), 
			     (int) (lf_height * lzoom),
			     zrsrc_pic,
			     (int) (rf_width * rzoom) 
			     - (int) (lf_width * lzoom), 
			     (int) (rf_height * rzoom) 
			     - (int) (lf_height * lzoom));

      gtk_widget_set_size_request (GTK_WIDGET (mw->video_frame),
				   (int) (rf_width * rzoom),
				   (int) (rf_height * rzoom));

      gtk_image_set_from_pixbuf (GTK_IMAGE (mw->main_video_image), 
				 GDK_PIXBUF (zrsrc_pic));
      g_object_unref (zrsrc_pic);
      g_object_unref (zlsrc_pic);
    }
    break;

  case BOTH_SIDE:

    if (zlsrc_pic && zrsrc_pic) {

      GdkPixbuf *tmp_pixbuf = 
	gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 
			(int) (rf_width * rzoom * 2), 
			(int) (rf_height * rzoom));

      gdk_pixbuf_copy_area (zrsrc_pic,
			    0, 0,
			    (int) (rf_width * rzoom), 
			    (int) (rf_height * rzoom),
			    tmp_pixbuf,
			    0, 0);
			    
      gdk_pixbuf_copy_area (zlsrc_pic,
			    0, 0,
			    (int) (rf_width * rzoom), 
			    (int) (rf_height * rzoom),
			    tmp_pixbuf,
			    (int) (rf_width * rzoom), 0);
      
      gtk_widget_set_size_request (GTK_WIDGET (mw->video_frame),
				   (int) (rf_width * rzoom * 2),
				   (int) (rf_height * rzoom));
      
      gtk_image_set_from_pixbuf (GTK_IMAGE (mw->main_video_image), 
				 GDK_PIXBUF (tmp_pixbuf));
      g_object_unref (zrsrc_pic);
      g_object_unref (zlsrc_pic);
      g_object_unref (tmp_pixbuf);
    }

    break;

  case BOTH:

    if (zlsrc_pic && zrsrc_pic) {

      gtk_widget_set_size_request (GTK_WIDGET (mw->remote_video_window),
				   (int) (rf_width * rzoom),
				   (int) (rf_height * rzoom));
      gtk_widget_set_size_request (GTK_WIDGET (mw->local_video_window),
				   (int) (lf_width * lzoom),
				   (int) (lf_height * lzoom));
      gtk_image_set_from_pixbuf (GTK_IMAGE (mw->remote_video_image), 
				 GDK_PIXBUF (zrsrc_pic));
      gtk_image_set_from_pixbuf (GTK_IMAGE (mw->local_video_image), 
				 GDK_PIXBUF (zlsrc_pic));
      
      g_object_unref (zrsrc_pic);
      g_object_unref (zlsrc_pic);
    }
    break;
    
#ifdef HAS_SDL
  case FULLSCREEN:

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    if (zrsrc_pic) {

      rsurface =
	SDL_CreateRGBSurfaceFrom ((void *) gdk_pixbuf_get_pixels (zrsrc_pic),
				  (int) (rf_width * rzoom),
				  (int) (rf_height * rzoom),
				  24,
				  (int) (rf_width * rzoom * 3), 
				  rmask, gmask, bmask, amask);

      rblit_conf = SDL_DisplayFormat (rsurface);

      if (zlsrc_pic)
	dest.x = (int) (mw->screen->w - (int) (rf_width * rzoom) - (int) (lf_width * lzoom) - 50) / 2;
      else
	dest.x = (int) (mw->screen->w - (int) (rf_width * rzoom)) / 2;
	
      dest.y = (int) (mw->screen->h - (int) (rf_height * rzoom)) / 2;
      dest.w = (int) (rf_width * rzoom);
      dest.h = (int) (rf_height * rzoom);

      SDL_BlitSurface (rblit_conf, NULL, mw->screen, &dest);

      SDL_FreeSurface (rsurface);
      SDL_FreeSurface (rblit_conf);

      g_object_unref (zrsrc_pic);

      if (zlsrc_pic) {

	lsurface =
	  SDL_CreateRGBSurfaceFrom ((void *) gdk_pixbuf_get_pixels (zlsrc_pic),
				    (int) (lf_width * lzoom),
				    (int) (lf_height * lzoom),
				    24,
				    (int) (lf_width * lzoom * 3), 
				    rmask, gmask, bmask, amask);

	lblit_conf = SDL_DisplayFormat (lsurface);

	dest.x = 640 - (int) (lf_width * lzoom);
	dest.y = 480 - (int) (lf_height * lzoom);
	dest.w = (int) (lf_width * lzoom);
	dest.h = (int) (lf_height * lzoom);

	SDL_BlitSurface (lblit_conf, NULL, mw->screen, &dest);

	SDL_FreeSurface (lsurface);
	SDL_FreeSurface (lblit_conf);

	g_object_unref (zlsrc_pic);
      }
    }

    SDL_UpdateRect (mw->screen, 0, 0, 640, 480);


    break;
#endif
  } 
}


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
    
    gtk_menu_set_sensitive (mw->main_menu, "suspend_audio", FALSE);
    gtk_menu_set_sensitive (mw->main_menu, "suspend_video", FALSE);
    
    gm_main_window_set_channel_pause (main_window, TRUE, FALSE);
    gm_main_window_set_channel_pause (main_window, TRUE, TRUE);
  }
  else {

    if (GTK_IS_LABEL (child))
      gtk_label_set_text_with_mnemonic (GTK_LABEL (child),
					_("_Hold Call"));

    gtk_widget_set_sensitive (GTK_WIDGET (mw->audio_chan_button), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (mw->video_chan_button), TRUE);
    
    gtk_menu_set_sensitive (mw->main_menu, "suspend_audio", TRUE);
    gtk_menu_set_sensitive (mw->main_menu, "suspend_video", TRUE);

    gm_main_window_set_channel_pause (main_window, FALSE, FALSE);
    gm_main_window_set_channel_pause (main_window, FALSE, TRUE);
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
gm_main_window_update_calling_state (GtkWidget *main_window,
				     unsigned calling_state)
{
  GmWindow *mw = NULL;
  
  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw!= NULL);


  switch (calling_state)
    {
    case GMEndPoint::Standby:

      
      /* Update the hold state */
      gm_main_window_set_call_hold (main_window, FALSE);


      /* Update the sensitivity, all channels are closed */
      gm_main_window_update_sensitivity (main_window, TRUE, FALSE, FALSE);
      gm_main_window_update_sensitivity (main_window, FALSE, FALSE, FALSE);

      
      /* Update the menus and toolbar items */
      gtk_menu_set_sensitive (mw->main_menu, "connect", TRUE);
      gtk_menu_set_sensitive (mw->main_menu, "disconnect", FALSE);
      gtk_menu_section_set_sensitive (mw->main_menu, "hold_call", FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (mw->preview_button), TRUE);

      
      /* Update the connect button */
      gm_mw_update_connect_button (main_window, FALSE);
      
	
      /* Destroy the incoming call popup */
      if (mw->incoming_call_popup) {

	gtk_widget_destroy (mw->incoming_call_popup);
	mw->incoming_call_popup = NULL;
      }

      /* Destroy the transfer call popup */
      if (mw->transfer_call_popup) 
	gtk_dialog_response (GTK_DIALOG (mw->transfer_call_popup),
			     GTK_RESPONSE_REJECT);
  
      
      /* Delete the full screen window */
#ifdef HAS_SDL
      gm_mw_destroy_fullscreen_video_window (main_window);
#endif
      
      
      /* Hide the local and remove video windows */
      gnomemeeting_window_hide (mw->remote_video_window);
      gnomemeeting_window_hide (mw->local_video_window);
	
      break;


    case GMEndPoint::Calling:

      /* Update the menus and toolbar items */
      gtk_menu_set_sensitive (mw->main_menu, "connect", FALSE);
      gtk_menu_set_sensitive (mw->main_menu, "disconnect", TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (mw->preview_button), FALSE);

      /* Update the connect button */
      gm_mw_update_connect_button (main_window, TRUE);
      
      break;


    case GMEndPoint::Connected:

      /* Update the menus and toolbar items */
      gtk_menu_set_sensitive (mw->main_menu, "connect", FALSE);
      gtk_menu_set_sensitive (mw->main_menu, "disconnect", TRUE);
      gtk_menu_section_set_sensitive (mw->main_menu, "hold_call", TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (mw->preview_button), FALSE);

      /* Update the connect button */
      gm_mw_update_connect_button (main_window, TRUE);
      
      /* Destroy the incoming call popup */
      if (mw->incoming_call_popup) {

	gtk_widget_destroy (mw->incoming_call_popup);
	mw->incoming_call_popup = NULL;
      }
      break;


    case GMEndPoint::Called:

      /* Update the menus and toolbar items */
      gtk_menu_set_sensitive (mw->main_menu, "disconnect", TRUE);

      /* Update the connect button */
      gm_mw_update_connect_button (main_window, FALSE);
      
      break;
    }
}


void
gm_main_window_update_sensitivity (GtkWidget *main_window,
				   BOOL is_video,
				   BOOL is_receiving,
				   BOOL is_transmitting)
{
  GmWindow *mw = NULL;
  
  GtkWidget *button = NULL;
  GtkWidget *frame = NULL;

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  
  /* We are updating video related items */
  if (is_video) {

  
    /* Receiving and sending => Everything sensitive in the section control */
    if (is_receiving && is_transmitting) {

      gtk_menu_section_set_sensitive (mw->main_menu,
				      "local_video", TRUE);
      gtk_menu_section_set_sensitive (mw->main_menu,
				      "zoom_in", TRUE);
    }
    else { /* Not receiving or not sending or both */


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
	if (!is_receiving)
	  gtk_menu_section_set_sensitive (mw->main_menu,
					  "fullscreen", FALSE);
	  
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


void 
gm_main_window_show_chat_window (GtkWidget *main_window,
				 gboolean show)
{
  GmWindow *mw = NULL;
  
  GtkWidget *menu = NULL;
  GtkWidget *chat_window = NULL;
  
  g_return_if_fail (main_window != NULL);
  
  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  chat_window = GnomeMeeting::Process ()->GetChatWindow ();


  menu = gtk_menu_get_widget (mw->main_menu, "text_chat");
  
  if (show) 
    gtk_widget_show_all (chat_window);
  else
    gtk_widget_hide_all (chat_window);
  
  gtk_toggle_menu_enable (menu, show);
}


void 
gm_main_window_show_control_panel_section (GtkWidget *main_window,
					   int section)
{
  GmWindow *mw = NULL;
  
  GtkWidget *menu = NULL;
  
  g_return_if_fail (main_window != NULL);
  
  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  if (section == GM_MAIN_NOTEBOOK_HIDDEN)
    gtk_widget_hide_all (mw->main_notebook);
  else {

    gtk_widget_show_all (mw->main_notebook);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (mw->main_notebook), section);
  }

  
  menu = gtk_menu_get_widget (mw->main_menu, "statistics");
  
  gtk_radio_menu_select_with_widget (GTK_WIDGET (menu), section);
}


void 
gm_main_window_set_incoming_call_mode (GtkWidget *main_window,
				       IncomingCallMode i)
{
  GmWindow *mw = NULL;
  
  GtkWidget *menu = NULL;
  
  g_return_if_fail (main_window != NULL);
  
  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  
  menu = gtk_menu_get_widget (mw->main_menu, "available");
  
  gtk_radio_menu_select_with_widget (GTK_WIDGET (menu), i);
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
  
  item = gtk_menu_get_attach_widget (GTK_MENU (menu));
  if (!g_slist_length (glist)) {

    gtk_widget_set_sensitive (item, FALSE);
    return;
  }
  gtk_widget_set_sensitive (item, TRUE);

  glist_iter = glist;
  while (glist_iter && glist_iter->data) {

    contact = GM_CONTACT (glist_iter->data);

    ml = g_strdup_printf ("<b>%s#</b> <i>%s</i>", 
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
gm_main_window_urls_history_update (GtkWidget *main_window)
{
  GmWindow *mw = NULL;

  GmContact *c = NULL;

  GValue val = {0, };

  GtkTreeModel *history_model = NULL;
  GtkTreeModel *cache_model = NULL;
  GtkEntryCompletion *completion = NULL;
  
  GtkTreeIter tree_iter;
  
  GSList *c1 = NULL;
  GSList *c2 = NULL;
  GSList *contacts = NULL;
  GSList *iter = NULL;

  unsigned int cpt = 0;
  int nbr = 0;

  gchar *entry = NULL;
  
  g_return_if_fail (main_window != NULL);
  
  mw = gm_mw_get_mw (main_window);
  
  
  /* Get the placed calls history */
  g_signal_handlers_block_by_func (G_OBJECT (mw->combo), 
				   (void *) combo_url_selected_cb, NULL);

  g_value_init (&val, G_TYPE_INT);
  g_value_set_int (&val, -1);
  g_object_set_property (G_OBJECT (mw->combo), "active", &val);

  c2 = gm_calls_history_get_calls (PLACED_CALL, 10, TRUE);

  history_model = 
    gtk_combo_box_get_model (GTK_COMBO_BOX (mw->combo));
  gtk_list_store_clear (GTK_LIST_STORE (history_model));

  iter = c2;
  while (iter) {
    
    c = GM_CONTACT (iter->data);
    if (c->url && strcmp (c->url, "")) {

      gtk_combo_box_prepend_text (GTK_COMBO_BOX (mw->combo), c->url);
      cpt++;
    }
    
    iter = g_slist_next (iter);
  }
  g_slist_foreach (c2, (GFunc) gm_contact_delete, NULL);
  g_slist_free (c2);
  c2 = NULL;
  
  g_signal_handlers_unblock_by_func (G_OBJECT (mw->combo), 
				     (void *) combo_url_selected_cb, NULL);
  

  /* Get the full address book */
  c1 = gnomemeeting_addressbook_get_contacts (NULL,
					      nbr,
					      FALSE,
					      NULL,
					      NULL,
					      NULL,
					      NULL);
  
  
  /* Get the full calls history */
  c2 = gm_calls_history_get_calls (MAX_VALUE_CALL, -1, FALSE);
  contacts = g_slist_concat (c1, c2);

  completion = 
    gtk_entry_get_completion (GTK_ENTRY (GTK_BIN (mw->combo)->child));
  cache_model = 
    gtk_entry_completion_get_model (GTK_ENTRY_COMPLETION (completion));
  gtk_list_store_clear (GTK_LIST_STORE (cache_model));


  iter = contacts;
  while (iter) {

    c = GM_CONTACT (iter->data);
    if (c->url && strcmp (c->url, "")) {

      entry = NULL;

      if (c->fullname && strcmp (c->fullname, ""))
	entry = g_strdup_printf ("%s [%s]",
				 c-> url, 
				 c->fullname);
      else
	entry = g_strdup (c->url);
      
      gtk_list_store_append (GTK_LIST_STORE (cache_model), &tree_iter);
      gtk_list_store_set (GTK_LIST_STORE (cache_model), &tree_iter, 
			  0, c->fullname,
			  1, c->url,
			  2, (char *) entry, -1);

      g_free (entry);
    }
    
    iter = g_slist_next (iter);
  }

  g_slist_foreach (contacts, (GFunc) gm_contact_delete, NULL);
  g_slist_free (contacts);
}


void 
gm_main_window_transfer_dialog_run (GtkWidget *main_window,
				    gchar *u)
{
  GMEndPoint *endpoint = NULL;
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


void 
gm_main_window_incoming_call_dialog_show (GtkWidget *main_window,
					  gchar *utf8_name, 
					  gchar *utf8_app,
					  gchar *utf8_url)
{
  GmWindow *mw = NULL;
  
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *b1 = NULL;
  GtkWidget *b2 = NULL;
  GtkWidget *b3 = NULL;

  gchar *msg = NULL;

  g_return_if_fail (main_window);
  
  mw = gm_mw_get_mw (main_window);
  
  g_return_if_fail (mw != NULL);


  mw->incoming_call_popup = gtk_dialog_new ();
  b2 = gtk_dialog_add_button (GTK_DIALOG (mw->incoming_call_popup),
			      _("Reject"), GTK_RESPONSE_REJECT);
  b3 = gtk_dialog_add_button (GTK_DIALOG (mw->incoming_call_popup),
			      _("Transfer"), GTK_RESPONSE_OK);
  b1 = gtk_dialog_add_button (GTK_DIALOG (mw->incoming_call_popup),
			      _("Accept"), GTK_RESPONSE_ACCEPT);

  gtk_dialog_set_default_response (GTK_DIALOG (mw->incoming_call_popup), 
				   GTK_RESPONSE_ACCEPT);

  vbox = GTK_DIALOG (mw->incoming_call_popup)->vbox;

  msg = g_strdup_printf ("%s <i>%s</i>",
			 _("Incoming call from"), (const char*) utf8_name);
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), msg);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 10);
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.0);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  g_free (msg);

  if (utf8_url) {
    
    label = gtk_label_new (NULL);
    msg =
      g_strdup_printf ("<b>%s</b> <span foreground=\"blue\"><u>%s</u></span>",
		       _("Remote URL:"), utf8_url);
    gtk_label_set_markup (GTK_LABEL (label), msg);
    gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 2);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
    g_free (msg);
  }

  if (utf8_app) {

    label = gtk_label_new (NULL);
    msg = g_strdup_printf ("<b>%s</b> %s",
			   _("Remote Application:"), utf8_app);
    gtk_label_set_markup (GTK_LABEL (label), msg);
    gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 2);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
    g_free (msg);
  }

  
  gtk_window_set_transient_for (GTK_WINDOW (mw->incoming_call_popup),
				GTK_WINDOW (main_window));

  
  g_signal_connect (G_OBJECT (b1), "clicked",
		    GTK_SIGNAL_FUNC (connect_cb), main_window);
  g_signal_connect (G_OBJECT (b2), "clicked",
		    GTK_SIGNAL_FUNC (disconnect_cb), NULL);
  g_signal_connect (G_OBJECT (b3), "clicked",
		    GTK_SIGNAL_FUNC (transfer_current_call_cb), main_window);
  
  g_signal_connect_swapped (G_OBJECT (b1), "clicked",
			    GTK_SIGNAL_FUNC (gtk_widget_hide), 
			    mw->incoming_call_popup);
  g_signal_connect_swapped (G_OBJECT (b2), "clicked",
			    GTK_SIGNAL_FUNC (gtk_widget_hide),
			    mw->incoming_call_popup);
  g_signal_connect_swapped (G_OBJECT (b3), "clicked",
			    GTK_SIGNAL_FUNC (gtk_widget_hide),
			    mw->incoming_call_popup);

  g_signal_connect (G_OBJECT (mw->incoming_call_popup), "delete-event",
		    GTK_SIGNAL_FUNC (delete_incoming_call_dialog_cb), 
		    main_window);
  
  gtk_widget_show_all (mw->incoming_call_popup);
}


GtkWidget *
gm_main_window_new ()
{
  GmWindow *mw = NULL;

  GtkWidget *window = NULL;
  GtkWidget *table = NULL;	
  GtkWidget *frame = NULL;
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
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("main_window"), g_free);

  gtk_window_set_title (GTK_WINDOW (window), 
			_("GnomeMeeting"));
  gtk_window_set_position (GTK_WINDOW (window), 
			   GTK_WIN_POS_CENTER);


  /* The GMObject data */
  mw = new GmWindow ();
  mw->incoming_call_popup = mw->transfer_call_popup = NULL;
#ifdef HAS_SDL
  mw->screen = NULL;
#endif
  g_object_set_data_full (G_OBJECT (window), "GMObject", 
			  mw, (GDestroyNotify) gm_mw_destroy);

  
  /* Tooltips and accelerators */
  mw->tips = gtk_tooltips_new ();
  mw->accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (window), mw->accel);

#ifdef DISABLE_GNOME
  mw->window_vbox = gtk_vbox_new (0, FALSE);
  gtk_container_add (GTK_CONTAINER (window), mw->window_vbox);
  gtk_widget_show_all (mw->window_vbox);

  /* The hbox */
  mw->window_hbox = gtk_hbox_new (0, FALSE);
  gtk_widget_show_all (mw->window_hbox);
#endif
  
  /* The main menu and the toolbars */
  gm_mw_init_menu (window); 
#ifdef DISABLE_GNOME
  gtk_box_pack_start (GTK_BOX (mw->window_vbox), mw->main_menu,
		      FALSE, FALSE, 0);
#endif
  
  gm_mw_init_toolbars (window);

#ifndef DISABLE_GNOME
  gnome_app_set_menus (GNOME_APP (window), 
		       GTK_MENU_BAR (mw->main_menu));
#else
  gtk_box_pack_start (GTK_BOX (mw->window_vbox), mw->window_hbox, 
		      FALSE, FALSE, 0);
#endif
  
  
  /* Create a table in the main window to attach things like buttons */
  table = gtk_table_new (3, 4, FALSE);
#ifdef DISABLE_GNOME
  gtk_box_pack_start (GTK_BOX (mw->window_hbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);
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

  
  /* The statusbar */
  event_box = gtk_event_box_new ();
  mw->statusbar = gtk_statusbar_new ();
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (mw->statusbar), FALSE);
  gtk_container_add (GTK_CONTAINER (event_box), mw->statusbar);

#ifdef DISABLE_GNOME
  gtk_box_pack_start (GTK_BOX (mw->window_vbox), event_box, 
		      FALSE, FALSE, 0);
#else
  gnome_app_set_statusbar_custom (GNOME_APP (window), event_box, mw->statusbar);
#endif
  gtk_widget_show_all (event_box);
  
  g_signal_connect (G_OBJECT (event_box), "button-press-event",
		    GTK_SIGNAL_FUNC (statusbar_clicked_cb), window);
  

  /* The 2 video window popups */
  mw->local_video_window =
    gm_mw_video_window_new (window,
			    TRUE,
			    _("Local Video"),
			    mw->local_video_image,
			    "local_video_window");
  mw->remote_video_window =
    gm_mw_video_window_new (window,
			    FALSE,
			    _("Remote Video"),
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


void 
gm_main_window_flash_message (GtkWidget *main_window, 
			      const char *msg, 
			      ...)
{
  va_list args;

  va_start (args, msg);
  gm_mw_push_message (main_window, TRUE, FALSE, msg, args);

  va_end (args);
}


void 
gm_main_window_push_message (GtkWidget *main_window, 
			     const char *msg, 
			     ...)
{
  va_list args;

  va_start (args, msg);
  gm_mw_push_message (main_window, FALSE, FALSE, msg, args);

  va_end (args);
}


void 
gm_main_window_push_info_message (GtkWidget *main_window, 
				  const char *msg, 
				  ...)
{
  gchar *info = NULL;
  
  va_list args;
  char buffer [1025];

  va_start (args, msg);
  vsnprintf (buffer, 1024, msg, args);

  info = 
    g_strdup_printf ("%s (%s)", 
		     buffer, _("Click to clear"));
  gm_mw_push_message (main_window, FALSE, TRUE, info);
  g_free (info);
  
  va_end (args);
}


void 
gm_main_window_set_call_url (GtkWidget *main_window, 
			     const char *url)
{
  GmWindow *mw = NULL;

  g_return_if_fail (main_window != NULL && url != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);
 
  gtk_entry_set_text (GTK_ENTRY (GTK_BIN (mw->combo)->child), url);
  gtk_editable_set_position (GTK_EDITABLE (GTK_BIN (mw->combo)->child), -1);
}


const char *
gm_main_window_get_call_url (GtkWidget *main_window)
{
  GmWindow *mw = NULL;

  g_return_val_if_fail (main_window != NULL, NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_val_if_fail (mw != NULL, NULL);
 
  return gtk_entry_get_text (GTK_ENTRY (GTK_BIN (mw->combo)->child));
}


void 
gm_main_window_set_remote_user_name (GtkWidget *main_window,
				     const char *name)
{
  GmWindow *mw = NULL;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  gtk_label_set_text (GTK_LABEL (mw->remote_name), 
		      (const char *) name);
}


void 
gm_main_window_clear_stats (GtkWidget *main_window)
{
  GmWindow *mw = NULL;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  stats_drawing_area_clear (mw->stats_drawing_area);
  gtk_label_set_text (GTK_LABEL (mw->stats_label), 
		      _("Lost packets:\nLate packets:\nOut of order packets:\nJitter buffer:"));
}


void 
gm_main_window_update_stats (GtkWidget *main_window,
			     float lost,
			     float late,
			     float out_of_order,
			     int jitter,
			     float new_video_octets_received,
			     float new_video_octets_transmitted,
			     float new_audio_octets_received,
			     float new_audio_octets_transmitted)
{
  GmWindow *mw = NULL;
  
  gchar *stats_msg = NULL;

  
  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  stats_msg =  g_strdup_printf (_("Lost packets: %.1f %%\nLate packets: %.1f %%\nOut of order packets: %.1f %%\nJitter buffer: %d ms"), lost, late, out_of_order, jitter);
  gtk_label_set_text (GTK_LABEL (mw->stats_label), stats_msg);
  g_free (stats_msg);

  stats_drawing_area_new_data (mw->stats_drawing_area,
			       new_video_octets_received,
			       new_video_octets_transmitted,
			       new_audio_octets_received,
			       new_audio_octets_transmitted);
}


GdkPixbuf *
gm_main_window_get_current_picture (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  
  g_return_val_if_fail (main_window != NULL, NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_val_if_fail (mw != NULL, NULL);

  return gtk_image_get_pixbuf (GTK_IMAGE (mw->main_video_image));
}


void 
gm_main_window_set_stay_on_top (GtkWidget *main_window,
				gboolean stay_on_top)
{
  GmWindow *mw = NULL;
  
  GdkWindow *gm_window = NULL;
  GdkWindow *local_window = NULL;
  GdkWindow *remote_window = NULL;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);
  

  gm_window = GDK_WINDOW (main_window->window);
  local_window = GDK_WINDOW (mw->local_video_window->window);
  remote_window = GDK_WINDOW (mw->remote_video_window->window);

  /* Update the stay-on-top attribute */
  gdk_window_set_always_on_top (GDK_WINDOW (gm_window), stay_on_top);
  gdk_window_set_always_on_top (GDK_WINDOW (local_window), stay_on_top);
  gdk_window_set_always_on_top (GDK_WINDOW (remote_window), stay_on_top);
}


/* The main () */
int 
main (int argc, 
      char ** argv, 
      char ** envp)
{
  PProcess::PreInitialise (argc, argv, envp);

  GtkWidget *main_window = NULL;
  GtkWidget *dialog = NULL;
  
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
  gm_events_init ();
  
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

    dialog = gnomemeeting_error_dialog (NULL, _("No usable audio plugin detected"), _("GnomeMeeting didn't find any usable audio plugin. Make sure that your installation is correct."));
    
    g_signal_handlers_disconnect_by_func (G_OBJECT (dialog),
					  (gpointer) gtk_widget_destroy,
					  G_OBJECT (dialog));

    gtk_dialog_run (GTK_DIALOG (dialog));
    exit (-1);
  }


  /* Init the process and build the GUI */
  GnomeMeeting::Process ()->BuildGUI ();
  GnomeMeeting::Process ()->Init ();

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  

  /* Init the config DB, exit if it fails */
  if (!gnomemeeting_conf_init ()) {

    key_name = g_strdup ("\"/apps/gnomemeeting/general/gconf_test_age\"");
    msg = g_strdup_printf (_("GnomeMeeting got an invalid value for the GConf key %s.\n\nIt probably means that your GConf schemas have not been correctly installed or the that permissions are not correct.\n\nPlease check the FAQ (http://www.gnomemeeting.org/faq.php), the throubleshoot section of the GConf site (http://www.gnome.org/projects/gconf/) or the mailing list archives for more information (http://mail.gnome.org) about this problem."), key_name);
    
    dialog = gnomemeeting_error_dialog (GTK_WINDOW (main_window),
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
  if (url) 
    GnomeMeeting::Process ()->Connect (url);

  
  /* The GTK loop */
  gtk_main ();
  gdk_threads_leave ();

  gm_conf_save ();

  return 0;
}


#ifdef WIN32
int 
APIENTRY WinMain (HINSTANCE hInstance,
		  HINSTANCE hPrevInstance,
		  LPSTR     lpCmdLine,
		  int       nCmdShow)
{
  return main (0, NULL, NULL);
}
#endif
