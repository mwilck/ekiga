
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
 *                         callbacks.cpp  -  description
 *                         -----------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains callbacks common to several
 *                          files.
 *
 */


#include "config.h"

#include "gmcallbacks.h"

#include "gmwindow.h"
#include "gmstockicons.h"

#ifdef WIN32
#include "platform/winpaths.h"
#include <windows.h>
#include <shellapi.h>
#define WIN32_HELP_DIR "help"
#define WIN32_HELP_FILE "index.html"
#endif

#include <glib/gi18n.h>


/* The callbacks */

void
about_callback (G_GNUC_UNUSED GtkWidget *widget,
		gpointer parent_window)
{
  const gchar *authors [] = {
      "Damien Sandras <dsandras@seconix.com>",
      "",
      N_("Contributors:"),
      "Eugen Dedu <eugen.dedu@pu-pm.univ-fcomte.fr>",
      "Julien Puydt <julien.puydt@laposte.net>",
      "Robert Jongbloed <rjongbloed@postincrement.com>",
      "",
      N_("Artwork:"),
      "Fabian Deutsch <fabian.deutsch@gmx.de>",
      "Vinicius Depizzol <vdepizzol@gmail.com>",
      "Andreas Kwiatkowski <post@kwiat.org>",
      "Carlos Pardo <me@m4de.com>",
      "Jakub Steiner <jimmac@ximian.com>",
      "",
      N_("See AUTHORS file for full credits"),
      NULL
  };

  authors [2] = gettext (authors [2]);
  authors [7] = gettext (authors [7]);
  authors [14] = gettext (authors [14]);

  const gchar *documenters [] = {
    "Damien Sandras <dsandras@seconix.com>",
    "Christopher Warner <zanee@kernelcode.com>",
    "Matthias Redlich <m-redlich@t-online.de>",
    NULL
  };

  const gchar *license[] = {
N_("This program is free software; you can redistribute it and/or modify \
it under the terms of the GNU General Public License as published by \
the Free Software Foundation; either version 2 of the License, or \
(at your option) any later version. "),
N_("This program is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY; without even the implied warranty of \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the \
GNU General Public License for more details. \
You should have received a copy of the GNU General Public License \
along with this program; if not, write to the Free Software Foundation, \
Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA."),
N_("Ekiga is licensed under the GPL license and as a special exception, \
you have permission to link or otherwise combine this program with the \
programs OPAL, OpenH323 and PWLIB, and distribute the combination, \
without applying the requirements of the GNU GPL to the OPAL, OpenH323 \
and PWLIB programs, as long as you do follow the requirements of the \
GNU GPL for all the rest of the software thus combined.")
  };

  gchar *license_trans;

  /* Translators: Please write translator credits here, and
   * separate names with \n */
  const gchar *translator_credits = _("translator-credits");
  if (g_strcmp0 (translator_credits, "translator-credits") == 0)
    translator_credits = "No translators, English by\n"
        "Damien Sandras <dsandras@seconix.com>";

  const gchar *comments =  _("Ekiga is full-featured SIP and H.323 compatible VoIP, IP-Telephony and Videoconferencing application that allows you to make audio and video calls to remote users with SIP and H.323 hardware or software.");

  license_trans = g_strconcat (_(license[0]), "\n\n", _(license[1]), "\n\n",
                               _(license[2]), "\n\n", NULL);

  gtk_show_about_dialog (GTK_WINDOW (parent_window),
		"name", "Ekiga",
		"version", VERSION,
                "copyright", "Copyright © 2000-2012 Damien Sandras",
		"authors", authors,
		"documenters", documenters,
		"translator-credits", translator_credits,
		"comments", comments,
		"logo-icon-name", GM_ICON_LOGO,
		"license", license_trans,
		"wrap-license", TRUE,
		"website", "http://www.ekiga.org",
		NULL);

  g_free (license_trans);
}


void
help_callback (G_GNUC_UNUSED GtkWidget *widget,
	       G_GNUC_UNUSED gpointer data)
{
#ifdef WIN32
  gchar *locale, *loc_ , *index_path;
  int hinst = 0;

  locale = g_win32_getlocale ();
  if (strlen (locale) > 0) {

    /* try returned locale first, it may be fully qualified e.g. zh_CN */
    index_path = g_build_filename (WIN32_HELP_DIR, locale,
				   WIN32_HELP_FILE, NULL);
    hinst = (int) ShellExecute (NULL, "open", index_path, NULL,
			  	DATA_DIR, SW_SHOWNORMAL);
    g_free (index_path);
  }

  if (hinst <= 32 && (loc_ = g_strrstr (locale, "_"))) {
    /* on error, try short locale */
    *loc_ = 0;
    index_path = g_build_filename (WIN32_HELP_DIR, locale,
				   WIN32_HELP_FILE, NULL);
    hinst = (int) ShellExecute (NULL, "open", index_path, NULL,
				DATA_DIR, SW_SHOWNORMAL);
    g_free (index_path);
  }

  g_free (locale);

  if (hinst <= 32) {

    /* on error or missing locale, try default locale */
    index_path = g_build_filename (WIN32_HELP_DIR, "C", WIN32_HELP_FILE, NULL);
    (void)ShellExecute (NULL, "open", index_path, NULL,
			DATA_DIR, SW_SHOWNORMAL);
    g_free (index_path);
  }
#else /* !WIN32 */
  GError *err = NULL;
  gboolean success = FALSE;

  success = gtk_show_uri (NULL, "ghelp:" PACKAGE_NAME, GDK_CURRENT_TIME, &err);

  if (!success) {
    GtkWidget *d;
    d = gtk_message_dialog_new (NULL,
                                (GtkDialogFlags) (GTK_DIALOG_MODAL),
                                GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                                "%s", _("Unable to open help file."));
    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (d),
                                              "%s", err->message);
    g_signal_connect (d, "response", G_CALLBACK (gtk_widget_destroy), NULL);
    gtk_window_present (GTK_WINDOW (d));
    g_error_free (err);
  }
#endif
}


void
quit_callback (G_GNUC_UNUSED GtkWidget *widget,
	       G_GNUC_UNUSED gpointer data)
{
  while (gtk_events_pending ())
    gtk_main_iteration ();

  gtk_main_quit ();
}
