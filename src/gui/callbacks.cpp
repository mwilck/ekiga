
/* GnomeMeeting -- A Video-Conferencing application
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
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains callbacks common to several
 *                          files.
 *
 */


#include "../../config.h"

#include "history.h"
#include "callbacks.h"
#include "ekiga.h"
#include "main.h"
#include "misc.h"
#include "urlhandler.h"

#include "gmentrydialog.h"
#include "gmconf.h"
#include "gmdialog.h"
#include "gmconnectbutton.h"
#include "gmmenuaddon.h"

#ifdef WIN32
#include "winpaths.h"
#endif

/* The callbacks */
void
save_callback (GtkWidget *widget,
	       gpointer data)
{
  GnomeMeeting::Process ()->GetManager ()->SavePicture ();
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
  gdk_threads_leave ();
  GnomeMeeting::Process ()->Disconnect ();
  gdk_threads_enter ();
}


void
about_callback (GtkWidget *widget, 
		gpointer parent_window)
{
  GtkWidget *abox = NULL;
  GdkPixbuf *pixbuf = NULL;
  gchar     *filename = NULL;

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
  
  filename = g_build_filename (DATA_DIR, "pixmaps", PACKAGE_NAME ".png", NULL);
  pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
  g_free (filename);

  abox = gnome_about_new ("GnomeMeeting",
			  VERSION,
			  "Copyright © 2000-2006 Damien Sandras",
                          /* Translators: Please test to see if your translation
                           * looks OK and fits within the box */
			  _("Ekiga is full-featured SIP and H.323 compatible VoIP, IP-Telephony and Videoconferencing application that allows you to make audio and video calls to remote users with SIP and H.323 hardware or software."),
			  (const char **) authors,
                          (const char **) documenters,
                          strcmp (translator_credits, 
				  "translator_credits") != 0 ? 
                          translator_credits : "No translators, English by\n"
                          "Damien Sandras <dsandras@seconix.com>",
			  pixbuf);

  if (pixbuf)
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
  GtkWidget *main_window = NULL;
  GtkWidget *prefs_window = NULL;
  GtkWidget *accounts_window = NULL;
  GtkWidget *addressbook_window = NULL;
  GtkWidget *calls_history_window = NULL;
  GtkWidget *history_window = NULL;
  
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  addressbook_window = GnomeMeeting::Process ()->GetAddressbookWindow ();
  calls_history_window = GnomeMeeting::Process ()->GetCallsHistoryWindow ();
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow ();
  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();
  history_window = GnomeMeeting::Process ()->GetHistoryWindow ();
  
  gnomemeeting_window_hide (main_window);
  gnomemeeting_window_hide (history_window);
  gnomemeeting_window_hide (calls_history_window);
  gnomemeeting_window_hide (addressbook_window);
  gnomemeeting_window_hide (prefs_window);
  gnomemeeting_window_hide (accounts_window);

  gtk_main_quit ();
}  


gboolean 
entry_completion_url_match_cb (GtkEntryCompletion *completion,
			       const gchar *key,
			       GtkTreeIter *iter,
			       gpointer data)
{
  GtkListStore *list_store = NULL;
  GtkTreeIter tree_iter;
  
  GtkTreePath *current_path = NULL;
  GtkTreePath *path = NULL;
    
  gchar *val = NULL;
  gchar *entry = NULL;
  gchar *tmp_entry = NULL;
  
  PCaselessString s;

  PINDEX j = 0;
  BOOL found = FALSE;
  
  g_return_val_if_fail (data != NULL, FALSE);
  
  list_store = GTK_LIST_STORE (data);

  if (!key || GMURL (key).GetCanonicalURL ().GetLength () < 2)
    return FALSE;

  for (int i = 0 ; (i < 2 && !found) ; i++) {
    
    gtk_tree_model_get (GTK_TREE_MODEL (list_store), iter, i, &val, -1);
    s = val;
    /* Check if one of the names matches the canonical form of the URL */
    if (i == 0) {
      
      j = s.Find (GMURL (key).GetCanonicalURL ());

      if (j != P_MAX_INDEX && j > 0) {

	char c = s [j - 1];
	
	found = (c == 32);
      }
      else if (j == 0)
	found = TRUE;
      else
	found = FALSE;
    }
    /* Check if both GMURLs match */
    else if (i == 1 && GMURL(s).Find (GMURL (key))) 
      found = TRUE;

    g_free (val);
  }
  
  if (!found)
    return FALSE;
  
  /* We have found something, but is it the first item ? */
  gtk_tree_model_get (GTK_TREE_MODEL (list_store), iter, 2, &entry, -1);

  if (found) {

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list_store),
				       &tree_iter)) {

      do {

	gtk_tree_model_get (GTK_TREE_MODEL (list_store), &tree_iter, 
			    2, &tmp_entry, -1);

	if (tmp_entry && !strcmp (tmp_entry, entry)) {

	  current_path = 
	    gtk_tree_model_get_path (GTK_TREE_MODEL (list_store),
				     iter);
	  path = 
	    gtk_tree_model_get_path (GTK_TREE_MODEL (list_store), 
				     &tree_iter);

	  if (gtk_tree_path_compare (path, current_path) < 0) 
	    found = FALSE;

	  gtk_tree_path_free (path);
	  gtk_tree_path_free (current_path);
	}
	
      } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list_store), 
					 &tree_iter) && found);

    }
  }
  
  g_free (entry);

  return found;
}


void 
connect_button_clicked_cb (GtkToggleButton *w, 
			   gpointer data)
{
  GMManager *ep = NULL;
  PString url;

  g_return_if_fail (data != NULL);

  url = gtk_entry_get_text (GTK_ENTRY (data));
  ep = GnomeMeeting::Process ()->GetManager ();
  
  /* Button is in disconnected state */
  if (!gm_connect_button_get_connected (GM_CONNECT_BUTTON (w))
      && ep->GetCallingState () == GMManager::Standby) {
      
    if (!GMURL (url).IsEmpty ())
      GnomeMeeting::Process ()->Connect (url);
    else
      gm_connect_button_set_connected (GM_CONNECT_BUTTON (w), FALSE);
  }
  else if (gm_connect_button_get_connected (GM_CONNECT_BUTTON (w))
	   && ep->GetCallingState () != GMManager::Standby) {

    gdk_threads_leave();
    GnomeMeeting::Process ()->Disconnect ();
    gdk_threads_enter();
  }
  else 
    gm_connect_button_set_connected (GM_CONNECT_BUTTON (w), FALSE);
}
