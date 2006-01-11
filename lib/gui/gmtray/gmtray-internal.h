
/*  gmtray-internal.h
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
 *                         gmtray-internal.h  -  description
 *                         ------------------------
 *   begin                : Sat Jan 7 2002
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Internal interface of the tray
 *                          (common to all platforms)
 */

#include "gmtray.h"

#ifndef __GMTRAY_INTERNAL_H__
#define __GMTRAY_INTERNAL_H__

#ifndef __GMTRAY_IMPLEMENTATION__
#error "Shouldn't be used outside of the implementation of libgmtray !"
#endif

G_BEGIN_DECLS

/* declare -- but don't define -- the structure that will hold all
 * implementation-specific details */
typedef struct _GmTraySpecific GmTraySpecific;

/* this structure makes available the os-independent part of the data
 */
struct _GmTray
{
  gchar *base_image; /* the stock-id of the image supposedly shown */

  gchar *blink_image; /* the stock-id of the image shown half of the time
		       * when blinking
		       */
  gboolean blink_shown; /* do we show the blink image or the base image ? */

  guint blink_id; /* the id of the timeout function -- kept to be able to
		   * disable it whenever we want */

  void (*left_clicked_callback) (gpointer); /* the callback the user said to
					     * call when the tray is left
					     * clicked (the fact that we're not
					     * a real GObject makes so that
					     * there can be only one, but that
					     * terrible restriction will go
					     * away when the gtk+ team will
					     * have released a version with
					     * GtkStatusIcon)
					     */

  gpointer left_clicked_callback_data; /* the pointer we'll give to the left
					* clicked callback when triggered
					*/

  void (*middle_clicked_callback) (gpointer); /* the callback the user said to
					       * call when the tray is middle
					       * clicked
					       */

  gpointer middle_clicked_callback_data; /* the pointer we'll give to the
					  * middle clicked callback when
					  * triggered
					  */

  GtkMenu *(*menu_callback) (gpointer); /* the callback which tells us which
					 * menu to show when the tray is
					 * right-clicked (should hopefully
					 * allow to make said menu more
					 * context-sensitive)
					 */

  gpointer menu_callback_data;          /* the pointer we'll give to the menu
					 * callback when triggered
					 */

  GmTraySpecific *specific; /* to let each implementation keep what it needs */
};


/* DESCRIPTION : /
 * BEHAVIOR    : creates a new common tray -- used by the implementations
 * PRE         : image should be a valid stock id
 * NOTICE      : implemented in gmtray-common.c
 */
GmTray *gmtray_new_common (const gchar *image);


/* DESCRIPTION : /
 * BEHAVIOR    : creates a new common tray -- used by the implementations
 * PRE         : image should be a valid stock id
 * NOTICE      : implemented in gmtray-common.c
 */
void gmtray_delete_common (GmTray *tray);


/* DESCRIPTION : /
 * BEHAVIOR    : Sets the tray to the given image
 *               (changes what is shown only, contrary to gmtray_set_image)
 * PRE         : tray and image shouldn't be NULL
 * NOTICE      : this is os-specific
 */
void gmtray_show_image (GmTray *tray,
			const gchar *image);


/* DESCRIPTION : /
 * BEHAVIOR    : Prompts the tray to show its associated menu
 * PRE         : tray shouldn't be NULL
 * NOTICE      : this is os-specific
 */
void gmtray_menu (GmTray *tray);

G_END_DECLS

#endif /* __GMTRAY_INTERNAL_H__ */
