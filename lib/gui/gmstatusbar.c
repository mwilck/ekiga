
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
 *                         gmstatusbar.c  -  description
 *                         -------------------------------
 *   begin                : Tue Nov 01 2005
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Contains a statusbar widget
 *
 */


#include "gmstatusbar.h"

G_DEFINE_TYPE (GmStatusbar, gm_statusbar, GTK_TYPE_STATUSBAR);

/* this is ugly, but we want to give several pieces of data to a callback...
 * (and this hack replaces one using a global piece of data... yurk!)
 */
typedef struct {

  GtkStatusbar* statusbar;
  gint msg_id;
} callback_info;

/* Static functions and declarations */
static void gm_statusbar_class_init (GmStatusbarClass *);

static void gm_statusbar_init (GmStatusbar *);

static void gm_sb_push_message (GmStatusbar *,
				gboolean,
				gboolean,
				const char *,
				va_list args);

static int  gm_statusbar_clear_msg_cb (gpointer);

static void
gm_statusbar_class_init (G_GNUC_UNUSED GmStatusbarClass *klass)
{
}


static void
gm_statusbar_init (G_GNUC_UNUSED GmStatusbar *sb)
{
}


static int
gm_statusbar_clear_msg_cb (gpointer data)
{
  GtkStatusbar* statusbar = ((callback_info*)data)->statusbar;
  gint msg_id = ((callback_info*)data)->msg_id;
  gint id = 0;

  id = gtk_statusbar_get_context_id (statusbar, "statusbar");
  gtk_statusbar_remove (statusbar, id, msg_id);

  /*  g_free (data); yes, it's tempting, but we have a destroy notifier which will do it */

  return FALSE;
}


static void
gm_sb_push_message (GmStatusbar *sb,
		    gboolean flash_message,
		    gboolean info_message,
		    const char *msg,
		    va_list args)
{
  static guint timer_source;
  gint id = 0;
  gint msg_id = 0;
#if GTK_CHECK_VERSION (2, 21, 2)
#else
  int len = 0;
  int i = 0;
#endif

  g_return_if_fail (sb != NULL);

  if (info_message)
    id = gtk_statusbar_get_context_id (GTK_STATUSBAR (sb), "info");
  else
    id = gtk_statusbar_get_context_id (GTK_STATUSBAR (sb), "statusbar");

#if GTK_CHECK_VERSION (2, 21, 2)
  gtk_statusbar_remove_all (GTK_STATUSBAR (sb), id);
#else
  len = g_slist_length ((GSList *) (GTK_STATUSBAR (sb)->messages));
  for (i = 0 ; i < len ; i++)
    gtk_statusbar_pop (GTK_STATUSBAR (sb), id);
#endif

  if (msg) {

    char buffer [1025];

    g_vsnprintf (buffer, 1024, msg, args);

    msg_id = gtk_statusbar_push (GTK_STATUSBAR (sb), id, buffer);

    if (flash_message)
    {
      if (timer_source != 0)
      {
        g_source_remove (timer_source);
        timer_source = 0;
      }

      callback_info* info = g_new0 (callback_info, 1);
      info->statusbar = GTK_STATUSBAR (sb);
      info->msg_id = msg_id;
      timer_source = g_timeout_add_seconds_full (G_PRIORITY_DEFAULT,
						 15, gm_statusbar_clear_msg_cb, info,
						 g_free);
    }
  }
}


/* public api */

GtkWidget *
gm_statusbar_new ()
{
  GObject* result = NULL;

  result = g_object_new (GM_TYPE_STATUSBAR, NULL);

  return GTK_WIDGET (result);
}


void
gm_statusbar_flash_message (GmStatusbar *sb,
			    const char *msg,
			    ...)
{
  va_list args;

  va_start (args, msg);
  gm_sb_push_message (sb, TRUE, FALSE, msg, args);

  va_end (args);
}


void
gm_statusbar_push_message (GmStatusbar *sb,
			   const char *msg,
			   ...)
{
  va_list args;

  va_start (args, msg);
  gm_sb_push_message (sb, FALSE, FALSE, msg, args);

  va_end (args);
}


void
gm_statusbar_push_info_message (GmStatusbar *sb,
				const char *msg,
				...)
{
  va_list args;

  va_start (args, msg);
  gm_sb_push_message (sb, FALSE, TRUE, msg, args);

  va_end (args);
}
