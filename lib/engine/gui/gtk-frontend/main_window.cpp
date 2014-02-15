
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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

#include "config.h"

#include "settings-mappings.h"

#include "main_window.h"

#include "dialpad.h"
#include "statusmenu.h"

#include "gmstockicons.h"
#include "gmentrydialog.h"
#include "gmstatusbar.h"
#include "gmmenuaddon.h"
#include "trigger.h"
#include "menu-builder-tools.h"
#include "menu-builder-gtk.h"
#include "scoped-connections.h"

#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <gio/gio.h>

#include "engine.h"

#include "videoinput-core.h"
#include "audioinput-core.h"
#include "audiooutput-core.h"

#include "ekiga-app.h"
#include "roster-view-gtk.h"
#include "call-history-view-gtk.h"
#include "history-source.h"

#include "opal-bank.h"

#include <algorithm>

enum CallingState {Standby, Calling, Connected, Called};

typedef enum {
  CONTACTS,
  DIALPAD,
  CALL,
  NUM_SECTIONS
} PanelSection;

enum DeviceType {AudioInput, AudioOutput, Ringer, VideoInput};

struct deviceStruct {
  char name[256];
  DeviceType deviceType;
};

G_DEFINE_TYPE (EkigaMainWindow, ekiga_main_window, GM_TYPE_WINDOW);


struct _EkigaMainWindowPrivate
{
  GmApplication *app;
  Ekiga::ServiceCorePtr core;

  boost::shared_ptr<Ekiga::AccountCore> account_core;
  boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core;
  boost::shared_ptr<Ekiga::CallCore> call_core;
  boost::shared_ptr<Ekiga::ContactCore> contact_core;
  boost::shared_ptr<Ekiga::PresenceCore> presence_core;
  boost::shared_ptr<Opal::Bank> bank;
  boost::shared_ptr<Ekiga::Trigger> local_cluster_trigger;
  boost::shared_ptr<History::Source> history_source;

  GtkWidget *call_window;

  GtkAccelGroup *accel;
  GtkWidget *main_menu;
  GtkWidget *main_notebook;
  GtkBuilder *builder;

  /* Dialpad uri toolbar */
  GtkWidget *uri_toolbar;
  GtkWidget *entry;
  GtkListStore *completion;

  /* Actions toolbar */
  GtkWidget *actions_toolbar;
  GtkWidget *preview_button;

  /* notebook pages
   *  (we store the numbers so we know where we are)
   */
  gint roster_view_page_number;
  gint dialpad_page_number;
  gint call_history_page_number;
  GtkWidget* roster_view;
  GtkWidget* call_history_view;

  /* Status Toolbar */
  GtkWidget *status_toolbar;
  GtkWidget *status_option_menu;

  /* Statusbar */
  GtkWidget *statusbar;
  GtkWidget *statusbar_ebox;

  /* Calls */
  boost::shared_ptr<Ekiga::Call> current_call;
  unsigned calling_state;

  gulong roster_selection_connection_id;
  Ekiga::scoped_connections connections;

  /* GSettings */
  boost::shared_ptr<Ekiga::Settings> user_interface_settings;
  boost::shared_ptr<Ekiga::Settings> sound_events_settings;
  boost::shared_ptr<Ekiga::Settings> video_devices_settings;
  boost::shared_ptr<Ekiga::Settings> contacts_settings;
};

/* channel types */
enum {
  CHANNEL_FIRST,
  CHANNEL_AUDIO,
  CHANNEL_VIDEO,
  CHANNEL_LAST
};


/* GUI Functions */
static bool account_completion_helper_cb (Ekiga::AccountPtr acc,
                                          const gchar* text,
                                          EkigaMainWindow* mw);

static void place_call_cb (GtkWidget * /*widget*/,
                           gpointer data);

static void url_changed_cb (GtkEditable *e,
                            gpointer data);

static void ekiga_main_window_append_call_url (EkigaMainWindow *mw,
                                               const char *url);

static const std::string ekiga_main_window_get_call_url (EkigaMainWindow *mw);



/* DESCRIPTION  :  This callback is called when the control panel
 *                 section changes.
 * BEHAVIOR     :  Disable the Contact menu item when the dialpad is
 *                 displayed.
 * PRE          :  /
 */
static void panel_section_changed (GtkNotebook *notebook,
                                   GtkWidget *page,
                                   guint page_num,
                                   gpointer user_data);


/* DESCRIPTION  :  This callback is called when the preview button is toggled.
 * BEHAVIOR     :  Show / hide the call window.
 * PRE          :  /
 */
static void video_preview_changed (GtkToggleToolButton *button,
                                   gpointer data);


/* DESCRIPTION  :  This callback is called when the user
 *                 presses a key.
 * BEHAVIOR     :  Sends a DTMF if we are in a call.
 * PRE          :  A valid pointer to the main window GMObject.
 */
static gboolean key_press_event_cb (EkigaMainWindow *mw,
                                    GdkEventKey *key);


/* DESCRIPTION  :  This callback is called when the user
 *                 clicks on the dialpad button.
 * BEHAVIOR     :  Generates a dialpad event.
 * PRE          :  A valid pointer to the main window GMObject.
 */
static void dialpad_button_clicked_cb (EkigaDialpad  *dialpad,
				       const gchar *button_text,
				       EkigaMainWindow *main_window);


/** Call a number action activated.
 *
 * @param data is a pointer to the EkigaMainWindow.
 */
static void show_dialpad_activated (G_GNUC_UNUSED GSimpleAction *action,
                                    G_GNUC_UNUSED GVariant *parameter,
                                    gpointer data);


/** Pull a trigger from a Ekiga::Service
 *
 * @param data is a pointer to the EkigaMainWindow.
 */
static void pull_trigger_activated (G_GNUC_UNUSED GSimpleAction *action,
                                    G_GNUC_UNUSED GVariant *parameter,
                                    gpointer data);


/* DESCRIPTION  :  This callback is called when the user tries to close
 *                 the application using the window manager.
 * BEHAVIOR     :  Calls the real callback if the notification icon is
 *                 not shown else hide GM.
 * PRE          :  A valid pointer to the main window GMObject.
 */
static void close_activated (G_GNUC_UNUSED GSimpleAction *action,
                             G_GNUC_UNUSED GVariant *parameter,
                             gpointer data);


/* DESCRIPTION  :  This callback is called when the status bar is clicked.
 * BEHAVIOR     :  Clear all info message, not normal messages.
 * PRE          :  The main window GMObject.
 */
static gboolean statusbar_clicked_cb (GtkWidget *,
				      GdkEventButton *,
				      gpointer);

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates the uri toolbar in the dialpad panel.
 * PRE          :  The main window GMObject.
 */
static void ekiga_main_window_init_uri_toolbar (EkigaMainWindow *mw);

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates the actions toolbar in the main window.
 * PRE          :  The main window GMObject.
 */
static void ekiga_main_window_init_actions_toolbar (EkigaMainWindow *mw);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Flashes a message on the statusbar during a few seconds.
 *                 Removes the previous message.
 * PRE           : The main window GMObject, followed by printf syntax format.
 */
static void ekiga_main_window_flash_message (EkigaMainWindow *main_window,
                                             const char *msg,
                                             ...) G_GNUC_PRINTF(2,3);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Displays a message on the statusbar or clears it if msg = 0.
 *                 Removes the previous message.
 * PRE           : The main window GMObject, followed by printf syntax format.
 */
static void ekiga_main_window_push_message (EkigaMainWindow *main_window,
                                            const char *msg,
                                            ...) G_GNUC_PRINTF(2,3);


static GActionEntry win_entries[] =
{
    { "call", show_dialpad_activated, NULL, NULL, NULL, 0 },
    { "add", pull_trigger_activated, NULL, NULL, NULL, 0 },
    { "close", close_activated, NULL, NULL, NULL, 0 }
};


/*
 * Callbacks
 */
static bool
account_completion_helper_cb (Ekiga::AccountPtr acc,
                              const gchar* text,
                              EkigaMainWindow* mw)
{
  Opal::AccountPtr account = boost::dynamic_pointer_cast<Opal::Account>(acc);
  if (account && account->is_enabled ()) {

    if (g_ascii_strncasecmp (text, "sip:", 4) == 0 && account->get_protocol_name () == "SIP") {

      GtkTreeIter iter;
      gchar* entry = NULL;

      entry = g_strdup_printf ("%s@%s", text, account->get_host ().c_str ());
      gtk_list_store_append (mw->priv->completion, &iter);
      gtk_list_store_set (mw->priv->completion, &iter, 0, entry, -1);
      g_free (entry);
    }
  }
  return true;
}

static void
place_call_cb (GtkWidget * /*widget*/,
               gpointer data)
{
  std::string uri;
  EkigaMainWindow *mw = NULL;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (data));

  mw = EKIGA_MAIN_WINDOW (data);

  if (mw->priv->calling_state == Standby) {

    size_t pos;

    // Check for empty uri
    uri = ekiga_main_window_get_call_url (mw);
    pos = uri.find (":");
    if (pos != std::string::npos)
      if (uri.substr (++pos).empty ())
        return;

    // Remove appended spaces
    pos = uri.find_first_of (' ');
    if (pos != std::string::npos)
      uri = uri.substr (0, pos);

    // Dial
    if (!mw->priv->call_core->dial (uri))
      gm_statusbar_flash_message (GM_STATUSBAR (mw->priv->statusbar), _("Could not connect to remote host"));
  }
}

static void
url_changed_cb (GtkEditable *e,
		gpointer data)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (data);
  const char *tip_text = NULL;

  tip_text = gtk_entry_get_text (GTK_ENTRY (e));

  if (g_strrstr (tip_text, "@") == NULL) {
    if (mw->priv->bank) {
      gtk_list_store_clear (mw->priv->completion);
      mw->priv->bank->visit_accounts (boost::bind (&account_completion_helper_cb, _1, tip_text, mw));
    }
  }

  gtk_widget_set_tooltip_text (GTK_WIDGET (e), tip_text);
}


static void
on_account_updated (Ekiga::BankPtr /*bank*/,
		    Ekiga::AccountPtr account,
		    gpointer self)
{
  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (self));

  if (account->get_status () != "") {

    EkigaMainWindow *mw = NULL;
    gchar *msg = NULL;

    mw = EKIGA_MAIN_WINDOW (self);
    msg = g_strdup_printf ("%s: %s",
			   account->get_name ().c_str (),
			   account->get_status ().c_str ());

    ekiga_main_window_flash_message (mw, "%s", msg);

    g_free (msg);
  }
}


static void on_setup_call_cb (boost::shared_ptr<Ekiga::CallManager> manager,
                              boost::shared_ptr<Ekiga::Call>  call,
                              gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  if (!call->is_outgoing () && !manager->get_auto_answer ()) {
    if (mw->priv->current_call)
      return; // No call setup needed if already in a call

    mw->priv->audiooutput_core->start_play_event ("incoming_call_sound", 4000, 256);

    mw->priv->current_call = call;
    mw->priv->calling_state = Called;
  }
  else {

    gm_application_show_call_window (mw->priv->app);

    mw->priv->current_call = call;
    mw->priv->calling_state = Calling;
  }

  /* Unsensitive a few things */
  gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->uri_toolbar), false);
  gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->preview_button), false);
}


static void on_ringing_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                                boost::shared_ptr<Ekiga::Call>  call,
                                gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  if (call->is_outgoing ()) {
    mw->priv->audiooutput_core->start_play_event("ring_tone_sound", 3000, 256);
  }
}


static void on_established_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                                    boost::shared_ptr<Ekiga::Call>  call,
                                    gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);
  gchar* info = NULL;

  /* Update calling state */
  mw->priv->calling_state = Connected;

  /* %s is the SIP/H.323 address of the remote user, this text is shown
     below video during a call */
  info = g_strdup_printf (_("Connected with %s"),
			  call->get_remote_party_name ().c_str ());
  ekiga_main_window_flash_message (mw, "%s", info);
  g_free (info);

  /* Manage sound events */

  mw->priv->audiooutput_core->stop_play_event("incoming_call_sound");
  mw->priv->audiooutput_core->stop_play_event("ring_tone_sound");

  gm_application_show_call_window (mw->priv->app);
}


static void on_cleared_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                                boost::shared_ptr<Ekiga::Call> call,
                                std::string reason,
                                gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  if (mw->priv->current_call && mw->priv->current_call->get_id () != call->get_id ()) {
    return; // Trying to clear another call than the current active one
  }

  /* Update calling state */
  if (mw->priv->current_call)
    mw->priv->current_call = boost::shared_ptr<Ekiga::Call>();
  mw->priv->calling_state = Standby;

  /* Info message */
  ekiga_main_window_flash_message (mw, "%s", reason.c_str ());

  /* Sound events */
  mw->priv->audiooutput_core->stop_play_event("incoming_call_sound");
  mw->priv->audiooutput_core->stop_play_event("ring_tone_sound");

  /* Sensitive a few things back */
  gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->uri_toolbar), true);
  gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->preview_button), true);
}


// FIXME: this should be done through a notification
static void on_missed_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                               boost::shared_ptr<Ekiga::Call>  call,
                               gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  /* Display info first */
  gchar* info = NULL;
  info = g_strdup_printf (_("Missed call from %s"),
			  call->get_remote_party_name ().c_str ());
  ekiga_main_window_push_message (mw, "%s", info);
  g_free (info);

  // FIXME: the engine should take care of this
  /* If the cleared call is the current one, switch back to standby, otherwise return
   * as long as the information has been displayed */
  if (mw->priv->current_call && mw->priv->current_call->get_id () == call->get_id ()) {
    mw->priv->current_call = boost::shared_ptr<Ekiga::Call>();
    mw->priv->calling_state = Standby;

    /* Sensitive a few things back */
    gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->uri_toolbar), true);
    gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->preview_button), true);

    /* Clear sounds */
    mw->priv->audiooutput_core->stop_play_event ("incoming_call_sound");
    mw->priv->audiooutput_core->stop_play_event ("ring_tone_sound");
  }
}


static bool on_handle_errors (std::string error,
                              gpointer data)
{
  g_return_val_if_fail (data != NULL, false);

  GtkWidget *dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW (data),
                                                          GTK_DIALOG_MODAL,
                                                          GTK_MESSAGE_ERROR,
                                                          GTK_BUTTONS_OK,
                                                          "%s",
                                                          error.c_str ());

  gtk_window_set_title (GTK_WINDOW (dialog), _("Error"));
  g_signal_connect_swapped (dialog, "response",
                            G_CALLBACK (gtk_widget_destroy),
                            dialog);

  gtk_widget_show_all (dialog);

  return true;
}


/* GTK callbacks */
static void
on_history_selection_changed (G_GNUC_UNUSED GtkWidget* view,
			      gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);
  gint section;
  GtkWidget* menu = gtk_menu_new_from_model (G_MENU_MODEL (gtk_builder_get_object (mw->priv->builder, "contact")));

  section = gtk_notebook_get_current_page (GTK_NOTEBOOK (mw->priv->main_notebook));

  if (section == mw->priv->call_history_page_number) {

    MenuBuilderGtk builder (menu);
    gtk_widget_set_sensitive (menu, TRUE);

    if (call_history_view_gtk_populate_menu_for_selected (CALL_HISTORY_VIEW_GTK (mw->priv->call_history_view), builder)) {

      gtk_widget_show_all (builder.menu);
    } else {

      gtk_widget_set_sensitive (menu, FALSE);
      g_object_ref_sink (builder.menu);
      g_object_unref (builder.menu);
    }
  } else {

    gtk_widget_set_sensitive (menu, FALSE);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), NULL);
  }
}

static void
on_roster_selection_changed (G_GNUC_UNUSED GtkWidget* view,
			     gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);
  gint section;
  GtkWidget* menu = gtk_menu_get_widget (mw->priv->main_menu, "contact");

  if (GTK_IS_MENU_ITEM (menu)) {

    section = gtk_notebook_get_current_page (GTK_NOTEBOOK (mw->priv->main_notebook));

    if (section == mw->priv->roster_view_page_number) {

      MenuBuilderGtk builder;
      gtk_widget_set_sensitive (menu, TRUE);

      if (roster_view_gtk_populate_menu_for_selected (ROSTER_VIEW_GTK (mw->priv->roster_view), builder)) {

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), builder.menu);
	gtk_widget_show_all (builder.menu);
      } else {

	gtk_widget_set_sensitive (menu, FALSE);
	g_object_ref_sink (builder.menu);
	g_object_unref (builder.menu);
      }
    } else {

      gtk_widget_set_sensitive (menu, FALSE);
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), NULL);
    }
  }
}


static void
panel_section_changed (G_GNUC_UNUSED GtkNotebook *notebook,
                       G_GNUC_UNUSED GtkWidget *page,
                       guint section,
                       gpointer data)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (data);
  GtkWidget* menu = NULL;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (data));

  if (section != (unsigned) mw->priv->roster_view_page_number
      && section != (unsigned) mw->priv->call_history_page_number) {

    menu = gtk_menu_get_widget (mw->priv->main_menu, "contact");
    gtk_widget_set_sensitive (menu, FALSE);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), NULL);
  }
}


static void
video_preview_changed (GtkToggleToolButton *button,
                       gpointer data)
{
  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (data));

  EkigaMainWindow* mw = EKIGA_MAIN_WINDOW (data);

  if (mw->priv->calling_state == Standby) {

    bool toggled = gtk_toggle_tool_button_get_active (button);
    if (!toggled) {
      if (mw->priv->call_window)
        gtk_widget_destroy (mw->priv->call_window);
      mw->priv->call_window = NULL;
    }
    else
      mw->priv->call_window = gm_application_show_call_window (mw->priv->app);
  }
}


static void
dialpad_button_clicked_cb (EkigaDialpad  * /* dialpad */,
			   const gchar *button_text,
			   EkigaMainWindow *mw)
{
  if (mw->priv->current_call && mw->priv->calling_state == Connected)
    mw->priv->current_call->send_dtmf (button_text[0]);
  else
    ekiga_main_window_append_call_url (mw, button_text);
}


static gboolean
key_press_event_cb (EkigaMainWindow *mw,
                    GdkEventKey *key)
{
  const char valid_dtmfs[] = "1234567890#*";
  unsigned i = 0;

  if (mw->priv->current_call) {
    while (i < strlen (valid_dtmfs)) {
      if (key->string[0] && key->string[0] == valid_dtmfs[i]) {
        mw->priv->current_call->send_dtmf (key->string[0]);
        return true;
      }
      i++;
    }
  }

  return false;
}


static void
show_dialpad_activated (G_GNUC_UNUSED GSimpleAction *action,
                        G_GNUC_UNUSED GVariant *parameter,
                        gpointer data)
{
  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (data));
  EkigaMainWindow *self = EKIGA_MAIN_WINDOW (data);

  self->priv->user_interface_settings->set_enum ("panel-section", DIALPAD);
}


static void
pull_trigger_activated (G_GNUC_UNUSED GSimpleAction *action,
                        G_GNUC_UNUSED GVariant *parameter,
                        gpointer self)
{
  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (self));
  boost::shared_ptr<Ekiga::Trigger> trigger = EKIGA_MAIN_WINDOW (self)->priv->local_cluster_trigger;

  g_return_if_fail (trigger != NULL);

  trigger->pull ();
}


static void
close_activated (G_GNUC_UNUSED GSimpleAction *action,
                 G_GNUC_UNUSED GVariant *parameter,
                 gpointer data)
{
  // If we have persistent notifications:
  //  - we can hide the window
  //  - clicking on a notification should show the window back
  //  - launching the application again should show the window back
  // If we do not have persistent notifications:
  //  - the status icon allows showing the window back
  gtk_widget_hide (GTK_WIDGET (data));
}


static gboolean
statusbar_clicked_cb (G_GNUC_UNUSED GtkWidget *widget,
		      G_GNUC_UNUSED GdkEventButton *event,
		      gpointer data)
{
  g_return_val_if_fail (EKIGA_IS_MAIN_WINDOW (data), FALSE);

  ekiga_main_window_push_message (EKIGA_MAIN_WINDOW (data), NULL);

  return FALSE;
}


static void
ekiga_main_window_append_call_url (EkigaMainWindow *mw,
				   const char *url)
{
  int pos = -1;
  GtkEditable *entry;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));
  g_return_if_fail (url != NULL);

  entry = GTK_EDITABLE (mw->priv->entry);

  if (gtk_editable_get_selection_bounds (entry, NULL, NULL))
    gtk_editable_delete_selection (entry);

  pos = gtk_editable_get_position (entry);
  gtk_editable_insert_text (entry, url, strlen (url), &pos);
  gtk_editable_select_region (entry, -1, -1);
  gtk_editable_set_position (entry, pos);
}


static const std::string
ekiga_main_window_get_call_url (EkigaMainWindow *mw)
{
  g_return_val_if_fail (EKIGA_IS_MAIN_WINDOW (mw), NULL);

  const gchar* entry_text = gtk_entry_get_text (GTK_ENTRY (mw->priv->entry));

  if (entry_text != NULL)
    return entry_text;
  else
    return "";
}

static void
ekiga_main_window_init_uri_toolbar (EkigaMainWindow *mw)
{
  GtkWidget *call_button = NULL;
  GtkWidget *image = NULL;
  GtkToolItem *item = NULL;
  GtkEntryCompletion *completion = NULL;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  /* The call horizontal toolbar */
  mw->priv->uri_toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_style (GTK_TOOLBAR (mw->priv->uri_toolbar), GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (mw->priv->uri_toolbar), FALSE);

  /* URL bar */
  /* Entry */
  item = gtk_tool_item_new ();
  mw->priv->entry = gtk_entry_new ();
  mw->priv->completion = gtk_list_store_new (1, G_TYPE_STRING);
  completion = gtk_entry_completion_new ();
  gtk_entry_completion_set_model (GTK_ENTRY_COMPLETION (completion), GTK_TREE_MODEL (mw->priv->completion));
  gtk_entry_set_completion (GTK_ENTRY (mw->priv->entry), completion);
  gtk_entry_set_text (GTK_ENTRY (mw->priv->entry), "sip:");
  gtk_entry_completion_set_inline_completion (GTK_ENTRY_COMPLETION (completion), false);
  gtk_entry_completion_set_popup_completion (GTK_ENTRY_COMPLETION (completion), true);
  gtk_entry_completion_set_text_column (GTK_ENTRY_COMPLETION (completion), 0);

  gtk_container_add (GTK_CONTAINER (item), mw->priv->entry);
  gtk_container_set_border_width (GTK_CONTAINER (item), 0);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), true);

  // activate Ctrl-L to get the entry focus
  gtk_widget_add_accelerator (mw->priv->entry, "grab-focus",
			      mw->priv->accel, GDK_KEY_L,
			      (GdkModifierType) GDK_CONTROL_MASK,
			      (GtkAccelFlags) 0);

  gtk_editable_set_position (GTK_EDITABLE (mw->priv->entry), -1);

  g_signal_connect (mw->priv->entry, "changed",
		    G_CALLBACK (url_changed_cb), mw);
  g_signal_connect (mw->priv->entry, "activate",
		    G_CALLBACK (place_call_cb), mw);

  gtk_toolbar_insert (GTK_TOOLBAR (mw->priv->uri_toolbar), item, 0);

  /* The call button */
  item = gtk_tool_item_new ();
  call_button = gtk_button_new ();
  image = gtk_image_new_from_icon_name ("phone-pick-up", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_button_set_image (GTK_BUTTON (call_button), image);
  gtk_button_set_relief (GTK_BUTTON (call_button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (item), call_button);
  gtk_container_set_border_width (GTK_CONTAINER (call_button), 0);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);

  gtk_widget_set_tooltip_text (GTK_WIDGET (call_button),
			       _("Enter a URI on the left, and click this button to place a call or to hang up"));

  gtk_toolbar_insert (GTK_TOOLBAR (mw->priv->uri_toolbar), item, -1);

  g_signal_connect (call_button, "clicked",
                    G_CALLBACK (place_call_cb),
                    mw);
}

static void
ekiga_main_window_init_actions_toolbar (EkigaMainWindow *mw)
{
  GtkWidget *image = NULL;
  GtkWidget *menu_button = NULL;
  GtkWidget *box = NULL;
  GtkWidget *button = NULL;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET (mw)),
                               "header-bar");

  gtk_style_context_set_junction_sides (gtk_widget_get_style_context (GTK_WIDGET (mw)),
                                        GTK_JUNCTION_BOTTOM);

  mw->priv->actions_toolbar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  button = gtk_toggle_button_new ();
  image = gtk_image_new_from_icon_name ("camera-web-symbolic", GTK_ICON_SIZE_MENU);
  g_object_set (G_OBJECT (image), "margin", 3, NULL);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_widget_set_tooltip_text (GTK_WIDGET (button),
                               _("Display images from your camera device"));
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "win.enable-preview");
  gtk_box_pack_start (GTK_BOX (mw->priv->actions_toolbar), button, FALSE, FALSE, 0);
  gtk_widget_set_margin_left (button, 6);
  gtk_widget_set_margin_right (button, 6);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  button = gtk_toggle_button_new ();
  image = gtk_image_new_from_icon_name ("avatar-default-symbolic", GTK_ICON_SIZE_MENU);
  g_object_set (G_OBJECT (image), "margin", 3, NULL);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_widget_set_tooltip_text (GTK_WIDGET (button),
                               _("View the contacts list"));
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "win.panel-section::contacts");
  gtk_container_add (GTK_CONTAINER (box), button);

  button = gtk_toggle_button_new ();
  image = gtk_image_new_from_icon_name ("input-dialpad-symbolic", GTK_ICON_SIZE_MENU);
  g_object_set (G_OBJECT (image), "margin", 3, NULL);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_widget_set_tooltip_text (GTK_WIDGET (button),
                               _("View the dialpad"));
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "win.panel-section::dialpad");
  gtk_container_add (GTK_CONTAINER (box), button);

  button = gtk_toggle_button_new ();
  image = gtk_image_new_from_icon_name ("document-open-recent-symbolic", GTK_ICON_SIZE_MENU);
  g_object_set (G_OBJECT (image), "margin", 3, NULL);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_widget_set_tooltip_text (GTK_WIDGET (button),
                               _("View the call history"));
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "win.panel-section::call-history");
  gtk_container_add (GTK_CONTAINER (box), button);

  gtk_style_context_add_class (gtk_widget_get_style_context (box),
                               GTK_STYLE_CLASS_RAISED);
  gtk_style_context_add_class (gtk_widget_get_style_context (box),
                               GTK_STYLE_CLASS_LINKED);

  gtk_box_pack_start (GTK_BOX (mw->priv->actions_toolbar), box, FALSE, FALSE, 0);
  gtk_widget_set_margin_left (box, 6);
  gtk_widget_set_margin_right (box, 6);

  button = gtk_menu_button_new ();
  image = gtk_image_new_from_icon_name ("emblem-system-symbolic", GTK_ICON_SIZE_MENU);
  g_object_set (G_OBJECT (image), "margin", 3, NULL);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (button),
                                  G_MENU_MODEL (gtk_builder_get_object (mw->priv->builder, "menubar")));
  gtk_box_pack_end (GTK_BOX (mw->priv->actions_toolbar), button, FALSE, FALSE, 0);
  gtk_widget_set_margin_left (button, 6);
  gtk_widget_set_margin_right (button, 6);
}

static void
ekiga_main_window_init_menu (EkigaMainWindow *mw)
{
  static const char* win_menu =
    "<?xml version='1.0'?>"
    "<interface>"
    "  <menu id='menubar'>"
    "    <submenu>"
    "      <attribute name='label' translatable='yes'>Co_ntact</attribute>"
    "      <section id='action'>"
    "      </section>"
    "      <section>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>_Add Contact</attribute>"
    "          <attribute name='action'>win.add</attribute>"
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Place _Call</attribute>"
    "          <attribute name='action'>win.call</attribute>"
    "        </item>"
    "      </section>"
    "      <section>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>_Close</attribute>"
    "          <attribute name='action'>win.close</attribute>"
    "        </item>"
    "      </section>"
    "    </submenu>"
    "    <submenu>"
    "      <attribute name='label' translatable='yes'>_View</attribute>"
    "      <section>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>_Video Preview</attribute>"
    "          <attribute name='action'>win.enable-preview</attribute>"
    "        </item>"
    "      </section>"
    "      <section>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Con_tacts</attribute>"
    "          <attribute name='action'>win.panel-section</attribute>"
    "          <attribute name='target'>contacts</attribute>"
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>_Dialpad</attribute>"
    "          <attribute name='action'>win.panel-section</attribute>"
    "          <attribute name='target'>dialpad</attribute>"
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Call _History</attribute>"
    "          <attribute name='action'>win.panel-section</attribute>"
    "          <attribute name='target'>call-history</attribute>"
    "        </item>"
    "      </section>"
    "      <section>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Show _Offline Contacts</attribute>"
    "          <attribute name='action'>win.show-offline-contacts</attribute>"
    "        </item>"
    "      </section>"
    "    </submenu>"
    "  </menu>"
    "</interface>";

  mw->priv->builder = gtk_builder_new ();
  gtk_builder_add_from_string (mw->priv->builder, win_menu, -1, NULL);

  g_action_map_add_action (G_ACTION_MAP (mw),
                           g_settings_create_action (mw->priv->video_devices_settings->get_g_settings (),
                                                     "enable-preview"));
  g_action_map_add_action (G_ACTION_MAP (mw),
                           g_settings_create_action (mw->priv->user_interface_settings->get_g_settings (),
                                                     "panel-section"));
  g_action_map_add_action (G_ACTION_MAP (mw),
                           g_settings_create_action (mw->priv->contacts_settings->get_g_settings (),
                                                     "show-offline-contacts"));
  g_action_map_add_action_entries (G_ACTION_MAP (mw),
                                   win_entries, G_N_ELEMENTS (win_entries),
                                   mw);
}


static void
ekiga_main_window_init_status_toolbar (EkigaMainWindow *mw)
{
  GtkToolItem *item = NULL;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  /* The main horizontal toolbar */
  mw->priv->status_toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_style (GTK_TOOLBAR (mw->priv->status_toolbar), GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (mw->priv->status_toolbar), FALSE);

  item = gtk_tool_item_new ();
  mw->priv->status_option_menu = status_menu_new (*mw->priv->core);
  status_menu_set_parent_window (STATUS_MENU (mw->priv->status_option_menu),
                                 GTK_WINDOW (mw));
  gtk_container_add (GTK_CONTAINER (item), mw->priv->status_option_menu);
  gtk_container_set_border_width (GTK_CONTAINER (item), 0);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), TRUE);

  gtk_toolbar_insert (GTK_TOOLBAR (mw->priv->status_toolbar), item, 0);

  gtk_widget_show_all (mw->priv->status_toolbar);
}


static void
ekiga_main_window_init_contact_list (EkigaMainWindow *mw)
{
  GtkWidget *label = NULL;

  label = gtk_label_new (_("Contacts"));
  mw->priv->roster_view = roster_view_gtk_new (mw->priv->presence_core);
  mw->priv->roster_view_page_number = gtk_notebook_append_page (GTK_NOTEBOOK (mw->priv->main_notebook), mw->priv->roster_view, label);
  g_object_ref (mw->priv->roster_view); // keep it alive as long as we didn't unconnect the signal :
  mw->priv->roster_selection_connection_id = g_signal_connect (mw->priv->roster_view, "selection-changed",
							       G_CALLBACK (on_roster_selection_changed), mw);
}


static void
ekiga_main_window_init_dialpad (EkigaMainWindow *mw)
{
  GtkWidget *dialpad = NULL;
  GtkWidget *alignment = NULL;
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  dialpad = ekiga_dialpad_new (mw->priv->accel);
  g_signal_connect (dialpad, "button-clicked",
                    G_CALLBACK (dialpad_button_clicked_cb), mw);

  alignment = gtk_alignment_new (0.5, 0.5, 0.2, 0.2);
  gtk_container_add (GTK_CONTAINER (alignment), dialpad);
  gtk_box_pack_start (GTK_BOX (vbox), alignment, true, true, 0);

  ekiga_main_window_init_uri_toolbar (mw);
  gtk_box_pack_start (GTK_BOX (vbox), mw->priv->uri_toolbar, false, false, 0);

  label = gtk_label_new (_("Dialpad"));
  mw->priv->dialpad_page_number = gtk_notebook_append_page (GTK_NOTEBOOK (mw->priv->main_notebook), vbox, label);

  g_signal_connect (mw, "key-press-event",
                    G_CALLBACK (key_press_event_cb), mw);
}


static void
ekiga_main_window_init_history (EkigaMainWindow *mw)
{
  GtkWidget *label = NULL;

  boost::shared_ptr<History::Book> history_book
    = mw->priv->history_source->get_book ();

  mw->priv->call_history_view = call_history_view_gtk_new (history_book);

  label = gtk_label_new (_("Call history"));
  mw->priv->call_history_page_number =
    gtk_notebook_append_page (GTK_NOTEBOOK (mw->priv->main_notebook), mw->priv->call_history_view, label);
  g_signal_connect (mw->priv->call_history_view, "selection-changed",
		    G_CALLBACK (on_history_selection_changed), mw);
}

static void
ekiga_main_window_init_gui (EkigaMainWindow *mw)
{
  GtkWidget *window_vbox;
  // FIXME ??? ekiga-settings.h
  static const gchar *main_views [] = { "contacts", "dialpad", "call-history", NULL };

  gtk_window_set_title (GTK_WINDOW (mw), _("Ekiga Softphone"));
  gtk_window_set_icon_name (GTK_WINDOW (mw), GM_ICON_LOGO);

  window_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (mw), window_vbox);
  gtk_widget_show_all (window_vbox);

  /* The main menu */
  ekiga_main_window_init_menu (mw);

  /* Status bar */
  mw->priv->statusbar = gm_statusbar_new ();

  /* The actions toolbar */
  ekiga_main_window_init_actions_toolbar (mw);
  gtk_box_pack_start (GTK_BOX (window_vbox), mw->priv->actions_toolbar,
                      false, false, 0);

  /* The status toolbar */
  ekiga_main_window_init_status_toolbar (mw);
  gtk_box_pack_start (GTK_BOX (window_vbox), mw->priv->status_toolbar,
                      false, true, 0);

  /* The notebook pages */
  mw->priv->main_notebook = gtk_notebook_new ();
  gtk_notebook_popup_enable (GTK_NOTEBOOK (mw->priv->main_notebook));
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (mw->priv->main_notebook), false);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (mw->priv->main_notebook), false);

  ekiga_main_window_init_contact_list (mw);
  ekiga_main_window_init_dialpad (mw);
  ekiga_main_window_init_history (mw);
  gtk_box_pack_start (GTK_BOX (window_vbox), mw->priv->main_notebook,
                      true, true, 0);

  /* The statusbar */
  mw->priv->statusbar_ebox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (mw->priv->statusbar_ebox), mw->priv->statusbar);
  gtk_box_pack_start (GTK_BOX (window_vbox), mw->priv->statusbar_ebox,
                      FALSE, FALSE, 0);
  gtk_widget_show_all (mw->priv->statusbar_ebox);

  g_signal_connect (mw->priv->statusbar_ebox, "button-press-event",
		    G_CALLBACK (statusbar_clicked_cb), mw);

  /* Realize */
  gtk_widget_realize (GTK_WIDGET (mw));
  gtk_widget_show_all (window_vbox);

  /* Update the widget when the user changes the configuration */
  g_settings_bind_with_mapping (mw->priv->user_interface_settings->get_g_settings (),
                                "panel-section", mw->priv->main_notebook,
                                "page",
                                G_SETTINGS_BIND_DEFAULT,
                                string_gsettings_get_from_int,
                                string_gsettings_set_from_int,
                                (gpointer) main_views,
                                NULL);

  /* Update the menu when the page is changed */
  g_signal_connect (mw->priv->main_notebook, "switch-page",
		    G_CALLBACK (panel_section_changed), mw);
}


static void
ekiga_main_window_init (EkigaMainWindow *mw)
{
  mw->priv = new EkigaMainWindowPrivate;

  /* Accelerators */
  mw->priv->accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (mw), mw->priv->accel);
  g_object_unref (mw->priv->accel);

  mw->priv->current_call = boost::shared_ptr<Ekiga::Call>();
  mw->priv->calling_state = Standby;
  mw->priv->call_window = NULL;

  mw->priv->user_interface_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (USER_INTERFACE ".main-window"));
  mw->priv->sound_events_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (SOUND_EVENTS_SCHEMA));
  mw->priv->video_devices_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (VIDEO_DEVICES_SCHEMA));
  mw->priv->contacts_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (CONTACTS_SCHEMA));
}


static GObject *
ekiga_main_window_constructor (GType the_type,
                               guint n_construct_properties,
                               GObjectConstructParam *construct_params)
{
  GObject *object;

  object = G_OBJECT_CLASS (ekiga_main_window_parent_class)->constructor
                          (the_type, n_construct_properties, construct_params);

  return object;
}

static void
ekiga_main_window_dispose (GObject* gobject)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (gobject);

  if (mw->priv->roster_view) {

    g_signal_handler_disconnect (mw->priv->roster_view,
				 mw->priv->roster_selection_connection_id);
    g_object_unref (mw->priv->roster_view);
    mw->priv->roster_view = NULL;
  }

  G_OBJECT_CLASS (ekiga_main_window_parent_class)->dispose (gobject);
}

static void
ekiga_main_window_finalize (GObject *gobject)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (gobject);

  delete mw->priv;

  G_OBJECT_CLASS (ekiga_main_window_parent_class)->finalize (gobject);
}

static gboolean
ekiga_main_window_focus_in_event (GtkWidget     *widget,
                                  GdkEventFocus *event)
{
  if (gtk_window_get_urgency_hint (GTK_WINDOW (widget)))
    gtk_window_set_urgency_hint (GTK_WINDOW (widget), FALSE);

  return GTK_WIDGET_CLASS (ekiga_main_window_parent_class)->focus_in_event (widget, event);
}


static void
ekiga_main_window_class_init (EkigaMainWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructor = ekiga_main_window_constructor;
  object_class->dispose = ekiga_main_window_dispose;
  object_class->finalize = ekiga_main_window_finalize;

  widget_class->focus_in_event = ekiga_main_window_focus_in_event;
}


static void
ekiga_main_window_connect_engine_signals (EkigaMainWindow *mw)
{
  boost::signals2::connection conn;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  /* Engine Signals callbacks */
  conn = mw->priv->account_core->account_updated.connect (boost::bind (&on_account_updated, _1, _2, (gpointer) mw));
  mw->priv->connections.add (conn);

  conn = mw->priv->call_core->setup_call.connect (boost::bind (&on_setup_call_cb, _1, _2, (gpointer) mw));
  mw->priv->connections.add (conn);

  conn = mw->priv->call_core->ringing_call.connect (boost::bind (&on_ringing_call_cb, _1, _2, (gpointer) mw));
  mw->priv->connections.add (conn);

  conn = mw->priv->call_core->established_call.connect (boost::bind (&on_established_call_cb, _1, _2, (gpointer) mw));
  mw->priv->connections.add (conn);

  conn = mw->priv->call_core->cleared_call.connect (boost::bind (&on_cleared_call_cb, _1, _2, _3, (gpointer) mw));
  mw->priv->connections.add (conn);

  conn = mw->priv->call_core->missed_call.connect (boost::bind (&on_missed_call_cb, _1, _2, (gpointer) mw));
  mw->priv->connections.add (conn);

  conn = mw->priv->call_core->errors.connect (boost::bind (&on_handle_errors, _1, (gpointer) mw));
  mw->priv->connections.add (conn);
}

GtkWidget *
gm_main_window_new (GmApplication *app)
{
  EkigaMainWindow *mw;

  g_return_val_if_fail (GM_IS_APPLICATION (app), NULL);

  /* basic gtk+ setup  */
  mw = EKIGA_MAIN_WINDOW (g_object_new (EKIGA_TYPE_MAIN_WINDOW,
                                        "application", GTK_APPLICATION (app),
					"key", USER_INTERFACE ".main-window",
					NULL));
  Ekiga::ServiceCorePtr core = gm_application_get_core (app);

  /* fetching needed engine objects */
  mw->priv->core = core;
  mw->priv->app = app;

  mw->priv->account_core
    = core->get<Ekiga::AccountCore> ("account-core");
  mw->priv->audiooutput_core
    = core->get<Ekiga::AudioOutputCore>("audiooutput-core");
  mw->priv->call_core
    = core->get<Ekiga::CallCore> ("call-core");
  mw->priv->contact_core
    = core->get<Ekiga::ContactCore> ("contact-core");
  mw->priv->presence_core
    = core->get<Ekiga::PresenceCore> ("presence-core");
  mw->priv->bank
    = core->get<Opal::Bank> ("opal-account-store");
  mw->priv->local_cluster_trigger
    = core->get<Ekiga::Trigger> ("local-cluster");
  mw->priv->history_source
    = core->get<History::Source> ("call-history-store");

  ekiga_main_window_connect_engine_signals (mw);

  ekiga_main_window_init_gui (mw);

  return GTK_WIDGET(mw);
}


static void
ekiga_main_window_flash_message (EkigaMainWindow *mw,
				 const char *msg,
				 ...)
{
  char buffer [1025];
  va_list args;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  va_start (args, msg);

  if (msg == NULL)
    buffer[0] = 0;
  else
    vsnprintf (buffer, 1024, msg, args);

  gm_statusbar_flash_message (GM_STATUSBAR (mw->priv->statusbar), "%s", buffer);
  va_end (args);
}


static void
ekiga_main_window_push_message (EkigaMainWindow *mw,
				const char *msg,
				...)
{
  char buffer [1025];
  va_list args;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  va_start (args, msg);

  if (msg == NULL)
    buffer[0] = 0;
  else
    vsnprintf (buffer, 1024, msg, args);

  gm_statusbar_push_message (GM_STATUSBAR (mw->priv->statusbar), "%s", buffer);
  va_end (args);
}
