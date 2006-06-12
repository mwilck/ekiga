
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
 *                         statusicon.cpp  -  description
 *                         --------------------------
 *   begin                : Thu Jan 12 2006
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *                          (C) 2002 by Miguel Rodriguez
 *                          (C) 2006 by Julien Puydt
 *   description          : High level tray api implementation
 */


#include <gdk/gdkkeysyms.h>

#include "../../config.h"
#include "statusicon.h"

#include "gmstockicons.h"
#include "gmmenuaddon.h"
#include "misc.h"
#include "gmtray/gmtray.h"

#include "callbacks.h"
#include "ekiga.h"

struct GmStatusicon {
  GmTray *tray;
  GtkWidget *popup_menu;
  gboolean has_message;
};


static void
free_statusicon (gpointer data)
{
  GmStatusicon *statusicon = (GmStatusicon *)data;

  gmtray_delete (statusicon->tray);
  gtk_widget_destroy (statusicon->popup_menu);

  g_free (statusicon);
}


static GmStatusicon *
get_statusicon (GtkWidget *widget)
{
  g_return_val_if_fail (widget != NULL, NULL);

  return (GmStatusicon *)g_object_get_data (G_OBJECT (widget), "GMObject");
}


static GtkWidget *
build_menu (GtkWidget *widget)
{
  GtkWidget *addressbook_window = NULL;
  GtkWidget *main_window = NULL;
  GtkWidget *prefs_window = NULL;
  IncomingCallMode mode = AVAILABLE;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  addressbook_window = GnomeMeeting::Process ()->GetAddressbookWindow ();
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();

  mode =
    (IncomingCallMode) gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode");

  static MenuEntry menu [] =
    {
      GTK_MENU_ENTRY("connect", _("_Call"), _("Place a new call"),
		     GM_STOCK_CONNECT_16, 'c',
		     GTK_SIGNAL_FUNC (connect_cb), main_window, TRUE),
      GTK_MENU_ENTRY("disconnect", _("_Hang up"),
		     _("Terminate the current call"),
		     GM_STOCK_DISCONNECT_16, 'd',
		     GTK_SIGNAL_FUNC (disconnect_cb), NULL, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_RADIO_ENTRY("available", _("_Available"),
			   _("Display a popup to accept the call"),
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (mode == AVAILABLE), TRUE),
      GTK_MENU_RADIO_ENTRY("auto_answer", _("Aut_o Answer"),
			   _("Auto answer calls"),
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (mode == AUTO_ANSWER), TRUE),
      GTK_MENU_RADIO_ENTRY("do_not_disturb", _("_Do Not Disturb"),
			   _("Reject calls"),
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (mode == DO_NOT_DISTURB), TRUE),
      GTK_MENU_RADIO_ENTRY("forward", _("_Forward"), _("Forward calls"),
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (mode == FORWARD), TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("address_book", _("Address _Book"),
		     _("Open the address book"),
		     GM_STOCK_ADDRESSBOOK_16, 0,
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) addressbook_window, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("preferences", _("_Preferences"),
		     _("Change your preferences"),
		     GTK_STOCK_PREFERENCES, 'P',
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) prefs_window, TRUE),

      GTK_MENU_SEPARATOR,

#ifndef DISABLE_GNOME
       GTK_MENU_ENTRY("help", _("_Contents"),
                     _("Get help by reading the Ekiga manual"),
                     GTK_STOCK_HELP, GDK_F1,
                     GTK_SIGNAL_FUNC (help_cb), NULL, TRUE),

      GTK_MENU_ENTRY("about", _("_About"),
		     _("View information about Ekiga"),
		     GNOME_STOCK_ABOUT, 'a',
		     GTK_SIGNAL_FUNC (about_callback), (gpointer) main_window,
		     TRUE),
#else
      GTK_MENU_ENTRY("help", _("_Contents"),
                     _("Get help by reading the Ekiga manual"),
                     GTK_STOCK_HELP, GDK_F1,
                     NULL, NULL, FALSE),

      GTK_MENU_ENTRY("about", _("_About"),
		     _("View information about Ekiga"),
		     NULL, 'a',
		     GTK_SIGNAL_FUNC (about_callback), (gpointer) main_window,
		     TRUE),
#endif
      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("quit", _("_Quit"),
		     _("Quit Ekiga"),
		     GTK_STOCK_QUIT, 'Q',
		     GTK_SIGNAL_FUNC (quit_callback),
		     main_window, TRUE),

      GTK_MENU_END
    };

  return GTK_WIDGET (gtk_build_popup_menu (widget, menu, NULL));
}


static GtkMenu *
get_menu_callback (gpointer data)
{
  GmStatusicon *statusicon = NULL;

  g_return_val_if_fail (GTK_IS_WIDGET (data), NULL);

  statusicon = get_statusicon (GTK_WIDGET (data));

  g_return_val_if_fail (statusicon != NULL, NULL);

  return GTK_MENU (statusicon->popup_menu);
}


static void
left_clicked_callback (gpointer data)
{
  GmStatusicon *statusicon = NULL;
  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;
  GtkWidget *window = NULL;

  g_return_if_fail (GTK_IS_WIDGET (data));

  statusicon = get_statusicon (GTK_WIDGET (data));

  g_return_if_fail (statusicon != NULL);

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();

  if (statusicon->has_message) {

    if (!gnomemeeting_window_is_visible (chat_window)) {

      window = chat_window;
      gm_statusicon_signal_message (GTK_WIDGET (data), FALSE);
    } else
      window = main_window;
  } else
    window = main_window;

  if (!gnomemeeting_window_is_visible (window))
    gnomemeeting_window_show (window);
  else
    gnomemeeting_window_hide (window);
}


static void
middle_clicked_callback (gpointer data)
{
  GmStatusicon *statusicon = NULL;
  GtkWidget *window = NULL;

  g_return_if_fail (GTK_IS_WIDGET (data));

  statusicon = get_statusicon (GTK_WIDGET (data));

  g_return_if_fail (statusicon != NULL);

  window = GnomeMeeting::Process ()->GetAddressbookWindow ();

  if (!gnomemeeting_window_is_visible (window))
    gnomemeeting_window_show (window);
  else
    gnomemeeting_window_hide (window);
}


GtkWidget *
gm_statusicon_new ()
{
  GtkWidget *widget = NULL;
  GmStatusicon *statusicon = NULL;

  widget = gtk_label_new ("");

  statusicon = g_new0 (struct GmStatusicon, 1);

  statusicon->tray = gmtray_new (GM_STOCK_STATUS_AVAILABLE);
  gmtray_set_left_clicked_callback (statusicon->tray,
				    left_clicked_callback, (gpointer)widget);
  gmtray_set_middle_clicked_callback (statusicon->tray,
				      middle_clicked_callback,
				      (gpointer)widget);
  gmtray_set_menu_callback (statusicon->tray,
			    get_menu_callback, (gpointer)widget);

  statusicon->popup_menu = build_menu (widget);

  statusicon->has_message = FALSE;

  g_object_set_data_full (G_OBJECT (widget), "GMObject",
			  (gpointer)statusicon,
			  (GDestroyNotify)free_statusicon);

  return widget;
}


void
gm_statusicon_update_full (GtkWidget *widget,
			   GMManager::CallingState state,
			   IncomingCallMode mode,
			   gboolean forward_on_busy)
{
  GmStatusicon *statusicon = NULL;
  GtkWidget *menu = NULL;

  g_return_if_fail (widget != NULL);

  statusicon = get_statusicon (widget);

  g_return_if_fail (statusicon != NULL);

  /* Update the menu */
  gm_statusicon_update_menu (widget, state);
  menu = gtk_menu_get_widget (statusicon->popup_menu, "available");
  gtk_radio_menu_select_with_widget (GTK_WIDGET (menu), mode);

  /* Update the icon */
  if (state == GMManager::Standby) {

    switch (mode) {

    case AVAILABLE:
      gmtray_set_image (statusicon->tray, GM_STOCK_STATUS_AVAILABLE);
      break;

    case (AUTO_ANSWER):
      gmtray_set_image (statusicon->tray, GM_STOCK_STATUS_AUTO_ANSWER);
      break;

    case (DO_NOT_DISTURB):
      gmtray_set_image (statusicon->tray, GM_STOCK_STATUS_DO_NOT_DISTURB);
      break;

    case (FORWARD):
      gmtray_set_image (statusicon->tray, GM_STOCK_STATUS_FORWARD);
      break;

    default:
      break;
    }
  }
  else {

    if (forward_on_busy)
      gmtray_set_image (statusicon->tray, GM_STOCK_STATUS_FORWARD);
    else
      gmtray_set_image (statusicon->tray, GM_STOCK_STATUS_IN_A_CALL);
  }

}


void
gm_statusicon_update_menu (GtkWidget *widget,
			   GMManager::CallingState state)
{
  GmStatusicon *statusicon = NULL;

  g_return_if_fail (widget != NULL);

  statusicon = get_statusicon (widget);

  g_return_if_fail (statusicon != NULL);

  switch (state) {

  case GMManager::Standby:
    gtk_menu_set_sensitive (statusicon->popup_menu, "connect", TRUE);
    gtk_menu_set_sensitive (statusicon->popup_menu, "disconnect", FALSE);
    break;

  case GMManager::Calling:
    gtk_menu_set_sensitive (statusicon->popup_menu, "connect", FALSE);
    gtk_menu_set_sensitive (statusicon->popup_menu, "disconnect", TRUE);
    break;

  case GMManager::Connected:
    gtk_menu_set_sensitive (statusicon->popup_menu, "connect", FALSE);
    gtk_menu_set_sensitive (statusicon->popup_menu, "disconnect", TRUE);
    break;

  case GMManager::Called:
    gtk_menu_set_sensitive (statusicon->popup_menu, "disconnect", TRUE);
    break;
  }

}


void 
gm_statusicon_set_busy (GtkWidget *widget,
			BOOL busy)
{
  GmStatusicon *statusicon = NULL;

  g_return_if_fail (widget != NULL);

  statusicon = get_statusicon (widget);

  g_return_if_fail (statusicon != NULL);

  gtk_menu_set_sensitive (statusicon->popup_menu, "quit", !busy);
}


void
gm_statusicon_signal_message (GtkWidget *widget,
			      gboolean has_message)
{
  GmStatusicon *statusicon = NULL;

  g_return_if_fail (widget != NULL);

  statusicon = get_statusicon (widget);

  g_return_if_fail (statusicon != NULL);

  statusicon->has_message = has_message;

  if (has_message) {
    
    if (!gmtray_is_blinking (statusicon->tray))
      gmtray_blink (statusicon->tray, GM_STOCK_MESSAGE, 1000);
  }
  else
    gmtray_stop_blink (statusicon->tray);
}


void
gm_statusicon_ring (GtkWidget *widget,
		    guint interval)
{
  GmStatusicon *statusicon = NULL;

  g_return_if_fail (widget != NULL);

  statusicon = get_statusicon (widget);

  g_return_if_fail (statusicon != NULL);

  gmtray_blink (statusicon->tray, GM_STOCK_STATUS_RINGING, interval);
}


void
gm_statusicon_stop_ringing (GtkWidget *widget)
{
  GmStatusicon *statusicon = NULL;

  g_return_if_fail (widget != NULL);

  statusicon = get_statusicon (widget);

  g_return_if_fail (statusicon != NULL);

  gmtray_stop_blink (statusicon->tray);
}


gboolean
gm_statusicon_is_embedded (GtkWidget *widget)
{
  return TRUE;  /* FIXME */
}
