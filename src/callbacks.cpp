
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2003 Damien Sandras
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
 *                         callbacks.cpp  -  description
 *                         -----------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2003 by Damien Sandras
 *   description          : This file contains callbacks common to several
 *                          files.
 *
 */


#include "../config.h"

#include "ldap_window.h"
#include "callbacks.h"
#include "gnomemeeting.h"
#include "menu.h"
#include "misc.h"
#include "urlhandler.h"


/* Declarations */
extern GnomeMeeting *MyApp;	
extern GtkWidget *gm;


/* The callbacks */
/* DESCRIPTION† :† This callback is called when the user chooses to forward
 *†††††††††††††††† a call.
 * BEHAVIOR†††† :† Forward the current call.
 * PRE††††††††† :† /
 */
void
transfer_call_cb (GtkWidget* widget,
		  gpointer data)
{
  GtkWidget *transfer_call_popup = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *label = NULL;
  GtkWidget *entry = NULL;
  GtkWidget *b1 = NULL;
  GtkWidget *b2 = NULL;

  GMH323EndPoint *endpoint = NULL;
  GmWindow *gw = NULL;
  GMURL url;
  
  char *gconf_forward_value = NULL;
  gint answer = 0;
  
  GConfClient *client = NULL;

  client = gconf_client_get_default ();
  endpoint = MyApp->Endpoint ();
  gw = MyApp->GetMainWindow ();

  transfer_call_popup = gtk_dialog_new ();
  if (!data) {

    gtk_window_set_transient_for (GTK_WINDOW (transfer_call_popup),
				  GTK_WINDOW (gm));
    gconf_forward_value =
      gconf_client_get_string (GCONF_CLIENT (client),
			       CALL_FORWARDING_KEY "forward_host",
			       NULL);
  }
  else {
    
    gtk_window_set_transient_for (GTK_WINDOW (transfer_call_popup),
				  GTK_WINDOW (gw->ldap_window));
    gconf_forward_value = g_strdup ((gchar *) data);
  }
  

  
  b1 = gtk_dialog_add_button (GTK_DIALOG (transfer_call_popup),
			      GTK_STOCK_CANCEL, 0);
  b2 = gtk_dialog_add_button (GTK_DIALOG (transfer_call_popup),
			      _("_Transfer Call"), 1); 
  gtk_dialog_set_default_response (GTK_DIALOG (widget), 1);
  
  label = gtk_label_new (_("Forward call to:"));
  hbox = gtk_hbox_new (0, 0);
  
  gtk_box_pack_start (GTK_BOX 
		      (GTK_DIALOG (transfer_call_popup)->vbox), 
		      hbox, TRUE, TRUE, 0);
    
  entry = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (entry),
		      (strcmp (gconf_forward_value, "") ?
		       gconf_forward_value : 
		       (const char *) url.GetDefaultURL ()));
  g_free (gconf_forward_value);

  gconf_forward_value = NULL;

  gtk_box_pack_start (GTK_BOX (hbox), 
		      label, TRUE, TRUE, 20);
  gtk_box_pack_start (GTK_BOX (hbox), 
		      entry, TRUE, TRUE, 20);

  gtk_window_set_modal (GTK_WINDOW (transfer_call_popup), TRUE);

  gtk_widget_show_all (transfer_call_popup);

  answer = gtk_dialog_run (GTK_DIALOG (transfer_call_popup));
  switch (answer) {

  case 1:

    gconf_forward_value = (gchar *) gtk_entry_get_text (GTK_ENTRY (entry));
    new GMURLHandler (gconf_forward_value, TRUE);
      
    break;

  default:
    break;
  }

  gtk_widget_destroy (transfer_call_popup);
}


void save_callback (GtkWidget *widget, gpointer data)
{
  MyApp->Endpoint ()->SavePicture ();
}


void pause_channel_callback (GtkWidget *widget, gpointer data)
{
  MenuEntry *gnomemeeting_menu = NULL;
  GmWindow *gw = NULL;
  
  H323Connection *connection = NULL;
  H323Channel *channel = NULL;
  GMH323EndPoint *endpoint = NULL;
  PString current_call_token;

  GtkToggleButton *b = NULL;
  GtkWidget *child = NULL;
  
  gchar *menu_suspend_msg = NULL;
  gchar *menu_resume_msg = NULL;
  gchar *history_suspend_msg = NULL;
  gchar *history_resume_msg = NULL;
  
  endpoint = MyApp->Endpoint ();
  current_call_token = endpoint->GetCurrentCallToken ();

  gnomemeeting_menu = gnomemeeting_get_menu (gm);
  gw = MyApp->GetMainWindow ();
  
  if (!current_call_token.IsEmpty ())
    connection =
      endpoint->FindConnectionWithLock (current_call_token);


  if (connection) {

    if (GPOINTER_TO_INT (data) == 0)
      channel = 
	connection->FindChannel (RTP_Session::DefaultAudioSessionID, 
				 FALSE);
    else
      channel = 
	connection->FindChannel (RTP_Session::DefaultVideoSessionID, 
				 FALSE);

    if (channel) {

      if (GPOINTER_TO_INT (data) == 0) {

	menu_suspend_msg = g_strdup (_("Suspend _Audio"));
	menu_resume_msg = g_strdup (_("Resume _Audio"));
	history_suspend_msg = g_strdup (_("Audio transmission: suspended"));
	history_resume_msg = g_strdup (_("Audio transmission: resumed"));

	b = GTK_TOGGLE_BUTTON (gw->audio_chan_button);
	
	child =
	  GTK_BIN (gnomemeeting_menu [AUDIO_PAUSE_CALL_MENU_INDICE].widget)->child;
      }
      else {
	
	menu_suspend_msg = g_strdup (_("Suspend _Video"));
	menu_resume_msg = g_strdup (_("Resume _Video"));
	history_suspend_msg = g_strdup (_("Video transmission: suspended"));
	history_resume_msg = g_strdup (_("Video transmission: resumed"));

	b = GTK_TOGGLE_BUTTON (gw->video_chan_button);
	
	child =
	  GTK_BIN (gnomemeeting_menu [VIDEO_PAUSE_CALL_MENU_INDICE].widget)->child;
      }
    
      if (channel->IsPaused ()) {

	if (GTK_IS_LABEL (child)) 
	  gtk_label_set_text_with_mnemonic (GTK_LABEL (child),
					    menu_suspend_msg);

	gnomemeeting_log_insert (gw->history_text_view, history_suspend_msg);
	gnomemeeting_statusbar_flash (gw->statusbar, history_suspend_msg);

	g_signal_handlers_block_by_func (G_OBJECT (b),
					 (gpointer) pause_channel_callback,
					 GINT_TO_POINTER (0));
	gtk_toggle_button_set_active (b, FALSE);
	gtk_widget_queue_draw (GTK_WIDGET (b));
	g_signal_handlers_unblock_by_func (G_OBJECT (b),
					   (gpointer) pause_channel_callback,
					   GINT_TO_POINTER (0));

	channel->SetPause (FALSE);
      }
      else {

	if (GTK_IS_LABEL (child)) 
	  gtk_label_set_text_with_mnemonic (GTK_LABEL (child),
					    menu_resume_msg);

	gnomemeeting_log_insert (gw->history_text_view, history_resume_msg);
	gnomemeeting_statusbar_flash (gw->statusbar, history_resume_msg);

	g_signal_handlers_block_by_func (G_OBJECT (b),
					 (gpointer) pause_channel_callback,
					 GINT_TO_POINTER (1));
	gtk_toggle_button_set_active (b, TRUE);
	gtk_widget_queue_draw (GTK_WIDGET (b));
	g_signal_handlers_unblock_by_func (G_OBJECT (b),
					   (gpointer) pause_channel_callback,
					   GINT_TO_POINTER (1));
	
	channel->SetPause (TRUE);
      }
    }

    g_free (menu_suspend_msg);
    g_free (menu_resume_msg);
    g_free (history_suspend_msg);
    g_free (history_resume_msg);
    
    connection->Unlock ();
  }
}


void gnomemeeting_component_view (GtkWidget *w, gpointer data)
{
  if (!GTK_WIDGET_VISIBLE (GTK_WIDGET (data))) 
    gtk_widget_show_all (GTK_WIDGET (data));
  else
    gtk_widget_hide_all (GTK_WIDGET (data));
}


void connect_cb (GtkWidget *widget, gpointer data)
{	
  GmWindow *gw = MyApp->GetMainWindow ();

  if (gw->incoming_call_popup)
    gtk_widget_destroy (gw->incoming_call_popup);

  gw->incoming_call_popup = NULL;

  if ((MyApp->Endpoint ()->GetCallingState () == 0) ||
      (MyApp->Endpoint ()->GetCallingState () == 3))
    MyApp->Connect ();
}


void disconnect_cb (GtkWidget *widget, gpointer data)
{	
  GmWindow *gw = MyApp->GetMainWindow ();

  if (gw->incoming_call_popup)
    gtk_widget_destroy (gw->incoming_call_popup);

  gw->incoming_call_popup = NULL;
  
  MyApp->Disconnect ();
}


void about_callback (GtkWidget *widget, gpointer parent_window)
{
#ifndef DISABLE_GNOME
  GtkWidget *abox = NULL;
  GdkPixbuf *pixbuf = NULL;
	
  const gchar *authors [] = {
      "Damien Sandras <sandras@info.ucl.ac.be>",
      "",
      N_("Code Contributors:"),
      "Kenneth Rohde Christiansen <kenneth@gnu.org>",
      "Miguel Rodr√≠guez P√©rez <migrax@terra.es>",
      "Paul <paul@argo.dyndns.org>", 
      "Roger Hardiman <roger@freebsd.org>",
      "S√©bastien Josset <Sebastien.Josset@space.alcatel.fr>",
      "Tuan <tuan@info.ucl.ac.be>",
      "",
      N_("Artwork:"),
      "Jakub Steiner <jimmac@ximian.com>",
      "",
      N_("Contributors:"),
      "Alexander Larsson <alexl@redhat.com>",
      "Artur Flinta  <aflinta@at.kernel.pl>",
      "Bob Mroczka <bob@mroczka.com>",
      "Chih-Wei Huang <cwhuang@citron.com.tw>",
      "Christian Rose <menthos@menthos.com>",
      "Christian Strauf <strauf@uni-muenster.de>",
      "Christopher R. Gabriel <cgabriel@cgabriel.org>",
      "Cristiano De Michele <demichel@na.infn.it>",
      "Fabrice Alphonso <fabrice@alphonso.dyndns.org>",
      "Florin Grad <florin@mandrakesoft.com>",
      "Georgi Georgiev <chutz@gg3.net>",
      "Johnny Str√∂m <jonny.strom@netikka.fi>",
      "Julien Puydt <julien.puydt@club-internet.fr>",
      "Kilian Krause <kk@verfaction.de>",
      "Matthias Marks <matthias@marksweb.de>",
      "Rafael Pinilla <r_pinilla@yahoo.com>",
      "Santiago Garc√≠a Manti√±√°n <manty@manty.net>",
      "Shawn Pai-Hsiang Hsiao <shawn@eecs.harvard.edu>",
      "Stefan Bruens <lurch@gmx.li>",
      "St√©phane Wirtel<stephane.wirtel@belgacom.net>",
      "Vincent Deroo <crossdatabase@aol.com>",
      NULL
  };

  const gchar *translators [] = {
      N_("Internationalisation Maintainer:"),
      "Christian Rose <menthos@gnu.org>",
      NULL
  };
	
  authors [2] = gettext (authors [2]);
  authors [10] = gettext (authors [10]);
  authors [13] = gettext (authors [13]);
  translators [0] = gettext (translators [0]);
  
  const char *documenters [] = {
    "Damien Sandras <dsandras@seconix.com>",
    "Rafael Pinilla <r_pinilla@yahoo.com>",
    NULL
  };

  /* Translators: Please write translator credits here, and
   * seperate names with \n */
  const char *translator_credits = _("translator_credits");
  
  pixbuf = 
    gdk_pixbuf_new_from_file (GNOMEMEETING_IMAGES "/gnomemeeting-logo-icon.png", NULL);
  

  abox = gnome_about_new ("GnomeMeeting",
			  VERSION,
			  "Copyright ¬© 2000, 2003 Damien Sandras",
                          /* Translators: Please test to see if your translation
                           * looks OK and fits within the box */
			  _("GnomeMeeting is a full-featured H.323\nvideo conferencing application."),
			  (const char **) authors,
                          (const char **) documenters,
                          strcmp (translator_credits, 
				  "translator_credits") != 0 ? 
                          translator_credits : "No translators, English by\n"
                          "Damien Sandras <dsandras@seconix.com>",
			  pixbuf);

  g_object_unref (pixbuf);

  gtk_window_set_transient_for (GTK_WINDOW (abox), GTK_WINDOW (parent_window));
  gtk_window_present (GTK_WINDOW (abox));
#endif

  return;
}


void quit_callback (GtkWidget *widget, gpointer data)
{
  GmWindow *gw = MyApp->GetMainWindow ();

  gtk_widget_hide (gm);
  gtk_widget_hide (gw->docklet);
  gtk_widget_hide (gw->ldap_window);
  gtk_widget_hide (gw->pref_window);

  gtk_main_quit ();
}  


