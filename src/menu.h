
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
 */

/*
 *                         menu.h  -  description
 *                         ----------------------
 *   begin                : Tue Dec 23 2000
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : Functions to create the menus.
 *   email                : dsandras@seconix.com
 *
 */

#ifndef _MENU_H_
#define _MENU_H_

#include <gtk/gtkwidget.h>


struct _MenuEntry {
    
  char *name;
  char *tooltip;
  char *stock_id;
  char accel;
  short type;
  GtkSignalFunc func;
  gpointer data;
  GtkWidget *widget;
};
typedef _MenuEntry MenuEntry;

enum {

  MENU_ENTRY,
  MENU_ENTRY_TOGGLE,
  MENU_ENTRY_RADIO,
  MENU_SEP,
  MENU_TEAROFF,
  MENU_NEW,
  MENU_SUBMENU_NEW,
  MENU_END
};

enum {

  LOCAL_VIDEO, 
  REMOTE_VIDEO, 
  BOTH_INCRUSTED, 
  BOTH_LOCAL, 
  BOTH
};


/* The functions */


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Create the menu and return a pointer to the newly created
 *                 menu.
 * PRE          :  The accel group.
 */
GtkWidget *gnomemeeting_init_menu (GtkAccelGroup *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Changes sensitivity in the zoom section of the view menu
 *                 and of the popup menu. 
 * PRE          :  /
 */
void gnomemeeting_zoom_submenu_set_sensitive (gboolean);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Changes sensitivity of the full screen option of 
 *                 the view menu and of the popup menu. It can only be
 *                 enabled if SDL support has been compiled in.
 * PRE          :  /
 */
void gnomemeeting_fullscreen_option_set_sensitive (gboolean);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Changes sensitivity in the video section of the view menu
 *                 and of the popup menu.
 * PRE          :  Sensitive or not / the part of the menu to change the 
 *                 sensitivity / true if we change the sensitivity of the
 *                 BOTH, BOTH_LOCAL, and BOTH_INCRUSTED sections too, false
 *                 if not.
 */
void gnomemeeting_video_submenu_set_sensitive (gboolean, int, gboolean = true);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Select radio item i in the video section of the view menu
 *                 and of the popup menu.
 * PRE          :  0 <= i < 2
 */
void gnomemeeting_video_submenu_select (int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a video menu which will popup, and attach it
 *                 to the given widget.
 * PRE          :  The widget to attach the menu to, and the accelgroup.
 */
void gnomemeeting_popup_menu_init (GtkWidget *, GtkAccelGroup *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a video menu which will popup, and attach it
 *                 to the given widget.
 * PRE          :  The widget to attach the menu to, and the accelgroup.
 */
void gnomemeeting_popup_menu_tray_init (GtkWidget *, GtkAccelGroup *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds a menu given a first menu, a structure and an
 *                 AccelGroup.
 * PRE          :  Valid parameters.
 */
void gnomemeeting_build_menu (GtkWidget *, MenuEntry *, GtkAccelGroup *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Enable/disable sensitivity (bool) of connect/disconnect.
 * PRE          :  true/false, 0 (connect) <= int <= 1 (disconnect)
 */
void gnomemeeting_call_menu_connect_set_sensitive (int, bool);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Enable/disable sensitivity (bool) of pause audio/video.
 * PRE          :  true/false
 */
void gnomemeeting_call_menu_pause_set_sensitive (bool);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns a pointer to the main menu.
 * PRE          :  Valid pointer to gm.
 */
MenuEntry *gnomemeeting_get_menu (GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns a pointer to the video menu attached to the main 
 *                 video image.
 * PRE          :  Valid pointer to gm.
 */
MenuEntry *gnomemeeting_get_video_menu (GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns a pointer to the tray menu.
 * PRE          :  Valid pointer to gm.
 */
MenuEntry *gnomemeeting_get_tray_menu (GtkWidget *);
#endif
