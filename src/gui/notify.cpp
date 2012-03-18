
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


#include <libnotify/notify.h>
#include <glib/gi18n.h>

#include "config.h"

#include "notify.h"

#include "services.h"
#include "call-core.h"

struct CallNotificationInfo
{
  boost::shared_ptr<Ekiga::Call> call;
};

// return if the notify server accepts actions (i.e. buttons)
// taken from https://wiki.ubuntu.com/NotificationDevelopmentGuidelines#Avoiding%20actions
static int
hasActionsCap (void)
{
  static int accepts_actions = -1;
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
  return accepts_actions;
}

static void
notify_action_cb (NotifyNotification *notification,
                  gchar *action,
                  gpointer data)
{
  CallNotificationInfo *priv = (CallNotificationInfo *) (data);
  boost::shared_ptr<Ekiga::Call> call = priv->call;

  notify_notification_close (notification, NULL);

  if (call) {

    if (!strcmp (action, "accept"))
      call->answer ();
    else
      call->hangup ();
  }
  g_object_set_data (G_OBJECT (notification), "priv", NULL);
  if (priv)
    delete priv;
}

static void
on_incoming_call_gone_cb (gpointer self)
{
  CallNotificationInfo *priv = (CallNotificationInfo *) (g_object_get_data (G_OBJECT (self), "priv"));
  notify_notification_close (NOTIFY_NOTIFICATION (self), NULL);
  g_object_set_data (G_OBJECT (self), "priv", NULL);
  if (priv)
    delete priv;
}

static void
ekiga_incoming_call_notify (boost::shared_ptr<Ekiga::Call> call)
{
  NotifyNotification *notify = NULL;

  gchar *uri = NULL;
  gchar *body = NULL;
  gchar *title = NULL;

  const char *utf8_name = call->get_remote_party_name ().c_str ();
  const char *utf8_url = call->get_remote_uri ().c_str ();

  title = g_strdup_printf (_("Incoming call from %s"), (const char*) utf8_name);

  if (utf8_url)
    uri = g_strdup_printf ("<b>%s</b> %s", _("Remote URI:"), utf8_url);

  body = g_strdup_printf ("%s", uri);

  CallNotificationInfo *priv = new CallNotificationInfo ();
  priv->call = call;

  notify = notify_notification_new (title, body, NULL
// NOTIFY_CHECK_VERSION appeared in 0.5.2 only
#ifndef NOTIFY_CHECK_VERSION
                                    , NULL
#else
#if !NOTIFY_CHECK_VERSION(0,7,0)
                                    , NULL
#endif
#endif
                                    );
  g_object_set_data (G_OBJECT (notify), "priv", priv);
  notify_notification_add_action (notify, "reject", _("Reject"), notify_action_cb, priv, NULL);
  notify_notification_add_action (notify, "accept", _("Accept"), notify_action_cb, priv, NULL);
  notify_notification_set_app_name (notify, "ekiga");
  notify_notification_set_timeout (notify, NOTIFY_EXPIRES_NEVER);
  notify_notification_set_urgency (notify, NOTIFY_URGENCY_CRITICAL);

  notify_notification_show (notify, NULL);

  call->established.connect (boost::bind (&on_incoming_call_gone_cb,
                                          (gpointer) notify));
  call->cleared.connect (boost::bind (&on_incoming_call_gone_cb,
                                      (gpointer) notify));
  call->missed.connect (boost::bind (&on_incoming_call_gone_cb,
                                     (gpointer) notify));

  g_free (uri);
  g_free (title);
  g_free (body);
}

static void on_setup_call_cb (boost::shared_ptr<Ekiga::CallManager> manager,
                              boost::shared_ptr<Ekiga::Call>  call)
{
  if (!call->is_outgoing () && !manager->get_auto_answer () && hasActionsCap ())
    ekiga_incoming_call_notify (call);
}


/*
 * Public API
 */
void
notify_start (Ekiga::ServiceCore & core)
{
  boost::shared_ptr<Ekiga::CallCore> call_core = core.get<Ekiga::CallCore> ("call-core");

  call_core->setup_call.connect (boost::bind (&on_setup_call_cb, _1, _2));
}
