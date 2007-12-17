
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

#include "config.h"

#include "evolution-contact.h"
#include "form-request-simple.h"

Evolution::Contact::Contact (Ekiga::ServiceCore &_services,
			     EBook *ebook,
			     EContact *econtact) : services(_services),
						   book(ebook)
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

const std::set<std::string>
Evolution::Contact::get_groups () const
{
  return groups;
}

const std::map<std::string, std::string>
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
  gchar *categories = NULL;
  gchar **split = NULL;
  gchar **ptr = NULL;
  GList *attributes = NULL;

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
      groups.insert (*ptr);
    g_strfreev (split);
  }

  uris.clear ();

  attributes = e_vcard_get_attributes (E_VCARD (econtact));

  for (GList *attribute_ptr = attributes ;
       attribute_ptr != NULL;
       attribute_ptr = g_list_next (attribute_ptr)) {

    EVCardAttribute *attribute = (EVCardAttribute *)attribute_ptr->data;
    std::string attr_name = e_vcard_attribute_get_name (attribute);

    if (attr_name == EVC_TEL || attr_name ==  EVC_X_VIDEO_URL) {

      GList *params = e_vcard_attribute_get_params (attribute);

      for (GList *param_ptr = params;
	   param_ptr != NULL;
	   param_ptr = g_list_next (param_ptr)) {

	EVCardAttributeParam *param = (EVCardAttributeParam *)param_ptr->data;
	std::string param_name = e_vcard_attribute_param_get_name (param);

	if (param_name == "TYPE") {

	  GList *types = e_vcard_attribute_param_get_values (param);

	  for (GList *type_ptr = types ;
	       type_ptr != NULL;
	       type_ptr = g_list_next (type_ptr)) {

	    std::string type_name = (const gchar *)type_ptr->data;
	    GList *values = e_vcard_attribute_get_values_decoded (attribute);
	    for (GList *value_ptr = values;
		 value_ptr != NULL;
		 value_ptr = g_list_next (value_ptr)) {

	      std::string number = ((GString *)value_ptr->data)->str;
	      uris[type_name] = number;
	    }
	  }
	}
      }
    }
  }

  updated.emit ();
}


void
Evolution::Contact::remove ()
{
  e_book_remove_contact (book, id.c_str (), NULL);
}

bool
Evolution::Contact::populate_menu (Ekiga::MenuBuilder &builder)
{
  Ekiga::ContactCore *core = dynamic_cast<Ekiga::ContactCore *>(services.get ("contact-core"));
  bool populated = false;

  if (core != NULL)
    populated = core->populate_contact_menu (*this, builder);

  if (populated)
    builder.add_separator ();

  builder.add_action ("remove", _("_Remove"),
		      sigc::mem_fun (this, &Evolution::Contact::remove));
  builder.add_action ("edit", _("_Edit"),
		      sigc::mem_fun (this, &Evolution::Contact::edit_action));

  return true;
}

void
Evolution::Contact::commit (const std::map<EContactField, std::string> data)
{
  EContact *econtact = NULL;

  if (e_book_get_contact (book, id.c_str (), &econtact, NULL)) {

    for (std::map<EContactField, std::string>::const_iterator iter
	   = data.begin ();
	 iter != data.end ();
	 iter++)
      e_contact_set (econtact, iter->first,
		     (void *)iter->second.c_str ()); // why is this cast there?
    e_book_commit_contact (book, econtact, NULL);
  }
}

void
Evolution::Contact::edit_action ()
{
  Ekiga::FormRequestSimple request;

  request.title (_("Edit contact"));

  request.instructions (_("Please update the following fields:"));

  request.text ("name", _("Name:"), get_name ());

  {
    std::string home_uri;
    std::string cell_phone_uri;
    std::string work_uri;
    std::string pager_uri;
    std::string video_uri;

    for (std::map<std::string, std::string>::const_iterator iter
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

    request.text ("video", _("VoIP _URI:"), video_uri);
    request.text ("home", _("_Home phone:"), home_uri);
    request.text ("work", _("_Office phone:"), work_uri);
    request.text ("cell phone", _("_Cell phone:"), cell_phone_uri);
    request.text ("pager", _("_Pager:"), pager_uri);
  }

  request.submitted.connect (sigc::mem_fun (this,
					    &Evolution::Contact::on_edit_form_submitted));

  if (!questions.handle_request (&request)) {

    // FIXME: better error reporting
#ifdef __GNUC__
    std::cout << "Unhandled form request in "
	      << __PRETTY_FUNCTION__ << std::endl;
#endif
  }
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

    commit (data);

  } catch (Ekiga::Form::not_found) {

    std::cerr << "Invalid result form" << std::endl; // FIXME: do better
  }
}
