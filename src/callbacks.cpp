
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
 *                         callbacks.cpp  -  description
 *                         -----------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains callbacks common to several
 *                          files.
 *
 */


#include "../config.h"

#include "log_window.h"
#include "callbacks.h"
#include "gnomemeeting.h"
#include "main_window.h"
#include "misc.h"
#include "urlhandler.h"

#include "gmentrydialog.h"
#include "gm_conf.h"
#include "dialog.h"
#include "lib/gtk_menu_extensions.h"


/* The callbacks */
void
save_callback (GtkWidget *widget,
	       gpointer data)
{
  GnomeMeeting::Process ()->Endpoint ()->SavePicture ();
}



gboolean
delete_window_cb (GtkWidget *w,
                  GdkEvent *ev,
                  gpointer data)
{
  gnomemeeting_window_hide (GTK_WIDGET (w));

  return TRUE;
}


void
show_window_cb (GtkWidget *w,
		gpointer data)
{
  if (!gnomemeeting_window_is_visible (GTK_WIDGET (data)))
    gnomemeeting_window_show (GTK_WIDGET (data));
  else
    gnomemeeting_window_hide (GTK_WIDGET (data));
}


void
connect_cb (GtkWidget *widget,
	    gpointer data)
{	
  PString url;

  g_return_if_fail (data != NULL);
  
  url = gm_main_window_get_call_url (GTK_WIDGET (data)); 

  GnomeMeeting::Process ()->Connect (url);
}


void
disconnect_cb (GtkWidget *widget,
	       gpointer data)
{	
  GnomeMeeting::Process ()->Disconnect ();
}


void
about_callback (GtkWidget *widget, 
		gpointer parent_window)
{
  GtkWidget *abox = NULL;
  GdkPixbuf *pixbuf = NULL;
	
  const gchar *authors [] = {
      "Damien Sandras <dsandras@seconix.com>",
      "",
      N_("Code contributors:"),
      "Benjamin Leviant <belevian@gmail.com>",
      "Kenneth Rohde Christiansen <kenneth@gnu.org>",
      "Julien Puydt <julien.puydt@laposte.net>",
      "Miguel Rodríguez Pérez <miguelrp@gmail.com>",
      "Paul <paul@argo.dyndns.org>", 
      "Roger Hardiman <roger@freebsd.org>",
      "Sébastien Josset <Sebastien.Josset@space.alcatel.fr>",
      "Sébastien Estienne <sebastien.estienne@gmail.com>",
      "Stefan Bruëns <lurch@gmx.li>",
      "Tuan <tuan@info.ucl.ac.be>",
      "Xavier Ricco <ricco@mulitel.be>",
      "Julien Hamaide <hamaide@multitel.be>",
      "",
      N_("Artwork:"),
      "Jakub Steiner <jimmac@ximian.com>",
      "Carlos Pardo <me@m4de.com>",
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
      "Johnny Ström <jonny.strom@netikka.fi>",
      "Kilian Krause <kk@verfaction.de>",
      "Matthias Marks <matthias@marksweb.de>",
      "Rafael Pinilla <r_pinilla@yahoo.com>",
      "Santiago García Mantiñán <manty@manty.net>",
      "Shawn Pai-Hsiang Hsiao <shawn@eecs.harvard.edu>",
      "Stéphane Wirtel <stephane.wirtel@belgacom.net>",
      "Vincent Deroo <vincent.deroo@neuf.fr>",
      NULL
  };
	
  authors [2] = gettext (authors [2]);
  authors [16] = gettext (authors [16]);
  authors [20] = gettext (authors [20]);
  
  const char *documenters [] = {
    "Damien Sandras <dsandras@seconix.com>",
    "Christopher Warner <zanee@kernelcode.com>",
    "Matthias Redlich <m-redlich@t-online.de>",
    NULL
  };

  /* Translators: Please write translator credits here, and
   * seperate names with \n */
  const char *translator_credits = _("translator-credits");
  
  pixbuf = 
    gdk_pixbuf_new_from_file (GNOMEMEETING_IMAGES PACKAGE_NAME ".png", NULL);
  

  abox = gnome_about_new ("GnomeMeeting",
			  VERSION,
			  "Copyright © 2000-2004 Damien Sandras",
                          /* Translators: Please test to see if your translation
                           * looks OK and fits within the box */
			  _("GnomeMeeting is full-featured H.323 compatible videoconferencing, VoIP and IP-Telephony application that allows you to make audio and video calls to remote users with H.323 hardware or software."),
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
}


void
help_cb (GtkWidget *widget,
	 gpointer data)
{
#ifndef DISABLE_GNOME
  GError *err = NULL;
  gnome_help_display (PACKAGE_NAME ".xml", NULL, &err);
#endif
}


void
quit_callback (GtkWidget *widget, 
	       gpointer data)
{
  GMEndPoint *ep =NULL;
  
  GtkWidget *main_window = NULL;
  GtkWidget *prefs_window = NULL;
  GtkWidget *accounts_window = NULL;
  GtkWidget *addressbook_window = NULL;
  GtkWidget *calls_history_window = NULL;
  GtkWidget *history_window = NULL;
  GtkWidget *tray = NULL;
  
  ep = GnomeMeeting::Process ()->Endpoint ();
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  addressbook_window = GnomeMeeting::Process ()->GetAddressbookWindow ();
  calls_history_window = GnomeMeeting::Process ()->GetCallsHistoryWindow ();
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();
  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  tray = GnomeMeeting::Process ()->GetTray ();
  
  gnomemeeting_window_hide (main_window);
  gnomemeeting_window_hide (history_window);
  gnomemeeting_window_hide (calls_history_window);
  gnomemeeting_window_hide (addressbook_window);
  gnomemeeting_window_hide (prefs_window);
  gnomemeeting_window_hide (accounts_window);
  gtk_widget_hide (tray);
  
  gtk_main_quit ();
}  


