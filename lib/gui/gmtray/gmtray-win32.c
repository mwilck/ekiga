
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
 *                         gmtray-win32.c  -  description
 *                         ------------------------
 *   begin                : Sat Jan 7 2002
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Win32 implementation of the tray
 */

#define __GMTRAY_IMPLEMENTATION__

#include "../../../config.h"

#include <windows.h>

#include "gmtray-internal.h"

#include "pixbuf_to_hicon.h"

struct _GmTraySpecific
{
  NOTIFYICONDATA nid; /* this is the windows tray */
  GtkWidget *stupid_platform; /* we need this widget we never show to render
			       * the icons in it... */
};


/* helper functions */

/* this function will hide the popup menu when triggered by a timer
 * (this is because on win32, GTK+ doesn't hide a menu when the user goes
 * away -- thanks to the gaim project for that nice workaround !)
 */
static gboolean
popup_menu_hider (gpointer data)
{
  g_return_val_if_fail (GTK_IS_MENU (data), FALSE);

  gtk_menu_popdown (GTK_MENU (data));

  return FALSE;
}


/* this function checks if we leave/enter the popup menu, and decides to hide
 * it after some time if needed (this is because on win32, GTK+ doesn't hide a
 * menu when the user goes away -- thanks to the gaim project for that nice
 * workaround !)
 */
static gboolean
popup_menu_leave_enter_callback (GtkWidget *menu,
				 GdkEventCrossing *event,
				 gpointer data)
{
  static guint timer = 0;

  if (event->type == GDK_LEAVE_NOTIFY
      && event->detail == GDK_NOTIFY_ANCESTOR) {

    /* user is going away ! */
    if (timer == 0)
      timer = g_timeout_add (500, popup_menu_hider, (gpointer)menu);

  } else if (event->type == GDK_ENTER_NOTIFY
	     && event->detail == GDK_NOTIFY_ANCESTOR) {

    /* wait ! Finally the user comes back ! */
    if (timer != 0) {

      (void)g_source_remove (timer);
      timer = 0;
    }
  }
}

/* this function receives events on the tray, and decides whether to call
 * the click callback, show the menu or ignore
 */
static LRESULT CALLBACK
message_handler (HWND hwnd,
		 UINT msg,
		 WPARAM wparam,
		 LPARAM lparam)
{
  GmTray *tray = NULL;

  tray = (GmTray *)wparam;

  if (msg == WM_USER) {

    if (lparam == WM_LBUTTONDOWN) {

      if (tray->left_clicked_callback)
	tray->left_clicked_callback (tray->left_clicked_callback_data);
    } else if (lparam == WM_MBUTTONDOWN) {

      if (tray->middle_clicked_callback)
	tray->middle_clicked_callback (tray->middle_clicked_callback_data);
    } else if (lparam == WM_RBUTTONDOWN) {

      gmtray_menu (tray);
    }
    return 0;
  }

  return DefWindowProc (hwnd, msg, wparam, lparam);
}


/* creates a sort of win32 event box, associated with
 * our message callback function
 */
static HWND
create_message_window ()
{
  WNDCLASS wclass;
  ATOM klass;
  HINSTANCE hmodule = GetModuleHandle (NULL);

  memset (&wclass, 0, sizeof (WNDCLASS));
  wclass.lpszClassName = PACKAGE_NAME "-tray";
  wclass.lpfnWndProc = message_handler;

  klass = RegisterClass (&wclass);

  return CreateWindow (MAKEINTRESOURCE (klass), NULL, WS_POPUP,
		       0, 0, 1, 1, NULL, NULL,
		       hmodule, NULL);

}


/* public api implementation */


GmTray *
gmtray_new (const gchar *image)
{
  GmTray    *result = NULL;
  GdkPixbuf *pixbuf = NULL;
  GtkWidget *stupid = NULL;

  stupid = gtk_label_new ("");
  pixbuf = gtk_widget_render_icon (stupid, image,
				   GTK_ICON_SIZE_MENU, NULL);

  result = gmtray_new_common (image);

  result->specific = g_new (GmTraySpecific, 1);

  result->specific->stupid_platform = stupid;

  memset (&result->specific->nid, 0, sizeof (result->specific->nid));
  result->specific->nid.hWnd = create_message_window ();
  result->specific->nid.uID = GPOINTER_TO_UINT (result);
  result->specific->nid.uCallbackMessage = WM_USER;
  result->specific->nid.uFlags = NIF_ICON | NIF_MESSAGE;
  result->specific->nid.hIcon = _gdk_win32_pixbuf_to_hicon (pixbuf);

  Shell_NotifyIcon (NIM_ADD, &result->specific->nid);

  g_object_unref (pixbuf);

  return result;
}


void
gmtray_delete (GmTray *tray)
{
  g_return_if_fail (tray != NULL);

  gtk_widget_destroy (tray->specific->stupid_platform);
  Shell_NotifyIcon (NIM_DELETE, &tray->specific->nid);
  g_free (tray->specific);
  gmtray_delete_common (tray);
}


void
gmtray_show_image (GmTray *tray,
		   const gchar *image)
{
  GdkPixbuf *pixbuf = NULL;

  g_return_if_fail (tray != NULL);

  pixbuf = gtk_widget_render_icon (tray->specific->stupid_platform,
				   image, GTK_ICON_SIZE_MENU, NULL);

  tray->specific->nid.hIcon = _gdk_win32_pixbuf_to_hicon (pixbuf);
  tray->specific->nid.uFlags |= NIF_ICON;

  Shell_NotifyIcon (NIM_MODIFY, &tray->specific->nid);

  g_object_unref (pixbuf);
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

  g_signal_connect (menu, "leave-notify-event",
		    G_CALLBACK(popup_menu_leave_enter_callback), NULL);
  g_signal_connect (menu, "enter-notify-event",
		    G_CALLBACK(popup_menu_leave_enter_callback), NULL);

  gtk_menu_popup (menu, NULL, NULL, NULL, NULL,
		  0, gtk_get_current_event_time ());
}
