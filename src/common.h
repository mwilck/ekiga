
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2003 Damien Sandras
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
 *                         common.h  -  description
 *                         ------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains things common to the whole soft.
 *
 */


#ifndef GM_COMMON_H_
#define GM_COMMON_H_

#include <openh323buildopts.h>
#include <ptbuildopts.h>

#include <ptlib.h>
#ifdef TRY_PLUGINS
#include <ptlib/plugins.h>
#endif
#include <h323.h>

#ifndef DISABLE_GNOME
#include <gnome.h>
#else
#include <gtk/gtk.h>
#endif

#ifndef DISABLE_GCONF
#include <gconf/gconf-client.h>
#else
#include "../lib/win32/gconf-simu.h"
#endif

#ifdef WIN32
#include <string.h>
#define strcasecmp strcmpi
#define vsnprintf _vsnprintf
#endif

#include "menu.h"


#define GENERAL_KEY         "/apps/gnomemeeting/general/"
#define CALL_CONTROL_KEY    "/apps/gnomemeeting/call_control/"
#define CALL_FORWARDING_KEY "/apps/gnomemeeting/call_forwarding/"
#define NAT_KEY             "/apps/gnomemeeting/nat/"
#define PORTS_KEY           "/apps/gnomemeeting/ports/"
#define VIEW_KEY            "/apps/gnomemeeting/view/"
#define VIDEO_DISPLAY_KEY   "/apps/gnomemeeting/video_display/"
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
#define HISTORY_KEY         "/apps/gnomemeeting/history/"
#define CALL_FORWARDING_KEY "/apps/gnomemeeting/call_forwarding/"

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
typedef struct _GmLdapWindowPage GmLdapWindowPage;
typedef struct _GmTextChat GmTextChat;
typedef struct _GmDruidWindow GmDruidWindow;
typedef struct _GmCallsHistoryWindow GmCallsHistoryWindow;
typedef struct _GmRtpData GmRtpData;


/* Type of section */
typedef enum {
  CONTACTS_SERVERS,
  CONTACTS_GROUPS
} SectionType;


/* Incoming Call Mode */
typedef enum {

  AVAILABLE,
  FREE_FOR_CHAT,
  BUSY,
  FORWARD,
  NUM_MODES
} IncomingCallMode;


/* Control Panel Section */
typedef enum {

  STATISTICS,
  DIALPAD,
  AUDIO_SETTINGS,
  VIDEO_SETTINGS,
  CLOSED,
  NUM_SECTIONS
} ControlPanelSection;


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
  GtkWidget *main_menu;
  GtkWidget *tray_popup_menu;
  GtkWidget *video_popup_menu;
  GtkObject *adj_play;
  GtkObject *adj_rec;
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
  GtkWidget *preview_button;
  GtkWidget *connect_button;
  GtkWidget *video_chan_button;
  GtkWidget *audio_chan_button;
  GtkWidget *incoming_call_popup;
  GtkWidget *stats_label;
  GtkWidget *stats_drawing_area;

#ifndef DISABLE_GNOME
  GtkWidget *druid_window;
#endif

  GdkColor colors [6];

  int          progress_timeout;

  PStringArray video_devices;
  PStringArray audio_recorder_devices;
  PStringArray audio_player_devices;
  PStringArray audio_managers;
  PStringArray video_managers;
};


struct _GmLdapWindow
{
  GtkWidget *main_menu;
  GtkWidget *notebook;
  GtkWidget *tree_view;
  GtkWidget *option_menu;
};


struct _GmLdapWindowPage
{
  GtkWidget *section_name;
  GtkWidget *tree_view;
  GtkWidget *statusbar;
  GtkWidget *option_menu;
  GtkWidget *search_entry;

  PThread *ils_browser;
  PMutex search_quit_mutex;
  
  gchar *contact_section_name;
  gint page_type;
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
  GtkWidget *audio_manager;
  GtkWidget *video_manager;
  GtkWidget *audio_player;
  GtkWidget *audio_recorder;
  GtkWidget *video_device;
  GtkWidget *gk_alias;
  GtkWidget *gk_password;
#ifndef DISABLE_GNOME
  GnomeDruidPageEdge *page_edge;
#endif
};


struct _GmCallsHistoryWindow
{
  GtkListStore *given_calls_list_store;
  GtkListStore *received_calls_list_store;
  GtkListStore *missed_calls_list_store;
};


struct _GmPrefWindow
{
  GtkTooltips  *tips;
  GtkListStore *codecs_list_store;
  GtkWidget    *show_splash;
  GtkWidget    *start_hidden;
  GtkWidget    *incoming_call_popup;
  GtkWidget    *incoming_call_sound;
  GtkWidget    *auto_clear_text_chat;
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
  GtkWidget    *audio_manager;
  GtkWidget    *audio_recorder;
  GtkWidget    *video_device; 
  GtkWidget    *video_manager;
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
  GtkWidget    *stay_on_top;
  GtkWidget    *ip_translation;
  GtkWidget    *public_ip;
};
#endif /* GM_COMMON_H */
