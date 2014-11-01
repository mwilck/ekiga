
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
 *                         gactor-menu.cpp  -  description
 *                         -------------------------------
 *   begin                : written in 2014 by Damien Sandras
 *   copyright            : (c) 2014 by Damien Sandras
 *   description          : An Actor object menu implementation
 *
 */

#include <gio/gio.h>
#include <string>

#include "action.h"
#include "gactor-menu.h"


static void
action_activated (GSimpleAction *a,
                  G_GNUC_UNUSED GVariant *p,
                  gpointer data)
{
  const char *action = (const char *) g_object_get_data (G_OBJECT (a), "action");
  Ekiga::GActorMenu *menu = (Ekiga::GActorMenu *) data;

  g_return_if_fail (action && menu);
  menu->activate (action);
}


Ekiga::GActorMenu::GActorMenu (Ekiga::Actor & _obj) : obj (_obj)
{
  n = 0;
  builder = gtk_builder_new ();

  sync_gio_actions ();

  conns.add (obj.action_enabled.connect (boost::bind (static_cast<void (Ekiga::GActorMenu::*)(const std::string&)>(&Ekiga::GActorMenu::add_gio_action), this, _1)));
  conns.add (obj.action_disabled.connect (boost::bind (&Ekiga::GActorMenu::remove_gio_action, this, _1)));

  conns.add (obj.action_added.connect (boost::bind (static_cast<void (Ekiga::GActorMenu::*)(const std::string&)>(&Ekiga::GActorMenu::add_gio_action), this, _1)));
  conns.add (obj.action_removed.connect (boost::bind (&Ekiga::GActorMenu::remove_gio_action, this, _1)));
}


Ekiga::GActorMenu::~GActorMenu ()
{
  Actor::const_iterator it;

  for (it = obj.begin(); it != obj.end(); ++it)
    g_action_map_remove_action (G_ACTION_MAP (g_application_get_default ()),
                                (*it)->get_name ().c_str ());

  g_object_unref (builder);
}


void
Ekiga::GActorMenu::activate (const std::string & name)
{
  Actor::const_iterator it;

  for (it = obj.begin(); it != obj.end(); ++it) {
    if (name.empty () || (*it)->get_name () == name) {
      (*it)->activate ();
      return;
    }
  }
}


GMenuModel *
Ekiga::GActorMenu::get_model (const Ekiga::GActorMenuStore & store)
{
  int c = 0;
  std::string content = as_xml ();

  for (Ekiga::GActorMenuStore::const_iterator it = store.begin ();
       it != store.end ();
       it++) {
    content = content + (*it)->as_xml ();
    c += (*it)->size ();
  }
  content = "<?xml_content version=\"1.0\"?>"
            "<interface>"
            "  <menu id=\"menu\">" + content + "</menu>"
            "</interface>";

  gtk_builder_add_from_string (builder, content.c_str (), -1, NULL);
  return (n > 0 || c > 0 ? G_MENU_MODEL (gtk_builder_get_object (builder, "menu")) : NULL);
}


GtkWidget *
Ekiga::GActorMenu::get_menu (const Ekiga::GActorMenuStore & store)
{
  GtkWidget *menu = gtk_menu_new_from_model (get_model (store));
  gtk_widget_insert_action_group (menu, "win", G_ACTION_GROUP (g_application_get_default ()));
  g_object_ref (menu);

  return menu;
}


unsigned
Ekiga::GActorMenu::size ()
{
  return n;
}


void
Ekiga::GActorMenu::sync_gio_actions ()
{
  Actor::const_iterator it;

  for (it = obj.begin(); it != obj.end(); ++it) {
    if ((*it)->is_enabled ())
      add_gio_action (*it);
    else
      remove_gio_action ((*it)->get_name ());
  }
}


void
Ekiga::GActorMenu::add_gio_action (const std::string & name)
{
  Actor::const_iterator it;

  for (it = obj.begin(); it != obj.end(); ++it) {
    if ((*it)->get_name () == name) {
      add_gio_action (*it);
      return;
    }
  }
}


void
Ekiga::GActorMenu::add_gio_action (Ekiga::ActionPtr a)
{
  GSimpleAction *action = NULL;

  /* Action is disabled or already present */
  if (!a->is_enabled ()
      || g_action_map_lookup_action (G_ACTION_MAP (g_application_get_default ()),
                                     a->get_name ().c_str ()))
    return;

  action = g_simple_action_new (a->get_name ().c_str (), NULL);
  g_object_set_data_full (G_OBJECT (action), "action",
                          g_strdup (a->get_name ().c_str ()),
                          (GDestroyNotify) g_free);
  g_action_map_add_action (G_ACTION_MAP (g_application_get_default ()),
                           G_ACTION (action));
  g_signal_connect (action, "activate",
                    G_CALLBACK (action_activated),
                    (gpointer) this);
  g_object_unref (action);
}


void
Ekiga::GActorMenu::remove_gio_action (const std::string & name)
{
  g_action_map_remove_action (G_ACTION_MAP (g_application_get_default ()),
                              name.c_str ());
}


const std::string
Ekiga::GActorMenu::as_xml (const std::string & id)
{
  Actor::const_iterator it;
  std::string xml_content;
  n = 0;

  if (!id.empty ())
   xml_content += "    <section id=\"" + id + "\">";
  else
   xml_content += "    <section>";

  for (it = obj.begin(); it != obj.end(); ++it) {

      xml_content +=
        "      <item>"
        "        <attribute name=\"label\" translatable=\"yes\">"+(*it)->get_description ()+"</attribute>"
        "        <attribute name=\"action\">win."+(*it)->get_name ()+"</attribute>"
        "      </item>";
      n++;
  }

  xml_content +=
    "    </section>";

  return (n > 0 ? xml_content : "");
}


const std::string
Ekiga::GActorMenu::build ()
{
  std::string xml_content = "<menu id=\"menu\">" + as_xml () + "</menu>";
  return "<?xml_content version=\"1.0\"?><interface>" + xml_content + "</interface>";
}
