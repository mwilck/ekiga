/***************************************************************************
                          common.h  -  description
                             -------------------
    begin                : Wed Mar 21 2001
    copyright            : (C) 2000-2001 by Damien Sandras
    email                : dsandras@acm.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _COMMON_H
#define _COMMON_H

#include <gtk/gtkwidget.h>
#include <gtk/gtkobject.h>
#include <gdk/gdk.h>
#include <gtk/gtkadjustment.h>
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

typedef struct _GM_window_widgets GM_window_widgets;
typedef struct _GM_pref_window_widgets GM_pref_window_widgets;
typedef struct _GM_ldap_window_widgets GM_ldap_window_widgets;
typedef struct _GM_rtp_data GM_rtp_data;

struct _GM_rtp_data
{
  int tr_audio_bytes;
  int tr_video_bytes;
  int re_audio_bytes;
  int re_video_bytes;
};


struct _GM_window_widgets
{
  int x, y; /* the position of the main window */
  // widgets
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
  GtkWidget *log_text;
  GtkWidget *main_notebook;
  GdkPixmap *pixmap;
  GtkWidget *drawing_area;
  GtkWidget *video_frame;
  GtkWidget *pref_window;
  GtkWidget *ldap_window;
  GtkWidget *chat_window;
  GtkWidget *chat_text;
  GtkWidget *preview_button;
  GtkWidget *connect_button;
  GtkWidget *video_chan_button;
  GtkWidget *audio_chan_button;
  GtkWidget *incoming_call_popup;

  int video_grabber_thread_count;
  int cleaner_thread_count;
  PStringArray video_devices;
  PStringArray audio_recorder_devices;
  PStringArray audio_player_devices;
 
  double zoom;
};


struct _GM_ldap_window_widgets
{
  GtkWidget *statusbar;
  GtkWidget *search_entry;
  GtkWidget *ils_server_combo;
  GtkWidget *notebook;
  GtkWidget *option_menu;
  GtkWidget *refresh_button;
  
  int thread_count;

  GList *ldap_servers_list;

  // pointer to GM_window_widgets ;-)
  GM_window_widgets * gw;
};


struct _GM_pref_window_widgets
{
  GtkWidget *show_splash;
  GtkWidget *show_chat_window;
  GtkWidget *show_statusbar;
  GtkWidget *show_notebook;
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
  GtkAdjustment *g711_frames_spin_adj;
  GtkAdjustment *gsm_frames_spin_adj;
  GtkWidget *g711_sd;
  GtkWidget *gsm_sd;
  GtkAdjustment *jitter_buffer_spin_adj;
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
  GtkWidget *clist_avail;
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
  GtkAdjustment *video_channel_spin_adj;
  GtkWidget *video_channel;
  GtkWidget *show_docklet;
  GtkWidget *directory_update_button;
  GtkWidget *gatekeeper_update_button;
  GtkWidget *video_device_apply_button;
  GtkWidget *g711_frames;
  GtkWidget *gsm_frames;

  /* Miscellaneous */
  // contains the row selected by the user
  int row_avail;
  // pointer to GM_window_widgets ;-)
  GM_window_widgets * gw;
  // user has changed ldap related settings
  int ldap_changed;
  // user has changed audio mixer related settings
  int audio_mixer_changed;
  // user had toggled the video transmission button
  int vid_tr_changed;
  // user has changed a gatekeeper relative option
  int gk_changed;
  // user has changed the capabilities order, or added/deleted capabilities
  int capabilities_changed;

  int audio_codecs_changed;
};
/******************************************************************************/


#endif
