
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2002 Damien Sandras
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
 * begin      : Wed Mar 21 2001
 * description: This file contains all the general things.
 */


#ifndef GM_COMMON_H_
#define GM_COMMON_H_

#include "../config.h"


#ifndef DISABLE_GNOME
#include <gnome.h>
#else
#include <gtk/gtk.h>
#endif

#include <ptlib.h>
#include <iostream>

#define GENERAL_KEY         "/apps/gnomemeeting/general/"
#define VIEW_KEY            "/apps/gnomemeeting/view/"
#define DEVICES_KEY         "/apps/gnomemeeting/devices/"
#define PERSONAL_DATA_KEY   "/apps/gnomemeeting/personal_data/"
#define LDAP_KEY            "/apps/gnomemeeting/ldap/"
#define GATEKEEPER_KEY      "/apps/gnomemeeting/gatekeeper/"
#define SERVICES_KEY        "/apps/gnomemeeting/services/"
#define CALL_FORWARDING_KEY "/apps/gnomemeeting/call_forwarding/"
#define VIDEO_SETTINGS_KEY  "/apps/gnomemeeting/video_settings/"
#define AUDIO_CODECS_KEY    "/apps/gnomemeeting/audio_codecs/"
#define AUDIO_SETTINGS_KEY  "/apps/gnomemeeting/audio_settings/"
#define DEVICES_KEY         "/apps/gnomemeeting/devices/"
#define CONTACTS_KEY        "/apps/gnomemeeting/contacts/"
#define CONTACTS_GROUPS_KEY "/apps/gnomemeeting/contacts/groups/"


#define GM_CIF_WIDTH   352
#define GM_CIF_HEIGHT  288
#define GM_QCIF_WIDTH  176
#define GM_QCIF_HEIGHT 144
#define GM_SIF_WIDTH   320
#define GM_SIF_HEIGHT  240
#define GM_QSIF_WIDTH  160
#define GM_QSIF_HEIGHT 120
#define GM_FRAME_SIZE  10

#define GM_MAIN_NOTEBOOK_HIDDEN 4

#ifdef SPEEX_CODEC
#define GM_AUDIO_CODECS_NUMBER 9
#else
#define GM_AUDIO_CODECS_NUMBER 7
#endif

#define GNOMEMEETING_PAD_SMALL 1

#define GM_WINDOW(x) (GmWindow *)(x)

#ifndef _
#ifdef DISABLE_GNOME
#include <libintl.h>
#define _(x) gettext(x)
#ifdef gettext_noop
#define N_(String) gettext_noop (String)
#else
#define N_(String) (String)
#endif
#endif
#endif


typedef struct _GmWindow GmWindow;
typedef struct _GmPrefWindow GmPrefWindow;
typedef struct _GmLdapWindow GmLdapWindow;
typedef struct _GmTextChat GmTextChat;
typedef struct _GmDruidWindow GmDruidWindow;
typedef struct _GmRtpData GmRtpData;
typedef struct _GmCommandLineOptions GmCommandLineOptions;

enum {CONTACTS_SERVERS, CONTACTS_GROUPS};


struct _GmTextChat
{
  GtkWidget     *text_view;
  GtkTextBuffer *text_buffer;
  gboolean       buffer_is_empty;
};


struct _GmRtpData
{
  int   tr_audio_bytes;
  float tr_audio_speed [100];
  int   tr_audio_pos;
  float tr_video_speed [100];
  int   tr_video_pos;
  int   tr_video_bytes;
  int   re_audio_bytes;
  float re_audio_speed [100];
  int   re_audio_pos;
  int   re_video_bytes;
  float re_video_speed [100];
  int   re_video_pos;
};


struct _GmWindow
{
  GtkTooltips *tips;
  GtkObject *adj_play, *adj_rec;
  GtkObject *adj_whiteness;
  GtkObject *adj_brightness;
  GtkObject *adj_colour;
  GtkObject *adj_contrast;
  GtkWidget *docklet;
  GtkWidget *video_settings_frame;
  GtkWidget *audio_settings_frame;
  GtkWidget *statusbar;
  GtkWidget *progressbar;
  GtkWidget *remote_name;
  GtkWidget *splash_win;
  GtkWidget *combo;
  GtkWidget *history_window;
  GtkWidget *history_text_view;
  GtkWidget *main_notebook;
  GtkWidget *main_video_image;
  GtkWidget *local_video_image;
  GtkWidget *local_video_window;
  GtkWidget *remote_video_image;
  GtkWidget *remote_video_window;
  GtkWidget *video_frame;
  GtkWidget *pref_window;
  GtkWidget *ldap_window;
  GtkWidget *chat_window;
  GtkWidget *calls_history_window;
  GtkWidget *calls_history_text_view;
  GtkWidget *preview_button;
  GtkWidget *connect_button;
  GtkWidget *video_chan_button;
  GtkWidget *audio_chan_button;
  GtkWidget *speaker_phone_button;
  GtkWidget *incoming_call_popup;
  GtkWidget *stats_label;
  GtkWidget *stats_drawing_area;

#ifndef DISABLE_GNOME
  GtkWidget *druid_window;
#endif

  GdkColor colors [6];

  int          progress_timeout;
  int          cleaner_thread_count;

  PStringArray video_devices;
  PStringArray audio_recorder_devices;
  PStringArray audio_player_devices;
  PStringArray audio_mixers;
};


struct _GmLdapWindow
{
  GtkWidget *notebook;
  GtkWidget *tree_view;
  GtkWidget *option_menu;
};


struct _GmDruidWindow
{
#ifndef DISABLE_GNOME
  GnomeDruid *druid;
#endif
  GtkWidget *ils_register;
  GtkWidget *audio_test_button;
  GtkWidget *video_test_button;
  GtkWidget *enable_microtelco;
  GtkWidget *kind_of_net;
  GtkWidget *progress;
  GtkWidget *audio_player;
  GtkWidget *audio_player_mixer;
  GtkWidget *audio_recorder;
  GtkWidget *audio_recorder_mixer;
  GtkWidget *video_device;
#ifndef DISABLE_GNOME
  GnomeDruidPageEdge *page_edge;
#endif
};


struct _GmPrefWindow
{
  GtkTooltips  *tips;
  GtkListStore *codecs_list_store;
  GtkWidget    *show_splash;
  GtkWidget    *start_hidden;
  GtkWidget    *incoming_call_popup;
  GtkWidget    *incoming_call_sound;
  GtkWidget    *opt1, *opt2;
  GtkWidget    *tr_vq;
  GtkWidget    *maximum_video_bandwidth;
  GtkWidget    *tr_ub;
  GtkWidget    *bps_frame;
  GtkWidget    *tr_fps;
  GtkWidget    *vid_tr;
  GtkWidget    *vid_re;
  GtkWidget    *sd;
  GtkWidget    *min_jitter_buffer;
  GtkWidget    *max_jitter_buffer;
  GtkWidget    *video_bandwidth;
  GtkWidget    *vb;
  GtkWidget    *firstname, *entry_port;
  GtkWidget    *surname;
  GtkWidget    *mail;
  GtkWidget    *location;
  GtkWidget    *comment;
  GtkWidget    *ht;
  GtkWidget    *fs;
  GtkWidget    *uic;
  GtkWidget    *aa;
  GtkWidget    *dnd;
  GtkWidget    *ldap_server;
  GtkWidget    *ldap_port;
  GtkWidget    *ldap_visible;
  GtkWidget    *ldap;
  GtkWidget    *gk;
  GtkWidget    *gk_host;
  GtkWidget    *gk_alias;
  GtkWidget    *gk_id;
  GtkWidget    *gk_password;
  GtkWidget    *audio_player;
  GtkWidget    *audio_recorder;
  GtkWidget    *audio_player_mixer;
  GtkWidget    *audio_recorder_mixer;
  GtkWidget    *video_device; 
  GtkWidget    *video_channel;
  GtkWidget    *video_image;
  GtkWidget    *lid_device;
  GtkWidget    *lid_aec;
  GtkWidget    *lid_country;
  GtkWidget    *directory_update_button;
  GtkWidget    *video_device_apply_button;
  GtkWidget    *forward_host;
  GtkWidget    *always_forward;
  GtkWidget    *busy_forward;
  GtkWidget    *no_answer_forward;
  GtkWidget    *fullscreen_width;
  GtkWidget    *fullscreen_height;
  GtkWidget    *bilinear_filtering;
  GtkWidget    *ip_translation;
  GtkWidget    *public_ip;
};


struct _GmCommandLineOptions
{
  int    debug_level;
  gchar *url;
  int    daemon;
};


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Returns the structure of widgets of the main window.
 * PRE          :  The GtkWidget must be a pointer to the Main gnomeMeeting 
 *                 window.
 */
GmWindow *gnomemeeting_get_main_window (GtkWidget *);


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Returns the structure of widgets of the prefs window.
 * PRE          :  The GtkWidget must be a pointer to the Main gnomeMeeting 
 *                 window.
 */
GmPrefWindow *gnomemeeting_get_pref_window (GtkWidget *);


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Returns the structure of widgets of the ldap window.
 * PRE          :  The GtkWidget must be a pointer to the Main gnomeMeeting 
 *                 window.
 */
GmLdapWindow *gnomemeeting_get_ldap_window (GtkWidget *);


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Returns the structure of widgets of the chat window.
 * PRE          :  The GtkWidget must be a pointer to the Main gnomeMeeting 
 *                 window.
 */
GmTextChat *gnomemeeting_get_chat_window (GtkWidget *);


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Returns the structure of widgets of the druid window.
 * PRE          :  The GtkWidget must be a pointer to the Main gnomeMeeting 
 *                 window.
 */
GmDruidWindow *gnomemeeting_get_druid_window (GtkWidget *);


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Returns the rtp data. Only valid during calls.
 * PRE          :  The GtkWidget must be a pointer to the Main gnomeMeeting 
 *                 window.
 */
GmRtpData *gnomemeeting_get_rtp_data (GtkWidget *);

#endif /* GM_COMMON_H */
