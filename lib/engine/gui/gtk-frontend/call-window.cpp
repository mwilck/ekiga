
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

#include <clutter-gtk/clutter-gtk.h>

#include "config.h"

#include "ekiga-settings.h"

#include "call-window.h"

#include "dialpad.h"

#include "gmdialog.h"
#include "gmentrydialog.h"
#include "gmstatusbar.h"
#include "gmstockicons.h"
#include <boost/smart_ptr.hpp>
#include "gmmenuaddon.h"
#include "gmpowermeter.h"
#include "trigger.h"
#include "scoped-connections.h"
#include "menu-builder-tools.h"
#include "menu-builder-gtk.h"
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

#define STAGE_WIDTH 640
#define STAGE_HEIGHT 480
#define LOCAL_VIDEO_MARGIN 12
#define LOCAL_VIDEO_RATIO 0.25
#define EMBLEM_MARGIN 12
#define EMBLEM_RATIO 0.15

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
  GtkWidget *main_video_image; //FIXME
  GtkWidget *spinner;
  GtkWidget *info_text;

  ClutterActor *stage;

  ClutterActor *local_video;
  unsigned local_video_natural_width;
  unsigned local_video_natural_height;

  ClutterActor *remote_video;
  unsigned remote_video_natural_width;
  unsigned remote_video_natural_height;

  bool fullscreen;

  GtkWidget *call_frame;
  GtkWidget *camera_image;

  GtkWidget *main_menu;
  GtkWidget *call_panel_toolbar;
  GtkWidget *pick_up_button;
  GtkWidget *hang_up_button;
  GtkWidget *hold_button;
  GtkWidget *audio_settings_button;
  GtkWidget *video_settings_button;

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


  /* Statusbar */
  GtkWidget *statusbar;
  GtkWidget *statusbar_ebox;
  GtkWidget *qualitymeter;

  /* The problem is the following :
   * without that boolean, changing the ui will trigger a callback,
   * which will store in the settings that the user wants only local
   * video... in fact the problem is that we use a single ui+settings
   * to mean both "what the user wants during a call" and "what we are
   * doing right now".
   *
   * So we set that boolean to true,
   * ask the ui to change,
   * notice that we don't want to update the settings
   * set the boolean to false...
   * then run as usual.
   */
  bool changing_back_to_local_after_a_call;

  bool automatic_zoom_in;

  GtkWidget *transfer_call_popup;

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

static bool notify_has_actions (EkigaCallWindow* cw);

static void zoom_in_changed_cb (GtkWidget *widget,
                                gpointer data);

static void zoom_out_changed_cb (GtkWidget *widget,
                                 gpointer data);

static void zoom_normal_changed_cb (GtkWidget *widget,
                                    gpointer data);

static void display_changed_cb (GtkWidget *widget,
                                gpointer data);

static void fullscreen_changed_cb (GtkWidget *widget,
                                   gpointer data);

static void stay_on_top_changed_cb (GSettings *settings,
                                    gchar *key,
                                    gpointer self);

static void pick_up_call_cb (GtkWidget * /*widget*/,
                            gpointer data);

static void hang_up_call_cb (GtkWidget * /*widget*/,
                            gpointer data);

static void hold_current_call_cb (GtkWidget *widget,
                                  gpointer data);

static void toggle_audio_stream_pause_cb (GtkWidget * /*widget*/,
                                          gpointer data);

static void toggle_video_stream_pause_cb (GtkWidget * /*widget*/,
                                          gpointer data);

static void transfer_current_call_cb (GtkWidget *widget,
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
                                             bool both_streams,
                                             bool ext_stream,
                                             gpointer self);

static void on_videooutput_device_closed_cb (Ekiga::VideoOutputManager & /* manager */,
                                             gpointer self);

static void on_videooutput_device_error_cb (Ekiga::VideoOutputManager & /* manager */,
                                            Ekiga::VideoOutputErrorCodes error_code,
                                            gpointer self);

static void ekiga_call_window_set_video_size (EkigaCallWindow *cw,
                                              int width,
                                              int height);

static void on_size_changed_cb (Ekiga::VideoOutputManager & /* manager */,
                                unsigned width,
                                unsigned height,
                                unsigned type,
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

static void on_setup_call_cb (boost::shared_ptr<Ekiga::CallManager> manager,
                              boost::shared_ptr<Ekiga::Call>  call,
                              gpointer self);

static void on_ringing_call_cb (boost::shared_ptr<Ekiga::CallManager> manager,
                                boost::shared_ptr<Ekiga::Call>  call,
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

static void on_stream_paused_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                                 boost::shared_ptr<Ekiga::Call>  /*call*/,
                                 std::string /*name*/,
                                 Ekiga::Call::StreamType type,
                                 gpointer self);


static void on_stream_resumed_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                                  boost::shared_ptr<Ekiga::Call>  /*call*/,
                                  std::string /*name*/,
                                  Ekiga::Call::StreamType type,
                                  gpointer self);

static gboolean on_stats_refresh_cb (gpointer self);

static gboolean ekiga_call_window_delete_event_cb (GtkWidget *widget,
                                                   G_GNUC_UNUSED GdkEventAny *event);

static gboolean ekiga_call_window_fullscreen_event_cb (GtkWidget *widget,
                                                       G_GNUC_UNUSED GdkEventAny *event);

static void window_closed_from_menu_cb (G_GNUC_UNUSED GtkWidget *,
                                        gpointer);

static void animate_logo_cb (ClutterActor *actor,
                             gpointer logo);

static void resize_actor_cb (ClutterActor *actor,
                             ClutterActorBox *box,
                             ClutterAllocationFlags flags,
                             gpointer data);

/**/
static void resize_actor (ClutterActor *texture,
                          unsigned natural_width,
                          unsigned natural_height,
                          unsigned available_height,
                          unsigned easing_delay = 0);

static void ekiga_call_window_update_calling_state (EkigaCallWindow *cw,
                                                    unsigned calling_state);

static void ekiga_call_window_clear_signal_levels (EkigaCallWindow *cw);

static void ekiga_call_window_clear_stats (EkigaCallWindow *cw);

static void ekiga_call_window_update_stats (EkigaCallWindow *cw,
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

static void ekiga_call_window_set_status (EkigaCallWindow *cw,
                                          const char *status,
                                          ...);

static void ekiga_call_window_set_bandwidth (EkigaCallWindow *cw,
                                             float ta,
                                             float ra,
                                             float tv,
                                             float rv,
                                             int tfps,
                                             int rfps);

static void ekiga_call_window_set_call_hold (EkigaCallWindow *cw,
                                             bool is_on_hold);

static void ekiga_call_window_set_channel_pause (EkigaCallWindow *cw,
                                                 gboolean pause,
                                                 gboolean is_video);

static void ekiga_call_window_init_menu (EkigaCallWindow *cw);

static void ekiga_call_window_init_clutter (EkigaCallWindow *cw);

static GtkWidget * gm_cw_audio_settings_window_new (EkigaCallWindow *cw);

static GtkWidget *gm_cw_video_settings_window_new (EkigaCallWindow *cw);

static void ekiga_call_window_update_logo (EkigaCallWindow *cw);

static void ekiga_call_window_toggle_fullscreen (EkigaCallWindow *cw);

static void ekiga_call_window_zooms_menu_update_sensitivity (EkigaCallWindow *cw,
                                                             unsigned int zoom);

static void ekiga_call_window_channels_menu_update_sensitivity (EkigaCallWindow *cw,
                                                                bool is_video,
                                                                bool is_transmitting);

static gboolean ekiga_call_window_transfer_dialog_run (EkigaCallWindow *cw,
                                                       GtkWidget *parent_window,
                                                       const char *u);

static void ekiga_call_window_connect_engine_signals (EkigaCallWindow *cw);

static void ekiga_call_window_init_gui (EkigaCallWindow *cw);

static bool
notify_has_actions (EkigaCallWindow *cw)
{
  bool result = false;

  if (cw->priv->libnotify) {

    boost::optional<bool> val = cw->priv->libnotify->get_bool_property ("actions");
    if (val) {

      result = *val;
    }
  }
  return result;
}

static void
stay_on_top_changed_cb (GSettings *settings,
                        gchar *key,
                        gpointer self)

{
  bool val = false;

  g_return_if_fail (self != NULL);

  val = g_settings_get_boolean (settings, key);
  gdk_window_set_keep_above (GDK_WINDOW (gtk_widget_get_window (GTK_WIDGET (self))), val);
}


static void
zoom_in_changed_cb (G_GNUC_UNUSED GtkWidget *widget,
		    gpointer data)
{
  g_return_if_fail (data != NULL);

  Ekiga::DisplayInfo display_info;
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (data);

  ekiga_call_window_set_video_size (cw, GM_CIF_WIDTH, GM_CIF_HEIGHT);

  display_info.zoom = cw->priv->video_display_settings->get_int ("zoom");

  if (display_info.zoom < 200)
    display_info.zoom = display_info.zoom * 2;

  cw->priv->video_display_settings->set_int ("zoom", display_info.zoom);
  ekiga_call_window_zooms_menu_update_sensitivity (cw, display_info.zoom);
}

static void
zoom_out_changed_cb (G_GNUC_UNUSED GtkWidget *widget,
		     gpointer data)
{
  g_return_if_fail (data != NULL);

  Ekiga::DisplayInfo display_info;
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (data);

  ekiga_call_window_set_video_size (cw, GM_CIF_WIDTH, GM_CIF_HEIGHT);

  display_info.zoom = cw->priv->video_display_settings->get_int ( "zoom");

  if (display_info.zoom  > 50)
    display_info.zoom  = (unsigned int) (display_info.zoom  / 2);

  cw->priv->video_display_settings->set_int ("zoom", display_info.zoom);
  ekiga_call_window_zooms_menu_update_sensitivity (cw, display_info.zoom);
}

static void
zoom_normal_changed_cb (G_GNUC_UNUSED GtkWidget *widget,
			gpointer data)
{
  g_return_if_fail (data != NULL);

  Ekiga::DisplayInfo display_info;
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (data);

  ekiga_call_window_set_video_size (cw, GM_CIF_WIDTH, GM_CIF_HEIGHT);

  display_info.zoom  = 100;

  cw->priv->video_display_settings->set_int ("zoom", display_info.zoom);
  ekiga_call_window_zooms_menu_update_sensitivity (cw, display_info.zoom);
}

static void
display_changed_cb (GtkWidget *widget,
		    gpointer data)
{
  g_return_if_fail (data != NULL);

  GSList *group = NULL;
  int group_last_pos = 0;
  int active = 0;
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (data);

  group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (widget));
  group_last_pos = g_slist_length (group) - 1; /* If length 1, last pos is 0 */

  /* Only do something when a new CHECK_MENU_ITEM becomes active,
     not when it becomes inactive */
  if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))) {

    while (group) {
      if (group->data == widget)
        break;

      active++;
      group = g_slist_next (group);
    }

    if (!cw->priv->changing_back_to_local_after_a_call) {
      int view = group_last_pos - active;
      if (view > 2) /* let's skip VO_MODE_PIP_WINDOW & VO_MODE_FULLSCREEN modes
                       which are not found in the View menu */
        view += 2;
      cw->priv->video_display_settings->set_int ("video-view", view);
    }
  }
}

static void
fullscreen_changed_cb (G_GNUC_UNUSED GtkWidget *widget,
		       gpointer data)
{
  g_return_if_fail (data);

  ekiga_call_window_toggle_fullscreen (EKIGA_CALL_WINDOW (data));
}

static void
pick_up_call_cb (GtkWidget * /*widget*/,
                gpointer data)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (data);

  if (cw->priv->current_call)
    cw->priv->current_call->answer ();
}

static void
hang_up_call_cb (GtkWidget * /*widget*/,
                gpointer data)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (data);

  if (cw->priv->current_call)
    cw->priv->current_call->hang_up ();

  if (cw->priv->ext_video_win) {
    ekiga_ext_window_destroy (EKIGA_EXT_WINDOW (cw->priv->ext_video_win));
  }
}


static void
hold_current_call_cb (G_GNUC_UNUSED GtkWidget *widget,
                      gpointer data)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (data);

  if (cw->priv->current_call)
    cw->priv->current_call->toggle_hold ();
}

static void
toggle_audio_stream_pause_cb (GtkWidget * /*widget*/,
                              gpointer data)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (data);

  if (cw->priv->current_call)
    cw->priv->current_call->toggle_stream_pause (Ekiga::Call::Audio);
}

static void
toggle_video_stream_pause_cb (GtkWidget * /*widget*/,
                              gpointer data)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (data);

  if (cw->priv->current_call)
    cw->priv->current_call->toggle_stream_pause (Ekiga::Call::Video);
}

static void
transfer_current_call_cb (G_GNUC_UNUSED GtkWidget *widget,
			  gpointer data)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (data);

  g_return_if_fail (data != NULL);
  ekiga_call_window_transfer_dialog_run (EKIGA_CALL_WINDOW (cw), GTK_WIDGET (data), NULL);
}

static void
audio_volume_changed_cb (GtkAdjustment * /*adjustment*/,
			 gpointer data)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (data);

  cw->priv->audiooutput_core->set_volume (Ekiga::primary, (unsigned) gtk_adjustment_get_value (GTK_ADJUSTMENT (cw->priv->adj_output_volume)));
  cw->priv->audioinput_core->set_volume ((unsigned) gtk_adjustment_get_value (GTK_ADJUSTMENT (cw->priv->adj_input_volume)));
}

static void
audio_volume_window_shown_cb (GtkWidget * /*widget*/,
	                      gpointer data)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (data);

  cw->priv->audioinput_core->set_average_collection (true);
  cw->priv->audiooutput_core->set_average_collection (true);
  cw->priv->levelmeter_timeout_id = g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, 50, on_signal_level_refresh_cb, data, NULL);
}

static void
audio_volume_window_hidden_cb (GtkWidget * /*widget*/,
                               gpointer data)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (data);

  g_source_remove (cw->priv->levelmeter_timeout_id);
  cw->priv->audioinput_core->set_average_collection (false);
  cw->priv->audiooutput_core->set_average_collection (false);
}

static void
video_settings_changed_cb (GtkAdjustment * /*adjustment*/,
			   gpointer data)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (data);


  cw->priv->videoinput_core->set_whiteness ((unsigned) gtk_adjustment_get_value (GTK_ADJUSTMENT (cw->priv->adj_whiteness)));
  cw->priv->videoinput_core->set_brightness ((unsigned) gtk_adjustment_get_value (GTK_ADJUSTMENT (cw->priv->adj_brightness)));
  cw->priv->videoinput_core->set_colour ((unsigned) gtk_adjustment_get_value (GTK_ADJUSTMENT (cw->priv->adj_colour)));
  cw->priv->videoinput_core->set_contrast ((unsigned) gtk_adjustment_get_value (GTK_ADJUSTMENT (cw->priv->adj_contrast)));
}

static gboolean
on_signal_level_refresh_cb (gpointer /*self*/)
{
  //EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  //gm_level_meter_set_level (GM_LEVEL_METER (cw->priv->output_signal), cw->priv->audiooutput_core->get_average_level());
  //gm_level_meter_set_level (GM_LEVEL_METER (cw->priv->input_signal), cw->priv->audioinput_core->get_average_level());
  return true;
}

static void
on_videooutput_device_opened_cb (Ekiga::VideoOutputManager & /* manager */,
                                 bool both_streams,
                                 bool ext_stream,
                                 gpointer self)
{
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (self));

  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  if (both_streams) {
    clutter_actor_save_easing_state (CLUTTER_ACTOR (cw->priv->local_video));
    clutter_actor_set_easing_duration (CLUTTER_ACTOR (cw->priv->local_video), 2000);
    clutter_actor_set_opacity (CLUTTER_ACTOR (cw->priv->local_video), 255);
    clutter_actor_restore_easing_state (CLUTTER_ACTOR (cw->priv->local_video));
  }

  // FIXME
  return;
  /*
  int vv;

  if (both_streams) {
    gtk_menu_section_set_sensitive (cw->priv->main_menu, "local_video", true);
    gtk_menu_section_set_sensitive (cw->priv->main_menu, "fullscreen", true);
  }
  else {
    if (mode == Ekiga::VO_MODE_LOCAL)
      gtk_menu_set_sensitive (cw->priv->main_menu, "local_video", true);
    else if (mode == Ekiga::VO_MODE_REMOTE)
      gtk_menu_set_sensitive (cw->priv->main_menu, "remote_video", true);
  }

  if (cw->priv->ext_video_win && ext_stream) {
    gtk_widget_show_now (cw->priv->ext_video_win);
  }

  // when ending a call and going back to local video, the video_view
  // setting should not be updated, so memorise the setting and
  // restore it afterwards
  vv = cw->priv->video_display_settings->get_int ("video-view");
  cw->priv->changing_back_to_local_after_a_call = true;
  gtk_radio_menu_select_with_id (cw->priv->main_menu, "local_video", mode);
  cw->priv->changing_back_to_local_after_a_call = false;
  if (!both_streams && mode != Ekiga::VO_MODE_LOCAL)
    cw->priv->video_display_settings->set_int ("video-view", Ekiga::VO_MODE_LOCAL);

  // if in a past video we left in the extended video stream, but the new
  // one doesn't have it, we reset the view to the local one
  if (vv == Ekiga::VO_MODE_REMOTE_EXT && !ext_stream)
    cw->priv->video_display_settings->set_int ("video-view", Ekiga::VO_MODE_LOCAL);

  ekiga_call_window_zooms_menu_update_sensitivity (cw, zoom);
  */
}

static void
on_videooutput_device_closed_cb (Ekiga::VideoOutputManager & /* manager */, gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);
  //FIXME
}

//FIXME Set_stay_on_top "window_show object"

static void
on_videooutput_device_error_cb (Ekiga::VideoOutputManager & /* manager */,
                                Ekiga::VideoOutputErrorCodes error_code,
                                gpointer self)
{
  GtkWidget *dialog = NULL;

  const gchar *dialog_title =  _("Error while initializing video output");
  const gchar *tmp_msg = _("No video will be displayed on your machine during this call");
  gchar *dialog_msg = NULL;

  switch (error_code) {

    case Ekiga::VO_ERROR_NONE:
      break;
    case Ekiga::VO_ERROR:
    default:
      dialog_msg = g_strconcat (_("There was an error opening or initializing the video output. Please verify that no other application is using the accelerated video output."), "\n\n", tmp_msg, NULL);
      break;
  }

  dialog = gtk_message_dialog_new (GTK_WINDOW (self), GTK_DIALOG_MODAL,
                                   GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                   dialog_msg);
  gtk_window_set_title (GTK_WINDOW (dialog), dialog_title);
  g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
  gtk_widget_show_all (GTK_WIDGET (dialog));

  g_free (dialog_msg);
}


static void
ekiga_call_window_set_video_size (EkigaCallWindow *cw,
                                  int width,
                                  int height)
{
  int pw, ph;
  GdkRectangle a;

  g_return_if_fail (width > 0 && height > 0);

  if (width < GM_CIF_WIDTH && height < GM_CIF_HEIGHT && !cw->priv->automatic_zoom_in) {
    cw->priv->automatic_zoom_in = true;
    zoom_in_changed_cb (NULL, (gpointer) cw);
  }

  gtk_widget_get_size_request (cw->priv->event_box, &pw, &ph);

  /* No size requisition yet
   * It's our first call so we silently set the new requisition and exit...
   */
  if (pw == -1) {
    gtk_widget_set_size_request (cw->priv->event_box, width, height);
    return;
  }

  /* Do some kind of filtering here. We often get duplicate "size-changed" events...
   * Note that we currently only bother about the width of the video.
   */
  if (pw == width)
    return;

  gtk_widget_set_size_request (cw->priv->event_box, width, height);

  gtk_widget_get_allocation (GTK_WIDGET (cw), &a);
  gdk_window_invalidate_rect (gtk_widget_get_window (GTK_WIDGET (cw)), &a, true);
}

static void
on_size_changed_cb (Ekiga::VideoOutputManager & /* manager */,
                    unsigned width,
                    unsigned height,
                    unsigned type,
                    gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  /* Resize the clutter texture when we know the natural video size.
   * The real size will depend on the stage available space.
   */
  switch (type) {
    case 0:
      cw->priv->local_video_natural_width = width;
      cw->priv->local_video_natural_height = height;
      resize_actor (CLUTTER_ACTOR (cw->priv->local_video),
                    width,
                    height,
                    clutter_actor_get_height (CLUTTER_ACTOR (cw->priv->stage)) * LOCAL_VIDEO_RATIO,
                    1000);
      break;

    case 1:
      cw->priv->remote_video_natural_width = width;
      cw->priv->remote_video_natural_height = height;
      resize_actor (CLUTTER_ACTOR (cw->priv->remote_video),
                    width,
                    height,
                    clutter_actor_get_height (CLUTTER_ACTOR (cw->priv->stage)),
                    1000);
      break;

    default:
      break;
  }

  gtk_widget_show (GTK_WIDGET (cw));
}

static void
on_videoinput_device_opened_cb (Ekiga::VideoInputManager & /* manager */,
                                Ekiga::VideoInputDevice & /* device */,
                                Ekiga::VideoInputSettings & settings,
                                gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  gtk_widget_set_sensitive (cw->priv->video_settings_frame,  settings.modifyable ? true : false);
  gtk_widget_set_sensitive (cw->priv->video_settings_button,  settings.modifyable ? true : false);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (cw->priv->adj_whiteness), settings.whiteness);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (cw->priv->adj_brightness), settings.brightness);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (cw->priv->adj_colour), settings.colour);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (cw->priv->adj_contrast), settings.contrast);

  gtk_widget_queue_draw (cw->priv->video_settings_frame);
}

static void
on_videoinput_device_closed_cb (Ekiga::VideoInputManager & /* manager */,
                                Ekiga::VideoInputDevice & /*device*/,
                                gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  ekiga_call_window_channels_menu_update_sensitivity (cw, true, false);
  ekiga_call_window_update_logo (cw);

  gtk_widget_set_sensitive (cw->priv->video_settings_button,  false);
  if (cw->priv->automatic_zoom_in) {
    cw->priv->automatic_zoom_in = false;
    zoom_out_changed_cb (NULL, (gpointer) cw);
  }
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
                                gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  gtk_widget_set_sensitive (cw->priv->audio_input_volume_frame, settings.modifyable);
  if (cw->priv->audio_settings_button)
    gtk_widget_set_sensitive (cw->priv->audio_settings_button, settings.modifyable);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (cw->priv->adj_input_volume), settings.volume);

  gtk_widget_queue_draw (cw->priv->audio_input_volume_frame);
}

static void
on_audioinput_device_closed_cb (Ekiga::AudioInputManager & /* manager */,
                                Ekiga::AudioInputDevice & /*device*/,
                                gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  if (cw->priv->audio_settings_button)
    gtk_widget_set_sensitive (cw->priv->audio_settings_button, false);
  gtk_widget_set_sensitive (cw->priv->audio_input_volume_frame, false);
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
                                 gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  if (ps == Ekiga::secondary)
    return;

  if (cw->priv->audio_settings_button)
    gtk_widget_set_sensitive (cw->priv->audio_settings_button, settings.modifyable);
  gtk_widget_set_sensitive (cw->priv->audio_output_volume_frame, settings.modifyable);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (cw->priv->adj_output_volume), settings.volume);

  gtk_widget_queue_draw (cw->priv->audio_output_volume_frame);
}

static void
on_audiooutput_device_closed_cb (Ekiga::AudioOutputManager & /*manager*/,
                                 Ekiga::AudioOutputPS ps,
                                 Ekiga::AudioOutputDevice & /*device*/,
                                 gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  if (ps == Ekiga::secondary)
    return;

  if (cw->priv->audio_settings_button)
    gtk_widget_set_sensitive (cw->priv->audio_settings_button, false);
  gtk_widget_set_sensitive (cw->priv->audio_output_volume_frame, false);
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
on_setup_call_cb (G_GNUC_UNUSED boost::shared_ptr<Ekiga::CallManager> manager,
                  boost::shared_ptr<Ekiga::Call>  call,
                  gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  if (!call->is_outgoing () && !manager->get_auto_answer ()) {
    if (cw->priv->current_call)
      return; // No call setup needed if already in a call

    cw->priv->current_call = call;
    cw->priv->calling_state = Called;
  }
  else {

    cw->priv->current_call = call;
    cw->priv->calling_state = Calling;
  }

  gtk_window_set_title (GTK_WINDOW (cw), call->get_remote_uri ().c_str ());

  if (call->is_outgoing ())
    ekiga_call_window_set_status (cw, _("Calling %s..."), call->get_remote_uri ().c_str ());

  ekiga_call_window_update_calling_state (cw, cw->priv->calling_state);
}

static void
on_ringing_call_cb (G_GNUC_UNUSED boost::shared_ptr<Ekiga::CallManager> manager,
                    G_GNUC_UNUSED boost::shared_ptr<Ekiga::Call>  call,
                    gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  g_return_if_fail (cw);

  cw->priv->calling_state = Ringing;

  ekiga_call_window_update_calling_state (cw, cw->priv->calling_state);
}

static void
on_established_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                        boost::shared_ptr<Ekiga::Call>  call,
                        gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  gtk_window_set_title (GTK_WINDOW (cw), call->get_remote_party_name ().c_str ());

  if (cw->priv->video_display_settings->get_bool ("stay-on-top"))
    gdk_window_set_keep_above (gtk_widget_get_window (GTK_WIDGET (cw)), true);
  ekiga_call_window_set_status (cw, _("Connected with %s"), call->get_remote_party_name ().c_str ());
  ekiga_call_window_update_calling_state (cw, Connected);

  cw->priv->current_call = call;

  cw->priv->timeout_id = g_timeout_add_seconds (1, on_stats_refresh_cb, self);
}

static void
on_cleared_call_cb (G_GNUC_UNUSED boost::shared_ptr<Ekiga::CallManager> manager,
                    boost::shared_ptr<Ekiga::Call>  call,
                    G_GNUC_UNUSED std::string reason,
                    gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  if (cw->priv->current_call && cw->priv->current_call->get_id () != call->get_id ()) {
    return; // Trying to clear another call than the current active one
  }

  if (cw->priv->video_display_settings->get_bool ("stay-on-top"))
    gdk_window_set_keep_above (gtk_widget_get_window (GTK_WIDGET (cw)), false);
  ekiga_call_window_update_calling_state (cw, Standby);
  ekiga_call_window_set_status (cw, _("Standby"));
  ekiga_call_window_set_bandwidth (cw, 0.0, 0.0, 0.0, 0.0, 0, 0);
  ekiga_call_window_clear_stats (cw);

  if (cw->priv->ext_video_win) {
    ekiga_ext_window_destroy (EKIGA_EXT_WINDOW (cw->priv->ext_video_win));
  }

  if (cw->priv->current_call) {
    cw->priv->current_call = boost::shared_ptr<Ekiga::Call>();
    g_source_remove (cw->priv->timeout_id);
    cw->priv->timeout_id = -1;
  }

  ekiga_call_window_clear_signal_levels (cw);

  gtk_window_set_title (GTK_WINDOW (cw), _("Call Window"));
}

static void on_missed_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                               boost::shared_ptr<Ekiga::Call> call,
                               gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  if (cw->priv->current_call && call && cw->priv->current_call->get_id () != call->get_id ()) {
    return; // Trying to clear another call than the current active one
  }

  gtk_window_set_title (GTK_WINDOW (cw), _("Call Window"));
  ekiga_call_window_update_calling_state (cw, Standby);
  ekiga_call_window_set_status (cw, _("Standby"));
}

static void
on_held_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                 boost::shared_ptr<Ekiga::Call>  /*call*/,
                 gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  ekiga_call_window_set_call_hold (cw, true);
  gm_statusbar_flash_message (GM_STATUSBAR (cw->priv->statusbar), _("Call on hold"));
}

static void
on_retrieved_call_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                      boost::shared_ptr<Ekiga::Call>  /*call*/,
                      gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  ekiga_call_window_set_call_hold (cw, false);
  gm_statusbar_flash_message (GM_STATUSBAR (cw->priv->statusbar), _("Call retrieved"));
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
                     gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);
  bool is_video = (type == Ekiga::Call::Video);

  set_codec (cw->priv, name, is_video, is_transmitting);
  ekiga_call_window_channels_menu_update_sensitivity (cw, is_video, true);
}


static void
on_stream_closed_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                                 boost::shared_ptr<Ekiga::Call>  /* call */,
                                 G_GNUC_UNUSED std::string name,
                                 Ekiga::Call::StreamType type,
                                 bool is_transmitting,
                                 gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);
  bool is_video = (type == Ekiga::Call::Video);

  set_codec (cw->priv, "", is_video, is_transmitting);
  ekiga_call_window_channels_menu_update_sensitivity (cw, is_video, false);
}

static void
on_stream_paused_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                     boost::shared_ptr<Ekiga::Call>  /*call*/,
                     std::string /*name*/,
                     Ekiga::Call::StreamType type,
                     gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  ekiga_call_window_set_channel_pause (cw, true, (type == Ekiga::Call::Video));
}

static void
on_stream_resumed_cb (boost::shared_ptr<Ekiga::CallManager>  /*manager*/,
                      boost::shared_ptr<Ekiga::Call>  /*call*/,
                      std::string /*name*/,
                      Ekiga::Call::StreamType type,
                      gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  ekiga_call_window_set_channel_pause (cw, false, (type == Ekiga::Call::Video));
}

static gboolean
on_stats_refresh_cb (gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  if (cw->priv->calling_state == Connected && cw->priv->current_call) {

    Ekiga::VideoOutputStats videooutput_stats;
    cw->priv->videooutput_core->get_videooutput_stats(videooutput_stats);

    ekiga_call_window_set_status (cw, _("Connected with %s\n%s"), cw->priv->current_call->get_remote_party_name ().c_str (),
                                  cw->priv->current_call->get_duration ().c_str ());
    ekiga_call_window_set_bandwidth (cw,
                                     cw->priv->current_call->get_transmitted_audio_bandwidth (),
                                     cw->priv->current_call->get_received_audio_bandwidth (),
                                     cw->priv->current_call->get_transmitted_video_bandwidth (),
                                     cw->priv->current_call->get_received_video_bandwidth (),
                                     videooutput_stats.tx_fps,
                                     videooutput_stats.rx_fps);

    unsigned int jitter = cw->priv->current_call->get_jitter_size ();
    double lost = cw->priv->current_call->get_lost_packets ();
    double late = cw->priv->current_call->get_late_packets ();
    double out_of_order = cw->priv->current_call->get_out_of_order_packets ();

    ekiga_call_window_update_stats (cw, lost, late, out_of_order, jitter,
                                    videooutput_stats.rx_width,
                                    videooutput_stats.rx_height,
                                    videooutput_stats.tx_width,
                                    videooutput_stats.tx_height,
                                    cw->priv->transmitted_audio_codec.c_str (),
                                    cw->priv->transmitted_video_codec.c_str ());
  }

  return true;
}

static gboolean
ekiga_call_window_delete_event_cb (GtkWidget *widget,
                                   G_GNUC_UNUSED GdkEventAny *event)
{
  EkigaCallWindow *cw = NULL;
  GSettings *settings = NULL;

  cw = EKIGA_CALL_WINDOW (widget);
  g_return_val_if_fail (EKIGA_IS_CALL_WINDOW (cw), false);

  /* Hang up or disable preview */
  if (cw->priv->calling_state != Standby && cw->priv->current_call) {
    cw->priv->current_call->hang_up ();
  }
  else if (cw->priv->fullscreen) {
    ekiga_call_window_toggle_fullscreen (cw);
  }
  else {
    settings = g_settings_new (VIDEO_DEVICES_SCHEMA);
    g_settings_set_boolean (settings, "enable-preview", false);
    g_clear_object (&settings);
  }

  return true; // Do not relay the event anymore
}

static gboolean
ekiga_call_window_fullscreen_event_cb (GtkWidget *widget,
                                       G_GNUC_UNUSED GdkEventAny *event)
{
  EkigaCallWindow *cw = NULL;

  cw = EKIGA_CALL_WINDOW (widget);
  g_return_val_if_fail (EKIGA_IS_CALL_WINDOW (cw), false);
  ekiga_call_window_toggle_fullscreen (cw);

  return true; // Do not relay the event anymore
}

static void
window_closed_from_menu_cb (G_GNUC_UNUSED GtkWidget *w,
                            gpointer data)
{
  EkigaCallWindow *cw = NULL;

  cw = EKIGA_CALL_WINDOW (data);
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (cw));

  ekiga_call_window_delete_event_cb (GTK_WIDGET (cw), NULL);
}

static void
animate_logo_cb (G_GNUC_UNUSED ClutterActor *actor,
                 gpointer logo)
{
  clutter_actor_save_easing_state (CLUTTER_ACTOR (logo));
  clutter_actor_set_easing_duration (CLUTTER_ACTOR (logo), 8000);
  clutter_actor_set_opacity (CLUTTER_ACTOR (logo), 255);
  clutter_actor_restore_easing_state (CLUTTER_ACTOR (logo));
}

static void
resize_actor_cb (ClutterActor *actor,
                 G_GNUC_UNUSED ClutterActorBox *box,
                 G_GNUC_UNUSED ClutterAllocationFlags flags,
                 gpointer data)
{
  EkigaCallWindow *cw = NULL;

  cw = EKIGA_CALL_WINDOW (data);
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (cw));

  resize_actor (CLUTTER_ACTOR (cw->priv->remote_video),
                cw->priv->remote_video_natural_width,
                cw->priv->remote_video_natural_height,
                clutter_actor_get_height (CLUTTER_ACTOR (actor)),
                1000);
  resize_actor (CLUTTER_ACTOR (cw->priv->local_video),
                cw->priv->local_video_natural_width,
                cw->priv->local_video_natural_height,
                clutter_actor_get_height (CLUTTER_ACTOR (actor)) * LOCAL_VIDEO_RATIO);
}

static void
resize_actor (ClutterActor *texture,
              unsigned natural_width,
              unsigned natural_height,
              unsigned available_height,
              unsigned easing_delay)
{
  gfloat zoom = (gfloat) available_height / natural_height;

  clutter_actor_save_easing_state (texture);
  clutter_actor_set_easing_duration (texture, easing_delay);
  clutter_actor_set_height (texture, natural_height * zoom);
  clutter_actor_set_width (texture, natural_width * zoom);
  clutter_actor_restore_easing_state (texture);
}

static void
ekiga_call_window_update_calling_state (EkigaCallWindow *cw,
					unsigned calling_state)
{
  g_return_if_fail (cw != NULL);

  switch (calling_state)
    {
    case Standby:

      /* Update the hold state */
      ekiga_call_window_set_call_hold (cw, false);

      /* Update the sensitivity, all channels are closed */
      ekiga_call_window_channels_menu_update_sensitivity (cw, true, false);
      ekiga_call_window_channels_menu_update_sensitivity (cw, false, false);

      /* Update the menus and toolbar items */
      gtk_menu_set_sensitive (cw->priv->main_menu, "connect", false);
      gtk_menu_set_sensitive (cw->priv->main_menu, "disconnect", false);
      gtk_menu_section_set_sensitive (cw->priv->main_menu, "hold_call", false);
      gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->pick_up_button), false);
      gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->hang_up_button), false);
      gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->hold_button), false);

      /* Spinner updates */
      gtk_widget_show (cw->priv->camera_image);
      gtk_widget_hide (cw->priv->spinner);
      gtk_spinner_stop (GTK_SPINNER (cw->priv->spinner));

      /* Show/hide call frame */
      gtk_widget_hide (cw->priv->call_frame);

      /* Destroy the transfer call popup */
      if (cw->priv->transfer_call_popup)
        gtk_dialog_response (GTK_DIALOG (cw->priv->transfer_call_popup),
			     GTK_RESPONSE_REJECT);
      break;


    case Calling:

      /* Show/hide call frame */
      gtk_widget_show (cw->priv->call_frame);

      /* Update the menus and toolbar items */
      gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->pick_up_button), false);
      gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->hang_up_button), true);
      gtk_menu_set_sensitive (cw->priv->main_menu, "connect", false);
      gtk_menu_set_sensitive (cw->priv->main_menu, "disconnect", true);
      break;

    case Ringing:

      /* Spinner updates */
      gtk_widget_hide (cw->priv->camera_image);
      gtk_widget_show (cw->priv->spinner);
      gtk_spinner_start (GTK_SPINNER (cw->priv->spinner));
      break;

    case Connected:

      /* Show/hide call frame */
      gtk_widget_show (cw->priv->call_frame);

      /* Spinner updates */
      gtk_widget_show (cw->priv->camera_image);
      gtk_widget_hide (cw->priv->spinner);
      gtk_spinner_start (GTK_SPINNER (cw->priv->spinner));

      /* Update the menus and toolbar items */
      gtk_menu_set_sensitive (cw->priv->main_menu, "connect", false);
      gtk_menu_set_sensitive (cw->priv->main_menu, "disconnect", true);
      gtk_menu_section_set_sensitive (cw->priv->main_menu, "hold_call", true);
      gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->pick_up_button), false);
      gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->hang_up_button), true);
      gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->hold_button), true);
      break;


    case Called:

      /* Update the menus and toolbar items */
      gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->pick_up_button), true);
      gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->hang_up_button), true);
      gtk_menu_set_sensitive (cw->priv->main_menu, "connect", true);
      gtk_menu_set_sensitive (cw->priv->main_menu, "disconnect", true);

      /* Show/hide call frame and call window (if no notifications */
      gtk_widget_show (cw->priv->call_frame);
      if (!notify_has_actions (cw)) {
        gtk_window_present (GTK_WINDOW (cw));
        gtk_widget_show (GTK_WIDGET (cw));
      }
      break;

    default:
      break;
    }

  cw->priv->calling_state = calling_state;
}

static void
ekiga_call_window_clear_signal_levels (EkigaCallWindow *cw)
{
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (cw));

  //gm_level_meter_clear (GM_LEVEL_METER (cw->priv->output_signal));
  //gm_level_meter_clear (GM_LEVEL_METER (cw->priv->input_signal));
}

static void
ekiga_call_window_clear_stats (EkigaCallWindow *cw)
{
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (cw));

  ekiga_call_window_update_stats (cw, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);
  if (cw->priv->qualitymeter)
    gm_powermeter_set_level (GM_POWERMETER (cw->priv->qualitymeter), 0.0);
}


static void
ekiga_call_window_update_stats (EkigaCallWindow *cw,
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

  g_return_if_fail (EKIGA_IS_CALL_WINDOW (cw));

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

  gtk_widget_set_tooltip_text (GTK_WIDGET (cw->priv->main_video_image), stats_msg);
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

  if (cw->priv->qualitymeter)
    gm_powermeter_set_level (GM_POWERMETER (cw->priv->qualitymeter),
			     quality_level);
}


static void
ekiga_call_window_set_status (EkigaCallWindow *cw,
			      const char *msg,
                              ...)
{
  GtkTextIter iter;
  GtkTextBuffer *text_buffer = NULL;

  char buffer [1025];
  va_list args;

  g_return_if_fail (EKIGA_IS_CALL_WINDOW (cw));

  text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (cw->priv->info_text));

  va_start (args, msg);

  if (msg == NULL)
    buffer[0] = 0;
  else
    vsnprintf (buffer, 1024, msg, args);

  gtk_text_buffer_set_text (text_buffer, buffer, -1);
  if (!g_strrstr (buffer, "\n")) {
    gtk_text_buffer_get_end_iter (text_buffer, &iter);
    gtk_text_buffer_insert (text_buffer, &iter, "\n", -1);
  }

  va_end (args);
}


static void
ekiga_call_window_set_bandwidth (EkigaCallWindow *cw,
                                 float ta,
                                 float ra,
                                 float tv,
                                 float rv,
                                 int tfps,
                                 int rfps)
{
  gchar *msg = NULL;

  g_return_if_fail (EKIGA_IS_CALL_WINDOW (cw));

  if (ta > 0.0 || ra > 0.0 || tv > 0.0 || rv > 0.0 || tfps > 0 || rfps > 0)
    /* Translators: A = Audio, V = Video, FPS = Frames per second */
    msg = g_strdup_printf (_("A:%.1f/%.1f V:%.1f/%.1f FPS:%d/%d"),
                           ta, ra, tv, rv, tfps, rfps);

  if (msg)
    gm_statusbar_push_message (GM_STATUSBAR (cw->priv->statusbar), "%s", msg);
  else
    gm_statusbar_push_message (GM_STATUSBAR (cw->priv->statusbar), NULL);
  g_free (msg);
}

static void
ekiga_call_window_set_call_hold (EkigaCallWindow *cw,
                                 bool is_on_hold)
{
  GtkWidget *child = NULL;

  g_return_if_fail (EKIGA_IS_CALL_WINDOW (cw));

  child = gtk_bin_get_child (GTK_BIN (gtk_menu_get_widget (cw->priv->main_menu, "hold_call")));

  if (is_on_hold) {

    if (GTK_IS_LABEL (child))
      gtk_label_set_text_with_mnemonic (GTK_LABEL (child),
					_("_Retrieve Call"));

    /* Set the audio and video menu to unsensitive */
    gtk_menu_set_sensitive (cw->priv->main_menu, "suspend_audio", false);
    gtk_menu_set_sensitive (cw->priv->main_menu, "suspend_video", false);

    ekiga_call_window_set_channel_pause (cw, true, false);
    ekiga_call_window_set_channel_pause (cw, true, true);
  }
  else {

    if (GTK_IS_LABEL (child))
      gtk_label_set_text_with_mnemonic (GTK_LABEL (child),
					_("H_old Call"));

    gtk_menu_set_sensitive (cw->priv->main_menu, "suspend_audio", true);
    gtk_menu_set_sensitive (cw->priv->main_menu, "suspend_video", true);

    ekiga_call_window_set_channel_pause (cw, false, false);
    ekiga_call_window_set_channel_pause (cw, false, true);
  }

  g_signal_handlers_block_by_func (cw->priv->hold_button,
                                   (gpointer) hold_current_call_cb,
                                   cw);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cw->priv->hold_button),
                                is_on_hold);
  g_signal_handlers_unblock_by_func (cw->priv->hold_button,
                                     (gpointer) hold_current_call_cb,
                                     cw);
}

static void
ekiga_call_window_set_channel_pause (EkigaCallWindow *cw,
				     gboolean pause,
				     gboolean is_video)
{
  GtkWidget *widget = NULL;
  GtkWidget *child = NULL;
  gchar *msg = NULL;

  g_return_if_fail (EKIGA_IS_CALL_WINDOW (cw));

  if (!pause && !is_video)
    msg = _("Suspend _Audio");
  else if (!pause && is_video)
    msg = _("Suspend _Video");
  else if (pause && !is_video)
    msg = _("Resume _Audio");
  else if (pause && is_video)
    msg = _("Resume _Video");

  widget = gtk_menu_get_widget (cw->priv->main_menu,
			        is_video ? "suspend_video" : "suspend_audio");
  child = gtk_bin_get_child (GTK_BIN (widget));

  if (GTK_IS_LABEL (child))
    gtk_label_set_text_with_mnemonic (GTK_LABEL (child), msg);
}

static GtkWidget *
gm_cw_video_settings_window_new (EkigaCallWindow *cw)
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
  cw->priv->video_settings_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (cw->priv->video_settings_frame),
			     GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (cw->priv->video_settings_frame), 5);

  /* Category */
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (cw->priv->video_settings_frame), vbox);

  /* Brightness */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  image = gtk_image_new_from_icon_name ("brightness", GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, false, false, 0);

  cw->priv->adj_brightness = gtk_adjustment_new (brightness, 0.0,
                                                 255.0, 1.0, 5.0, 1.0);
  hscale_brightness = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL,
                                     GTK_ADJUSTMENT (cw->priv->adj_brightness));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_brightness), false);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_brightness), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_brightness, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 3);

  gtk_widget_set_tooltip_text (hscale_brightness, _("Adjust brightness"));

  g_signal_connect (cw->priv->adj_brightness, "value-changed",
		    G_CALLBACK (video_settings_changed_cb),
		    (gpointer) cw);

  /* Whiteness */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  image = gtk_image_new_from_icon_name ("whiteness", GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, false, false, 0);

  cw->priv->adj_whiteness = gtk_adjustment_new (whiteness, 0.0,
						255.0, 1.0, 5.0, 1.0);
  hscale_whiteness = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL,
                                    GTK_ADJUSTMENT (cw->priv->adj_whiteness));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_whiteness), false);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_whiteness), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_whiteness, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 3);

  gtk_widget_set_tooltip_text (hscale_whiteness, _("Adjust whiteness"));

  g_signal_connect (cw->priv->adj_whiteness, "value-changed",
		    G_CALLBACK (video_settings_changed_cb),
		    (gpointer) cw);

  /* Colour */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  image = gtk_image_new_from_icon_name ("color", GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, false, false, 0);

  cw->priv->adj_colour = gtk_adjustment_new (colour, 0.0,
					     255.0, 1.0, 5.0, 1.0);
  hscale_colour = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL,
                                 GTK_ADJUSTMENT (cw->priv->adj_colour));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_colour), false);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_colour), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_colour, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 3);

  gtk_widget_set_tooltip_text (hscale_colour, _("Adjust color"));

  g_signal_connect (cw->priv->adj_colour, "value-changed",
		    G_CALLBACK (video_settings_changed_cb),
		    (gpointer) cw);

  /* Contrast */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  image = gtk_image_new_from_icon_name ("contrast", GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, false, false, 0);

  cw->priv->adj_contrast = gtk_adjustment_new (contrast, 0.0,
					       255.0, 1.0, 5.0, 1.0);
  hscale_contrast = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL,
                                   GTK_ADJUSTMENT (cw->priv->adj_contrast));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_contrast), false);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_contrast), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_contrast, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 3);

  gtk_widget_set_tooltip_text (hscale_contrast, _("Adjust contrast"));

  g_signal_connect (cw->priv->adj_contrast, "value-changed",
		    G_CALLBACK (video_settings_changed_cb),
		    (gpointer) cw);

  gtk_container_add (GTK_CONTAINER (window),
                     cw->priv->video_settings_frame);
  gtk_widget_show_all (cw->priv->video_settings_frame);

  gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->video_settings_frame), false);

  gtk_widget_hide_on_delete (window);

  return window;
}

static GtkWidget *
gm_cw_audio_settings_window_new (EkigaCallWindow *cw)
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
  cw->priv->audio_output_volume_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (cw->priv->audio_output_volume_frame),
			     GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (cw->priv->audio_output_volume_frame), 5);
  main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

  /* The vbox */
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (cw->priv->audio_output_volume_frame), vbox);

  /* Output volume */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start (GTK_BOX (hbox),
		      gtk_image_new_from_icon_name ("audio-volume", GTK_ICON_SIZE_SMALL_TOOLBAR),
		      false, false, 0);

  small_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  cw->priv->adj_output_volume = gtk_adjustment_new (0, 0.0, 101.0, 1.0, 5.0, 1.0);
  hscale_play = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL,
                               GTK_ADJUSTMENT (cw->priv->adj_output_volume));
  gtk_scale_set_value_pos (GTK_SCALE (hscale_play), GTK_POS_RIGHT);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_play), false);
  gtk_box_pack_start (GTK_BOX (small_vbox), hscale_play, true, true, 0);

  //cw->priv->output_signal = gm_level_meter_new ();
  //gtk_box_pack_start (GTK_BOX (small_vbox), cw->priv->output_signal, true, true, 0);
  gtk_box_pack_start (GTK_BOX (hbox), small_vbox, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 3);

  gtk_box_pack_start (GTK_BOX (main_vbox), cw->priv->audio_output_volume_frame,
                      false, false, 0);
  gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->audio_output_volume_frame),  false);

  /* Audio control frame, we need it to disable controls */
  cw->priv->audio_input_volume_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (cw->priv->audio_input_volume_frame),
			     GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (cw->priv->audio_input_volume_frame), 5);

  /* The vbox */
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (cw->priv->audio_input_volume_frame), vbox);

  /* Input volume */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start (GTK_BOX (hbox),
		      gtk_image_new_from_icon_name ("audio-input-microphone",
						    GTK_ICON_SIZE_SMALL_TOOLBAR),
		      false, false, 0);

  small_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  cw->priv->adj_input_volume = gtk_adjustment_new (0, 0.0, 101.0, 1.0, 5.0, 1.0);
  hscale_rec = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL,
                              GTK_ADJUSTMENT (cw->priv->adj_input_volume));
  gtk_scale_set_value_pos (GTK_SCALE (hscale_rec), GTK_POS_RIGHT);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_rec), false);
  gtk_box_pack_start (GTK_BOX (small_vbox), hscale_rec, true, true, 0);

//  cw->priv->input_signal = gm_level_meter_new ();
//  gtk_box_pack_start (GTK_BOX (small_vbox), cw->priv->input_signal, true, true, 0);
  gtk_box_pack_start (GTK_BOX (hbox), small_vbox, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 3);

  gtk_box_pack_start (GTK_BOX (main_vbox), cw->priv->audio_input_volume_frame,
                      false, false, 0);
  gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->audio_input_volume_frame),  false);

  gtk_container_add (GTK_CONTAINER (window), main_vbox);
  gtk_widget_show_all (main_vbox);

  g_signal_connect (cw->priv->adj_output_volume, "value-changed",
		    G_CALLBACK (audio_volume_changed_cb), cw);

  g_signal_connect (cw->priv->adj_input_volume, "value-changed",
		    G_CALLBACK (audio_volume_changed_cb), cw);

  gtk_widget_hide_on_delete (window);

  g_signal_connect (window, "show",
                    G_CALLBACK (audio_volume_window_shown_cb), cw);

  g_signal_connect (window, "hide",
                    G_CALLBACK (audio_volume_window_hidden_cb), cw);

  return window;
}

static void
ekiga_call_window_init_menu (EkigaCallWindow *cw)
{
  g_return_if_fail (cw != NULL);

  cw->priv->main_menu = gtk_menu_bar_new ();

  static MenuEntry gnomemeeting_menu [] =
    {
      GTK_MENU_NEW (_("_Call")),

      GTK_MENU_THEME_ENTRY("connect", _("_Pick up"), _("Pick up the current call"),
                           "phone-pick-up", 'd',
                           G_CALLBACK (pick_up_call_cb), cw, false),

      GTK_MENU_THEME_ENTRY("disconnect", _("_Hang up"), _("Hang up the current call"),
                           "phone-hang-up", 'd',
                           G_CALLBACK (hang_up_call_cb), cw, false),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("hold_call", _("H_old Call"), _("Hold the current call"),
		     NULL, GDK_KEY_h,
		     G_CALLBACK (hold_current_call_cb), cw,
		     false),
      GTK_MENU_ENTRY("transfer_call", _("_Transfer Call"),
		     _("Transfer the current call"),
 		     NULL, GDK_KEY_t,
		     G_CALLBACK (transfer_current_call_cb), cw,
		     false),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("suspend_audio", _("Suspend _Audio"),
		     _("Suspend or resume the audio transmission"),
		     NULL, GDK_KEY_m,
		     G_CALLBACK (toggle_audio_stream_pause_cb),
		     cw, false),
      GTK_MENU_ENTRY("suspend_video", _("Suspend _Video"),
		     _("Suspend or resume the video transmission"),
		     NULL, GDK_KEY_p,
		     G_CALLBACK (toggle_video_stream_pause_cb),
		     cw, false),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("close", NULL, _("Close the Ekiga window"),
		     GTK_STOCK_CLOSE, 'W',
		     G_CALLBACK (window_closed_from_menu_cb),
		     cw, TRUE),

      GTK_MENU_NEW(_("_View")),

      /*
      GTK_MENU_RADIO_ENTRY("local_video", _("_Local Video"),
			   _("Local video image"),
			   NULL, '1',
			   NULL,
			   G_CALLBACK (display_changed_cb), cw,
			   true, false),
      GTK_MENU_RADIO_ENTRY("remote_video", _("_Remote Video"),
			   _("Remote video image"),
			   NULL, '2',
			   NULL,
			   G_CALLBACK (display_changed_cb), cw,
			   false, false),
      GTK_MENU_RADIO_ENTRY("both_incrusted", _("_Picture-in-Picture"),
			   _("Both video images"),
			   NULL, '3',
			   NULL,
			   G_CALLBACK (display_changed_cb), cw,
			   false, false),
                           */
      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("zoom_in", NULL, _("Zoom in"),
		     GTK_STOCK_ZOOM_IN, '+',
		     G_CALLBACK (zoom_in_changed_cb),
		     (gpointer) cw, false),
      GTK_MENU_ENTRY("zoom_out", NULL, _("Zoom out"),
		     GTK_STOCK_ZOOM_OUT, '-',
		     G_CALLBACK (zoom_out_changed_cb),
		     (gpointer) cw, false),
      GTK_MENU_ENTRY("normal_size", NULL, _("Normal size"),
		     GTK_STOCK_ZOOM_100, '0',
		     G_CALLBACK (zoom_normal_changed_cb),
		     (gpointer) cw, false),
      GTK_MENU_ENTRY("fullscreen", _("_Fullscreen"), _("Switch to fullscreen"),
		     GTK_STOCK_ZOOM_IN, GDK_KEY_F11,
		     G_CALLBACK (fullscreen_changed_cb),
		     (gpointer) cw, true),

      GTK_MENU_END
    };


  gtk_build_menu (cw->priv->main_menu,
		  gnomemeeting_menu,
		  cw->priv->accel,
		  cw->priv->statusbar);

  gtk_widget_show_all (GTK_WIDGET (cw->priv->main_menu));
}

static void
ekiga_call_window_init_clutter (EkigaCallWindow *cw)
{
  gchar *filename = NULL;

  GtkWidget *clutter_widget = NULL;
  GdkPixbuf *pixbuf  = NULL;

  ClutterActor *emblem = NULL;
  ClutterContent *image = NULL;

  clutter_widget = gtk_clutter_embed_new ();
  gtk_widget_set_size_request (GTK_WIDGET (clutter_widget), STAGE_WIDTH, STAGE_HEIGHT);
  gtk_container_add (GTK_CONTAINER (cw->priv->event_box), clutter_widget);

  cw->priv->stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (clutter_widget));
  clutter_actor_set_background_color (CLUTTER_ACTOR (cw->priv->stage), CLUTTER_COLOR_Black);
  clutter_stage_set_user_resizable (CLUTTER_STAGE (cw->priv->stage), TRUE);

  cw->priv->remote_video =
    CLUTTER_ACTOR (g_object_new (CLUTTER_TYPE_TEXTURE, "disable-slicing", TRUE, NULL));
  clutter_actor_add_child (CLUTTER_ACTOR (cw->priv->stage), CLUTTER_ACTOR (cw->priv->remote_video));
  clutter_actor_add_constraint (cw->priv->remote_video,
                                clutter_align_constraint_new (cw->priv->stage,
                                                              CLUTTER_ALIGN_BOTH,
                                                              0.5));

  /* Ekiga Logo */
  filename = g_build_filename (DATA_DIR, "pixmaps", PACKAGE_NAME,
                               PACKAGE_NAME "-full-icon.png", NULL);
  pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
  g_free (filename);

  emblem = clutter_actor_new ();
  image = clutter_image_new ();
  clutter_image_set_data (CLUTTER_IMAGE (image),
                          gdk_pixbuf_get_pixels (pixbuf),
                          gdk_pixbuf_get_has_alpha (pixbuf)?
                          COGL_PIXEL_FORMAT_RGBA_8888:COGL_PIXEL_FORMAT_RGB_888,
                          gdk_pixbuf_get_width (pixbuf),
                          gdk_pixbuf_get_height (pixbuf),
                          gdk_pixbuf_get_rowstride (pixbuf),
                          NULL);
  clutter_actor_set_content (emblem, image);
  g_object_unref (image);
  resize_actor (CLUTTER_ACTOR (emblem),
                gdk_pixbuf_get_width (pixbuf),
                gdk_pixbuf_get_height (pixbuf),
                STAGE_HEIGHT * EMBLEM_RATIO);
  clutter_actor_set_margin_top (CLUTTER_ACTOR (emblem), EMBLEM_MARGIN);
  clutter_actor_set_margin_right (CLUTTER_ACTOR (emblem), EMBLEM_MARGIN);
  clutter_actor_add_constraint (emblem,
                                clutter_align_constraint_new (cw->priv->stage,
                                                              CLUTTER_ALIGN_X_AXIS,
                                                              1.0));
  clutter_actor_add_constraint (emblem,
                                clutter_align_constraint_new (cw->priv->stage,
                                                              CLUTTER_ALIGN_Y_AXIS,
                                                              0.0));
  clutter_actor_add_child (cw->priv->stage, emblem);
  clutter_actor_set_opacity (CLUTTER_ACTOR (emblem), 0);
  g_object_unref (pixbuf);

  /* Preview Video */
  cw->priv->local_video =
    CLUTTER_ACTOR (g_object_new (CLUTTER_TYPE_TEXTURE, "disable-slicing", TRUE, NULL));
  clutter_actor_add_constraint (cw->priv->local_video,
                                clutter_align_constraint_new (cw->priv->stage,
                                                              CLUTTER_ALIGN_X_AXIS,
                                                              0.0));
  clutter_actor_add_constraint (cw->priv->local_video,
                                clutter_align_constraint_new (cw->priv->stage,
                                                              CLUTTER_ALIGN_Y_AXIS,
                                                              1.0));
  clutter_actor_set_margin_bottom (CLUTTER_ACTOR (cw->priv->local_video), LOCAL_VIDEO_MARGIN);
  clutter_actor_set_margin_left (CLUTTER_ACTOR (cw->priv->local_video), LOCAL_VIDEO_MARGIN);
  clutter_actor_add_child (CLUTTER_ACTOR (cw->priv->stage), cw->priv->local_video);
  clutter_actor_set_opacity (CLUTTER_ACTOR (cw->priv->local_video), 0);

  g_signal_connect (cw->priv->stage, "allocation-changed", G_CALLBACK (resize_actor_cb), cw);
  g_signal_connect (cw->priv->stage, "show", G_CALLBACK (animate_logo_cb), emblem);

  cw->priv->videooutput_core->set_display_info (cw->priv->local_video,
                                                cw->priv->remote_video);
}

static void
ekiga_call_window_update_logo (EkigaCallWindow *cw)
{
  return; //FIXME
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (cw));

  gtk_widget_realize (GTK_WIDGET (cw));
  g_object_set (G_OBJECT (cw->priv->main_video_image),
                "icon-name", "avatar-default",
                "pixel-size", 128,
                NULL);

  ekiga_call_window_set_video_size (cw, GM_CIF_WIDTH, GM_CIF_HEIGHT);
}

static void
ekiga_call_window_toggle_fullscreen (EkigaCallWindow *cw)
{
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (cw));

  cw->priv->fullscreen = !cw->priv->fullscreen;

  if (cw->priv->fullscreen) {
    gm_window_save (GM_WINDOW (cw));
    gtk_widget_hide (cw->priv->main_menu);
    gtk_widget_hide (cw->priv->call_panel_toolbar);
    gtk_widget_hide (cw->priv->statusbar_ebox);
    gtk_window_fullscreen (GTK_WINDOW (cw));
    gtk_window_set_keep_above (GTK_WINDOW (cw), true);
  }
  else {
    gtk_widget_show (cw->priv->main_menu);
    gtk_widget_show (cw->priv->call_panel_toolbar);
    gtk_widget_show (cw->priv->statusbar_ebox);
    gtk_window_unfullscreen (GTK_WINDOW (cw));
    gtk_window_set_keep_above (GTK_WINDOW (cw),
                               cw->priv->video_display_settings->get_bool ("stay-on-top"));
    gm_window_restore (GM_WINDOW (cw));
  }
}

static void
ekiga_call_window_zooms_menu_update_sensitivity (EkigaCallWindow *cw,
                                                 unsigned int zoom)
{
  /* between 0.5 and 2.0 zoom */
  /* like above, also update the popup menus of the separate video windows */
  gtk_menu_set_sensitive (cw->priv->main_menu, "zoom_in", zoom != 200);
  gtk_menu_set_sensitive (cw->priv->main_menu, "zoom_out", zoom != 50);
  gtk_menu_set_sensitive (cw->priv->main_menu, "normal_size", zoom != 100);
}

static void
ekiga_call_window_channels_menu_update_sensitivity (EkigaCallWindow *cw,
                                                    bool is_video,
                                                    bool is_transmitting)
{
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (cw));

  if (is_transmitting) {
    if (!is_video)
      gtk_menu_set_sensitive (cw->priv->main_menu, "suspend_audio", true);
    else
      gtk_menu_set_sensitive (cw->priv->main_menu, "suspend_video", true);
  }
  else {
    if (!is_video)
      gtk_menu_set_sensitive (cw->priv->main_menu, "suspend_audio", false);
    else
      gtk_menu_set_sensitive (cw->priv->main_menu, "suspend_video", false);
  }
}

static gboolean
ekiga_call_window_transfer_dialog_run (EkigaCallWindow *cw,
				       GtkWidget *parent_window,
				       const char *u)
{
  gint answer = 0;

  const char *forward_url = NULL;

  g_return_val_if_fail (EKIGA_IS_CALL_WINDOW (cw), false);
  g_return_val_if_fail (GTK_IS_WINDOW (parent_window), false);

  cw->priv->transfer_call_popup =
    gm_entry_dialog_new (_("Transfer call to:"),
			 _("Transfer"));

  gtk_window_set_transient_for (GTK_WINDOW (cw->priv->transfer_call_popup),
				GTK_WINDOW (parent_window));

  gtk_dialog_set_default_response (GTK_DIALOG (cw->priv->transfer_call_popup),
				   GTK_RESPONSE_ACCEPT);

  if (u && !g_strcmp0 (u, ""))
    gm_entry_dialog_set_text (GM_ENTRY_DIALOG (cw->priv->transfer_call_popup), u);
  else
    gm_entry_dialog_set_text (GM_ENTRY_DIALOG (cw->priv->transfer_call_popup), "sip:");

  gtk_widget_show_all (cw->priv->transfer_call_popup);

  answer = gtk_dialog_run (GTK_DIALOG (cw->priv->transfer_call_popup));
  switch (answer) {

  case GTK_RESPONSE_ACCEPT:

    forward_url = gm_entry_dialog_get_text (GM_ENTRY_DIALOG (cw->priv->transfer_call_popup));
    if (g_strcmp0 (forward_url, "") && cw->priv->current_call)
      cw->priv->current_call->transfer (forward_url);
    break;

  default:
    break;
  }

  gtk_widget_destroy (cw->priv->transfer_call_popup);
  cw->priv->transfer_call_popup = NULL;

  return (answer == GTK_RESPONSE_ACCEPT);
}

static void
ekiga_call_window_connect_engine_signals (EkigaCallWindow *cw)
{
  boost::signals2::connection conn;

  g_return_if_fail (EKIGA_IS_CALL_WINDOW (cw));

  /* New Display Engine signals */

  conn = cw->priv->videooutput_core->device_opened.connect (boost::bind (&on_videooutput_device_opened_cb, _1, _2, _3, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->videooutput_core->device_closed.connect (boost::bind (&on_videooutput_device_closed_cb, _1, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->videooutput_core->device_error.connect (boost::bind (&on_videooutput_device_error_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->videooutput_core->size_changed.connect (boost::bind (&on_size_changed_cb, _1, _2, _3, _4, (gpointer) cw));
  cw->priv->connections.add (conn);


  /* New VideoInput Engine signals */
  conn = cw->priv->videoinput_core->device_opened.connect (boost::bind (&on_videoinput_device_opened_cb, _1, _2, _3, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->videoinput_core->device_closed.connect (boost::bind (&on_videoinput_device_closed_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->videoinput_core->device_error.connect (boost::bind (&on_videoinput_device_error_cb, _1, _2, _3, (gpointer) cw));
  cw->priv->connections.add (conn);

  /* New AudioInput Engine signals */

  conn = cw->priv->audioinput_core->device_opened.connect (boost::bind (&on_audioinput_device_opened_cb, _1, _2, _3, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->audioinput_core->device_closed.connect (boost::bind (&on_audioinput_device_closed_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->audioinput_core->device_error.connect (boost::bind (&on_audioinput_device_error_cb, _1, _2, _3, (gpointer) cw));
  cw->priv->connections.add (conn);

  /* New AudioOutput Engine signals */

  conn = cw->priv->audiooutput_core->device_opened.connect (boost::bind (&on_audiooutput_device_opened_cb, _1, _2, _3, _4, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->audiooutput_core->device_closed.connect (boost::bind (&on_audiooutput_device_closed_cb, _1, _2, _3, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->audiooutput_core->device_error.connect (boost::bind (&on_audiooutput_device_error_cb, _1, _2, _3, _4, (gpointer) cw));
  cw->priv->connections.add (conn);

  /* New Call Engine signals */

  conn = cw->priv->call_core->setup_call.connect (boost::bind (&on_setup_call_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->call_core->ringing_call.connect (boost::bind (&on_ringing_call_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->call_core->established_call.connect (boost::bind (&on_established_call_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->call_core->cleared_call.connect (boost::bind (&on_cleared_call_cb, _1, _2, _3, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->call_core->missed_call.connect (boost::bind (&on_missed_call_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->call_core->held_call.connect (boost::bind (&on_held_call_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->call_core->retrieved_call.connect (boost::bind (&on_retrieved_call_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->call_core->stream_opened.connect (boost::bind (&on_stream_opened_cb, _1, _2, _3, _4, _5, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->call_core->stream_closed.connect (boost::bind (&on_stream_closed_cb, _1, _2, _3, _4, _5, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->call_core->stream_paused.connect (boost::bind (&on_stream_paused_cb, _1, _2, _3, _4, (gpointer) cw));
  cw->priv->connections.add (conn);

  conn = cw->priv->call_core->stream_resumed.connect (boost::bind (&on_stream_resumed_cb, _1, _2, _3, _4, (gpointer) cw));
  cw->priv->connections.add (conn);
}

static void
ekiga_call_window_init_gui (EkigaCallWindow *cw)
{
  GtkWidget *event_box = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *frame = NULL;

  GtkToolItem *item = NULL;

  GtkWidget *image = NULL;
  GtkWidget *alignment = NULL;

  GtkShadowType shadow_type;

  /* The Audio & Video Settings windows */
  cw->priv->audio_settings_window = gm_cw_audio_settings_window_new (cw);
  cw->priv->video_settings_window = gm_cw_video_settings_window_new (cw);

  /* The extended video stream window */
  cw->priv->ext_video_win = ext_window_new (cw->priv->videooutput_core);

  /* The main table */
  event_box = gtk_event_box_new ();
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
  event_box = gtk_event_box_new ();
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
  gtk_container_add (GTK_CONTAINER (event_box), vbox);
  gtk_container_add (GTK_CONTAINER (frame), event_box);
  gtk_container_add (GTK_CONTAINER (cw), frame);
  gtk_widget_show_all (frame);

  /* Menu */
  ekiga_call_window_init_menu (cw);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (cw->priv->main_menu), false, false, 0);
  gtk_widget_show_all (cw->priv->main_menu);

  /* The widgets toolbar */
  cw->priv->call_panel_toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (cw->priv->call_panel_toolbar), true);
  gtk_toolbar_set_style (GTK_TOOLBAR (cw->priv->call_panel_toolbar), GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (cw->priv->call_panel_toolbar), false);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (alignment), cw->priv->call_panel_toolbar);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (alignment), false, false, 0);
  gtk_widget_show_all (alignment);

  /* The frame that contains the video */
  cw->priv->event_box = gtk_event_box_new ();
  ekiga_call_window_init_clutter (cw);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (cw->priv->event_box), true, true, 0);
  gtk_widget_show_all (cw->priv->event_box);

  /* The frame that contains information about the call */
  cw->priv->call_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (cw->priv->call_frame), GTK_SHADOW_NONE);
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  cw->priv->camera_image = gtk_image_new_from_icon_name ("camera-web", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_box_pack_start (GTK_BOX (hbox), cw->priv->camera_image, false, false, 12);

  cw->priv->spinner = gtk_spinner_new ();
  gtk_widget_set_size_request (GTK_WIDGET (cw->priv->spinner), 24, 24);
  gtk_box_pack_start (GTK_BOX (hbox), cw->priv->spinner, false, false, 12);

  cw->priv->info_text = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (cw->priv->info_text), false);
  gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->info_text), false);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (cw->priv->info_text),
			       GTK_WRAP_NONE);
  gtk_text_view_set_cursor_visible  (GTK_TEXT_VIEW (cw->priv->info_text), false);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (alignment), cw->priv->info_text);
  gtk_box_pack_start (GTK_BOX (hbox), alignment, false, false, 2);
  gtk_container_add (GTK_CONTAINER (cw->priv->call_frame), hbox);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (cw->priv->call_frame), true, true, 2);
  gtk_widget_show_all (cw->priv->call_frame);
  gtk_widget_hide (cw->priv->spinner);

  /* Pick up */
  item = gtk_tool_item_new ();
  cw->priv->pick_up_button = gtk_button_new ();
  image = gtk_image_new_from_icon_name ("phone-pick-up", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_container_add (GTK_CONTAINER (cw->priv->pick_up_button), image);
  gtk_container_add (GTK_CONTAINER (item), cw->priv->pick_up_button);
  gtk_button_set_relief (GTK_BUTTON (cw->priv->pick_up_button), GTK_RELIEF_NONE);
  gtk_widget_show (cw->priv->pick_up_button);
  gtk_toolbar_insert (GTK_TOOLBAR (cw->priv->call_panel_toolbar),
		      GTK_TOOL_ITEM (item), -1);
  gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->pick_up_button), false);
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (item),
				  _("Pick up the current call"));
  g_signal_connect (cw->priv->pick_up_button, "clicked",
		    G_CALLBACK (pick_up_call_cb), cw);

  /* Hang up */
  item = gtk_tool_item_new ();
  cw->priv->hang_up_button = gtk_button_new ();
  image = gtk_image_new_from_icon_name ("phone-hang-up", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_container_add (GTK_CONTAINER (cw->priv->hang_up_button), image);
  gtk_container_add (GTK_CONTAINER (item), cw->priv->hang_up_button);
  gtk_button_set_relief (GTK_BUTTON (cw->priv->hang_up_button), GTK_RELIEF_NONE);
  gtk_widget_show (cw->priv->hang_up_button);
  gtk_toolbar_insert (GTK_TOOLBAR (cw->priv->call_panel_toolbar),
		      GTK_TOOL_ITEM (item), -1);
  gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->hang_up_button), false);
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (item),
				  _("Hang up the current call"));
  g_signal_connect (cw->priv->hang_up_button, "clicked",
		    G_CALLBACK (hang_up_call_cb), cw);

  /* Separator */
  item = gtk_separator_tool_item_new ();
  gtk_toolbar_insert (GTK_TOOLBAR (cw->priv->call_panel_toolbar),
		      GTK_TOOL_ITEM (item), -1);

  /* Audio Volume */
  std::vector <Ekiga::AudioOutputDevice> devices;
  cw->priv->audiooutput_core->get_devices (devices);
  if (!(devices.size () == 1 && devices[0].source == "Pulse")) {

    item = gtk_tool_item_new ();
    cw->priv->audio_settings_button = gtk_button_new ();
    gtk_button_set_relief (GTK_BUTTON (cw->priv->audio_settings_button), GTK_RELIEF_NONE);
    image = gtk_image_new_from_icon_name ("audio-volume", GTK_ICON_SIZE_MENU);
    gtk_container_add (GTK_CONTAINER (cw->priv->audio_settings_button), image);
    gtk_container_add (GTK_CONTAINER (item), cw->priv->audio_settings_button);
    gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), false);

    gtk_widget_show (cw->priv->audio_settings_button);
    gtk_widget_set_sensitive (cw->priv->audio_settings_button, false);
    gtk_toolbar_insert (GTK_TOOLBAR (cw->priv->call_panel_toolbar),
                        GTK_TOOL_ITEM (item), -1);
    gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (item),
                                    _("Change the volume of your soundcard"));
    g_signal_connect_swapped (cw->priv->audio_settings_button, "clicked",
                              G_CALLBACK (gtk_widget_show),
                              (gpointer) cw->priv->audio_settings_window);
  }

  /* Video Settings */
  item = gtk_tool_item_new ();
  cw->priv->video_settings_button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (cw->priv->video_settings_button), GTK_RELIEF_NONE);
  image = gtk_image_new_from_icon_name ("video-settings", GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (cw->priv->video_settings_button), image);
  gtk_container_add (GTK_CONTAINER (item), cw->priv->video_settings_button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), false);

  gtk_widget_show (cw->priv->video_settings_button);
  gtk_widget_set_sensitive (cw->priv->video_settings_button, false);
  gtk_toolbar_insert (GTK_TOOLBAR (cw->priv->call_panel_toolbar),
		      GTK_TOOL_ITEM (item), -1);
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (item),
				   _("Change the color settings of your video device"));

  g_signal_connect_swapped (cw->priv->video_settings_button, "clicked",
                            G_CALLBACK (gtk_widget_show),
                            (gpointer) cw->priv->video_settings_window);

  /* Call Pause */
  item = gtk_tool_item_new ();
  cw->priv->hold_button = gtk_toggle_button_new ();
  image = gtk_image_new_from_icon_name ("media-playback-pause", GTK_ICON_SIZE_MENU);
  gtk_button_set_relief (GTK_BUTTON (cw->priv->hold_button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (cw->priv->hold_button), image);
  gtk_container_add (GTK_CONTAINER (item), cw->priv->hold_button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), false);

  gtk_widget_show (cw->priv->hold_button);
  gtk_toolbar_insert (GTK_TOOLBAR (cw->priv->call_panel_toolbar),
		      GTK_TOOL_ITEM (item), -1);
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (item),
				  _("Hold the current call"));
  gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->hold_button), false);

  g_signal_connect (cw->priv->hold_button, "clicked",
		    G_CALLBACK (hold_current_call_cb), cw);
  gtk_widget_show_all (cw->priv->call_panel_toolbar);

  /* The statusbar */
  cw->priv->statusbar = gm_statusbar_new ();
  gtk_widget_style_get (cw->priv->statusbar, "shadow-type", &shadow_type, NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), shadow_type);
  gtk_box_pack_start (GTK_BOX (cw->priv->statusbar), frame, false, false, 0);
  gtk_box_reorder_child (GTK_BOX (cw->priv->statusbar), frame, 0);

  cw->priv->qualitymeter = gm_powermeter_new ();
  gtk_container_add (GTK_CONTAINER (frame), cw->priv->qualitymeter);

  cw->priv->statusbar_ebox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (cw->priv->statusbar_ebox), cw->priv->statusbar);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (cw->priv->statusbar_ebox), false, false, 0);
  gtk_widget_show_all (cw->priv->statusbar_ebox);

  /* Logo */
  gtk_window_set_resizable (GTK_WINDOW (cw), true);
  ekiga_call_window_update_logo (cw);

  /* Init */
  ekiga_call_window_set_status (cw, _("Standby"));
  ekiga_call_window_set_bandwidth (cw, 0.0, 0.0, 0.0, 0.0, 0, 0);

  gtk_widget_hide (cw->priv->call_frame);
}

static void
ekiga_call_window_init (EkigaCallWindow *cw)
{
  cw->priv = new EkigaCallWindowPrivate ();

  cw->priv->accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (cw), cw->priv->accel);
  gtk_accel_group_connect (cw->priv->accel, GDK_KEY_Escape, (GdkModifierType) 0, GTK_ACCEL_LOCKED,
                           g_cclosure_new_swap (G_CALLBACK (ekiga_call_window_delete_event_cb),
                                                (gpointer) cw, NULL));
  gtk_accel_group_connect (cw->priv->accel, GDK_KEY_F11, (GdkModifierType) 0, GTK_ACCEL_LOCKED,
                           g_cclosure_new_swap (G_CALLBACK (ekiga_call_window_fullscreen_event_cb),
                                                (gpointer) cw, NULL));
  g_object_unref (cw->priv->accel);

  cw->priv->changing_back_to_local_after_a_call = false;
  cw->priv->automatic_zoom_in = false;

  cw->priv->transfer_call_popup = NULL;
  cw->priv->current_call = boost::shared_ptr<Ekiga::Call>();
  cw->priv->timeout_id = -1;
  cw->priv->levelmeter_timeout_id = -1;
  cw->priv->calling_state = Standby;
  cw->priv->fullscreen = false;
#ifndef WIN32
  cw->priv->gc = NULL;
#endif
  cw->priv->video_display_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (VIDEO_DISPLAY_SCHEMA));

  g_signal_connect (cw, "delete_event",
		    G_CALLBACK (ekiga_call_window_delete_event_cb), NULL);
}

static void
ekiga_call_window_finalize (GObject *gobject)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (gobject);

  gtk_widget_destroy (cw->priv->audio_settings_window);
  gtk_widget_destroy (cw->priv->video_settings_window);
  gtk_widget_destroy (cw->priv->ext_video_win);

  delete cw->priv;

  G_OBJECT_CLASS (ekiga_call_window_parent_class)->finalize (gobject);
}

static void
ekiga_call_window_show (GtkWidget *widget)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (widget);

  gtk_window_set_keep_above (GTK_WINDOW (cw),
                             cw->priv->video_display_settings->get_bool ("stay-on-top"));
  GTK_WIDGET_CLASS (ekiga_call_window_parent_class)->show (widget);

  gtk_widget_queue_draw (GTK_WIDGET (cw));
}

static gboolean
ekiga_call_window_draw (GtkWidget *widget,
                        cairo_t *context)
{
  return true; //FIXME
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (widget);
  GtkWidget* video_widget = cw->priv->main_video_image;
  Ekiga::DisplayInfo display_info;
  gboolean handled = false;
  GtkAllocation a;

  handled = (*GTK_WIDGET_CLASS (ekiga_call_window_parent_class)->draw) (widget, context);

  gtk_widget_get_allocation (video_widget, &a);
  display_info.x = a.x;
  display_info.y = a.y;

#ifdef WIN32
  display_info.hwnd = ((HWND) GDK_WINDOW_HWND (gtk_widget_get_window (video_widget)));
#else
  display_info.window = gdk_x11_window_get_xid (gtk_widget_get_window (video_widget));
  g_return_val_if_fail (display_info.window != 0, handled);

  if (!cw->priv->gc) {
    Display *display;
    display = GDK_DISPLAY_XDISPLAY (gtk_widget_get_display (video_widget));
    cw->priv->gc = XCreateGC(display, display_info.window, 0, 0);
    g_return_val_if_fail (cw->priv->gc != NULL, handled);
  }
  display_info.gc = cw->priv->gc;

  gdk_flush();
#endif

  display_info.widget_info_set = true;

  return handled;
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
  widget_class->draw = ekiga_call_window_draw;
  widget_class->focus_in_event = ekiga_call_window_focus_in_event;
}

GtkWidget *
call_window_new (Ekiga::ServiceCore & core)
{
  EkigaCallWindow *cw;

  cw = EKIGA_CALL_WINDOW (g_object_new (EKIGA_TYPE_CALL_WINDOW,
					"key", USER_INTERFACE ".call-window",
					"hide_on_delete", false,
					"hide_on_esc", false, NULL));

  cw->priv->libnotify = core.get ("libnotify");
  cw->priv->videoinput_core = core.get<Ekiga::VideoInputCore> ("videoinput-core");
  cw->priv->videooutput_core = core.get<Ekiga::VideoOutputCore> ("videooutput-core");
  cw->priv->audioinput_core = core.get<Ekiga::AudioInputCore> ("audioinput-core");
  cw->priv->audiooutput_core = core.get<Ekiga::AudioOutputCore> ("audiooutput-core");
  cw->priv->call_core = core.get<Ekiga::CallCore> ("call-core");

  ekiga_call_window_connect_engine_signals (cw);

  ekiga_call_window_init_gui (cw);

  g_signal_connect (cw->priv->video_display_settings->get_g_settings (),
                    "changed::stay-on-top",
                    G_CALLBACK (stay_on_top_changed_cb), cw);

  gtk_window_set_title (GTK_WINDOW (cw), _("Call Window"));

  return GTK_WIDGET (cw);
}
