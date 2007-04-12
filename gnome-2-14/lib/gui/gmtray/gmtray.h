
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
 * Authors: Julien Puydt <jpuydt@free.fr>
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
 * BEHAVIOR    : Define which function to call when the tray is left clicked
 * PRE         : tray shouldn't be NULL
 */
void gmtray_set_left_clicked_callback (GmTray *tray,
				       void (*callback)(gpointer),
				       gpointer);


/* DESCRIPTION : /
 * BEHAVIOR    : Define which function to call when the tray is middle clicked
 * PRE         : tray shouldn't be NULL
 */
void gmtray_set_middle_clicked_callback (GmTray *tray,
					 void (*callback)(gpointer),
					 gpointer);


/* DESCRIPTION : /
 * BEHAVIOR    : Define which function to call to obtain a menu for the tray
 *               (notice : you'll be responsible with disposing from it)
 * PRE         : tray shouldn't be NULL
 */
void gmtray_set_menu_callback (GmTray *tray,
			       GtkMenu *(*callback)(gpointer),
			       gpointer);

G_END_DECLS

#endif /* __GMTRAY_H__ */
