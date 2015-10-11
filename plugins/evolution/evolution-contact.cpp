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
 *                         evolution-contact.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of an evolution contact
 *
 */

#include <glib/gi18n.h>

#include "evolution-contact.h"
#include "form-request-simple.h"

/* at one point we will return a smart pointer on this... and if we don't use
 * a false smart pointer, we will crash : the reference count isn't embedded!
 */
struct null_deleter
{
    void operator()(void const *) const
    {
    }
};


boost::shared_ptr<Evolution::Contact>
Evolution::Contact::create (Ekiga::ServiceCore &_services,
                            EBook *ebook,
                            EContact *econtact)
{
  boost::shared_ptr<Evolution::Contact> contact = boost::shared_ptr<Evolution::Contact> (new Evolution::Contact (_services, ebook));

  if (E_IS_CONTACT (econtact))
    contact->update_econtact (econtact);

  return contact;
}

Evolution::Contact::Contact (Ekiga::ServiceCore &_services,
			     EBook *ebook) : services(_services),
                                             book(ebook),
                                             econtact(NULL)
{
  for (unsigned int ii = 0;
       ii < ATTR_NUMBER;
       ii++)
    attributes[ii] = NULL;


  /* Actor stuff */
  add_action (Ekiga::ActionPtr (new Ekiga::Action ("edit-contact", _("_Edit"),
                                                   boost::bind (&Evolution::Contact::edit_action, this))));
  add_action (Ekiga::ActionPtr (new Ekiga::Action ("remove-contact", _("_Remove"),
                                                   boost::bind (&Evolution::Contact::remove_action, this))));
}

Evolution::Contact::~Contact ()
{
  if (E_IS_CONTACT (econtact))
    g_object_unref (econtact);
}

const std::string
Evolution::Contact::get_id () const
{
  std::string id;

  id = (gchar *)e_contact_get_const (econtact, E_CONTACT_UID);

  return id;
}

const std::string
Evolution::Contact::get_name () const
{
  std::string name;

  name = (const gchar *)e_contact_get_const (econtact,
					     E_CONTACT_FULL_NAME);

  return name;
}

bool
Evolution::Contact::has_uri (const std::string uri) const
{
  return (get_attribute_value (ATTR_HOME) == uri
	  || get_attribute_value (ATTR_CELL) == uri
	  || get_attribute_value (ATTR_WORK) == uri
	  || get_attribute_value (ATTR_PAGER) == uri
	  || get_attribute_value (ATTR_VIDEO) == uri);
}

void
Evolution::Contact::update_econtact (EContact *_econtact)
{
  GList *attrs = NULL;
  unsigned int attr_type = 0;

  if (E_IS_CONTACT (econtact))
    g_object_unref (econtact);

  econtact = _econtact;
  g_object_ref (econtact);

  for (unsigned int ii = 0;
       ii < ATTR_NUMBER;
       ii++)
    attributes[ii] = NULL;

  attrs = e_vcard_get_attributes (E_VCARD (econtact));

  boost::shared_ptr<Ekiga::ContactCore> core = services.get<Ekiga::ContactCore> ("contact-core");

  for (GList *attribute_ptr = attrs ;
       attribute_ptr != NULL;
       attribute_ptr = g_list_next (attribute_ptr)) {

    EVCardAttribute *attribute = (EVCardAttribute *)attribute_ptr->data;
    std::string attr_name = e_vcard_attribute_get_name (attribute);

    if (attr_name == EVC_TEL) {

      GList *params = e_vcard_attribute_get_params (attribute);
      for (GList *param_ptr = params;
	   param_ptr != NULL;
	   param_ptr = g_list_next (param_ptr)) {

	EVCardAttributeParam *param = (EVCardAttributeParam *)param_ptr->data;
	const gchar *param_name_raw = NULL;
	gchar *param_name_cased = NULL;
	std::string param_name;

	param_name_raw = e_vcard_attribute_param_get_name (param);
	param_name_cased = g_utf8_strup (param_name_raw, -1);
	param_name = param_name_cased;
	g_free (param_name_cased);

	if (param_name == "TYPE") {

	  for (GList *type_ptr = e_vcard_attribute_param_get_values (param);
	       type_ptr != NULL;
	       type_ptr = g_list_next (type_ptr)) {

	    const gchar *type_name_raw = NULL;
	    gchar *type_name_cased = NULL;
	    std::string type_name;

	    type_name_raw = (const gchar *)type_ptr->data;
	    type_name_cased = g_utf8_strup (type_name_raw, -1);
	    type_name = type_name_cased;
	    g_free (type_name_cased);

	    if (type_name == "HOME") {

	      attributes[ATTR_HOME] = attribute;
              attr_type = ATTR_HOME;
	    }
            else if (type_name == "CELL") {

	      attributes[ATTR_CELL] = attribute;
              attr_type = ATTR_CELL;
	    }
            else if (type_name == "WORK") {

	      attributes[ATTR_WORK] = attribute;
              attr_type = ATTR_WORK;
	    }
            else if (type_name == "PAGER") {

	      attributes[ATTR_PAGER] = attribute;
              attr_type = ATTR_PAGER;
	    }
            else if (type_name == "VIDEO") {

	      attributes[ATTR_VIDEO] = attribute;
              attr_type = ATTR_VIDEO;
	    }
            if (core)
              core->pull_actions (*this, get_name (), get_attribute_value (attr_type));
	  }
	}
      }
    }
  }

  updated (this->shared_from_this ());
}

void
Evolution::Contact::remove ()
{
  e_book_remove_contact (book, get_id().c_str (), NULL);
}

std::string
Evolution::Contact::get_attribute_name_from_type (unsigned int attribute_type) const
{
  std::string result;

  switch (attribute_type) {

  case ATTR_HOME:
    result = "HOME";
    break;
  case ATTR_CELL:
    result = "CELL";
    break;
  case ATTR_WORK:
    result = "WORK";
    break;
  case ATTR_PAGER:
    result = "PAGER";
    break;
  case ATTR_VIDEO:
    result = "VIDEO";
    break;
  default:
    result = "";
    break;
  }

  return result;
}

std::string
Evolution::Contact::get_attribute_value (unsigned int attr_type) const
{
  EVCardAttribute *attribute = attributes[attr_type];

  if (attribute != NULL) {

    GList *values = e_vcard_attribute_get_values_decoded (attribute);
    if (values != NULL)
      return ((GString *)values->data)->str; // only the first
    else
      return "";
  } else
    return "";
}

void
Evolution::Contact::set_attribute_value (unsigned int attr_type,
					 const std::string value)
{
  EVCardAttribute *attribute = attributes[attr_type];

  if ( !value.empty ()) {

    if (attribute == NULL) {

      EVCardAttributeParam *param = NULL;

      attribute = e_vcard_attribute_new ("", EVC_TEL);
      param = e_vcard_attribute_param_new (EVC_TYPE);
      e_vcard_attribute_param_add_value (param,
					 get_attribute_name_from_type (attr_type).c_str ());
      e_vcard_attribute_add_param (attribute, param);
      e_vcard_add_attribute (E_VCARD (econtact), attribute);

      attributes[attr_type]=attribute;
    }
    e_vcard_attribute_remove_values (attribute);
    e_vcard_attribute_add_value (attribute, value.c_str ());
  } else { // empty valued : remove the attribute

    if (attribute != NULL)
      e_vcard_remove_attribute (E_VCARD (econtact), attribute);
    attributes[attr_type] = NULL;
  }
}

void
Evolution::Contact::edit_action ()
{
  boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple> (new Ekiga::FormRequestSimple (boost::bind (&Evolution::Contact::on_edit_form_submitted, this, _1, _2, _3)));;

  /* Translators: This is Edit name of the contact
   * e.g. Editing Damien SANDRAS details
   */
  char *title = g_strdup_printf (_("Editing %s details"), get_name ().c_str ());
  request->title (title);
  g_free (title);
  request->text ("name", _("_Name"), get_name (), _("John Doe"));

  {
    std::string home_uri = get_attribute_value (ATTR_HOME);
    std::string cell_phone_uri = get_attribute_value (ATTR_CELL);
    std::string work_uri = get_attribute_value (ATTR_WORK);
    std::string pager_uri = get_attribute_value (ATTR_PAGER);
    std::string video_uri = get_attribute_value (ATTR_VIDEO);

    request->text ("video", _("_URI"), video_uri,
                   _("sip:john.doe@ekiga.net"), Ekiga::FormVisitor::URI);
    request->text ("home", _("_Home Phone"), home_uri,
                   _("+3268123456"), Ekiga::FormVisitor::PHONE_NUMBER);
    request->text ("work", _("_Office Phone"), work_uri,
                   _("+3268123456"), Ekiga::FormVisitor::PHONE_NUMBER);
    request->text ("cell", _("_Cell Phone"), cell_phone_uri,
                   _("+3268123456"), Ekiga::FormVisitor::PHONE_NUMBER);
    request->text ("pager", _("_Pager"), pager_uri,
                   _("+3268123456"), Ekiga::FormVisitor::PHONE_NUMBER);
  }

  questions (request);
}

bool
Evolution::Contact::on_edit_form_submitted (bool submitted,
					    Ekiga::Form &result,
                                            std::string &/*error*/)
{
  if (!submitted)
    return false;

  std::string name = result.text ("name");
  std::string home = result.text ("home");
  std::string cell = result.text ("cell");
  std::string work = result.text ("work");
  std::string pager = result.text ("pager");
  std::string video = result.text ("video");

  set_attribute_value (ATTR_HOME, home);
  set_attribute_value (ATTR_CELL, cell);
  set_attribute_value (ATTR_WORK, work);
  set_attribute_value (ATTR_PAGER, pager);
  set_attribute_value (ATTR_VIDEO, video);

  e_contact_set (econtact, E_CONTACT_FULL_NAME, (gpointer)name.c_str ());

  e_book_commit_contact (book, econtact, NULL);

  return true;
}

void
Evolution::Contact::remove_action ()
{
  boost::shared_ptr<Ekiga::FormRequestSimple> request = boost::shared_ptr<Ekiga::FormRequestSimple>(new Ekiga::FormRequestSimple (boost::bind (&Evolution::Contact::on_remove_form_submitted, this, _1, _2, _3)));
  gchar* instructions = NULL;

  request->title (_("Remove contact"));

  instructions = g_strdup_printf (_("Are you sure you want to remove %s from the addressbook?"), get_name ().c_str ());
  request->instructions (instructions);
  g_free (instructions);

  questions (request);
}

bool
Evolution::Contact::on_remove_form_submitted (bool submitted,
					      Ekiga::Form& /*result*/,
                                              std::string& /*error*/)
{
  if (!submitted)
    return false;

  remove ();
  return true;
}
