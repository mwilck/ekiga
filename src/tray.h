
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
 *                         tray.h  -  description
 *                         ----------------------
 *   begin                : Wed Oct 3 2001
 *   copyright            : (C) 2000-2002 by Miguel Rodríguez
 *   description          : This file contains all functions needed for
 *                          system tray icon.
 *   Additional code      : migrax@terra.es (all the new code)
 *                          dsandras@seconix.com (old applet code).
 *
 */


#ifndef _TRAY_H_
#define _TRAY_H_

#include "config.h"
#include <glib-object.h>

G_BEGIN_DECLS

/* DESCRIPTION  :  This callback is called by a timeout function
 * BEHAVIOR     :  If current picture in the tray is globe,
 *                 then displays globe2, else displays globe
 * PRE          :  /
 */
gint gnomemeeting_tray_flash (GObject *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Init the tray and menus and callbacks (for this tray icon)
 * PRE          :  The GtkAccelGroup.
 */
GObject *gnomemeeting_init_tray (GtkAccelGroup *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  If int = 0, displays the available icon.
 *                 If int = 1, displays the ringing icon and plays a sound.
 *                 If int = 2, displays the busy icon.
 * PRE          :  /
 */
void gnomemeeting_tray_set_content (GObject *, int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Show the tray icon.
 * PRE          :  /
 */
void gnomemeeting_tray_show (GObject *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Hide the tray window
 * PRE          : /
 */
void gnomemeeting_tray_hide (GObject *);


/* DESCRIPTION  : Returns true if the tray shows a rining phone
 * BEHAVIOR     : 
 * PRE          : /
 */
gboolean gnomemeeting_tray_is_ringing (GObject *tray);

/* DESCRIPTION  : Returns true if the tray is visible
 * BEHAVIOR     : 
 * PRE          : /
 */
gboolean gnomemeeting_tray_is_visible (GObject *tray);

G_END_DECLS

#endif
