
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
 *                         tray.cpp  -  description
 *                         ------------------------
 *   begin                : Wed Oct 3 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras, 2002 by Miguel 
 *                          Rodríguez
 *   description          : This file contains all functions needed for
 *                          system tray icon.
 *   Additional code      : miguelrp@gmail.com
 *
 */


#include "../config.h"

#include "tray.h"
#include "gnomemeeting.h"

#ifndef DISABLE_GNOME
#include "eggtrayicon.h"
#endif

#include "callbacks.h"
#include "misc.h"

#include "stock-icons.h"
#include "gm_conf.h"
#include "lib/gtk_menu_extensions.h"


#include <gdk/gdkkeysyms.h>


/* Declarations */

typedef struct _GmTray {

  GtkWidget *popup_menu;
  GtkWidget *image;
  
  gboolean ringing;
  gboolean embedded;
  
  gint message;
  gchar *current_stock;

} GmTray;

#define GM_TRAY(x) (GmTray *) (x)



/* DESCRIPTION  : / 
 * BEHAVIOR     : Frees a GmAddressbookWindow and its content.
 * PRE          : A non-NULL pointer to a GmAddressbookWindow.
 */
static void gm_tray_destroy (gpointer);
	

/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to the private GmTray
 * 		  used by the tray GMObject.
 * PRE          : The given GtkWidget pointer must be a tray GMObject.
 */
static GmTray *gm_tray_get_tray (GtkWidget *);


/* DESCRIPTION  :  This callback is called when the tray appears on the panel.
 * BEHAVIOR     :  Store the info in the GMObject.
 * PRE          :  /
 */
static gint tray_embedded_cb (GtkWidget *, 
			      gpointer);



/* DESCRIPTION  :  This callback is called when the user clicks on the 
 *                 tray event-box.
 * BEHAVIOR     :  Show / hide the GnomeMeeting GUI or address book.
 * PRE          :  /
 */
static gint tray_clicked_cb (GtkWidget *, 
			     GdkEventButton *, 
			     gpointer);


/* DESCRIPTION  :  This callback is called when the panel gets closed
 *                 after the tray has been embedded.
 * BEHAVIOR     :  Create a new tray_icon and substitute the old one.
 * PRE          :  /
 */
static gint tray_destroyed_cb (GtkWidget *, 
			       gpointer);


/* DESCRIPTION  :  This callback is called to flash an envelope icon to
 *                 indicate that a text message was received.
 * BEHAVIOR     :  Update the icon.
 * PRE          :  /
 */
static gint flash_message_cb (gpointer);


/* Implementation */

static void
gm_tray_destroy (gpointer tray)
{
  GmTray *gt = NULL;
  
  g_return_if_fail (tray != NULL);

  gt = GM_TRAY (tray);
  
  g_free (gt->current_stock);

  delete (gt);
}


static GmTray *
gm_tray_get_tray (GtkWidget *tray)
{
  g_return_val_if_fail (tray != NULL, NULL);

  return GM_TRAY (g_object_get_data (G_OBJECT (tray), "GMObject"));
}


static gint 
tray_embedded_cb (GtkWidget *tray_icon, 
		  gpointer data)
{
  GmTray *gt = NULL;
  
  IncomingCallMode icm = AVAILABLE;

  gt = gm_tray_get_tray (GTK_WIDGET (tray_icon));
  g_return_val_if_fail (gt != NULL, FALSE);
   
  /* Check the current incoming call mode */
  icm =
    (IncomingCallMode) gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode");

  gm_tray_update (tray_icon, GMEndPoint::Standby, icm);
  gt->embedded = TRUE;

  return true;
}


static gint 
tray_destroyed_cb (GtkWidget *tray, 
		   gpointer data) 
{
  g_return_val_if_fail (data != NULL, FALSE);

  gnomemeeting_window_show (GTK_WIDGET (data));

  return FALSE;
}


static gint 
flash_message_cb (gpointer data)
{
  GmTray *gt = NULL;
  gchar *current_stock = NULL;
  GtkIconSize size;
  
  g_return_val_if_fail (data != NULL, FALSE);
  gt = gm_tray_get_tray (GTK_WIDGET (data));
  g_return_val_if_fail (gt != NULL, FALSE);

  gtk_image_get_stock (GTK_IMAGE (gt->image), &current_stock, &size);

  if ((current_stock && !strcmp (current_stock, GM_STOCK_MESSAGE))
      || gt->message == 0)
    gtk_image_set_from_stock (GTK_IMAGE (gt->image), 
			      gt->current_stock, 
			      GTK_ICON_SIZE_MENU);
  else
    gtk_image_set_from_stock (GTK_IMAGE (gt->image), 
			      GM_STOCK_MESSAGE, 
			      GTK_ICON_SIZE_MENU);

  return (gt->message != 0);
}


static gint
tray_clicked_cb (GtkWidget *w,
		 GdkEventButton *event,
		 gpointer data)
{
  GmTray *gt = NULL;
  
  GtkWidget *widget = NULL;
  
  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;
  GtkWidget *addressbook_window = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  addressbook_window = GnomeMeeting::Process ()->GetAddressbookWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();

  g_return_val_if_fail (data != NULL, FALSE);
  gt = gm_tray_get_tray (GTK_WIDGET (data));
  g_return_val_if_fail (gt != NULL, FALSE);

  if (event->type == GDK_BUTTON_PRESS) {

    if (event->button == 1 && gt->message != 0) {

      gm_tray_update_has_message (GTK_WIDGET (data), FALSE);
      if (!gnomemeeting_window_is_visible (chat_window))
	gnomemeeting_window_show (chat_window);
      return TRUE;
    }
    
    if (event->button == 1)
      widget = main_window;
    else if (event->button == 2)
      widget = addressbook_window;
    else
      return FALSE;


    if (!gnomemeeting_window_is_visible (widget))
      gnomemeeting_window_show (widget);
    else
      gnomemeeting_window_hide (widget);

    return TRUE;
  }

  return FALSE;
}


/* The functions */
GtkWidget *
gm_tray_new ()
{
  GmTray *gt = NULL;

  GtkWidget *tray_icon = NULL;
  GtkWidget *event_box = NULL;

  GtkWidget *addressbook_window = NULL;
  GtkWidget *calls_history_window = NULL;
  GtkWidget *main_window = NULL;
  GtkWidget *prefs_window = NULL;

  IncomingCallMode icm = AVAILABLE;


  /* Get the data */
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  addressbook_window = GnomeMeeting::Process ()->GetAddressbookWindow ();
  calls_history_window = GnomeMeeting::Process ()->GetCallsHistoryWindow ();
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();

  icm = 
    (IncomingCallMode) gm_conf_get_int (CALL_OPTIONS_KEY "incoming_call_mode"); 


  /* Start building the GMObject and associate the structure
   * to the object so that it is deleted when the object is
   * destroyed
   */
#ifdef DISABLE_GNOME
  tray_icon = gtk_window_new (GTK_WINDOW_TOPLEVEL);
#else
  tray_icon = GTK_WIDGET (egg_tray_icon_new (_("GnomeMeeting Tray Icon")));
#endif


  /* The GMObject data */
  gt = new GmTray ();
  g_object_set_data_full (G_OBJECT (tray_icon), "GMObject",
			  (gpointer) gt, 
			  (GDestroyNotify) (gm_tray_destroy));


  /* The menu */
  static MenuEntry tray_menu [] =
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
			   (icm == AVAILABLE), TRUE),
      GTK_MENU_RADIO_ENTRY("auto_answer", _("Aut_o Answer"),
			   _("Auto answer calls"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == AUTO_ANSWER), TRUE),
      GTK_MENU_RADIO_ENTRY("do_not_disturb", _("_Do Not Disturb"), 
			   _("Reject calls"),
			   NULL, 0, 
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
			   (gpointer) CALL_OPTIONS_KEY "incoming_call_mode",
			   (icm == DO_NOT_DISTURB), TRUE),
      GTK_MENU_RADIO_ENTRY("forward", _("_Forward"), _("Forward calls"),
			   NULL, 0,
			   GTK_SIGNAL_FUNC (radio_menu_changed_cb),
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
     
#ifndef DISABLE_GNOME
       GTK_MENU_ENTRY("help", _("_Contents"),
                     _("Get help by reading the GnomeMeeting manual"),
                     GTK_STOCK_HELP, GDK_F1, 
                     GTK_SIGNAL_FUNC (help_cb), NULL, TRUE),
       
      GTK_MENU_ENTRY("about", _("_About"),
		     _("View information about GnomeMeeting"),
		     GNOME_STOCK_ABOUT, 'a', 
		     GTK_SIGNAL_FUNC (about_callback), (gpointer) main_window,
		     TRUE),
#else
      GTK_MENU_ENTRY("help", _("_Contents"),
                     _("Get help by reading the GnomeMeeting manual"),
                     GTK_STOCK_HELP, GDK_F1, 
                     NULL, NULL, FALSE),
       
      GTK_MENU_ENTRY("about", _("_About"),
		     _("View information about GnomeMeeting"),
		     NULL, 'a', 
		     GTK_SIGNAL_FUNC (about_callback), (gpointer) main_window,
		     TRUE),
#endif

      GTK_MENU_ENTRY("quit", _("_Quit"), 
		     _("Quit GnomeMeeting"),
		     GTK_STOCK_QUIT, 'Q', 
		     GTK_SIGNAL_FUNC (quit_callback),
		     main_window, TRUE),

      GTK_MENU_END
    };

  
  gt->popup_menu = gtk_build_popup_menu (tray_icon, tray_menu, NULL);

  
  /* The rest of the tray */
  event_box = gtk_event_box_new ();
  gt->image = gtk_image_new_from_stock (GM_STOCK_STATUS_AVAILABLE,
                                        GTK_ICON_SIZE_MENU);
  gt->ringing = FALSE;
  gt->embedded = FALSE;
  gt->message = 0;
  gt->current_stock = NULL;
  
  gtk_container_add (GTK_CONTAINER (event_box), gt->image);
  gtk_container_add (GTK_CONTAINER (tray_icon), event_box);
  

  /* Connect the signals */
#ifndef DISABLE_GNOME
  g_signal_connect (G_OBJECT (tray_icon), "embedded",
		    G_CALLBACK (tray_embedded_cb), NULL);
#endif
  g_signal_connect (G_OBJECT (tray_icon), "destroy",
		    G_CALLBACK (tray_destroyed_cb), main_window);
  g_signal_connect (G_OBJECT (event_box), "button_press_event",
		    G_CALLBACK (tray_clicked_cb), tray_icon);

  
  gtk_widget_show_all (tray_icon);
  
  return tray_icon;
}


void 
gm_tray_update (GtkWidget *tray_icon,
		GMEndPoint::CallingState calling_state, 
		IncomingCallMode icm,
		BOOL forward_on_busy)
{
  GmTray *gt = NULL;

  GtkWidget *menu = NULL;
  
  g_return_if_fail (tray_icon != NULL);
  

  gt = gm_tray_get_tray (tray_icon);
  g_return_if_fail (gt != NULL);
  
  
  /* Update the menu */
  menu = gtk_menu_get_widget (gt->popup_menu, "available");
  gtk_radio_menu_select_with_widget (GTK_WIDGET (menu), icm);
  

  /* Update the icon */
  if (calling_state == GMEndPoint::Standby) {

    switch (icm) {

    case (AVAILABLE): 
      gtk_image_set_from_stock (GTK_IMAGE (gt->image), 
                                GM_STOCK_STATUS_AVAILABLE, 
                                GTK_ICON_SIZE_MENU);
      gt->current_stock = g_strdup (GM_STOCK_STATUS_AVAILABLE);
      break;
   
    case (AUTO_ANSWER):  
      gtk_image_set_from_stock (GTK_IMAGE (gt->image), 
                                GM_STOCK_STATUS_AUTO_ANSWER,
                                GTK_ICON_SIZE_MENU);
      gt->current_stock = g_strdup (GM_STOCK_STATUS_AUTO_ANSWER);
      break;
    
    case (DO_NOT_DISTURB):  
      gtk_image_set_from_stock (GTK_IMAGE (gt->image), 
                                GM_STOCK_STATUS_DO_NOT_DISTURB, 
                                GTK_ICON_SIZE_MENU);
      gt->current_stock = g_strdup (GM_STOCK_STATUS_DO_NOT_DISTURB);
      break;
    
    case (FORWARD):  
      gtk_image_set_from_stock (GTK_IMAGE (gt->image), 
                                GM_STOCK_STATUS_FORWARD, 
                                GTK_ICON_SIZE_MENU);
      gt->current_stock = g_strdup (GM_STOCK_STATUS_FORWARD);
      break;

    default:
      break;
    }
  }
  else {

    if (forward_on_busy) {
      
      gtk_image_set_from_stock (GTK_IMAGE (gt->image), 
                                GM_STOCK_STATUS_FORWARD, 
                                GTK_ICON_SIZE_MENU);
      gt->current_stock = g_strdup (GM_STOCK_STATUS_FORWARD);
    }

    else {
      gtk_image_set_from_stock (GTK_IMAGE (gt->image), 
                                GM_STOCK_STATUS_IN_A_CALL, 
                                GTK_ICON_SIZE_MENU);
      gt->current_stock = g_strdup (GM_STOCK_STATUS_IN_A_CALL);
    }
  }
}


void 
gm_tray_update_has_message (GtkWidget *tray_icon,
			    gboolean has_message)
{
  GmTray *gt = NULL;

  g_return_if_fail (tray_icon != NULL);
  
  gt = gm_tray_get_tray (tray_icon);
  
  g_return_if_fail (gt != NULL);
  
  if (has_message && gt->message == 0) {
    
    gt->message = 
      g_timeout_add (1000, 
		     flash_message_cb, 
		     (gpointer) tray_icon);
  }
  else if (!has_message) {

    gt->message = 0;
  }
}


void 
gm_tray_update_calling_state (GtkWidget *tray,
			      int calling_state)
{
  GmTray *gt = NULL;

  g_return_if_fail (tray != NULL);


  gt = gm_tray_get_tray (tray);
  g_return_if_fail (gt != NULL);


  switch (calling_state)
    {
    case GMEndPoint::Standby:

      gtk_menu_set_sensitive (gt->popup_menu, "connect", TRUE);
      gtk_menu_set_sensitive (gt->popup_menu, "disconnect", FALSE);
      break;


    case GMEndPoint::Calling:

      gtk_menu_set_sensitive (gt->popup_menu, "connect", FALSE);
      gtk_menu_set_sensitive (gt->popup_menu, "disconnect", TRUE);
      break;


    case GMEndPoint::Connected:
      gtk_menu_set_sensitive (gt->popup_menu, "connect", FALSE);
      gtk_menu_set_sensitive (gt->popup_menu, "disconnect", TRUE);
      break;


    case GMEndPoint::Called:
      gtk_menu_set_sensitive (gt->popup_menu, "disconnect", TRUE);
      break;
    }
}


void 
gm_tray_ring (GtkWidget *tray)
{
  GmTray *gt = NULL;

  g_return_if_fail (tray != NULL);

  
  gt = gm_tray_get_tray (tray);
  g_return_if_fail (gt != NULL);
  

  if (gt->ringing) {

    gtk_image_set_from_stock (GTK_IMAGE (gt->image), 
                              GM_STOCK_STATUS_AVAILABLE, 
                              GTK_ICON_SIZE_MENU);
    gt->ringing = FALSE;
  }
  else {

    gtk_image_set_from_stock (GTK_IMAGE (gt->image), 
                              GM_STOCK_STATUS_RINGING, 
                              GTK_ICON_SIZE_MENU); 
    gt->ringing = TRUE;
  }
}


gboolean 
gm_tray_is_ringing (GtkWidget *tray)
{
  GmTray *gt = NULL;

  g_return_val_if_fail (tray != NULL, FALSE);
  
  gt = gm_tray_get_tray (tray);
  g_return_val_if_fail (gt != NULL, FALSE);


  if (!gt)
    return FALSE;
  
  return (gt->ringing);
}


gboolean 
gm_tray_is_embedded (GtkWidget *tray)
{
  GmTray *gt = NULL;

  g_return_val_if_fail (tray != NULL, FALSE);

  
  gt = gm_tray_get_tray (tray);
  g_return_val_if_fail (gt != NULL, FALSE);

  if (!gt)
    return FALSE;

  return (gt->embedded);
}
