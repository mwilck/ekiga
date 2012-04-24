
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

#include "services.h"
#include "notification-core.h"

#include "libnotify-main.h"


class LibNotify:
  public Ekiga::Service,
  public boost::signals::trackable
{
public:

  LibNotify (boost::shared_ptr<Ekiga::NotificationCore> core);

  ~LibNotify ();

  const std::string get_name () const
  { return "libnotify"; }

  const std::string get_description () const
  { return "\tService pushing user notifications to the desktop"; }

private:

  void on_notification_added (boost::shared_ptr<Ekiga::Notification> notif);
  void on_notification_removed (boost::shared_ptr<Ekiga::Notification> notif);

  typedef std::map<boost::shared_ptr<Ekiga::Notification>, std::pair<boost::signals::connection, boost::shared_ptr<NotifyNotification> > > container_type;
  container_type live;
};


struct LIBNOTIFYSpark: public Ekiga::Spark
{
  LIBNOTIFYSpark (): result(false)
  {}

  bool try_initialize_more (Ekiga::ServiceCore& core,
			    int* /*argc*/,
			    char** /*argv*/[])
  {
    boost::shared_ptr<Ekiga::NotificationCore> notification = core.get<Ekiga::NotificationCore> ("notification-core");
    Ekiga::ServicePtr service = core.get ("libnotify");

    if (notification && !service) {

      core.add (Ekiga::ServicePtr (new LibNotify (notification)));
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

void
libnotify_init (Ekiga::KickStart& kickstart)
{
  boost::shared_ptr<Ekiga::Spark> spark(new LIBNOTIFYSpark);
  kickstart.add_spark (spark);
}

LibNotify::LibNotify (boost::shared_ptr<Ekiga::NotificationCore> core)
{
  notify_init ("Ekiga");
  core->notification_added.connect (boost::bind (&LibNotify::on_notification_added, this, _1));
}

LibNotify::~LibNotify ()
{
  notify_uninit ();
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
                                                       "Ekiga"
                                                       // NOTIFY_CHECK_VERSION appeared in 0.5.2 only
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

  g_signal_connect (notif, "closed",
		    G_CALLBACK (on_notif_closed), notification.get ());
  boost::signals::connection conn =
    notification->removed.connect (boost::bind (&LibNotify::on_notification_removed, this, notification));

  live[notification] = std::pair<boost::signals::connection, boost::shared_ptr<NotifyNotification> > (conn, boost::shared_ptr<NotifyNotification> (notif, g_object_unref));

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
