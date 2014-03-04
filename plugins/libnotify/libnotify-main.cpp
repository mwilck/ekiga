
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
 *                         libnotify-main.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2009 by Damien Sandras
 *   copyright            : (c) 2009 by Julien Puydt
 *   description          : code to push user notifications to the desktop
 *
 */

#include <map>
#include <boost/smart_ptr.hpp>

#include <libnotify/notify.h>

#include <glib/gi18n.h>

#include "config.h"

#include "kickstart.h"
#include "notification-core.h"
#include "call-core.h"
#include "scoped-connections.h"

class LibNotify:
  public Ekiga::Service
{
public:

  LibNotify (Ekiga::ServiceCore& core);

  ~LibNotify ();

  const std::string get_name () const
  { return "libnotify"; }

  const std::string get_description () const
  { return "\tService pushing user notifications to the desktop"; }

  boost::optional<bool> get_bool_property (const std::string name) const;

private:

  Ekiga::scoped_connections connections;

  bool has_actions;

  void on_notification_added (boost::shared_ptr<Ekiga::Notification> notif);
  void on_notification_removed (boost::shared_ptr<Ekiga::Notification> notif);
  void on_call_notification (boost::shared_ptr<Ekiga::CallManager> manager,
                             boost::shared_ptr<Ekiga::Call>  call);
  void on_call_notification_closed (gpointer self);

  typedef std::map<boost::shared_ptr<Ekiga::Notification>, std::pair<boost::signals2::connection, boost::shared_ptr<NotifyNotification> > > container_type;
  container_type live;
};

struct call_reference
{
  call_reference(boost::shared_ptr<Ekiga::Call> _call): call(_call)
  {}

  boost::shared_ptr<Ekiga::Call> call;
};

static void
delete_call_reference (gpointer data)
{
  delete (call_reference *)data;
}

static void
call_notification_action_cb (NotifyNotification *notification,
                             gchar *action,
                             gpointer data)
{
  call_reference* ref = (call_reference *) data;

  notify_notification_close (notification, NULL);
  if (!g_strcmp0 (action, "accept"))
    ref->call->answer ();
  else
    ref->call->hang_up ();
}

static void
notify_action_cb (NotifyNotification *notification,
                  gchar * /*action*/,
                  gpointer data)
{
  Ekiga::Notification *notif = (Ekiga::Notification *) data;
  notify_notification_close (notification, NULL);

  notif->action_trigger ();
}

struct LIBNOTIFYSpark: public Ekiga::Spark
{
  LIBNOTIFYSpark (): result(false)
  {}

  bool try_initialize_more (Ekiga::ServiceCore& core,
			    int* /*argc*/,
			    char** /*argv*/[])
  {
    Ekiga::ServicePtr service = core.get ("libnotify");

    if (!service) {

      core.add (Ekiga::ServicePtr (new LibNotify (core)));
      result = true;
    }

    return result;
  }

  Ekiga::Spark::state get_state () const
  { return result?FULL:BLANK; }

  const std::string get_name () const
  { return "LIBNOTIFY"; }

  bool result;

};

extern "C" void
ekiga_plugin_init (Ekiga::KickStart& kickstart)
{
  boost::shared_ptr<Ekiga::Spark> spark(new LIBNOTIFYSpark);
  kickstart.add_spark (spark);
}


LibNotify::LibNotify (Ekiga::ServiceCore& core)
{
  boost::shared_ptr<Ekiga::NotificationCore> notification_core = core.get<Ekiga::NotificationCore> ("notification-core");
  boost::shared_ptr<Ekiga::CallCore> call_core = core.get<Ekiga::CallCore> ("call-core");

  notify_init ("ekiga");

  has_actions = false;
  GList *capabilities = notify_get_server_caps ();
  if (capabilities != NULL) {
    for (GList *c = capabilities ; c != NULL ; c = c->next) {
      if (g_strcmp0 ((char*)c->data, "actions") == 0 ) {

	has_actions=true;
	break;
      }
    }
    g_list_foreach (capabilities, (GFunc)g_free, NULL);
    g_list_free (capabilities);
  }
  /* Notifications coming from various components */
  connections.add (notification_core->notification_added.connect (boost::bind (&LibNotify::on_notification_added, this, _1)));

  /* Specific notifications */
  connections.add (call_core->setup_call.connect (boost::bind (&LibNotify::on_call_notification, this, _1, _2)));
}

LibNotify::~LibNotify ()
{
  notify_uninit ();
}

boost::optional<bool>
LibNotify::get_bool_property (const std::string name) const
{
  boost::optional<bool> result;

  if (name == "actions") {

    result.reset (has_actions);
  }

  return result;
}

static void
on_notif_closed (NotifyNotification* /*notif*/,
		 gpointer data)
{
  Ekiga::Notification* notification = (Ekiga::Notification*)data;

  notification->removed ();
}

void
LibNotify::on_notification_added (boost::shared_ptr<Ekiga::Notification> notification)
{
  NotifyNotification* notif = notify_notification_new (notification->get_title ().c_str (),
                                                       notification->get_body ().c_str (),
                                                       "ekiga"
#ifdef NOTIFY_CHECK_VERSION
#if !NOTIFY_CHECK_VERSION(0,7,0)
                                                       , NULL
#endif
#else
                                                       , NULL
#endif
                                                      );

  if (notification->get_level () == Ekiga::Notification::Error)
    notify_notification_set_urgency (notif, NOTIFY_URGENCY_CRITICAL);
  if (!notification->get_action_name ().empty ())
    notify_notification_add_action (notif, "action", notification->get_action_name ().c_str (),
                                    notify_action_cb, notification.get (), NULL);

  g_signal_connect (notif, "closed", G_CALLBACK (on_notif_closed), notification.get ());
  boost::signals2::connection conn = notification->removed.connect (boost::bind (&LibNotify::on_notification_removed,
                                                                                this, notification));

  live[notification] = std::pair<boost::signals2::connection, boost::shared_ptr<NotifyNotification> > (conn, boost::shared_ptr<NotifyNotification> (notif, g_object_unref));

  notify_notification_show (notif, NULL);
}

void
LibNotify::on_notification_removed (boost::shared_ptr<Ekiga::Notification> notification)
{
  container_type::iterator iter = live.find (notification);

  if (iter != live.end ()) {
    iter->second.first.disconnect ();
    live.erase (iter);
  }
}

void
LibNotify::on_call_notification_closed (gpointer self)
{
  notify_notification_close (NOTIFY_NOTIFICATION (self), NULL);
}

void
LibNotify::on_call_notification (boost::shared_ptr<Ekiga::CallManager> manager,
                                 boost::shared_ptr<Ekiga::Call> call)
{
  NotifyNotification *notify = NULL;
  call_reference* ref = NULL;

  if (call->is_outgoing () || manager->get_auto_answer ())
    return; // Ignore

  gchar *title = g_strdup_printf (_("Incoming call from %s"), call->get_remote_party_name ().c_str ());
  gchar *body = g_strdup_printf ("<b>%s</b> %s", _("Remote URI:"), call->get_remote_uri ().c_str ());

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
  ref = new call_reference (call);
  notify_notification_add_action (notify, "reject", _("Reject"), call_notification_action_cb, ref, delete_call_reference);
  ref = new call_reference (call);
  notify_notification_add_action (notify, "accept", _("Accept"), call_notification_action_cb, ref, delete_call_reference);
  notify_notification_set_timeout (notify, NOTIFY_EXPIRES_NEVER);
  notify_notification_set_urgency (notify, NOTIFY_URGENCY_CRITICAL);

  connections.add (call->established.connect (boost::bind (&LibNotify::on_call_notification_closed, this, (gpointer) notify)));
  connections.add (call->missed.connect (boost::bind (&LibNotify::on_call_notification_closed, this, (gpointer) notify)));
  connections.add (call->cleared.connect (boost::bind (&LibNotify::on_call_notification_closed, this, (gpointer) notify)));

  notify_notification_show (notify, NULL);

#ifdef NOTIFY_CHECK_VERSION
#if !NOTIFY_CHECK_VERSION(0,7,0)
  notify_notification_set_hint (notify, "transient", g_variant_new_boolean (TRUE));
#endif
#endif

  g_free (title);
  g_free (body);
}
