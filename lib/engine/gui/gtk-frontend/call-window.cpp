/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2012 Damien Sandras <dsandras@seconix.com>
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
 *                         call_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Wed Dec 28 2012
 *   copyright            : (C) 2000-2012 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          build the call window.
 */

#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>

#include <boost/smart_ptr.hpp>

#include "config.h"

#include "ekiga-settings.h"

#include "call-window.h"

#include "dialpad.h"

#include "gmvideowidget.h"
#include "gm-info-bar.h"
#include "gactor-menu.h"
#include "trigger.h"
#include "scoped-connections.h"
#include "menu-builder-tools.h"
#include "menu-builder-gtk.h"
#include "form-dialog-gtk.h"
#include "ext-window.h"

#ifndef WIN32
#include <signal.h>
#include <gdk/gdkx.h>
#else
#include "platform/winpaths.h"
#include <gdk/gdkwin32.h>
#include <cstdio>
#endif

#if defined(P_FREEBSD) || defined (P_MACOSX)
#include <libintl.h>
#endif

#include "engine.h"
#include "call-core.h"
#include "videoinput-core.h"
#include "audioinput-core.h"
#include "audiooutput-core.h"
#include "videooutput-manager.h"
#include "foe-list.h"
#include "rtcp-statistics.h"

#define STAGE_WIDTH 640
#define STAGE_HEIGHT 480

enum CallingState {Standby, Calling, Ringing, Connected, Called};

enum DeviceType {AudioInput, AudioOutput, Ringer, VideoInput};
struct deviceStruct {
  char name[256];
  DeviceType deviceType;
};

G_DEFINE_TYPE (EkigaCallWindow, ekiga_call_window, GM_TYPE_WINDOW);

enum {

  COLOR,
  CONTRAST,
  BRIGHTNESS,
  WHITENESS,
  SPEAKER_VOLUME,
  MIC_VOLUME,
  MAX_SETTINGS,
};

struct _EkigaCallWindowPrivate
{
  boost::shared_ptr<Ekiga::VideoInputCore> videoinput_core;
  boost::shared_ptr<Ekiga::VideoOutputCore> videooutput_core;
  boost::shared_ptr<Ekiga::AudioInputCore> audioinput_core;
  boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core;
  boost::shared_ptr<Ekiga::CallCore> call_core;
  boost::shared_ptr<Ekiga::FriendOrFoe> friend_or_foe;
  boost::shared_ptr<Ekiga::FoeList> foe_list;

  GtkAccelGroup *accel;

  boost::shared_ptr<Ekiga::Call> current_call;
  unsigned calling_state;

  GtkWidget *ext_video_win;
  GtkWidget *event_box;
  GtkWidget *spinner;
  GtkBuilder *builder;

  GtkWidget *video_widget;
  bool fullscreen;
  bool dead;
  bool bad_connection;

  GtkWidget *call_panel_toolbar;
  GtkWidget *blacklist_button;

  GtkWidget *audio_settings_window;
  GtkWidget *audio_input_volume_frame;
  GtkWidget *audio_output_volume_frame;
  GtkWidget *input_signal;
  GtkWidget *output_signal;
#if GTK_CHECK_VERSION (3, 0, 0)
  GtkAdjustment *adj_input_volume;
  GtkAdjustment *adj_output_volume;
#else
  GtkObject *adj_input_volume;
  GtkObject *adj_output_volume;
#endif
#ifndef WIN32
  GC gc;
#endif
  GtkWidget *settings_button;

  unsigned int timeout_id;

  GtkWidget *info_bar;

  /* Audio and video settings */
  int settings[MAX_SETTINGS];
  GtkWidget *settings_range[MAX_SETTINGS];

  std::string transmitted_video_codec;
  std::string transmitted_audio_codec;
  std::string received_video_codec;
  std::string received_audio_codec;

  Ekiga::GActorMenuPtr menu;

  Ekiga::scoped_connections connections;
  boost::shared_ptr<Ekiga::Settings> video_display_settings;
};

/* channel types */
enum {
  CHANNEL_FIRST,
  CHANNEL_AUDIO,
  CHANNEL_VIDEO,
  CHANNEL_LAST
};


static void show_extended_video_window_cb (G_GNUC_UNUSED GSimpleAction *action,
                                           G_GNUC_UNUSED GVariant *parameter,
                                           gpointer data);

static void fullscreen_changed_cb (G_GNUC_UNUSED GSimpleAction *action,
                                   G_GNUC_UNUSED GVariant *parameter,
                                   gpointer data);

static void pick_up_call_cb (GtkWidget * /*widget*/,
                             gpointer data);

static void hang_up_call_cb (GtkWidget * /*widget*/,
                             gpointer data);

static void hold_current_call_cb (GtkWidget *widget,
                                  gpointer data);

static void blacklist_cb (GtkWidget* widget,
			  gpointer data);

static void toggle_audio_stream_pause_cb (GtkWidget * /*widget*/,
                                          gpointer data);

static void toggle_video_stream_pause_cb (GtkWidget * /*widget*/,
                                          gpointer data);

static void transfer_current_call_cb (GtkWidget *widget,
                                      gpointer data);

static void show_call_devices_settings_cb (G_GNUC_UNUSED GSimpleAction *action,
                                           G_GNUC_UNUSED GVariant *parameter,
                                           gpointer data);

static void call_devices_settings_changed_cb (GtkRange *range,
                                              gpointer data);

static void on_videooutput_device_opened_cb (Ekiga::VideoOutputManager & /* manager */,
                                             Ekiga::VideoOutputManager::VideoView type,
                                             unsigned width,
                                             unsigned height,
                                             bool both_streams,
                                             bool ext_stream,
                                             gpointer self);

static void on_videooutput_device_closed_cb (Ekiga::VideoOutputManager & /* manager */,
                                             gpointer self);

static void on_videooutput_device_error_cb (Ekiga::VideoOutputManager & /* manager */,
                                            gpointer self);

static void on_size_changed_cb (Ekiga::VideoOutputManager & /* manager */,
                                Ekiga::VideoOutputManager::VideoView type,
                                unsigned width,
                                unsigned height,
                                gpointer self);

static void on_videoinput_device_opened_cb (Ekiga::VideoInputManager & /* manager */,
                                            Ekiga::VideoInputDevice & /* device */,
                                            Ekiga::VideoInputSettings & settings,
                                            gpointer self);

static void on_videoinput_device_closed_cb (Ekiga::VideoInputManager & /* manager */,
                                            Ekiga::VideoInputDevice & /*device*/,
                                            gpointer self);

static void on_videoinput_device_error_cb (Ekiga::VideoInputManager & /* manager */,
                                           Ekiga::VideoInputDevice & device,
                                           Ekiga::VideoInputErrorCodes error_code,
                                           gpointer self);


static void on_audioinput_device_opened_cb (Ekiga::AudioInputManager & /* manager */,
                                            Ekiga::AudioInputDevice & /* device */,
                                            Ekiga::AudioInputSettings & settings,
                                            gpointer self);

static void on_audioinput_device_closed_cb (Ekiga::AudioInputManager & /* manager */,
                                            Ekiga::AudioInputDevice & /*device*/,
                                            gpointer self);

static void on_audioinput_device_error_cb (Ekiga::AudioInputManager & /* manager */,
                                           Ekiga::AudioInputDevice & device,
                                           Ekiga::AudioInputErrorCodes error_code,
                                           gpointer self);

static void on_audiooutput_device_opened_cb (Ekiga::AudioOutputManager & /*manager*/,
                                             Ekiga::AudioOutputPS ps,
                                             Ekiga::AudioOutputDevice & /*device*/,
                                             Ekiga::AudioOutputSettings & settings,
                                             gpointer self);

static void on_audiooutput_device_closed_cb (Ekiga::AudioOutputManager & /*manager*/,
                                             Ekiga::AudioOutputPS ps,
                                             Ekiga::AudioOutputDevice & /*device*/,
                                             gpointer self);

static void on_audiooutput_device_error_cb (Ekiga::AudioOutputManager & /*manager */,
                                            Ekiga::AudioOutputPS ps,
                                            Ekiga::AudioOutputDevice & device,
                                            Ekiga::AudioOutputErrorCodes error_code,
                                            gpointer self);

static void on_ringing_call_cb (boost::shared_ptr<Ekiga::Call> call,
                                gpointer self);

static void on_established_call_cb (boost::shared_ptr<Ekiga::Call> call,
                                    gpointer self);

static void on_cleared_call_cb (boost::shared_ptr<Ekiga::Call> call,
                                std::string reason,
                                gpointer self);

static void on_missed_call_cb (boost::shared_ptr<Ekiga::Call> /*call*/,
                               gpointer self);

static void on_held_call_cb (boost::shared_ptr<Ekiga::Call> /*call*/,
                             gpointer self);

static void on_setup_call_cb (boost::shared_ptr<Ekiga::Call> call,
                              gpointer self);

static void on_retrieved_call_cb (boost::shared_ptr<Ekiga::Call> /*call*/,
                                  gpointer self);

static void on_stream_opened_cb (boost::shared_ptr<Ekiga::Call> /* call */,
                                 std::string name,
                                 Ekiga::Call::StreamType type,
                                 bool is_transmitting,
                                 gpointer self);

static void on_stream_closed_cb (boost::shared_ptr<Ekiga::Call> /* call */,
                                 G_GNUC_UNUSED std::string name,
                                 Ekiga::Call::StreamType type,
                                 bool is_transmitting,
                                 gpointer self);

static bool on_handle_questions (Ekiga::FormRequestPtr request,
                                 gpointer data);

static gboolean on_stats_refresh_cb (gpointer self);

static gboolean on_delayed_destroy_cb (gpointer self);

static gboolean ekiga_call_window_delete_event_cb (GtkWidget *widget,
                                                   G_GNUC_UNUSED GdkEventAny *event);

static gboolean ekiga_call_window_fullscreen_event_cb (GtkWidget *widget,
                                                       G_GNUC_UNUSED GdkEventAny *event);

/**/
static void ekiga_call_window_remove_action_entries (GActionMap *map,
                                                      const GActionEntry *entries);

static void ekiga_call_window_update_calling_state (EkigaCallWindow *self,
                                                    unsigned calling_state);

static void ekiga_call_window_update_header_bar_actions (EkigaCallWindow *self,
                                                         unsigned calling_state);

static void ekiga_call_window_clear_signal_levels (EkigaCallWindow *self);

static void ekiga_call_window_clear_stats (EkigaCallWindow *self);

static void ekiga_call_window_update_title (EkigaCallWindow *self,
                                            unsigned calling_state,
                                            const std::string & remote_party = std::string ());

static void ekiga_call_window_update_stats (EkigaCallWindow *self,
                                            const RTCPStatistics & statistics);

static void ekiga_call_window_init_menu (EkigaCallWindow *self);

static void ekiga_call_window_init_clutter (EkigaCallWindow *self);

static GtkWidget *gm_call_window_build_settings_popover (EkigaCallWindow *call_window,
                                                         GtkWidget *relative);

static void ekiga_call_window_toggle_fullscreen (EkigaCallWindow *self);

static void ekiga_call_window_connect_engine_signals (EkigaCallWindow *self);

static void ekiga_call_window_init_gui (EkigaCallWindow *self);

/**/
static const char* win_menu =
  "<?xml version='1.0'?>"
  "<interface>"
  "  <menu id='menubar'>"
  "    <section>"
  "      <item>"
  "        <attribute name='label' translatable='yes'>Transmit Video</attribute>"
  "        <attribute name='action'>win.transmit-video</attribute>"
  "      </item>"
  "    </section>"
  "    <section>"
  "      <item>"
  "        <attribute name='label' translatable='yes'>_Picture-In-Picture Mode</attribute>"
  "        <attribute name='action'>win.enable-pip</attribute>"
  "      </item>"
  "      <item>"
  "        <attribute name='label' translatable='yes'>_Extended Video</attribute>"
  "        <attribute name='action'>win.show-extended-video</attribute>"
  "      </item>"
  "    </section>"
  "  </menu>"
  "</interface>";

static GActionEntry win_entries[] =
{
    { "show-extended-video",  show_extended_video_window_cb, NULL, NULL, NULL, 0 },
    { "enable-fullscreen", fullscreen_changed_cb, NULL, NULL, NULL, 0 }
};

static GActionEntry video_settings_entries[] =
{
    { "call-devices-settings", show_call_devices_settings_cb, NULL, NULL, NULL, 0 },
};

/**/

static void
show_extended_video_window_cb (G_GNUC_UNUSED GSimpleAction *action,
                               G_GNUC_UNUSED GVariant *parameter,
                               gpointer data)
{
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (data));
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  if (self->priv->ext_video_win)
    gtk_widget_show (GTK_WIDGET (self->priv->ext_video_win));
}

static void
fullscreen_changed_cb (G_GNUC_UNUSED GSimpleAction *action,
                       G_GNUC_UNUSED GVariant *parameter,
                       gpointer data)
{
  g_return_if_fail (data);

  ekiga_call_window_toggle_fullscreen (EKIGA_CALL_WINDOW (data));
}


static void
blacklist_cb (G_GNUC_UNUSED GtkWidget *widget,
	      gpointer data)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (data);

  if (cw->priv->current_call) {

    const std::string uri = cw->priv->current_call->get_remote_uri ();
    Ekiga::FriendOrFoe::Identification id = cw->priv->friend_or_foe->decide ("call", uri);
    if (id == Ekiga::FriendOrFoe::Unknown) {

      cw->priv->foe_list->add_foe (uri);
      gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->blacklist_button), false);
    }
  }
}


static void
show_call_devices_settings_cb (G_GNUC_UNUSED GSimpleAction *action,
                               G_GNUC_UNUSED GVariant *parameter,
                               gpointer data)
{
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (data));
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  gtk_widget_show_all (gm_call_window_build_settings_popover (self,
                                                              self->priv->settings_button));
}

static void
call_devices_settings_changed_cb (G_GNUC_UNUSED GtkRange *range,
                                  gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  for (int i = 0 ; i < MAX_SETTINGS ; i++) {
    if (self->priv->settings_range[i]) {
      self->priv->settings[i] = gtk_range_get_value (GTK_RANGE (self->priv->settings_range[i]));
    }
  }

  if (self->priv->settings[WHITENESS] != -1)
    self->priv->videoinput_core->set_whiteness (self->priv->settings[WHITENESS]);
  if (self->priv->settings[BRIGHTNESS] != -1)
    self->priv->videoinput_core->set_brightness (self->priv->settings[BRIGHTNESS]);
  if (self->priv->settings[COLOR] != -1)
    self->priv->videoinput_core->set_colour (self->priv->settings[COLOR]);
  if (self->priv->settings[CONTRAST] != -1)
    self->priv->videoinput_core->set_contrast (self->priv->settings[CONTRAST]);
  if (self->priv->settings[SPEAKER_VOLUME] != -1)
    self->priv->audiooutput_core->set_volume (Ekiga::primary, self->priv->settings[SPEAKER_VOLUME]);
  if (self->priv->settings[MIC_VOLUME] != -1)
    self->priv->audioinput_core->set_volume (self->priv->settings[MIC_VOLUME]);
}

static void
on_videooutput_device_opened_cb (Ekiga::VideoOutputManager & /* manager */,
                                 Ekiga::VideoOutputManager::VideoView type,
                                 unsigned width,
                                 unsigned height,
                                 G_GNUC_UNUSED bool both_streams,
                                 G_GNUC_UNUSED bool ext_stream,
                                 gpointer data)
{
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (data));

  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);
  GtkWidget *video_widget = NULL;

  if (type == Ekiga::VideoOutputManager::EXTENDED) {

    if (!self->priv->ext_video_win) {
      self->priv->ext_video_win =
        gm_window_new_with_key (USER_INTERFACE ".video-settings-window");
      video_widget = gm_video_widget_new ();
      gtk_widget_set_size_request (video_widget, STAGE_WIDTH, STAGE_HEIGHT);
      gtk_widget_show (video_widget);
      gtk_container_add (GTK_CONTAINER (self->priv->ext_video_win), video_widget);

      self->priv->videooutput_core->set_ext_display_info (gm_video_widget_get_stream (GM_VIDEO_WIDGET (video_widget),
                                                                                    PRIMARY_STREAM));
    }
    gtk_widget_show (GTK_WIDGET (self->priv->ext_video_win));
    gm_video_widget_set_stream_natural_size (GM_VIDEO_WIDGET (video_widget),
                                             PRIMARY_STREAM, width, height);
    gm_video_widget_set_stream_state (GM_VIDEO_WIDGET (video_widget),
                                      PRIMARY_STREAM, STREAM_STATE_PLAYING);
  }
  else {
    GM_STREAM_TYPE t =
      (type == Ekiga::VideoOutputManager::REMOTE) ? PRIMARY_STREAM : SECONDARY_STREAM;

    gtk_widget_show (GTK_WIDGET (self));
    gm_video_widget_set_stream_natural_size (GM_VIDEO_WIDGET (self->priv->video_widget),
                                             t, width, height);
    gm_video_widget_set_stream_state (GM_VIDEO_WIDGET (self->priv->video_widget),
                                      t, STREAM_STATE_PLAYING);

  }
}

static void
on_videooutput_device_closed_cb (Ekiga::VideoOutputManager & /* manager */,
                                 gpointer data)
{
  g_return_if_fail (data);

  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);
  for (int i = 0 ; i < MAX_STREAM ; i++) {
    GM_STREAM_TYPE type = (GM_STREAM_TYPE) i;
    gm_video_widget_set_stream_state (GM_VIDEO_WIDGET (self->priv->video_widget),
                                      type, STREAM_STATE_STOPPED);
  }
  if (self->priv->ext_video_win) {
      self->priv->videooutput_core->set_ext_display_info (NULL);
    gtk_widget_destroy (self->priv->ext_video_win);
    self->priv->ext_video_win = NULL;
  }
}

static void
on_videooutput_device_error_cb (Ekiga::VideoOutputManager & /* manager */,
                                gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  gm_info_bar_push_message (GM_INFO_BAR (self->priv->info_bar),
                            GTK_MESSAGE_ERROR,
                            _("There was an error opening or initializing the video output. Please verify that no other application is using the accelerated video output."));
}


static void
on_size_changed_cb (Ekiga::VideoOutputManager & /* manager */,
                    Ekiga::VideoOutputManager::VideoView type,
                    unsigned width,
                    unsigned height,
                    gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);
  GM_STREAM_TYPE t =
    (type == Ekiga::VideoOutputManager::REMOTE) ? PRIMARY_STREAM : SECONDARY_STREAM;

  gm_video_widget_set_stream_natural_size (GM_VIDEO_WIDGET (self->priv->video_widget),
                                           t, width, height);
}

static void
on_videoinput_device_opened_cb (Ekiga::VideoInputManager & /* manager */,
                                Ekiga::VideoInputDevice & /* device */,
                                Ekiga::VideoInputSettings & settings,
                                gpointer data)
{
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (data));
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  if (settings.modifyable) {
    self->priv->settings[WHITENESS] = settings.whiteness;
    self->priv->settings[BRIGHTNESS] = settings.brightness;
    self->priv->settings[COLOR] = settings.colour;
    self->priv->settings[CONTRAST] = settings.contrast;

    g_action_map_add_action_entries (G_ACTION_MAP (g_application_get_default ()),
                                     video_settings_entries, G_N_ELEMENTS (video_settings_entries),
                                     self);
  }
}

static void
on_videoinput_device_closed_cb (Ekiga::VideoInputManager & /* manager */,
                                Ekiga::VideoInputDevice & /*device*/,
                                gpointer data)
{
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (data));
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  self->priv->settings[WHITENESS] = -1;
  self->priv->settings[BRIGHTNESS] = -1;
  self->priv->settings[COLOR] = -1;
  self->priv->settings[CONTRAST] = -1;

  ekiga_call_window_remove_action_entries (G_ACTION_MAP (g_application_get_default ()),
                                           video_settings_entries);
}

static void
on_videoinput_device_error_cb (Ekiga::VideoInputManager & /* manager */,
                               Ekiga::VideoInputDevice & device,
                               Ekiga::VideoInputErrorCodes error_code,
                               gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);
  gchar *message = NULL;

  switch (error_code) {

  case Ekiga::VI_ERROR_DEVICE:
    message = g_strdup_printf (_("There was an error while opening %s.\n\nIn case it is a pluggable device, it may be sufficient to reconnect it. If not, or if it still does not work, please check your permissions and make sure that the appropriate driver is loaded."), (const char *) device.name.c_str());
    break;

  case Ekiga::VI_ERROR_FORMAT:
    message = g_strdup_printf (_("There was an error while opening %s.\n\nYour video driver doesn't support the requested video format."), (const char *) device.name.c_str());
    break;

  case Ekiga::VI_ERROR_CHANNEL:
    message = g_strdup_printf (_("There was an error while opening %s.\n\nCould not open the chosen channel."), (const char *) device.name.c_str());
    break;

  case Ekiga::VI_ERROR_COLOUR:
    message = g_strdup_printf (_("There was an error while opening %s.\n\nYour driver doesn't seem to support any of the color formats supported by Ekiga.\n Please check your kernel driver documentation in order to determine which Palette is supported."), (const char *) device.name.c_str());
    break;

  case Ekiga::VI_ERROR_FPS:
    message = g_strdup_printf (_("There was an error while opening %s.\n\nError while setting the frame rate."), (const char *) device.name.c_str());
    break;

  case Ekiga::VI_ERROR_SCALE:
    message = g_strdup_printf (_("There was an error while opening %s.\n\nError while setting the frame size."), (const char *) device.name.c_str());
    break;

  case Ekiga::VI_ERROR_NONE:
  default:
    message = g_strdup_printf (_("There was an error while opening %s."), (const char *) device.name.c_str());
    break;
  }

  gm_info_bar_push_message (GM_INFO_BAR (self->priv->info_bar),
                            GTK_MESSAGE_ERROR,
                            message);

  g_free (message);
}


static void
on_audioinput_device_opened_cb (Ekiga::AudioInputManager & /* manager */,
                                Ekiga::AudioInputDevice & /* device */,
                                Ekiga::AudioInputSettings & settings,
                                gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);
  if (settings.modifyable)
    self->priv->settings[MIC_VOLUME] = settings.volume;
}


static void
on_audioinput_device_closed_cb (Ekiga::AudioInputManager & /* manager */,
                                Ekiga::AudioInputDevice & /*device*/,
                                gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  self->priv->settings[MIC_VOLUME] = -1;
}


static void
on_audioinput_device_error_cb (Ekiga::AudioInputManager & /* manager */,
                               Ekiga::AudioInputDevice & device,
                               Ekiga::AudioInputErrorCodes error_code,
                               gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);
  gchar *message = NULL;

  switch (error_code) {

  case Ekiga::AI_ERROR_DEVICE:
    message = g_strdup_printf (_("Unable to open %s for recording.\n\nIn case it is a pluggable device, it may be sufficient to reconnect it. If not, or if it still does not work, please check your audio setup, the permissions and that the device is not busy."), (const char *) device.name.c_str ());
    break;

  case Ekiga::AI_ERROR_READ:
    message = g_strdup_printf (_("%s was successfully opened but it is impossible to read data from this device.\n\nIn case it is a pluggable device it may be sufficient to reconnect it. If not, or if it still does not work, please check your audio setup."), (const char *) device.name.c_str ());
    break;

  case Ekiga::AI_ERROR_NONE:
  default:
    message = g_strdup_printf (_("Error while opening audio input device %s"), (const char *) device.name.c_str ());
    break;
  }

  gm_info_bar_push_message (GM_INFO_BAR (self->priv->info_bar),
                            GTK_MESSAGE_ERROR, message);
  g_free (message);
}


static void
on_audiooutput_device_opened_cb (Ekiga::AudioOutputManager & /*manager*/,
                                 Ekiga::AudioOutputPS ps,
                                 Ekiga::AudioOutputDevice & /*device*/,
                                 Ekiga::AudioOutputSettings & settings,
                                 gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  if (ps == Ekiga::secondary)
    return;

  if (settings.modifyable)
    self->priv->settings[SPEAKER_VOLUME] = settings.volume;
}


static void
on_audiooutput_device_closed_cb (Ekiga::AudioOutputManager & /*manager*/,
                                 Ekiga::AudioOutputPS ps,
                                 Ekiga::AudioOutputDevice & /*device*/,
                                 gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  if (ps == Ekiga::secondary)
    return;

  self->priv->settings[SPEAKER_VOLUME] = -1;
}


static void
on_audiooutput_device_error_cb (Ekiga::AudioOutputManager & /*manager */,
                                Ekiga::AudioOutputPS ps,
                                Ekiga::AudioOutputDevice & device,
                                Ekiga::AudioOutputErrorCodes error_code,
                                gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);
  gchar *message = NULL;

  if (ps == Ekiga::secondary)
    return;

  switch (error_code) {

  case Ekiga::AO_ERROR_DEVICE:
    message = g_strdup_printf (_("Unable to open %s for playing.\n\nIn case it is a pluggable device it may be sufficient to reconnect it. If not, or if it still does not work, please check your audio setup, the permissions and that the device is not busy."), (const char *) device.name.c_str ());
    break;

  case Ekiga::AO_ERROR_WRITE:
    message = g_strdup_printf (_("%s was successfully opened but it is impossible to write data to this device.\n\nIn case it is a pluggable device it may be sufficient to reconnect it. If not, or if it still does not work, please check your audio setup."), (const char *) device.name.c_str ());
    break;

  case Ekiga::AO_ERROR_NONE:
  default:
    message = g_strdup_printf (_("Error while opening audio output device %s"),
                               (const char *) device.name.c_str ());
    break;
  }

  gm_info_bar_push_message (GM_INFO_BAR (self->priv->info_bar),
                            GTK_MESSAGE_ERROR, message);

  g_free (message);
}


static void
on_ringing_call_cb (G_GNUC_UNUSED boost::shared_ptr<Ekiga::Call>  call,
                    gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  g_return_if_fail (self);

  ekiga_call_window_update_calling_state (self, Ringing);
  ekiga_call_window_update_header_bar_actions (self, Ringing);
}


static void
on_setup_call_cb (boost::shared_ptr<Ekiga::Call> call,
                  gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);
  boost::signals2::connection conn;

  self->priv->menu = Ekiga::GActorMenuPtr (new Ekiga::GActorMenu (*call));

  if (!call->is_outgoing ()) {
    if (self->priv->current_call)
      return; // No call setup needed if already in a call

    self->priv->current_call = call;
    ekiga_call_window_update_calling_state (self, Called);
    ekiga_call_window_update_title (self, Called, call->get_remote_party_name ());
    ekiga_call_window_update_header_bar_actions (self, Called);
  }
  else {

    self->priv->current_call = call;
    ekiga_call_window_update_calling_state (self, Calling);
    ekiga_call_window_update_title (self, Calling, call->get_remote_uri ());
    ekiga_call_window_update_header_bar_actions (self, Calling);
  }

  { // do we know about this contact already?
    const std::string uri = self->priv->current_call->get_remote_uri ();
    Ekiga::FriendOrFoe::Identification id = self->priv->friend_or_foe->decide ("call", uri);
    if (id == Ekiga::FriendOrFoe::Unknown)
      gtk_widget_set_sensitive (GTK_WIDGET (self->priv->blacklist_button), true);
    else
      gtk_widget_set_sensitive (GTK_WIDGET (self->priv->blacklist_button), false);
  }

  conn = call->questions.connect (boost::bind (&on_handle_questions, _1, (gpointer) self));
  self->priv->connections.add (conn);
}


static void
on_established_call_cb (boost::shared_ptr<Ekiga::Call> call,
                        gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  ekiga_call_window_update_calling_state (self, Connected);
  ekiga_call_window_update_title (self, Connected, call->get_remote_party_name ());
  ekiga_call_window_update_header_bar_actions (self, Connected);

  self->priv->current_call = call;

  self->priv->timeout_id = g_timeout_add_seconds (1, on_stats_refresh_cb, self);
}

static void
on_cleared_call_cb (boost::shared_ptr<Ekiga::Call> call,
                    G_GNUC_UNUSED std::string reason,
                    gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  if (self->priv->current_call && self->priv->current_call->get_id () != call->get_id ()) {
    return; // Trying to clear another call than the current active one
  }

  ekiga_call_window_update_calling_state (self, Standby);
  ekiga_call_window_update_title (self, Standby);
  ekiga_call_window_update_header_bar_actions (self, Standby);
  ekiga_call_window_clear_stats (self);

  if (self->priv->current_call) {
    self->priv->current_call = boost::shared_ptr<Ekiga::Call>();
    g_source_remove (self->priv->timeout_id);
    self->priv->timeout_id = -1;
    self->priv->bad_connection = false;
    self->priv->menu.reset ();
  }

  ekiga_call_window_clear_signal_levels (self);

  gm_info_bar_push_message (GM_INFO_BAR (self->priv->info_bar),
                            GTK_MESSAGE_INFO, reason.c_str ());
}

static void on_missed_call_cb (boost::shared_ptr<Ekiga::Call> call,
                               gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  if (self->priv->current_call && call && self->priv->current_call->get_id () != call->get_id ()) {
    return; // Trying to clear another call than the current active one
  }
  self->priv->bad_connection = false;
  self->priv->menu.reset ();

  ekiga_call_window_update_calling_state (self, Standby);
  ekiga_call_window_update_title (self, Standby);
  ekiga_call_window_update_header_bar_actions (self, Standby);
}

static void
on_held_call_cb (boost::shared_ptr<Ekiga::Call>  /*call*/,
                 gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  gm_info_bar_push_message (GM_INFO_BAR (self->priv->info_bar),
                            GTK_MESSAGE_INFO, _("Call on hold"));
}


static void
on_retrieved_call_cb (boost::shared_ptr<Ekiga::Call>  /*call*/,
                      gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  gm_info_bar_push_message (GM_INFO_BAR (self->priv->info_bar),
                            GTK_MESSAGE_INFO, _("Call retrieved"));
}


static void
set_codec (EkigaCallWindowPrivate *priv,
           std::string name,
           bool is_video,
           bool is_transmitting)
{
  if (is_video) {
    if (is_transmitting)
      priv->transmitted_video_codec = name;
    else
      priv->received_video_codec = name;
  } else {
    if (is_transmitting)
      priv->transmitted_audio_codec = name;
    else
      priv->received_audio_codec = name;
  }
}

static void
on_stream_opened_cb (boost::shared_ptr<Ekiga::Call>  /* call */,
                     std::string name,
                     Ekiga::Call::StreamType type,
                     bool is_transmitting,
                     gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);
  bool is_video = (type == Ekiga::Call::Video);

  set_codec (self->priv, name, is_video, is_transmitting);
}


static void
on_stream_closed_cb (boost::shared_ptr<Ekiga::Call>  /* call */,
                     G_GNUC_UNUSED std::string name,
                     Ekiga::Call::StreamType type,
                     bool is_transmitting,
                     gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);
  bool is_video = (type == Ekiga::Call::Video);

  set_codec (self->priv, "", is_video, is_transmitting);
}


static bool
on_handle_questions (Ekiga::FormRequestPtr request,
		     gpointer data)
{
  FormDialog dialog (request, GTK_WIDGET (data));

  dialog.run ();

  return true;
}

static gboolean
on_stats_refresh_cb (gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  if (self->priv->calling_state == Connected && self->priv->current_call) {
    gtk_header_bar_set_subtitle (GTK_HEADER_BAR (self->priv->call_panel_toolbar),
                                  self->priv->current_call->get_duration ().c_str ());

    ekiga_call_window_update_stats (self, self->priv->current_call->get_statistics ());
  }

  return true;
}

static gboolean
on_delayed_destroy_cb (gpointer self)
{
  gtk_widget_destroy (GTK_WIDGET (self));

  return FALSE;
}

static gboolean
ekiga_call_window_delete_event_cb (GtkWidget *widget,
                                   G_GNUC_UNUSED GdkEventAny *event)
{
  EkigaCallWindow *self = NULL;
  GSettings *settings = NULL;

  self = EKIGA_CALL_WINDOW (widget);
  g_return_val_if_fail (EKIGA_IS_CALL_WINDOW (self), false);
  if (self->priv->dead)
    return true;

  self->priv->dead = true;

  /* Hang up or disable preview */
  if (self->priv->fullscreen) {
    ekiga_call_window_toggle_fullscreen (self);
  }

  if (self->priv->calling_state != Standby && self->priv->current_call) {
    self->priv->current_call->hang_up ();
  }
  else {
    settings = g_settings_new (VIDEO_DEVICES_SCHEMA);
    g_settings_set_boolean (settings, "enable-preview", false);
    g_clear_object (&settings);
  }

  /* Destroying the call window directly is not nice
   * from the user perspective.
   */
  g_timeout_add_seconds (2, on_delayed_destroy_cb, self);

  return true;
}

static gboolean
ekiga_call_window_fullscreen_event_cb (GtkWidget *widget,
                                       G_GNUC_UNUSED GdkEventAny *event)
{
  EkigaCallWindow *self = NULL;

  self = EKIGA_CALL_WINDOW (widget);
  g_return_val_if_fail (EKIGA_IS_CALL_WINDOW (self), false);
  ekiga_call_window_toggle_fullscreen (self);

  return true; // Do not relay the event anymore
}

static void
ekiga_call_window_remove_action_entries (GActionMap *map,
                                         const GActionEntry *entries)
{
  for (unsigned int i = 0 ; i < G_N_ELEMENTS (entries) ; i++)
    g_action_map_remove_action (map, entries[i].name);
}

static void
ekiga_call_window_update_calling_state (EkigaCallWindow *self,
                                        unsigned calling_state)
{
  g_return_if_fail (self != NULL);

  switch (calling_state)
    {
    case Standby:
      /* Spinner updates */
      gtk_widget_hide (self->priv->spinner);
      gtk_spinner_stop (GTK_SPINNER (self->priv->spinner));

      /* Auto destroy */
      g_timeout_add_seconds (2, on_delayed_destroy_cb, self);
      break;

    case Calling:
      /* Spinner updates */
      gtk_widget_show (self->priv->spinner);
      gtk_spinner_start (GTK_SPINNER (self->priv->spinner));
      break;

    case Ringing:

      /* Spinner updates */
      gtk_widget_show (self->priv->spinner);
      gtk_spinner_start (GTK_SPINNER (self->priv->spinner));
      break;

    case Connected:

      /* Spinner updates */
      gtk_widget_hide (self->priv->spinner);
      gtk_spinner_start (GTK_SPINNER (self->priv->spinner));
      break;


    case Called:
    default:
      break;
    }

  self->priv->calling_state = calling_state;
}

static void
ekiga_call_window_update_header_bar_actions (EkigaCallWindow *self,
                                             unsigned calling_state)
{
  GList *it = NULL;
  g_return_if_fail (self != NULL);

  it = gtk_container_get_children (GTK_CONTAINER (self->priv->call_panel_toolbar));


  switch (calling_state) {

  case Called:
    while (it != NULL) {
      if (it->data && GTK_IS_ACTIONABLE (it->data)) {
        const char* action_name = gtk_actionable_get_action_name (GTK_ACTIONABLE (it->data));
        if (!g_strcmp0 (action_name, "win.reject") || !g_strcmp0 (action_name, "win.answer"))
          gtk_widget_show (GTK_WIDGET (it->data));
        else
          gtk_widget_hide (GTK_WIDGET (it->data));
      }
      it = g_list_next (it);
    }
    gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (self->priv->call_panel_toolbar), FALSE);
    break;

  case Standby:
  case Calling:
  case Ringing:
  case Connected:
  default:
    while (it != NULL) {
      if (it->data && GTK_IS_ACTIONABLE (it->data)) {
        const char* action_name = gtk_actionable_get_action_name (GTK_ACTIONABLE (it->data));
        if (!g_strcmp0 (action_name, "win.reject") || !g_strcmp0 (action_name, "win.answer"))
          gtk_widget_hide (GTK_WIDGET (it->data));
        else
          gtk_widget_show (GTK_WIDGET (it->data));
      }
      it = g_list_next (it);
    }
    gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (self->priv->call_panel_toolbar), TRUE);
    break;
  }

  g_list_free (it);
}

static void
ekiga_call_window_clear_signal_levels (EkigaCallWindow *self)
{
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (self));

  //gm_level_meter_clear (GM_LEVEL_METER (self->priv->output_signal));
  //gm_level_meter_clear (GM_LEVEL_METER (self->priv->input_signal));
}

static void
ekiga_call_window_clear_stats (EkigaCallWindow *self)
{
  RTCPStatistics stats;
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (self));

  ekiga_call_window_update_stats (self, stats);
}

static void
ekiga_call_window_update_title (EkigaCallWindow *self,
                                unsigned calling_state,
                                const std::string & remote_party)
{
  g_return_if_fail (self != NULL);
  gchar *title = NULL;

  switch (calling_state)
    {
    case Calling:
      if (!remote_party.empty ())
        title = g_strdup_printf (_("Calling %s"), remote_party.c_str ());
      break;

    case Connected:
      if (!remote_party.empty ())
        title = g_strdup_printf (_("Connected with %s"), remote_party.c_str ());
      break;

    case Ringing:
      break;
    case Called:
      if (!remote_party.empty ())
        title = g_strdup_printf (_("Call from %s"), remote_party.c_str ());
      break;

    case Standby:
    default:
      title = g_strdup (_("Call Window"));
      break;
    }

  if (!title)
      title = g_strdup (_("Call Window"));

  gtk_header_bar_set_title (GTK_HEADER_BAR (self->priv->call_panel_toolbar), title);
  g_free (title);
}

static void
ekiga_call_window_update_stats (EkigaCallWindow *self,
                                const RTCPStatistics & stats)
{
  gchar *stats_msg = NULL;
  gchar *re_video_msg = NULL;
  gchar *tr_video_msg = NULL;
  unsigned received_width, received_height, transmitted_width, transmitted_height = 0;

  g_return_if_fail (EKIGA_IS_CALL_WINDOW (self));

  gm_video_widget_get_stream_natural_size (GM_VIDEO_WIDGET (self->priv->video_widget),
                                           PRIMARY_STREAM, &received_width, &received_height);
  gm_video_widget_get_stream_natural_size (GM_VIDEO_WIDGET (self->priv->video_widget),
                                           SECONDARY_STREAM, &transmitted_width, &transmitted_height);
  if (received_width != 0 && received_height != 0)
    re_video_msg = g_strdup_printf (" - %s (%dx%d)",
                                    stats.received_video_codec.c_str (), received_width, received_height);
  else
    re_video_msg = g_strdup ("");

  if (transmitted_width != 0 && transmitted_height != 0)
    tr_video_msg = g_strdup_printf (" - %s (%dx%d)",
                                    stats.transmitted_video_codec.c_str (), transmitted_width, transmitted_height);
  else
    tr_video_msg = g_strdup ("");

  stats_msg =
    g_strdup_printf (_("<b><u>Reception:</u></b> %s %s\nLost Packets: %d %%\nJitter: %d ms\nFramerate: %d fps\nBandwidth: %d kbits/s\n\n"
                       "<b><u>Transmission:</u></b> %s %s\nRemote Lost Packets: %d %%\nRemote Jitter: %d ms\nFramerate: %d fps\nBandwidth: %d kbits/s\n\n"),
                       stats.received_audio_codec.c_str (), re_video_msg, stats.lost_packets, stats.jitter,
                       stats.received_fps, stats.received_audio_bandwidth + stats.received_video_bandwidth,
                       stats.transmitted_audio_codec.c_str (), tr_video_msg, stats.remote_lost_packets, stats.remote_jitter,
                       stats.transmitted_fps, stats.transmitted_audio_bandwidth + stats.transmitted_video_bandwidth);
  gtk_widget_set_tooltip_markup (GTK_WIDGET (self->priv->event_box), stats_msg);

  if (!self->priv->bad_connection && (stats.jitter > 250 || stats.lost_packets > 2)) {
    gm_info_bar_push_message (GM_INFO_BAR (self->priv->info_bar),
                              GTK_MESSAGE_WARNING,
                              _("The call quality is rather bad. Please check your Internet connection or your audio driver."));
    self->priv->bad_connection = true;
  }

  g_free (stats_msg);
  g_free (re_video_msg);
  g_free (tr_video_msg);
}


static GtkWidget *
gm_call_window_build_settings_popover (EkigaCallWindow *self,
                                       GtkWidget *relative)
{
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *image = NULL;

  GtkWidget *popover = NULL;

  GIcon *icon = NULL;
  gboolean audio = FALSE;

  popover = gtk_popover_new (NULL);
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_set_border_width (GTK_CONTAINER (popover), 18);
  gtk_container_add (GTK_CONTAINER (popover), vbox);
  gtk_popover_set_relative_to (GTK_POPOVER (popover), relative);

  const char *icons[MAX_SETTINGS] = {
    "preferences-color-symbolic",
    "display-brightness-symbolic",
    "display-brightness-symbolic",
    "display-brightness-symbolic",
    "audio-speakers-symbolic",
    "audio-input-microphone-symbolic",
  };

  for (int i = 0 ; i < MAX_SETTINGS ; i++) {

    if (self->priv->settings[i] == -1)
      continue;

    audio = (i == SPEAKER_VOLUME || i == MIC_VOLUME);
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    icon = g_themed_icon_new (icons[i]);
    image = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_MENU);
    g_object_unref (icon);
    gtk_box_pack_start (GTK_BOX (hbox), image, false, false, 6);

    self->priv->settings_range[i] = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0.0, audio ? 100.0 : 255.0, 5.0);
    gtk_range_set_value (GTK_RANGE (self->priv->settings_range[i]), self->priv->settings[i]);
    gtk_scale_set_draw_value (GTK_SCALE (self->priv->settings_range[i]), false);
    gtk_scale_set_value_pos (GTK_SCALE (self->priv->settings_range[i]), GTK_POS_RIGHT);
    gtk_box_pack_start (GTK_BOX (hbox), self->priv->settings_range[i], true, true, 6);

    gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 0);
    gtk_widget_set_size_request (GTK_WIDGET (self->priv->settings_range[i]), 150, -1);

    g_signal_connect (self->priv->settings_range[i], "value-changed",
                      G_CALLBACK (call_devices_settings_changed_cb), self);
  }

  g_signal_connect_swapped (popover, "hide",
                            G_CALLBACK (gtk_widget_destroy), popover);

  return popover;
}


static void
ekiga_call_window_init_menu (EkigaCallWindow *self)
{
  g_return_if_fail (self != NULL);
  self->priv->builder = gtk_builder_new ();
  gtk_builder_add_from_string (self->priv->builder, win_menu, -1, NULL);

  g_action_map_add_action (G_ACTION_MAP (g_application_get_default ()),
                           g_settings_create_action (self->priv->video_display_settings->get_g_settings (),
                                                     "enable-pip"));

  g_action_map_add_action_entries (G_ACTION_MAP (g_application_get_default ()),
                                   win_entries, G_N_ELEMENTS (win_entries),
                                   self);

  gtk_widget_insert_action_group (GTK_WIDGET (self), "win",
                                  G_ACTION_GROUP (g_application_get_default ()));
}


static void
ekiga_call_window_init_clutter (EkigaCallWindow *self)
{
  gchar *filename = NULL;

  self->priv->video_widget = gm_video_widget_new ();
  gtk_widget_show (self->priv->video_widget);
  gtk_widget_set_size_request (GTK_WIDGET (self->priv->video_widget),
                               STAGE_WIDTH, STAGE_HEIGHT);
  gtk_container_add (GTK_CONTAINER (self->priv->event_box), self->priv->video_widget);

  filename = g_build_filename (DATA_DIR, "pixmaps", PACKAGE_NAME,
                               PACKAGE_NAME "-full-icon.png", NULL);
  gm_video_widget_set_logo (GM_VIDEO_WIDGET (self->priv->video_widget), filename);
  g_free (filename);

  self->priv->videooutput_core->set_display_info (gm_video_widget_get_stream (GM_VIDEO_WIDGET (self->priv->video_widget), SECONDARY_STREAM), gm_video_widget_get_stream (GM_VIDEO_WIDGET (self->priv->video_widget), PRIMARY_STREAM));
}

static void
ekiga_call_window_toggle_fullscreen (EkigaCallWindow *self)
{
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (self));

  self->priv->fullscreen = !self->priv->fullscreen;

  if (self->priv->fullscreen) {
    gm_window_save (GM_WINDOW (self));
    gtk_widget_hide (self->priv->call_panel_toolbar);
    gtk_window_maximize (GTK_WINDOW (self));
    gtk_window_fullscreen (GTK_WINDOW (self));
    gm_video_widget_set_fullscreen (GM_VIDEO_WIDGET (self->priv->video_widget), true);
    gtk_window_set_keep_above (GTK_WINDOW (self), true);
  }
  else {
    gtk_widget_show (self->priv->call_panel_toolbar);
    gtk_window_unmaximize (GTK_WINDOW (self));
    gtk_window_unfullscreen (GTK_WINDOW (self));
    gtk_window_set_keep_above (GTK_WINDOW (self),
                               self->priv->video_display_settings->get_bool ("stay-on-top"));
    gm_window_restore (GM_WINDOW (self));
    gm_video_widget_set_fullscreen (GM_VIDEO_WIDGET (self->priv->video_widget), false);
  }
}


static void
ekiga_call_window_connect_engine_signals (EkigaCallWindow *self)
{
  boost::signals2::connection conn;

  g_return_if_fail (EKIGA_IS_CALL_WINDOW (self));

  /* New Display Engine signals */
  conn = self->priv->videooutput_core->device_opened.connect (boost::bind (&on_videooutput_device_opened_cb, _1, _2, _3, _4, _5, _6, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->videooutput_core->device_closed.connect (boost::bind (&on_videooutput_device_closed_cb, _1, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->videooutput_core->device_error.connect (boost::bind (&on_videooutput_device_error_cb, _1, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->videooutput_core->size_changed.connect (boost::bind (&on_size_changed_cb, _1, _2, _3, _4, (gpointer) self));
  self->priv->connections.add (conn);


  /* New VideoInput Engine signals */
  conn = self->priv->videoinput_core->device_opened.connect (boost::bind (&on_videoinput_device_opened_cb, _1, _2, _3, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->videoinput_core->device_closed.connect (boost::bind (&on_videoinput_device_closed_cb, _1, _2, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->videoinput_core->device_error.connect (boost::bind (&on_videoinput_device_error_cb, _1, _2, _3, (gpointer) self));
  self->priv->connections.add (conn);

  /* New AudioInput Engine signals */
  conn = self->priv->audioinput_core->device_opened.connect (boost::bind (&on_audioinput_device_opened_cb, _1, _2, _3, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->audioinput_core->device_closed.connect (boost::bind (&on_audioinput_device_closed_cb, _1, _2, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->audioinput_core->device_error.connect (boost::bind (&on_audioinput_device_error_cb, _1, _2, _3, (gpointer) self));
  self->priv->connections.add (conn);

  /* New AudioOutput Engine signals */
  conn = self->priv->audiooutput_core->device_opened.connect (boost::bind (&on_audiooutput_device_opened_cb, _1, _2, _3, _4, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->audiooutput_core->device_closed.connect (boost::bind (&on_audiooutput_device_closed_cb, _1, _2, _3, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->audiooutput_core->device_error.connect (boost::bind (&on_audiooutput_device_error_cb, _1, _2, _3, _4, (gpointer) self));
  self->priv->connections.add (conn);

  /* New Call Engine signals */
  conn = self->priv->call_core->setup_call.connect (boost::bind (&on_setup_call_cb, _1, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->call_core->ringing_call.connect (boost::bind (&on_ringing_call_cb, _1, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->call_core->established_call.connect (boost::bind (&on_established_call_cb, _1, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->call_core->cleared_call.connect (boost::bind (&on_cleared_call_cb, _1, _2, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->call_core->missed_call.connect (boost::bind (&on_missed_call_cb, _1, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->call_core->held_call.connect (boost::bind (&on_held_call_cb, _1, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->call_core->retrieved_call.connect (boost::bind (&on_retrieved_call_cb, _1, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->call_core->stream_opened.connect (boost::bind (&on_stream_opened_cb, _1, _2, _3, _4, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->call_core->stream_closed.connect (boost::bind (&on_stream_closed_cb, _1, _2, _3, _4, (gpointer) self));
  self->priv->connections.add (conn);
}

static void
ekiga_call_window_init_gui (EkigaCallWindow *self)
{
  GtkWidget *event_box = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *button = NULL;

  GtkWidget *image = NULL;

  GIcon *icon = NULL;

  /* The extended video stream window */
  self->priv->ext_video_win = NULL;

  /* The main table */
  event_box = gtk_event_box_new ();
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
  gtk_container_add (GTK_CONTAINER (event_box), vbox);
  gtk_container_add (GTK_CONTAINER (frame), event_box);
  gtk_container_add (GTK_CONTAINER (self), frame);
  gtk_widget_show_all (frame);

  /* Menu */
  ekiga_call_window_init_menu (self);

  /* The widgets header bar */
  self->priv->call_panel_toolbar = gtk_header_bar_new ();
  gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (self->priv->call_panel_toolbar), TRUE);
  gtk_window_set_titlebar (GTK_WINDOW (self), self->priv->call_panel_toolbar);
  gtk_window_set_icon_name (GTK_WINDOW (self), PACKAGE_NAME);
  gtk_widget_show (self->priv->call_panel_toolbar);

  /* The info bar */
  self->priv->info_bar = gm_info_bar_new ();
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (self->priv->info_bar), FALSE, FALSE, 0);

  /* The frame that contains the video */
  self->priv->event_box = gtk_event_box_new ();
  ekiga_call_window_init_clutter (self);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (self->priv->event_box), TRUE, TRUE, 0);
  gtk_widget_show_all (self->priv->event_box);


  /* FIXME:
   * All those actions should be call specific.
   * We should generate the header bar actions like we generate a menu.
   * Probably introducing a GActorHeaderBar would be nice.
   * However, it is unneeded right now as we only support one call at a time.
   */

  /* Reject */
  button = gtk_button_new_with_mnemonic (_("_Reject"));
  icon = g_themed_icon_new ("phone-hang-up");
  image = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_BUTTON);
  g_object_unref (icon);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_button_set_always_show_image (GTK_BUTTON (button), TRUE);
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "win.reject");
  gtk_header_bar_pack_start (GTK_HEADER_BAR (self->priv->call_panel_toolbar), button);
  gtk_widget_set_tooltip_text (GTK_WIDGET (button),
                               _("Reject the incoming call"));
  gtk_widget_show (button);

  /* Hang up */
  button = gtk_button_new ();
  icon = g_themed_icon_new ("call-end-symbolic");
  image = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_BUTTON);
  g_object_unref (icon);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "win.hangup");
  gtk_header_bar_pack_start (GTK_HEADER_BAR (self->priv->call_panel_toolbar), button);
  gtk_widget_set_tooltip_text (GTK_WIDGET (button),
                               _("Hang up the current call"));
  gtk_widget_show (button);

  /* Call Hold */
  button = gtk_toggle_button_new ();
  icon = g_themed_icon_new ("call-hold-symbolic");
  image = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_BUTTON);
  g_object_unref (icon);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "win.hold");
  gtk_header_bar_pack_start (GTK_HEADER_BAR (self->priv->call_panel_toolbar), button);
  gtk_widget_set_tooltip_text (GTK_WIDGET (button),
                               _("Hold or retrieve the current call"));
  gtk_widget_show (button);

  /* Call Transfer */
  button = gtk_button_new ();
  icon = g_themed_icon_new ("call-transfer-symbolic");
  image = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_BUTTON);
  g_object_unref (icon);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "win.transfer");
  gtk_header_bar_pack_start (GTK_HEADER_BAR (self->priv->call_panel_toolbar), button);
  gtk_widget_set_tooltip_text (GTK_WIDGET (button),
                               _("Transfer the current call"));
  gtk_widget_show (button);

  /* Devices settings */
  self->priv->settings_button = gtk_button_new ();
  icon = g_themed_icon_new ("emblem-system-symbolic");
  image = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_BUTTON);
  g_object_unref (icon);
  gtk_button_set_image (GTK_BUTTON (self->priv->settings_button), image);
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (self->priv->settings_button),
                                           "win.call-devices-settings");
  gtk_header_bar_pack_start (GTK_HEADER_BAR (self->priv->call_panel_toolbar),
                             self->priv->settings_button);
  gtk_widget_set_tooltip_text (GTK_WIDGET (self->priv->settings_button),
                               _("Change audio and video settings"));
  gtk_widget_show (self->priv->settings_button);

  /* Call Accept */
  button = gtk_button_new_with_mnemonic (_("_Answer"));
  icon = g_themed_icon_new ("phone-pick-up");
  image = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_BUTTON);
  g_object_unref (icon);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_button_set_always_show_image (GTK_BUTTON (button), TRUE);
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "win.answer");
  gtk_header_bar_pack_end (GTK_HEADER_BAR (self->priv->call_panel_toolbar), button);
  gtk_widget_set_tooltip_text (GTK_WIDGET (button),
                               _("Answer the incoming call"));
  gtk_widget_show (button);

  /* Spinner */
  self->priv->spinner = gtk_spinner_new ();
  gtk_widget_set_size_request (GTK_WIDGET (self->priv->spinner), 24, 24);
  gtk_header_bar_pack_end (GTK_HEADER_BAR (self->priv->call_panel_toolbar), self->priv->spinner);

  /* Menu button */
  button = gtk_menu_button_new ();
  icon = g_themed_icon_new ("open-menu-symbolic");
  image = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_BUTTON);
  g_object_unref (icon);
  g_object_set (G_OBJECT (button), "use-popover", true, NULL);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (button),
                                  G_MENU_MODEL (gtk_builder_get_object (self->priv->builder, "menubar")));
  gtk_header_bar_pack_end (GTK_HEADER_BAR (self->priv->call_panel_toolbar), button);
  gtk_widget_show (button);

  /* Full Screen */
  button = gtk_button_new ();
  icon = g_themed_icon_new ("view-fullscreen-symbolic");
  image = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_BUTTON);
  g_object_unref (icon);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "win.enable-fullscreen");
  gtk_header_bar_pack_end (GTK_HEADER_BAR (self->priv->call_panel_toolbar), button);
  gtk_widget_set_tooltip_text (GTK_WIDGET (button),
                               _("Switch to fullscreen"));
  gtk_widget_show (button);

  gtk_window_set_resizable (GTK_WINDOW (self), true);

  /* Init */
  ekiga_call_window_update_header_bar_actions (self, Standby);
}

static void
ekiga_call_window_init (EkigaCallWindow *self)
{
  self->priv = new EkigaCallWindowPrivate ();

  self->priv->accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (self), self->priv->accel);
  gtk_accel_group_connect (self->priv->accel, GDK_KEY_Escape, (GdkModifierType) 0, GTK_ACCEL_LOCKED,
                           g_cclosure_new_swap (G_CALLBACK (ekiga_call_window_delete_event_cb),
                                                (gpointer) self, NULL));
  gtk_accel_group_connect (self->priv->accel, GDK_KEY_F11, (GdkModifierType) 0, GTK_ACCEL_LOCKED,
                           g_cclosure_new_swap (G_CALLBACK (ekiga_call_window_fullscreen_event_cb),
                                                (gpointer) self, NULL));
  g_object_unref (self->priv->accel);

  self->priv->current_call = boost::shared_ptr<Ekiga::Call>();
  self->priv->timeout_id = -1;
  self->priv->calling_state = Standby;
  self->priv->fullscreen = false;
  self->priv->dead = false;
  self->priv->bad_connection = false;
  self->priv->video_display_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (VIDEO_DISPLAY_SCHEMA));

  for (int i = 0 ; i < MAX_SETTINGS ; i++)
    self->priv->settings[i] = -1;

  g_signal_connect (self, "delete_event",
                    G_CALLBACK (ekiga_call_window_delete_event_cb), NULL);
}

static void
ekiga_call_window_finalize (GObject *gobject)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (gobject);

  if (self->priv->ext_video_win)
    gtk_widget_destroy (self->priv->ext_video_win);

  delete self->priv;

  G_OBJECT_CLASS (ekiga_call_window_parent_class)->finalize (gobject);
}

static void
ekiga_call_window_show (GtkWidget *widget)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (widget);

  GTK_WIDGET_CLASS (ekiga_call_window_parent_class)->show (widget);

  gtk_widget_queue_draw (GTK_WIDGET (self));
}

static gboolean
ekiga_call_window_focus_in_event (GtkWidget     *widget,
                                  GdkEventFocus *event)
{
  if (gtk_window_get_urgency_hint (GTK_WINDOW (widget)))
    gtk_window_set_urgency_hint (GTK_WINDOW (widget), false);

  return GTK_WIDGET_CLASS (ekiga_call_window_parent_class)->focus_in_event (widget, event);
}

static void
ekiga_call_window_class_init (EkigaCallWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = ekiga_call_window_finalize;

  widget_class->show = ekiga_call_window_show;
  widget_class->focus_in_event = ekiga_call_window_focus_in_event;
}

GtkWidget *
call_window_new (GmApplication *app)
{
  EkigaCallWindow *self;

  g_return_val_if_fail (GM_IS_APPLICATION (app), NULL);

  self = EKIGA_CALL_WINDOW (g_object_new (EKIGA_TYPE_CALL_WINDOW,
                                          "application", GTK_APPLICATION (app),
                                          "key", USER_INTERFACE ".call-window",
                                          "hide_on_delete", false,
                                          "hide_on_esc", false, NULL));
  Ekiga::ServiceCorePtr core = gm_application_get_core (app);

  self->priv->videoinput_core = core->get<Ekiga::VideoInputCore> ("videoinput-core");
  self->priv->videooutput_core = core->get<Ekiga::VideoOutputCore> ("videooutput-core");
  self->priv->audioinput_core = core->get<Ekiga::AudioInputCore> ("audioinput-core");
  self->priv->audiooutput_core = core->get<Ekiga::AudioOutputCore> ("audiooutput-core");
  self->priv->call_core = core->get<Ekiga::CallCore> ("call-core");
  self->priv->friend_or_foe = core->get<Ekiga::FriendOrFoe> ("friend-or-foe");
  self->priv->foe_list = core->get<Ekiga::FoeList> ("foe-list");

  ekiga_call_window_init_gui (self);

  g_settings_bind (self->priv->video_display_settings->get_g_settings (),
                   "stay-on-top",
                   self,
                   "stay_on_top",
                   G_SETTINGS_BIND_DEFAULT);
  g_settings_bind (self->priv->video_display_settings->get_g_settings (),
                   "enable-pip",
                   self->priv->video_widget,
                   "secondary_stream_display",
                   G_SETTINGS_BIND_DEFAULT);

  ekiga_call_window_connect_engine_signals (self);

  return GTK_WIDGET (self);
}
