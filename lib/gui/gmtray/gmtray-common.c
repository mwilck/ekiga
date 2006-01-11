
/*  gmtray-common.c
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
 *                         gmtray-common.c  -  description
 *                         ------------------------
 *   begin                : Sat Jan 7 2002
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Implementation of the tray common to all platforms
 */

#define __GMTRAY_IMPLEMENTATION__

#include "gmtray-internal.h"


/* helper function that makes the tray blink
 * it always returns TRUE, because its id is kept
 * within the tray, and can else be cancelled whenever
 * we want
 */
static gboolean
blink_timeout (gpointer data)
{
  GmTray *tray = data;

  g_return_val_if_fail (tray != NULL, FALSE);

  if (tray->blink_shown) {

    gmtray_show_image (tray, tray->base_image);
    tray->blink_shown = FALSE;
  } else {

    gmtray_show_image (tray, tray->blink_image);
    tray->blink_shown = TRUE;
  }

  return TRUE;
}


/* implementation of public functions */


GmTray *
gmtray_new_common (const gchar *image)
{
  GmTray *result = NULL;

  result = g_new (GmTray, 1);

  result->base_image = g_strdup (image);
  result->blink_image = NULL;
  result->blink_shown = FALSE;
  result->blink_id = -1;
  result->left_clicked_callback = NULL;
  result->left_clicked_callback_data = NULL;
  result->middle_clicked_callback = NULL;
  result->middle_clicked_callback_data = NULL;
  result->menu_callback = NULL;
  result->menu_callback_data = NULL;

  return result;
}


void
gmtray_delete_common (GmTray *tray)
{
  g_return_if_fail (tray != NULL);

  gmtray_stop_blink (tray);

  g_free (tray->base_image);
  tray->base_image = NULL;

  g_free (tray);
}


void
gmtray_set_image (GmTray *tray,
		  const gchar *image)
{
  gchar *old_image = NULL;

  g_return_if_fail (tray != NULL);

  old_image = tray->base_image;
  tray->base_image = g_strdup (image);
  g_free (old_image);

  gmtray_show_image (tray, tray->base_image);
}


gboolean
gmtray_is_blinking (GmTray *tray)
{
  g_return_val_if_fail (tray != NULL, FALSE);

  if (tray->blink_id == -1)
    return FALSE;
  else
    return TRUE;
}


void
gmtray_blink (GmTray *tray,
	      const gchar *blink_image,
	      guint interval)
{
  g_return_if_fail (tray != NULL);
  g_return_if_fail (tray->blink_image == NULL);
  g_return_if_fail (interval > 0);

  tray->blink_image = g_strdup (blink_image);
  tray->blink_id = g_timeout_add (interval, blink_timeout, (gpointer)tray);
}


void
gmtray_stop_blink (GmTray *tray)
{
  g_return_if_fail (tray != NULL);

  if (tray->blink_id != -1) {

    g_source_remove (tray->blink_id);
    tray->blink_id = -1;
    g_free (tray->blink_image);
    tray->blink_image = NULL;
  }

  gmtray_show_image (tray, tray->base_image);
  tray->blink_shown = FALSE;
}


void
gmtray_set_left_clicked_callback (GmTray *tray,
				  void (*callback)(gpointer),
				  gpointer data)
{
  g_return_if_fail (tray != NULL);

  tray->left_clicked_callback = callback;
  tray->left_clicked_callback_data = data;
}


void
gmtray_set_middle_clicked_callback (GmTray *tray,
				    void (*callback)(gpointer),
				    gpointer data)
{
  g_return_if_fail (tray != NULL);

  tray->middle_clicked_callback = callback;
  tray->middle_clicked_callback_data = data;
}


void
gmtray_set_menu_callback (GmTray *tray,
			  GtkMenu *(*callback)(gpointer),
			  gpointer data)
{
  g_return_if_fail (tray != NULL);

  tray->menu_callback = callback;
  tray->menu_callback_data = data;
}
