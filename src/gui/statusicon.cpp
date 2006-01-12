#include <gdk/gdkkeysyms.h>

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
  GtkWidget *calls_history_window = NULL;
  GtkWidget *main_window = NULL;
  GtkWidget *prefs_window = NULL;
  IncomingCallMode mode = AVAILABLE;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  addressbook_window = GnomeMeeting::Process ()->GetAddressbookWindow ();
  calls_history_window = GnomeMeeting::Process ()->GetCallsHistoryWindow ();
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();

  mode =
    (IncomingCallMode) gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode");

  static MenuEntry menu [] =
    {
      GTK_MENU_ENTRY("connect", _("_Connect"), _("Create a new connection"),
		     GM_STOCK_CONNECT_16, 'c',
		     GTK_SIGNAL_FUNC (connect_cb), main_window, TRUE),
      GTK_MENU_ENTRY("disconnect", _("_Disconnect"),
		     _("Close the current connection"),
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

      GTK_MENU_ENTRY("calls_history", _("Calls History"),
		     _("View the calls history"),
		     GM_STOCK_CALLS_HISTORY, 0,
		     GTK_SIGNAL_FUNC (show_window_cb),
		     (gpointer) calls_history_window, TRUE),

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
gm_statusicon_signal_message (GtkWidget *widget,
			      gboolean has_message)
{
  GmStatusicon *statusicon = NULL;

  g_return_if_fail (widget != NULL);

  statusicon = get_statusicon (widget);

  g_return_if_fail (statusicon != NULL);

  statusicon->has_message = has_message;

  if (has_message)
    gmtray_blink (statusicon->tray, GM_STOCK_MESSAGE, 1000);
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
