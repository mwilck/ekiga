
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
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
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         menu.cpp  -  description 
 *                         ------------------------
 *   begin                : Tue Dec 23 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Functions to create the menus.
 *
 */


#include "../config.h"

#include "menu.h"
#include "connection.h"
#include "endpoint.h"
#include "callbacks.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "chat_window.h"
#include "addressbook_window.h"

#include "stock-icons.h"
#include "gtk_menu_extensions.h"
#include "gm_conf.h"
#include "contacts/gm_contacts.h"

#include <gdk/gdkkeysyms.h>


/* Declarations */
extern GtkWidget *gm;


/* Static functions */
static void speed_dial_menu_item_selected (GtkWidget *,
					   gpointer);


/* Those 2 callbacks update a config key when 
   a menu item is toggled */
static void radio_menu_changed (GtkWidget *, 
				gpointer);





/* GTK Callbacks */



/* DESCRIPTION  :  This callback is called when the user toggles an 
 *                 item in the speed dials menu.
 * BEHAVIOR     :  Calls the given speed dial.
 * PRE          :  data is the speed dial as a gchar *
 */
static void
speed_dial_menu_item_selected (GtkWidget *w,
			       gpointer data)
{
  GmWindow *gw = NULL;
  GMH323EndPoint *ep = NULL;
  
  gchar *url = NULL;
    
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  g_return_if_fail (data != NULL);

  url = g_strdup_printf ("%s#", (gchar *) data);
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry),
		      (gchar *) url);
    
  if (ep->GetCallingState () == GMH323EndPoint::Connected)
    transfer_call_cb (NULL, (gpointer) url);
  else
    connect_cb (NULL, NULL);

  g_free (url);
}


/* DESCRIPTION  :  This callback is called when the user 
 *                 selects a different option in a radio menu.
 * BEHAVIOR     :  Sets the config key.
 * PRE          :  data is the config key.
 */
static void 
radio_menu_changed (GtkWidget *widget,
		    gpointer data)
{
  GSList *group = NULL;

  int group_last_pos = 0;
  int active = 0;

  group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (widget));
  group_last_pos = g_slist_length (group) - 1; /* If length 1, last pos is 0 */

  /* Only do something when a new CHECK_MENU_ITEM becomes active,
     not when it becomes inactive */
  if (GTK_CHECK_MENU_ITEM (widget)->active) {

    while (group) {

      if (group->data == widget) 
	break;
      
      active++;
      group = g_slist_next (group);
    }

    gm_conf_set_int ((gchar *) data, group_last_pos - active);
  }
}


/* The functions */
GtkWidget *
gnomemeeting_video_popup_init_menu (GtkWidget *widget, GtkAccelGroup *accel)
{
  GtkWidget *popup_menu_widget = NULL;
  
  static MenuEntry video_menu [] =
    {

      GTK_MENU_END
    };

  cout << "FIX ME" << endl << flush;
  popup_menu_widget = gtk_build_popup_menu (widget, video_menu, accel);
  
  return popup_menu_widget;
}


GtkWidget *
gnomemeeting_tray_init_menu (GtkWidget *widget)
{
  GtkWidget *popup_menu_widget = NULL;
  GtkWidget *addressbook_window = NULL;
  GtkWidget *calls_history_window = NULL;
  GtkWidget *prefs_window = NULL;

  IncomingCallMode icm = AVAILABLE;
  GmWindow *gw = NULL;

  
  gw = GnomeMeeting::Process ()->GetMainWindow ();
  addressbook_window = GnomeMeeting::Process ()->GetAddressbookWindow ();
  calls_history_window = GnomeMeeting::Process ()->GetCallsHistoryWindow ();
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();
  
  icm = (IncomingCallMode)gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode"); 

  static MenuEntry tray_menu [] =
    {
      GTK_MENU_ENTRY("connect", _("_Connect"), _("Create a new connection"), 
		     GM_STOCK_CONNECT_16, 'c', 
		     GTK_SIGNAL_FUNC (connect_cb), gw, TRUE),
      GTK_MENU_ENTRY("disconnect", _("_Disconnect"),
		     _("Close the current connection"), 
		     GM_STOCK_DISCONNECT_16, 'd',
		     GTK_SIGNAL_FUNC (disconnect_cb), gw, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_RADIO_ENTRY("available", _("_Available"),
			   _("Display a popup to accept the call"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == AVAILABLE), TRUE),
      GTK_MENU_RADIO_ENTRY("free_for_chat", _("Free for Cha_t"),
			   _("Auto answer calls"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == FREE_FOR_CHAT), TRUE),
      GTK_MENU_RADIO_ENTRY("busy", _("_Busy"), _("Reject calls"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == BUSY), TRUE),
      GTK_MENU_RADIO_ENTRY("forward", _("_Forward"), _("Forward calls"),
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == FORWARD), TRUE),

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
     
      GTK_MENU_ENTRY("about", _("_About..."),
		     _("View information about GnomeMeeting"),
		     NULL, 'a', 
		     GTK_SIGNAL_FUNC (about_callback),
		     (gpointer) gm, TRUE),

      GTK_MENU_ENTRY("quit", _("_Quit"), 
		     _("Quit GnomeMeeting"),
		     GTK_STOCK_QUIT, 'Q', 
		     GTK_SIGNAL_FUNC (quit_callback),
		     (gpointer) gw, TRUE),

      GTK_MENU_END
    };

  
  popup_menu_widget = gtk_build_popup_menu (widget, tray_menu, NULL);

  return popup_menu_widget;
}


void
gnomemeeting_speed_dials_menu_update (GtkWidget *menubar)
{
  GtkWidget *item = NULL;
  GtkWidget *menu = NULL;

  GmContact *contact = NULL;

  GSList *glist = NULL;
  GSList *glist_iter = NULL;
  GList *old_glist_iter = NULL;

  gchar *ml = NULL;  

  glist = gnomemeeting_addressbook_get_contacts (NULL, FALSE, NULL, NULL, NULL, "*"); 
  menu = gtk_menu_get_widget (menubar, "speed_dials");

  while ((old_glist_iter = GTK_MENU_SHELL (menu)->children)) 
    gtk_container_remove (GTK_CONTAINER (menu),
			  GTK_WIDGET (old_glist_iter->data));

  glist_iter = glist;
  while (glist_iter && glist_iter->data) {

    contact = GM_CONTACT (glist_iter->data);

    ml = g_strdup_printf ("<b>%s#</b>   <i>%s</i>", 
			  contact->speeddial, 
			  contact->fullname);

    item = gtk_menu_item_new_with_label (ml);
    gtk_label_set_markup (GTK_LABEL (gtk_bin_get_child (GTK_BIN (item))),
			  ml);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    gtk_widget_show (item);

    g_signal_connect_data (G_OBJECT (item), "activate",
			   GTK_SIGNAL_FUNC (speed_dial_menu_item_selected),
			   (gpointer) g_strdup (contact->url),
			   (GClosureNotify) g_free, (GConnectFlags) 0);


    glist_iter = g_slist_next (glist_iter);

    g_free (ml);
  }

  g_slist_foreach (glist, (GFunc) gm_contact_delete, NULL);
  g_slist_free (glist);
}


void
gnomemeeting_menu_update_sensitivity (unsigned calling_state)
{
  GmWindow *gw = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  switch (calling_state)
    {
    case GMH323EndPoint::Standby:

      gtk_menu_set_sensitive (gw->main_menu, "connect", TRUE);
      gtk_menu_set_sensitive (gw->tray_popup_menu, "connect", TRUE);
      gtk_menu_set_sensitive (gw->main_menu, "disconnect", FALSE);
      gtk_menu_set_sensitive (gw->tray_popup_menu, "disconnect", FALSE);
      gtk_menu_section_set_sensitive (gw->main_menu, "hold_call", FALSE);
      break;


    case GMH323EndPoint::Calling:

      gtk_menu_set_sensitive (gw->main_menu, "connect", FALSE);
      gtk_menu_set_sensitive (gw->tray_popup_menu, "connect", FALSE);
      gtk_menu_set_sensitive (gw->main_menu, "disconnect", TRUE);
      gtk_menu_set_sensitive (gw->tray_popup_menu, "disconnect", TRUE);
      break;


    case GMH323EndPoint::Connected:
      gtk_menu_set_sensitive (gw->main_menu, "connect", FALSE);
      gtk_menu_set_sensitive (gw->tray_popup_menu, "connect", FALSE);
      gtk_menu_set_sensitive (gw->main_menu, "disconnect", TRUE);
      gtk_menu_set_sensitive (gw->tray_popup_menu, "disconnect", TRUE);
      gtk_menu_section_set_sensitive (gw->main_menu, "hold_call", TRUE);
      break;


    case GMH323EndPoint::Called:
      gtk_menu_set_sensitive (gw->main_menu, "disconnect", TRUE);
      gtk_menu_set_sensitive (gw->tray_popup_menu, "disconnect", TRUE);
      break;
    }
}


void
gnomemeeting_menu_update_sensitivity (BOOL is_video,
				      BOOL is_receiving,
				      BOOL is_transmitting)
{
  GmWindow *gw = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  if (is_video) {

    if (is_transmitting) {
      
      gtk_menu_section_set_sensitive (gw->main_menu, "local_video", TRUE);
      gtk_menu_section_set_sensitive (gw->main_menu, "suspend_video", TRUE);
    }
    else {
      
      gtk_menu_section_set_sensitive (gw->main_menu, "local_video", FALSE);
      gtk_menu_section_set_sensitive (gw->main_menu, "suspend_video", FALSE);
    }
    
    if (is_receiving && is_transmitting) {
    
      gtk_menu_section_set_sensitive (gw->main_menu,
				      "local_video", TRUE);
      gtk_menu_section_set_sensitive (gw->video_popup_menu,
				      "local_video", TRUE);
    }
    else {
      
      gtk_menu_section_set_sensitive (gw->main_menu,
				      "local_video", FALSE);
      gtk_menu_section_set_sensitive (gw->video_popup_menu,
				      "local_video", FALSE);
      if (is_transmitting) {
	
	gtk_menu_set_sensitive (gw->main_menu,
				"local_video", TRUE);
	gtk_menu_set_sensitive (gw->video_popup_menu,
				"local_video", TRUE);
      }
      else if (is_receiving) {

	gtk_menu_set_sensitive (gw->main_menu,
				"remote_video", TRUE);
	gtk_menu_set_sensitive (gw->video_popup_menu,
				"remote_video", TRUE);
      }

      if (!is_receiving && !is_transmitting) {

	gtk_menu_section_set_sensitive (gw->main_menu,
					"zoom_in", FALSE);
	gtk_menu_section_set_sensitive (gw->video_popup_menu,
					"zoom_in", FALSE);
	gtk_menu_set_sensitive (gw->main_menu, "save_picture", FALSE);
      }
      else {

	gtk_menu_section_set_sensitive (gw->main_menu,
					"zoom_in", TRUE);
	gtk_menu_section_set_sensitive (gw->video_popup_menu,
					"zoom_in", TRUE);
	gtk_menu_set_sensitive (gw->main_menu, "save_picture", TRUE);
      }
    }
  }
  else {

    if (is_transmitting)
      gtk_menu_set_sensitive (gw->main_menu, "suspend_audio", TRUE);
    else
      gtk_menu_set_sensitive (gw->main_menu, "suspend_audio", FALSE);
  }
}
