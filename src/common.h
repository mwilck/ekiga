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

#include <gnome.h>
#include <ptlib.h>

#define GM_CIF_WIDTH   352
#define GM_CIF_HEIGHT  288
#define GM_QCIF_WIDTH  176
#define GM_QCIF_HEIGHT 144
#define GM_SIF_WIDTH   320
#define GM_SIF_HEIGHT  240
#define GM_QSIF_WIDTH  160
#define GM_QSIF_HEIGHT 120
#define GM_FRAME_SIZE  4

typedef struct _GM_window_widgets GM_window_widgets;
typedef struct _GM_pref_window_widgets GM_pref_window_widgets;
typedef struct _GM_ldap_window_widgets GM_ldap_window_widgets;
typedef struct _options options;


/* This structure contains the fields for all the parameters of gnomemeeting */
struct _options
{
  int video_preview;
  int video_size;   // 1 = small ; 2 = large
  int video_format; // 1 = default ; 2 = pal ; 3 = ntsc
  int tr_vq;  // transmitted video quality
  int tr_ub;  // transmitted updated background blocks per frame
  int tr_fps; // transmitted frame / s
  int re_vq;  // received video quality
  int vid_tr;  // Enable/disable video transmission
  char * video_dev; // Video device (e.g. : /dev/video)
  char * firstname; // The user name
  int aa;  // Auto Answer
  int ht;  // H245 Tunneling
  int fs;  // Fast Start
  int popup; // Display popup or no when incoming call
  char *listen_port;  // Listen Port
  char *surname;
  char *mail;
  char *location;
  char *comment;
  int bps; // Max bps
  int dnd;
  char *audio_codecs [5] [2]; // [0] = name; [1] = value
  int sd; // TO BE REMOVED
  int g711_sd;
  int gsm_sd;
  int g711_frames;
  int gsm_frames;
  int jitter_buffer;
  int vol_play;
  int vol_rec;
  int ldap;
  char *ldap_server;
  char *ldap_servers_list;
  char *ldap_port;
  int gk;
  char *gk_host;
  char *gk_id;
  int notfirst;
  int show_splash;
  int show_statusbar;
  int show_notebook;
  int show_quickbar;
  int incoming_call_sound;
  char *audio_player;
  char *audio_recorder;
  char *audio_player_mixer;
  char *audio_recorder_mixer;
  char *video_device;
  int video_channel; 
  int video_bandwidth;
  int vb;
};


struct _GM_window_widgets
{
  // widgets
  GtkObject *adj_play, *adj_rec;
  GtkObject *adj_whiteness;
  GtkObject *adj_brightness;
  GtkObject *adj_colour;
  GtkObject *adj_contrast;
  GtkObject *docklet;
  GtkWidget *video_settings_frame;
  GtkWidget *statusbar;
  GtkWidget *splash_win;
  GtkWidget *combo;
  GtkWidget *log_text;
  GtkWidget *user_list;
  GtkWidget *main_notebook;
  GdkPixmap *pixmap;
  GtkWidget *drawing_area;
  GtkWidget *video_frame;
  GtkWidget *quickbar_frame;
  GtkWidget *pref_window;
  GtkWidget *ldap_window;
  GtkWidget *preview_button;
  GtkWidget *silence_detection_button;
  GtkWidget *video_chan_button;
  GtkWidget *audio_chan_button;

  int video_grabber_thread_count;
  PStringArray video_devices;
  PStringArray audio_recorder_devices;
  PStringArray audio_player_devices;
};


struct _GM_ldap_window_widgets
{
  GtkWidget *ldap_users_clist [25];
  GtkWidget *statusbar;
  GtkWidget *search_entry;
  GtkWidget *ils_server_combo;
  GtkWidget *notebook;
  GtkWidget *option_menu;
  GtkWidget *refresh_button;
  
  // the last selected row and column
  int last_selected_row [25];
  int last_selected_col [25];

  int current_page;

  // the sorted column number
  int sorted_column [25];

  // ascending or descending
  int sorted_order [25];

  int thread_count;

  GList *ldap_servers_list;

  // pointer to GM_window_widgets ;-)
  GM_window_widgets * gw;
};


struct _GM_pref_window_widgets
{
  /* GnomeMeeting Settings */
  // Toggle to show splash screen at startup time
  GtkWidget *show_splash;
  // Toggle to show the statusbar at startup time
  GtkWidget *show_statusbar;
  GtkWidget *show_quickbar;
  // Toggle to show the notebook at startup time
  GtkWidget *show_notebook;
  // Toggle to popup a window when receiving an incoming call
  GtkWidget *popup;
  // Toggle to enable video preview or not
  GtkWidget *video_preview;
  // Toggle to play or not a sound when receiving an incoming call
  GtkWidget *incoming_call_sound;

  /* Codecs Settings */
  // Option menus for size and format of the transmitted video
  GtkWidget *opt1, *opt2;
  // Transmitted Video Quality
  GtkAdjustment *tr_vq_spin_adj;
  GtkWidget *tr_vq_label;
  GtkWidget *tr_ub_label;
  GtkWidget *tr_vq;
  GtkWidget *tr_ub;
  GtkWidget *video_bandwidth;
  // Number of Updated Blocks / Frame
  GtkAdjustment *tr_ub_spin_adj;
  // Received Video Quality
  GtkAdjustment *re_vq_spin_adj;
  // Transmitted FPS
  GtkAdjustment *tr_fps_spin_adj;
  // Enable / disable video transmission
  GtkWidget *vid_tr;
  GtkAdjustment *g711_frames_spin_adj;
  GtkAdjustment *gsm_frames_spin_adj;
  GtkWidget *g711_sd;
  GtkWidget *gsm_sd;
  GtkAdjustment *jitter_buffer_spin_adj;
  GtkAdjustment *video_bandwidth_spin_adj;
  GtkWidget *video_bandwidth_label;
  GtkWidget *vb;

  /* General Settings */
  // User name, and listener port
  GtkWidget * firstname, *entry_port;
  // Surname
  GtkWidget *surname;
  // Mail
  GtkWidget *mail;
  // Location
  GtkWidget *location;
  // Comment
  GtkWidget *comment;
  // The H245 Tunnelling button
  GtkWidget *ht;
  // The Fast Start button
  GtkWidget *fs;
  // Max bps
  GtkAdjustment *bps_spin_adj;


  /* Advanced Settings */
  // The auto answer button
  GtkWidget *aa;
  // The Do not Disturb button
  GtkWidget *dnd;

  
  /* The audio codecs Settings */
  // The available codecs clist
  GtkWidget *clist_avail;
  // The silence detection widget
  GtkWidget *sd;

  
  /* LDAP Settings */
  // LDAP server
  GtkWidget *ldap_server;
  // LDAP server port
  GtkWidget *ldap_port;
  // Enable / Disable LDAP
  GtkWidget *ldap;

  /* Gatekeeper Settings */
  // Enable / Disable GK support
  GtkWidget *gk;
  // Gatekeeper host
  GtkWidget *gk_host;
  // Gatekeeper ID 
  GtkWidget *gk_id;

  /* Device Settings */
  // The audio player device
  GtkWidget *audio_player;
  // The audio recorder device
  GtkWidget *audio_recorder;
  // The audio mixers
  GtkWidget *audio_player_mixer;
  GtkWidget *audio_recorder_mixer;
  // The video device
  GtkWidget *video_device; 
  // The video channel to use
  GtkAdjustment *video_channel_spin_adj;

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
};
/******************************************************************************/


#endif
