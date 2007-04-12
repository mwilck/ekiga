
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras
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
 *                         gmtray-gtk.c  -  description
 *                         ------------------------
 *   begin                : Sat Feb 15 2007
 *   copyright            : (C) 2007 by Luis Menina <liberforce@fr.st>
 *                          (C) 2007 by Julien Puydt <jpuydt@free.fr>
 *   description          : Cross-platform implementation of the tray
 */

#define __GMTRAY_IMPLEMENTATION__

#include "../../../config.h"

#include "gmtray-internal.h"

struct _GmTraySpecific
{
  GtkStatusIcon *status_icon; /* the status icon of the system tray */
};



static void
left_click_cb (GtkStatusIcon *si,
		gpointer user_data)
{
  GmTray *tray = user_data;

  g_return_if_fail (tray != NULL);

  if (tray->left_clicked_callback)
    tray->left_clicked_callback (tray->left_clicked_callback_data);
}


/* FIXME: the callback for middle click is missing, but as of GTK 2.10, it's 
 * not supported by GtkStatusIcon. */


static void
right_click_cb (GtkStatusIcon *si,
		guint button,
		guint activate_time,
		gpointer user_data)
{
  GmTray *tray = user_data;
  GtkMenu *menu;
  
  g_return_if_fail (tray != NULL);

  if (tray->menu_callback == NULL)
    return;

  menu = tray->menu_callback (tray->menu_callback_data);

  gtk_widget_show_all (GTK_WIDGET (menu));

  gtk_menu_popup (menu, 
                  NULL, 
                  NULL, 
                  gtk_status_icon_position_menu, 
                  tray->specific->status_icon,
                  button, 
                  activate_time);
}


/* public api implementation */


GmTray *
gmtray_new (const gchar *image)
{
  GmTray *tray;
  GtkStatusIcon *si;

  tray = gmtray_new_common (image);
  tray->specific = g_new0 (GmTraySpecific, 1);
  
  si = tray->specific->status_icon = gtk_status_icon_new_from_stock (image);
  gtk_status_icon_set_visible (si, TRUE);

  g_signal_connect (G_OBJECT (si), "activate", 
		  G_CALLBACK (left_click_cb), tray);
  
  /* FIXME: add here the missing middle click signal */

  g_signal_connect (G_OBJECT (si), "popup-menu", 
		  G_CALLBACK (right_click_cb), tray);

  return tray;
}


void
gmtray_delete (GmTray *tray)
{
  g_return_if_fail (tray != NULL);

  g_object_unref (tray->specific->status_icon);
  g_free (tray->specific);
  gmtray_delete_common (tray);
}


gboolean
gmtray_is_embedded (GmTray *tray)
{
  g_return_val_if_fail (tray != NULL, FALSE);

  return gtk_status_icon_is_embedded (tray->specific->status_icon);
}


void
gmtray_show_image (GmTray *tray,
		   const gchar *image)
{
  GtkStatusIcon *si;
  g_return_if_fail (tray != NULL);

  si = tray->specific->status_icon;

  gtk_status_icon_set_from_stock (si, image);
  gtk_status_icon_set_visible (si, TRUE);
}


void
gmtray_menu (GmTray *tray)
{
  GtkMenu *menu = NULL;

  g_return_if_fail (tray != NULL);

  right_click_cb (tray->specific->status_icon,
		0,
		gtk_get_current_event_time(),
		tray);
}
