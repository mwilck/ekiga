
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2001 Damien Sandras
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
 */

/*                            common.h
 *                       -------------------
 *   begin                : Wed Mar 21 2001
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains all the general things.
 *   email                : dsandras@seconix.com
 */


#ifndef _COMMON_H_
#define _COMMON_H_

#include <gtk/gtk.h>
#include <ptlib.h>


#define GM_CIF_WIDTH   352
#define GM_CIF_HEIGHT  288
#define GM_QCIF_WIDTH  176
#define GM_QCIF_HEIGHT 144
#define GM_SIF_WIDTH   320
#define GM_SIF_HEIGHT  240
#define GM_QSIF_WIDTH  160
#define GM_QSIF_HEIGHT 120
#define GM_FRAME_SIZE  10

#define GM_AUDIO_CODECS_NUMBER 7

#define GM_WINDOW(x) (GmWindow *)(x)


typedef struct _GmWindow GmWindow;
typedef struct _GmPrefWindow GmPrefWindow;
typedef struct _GmLdapWindow GmLdapWindow;
typedef struct _GmTextChat GmTextChat;
typedef struct _GmRtpData GmRtpData;
typedef struct _GmCommandLineOptions GmCommandLineOptions;

struct _GmTextChat
{
  GtkWidget     *text_view;
  GtkTextBuffer *text_buffer;
  gboolean       buffer_is_empty;
};


struct _GmRtpData
{
  int tr_audio_bytes;
  int tr_video_bytes;
  int re_audio_bytes;
  int re_video_bytes;
};


struct _GmWindow
{
  GtkObject *adj_play, *adj_rec;
  GtkObject *adj_whiteness;
  GtkObject *adj_brightness;
  GtkObject *adj_colour;
  GtkObject *adj_contrast;
  GtkWidget *docklet;
  GtkWidget *video_settings_frame;
  GtkWidget *statusbar;
  GtkWidget *remote_name;
  GtkWidget *splash_win;
  GtkWidget *combo;
  GtkTextBuffer *history;
  GtkWidget *history_view;
  GtkWidget *main_notebook;
  GtkWidget *video_image;
  GtkWidget *video_frame;
  GtkWidget *pref_window;
  GtkWidget *ldap_window;
  GtkWidget *chat_window;
  GtkWidget *preview_button;
  GtkWidget *connect_button;
  GtkWidget *video_chan_button;
  GtkWidget *audio_chan_button;
  GtkWidget *speaker_phone_button;
  GtkWidget *incoming_call_popup;

  int progress_timeout;
  int cleaner_thread_count;
  PStringArray video_devices;
  PStringArray audio_recorder_devices;
  PStringArray audio_player_devices;
 
  double zoom;
  gboolean fullscreen;
};


struct _GmLdapWindow
{
  GtkWidget *statusbar;
  GtkWidget *search_entry;
  GtkWidget *ils_server_combo;
  GtkWidget *notebook;
  GtkWidget *option_menu;
  GtkWidget *refresh_button;
  
  int thread_count;
};


struct _GmPrefWindow
{
  GtkWidget *show_splash;
  GtkWidget *start_hidden;
  GtkWidget *incoming_call_popup;
  GtkWidget *video_preview;
  GtkWidget *incoming_call_sound;
  GtkWidget *opt1, *opt2;
  GtkAdjustment *tr_vq_spin_adj;
  GtkWidget *tr_vq;
  GtkWidget *tr_ub;
  GtkWidget *bps_frame;
  GtkAdjustment *tr_ub_spin_adj;
  GtkWidget *re_vq;
  GtkAdjustment *re_vq_spin_adj;
  GtkAdjustment *tr_fps_spin_adj;
  GtkWidget *tr_fps;
  GtkWidget *vid_tr;
  GtkWidget *sd;
  GtkWidget *jitter_buffer;
  GtkAdjustment *video_bandwidth_spin_adj;
  GtkWidget *video_bandwidth;
  GtkWidget *vb;
  GtkWidget *fps;
  GtkWidget *firstname, *entry_port;
  GtkWidget *surname;
  GtkWidget *mail;
  GtkWidget *location;
  GtkWidget *comment;
  GtkWidget *ht;
  GtkWidget *fs;
  GtkAdjustment *bps_spin_adj;
  GtkWidget *aa;
  GtkWidget *dnd;
  GtkListStore *codecs_list_store;
  GtkWidget *ldap_server;
  GtkWidget *ldap_port;
  GtkWidget *ldap;
  GtkWidget *gk;
  GtkWidget *gk_host;
  GtkWidget *gk_alias;
  GtkWidget *gk_id;
  GtkWidget *audio_player;
  GtkWidget *audio_recorder;
  GtkWidget *audio_player_mixer;
  GtkWidget *audio_recorder_mixer;
  GtkWidget *video_device; 
  GtkWidget *video_channel;
  GtkWidget *lid_device;
  GtkWidget *lid_aec;
  GtkWidget *lid_country;
  GtkWidget *lid;
  GtkWidget *show_docklet;
  GtkWidget *directory_update_button;
  GtkWidget *gatekeeper_update_button;
  GtkWidget *video_device_apply_button;
  GtkWidget *g711_frames;
  GtkWidget *gsm_frames;
  GtkWidget *forward_host;
  GtkWidget *always_forward;
  GtkWidget *busy_forward;
  GtkWidget *no_answer_forward;
};


struct _GmCommandLineOptions
{
  int debug_level;
  gchar *url;
  int daemon;
};


#endif 
/* _COMMON_H */
