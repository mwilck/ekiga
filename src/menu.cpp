
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2002 Damien Sandras
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
 */

/*
 *                         menu.cpp  -  description  <--> OK
 *                         ------------------------
 *   begin                : Tue Dec 23 2000
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : Functions to create the menus.
 *   email                : dsandras@seconix.com
 *
 */

#include "../config.h"

#include "callbacks.h"
#include "menu.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "stock-icons.h"

#include <gtk/gtkwidget.h>
#include <gconf/gconf-client.h>
#ifndef DISABLE_GNOME
#include <gnome.h>
#endif


/* Declarations */
extern GtkWidget *gm;
extern GnomeMeeting *MyApp;


/* Static functions */
static void microtelco_consult_cb (GtkWidget *, gpointer);
static gint popup_menu_callback (GtkWidget *, GdkEventButton *, gpointer);
static void menu_item_selected (GtkWidget *, gpointer);
static void zoom_changed_callback (GtkWidget *, gpointer);
static void fullscreen_changed_callback (GtkWidget *, gpointer);

static void video_view_changed_callback (GtkWidget *, gpointer);
static void view_menu_toggles_changed (GtkWidget *, gpointer);
static void menu_toggle_changed (GtkWidget *, gpointer);


/* GTK Callbacks */

static void
microtelco_consult_cb (GtkWidget *widget, gpointer data)
{
#ifndef DISABLE_GNOME
  GConfClient *client = NULL;

  gchar *filename = NULL;
  gchar *account = NULL;
  gchar *pin = NULL;
  gchar *buffer = NULL;
  
  int fd = -1;

  client = gconf_client_get_default ();

  account = gconf_client_get_string (client, GATEKEEPER_KEY "gk_alias", NULL);
  pin = gconf_client_get_string (client, GATEKEEPER_KEY "gk_password", NULL);

  if (!account || !pin)
    return;
  
  buffer =
    g_strdup_printf ("<HTML><HEAD><TITLE>MicroTelco Auto-Post</TITLE></HEAD>"
		     "<BODY BGCOLOR=\"#FFFFFF\" "
		     "onLoad=\"Javascript:document.autoform.submit()\">"
		     "<FORM NAME=\"autoform\" "
		     "ACTION=\"https://%s.an.microtelco.com/acct/Controller\" "
		     "METHOD=\"POST\">"
		     "<input type=\"hidden\" name=\"command\" value=\"caller_login\">"
		     "<input type=\"hidden\" name=\"caller_id\" value=\"%s\">"
		     "<input type=\"hidden\" name=\"caller_pin\" value=\"%s\">"
		     "</FORM></BODY></HTML>", account, account, pin);

  fd = g_file_open_tmp ("mktmicro-XXXXXX", &filename, NULL);

  write (fd, (char *) buffer, strlen (buffer));

  gnome_url_show (filename, NULL);

  g_free (filename);
  g_free (buffer);

  close (fd);
#endif
}


/* DESCRIPTION  :  This callback is called when the user clicks on an 
 *                 event-box.
 * BEHAVIOR     :  Displays the menu given as data if it was a right click.
 * PRE          :  data != NULL.
 */
static gint
popup_menu_callback (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  GtkMenu *menu;
  GdkEventButton *event_button;

  menu = GTK_MENU (data);
  
  if (event->type == GDK_BUTTON_PRESS) {

    event_button = (GdkEventButton *) event;
    if (event_button->button == 3) {

      gtk_menu_popup (menu, NULL, NULL, NULL, NULL,
		      event_button->button, event_button->time);
      return TRUE;
    }
  }
  
  return FALSE;
}


/* DESCRIPTION  :  This callback is called when a menu item is selected or
 *                 deselected.
 * BEHAVIOR     :  Displays the data in the statusbar.
 * PRE          :  If data is NULL, clears the statusbar, else displays data
 *                 as message in the statusbar.
 */
static void 
menu_item_selected (GtkWidget *w, gpointer data)
{
  GmWindow *gw = gnomemeeting_get_main_window (gm);

  gnomemeeting_statusbar_push (gw->statusbar, (char *) data);
}


/* DESCRIPTION  :  This callback is called when the user changes the zoom
 *                 factor in the menu.
 * BEHAVIOR     :  Sets zoom to 1:2 if data == 0, 1:1 if data == 1, 
 *                 2:1 if data == 2. (Updates the gconf key).
 * PRE          :  /
 */
static void 
zoom_changed_callback (GtkWidget *widget, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();
  
  double zoom = 
    gconf_client_get_float (client, 
			    "/apps/gnomemeeting/video_display/zoom_factor", 
			    NULL);

  switch (GPOINTER_TO_INT (data)) {

  case 0:
    if (zoom > 0.5)
      zoom = zoom / 2.0;
    break;

  case 1:
    zoom = 1.0;
    break;

  case 2:
    if (zoom < 2.00)
      zoom = zoom * 2.0;
  }

  gconf_client_set_float (client, 
			  "/apps/gnomemeeting/video_display/zoom_factor", 
			  zoom, 0);
}


/* DESCRIPTION  :  This callback is called when the user toggles fullscreen
 *                 factor in the popup menu.
 * BEHAVIOR     :  Toggles fullscreen.
 * PRE          :  gpointer is a valid pointer to a GmWindow structure.
 */
static void 
fullscreen_changed_callback (GtkWidget *widget, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();
  gboolean fs = false;
  
  fs = 
    gconf_client_get_bool (client, 
			   "/apps/gnomemeeting/video_display/fullscreen", 0);
  gconf_client_set_bool (client, "/apps/gnomemeeting/video_display/fullscreen",
			 !fs, NULL);
}


/* DESCRIPTION  :  This callback is called when the user changes the current
 *                 video view.
 * BEHAVIOR     :  Updates the popup and the normal menu so that they have
 *                 the same values. Updates the gconf key to remember what
 *                 video view is used.
 * PRE          :  gpointer is a valid pointer to a GmWindow structure.
 */
static void 
video_view_changed_callback (GtkWidget *widget, gpointer data)
{
  int view_number = 4;
  int i = 0;
  int j = 0;
  int key_value = 0;

  MenuEntry *right_menu = NULL;
  MenuEntry *bad_menu = NULL;
  MenuEntry *gnomemeeting_menu = gnomemeeting_get_menu (gm);
  MenuEntry *video_menu = gnomemeeting_get_video_menu (gm);

  if (data && !strcmp ((char *) data, "view")) {

    right_menu = gnomemeeting_menu;
    bad_menu = video_menu;
    i = VIDEO_VIEW_MENU_INDICE;
    j = 0;
  }
  else {

    right_menu = video_menu;
    bad_menu = gnomemeeting_menu;
    i = 0;
    j = VIDEO_VIEW_MENU_INDICE;
  }

  
  for (int cpt = i ; cpt <= view_number + i ; cpt++) {

    GTK_CHECK_MENU_ITEM (bad_menu [j].widget)->active =
      GTK_CHECK_MENU_ITEM (right_menu [cpt].widget)->active;
    gtk_widget_queue_draw (GTK_WIDGET (bad_menu [j].widget));

    if (GTK_CHECK_MENU_ITEM (right_menu [cpt].widget)->active)
      gconf_client_set_int (gconf_client_get_default (), 
			    "/apps/gnomemeeting/video_display/video_view", 
			    key_value, NULL);

    j++;
    key_value++;
  }
}


/* DESCRIPTION  :  This callback is called when the user toggles the 
 *                 corresponding option in the "Control Panel" View Menu.
 * BEHAVIOR     :  Sets the gconf key.
 * PRE          :  data is the gconf key.
 */
static void 
view_menu_toggles_changed (GtkWidget *widget, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();
  int active = 0;
  MenuEntry *gnomemeeting_menu = gnomemeeting_get_menu (gm);

  /* Only do something when a new CHECK_MENU_ITEM becomes active,
     not when it becomes inactive */
  if (GTK_CHECK_MENU_ITEM (widget)->active) {

    for (int i = 0; i <= GM_MAIN_NOTEBOOK_HIDDEN; i++) 
      if (GTK_CHECK_MENU_ITEM (gnomemeeting_menu [i+CONTROL_PANEL_VIEW_MENU_INDICE].widget)->active) 
 	active = i;
  }

  gconf_client_set_int (client, 
			"/apps/gnomemeeting/view/control_panel_section", 
			active, 0);
}


/* DESCRIPTION  :  This callback is called when the user toggles the 
 *                 corresponding option in the View Menu. (it is a check menu)
 * BEHAVIOR     :  Updates the gconf cache.
 * PRE          :  data is the key.
 */
static void 
menu_toggle_changed (GtkWidget *widget, gpointer data)
{
  GConfClient *client = gconf_client_get_default ();

  gconf_client_set_bool (client,
			 (gchar *) data,
			 GTK_CHECK_MENU_ITEM (widget)->active, NULL);
}


/* The functions */
void 
gnomemeeting_build_menu (GtkWidget *menubar, MenuEntry *gnomemeeting_menu,
			 GtkAccelGroup *accel)
{
  GtkWidget *menu = menubar;
  GtkWidget *old_menu = NULL;
  GSList *group = NULL;
  GtkWidget *image = NULL;
  int i = 0;

  while (gnomemeeting_menu [i].type != MENU_END) {

    GSList *new_group = NULL;

    if (gnomemeeting_menu [i].type != MENU_ENTRY_RADIO) 
      group = NULL;

    if (gnomemeeting_menu [i].name) {

      if (gnomemeeting_menu [i].type == MENU_ENTRY 
	  || gnomemeeting_menu [i].type == MENU_SUBMENU_NEW
	  || gnomemeeting_menu [i].type == MENU_NEW)
	gnomemeeting_menu [i].widget = 
	  gtk_image_menu_item_new_with_mnemonic (gnomemeeting_menu [i].name);
      else if (gnomemeeting_menu [i].type == MENU_ENTRY_TOGGLE)
	gnomemeeting_menu [i].widget = 
	  gtk_check_menu_item_new_with_mnemonic (gnomemeeting_menu [i].name);
      else if (gnomemeeting_menu [i].type == MENU_ENTRY_RADIO) {

	if (group == NULL)
	  group = new_group;

	gnomemeeting_menu [i].widget = 
	  gtk_radio_menu_item_new_with_mnemonic (group, 
						 gnomemeeting_menu [i].name);
	group = 
	  gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (gnomemeeting_menu [i].widget));
      }

      if (gnomemeeting_menu [i].stock_id) {

	image = gtk_image_new_from_stock (gnomemeeting_menu [i].stock_id,
					  GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (gnomemeeting_menu [i].widget), image);

	gtk_widget_show (GTK_WIDGET (image));
      }

      if (gnomemeeting_menu [i].accel)
	gtk_widget_add_accelerator (gnomemeeting_menu [i].widget, "activate", 
				    accel, gnomemeeting_menu [i].accel, 
				    GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

      if (gnomemeeting_menu [i].func) 
	g_signal_connect (G_OBJECT (gnomemeeting_menu [i].widget),
			  "activate", gnomemeeting_menu [i].func,
			  gnomemeeting_menu [i].data);

      g_signal_connect (G_OBJECT (gnomemeeting_menu [i].widget),
			"select", GTK_SIGNAL_FUNC (menu_item_selected), 
			(gpointer) gnomemeeting_menu [i].tooltip);
      g_signal_connect (G_OBJECT (gnomemeeting_menu [i].widget),
			"deselect", GTK_SIGNAL_FUNC (menu_item_selected), 
			NULL);
    }

    if (gnomemeeting_menu [i].type == MENU_SEP) {

      gnomemeeting_menu [i].widget = 
	gtk_separator_menu_item_new ();      

      if (old_menu) {

	menu = old_menu;
	old_menu = NULL;
      }
    }    

    if (gnomemeeting_menu [i].type == MENU_NEW
	|| gnomemeeting_menu [i].type == MENU_SUBMENU_NEW) {
	
      if (gnomemeeting_menu [i].type == MENU_SUBMENU_NEW) 
	old_menu = menu;
      menu = gtk_menu_new ();
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (gnomemeeting_menu [i].widget),
				 menu);

      if (gnomemeeting_menu [i].type == MENU_NEW)
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), 
			       gnomemeeting_menu [i].widget);
      else
	gtk_menu_shell_append (GTK_MENU_SHELL (old_menu), 
			       gnomemeeting_menu [i].widget);
    }
    else
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), 
			     gnomemeeting_menu [i].widget);      

    gtk_widget_show (gnomemeeting_menu [i].widget);

    i++;
  }
}


GtkWidget *
gnomemeeting_init_menu (GtkAccelGroup *accel)
{
  /* Get the data */
  GmWindow *gw = gnomemeeting_get_main_window (gm);
  GConfClient *client = gconf_client_get_default ();
  GtkWidget *menubar = NULL;

  menubar = gtk_menu_bar_new ();
  
  static MenuEntry gnomemeeting_menu [] =
    {
      {_("C_all"), NULL, NULL, 0, MENU_NEW, NULL, NULL, NULL},

      {_("_Connect"), _("Create a new connection"), 
       GM_STOCK_CONNECT, 'c', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (connect_cb),
       gw, NULL},

      {_("_Disconnect"), _("Close the current connection"), 
       GM_STOCK_DISCONNECT, 'd', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (disconnect_cb),
       gw, NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},

      {_("Do _Not Disturb"), _("Do not disturb"),
       NULL, 'n', MENU_ENTRY_TOGGLE, 
       GTK_SIGNAL_FUNC (menu_toggle_changed),
       (gpointer) "/apps/gnomemeeting/general/do_not_disturb", NULL},

      {_("Aut_o Answer"), _("Auto answer incoming call"),
       NULL, 'o', MENU_ENTRY_TOGGLE, 
       GTK_SIGNAL_FUNC (menu_toggle_changed),
       (gpointer) "/apps/gnomemeeting/general/auto_answer", NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},

      {_("_Audio Mute"), _("Mute the audio transmission"),
       NULL, 0, MENU_ENTRY, 
       GTK_SIGNAL_FUNC (pause_audio_callback),
       (gpointer) gw, NULL},

      {_("_Video Mute"), _("Mute the video transmission"),
       NULL, 0, MENU_ENTRY, 
       GTK_SIGNAL_FUNC (pause_video_callback),
       (gpointer) gw, NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},
      
      {_("_Save Current Picture"), _("Save a snapshot of the current video"), GTK_STOCK_SAVE, 
       'S', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (save_callback), NULL, NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},

      {_("_Quit"), _("Quit GnomeMeeting"), GTK_STOCK_QUIT, 'Q', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (quit_callback), (gpointer) gw, NULL},

      {_("_Edit"), NULL, NULL, 0, MENU_NEW, NULL, NULL, NULL},

      {_("Configuration Druid"), _("Rerun the configuration druid"),
       NULL, 0, MENU_ENTRY, 
       GTK_SIGNAL_FUNC (gnomemeeting_component_view),
       (gpointer) gw->druid_window, NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},

      {_("_Preferences..."), _("Change your preferences"), 
       GTK_STOCK_PREFERENCES, 0, MENU_ENTRY, 
       GTK_SIGNAL_FUNC (gnomemeeting_component_view),
       (gpointer) gw->pref_window, NULL},

      {_("_View"), NULL, NULL, 0, MENU_NEW, NULL, NULL, NULL},

      {_("Text Chat"), _("View/Hide the text chat window"), 
       NULL, 0, MENU_ENTRY_TOGGLE, 
       GTK_SIGNAL_FUNC (menu_toggle_changed),
       (gpointer) "/apps/gnomemeeting/view/show_chat_window", NULL},

      {_("Status Bar"), _("View/Hide the status bar"), 
       NULL, 0, MENU_ENTRY_TOGGLE, 
       GTK_SIGNAL_FUNC (menu_toggle_changed),
       (gpointer) "/apps/gnomemeeting/view/show_status_bar", NULL},

      {_("Control Panel"), NULL, NULL, 0, MENU_SUBMENU_NEW, NULL, NULL, NULL},

      {_("Statistics"), 
       _("View audio/video transmission and reception statistics"),
       NULL, 0, MENU_ENTRY_RADIO, 
       GTK_SIGNAL_FUNC (view_menu_toggles_changed), 
       (gpointer) "/apps/gnomemeeting/view/control_panel_section",
       NULL},

      {_("_Dialpad"), _("View the dialpad"),
       NULL, 0, MENU_ENTRY_RADIO, 
       GTK_SIGNAL_FUNC (view_menu_toggles_changed), 
       (gpointer) "/apps/gnomemeeting/view/control_panel_section",
       NULL},

      {_("_Audio Settings"), _("View audio settings"),
       NULL, 0, MENU_ENTRY_RADIO, 
       GTK_SIGNAL_FUNC (view_menu_toggles_changed), 
       (gpointer) "/apps/gnomemeeting/view/control_panel_section",
       NULL},

      {_("_Video Settings"), _("View video settings"),
       NULL, 0, MENU_ENTRY_RADIO, 
       GTK_SIGNAL_FUNC (view_menu_toggles_changed), 
       (gpointer) "/apps/gnomemeeting/view/control_panel_section",
       NULL},

      {_("Off"), _("Hide the control panel"),
       NULL, 0, MENU_ENTRY_RADIO, 
       GTK_SIGNAL_FUNC (view_menu_toggles_changed), 
       (gpointer) "/apps/gnomemeeting/view/control_panel_section",
       NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},

      {_("Local Video"), _("Local video image"),
       NULL, 0, MENU_ENTRY_RADIO, 
       GTK_SIGNAL_FUNC (video_view_changed_callback), 
       (gpointer) "view",
       NULL},

      {_("Remote Video"), _("Remote video image"),
       NULL, 0, MENU_ENTRY_RADIO, 
       GTK_SIGNAL_FUNC (video_view_changed_callback), 
       (gpointer) "view",
       NULL},

      {_("Both (Local Video Incrusted)"), _("Both video images"),
       NULL, 0, MENU_ENTRY_RADIO, 
       GTK_SIGNAL_FUNC (video_view_changed_callback), 
       (gpointer) "view",
       NULL},

      {_("Both (Local Video in New Window)"), _("Both video images"),
       NULL, 0, MENU_ENTRY_RADIO, 
       GTK_SIGNAL_FUNC (video_view_changed_callback), 
       (gpointer) "view",
       NULL},

      {_("Both (Both in New Windows)"), 
       _("Both video images"),
       NULL, 0, MENU_ENTRY_RADIO, 
       GTK_SIGNAL_FUNC (video_view_changed_callback), 
       (gpointer) "view",
       NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},

      {_("Zoom In"), _("Zoom in"), 
       GTK_STOCK_ZOOM_IN, '+', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (zoom_changed_callback),
       GINT_TO_POINTER (2), NULL},

      {_("Zoom Out"), _("Zoom out"), 
       GTK_STOCK_ZOOM_OUT, '-', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (zoom_changed_callback),
       GINT_TO_POINTER (0), NULL},

      {_("Normal Size"), _("Normal size"), 
       GTK_STOCK_ZOOM_100, '=', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (zoom_changed_callback),
       GINT_TO_POINTER (1), NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},

      {_("Fullscreen"), _("Switch to fullscreen"), 
       GTK_STOCK_ZOOM_IN, 'f', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (fullscreen_changed_callback),
       NULL, NULL},

      {_("_Tools"), NULL, NULL, 0, MENU_NEW, NULL, NULL, NULL},

      {_("Address _Book"), _("Open the address book"),
       NULL, 0, MENU_ENTRY, 
       GTK_SIGNAL_FUNC (gnomemeeting_component_view),
       (gpointer) gw->ldap_window, NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},

      {_("Generic History"), _("View the operations history"),
       NULL, 0, MENU_ENTRY, 
       GTK_SIGNAL_FUNC (gnomemeeting_component_view),
       (gpointer) gw->history_window, NULL},

      {_("Calls History"), _("View the calls history"),
       NULL, 0, MENU_ENTRY, 
       GTK_SIGNAL_FUNC (gnomemeeting_component_view),
       (gpointer) gw->calls_history_window, NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},

      {_("Consult MicroTelco Account"), _("View the details of your account"),
       NULL, 0, MENU_ENTRY, 
       GTK_SIGNAL_FUNC (microtelco_consult_cb),
       NULL, NULL},
      
      {_("_Help"), NULL, NULL, 0, MENU_NEW, NULL, NULL, NULL},

#ifndef DISABLE_GNOME
      {_("_About GnomeMeeting"), _("View information about GnomeMeeting"),
       GNOME_STOCK_ABOUT, 'a', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (about_callback),
       (gpointer) gm, NULL},
#else
      {_("_About GnomeMeeting"), _("View information about GnomeMeeting"),
       NULL, 'a', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (about_callback),
       (gpointer) gm, NULL},
#endif

      {NULL, NULL, NULL, 0, MENU_END, NULL, NULL, NULL}
    };

  gnomemeeting_build_menu (menubar, gnomemeeting_menu, accel);
  gtk_widget_show_all (GTK_WIDGET (menubar));
  
  g_object_set_data(G_OBJECT(gm), "gnomemeeting_menu", 
		    gnomemeeting_menu);

  /* Update to the initial values */
  GTK_CHECK_MENU_ITEM (gnomemeeting_menu [CHAT_WINDOW_VIEW_MENU_INDICE].widget)->active = 
    gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_chat_window",
			   0);
  GTK_CHECK_MENU_ITEM (gnomemeeting_menu [STATUS_BAR_VIEW_MENU_INDICE].widget)->active = 
    gconf_client_get_bool (client, "/apps/gnomemeeting/view/show_status_bar", 
			   0);
  
  for (int i = 0 ; i <= GM_MAIN_NOTEBOOK_HIDDEN ; i++) {
    
    if (gconf_client_get_int (client, "/apps/gnomemeeting/view/control_panel_section", 0) == i) 
      GTK_CHECK_MENU_ITEM (gnomemeeting_menu [CONTROL_PANEL_VIEW_MENU_INDICE+i].widget)->active = TRUE;
    else
      GTK_CHECK_MENU_ITEM (gnomemeeting_menu [CONTROL_PANEL_VIEW_MENU_INDICE+i].widget)->active = FALSE;

    gtk_widget_queue_draw (GTK_WIDGET (gnomemeeting_menu [CONTROL_PANEL_VIEW_MENU_INDICE+i].widget));
  }
  
  GTK_CHECK_MENU_ITEM (gnomemeeting_menu [DND_CALL_MENU_INDICE].widget)->active =
    gconf_client_get_bool (client, "/apps/gnomemeeting/general/do_not_disturb", 0);
  GTK_CHECK_MENU_ITEM (gnomemeeting_menu [AA_CALL_MENU_INDICE].widget)->active =
    gconf_client_get_bool (client, "/apps/gnomemeeting/general/auto_answer", 
  		   0);
  
  /* Disable disconnect */
  gnomemeeting_call_menu_connect_set_sensitive (1, FALSE);

  /* Pause is unsensitive when not in a call */
  gnomemeeting_call_menu_pause_set_sensitive (FALSE);

#ifdef DISABLE_GNOME
  gtk_widget_set_sensitive (GTK_WIDGET (gnomemeeting_menu [DRUID_EDIT_MENU_INDICE].widget), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (gnomemeeting_menu [ABOUT_HELP_MENU_INDICE].widget), FALSE);
#endif

#ifndef DISABLE_GNOME
  if (!gconf_client_get_bool (client, SERVICES_KEY "enable_microtelco", 0)) {

    gtk_widget_hide (GTK_WIDGET (gnomemeeting_menu [MICROTELCO1_TOOLS_MENU_INDICE].widget));
    gtk_widget_hide (GTK_WIDGET (gnomemeeting_menu [MICROTELCO2_TOOLS_MENU_INDICE].widget));
  }
#endif

  return menubar;
}


void 
gnomemeeting_zoom_submenu_set_sensitive (gboolean b)
{
  MenuEntry *gnomemeeting_menu = gnomemeeting_get_menu (gm);
  MenuEntry *video_menu = gnomemeeting_get_video_menu (gm);

  for (int i = 0 ; i < 3 ; i++) {

    gtk_widget_set_sensitive (GTK_WIDGET (gnomemeeting_menu [i+ZOOM_VIEW_MENU_INDICE].widget), b);
    gtk_widget_set_sensitive (GTK_WIDGET (video_menu [i+6].widget), b);
  }
}


void 
gnomemeeting_fullscreen_option_set_sensitive (gboolean b)
{
  MenuEntry *gnomemeeting_menu = gnomemeeting_get_menu (gm);
  MenuEntry *video_menu = gnomemeeting_get_video_menu (gm);


  if (b == FALSE) {

    gtk_widget_set_sensitive (GTK_WIDGET (gnomemeeting_menu [FULLSCREEN_VIEW_MENU_INDICE].widget), b);
    gtk_widget_set_sensitive (GTK_WIDGET (video_menu [10].widget), b);
  }
#ifdef HAS_SDL
  else {

    gtk_widget_set_sensitive (GTK_WIDGET (gnomemeeting_menu [FULLSCREEN_VIEW_MENU_INDICE].widget), b);
    gtk_widget_set_sensitive (GTK_WIDGET (video_menu [10].widget), b);
  }
#endif
}


void 
gnomemeeting_video_submenu_set_sensitive (gboolean b, int j, gboolean both)
{
  int cpt = j;
  int limit = BOTH;

  MenuEntry *gnomemeeting_menu = gnomemeeting_get_menu (gm);
  MenuEntry *video_menu = gnomemeeting_get_video_menu (gm);

  if (both)
    limit = BOTH;
  else
    limit = j;

  while (cpt <= limit) {

    gtk_widget_set_sensitive (GTK_WIDGET (gnomemeeting_menu [cpt+VIDEO_VIEW_MENU_INDICE].widget), 
			      b);
    gtk_widget_set_sensitive (GTK_WIDGET (video_menu [cpt].widget), 
			      b);
    cpt++;
  }
}


void 
gnomemeeting_video_submenu_select (int j)
{
  int view_number = 4;

  MenuEntry *gnomemeeting_menu = gnomemeeting_get_menu (gm);
  MenuEntry *video_menu = gnomemeeting_get_video_menu (gm);

  for (int i = 0 ; i <= view_number ; i++) {

    GTK_CHECK_MENU_ITEM (video_menu [i].widget)->active = (i == j);
    gtk_widget_queue_draw (GTK_WIDGET (video_menu [i].widget)); 

    GTK_CHECK_MENU_ITEM (gnomemeeting_menu [i+VIDEO_VIEW_MENU_INDICE].widget)->active 
      = (i == j);
    gtk_widget_queue_draw (GTK_WIDGET (gnomemeeting_menu [i+VIDEO_VIEW_MENU_INDICE].widget)); 
  }

}


void 
gnomemeeting_popup_menu_init (GtkWidget *widget, GtkAccelGroup *accel)
{
  GtkWidget *popup_menu_widget = NULL;
  popup_menu_widget = gtk_menu_new ();

  static MenuEntry video_menu [] =
    {
      {_("Local Video"), _("Local Video Image"),
       NULL, 0, MENU_ENTRY_RADIO, 
       GTK_SIGNAL_FUNC (video_view_changed_callback), 
       NULL, NULL},

      {_("Remote Video"), _("Remote Video Image"),
       NULL, 0, MENU_ENTRY_RADIO, 
       GTK_SIGNAL_FUNC (video_view_changed_callback), 
       NULL, NULL},

      {_("Both (Local Video Incrusted)"), _("Both Video Images"),
       NULL, 0, MENU_ENTRY_RADIO, 
       GTK_SIGNAL_FUNC (video_view_changed_callback), 
       NULL, NULL},

      {_("Both (Local Video in New Window)"), _("Both Video Images"),
       NULL, 0, MENU_ENTRY_RADIO, 
       GTK_SIGNAL_FUNC (video_view_changed_callback), 
       NULL, NULL},

      {_("Both (Local and Remote Video in New Windows)"), 
       _("Both Video Images"),
       NULL, 0, MENU_ENTRY_RADIO, 
       GTK_SIGNAL_FUNC (video_view_changed_callback), 
       NULL, NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},

      {_("Zoom In"), _("Zoom In"), 
       GTK_STOCK_ZOOM_IN, '+', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (zoom_changed_callback),
       GINT_TO_POINTER (2), NULL},

      {_("Zoom Out"), _("Zoom Out"), 
       GTK_STOCK_ZOOM_OUT, '-', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (zoom_changed_callback),
       GINT_TO_POINTER (0), NULL},

      {_("Normal Size"), _("Normal Size"), 
       GTK_STOCK_ZOOM_100, '=', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (zoom_changed_callback),
       GINT_TO_POINTER (1), NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},

      {_("Fullscreen"), _("Switch to fullscreen"), 
       GTK_STOCK_ZOOM_IN, 'f', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (fullscreen_changed_callback),
       NULL, NULL},

      {NULL, NULL, NULL, 0, MENU_END, NULL, NULL, NULL}
    };

  gnomemeeting_build_menu (popup_menu_widget, video_menu, accel);
  gtk_widget_show_all (popup_menu_widget);

  g_object_set_data(G_OBJECT(gm), "video_menu", 
		    video_menu);

  g_signal_connect (G_OBJECT (widget), "button_press_event",
		    G_CALLBACK (popup_menu_callback), 
		    (gpointer) popup_menu_widget);

  gtk_widget_add_events (gm, GDK_BUTTON_PRESS_MASK |
			 GDK_KEY_PRESS_MASK);
}

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a video menu which will popup, and attach it
 *                 to the given widget.
 * PRE          :  The widget to attach the menu to, and the accelgroup.
 */
void 
gnomemeeting_popup_menu_tray_init (GtkWidget *widget, GtkAccelGroup *accel)
{
  GtkWidget *popup_menu_widget = NULL;
  GConfClient *client = gconf_client_get_default ();
  GmWindow *gw = gnomemeeting_get_main_window (gm);

  popup_menu_widget = gtk_menu_new ();

  static MenuEntry tray_menu [] =
    {
      {_("_Connect"), _("Create a new connection"), 
       GM_STOCK_CONNECT, 'c', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (connect_cb),
       gw, NULL},

      {_("_Disconnect"), _("Close the current connection"), 
       GM_STOCK_DISCONNECT, 'd', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (disconnect_cb),
       gw, NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},

      {_("Do _Not Disturb"), _("Do not disturb (reject incoming calls)"),
       NULL, 'n', MENU_ENTRY_TOGGLE, 
       GTK_SIGNAL_FUNC (menu_toggle_changed),
       (gpointer) GENERAL_KEY "do_not_disturb", NULL},

      {_("Aut_o Answer"), _("Automatically answer calls"),
       NULL, 'o', MENU_ENTRY_TOGGLE, 
       GTK_SIGNAL_FUNC (menu_toggle_changed),
       (gpointer) GENERAL_KEY "auto_answer", NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},

      {_("_Preferences..."), _("Change your preferences"),
       GTK_STOCK_PREFERENCES, 'P', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (gnomemeeting_component_view),
       (gpointer) gw->pref_window, NULL},

      {_("Address _Book"), _("Open the address book"),
       NULL, 0, MENU_ENTRY, 
       GTK_SIGNAL_FUNC (gnomemeeting_component_view),
       (gpointer) gw->ldap_window, NULL},

      {NULL, NULL, NULL, 0, MENU_SEP, NULL, NULL, NULL},

#ifndef DISABLE_GNOME
      {_("_About GnomeMeeting"), _("View information about GnomeMeeting"),
       GNOME_STOCK_ABOUT, 'a', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (about_callback),
       (gpointer) gm, NULL},
#else
      {_("_About GnomeMeeting"), _("View information about GnomeMeeting"),
       NULL, 'a', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (about_callback),
       (gpointer) gm, NULL},
#endif

      {_("_Quit"), _("Quit GnomeMeeting"),
       GTK_STOCK_QUIT, 'Q', MENU_ENTRY, 
       GTK_SIGNAL_FUNC (quit_callback),
       (gpointer) gw, NULL},

      {NULL, NULL, NULL, 0, MENU_END, NULL, NULL, NULL}
    };

  gnomemeeting_build_menu (popup_menu_widget, tray_menu, accel);
  gtk_widget_show_all (popup_menu_widget);

  g_object_set_data (G_OBJECT (gm), "tray_menu", 
		     tray_menu);

  g_signal_connect (G_OBJECT (widget), "button_press_event",
		    G_CALLBACK (popup_menu_callback), 
		    (gpointer) popup_menu_widget);

  gtk_widget_add_events (gm, GDK_BUTTON_PRESS_MASK |
			 GDK_KEY_PRESS_MASK);

#ifdef DISABLE_GNOME
  gtk_widget_set_sensitive (GTK_WIDGET (gnomemeeting_menu [9].widget), FALSE);
#endif

  /* Update the menu according to the gconf values */
  GTK_CHECK_MENU_ITEM (tray_menu [3].widget)->active =
    gconf_client_get_bool (client, "/apps/gnomemeeting/general/do_not_disturb", 0);
  GTK_CHECK_MENU_ITEM (tray_menu [4].widget)->active =
    gconf_client_get_bool (client, "/apps/gnomemeeting/general/auto_answer", 
  		   0);
}


void
gnomemeeting_call_menu_connect_set_sensitive (int i, bool b)
{
  MenuEntry *gnomemeeting_menu = gnomemeeting_get_menu (gm);
  MenuEntry *tray_menu = gnomemeeting_get_tray_menu (gm);

  gtk_widget_set_sensitive (GTK_WIDGET (gnomemeeting_menu [CONNECT_CALL_MENU_INDICE+i].widget), b);
  gtk_widget_set_sensitive (GTK_WIDGET (tray_menu [i].widget), b);
}


void
gnomemeeting_call_menu_pause_set_sensitive (bool b)
{
  MenuEntry *gnomemeeting_menu = gnomemeeting_get_menu (gm);

  gtk_widget_set_sensitive (GTK_WIDGET (gnomemeeting_menu [AUDIO_PAUSE_CALL_MENU_INDICE].widget), b);
  gtk_widget_set_sensitive (GTK_WIDGET (gnomemeeting_menu [VIDEO_PAUSE_CALL_MENU_INDICE].widget), b);
}


MenuEntry *
gnomemeeting_get_menu (GtkWidget *widget)
{
  MenuEntry *m =
    (MenuEntry *) g_object_get_data (G_OBJECT (widget), "gnomemeeting_menu");

  return m;
}


MenuEntry *
gnomemeeting_get_video_menu (GtkWidget *widget)
{
  MenuEntry *m =
    (MenuEntry *) g_object_get_data (G_OBJECT (widget), "video_menu");

  return m;
}


MenuEntry *
gnomemeeting_get_tray_menu (GtkWidget *widget)
{
  MenuEntry *m =
    (MenuEntry *) g_object_get_data (G_OBJECT (widget), "tray_menu");

  return m;
}


