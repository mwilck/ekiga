
/* Ekiga/GnomeMeeting -- A Video-Conferencing application
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
 * programs Opal and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         toolbox-common.c  -  description 
 *                         ------------------------------------------
 *   begin                : Jan 2006
 *   copyright            : (C) by various authors (see function code)
 *   description          : Various helper functions -- general
 */

#include "toolbox.h"

/* gm_mkdir_with_parents taken more or less 1:1 from glib 2.8 */
gboolean
gm_mkdir_with_parents (const gchar *pathname,
		        int mode)
{
  gchar *fn, *p;

  if (pathname == NULL)
    return FALSE;

  fn = g_strdup (pathname);

  if (g_path_is_absolute (fn))
    p = (gchar *) g_path_skip_root (fn);
    /* p is now a pointer INSIDE the memory of fn, not a new string, see GLib doc
     * for g_path_skip_root()
     */
  else
    p = fn;

  /* the following loop means for fn="/dir1/dir2/dir3" per iteration:
   * fn == "/dir1"
   * fn == "/dir1/dir2"
   * fn == "/dir1/dir2/dir3"
   *
   * This is done by setting the terminating \0 at the position of
   * every found dir separator per iteration.
   */
  do {
    /* increase p, until we reach the first dir separator */
    while (*p && !G_IS_DIR_SEPARATOR (*p))
      p++;

    if (!*p)
      /* did we reach the end? */
      p = NULL;
    else
      /* this has the effect to give fn a new \0 at
       * the dir separator for termination */
      *p = '\0';

    if (!g_file_test (fn, G_FILE_TEST_EXISTS)) {
      if (g_mkdir (fn, mode) == -1) {
	g_free (fn);
	return FALSE;
      }
    }
    
    else if (!g_file_test (fn, G_FILE_TEST_IS_DIR)) {
      g_free (fn);
      return FALSE;
    }
    
    if (p) {
      /* restore the dir separator and search the next one */
      *p++ = G_DIR_SEPARATOR;
      while (*p && G_IS_DIR_SEPARATOR (*p))
	p++;
    }
  } while (p);

  /* we only need to free fn here, as p doesn't point to extra
   * allocated memory, it points "inside" fn
   */
  g_free (fn);

  return TRUE;
}

