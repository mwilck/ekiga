
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras
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
 *                         evolution-contact.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of an evolution contact
 *
 */

#include <iostream>

#include "evolution-contact.h"
#include "form-request-simple.h"

Evolution::Contact::Contact (Ekiga::ServiceCore &_services,
			     EContact *econtact) : services(_services)
{
  update_econtact (econtact);
}

Evolution::Contact::~Contact ()
{
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

const std::string
Evolution::Contact::get_id () const
{
  return id;
}

const std::string
Evolution::Contact::get_name () const
{
  return name;
}

const std::list<std::string>
Evolution::Contact::get_groups () const
{
  return groups;
}

const std::list<std::pair<std::string, std::string> >
Evolution::Contact::get_uris () const
{
  return uris;
}

bool
Evolution::Contact::is_found (const std::string /*test*/) const
{
  return false;
}

void
Evolution::Contact::update_econtact (EContact *econtact)
{
  const gchar *number = NULL;
  gchar *categories = NULL;
  gchar **split = NULL;
  gchar **ptr = NULL;

  if (econtact == NULL)
    return;

  id = (const gchar *)e_contact_get_const (econtact, E_CONTACT_UID);

  name = (const gchar *)e_contact_get_const (econtact,
					     E_CONTACT_FULL_NAME);

  groups.clear ();
  categories = (gchar *)e_contact_get_const (econtact, E_CONTACT_CATEGORIES);

  if (categories != NULL) {

    split = g_strsplit (categories, ",", 0);
    for (ptr = split; *ptr != NULL; ptr++)
      groups.push_front (*ptr);
    g_strfreev (split);
  }

  uris.clear ();
  number = (const gchar *)e_contact_get_const (econtact,
					       E_CONTACT_PHONE_HOME);
  if (number != NULL)
    uris.push_back (std::pair<std::string, std::string>("home", number));

  number = (const gchar *)e_contact_get_const (econtact,
					       E_CONTACT_PHONE_MOBILE);
  if (number != NULL)
    uris.push_back (std::pair<std::string, std::string>("cell phone", number));

  number = (const gchar *)e_contact_get_const (econtact,
					       E_CONTACT_PHONE_BUSINESS);
  if (number != NULL)
    uris.push_back (std::pair<std::string, std::string>("work", number));

  number = (const gchar *)e_contact_get_const (econtact,
					       E_CONTACT_PHONE_PAGER);
  if (number != NULL)
    uris.push_back (std::pair<std::string, std::string>("pager", number));

  number = (const gchar *)e_contact_get_const (econtact,
					       E_CONTACT_VIDEO_URL);
  if (number != NULL)
    uris.push_back (std::pair<std::string, std::string>("video", number));

  updated.emit ();
}

void
Evolution::Contact::populate_menu (Ekiga::MenuBuilder &builder)
{
  Ekiga::ContactCore *core = dynamic_cast<Ekiga::ContactCore *>(services.get ("contact-core"));
  Ekiga::UI *ui = dynamic_cast<Ekiga::UI *>(services.get ("ui"));

  if (ui != NULL)
    builder.add_action ("Edit", sigc::bind (sigc::mem_fun (this, &Evolution::Contact::edit_action), ui));

  builder.add_action ("Remove", remove_me.make_slot ());

  if (core != NULL)
    core->populate_contact_menu (*this, builder);
}

void
Evolution::Contact::edit_action (Ekiga::UI *ui)
{
  Ekiga::FormRequestSimple request;

  request.title ("Edit contact");

  request.instructions ("Please edit the following fields\n(groups are comma-separated)");

  request.text ("name",
		e_contact_pretty_name (E_CONTACT_FULL_NAME), get_name ());

  {
    std::string home_uri;
    std::string cell_phone_uri;
    std::string work_uri;
    std::string pager_uri;
    std::string video_uri;

    for (std::list<std::pair<std::string, std::string> >::const_iterator iter
	   = uris.begin ();
	 iter != uris.end ();
	 iter++) {

      if (iter->first == "home")
	home_uri = iter->second;
      else if (iter->first == "cell phone")
	cell_phone_uri = iter->second;
      else if (iter->first == "work")
	work_uri = iter->second;
      else if (iter->first == "pager")
	pager_uri = iter->second;
      else if (iter->first == "video")
	video_uri = iter->second;
    }

    request.text ("home",
		  e_contact_pretty_name (E_CONTACT_PHONE_HOME),
		  home_uri);
    request.text ("cell phone",
		  e_contact_pretty_name (E_CONTACT_PHONE_MOBILE),
		  cell_phone_uri);
    request.text ("work",
		  e_contact_pretty_name (E_CONTACT_PHONE_BUSINESS),
		  work_uri);
    request.text ("pager",
		  e_contact_pretty_name (E_CONTACT_PHONE_PAGER),
		  pager_uri);
    request.text ("video",
		  e_contact_pretty_name (E_CONTACT_VIDEO_URL),
		  video_uri);
  }

  {
    std::string categories;
    std::list<std::string>::const_iterator iter = groups.begin ();
    if (iter != groups.end ()) {

      categories += *iter;
      iter++;
    }
    for (; iter != groups.end (); iter++) {

      categories += ',';
      categories += *iter;
    }
    request.text ("groups", "Groups", categories);
  }

  request.submitted.connect (sigc::mem_fun (this,
					    &Evolution::Contact::on_edit_form_submitted));

  ui->run_form_request (request);
}

void
Evolution::Contact::on_edit_form_submitted (Ekiga::Form &result)
{
  try {

    std::map<EContactField, std::string> data;

    data[E_CONTACT_FULL_NAME] = result.text ("name");
    data[E_CONTACT_PHONE_HOME] = result.text ("home");
    data[E_CONTACT_PHONE_MOBILE] = result.text ("cell phone");
    data[E_CONTACT_PHONE_BUSINESS] = result.text ("work");
    data[E_CONTACT_PHONE_PAGER] = result.text ("pager");
    data[E_CONTACT_VIDEO_URL] = result.text ("video");
    data[E_CONTACT_CATEGORIES] = result.text ("groups");

    commit_me.emit (data);

  } catch (Ekiga::Form::not_found) {

    std::cerr << "Invalid result form" << std::endl; // FIXME: do better
  }
}
