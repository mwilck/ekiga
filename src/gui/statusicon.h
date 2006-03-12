
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 *                         statusicon.h  -  description
 *                         --------------------------
 *   begin                : Thu Jan 12 2006
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *                          (C) 2002 by Miguel Rodriguez
 *                          (C) 2006 by Julien Puydt
 *   description          : High level tray api interface
 */


#ifndef _STATUSICON_H_
#define _STATUSICON_H_

#include "common.h"
#include "manager.h"

G_BEGIN_DECLS

/* DESCRIPTION  : /
 * BEHAVIOR     : Returns a new statusicon, with the default icon and menu
 * PRE          : /
 */
GtkWidget *gm_statusicon_new (void);


/* DESCRIPTION  : /
 * BEHAVIOR     : Updates the statusicon, according to the current calling
 *                state and mode, and whether we forward on busy or not
 *                (updates both the icon and the menu)
 * PRE          : /
 */
void gm_statusicon_update_full (GtkWidget *widget,
				GMManager::CallingState state,
				IncomingCallMode mode,
				gboolean forward_on_busy);



/* DESCRIPTION  : /
 * BEHAVIOR     : Updates the statusicon's menu, according to the current 
 *                calling state
 * PRE          : /
 */
void gm_statusicon_update_menu (GtkWidget *widget,
				GMManager::CallingState state);



/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the status icon busy state. When the window is busy,
 *                 you can not exit. Similar to the main window function
 *                 of the same name.
 * PRE          :  The status icon GMObject.
 * 		   The first parameter is TRUE if we are busy.
 */
void gm_statusicon_set_busy (GtkWidget *,
			     BOOL);


/* DESCRIPTION  : /
 * BEHAVIOR     : If has_message is TRUE, the statusicon will blink with the
 *                message icon, and a click on it will bring up the chat window
 *                If has_message is FALSE, the statusicon will resume normal
 *                behaviour
 * PRE          : /
 */
void gm_statusicon_signal_message (GtkWidget *widget,
				   gboolean has_message);


/* DESCRIPTION  : /
 * BEHAVIOR     : Makes the statusicon blink with the ringing icon
 *                (interval gives the period the blink)
 * PRE          : /
 */
void gm_statusicon_ring (GtkWidget *widget,
			 guint interval);


/* DESCRIPTION  : /
 * BEHAVIOR     : Makes the statusicon resume normal behaviour
 * PRE          : /
 */
void gm_statusicon_stop_ringing (GtkWidget *widget);


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns TRUE if the statusicon is really embedded in an area
 * PRE          : /
 */
gboolean gm_statusicon_is_embedded (GtkWidget *widget);

G_END_DECLS

#endif
