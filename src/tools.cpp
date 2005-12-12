
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
#include "h323endpoint.h"
#include "gnomemeeting.h"
#include "callbacks.h"
#include "misc.h"

#include <gm_conf.h>
#include <gnome_prefs_window.h>
#include <dialog.h>

#ifdef WIN32
#include "winpaths.h"
#endif

typedef struct _GmPC2PhoneWindow
{
  GtkWidget *username_entry;
  GtkWidget *password_entry;
  GtkWidget *use_service_toggle;
} GmPC2PhoneWindow;

#define GM_PC2PHONE_WINDOW(x) (GmPC2PhoneWindow *) (x)


/* Declarations */

/* GUI Functions */


/* DESCRIPTION  : /
 * BEHAVIOR     : Frees a GmPC2PhoneWindow and its content.
 * PRE          : A non-NULL pointer to a GmPC2PhoneWindow structure.
 */
static void gm_pcw_destroy (gpointer);


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns a pointer to the private GmPC2PhoneWindow structure
 *                used by the pc2phone window GMObject.
 * PRE          : The given GtkWidget pointer must be a pc2phone window 
 * 		  GMObject.
 */
static GmPC2PhoneWindow *gm_pcw_get_pcw (GtkWidget *);


/* Callbacks */

/* DESCRIPTION  :  This callback is called when the user validates an answer
 *                 to the PC-To-Phone window.
 * BEHAVIOR     :  Hide the window (if not Apply), and apply the settings
 *                 (if not cancel), ie change the settings and register to gk.
 * PRE          :  /
 */
static void pc2phone_window_response_cb (GtkWidget *, 
					 gint, 
					 gpointer);


/* DESCRIPTION  :  This callback is called when the user clicks on the link
 *                 button to consult his account details.
 * BEHAVIOR     :  Builds a filename with autopost html in /tmp/ and opens it
 *                 with the GNOME preferred browser.
 * PRE          :  GPOINTER_TO_INT (data) == 0 : recharge,
 * 				          == 1 : balance history,
 * 				          == 2 : calls history
 * 					  == 3 : get an account
 */
static void pc2phone_consult_cb (GtkWidget *, 
				 gpointer);


/* Implementation */
static void
gm_pcw_destroy (gpointer pcw)
{
  g_return_if_fail (pcw != NULL);

  delete ((GmPC2PhoneWindow *) pcw);
}


static GmPC2PhoneWindow *
gm_pcw_get_pcw (GtkWidget *pc2phone_window)
{
  g_return_val_if_fail (pc2phone_window != NULL, NULL);

  return GM_PC2PHONE_WINDOW (g_object_get_data (G_OBJECT (pc2phone_window), "GMObject"));
}


static void
pc2phone_window_response_cb (GtkWidget *w,
			     gint response,
			     gpointer data)
{
  GMEndPoint *ep = NULL;
  
  GmAccount *account = NULL;
  GmPC2PhoneWindow *pcw = NULL;

  const char *username = NULL;
  const char *password = NULL;

  gboolean new_account = FALSE;
  gboolean use_service = FALSE;
  
  g_return_if_fail (data != NULL);

  pcw = gm_pcw_get_pcw (GTK_WIDGET (data));
  
  g_return_if_fail (pcw != NULL);
  
  ep = GnomeMeeting::Process ()->Endpoint ();

  
  /* Get the data from the widgets */
  username = gtk_entry_get_text (GTK_ENTRY (pcw->username_entry));
  password = gtk_entry_get_text (GTK_ENTRY (pcw->password_entry));
  use_service = 
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pcw->use_service_toggle));

  /* If validate or apply, check all settings are present */
  if (response != 2 && use_service 
      && (!strcmp (username, "") || !strcmp (password, ""))) {
    
    gnomemeeting_error_dialog (GTK_WINDOW (data), _("Invalid parameters"), _("Please provide your username and password in order to be able to use the PC-To-Phone service."));
    return;
  }
  
  /* Let's go */
  account = gnomemeeting_get_account ("eugw.ast.diamondcard.us");
  if (account == NULL) {

    account = gm_account_new ();
    account->account_name = g_strdup ("GnomeMeeting PC-To-Phone");
    account->host = g_strdup ("eugw.ast.diamondcard.us");
    account->domain = g_strdup ("eugw.ast.diamondcard.us");
    account->protocol_name = g_strdup ("SIP");
  
    new_account = TRUE;
  }
  
  if (response != 1)
    gnomemeeting_window_hide (w);
  
  if (response == 1 || response == 0) {

    if (account->username)
      g_free (account->username);
    if (account->auth_username)
      g_free (account->auth_username);
    if (account->password)
      g_free (account->password);
    
    account->username = 
      g_strdup (gtk_entry_get_text (GTK_ENTRY (pcw->username_entry)));
    account->auth_username = 
      g_strdup (gtk_entry_get_text (GTK_ENTRY (pcw->username_entry)));
    account->password = 
      g_strdup (gtk_entry_get_text (GTK_ENTRY (pcw->password_entry)));
    account->enabled =
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pcw->use_service_toggle));

    /* Update the account or create it */
    if (new_account)
      gnomemeeting_account_add (account);
    else
      gnomemeeting_account_modify (account);
    
    /* Register the current Endpoint to the Gatekeeper */
    ep->Register (account);
  }

  gm_account_delete (account);
}


static void
pc2phone_consult_cb (GtkWidget *widget,
		     gpointer data)
{
  GmPC2PhoneWindow *pcw = NULL;

  GtkWidget *pc2phone_window = NULL;
  
  const char *account = NULL;
  const char *password = NULL;

  gchar *url = NULL;
#ifdef DISABLE_GNOME
  gchar *command = NULL;
#endif

  pc2phone_window = GnomeMeeting::Process ()->GetPC2PhoneWindow ();
  pcw = gm_pcw_get_pcw (pc2phone_window);

  account = gtk_entry_get_text (GTK_ENTRY (pcw->username_entry));
  password = gtk_entry_get_text (GTK_ENTRY (pcw->password_entry));


  if (account == NULL || password == NULL)
    return; /* no account configured yet */
  
  if (GPOINTER_TO_INT (data) == 3)
    url = g_strdup ("https://www.diamondcard.us/exec/voip-username?act=sgn&spo=gnomemeeting");
  else if (GPOINTER_TO_INT (data) == 0)
    url = g_strdup_printf ("https://www.diamondcard.us/exec/voip-username?accId=%s&passwordCode=%s&act=rch&spo=gnomemeeting", account, password);
  else if (GPOINTER_TO_INT (data) == 1)
    url = g_strdup_printf ("https://www.diamondcard.us/exec/voip-username?accId=%s&passwordCode=%s&act=bh&spo=gnomemeeting", account, password);
  else if (GPOINTER_TO_INT (data) == 2)
    url = g_strdup_printf ("https://www.diamondcard.us/exec/voip-username?accId=%s&passwordCode=%s&act=ch&spo=gnomemeeting", account, password);
    
#ifdef DISABLE_GNOME
  command = g_strdup_printf ("mozilla %s", url);
  g_spawn_command_line_async (command, NULL);
  g_free (command);
#else
  gnome_url_show (url, NULL);
#endif

  g_free (url);
}


GtkWidget *
gm_pc2phone_window_new ()
{
  GmAccount *account = NULL;

  GmPC2PhoneWindow *pcw = NULL;
  
  GtkWidget *window = NULL;
  GtkWidget *button = NULL;
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *subsection = NULL;

  GdkPixbuf *pixbuf = NULL;

  gchar *filename = NULL;
  gchar *txt = NULL;
  

  /* Get the PC-To-Phone account, if any */
  account = gnomemeeting_get_account ("eugw.ast.diamondcard.us");

  
  /* Build the window */
  window = gtk_dialog_new ();
  gtk_dialog_add_buttons (GTK_DIALOG (window),
			  GTK_STOCK_APPLY,  1,
			  GTK_STOCK_CANCEL, 2,
			  GTK_STOCK_OK, 0,
			  NULL);

  pcw = new GmPC2PhoneWindow ();
  g_object_set_data_full (G_OBJECT (window), "GMObject", 
			  pcw, (GDestroyNotify) gm_pcw_destroy);
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("pc_to_phone_window"), g_free);
  
  gtk_window_set_title (GTK_WINDOW (window), _("PC-To-Phone Settings"));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  filename = g_build_filename (DATA_DIR, "pixmaps", PACKAGE_NAME ".png", NULL);
  pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
  g_free (filename);

  if (pixbuf) {

    gtk_window_set_icon (GTK_WINDOW (window), pixbuf);
    g_object_unref (pixbuf);
  }

  /* Introduction label */
  label = gtk_label_new (_("You can make calls to regular phones and cell numbers worldwide using GnomeMeeting. To enable this, you need to register an account using the URL below, then enter your Account number and password, and finally enable registering to the GnomeMeeting PC-To-Phone service.\n\nPlease make sure you are using the URL below to get your account otherwise the service will not work."));
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), label,
		      FALSE, FALSE, 20);

  /* Settings */
  vbox = GTK_DIALOG (window)->vbox;
  subsection =
    gnome_prefs_subsection_new (window, vbox,
				_("PC-To-Phone Settings"), 3, 2);
  
  label = gtk_label_new_with_mnemonic (_("Account _number:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_table_attach (GTK_TABLE (subsection), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    0, 0);

  pcw->username_entry = gtk_entry_new ();
  if (account && account->username)
    gtk_entry_set_text (GTK_ENTRY (pcw->username_entry), account->username);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), pcw->username_entry);
  gtk_table_attach (GTK_TABLE (subsection), pcw->username_entry, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    0, 0);
  
  label = gtk_label_new_with_mnemonic (_("_password:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_table_attach (GTK_TABLE (subsection), label, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    0, 0);
  
  pcw->password_entry = gtk_entry_new ();
  gtk_entry_set_visibility (GTK_ENTRY (pcw->password_entry), FALSE);
  if (account && account->password)
    gtk_entry_set_text (GTK_ENTRY (pcw->password_entry), account->password);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), pcw->password_entry);
  gtk_table_attach (GTK_TABLE (subsection), pcw->password_entry, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    0, 0);

  pcw->use_service_toggle =
    gtk_check_button_new_with_label (_("Use PC-To-Phone service"));
  if (account && account->enabled)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pcw->use_service_toggle), 
				  TRUE);
  gtk_table_attach (GTK_TABLE (subsection), 
		    pcw->use_service_toggle, 0, 2, 2, 3,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    0, 0);

  /* Explanation label */
  label =
    gtk_label_new (_("Click on one of the following links to get more information about your existing GnomeMeeting PC-To-Phone account, or to create a new account."));
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (label), FALSE, FALSE, 20);

  /* Get an account, good idea */
  button = gtk_button_new ();
  label = gtk_label_new (NULL);
  txt = g_strdup_printf ("<span foreground=\"blue\"><u>%s</u></span>",
			 _("Get a GnomeMeeting PC-To-Phone account"));
  gtk_label_set_markup (GTK_LABEL (label), txt);
  g_free (txt);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (button), label);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (button), FALSE, FALSE, 0);
  g_signal_connect (GTK_OBJECT (button), "clicked",
		    G_CALLBACK (pc2phone_consult_cb), GINT_TO_POINTER (3));

  /* Recharge account */
  button = gtk_button_new ();
  label = gtk_label_new (NULL);
  txt = g_strdup_printf ("<span foreground=\"blue\"><u>%s</u></span>",
			 _("Recharge the account"));
  gtk_label_set_markup (GTK_LABEL (label), txt);
  g_free (txt);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (button), label);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (button), FALSE, FALSE, 0);
  g_signal_connect (GTK_OBJECT (button), "clicked",
		    G_CALLBACK (pc2phone_consult_cb), GINT_TO_POINTER (0));

  /* Consult the balance history */
  button = gtk_button_new ();
  label = gtk_label_new (NULL);
  txt = g_strdup_printf ("<span foreground=\"blue\"><u>%s</u></span>",
			 _("Consult the balance history"));
  gtk_label_set_markup (GTK_LABEL (label), txt);
  g_free (txt);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (button), label);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (button), FALSE, FALSE, 0);
  g_signal_connect (GTK_OBJECT (button), "clicked",
		    G_CALLBACK (pc2phone_consult_cb), GINT_TO_POINTER (1));

  /* Consult the calls history */
  button = gtk_button_new ();
  label = gtk_label_new (NULL);
  txt = g_strdup_printf ("<span foreground=\"blue\"><u>%s</u></span>",
			 _("Consult the calls history"));
  gtk_label_set_markup (GTK_LABEL (label), txt);
  g_free (txt);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (button), label);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (button), FALSE, FALSE, 0);
  g_signal_connect (GTK_OBJECT (button), "clicked",
		    G_CALLBACK (pc2phone_consult_cb), GINT_TO_POINTER (2));
				
  g_signal_connect (GTK_OBJECT (window), 
		    "response", 
		    G_CALLBACK (pc2phone_window_response_cb),
		    (gpointer) window);

  g_signal_connect (GTK_OBJECT (window), "delete-event", 
                    G_CALLBACK (delete_window_cb), NULL);
  
  gtk_widget_show_all (GTK_WIDGET (GTK_DIALOG (window)->vbox));

  if (account)
    gm_account_delete (account);
  
  return window;
}
