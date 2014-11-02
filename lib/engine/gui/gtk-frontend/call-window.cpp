
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

#include "config.h"

#include "ekiga-settings.h"

#include "call-window.h"

#include "dialpad.h"

#include "gmvideowidget.h"
#include "gmdialog.h"
#include "gmstatusbar.h"
#include "gmstockicons.h"
#include "gactor-menu.h"
#include <boost/smart_ptr.hpp>
#include "gmmenuaddon.h"
#include "gmpowermeter.h"
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

#define STAGE_WIDTH 640
#define STAGE_HEIGHT 480

enum CallingState {Standby, Calling, Ringing, Connected, Called};

enum DeviceType {AudioInput, AudioOutput, Ringer, VideoInput};
struct deviceStruct {
  char name[256];
  DeviceType deviceType;
};

G_DEFINE_TYPE (EkigaCallWindow, ekiga_call_window, GM_TYPE_WINDOW);

struct _EkigaCallWindowPrivate
{
  Ekiga::ServicePtr libnotify;
  boost::shared_ptr<Ekiga::VideoInputCore> videoinput_core;
  boost::shared_ptr<Ekiga::VideoOutputCore> videooutput_core;
  boost::shared_ptr<Ekiga::AudioInputCore> audioinput_core;
  boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core;
  boost::shared_ptr<Ekiga::CallCore> call_core;

  GtkAccelGroup *accel;

  boost::shared_ptr<Ekiga::Call> current_call;
  unsigned calling_state;

  GtkWidget *ext_video_win;
  GtkWidget *event_box;
  GtkWidget *spinner;
  GtkWidget *info_text;
  GtkBuilder *builder;

  GtkWidget *video_widget;
  bool fullscreen;

  GtkWidget *call_frame;
  GtkWidget *camera_image;

  GtkWidget *call_panel_toolbar;

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

  unsigned int levelmeter_timeout_id;
  unsigned int timeout_id;

  GtkWidget *video_settings_window;
  GtkWidget *video_settings_frame;
  GtkAdjustment *adj_whiteness;
  GtkAdjustment *adj_brightness;
  GtkAdjustment *adj_colour;
  GtkAdjustment *adj_contrast;

  std::string transmitted_video_codec;
  std::string transmitted_audio_codec;
  std::string received_video_codec;
  std::string received_audio_codec;

  Ekiga::GActorMenuPtr menu;

  /* Statusbar */
  GtkWidget *statusbar;
  GtkWidget *statusbar_ebox;
  GtkWidget *qualitymeter;

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


static bool notify_has_actions (EkigaCallWindow* self);

static void show_extended_video_window_cb (G_GNUC_UNUSED GSimpleAction *action,
                                           G_GNUC_UNUSED GVariant *parameter,
                                           gpointer data);

static void fullscreen_changed_cb (G_GNUC_UNUSED GSimpleAction *action,
                                   G_GNUC_UNUSED GVariant *parameter,
                                   gpointer data);

static void show_audio_settings_cb (G_GNUC_UNUSED GSimpleAction *action,
                                    G_GNUC_UNUSED GVariant *parameter,
                                    gpointer data);

static void show_video_settings_cb (G_GNUC_UNUSED GSimpleAction *action,
                                    G_GNUC_UNUSED GVariant *parameter,
                                    gpointer data);

static void audio_volume_changed_cb (GtkAdjustment * /*adjustment*/,
                                     gpointer data);

static void audio_volume_window_shown_cb (GtkWidget * /*widget*/,
                                          gpointer data);

static void audio_volume_window_hidden_cb (GtkWidget * /*widget*/,
                                           gpointer data);

static void video_settings_changed_cb (GtkAdjustment * /*adjustment*/,
                                       gpointer data);

static gboolean on_signal_level_refresh_cb (gpointer self);

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

static void on_ringing_call_cb (boost::shared_ptr<Ekiga::CallManager> manager,
                                boost::shared_ptr<Ekiga::Call> call,
                                gpointer self);

static void on_established_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                                    boost::shared_ptr<Ekiga::Call>  call,
                                    gpointer self);

static void on_cleared_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                                boost::shared_ptr<Ekiga::Call>  call,
                                std::string reason,
                                gpointer self);

static void on_missed_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                               boost::shared_ptr<Ekiga::Call> /*call*/,
                               gpointer self);

static void on_held_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                             boost::shared_ptr<Ekiga::Call>  /*call*/,
                             gpointer self);

static void on_setup_call_cb (boost::shared_ptr<Ekiga::CallManager> manager,
                              boost::shared_ptr<Ekiga::Call> call,
                              gpointer self);

static void on_retrieved_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                                  boost::shared_ptr<Ekiga::Call>  /*call*/,
                                  gpointer self);

static void on_stream_opened_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                                 boost::shared_ptr<Ekiga::Call>  /* call */,
                                 std::string name,
                                 Ekiga::Call::StreamType type,
                                 bool is_transmitting,
                                 gpointer self);

static void on_stream_closed_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                                 boost::shared_ptr<Ekiga::Call>  /* call */,
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
static void ekiga_call_window_update_calling_state (EkigaCallWindow *self,
                                                    unsigned calling_state);

static void ekiga_call_window_clear_signal_levels (EkigaCallWindow *self);

static void ekiga_call_window_clear_stats (EkigaCallWindow *self);

static void ekiga_call_window_update_stats (EkigaCallWindow *self,
                                            float lost,
                                            float late,
                                            float out_of_order,
                                            int jitter,
                                            unsigned int re_width,
                                            unsigned int re_height,
                                            unsigned int tr_width,
                                            unsigned int tr_height,
                                            const char *tr_audio_codec,
                                            const char *tr_video_codec);

static void ekiga_call_window_set_bandwidth (EkigaCallWindow *self,
                                             float ta,
                                             float ra,
                                             float tv,
                                             float rv);

static void ekiga_call_window_init_menu (EkigaCallWindow *self);

static void ekiga_call_window_init_clutter (EkigaCallWindow *self);

static GtkWidget *gm_call_window_audio_settings_window_new (EkigaCallWindow *call_window);

static GtkWidget *gm_call_window_video_settings_window_new (EkigaCallWindow *call_window);

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
  "        <attribute name='label' translatable='yes'>Change _Volume</attribute>"
  "        <attribute name='action'>win.audio-volume-settings</attribute>"
  "      </item>"
  "      <item>"
  "        <attribute name='label' translatable='yes'>Change _Color Settings</attribute>"
  "        <attribute name='action'>win.video-color-settings</attribute>"
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
    { "audio-volume-settings", show_audio_settings_cb, NULL, NULL, NULL, 0 },
    { "video-color-settings", show_video_settings_cb, NULL, NULL, NULL, 0 },
    { "show-extended-video",  show_extended_video_window_cb, NULL, NULL, NULL, 0 },
    { "enable-fullscreen", fullscreen_changed_cb, NULL, NULL, NULL, 0 }
};
/**/


static bool
notify_has_actions (EkigaCallWindow *self)
{
  bool result = false;

  if (self->priv->libnotify) {

    boost::optional<bool> val = self->priv->libnotify->get_bool_property ("actions");
    if (val) {

      result = *val;
    }
  }
  return result;
}

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
show_audio_settings_cb (G_GNUC_UNUSED GSimpleAction *action,
                                    G_GNUC_UNUSED GVariant *parameter,
                                    gpointer data)
{
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (data));
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  if (self->priv->audio_settings_window)
    gtk_widget_show (GTK_WIDGET (self->priv->audio_settings_window));
}

static void
show_video_settings_cb (G_GNUC_UNUSED GSimpleAction *action,
                                    G_GNUC_UNUSED GVariant *parameter,
                                    gpointer data)
{
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (data));
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  if (self->priv->video_settings_window)
    gtk_widget_show (GTK_WIDGET (self->priv->video_settings_window));
}

static void
audio_volume_changed_cb (GtkAdjustment * /*adjustment*/,
                         gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  self->priv->audiooutput_core->set_volume (Ekiga::primary, (unsigned) gtk_adjustment_get_value (GTK_ADJUSTMENT (self->priv->adj_output_volume)));
  self->priv->audioinput_core->set_volume ((unsigned) gtk_adjustment_get_value (GTK_ADJUSTMENT (self->priv->adj_input_volume)));
}

static void
audio_volume_window_shown_cb (GtkWidget * /*widget*/,
                              gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  self->priv->audioinput_core->set_average_collection (true);
  self->priv->audiooutput_core->set_average_collection (true);
  self->priv->levelmeter_timeout_id = g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, 50, on_signal_level_refresh_cb, data, NULL);
}

static void
audio_volume_window_hidden_cb (GtkWidget * /*widget*/,
                               gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  g_source_remove (self->priv->levelmeter_timeout_id);
  self->priv->audioinput_core->set_average_collection (false);
  self->priv->audiooutput_core->set_average_collection (false);
}

static void
video_settings_changed_cb (GtkAdjustment * /*adjustment*/,
                           gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);


  self->priv->videoinput_core->set_whiteness ((unsigned) gtk_adjustment_get_value (GTK_ADJUSTMENT (self->priv->adj_whiteness)));
  self->priv->videoinput_core->set_brightness ((unsigned) gtk_adjustment_get_value (GTK_ADJUSTMENT (self->priv->adj_brightness)));
  self->priv->videoinput_core->set_colour ((unsigned) gtk_adjustment_get_value (GTK_ADJUSTMENT (self->priv->adj_colour)));
  self->priv->videoinput_core->set_contrast ((unsigned) gtk_adjustment_get_value (GTK_ADJUSTMENT (self->priv->adj_contrast)));
}

static gboolean
on_signal_level_refresh_cb (gpointer /*self*/)
{
  //EkigaCallWindow *self = EKIGA_CALL_WINDOW (self);

  //gm_level_meter_set_level (GM_LEVEL_METER (self->priv->output_signal), self->priv->audiooutput_core->get_average_level());
  //gm_level_meter_set_level (GM_LEVEL_METER (self->priv->input_signal), self->priv->audioinput_core->get_average_level());
  return true;
}

static void
on_videooutput_device_opened_cb (Ekiga::VideoOutputManager & /* manager */,
                                 Ekiga::VideoOutputManager::VideoView type,
                                 unsigned width,
                                 unsigned height,
                                 bool both_streams,
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
                                gpointer self)
{
  GtkWidget *dialog = NULL;

  const gchar *dialog_title =  _("Error while initializing video output");
  const gchar *tmp_msg = _("No video will be displayed on your machine during this call");
  gchar *dialog_msg = g_strconcat (_("There was an error opening or initializing the video output. Please verify that no other application is using the accelerated video output."), "\n\n", tmp_msg, NULL);

  dialog = gtk_message_dialog_new (GTK_WINDOW (self), GTK_DIALOG_MODAL,
                                   GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                   dialog_msg);
  gtk_window_set_title (GTK_WINDOW (dialog), dialog_title);
  g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
  gtk_widget_show_all (GTK_WIDGET (dialog));

  g_free (dialog_msg);
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
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  gtk_widget_set_sensitive (self->priv->video_settings_frame,  settings.modifyable ? true : false);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (self->priv->adj_whiteness), settings.whiteness);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (self->priv->adj_brightness), settings.brightness);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (self->priv->adj_colour), settings.colour);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (self->priv->adj_contrast), settings.contrast);

  gtk_widget_queue_draw (self->priv->video_settings_frame);
}

static void
on_videoinput_device_closed_cb (Ekiga::VideoInputManager & /* manager */,
                                Ekiga::VideoInputDevice & /*device*/,
                                gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

}

static void
on_videoinput_device_error_cb (Ekiga::VideoInputManager & /* manager */,
                               Ekiga::VideoInputDevice & device,
                               Ekiga::VideoInputErrorCodes error_code,
                               gpointer self)
{
  GtkWidget *dialog = NULL;

  gchar *dialog_title = NULL;
  gchar *dialog_msg = NULL;
  gchar *tmp_msg = NULL;

  dialog_title = g_strdup_printf (_("Error while accessing video device %s"),
                                  (const char *) device.name.c_str());

  tmp_msg = g_strdup (_("A moving logo will be transmitted during calls."));
  switch (error_code) {

  case Ekiga::VI_ERROR_DEVICE:
    dialog_msg = g_strconcat (_("There was an error while opening the device. In case it is a pluggable device it may be sufficient to reconnect it. If not, or if it still is not accessible, please check your permissions and make sure that the appropriate driver is loaded."), "\n\n", tmp_msg, NULL);
    break;

  case Ekiga::VI_ERROR_FORMAT:
    dialog_msg = g_strconcat (_("Your video driver doesn't support the requested video format."), "\n\n", tmp_msg, NULL);
    break;

  case Ekiga::VI_ERROR_CHANNEL:
    dialog_msg = g_strconcat (_("Could not open the chosen channel."), "\n\n", tmp_msg, NULL);
    break;

  case Ekiga::VI_ERROR_COLOUR:
    dialog_msg = g_strconcat (_("Your driver doesn't seem to support any of the color formats supported by Ekiga.\n Please check your kernel driver documentation in order to determine which Palette is supported."), "\n\n", tmp_msg, NULL);
    break;

  case Ekiga::VI_ERROR_FPS:
    dialog_msg = g_strconcat (_("Error while setting the frame rate."), "\n\n", tmp_msg, NULL);
    break;

  case Ekiga::VI_ERROR_SCALE:
    dialog_msg = g_strconcat (_("Error while setting the frame size."), "\n\n", tmp_msg, NULL);
    break;

  case Ekiga::VI_ERROR_NONE:
  default:
    dialog_msg = g_strconcat (_("Unknown error."), "\n\n", tmp_msg, NULL);
    break;
  }

  dialog = gtk_message_dialog_new (GTK_WINDOW (self), GTK_DIALOG_MODAL,
                                   GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                   dialog_msg);
  gtk_window_set_title (GTK_WINDOW (dialog), dialog_title);
  g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
  gtk_widget_show_all (dialog);

  g_free (dialog_msg);
  g_free (dialog_title);
  g_free (tmp_msg);
}

static void
on_audioinput_device_opened_cb (Ekiga::AudioInputManager & /* manager */,
                                Ekiga::AudioInputDevice & /* device */,
                                Ekiga::AudioInputSettings & settings,
                                gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  gtk_widget_set_sensitive (self->priv->audio_input_volume_frame, settings.modifyable);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (self->priv->adj_input_volume), settings.volume);

  gtk_widget_queue_draw (self->priv->audio_input_volume_frame);
}

static void
on_audioinput_device_closed_cb (Ekiga::AudioInputManager & /* manager */,
                                Ekiga::AudioInputDevice & /*device*/,
                                gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  gtk_widget_set_sensitive (self->priv->audio_input_volume_frame, false);
}

static void
on_audioinput_device_error_cb (Ekiga::AudioInputManager & /* manager */,
                               Ekiga::AudioInputDevice & device,
                               Ekiga::AudioInputErrorCodes error_code,
                               gpointer self)
{
  GtkWidget *dialog = NULL;

  gchar *dialog_title = NULL;
  gchar *dialog_msg = NULL;
  gchar *tmp_msg = NULL;

  dialog_title =
    g_strdup_printf (_("Error while opening audio input device %s"),
                     (const char *) device.name.c_str());

  /* Translators: This happens when there is an error with audio input:
   * Nothing ("silence") will be transmitted */
  tmp_msg = g_strdup (_("Only silence will be transmitted."));
  switch (error_code) {

  case Ekiga::AI_ERROR_DEVICE:
    dialog_msg = g_strconcat (tmp_msg, "\n\n", _("Unable to open the selected audio device for recording. In case it is a pluggable device it may be sufficient to reconnect it. If not, or if it still is not accessible, please check your audio setup, the permissions and that the device is not busy."), NULL);
    break;

  case Ekiga::AI_ERROR_READ:
    dialog_msg = g_strconcat (tmp_msg, "\n\n", _("The selected audio device was successfully opened but it is impossible to read data from this device. In case it is a pluggable device it may be sufficient to reconnect it. If not, or if it still is not accessible, please check your audio setup."), NULL);
    break;

  case Ekiga::AI_ERROR_NONE:
  default:
    dialog_msg = g_strconcat (tmp_msg, "\n\n", _("Unknown error."), NULL);
    break;
  }

  dialog = gtk_message_dialog_new (GTK_WINDOW (self), GTK_DIALOG_MODAL,
                                   GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                   dialog_msg);
  gtk_window_set_title (GTK_WINDOW (dialog), dialog_title);
  g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
  gtk_widget_show_all (GTK_WIDGET (dialog));

  g_free (dialog_msg);
  g_free (dialog_title);
  g_free (tmp_msg);

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

  gtk_widget_set_sensitive (self->priv->audio_output_volume_frame, settings.modifyable);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (self->priv->adj_output_volume), settings.volume);

  gtk_widget_queue_draw (self->priv->audio_output_volume_frame);
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

  gtk_widget_set_sensitive (self->priv->audio_output_volume_frame, false);
}

static void
on_audiooutput_device_error_cb (Ekiga::AudioOutputManager & /*manager */,
                                Ekiga::AudioOutputPS ps,
                                Ekiga::AudioOutputDevice & device,
                                Ekiga::AudioOutputErrorCodes error_code,
                                gpointer self)
{
  GtkWidget *dialog = NULL;

  if (ps == Ekiga::secondary)
    return;

  gchar *dialog_title = NULL;
  gchar *dialog_msg = NULL;
  gchar *tmp_msg = NULL;

  dialog_title =
    g_strdup_printf (_("Error while opening audio output device %s"),
                     (const char *) device.name.c_str());

  tmp_msg = g_strdup (_("No incoming sound will be played."));
  switch (error_code) {

  case Ekiga::AO_ERROR_DEVICE:
    dialog_msg = g_strconcat (tmp_msg, "\n\n", _("Unable to open the selected audio device for playing. In case it is a pluggable device it may be sufficient to reconnect it. If not, or if it still is not accessible, please check your audio setup, the permissions and that the device is not busy."), NULL);
    break;

  case Ekiga::AO_ERROR_WRITE:
    dialog_msg = g_strconcat (tmp_msg, "\n\n", _("The selected audio device was successfully opened but it is impossible to write data to this device. In case it is a pluggable device it may be sufficient to reconnect it. If not, or if it still is not accessible, please check your audio setup."), NULL);
    break;

  case Ekiga::AO_ERROR_NONE:
  default:
    dialog_msg = g_strconcat (tmp_msg, "\n\n", _("Unknown error."), NULL);
    break;
  }

  dialog = gtk_message_dialog_new (GTK_WINDOW (self), GTK_DIALOG_MODAL,
                                   GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                   dialog_msg);
  gtk_window_set_title (GTK_WINDOW (dialog), dialog_title);
  g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
  gtk_widget_show_all (GTK_WIDGET (dialog));

  g_free (dialog_msg);
  g_free (dialog_title);
  g_free (tmp_msg);
}

static void
on_ringing_call_cb (G_GNUC_UNUSED boost::shared_ptr<Ekiga::CallManager> manager,
                    G_GNUC_UNUSED boost::shared_ptr<Ekiga::Call>  call,
                    gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  g_return_if_fail (self);

  self->priv->calling_state = Ringing;

  ekiga_call_window_update_calling_state (self, self->priv->calling_state);
}


static void
on_setup_call_cb (boost::shared_ptr<Ekiga::CallManager> manager,
                  boost::shared_ptr<Ekiga::Call>  call,
                  gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);
  boost::signals2::connection conn;

  self->priv->menu = Ekiga::GActorMenuPtr (new Ekiga::GActorMenu (*call));

  if (!call->is_outgoing () && !manager->get_auto_answer ()) {
    if (self->priv->current_call)
      return; // No call setup needed if already in a call

    self->priv->current_call = call;
    self->priv->calling_state = Called;
  }
  else {

    self->priv->current_call = call;
    self->priv->calling_state = Calling;
  }

  gtk_window_set_title (GTK_WINDOW (self), call->get_remote_uri ().c_str ());

  ekiga_call_window_update_calling_state (self, self->priv->calling_state);

  conn = call->questions.connect (boost::bind (&on_handle_questions, _1, (gpointer) self));
  self->priv->connections.add (conn);
}


static void
on_established_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                        boost::shared_ptr<Ekiga::Call>  call,
                        gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  gtk_window_set_title (GTK_WINDOW (self), call->get_remote_party_name ().c_str ());

  ekiga_call_window_update_calling_state (self, Connected);

  self->priv->current_call = call;

  self->priv->timeout_id = g_timeout_add_seconds (1, on_stats_refresh_cb, self);
}

static void
on_cleared_call_cb (G_GNUC_UNUSED boost::shared_ptr<Ekiga::CallManager> manager,
                    boost::shared_ptr<Ekiga::Call>  call,
                    G_GNUC_UNUSED std::string reason,
                    gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  if (self->priv->current_call && self->priv->current_call->get_id () != call->get_id ()) {
    return; // Trying to clear another call than the current active one
  }

  ekiga_call_window_update_calling_state (self, Standby);
  ekiga_call_window_set_bandwidth (self, 0.0, 0.0, 0.0, 0.0);
  ekiga_call_window_clear_stats (self);

  if (self->priv->current_call) {
    self->priv->current_call = boost::shared_ptr<Ekiga::Call>();
    g_source_remove (self->priv->timeout_id);
    self->priv->timeout_id = -1;
    self->priv->menu.reset ();
  }

  ekiga_call_window_clear_signal_levels (self);

  gtk_window_set_title (GTK_WINDOW (self), _("Call Window"));
}

static void on_missed_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                               boost::shared_ptr<Ekiga::Call> call,
                               gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  if (self->priv->current_call && call && self->priv->current_call->get_id () != call->get_id ()) {
    return; // Trying to clear another call than the current active one
  }
  self->priv->menu.reset ();

  gtk_window_set_title (GTK_WINDOW (self), _("Call Window"));
  ekiga_call_window_update_calling_state (self, Standby);
}

static void
on_held_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                 boost::shared_ptr<Ekiga::Call>  /*call*/,
                 gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  gm_statusbar_flash_message (GM_STATUSBAR (self->priv->statusbar), _("Call on hold"));
}


static void
on_retrieved_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                      boost::shared_ptr<Ekiga::Call>  /*call*/,
                      gpointer data)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (data);

  gm_statusbar_flash_message (GM_STATUSBAR (self->priv->statusbar), _("Call retrieved"));
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
on_stream_opened_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                     boost::shared_ptr<Ekiga::Call>  /* call */,
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
on_stream_closed_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                     boost::shared_ptr<Ekiga::Call>  /* call */,
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
  unsigned local_width = 0;
  unsigned local_height = 0;
  unsigned remote_width = 0;
  unsigned remote_height = 0;

  if (self->priv->calling_state == Connected && self->priv->current_call) {

    gtk_header_bar_set_title (GTK_HEADER_BAR (self->priv->call_panel_toolbar),
                              self->priv->current_call->get_remote_party_name ().c_str ());
    gtk_header_bar_set_subtitle (GTK_HEADER_BAR (self->priv->call_panel_toolbar),
                                  self->priv->current_call->get_duration ().c_str ());
    ekiga_call_window_set_bandwidth (self,
                                     self->priv->current_call->get_transmitted_audio_bandwidth (),
                                     self->priv->current_call->get_received_audio_bandwidth (),
                                     self->priv->current_call->get_transmitted_video_bandwidth (),
                                     self->priv->current_call->get_received_video_bandwidth ());

    unsigned int jitter = self->priv->current_call->get_jitter_size ();
    double lost = self->priv->current_call->get_lost_packets ();
    double late = self->priv->current_call->get_late_packets ();
    double out_of_order = self->priv->current_call->get_out_of_order_packets ();
    gm_video_widget_get_stream_natural_size (GM_VIDEO_WIDGET (self->priv->video_widget),
                                             PRIMARY_STREAM, &remote_width, &remote_height);
    gm_video_widget_get_stream_natural_size (GM_VIDEO_WIDGET (self->priv->video_widget),
                                             SECONDARY_STREAM, &local_width, &local_height);

    ekiga_call_window_update_stats (self, lost, late, out_of_order, jitter,
                                    local_width, local_height, remote_width, remote_height,
                                    self->priv->transmitted_audio_codec.c_str (),
                                    self->priv->transmitted_video_codec.c_str ());
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

  /* Hang up or disable preview */
  if (self->priv->calling_state != Standby && self->priv->current_call) {
    self->priv->current_call->hang_up ();
  }
  else if (self->priv->fullscreen) {
    ekiga_call_window_toggle_fullscreen (self);
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
ekiga_call_window_update_calling_state (EkigaCallWindow *self,
                                        unsigned calling_state)
{
  g_return_if_fail (self != NULL);

  switch (calling_state)
    {
    case Standby:

      /* Spinner updates */
      gtk_widget_show (self->priv->camera_image);
      gtk_widget_hide (self->priv->spinner);
      gtk_spinner_stop (GTK_SPINNER (self->priv->spinner));

      /* Show/hide call frame */
      gtk_widget_hide (self->priv->call_frame);
      break;


    case Calling:

      /* Show/hide call frame */
      gtk_widget_show (self->priv->call_frame);
      break;

    case Ringing:

      /* Spinner updates */
      gtk_widget_hide (self->priv->camera_image);
      gtk_widget_show (self->priv->spinner);
      gtk_spinner_start (GTK_SPINNER (self->priv->spinner));
      break;

    case Connected:

      /* Show/hide call frame */
      gtk_widget_show (self->priv->call_frame);

      /* Spinner updates */
      gtk_widget_show (self->priv->camera_image);
      gtk_widget_hide (self->priv->spinner);
      gtk_spinner_start (GTK_SPINNER (self->priv->spinner));
      break;


    case Called:

      /* Show/hide call frame and call window (if no notifications */
      gtk_widget_show (self->priv->call_frame);
      if (!notify_has_actions (self)) {
        gtk_window_present (GTK_WINDOW (self));
        gtk_widget_show (GTK_WIDGET (self));
      }
      break;

    default:
      break;
    }

  self->priv->calling_state = calling_state;
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
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (self));

  ekiga_call_window_update_stats (self, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
  if (self->priv->qualitymeter)
    gm_powermeter_set_level (GM_POWERMETER (self->priv->qualitymeter), 0.0);
}


static void
ekiga_call_window_update_stats (EkigaCallWindow *self,
                                float lost,
                                float late,
                                float out_of_order,
                                int jitter,
                                unsigned int re_width,
                                unsigned int re_height,
                                unsigned int tr_width,
                                unsigned int tr_height,
                                const char *tr_audio_codec,
                                const char *tr_video_codec)
{
  gchar *stats_msg = NULL;
  gchar *stats_msg_tr = NULL;
  gchar *stats_msg_re = NULL;
  gchar *stats_msg_codecs = NULL;

  int jitter_quality = 0;
  gfloat quality_level = 0.0;

  g_return_if_fail (EKIGA_IS_CALL_WINDOW (self));

  if ((tr_width > 0) && (tr_height > 0))
    /* Translators: TX is a common abbreviation for "transmit".  As it
     * is shown in a tooltip, there is no space constraint */
    stats_msg_tr = g_strdup_printf (_("TX: %dx%d"), tr_width, tr_height);
  else
    stats_msg_tr = g_strdup (_("TX: / "));

  if ((re_width > 0) && (re_height > 0))
    /* Translators: RX is a common abbreviation for "receive".  As it
     * is shown in a tooltip, there is no space constraint */
    stats_msg_re = g_strdup_printf (_("RX: %dx%d"), re_width, re_height);
  else
    stats_msg_re = g_strdup (_("RX: / "));

  if (!tr_audio_codec && !tr_video_codec)
    stats_msg_codecs = g_strdup (" ");
  else
    stats_msg_codecs = g_strdup_printf ("%s - %s",
                                        tr_audio_codec?tr_audio_codec:"",
                                        tr_video_codec?tr_video_codec:"");

  stats_msg = g_strdup_printf (_("Lost packets: %.1f %%\nLate packets: %.1f %%\nOut of order packets: %.1f %%\nJitter buffer: %d ms\nCodecs: %s\nResolution: %s %s"),
                               lost,
                               late,
                               out_of_order,
                               jitter,
                               stats_msg_codecs,
                               stats_msg_tr,
                               stats_msg_re);

  g_free(stats_msg_tr);
  g_free(stats_msg_re);
  g_free(stats_msg_codecs);

  gtk_widget_set_tooltip_text (GTK_WIDGET (self->priv->event_box), stats_msg);
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

  if ( (lost > 0.0) ||
       (late > 0.0) ||
       ((out_of_order > 0.0) && quality_level > 0.2) ) {
    quality_level = 0.2;
  }

  if ( (lost > 0.02) ||
       (late > 0.02) ||
       (out_of_order > 0.02) ) {
    quality_level = 0;
  }

  if (self->priv->qualitymeter)
    gm_powermeter_set_level (GM_POWERMETER (self->priv->qualitymeter),
                             quality_level);
}


static void
ekiga_call_window_set_bandwidth (EkigaCallWindow *self,
                                 float ta,
                                 float ra,
                                 float tv,
                                 float rv)
{
  gchar *msg = NULL;

  g_return_if_fail (EKIGA_IS_CALL_WINDOW (self));

  if (ta > 0.0 || ra > 0.0 || tv > 0.0 || rv > 0.0)
    /* Translators: A = Audio, V = Video */
    msg = g_strdup_printf (_("A:%.1f/%.1f V:%.1f/%.1f"),
                           ta, ra, tv, rv);

  if (msg)
    gm_statusbar_push_message (GM_STATUSBAR (self->priv->statusbar), "%s", msg);
  else
    gm_statusbar_push_message (GM_STATUSBAR (self->priv->statusbar), NULL);
  g_free (msg);
}


static GtkWidget *
gm_call_window_video_settings_window_new (EkigaCallWindow *self)
{
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *image = NULL;
  GtkWidget *window = NULL;

  GtkWidget *hscale_brightness = NULL;
  GtkWidget *hscale_colour = NULL;
  GtkWidget *hscale_contrast = NULL;
  GtkWidget *hscale_whiteness = NULL;

  int brightness = 0, colour = 0, contrast = 0, whiteness = 0;

  /* Build the window */
  window = gm_window_new_with_key (USER_INTERFACE ".video-settings-window");
  gtk_window_set_title (GTK_WINDOW (window), _("Video Settings"));

  /* Webcam Control Frame, we need it to disable controls */
  self->priv->video_settings_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (self->priv->video_settings_frame),
                             GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (self->priv->video_settings_frame), 5);

  /* Category */
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (self->priv->video_settings_frame), vbox);

  /* Brightness */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  image = gtk_image_new_from_icon_name ("brightness", GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, false, false, 0);

  self->priv->adj_brightness = gtk_adjustment_new (brightness, 0.0,
                                                 255.0, 1.0, 5.0, 1.0);
  hscale_brightness = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL,
                                     GTK_ADJUSTMENT (self->priv->adj_brightness));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_brightness), false);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_brightness), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_brightness, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 3);

  gtk_widget_set_tooltip_text (hscale_brightness, _("Adjust brightness"));

  g_signal_connect (self->priv->adj_brightness, "value-changed",
                    G_CALLBACK (video_settings_changed_cb),
                    (gpointer) self);

  /* Whiteness */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  image = gtk_image_new_from_icon_name ("whiteness", GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, false, false, 0);

  self->priv->adj_whiteness = gtk_adjustment_new (whiteness, 0.0,
                                                255.0, 1.0, 5.0, 1.0);
  hscale_whiteness = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL,
                                    GTK_ADJUSTMENT (self->priv->adj_whiteness));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_whiteness), false);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_whiteness), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_whiteness, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 3);

  gtk_widget_set_tooltip_text (hscale_whiteness, _("Adjust whiteness"));

  g_signal_connect (self->priv->adj_whiteness, "value-changed",
                    G_CALLBACK (video_settings_changed_cb),
                    (gpointer) self);

  /* Colour */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  image = gtk_image_new_from_icon_name ("color", GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, false, false, 0);

  self->priv->adj_colour = gtk_adjustment_new (colour, 0.0,
                                             255.0, 1.0, 5.0, 1.0);
  hscale_colour = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL,
                                 GTK_ADJUSTMENT (self->priv->adj_colour));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_colour), false);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_colour), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_colour, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 3);

  gtk_widget_set_tooltip_text (hscale_colour, _("Adjust color"));

  g_signal_connect (self->priv->adj_colour, "value-changed",
                    G_CALLBACK (video_settings_changed_cb),
                    (gpointer) self);

  /* Contrast */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  image = gtk_image_new_from_icon_name ("contrast", GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, false, false, 0);

  self->priv->adj_contrast = gtk_adjustment_new (contrast, 0.0,
                                               255.0, 1.0, 5.0, 1.0);
  hscale_contrast = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL,
                                   GTK_ADJUSTMENT (self->priv->adj_contrast));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_contrast), false);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_contrast), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_contrast, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 3);

  gtk_widget_set_tooltip_text (hscale_contrast, _("Adjust contrast"));

  g_signal_connect (self->priv->adj_contrast, "value-changed",
                    G_CALLBACK (video_settings_changed_cb),
                    (gpointer) self);

  gtk_container_add (GTK_CONTAINER (window),
                     self->priv->video_settings_frame);
  gtk_widget_show_all (self->priv->video_settings_frame);

  gtk_widget_set_sensitive (GTK_WIDGET (self->priv->video_settings_frame), false);

  gtk_widget_hide_on_delete (window);

  return window;
}

static GtkWidget *
gm_call_window_audio_settings_window_new (EkigaCallWindow *self)
{
  GtkWidget *hscale_play = NULL;
  GtkWidget *hscale_rec = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *main_vbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *small_vbox = NULL;
  GtkWidget *window = NULL;

  /* Build the window */
  window = gm_window_new_with_key (USER_INTERFACE ".audio-settings-window");
  gtk_window_set_title (GTK_WINDOW (window), _("Audio Settings"));

  /* Audio control frame, we need it to disable controls */
  self->priv->audio_output_volume_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (self->priv->audio_output_volume_frame),
                             GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (self->priv->audio_output_volume_frame), 5);
  main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

  /* The vbox */
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (self->priv->audio_output_volume_frame), vbox);

  /* Output volume */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start (GTK_BOX (hbox),
                      gtk_image_new_from_icon_name ("audio-volume", GTK_ICON_SIZE_SMALL_TOOLBAR),
                      false, false, 0);

  small_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  self->priv->adj_output_volume = gtk_adjustment_new (0, 0.0, 101.0, 1.0, 5.0, 1.0);
  hscale_play = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL,
                               GTK_ADJUSTMENT (self->priv->adj_output_volume));
  gtk_scale_set_value_pos (GTK_SCALE (hscale_play), GTK_POS_RIGHT);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_play), false);
  gtk_box_pack_start (GTK_BOX (small_vbox), hscale_play, true, true, 0);

  //self->priv->output_signal = gm_level_meter_new ();
  //gtk_box_pack_start (GTK_BOX (small_vbox), self->priv->output_signal, true, true, 0);
  gtk_box_pack_start (GTK_BOX (hbox), small_vbox, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 3);

  gtk_box_pack_start (GTK_BOX (main_vbox), self->priv->audio_output_volume_frame,
                      false, false, 0);
  gtk_widget_set_sensitive (GTK_WIDGET (self->priv->audio_output_volume_frame),  false);

  /* Audio control frame, we need it to disable controls */
  self->priv->audio_input_volume_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (self->priv->audio_input_volume_frame),
                             GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (self->priv->audio_input_volume_frame), 5);

  /* The vbox */
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (self->priv->audio_input_volume_frame), vbox);

  /* Input volume */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start (GTK_BOX (hbox),
                      gtk_image_new_from_icon_name ("audio-input-microphone",
                                                    GTK_ICON_SIZE_SMALL_TOOLBAR),
                      false, false, 0);

  small_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  self->priv->adj_input_volume = gtk_adjustment_new (0, 0.0, 101.0, 1.0, 5.0, 1.0);
  hscale_rec = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL,
                              GTK_ADJUSTMENT (self->priv->adj_input_volume));
  gtk_scale_set_value_pos (GTK_SCALE (hscale_rec), GTK_POS_RIGHT);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_rec), false);
  gtk_box_pack_start (GTK_BOX (small_vbox), hscale_rec, true, true, 0);

  //  self->priv->input_signal = gm_level_meter_new ();
  //  gtk_box_pack_start (GTK_BOX (small_vbox), self->priv->input_signal, true, true, 0);
  gtk_box_pack_start (GTK_BOX (hbox), small_vbox, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 3);

  gtk_box_pack_start (GTK_BOX (main_vbox), self->priv->audio_input_volume_frame,
                      false, false, 0);
  gtk_widget_set_sensitive (GTK_WIDGET (self->priv->audio_input_volume_frame),  false);

  gtk_container_add (GTK_CONTAINER (window), main_vbox);
  gtk_widget_show_all (main_vbox);

  g_signal_connect (self->priv->adj_output_volume, "value-changed",
                    G_CALLBACK (audio_volume_changed_cb), self);

  g_signal_connect (self->priv->adj_input_volume, "value-changed",
                    G_CALLBACK (audio_volume_changed_cb), self);

  gtk_widget_hide_on_delete (window);

  g_signal_connect (window, "show",
                    G_CALLBACK (audio_volume_window_shown_cb), self);

  g_signal_connect (window, "hide",
                    G_CALLBACK (audio_volume_window_hidden_cb), self);

  return window;
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
    gtk_widget_hide (self->priv->statusbar_ebox);
    gtk_window_maximize (GTK_WINDOW (self));
    gtk_window_fullscreen (GTK_WINDOW (self));
    gm_video_widget_set_fullscreen (GM_VIDEO_WIDGET (self->priv->video_widget), true);
    gtk_window_set_keep_above (GTK_WINDOW (self), true);
  }
  else {
    gtk_widget_show (self->priv->call_panel_toolbar);
    gtk_widget_show (self->priv->statusbar_ebox);
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
  conn = self->priv->call_core->setup_call.connect (boost::bind (&on_setup_call_cb, _1, _2, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->call_core->ringing_call.connect (boost::bind (&on_ringing_call_cb, _1, _2, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->call_core->established_call.connect (boost::bind (&on_established_call_cb, _1, _2, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->call_core->cleared_call.connect (boost::bind (&on_cleared_call_cb, _1, _2, _3, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->call_core->missed_call.connect (boost::bind (&on_missed_call_cb, _1, _2, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->call_core->held_call.connect (boost::bind (&on_held_call_cb, _1, _2, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->call_core->retrieved_call.connect (boost::bind (&on_retrieved_call_cb, _1, _2, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->call_core->stream_opened.connect (boost::bind (&on_stream_opened_cb, _1, _2, _3, _4, _5, (gpointer) self));
  self->priv->connections.add (conn);

  conn = self->priv->call_core->stream_closed.connect (boost::bind (&on_stream_closed_cb, _1, _2, _3, _4, _5, (gpointer) self));
  self->priv->connections.add (conn);
}

static void
ekiga_call_window_init_gui (EkigaCallWindow *self)
{
  GtkWidget *event_box = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *button = NULL;

  GtkWidget *image = NULL;
  GtkWidget *alignment = NULL;

  GtkShadowType shadow_type;

  /* The Audio & Video Settings windows */
  self->priv->audio_settings_window = gm_call_window_audio_settings_window_new (self);
  self->priv->video_settings_window = gm_call_window_video_settings_window_new (self);

  /* The extended video stream window */
  self->priv->ext_video_win = NULL;

  /* The main table */
  event_box = gtk_event_box_new ();
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
  event_box = gtk_event_box_new ();
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
  gtk_header_bar_set_title (GTK_HEADER_BAR (self->priv->call_panel_toolbar),
                            _("Call Window"));

  /* The frame that contains the video */
  self->priv->event_box = gtk_event_box_new ();
  ekiga_call_window_init_clutter (self);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (self->priv->event_box), true, true, 0);
  gtk_widget_show_all (self->priv->event_box);

  /* The frame that contains information about the call */
  self->priv->call_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (self->priv->call_frame), GTK_SHADOW_NONE);
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  self->priv->camera_image = gtk_image_new_from_icon_name ("camera-web", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_box_pack_start (GTK_BOX (hbox), self->priv->camera_image, false, false, 12);

  self->priv->spinner = gtk_spinner_new ();
  gtk_widget_set_size_request (GTK_WIDGET (self->priv->spinner), 24, 24);
  gtk_box_pack_start (GTK_BOX (hbox), self->priv->spinner, false, false, 12);

  self->priv->info_text = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (self->priv->info_text), false);
  gtk_widget_set_sensitive (GTK_WIDGET (self->priv->info_text), false);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (self->priv->info_text),
                               GTK_WRAP_NONE);
  gtk_text_view_set_cursor_visible  (GTK_TEXT_VIEW (self->priv->info_text), false);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (alignment), self->priv->info_text);
  gtk_box_pack_start (GTK_BOX (hbox), alignment, false, false, 2);
  gtk_container_add (GTK_CONTAINER (self->priv->call_frame), hbox);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (self->priv->call_frame), true, true, 2);
  gtk_widget_show_all (self->priv->call_frame);
  gtk_widget_hide (self->priv->spinner);

  /* FIXME:
   * All those actions should be call specific.
   * We should generate the header bar actions like we generate a menu.
   * Probably introducing a GActorHeaderBar would be nice.
   * However, it is unneeded right now as we only support one call at a time.
   */

  /* Hang up */
  button = gtk_button_new ();
  image = gtk_image_new_from_icon_name ("call-end-symbolic", GTK_ICON_SIZE_MENU);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "win.hangup");
  gtk_header_bar_pack_start (GTK_HEADER_BAR (self->priv->call_panel_toolbar), button);
  gtk_widget_set_tooltip_text (GTK_WIDGET (button),
                               _("Hang up the current call"));

  /* Call Hold */
  button = gtk_toggle_button_new ();
  image = gtk_image_new_from_icon_name ("media-playback-pause-symbolic", GTK_ICON_SIZE_MENU);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "win.hold");
  gtk_header_bar_pack_start (GTK_HEADER_BAR (self->priv->call_panel_toolbar), button);
  gtk_widget_set_tooltip_text (GTK_WIDGET (button),
                               _("Hold or retrieve the current call"));

  /* Call Transfer */
  button = gtk_button_new ();
  image = gtk_image_new_from_icon_name ("send-to-symbolic", GTK_ICON_SIZE_MENU);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "win.transfer");
  gtk_header_bar_pack_start (GTK_HEADER_BAR (self->priv->call_panel_toolbar), button);
  gtk_widget_set_tooltip_text (GTK_WIDGET (button),
                               _("Transfer the current call"));

  /* Menu button */
  button = gtk_menu_button_new ();
  g_object_set (G_OBJECT (button), "use-popover", true, NULL);
  image = gtk_image_new_from_icon_name ("document-properties-symbolic", GTK_ICON_SIZE_MENU);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (button),
                                  G_MENU_MODEL (gtk_builder_get_object (self->priv->builder, "menubar")));
  gtk_header_bar_pack_end (GTK_HEADER_BAR (self->priv->call_panel_toolbar), button);

  /* Full Screen */
  button = gtk_button_new ();
  image = gtk_image_new_from_icon_name ("view-fullscreen-symbolic", GTK_ICON_SIZE_MENU);
  gtk_button_set_image (GTK_BUTTON (button), image);
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "win.enable-fullscreen");
  gtk_header_bar_pack_end (GTK_HEADER_BAR (self->priv->call_panel_toolbar), button);
  gtk_widget_set_tooltip_text (GTK_WIDGET (button),
                               _("Switch to fullscreen"));

  gtk_widget_show_all (self->priv->call_panel_toolbar);


  /* The statusbar */
  self->priv->statusbar = gm_statusbar_new ();
  gtk_widget_style_get (self->priv->statusbar, "shadow-type", &shadow_type, NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), shadow_type);
  gtk_box_pack_start (GTK_BOX (self->priv->statusbar), frame, false, false, 0);
  gtk_box_reorder_child (GTK_BOX (self->priv->statusbar), frame, 0);

  self->priv->qualitymeter = gm_powermeter_new ();
  gtk_container_add (GTK_CONTAINER (frame), self->priv->qualitymeter);

  self->priv->statusbar_ebox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (self->priv->statusbar_ebox), self->priv->statusbar);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (self->priv->statusbar_ebox), false, false, 0);
  gtk_widget_show_all (self->priv->statusbar_ebox);

  gtk_window_set_resizable (GTK_WINDOW (self), true);

  /* Init */
  ekiga_call_window_set_bandwidth (self, 0.0, 0.0, 0.0, 0.0);

  gtk_widget_hide (self->priv->call_frame);
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
  self->priv->levelmeter_timeout_id = -1;
  self->priv->calling_state = Standby;
  self->priv->fullscreen = false;
#ifndef WIN32
  self->priv->gc = NULL;
#endif
  self->priv->video_display_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (VIDEO_DISPLAY_SCHEMA));

  g_signal_connect (self, "delete_event",
                    G_CALLBACK (ekiga_call_window_delete_event_cb), NULL);
}

static void
ekiga_call_window_finalize (GObject *gobject)
{
  EkigaCallWindow *self = EKIGA_CALL_WINDOW (gobject);

  gtk_widget_destroy (self->priv->audio_settings_window);
  gtk_widget_destroy (self->priv->video_settings_window);
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

  self->priv->libnotify = core->get ("libnotify");
  self->priv->videoinput_core = core->get<Ekiga::VideoInputCore> ("videoinput-core");
  self->priv->videooutput_core = core->get<Ekiga::VideoOutputCore> ("videooutput-core");
  self->priv->audioinput_core = core->get<Ekiga::AudioInputCore> ("audioinput-core");
  self->priv->audiooutput_core = core->get<Ekiga::AudioOutputCore> ("audiooutput-core");
  self->priv->call_core = core->get<Ekiga::CallCore> ("call-core");

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
