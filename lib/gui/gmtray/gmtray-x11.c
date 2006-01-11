
/*  gmtray-x11.c
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
 *                         gmtray-x11.c  -  description
 *                         ------------------------
 *   begin                : Sat Jan 7 2002
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : X11 implementation of the tray
 */

#define __GMTRAY_IMPLEMENTATION__

#include "../../../config.h"

#include "gmtray-internal.h"
#include "eggtrayicon.h"

struct _GmTraySpecific
{
  GtkWidget *eggtray; /* the eggtray */
  GtkImage *image; /* eggtray's image (ie : what we really show) */
};


/* helper functions */


/* this function is the one which receives the clicks on the tray, and will
 * decide whether to call the click callback, show the menu, or ignore
 */
static gint
clicked_cb (GtkWidget *unused,
	    GdkEventButton *event,
	    gpointer data)
{
  GmTray *tray = data;

  g_return_val_if_fail (tray != NULL, FALSE);

  if (event->type == GDK_BUTTON_PRESS) {

    if (event->button == 1) {

      if (tray->left_clicked_callback)
	tray->left_clicked_callback (tray->left_clicked_callback_data);
      return TRUE;
    } else if (event->button == 2) {

      if (tray->middle_clicked_callback)
	tray->middle_clicked_callback (tray->middle_clicked_callback_data);
      return TRUE;
    } else if (event->button == 3) {

      gmtray_menu (tray);
      return TRUE;
    }
  }

  return FALSE;
}


/* public api implementation */


GmTray *
gmtray_new (const gchar *image)
{
  GmTray    *result    = NULL;
  GtkWidget *event_box = NULL;
  GtkWidget *eggtray   = NULL;
  GtkWidget *my_image  = NULL;

  eggtray = GTK_WIDGET (egg_tray_icon_new (PACKAGE_NAME));
  event_box = gtk_event_box_new ();
  my_image = gtk_image_new_from_stock (image, GTK_ICON_SIZE_SMALL_TOOLBAR);

  gtk_container_add (GTK_CONTAINER (event_box), my_image);
  gtk_container_add (GTK_CONTAINER (eggtray), event_box);

  gtk_widget_show_all (eggtray);

  result = gmtray_new_common (image);
  result->specific = g_new0 (GmTraySpecific, 1);
  result->specific->eggtray = eggtray;
  result->specific->image = GTK_IMAGE (my_image);

  g_signal_connect (G_OBJECT (event_box), "button_press_event",
		    G_CALLBACK (clicked_cb), result);

  return result;
}


void
gmtray_delete (GmTray *tray)
{
  g_return_if_fail (tray != NULL);

  g_free (tray->specific);
  gmtray_delete_common (tray);
}


void
gmtray_show_image (GmTray *tray,
		   const gchar *image)
{
  g_return_if_fail (tray != NULL);

  gtk_image_set_from_stock (tray->specific->image, image,
			    GTK_ICON_SIZE_SMALL_TOOLBAR);
}


void
gmtray_menu (GmTray *tray)
{
  GtkMenu *menu = NULL;

  g_return_if_fail (tray != NULL);

  if (tray->menu_callback == NULL)
    return;

  menu = tray->menu_callback (tray->menu_callback_data);

  gtk_widget_show_all (GTK_WIDGET (menu));

  gtk_menu_popup (menu, NULL, NULL, NULL, NULL,
		  0, gtk_get_current_event_time ());
}
