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


typedef struct _GM_window_widgets GM_window_widgets;
typedef struct _GM_pref_window_widgets GM_pref_window_widgets;
typedef struct _GM_ldap_window_widgets GM_ldap_window_widgets;
typedef struct _options options;


/* This structure contains the fields for all the parameters of gnomemeeting */
struct _options
{
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
  char *listen_port;  // Listen Port
  char *surname;
  char *mail;
  char *location;
  char *comment;
  int bps; // Max bps
  int dnd;
  char *audio_codecs [5] [2]; // [0] = name; [1] = value
  int sd;
  int vol_play;
  int vol_rec;
  int ldap;
  char *ldap_server;
  char *ldap_port;
  int notfirst;
  int show_splash;
  int show_statusbar;
  int show_notebook;
  int incoming_call_sound;
  int applet;
  char *audio_device;
  char *audio_mixer;
  char *video_device;
  int video_channel; 
};


struct _GM_window_widgets
{
  // widgets
  GtkObject *adj_play, *adj_rec;
  GtkObject *adj_whiteness;
  GtkObject *adj_brightness;
  GtkObject *adj_colour;
  GtkObject *adj_contrast;
  GtkWidget *video_settings_frame;
  GtkWidget *statusbar;
  GtkWidget *applet;
  GtkWidget *splash_win;
  GtkWidget *combo;
  GtkWidget *log_text;
  GtkWidget *user_list;
  GtkWidget *main_notebook;
  GdkPixmap *pixmap;
  GtkWidget *drawing_area;
  GtkWidget *video_frame;
  GtkWidget *pref_window;
  GtkWidget *ldap_window;
  GtkWidget *preview_button;
  GtkWidget *video_chan_button;
  GtkWidget *audio_chan_button;
};


struct _GM_ldap_window_widgets
{
  GtkWidget *ldap_users_clist;
  GtkWidget *statusbar;
  GtkWidget *search_entry;
  GtkWidget *option_menu;
  
  // the last selected row and column
  int last_selected_row;
  int last_selected_col;
  
  // the sorted column number
  int sorted_column;

  // ascending or descending
  int sorted_order;

  // Searhc Thread ID
  pthread_t fetch_results_thread;

  int thread_count;

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
  // Toggle to show the notebook at startup time
  GtkWidget *show_notebook;
  // Toggle to play or not a sound when receiving an incoming call
  GtkWidget *incoming_call_sound;

  /* Video Codecs Configuration */
  // Option menus for size and format of the transmitted video
  GtkWidget *opt1, *opt2;
  // Transmitted Video Quality
  GtkAdjustment *tr_vq_spin_adj;
  // Number of Updated Blocks / Frame
  GtkAdjustment *tr_ub_spin_adj;
  // Received Video Quality
  GtkAdjustment *re_vq_spin_adj;
  // Transmitted FPS
  GtkAdjustment *tr_fps_spin_adj;
  // Enable / disable video transmission
  GtkWidget *vid_tr;
  
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

 
  /* Device Settings */
  // The audio device
  GtkWidget *audio_device;
  // The audio mixer
  GtkWidget *audio_mixer;
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
};
/******************************************************************************/


#endif
