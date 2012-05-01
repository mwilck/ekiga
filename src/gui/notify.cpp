
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2012 Damien Sandras <dsandras@seconix.com>
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
 *                         notify.cpp  -  description
 *                         --------------------------
 *   begin                : Sun Mar 18 2012
 *   copyright            : (C) 2000-2012 by Damien Sandras
 *   description          : Global notifications based on libnotify
 */


#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "config.h"

#include "notify.h"

#include "common.h"
#include "gmconf.h"

#include "ekiga.h" //FIXME Can get rid of this

#include "gmstockicons.h"
#include "services.h"
#include "account-core.h"
#include "call-core.h"
#include "gtk-frontend.h"

#ifdef HAVE_NOTIFY
#include <libnotify/notify.h>


static void
notify_show_window_action_cb (NotifyNotification *notification,
                              G_GNUC_UNUSED gchar *action,
                              gpointer data)
{
  GtkWidget *window = GTK_WIDGET (data);

  if (!gtk_widget_get_visible (window)
      || (gdk_window_get_state (GDK_WINDOW (window->window)) & GDK_WINDOW_STATE_ICONIFIED))
    gtk_widget_show (window);

  notify_notification_close (notification, NULL);
}

static void
on_unread_count_cb (G_GNUC_UNUSED GtkWidget *widget,
                    guint messages,
                    gpointer data)
{
  NotifyNotification *notify = NULL;

  gchar *body = NULL;

  if (messages > 0) {
    body = g_strdup_printf (ngettext ("You have %d message",
                                      "You have %d messages",
                                      messages), messages);

    notify = notify_notification_new (_("Unread message"), body, GM_ICON_LOGO
                                      // NOTIFY_CHECK_VERSION appeared in 0.5.2 only
#ifndef NOTIFY_CHECK_VERSION
                                      , NULL
#else
#if !NOTIFY_CHECK_VERSION(0,7,0)
                                      , NULL
#endif
#endif
                                     );

    notify_notification_set_urgency (notify, NOTIFY_URGENCY_NORMAL);
    notify_notification_set_timeout (notify, NOTIFY_EXPIRES_NEVER);
    notify_notification_add_action (notify, "ignore", _("Ignore"), notify_show_window_action_cb, data, NULL);
    notify_notification_add_action (notify, "default", _("Show"), notify_show_window_action_cb, data, NULL);
    notify_notification_show (notify, NULL);
  }
}
#endif

/*
 * Public API
 */
bool
notify_start (Ekiga::ServiceCore & core)
{
#ifdef HAVE_NOTIFY
  boost::shared_ptr<GtkFrontend> frontend = core.get<GtkFrontend> ("gtk-frontend");
  boost::shared_ptr<Ekiga::AccountCore> account_core = core.get<Ekiga::AccountCore> ("account-core");

  GtkWidget *chat_window = GTK_WIDGET (frontend->get_chat_window ());

  g_signal_connect (chat_window, "unread-count", G_CALLBACK (on_unread_count_cb), chat_window);
  return true;
#else
  return false;
#endif
}

bool
notify_has_actions (void)
{
  static int accepts_actions = -1;
#ifdef HAVE_NOTIFY
  if (accepts_actions == -1) {  // initialise accepts_actions at the first call
    accepts_actions = 0;
    GList *capabilities = notify_get_server_caps ();
    if (capabilities != NULL) {
      for (GList *c = capabilities ; c != NULL ; c = c->next) {
        if (strcmp ((char*)c->data, "actions") == 0 ) {
          accepts_actions = 1;
          break;
        }
      }
      g_list_foreach (capabilities, (GFunc)g_free, NULL);
      g_list_free (capabilities);
    }
  }
#endif
  return (accepts_actions > 0);
}
