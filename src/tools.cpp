
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
 *                         tools.cpp  -  description
 *                         -------------------------
 *   begin                : Sun Sep 1 2002
 *   copyright            : (C) 2000-2004 by Damien Sandras 
 *   description          : This file contains functions to build the simple
 *                          tools of the tools menu.
 *
 */


#include "../config.h"

#include "tools.h"
#include "gnomemeeting.h"
#include "callbacks.h"
#include "misc.h"

#include "gm_conf.h"
#include "gnome_prefs_window.h"


#ifndef DISABLE_GNOME
static void pc2phone_window_response_cb (GtkWidget *, gint, gpointer);

static void pc2phone_consult_cb (GtkWidget *, gpointer);
#endif


/* DESCRIPTION  :  This callback is called when the user validates an answer
 *                 to the PC-To-Phone window.
 * BEHAVIOR     :  Hide the window (if not Apply), and apply the settings
 *                 (if not cancel), ie change the settings and register to gk.
 * PRE          :  /
 */
#ifndef DISABLE_GNOME
static void
pc2phone_window_response_cb (GtkWidget *w,
			     gint response,
			     gpointer data)
{
  GMH323EndPoint *ep = NULL;

  ep = GnomeMeeting::Process ()->Endpoint ();
  
  if (response != 1)
    gnomemeeting_window_hide (w);
  

  if (response == 1 || response == 0) {

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data))) {
	
      /* The Username and PIN already are correct, update the other settings */
      gm_conf_set_bool (H323_ADVANCED_KEY "enable_fast_start", TRUE);
      gm_conf_set_bool (H323_ADVANCED_KEY "enable_h245_tunneling", TRUE);
      gm_conf_set_bool (H323_ADVANCED_KEY "enable_early_h245", TRUE);
      gm_conf_set_string (H323_GATEKEEPER_KEY "host", "gk.ast.diamondcard.us");
      gm_conf_set_int (H323_GATEKEEPER_KEY "registering_method", 1);
    }
    else
      gm_conf_set_int (H323_GATEKEEPER_KEY "registering_method", 0);
    
    /* Register the current Endpoint to the Gatekeeper */
    ep->GatekeeperRegister ();
  }
}
#endif


/* DESCRIPTION  :  This callback is called when the user clicks on the link
 *                 button to consult his account details.
 * BEHAVIOR     :  Builds a filename with autopost html in /tmp/ and opens it
 *                 with the GNOME preferred browser.
 * PRE          :  /
 */
#ifndef DISABLE_GNOME
static void
pc2phone_consult_cb (GtkWidget *widget,
		       gpointer data)
{
  gchar *tmp_filename = NULL;
  gchar *filename = NULL;
  gchar *account = NULL;
  gchar *pin = NULL;
  gchar *buffer = NULL;
  
  int fd = -1;

  account = gm_conf_get_string (H323_GATEKEEPER_KEY "alias");
  pin = gm_conf_get_string (H323_GATEKEEPER_KEY "password");

  if (account == NULL || pin == NULL)
    return; /* no account configured yet */
  
  buffer =
    g_strdup_printf ("<HTML><HEAD><TITLE>MicroTelco Auto-Post</TITLE></HEAD>"
		     "<BODY BGCOLOR=\"#FFFFFF\" "
		     "onLoad=\"Javascript:document.autoform.submit()\">"
		     "<FORM NAME=\"autoform\" "
		     "ACTION=\"https://%s.an.pc2phone.com/acct/Controller\" "
		     "METHOD=\"POST\">"
		     "<input type=\"hidden\" name=\"command\" value=\"caller_login\">"
		     "<input type=\"hidden\" name=\"caller_id\" value=\"%s\">"
		     "<input type=\"hidden\" name=\"caller_pin\" value=\"%s\">"
		     "</FORM></BODY></HTML>", account, account, pin);

  fd = g_file_open_tmp ("mktmicro-XXXXXX", &tmp_filename, NULL);
  filename = g_strdup_printf ("file:///%s", tmp_filename);
  
  write (fd, (char *) buffer, strlen (buffer)); 
  close (fd);
  
  gnome_url_show (filename, NULL);

  g_free (tmp_filename);
  g_free (filename);
  g_free (buffer);
  g_free (account);
  g_free (pin);
}
#endif


GtkWidget *
gm_pc2phone_window_new ()
{
  GtkWidget *window = NULL;
#ifndef DISABLE_GNOME
  GtkWidget *button = NULL;
  GtkWidget *label = NULL;
  GtkWidget *use_service_button = NULL;
  GtkWidget *entry = NULL;
  GtkWidget *href = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *subsection = NULL;

  gchar *txt = NULL;
  
  window = gtk_dialog_new ();
  gtk_dialog_add_buttons (GTK_DIALOG (window),
			  GTK_STOCK_APPLY,  1,
			  GTK_STOCK_CANCEL, 2,
			  GTK_STOCK_OK, 0,
			  NULL);
  
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("pc_to_phone_window"), g_free);
  
  gtk_window_set_title (GTK_WINDOW (window), _("PC-To-Phone Settings"));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

  label = gtk_label_new (_("You can make calls to regular phones and cell numbers worldwide using GnomeMeeting. To enable this you need to register an account using the URL below, then enter your Account number and PIN, and finally enable registering to the GnomeMeeting PC-To-Phone service.\n\nPlease make sure you are using the URL below to get your account or the service will not work."));
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), label,
		      FALSE, FALSE, 20);


  vbox = GTK_DIALOG (window)->vbox;
  
  subsection =
    gnome_prefs_subsection_new (window, vbox,
				_("PC-To-Phone Settings"), 2, 1);

  gnome_prefs_entry_new (subsection, _("Account _number:"), H323_GATEKEEPER_KEY "alias", _("Use your MicroTelco account number"), 1, false);

  entry =
    gnome_prefs_entry_new (subsection, _("_Pin:"), H323_GATEKEEPER_KEY "password", _("Use your MicroTelco PIN"), 2, false);
  gtk_entry_set_visibility (GTK_ENTRY (entry), FALSE);

  use_service_button =
    gtk_check_button_new_with_label (_("Use PC-To-Phone service"));
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (use_service_button),
		      FALSE, TRUE, 0);
  
  label =
    gtk_label_new (_("Click on one of the following links to get more information about your existing GnomeMeeting PC-To-Phone account, or to create a new account."));
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);

  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (label), FALSE, FALSE, 20);
  href = gnome_href_new ("http://www.diamondcard.us/gnomemeeting", _("Get a GnomeMeeting PC-To-Phone account"));
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (href), FALSE, FALSE, 0);
  button = gtk_button_new ();
  label = gtk_label_new (NULL);
  txt = g_strdup_printf ("<span foreground=\"blue\"><u>%s</u></span>",
			 _("Consult my account details"));
  gtk_label_set_markup (GTK_LABEL (label), txt);
  g_free (txt);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (button), label);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (button), FALSE, FALSE, 0);
  g_signal_connect (GTK_OBJECT (button), "clicked",
		    G_CALLBACK (pc2phone_consult_cb), NULL);
				
  g_signal_connect (GTK_OBJECT (window), 
		    "response", 
		    G_CALLBACK (pc2phone_window_response_cb),
		    (gpointer) use_service_button);

  g_signal_connect (GTK_OBJECT (window), "delete-event", 
                    G_CALLBACK (delete_window_cb), NULL);
  
  gtk_widget_show_all (GTK_WIDGET (GTK_DIALOG (window)->vbox));
#endif
  
  return window;
}
