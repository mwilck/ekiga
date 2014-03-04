
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2014 Damien Sandras <dsandras@seconix.com>
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
 *                         live-object-menu.cpp  -  description
 *                         -------------------------------------
 *   begin                : written in 2014 by Damien Sandras
 *   copyright            : (c) 2014 by Damien Sandras
 *   description          : A live object menu implementation
 *
 */

#include <gio/gio.h>
#include <string>

#include "action.h"
#include "contact-core.h"
#include "actor-menu.h"


static void
action_activated (GSimpleAction *a,
                  G_GNUC_UNUSED GVariant *p,
                  gpointer data)
{
  Ekiga::Action *action = (Ekiga::Action *) g_object_get_data (G_OBJECT (a), "action");
  Ekiga::ActorMenu *menu = (Ekiga::ActorMenu *) data;

  g_return_if_fail (action && menu);
  menu->activate (action);
}



Ekiga::ActorMenu::ActorMenu (const Ekiga::Actor & _obj) : obj (_obj)
{
}


Ekiga::ActorMenu::~ActorMenu ()
{
  ActionMap::const_iterator it;

  for (it = obj.actions.begin(); it != obj.actions.end(); ++it)
    g_action_map_remove_action (G_ACTION_MAP (g_application_get_default ()),
                                it->first.c_str ());
}


void
Ekiga::ActorMenu::add_gio_actions ()
{
  GSimpleAction *action = NULL;
  ActionMap::const_iterator it;

  for (it = obj.actions.begin(); it != obj.actions.end(); ++it) {

    if (!g_action_map_lookup_action (G_ACTION_MAP (g_application_get_default ()),
                                     it->first.c_str ())) {

      action = g_simple_action_new (it->first.c_str (), NULL);
      g_object_set_data (G_OBJECT (action), "action", it->second.get ());
      g_action_map_add_action (G_ACTION_MAP (g_application_get_default ()),
                               G_ACTION (action));
      g_signal_connect (action, "activate",
                        G_CALLBACK (action_activated),
                        (gpointer) this);
      g_object_unref (action);
    }
  }
}


const std::string
Ekiga::ActorMenu::get_xml_menu (const std::string & id,
                                const std::string & content,
                                bool full)
{
  std::string xml_content = "<menu id=\"" + id + "\">" + content + "</menu>";

  if (full)
    return "<?xml_content version=\"1.0\"?><interface>" + xml_content + "</interface>";

  return xml_content;
}


Ekiga::ActorMenu *
Ekiga::ActorMenu::create (const Ekiga::Actor & obj)
{
  Ekiga::ActorMenu *m = new Ekiga::ActorMenu (obj);
  m->add_gio_actions ();

  return m;
}


void
Ekiga::ActorMenu::activate (Ekiga::Action *action)
{
  action->activate ();
}


const std::string
Ekiga::ActorMenu::as_xml (const std::string & id)
{
  ActionMap::const_iterator it;
  std::string xml_content;

  if (!id.empty ())
   xml_content += "    <section id=\"" + id + "\">";
  else
   xml_content += "    <section>";

  for (it = obj.actions.begin(); it != obj.actions.end(); ++it) {

    xml_content +=
      "      <item>"
      "        <attribute name=\"label\" translatable=\"yes\">"+it->second->get_description ()+"</attribute>"
      "        <attribute name=\"action\">win."+it->second->get_name ()+"</attribute>"
      "      </item>";
  }

  xml_content +=
    "    </section>";

  return xml_content;
}


Ekiga::ContactActorMenu::ContactActorMenu (const Ekiga::Actor & _obj) : ActorMenu (_obj)
{
}


void
Ekiga::ContactActorMenu::add_gio_actions ()
{
  GSimpleAction *action = NULL;
  ActionMap::const_iterator it;

  for (it = obj.actions.begin(); it != obj.actions.end(); ++it) {

    Ekiga::ContactAction *a = dynamic_cast<Ekiga::ContactAction *> (it->second.get ());

    if (!a || !a->can_run_with_data (contact, uri)) {
      g_action_map_remove_action (G_ACTION_MAP (g_application_get_default ()),
                                  it->first.c_str ());
    }
    else if (a && a->can_run_with_data (contact, uri)) {

      action = g_simple_action_new (it->first.c_str (), NULL);
      g_object_set_data (G_OBJECT (action), "action", it->second.get ());
      g_action_map_add_action (G_ACTION_MAP (g_application_get_default ()),
                               G_ACTION (action));
      g_signal_connect (action, "activate",
                        G_CALLBACK (action_activated),
                        (gpointer) this);
      g_object_unref (action);
    }
  }
}


void
Ekiga::ContactActorMenu::set_data (Ekiga::ContactPtr _contact,
                                   const std::string & _uri)
{
  contact = _contact;
  uri = _uri;

  add_gio_actions ();
}


const std::string
Ekiga::ContactActorMenu::as_xml (const std::string & id)
{
  ActionMap::const_iterator it;
  std::string xml_content;

  if (!id.empty ())
   xml_content += "    <section id=\"" + id + "\">";
  else
   xml_content += "    <section>";

  for (it = obj.actions.begin(); it != obj.actions.end(); ++it) {

    Ekiga::ContactAction *action = dynamic_cast<Ekiga::ContactAction *> (it->second.get ());

    if (action && action->can_run_with_data (contact, uri)) {
      action->set_data (contact, uri);
      xml_content +=
        "      <item>"
        "        <attribute name=\"label\" translatable=\"yes\">"+it->second->get_description ()+"</attribute>"
        "        <attribute name=\"action\">win."+it->second->get_name ()+"</attribute>"
        "      </item>";
    }
  }

  xml_content +=
    "    </section>";

  return xml_content;
}


Ekiga::ContactActorMenu *
Ekiga::ContactActorMenu::create (const Ekiga::Actor & obj)
{
  Ekiga::ContactActorMenu *m = new Ekiga::ContactActorMenu (obj);
  m->add_gio_actions ();

  return m;
}
