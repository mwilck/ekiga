
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
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         pref_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Tue Dec 26 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          create the preferences window and all its callbacks
 *   Additional code      : Miguel Rodríguez Pérez  <miguelrp@gmail.com> 
 */


#include "../config.h"

#include "pref_window.h"

#include "accounts.h"
#include "h323endpoint.h"
#include "sipendpoint.h"
#include "gnomemeeting.h"
#include "ils.h"
#include "sound_handling.h"
#include "misc.h"
#include "urlhandler.h"
#include "callbacks.h"
#include "lid.h"

#include "dialog.h"
#include "gnome_prefs_window.h"
#include "gm_conf.h"




typedef struct _GmPreferencesWindow
{
  GtkWidget *audio_codecs_list;
  GtkWidget *accounts_list;
  GtkWidget *accounts_box;
  GtkWidget *sound_events_list;
  GtkWidget *audio_player;
  GtkWidget *sound_events_output;
  GtkWidget *audio_recorder;
  GtkWidget *video_device;
} GmPreferencesWindow;

#define GM_PREFERENCES_WINDOW(x) (GmPreferencesWindow *) (x)


typedef struct GmAccountsWindow_ {

  GtkWidget *account_entry;
  GtkWidget *protocol_option_menu;
  GtkWidget *host_label;
  GtkWidget *host_entry;
  GtkWidget *username_entry;
  GtkWidget *password_entry;
  GtkWidget *domain_label;
  GtkWidget *domain_entry;
} GmAccountsWindow;

#define GM_ACCOUNTS_WINDOW(x) (GmAccountsWindow *) (x)


/* Declarations */

/* GUI Functions */


/* DESCRIPTION  : /
 * BEHAVIOR     : Frees a GmPreferencesWindow and its content.
 * PRE          : A non-NULL pointer to a GmPreferencesWindow structure.
 */
static void gm_pw_destroy (gpointer);


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns a pointer to the private GmPrerencesWindow structure
 *                used by the preferences window GMObject.
 * PRE          : The given GtkWidget pointer must be a preferences window 
 * 		  GMObject.
 */
static GmPreferencesWindow *gm_pw_get_pw (GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Creates a GtkTreeView able to display the list of VoIP
 * 		  accounts.
 * PRE          : /
 */
static GtkWidget *gm_accounts_list_new (); 


/* DESCRIPTION  : /
 * BEHAVIOR     : Creates a GtkBox containing the accounts and buttons
 * 		  to edit them.
 * PRE          : /
 */
static GtkWidget *gm_accounts_list_box_new (GtkWidget *);
	

/* DESCRIPTION  : /
 * BEHAVIOR     : Creates and run a dialog where the user can edit a new
 * 		  or an already selected account. It also checks if all
 * 		  required parameters are present, and once a valid choice
 * 		  has been entered and validated, it is added/modified in
 * 		  the accounts database.
 * PRE          : The GmAccount is a valid pointer to a valid GmAccount
 * 		  that we are editing. If it is NULL, it means we
 * 		  are creating a new account.
 */
static void gm_pw_edit_account_dialog_run (GtkWidget *prefs_window,
					   GmAccount *account,
					   GtkWidget *parent_window);


/* DESCRIPTION  : /
 * BEHAVIOR     : Creates and run a dialog asking to the user if he wants
 * 		  to delete the given account.
 * PRE          : The GmAccount is a valid pointer to a valid GmAccount
 * 		  that we are editing. If can not be NULL.
 */
static void gm_pw_delete_account_dialog_run (GtkWidget *prefs_window,
					     GmAccount *account,
					     GtkWidget *parent_window);


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns the currently selected account in the main window.
 * 		  (if any).
 * PRE          : /
 */
static GmAccount *gm_pw_get_selected_account (GtkWidget *prefs_window);


/* DESCRIPTION  : /
 * BEHAVIOR     : Takes a GmCodecsList (which is a GtkTreeView as argument)
 * 		  and builds a GSList of the form : codec_name=0 (or 1 if 
 * 		  the codec is active).
 * PRE          : A valid pointer to a GmCodecsList GtkTreeView.
 */
static GSList *gm_codecs_list_to_gm_conf_list (GtkWidget *codecs_list);


/* DESCRIPTION  : /
 * BEHAVIOR     : Creates a GtkTreeView able to display a GmCodecsList 
 * 		  and returns it. A signal is connected to each of the codecs
 * 		  so that they are enabled/disabled at the endpoint leve, 
 * 		  the GmConf key being updated in that case.
 * PRE          : /
 */
static GtkWidget *gm_codecs_list_new (); 


/* DESCRIPTION  : /
 * BEHAVIOR     : Creates a GtkBox with a scrolled window containing the 
 * 		  given codecs list and 2 buttons to reorder the codecs.
 * 		  A signal is connected to the 2 reordering buttons so that
 * 		  so that codecs are really reordered at the endpoint level,
 * 		  the result is stored in the appropriate GmConf key.
 * PRE          : /
 */
static GtkWidget *gm_codecs_list_box_new (GtkWidget *codecs_list);


/* DESCRIPTION  : /
 * BEHAVIOR     : Adds an update button connected to the given callback to
 * 		  the given GtkBox.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the button, followed by
 * 		  a stock ID, a label, the callback, a tooltip and the 
 * 		  alignment.
 */
static GtkWidget *gm_pw_add_update_button (GtkWidget *,
					   GtkWidget *,
					   const char *,
					   const char *,
					   GtkSignalFunc,
					   gchar *,
					   gfloat);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the general settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_general_page (GtkWidget *,
				     GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the interface settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_interface_page (GtkWidget *,
				       GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the directories settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_directories_page (GtkWidget *,
					 GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the sound events settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_sound_events_page (GtkWidget *,
					  GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the call forwarding settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_call_forwarding_page (GtkWidget *,
					     GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the call options page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_call_options_page (GtkWidget *,
					  GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the H.323 settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_h323_page (GtkWidget *,
				  GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the SIP settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_sip_page (GtkWidget *,
				 GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the nat settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_nat_page (GtkWidget *,
				 GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the video devices settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_video_devices_page (GtkWidget *,
					   GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the audio devices settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_audio_devices_page (GtkWidget *,
					   GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the audio codecs settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_audio_codecs_page (GtkWidget *,
					  GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the video codecs settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_video_codecs_page (GtkWidget *,
					  GtkWidget *);


/* GTK Callbacks */

/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on an account to enable it in the prefs window.
 * BEHAVIOR     :  It updates the accounts list configuration to enable/disable
 *                 the account. It also calls the Register operation from
 *                 the GMEndpoint to refresh the status of that account.
 * PRE          :  /
 */
static void account_toggled_cb (GtkCellRendererToggle *cell,
				gchar *path_str,
				gpointer data);


/* DESCRIPTION  :  This callback is called when the user chooses to add
 * 		   an account.
 * BEHAVIOR     :  It runs the edit account dialog until the user validates
 * 		   it with correct data.
 * PRE          :  /
 */
static void add_account_cb (GtkWidget *button, 
			    gpointer data);


/* DESCRIPTION  :  This callback is called when the user chooses to edit
 * 		   an account.
 * BEHAVIOR     :  It runs the edit account dialog until the user validates
 * 		   it with correct data.
 * PRE          :  /
 */
static void edit_account_cb (GtkWidget *button, 
			     gpointer data);


/* DESCRIPTION  :  This callback is called when the user chooses to delete
 * 		   an account.
 * BEHAVIOR     :  It runs the delete account dialog until the user validates
 * 		   it.
 * PRE          :  /
 */
static void delete_account_cb (GtkWidget *button, 
			       gpointer data);


/* DESCRIPTION  :  This callback is called when the user changes the protocol
 * 		   in the account dialog.
 * BEHAVIOR     :  Updates the content and labels.
 * PRE          :  data is a valid pointer to a valid GmAccountWindow.
 */
static void account_dialog_protocol_changed_cb (GtkWidget *menu,
						gpointer data);


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a codec in the GmCodecsList.
 * BEHAVIOR     :  It updates the codecs list to enable/disable the codec
 * 		   and also the associated GmConf key value.
 * PRE          :  /
 */
static void codec_toggled_cb (GtkCellRendererToggle *,
			      gchar *, 
			      gpointer);


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a button in the GmCodecsList box.
 *                 (Up, Down)
 * BEHAVIOR     :  It updates the codecs list order and the GmConf key value.
 * PRE          :  data = GtkTreeModel, the button "operation" data contains
 * 		   "up" or "down".
 */
static void codec_moved_cb (GtkWidget *,
			    gpointer);


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the refresh devices list button in the prefs.
 * BEHAVIOR     :  Redetects the devices and refreshes the menu.
 * PRE          :  data is a valid pointer to the preferences window GMObject.
 */
static void refresh_devices_list_cb (GtkWidget *,
				     gpointer);


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the Update button of the STUN server Settings.
 * BEHAVIOR     :  Update the stun server on the endpoint. 
 * PRE          :  /
 */
static void stunserver_update_cb (GtkWidget *,
				  gpointer);


/* DESCRIPTION  :  This callback is called when the user changes
 *                 the sound file in the GtkEntry widget.
 * BEHAVIOR     :  It udpates the config key corresponding the currently
 *                 selected sound event and updates it to the new value
 *                 if required.
 * PRE          :  The preferences window GMObject.
 */
static void sound_event_changed_cb (GtkEntry *,
				    gpointer);


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a sound event in the list.
 * BEHAVIOR     :  It udpates the GtkEntry to the config value for the key
 *                 corresponding to the currently selected sound event.
 *                 The sound_event_changed_cb is blocked to prevent it to
 *                 be triggered when the GtkEntry is udpated with the new
 *                 value.
 * PRE          :  /
 */
static void sound_event_clicked_cb (GtkTreeSelection *,
				    gpointer);


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the play button in the sound events list.
 * BEHAVIOR     :  Plays the currently selected sound event using the 
 * 		   selected audio player and plugin through a GMSoundEvent.
 * PRE          :  /
 */
static void sound_event_play_cb (GtkWidget *,
				 gpointer);


#if !GTK_CHECK_VERSION (2, 3, 2)
/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a button of the file selector.
 * BEHAVIOR     :  It sets the selected filename in the good entry (given
 *                 as data of the object because of the bad API). Emits the
 *                 focus-out-event to simulate it.
 * PRE          :  data = the file selector.
 */
static void file_selector_cb (GtkFileSelection *,
			      gpointer);
#endif


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a sound event in the list and change the toggle.
 * BEHAVIOR     :  It udpates the config key associated with the currently
 *                 selected sound event so that it reflects the state of the
 *                 sound event (enabled or disabled) and also updates the list.
 * PRE          :  /
 */
static void sound_event_toggled_cb (GtkCellRendererToggle *,
				    gchar *, 
				    gpointer);


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the browse button (in the video devices or sound events).
 * BEHAVIOR     :  It displays the file selector widget.
 * PRE          :  /
 */
static void browse_cb (GtkWidget *,
		       gpointer);


/* Columns for the codecs page */
enum {

  COLUMN_CODEC_ACTIVE,
  COLUMN_CODEC_NAME,
  COLUMN_CODEC_BANDWIDTH,
  COLUMN_CODEC_SELECTABLE,
  COLUMN_CODEC_COLOR,
  COLUMN_CODEC_NUMBER
};

/* Columns for the VoIP accounts page */
enum {

  COLUMN_ACCOUNT_ENABLED,
  COLUMN_ACCOUNT_AID,
  COLUMN_ACCOUNT_ACCOUNT_NAME,
  COLUMN_ACCOUNT_PROTOCOL_NAME,
  COLUMN_ACCOUNT_HOST,
  COLUMN_ACCOUNT_DOMAIN,
  COLUMN_ACCOUNT_LOGIN,
  COLUMN_ACCOUNT_PASSWORD,
  COLUMN_ACCOUNT_STATE,
  COLUMN_ACCOUNT_TIMEOUT,
  COLUMN_ACCOUNT_METHOD,
  COLUMN_ACCOUNT_ERROR_MESSAGE,
  COLUMN_ACCOUNT_ACTIVATABLE,
  COLUMN_ACCOUNT_NUMBER
};


/* Implementation */
static void
gm_pw_destroy (gpointer pw)
{
  g_return_if_fail (pw != NULL);

  delete ((GmPreferencesWindow *) pw);
}


static GmPreferencesWindow *
gm_pw_get_pw (GtkWidget *preferences_window)
{
  g_return_val_if_fail (preferences_window != NULL, NULL);

  return GM_PREFERENCES_WINDOW (g_object_get_data (G_OBJECT (preferences_window), "GMObject"));
}


static GtkWidget *
gm_accounts_list_new ()
{
  GtkCellRenderer *renderer = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkListStore *list_store = NULL;
  GtkWidget *tree_view = NULL;

  gchar *column_names [] = {

    "",
    "",
    _("Account Name"),
    _("Protocol"),
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    _("Status"),
    ""
  };
  
  list_store = gtk_list_store_new (COLUMN_ACCOUNT_NUMBER,
				   G_TYPE_BOOLEAN, /* Enabled? */
				   G_TYPE_STRING,  /* AID */
				   G_TYPE_STRING,  /* Account Name */
				   G_TYPE_STRING,  /* Protocol Name */
				   G_TYPE_STRING,  /* Host */
				   G_TYPE_STRING,  /* Domain */
				   G_TYPE_STRING,  /* Login */
				   G_TYPE_STRING,  /* Password */
				   G_TYPE_INT,     /* State */
				   G_TYPE_INT,     /* Timeout */
				   G_TYPE_INT,     /* Method */
				   G_TYPE_STRING,  /* Error Message */  
				   G_TYPE_INT);    /* Activatable */

  tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree_view), TRUE);
  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (tree_view), TRUE);
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (tree_view),0);

  /* Set all Colums */
  renderer = gtk_cell_renderer_toggle_new ();
  column = gtk_tree_view_column_new_with_attributes (_("A"),
						     renderer,
						     "active", 
						     COLUMN_ACCOUNT_ENABLED,
						     NULL);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_add_attribute (column, renderer, 
				      "activatable", 
				      COLUMN_ACCOUNT_ACTIVATABLE);
  g_signal_connect (G_OBJECT (renderer), "toggled",
		    G_CALLBACK (account_toggled_cb),
		    (gpointer) tree_view);

  /* Add all text renderers, ie all except the "ACCOUNT_ENABLED" column */
  for (int i = COLUMN_ACCOUNT_AID ; i < COLUMN_ACCOUNT_NUMBER - 1 ; i++) {
    
      renderer = gtk_cell_renderer_text_new ();
      column = gtk_tree_view_column_new_with_attributes (column_names [i],
							 renderer,
							 "text", 
							 i,
							 NULL);
      gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

      if (i == COLUMN_ACCOUNT_AID 
	  || i == COLUMN_ACCOUNT_HOST
	  || i == COLUMN_ACCOUNT_TIMEOUT
	  || i == COLUMN_ACCOUNT_METHOD 
	  || i == COLUMN_ACCOUNT_DOMAIN
	  || i == COLUMN_ACCOUNT_LOGIN
	  || i == COLUMN_ACCOUNT_PASSWORD
	  || i == COLUMN_ACCOUNT_STATE)
	g_object_set (G_OBJECT (column), "visible", false, NULL);
  }
  
  return tree_view;
}


static GtkWidget *
gm_accounts_list_box_new (GtkWidget *accounts_list)
{
  GtkWidget *event_box = NULL;
  GtkWidget *scroll_window = NULL;
  
  GtkWidget *frame = NULL;
  GtkWidget *hbox = NULL;
  
  GtkWidget *button = NULL;
  GtkWidget *buttons_vbox = NULL;
  GtkWidget *alignment = NULL;

  GtkListStore *list_store = NULL;
  GtkTreeView *tree_view = NULL;

  g_return_val_if_fail (accounts_list != NULL, NULL);
  

  /* The scrolled window with the accounts list store */
  tree_view = GTK_TREE_VIEW (accounts_list);
  list_store = GTK_LIST_STORE (gtk_tree_view_get_model (tree_view));

  scroll_window = gtk_scrolled_window_new (FALSE, FALSE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), 
				  GTK_POLICY_NEVER, 
				  GTK_POLICY_AUTOMATIC);

  event_box = gtk_event_box_new ();
  hbox = gtk_hbox_new (FALSE, 4);
  gtk_container_add (GTK_CONTAINER (event_box), hbox);

  frame = gtk_frame_new (NULL);
  gtk_widget_set_size_request (GTK_WIDGET (frame), -1, 150);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 
				  2 * GNOMEMEETING_PAD_SMALL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (frame), scroll_window);
  gtk_container_add (GTK_CONTAINER (scroll_window), accounts_list);
  gtk_container_set_border_width (GTK_CONTAINER (accounts_list), 0);
  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
  
  
  /* The buttons */
  alignment = gtk_alignment_new (1, 0.5, 0, 0);
  buttons_vbox = gtk_vbutton_box_new ();

  gtk_box_set_spacing (GTK_BOX (buttons_vbox), 2 * GNOMEMEETING_PAD_SMALL);

  gtk_container_add (GTK_CONTAINER (alignment), buttons_vbox);
  gtk_box_pack_start (GTK_BOX (hbox), alignment, 
		      TRUE, TRUE, 2 * GNOMEMEETING_PAD_SMALL);

  button = gtk_button_new_from_stock (GTK_STOCK_ADD);
  gtk_box_pack_start (GTK_BOX (buttons_vbox), button, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button), "clicked", 
		    GTK_SIGNAL_FUNC (add_account_cb), NULL); 

  button = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
  gtk_box_pack_start (GTK_BOX (buttons_vbox), button, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button), "clicked", 
		    GTK_SIGNAL_FUNC (delete_account_cb), NULL); 

  button = gtk_button_new_from_stock (GTK_STOCK_PROPERTIES);
  gtk_box_pack_start (GTK_BOX (buttons_vbox), button, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button), "clicked", 
		    GTK_SIGNAL_FUNC (edit_account_cb), NULL); 

  gtk_widget_show_all (event_box);

  return event_box;
}


static void
gm_pw_edit_account_dialog_run (GtkWidget *prefs_window,
			       GmAccount *account,
			       GtkWidget *parent_window)
{
  GmPreferencesWindow *pw = NULL;
  
  GtkWidget *dialog = NULL;

  GtkWidget *table = NULL;
  GtkWidget *label = NULL;

  GtkWidget *menu = NULL;
  GtkWidget *item = NULL;

  PRegularExpression regex ("^[a-z0-9][a-z0-9. ]*$", 
			    PRegularExpression::IgnoreCase);

  PString username;
  PString host;
  PString domain;
  PString password;
  PString account_name;

  gint protocol = 0;

  gboolean valid = FALSE;
  gboolean is_editing = FALSE;
  
  GmAccountsWindow *aw = NULL;
  
  pw = gm_pw_get_pw (prefs_window);

  is_editing = (account != NULL);

  /**/
  aw = new GmAccountsWindow ();
  dialog =
    gtk_dialog_new_with_buttons (_("Edit the Account Information"), 
				 GTK_WINDOW (NULL),
				 GTK_DIALOG_MODAL,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
				 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
				 NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
				   GTK_RESPONSE_ACCEPT);
  gtk_window_set_transient_for (GTK_WINDOW (dialog), 
				GTK_WINDOW (parent_window));

  table = gtk_table_new (6, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_container_set_border_width (GTK_CONTAINER (table), 12);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), table);

  
  /* Account Name */
  label = gtk_label_new (_("Account Name:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  aw->account_entry = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1); 
  gtk_table_attach_defaults (GTK_TABLE (table), aw->account_entry, 1, 2, 0, 1); 
  gtk_entry_set_activates_default (GTK_ENTRY (aw->account_entry), TRUE);
  if (account && account->account_name)
    gtk_entry_set_text (GTK_ENTRY (aw->account_entry), account->account_name);

  /* Protocol */
  if (!is_editing) {

    label = gtk_label_new (_("Protocol:"));
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    menu = gtk_menu_new ();
    aw->protocol_option_menu = gtk_option_menu_new ();
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), aw->protocol_option_menu);
    item = gtk_menu_item_new_with_label ("SIP");
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    item = gtk_menu_item_new_with_label ("H.323");
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    gtk_option_menu_set_menu (GTK_OPTION_MENU (aw->protocol_option_menu), menu);
    gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2); 
    gtk_table_attach_defaults (GTK_TABLE (table), aw->protocol_option_menu, 
			       1, 2, 1, 2); 
    
    g_signal_connect (GTK_OPTION_MENU (aw->protocol_option_menu)->menu,
		      "deactivate", 
		      G_CALLBACK (account_dialog_protocol_changed_cb),
		      (gpointer) aw);
  }
  
  /* Host */
  if (!account || !strcmp (account->protocol_name, "SIP"))
    aw->host_label = gtk_label_new (_("Registrar:"));
  else
    aw->host_label = gtk_label_new (_("Gatekeeper:"));
  gtk_misc_set_alignment (GTK_MISC (aw->host_label), 0.0, 0.5);
  aw->host_entry = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), aw->host_label, 0, 1, 2, 3); 
  gtk_table_attach_defaults (GTK_TABLE (table), aw->host_entry, 1, 2, 2, 3); 
  gtk_entry_set_activates_default (GTK_ENTRY (aw->host_entry), TRUE);
  if (account && account->host)
    gtk_entry_set_text (GTK_ENTRY (aw->host_entry), account->host);

  /* Realm/Domain */
  if (!account || !strcmp (account->protocol_name, "SIP"))
    aw->domain_label = gtk_label_new (_("Realm/Domain:"));
  else
    aw->domain_label = gtk_label_new (_("Gatekeeper ID:"));
  gtk_misc_set_alignment (GTK_MISC (aw->domain_label), 0.0, 0.5);
  aw->domain_entry = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), aw->domain_label, 0, 1, 3, 4); 
  gtk_table_attach_defaults (GTK_TABLE (table), aw->domain_entry, 1, 2, 3, 4); 
  gtk_entry_set_activates_default (GTK_ENTRY (aw->domain_entry), TRUE);
  if (account && account->domain)
    gtk_entry_set_text (GTK_ENTRY (aw->domain_entry), account->domain);
  
  /* User Name */
  label = gtk_label_new (_("User Name:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  aw->username_entry = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 4, 5); 
  gtk_table_attach_defaults (GTK_TABLE (table), aw->username_entry, 
			     1, 2, 4, 5); 
  gtk_entry_set_activates_default (GTK_ENTRY (aw->username_entry), TRUE);
  if (account && account->login)
    gtk_entry_set_text (GTK_ENTRY (aw->username_entry), account->login);
  
  /* Password */
  label = gtk_label_new (_("Password:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  aw->password_entry = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 5, 6); 
  gtk_table_attach_defaults (GTK_TABLE (table), aw->password_entry, 
			     1, 2, 5, 6); 
  gtk_entry_set_activates_default (GTK_ENTRY (aw->password_entry), TRUE);
  if (account && account->password)
    gtk_entry_set_text (GTK_ENTRY (aw->password_entry), account->password);
  

  gtk_widget_show_all (dialog);
  while (!valid) {

    switch (gtk_dialog_run (GTK_DIALOG (dialog))) {

    case GTK_RESPONSE_ACCEPT:

      username = gtk_entry_get_text (GTK_ENTRY (aw->username_entry));
      account_name = gtk_entry_get_text (GTK_ENTRY (aw->account_entry));
      host = gtk_entry_get_text (GTK_ENTRY (aw->host_entry));
      password = gtk_entry_get_text (GTK_ENTRY (aw->password_entry));
      domain = gtk_entry_get_text (GTK_ENTRY (aw->domain_entry));
      if (!is_editing)
	protocol = // take it from the menu
	  gtk_option_menu_get_history (GTK_OPTION_MENU (aw->protocol_option_menu));
      else // take it from the existing account field
	protocol = (account->protocol_name 
		    && !strcmp (account->protocol_name, "SIP") ? 0 : 1);

      /* Check at least an account name, registrar, 
       * and username are provided */
      if (protocol == 0) // SIP
	valid = (username.FindRegEx (regex) != P_MAX_INDEX
		 && account_name.FindRegEx (regex) != P_MAX_INDEX
		 && host.FindRegEx (regex) != P_MAX_INDEX);
      else // H.323
	valid = (account_name.FindRegEx (regex) != P_MAX_INDEX);

      if (valid) {

	if (!is_editing)
	  account = gm_account_new ();

	g_free (account->login);
	g_free (account->account_name);
	g_free (account->host);
	g_free (account->password);
	g_free (account->domain);
	if (!is_editing)
	  g_free (account->protocol_name);
	
	account->account_name = g_strdup (account_name);
	account->host = g_strdup (host);
	account->domain = g_strdup (domain);
	account->login = g_strdup (username);
	account->password = g_strdup (password);
	if (!is_editing)
	  account->protocol_name = g_strdup ((protocol == 0) ? "SIP" : "H.323");

	/* The GUI will be updated through the GmConf notifiers */
	if (is_editing) 
	  gnomemeeting_account_modify (account);
	else 
	  gnomemeeting_account_add (account);
      }
      else {
	if (protocol == 0) // SIP
	  gnomemeeting_error_dialog (GTK_WINDOW (dialog), _("Missing information"), _("Please make sure to provide a valid account name, host name and user name."));
	else // H.323
	  gnomemeeting_error_dialog (GTK_WINDOW (dialog), _("Missing information"), _("Please make sure to provide a valid account name, and host name."));
      }
      break;
      
    case GTK_RESPONSE_REJECT:
      valid = TRUE;
      break;
    }
  }

  delete ((GmAccountsWindow *) aw);
  gtk_widget_destroy (dialog);
}


static void 
gm_pw_delete_account_dialog_run (GtkWidget *prefs_window,
				 GmAccount *account,
				 GtkWidget *parent_window)
{
  GtkWidget *dialog = NULL;

  gchar *confirm_msg = NULL;

  g_return_if_fail (prefs_window != NULL);
  g_return_if_fail (account != NULL);

  /* Create the dialog to delete the account */
  confirm_msg = 
    g_strdup_printf (_("Are you sure you want to delete account %s?"), 
		     account->account_name);
  dialog =
    gtk_message_dialog_new (GTK_WINDOW (prefs_window),
			    GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
			    GTK_BUTTONS_YES_NO, confirm_msg);
  g_free (confirm_msg);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
				   GTK_RESPONSE_YES);

  gtk_widget_show_all (dialog);


  /* Now run the dialg */
  switch (gtk_dialog_run (GTK_DIALOG (dialog))) {

  case GTK_RESPONSE_YES:

    /* The GUI will be updated throught the GmConf notifiers */
    gnomemeeting_account_delete (account);
    break;
  }

  gtk_widget_destroy (dialog);
}


GmAccount *
gm_pw_get_selected_account (GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;
  
  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;

  GmAccount *account = NULL;
  
  g_return_val_if_fail (prefs_window != NULL, NULL);
  
  /* Get the required data from the GtkNotebook page */
  pw = gm_pw_get_pw (prefs_window);

  g_return_val_if_fail (pw != NULL, NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pw->accounts_list));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (pw->accounts_list));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    account = gm_account_new ();

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			COLUMN_ACCOUNT_ENABLED, &account->enabled,
			COLUMN_ACCOUNT_AID, &account->aid,
			COLUMN_ACCOUNT_ACCOUNT_NAME, &account->account_name,
			COLUMN_ACCOUNT_PROTOCOL_NAME, &account->protocol_name,
			COLUMN_ACCOUNT_HOST, &account->host,
			COLUMN_ACCOUNT_DOMAIN, &account->domain,
			COLUMN_ACCOUNT_LOGIN, &account->login,
			COLUMN_ACCOUNT_PASSWORD, &account->password,
			COLUMN_ACCOUNT_TIMEOUT, &account->timeout,
			COLUMN_ACCOUNT_METHOD, &account->method,
			-1); 
  }

  return account;
}


static GSList *
gm_codecs_list_to_gm_conf_list (GtkWidget *codecs_list)
{
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gboolean fixed = FALSE;
  gchar *codec_data = NULL;
  gchar *codec = NULL;

  GSList *codecs_data = NULL;

  g_return_val_if_fail (codecs_list != NULL, NULL);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (codecs_list));

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)) {

    do {

      gtk_tree_model_get (model, &iter, 
			  COLUMN_CODEC_ACTIVE, &fixed, -1);
      gtk_tree_model_get (model, &iter, 
			  COLUMN_CODEC_NAME, &codec, -1);
      codec_data = 
	g_strdup_printf ("%s=%d", codec, fixed); 

      codecs_data = g_slist_append (codecs_data, codec_data);

      g_free (codec);

    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
  }

  return codecs_data;
}


static GtkWidget *
gm_codecs_list_new ()
{
  GtkCellRenderer *renderer = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkListStore *list_store = NULL;
  GtkWidget *tree_view = NULL;


  list_store = gtk_list_store_new (COLUMN_CODEC_NUMBER,
				   G_TYPE_BOOLEAN,
				   G_TYPE_STRING,
				   G_TYPE_STRING,
				   G_TYPE_BOOLEAN,
				   G_TYPE_STRING);

  tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree_view), TRUE);
  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (tree_view), TRUE);
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (tree_view),0);

  /* Set all Colums */
  renderer = gtk_cell_renderer_toggle_new ();
  column = gtk_tree_view_column_new_with_attributes (_("A"),
						     renderer,
						     "active", 
						     COLUMN_CODEC_ACTIVE,
						     NULL);
  gtk_tree_view_column_add_attribute (column, renderer, 
				      "activatable", COLUMN_CODEC_SELECTABLE);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  g_signal_connect (G_OBJECT (renderer), "toggled",
		    G_CALLBACK (codec_toggled_cb),
		    (gpointer) tree_view);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Name"),
						     renderer,
						     "text", 
						     COLUMN_CODEC_NAME,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
				      COLUMN_CODEC_COLOR);
  g_object_set (G_OBJECT (renderer), "weight", "bold", NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Bandwidth"),
						     renderer,
						     "text", 
						     COLUMN_CODEC_BANDWIDTH,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
				      COLUMN_CODEC_COLOR);

  return tree_view;
}


static GtkWidget *
gm_codecs_list_box_new (GtkWidget *codecs_list)
{
  GtkWidget *scroll_window = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *button = NULL;

  GtkWidget *buttons_vbox = NULL;
  GtkWidget *alignment = NULL;

  GtkListStore *list_store = NULL;

  list_store = 
    GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (codecs_list)));

  scroll_window = gtk_scrolled_window_new (FALSE, FALSE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), 
				  GTK_POLICY_NEVER, 
				  GTK_POLICY_ALWAYS);

  hbox = gtk_hbox_new (FALSE, 4);


  frame = gtk_frame_new (NULL);
  gtk_widget_set_size_request (GTK_WIDGET (frame), -1, 200);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 
				  2 * GNOMEMEETING_PAD_SMALL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (frame), scroll_window);
  gtk_container_add (GTK_CONTAINER (scroll_window), codecs_list);
  gtk_container_set_border_width (GTK_CONTAINER (codecs_list), 0);
  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);


  /* The buttons */
  alignment = gtk_alignment_new (1, 0.5, 0, 0);
  buttons_vbox = gtk_vbutton_box_new ();

  gtk_box_set_spacing (GTK_BOX (buttons_vbox), 2 * GNOMEMEETING_PAD_SMALL);

  gtk_container_add (GTK_CONTAINER (alignment), buttons_vbox);
  gtk_box_pack_start (GTK_BOX (hbox), alignment, 
		      TRUE, TRUE, 2 * GNOMEMEETING_PAD_SMALL);

  button = gtk_button_new_from_stock (GTK_STOCK_GO_UP);
  gtk_box_pack_start (GTK_BOX (buttons_vbox), button, TRUE, TRUE, 0);
  g_object_set_data (G_OBJECT (button), "operation", (gpointer) "up");
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (codec_moved_cb), 
		    (gpointer) codecs_list);

  button = gtk_button_new_from_stock (GTK_STOCK_GO_DOWN);
  gtk_box_pack_start (GTK_BOX (buttons_vbox), button, TRUE, TRUE, 0);
  g_object_set_data (G_OBJECT (button), "operation", (gpointer) "down");
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (codec_moved_cb), 
		    (gpointer) codecs_list);


  gtk_widget_show_all (hbox);


  return hbox;
}


static GtkWidget *
gm_pw_add_update_button (GtkWidget *prefs_window,
			 GtkWidget *box,
			 const char *stock_id,
			 const char *label,
			 GtkSignalFunc func,
			 gchar *tooltip,
			 gfloat valign)  
{
  GtkWidget *alignment = NULL;
  GtkWidget *image = NULL;
  GtkWidget *button = NULL;                                                    


  /* Update Button */
  image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_BUTTON);
  button = gnomemeeting_button_new (label, image);

  alignment = gtk_alignment_new (1, valign, 0, 0);
  gtk_container_add (GTK_CONTAINER (alignment), button);
  gtk_container_set_border_width (GTK_CONTAINER (button), 6);

  gtk_box_pack_start (GTK_BOX (box), alignment, TRUE, TRUE, 0);

  g_signal_connect (G_OBJECT (button), "clicked",                          
		    G_CALLBACK (func), 
		    (gpointer) prefs_window);


  return button;                                                               
}                                                                              


static void
gm_pw_init_general_page (GtkWidget *prefs_window,
			 GtkWidget *container)
{
  GmPreferencesWindow *pw = NULL;
  
  GtkWidget *subsection = NULL;
  GtkWidget *entry = NULL;

  pw = gm_pw_get_pw (prefs_window);
  
  /* Personal Information */
  subsection = 
    gnome_prefs_subsection_new (prefs_window, container,
				_("Personal Information"), 2, 2);
  
  entry =
    gnome_prefs_entry_new (subsection, _("_First name:"),
			   PERSONAL_DATA_KEY "firstname",
			   _("Enter your first name"), 0, false);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 65);

  entry =
    gnome_prefs_entry_new (subsection, _("Sur_name:"),
			   PERSONAL_DATA_KEY "lastname",
			   _("Enter your surname"), 1, false);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 65);

  
  /* VoIP accounts */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
					   _("VoIP Accounts"), 5, 3);
  
  pw->accounts_list = gm_accounts_list_new ();
  pw->accounts_box = gm_accounts_list_box_new (pw->accounts_list);
  gtk_table_attach (GTK_TABLE (subsection),  
		    pw->accounts_box,
		    0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_SHRINK), 
		    (GtkAttachOptions) (GTK_SHRINK),
		    0, 0);
  gtk_widget_realize (pw->accounts_box);

  
  /* Update the GUI */
  gm_prefs_window_update_accounts_list (prefs_window);
}                                                                              


static void
gm_pw_init_interface_page (GtkWidget *prefs_window,
			   GtkWidget *container)
{
  GtkWidget *subsection = NULL;


  /* GnomeMeeting GUI */
  subsection =
    gnome_prefs_subsection_new (prefs_window, container,
				_("GnomeMeeting GUI"), 2, 2);

  gnome_prefs_toggle_new (subsection, _("_Show splash screen"), USER_INTERFACE_KEY "show_splash_screen", _("If enabled, the splash screen will be displayed when GnomeMeeting starts"), 0);

  gnome_prefs_toggle_new (subsection, _("Start _hidden"), USER_INTERFACE_KEY "start_hidden", _("If enabled, GnomeMeeting will start hidden provided that the notification area is present in the GNOME panel"), 1);


  /* Packing widget */
  subsection =
    gnome_prefs_subsection_new (prefs_window, container, 
				_("Video Display"), 1, 1);

  gnome_prefs_toggle_new (subsection, _("Place windows displaying video _above other windows"), VIDEO_DISPLAY_KEY "stay_on_top", _("Place windows displaying video above other windows during calls"), 0);


  /* Text Chat */
  subsection =
    gnome_prefs_subsection_new (prefs_window, container, _("Text Chat"), 1, 1);

  gnome_prefs_toggle_new (subsection, _("Automatically clear the text chat at the end of calls"), USER_INTERFACE_KEY "auto_clear_text_chat", _("If enabled, the text chat will automatically be cleared at the end of calls"), 0);
}


static void
gm_pw_init_directories_page (GtkWidget *prefs_window,
			     GtkWidget *container)
{
  GtkWidget *subsection = NULL;


  /* Packing widgets for the XDAP directory */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
					   _("Users Directory"), 3, 2);


  /* Add all the fields */                                                     
  gnome_prefs_entry_new (subsection, _("Users directory:"), LDAP_KEY "server", _("The users directory server to register with"), 0, true);

  gnome_prefs_toggle_new (subsection, _("Enable _registering"), LDAP_KEY "enable_registering", _("If enabled, register with the selected users directory"), 1);

  gnome_prefs_toggle_new (subsection, _("_Publish my details in the users directory when registering"), LDAP_KEY "show_details", _("If enabled, your details are shown to people browsing the users directory. If disabled, you are not visible to users browsing the users directory, but they can still use the callto URL to call you."), 2);
}


static void
gm_pw_init_call_forwarding_page (GtkWidget *prefs_window,
				 GtkWidget *container)
{
  GtkWidget *subsection = NULL;

  subsection = gnome_prefs_subsection_new (prefs_window, container,
					   _("Call Forwarding"), 3, 2);

  gnome_prefs_toggle_new (subsection, _("_Always forward calls to the given host"), CALL_FORWARDING_KEY "always_forward", _("If enabled, all incoming calls will be forwarded to the host that is specified in the field above"), 0);

  gnome_prefs_toggle_new (subsection, _("Forward calls to the given host if _no answer"), CALL_FORWARDING_KEY "forward_on_no_answer", _("If enabled, all incoming calls will be forwarded to the host that is specified in the field above if you do not answer the call"), 1);

  gnome_prefs_toggle_new (subsection, _("Forward calls to the given host if _busy"), CALL_FORWARDING_KEY "forward_on_busy", _("If enabled, all incoming calls will be forwarded to the host that is specified in the field above if you already are in a call or if you are in Do Not Disturb mode"), 2);
}


static void
gm_pw_init_call_options_page (GtkWidget *prefs_window,
			      GtkWidget *container)
{
  GtkWidget *subsection = NULL;


  subsection = gnome_prefs_subsection_new (prefs_window, container,
					   _("Call Options"), 2, 3);


  /* Add all the fields */
  gnome_prefs_toggle_new (subsection, _("Automatically _clear calls after 30 seconds of inactivity"), CALL_OPTIONS_KEY "clear_inactive_calls", _("If enabled, calls for which no audio and video has been received in the last 30 seconds are automatically cleared"), 0);  

  /* Translators: the full sentence is Reject or forward
     unanswered incoming calls after X s (seconds) */
  gnome_prefs_spin_new (subsection, _("Reject or forward unanswered incoming calls after "), CALL_OPTIONS_KEY "no_answer_timeout", _("Automatically reject or forward incoming calls if no answer is given after the specified amount of time (in seconds)"), 10.0, 299.0, 1.0, 1, _("seconds"), true);
}


static void
gm_pw_init_sound_events_page (GtkWidget *prefs_window,
			      GtkWidget *container)
{
  GmPreferencesWindow *pw= NULL;

  GtkWidget *label = NULL;
  GtkWidget *entry = NULL;
  GtkWidget *button = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *subsection = NULL;

  GtkListStore *list_store = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeViewColumn *column = NULL;

  GtkCellRenderer *renderer = NULL;

  PStringArray devs;

  gchar **array = NULL;


  pw = gm_pw_get_pw (prefs_window);

  subsection = gnome_prefs_subsection_new (prefs_window, container,
					   _("GnomeMeeting Sound Events"), 
					   1, 1);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_table_attach (GTK_TABLE (subsection), vbox, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_SHRINK), 
		    (GtkAttachOptions) (GTK_SHRINK),
		    0, 0);

  /* The 3rd column will be invisible and contain the config key containing
     the file to play. The 4th one contains the key determining if the
     sound event is enabled or not. */
  list_store =
    gtk_list_store_new (4,
			G_TYPE_BOOLEAN,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_STRING);

  pw->sound_events_list =
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (pw->sound_events_list), TRUE);

  selection =
    gtk_tree_view_get_selection (GTK_TREE_VIEW (pw->sound_events_list));

  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 
				  2 * GNOMEMEETING_PAD_SMALL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (frame), pw->sound_events_list);
  gtk_container_set_border_width (GTK_CONTAINER (pw->sound_events_list), 0);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);


  /* Set all Colums */
  renderer = gtk_cell_renderer_toggle_new ();
  column = gtk_tree_view_column_new_with_attributes (_("A"),
						     renderer,
						     "active", 
						     0,
						     NULL);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
  gtk_tree_view_append_column (GTK_TREE_VIEW (pw->sound_events_list), column);
  g_signal_connect (G_OBJECT (renderer), "toggled",
		    G_CALLBACK (sound_event_toggled_cb), 
		    GTK_TREE_MODEL (list_store));

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Event"),
						     renderer,
						     "text", 
						     1,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (pw->sound_events_list), column);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 325);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Event"),
						     renderer,
						     "text", 
						     2,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (pw->sound_events_list), column);
  gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN (column), FALSE);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Event"),
						     renderer,
						     "text", 
						     3,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (pw->sound_events_list), column);
  gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN (column), FALSE);

  hbox = gtk_hbox_new (0, FALSE);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);

  label = gtk_label_new (_("Sound to play:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);

  entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 2);

  button = gtk_button_new_from_stock (GTK_STOCK_OPEN);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);

  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (browse_cb),
		    (gpointer) entry);

  button = gtk_button_new_with_label (_("Play"));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);

  g_signal_connect (G_OBJECT (selection), "changed",
		    G_CALLBACK (sound_event_clicked_cb),
		    (gpointer) entry);

  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (sound_event_play_cb),
		    (gpointer) entry);

  g_signal_connect (G_OBJECT (entry), "changed",
		    G_CALLBACK (sound_event_changed_cb),
		    (gpointer) prefs_window);


  /* Place it after the signals so that we can make sure they are run if
     required */
  gm_prefs_window_sound_events_list_build (prefs_window);


  /* The audio output */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
					   _("Ring Output Device"), 
					   1, 1);

  devs = GnomeMeeting::Process ()->GetAudioInputDevices ();
  array = devs.ToCharArray ();
  pw->sound_events_output =
    gnome_prefs_string_option_menu_new (subsection, _("Ring Output device:"), array, SOUND_EVENTS_KEY "output_device", _("Select the audio output device to use for the ring sound event"), 0);
  free (array);
}


static void
gm_pw_init_h323_page (GtkWidget *prefs_window,
		      GtkWidget *container)
{
  GtkWidget *entry = NULL;
  GtkWidget *subsection = NULL;

  gchar *capabilities [] = 
    {_("String"),
    _("Tone"),
    _("RFC2833"),
    _("Q.931"),
    NULL};

  
  /* Add Misc Settings */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
					   _("Misc Settings"), 2, 2);

  gnome_prefs_entry_new (subsection, _("Default _gateway:"), H323_KEY "default_gateway", _("The Gateway host is the host to use to do H.323 calls through a gateway that will relay calls"), 0, false);
  
  entry =
    gnome_prefs_entry_new (subsection, _("Forward _URL:"), H323_KEY "forward_host", _("The host where calls should be forwarded if call forwarding is enabled"), 1, false);
  if (!strcmp (gtk_entry_get_text (GTK_ENTRY (entry)), ""))
    gtk_entry_set_text (GTK_ENTRY (entry), GMURL ().GetDefaultURL ());



  /* Packing widget */
  subsection =
    gnome_prefs_subsection_new (prefs_window, container,
				_("Advanced Settings"), 3, 1);

  /* The toggles */
  gnome_prefs_toggle_new (subsection, _("Enable H.245 _tunneling"), H323_KEY "enable_h245_tunneling", _("This enables H.245 Tunneling mode. In H.245 Tunneling mode H.245 messages are encapsulated into the the H.225 channel (port 1720). This saves one TCP connection during calls. H.245 Tunneling was introduced in H.323v2 and Netmeeting does not support it. Using both Fast Start and H.245 Tunneling can crash some versions of Netmeeting."), 0);

  gnome_prefs_toggle_new (subsection, _("Enable _early H.245"), H323_KEY "enable_early_h245", _("This enables H.245 early in the setup"), 1);

  gnome_prefs_toggle_new (subsection, _("Enable fast _start procedure"), H323_KEY "enable_fast_start", _("Connection will be established in Fast Start mode. Fast Start is a new way to start calls faster that was introduced in H.323v2. It is not supported by Netmeeting and using both Fast Start and H.245 Tunneling can crash some versions of Netmeeting."), 2);


  /* Packing widget */                                                         
  subsection =
    gnome_prefs_subsection_new (prefs_window, container,
				_("DTMF Mode"), 1, 1);

  gnome_prefs_int_option_menu_new (subsection, _("_Send DTMF as:"), capabilities, H323_KEY "dtmf_mode", _("This permits to set the mode for DTMFs sending. The values can be \"String\" (0), \"Tone\" (1), \"RFC2833\" (2), \"Q.931\" (3) (default is \"String\"). Choosing other values than \"String\" disables the Text Chat."), 0);
}


static void
gm_pw_init_sip_page (GtkWidget *prefs_window,
		      GtkWidget *container)
{
  GmPreferencesWindow *pw = NULL;

  GtkWidget *entry = NULL;
  GtkWidget *subsection = NULL;
  
  gchar *capabilities [] = 
    {_("RFC2833"),
    NULL};

  pw = gm_pw_get_pw (prefs_window);

  
  /* Outbound Proxy */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
					   _("SIP Outbound Proxy"), 3, 3);
  
  gnome_prefs_entry_new (subsection, _("_Outbound Proxy:"), SIP_KEY "outbound_proxy_host", _("The SIP Outbound Proxy to use for outgoing calls"), 0, false);

  gnome_prefs_entry_new (subsection, _("Lo_gin:"), SIP_KEY "outbound_proxy_login", _("The SIP Outbound Proxy login to use for outgoing calls (if any)"), 1, false);
  
  entry =
    gnome_prefs_entry_new (subsection, _("Pa_ssword:"), SIP_KEY "outbound_proxy_password", _("The SIP Outbound Proxy password to use for outgoing calls (if any)"), 2, false);
  gtk_entry_set_visibility (GTK_ENTRY (entry), FALSE);

  
  /* Add Misc Settings */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
					   _("Misc Settings"), 2, 2);

  gnome_prefs_entry_new (subsection, _("Default _proxy:"), SIP_KEY "default_proxy", _("The default proxy is the SIP proxy to use by default to make outgoing calls"), 0, false);
  
  entry =
    gnome_prefs_entry_new (subsection, _("Forward _URL:"), SIP_KEY "forward_host", _("The host where calls should be forwarded if call forwarding is enabled"), 1, false);
  if (!strcmp (gtk_entry_get_text (GTK_ENTRY (entry)), ""))
    gtk_entry_set_text (GTK_ENTRY (entry), GMURL ().GetDefaultURL ());


  /* Packing widget */                                                         
  subsection =
    gnome_prefs_subsection_new (prefs_window, container,
				_("DTMF Mode"), 1, 1);

  gnome_prefs_int_option_menu_new (subsection, _("_Send DTMF as:"), capabilities, SIP_KEY "dtmf_mode", _("This permits to set the mode for DTMFs sending. The value can be \"RFC2833\" (0) only."), 0);


  /* Packing widget */
  //FIXME subsection =
    //gnome_prefs_subsection_new (prefs_window, container,
//				_("Advanced Settings"), 1, 1);

  /* The toggles */
  //gnome_prefs_toggle_new (subsection, _("Use long MIME _headers"), H323_KEY "enable_h245_tunneling", _("This enables H.245 Tunneling mode. In H.245 Tunneling mode H.245 messages are encapsulated into the the H.225 channel (port 1720). This saves one TCP connection during calls. H.245 Tunneling was introduced in H.323v2 and Netmeeting does not support it. Using both Fast Start and H.245 Tunneling can crash some versions of Netmeeting."), 0);
}


static void
gm_pw_init_nat_page (GtkWidget *prefs_window,
		     GtkWidget *container)
{
  GtkWidget *subsection = NULL;


  /* IP translation */
  subsection =
    gnome_prefs_subsection_new (prefs_window, container,
				_("IP Translation"), 3, 1);

  gnome_prefs_toggle_new (subsection, _("Enable IP _translation"), NAT_KEY "enable_ip_translation", _("This enables IP translation. IP translation is useful if GnomeMeeting is running behind a NAT/PAT router. You have to put the public IP of the router in the field below. If you are registered to ils.seconix.com, GnomeMeeting will automatically fetch the public IP using the ILS service. If your router natively supports H.323, you can disable this."), 1);

  gnome_prefs_toggle_new (subsection, _("Enable _automatic IP checking"), NAT_KEY "enable_ip_checking", _("This enables IP checking from seconix.com and fills the IP in the public IP of the NAT/PAT gateway field of GnomeMeeting. The returned IP is only used when IP Translation is enabled. If you disable IP checking, you will have to manually enter the IP of your gateway in the GnomeMeeting preferences."), 2);

  gnome_prefs_entry_new (subsection, _("Public _IP of the NAT/PAT router:"), NAT_KEY "public_ip", _("Enter the public IP of your NAT/PAT router if you want to use IP translation. If you are registered to ils.seconix.com, GnomeMeeting will automatically fetch the public IP using the ILS service."), 3, false);


  /* STUN Support */
  subsection =
    gnome_prefs_subsection_new (prefs_window, container,
				_("STUN Support"), 2, 1);

  gnome_prefs_toggle_new (subsection, _("Enable _STUN Support"), NAT_KEY "enable_stun_support", _("This enables STUN Support. STUN is a technic that permits to go through some types of NAT gateways."), 1);

  gnome_prefs_entry_new (subsection, _("STUN Se_rver:"), NAT_KEY "stun_server", _("The STUN server to use for STUN Support. STUN is a technic that permits to go through some types of NAT gateways."), 2, false);

  gm_pw_add_update_button (prefs_window, container, GTK_STOCK_APPLY, _("_Apply"), GTK_SIGNAL_FUNC (stunserver_update_cb), _("Click here to update your STUN Server settings"), 0);
}


static void
gm_pw_init_audio_devices_page (GtkWidget *prefs_window,
			       GtkWidget *container)
{
  GmPreferencesWindow *pw = NULL;

  GtkWidget *subsection = NULL;

  PStringArray devs;

  gchar **array = NULL;

#ifdef HAS_IXJ
  GtkWidget *entry = NULL;

  gchar *aec [] = {_("Off"), _("Low"), _("Medium"), _("High"), _("AGC"), NULL};
  gchar *types_array [] = {_("POTS"), _("Headset"), NULL};
#endif

  pw = gm_pw_get_pw (prefs_window);


  subsection = gnome_prefs_subsection_new (prefs_window, container,
					   _("Audio Plugin"), 1, 2);

  /* Add all the fields for the audio manager */
  devs = GnomeMeeting::Process ()->GetAudioPlugins ();
  array = devs.ToCharArray ();
  gnome_prefs_string_option_menu_new (subsection, _("Audio plugin:"), array, AUDIO_DEVICES_KEY "plugin", _("The audio plugin that will be used to detect the devices and manage them."), 0);
  free (array);


  /* Add all the fields */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
					   _("Audio Devices"), 4, 2);


  /* The player */
  devs = GnomeMeeting::Process ()->GetAudioOutpoutDevices ();
  array = devs.ToCharArray ();
  pw->audio_player =
    gnome_prefs_string_option_menu_new (subsection, _("Output device:"), array, AUDIO_DEVICES_KEY "output_device", _("Select the audio output device to use"), 0);
  free (array);

  /* The recorder */
  devs = GnomeMeeting::Process ()->GetAudioInputDevices ();
  array = devs.ToCharArray ();
  pw->audio_recorder =
    gnome_prefs_string_option_menu_new (subsection, _("Input device:"), array, AUDIO_DEVICES_KEY "input_device", _("Select the audio input device to use"), 2);
  free (array);

#ifdef HAS_IXJ
  /* The Quicknet devices related options */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
					   _("Quicknet Hardware"), 3, 2);

  gnome_prefs_int_option_menu_new (subsection, _("Echo _cancellation:"), aec, AUDIO_DEVICES_KEY "lid_echo_cancellation_level", _("The Automatic Echo Cancellation level: Off, Low, Medium, High, Automatic Gain Compensation. Choosing Automatic Gain Compensation modulates the volume for best quality."), 0);

  gnome_prefs_int_option_menu_new (subsection, _("Output device type:"), types_array, AUDIO_DEVICES_KEY "lid_output_device_type", _("The output device type is the type of device connected to your Quicknet card. It can be either a POTS (Plain Old Telephone System) or a headset."), 1);

  entry =
    gnome_prefs_entry_new (subsection, _("Country _code:"), AUDIO_DEVICES_KEY "lid_country_code", _("The two-letter country code of your country (e.g.: BE, UK, FR, DE, ...)."), 2, false);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 2);
  gtk_widget_set_size_request (GTK_WIDGET (entry), 100, -1);
#endif


  /* That button will refresh the devices list */
  gm_pw_add_update_button (prefs_window, container, GTK_STOCK_REFRESH, _("_Detect devices"), GTK_SIGNAL_FUNC (refresh_devices_list_cb), _("Click here to refresh the devices list"), 1);
}


static void
gm_pw_init_video_devices_page (GtkWidget *prefs_window,
			       GtkWidget *container)
{
  GmPreferencesWindow *pw = NULL;

  GtkWidget *entry = NULL;
  GtkWidget *subsection = NULL;

  GtkWidget *button = NULL;

  PStringArray devs;

  gchar **array = NULL;
  gchar *video_size [] = {_("Small"),
    _("Large"), 
    NULL};
  gchar *video_format [] = {_("PAL (Europe)"), 
    _("NTSC (America)"), 
    _("SECAM (France)"), 
    _("Auto"), 
    NULL};


  pw = gm_pw_get_pw (prefs_window); 


  /* The video manager */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
					   _("Video Plugin"), 1, 2);

  devs = GnomeMeeting::Process ()->GetVideoPlugins ();
  array = devs.ToCharArray ();
  gnome_prefs_string_option_menu_new (subsection, _("Video plugin:"), array, VIDEO_DEVICES_KEY "plugin", _("The video plugin that will be used to detect the devices and manage them"), 0);
  free (array);


  /* The video devices related options */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
					   _("Video Devices"), 5, 3);

  /* The video device */
  devs = GnomeMeeting::Process ()->GetVideoInputDevices ();
  array = devs.ToCharArray ();
  pw->video_device =
    gnome_prefs_string_option_menu_new (subsection, _("Input device:"), array, VIDEO_DEVICES_KEY "input_device", _("Select the video input device to use. If an error occurs when using this device a test picture will be transmitted."), 0);
  free (array);

  /* Video Channel */
  gnome_prefs_spin_new (subsection, _("Channel:"), VIDEO_DEVICES_KEY "channel", _("The video channel number to use (to select camera, tv or other sources)"), 0.0, 10.0, 1.0, 3, NULL, false);

  gnome_prefs_int_option_menu_new (subsection, _("Size:"), video_size, VIDEO_DEVICES_KEY "size", _("Select the transmitted video size: Small (QCIF 176x144) or Large (CIF 352x288)"), 1);

  gnome_prefs_int_option_menu_new (subsection, _("Format:"), video_format, VIDEO_DEVICES_KEY "format", _("Select the format for video cameras (does not apply to most USB cameras)"), 2);

  entry =
    gnome_prefs_entry_new (subsection, _("Image:"), VIDEO_DEVICES_KEY "image", _("The image to transmit if \"Picture\" is selected as video plugin or if the opening of the device fails. Leave blank to use the default GnomeMeeting logo."), 4, false);

  /* The file selector button */
  button = gtk_button_new_from_stock (GTK_STOCK_OPEN);
  gtk_table_attach (GTK_TABLE (subsection), button, 2, 3, 4, 5,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);

  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (browse_cb),
		    (gpointer) entry);

  /* That button will refresh the devices list */
  gm_pw_add_update_button (prefs_window, container, GTK_STOCK_REFRESH, _("_Detect devices"), GTK_SIGNAL_FUNC (refresh_devices_list_cb), _("Click here to refresh the devices list."), 1);
}


static void
gm_pw_init_audio_codecs_page (GtkWidget *prefs_window,
			      GtkWidget *container)
{
  GMEndPoint *ep = NULL;

  GtkWidget *subsection = NULL;
  GtkWidget *box = NULL;  

  GmPreferencesWindow *pw = NULL;


  pw = gm_pw_get_pw (prefs_window);
  ep = GnomeMeeting::Process ()->Endpoint ();


  /* Packing widgets */
  subsection =
    gnome_prefs_subsection_new (prefs_window, container,
				_("Available Audio Codecs"), 1, 1);

  pw->audio_codecs_list = gm_codecs_list_new ();
  box = gm_codecs_list_box_new (pw->audio_codecs_list);

  gtk_table_attach (GTK_TABLE (subsection), box,
		    0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_SHRINK), 
		    (GtkAttachOptions) (GTK_SHRINK),
		    0, 0);


  /* Here we add the audio codecs options */
  subsection = 
    gnome_prefs_subsection_new (prefs_window, container,
				_("Audio Codecs Settings"), 2, 1);

  /* Translators: the full sentence is Automatically adjust jitter buffer
     between X and Y ms */
  gnome_prefs_range_new (subsection, _("Automatically adjust _jitter buffer between"), NULL, _("and"), NULL, _("ms"), AUDIO_CODECS_KEY "minimum_jitter_buffer", AUDIO_CODECS_KEY "maximum_jitter_buffer", _("The minimum jitter buffer size for audio reception (in ms)."), _("The maximum jitter buffer size for audio reception (in ms)."), 20.0, 20.0, 1000.0, 1000.0, 1.0, 0);

  gnome_prefs_toggle_new (subsection, _("Enable silence _detection"), AUDIO_CODECS_KEY "enable_silence_detection", _("If enabled, use silence detection with the GSM and G.711 codecs."), 1);
}


static void
gm_pw_init_video_codecs_page (GtkWidget *prefs_window,
			      GtkWidget *container)
{
  GtkWidget *subsection = NULL;


  subsection = gnome_prefs_subsection_new (prefs_window, container,
					   _("General Settings"), 2, 1);


  /* Add fields */
  gnome_prefs_toggle_new (subsection, _("Enable video _transmission"), VIDEO_CODECS_KEY "enable_video_transmission", _("If enabled, video is transmitted during a call."), 0);

  gnome_prefs_toggle_new (subsection, _("Enable video _reception"), VIDEO_CODECS_KEY "enable_video_reception", _("If enabled, allows video to be received during a call."), 1);


  /* H.261 Settings */
  subsection = gnome_prefs_subsection_new (prefs_window, container,
					   _("Bandwidth Control"), 1, 1);

  /* Translators: the full sentence is Maximum video bandwidth of X kB/s */
  gnome_prefs_spin_new (subsection, _("Maximum video _bandwidth of"), VIDEO_CODECS_KEY "maximum_video_bandwidth", _("The maximum video bandwidth in kbytes/s. The video quality and the number of transmitted frames per second will be dynamically adjusted above their minimum during calls to try to minimize the bandwidth to the given value."), 2.0, 100.0, 1.0, 0, _("kB/s"), true);


  /* Advanced quality settings */
  subsection =
    gnome_prefs_subsection_new (prefs_window, container,
				_("Advanced Quality Settings"), 3, 1);

  /* Translators: the full sentence is Keep a minimum video quality of X % */
  gnome_prefs_scale_new (subsection, _("Frame Rate"), _("Picture Quality"), VIDEO_CODECS_KEY "transmitted_video_quality", _("Choose if you want to favour speed or quality for the transmitted video."), 1.0, 100.0, 1.0, 0);

  /* Translators: the full sentence is Transmit X background blocks with each
     frame */
  gnome_prefs_spin_new (subsection, _("Transmit"), VIDEO_CODECS_KEY "transmitted_background_blocks", _("Choose the number of blocks (that have not changed) transmitted with each frame. These blocks fill in the background."), 1.0, 99.0, 1.0, 2, _("background _blocks with each frame"), true);
}


/* GTK Callbacks */
static void
account_toggled_cb (GtkCellRendererToggle *cell,
		    gchar *path_str,
		    gpointer data)
{
  GMEndPoint *ep = NULL;
  
  GmPreferencesWindow *pw = NULL;
  GmAccount *account = NULL;

  GtkWidget *prefs_window = NULL;

  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  pw = gm_pw_get_pw (prefs_window);

  /* Update the config */
  account = gm_pw_get_selected_account (prefs_window);
  gnomemeeting_account_toggle_active (account);

  gdk_threads_leave ();
  ep->Register (account);
  gdk_threads_enter ();

  gm_account_delete (account);
}


static void
add_account_cb (GtkWidget *button, 
		gpointer data)
{
  GtkWidget *prefs_window = NULL;

  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow (); 

  gm_pw_edit_account_dialog_run (GTK_WIDGET (prefs_window), 
				 NULL, 
				 GTK_WIDGET (prefs_window));
}


static void
edit_account_cb (GtkWidget *button, 
		gpointer data)
{
  GmAccount *account = NULL;
  GtkWidget *prefs_window = NULL;

  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow (); 

  account = gm_pw_get_selected_account (prefs_window);
  gm_pw_edit_account_dialog_run (GTK_WIDGET (prefs_window), 
				 account, 
				 GTK_WIDGET (prefs_window));
  gm_account_delete (account);
}


static void
delete_account_cb (GtkWidget *button, 
		   gpointer data)
{
  GmAccount *account = NULL;
  GtkWidget *prefs_window = NULL;

  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow (); 

  account = gm_pw_get_selected_account (prefs_window);
  if (account)
    gm_pw_delete_account_dialog_run (GTK_WIDGET (prefs_window), 
				     account, 
				     GTK_WIDGET (prefs_window));
  gm_account_delete (account);
}


static void
account_dialog_protocol_changed_cb (GtkWidget *menu,
				    gpointer data)
{
  GmAccountsWindow *aw = NULL;

  g_return_if_fail (data != NULL);
  
  aw = GM_ACCOUNTS_WINDOW (data);
  
  switch (gtk_option_menu_get_history (GTK_OPTION_MENU (aw->protocol_option_menu)))
    {
    case 0:
      gtk_label_set_text (GTK_LABEL (aw->host_label), _("Registrar:"));
      gtk_label_set_text (GTK_LABEL (aw->domain_label), _("Realm/Domain:"));
      break;

    case 1:
      gtk_label_set_text (GTK_LABEL (aw->host_label), _("Gatekeeper:"));
      gtk_label_set_text (GTK_LABEL (aw->domain_label), _("Gatekeeper ID:"));
      break;
    };
}


static void
codec_toggled_cb (GtkCellRendererToggle *cell,
		  gchar *path_str,
		  gpointer data)
{
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;

  GSList *codecs_data = NULL;

  gboolean fixed = FALSE;


  g_return_if_fail (data != NULL);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (data));
  path = gtk_tree_path_new_from_string (path_str);


  /* Update the tree model */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, COLUMN_CODEC_ACTIVE, &fixed, -1);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      COLUMN_CODEC_ACTIVE, fixed^1, -1);
  gtk_tree_path_free (path);


  /* Update the gconf key */
  codecs_data = gm_codecs_list_to_gm_conf_list (GTK_WIDGET (data));

  gm_conf_set_string_list (AUDIO_CODECS_KEY "list", codecs_data);

  g_slist_foreach (codecs_data, (GFunc) g_free, NULL);
  g_slist_free (codecs_data);
}


static void codec_moved_cb (GtkWidget *widget, 
			    gpointer data)
{ 	
  GtkTreeIter iter;
  GtkTreeIter *iter2 = NULL;
  GtkTreeView *tree_view = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreePath *tree_path = NULL;

  GSList *codecs_data = NULL;

  gchar *path_str = NULL;

  g_return_if_fail (data != NULL);

  tree_view = GTK_TREE_VIEW (data);
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

  gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), 
				   NULL, &iter);

  iter2 = gtk_tree_iter_copy (&iter);

  path_str = gtk_tree_model_get_string_from_iter (GTK_TREE_MODEL (model), 
						  &iter);
  tree_path = gtk_tree_path_new_from_string (path_str);
  if (!strcmp ((gchar *) g_object_get_data (G_OBJECT (widget), "operation"), "up"))
    gtk_tree_path_prev (tree_path);
  else
    gtk_tree_path_next (tree_path);

  gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, tree_path);
  if (gtk_list_store_iter_is_valid (GTK_LIST_STORE (model), &iter)
      && gtk_list_store_iter_is_valid (GTK_LIST_STORE (model), iter2))
    gtk_list_store_swap (GTK_LIST_STORE (model), &iter, iter2);

  /* Scroll to the new position */
  gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), 
				tree_path, NULL, FALSE, 0, 0);

  gtk_tree_path_free (tree_path);
  gtk_tree_iter_free (iter2);
  g_free (path_str);


  /* Update the gconf key */
  codecs_data = gm_codecs_list_to_gm_conf_list (GTK_WIDGET (data));

  gm_conf_set_string_list (AUDIO_CODECS_KEY "list", codecs_data);

  g_slist_foreach (codecs_data, (GFunc) g_free, NULL);
  g_slist_free (codecs_data);
}


static void
refresh_devices_list_cb (GtkWidget *w,
			 gpointer data)
{
  g_return_if_fail (data != NULL);

  GnomeMeeting::Process ()->DetectDevices ();
}


static void 
stunserver_update_cb (GtkWidget *widget, 
		      gpointer data)
{
  GMEndPoint *ep = NULL;

  ep = GnomeMeeting::Process ()->Endpoint ();

  /* Prevent GDK deadlock */
  gdk_threads_leave ();

  ep->SetSTUNServer ();

  gdk_threads_enter ();
}


#if !GTK_CHECK_VERSION (2, 3, 2)
static void  
file_selector_cb (GtkFileSelection *b, 
		  gpointer data) 
{
  gchar *filename = NULL;

  filename =
    (gchar *) gtk_file_selection_get_filename (GTK_FILE_SELECTION (data));

  gtk_entry_set_text (GTK_ENTRY (g_object_get_data (G_OBJECT (data), "entry")),
		      filename);

  g_signal_emit_by_name (G_OBJECT (g_object_get_data (G_OBJECT (data), "entry")), "activate");
}
#endif


static void
browse_cb (GtkWidget *b, 
	   gpointer data)
{
  GtkWidget *selector = NULL;
#if GTK_CHECK_VERSION (2, 3, 2)
  selector = gtk_file_chooser_dialog_new (_("Choose a Picture"),
					  GTK_WINDOW (gtk_widget_get_toplevel (b)),
					  GTK_FILE_CHOOSER_ACTION_OPEN, 
					  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					  GTK_STOCK_OPEN,
					  GTK_RESPONSE_ACCEPT,
					  NULL);
#ifndef DISABLE_GNOME
  gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (selector), FALSE);
#endif

  if (gtk_dialog_run (GTK_DIALOG (selector)) == GTK_RESPONSE_ACCEPT)
    {
      char *filename;

#ifdef DISABLE_GNOME
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (selector));
#else
      filename = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (selector));
#endif
      gtk_entry_set_text (GTK_ENTRY (data), filename);
      g_free (filename);

      g_signal_emit_by_name (G_OBJECT (data), "activate");
    }

  gtk_widget_destroy (selector);
#else
  selector = gtk_file_selection_new (_("Choose a Picture"));

  gtk_widget_show (selector);

  /* FIX ME: Ugly hack cause the file selector API is not good and I don't
     want to use global variables */
  g_object_set_data (G_OBJECT (selector), "entry", (gpointer) data);

  g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (selector)->ok_button),
		    "clicked",
		    G_CALLBACK (file_selector_cb),
		    (gpointer) selector);

  /* Ensure that the dialog box is destroyed when the user clicks a button. */
  g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (selector)->ok_button),
			    "clicked",
			    G_CALLBACK (gtk_widget_destroy),
			    (gpointer) selector);

  g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (selector)->cancel_button),
			    "clicked",
			    G_CALLBACK (gtk_widget_destroy),
			    (gpointer) selector);

#endif
}


static void
sound_event_changed_cb (GtkEntry *entry,
			gpointer data)
{
  GmPreferencesWindow *pw = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;

  const char *entry_text = NULL;
  gchar *conf_key = NULL;
  gchar *sound_event = NULL;


  g_return_if_fail (data != NULL);
  pw = gm_pw_get_pw (GTK_WIDGET (data));

  selection =
    gtk_tree_view_get_selection (GTK_TREE_VIEW (pw->sound_events_list));
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			2, &conf_key, -1);

    if (conf_key) { 

      entry_text = gtk_entry_get_text (GTK_ENTRY (entry));
      sound_event = gm_conf_get_string (conf_key);

      if (!sound_event || strcmp (entry_text, sound_event))
	gm_conf_set_string (conf_key, (gchar *) entry_text);

      g_free (conf_key);
      g_free (sound_event);
    }
  } 
}


static void
sound_event_clicked_cb (GtkTreeSelection *selection,
			gpointer data)
{
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gchar *conf_key = NULL;
  gchar *sound_event = NULL;

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			2, &conf_key, -1);

    if (conf_key) { 

      sound_event = gm_conf_get_string (conf_key);
      g_signal_handlers_block_matched (G_OBJECT (data),
				       G_SIGNAL_MATCH_FUNC,
				       0, 0, NULL,
				       (gpointer) sound_event_changed_cb,
				       NULL);
      if (sound_event)
	gtk_entry_set_text (GTK_ENTRY (data), sound_event);
      g_signal_handlers_unblock_matched (G_OBJECT (data),
					 G_SIGNAL_MATCH_FUNC,
					 0, 0, NULL,
					 (gpointer) sound_event_changed_cb,
					 NULL);

      g_free (conf_key);
      g_free (sound_event);
    }
  }
}


static void
sound_event_play_cb (GtkWidget *b,
		     gpointer data)
{
  GMSoundEvent ((const char *) gtk_entry_get_text (GTK_ENTRY (data)));
}


static void
sound_event_toggled_cb (GtkCellRendererToggle *cell,
			gchar *path_str,
			gpointer data)
{
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;

  gchar *conf_key = NULL;

  BOOL fixed = FALSE;


  model = (GtkTreeModel *) data;
  path = gtk_tree_path_new_from_string (path_str);

  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, 0, &fixed, 3, &conf_key, -1);

  fixed ^= 1;

  gm_conf_set_bool (conf_key, fixed);

  g_free (conf_key);
  gtk_tree_path_free (path);
}


/* Public functions */
void 
gm_prefs_window_update_devices_list (GtkWidget *prefs_window, 
				     PStringArray audio_input_devices,
				     PStringArray audio_output_devices,
				     PStringArray video_input_devices)
{
  GmPreferencesWindow *pw = NULL;

  gchar **array = NULL;


  g_return_if_fail (prefs_window != NULL);
  pw = gm_pw_get_pw (prefs_window);


  /* The player */
  array = audio_output_devices.ToCharArray ();
  gnome_prefs_string_option_menu_update (pw->audio_player,
					 array,
					 AUDIO_DEVICES_KEY "output_device");
  gnome_prefs_string_option_menu_update (pw->sound_events_output,
					 array,
					 SOUND_EVENTS_KEY "output_device");
  free (array);


  /* The recorder */
  array = audio_input_devices.ToCharArray ();
  gnome_prefs_string_option_menu_update (pw->audio_recorder,
					 array,
					 AUDIO_DEVICES_KEY "input_device");
  free (array);


  /* The Video player */
  array = video_input_devices.ToCharArray ();

  gnome_prefs_string_option_menu_update (pw->video_device,
					 array,
					 VIDEO_DEVICES_KEY "input_device");
  free (array);
}


void
gm_prefs_window_update_account_state (GtkWidget *prefs_window,
				      gboolean refreshing,
				      const char *domain,
				      const char *login,
				      const char *status)
{
  GtkTreeModel *model = NULL;
  
  GtkTreeIter iter;
  
  gchar *host = NULL;
  gchar *username = NULL;
  
  GmPreferencesWindow *pw = NULL;
  
  g_return_if_fail (prefs_window != NULL);
  g_return_if_fail (login != NULL);
  g_return_if_fail (status != NULL);

  
  pw = gm_pw_get_pw (prefs_window);
  
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (pw->accounts_list));
					
  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)){

    do {

      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			  COLUMN_ACCOUNT_HOST, &host,
			  COLUMN_ACCOUNT_LOGIN, &username,
			  -1);

      if ((host && domain && !strcmp (host, domain))
	  && (login && username && !strcmp (login, username))) 
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			    COLUMN_ACCOUNT_STATE, refreshing,
			    COLUMN_ACCOUNT_ERROR_MESSAGE, status, -1);
    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
  }
  

  /* We update it the accounts list to make sure the cursor is updated */
  gm_prefs_window_update_accounts_list (prefs_window);
}


void
gm_prefs_window_update_accounts_list (GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;
  
  GdkCursor *cursor = NULL;

  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gchar *selected_aid = NULL;
  gchar *aid = NULL;

  GmAccount *account = NULL;
  
  GSList *accounts_data = NULL;
  GSList *accounts_data_iter = NULL;

  gboolean found = FALSE;
  gboolean enabled = FALSE;
  gboolean refreshing = FALSE;
  gboolean busy = FALSE;
  gboolean valid_iter = TRUE;
  
  g_return_if_fail (prefs_window != NULL);

  pw = gm_pw_get_pw (prefs_window);


  /* Get the data and the selected codec */
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (pw->accounts_list));
  selection = 
    gtk_tree_view_get_selection (GTK_TREE_VIEW (pw->accounts_list));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			COLUMN_ACCOUNT_AID, &selected_aid, -1);
  }


  /* Loop through all accounts in the configuration.
   * Then find that account in the GUI and updates it. 
   * If we do not find the account in the GUI, append the new account
   * at the end.
   */
  accounts_data = gnomemeeting_get_accounts_list (); 
  accounts_data_iter = accounts_data;
  while (accounts_data_iter && accounts_data_iter->data) {

    account = GM_ACCOUNT (accounts_data_iter->data);
      
    found = FALSE;
    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)) {

      do {
	
	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			    COLUMN_ACCOUNT_ENABLED, &enabled,
			    COLUMN_ACCOUNT_STATE, &refreshing,
			    COLUMN_ACCOUNT_AID, &aid, -1);
	if (aid && account->aid && !strcmp (account->aid, aid)) {

	  busy = busy || refreshing;
	  found = TRUE;
	}
	g_free (aid);

      } while (!found && 
	       gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter)); 
    }
    if (!found) /* No existing entry for that account */ 
      gtk_list_store_append (GTK_LIST_STORE (model), &iter);
      
    gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
			COLUMN_ACCOUNT_ENABLED, account->enabled,
			COLUMN_ACCOUNT_AID, account->aid,
			COLUMN_ACCOUNT_ACCOUNT_NAME, account->account_name,
			COLUMN_ACCOUNT_PROTOCOL_NAME, account->protocol_name,
			COLUMN_ACCOUNT_HOST, account->host,
			COLUMN_ACCOUNT_DOMAIN, account->domain,
			COLUMN_ACCOUNT_LOGIN, account->login,
			COLUMN_ACCOUNT_PASSWORD, account->password,
			COLUMN_ACCOUNT_TIMEOUT, account->timeout,
			COLUMN_ACCOUNT_METHOD, account->method,
			-1); 

    if (selected_aid && account->aid 
	&& !strcmp (selected_aid, account->aid))
      gtk_tree_selection_select_iter (selection, &iter);
    
    accounts_data_iter = g_slist_next (accounts_data_iter);
  }


  /* Loop through the accounts in the window, and check
   * if they are in the configuration. If not, remove them from the GUI.
   */
  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)) {

    do {

      found = FALSE;
      valid_iter = TRUE;

      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			  COLUMN_ACCOUNT_AID, &aid, -1);
      
      accounts_data_iter = accounts_data;
      while (accounts_data_iter && accounts_data_iter->data && !found) {

	account = GM_ACCOUNT (accounts_data_iter->data);

	if (account->aid && aid && !strcmp (account->aid, aid))
	  found = TRUE;
      
	accounts_data_iter = g_slist_next (accounts_data_iter);
      }
      
      if (!found)
	valid_iter = gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

      g_free (aid);

    } while (valid_iter &&
	     gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter)); 
  }


  /* Free things */
  g_slist_foreach (accounts_data, (GFunc) gm_account_delete, NULL);
  g_slist_free (accounts_data);


  /* Update the cursor and the activatable state of all accounts
   * following we are refreshing or not */
  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)) {

    do {

      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_ACCOUNT_ACTIVATABLE, !busy, -1);
    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
  }

  if (busy) {

    cursor = gdk_cursor_new (GDK_WATCH);
    gdk_window_set_cursor (GTK_WIDGET (pw->accounts_box)->window, cursor);
    gdk_cursor_unref (cursor);
  }
  else
    gdk_window_set_cursor (GTK_WIDGET (pw->accounts_box)->window, NULL);
}


void 
gm_prefs_window_update_audio_codecs_list (GtkWidget *prefs_window,
					  OpalMediaFormatList & l)
{
  GmPreferencesWindow *pw = NULL;

  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  OpalMediaFormatList k;
  PStringList m;

  gchar *bandwidth = NULL;
  gchar *selected_codec = NULL;
  gchar **couple = NULL;

  GSList *codecs_data = NULL;
  GSList *codecs_data_iter = NULL;

  int i = 0;


  g_return_if_fail (prefs_window != NULL);

  pw = gm_pw_get_pw (prefs_window);


  /* Get the data and the selected codec */
  k = l;
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (pw->audio_codecs_list));
  codecs_data = gm_conf_get_string_list (AUDIO_CODECS_KEY "list"); 
  selection = 
    gtk_tree_view_get_selection (GTK_TREE_VIEW (pw->audio_codecs_list));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			COLUMN_CODEC_NAME, &selected_codec, -1);
  }

  gtk_list_store_clear (GTK_LIST_STORE (model));


  /* First we add all codecs in the preferences if they are in the list
   * of possible codecs */
  codecs_data_iter = codecs_data;
  while (codecs_data_iter) {

    couple = g_strsplit ((gchar *) codecs_data_iter->data, "=", 0);

    if (couple [0] && couple [1]) {

      if ((i = k.GetValuesIndex (PString (couple [0]))) != P_MAX_INDEX) {

	bandwidth = g_strdup_printf ("%.1f kbps", k [i].GetBandwidth ()/1000.0);

	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			    COLUMN_CODEC_ACTIVE, (PString (couple [1]) == "1"),
			    COLUMN_CODEC_NAME, (const char *) k [i],
			    COLUMN_CODEC_BANDWIDTH, bandwidth,
			    COLUMN_CODEC_SELECTABLE, "true",
			    COLUMN_CODEC_COLOR, "black",
			    -1);
	if (selected_codec && !strcmp (selected_codec, (const char *) k [i]))
	  gtk_tree_selection_select_iter (selection, &iter);

	k.RemoveAt (i);
	g_free (bandwidth);
      }
    }

    codecs_data_iter = codecs_data_iter->next;


    g_strfreev (couple);
  }


  /* #INV: m contains the list of possible codecs from the prefs */

  /* Now we add the codecs */
  for (i = 0 ; i < k.GetSize () ; i++) {

    bandwidth = g_strdup_printf ("%.1f kbps", k [i].GetBandwidth ()/1000.0);

    gtk_list_store_append (GTK_LIST_STORE (model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			COLUMN_CODEC_ACTIVE, FALSE,
			COLUMN_CODEC_NAME, (const char *) k [i],
			COLUMN_CODEC_BANDWIDTH, bandwidth,
			COLUMN_CODEC_SELECTABLE, "true",
			COLUMN_CODEC_COLOR, "black",
			-1);

    if (selected_codec && !strcmp (selected_codec, (const char *) k [i]))
      gtk_tree_selection_select_iter (selection, &iter);

    g_free (bandwidth);
  }
}


void
gm_prefs_window_sound_events_list_build (GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;

  GtkTreeSelection *selection = NULL;
  GtkTreePath *path = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter, selected_iter;

  BOOL enabled = FALSE;

  pw = gm_pw_get_pw (prefs_window);

  selection = 
    gtk_tree_view_get_selection (GTK_TREE_VIEW (pw->sound_events_list));

  if (gtk_tree_selection_get_selected (selection, &model, &selected_iter))
    path = gtk_tree_model_get_path (model, &selected_iter);

  gtk_list_store_clear (GTK_LIST_STORE (model));

  /* Sound on incoming calls */
  enabled = gm_conf_get_bool (SOUND_EVENTS_KEY "enable_incoming_call_sound");
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      0, enabled,
		      1, _("Play sound on incoming calls"),
		      2, SOUND_EVENTS_KEY "incoming_call_sound",
		      3, SOUND_EVENTS_KEY "enable_incoming_call_sound",
		      -1);

  enabled = gm_conf_get_bool (SOUND_EVENTS_KEY "enable_ring_tone_sound");
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      0, enabled,
		      1, _("Play ring tone"),
		      2, SOUND_EVENTS_KEY "ring_tone_sound",
		      3, SOUND_EVENTS_KEY "enable_ring_tone_sound",
		      -1);

  enabled = gm_conf_get_bool (SOUND_EVENTS_KEY "enable_busy_tone_sound");
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      0, enabled,
		      1, _("Play busy tone"),
		      2, SOUND_EVENTS_KEY "busy_tone_sound",
		      3, SOUND_EVENTS_KEY "enable_busy_tone_sound",
		      -1);

  if (!path)
    path = gtk_tree_path_new_from_string ("0");

  gtk_tree_view_set_cursor (GTK_TREE_VIEW (pw->sound_events_list),
			    path, NULL, false);
  gtk_tree_path_free (path);
}


GtkWidget *
gm_prefs_window_new ()
{
  GmPreferencesWindow *pw = NULL;

  GdkPixbuf *pixbuf = NULL;
  GtkWidget *window = NULL;
  GtkWidget *container = NULL;


  window = 
    gnome_prefs_window_new (GNOMEMEETING_IMAGES "gnomemeeting-logo.png");
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("preferences_window"), g_free);
  gtk_window_set_title (GTK_WINDOW (window), _("GnomeMeeting Preferences"));
  pixbuf = gtk_widget_render_icon (GTK_WIDGET (window),
				   GTK_STOCK_PREFERENCES,
				   GTK_ICON_SIZE_MENU, NULL);
  gtk_window_set_icon (GTK_WINDOW (window), pixbuf);
  gtk_widget_realize (GTK_WIDGET (window));
  g_object_unref (pixbuf);


  /* The GMObject data */
  pw = new GmPreferencesWindow ();
  g_object_set_data_full (G_OBJECT (window), "GMObject", 
			  pw, (GDestroyNotify) gm_pw_destroy);


  gnome_prefs_window_section_new (window, _("General"));
  container = gnome_prefs_window_subsection_new (window, _("Personal Data"));
  gm_pw_init_general_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window,
						 _("General Settings"));
  gm_pw_init_interface_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window,
						 _("Directory Settings"));
  gm_pw_init_directories_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window, _("Call Options"));
  gm_pw_init_call_options_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window, _("Call Forwarding"));
  gm_pw_init_call_forwarding_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window, _("NAT Settings"));
  gm_pw_init_nat_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window,
						 _("Sound Events"));
  gm_pw_init_sound_events_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  gnome_prefs_window_section_new (window, _("Protocols"));
  container = gnome_prefs_window_subsection_new (window,
						 _("SIP Settings"));
  gm_pw_init_sip_page (window, container);          
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window,
						 _("H.323 Settings"));
  gm_pw_init_h323_page (window, container);          
  gtk_widget_show_all (GTK_WIDGET (container));

  gnome_prefs_window_section_new (window, _("Codecs"));

  container = gnome_prefs_window_subsection_new (window, _("Audio Codecs"));
  gm_pw_init_audio_codecs_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window, _("Video Codecs"));
  gm_pw_init_video_codecs_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  gnome_prefs_window_section_new (window, _("Devices"));
  container = gnome_prefs_window_subsection_new (window, _("Audio Devices"));
  gm_pw_init_audio_devices_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gnome_prefs_window_subsection_new (window, _("Video Devices"));
  gm_pw_init_video_devices_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

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

