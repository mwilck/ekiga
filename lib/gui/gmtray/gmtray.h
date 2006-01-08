
/*  gmtray.h
 *
 *  GnomeMeeting -- A Video-Conferencing application
 *  Copyright (C) 2000-2006 Damien Sandras
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 *  Authors: Julien Puydt <jpuydt@free.fr>
 */

/*
 *                         gmtray.h  -  description
 *                         ------------------------
 *   begin                : Sat Jan 7 2002
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : External interface to the tray
 */

#include <gtk/gtk.h>

#ifndef __GMTRAY_H__
#define __GMTRAY_H__

G_BEGIN_DECLS

/* our opaque data structure */
typedef struct _GmTray GmTray;

/* DESCRIPTION : /
 * BEHAVIOR    : Creates a tray with the given image
 * PRE         : image should be a valid stock id
 */
GmTray *gmtray_new (const gchar *image);


/* DESCRIPTION : /
 * BEHAVIOR    : Deletes the tray
 * PRE         : tray shouldn't be NULL
 */
void gmtray_delete (GmTray *tray);


/* DESCRIPTION : /
 * BEHAVIOR    : Sets the tray to the given image
 * PRE         : tray shouldn't be NULL and image should be a valid stock id
 */
void gmtray_set_image (GmTray *tray,
		       const gchar *image);


/* DESCRIPTION : /
 * BEHAVIOR    : Checks if the tray isn't already blinking
 * PRE         : tray shouldn't be NULL
 */
gboolean gmtray_is_blinking (GmTray *tray);


/* DESCRIPTION : /
 * BEHAVIOR    : Make the tray blink between its base image and the blink_image
 *               (changing which is shown every interval)
 * PRE         : tray and blink_image shouldn't be NULL, interval shouldn't be
 *               zero and the tray shouldn't already be blinking
 */
void gmtray_blink (GmTray *tray,
		   const gchar *blink_image,
		   guint interval);


/* DESCRIPTION : /
 * BEHAVIOR    : Make the tray stop blinking, if it already was (ok if not)
 * PRE         : tray shouldn't be NULL
 */
void gmtray_stop_blink (GmTray *tray);


/* DESCRIPTION : /
 * BEHAVIOR    : Define which function to call when the tray is clicked
 * PRE         : tray shouldn't be NULL
 */
void gmtray_set_clicked_callback (GmTray *tray,
				  void (*callback)(void));


/* DESCRIPTION : /
 * BEHAVIOR    : Define which function to call to obtain a menu for the tray
 *               (notice : you'll be responsible with disposing from it)
 * PRE         : tray shouldn't be NULL
 */
void gmtray_set_menu_callback (GmTray *tray,
			       GtkMenu *(*callback)(void));

G_END_DECLS

#endif /* __GMTRAY_H__ */
