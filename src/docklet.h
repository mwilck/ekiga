
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

/*
 *                         docklet.h  -  description
 *                         -------------------------
 *   begin                : Wed Oct 3 2001
 *   copyright            : (C) 2000-2001 by Miguel Rodriguez
 *   description          : This file contains all functions needed for
 *                          Gnome Panel docklet.
 *   email                : migrax@terra.es (all the new code)
 *                          dsandras@seconix.com (old applet code).
 *
 */

#ifndef _DOCKLET_H_
#define _DOCKLET_H_

#include <glib.h>
#include <gnome.h>

#include "config.h"


/* DESCRIPTION  :  This callback is called by a timeout function
 * BEHAVIOR     :  If current picture in the docklet is globe,
 *                 then displays globe2, else displays globe
 * PRE          :  /
 */
gint gnomemeeting_docklet_flash (GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Init the docklet and menus and callbacks (for this docklet)
 * PRE          :  /
 */
GtkWidget *gnomemeeting_init_docklet ();


/* DESCRIPTION  :  /
 * BEHAVIOR     :  If int = 1, displays globe2 and plays a sound, else displays
 *                 globe
 * PRE          :  /
 */
void gnomemeeting_docklet_set_content (GtkWidget *, int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Show the docklet.
 * PRE          :  /
 */
void gnomemeeting_docklet_show (GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Hide the docklet window
 * PRE          : /
 */
void gnomemeeting_docklet_hide (GtkWidget *);

#endif
