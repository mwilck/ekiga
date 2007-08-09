
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras

 * This program is free software; you can  redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ekiga is licensed under the GPL license and as a special exception, you
 * have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OPAL, OpenH323 and PWLIB
 * programs, as long as you do follow the requirements of the GNU GPL for all
 * the rest of the software thus combined.
 */


/*
 *                         form-builder.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of an object able to build a form
 *
 */

#include "form-builder.h"

Ekiga::FormBuilder::FormBuilder ()
{
  // nothing
}

void
Ekiga::FormBuilder::visit (Ekiga::FormVisitor &visitor) const
{
  std::list<struct HiddenField>::const_iterator iter_hidden = hiddens.begin ();
  std::list<struct BooleanField>::const_iterator iter_bool = booleans.begin ();
  std::list<struct TextField>::const_iterator iter_text = texts.begin ();
  std::list<struct TextField>::const_iterator iter_private_text = private_texts.begin ();
  std::list<struct MultiTextField>::const_iterator iter_multi_text = multi_texts.begin ();
  std::list<struct SingleListField>::const_iterator iter_single_list = single_lists.begin ();
  std::list<struct MultipleListField>::const_iterator iter_multiple_list = multiple_lists.begin ();

  visitor.title (my_title);
  visitor.instructions (my_instructions);
  visitor.error (my_error);

  for (std::list<FieldType>::const_iterator iter = ordering.begin ();
       iter != ordering.end ();
       iter++) {

    switch (*iter) {

    case HIDDEN:

      visitor.hidden (iter_hidden->name, iter_hidden->value);
      iter_hidden++;
      break;

    case BOOLEAN:

      visitor.boolean (iter_bool->name,
		       iter_bool->description,
		       iter_bool->value);
      iter_bool++;
      break;

    case TEXT:

      visitor.text (iter_text->name, iter_text->description, iter_text->value);
      iter_text++;
      break;

    case PRIVATE_TEXT:

      visitor.private_text (iter_private_text->name,
			    iter_private_text->description,
			    iter_private_text->value);
      iter_private_text++;
      break;

    case MULTI_TEXT:

      visitor.multi_text (iter_multi_text->name,
			  iter_multi_text->description,
			  iter_multi_text->value);
      iter_multi_text++;
      break;

    case SINGLE_LIST:

      visitor.single_list (iter_single_list->name,
			   iter_single_list->description,
			   iter_single_list->value,
			   iter_single_list->choices);
      iter_single_list++;
      break;

    case MULTIPLE_LIST:

      visitor.multiple_list (iter_multiple_list->name,
			     iter_multiple_list->description,
			     iter_multiple_list->values,
			     iter_multiple_list->choices);
      iter_multiple_list++;
      break;
    }
  }
}

const std::string
Ekiga::FormBuilder::hidden (const std::string name) const
{
  for (std::list<struct HiddenField>::const_iterator iter = hiddens.begin ();
       iter != hiddens.end ();
       iter++)
    if (iter->name == name)
      return iter->value;

  throw Ekiga::Form::not_found ();
}

bool
Ekiga::FormBuilder::boolean (const std::string name) const
{
  for (std::list<struct BooleanField>::const_iterator iter = booleans.begin ();
       iter != booleans.end ();
       iter++)
    if (iter->name == name)
      return iter->value;

  throw Ekiga::Form::not_found ();
}

const std::string
Ekiga::FormBuilder::private_text (const std::string name) const
{
  for (std::list<struct TextField>::const_iterator iter = private_texts.begin ();
       iter != private_texts.end ();
       iter++)
    if (iter->name == name)
      return iter->value;

  throw Ekiga::Form::not_found ();
}

const std::string
Ekiga::FormBuilder::text (const std::string name) const
{
  for (std::list<struct TextField>::const_iterator iter = texts.begin ();
       iter != texts.end ();
       iter++)
    if (iter->name == name)
      return iter->value;

  throw Ekiga::Form::not_found ();
}

const std::string
Ekiga::FormBuilder::multi_text (const std::string name) const
{
  for (std::list<struct MultiTextField>::const_iterator iter = multi_texts.begin ();
       iter != multi_texts.end ();
       iter++)
    if (iter->name == name)
      return iter->value;

  throw Ekiga::Form::not_found ();
}

const std::string
Ekiga::FormBuilder::single_list (const std::string name) const
{
  for (std::list<struct SingleListField>::const_iterator iter = single_lists.begin ();
       iter != single_lists.end ();
       iter++)
    if (iter->name == name)
      return iter->value;

  throw Ekiga::Form::not_found ();
}

const std::list<std::string>
Ekiga::FormBuilder::multiple_list (const std::string name) const
{
  for (std::list<struct MultipleListField>::const_iterator iter = multiple_lists.begin ();
       iter != multiple_lists.end ();
       iter++)
    if (iter->name == name)
      return iter->values;

  throw Ekiga::Form::not_found ();
}

void
Ekiga::FormBuilder::title (const std::string _title)
{
  my_title = _title;
}

void
Ekiga::FormBuilder::instructions (const std::string _instructions)
{
  my_instructions = _instructions;
}

void
Ekiga::FormBuilder::error (const std::string _error)
{
  my_error = _error;
}

void
Ekiga::FormBuilder::hidden (const std::string name,
			    const std::string value)
{
  hiddens.push_back (HiddenField (name, value));
  ordering.push_back (HIDDEN);
}

void
Ekiga::FormBuilder::boolean (const std::string name,
			     const std::string description,
			     bool value)
{
  booleans.push_back (BooleanField (name, description, value));
  ordering.push_back (BOOLEAN);
}

void
Ekiga::FormBuilder::text (const std::string name,
			  const std::string description,
			  const std::string value)
{
  texts.push_back (TextField (name, description, value));
  ordering.push_back (TEXT);
}

void
Ekiga::FormBuilder::private_text (const std::string name,
				  const std::string description,
				  const std::string value)
{
  private_texts.push_back (TextField (name, description, value));
  ordering.push_back (PRIVATE_TEXT);
}

void
Ekiga::FormBuilder::multi_text (const std::string name,
				const std::string description,
				const std::string value)
{
  multi_texts.push_back (MultiTextField (name, description, value));
  ordering.push_back (MULTI_TEXT);
}

void
Ekiga::FormBuilder::single_list (const std::string name,
				 const std::string description,
				 const std::string value,
				 const std::map<std::string, std::string> choices)
{
  single_lists.push_back (SingleListField (name, description, value, choices));
  ordering.push_back (SINGLE_LIST);
}

void
Ekiga::FormBuilder::multiple_list (const std::string name,
				   const std::string description,
				   const std::list<std::string> values,
				   const std::map<std::string, std::string> choices)
{
  multiple_lists.push_back (MultipleListField (name, description, values, choices));
  ordering.push_back (MULTIPLE_LIST);
}
