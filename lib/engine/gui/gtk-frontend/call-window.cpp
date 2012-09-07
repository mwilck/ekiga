
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

#include "call-window.h"

#include "dialpad.h"

#include "gmdialog.h"
#include "gmentrydialog.h"
#include "gmstatusbar.h"
#include "gmstockicons.h"
#include "gmconf.h"
#include <boost/smart_ptr.hpp>
#include "gmmenuaddon.h"
#include "gmlevelmeter.h"
#include "gmpowermeter.h"
#include "trigger.h"
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

#include "gtk-frontend.h"

#include <algorithm>

enum CallingState {Standby, Calling, Ringing, Connected, Called};

enum DeviceType {AudioInput, AudioOutput, Ringer, VideoInput};
struct deviceStruct {
  char name[256];
  DeviceType deviceType;
};

G_DEFINE_TYPE (EkigaCallWindow, ekiga_call_window, GM_TYPE_WINDOW);

#define USER_INTERFACE_KEY "/apps/" PACKAGE_NAME "/general/user_interface/"
#define VIDEO_DISPLAY_KEY USER_INTERFACE_KEY "video_display/"
#define VIDEO_DEVICES_KEY "/apps/" PACKAGE_NAME "/devices/video/"

struct _EkigaCallWindowPrivate
{
  Ekiga::ServiceCore *core;
  GtkAccelGroup *accel;

  boost::shared_ptr<Ekiga::Call> current_call;
  unsigned calling_state;

  GtkWidget *ext_video_win;
  GtkWidget *main_video_image;
  GtkWidget *spinner;
  GtkWidget *info_text;

  GtkWidget *call_frame;
  GtkWidget *camera_image;

  GtkWidget *main_menu;
  GtkWidget *call_panel_toolbar;
  GtkWidget *pick_up_button;
  GtkWidget *hang_up_button;
  GtkWidget *hold_button;
  GtkWidget *audio_settings_button;
  GtkWidget *video_settings_button;
#ifndef WIN32
  GdkGC* video_widget_gc;
#endif

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

  unsigned int levelmeter_timeout_id;
  unsigned int timeout_id;

  GtkWidget *video_settings_window;
  GtkWidget *video_settings_frame;
#if GTK_CHECK_VERSION (3, 0, 0)
  GtkWidget *adj_whiteness;
  GtkAdjustment *adj_brightness;
  GtkAdjustment *adj_colour;
  GtkAdjustment *adj_contrast;
#else
  GtkObject *adj_whiteness;
  GtkObject *adj_brightness;
  GtkObject *adj_colour;
  GtkObject *adj_contrast;
#endif

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

  std::vector<boost::signals::connection> connections;
};

/* properties */
enum {
  PROP_0,
  PROP_SERVICE_CORE
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

static void stay_on_top_changed_nt (gpointer id,
                                    GmConfEntry *entry,
                                    gpointer data);

static void pick_up_call_cb (GtkWidget * /*widget*/,
                            gpointer data);

static void hang_up_call_cb (GtkWidget * /*widget*/,
                            gpointer data);

static void show_window_cb (GtkWidget *widget,
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
                                             Ekiga::VideoOutputAccel /* accel */,
                                             Ekiga::VideoOutputMode mode,
                                             unsigned zoom,
                                             bool both_streams,
                                             bool ext_stream,
                                             gpointer self);

static void on_videooutput_device_closed_cb (Ekiga::VideoOutputManager & /* manager */,
                                             gpointer self);

static void on_videooutput_device_error_cb (Ekiga::VideoOutputManager & /* manager */,
                                            Ekiga::VideoOutputErrorCodes error_code,
                                            gpointer self);

static void on_fullscreen_mode_changed_cb (Ekiga::VideoOutputManager & /* manager */,
                                           Ekiga::VideoOutputFSToggle toggle,
                                           gpointer self);

static void ekiga_call_window_set_video_size (EkigaCallWindow *cw,
                                              int width,
                                              int height);

static void on_size_changed_cb (Ekiga::VideoOutputManager & /* manager */,
                                unsigned width,
                                unsigned height,
                                Ekiga::VideoOutputMode mode,
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

static void window_closed_from_menu_cb (G_GNUC_UNUSED GtkWidget *,
                                        gpointer);

/**/
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

static GtkWidget * gm_cw_audio_settings_window_new (EkigaCallWindow *cw);

static GtkWidget *gm_cw_video_settings_window_new (EkigaCallWindow *cw);

static void ekiga_call_window_update_logo (EkigaCallWindow *cw);

static void ekiga_call_window_toggle_fullscreen (Ekiga::VideoOutputFSToggle toggle);

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
  Ekiga::ServicePtr libnotify = cw->priv->core->get ("libnotify");

  if (libnotify) {

    boost::optional<bool> val = libnotify->get_bool_property ("actions");
    if (val) {

      result = *val;
    }
  }
  return result;
}

static void
stay_on_top_changed_nt (G_GNUC_UNUSED gpointer id,
                        GmConfEntry *entry,
                        gpointer data)
{
  bool val = false;

  g_return_if_fail (data != NULL);

  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    val = gm_conf_entry_get_bool (entry);
    gdk_window_set_keep_above (GTK_WIDGET (data)->window, val);
  }
}


static void
zoom_in_changed_cb (G_GNUC_UNUSED GtkWidget *widget,
		    gpointer data)
{
  g_return_if_fail (data != NULL);

  Ekiga::DisplayInfo display_info;
  ekiga_call_window_set_video_size (EKIGA_CALL_WINDOW (data), GM_CIF_WIDTH, GM_CIF_HEIGHT);

  display_info.zoom = gm_conf_get_int (VIDEO_DISPLAY_KEY "zoom");

  if (display_info.zoom < 200)
    display_info.zoom = display_info.zoom * 2;

  gm_conf_set_int (VIDEO_DISPLAY_KEY "zoom", display_info.zoom);
  ekiga_call_window_zooms_menu_update_sensitivity (EKIGA_CALL_WINDOW (data), display_info.zoom);
}

static void
zoom_out_changed_cb (G_GNUC_UNUSED GtkWidget *widget,
		     gpointer data)
{
  g_return_if_fail (data != NULL);

  Ekiga::DisplayInfo display_info;
  ekiga_call_window_set_video_size (EKIGA_CALL_WINDOW (data), GM_CIF_WIDTH, GM_CIF_HEIGHT);

  display_info.zoom = gm_conf_get_int (VIDEO_DISPLAY_KEY "zoom");

  if (display_info.zoom  > 50)
    display_info.zoom  = (unsigned int) (display_info.zoom  / 2);

  gm_conf_set_int (VIDEO_DISPLAY_KEY "zoom", display_info.zoom);
  ekiga_call_window_zooms_menu_update_sensitivity (EKIGA_CALL_WINDOW (data), display_info.zoom);
}

static void
zoom_normal_changed_cb (G_GNUC_UNUSED GtkWidget *widget,
			gpointer data)
{
  g_return_if_fail (data != NULL);

  Ekiga::DisplayInfo display_info;
  ekiga_call_window_set_video_size (EKIGA_CALL_WINDOW (data), GM_CIF_WIDTH, GM_CIF_HEIGHT);

  display_info.zoom  = 100;

  gm_conf_set_int (VIDEO_DISPLAY_KEY "zoom", display_info.zoom);
  ekiga_call_window_zooms_menu_update_sensitivity (EKIGA_CALL_WINDOW (data), display_info.zoom);
}

static void
display_changed_cb (GtkWidget *widget,
		    gpointer data)
{
  g_return_if_fail (data != NULL);

  GSList *group = NULL;
  int group_last_pos = 0;
  int active = 0;

  group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (widget));
  group_last_pos = g_slist_length (group) - 1; /* If length 1, last pos is 0 */

  /* Only do something when a new CHECK_MENU_ITEM becomes active,
     not when it becomes inactive */
  if (GTK_CHECK_MENU_ITEM (widget)->active) {

    while (group) {
      if (group->data == widget)
        break;

      active++;
      group = g_slist_next (group);
    }

    if (!EKIGA_CALL_WINDOW (data)->priv->changing_back_to_local_after_a_call) {
      int view = group_last_pos - active;
      if (view > 2) /* let's skip VO_MODE_PIP_WINDOW & VO_MODE_FULLSCREEN modes
                       which are not found in the View menu */
        view += 2;
      gm_conf_set_int (VIDEO_DISPLAY_KEY "video_view", view);
    }
  }
}

static void
fullscreen_changed_cb (G_GNUC_UNUSED GtkWidget *widget,
		       G_GNUC_UNUSED gpointer data)
{
  ekiga_call_window_toggle_fullscreen (Ekiga::VO_FS_TOGGLE);
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
show_window_cb (G_GNUC_UNUSED GtkWidget *widget,
		gpointer data)
{
  gm_window_show (GTK_WIDGET (data));
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

  boost::shared_ptr<Ekiga::AudioInputCore> audioinput_core = cw->priv->core->get<Ekiga::AudioInputCore> ("audioinput-core");
  boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core = cw->priv->core->get<Ekiga::AudioOutputCore> ("audiooutput-core");

  audiooutput_core->set_volume (Ekiga::primary, (unsigned) GTK_ADJUSTMENT (cw->priv->adj_output_volume)->value);
  audioinput_core->set_volume ((unsigned) GTK_ADJUSTMENT (cw->priv->adj_input_volume)->value);
}

static void
audio_volume_window_shown_cb (GtkWidget * /*widget*/,
	                      gpointer data)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (data);

  boost::shared_ptr<Ekiga::AudioInputCore> audioinput_core = cw->priv->core->get<Ekiga::AudioInputCore> ("audioinput-core");
  boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core = cw->priv->core->get<Ekiga::AudioOutputCore> ("audiooutput-core");

  audioinput_core->set_average_collection (true);
  audiooutput_core->set_average_collection (true);
  cw->priv->levelmeter_timeout_id = g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, 50, on_signal_level_refresh_cb, data, NULL);
}

static void
audio_volume_window_hidden_cb (GtkWidget * /*widget*/,
                               gpointer data)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (data);

  boost::shared_ptr<Ekiga::AudioInputCore> audioinput_core = cw->priv->core->get<Ekiga::AudioInputCore> ("audioinput-core");
  boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core = cw->priv->core->get<Ekiga::AudioOutputCore> ("audiooutput-core");

  g_source_remove (cw->priv->levelmeter_timeout_id);
  audioinput_core->set_average_collection (false);
  audiooutput_core->set_average_collection (false);
}

static void
video_settings_changed_cb (GtkAdjustment * /*adjustment*/,
			   gpointer data)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (data);

  boost::shared_ptr<Ekiga::VideoInputCore> videoinput_core = cw->priv->core->get<Ekiga::VideoInputCore> ("videoinput-core");

  videoinput_core->set_whiteness ((unsigned) GTK_ADJUSTMENT (cw->priv->adj_whiteness)->value);
  videoinput_core->set_brightness ((unsigned) GTK_ADJUSTMENT (cw->priv->adj_brightness)->value);
  videoinput_core->set_colour ((unsigned) GTK_ADJUSTMENT (cw->priv->adj_colour)->value);
  videoinput_core->set_contrast ((unsigned) GTK_ADJUSTMENT (cw->priv->adj_contrast)->value);
}

static gboolean
on_signal_level_refresh_cb (gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  boost::shared_ptr<Ekiga::AudioInputCore> audioinput_core = cw->priv->core->get<Ekiga::AudioInputCore> ("audioinput-core");
  boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core = cw->priv->core->get<Ekiga::AudioOutputCore> ("audiooutput-core");

  gm_level_meter_set_level (GM_LEVEL_METER (cw->priv->output_signal), audiooutput_core->get_average_level());
  gm_level_meter_set_level (GM_LEVEL_METER (cw->priv->input_signal), audioinput_core->get_average_level());
  return true;
}

static void
on_videooutput_device_opened_cb (Ekiga::VideoOutputManager & /* manager */,
                                 Ekiga::VideoOutputAccel /* accel */,
                                 Ekiga::VideoOutputMode mode,
                                 unsigned zoom,
                                 bool both_streams,
                                 bool ext_stream,
                                 gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);
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
  vv = gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view");
  cw->priv->changing_back_to_local_after_a_call = true;
  gtk_radio_menu_select_with_id (cw->priv->main_menu, "local_video", mode);
  cw->priv->changing_back_to_local_after_a_call = false;
  if (!both_streams && mode == Ekiga::VO_MODE_LOCAL)
    gm_conf_set_int (VIDEO_DISPLAY_KEY "video_view", vv);

  // if in a past video we left in the extended video stream, but the new
  // one doesn't have it, we reset the view to the local one
  if (vv == Ekiga::VO_MODE_REMOTE_EXT && !ext_stream)
    gm_conf_set_int (VIDEO_DISPLAY_KEY "video_view", Ekiga::VO_MODE_LOCAL);

  ekiga_call_window_zooms_menu_update_sensitivity (cw, zoom);
}

static void
on_videooutput_device_closed_cb (Ekiga::VideoOutputManager & /* manager */, gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  gtk_menu_section_set_sensitive (cw->priv->main_menu, "local_video", false);
  gtk_menu_section_set_sensitive (cw->priv->main_menu, "fullscreen", true);
  gtk_menu_section_set_sensitive (cw->priv->main_menu, "zoom_in", false);
}

//FIXME Set_stay_on_top "window_show object"

static void
on_videooutput_device_error_cb (Ekiga::VideoOutputManager & /* manager */,
                                Ekiga::VideoOutputErrorCodes error_code,
                                gpointer self)
{
  const gchar *dialog_title =  _("Error while initializing video output");
  const gchar *tmp_msg = _("No video will be displayed on your machine during this call");
  gchar *dialog_msg = NULL;

  switch (error_code) {

    case Ekiga::VO_ERROR_NONE:
      break;
    case Ekiga::VO_ERROR:
    default:
#ifdef WIN32
      dialog_msg = g_strconcat (_("There was an error opening or initializing the video output. Please verify that no other application is using the accelerated video output."), "\n\n", tmp_msg, NULL);
#else
      dialog_msg = g_strconcat (_("There was an error opening or initializing the video output. Please verify that you are using a color depth of 24 or 32 bits per pixel."), "\n\n", tmp_msg, NULL);
#endif
      break;
  }

  gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (self),
                                         "show_device_warnings",
                                         dialog_title,
                                         "%s", dialog_msg);
  g_free (dialog_msg);
}

static void
on_fullscreen_mode_changed_cb (G_GNUC_UNUSED Ekiga::VideoOutputManager & manager,
                               Ekiga::VideoOutputFSToggle toggle,
                               G_GNUC_UNUSED gpointer self)
{
  ekiga_call_window_toggle_fullscreen (toggle);
}

static void
ekiga_call_window_set_video_size (EkigaCallWindow *cw,
                                  int width,
                                  int height)
{
  int pw, ph;

  g_return_if_fail (width > 0 && height > 0);

  if (width < GM_CIF_WIDTH && height < GM_CIF_HEIGHT && !cw->priv->automatic_zoom_in) {
    cw->priv->automatic_zoom_in = true;
    zoom_in_changed_cb (NULL, (gpointer) cw);
  }

  gtk_widget_get_size_request (cw->priv->main_video_image, &pw, &ph);

  /* No size requisition yet
   * It's our first call so we silently set the new requisition and exit...
   */
  if (pw == -1) {
    gtk_widget_set_size_request (cw->priv->main_video_image, width, height);
    return;
  }

  /* Do some kind of filtering here. We often get duplicate "size-changed" events...
   * Note that we currently only bother about the width of the video.
   */
  if (pw == width)
    return;

  gtk_widget_set_size_request (cw->priv->main_video_image, width, height);

  gdk_window_invalidate_rect (GTK_WIDGET (cw)->window, &(GTK_WIDGET (cw)->allocation), true);
}

static void
on_size_changed_cb (Ekiga::VideoOutputManager & /* manager */,
                    unsigned width,
                    unsigned height,
                    Ekiga::VideoOutputMode mode,
                    gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

  if (mode == Ekiga::VO_MODE_REMOTE_EXT && cw->priv->ext_video_win) {
    ekiga_ext_window_set_size (EKIGA_EXT_WINDOW (cw->priv->ext_video_win),
                               width, height);
    gtk_widget_show (cw->priv->ext_video_win);
    return;
  }

  ekiga_call_window_set_video_size (EKIGA_CALL_WINDOW (cw), width, height);
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
  GTK_ADJUSTMENT (cw->priv->adj_whiteness)->value = settings.whiteness;
  GTK_ADJUSTMENT (cw->priv->adj_brightness)->value = settings.brightness;
  GTK_ADJUSTMENT (cw->priv->adj_colour)->value = settings.colour;
  GTK_ADJUSTMENT (cw->priv->adj_contrast)->value = settings.contrast;

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
  gchar *dialog_title = NULL;
  gchar *dialog_msg = NULL;
  gchar *tmp_msg = NULL;

  dialog_title =
  g_strdup_printf (_("Error while accessing video device %s"),
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

  gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (GTK_WIDGET (self)),
                                         "show_device_warnings",
                                         dialog_title,
                                         "%s", dialog_msg);
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
  GTK_ADJUSTMENT (cw->priv->adj_input_volume)->value = settings.volume;

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

  gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (self),
                                         "show_device_warnings",
                                         dialog_title,
                                         "%s", dialog_msg);
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
  GTK_ADJUSTMENT (cw->priv->adj_output_volume)->value = settings.volume;

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

  gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (GTK_WIDGET (self)),
                                         "show_device_warnings",
                                         dialog_title,
                                         "%s", dialog_msg);
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

  if (gm_conf_get_bool (VIDEO_DISPLAY_KEY "stay_on_top"))
    gdk_window_set_keep_above (GTK_WIDGET (cw)->window, true);
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

  if (gm_conf_get_bool (VIDEO_DISPLAY_KEY "stay_on_top"))
    gdk_window_set_keep_above (GTK_WIDGET (cw)->window, false);
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
                               boost::shared_ptr<Ekiga::Call> /*call*/,
                               gpointer self)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (self);

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
    boost::shared_ptr<Ekiga::VideoOutputCore> videooutput_core = cw->priv->core->get<Ekiga::VideoOutputCore> ("videooutput-core");
    videooutput_core->get_videooutput_stats(videooutput_stats);

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

  cw = EKIGA_CALL_WINDOW (widget);
  g_return_val_if_fail (EKIGA_IS_CALL_WINDOW (cw), false);

  /* Hang up or disable preview */
  if (cw->priv->calling_state != Standby && cw->priv->current_call) {
    cw->priv->current_call->hang_up ();
  }
  else {
    gm_conf_set_bool (VIDEO_DEVICES_KEY "enable_preview", false);
  }

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

  gm_level_meter_clear (GM_LEVEL_METER (cw->priv->output_signal));
  gm_level_meter_clear (GM_LEVEL_METER (cw->priv->input_signal));
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

  child = GTK_BIN (gtk_menu_get_widget (cw->priv->main_menu, "hold_call"))->child;

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
  child = GTK_BIN (widget)->child;

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
  window = gtk_dialog_new ();
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("video_settings_window"), g_free);
  gtk_dialog_add_button (GTK_DIALOG (window),
                         GTK_STOCK_CLOSE,
                         GTK_RESPONSE_CANCEL);

  gtk_window_set_title (GTK_WINDOW (window),
                        _("Video Settings"));

  /* Webcam Control Frame, we need it to disable controls */
  cw->priv->video_settings_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (cw->priv->video_settings_frame),
			     GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (cw->priv->video_settings_frame), 5);

  /* Category */
  vbox = gtk_vbox_new (false, 0);
  gtk_container_add (GTK_CONTAINER (cw->priv->video_settings_frame), vbox);

  /* Brightness */
  hbox = gtk_hbox_new (false, 0);
  image = gtk_image_new_from_icon_name ("brightness", GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, false, false, 0);

  cw->priv->adj_brightness = gtk_adjustment_new (brightness, 0.0,
                                                 255.0, 1.0, 5.0, 1.0);
  hscale_brightness = gtk_hscale_new (GTK_ADJUSTMENT (cw->priv->adj_brightness));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_brightness), false);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_brightness), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_brightness, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 3);

  gtk_widget_set_tooltip_text (hscale_brightness, _("Adjust brightness"));

  g_signal_connect (cw->priv->adj_brightness, "value-changed",
		    G_CALLBACK (video_settings_changed_cb),
		    (gpointer) cw);

  /* Whiteness */
  hbox = gtk_hbox_new (false, 0);
  image = gtk_image_new_from_icon_name ("whiteness", GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, false, false, 0);

  cw->priv->adj_whiteness = gtk_adjustment_new (whiteness, 0.0,
						255.0, 1.0, 5.0, 1.0);
  hscale_whiteness = gtk_hscale_new (GTK_ADJUSTMENT (cw->priv->adj_whiteness));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_whiteness), false);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_whiteness), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_whiteness, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 3);

  gtk_widget_set_tooltip_text (hscale_whiteness, _("Adjust whiteness"));

  g_signal_connect (cw->priv->adj_whiteness, "value-changed",
		    G_CALLBACK (video_settings_changed_cb),
		    (gpointer) cw);

  /* Colour */
  hbox = gtk_hbox_new (false, 0);
  image = gtk_image_new_from_icon_name ("color", GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, false, false, 0);

  cw->priv->adj_colour = gtk_adjustment_new (colour, 0.0,
					     255.0, 1.0, 5.0, 1.0);
  hscale_colour = gtk_hscale_new (GTK_ADJUSTMENT (cw->priv->adj_colour));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_colour), false);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_colour), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_colour, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 3);

  gtk_widget_set_tooltip_text (hscale_colour, _("Adjust color"));

  g_signal_connect (cw->priv->adj_colour, "value-changed",
		    G_CALLBACK (video_settings_changed_cb),
		    (gpointer) cw);

  /* Contrast */
  hbox = gtk_hbox_new (false, 0);
  image = gtk_image_new_from_icon_name ("contrast", GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, false, false, 0);

  cw->priv->adj_contrast = gtk_adjustment_new (contrast, 0.0,
					       255.0, 1.0, 5.0, 1.0);
  hscale_contrast = gtk_hscale_new (GTK_ADJUSTMENT (cw->priv->adj_contrast));
  gtk_scale_set_draw_value (GTK_SCALE (hscale_contrast), false);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_contrast), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_contrast, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 3);

  gtk_widget_set_tooltip_text (hscale_contrast, _("Adjust contrast"));

  g_signal_connect (cw->priv->adj_contrast, "value-changed",
		    G_CALLBACK (video_settings_changed_cb),
		    (gpointer) cw);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (window)->vbox),
                     cw->priv->video_settings_frame);
  gtk_widget_show_all (cw->priv->video_settings_frame);

  gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->video_settings_frame), false);

  /* That's an usual GtkWindow, connect it to the signals */
  g_signal_connect_swapped (window,
			    "response",
			    G_CALLBACK (gm_window_hide),
			    (gpointer) window);

  gm_window_hide_on_delete (window);

  return window;
}

static GtkWidget *
gm_cw_audio_settings_window_new (EkigaCallWindow *cw)
{
  GtkWidget *hscale_play = NULL;
  GtkWidget *hscale_rec = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *small_vbox = NULL;
  GtkWidget *window = NULL;

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
  cw->priv->audio_output_volume_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (cw->priv->audio_output_volume_frame),
			     GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (cw->priv->audio_output_volume_frame), 5);


  /* The vbox */
  vbox = gtk_vbox_new (false, 0);
  gtk_container_add (GTK_CONTAINER (cw->priv->audio_output_volume_frame), vbox);

  /* Output volume */
  hbox = gtk_hbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (hbox),
		      gtk_image_new_from_icon_name ("audio-volume", GTK_ICON_SIZE_SMALL_TOOLBAR),
		      false, false, 0);

  small_vbox = gtk_vbox_new (false, 0);
  cw->priv->adj_output_volume = gtk_adjustment_new (0, 0.0, 101.0, 1.0, 5.0, 1.0);
  hscale_play = gtk_hscale_new (GTK_ADJUSTMENT (cw->priv->adj_output_volume));
  gtk_scale_set_value_pos (GTK_SCALE (hscale_play), GTK_POS_RIGHT);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_play), false);
  gtk_box_pack_start (GTK_BOX (small_vbox), hscale_play, true, true, 0);

  cw->priv->output_signal = gm_level_meter_new ();
  gtk_box_pack_start (GTK_BOX (small_vbox), cw->priv->output_signal, true, true, 0);
  gtk_box_pack_start (GTK_BOX (hbox), small_vbox, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 3);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (window)->vbox),
                     cw->priv->audio_output_volume_frame);
  gtk_widget_show_all (cw->priv->audio_output_volume_frame);
  gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->audio_output_volume_frame),  false);

  /* Audio control frame, we need it to disable controls */
  cw->priv->audio_input_volume_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (cw->priv->audio_input_volume_frame),
			     GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (cw->priv->audio_input_volume_frame), 5);

  /* The vbox */
  vbox = gtk_vbox_new (false, 0);
  gtk_container_add (GTK_CONTAINER (cw->priv->audio_input_volume_frame), vbox);

  /* Input volume */
  hbox = gtk_hbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (hbox),
		      gtk_image_new_from_icon_name ("audio-input-microphone",
						    GTK_ICON_SIZE_SMALL_TOOLBAR),
		      false, false, 0);

  small_vbox = gtk_vbox_new (false, 0);
  cw->priv->adj_input_volume = gtk_adjustment_new (0, 0.0, 101.0, 1.0, 5.0, 1.0);
  hscale_rec = gtk_hscale_new (GTK_ADJUSTMENT (cw->priv->adj_input_volume));
  gtk_scale_set_value_pos (GTK_SCALE (hscale_rec), GTK_POS_RIGHT);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_rec), false);
  gtk_box_pack_start (GTK_BOX (small_vbox), hscale_rec, true, true, 0);

  cw->priv->input_signal = gm_level_meter_new ();
  gtk_box_pack_start (GTK_BOX (small_vbox), cw->priv->input_signal, true, true, 0);
  gtk_box_pack_start (GTK_BOX (hbox), small_vbox, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 3);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (window)->vbox),
                     cw->priv->audio_input_volume_frame);
  gtk_widget_show_all (cw->priv->audio_input_volume_frame);
  gtk_widget_set_sensitive (GTK_WIDGET (cw->priv->audio_input_volume_frame),  false);

  g_signal_connect (cw->priv->adj_output_volume, "value-changed",
		    G_CALLBACK (audio_volume_changed_cb), cw);

  g_signal_connect (cw->priv->adj_input_volume, "value-changed",
		    G_CALLBACK (audio_volume_changed_cb), cw);

  /* That's an usual GtkWindow, connect it to the signals */
  g_signal_connect_swapped (window,
			    "response",
			    G_CALLBACK (gm_window_hide),
			    (gpointer) window);

  gm_window_hide_on_delete (window);

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
		     NULL, GDK_h,
		     G_CALLBACK (hold_current_call_cb), cw,
		     false),
      GTK_MENU_ENTRY("transfer_call", _("_Transfer Call"),
		     _("Transfer the current call"),
 		     NULL, GDK_t,
		     G_CALLBACK (transfer_current_call_cb), cw,
		     false),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("suspend_audio", _("Suspend _Audio"),
		     _("Suspend or resume the audio transmission"),
		     NULL, GDK_m,
		     G_CALLBACK (toggle_audio_stream_pause_cb),
		     cw, false),
      GTK_MENU_ENTRY("suspend_video", _("Suspend _Video"),
		     _("Suspend or resume the video transmission"),
		     NULL, GDK_p,
		     G_CALLBACK (toggle_video_stream_pause_cb),
		     cw, false),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("close", NULL, _("Close the Ekiga window"),
		     GTK_STOCK_CLOSE, 'W',
		     G_CALLBACK (window_closed_from_menu_cb),
		     cw, TRUE),

      GTK_MENU_NEW(_("_View")),

      GTK_MENU_RADIO_ENTRY("local_video", _("_Local Video"),
			   _("Local video image"),
			   NULL, '1',
			   G_CALLBACK (display_changed_cb), cw,
			   true, false),
      GTK_MENU_RADIO_ENTRY("remote_video", _("_Remote Video"),
			   _("Remote video image"),
			   NULL, '2',
			   G_CALLBACK (display_changed_cb), cw,
			   false, false),
      GTK_MENU_RADIO_ENTRY("both_incrusted", _("_Picture-in-Picture"),
			   _("Both video images"),
			   NULL, '3',
			   G_CALLBACK (display_changed_cb), cw,
			   false, false),
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
		     GTK_STOCK_ZOOM_IN, GDK_F11,
		     G_CALLBACK (fullscreen_changed_cb),
		     (gpointer) cw, false),

      GTK_MENU_END
    };


  gtk_build_menu (cw->priv->main_menu,
		  gnomemeeting_menu,
		  cw->priv->accel,
		  cw->priv->statusbar);

  gtk_widget_show_all (GTK_WIDGET (cw->priv->main_menu));
}

static void
ekiga_call_window_update_logo (EkigaCallWindow *cw)
{
  g_return_if_fail (EKIGA_IS_CALL_WINDOW (cw));

  gtk_widget_realize (GTK_WIDGET (cw));
  g_object_set (G_OBJECT (cw->priv->main_video_image),
                "icon-name", "avatar-default",
                "pixel-size", 128,
                NULL);

  ekiga_call_window_set_video_size (cw, GM_CIF_WIDTH, GM_CIF_HEIGHT);
}

static void
ekiga_call_window_toggle_fullscreen (Ekiga::VideoOutputFSToggle toggle)
{
  Ekiga::VideoOutputMode videooutput_mode;

  switch (toggle) {
    case Ekiga::VO_FS_OFF:
      if (gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view") == Ekiga::VO_MODE_FULLSCREEN) {

        videooutput_mode = (Ekiga::VideoOutputMode) gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view_before_fullscreen");
        gm_conf_set_int (VIDEO_DISPLAY_KEY "video_view", videooutput_mode);
      }
      break;
    case Ekiga::VO_FS_ON:
      if (gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view") != Ekiga::VO_MODE_FULLSCREEN) {

        videooutput_mode = (Ekiga::VideoOutputMode) gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view");
        gm_conf_set_int (VIDEO_DISPLAY_KEY "video_view_before_fullscreen", videooutput_mode);
        gm_conf_set_int (VIDEO_DISPLAY_KEY "video_view", Ekiga::VO_MODE_FULLSCREEN);
      }
      break;

    case Ekiga::VO_FS_TOGGLE:
    default:
      if (gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view") == Ekiga::VO_MODE_FULLSCREEN) {

        videooutput_mode = (Ekiga::VideoOutputMode) gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view_before_fullscreen");
        gm_conf_set_int (VIDEO_DISPLAY_KEY "video_view", videooutput_mode);
      }
      else {

        videooutput_mode =  (Ekiga::VideoOutputMode) gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view");
        gm_conf_set_int (VIDEO_DISPLAY_KEY "video_view_before_fullscreen", videooutput_mode);
        gm_conf_set_int (VIDEO_DISPLAY_KEY "video_view", Ekiga::VO_MODE_FULLSCREEN);
      }
      break;
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

  if (u && !strcmp (u, ""))
    gm_entry_dialog_set_text (GM_ENTRY_DIALOG (cw->priv->transfer_call_popup), u);
  else
    gm_entry_dialog_set_text (GM_ENTRY_DIALOG (cw->priv->transfer_call_popup), "sip:");

  gm_window_show (cw->priv->transfer_call_popup);

  answer = gtk_dialog_run (GTK_DIALOG (cw->priv->transfer_call_popup));
  switch (answer) {

  case GTK_RESPONSE_ACCEPT:

    forward_url = gm_entry_dialog_get_text (GM_ENTRY_DIALOG (cw->priv->transfer_call_popup));
    if (strcmp (forward_url, "") && cw->priv->current_call)
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
  boost::signals::connection conn;

  g_return_if_fail (EKIGA_IS_CALL_WINDOW (cw));

  /* New Display Engine signals */
  boost::shared_ptr<Ekiga::VideoOutputCore> videooutput_core = cw->priv->core->get<Ekiga::VideoOutputCore> ("videooutput-core");

  conn = videooutput_core->device_opened.connect (boost::bind (&on_videooutput_device_opened_cb, _1, _2, _3, _4, _5, _6, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = videooutput_core->device_closed.connect (boost::bind (&on_videooutput_device_closed_cb, _1, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = videooutput_core->device_error.connect (boost::bind (&on_videooutput_device_error_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = videooutput_core->size_changed.connect (boost::bind (&on_size_changed_cb, _1, _2, _3, _4, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = videooutput_core->fullscreen_mode_changed.connect (boost::bind (&on_fullscreen_mode_changed_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  /* New VideoInput Engine signals */
  boost::shared_ptr<Ekiga::VideoInputCore> videoinput_core = cw->priv->core->get<Ekiga::VideoInputCore> ("videoinput-core");

  conn = videoinput_core->device_opened.connect (boost::bind (&on_videoinput_device_opened_cb, _1, _2, _3, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = videoinput_core->device_closed.connect (boost::bind (&on_videoinput_device_closed_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = videoinput_core->device_error.connect (boost::bind (&on_videoinput_device_error_cb, _1, _2, _3, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  /* New AudioInput Engine signals */
  boost::shared_ptr<Ekiga::AudioInputCore> audioinput_core = cw->priv->core->get<Ekiga::AudioInputCore> ("audioinput-core");

  conn = audioinput_core->device_opened.connect (boost::bind (&on_audioinput_device_opened_cb, _1, _2, _3, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = audioinput_core->device_closed.connect (boost::bind (&on_audioinput_device_closed_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = audioinput_core->device_error.connect (boost::bind (&on_audioinput_device_error_cb, _1, _2, _3, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  /* New AudioOutput Engine signals */
  boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core = cw->priv->core->get<Ekiga::AudioOutputCore> ("audiooutput-core");

  conn = audiooutput_core->device_opened.connect (boost::bind (&on_audiooutput_device_opened_cb, _1, _2, _3, _4, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = audiooutput_core->device_closed.connect (boost::bind (&on_audiooutput_device_closed_cb, _1, _2, _3, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = audiooutput_core->device_error.connect (boost::bind (&on_audiooutput_device_error_cb, _1, _2, _3, _4, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  /* New Call Engine signals */
  boost::shared_ptr<Ekiga::CallCore> call_core = cw->priv->core->get<Ekiga::CallCore> ("call-core");

  /* Engine Signals callbacks */
  conn = call_core->setup_call.connect (boost::bind (&on_setup_call_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = call_core->ringing_call.connect (boost::bind (&on_ringing_call_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = call_core->established_call.connect (boost::bind (&on_established_call_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = call_core->cleared_call.connect (boost::bind (&on_cleared_call_cb, _1, _2, _3, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = call_core->missed_call.connect (boost::bind (&on_missed_call_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = call_core->held_call.connect (boost::bind (&on_held_call_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = call_core->retrieved_call.connect (boost::bind (&on_retrieved_call_cb, _1, _2, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = call_core->stream_opened.connect (boost::bind (&on_stream_opened_cb, _1, _2, _3, _4, _5, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = call_core->stream_closed.connect (boost::bind (&on_stream_closed_cb, _1, _2, _3, _4, _5, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = call_core->stream_paused.connect (boost::bind (&on_stream_paused_cb, _1, _2, _3, _4, (gpointer) cw));
  cw->priv->connections.push_back (conn);

  conn = call_core->stream_resumed.connect (boost::bind (&on_stream_resumed_cb, _1, _2, _3, _4, (gpointer) cw));
  cw->priv->connections.push_back (conn);
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
  boost::shared_ptr<Ekiga::VideoOutputCore> videooutput_core =
    cw->priv->core->get<Ekiga::VideoOutputCore> ("videooutput-core");
  cw->priv->ext_video_win = ext_window_new (videooutput_core);

  /* The main table */
  event_box = gtk_event_box_new ();
  vbox = gtk_vbox_new (false, 0);
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
  cw->priv->main_video_image = gtk_image_new ();
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (cw->priv->main_video_image), true, true, 0);
  gtk_widget_show (cw->priv->main_video_image);

  /* The frame that contains information about the call */
  cw->priv->call_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (cw->priv->call_frame), GTK_SHADOW_NONE);
  hbox = gtk_hbox_new (false, 0);

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
  boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core = cw->priv->core->get<Ekiga::AudioOutputCore> ("audiooutput-core");
  std::vector <Ekiga::AudioOutputDevice> devices;
  audiooutput_core->get_devices (devices);
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
    g_signal_connect (cw->priv->audio_settings_button, "clicked",
                      G_CALLBACK (show_window_cb),
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

  g_signal_connect (cw->priv->video_settings_button, "clicked",
		    G_CALLBACK (show_window_cb),
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
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (cw->priv->statusbar), false);
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
  gtk_widget_realize (cw->priv->main_video_image);
  gtk_window_set_resizable (GTK_WINDOW (cw), false);
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
  gtk_accel_group_connect (cw->priv->accel, GDK_Escape, (GdkModifierType) 0, GTK_ACCEL_LOCKED,
                           g_cclosure_new_swap (G_CALLBACK (ekiga_call_window_delete_event_cb), (gpointer) cw, NULL));
  g_object_unref (cw->priv->accel);

  cw->priv->changing_back_to_local_after_a_call = false;
  cw->priv->automatic_zoom_in = false;

  cw->priv->transfer_call_popup = NULL;
  cw->priv->current_call = boost::shared_ptr<Ekiga::Call>();
  cw->priv->timeout_id = -1;
  cw->priv->levelmeter_timeout_id = -1;
  cw->priv->calling_state = Standby;
#ifndef WIN32
  cw->priv->video_widget_gc = NULL;
#endif

  g_signal_connect (cw, "delete_event",
		    G_CALLBACK (ekiga_call_window_delete_event_cb), NULL);
}

static GObject *
ekiga_call_window_constructor (GType the_type,
                               guint n_construct_properties,
                               GObjectConstructParam *construct_params)
{
  GObject *object;

  object = G_OBJECT_CLASS (ekiga_call_window_parent_class)->constructor
                          (the_type, n_construct_properties, construct_params);

  ekiga_call_window_init_gui (EKIGA_CALL_WINDOW (object));

  gm_conf_notifier_add (VIDEO_DISPLAY_KEY "stay_on_top",
			stay_on_top_changed_nt, object);

  return object;
}

static void
ekiga_call_window_dispose (GObject* gobject)
{
  G_OBJECT_CLASS (ekiga_call_window_parent_class)->dispose (gobject);
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
  if (gm_conf_get_bool (VIDEO_DISPLAY_KEY "stay_on_top") && cw->priv->current_call)
    gdk_window_set_keep_above (widget->window, true);
  GTK_WIDGET_CLASS (ekiga_call_window_parent_class)->show (widget);

  gtk_widget_queue_draw (GTK_WIDGET (cw));
}

static gboolean
ekiga_call_window_expose_event (GtkWidget *widget,
                                GdkEventExpose *event)
{
  EkigaCallWindow *cw = EKIGA_CALL_WINDOW (widget);
  GtkWidget* video_widget = cw->priv->main_video_image;
  Ekiga::DisplayInfo display_info;
  gboolean handled = false;

  handled = GTK_WIDGET_CLASS (ekiga_call_window_parent_class)->expose_event (widget, event);

  display_info.x = video_widget->allocation.x;
  display_info.y = video_widget->allocation.y;

#ifdef WIN32
  display_info.hwnd = ((HWND) GDK_WINDOW_HWND (video_widget->window));
#else
  if (!cw->priv->video_widget_gc) {
    cw->priv->video_widget_gc = gdk_gc_new (video_widget->window);
    g_return_val_if_fail (cw->priv->video_widget_gc != NULL, handled);
  }

  display_info.gc = GDK_GC_XGC (cw->priv->video_widget_gc);
  display_info.window = GDK_WINDOW_XWINDOW (video_widget->window);

  g_return_val_if_fail (display_info.window != 0, handled);

  gdk_flush();
#endif

  display_info.widget_info_set = true;

  boost::shared_ptr<Ekiga::VideoOutputCore> videooutput_core = cw->priv->core->get<Ekiga::VideoOutputCore> ("videooutput-core");
  videooutput_core->set_display_info (display_info);

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
ekiga_call_window_get_property (GObject *object,
                                guint property_id,
                                GValue *value,
                                GParamSpec *pspec)
{
  EkigaCallWindow *cw;

  g_return_if_fail (EKIGA_IS_CALL_WINDOW (object));

  cw = EKIGA_CALL_WINDOW (object);

  switch (property_id) {
    case PROP_SERVICE_CORE:
      g_value_set_pointer (value, cw->priv->core);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
ekiga_call_window_set_property (GObject *object,
                                guint property_id,
                                const GValue *value,
                                GParamSpec *pspec)
{
  EkigaCallWindow *cw;

  g_return_if_fail (EKIGA_IS_CALL_WINDOW (object));

  cw = EKIGA_CALL_WINDOW (object);

  switch (property_id) {
    case PROP_SERVICE_CORE:
      cw->priv->core = static_cast<Ekiga::ServiceCore *> (g_value_get_pointer (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
ekiga_call_window_class_init (EkigaCallWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructor = ekiga_call_window_constructor;
  object_class->dispose = ekiga_call_window_dispose;
  object_class->finalize = ekiga_call_window_finalize;
  object_class->get_property = ekiga_call_window_get_property;
  object_class->set_property = ekiga_call_window_set_property;

  widget_class->show = ekiga_call_window_show;
  widget_class->expose_event = ekiga_call_window_expose_event;
  widget_class->focus_in_event = ekiga_call_window_focus_in_event;

  g_object_class_install_property (object_class,
                                   PROP_SERVICE_CORE,
                                   g_param_spec_pointer ("service-core",
                                                         "Service Core",
                                                         "Service Core",
                                                         (GParamFlags)
                                                         (G_PARAM_READWRITE |
                                                          G_PARAM_CONSTRUCT_ONLY)));
}

GtkWidget *
call_window_new (Ekiga::ServiceCore & core)
{
  EkigaCallWindow *cw;

  cw = EKIGA_CALL_WINDOW (g_object_new (EKIGA_TYPE_CALL_WINDOW,
                                        "service-core", &core, NULL));
  gm_window_set_key (GM_WINDOW (cw), USER_INTERFACE_KEY "call_window");
  gm_window_set_hide_on_delete (GM_WINDOW (cw), false);
  gm_window_set_hide_on_escape (GM_WINDOW (cw), false);
  ekiga_call_window_connect_engine_signals (cw);

  gtk_window_set_title (GTK_WINDOW (cw), _("Call Window"));

  return GTK_WIDGET (cw);
}
