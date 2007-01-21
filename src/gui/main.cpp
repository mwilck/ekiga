
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         main_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Mon Mar 26 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          build the main window.
 */


#include "../../config.h"

#include "main.h"
#include "callshistory.h"

#include "pcss.h"
#include "ekiga.h"
#include "chat.h"
#include "addressbook.h"
#include "config.h"
#include "misc.h"
#include "callbacks.h"
#include "statusicon.h"
#include "audio.h"
#include "urlhandler.h"

#include "gmdialog.h"
#include "gmentrydialog.h"
#include "gmstatusbar.h"
#include "gmconnectbutton.h"
#include "gmstockicons.h"
#include "gmconf.h"
#include "gmcontacts.h"
#include "gmmenuaddon.h"
#include "gmlevelmeter.h"
#include "gmroster.h"
#include "gmpowermeter.h"
#include "contacts.h"


#include "../pixmaps/text_logo.xpm"
#include "../pixmaps/shadow.xpm"
#include "../pixmaps/gm_both_incrusted_frame.xpm"

#include <gdk/gdkkeysyms.h>

#ifndef DISABLE_GNOME
#include <libgnomeui/gnome-window-icon.h>
#include "bonobo.h"
#endif

#ifndef WIN32
#include <signal.h>
#include <gdk/gdkx.h>
#else
#include "winpaths.h"
#endif

#if defined(P_FREEBSD) || defined (P_MACOSX)
#include <libintl.h>
#endif

#include <libxml/parser.h>

#ifdef HAS_SDL
#include <SDL.h>
#define SDL_VIDEO_FLAGS SDL_SWSURFACE | SDL_HWSURFACE | SDL_ANYFORMAT | SDL_FULLSCREEN
#ifdef WIN32
  #define CALL_OR_IDLE_ADD(func,arg)  g_idle_add (func, arg)
#else
  #define CALL_OR_IDLE_ADD(func,arg)  func (arg)
#endif
#endif

#define GM_MAIN_WINDOW(x) (GmWindow *) (x)

/* Declarations */
struct _GmWindow
{
  GtkWidget *input_signal;
  GtkWidget *output_signal;
  GtkObject *adj_input_volume;
  GtkObject *adj_output_volume;
  GtkWidget *audio_volume_frame;
  GtkWidget *audio_settings_window;

  GtkObject *adj_whiteness;
  GtkObject *adj_brightness;
  GtkObject *adj_colour;
  GtkObject *adj_contrast;
  GtkWidget *video_settings_frame;
  GtkWidget *video_settings_window;
  
  GtkTooltips *tips;
  GtkAccelGroup *accel;

  GtkWidget *main_menu;
  
#ifdef DISABLE_GNOME
  GtkWidget *window_vbox;
  GtkWidget *window_hbox;
#endif

  GtkWidget *status_label_ebox;
  GtkWidget *info_text;
  GtkWidget *info_label;

  GtkWidget *statusbar;
  GtkWidget *statusbar_ebox;
  GtkWidget *qualitymeter;
  GtkWidget *combo;
  GtkWidget *main_notebook;
  GtkWidget *roster;
  GtkWidget *main_video_image;
  GtkWidget *local_video_image;
  GtkWidget *local_video_window;
  GtkWidget *remote_video_image;
  GtkWidget *remote_video_window;
  GtkWidget *video_frame;
  GtkWidget *preview_button;
  GtkWidget *connect_button;
  GtkWidget *hold_button;
  GtkWidget *incoming_call_popup;
  GtkWidget *transfer_call_popup;

  GmContactsUICallbackData *cb_data;

#ifdef HAS_SDL
  SDL_Surface *screen;
#endif
};


typedef struct _GmWindow GmWindow;


#define GM_WINDOW(x) (GmWindow *) (x)

/* channel types */
enum {
  CHANNEL_FIRST,
  CHANNEL_AUDIO,
  CHANNEL_VIDEO,
  CHANNEL_LAST
};


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


/* description  : /
 * behavior     : Builds the contacts list part of the main window.
 * pre          : The given GtkWidget pointer must be the main window GMObject. 
 */
static void gm_mw_init_contacts_list (GtkWidget *);


/* description  : /
 * behavior     : Builds the calls history part of the main window.
 * pre          : The given GtkWidget pointer must be the main window GMObject. 
 */
static void gm_mw_init_calls_history (GtkWidget *);


/* description  : /
 * behavior     : builds the dialpad part of the main window.
 * pre          : the given GtkWidget pointer must be the main window GMObject. 
 */
static void gm_mw_init_dialpad (GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the video settings popup of the main window.
 * PRE          : The given GtkWidget pointer must be the main window GMObject. 
 */
static GtkWidget *gm_mw_video_settings_window_new (GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the audio settings popup for the main window.
 * PRE          : The given GtkWidget pointer must be the main window GMObject. 
 */
static GtkWidget *gm_mw_audio_settings_window_new (GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the call part of the main window.
 * PRE          : The given GtkWidget pointer must be the main window GMObject. 
 */
static void gm_mw_init_call (GtkWidget *);


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
gboolean gm_mw_init_fullscreen_video_window (gpointer);


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
gboolean gm_mw_destroy_fullscreen_video_window (gpointer);
#endif


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Show / hide the video section.
 * PRE          :  The main window GMObject.
 */
void gm_mw_show_video_section (GtkWidget *,
			       gboolean);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  enables/disables the zoom related menuitems according
 *                 to zoom factor
 * PRE          :  The main window GMObject.
 */
static void gm_mw_zooms_menu_update_sensitivity (GtkWidget *,
			      			 double);

/* Callbacks */

#ifdef WIN32
/* DESCRIPTION  :  This callback is a workaround to GTK+ bugs on WIN32.
 *                 It is triggered to change the current control
 *                 panel section.
 * BEHAVIOR     :  Changes the current page selection.
 * PRE          :  /
 */
static gboolean
thread_safe_notebook_set_page (gpointer data);


/* DESCRIPTION  :  This callback is a workaround to GTK+ bugs on WIN32.
 *                 It is triggered to update the tooltip in the status bar.
 * BEHAVIOR     :  Updates the tooltip.
 * PRE          :  /
 */
static gboolean
thread_safe_set_stats_tooltip (gpointer data);
#endif


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Set the current active call on hold.
 * PRE          :  /
 */
static void hold_current_call_cb (GtkWidget *,
				  gpointer);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Set the current active call audio or video channel on pause
 * 		   or not and update the GUI accordingly.
 * PRE          :  GPOINTER_TO_INT (data) is a CHANNEL_*
 */
static void pause_current_call_channel_cb (GtkWidget *,
					   gpointer);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a dialog to transfer the current call and transfer
 * 		   it if required.
 * PRE          :  The parent window.
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
 * BEHAVIOR     :  Update the volume of the choosen mixers. If the update
 *                 fails, the sliders are put back to 0.
 * PRE          :  The main window GMObject.
 */
static void audio_volume_changed_cb (GtkAdjustment *, 
				     gpointer);


/* DESCRIPTION  :  This callback is called when the user changes one of the 
 *                 video settings sliders in the main notebook.
 * BEHAVIOR     :  Updates the value in real time, if it fails, reset 
 * 		   all sliders to 0.
 * PRE          :  gpointer is a valid pointer to the main window GmObject.
 */
static void video_settings_changed_cb (GtkAdjustment *, 
				       gpointer);


/* DESCRIPTION  :  This callback is called when the user drops a contact.
 * BEHAVIOR     :  Calls the user corresponding to the contact or transfer
 * 		   the calls to the user.
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
static void panel_section_changed_cb (GtkNotebook *, 
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


/* DESCRIPTION  :  This callback is called when the user tries to close
 *                 the main window using the FILE-menu
 * BEHAVIOUR    :  Directly calls window_closed_cb (i.e. it's just a wrapper)
 * PRE          :  ---
 */

static void window_closed_from_menu_cb (GtkWidget *,
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


/* DESCRIPTION  :  This callback is called when the user clicks on enter
 * 		   with a non-empty URL bar.
 * BEHAVIOR     :  It calls the URL.
 * PRE          :  /
 */
static void url_activated_cb (GtkWidget *, 
			      gpointer);


/* DESCRIPTION  :  This callback is called when the user presses a
 *                 button in the toolbar. 
 *                 (See menu_toggle_changed)
 * BEHAVIOR     :  Updates the config cache.
 * PRE          :  data is the key.
 */
static void toolbar_toggle_button_changed_cb (GtkWidget *, 
					      gpointer);


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


/* DESCRIPTION  :  Called when the chat icon is clicked.
 * BEHAVIOR     :  Show the chat window or hide it.
 * 		   If the chat window is shown during a call, the corresponding
 * 		   call tab is added.
 * 		   Reset the tray flashing state when the window is shown.
 * PRE          :  The pointer must be a valid pointer to the chat window
 * 		   GMObject.
 */
static void show_chat_window_cb (GtkWidget *w,
				 gpointer data);


/* DESCRIPTION  :  This callback is called in an idle loop.
 * BEHAVIOR     :  Do the job of gm_main_window_urls_history_update, but 
 *                 async.
 * PRE          :  A valid main window GMObject.
 */
static gboolean gm_mw_urls_history_update_cb (gpointer data);


/* DESCRIPTION  :  This callback is called when a contact in the roster
 *                 is doubleclicked
 * BEHAVIOR     :  TO BE DONE
 * PRE          : /
 */
static void contact_doubleclicked_cb (GMRoster *roster, gpointer data);


/* DESCRIPTION  :  This callback is called when a contact in the roster
 *                 is right-clicked
 * BEHAVIOR     :  display a contact context menu
 * PRE          : /
 */
static void contact_clicked_cb (GMRoster *roster, gpointer data);



/* Implementation */
static void
gm_mw_destroy (gpointer m)
{
  GmWindow *mw = GM_WINDOW (m);
  
  g_return_if_fail (mw != NULL);

  gm_contacts_callback_data_delete (((GmWindow *) mw)->cb_data);

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
  GtkWidget *chat_window = NULL;
  
#ifndef DISABLE_GNOME
  int behavior = 0;
  BOOL toolbar_detachable = TRUE;
#endif

  addressbook_window = GnomeMeeting::Process ()->GetAddressbookWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();

  
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

  // activate Ctrl-L to get the entry focus
  gtk_widget_add_accelerator (mw->combo, "grab-focus",
			      mw->accel, GDK_L,
			      (GdkModifierType) GDK_CONTROL_MASK,
			      (GtkAccelFlags) 0);

  // set the position to the end of the combo
  int len = strlen(gtk_entry_get_text (GTK_ENTRY (GTK_BIN (mw->combo)->child)));
  gtk_editable_set_position (GTK_EDITABLE (GTK_WIDGET ((GTK_BIN (mw->combo))->child)),len);

  gtk_entry_completion_set_match_func (GTK_ENTRY_COMPLETION (completion),
				       entry_completion_url_match_cb,
				       (gpointer) list_store,
				       NULL);
  
  gm_main_window_urls_history_update (main_window);

  g_signal_connect (G_OBJECT (GTK_BIN (mw->combo)->child), "changed", 
		    GTK_SIGNAL_FUNC (url_changed_cb), (gpointer) main_window);
  g_signal_connect (G_OBJECT (GTK_BIN (mw->combo)->child), "activate", 
		    GTK_SIGNAL_FUNC (url_activated_cb), NULL);
  g_signal_connect (G_OBJECT (completion), "match-selected", 
		    GTK_SIGNAL_FUNC (completion_url_selected_cb), NULL);

  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, 0);

  
  /* The connect button */
  item = gtk_tool_item_new ();
  mw->connect_button = gm_connect_button_new (GM_STOCK_CONNECT,
					      GM_STOCK_DISCONNECT,
					      GTK_ICON_SIZE_LARGE_TOOLBAR,
					      NULL,
					      NULL);
  gtk_container_add (GTK_CONTAINER (item), mw->connect_button);
  gtk_container_set_border_width (GTK_CONTAINER (mw->connect_button), 2);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);

  gtk_tooltips_set_tip (mw->tips, GTK_WIDGET (mw->connect_button), 
			_("Enter a URI on the left, and click this button to place a call"), NULL);
  
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

  g_signal_connect (G_OBJECT (mw->connect_button), "released",
                    G_CALLBACK (connect_button_clicked_cb), 
		    GTK_ENTRY (GTK_BIN (mw->combo)->child));

  gtk_widget_show_all (GTK_WIDGET (toolbar));
  
#ifndef DISABLE_GNOME
  behavior = (BONOBO_DOCK_ITEM_BEH_EXCLUSIVE
	      | BONOBO_DOCK_ITEM_BEH_NEVER_VERTICAL);

  if (!toolbar_detachable)
    behavior |= BONOBO_DOCK_ITEM_BEH_LOCKED;

  gnome_app_add_docked (GNOME_APP (main_window), toolbar, "main_toolbar",
			BonoboDockItemBehavior (behavior),
  			BONOBO_DOCK_BOTTOM, 1, 0, 0);
#else
  gtk_box_pack_start (GTK_BOX (mw->window_vbox), toolbar, 
		      FALSE, FALSE, 0);
#endif


  /* The normal toolbar */
  toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_orientation (GTK_TOOLBAR (toolbar), 
			       GTK_ORIENTATION_HORIZONTAL);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (toolbar), FALSE);
  

  /* The add contact icon */
  item = gtk_tool_item_new ();
  button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  image = gtk_image_new_from_stock (GM_STOCK_ADD_CONTACT, 
                                    GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_container_add (GTK_CONTAINER (button), image);
  gtk_container_add (GTK_CONTAINER (item), button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);
  
  gtk_widget_show (GTK_WIDGET (item));
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (item), 
			     mw->tips, _("New Contact"), NULL);

  mw->cb_data = gm_contacts_callback_data_new (NULL, NULL, 
                                               GTK_WINDOW (main_window));

  g_signal_connect (G_OBJECT (button), "clicked",
                    GTK_SIGNAL_FUNC (gm_contacts_add_new_contact_cb),
                    mw->cb_data);

  
  /* The address book icon */
  item = gtk_tool_item_new ();
  button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  image = gtk_image_new_from_stock (GM_STOCK_ADDRESSBOOK,
                                    GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_container_add (GTK_CONTAINER (button), image);
  gtk_container_add (GTK_CONTAINER (item), button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);
  
  gtk_widget_show (GTK_WIDGET (item));
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (item), 
			     mw->tips, _("Address Book"), NULL);

  g_signal_connect (G_OBJECT (button), "clicked",
		    GTK_SIGNAL_FUNC (show_window_cb), 
		    (gpointer) addressbook_window);

  /* The text chat */
  item = gtk_tool_item_new ();
  button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  image = gtk_image_new_from_stock (GM_STOCK_INTERNET_GROUP_CHAT, 
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
		    GTK_SIGNAL_FUNC (show_chat_window_cb),
		    (gpointer) chat_window);
  
  
  /* Add the toolbar to the UI */
#ifndef DISABLE_GNOME
  behavior = (BONOBO_DOCK_ITEM_BEH_EXCLUSIVE
	      | BONOBO_DOCK_ITEM_BEH_NEVER_VERTICAL);

  if (!toolbar_detachable)
    behavior |= BONOBO_DOCK_ITEM_BEH_LOCKED;

  gnome_app_add_toolbar (GNOME_APP (main_window), GTK_TOOLBAR (toolbar),
 			 "left_toolbar", 
			 BonoboDockItemBehavior (behavior),
 			 BONOBO_DOCK_TOP, 2, 0, 0);
#else
  gtk_box_pack_start (GTK_BOX (mw->window_vbox), toolbar, 
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
  GtkWidget *history_window = NULL;
  GtkWidget *prefs_window = NULL;
  GtkWidget *accounts_window = NULL;
  GtkWidget *pc2phone_window = NULL;
  
  IncomingCallMode icm = AVAILABLE;
  PanelSection cps = DIALPAD;
  gboolean show_video_section = FALSE;
  int nbr = 0;

  GSList *glist = NULL;

  g_return_if_fail (main_window != NULL);
  mw = gm_mw_get_mw (main_window);
  
  addressbook_window = GnomeMeeting::Process ()->GetAddressbookWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  druid_window = GnomeMeeting::Process ()->GetDruidWindow ();
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();
  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();
  pc2phone_window = GnomeMeeting::Process ()->GetPC2PhoneWindow ();

  mw->main_menu = gtk_menu_bar_new ();


  /* Default values */
  icm = (IncomingCallMode) 
    gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode"); 
  cps = (PanelSection)
    gm_conf_get_int (USER_INTERFACE_KEY "main_window/panel_section"); 
  show_video_section =
    gm_conf_get_bool (USER_INTERFACE_KEY "main_window/show_video_section");

  
  static MenuEntry gnomemeeting_menu [] =
    {
      GTK_MENU_NEW (_("C_all")),

      GTK_MENU_ENTRY("connect", _("Ca_ll"), _("Place a new call"), 
		     GM_STOCK_CONNECT_16, 'o',
		     GTK_SIGNAL_FUNC (connect_cb), main_window, TRUE),
      GTK_MENU_ENTRY("disconnect", _("_Hang up"),
		     _("Terminate the current call"), 
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
		     NULL, 'g', 
		     GTK_SIGNAL_FUNC (hold_current_call_cb), NULL,
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
		     GINT_TO_POINTER (CHANNEL_AUDIO), FALSE),
      GTK_MENU_ENTRY("suspend_video", _("Suspend _Video"),
		     _("Suspend or resume the video transmission"),
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (pause_current_call_channel_cb),
		     GINT_TO_POINTER (CHANNEL_VIDEO), FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("save_picture", NULL,
		     _("Save a snapshot of the current video"),
		     GTK_STOCK_SAVE, 'S',
		     GTK_SIGNAL_FUNC (save_callback), NULL, FALSE),

      GTK_MENU_SEPARATOR,
      
      GTK_MENU_ENTRY("close", NULL, _("Close the Ekiga window"),
		     GTK_STOCK_CLOSE, 'W', 
		     GTK_SIGNAL_FUNC (window_closed_from_menu_cb),
		     (gpointer) main_window, TRUE),

      GTK_MENU_SEPARATOR,
      
      GTK_MENU_ENTRY("quit", NULL, _("Quit"),
		     GTK_STOCK_QUIT, 'Q', 
		     GTK_SIGNAL_FUNC (quit_callback), NULL, TRUE),

      GTK_MENU_NEW (_("_Edit")),

      GTK_MENU_ENTRY("configuration_druid", _("Configuration Druid"),
		     _("Run the configuration druid"),
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) druid_window, TRUE),

      GTK_MENU_SEPARATOR,
      
      GTK_MENU_ENTRY("accounts", _("_Accounts"),
		     _("Edit your accounts"), 
		     NULL, 'E',
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) accounts_window, TRUE),

      GTK_MENU_ENTRY("preferences", NULL,
		     _("Change your preferences"), 
		     GTK_STOCK_PREFERENCES, 'P',
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) prefs_window, TRUE),

      GTK_MENU_NEW(_("_View")),

      GTK_SUBMENU_NEW("panel", _("Panel")),

      GTK_MENU_RADIO_ENTRY("contacts", _("Con_tacts"), _("View the contacts list"),
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) USER_INTERFACE_KEY "main_window/panel_section",
			   (cps == CONTACTS), TRUE),
      GTK_MENU_RADIO_ENTRY("dialpad", _("_Dialpad"), _("View the dialpad"),
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb), 
			   (gpointer) USER_INTERFACE_KEY "main_window/panel_section",
			   (cps == DIALPAD), TRUE),
      GTK_MENU_RADIO_ENTRY("calls_history", _("Calls _History"),
			   _("View the calls history"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb), 
			   (gpointer) USER_INTERFACE_KEY "main_window/panel_section",
			   (cps == CALLS_HISTORY), TRUE),
      GTK_MENU_RADIO_ENTRY("call", _("C_all"),
			   _("View the call information"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb), 
			   (gpointer) USER_INTERFACE_KEY "main_window/panel_section",
			   (cps == CALL), TRUE),

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
      GTK_MENU_RADIO_ENTRY("both_new_windows",
			   _("Both (Both in New Windows)"), 
			   _("Both video images"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb), 
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   FALSE, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("zoom_in", NULL, _("Zoom in"), 
		     GTK_STOCK_ZOOM_IN, '+', 
		     GTK_SIGNAL_FUNC (zoom_in_changed_cb),
		     (gpointer) VIDEO_DISPLAY_KEY "zoom_factor", FALSE),
      GTK_MENU_ENTRY("zoom_out", NULL, _("Zoom out"), 
		     GTK_STOCK_ZOOM_OUT, '-', 
		     GTK_SIGNAL_FUNC (zoom_out_changed_cb),
		     (gpointer) VIDEO_DISPLAY_KEY "zoom_factor", FALSE),
      GTK_MENU_ENTRY("normal_size", NULL, _("Normal size"), 
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
		     GM_STOCK_ADDRESSBOOK, 0,
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) addressbook_window, TRUE),
      
      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("chat_window", _("C_hat Window"),
		     _("Open the chat window"),
		     NULL, 0,
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) chat_window, TRUE),

      GTK_MENU_ENTRY("log", _("General History"),
		     _("View the operations history"),
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) history_window, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("pc-to-phone", _("PC-To-Phone Account"),
		     _("Manage your PC-To-Phone account"),
		     NULL, 0, 
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) pc2phone_window, TRUE),
      
      GTK_MENU_NEW(_("_Help")),

       GTK_MENU_ENTRY("help", NULL, 
                     _("Get help by reading the Ekiga manual"),
                     GTK_STOCK_HELP, GDK_F1, 
                     GTK_SIGNAL_FUNC (help_cb), NULL, TRUE),
#ifndef DISABLE_GNOME
      GTK_MENU_ENTRY("about", NULL,
		     _("View information about Ekiga"),
		     GNOME_STOCK_ABOUT, 0, 
		     GTK_SIGNAL_FUNC (about_callback), (gpointer) main_window,
		     TRUE),
#else
      GTK_MENU_ENTRY("about", NULL,
		     _("View information about Ekiga"),
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
					   FALSE, NULL, NULL, NULL, NULL, "*"); 
  gm_main_window_speed_dials_menu_update (main_window, glist);
  g_slist_foreach (glist, (GFunc) gmcontact_delete, NULL);
  g_slist_free (glist);

  gtk_widget_show_all (GTK_WIDGET (mw->main_menu));
}


static void
contact_doubleclicked_cb (GMRoster *roster,
			  gpointer data)
{
}


static void
contact_clicked_cb (GMRoster *roster, 
                    gpointer data)
{
  GtkWidget *main_window = NULL;
  GtkWidget *context_menu = NULL;
  GmContact *contact = NULL;
  gchar *uid = NULL;

  g_return_if_fail (roster != NULL);

  uid = gmroster_get_selected_uid (roster);

  if (!uid) return;

  contact = gnomemeeting_local_contact_get_by_uid (uid);

  g_return_if_fail (contact != NULL);

  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  context_menu = gm_contacts_contextmenu_new (contact,
					      (GmContactContextMenuFlags) 0,
					      GTK_WINDOW (main_window));

  gtk_menu_popup (GTK_MENU (context_menu), 
                  NULL, NULL, NULL, NULL,
                  0, gtk_get_current_event_time ());
  g_signal_connect (G_OBJECT (context_menu), "hide",
                    GTK_SIGNAL_FUNC (g_object_unref), (gpointer) context_menu);
  g_object_ref (G_OBJECT (context_menu));
  gtk_object_sink (GTK_OBJECT (context_menu));

  gmcontact_delete (contact);
}


static void 
gm_mw_init_contacts_list (GtkWidget *main_window)
{
  GmWindow *mw = NULL;

  GtkWidget *label = NULL;

  GtkWidget *frame = NULL;
  GtkWidget *scroll = NULL;

  GtkWidget *roster = NULL;

  GSList *contacts = NULL;
  int nbr = 0;

  g_return_if_fail (main_window != NULL);
  mw = gm_mw_get_mw (main_window);

  
  /* A frame and a scrolled window */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), 
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (frame), scroll);

  /* The roster */
  roster = gmroster_new ();
  gmroster_set_gmconf_key (GMROSTER (roster),
                           USER_INTERFACE_KEY "main_window");
  gmroster_set_show_in_multiple_groups (GMROSTER (roster), TRUE);
  gmroster_set_unknown_group_name (GMROSTER (roster), GM_CONTACTS_UNKNOWN_GROUP);
  gmroster_set_roster_group (GMROSTER (roster), GM_CONTACTS_ROSTER_GROUP);
  gmroster_set_show_groupless_contacts (GMROSTER (roster), TRUE);

  gmroster_set_status_icon (GMROSTER (roster), CONTACT_OFFLINE, 
                            GM_STOCK_STATUS_OFFLINE);
  gmroster_set_status_icon (GMROSTER (roster), CONTACT_ONLINE, 
                            GM_STOCK_STATUS_ONLINE);
  gmroster_set_status_icon (GMROSTER (roster), CONTACT_UNKNOWN,
			    GM_STOCK_STATUS_UNKNOWN);
  gmroster_set_status_icon (GMROSTER (roster), CONTACT_AWAY,
			    GM_STOCK_STATUS_AWAY);
  gmroster_set_status_icon (GMROSTER (roster), CONTACT_DND,
			    GM_STOCK_STATUS_DND);
  gmroster_set_status_icon (GMROSTER (roster), CONTACT_FREEFORCHAT,
			    GM_STOCK_STATUS_FREEFORCHAT);

  gtk_container_add (GTK_CONTAINER (scroll), roster);

  contacts = gnomemeeting_addressbook_get_contacts (NULL,
						    nbr,
						    FALSE,
						    NULL,
						    NULL,
						    NULL,
						    NULL,
						    NULL);

  gmroster_sync_with_contacts (GMROSTER (roster), contacts);

  g_slist_foreach (contacts, (GFunc) gmcontact_delete, NULL);
  g_slist_free (contacts);

  g_signal_connect (roster, "contact-clicked",
                    GTK_SIGNAL_FUNC (contact_clicked_cb),
                    NULL);

  g_signal_connect (roster, "contact-doubleclicked",
		    GTK_SIGNAL_FUNC (contact_doubleclicked_cb),
		    NULL);

  mw->roster = roster;


  label = gtk_label_new (_("Contacts"));

  gtk_notebook_append_page (GTK_NOTEBOOK (mw->main_notebook),
			    frame, label);
}


void
gm_main_window_update_contacts_list (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  GSList *contacts = NULL;
  int nbr = 0;

  g_return_if_fail (main_window != NULL);
  mw = gm_mw_get_mw (main_window);

  contacts = gnomemeeting_addressbook_get_contacts (NULL,
						    nbr,
						    FALSE,
						    NULL,
						    NULL,
						    NULL,
						    NULL,
						    NULL);

  gmroster_sync_with_contacts (GMROSTER (mw->roster), contacts);

  g_slist_foreach (contacts, (GFunc) gmcontact_delete, NULL);
  g_slist_free (contacts);
}


static void 
gm_mw_init_calls_history (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  
  GtkWidget *label = NULL;
  GtkWidget *calls_history_component = NULL;

  g_return_if_fail (main_window != NULL);
  mw = gm_mw_get_mw (main_window);

  calls_history_component = gm_calls_history_component_new ();
  gtk_container_set_border_width (GTK_CONTAINER (calls_history_component), 0);
  
  label = gtk_label_new (_("Calls History"));

  gtk_notebook_append_page (GTK_NOTEBOOK (mw->main_notebook),
			    calls_history_component, label);
}


static void 
gm_mw_init_dialpad (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  
  GtkSizeGroup *size_group_alpha = NULL;

  GtkWidget *alignment = NULL;
  GtkWidget *box = NULL;
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

  alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
  table = gtk_table_new (4, 3, TRUE);
  gtk_container_add (GTK_CONTAINER (alignment), table);

  size_group_alpha = gtk_size_group_new (GTK_SIZE_GROUP_BOTH);

  for (i = 0 ; i < 12 ; i++) {

    box = gtk_hbox_new (FALSE, 0);

    label = gtk_label_new (NULL);
    text_label =
      g_strdup_printf (" %s ",
		       key_n [i]);
    g_object_set (label, "xalign", 1.0, NULL);
    gtk_label_set_markup (GTK_LABEL (label), text_label); 
    gtk_box_pack_start (GTK_BOX(box), label, TRUE, TRUE, 0);
    g_free (text_label);

    label = gtk_label_new (NULL);
    text_label =
      g_strdup_printf ("<sub><span size=\"small\">%s</span></sub> ",
		       key_a [i]);
    g_object_set (label, "xalign", 0.0, NULL);
    gtk_label_set_markup (GTK_LABEL (label), text_label); 
    gtk_size_group_add_widget (size_group_alpha, label);
    gtk_box_pack_start (GTK_BOX(box), label, FALSE, TRUE, 0);
    g_free (text_label);

    button = gtk_button_new ();
    gtk_container_set_border_width (GTK_CONTAINER (button), 0);
    gtk_container_add (GTK_CONTAINER (button), box);
   
    gtk_widget_add_accelerator (button, "clicked", 
				mw->accel, key_kp [i], 
				(GdkModifierType) 0, (GtkAccelFlags) 0);
    
    gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (button), 
		      i%3, i%3+1, i/3, i/3+1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL),
		      1, 2);
    
    g_signal_connect (G_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (dialpad_button_clicked_cb), 
		      main_window);
  }
  
  label = gtk_label_new (_("Dialpad"));

  gtk_notebook_append_page (GTK_NOTEBOOK (mw->main_notebook),
			    alignment, label);
}


static GtkWidget * 
gm_mw_video_settings_window_new (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *image = NULL;
  GtkWidget *window = NULL;

  GtkWidget *hscale_brightness = NULL;
  GtkWidget *hscale_colour = NULL;
  GtkWidget *hscale_contrast = NULL;
  GtkWidget *hscale_whiteness = NULL;

  int brightness = 0, colour = 0, contrast = 0, whiteness = 0;

  g_return_val_if_fail (main_window != NULL, NULL);
  mw = gm_mw_get_mw (main_window);

  
  /* Build the window */
  window = gtk_dialog_new ();
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("video_settings_window"), g_free); 
  gtk_dialog_add_button (GTK_DIALOG (window), 
                         GTK_STOCK_CLOSE, 
                         GTK_RESPONSE_CANCEL);

  gtk_window_set_title (GTK_WINDOW (window), 
                        _("Video Settings"));

  /* Webcam Control Frame, we need it to disable controls */		
  mw->video_settings_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (mw->video_settings_frame), 
			     GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (mw->video_settings_frame), 5);
  
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
  gtk_box_pack_start (GTK_BOX (hbox), hscale_brightness, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);

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
  gtk_box_pack_start (GTK_BOX (hbox), hscale_whiteness, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);

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
  gtk_box_pack_start (GTK_BOX (hbox), hscale_colour, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);

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
  gtk_box_pack_start (GTK_BOX (hbox), hscale_contrast, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);

  gtk_tooltips_set_tip (mw->tips, hscale_contrast,
			_("Adjust contrast"), NULL);

  g_signal_connect (G_OBJECT (mw->adj_contrast), "value-changed",
		    G_CALLBACK (video_settings_changed_cb), 
		    (gpointer) main_window);
  
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (window)->vbox), 
                     mw->video_settings_frame);
  gtk_widget_show_all (mw->video_settings_frame);

  
  /* That's an usual GtkWindow, connect it to the signals */
  g_signal_connect_swapped (GTK_OBJECT (window), 
			    "response", 
			    G_CALLBACK (gnomemeeting_window_hide),
			    (gpointer) window);

  g_signal_connect (GTK_OBJECT (window), 
		    "delete-event", 
		    G_CALLBACK (delete_window_cb), NULL);

  return window;
}



static GtkWidget * 
gm_mw_audio_settings_window_new (GtkWidget *main_window)
{
  GmWindow *mw = NULL;
  
  GtkWidget *hscale_play = NULL; 
  GtkWidget *hscale_rec = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *small_vbox = NULL;
  GtkWidget *window = NULL;
  

  /* Get the data from the GMObject */
  g_return_val_if_fail (main_window != NULL, NULL);
  mw = gm_mw_get_mw (main_window);
  

  /* Build the window */
  window = gtk_dialog_new ();
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("audio_settings_window"), g_free); 
  gtk_dialog_add_button (GTK_DIALOG (window), 
                         GTK_STOCK_CLOSE, 
                         GTK_RESPONSE_CANCEL);

  gtk_window_set_title (GTK_WINDOW (window), 
                        _("Audio Settings"));

  /* Audio control frame, we need it to disable controls */		
  mw->audio_volume_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (mw->audio_volume_frame), 
			     GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (mw->audio_volume_frame), 5);


  /* The vbox */
  vbox = gtk_vbox_new (0, FALSE);
  gtk_container_add (GTK_CONTAINER (mw->audio_volume_frame), vbox);

  /* Output volume */
  hbox = gtk_hbox_new (0, FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), 
		      gtk_image_new_from_stock (GM_STOCK_VOLUME, 
						GTK_ICON_SIZE_SMALL_TOOLBAR),
		      FALSE, FALSE, 0);
  
  small_vbox = gtk_vbox_new (0, FALSE);
  mw->adj_output_volume = gtk_adjustment_new (0, 0.0, 101.0, 1.0, 5.0, 1.0);
  hscale_play = gtk_hscale_new (GTK_ADJUSTMENT (mw->adj_output_volume));
  gtk_range_set_update_policy (GTK_RANGE (hscale_play),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_play), GTK_POS_RIGHT); 
  gtk_scale_set_draw_value (GTK_SCALE (hscale_play), FALSE);
  gtk_box_pack_start (GTK_BOX (small_vbox), hscale_play, TRUE, TRUE, 0);

  mw->output_signal = gtk_levelmeter_new ();
  gtk_box_pack_start (GTK_BOX (small_vbox), mw->output_signal, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), small_vbox, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);

  /* Input volume */
  hbox = gtk_hbox_new (0, FALSE);
  gtk_box_pack_start (GTK_BOX (hbox),
		      gtk_image_new_from_stock (GM_STOCK_MICROPHONE, 
						GTK_ICON_SIZE_SMALL_TOOLBAR),
		      FALSE, FALSE, 0);

  small_vbox = gtk_vbox_new (0, FALSE);
  mw->adj_input_volume = gtk_adjustment_new (0, 0.0, 101.0, 1.0, 5.0, 1.0);
  hscale_rec = gtk_hscale_new (GTK_ADJUSTMENT (mw->adj_input_volume));
  gtk_range_set_update_policy (GTK_RANGE (hscale_rec),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_rec), GTK_POS_RIGHT); 
  gtk_scale_set_draw_value (GTK_SCALE (hscale_rec), FALSE);
  gtk_box_pack_start (GTK_BOX (small_vbox), hscale_rec, TRUE, TRUE, 0);

  mw->input_signal = gtk_levelmeter_new ();
  gtk_box_pack_start (GTK_BOX (small_vbox), mw->input_signal, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), small_vbox, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);

  g_signal_connect (G_OBJECT (mw->adj_output_volume), "value-changed",
		    G_CALLBACK (audio_volume_changed_cb), main_window);

  g_signal_connect (G_OBJECT (mw->adj_input_volume), "value-changed",
		    G_CALLBACK (audio_volume_changed_cb), main_window);


  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (window)->vbox), 
                     mw->audio_volume_frame);
  gtk_widget_show_all (mw->audio_volume_frame);

  
  /* That's an usual GtkWindow, connect it to the signals */
  g_signal_connect_swapped (GTK_OBJECT (window), 
			    "response", 
			    G_CALLBACK (gnomemeeting_window_hide),
			    (gpointer) window);

  g_signal_connect (GTK_OBJECT (window), 
		    "delete-event", 
		    G_CALLBACK (delete_window_cb), NULL);

  return window;
}


static void 
gm_mw_init_call (GtkWidget *main_window)
{
  GmWindow *mw = NULL;

  GtkWidget *event_box = NULL;
  GtkWidget *table = NULL;
  GtkWidget *label = NULL;

  GtkWidget *toolbar = NULL;
  GtkWidget *button = NULL;
  GtkToolItem *item = NULL;

  GtkWidget *image = NULL;

  GdkColor white;
  gdk_color_parse ("white", &white);

  /* Get the data from the GMObject */
  mw = gm_mw_get_mw (main_window);

  /* The main table */
  event_box = gtk_event_box_new ();
  gtk_widget_modify_bg (event_box, GTK_STATE_PRELIGHT, &white);
  gtk_widget_modify_bg (event_box, GTK_STATE_NORMAL, &white);
  table = gtk_table_new (3, 4, FALSE);
  gtk_container_add (GTK_CONTAINER (event_box), table);
  
  /* The frame that contains the video */
  mw->video_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (mw->video_frame), 
                             GTK_SHADOW_NONE);
  
  mw->main_video_image = gtk_image_new ();
  gtk_container_set_border_width (GTK_CONTAINER (mw->video_frame), 0);
  gtk_container_add (GTK_CONTAINER (mw->video_frame), mw->main_video_image);

  gtk_widget_set_size_request (GTK_WIDGET (mw->main_video_image), 
			       GM_QCIF_WIDTH, 
			       GM_QCIF_HEIGHT); 

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (mw->video_frame), 
		    0, 4, 0, 1,
		    (GtkAttachOptions) GTK_EXPAND,
		    (GtkAttachOptions) GTK_EXPAND,
		    0, 0);


  /* The frame that contains information about the call */
  /* Text buffer */
  GtkTextBuffer *buffer = NULL;
  
  mw->info_text = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (mw->info_text), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (mw->info_text), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (mw->info_text),
			       GTK_WRAP_WORD);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (mw->info_text));
  gtk_text_view_set_cursor_visible  (GTK_TEXT_VIEW (mw->info_text), FALSE);
  gtk_text_buffer_create_tag (buffer, "status",
			      "foreground", "black", 
                              "paragraph-background", "white",
                              "justification", GTK_JUSTIFY_CENTER,
                              "weight", PANGO_WEIGHT_BOLD,
                              "scale", 1.5,
                              NULL);
  gtk_text_buffer_create_tag (buffer, "codecs",
                              "justification", GTK_JUSTIFY_RIGHT,
                              "stretch", PANGO_STRETCH_CONDENSED,
			      "foreground", "darkgray", 
                              "paragraph-background", "white",
			      NULL);
  gtk_text_buffer_create_tag (buffer, "call-duration",
			      "foreground", "black", 
                              "paragraph-background", "white",
			      "justification", GTK_JUSTIFY_CENTER,
                              "weight", PANGO_WEIGHT_BOLD,
			      NULL);

  gm_main_window_set_status (main_window, _("Standby"));
  gm_main_window_set_call_duration (main_window, NULL);
  gm_main_window_set_call_info (main_window, NULL, NULL, NULL, NULL);
  
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (mw->info_text), 
		    0, 4, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    0, 0);
  
  /* The toolbar */
  toolbar = gtk_toolbar_new ();
  gtk_widget_modify_bg (toolbar, GTK_STATE_PRELIGHT, &white);
  gtk_widget_modify_bg (toolbar, GTK_STATE_NORMAL, &white);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (toolbar), FALSE);

  /* Audio Volume */
  item = gtk_tool_item_new ();
  button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  image = gtk_image_new_from_stock (GM_STOCK_AUDIO_VOLUME_HIGH,
                                    GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (button), image);
  gtk_container_add (GTK_CONTAINER (item), button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);
  
  gtk_widget_show (button);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), 
		      GTK_TOOL_ITEM (item), -1);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (item), mw->tips,
			     _("Change the volume of your soundcard"), 
			     NULL);

  gtk_widget_set_size_request (GTK_WIDGET (button), 28, 28);

  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (show_window_cb),
		    (gpointer) mw->audio_settings_window);
  
  /* Video Settings */
  item = gtk_tool_item_new ();
  button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  image = gtk_image_new_from_stock (GM_STOCK_COLOR_BRIGHTNESS_CONTRAST,
                                    GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (button), image);
  gtk_container_add (GTK_CONTAINER (item), button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);
  
  gtk_widget_show (button);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), 
		      GTK_TOOL_ITEM (item), -1);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (item), mw->tips,
			     _("Change the color settings of your video device"), 
			     NULL);

  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (show_window_cb),
		    (gpointer) mw->video_settings_window);

  /* Video Preview Button */
  item = gtk_tool_item_new ();
  mw->preview_button = gtk_toggle_button_new ();
  gtk_button_set_relief (GTK_BUTTON (mw->preview_button), GTK_RELIEF_NONE);
  image = gtk_image_new_from_stock (GM_STOCK_CAMERA_VIDEO, 
                                    GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (mw->preview_button), image);
  gtk_container_add (GTK_CONTAINER (item), mw->preview_button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);
  
  gtk_widget_show (mw->preview_button);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), 
		      GTK_TOOL_ITEM (item), -1);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (item), mw->tips,
			     _("Display images from your camera device"), 
			     NULL);

  g_signal_connect (G_OBJECT (mw->preview_button), "toggled",
		    G_CALLBACK (toolbar_toggle_button_changed_cb),
		    (gpointer) VIDEO_DEVICES_KEY "enable_preview");

  /* Call Pause */
  item = gtk_tool_item_new ();
  mw->hold_button = gtk_toggle_button_new ();
  image = gtk_image_new_from_stock (GM_STOCK_MEDIA_PLAYBACK_PAUSE,
                                    GTK_ICON_SIZE_MENU);
  gtk_button_set_relief (GTK_BUTTON (mw->hold_button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (mw->hold_button), image);
  gtk_container_add (GTK_CONTAINER (item), mw->hold_button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);
  
  gtk_widget_show (mw->hold_button);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), 
		      GTK_TOOL_ITEM (item), -1);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (item), mw->tips,
                             _("Hold the current call"), NULL);
  gtk_widget_set_sensitive (GTK_WIDGET (mw->hold_button), FALSE);

  g_signal_connect (G_OBJECT (mw->hold_button), "clicked",
		    G_CALLBACK (hold_current_call_cb), NULL); 

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (toolbar), 
		    1, 3, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    5, 5);
  
  label = gtk_label_new (_("Call"));

  gtk_notebook_append_page (GTK_NOTEBOOK (mw->main_notebook),
                            event_box, label);
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
    
      GTK_MENU_ENTRY("zoom_in", NULL, _("Zoom in"), 
		     GTK_STOCK_ZOOM_IN, '+', 
		     GTK_SIGNAL_FUNC (zoom_in_changed_cb),
		     (gpointer) VIDEO_DISPLAY_KEY "local_zoom_factor", TRUE),
      GTK_MENU_ENTRY("zoom_out", NULL, _("Zoom out"), 
		     GTK_STOCK_ZOOM_OUT, '-', 
		     GTK_SIGNAL_FUNC (zoom_out_changed_cb),
		     (gpointer) VIDEO_DISPLAY_KEY "local_zoom_factor", TRUE),
      GTK_MENU_ENTRY("normal_size", NULL, _("Normal size"), 
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
    
      GTK_MENU_ENTRY("zoom_in", NULL, _("Zoom in"), 
		     GTK_STOCK_ZOOM_IN, '+', 
		     GTK_SIGNAL_FUNC (zoom_in_changed_cb),
		     (gpointer) VIDEO_DISPLAY_KEY "remote_zoom_factor", TRUE),
      GTK_MENU_ENTRY("zoom_out", NULL, _("Zoom out"), 
		     GTK_STOCK_ZOOM_OUT, '-', 
		     GTK_SIGNAL_FUNC (zoom_out_changed_cb),
		     (gpointer) VIDEO_DISPLAY_KEY "remote_zoom_factor", TRUE),
      GTK_MENU_ENTRY("normal_size", NULL, _("Normal size"), 
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
gboolean
gm_mw_init_fullscreen_video_window (gpointer data)
{
  GmWindow *mw = NULL;
  GdkScreen *defaultscreen;
  int is_Init;

  g_return_val_if_fail (data != NULL, FALSE);

  mw = gm_mw_get_mw (GTK_WIDGET (data));

  g_return_val_if_fail (mw != NULL, FALSE);

  mw->screen = SDL_GetVideoSurface ();
  if (!mw->screen) {
    defaultscreen = gdk_screen_get_default ();

    is_Init = SDL_Init (SDL_INIT_VIDEO);
    g_return_val_if_fail (is_Init >= 0, FALSE);
    mw->screen = SDL_SetVideoMode (gdk_screen_get_width (defaultscreen),
				   gdk_screen_get_height (defaultscreen), 0, 
				   SDL_VIDEO_FLAGS);
    SDL_ShowCursor (SDL_DISABLE);
  }
  return FALSE;
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
        while (SDL_PollEvent (&event)) {}	/* empty SDL event queue */
	return TRUE;
      }
    }
  }

  return FALSE;
}


void
gm_mw_toggle_fullscreen (GtkWidget *main_window)
{
  double zoom = 1.0;

  zoom = gm_conf_get_float (VIDEO_DISPLAY_KEY "zoom_factor");

  if (zoom == -1.0)
    zoom = 1.0;
  else
    zoom = -1.0;
  
  gm_conf_set_float (VIDEO_DISPLAY_KEY "zoom_factor", zoom);
  gm_mw_zooms_menu_update_sensitivity (main_window, zoom);
}


gboolean
gm_mw_destroy_fullscreen_video_window (gpointer data)
{
  GmWindow *mw = NULL;

  g_return_val_if_fail (data != NULL, FALSE);

  mw = gm_mw_get_mw (GTK_WIDGET (data));

  g_return_val_if_fail (mw != NULL, FALSE);

  mw->screen = NULL;		/* do not use surface from now on */
  SDL_Quit ();
  
  return FALSE;
}
#endif


void 
gm_mw_show_video_section (GtkWidget *main_window,
			  gboolean show)
{
  GmWindow *mw = NULL;

  g_return_if_fail (main_window != NULL);
  
  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  if (show)
    gtk_widget_show_all (mw->video_frame);
  else 
    gtk_widget_hide (mw->video_frame);
}


/* GTK callbacks */
static gint
gnomemeeting_tray_hack_cb (gpointer data)
{
  GtkWidget *main_window = NULL;
  GtkWidget *statusicon = NULL;

  statusicon = GnomeMeeting::Process ()->GetStatusicon ();
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  
  gdk_threads_enter ();

  if (!gm_statusicon_is_embedded (statusicon)) {

    gnomemeeting_error_dialog (GTK_WINDOW (main_window), _("Notification area not detected"), _("The notification area is not present in your panel, so Ekiga cannot start hidden."));
    gtk_widget_show (main_window);
  }
  
  gdk_threads_leave ();

  return FALSE;
}


static void 
hold_current_call_cb (GtkWidget *widget,
		      gpointer data)
{
  PString call_token;
  GMManager *endpoint = NULL;

  BOOL is_on_hold = FALSE;
  
  endpoint = GnomeMeeting::Process ()->GetManager ();

  call_token = endpoint->GetCurrentCallToken ();
  is_on_hold = endpoint->IsCallOnHold (call_token);
  if (endpoint->SetCallOnHold (call_token, !is_on_hold))
    is_on_hold = !is_on_hold; /* It worked */
}


static void
pause_current_call_channel_cb (GtkWidget *widget,
			       gpointer data)
{
  GMManager *endpoint = NULL;
  GMVideoGrabber *vg = NULL;

  GtkWidget *main_window = NULL;
 
  PString current_call_token;
  BOOL is_paused = FALSE;
  gint channel_type = 0;
  
  endpoint = GnomeMeeting::Process ()->GetManager ();
  current_call_token = endpoint->GetCurrentCallToken ();
  channel_type = GPOINTER_TO_INT (data);

  g_return_if_fail (CHANNEL_FIRST < channel_type
		    && channel_type < CHANNEL_LAST); 

  main_window = GnomeMeeting::Process ()->GetMainWindow (); 

  if (current_call_token.IsEmpty ()
      && (channel_type == CHANNEL_VIDEO)
      && endpoint->GetCallingState () == GMManager::Standby) {

    gdk_threads_leave ();
    vg = endpoint->GetVideoGrabber ();
    if (vg) {
      
      if (vg->IsGrabbing ()) {

	vg->StopGrabbing ();
	gm_main_window_set_channel_pause (main_window, TRUE, TRUE);
      }
      else {

	vg->StartGrabbing ();
	gm_main_window_set_channel_pause (main_window, FALSE, TRUE);
      }

      vg->Unlock ();
    }
    gdk_threads_enter ();
  }
  else {

    if (channel_type == CHANNEL_AUDIO) {
      
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
  GtkWidget *main_window = NULL;
  
  g_return_if_fail (data != NULL);
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  gm_main_window_transfer_dialog_run (main_window, GTK_WIDGET (data), NULL);  
}


static void
video_window_shown_cb (GtkWidget *w, 
		       gpointer data)
{
  GMManager *endpoint = NULL;

  endpoint = GnomeMeeting::Process ()->GetManager ();

  if (endpoint 
      && gm_conf_get_bool (VIDEO_DISPLAY_KEY "stay_on_top")
      && endpoint->GetCallingState () == GMManager::Connected)
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
  GMManager *ep = NULL;
  
  GtkWidget *main_window = NULL;
  
  g_return_if_fail (data != NULL);
  
  ep = GnomeMeeting::Process ()->GetManager ();
  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  mw = GM_WINDOW (data);

  if (contact && contact->url) {
    
    if (ep->GetCallingState () == GMManager::Connected)
      gm_main_window_transfer_dialog_run (main_window, 
					  main_window, 
					  contact->url);
    else if (ep->GetCallingState () == GMManager::Standby) 
      GnomeMeeting::Process ()->Connect (contact->url);

    gmcontact_delete (contact);
  }
}


static void 
audio_volume_changed_cb (GtkAdjustment *adjustment, 
			 gpointer data)
{
  GMManager *ep = NULL;
  GMPCSSEndpoint *pcssEP = NULL;

  BOOL success = FALSE;

  int play_vol = 0; 
  int rec_vol = 0;

  g_return_if_fail (data != NULL);

  ep = GnomeMeeting::Process ()->GetManager ();
  pcssEP = ep->GetPCSSEndpoint ();

  gm_main_window_get_volume_sliders_values (GTK_WIDGET (data), 
					    play_vol, rec_vol);

  gdk_threads_leave ();
  success = pcssEP->SetDeviceVolume (play_vol, rec_vol);
  gdk_threads_enter ();

  if (!success)
    gm_main_window_set_volume_sliders_values (GTK_WIDGET (data), 0, 0);
}


static void 
video_settings_changed_cb (GtkAdjustment *adjustment, 
			   gpointer data)
{ 
  GMManager *ep = NULL;
  GMVideoGrabber *video_grabber = NULL;

  BOOL success = FALSE;

  int brightness = -1;
  int whiteness = -1;
  int colour = -1;
  int contrast = -1;

  g_return_if_fail (data != NULL);

  ep = GnomeMeeting::Process ()->GetManager ();

  gm_main_window_get_video_sliders_values (GTK_WIDGET (data),
					   whiteness,
					   brightness,
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
  if ((video_grabber = ep->GetVideoGrabber ())) {

    if (whiteness > 0)
      success = video_grabber->SetWhiteness (whiteness << 8);
    if (brightness > 0)
      success = video_grabber->SetBrightness (brightness << 8) || success;
    if (colour > 0)
      success = video_grabber->SetColour (colour << 8) || success;
    if (contrast > 0)
      success = video_grabber->SetContrast (contrast << 8) || success;
    video_grabber->Unlock ();
  }
  gdk_threads_enter ();

  if (!success)
    gm_main_window_set_video_sliders_values (GTK_WIDGET (data), 0, 0, 0, 0);
}


static void 
panel_section_changed_cb (GtkNotebook *notebook, 
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
  gm_conf_set_int (USER_INTERFACE_KEY "main_window/panel_section",
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

  GMManager *endpoint = NULL;

  g_return_if_fail (data != NULL);


  endpoint = GnomeMeeting::Process ()->GetManager ();

  label = ( (GtkBoxChild*) GTK_BOX (gtk_bin_get_child (GTK_BIN (button)) )->children->data )->widget;
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

      endpoint->SendDTMF (call_token, button_text [1]);
      sent = TRUE;
    }
    gdk_threads_enter ();


    /* Update the GUI, ie the URL bar if we are not in a call,
     * and a button press in all cases */
    if (!sent) {

      url += button_text [1];
      gm_main_window_append_call_url (GTK_WIDGET (data), url);
    }
    else
      gm_main_window_flash_message (GTK_WIDGET (data),
				    _("Sent DTMF %c"), 
				    button_text [1]);
  }
}


static gint 
window_closed_cb (GtkWidget *widget, 
		  GdkEvent *event,
		  gpointer data)
{
  GtkWidget *statusicon = NULL;
  GtkWidget *main_window = NULL;

  GmWindow *mw = NULL;

  gboolean b = FALSE;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  statusicon = GnomeMeeting::Process ()->GetStatusicon ();
  mw = gm_mw_get_mw (GTK_WIDGET (main_window));

  b = gm_statusicon_is_embedded (statusicon);

  if (!b)
    quit_callback (NULL, data);
  else 
    gnomemeeting_window_hide (GTK_WIDGET (data));

  return (TRUE);
}  


static void
window_closed_from_menu_cb (GtkWidget *widget,
                           gpointer data)
{
window_closed_cb (widget, NULL, data);
}


static void 
zoom_in_changed_cb (GtkWidget *widget,
		    gpointer data)
{
  GtkWidget *main_window = NULL;
  double zoom = 0.0;
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  g_return_if_fail (main_window != NULL);
  g_return_if_fail (data != NULL);

  zoom = gm_conf_get_float ((char *) data);

  if (zoom < 2.00)
    zoom = zoom * 2.0;

  gm_conf_set_float ((char *) data, zoom);
  gm_mw_zooms_menu_update_sensitivity (main_window, zoom);
}


static void 
zoom_out_changed_cb (GtkWidget *widget,
		     gpointer data)
{
  GtkWidget *main_window = NULL;
  double zoom = 0.0;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  g_return_if_fail (main_window != NULL);
  g_return_if_fail (data != NULL);

  zoom = gm_conf_get_float ((char *) data);

  if (zoom > 0.5)
    zoom = zoom / 2.0;

  gm_conf_set_float ((char *) data, zoom);
  gm_mw_zooms_menu_update_sensitivity (main_window, zoom);
}


static void 
zoom_normal_changed_cb (GtkWidget *widget,
			gpointer data)
{
  GtkWidget *main_window = NULL;
  double zoom = 1.0;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  g_return_if_fail (main_window != NULL);
  g_return_if_fail (data != NULL);

  gm_conf_set_float ((char *) data, zoom);

  gm_mw_zooms_menu_update_sensitivity (main_window, zoom);
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
  GMManager *ep = NULL;

  gchar *url = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  mw = gm_mw_get_mw (main_window); 
  ep = GnomeMeeting::Process ()->GetManager ();

  g_return_if_fail (data != NULL);

  url = g_strdup_printf ("%s#", (gchar *) data);
  gm_main_window_set_call_url (main_window, url);


  /* Directly Connect or run the transfer dialog */
  if (ep->GetCallingState () == GMManager::Connected)
    gm_main_window_transfer_dialog_run (main_window, main_window, url);
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

  gtk_tooltips_set_tip (mw->tips, GTK_WIDGET (e), tip_text, NULL);
}


#ifdef WIN32
static gboolean
thread_safe_notebook_set_page (gpointer data)
{
  GmWindow *mw = NULL;

  GtkWidget *main_window = NULL;
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  mw = gm_mw_get_mw (main_window);

  gdk_threads_enter ();
  gtk_notebook_set_current_page (GTK_NOTEBOOK (mw->main_notebook), 
                                 GPOINTER_TO_INT (data));
  gdk_threads_leave ();

  return FALSE;
}


static gboolean
thread_safe_set_stats_tooltip (gpointer data)
{
  GmWindow *mw = NULL;

  GtkWidget *main_window = NULL;
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  mw = gm_mw_get_mw (main_window);

  gdk_threads_enter ();
  gtk_tooltips_set_tip (mw->tips, mw->statusbar_ebox,
                        (gchar *) data, NULL);
  g_free ((gchar *) data);
  gdk_threads_leave ();

  return FALSE;

}
#endif


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
url_activated_cb (GtkWidget *w,
		  gpointer data)
{
  const char *url = NULL;

  url = gtk_entry_get_text (GTK_ENTRY (w));

  GnomeMeeting::Process ()->Connect (url);
}


static void 
toolbar_toggle_button_changed_cb (GtkWidget *widget, 
				  gpointer data)
{
  bool shown = gm_conf_get_bool ((gchar *) data);

  gm_conf_set_bool ((gchar *) data, !shown);
}


static gboolean 
statusbar_clicked_cb (GtkWidget *widget,
		      GdkEventButton *event,
		      gpointer data)
{
  GMManager *ep = NULL;
  gchar *info = NULL;

  ep = GnomeMeeting::Process ()->GetManager ();

  ep->ResetMissedCallsNumber ();

  info = g_strdup_printf (_("Missed calls: %d - Voice Mails: %s"),
			  ep->GetMissedCallsNumber (),
			  (const char *) ep->GetMWI ());
  gm_main_window_push_info_message (GTK_WIDGET (data), info);
  g_free (info);


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


static void 
show_chat_window_cb (GtkWidget *w,
		     gpointer data)
{
  GtkWidget *statusicon = NULL;

  gchar *name = NULL;
  gchar *url = NULL;

  GMManager *ep = NULL;

  ep = GnomeMeeting::Process ()->GetManager ();
  statusicon = GnomeMeeting::Process ()->GetStatusicon ();

  if (!gnomemeeting_window_is_visible (GTK_WIDGET (data))) {

    /* Check if there is an active call */
    gdk_threads_leave ();
    ep->GetCurrentConnectionInfo (name, url);
    gdk_threads_enter ();

    /* If we are in a call, add a tab with the given URL if there
     * is none.
     */
    if (url && !gm_text_chat_window_has_tab (GTK_WIDGET (data), url)) {

      gm_text_chat_window_add_tab (GTK_WIDGET (data), url, name);
      if (url)
	gm_chat_window_update_calling_state (GTK_WIDGET (data), 
					     name,
					     url, 
					     GMManager::Connected);
    }

    gnomemeeting_window_show (GTK_WIDGET (data));
    gm_statusicon_signal_message (GTK_WIDGET (statusicon), FALSE);
  }
  else
    gnomemeeting_window_hide (GTK_WIDGET (data));
}


static gboolean 
gm_mw_urls_history_update_cb (gpointer data)
{
  GmWindow *mw = NULL;

  GmContact *c = NULL;

  GValue val = {0, };

  GtkWidget *main_window = NULL;

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

  main_window = GTK_WIDGET (data);

  g_return_val_if_fail (main_window != NULL, FALSE);

  mw = gm_mw_get_mw (main_window);


  /* Get the placed calls history */
  g_value_init (&val, G_TYPE_INT);
  g_value_set_int (&val, -1);
  g_object_set_property (G_OBJECT (mw->combo), "active", &val);

  gdk_threads_enter ();
  c2 = gm_calls_history_get_calls (PLACED_CALL, 10, TRUE, TRUE);

  history_model = 
    gtk_combo_box_get_model (GTK_COMBO_BOX (mw->combo));
  gtk_list_store_clear (GTK_LIST_STORE (history_model));
  gdk_threads_leave ();

  iter = c2;
  while (iter) {

    c = GM_CONTACT (iter->data);
    if (c->url && strcmp (c->url, "")) {

      gdk_threads_enter ();
      gtk_combo_box_append_text (GTK_COMBO_BOX (mw->combo), c->url);
      gdk_threads_leave ();
      cpt++;
    }

    iter = g_slist_next (iter);
  }
  g_slist_foreach (c2, (GFunc) gmcontact_delete, NULL);
  g_slist_free (c2);
  c2 = NULL;


  /* Get the full address book */
  gdk_threads_enter ();
  c1 = gnomemeeting_addressbook_get_contacts (NULL,
					      nbr,
					      FALSE,
					      NULL,
					      NULL,
					      NULL,
					      NULL,
					      NULL);


  /* Get the full calls history */
  c2 = gm_calls_history_get_calls (MAX_VALUE_CALL, 25, TRUE, FALSE);
  contacts = g_slist_concat (c1, c2);

  completion = 
    gtk_entry_get_completion (GTK_ENTRY (GTK_BIN (mw->combo)->child));
  cache_model = 
    gtk_entry_completion_get_model (GTK_ENTRY_COMPLETION (completion));
  gtk_list_store_clear (GTK_LIST_STORE (cache_model));
  gdk_threads_leave ();


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

      gdk_threads_enter ();
      gtk_list_store_append (GTK_LIST_STORE (cache_model), &tree_iter);
      gtk_list_store_set (GTK_LIST_STORE (cache_model), &tree_iter, 
			  0, c->fullname,
			  1, c->url,
			  2, (char *) entry, -1);
      gdk_threads_leave ();

      g_free (entry);
    }

    iter = g_slist_next (iter);
  }

  g_slist_foreach (contacts, (GFunc) gmcontact_delete, NULL);
  g_slist_free (contacts);

  return FALSE;
}


static void
gm_mw_zooms_menu_update_sensitivity (GtkWidget *main_window,
			 double zoom)
{
  GmWindow *mw = NULL;

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  if (zoom == -1.0) {
    /* Fullscreen */
    gtk_menu_set_sensitive (mw->main_menu, "zoom_in", FALSE);
    gtk_menu_set_sensitive (mw->main_menu, "zoom_out", FALSE);
    gtk_menu_set_sensitive (mw->main_menu, "normal_size", FALSE);
  }
  else {
    /* between 0.5 and 2.0 zoom */
    gtk_menu_set_sensitive (mw->main_menu, "zoom_in",
				    (zoom == 2.0)?FALSE:TRUE);
    gtk_menu_set_sensitive (mw->main_menu, "zoom_out",
				    (zoom == 0.5)?FALSE:TRUE);
    gtk_menu_set_sensitive (mw->main_menu, "normal_size",
				    (zoom == 1.0)?FALSE:TRUE);
  }
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


static GdkPixbuf *
gm_mw_create_pixbuf_with_frame (GdkPixbuf *picture,
                                GdkPixbuf *frame,
                                double zoom,
                                double frame_ratio,
                                double frame_border)
{
  GdkPixbuf *zframe = NULL;
  GdkPixbuf *zpicture = NULL;
  
  int picture_height = 0;
  int picture_width = 0;
  int frame_height = 0;
  int frame_width = 0;

  int pos_x = 0;
  int pos_y = 0;
  
  picture_width = gdk_pixbuf_get_width (picture);
  picture_height = gdk_pixbuf_get_height (picture);
  frame_width = gdk_pixbuf_get_width (frame);
  frame_height = gdk_pixbuf_get_height (frame);

  zpicture = 
    gdk_pixbuf_scale_simple (picture,
                             (int) (picture_width * zoom),
                             (int) (picture_height * zoom),
                             GDK_INTERP_BILINEAR);
  zframe = 
    gdk_pixbuf_scale_simple (frame,
                             (int) ((picture_width + frame_border * 2) * (zoom * frame_ratio)),
                             (int) ((picture_height + frame_border * 2) * (zoom * frame_ratio)),
                             GDK_INTERP_BILINEAR);

  picture_width = gdk_pixbuf_get_width (zpicture);
  picture_height = gdk_pixbuf_get_height (zpicture);
  frame_width = gdk_pixbuf_get_width (zframe);
  frame_height = gdk_pixbuf_get_height (zframe);

  pos_x = (frame_width - picture_width) / 2;
  pos_y = (frame_height - picture_height) / 2;

  gdk_pixbuf_copy_area (zpicture,
                        0, 0, 
                        picture_width, 
                        picture_height,
                        zframe,
                        pos_x, pos_y);

  g_object_unref (zpicture);

  return zframe;
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

  GdkPixbuf *pic = NULL;
  GdkPixbuf *inside_pic = NULL;

  GdkPixbuf *lsrc_pic = NULL;
  GdkPixbuf *zlsrc_pic = NULL;
  GdkPixbuf *rsrc_pic = NULL;
  GdkPixbuf *zrsrc_pic = NULL;

  GdkPixbuf *inside_shadow_pic = NULL;
  GdkPixbuf *shadow_pic = NULL;

#ifdef HAS_SDL
  Uint32 rmask, gmask, bmask, amask = 0;
  SDL_Surface *lsurface = NULL;
  SDL_Surface *rsurface = NULL;
  SDL_Surface *lblit_conf = NULL;
  SDL_Surface *rblit_conf = NULL;
  SDL_Rect dest;
#endif

  int picture_width = 0;
  int picture_height = 0;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

#ifdef HAS_SDL
  /* FULLSCREEN config */
  const float fs_rratio = 0.7;
  const float fs_lratio = 0.3;
  const float fs_rposx = 0.15;
  const float fs_rposy = 0.0;
  const float fs_lposx = 0.7;
  const float fs_lposy = 0.7;
#endif

  /* booleans to handle some display types syntactically easier */
  gboolean fs_active = FALSE;

  /* Update the display selection in the main and in the video popup menus */
  gtk_radio_menu_select_with_id (mw->main_menu, "local_video", display_type);

#ifdef HAS_SDL
  if (display_type != FULLSCREEN)
    CALL_OR_IDLE_ADD (gm_mw_destroy_fullscreen_video_window, main_window);
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
    CALL_OR_IDLE_ADD(gm_mw_init_fullscreen_video_window, main_window);

    if (gm_mw_poll_fullscreen_video_window (main_window)) 
      gm_mw_toggle_fullscreen (main_window);
  }
#endif
  else {

    if (GTK_WIDGET_VISIBLE (mw->local_video_window))
      gnomemeeting_window_hide (GTK_WIDGET (mw->local_video_window));
    if (GTK_WIDGET_VISIBLE (mw->remote_video_window))
      gnomemeeting_window_hide (GTK_WIDGET (mw->remote_video_window));
  }

#ifdef HAS_SDL
  if (mw->screen != NULL) 
    fs_active = TRUE;
#else
  if (display_type == FULLSCREEN) 
    fs_active = TRUE;
#endif

  /* Create the pixbufs */
  if (display_type != REMOTE_VIDEO 
      && lbuffer && lf_width > 0 && lf_height > 0) {

      lsrc_pic = gdk_pixbuf_new_from_data (lbuffer, GDK_COLORSPACE_RGB, 
                                           FALSE, 8, lf_width, lf_height, 
                                           lf_width * 3, 
                                           NULL, NULL);

  }

  if (display_type != LOCAL_VIDEO 
      && rbuffer && rf_width > 0 && rf_height > 0) {
    
      rsrc_pic = gdk_pixbuf_new_from_data (rbuffer, GDK_COLORSPACE_RGB, 
                                           FALSE, 8, rf_width, rf_height, 
                                           rf_width * 3, 
                                           NULL, NULL);
  }

  shadow_pic = gdk_pixbuf_new_from_xpm_data ((const char **) shadow_xpm);


  switch (display_type) {

  case LOCAL_VIDEO:
    if (lsrc_pic) {

      pic = gm_mw_create_pixbuf_with_frame (lsrc_pic,
                                            shadow_pic,
                                            lzoom,
                                            1.25,
                                            9);
      gtk_image_set_from_pixbuf (GTK_IMAGE (mw->main_video_image), 
				 GDK_PIXBUF (pic));
      gtk_widget_set_size_request (mw->main_video_image,
                                   gdk_pixbuf_get_width (pic),
                                   gdk_pixbuf_get_height (pic));
      g_object_unref (pic);
    }
    break;

  case REMOTE_VIDEO:
    if (rsrc_pic) {

      pic = gm_mw_create_pixbuf_with_frame (rsrc_pic,
                                            shadow_pic,
                                            rzoom,
                                            1.25,
                                            9);
      gtk_image_set_from_pixbuf (GTK_IMAGE (mw->main_video_image), 
				 GDK_PIXBUF (pic));
      gtk_widget_set_size_request (mw->main_video_image,
                                   gdk_pixbuf_get_width (pic),
                                   gdk_pixbuf_get_height (pic));
      g_object_unref (pic);
    }
    break;

  case BOTH_INCRUSTED:

    if (lsrc_pic && rsrc_pic) {

      inside_shadow_pic =
	gdk_pixbuf_new_from_xpm_data ((const char **) gm_both_incrusted_frame_xpm);

      pic = gm_mw_create_pixbuf_with_frame (rsrc_pic,
                                            shadow_pic,
                                            rzoom,
                                            1.25,
                                            9);

      inside_pic = gm_mw_create_pixbuf_with_frame (lsrc_pic,
                                                   inside_shadow_pic,
                                                   lzoom,
                                                   1.00,
                                                   3);

      gdk_pixbuf_copy_area  (inside_pic, 
			     0 , 0,
			     gdk_pixbuf_get_width (inside_pic), 
			     gdk_pixbuf_get_height (inside_pic),
			     pic,
			     (int) (25 * rzoom), 
			     (int) (25 * rzoom));
      gtk_image_set_from_pixbuf (GTK_IMAGE (mw->main_video_image), 
				 GDK_PIXBUF (pic));
      gtk_widget_set_size_request (mw->main_video_image,
                                   gdk_pixbuf_get_width (pic),
                                   gdk_pixbuf_get_height (pic));
      g_object_unref (pic);
      g_object_unref (inside_pic);
      g_object_unref (inside_shadow_pic);
    }
    break;

  case BOTH:

    if (lsrc_pic && rsrc_pic) {
      
      /* Local Image */
      picture_width = gdk_pixbuf_get_width (lsrc_pic);
      picture_height = gdk_pixbuf_get_height (lsrc_pic);
      
      pic = gdk_pixbuf_scale_simple (lsrc_pic,
                                     (int) (picture_width * lzoom),
                                     (int) (picture_height * lzoom),
                                     GDK_INTERP_NEAREST);
      
      gtk_widget_set_size_request (mw->local_video_image,
                                   gdk_pixbuf_get_width (pic),
                                   gdk_pixbuf_get_height (pic));

      gtk_image_set_from_pixbuf (GTK_IMAGE (mw->local_video_image), 
				 GDK_PIXBUF (pic));

      g_object_unref (pic);
      
      /* Remote Image */
      picture_width = gdk_pixbuf_get_width (rsrc_pic);
      picture_height = gdk_pixbuf_get_height (rsrc_pic);
      
      pic = gdk_pixbuf_scale_simple (rsrc_pic,
                                     (int) (picture_width * rzoom),
                                     (int) (picture_height * rzoom),
                                     GDK_INTERP_NEAREST);
      
      gtk_widget_set_size_request (mw->remote_video_image,
                                   gdk_pixbuf_get_width (pic),
                                   gdk_pixbuf_get_height (pic));

      gtk_image_set_from_pixbuf (GTK_IMAGE (mw->remote_video_image), 
				 GDK_PIXBUF (pic));
      
      g_object_unref (pic);
    }
    break;

#ifdef HAS_SDL
  case FULLSCREEN:

    if (rsrc_pic && mw->screen) {
      zrsrc_pic =
	gdk_pixbuf_scale_simple (rsrc_pic,
				 (int) (mw->screen->w * fs_rratio),
				 (int) (mw->screen->h * fs_rratio),
				 GDK_INTERP_NEAREST);
    }

    if (lsrc_pic && mw->screen) {
      zlsrc_pic =
	gdk_pixbuf_scale_simple (lsrc_pic,
				 (int) (mw->screen->w * fs_lratio),
				 (int) (mw->screen->h * fs_lratio),
				 GDK_INTERP_NEAREST);
    }
  

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0x00ff0000;
    gmask = 0x0000ff00;
    bmask = 0x000000ff;
    amask = 0x00000000;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    if (zrsrc_pic && mw->screen) {
      rsurface =
	SDL_CreateRGBSurfaceFrom ((void *) gdk_pixbuf_get_pixels (zrsrc_pic),
				  gdk_pixbuf_get_width (zrsrc_pic),
				  gdk_pixbuf_get_height (zrsrc_pic),
				  24,
				  gdk_pixbuf_get_rowstride (zrsrc_pic), 
				  rmask, gmask, bmask, amask);

      rblit_conf = SDL_DisplayFormat (rsurface);

      dest.x = (int) (mw->screen->w * fs_rposx);
      dest.y = (int) (mw->screen->h * fs_rposy);
      dest.w = rsurface->w;
      dest.h = rsurface->h;

      if (dest.x + dest.w > mw->screen->w) dest.x = mw->screen->w - dest.w;
      if (dest.y + dest.h > mw->screen->h) dest.y = mw->screen->h - dest.h;
      if (dest.x < 0) dest.x = 0; if (dest.y < 0) dest.y = 0;

      SDL_BlitSurface (rblit_conf, NULL, mw->screen, &dest);

      SDL_FreeSurface (rsurface);
      SDL_FreeSurface (rblit_conf);

      g_object_unref (zrsrc_pic);
    }

    if (zlsrc_pic && mw->screen) {
      lsurface =
	SDL_CreateRGBSurfaceFrom ((void *) gdk_pixbuf_get_pixels (zlsrc_pic),
				  gdk_pixbuf_get_width (zlsrc_pic),
				  gdk_pixbuf_get_height (zlsrc_pic),
				  24,
				  gdk_pixbuf_get_rowstride (zlsrc_pic),
				  rmask, gmask, bmask, amask);

      lblit_conf = SDL_DisplayFormat (lsurface);

      dest.x = (int) (mw->screen->w * fs_lposx);
      dest.y = (int) (mw->screen->h * fs_lposy);
      dest.w = lsurface->w;
      dest.h = lsurface->h;

      if (dest.x + dest.w > mw->screen->w) dest.x = mw->screen->w - dest.w;
      if (dest.y + dest.h > mw->screen->h) dest.y = mw->screen->h - dest.h;
      if (dest.x < 0) dest.x = 0; if (dest.y < 0) dest.y = 0;

      SDL_BlitSurface (lblit_conf, NULL, mw->screen, &dest);

      SDL_FreeSurface (lsurface);
      SDL_FreeSurface (lblit_conf);

      g_object_unref (zlsrc_pic);
    }

    if (mw->screen)
      SDL_UpdateRect (mw->screen, 0, 0, mw->screen->w, mw->screen->h);


    break;
#endif
  } 

  if (lsrc_pic)
    g_object_unref (lsrc_pic);

  if (rsrc_pic)
    g_object_unref (rsrc_pic);

  g_object_unref (shadow_pic);
}


void 
gm_main_window_update_logo (GtkWidget *main_window)
{
  GmWindow *mw = NULL;

  GdkPixbuf *tmp = NULL;
  GdkPixbuf *text_logo_pix = NULL;
  GdkPixbuf *shadow_pic = NULL;
  GdkPixbuf *pic = NULL;

  int width = 0;
  int height = 0;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  shadow_pic = gdk_pixbuf_new_from_xpm_data ((const char **) shadow_xpm);
  text_logo_pix = gdk_pixbuf_new_from_xpm_data ((const char **) text_logo_xpm);
  width = gdk_pixbuf_get_width (text_logo_pix);
  height = gdk_pixbuf_get_height (text_logo_pix);

  tmp = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8,
			GM_QCIF_WIDTH, GM_QCIF_HEIGHT);
  gdk_pixbuf_fill (tmp, 0xFFFFFFFF);  

  gdk_pixbuf_copy_area (text_logo_pix, 0, 0, 
			width, height,
			tmp, 
			((GM_QCIF_WIDTH - width) / 2),
			(GM_QCIF_HEIGHT - height) / 2);

  pic = gm_mw_create_pixbuf_with_frame (tmp,
                                        shadow_pic,
                                        1.00,
                                        1.25,
                                        9);

  gtk_image_set_from_pixbuf (GTK_IMAGE (mw->main_video_image),
			     pic);

  gtk_widget_set_size_request (GTK_WIDGET (mw->main_video_image),
                               gdk_pixbuf_get_width (pic),
                               gdk_pixbuf_get_height (pic));

  g_object_unref (text_logo_pix);
  g_object_unref (shadow_pic);
  g_object_unref (pic);
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

    /* Set the audio and video menu to unsensitive */
    gtk_menu_set_sensitive (mw->main_menu, "suspend_audio", FALSE);
    gtk_menu_set_sensitive (mw->main_menu, "suspend_video", FALSE);
    
    gm_main_window_set_channel_pause (main_window, TRUE, FALSE);
    gm_main_window_set_channel_pause (main_window, TRUE, TRUE);
  }
  else {

    if (GTK_IS_LABEL (child))
      gtk_label_set_text_with_mnemonic (GTK_LABEL (child),
					_("_Hold Call"));

    gtk_menu_set_sensitive (mw->main_menu, "suspend_audio", TRUE);
    gtk_menu_set_sensitive (mw->main_menu, "suspend_video", TRUE);

    gm_main_window_set_channel_pause (main_window, FALSE, FALSE);
    gm_main_window_set_channel_pause (main_window, FALSE, TRUE);
  }
  
  g_signal_handlers_block_by_func (G_OBJECT (mw->hold_button),
                                   (gpointer) hold_current_call_cb,
                                   NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mw->hold_button), 
                                is_on_hold);
  g_signal_handlers_unblock_by_func (G_OBJECT (mw->hold_button),
                                     (gpointer) hold_current_call_cb,
                                     NULL);
}


void 
gm_main_window_set_channel_pause (GtkWidget *main_window,
				  gboolean pause,
				  gboolean is_video)
{
  GmWindow *mw = NULL;
  
  GtkWidget *child = NULL;

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
    
    child =
      GTK_BIN (gtk_menu_get_widget (mw->main_menu, "suspend_video"))->child;
  }
  else {
    
    child =
      GTK_BIN (gtk_menu_get_widget (mw->main_menu, "suspend_audio"))->child;
  }
	

  if (GTK_IS_LABEL (child)) 
    gtk_label_set_text_with_mnemonic (GTK_LABEL (child),
				      msg);

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
    case GMManager::Standby:
      
      /* Update the hold state */
      gm_main_window_set_call_hold (main_window, FALSE);

      /* Update the sensitivity, all channels are closed */
      gm_main_window_update_sensitivity (main_window, TRUE, FALSE, FALSE);
      gm_main_window_update_sensitivity (main_window, FALSE, FALSE, FALSE);
      
      /* Update the menus and toolbar items */
      gtk_menu_set_sensitive (mw->main_menu, "connect", TRUE);
      gtk_menu_set_sensitive (mw->main_menu, "disconnect", FALSE);
      gtk_menu_section_set_sensitive (mw->main_menu, "hold_call", FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (mw->hold_button), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (mw->preview_button), TRUE);
      
      /* Update the connect button */
      gm_connect_button_set_connected (GM_CONNECT_BUTTON (mw->connect_button),
				       FALSE);
	
      /* Destroy the incoming call popup */
      if (mw->incoming_call_popup) {

	gnomemeeting_threads_widget_destroy (mw->incoming_call_popup);
	mw->incoming_call_popup = NULL;
      }

      /* Destroy the transfer call popup */
      if (mw->transfer_call_popup) 
	gtk_dialog_response (GTK_DIALOG (mw->transfer_call_popup),
			     GTK_RESPONSE_REJECT);
  
      
      /* Delete the full screen window */
#ifdef HAS_SDL
      CALL_OR_IDLE_ADD(gm_mw_destroy_fullscreen_video_window, main_window);
#endif
      
      
      /* Hide the local and remove video windows */
      gnomemeeting_window_hide (mw->remote_video_window);
      gnomemeeting_window_hide (mw->local_video_window);
	
      break;


    case GMManager::Calling:

      /* Update the menus and toolbar items */
      gtk_menu_set_sensitive (mw->main_menu, "connect", FALSE);
      gtk_menu_set_sensitive (mw->main_menu, "disconnect", TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (mw->preview_button), FALSE);

      /* Update the connect button */
      gm_connect_button_set_connected (GM_CONNECT_BUTTON (mw->connect_button),
				       TRUE);
      
      break;


    case GMManager::Connected:

      /* Update the menus and toolbar items */
      gtk_menu_set_sensitive (mw->main_menu, "connect", FALSE);
      gtk_menu_set_sensitive (mw->main_menu, "disconnect", TRUE);
      gtk_menu_section_set_sensitive (mw->main_menu, "hold_call", TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (mw->hold_button), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (mw->preview_button), FALSE);

      /* Update the connect button */
      gm_connect_button_set_connected (GM_CONNECT_BUTTON (mw->connect_button),
				       TRUE);
      
      /* Destroy the incoming call popup */
      if (mw->incoming_call_popup) {

	gtk_widget_destroy (mw->incoming_call_popup);
	mw->incoming_call_popup = NULL;
      }
      break;


    case GMManager::Called:

      /* Update the menus and toolbar items */
      gtk_menu_set_sensitive (mw->main_menu, "disconnect", TRUE);

      /* Update the connect button */
      gm_connect_button_set_connected (GM_CONNECT_BUTTON (mw->connect_button),
				       FALSE);
      
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
  
  double zoom = 1.0;

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  zoom = gm_conf_get_float (VIDEO_DISPLAY_KEY "zoom_factor");

  
  /* We are updating video related items */
  if (is_video) {

    gm_mw_zooms_menu_update_sensitivity (main_window, zoom);

    /* Receiving and sending => Everything sensitive in the section control */
    if (is_receiving && is_transmitting) {
      gtk_menu_section_set_sensitive (mw->main_menu,
				      "fullscreen", TRUE);

      gtk_menu_section_set_sensitive (mw->main_menu,
				      "local_video", TRUE);
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
	/* set the sensitivity of the zoom related menuitems as
	 * if we were in fullscreen -> disable
	 * all: zoom_{in,out},normal_size */
	gm_mw_zooms_menu_update_sensitivity (main_window, -1.0);
	gtk_menu_section_set_sensitive (mw->main_menu,
					"fullscreen", FALSE);
	gtk_menu_set_sensitive (mw->main_menu, "save_picture", FALSE);
      }
      else {
	/* Or activate it as at least something is transmitted or 
	 * received */
	gm_mw_zooms_menu_update_sensitivity (main_window, zoom);
	gtk_menu_section_set_sensitive (mw->main_menu,
					"fullscreen", is_receiving?TRUE:FALSE);
	  
	gtk_menu_set_sensitive (mw->main_menu, "save_picture", TRUE);
      }
    }
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

  if (is_video) {

    g_signal_handlers_block_by_func (G_OBJECT (mw->preview_button),
                                     (gpointer) (toolbar_toggle_button_changed_cb),
                                     (gpointer) VIDEO_DEVICES_KEY "enable_preview");

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mw->preview_button), is_transmitting);

    g_signal_handlers_unblock_by_func (G_OBJECT (mw->preview_button),
                                       (gpointer) (toolbar_toggle_button_changed_cb),
                                       (gpointer) VIDEO_DEVICES_KEY "enable_preview");
  }
}


void gm_main_window_update_contact_presence (GtkWidget *main_window, 
                                             const PString & user, 
                                             ContactState state)
{
  GmWindow *mw = NULL;
  
  mw = gm_mw_get_mw (main_window);

  gmroster_presence_set_status (GMROSTER (mw->roster), 
                                (gchar *) (const char *) user, 
                                state);
}


void
gm_main_window_set_busy (GtkWidget *main_window,
			 BOOL busy)
{
  GmWindow *mw = NULL;
  
  GdkCursor *cursor = NULL;

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  gtk_menu_section_set_sensitive (mw->main_menu, "quit", !busy);

  if (busy) {

    cursor = gdk_cursor_new (GDK_WATCH);
    gdk_window_set_cursor (GTK_WIDGET (main_window)->window, cursor);
    gdk_cursor_unref (cursor);
  }
  else
    gdk_window_set_cursor (GTK_WIDGET (main_window)->window, NULL);
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
gm_main_window_set_signal_levels (GtkWidget *main_window,
				  float output, 
				  float input)
{
  GmWindow *mw = NULL;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  if (output >= 0)
    gtk_levelmeter_set_level (GTK_LEVELMETER (mw->output_signal), output);
  
  if (input >= 0)
    gtk_levelmeter_set_level (GTK_LEVELMETER (mw->input_signal), input);
}


void
gm_main_window_clear_signal_levels (GtkWidget *main_window)
{
  GmWindow *mw = NULL;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  gtk_levelmeter_clear (GTK_LEVELMETER (mw->output_signal));
  gtk_levelmeter_clear (GTK_LEVELMETER (mw->input_signal));
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
gm_main_window_set_panel_section (GtkWidget *main_window,
                                  int section)
{
  GmWindow *mw = NULL;
  
  GtkWidget *menu = NULL;
  
  g_return_if_fail (main_window != NULL);
  
  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

#ifndef WIN32
  gtk_notebook_set_current_page (GTK_NOTEBOOK (mw->main_notebook), section);
#else
  g_idle_add (thread_safe_notebook_set_page, GINT_TO_POINTER (section));
#endif
  
  menu = gtk_menu_get_widget (mw->main_menu, "dialpad");
  
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
gm_main_window_set_call_info (GtkWidget *main_window,
			      const char *tr_audio_codec,
			      const char *re_audio_codec,
			      const char *tr_video_codec,
			      const char *re_video_codec)
{
  GmWindow *mw = NULL;

  GtkTextIter iter;
  GtkTextIter *end_iter = NULL;
  GtkTextBuffer *buffer = NULL;
  
  gchar *info = NULL;
  
  g_return_if_fail (main_window != NULL);
  
  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  if (!tr_audio_codec && !tr_video_codec)
    info = g_strdup (" ");
  else
    info = g_strdup_printf ("%s - %s",
                            tr_audio_codec?tr_audio_codec:"", 
                            tr_video_codec?tr_video_codec:"");
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (mw->info_text));
  gtk_text_buffer_get_start_iter (buffer, &iter);
  gtk_text_iter_forward_lines (&iter, 2);
  end_iter = gtk_text_iter_copy (&iter);
  gtk_text_iter_forward_line (end_iter);
  gtk_text_buffer_delete (buffer, &iter, end_iter);
  gtk_text_iter_free (end_iter);
  gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, info, 
                                            -1, "codecs", NULL);
  g_free (info);
}


void 
gm_main_window_set_account_info (GtkWidget *main_window,
				 int registered_accounts)
{
  /*GmWindow *mw = NULL;
  
  GtkTextIter iter, end_iter;
  GtkTextBuffer *buffer = NULL;
  GtkTextMark *mark = NULL;

  gchar *info = NULL;
  
  g_return_if_fail (main_window != NULL);
  
  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  info = g_strdup_printf ("%s %d",
                          _("Registered accounts:"),
                          registered_accounts);
		   
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (mw->info_text));
  gtk_text_iter_forward_line (&iter);
  gtk_text_buffer_get_iter_at_line (buffer, &iter, 1);
  mark = gtk_text_buffer_get_mark (buffer, "account-info");
  if (mark) {

    gtk_text_buffer_get_iter_at_mark (buffer, &end_iter, mark);
    gtk_text_buffer_delete (buffer, &iter, &end_iter);

  }
  gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, info, 
                                            -1, "status", NULL);
  gtk_text_buffer_create_mark (buffer, "account-info", &iter, FALSE);
  g_free (info);
  */
}


void 
gm_main_window_set_status (GtkWidget *main_window,
			   const char *status)
{
  GmWindow *mw = NULL;

  GtkTextIter iter;
  GtkTextIter* end_iter = NULL;
  GtkTextBuffer *buffer = NULL;

  gchar *info = NULL;
  
  g_return_if_fail (main_window != NULL);
  
  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);
  
  info = g_strdup_printf ("%s\n", status);
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (mw->info_text));
  gtk_text_buffer_get_start_iter (buffer, &iter);
  end_iter = gtk_text_iter_copy (&iter);
  gtk_text_iter_forward_line (end_iter);
  gtk_text_buffer_delete (buffer, &iter, end_iter);
  gtk_text_iter_free (end_iter);
  gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, info, 
                                            -1, "status", NULL);
  g_free (info);
}


void 
gm_main_window_set_call_duration (GtkWidget *main_window,
                                  const char *duration)
{
  GmWindow *mw = NULL;

  GtkTextIter iter;
  GtkTextIter* end_iter = NULL;
  GtkTextBuffer *buffer = NULL;

  gchar *info = NULL;

  g_return_if_fail (main_window != NULL);
  
  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);
  
  if (duration)
    info = g_strdup_printf (_("Call Duration: %s\n"), duration);
  else
    info = g_strdup ("\n");
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (mw->info_text));
  gtk_text_buffer_get_start_iter (buffer, &iter);
  gtk_text_iter_forward_line (&iter);
  end_iter = gtk_text_iter_copy (&iter);
  gtk_text_iter_forward_line (end_iter);
  gtk_text_buffer_delete (buffer, &iter, end_iter);
  gtk_text_iter_free (end_iter);
  gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, info, 
                                            -1, "call-duration", NULL);

  g_free (info);
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
  g_idle_add (gm_mw_urls_history_update_cb, main_window);
}


gboolean 
gm_main_window_transfer_dialog_run (GtkWidget *main_window,
				    GtkWidget *parent_window,
				    const char *u)
{
  GMManager *endpoint = NULL;
  GmWindow *mw = NULL;
  
  GMURL url = GMURL (u);
 
  gint answer = 0;
  
  const char *forward_url = NULL;

  g_return_val_if_fail (main_window != NULL, FALSE);
  g_return_val_if_fail (parent_window != NULL, FALSE);
  
  mw = gm_mw_get_mw (main_window);

  g_return_val_if_fail (mw != NULL, FALSE);
  

  endpoint = GnomeMeeting::Process ()->GetManager ();
  
  mw->transfer_call_popup = 
    gm_entry_dialog_new (_("Transfer call to:"),
			 _("Transfer"));
  
  gtk_window_set_transient_for (GTK_WINDOW (mw->transfer_call_popup),
				GTK_WINDOW (parent_window));
  
  gtk_dialog_set_default_response (GTK_DIALOG (mw->transfer_call_popup),
				   GTK_RESPONSE_ACCEPT);
  
  if (!url.IsEmpty ())
    gm_entry_dialog_set_text (GM_ENTRY_DIALOG (mw->transfer_call_popup), u);
  else
    gm_entry_dialog_set_text (GM_ENTRY_DIALOG (mw->transfer_call_popup),
			      (const char *) url.GetDefaultURL ());

  gnomemeeting_threads_dialog_show (mw->transfer_call_popup);

  answer = gtk_dialog_run (GTK_DIALOG (mw->transfer_call_popup));
  switch (answer) {

  case GTK_RESPONSE_ACCEPT:

    forward_url =
      gm_entry_dialog_get_text (GM_ENTRY_DIALOG (mw->transfer_call_popup));
    new GMURLHandler (forward_url, TRUE);
      
    break;

  default:
    break;
  }

  gtk_widget_destroy (mw->transfer_call_popup);
  mw->transfer_call_popup = NULL;

  return (answer == GTK_RESPONSE_ACCEPT);
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
			      _("Reject"), 0);
  b3 = gtk_dialog_add_button (GTK_DIALOG (mw->incoming_call_popup),
			      _("Transfer"), 1);
  b1 = gtk_dialog_add_button (GTK_DIALOG (mw->incoming_call_popup),
			      _("Accept"), 2);

  gtk_dialog_set_default_response (GTK_DIALOG (mw->incoming_call_popup), 2);

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
		       _("Remote URI:"), utf8_url);
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

  
  gtk_window_set_title (GTK_WINDOW (mw->incoming_call_popup), utf8_name);
  gtk_window_set_modal (GTK_WINDOW (mw->incoming_call_popup), TRUE);
  gtk_window_set_keep_above (GTK_WINDOW (mw->incoming_call_popup), TRUE);
#if GTK_MINOR_VERSION >= 8
  gtk_window_set_urgency_hint (GTK_WINDOW (mw->incoming_call_popup), TRUE);
#endif
  gtk_window_set_transient_for (GTK_WINDOW (mw->incoming_call_popup),
				GTK_WINDOW (main_window));

  
  g_signal_connect (G_OBJECT (b1), "clicked",
		    GTK_SIGNAL_FUNC (connect_cb), main_window);
  g_signal_connect (G_OBJECT (b2), "clicked",
		    GTK_SIGNAL_FUNC (disconnect_cb), NULL);
  g_signal_connect (G_OBJECT (b3), "clicked",
		    GTK_SIGNAL_FUNC (transfer_current_call_cb), 
		    mw->incoming_call_popup);
  
  g_signal_connect_swapped (G_OBJECT (b1), "clicked",
			    GTK_SIGNAL_FUNC (gtk_widget_hide), 
			    mw->incoming_call_popup);
  g_signal_connect_swapped (G_OBJECT (b2), "clicked",
			    GTK_SIGNAL_FUNC (gtk_widget_hide),
			    mw->incoming_call_popup);

  g_signal_connect (G_OBJECT (mw->incoming_call_popup), "delete-event",
		    GTK_SIGNAL_FUNC (delete_incoming_call_dialog_cb), 
		    main_window);

  gtk_widget_show_all (vbox);
  gnomemeeting_threads_dialog_show (mw->incoming_call_popup);
}


GtkWidget *
gm_main_window_new ()
{
  GmWindow *mw = NULL;

  GtkWidget *window = NULL;
  GtkWidget *table = NULL;
  GtkWidget *hbox = NULL;
  
  GdkPixbuf *pixbuf = NULL;

  PanelSection section = DIALPAD;
  
  /* The Top-level window */
#ifndef DISABLE_GNOME
  window = gnome_app_new ("gnomemeeting", NULL);
#else
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
#endif
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("main_window"), g_free);

  gtk_window_set_title (GTK_WINDOW (window), 
			_("Ekiga"));
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
  gtk_container_set_border_width (GTK_CONTAINER (table), 6);
#ifdef DISABLE_GNOME
  gtk_box_pack_start (GTK_BOX (mw->window_hbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);
#else
  gnome_app_set_contents (GNOME_APP (window), table);
#endif
  gtk_widget_show (table);

  /* The Audio & Video Settings windows */
  mw->audio_settings_window = gm_mw_audio_settings_window_new (window);
  mw->video_settings_window = gm_mw_video_settings_window_new (window);

  /* The Notebook */
  mw->main_notebook = gtk_notebook_new ();
  gtk_notebook_popup_enable (GTK_NOTEBOOK (mw->main_notebook));
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (mw->main_notebook), TRUE);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (mw->main_notebook), TRUE);

  gm_mw_init_contacts_list (window);
  gm_mw_init_dialpad (window);
  gm_mw_init_calls_history (window);
  gm_mw_init_call (window);

  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (mw->main_notebook),
		    0, 2, 2, 3,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    3, 3); 

  section = (PanelSection) 
    gm_conf_get_int (USER_INTERFACE_KEY "main_window/panel_section");
  gtk_widget_show_all (GTK_WIDGET (mw->main_notebook));
  gm_main_window_set_panel_section (window, section);
  
  /* The statusbar with qualitymeter */
  hbox = gtk_hbox_new (FALSE, 1);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
    
  mw->qualitymeter = gm_powermeter_new ();
  gtk_box_pack_start (GTK_BOX (hbox), mw->qualitymeter,
		      FALSE, FALSE, 2);

  mw->statusbar_ebox = gtk_event_box_new ();
  mw->statusbar = gm_statusbar_new ();
  gtk_box_pack_start (GTK_BOX (hbox), mw->statusbar_ebox,
		      TRUE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (mw->statusbar_ebox), mw->statusbar);


#ifdef DISABLE_GNOME
  gtk_box_pack_start (GTK_BOX (mw->window_vbox), hbox, 
		      FALSE, FALSE, 0);
#else
  gnome_app_set_statusbar_custom (GNOME_APP (window), 
				  hbox, mw->statusbar);
#endif
  gtk_widget_show_all (hbox);
  
  g_signal_connect (G_OBJECT (mw->statusbar_ebox), "button-press-event",
		    GTK_SIGNAL_FUNC (statusbar_clicked_cb), window);
 
  pixbuf = gtk_widget_render_icon (GTK_WIDGET (window),
				   GM_STOCK_LOGO_SMALL,
				   GTK_ICON_SIZE_MENU, NULL);

  if (pixbuf) {
    gtk_window_set_default_icon (pixbuf);
    g_object_unref (G_OBJECT (pixbuf));
  }
  
  /* Add the window icon and title */
  gtk_window_set_title (GTK_WINDOW (window), _("Ekiga"));

  gtk_widget_realize (window);
  gtk_window_set_resizable (GTK_WINDOW (window), TRUE);

  g_signal_connect_after (G_OBJECT (mw->main_notebook), "switch-page",
			  G_CALLBACK (panel_section_changed_cb), 
			  window);

  
  /* Init the Drag and drop features */
  gmcontacts_dnd_set_dest (GTK_WIDGET (window), dnd_call_contact_cb, mw);

  
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
  GmWindow *mw = NULL;

  char buffer [1025];

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  va_list args;

  va_start (args, msg);
  vsnprintf (buffer, 1024, msg, args);
  gm_statusbar_flash_message (GM_STATUSBAR (mw->statusbar), buffer);
  va_end (args);
}


void 
gm_main_window_push_message (GtkWidget *main_window, 
			     int missed,
			     const char *vm)
{
  GmWindow *mw = NULL;

  gchar *info = NULL;
  
  g_return_if_fail (main_window != NULL);
  g_return_if_fail (vm != NULL);

  mw = gm_mw_get_mw (main_window);
  
  info = g_strdup_printf (_("Missed calls: %d - Voice Mails: %s"), missed, vm);
  gm_main_window_push_info_message (main_window, info);

  g_free (info);
}


void 
gm_main_window_push_message (GtkWidget *main_window, 
			     const char *msg, 
			     ...)
{
  GmWindow *mw = NULL;

  char buffer [1025];

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  va_list args;

  va_start (args, msg);
  vsnprintf (buffer, 1024, msg, args);
  gm_statusbar_push_message (GM_STATUSBAR (mw->statusbar), buffer);
  va_end (args);
}


void 
gm_main_window_push_info_message (GtkWidget *main_window, 
				  const char *msg, 
				  ...)
{
  GmWindow *mw = NULL;
  
  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  va_list args;

  va_start (args, msg);
  gm_statusbar_push_info_message (GM_STATUSBAR (mw->statusbar), msg, args);
  va_end (args);
}


void 
gm_main_window_set_call_url (GtkWidget *main_window, 
			     const char *url)
{
  GmWindow *mw = NULL;

  GtkWidget *entry = NULL;
  GtkEntryCompletion *completion = NULL;

  g_return_if_fail (main_window != NULL && url != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  entry = GTK_WIDGET (GTK_BIN (mw->combo)->child);
  completion = 
    gtk_entry_get_completion (GTK_ENTRY (GTK_BIN (mw->combo)->child));
 
  gtk_entry_completion_set_popup_completion (completion, FALSE);
  gtk_entry_set_text (GTK_ENTRY (entry), url);
  gtk_editable_set_position (GTK_EDITABLE (entry), -1);
  gtk_widget_grab_focus (GTK_WIDGET (entry));
  gtk_editable_select_region (GTK_EDITABLE (entry), -1, -1);
  gtk_entry_completion_set_popup_completion (completion, TRUE);
}


void 
gm_main_window_append_call_url (GtkWidget *main_window, 
				const char *url)
{
  GmWindow *mw = NULL;
  
  GtkWidget *entry = NULL;
  GtkEntryCompletion *completion = NULL;

  int pos = -1;

  g_return_if_fail (main_window != NULL && url != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL && url != NULL);
 
  entry = GTK_WIDGET (GTK_BIN (mw->combo)->child);

  if (gtk_editable_get_selection_bounds (GTK_EDITABLE (entry), NULL, NULL)) {

    gtk_editable_delete_selection (GTK_EDITABLE (entry));
    pos = gtk_editable_get_position (GTK_EDITABLE (entry));
  }

  completion = 
    gtk_entry_get_completion (GTK_ENTRY (GTK_BIN (mw->combo)->child));
  gtk_entry_completion_set_popup_completion (completion, FALSE);
  gtk_editable_insert_text (GTK_EDITABLE (entry), url, strlen (url), &pos);
  gtk_widget_grab_focus (GTK_WIDGET (entry));
  gtk_editable_select_region (GTK_EDITABLE (entry), -1, -1);
  gtk_entry_completion_set_popup_completion (completion, TRUE);
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
gm_main_window_clear_stats (GtkWidget *main_window)
{
  GmWindow *mw = NULL;

  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  gm_main_window_update_stats (main_window, 0, 0, 0, 0, 0, 0, 0, 0);
  if (mw->qualitymeter)
    gm_powermeter_set_level (GM_POWERMETER (mw->qualitymeter), 0.0);
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
  int jitter_quality = 0;
  gfloat quality_level = 0.0;

  
  g_return_if_fail (main_window != NULL);

  mw = gm_mw_get_mw (main_window);

  g_return_if_fail (mw != NULL);

  stats_msg = g_strdup_printf (_("Lost packets: %.1f %%\nLate packets: %.1f %%\nOut of order packets: %.1f %%\nJitter buffer: %d ms"), lost, late, out_of_order, jitter);

  if (mw->statusbar_ebox) {

#ifndef WIN32
    gtk_tooltips_set_tip (mw->tips, GTK_WIDGET (mw->statusbar_ebox), 
                          stats_msg, NULL);
#else
    g_idle_add (thread_safe_set_stats_tooltip, g_strdup (stats_msg));
#endif
  }
  g_free (stats_msg);

  /* "arithmetics" for the quality level */
  /* Thanks Snark for the math hints */
  if (jitter < 30)
    jitter_quality = 100;
  if (jitter >= 30 && jitter < 50)
    jitter_quality = 100 - (jitter - 30);
  if (jitter >= 50 && jitter < 100)
    jitter_quality = 80 - (jitter - 50) * 20 / 50;
  if (jitter >= 100 && jitter < 150)
    jitter_quality = 60 - (jitter - 100) * 20 / 50;
  if (jitter >= 150 && jitter < 200)
    jitter_quality = 40 - (jitter - 150) * 20 / 50;
  if (jitter >= 200 && jitter < 300)
    jitter_quality = 20 - (jitter - 200) * 20 / 100;
  if (jitter >= 300 || jitter_quality < 0)
    jitter_quality = 0;

  quality_level = (float) jitter_quality / 100;

  if ((lost < 0.02 && lost != 0.0) ||
      (late < 0.02 && late != 0.0) ||
      (out_of_order < 0.02 && out_of_order != 0.0) &&
      quality_level > 0.2)
    quality_level = 0.2;

  if (mw->qualitymeter)
    gm_powermeter_set_level (GM_POWERMETER (mw->qualitymeter),
			     quality_level);
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
  GOptionContext *context = NULL;

  GtkWidget *main_window = NULL;
  GtkWidget *druid_window = NULL;

  GtkWidget *dialog = NULL;
  
  gchar *path = NULL;
  gchar *url = NULL;
  gchar *key_name = NULL;
  gchar *msg = NULL;
  gchar *title = NULL;

  int debug_level = 0;
  int error = -1;
#ifndef DISABLE_GNOME
  GnomeProgram *program;
#endif

  /* Globals */
#ifndef WIN32
  setenv ("ESD_NO_SPAWN", "1", 1);
#endif

  /* PWLIB initialization */
  PProcess::PreInitialise (argc, argv, envp);
  
  /* GTK+ initialization */
  g_thread_init (NULL);
  gdk_threads_init ();
  gtk_init (&argc, &argv);
#ifndef WIN32
  signal (SIGPIPE, SIG_IGN);
#endif

  /* Configuration backend initialization */
  gm_conf_init (argc, argv);

  /* Gettext initialization */
  path = g_build_filename (DATA_DIR, "locale", NULL);
  textdomain (GETTEXT_PACKAGE);
  bindtextdomain (GETTEXT_PACKAGE, path);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  g_free (path);

  /* Arguments initialization */
  GOptionEntry arguments [] =
    {
      {
	"debug", 'd', 0, G_OPTION_ARG_INT, &debug_level, 
       N_("Prints debug messages in the console (level between 1 and 6)"), 
       NULL
      },
      {
	"call", 'c', 0, G_OPTION_ARG_STRING, &url,
	N_("Makes Ekiga call the given URI"),
	NULL
      },
      {
	NULL
      }
    };
  context = g_option_context_new (NULL);
  g_option_context_add_main_entries (context, arguments, PACKAGE_NAME);
  g_option_context_set_help_enabled (context, TRUE);
  
  /* GNOME Initialisation */
#ifndef DISABLE_GNOME
  program = gnome_program_init (PACKAGE_NAME, VERSION,
			        LIBGNOMEUI_MODULE, argc, argv,
			        GNOME_PARAM_GOPTION_CONTEXT, context,
			        GNOME_PARAM_HUMAN_READABLE_NAME, "ekiga",
			        GNOME_PARAM_APP_DATADIR, DATA_DIR,
			        (void *) NULL);
#else
  g_option_context_parse (context, &argc, &argv, NULL);
  g_option_context_free (context);
#endif
  
  gdk_threads_enter ();
 
  /* BONOBO initialization */
#ifndef DISABLE_GNOME
  if (bonobo_component_init (argc, argv))
    exit (1);
#endif

  /* Ekiga initialisation */
  static GnomeMeeting instance;
  if (debug_level != 0)
    PTrace::Initialise (PMAX (PMIN (4, debug_level), 0), NULL,
			PTrace::Timestamp | PTrace::Thread
			| PTrace::Blocks | PTrace::DateAndTime);
  if (!GnomeMeeting::Process ()->DetectDevices ()) 
    error = 1;
  GnomeMeeting::Process ()->BuildGUI ();
  GnomeMeeting::Process ()->DetectInterfaces ();
  GnomeMeeting::Process ()->Init ();
  if (!GnomeMeeting::Process ()->DetectCodecs ()) 
    error = 2;
  
  /* Configuration database initialization */
  if (!gnomemeeting_conf_check ()) 
    error = 3;
  gnomemeeting_conf_init ();

  /* Show the window if there is no error, exit with a popup if there
   * is a fatal error.
   */
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  druid_window = GnomeMeeting::Process ()->GetDruidWindow ();
  if (error == -1) {

    if (gm_conf_get_int (GENERAL_KEY "version") 
        < 1000 * MAJOR_VERSION + 10 * MINOR_VERSION + BUILD_NUMBER) {

      gnomemeeting_conf_upgrade ();
      gtk_widget_show_all (GTK_WIDGET (druid_window));
    }
    else {

      /* Show the main window */
      if (!gm_conf_get_bool (USER_INTERFACE_KEY "start_hidden")) 
        gnomemeeting_window_show (main_window);
      else
        g_timeout_add (15000, (GtkFunction) gnomemeeting_tray_hack_cb, NULL);
    }

    /* Call the given host if needed */
    if (url) 
      GnomeMeeting::Process ()->Connect (url);
  }
  else {

    switch (error) {

    case 1:
      title = g_strdup (_("No usable audio plugin detected"));
      msg = g_strdup (_("Ekiga didn't find any usable audio plugin. Make sure that your installation is correct."));
      break;
    case 2:
      title = g_strdup (_("No usable audio codecs detected"));
      msg = g_strdup (_("Ekiga didn't find any usable audio codec. Make sure that your installation is correct."));
      break;
    case 3:
      key_name = g_strdup ("\"/apps/" PACKAGE_NAME "/general/gconf_test_age\"");
      title = g_strdup (_("Configuration database corruption"));
      msg = g_strdup_printf (_("Ekiga got an invalid value for the configuration key %s.\n\nIt probably means that your configuration schemas have not been correctly installed or the that the permissions are not correct.\n\nPlease check the FAQ (http://www.ekiga.org/), the troubleshooting section of the GConf site (http://www.gnome.org/projects/gconf/) or the mailing list archives for more information (http://mail.gnome.org) about this problem."), key_name);
      g_free (key_name);
      break;
    }

    dialog = gtk_message_dialog_new (GTK_WINDOW (main_window), 
                                     GTK_DIALOG_MODAL, 
                                     GTK_MESSAGE_ERROR,
                                     GTK_BUTTONS_OK, NULL);

    gtk_window_set_title (GTK_WINDOW (dialog), title);
    gtk_label_set_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (dialog)->label), msg);
  
    g_signal_connect (GTK_OBJECT (dialog), "response",
                      G_CALLBACK (quit_callback),
                      GTK_OBJECT (dialog));
    g_signal_connect_swapped (GTK_OBJECT (dialog), "response",
                              G_CALLBACK (gtk_widget_destroy),
                              GTK_OBJECT (dialog));
  
    gtk_widget_show_all (dialog);

    g_free (title);
    g_free (msg);
  }

  /* The GTK loop */
  gtk_main ();
  gdk_threads_leave ();

  /* Exit Ekiga */
  GnomeMeeting::Process ()->Exit ();

  /* Save the configuration */
  gm_conf_save ();

#ifndef DISABLE_GNOME
  g_object_unref (program);
#endif

  return 0;
}


#ifdef WIN32

typedef struct {
  int newmode;
} _startupinfo;

extern "C" void __getmainargs (int *argcp, char ***argvp, char ***envp, int glob, _startupinfo *sinfo);

int 
APIENTRY WinMain (HINSTANCE hInstance,
		  HINSTANCE hPrevInstance,
		  LPSTR     lpCmdLine,
		  int       nCmdShow)
{
  HANDLE ekr_mutex;
  int iresult;
  char **env;
  char **argv;
  int argc;
  _startupinfo info = {0};

  ekr_mutex = CreateMutex (NULL, FALSE, "EkigaIsRunning");
  if (GetLastError () == ERROR_ALREADY_EXISTS)
    MessageBox (NULL, "Ekiga is running already !", "Ekiga - 2nd instance", MB_ICONEXCLAMATION | MB_OK);
  else {

    /* use msvcrt.dll to parse command line */
    __getmainargs (&argc, &argv, &env, 0, &info);

    std::freopen("stdout.txt", "w", stdout);
    std::freopen("stderr.txt", "w", stderr);

#ifdef HAS_SDL
    atexit (SDL_Quit);
#endif
    iresult = main (argc, argv, env);
  }
  CloseHandle (ekr_mutex);
  return iresult;
}
#endif

