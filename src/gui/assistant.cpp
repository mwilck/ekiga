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
 *                         druid.cpp  -  description
 *                         --------------------------
 *   begin                : Mon May 1 2002
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *                          (C) 2008 by Steve Frécinaux
 *   description          : This file contains all the functions needed to
 *                          build the druid.
 */

#include "config.h"

#include "framework/services.h"
#include "ekiga.h"
#include "gmconf.h"
#include "misc.h"
#include "devices/audio.h"
#include "toolbox/toolbox.h"
#include "assistant.h"

#include "vidinput-core.h"

G_DEFINE_TYPE(EkigaAssistant, ekiga_assistant, GTK_TYPE_ASSISTANT)

struct _EkigaAssistantPrivate
{
  Ekiga::ServiceCore *core;
  GdkPixbuf *icon;

  GtkWidget *welcome_page;
  GtkWidget *personal_data_page;
  GtkWidget *ekiga_net_page;
  GtkWidget *connection_type_page;
  GtkWidget *audio_manager_page;
  GtkWidget *audio_devices_page;
  GtkWidget *video_manager_page;
  GtkWidget *summary_page;

  GtkWidget *name;

  GtkWidget *username;
  GtkWidget *password;
  GtkWidget *skip_ekiga_net;

  GtkWidget *connection_type;

  GtkWidget *audio_manager;

  GtkWidget *audio_player;
  GtkWidget *audio_recorder;

  GtkWidget *video_manager;

  GtkListStore *summary_model;
};

/* presenting the network connectoin type to the user */
enum {
  CNX_LABEL_COLUMN,
  CNX_CODE_COLUMN
};

enum {
  SUMMARY_KEY_COLUMN,
  SUMMARY_VALUE_COLUMN
};

static GtkWidget *
create_page (EkigaAssistant       *assistant,
             const gchar          *title,
             GtkAssistantPageType  page_type)
{
  GtkWidget *vbox;

  vbox = gtk_vbox_new (FALSE, 6);

  gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);

  gtk_assistant_append_page (GTK_ASSISTANT (assistant), vbox);
  gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), vbox, title);
  gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), vbox, page_type);
  gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), vbox, assistant->priv->icon);

  if (page_type != GTK_ASSISTANT_PAGE_CONTENT)
    gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), vbox, TRUE);

  return vbox;
}

static void
set_current_page_complete (GtkAssistant *assistant,
                           gboolean      complete)
{
  gint page_number;
  GtkWidget *current_page;

  page_number = gtk_assistant_get_current_page (assistant);
  current_page = gtk_assistant_get_nth_page (assistant, page_number);
  gtk_assistant_set_page_complete (assistant, current_page, complete);
}

static void
update_combo_box (GtkComboBox         *combo_box,
                  const gchar * const *options,
                  const gchar         *default_value)
{
  int i;
  GtkTreeModel *model;
  int selected;

  g_return_if_fail (options != NULL);

  model = gtk_combo_box_get_model (combo_box);
  gtk_list_store_clear (GTK_LIST_STORE (model));

  selected = 0;
  for (i = 0; options[i]; i++) {
    if (default_value && strcmp (options[i], default_value) == 0)
      selected = i;

    gtk_combo_box_append_text (combo_box, options[i]);
  }

  gtk_combo_box_set_active(combo_box, selected);
}


/****************
 * Welcome page *
 ****************/

static void
create_welcome_page (EkigaAssistant *assistant)
{
  GtkWidget *label;

  label = gtk_label_new (_("This is the Ekiga general configuration assistant. "
                           "The following steps will set up Ekiga by asking "
                           "a few simple questions.\n\nOnce you have completed "
                           "these steps, you can always change them later by "
                           "selecting Preferences in the Edit menu."));
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_widget_show (label);
  gtk_assistant_append_page (GTK_ASSISTANT (assistant), label);
  gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), label, _("Welcome in Ekiga"));
  gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), label, GTK_ASSISTANT_PAGE_INTRO);
  gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), label, assistant->priv->icon);
  gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), label, TRUE);

  assistant->priv->welcome_page = label;
}

/**********************
 * Personal data page *
 **********************/

static void
name_changed_cb (GtkEntry     *entry,
                 GtkAssistant *assistant)
{
  gchar **couple;
  gboolean complete;

  couple = g_strsplit (gtk_entry_get_text (entry), " ", 2);

  complete = couple && couple[0] && couple[1]
          && !PString (couple[0]).Trim ().IsEmpty ()
          && !PString (couple[1]).Trim ().IsEmpty ();
  set_current_page_complete (assistant, complete);

  g_strfreev (couple);
}


static void
create_personal_data_page (EkigaAssistant *assistant)
{
  GtkWidget *vbox;
  GtkWidget *label;
  gchar *text;

  vbox = create_page (assistant, _("Personal Information"), GTK_ASSISTANT_PAGE_CONTENT);

  /* The user fields */
  label = gtk_label_new (_("Please enter your first name and your surname:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  assistant->priv->name = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (assistant->priv->name), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), assistant->priv->name, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("Your first name and surname will be "
                          "used when connecting to other VoIP and "
                          "videoconferencing software."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  g_signal_connect (G_OBJECT (assistant->priv->name), "changed",
                    G_CALLBACK (name_changed_cb), assistant);

  assistant->priv->personal_data_page = vbox;
  gtk_widget_show_all (vbox);
}

static void
prepare_personal_data_page (EkigaAssistant *assistant)
{
  gchar *full_name;

  full_name = gm_conf_get_string (PERSONAL_DATA_KEY "full_name");

  if (full_name)
    gtk_entry_set_text (GTK_ENTRY (assistant->priv->name), full_name);

  g_free (full_name);
}

static void
apply_personal_data_page (EkigaAssistant *assistant)
{
  GtkEntry *entry = GTK_ENTRY (assistant->priv->name);
  const gchar *full_name = gtk_entry_get_text (entry);

  if (full_name)
    gm_conf_set_string (PERSONAL_DATA_KEY "full_name", full_name);
}

/******************
 * Ekiga.net page *
 ******************/

static void
ekiga_net_button_clicked_cb (G_GNUC_UNUSED GtkWidget *button,
                             G_GNUC_UNUSED gpointer data)
{
  gm_open_uri ("http://www.ekiga.net");
}

static void
ekiga_net_info_changed_cb (G_GNUC_UNUSED GtkWidget *w,
                           EkigaAssistant *assistant)
{
  gboolean complete;

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (assistant->priv->skip_ekiga_net)))
    complete = TRUE;
  else {
    const char *username = gtk_entry_get_text (GTK_ENTRY (assistant->priv->username));
    const char *password = gtk_entry_get_text (GTK_ENTRY (assistant->priv->password));
    complete = strcmp(username, "") != 0 && strcmp(password, "") != 0;
  }

  set_current_page_complete (GTK_ASSISTANT (assistant), complete);
}

static void
create_ekiga_net_page (EkigaAssistant *assistant)
{
  GtkWidget *vbox;
  GtkWidget *label;
  gchar *text;
  GtkWidget *button;
  GtkWidget *align;

  vbox = create_page (assistant, _("ekiga.net Account"), GTK_ASSISTANT_PAGE_CONTENT);

  label = gtk_label_new (_("Please enter your username:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  assistant->priv->username = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (assistant->priv->username), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), assistant->priv->username, FALSE, FALSE, 0);

  label = gtk_label_new (_("Please enter your password:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  assistant->priv->password = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (assistant->priv->password), TRUE);
  gtk_entry_set_visibility (GTK_ENTRY (assistant->priv->password), FALSE);
  gtk_box_pack_start (GTK_BOX (vbox), assistant->priv->password, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The username and password are used "
                          "to login to your existing account at the ekiga.net "
                          "free SIP service. If you do not have an ekiga.net "
                          "SIP address yet, you may first create an account "
                          "below. This will provide a SIP address that allows "
                          "people to call you.\n\nYou may skip this step if "
                          "you use an alternative SIP service, or if you "
                          "would prefer to specify the login details later."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  button = gtk_button_new ();
  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<span foreground=\"blue\"><u>%s</u></span>",
                          _("Get an ekiga.net SIP account"));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (button), label);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (button), FALSE, FALSE, 10);
  g_signal_connect (button, "clicked",
                    G_CALLBACK (ekiga_net_button_clicked_cb), NULL);

  assistant->priv->skip_ekiga_net = gtk_check_button_new ();
  label = gtk_label_new (_("I do not want to sign up for the ekiga.net free service"));
  gtk_container_add (GTK_CONTAINER (assistant->priv->skip_ekiga_net), label);
  align = gtk_alignment_new (0, 1.0, 0, 0);
  gtk_container_add (GTK_CONTAINER (align), assistant->priv->skip_ekiga_net);
  gtk_box_pack_start (GTK_BOX (vbox), align, TRUE, TRUE, 0);

  g_signal_connect (assistant->priv->username, "changed",
                    G_CALLBACK (ekiga_net_info_changed_cb), assistant);
  g_signal_connect (assistant->priv->password, "changed",
                    G_CALLBACK (ekiga_net_info_changed_cb), assistant);
  g_signal_connect (assistant->priv->skip_ekiga_net, "toggled",
                    G_CALLBACK (ekiga_net_info_changed_cb), assistant);

  assistant->priv->ekiga_net_page = vbox;
  gtk_widget_show_all (vbox);
}

static void
prepare_ekiga_net_page (EkigaAssistant *assistant)
{
  GmAccount *account = gnomemeeting_get_account ("ekiga.net");

  if (account && account->username)
    gtk_entry_set_text (GTK_ENTRY (assistant->priv->username), account->username);
  if (account && account->password)
    gtk_entry_set_text (GTK_ENTRY (assistant->priv->password), account->password);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (assistant->priv->skip_ekiga_net),
                                FALSE);

  set_current_page_complete (GTK_ASSISTANT (assistant),
                             account && account->username && account->password);
}

static void
apply_ekiga_net_page (EkigaAssistant *assistant)
{
  GmAccount *account = gnomemeeting_get_account ("ekiga.net");
  GMManager *manager;
  gboolean new_account = FALSE;

  if (account == NULL) {
    account = gm_account_new ();
    account->default_account = TRUE;
    account->account_name = g_strdup ("ekiga.net SIP Service");
    account->host = g_strdup ("ekiga.net");
    account->domain = g_strdup ("ekiga.net");
    account->protocol_name = g_strdup ("SIP");

    new_account = TRUE;
  }

  if (account->auth_username)
    g_free (account->auth_username);
  if (account->username)
    g_free (account->username);
  if (account->password)
    g_free (account->password);

  account->username =
    g_strdup (gtk_entry_get_text (GTK_ENTRY (assistant->priv->username)));
  account->auth_username =
    g_strdup (gtk_entry_get_text (GTK_ENTRY (assistant->priv->username)));
  account->password =
    g_strdup (gtk_entry_get_text (GTK_ENTRY (assistant->priv->password)));
  account->enabled =
    !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (assistant->priv->skip_ekiga_net));

  /* If creating a new account, add it only if the user wants to use GM.NET,
   * and make it the default account */
  if (new_account) {
    if (account->enabled)
      gnomemeeting_account_add (account);
  }
  else {
    /* Modify the account, do not set it as default */
    gnomemeeting_account_modify (account);
  }

  /* Register the current Endpoint to GnomeMeeting.NET */
  gdk_threads_leave ();
  if (account->enabled) {
    manager = dynamic_cast<GMManager *> (assistant->priv->core->get ("opal-component"));
    manager->Register (account);
  }
  gdk_threads_enter ();

  gm_account_delete (account);
}

/************************
 * Connection type page *
 ************************/

static void
create_connection_type_page (EkigaAssistant *assistant)
{
  GtkWidget *vbox;
  GtkWidget *label;
  gchar *text;

  GtkListStore *store;
  GtkCellRenderer *cell;
  GtkTreeIter iter;

  vbox = create_page (assistant, _("Connection Type"), GTK_ASSISTANT_PAGE_CONTENT);

  /* The connection type */
  label = gtk_label_new (_("Please choose your connection type:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
  assistant->priv->connection_type = gtk_combo_box_new_with_model (GTK_TREE_MODEL (store));
  g_object_unref (store);
  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (assistant->priv->connection_type), cell, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (assistant->priv->connection_type), cell,
                                  "text", CNX_LABEL_COLUMN,
                                  NULL);
  gtk_box_pack_start (GTK_BOX (vbox), assistant->priv->connection_type, FALSE, FALSE, 0);

  /* Fill the model with available connection types */
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      CNX_LABEL_COLUMN, _("56k Modem"),
                      CNX_CODE_COLUMN, NET_PSTN,
                      -1);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      CNX_LABEL_COLUMN, _("ISDN"),
                      CNX_CODE_COLUMN, NET_ISDN,
                      -1);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      CNX_LABEL_COLUMN, _("xDSL/Cable"),
                      CNX_CODE_COLUMN, NET_DSL,
                      -1);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      CNX_LABEL_COLUMN, _("T1/LAN"),
                      CNX_CODE_COLUMN, NET_LAN,
                      -1);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      CNX_LABEL_COLUMN, _("Keep current settings"),
                      CNX_CODE_COLUMN, NET_CUSTOM,
                      -1);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The connection type will permit "
                          "determining the best quality settings that Ekiga "
                          "will use during calls. You can later change the "
                          "settings individually in the preferences window."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  assistant->priv->connection_type_page = vbox;
  gtk_widget_show_all (vbox);
  gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), vbox, TRUE);
}

static void
prepare_connection_type_page (EkigaAssistant *assistant)
{
  GtkComboBox *combo_box = GTK_COMBO_BOX (assistant->priv->connection_type);
  GtkTreeModel *model = gtk_combo_box_get_model (combo_box);
  GtkTreeIter iter;
  gint connection_type = gm_conf_get_int (GENERAL_KEY "kind_of_net");

  if (gtk_tree_model_get_iter_first (model, &iter)) {
    do {
      gint code;
      gtk_tree_model_get (model, &iter, CNX_CODE_COLUMN, &code, -1);
      if (code == connection_type) {
        gtk_combo_box_set_active_iter (combo_box, &iter);
        break;
      }
    } while (gtk_tree_model_iter_next (model, &iter));
  }
}

static void
apply_connection_type_page (EkigaAssistant *assistant)
{
  GtkComboBox *combo_box = GTK_COMBO_BOX (assistant->priv->connection_type);
  GtkTreeModel *model = gtk_combo_box_get_model (combo_box);
  GtkTreeIter iter;
  gint connection_type = NET_CUSTOM;

  if (gtk_combo_box_get_active_iter (combo_box, &iter))
    gtk_tree_model_get (model, &iter, CNX_CODE_COLUMN, &connection_type, -1);

  /* Set the connection quality settings */
  switch (connection_type) {
    case NET_PSTN:
      gm_conf_set_int (VIDEO_CODECS_KEY "frame_rate", 5);
      gm_conf_set_int (VIDEO_CODECS_KEY "maximum_video_tx_bitrate", 8);
      gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video", FALSE);
      break;

    case NET_ISDN:
      gm_conf_set_int (VIDEO_CODECS_KEY "frame_rate", 10);
      gm_conf_set_int (VIDEO_CODECS_KEY "maximum_video_tx_bitrate", 16);
      gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video", FALSE);
      break;

    case NET_DSL:
      gm_conf_set_int (VIDEO_CODECS_KEY "frame_rate", 15);
      gm_conf_set_int (VIDEO_CODECS_KEY "maximum_video_tx_bitrate", 64);
      gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video", TRUE);
      break;

    case NET_LAN:
      gm_conf_set_int (VIDEO_CODECS_KEY "frame_rate", 25);
      gm_conf_set_int (VIDEO_CODECS_KEY "maximum_video_tx_bitrate", 800);
      gm_conf_set_bool (VIDEO_CODECS_KEY "enable_video", TRUE);
      break;

    case NET_CUSTOM:
    default:
      break; /* don't touch anything */
  }

  gm_conf_set_int (GENERAL_KEY "kind_of_net", connection_type);
}

/**********************
 * Audio manager page *
 **********************/

static void
create_audio_manager_page (EkigaAssistant *assistant)
{
  GtkWidget *vbox;
  GtkWidget *label;
  gchar *text;

  vbox = create_page (assistant, _("Audio Manager"), GTK_ASSISTANT_PAGE_CONTENT);

  label = gtk_label_new (_("Please choose your audio manager:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  assistant->priv->audio_manager = gtk_combo_box_new_text ();
  gtk_box_pack_start (GTK_BOX (vbox), assistant->priv->audio_manager, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
#ifdef WIN32
  text = g_strdup_printf ("<i>%s</i>", _("The audio manager is the plugin that "
                          "will manage your audio devices. WindowsMultimedia "
                          "is probably the best choice when available."));
#else
  text = g_strdup_printf ("<i>%s</i>", _("The audio manager is the plugin that "
                          "will manage your audio devices. ALSA is probably "
                          "the best choice when available."));
#endif
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  assistant->priv->audio_manager_page = vbox;
  gtk_widget_show_all (vbox);
  gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), vbox, TRUE);
}

static void
prepare_audio_manager_page (EkigaAssistant *assistant)
{
  char **array;
  gchar *audio_manager;

  array = GnomeMeeting::Process ()->GetAudioPlugins ().ToCharArray ();
  audio_manager = gm_conf_get_string (AUDIO_DEVICES_KEY "plugin");

  update_combo_box (GTK_COMBO_BOX (assistant->priv->audio_manager),
                    array, audio_manager);
  free (array);
}

static void
apply_audio_manager_page (EkigaAssistant *assistant)
{
  GtkComboBox *combo_box;
  gchar *audio_manager;

  combo_box = GTK_COMBO_BOX (assistant->priv->audio_manager);
  audio_manager = gtk_combo_box_get_active_text (combo_box);
  if (audio_manager) {
    gm_conf_set_string (AUDIO_DEVICES_KEY "plugin", audio_manager);
    g_free (audio_manager);
  }
}

/**********************
 * Audio devices page *
 **********************/

static void
create_audio_devices_page (EkigaAssistant *assistant)
{
  GtkWidget *vbox;
  GtkWidget *label;
  gchar *text;

  vbox = create_page (assistant, _("Audio Devices"), GTK_ASSISTANT_PAGE_CONTENT);

  label = gtk_label_new (_("Please choose the audio output device:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  assistant->priv->audio_player = gtk_combo_box_new_text ();
  gtk_box_pack_start (GTK_BOX (vbox), assistant->priv->audio_player, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The audio output device is the device "
                          " managed by the audio manager that will be used to "
                          "play audio."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  label = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  label = gtk_label_new (_("Please choose the audio input device:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  assistant->priv->audio_recorder = gtk_combo_box_new_text ();
  gtk_box_pack_start (GTK_BOX (vbox), assistant->priv->audio_recorder, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The audio input device is the device "
                          "managed by the audio manager that will be used to "
                          "record your voice."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  label = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  assistant->priv->audio_devices_page = vbox;
  gtk_widget_show_all (vbox);
  gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), vbox, TRUE);
}

static void
prepare_audio_devices_page (EkigaAssistant *assistant)
{
  GMManager *manager;
  gchar *audio_manager;
  gchar *player;
  gchar *recorder;
  PStringArray devices;
  char **array;

  manager = dynamic_cast<GMManager *> (assistant->priv->core->get ("opal-component"));

  audio_manager = gtk_combo_box_get_active_text (GTK_COMBO_BOX (assistant->priv->audio_manager));

  player = gm_conf_get_string (AUDIO_DEVICES_KEY "output_device");
  recorder = gm_conf_get_string (AUDIO_DEVICES_KEY "input_device");

  /* FIXME: We should use DetectDevices, however DetectDevices
   * works only for the currently selected audio and video plugins,
   * not for a random one.
   */
  gnomemeeting_sound_daemons_suspend ();

  devices = PSoundChannel::GetDeviceNames (audio_manager, PSoundChannel::Player);
  if (devices.GetSize () == 0)
    devices += PString (_("No device found"));

  array = devices.ToCharArray ();
  update_combo_box (GTK_COMBO_BOX (assistant->priv->audio_player), array, player);
  free (array);

  devices = PSoundChannel::GetDeviceNames (audio_manager, PSoundChannel::Recorder);
  if (devices.GetSize () == 0)
    devices += PString (_("No device found"));

  array = devices.ToCharArray ();
  update_combo_box (GTK_COMBO_BOX (assistant->priv->audio_recorder), array, player);
  free (array);

  gnomemeeting_sound_daemons_resume ();

  g_free (player);
  g_free (recorder);
}

static void
apply_audio_devices_page (EkigaAssistant *assistant)
{
  GtkComboBox *combo_box;
  gchar *device;

  combo_box = GTK_COMBO_BOX (assistant->priv->audio_player);
  device = gtk_combo_box_get_active_text (combo_box);
  if (device) {
    gm_conf_set_string (AUDIO_DEVICES_KEY "output_device", device);
    g_free (device);
  }

  combo_box = GTK_COMBO_BOX (assistant->priv->audio_recorder);
  device = gtk_combo_box_get_active_text (combo_box);
  if (device) {
    gm_conf_set_string (AUDIO_DEVICES_KEY "input_device", device);
    g_free (device);
  }

}

/**********************
 * Video manager page *
 **********************/

static void
create_video_manager_page (EkigaAssistant *assistant)
{
  GtkWidget *vbox;
  GtkWidget *label;
  gchar *text;

  vbox = create_page (assistant, _("Video manager"), GTK_ASSISTANT_PAGE_CONTENT);

  label = gtk_label_new (_("Please choose your video manager:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  assistant->priv->video_manager = gtk_combo_box_new_text ();
  gtk_box_pack_start (GTK_BOX (vbox), assistant->priv->video_manager, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The video manager is the plugin that "
                          "will manage your video devices, Video4Linux is the "
                          "most common choice if you own a webcam."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  assistant->priv->video_manager_page = vbox;
  gtk_widget_show_all (vbox);
  gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), vbox, TRUE);
}

// FIXME: duplicate to gm_prefs_window_get_video_devices_list
void 
get_video_devices_list (Ekiga::ServiceCore *core,
                                        std::vector<std::string> & plugin_list,
                                        std::vector<std::string> & device_list)
{
  Ekiga::VidInputCore *vidinput_core = dynamic_cast<Ekiga::VidInputCore *> (core->get ("vidinput-core"));
  std::vector <Ekiga::VidInputDevice> vidinput_devices;
  Ekiga::VidInputDevice vidinput_device;

  vidinput_core->get_vidinput_devices(vidinput_devices);

  std::string plugin_string;
  std::string device_string;

  gchar *current_plugin = NULL;
  current_plugin = gm_conf_get_string (VIDEO_DEVICES_KEY "plugin");

  for (std::vector<Ekiga::VidInputDevice>::iterator iter = vidinput_devices.begin ();
       iter != vidinput_devices.end ();
       iter++) {

    vidinput_device = (*iter);
    plugin_string = vidinput_device.type + "/" + vidinput_device.source;
    plugin_list.push_back(plugin_string);

    if (current_plugin && plugin_string == current_plugin) {
      device_string = vidinput_device.device;
      device_list.push_back(device_string);
    }
  }

  if (device_list.size() == 0) {
    device_string = _("No device found");
      device_list.push_back(device_string);
  }

  g_free (current_plugin);

}
// FIXME: duplicate to gm_prefs_window_convert_string_list
gchar**
convert_string_list (const std::vector<std::string> & list)
{
  gchar **array = NULL;
  unsigned i;

  array = (gchar**) malloc (sizeof(gchar*) * (list.size() + 1));
  for (i = 0; i < list.size(); i++)
    array[i] = (gchar*) list[i].c_str();
  array[i] = NULL;

  return array;
}

static void
prepare_video_manager_page (EkigaAssistant *assistant)
{
  std::vector <std::string> plugin_list;
  std::vector <std::string> device_list;
  gchar** array;
  gchar* current_plugin;

  get_video_devices_list (assistant->priv->core, plugin_list, device_list);

  array = convert_string_list (plugin_list);

  current_plugin = gm_conf_get_string (VIDEO_DEVICES_KEY "plugin");

  update_combo_box (GTK_COMBO_BOX (assistant->priv->video_manager),
                    array, current_plugin);

  g_free (array);
}

static void
apply_video_manager_page (EkigaAssistant *assistant)
{
  GtkComboBox *combo_box;
  gchar *video_manager;

  combo_box = GTK_COMBO_BOX (assistant->priv->video_manager);
  video_manager = gtk_combo_box_get_active_text (combo_box);
  if (video_manager) {
    gm_conf_set_string (VIDEO_DEVICES_KEY "plugin", video_manager);
    g_free (video_manager);
  }
}

/****************
 * Summary page *
 ****************/

static void
create_summary_page (EkigaAssistant *assistant)
{
  GtkWidget *vbox;
  GtkWidget *label;
  GtkWidget *sw;
  GtkWidget *tree;
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;

  vbox = create_page (assistant, _("Configuration Complete"), GTK_ASSISTANT_PAGE_CONFIRM);

  label = gtk_label_new (_("You have now finished the Ekiga configuration. All "
                         "the settings can be changed in the Ekiga preferences. "
                         "Enjoy!"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  label = gtk_label_new (_("Configuration summary:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  GtkWidget *align = gtk_alignment_new (0.5, 0.0, 0.0, 1.0);
  gtk_box_pack_start (GTK_BOX (vbox), align, TRUE, TRUE, 0);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (align), sw);

  assistant->priv->summary_model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);

  tree = gtk_tree_view_new ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (tree),
                           GTK_TREE_MODEL (assistant->priv->summary_model));
  gtk_container_add (GTK_CONTAINER (sw), tree);

  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Option", cell,
                                                     "text", SUMMARY_KEY_COLUMN,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Value", cell,
                                                     "text", SUMMARY_VALUE_COLUMN,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  assistant->priv->summary_page = vbox;
  gtk_widget_show_all (vbox);
}

static void prepare_summary_page (EkigaAssistant *assistant)
{
  GtkListStore *model = assistant->priv->summary_model;
  GtkTreeIter iter;
  GtkTreeIter citer;
  gchar *value;

  gtk_list_store_clear (model);

  /* The full name */
  gtk_list_store_append (model, &iter);
  gtk_list_store_set (model, &iter,
                      SUMMARY_KEY_COLUMN, "Full name",
                      SUMMARY_VALUE_COLUMN, gtk_entry_get_text (GTK_ENTRY (assistant->priv->name)),
                      -1);

  /* The user name */
  gtk_list_store_append (model, &iter);
  gtk_list_store_set (model, &iter,
                      SUMMARY_KEY_COLUMN, "Username",
                      SUMMARY_VALUE_COLUMN, gtk_entry_get_text (GTK_ENTRY (assistant->priv->username)),
                      -1);

  /* The connection type */
  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (assistant->priv->connection_type), &citer)) {
    GtkTreeModel *cmodel = gtk_combo_box_get_model (GTK_COMBO_BOX (assistant->priv->connection_type));
    gtk_tree_model_get (cmodel, &citer, CNX_LABEL_COLUMN, &value, -1);

    gtk_list_store_append (model, &iter);
    gtk_list_store_set (model, &iter,
                        SUMMARY_KEY_COLUMN, "Connection type",
                        SUMMARY_VALUE_COLUMN, value,
                        -1);
    g_free (value);
  }

  /* The audio manager */
  gtk_list_store_append (model, &iter);
  value = gtk_combo_box_get_active_text (GTK_COMBO_BOX (assistant->priv->audio_manager));
  gtk_list_store_set (model, &iter,
                      SUMMARY_KEY_COLUMN, "Audio manager",
                      SUMMARY_VALUE_COLUMN, value,
                      -1);
  g_free (value);

  /* The audio playing device */
  gtk_list_store_append (model, &iter);
  value = gtk_combo_box_get_active_text (GTK_COMBO_BOX (assistant->priv->audio_player));
  gtk_list_store_set (model, &iter,
                      SUMMARY_KEY_COLUMN, "Audio player",
                      SUMMARY_VALUE_COLUMN, value,
                      -1);
  g_free (value);

  /* The audio recording device */
  gtk_list_store_append (model, &iter);
  value = gtk_combo_box_get_active_text (GTK_COMBO_BOX (assistant->priv->audio_recorder));
  gtk_list_store_set (model, &iter,
                      SUMMARY_KEY_COLUMN, "Audio recorder",
                      SUMMARY_VALUE_COLUMN, value,
                      -1);
  g_free (value);

  /* The video manager */
  gtk_list_store_append (model, &iter);
  value = gtk_combo_box_get_active_text (GTK_COMBO_BOX (assistant->priv->video_manager));
  gtk_list_store_set (model, &iter,
                      SUMMARY_KEY_COLUMN, "Video manager",
                      SUMMARY_VALUE_COLUMN, value,
                      -1);
  g_free (value);

  /* The ekiga.org account */
  gtk_list_store_append (model, &iter);
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (assistant->priv->skip_ekiga_net))) {
    value = g_strdup_printf ("sip:%s@ekiga.net",
                             gtk_entry_get_text (GTK_ENTRY (assistant->priv->username)));
    gtk_list_store_set (model, &iter,
                        SUMMARY_KEY_COLUMN, "SIP URI",
                        SUMMARY_VALUE_COLUMN, value,
                        -1);
    g_free (value);
  }
  else {
    gtk_list_store_set (model, &iter,
                        SUMMARY_KEY_COLUMN, "SIP URI",
                        SUMMARY_VALUE_COLUMN, "None",
                        -1);
  }
}

/****************
 * Window stuff *
 ****************/

static void
ekiga_assistant_init (EkigaAssistant *assistant)
{
  assistant->priv = G_TYPE_INSTANCE_GET_PRIVATE (assistant, EKIGA_TYPE_ASSISTANT,
                                                 EkigaAssistantPrivate);

  gtk_window_set_default_size (GTK_WINDOW (assistant), 500, 300);
  gtk_window_set_position (GTK_WINDOW (assistant), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width (GTK_CONTAINER (assistant), 12);

  assistant->priv->icon = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
                                                    "ekiga", 48,
                                                    (GtkIconLookupFlags) 0, NULL);

  create_welcome_page (assistant);
  create_personal_data_page (assistant);
  create_ekiga_net_page (assistant);
  create_connection_type_page (assistant);
  create_audio_manager_page (assistant);
  create_audio_devices_page (assistant);
  create_video_manager_page (assistant);
  create_summary_page (assistant);

  /* FIXME: what the hell is it needed for? */
  g_object_set_data (G_OBJECT (assistant), "window_name", (gpointer) "assistant");
}

static void
ekiga_assistant_prepare (GtkAssistant *gtkassistant,
                         GtkWidget    *page)
{
  EkigaAssistant *assistant = EKIGA_ASSISTANT (gtkassistant);
  gchar *title;

  title = g_strdup_printf (_("Ekiga Configuration Assistant (%d of %d)"),
                           gtk_assistant_get_current_page (gtkassistant) + 1,
                           gtk_assistant_get_n_pages (gtkassistant));

  gtk_window_set_title (GTK_WINDOW (assistant), title);
  g_free (title);

  if (page == assistant->priv->personal_data_page) {
    prepare_personal_data_page (assistant);
    return;
  }

  if (page == assistant->priv->ekiga_net_page) {
    prepare_ekiga_net_page (assistant);
    return;
  }

  if (page == assistant->priv->connection_type_page) {
    prepare_connection_type_page (assistant);
    return;
  }

  if (page == assistant->priv->audio_manager_page) {
    prepare_audio_manager_page (assistant);
    return;
  }

  if (page == assistant->priv->audio_devices_page) {
    prepare_audio_devices_page (assistant);
    return;
  }

  if (page == assistant->priv->video_manager_page) {
    prepare_video_manager_page (assistant);
    return;
  }

  if (page == assistant->priv->summary_page) {
    prepare_summary_page (assistant);
    return;
  }
}

static void
ekiga_assistant_apply (GtkAssistant *gtkassistant)
{
  EkigaAssistant *assistant = EKIGA_ASSISTANT (gtkassistant);

  GMManager *manager;

  GtkWidget *main_window;

  const int schema_version = MAJOR_VERSION * 1000
                           + MINOR_VERSION * 10
                           + BUILD_NUMBER;

  apply_personal_data_page (assistant);
  apply_ekiga_net_page (assistant);
  apply_connection_type_page (assistant);
  apply_audio_manager_page (assistant);
  apply_audio_devices_page (assistant);
  apply_video_manager_page (assistant);

  manager = dynamic_cast<GMManager *> (assistant->priv->core->get ("opal-component"));
  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  /* Hide the druid and show the main Ekiga window */
  gtk_widget_hide (GTK_WIDGET (assistant));
  gtk_assistant_set_current_page (gtkassistant, 0);
  gnomemeeting_window_show (main_window);

  /* Will be done through the config if the manager changes, but not
     if the manager doesn't change */
  gdk_threads_leave ();
  GnomeMeeting::Process ()->DetectDevices ();
  gdk_threads_enter ();

  /* Update the version number */
  gm_conf_set_int (GENERAL_KEY "version", schema_version);
}

static void
ekiga_assistant_cancel (GtkAssistant *gtkassistant)
{
  GtkWidget *main_window;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  gtk_widget_hide (GTK_WIDGET (gtkassistant));
  gtk_assistant_set_current_page (gtkassistant, 0);
  gnomemeeting_window_show (main_window);
}

static void
ekiga_assistant_finalize (GObject *object)
{
  EkigaAssistant *assistant = EKIGA_ASSISTANT (object);

  g_object_unref (assistant->priv->icon);

  G_OBJECT_CLASS (ekiga_assistant_parent_class)->finalize (object);
}

static void
ekiga_assistant_class_init (EkigaAssistantClass *klass)
{
  GtkAssistantClass *assistant_class = GTK_ASSISTANT_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  assistant_class->prepare = ekiga_assistant_prepare;
  assistant_class->apply = ekiga_assistant_apply;
  assistant_class->cancel = ekiga_assistant_cancel;

  object_class->finalize = ekiga_assistant_finalize;

  g_type_class_add_private (klass, sizeof (EkigaAssistantPrivate));
}

GtkWidget *
ekiga_assistant_new (Ekiga::ServiceCore *core)
{
  EkigaAssistant *assistant;

  assistant = EKIGA_ASSISTANT (g_object_new (EKIGA_TYPE_ASSISTANT, NULL));
  assistant->priv->core = core;

  /* FIXME: move this into the caller */
  g_signal_connect (assistant, "cancel",
                    G_CALLBACK (gtk_widget_hide), NULL);

  return GTK_WIDGET (assistant);
}

/* ex:set ts=2 sw=2 et: */
