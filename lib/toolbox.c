
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
 *                         toolbox.c  -  description
 *                         ------------------------------------------
 *   begin                : Dec 2005
 *   copyright            : (C) 2005 by Julien Puydt
 *   description          : Various helper functions
 */

#include "toolbox.h"
#include <string.h>

#ifndef WIN32
#include <gtk/gtk.h>
#else
#include <windows.h>
#endif

#ifndef WIN32
static void
gm_open_uri_fallback (const gchar *uri)
{
  gchar *commandline = NULL;
  gboolean success = FALSE;

  if (!success && g_getenv("KDE_FULL_SESSION") != NULL) {

    commandline = g_strdup_printf ("kfmclient exec %s", uri);
    success = g_spawn_command_line_async (commandline, NULL);
    g_free (commandline);
  }

  if (!success) {

    commandline = g_strdup_printf ("sensible-browser %s", uri);
    success = g_spawn_command_line_async (commandline, NULL);
    g_free (commandline);
  }

  if (!success) {

    commandline = g_strdup_printf ("firefox %s", uri);
    success = g_spawn_command_line_async (commandline, NULL);
    g_free (commandline);
  }

  if (!success) {

    commandline = g_strdup_printf ("konqueror %s", uri);
    success = g_spawn_command_line_async (commandline, NULL);
    g_free (commandline);
  }
}

void
gm_open_uri (const gchar *uri)
{
  GError *error = NULL;

  g_return_if_fail (uri != NULL);

  if (!gtk_show_uri (NULL, uri, GDK_CURRENT_TIME, &error)) {
    g_error_free (error);
    gm_open_uri_fallback (uri);
  }
}

#else

void
gm_open_uri (const gchar *uri)
{
  SHELLEXECUTEINFO sinfo;

  g_return_if_fail (uri != NULL);

  memset (&sinfo, 0, sizeof (sinfo));
  sinfo.cbSize = sizeof (sinfo);
  sinfo.fMask = SEE_MASK_CLASSNAME;
  sinfo.lpVerb = "open";
  sinfo.lpFile = uri;
  sinfo.nShow = SW_SHOWNORMAL;
  sinfo.lpClass = "http";

  (void)ShellExecuteEx (&sinfo); /* leave out any error */
}

#endif

GSList
*gm_string_gslist_remove_dups (GSList *origlist)
{
   /* from a GSList* of gchar*, remove all dup strings
   * (C) Jan Schampera <jan.schampera@web.de> */
  GSList *origlist_iter = NULL;
  GSList *seenlist = NULL;
  GSList *seenlist_iter = NULL;
  gboolean seen = FALSE;

  /* iterate through the original list and compare every stored gchar* to
   * our "seen list", if not there, append it */
  if (!origlist) return NULL;

  for (origlist_iter = origlist;
       origlist_iter != NULL;
       origlist_iter = g_slist_next (origlist_iter))
    {
      if (origlist_iter->data)
	{
	  seen = FALSE;
	  /* check if the string is already in the "seen list" */
	  for (seenlist_iter = seenlist;
	       seenlist_iter != NULL;
	       seenlist_iter = g_slist_next (seenlist_iter))
	    {
	      if (seenlist_iter->data &&
		  !g_strcmp0 ((const char*) origlist_iter->data,
			      (const char*) seenlist_iter->data))
		  seen = TRUE;
	    }
	  if (!seen)
	      /* not in list? append it... */
	      seenlist = g_slist_append (seenlist,
					 (gpointer) g_strdup
					 ((const gchar*) origlist_iter->data));
	}
    }

  /* free the memory of the original list */
  g_slist_foreach (origlist, (GFunc) g_free, NULL);
  g_slist_free (origlist);

  return seenlist;
}
