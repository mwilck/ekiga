
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
 *                         evolution-book.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of an evolution addressbook
 *
 */

#include <iostream>

#include "config.h"

#include "ui.h"
#include "form-request-simple.h"

#include "evolution-book.h"
#include "evolution-contact.h"

/* this structure is so that when we give ourself to a C callback,
 * the object lives long enough
 */
struct Wrapper
{
  Wrapper (Evolution::Book &_book): book(_book) {}

  Evolution::Book &book;
};

static void
on_view_contacts_added_c (EBook * /*ebook*/,
			  GList *contacts,
			  gpointer data)
{
  ((Evolution::Book *)data)->on_view_contacts_added (contacts);
}

void
Evolution::Book::on_view_contacts_added (GList *econtacts)
{
  EContact *econtact = NULL;

  for (; econtacts != NULL; econtacts = g_list_next (econtacts)) {

    econtact = E_CONTACT (econtacts->data);

    if (e_contact_get_const (econtact, E_CONTACT_FULL_NAME) != NULL) {

      Evolution::Contact *contact =  new Evolution::Contact (services, econtact);

      add_contact (*contact);
      contact->remove_me.connect (sigc::bind (sigc::mem_fun (this, &Evolution::Book::on_remove_me), contact));
      contact->commit_me.connect (sigc::bind (sigc::mem_fun (this, &Evolution::Book::on_commit_me), contact));
    }
  }
}

static void
on_view_contacts_removed_c (EBook */*ebook*/,
			    GList *ids,
			    gpointer data)
{
  ((Evolution::Book *)data)->on_view_contacts_removed (ids);
}

void
Evolution::Book::on_view_contacts_removed (GList *ids)
{
  for (; ids != NULL; ids = g_list_next (ids))
    for (iterator iter = begin ();
	 iter != end ();
	 iter++)
      if (iter->get_id () == (gchar *)ids->data) {

	remove_contact (*iter);
	break; // will do the loop on ids, but stop using iter which is invalid
      }
}

static void
on_view_contacts_changed_c (EBook */*ebook*/,
			    GList *econtacts,
			    gpointer data)
{
  ((Evolution::Book *)data)->on_view_contacts_changed (econtacts);
}

void
Evolution::Book::on_view_contacts_changed (GList *econtacts)
{
  EContact *econtact = NULL;

  for (; econtacts != NULL; econtacts = g_list_next (econtacts)) {

    econtact = E_CONTACT (econtacts->data);

    for (iterator iter = begin ();
	 iter != end ();
	 iter++)

      if (iter->get_id()
	  == (const gchar *)e_contact_get_const (econtact, E_CONTACT_UID))
	iter->update_econtact (econtact);
  }
}

static void
on_book_view_obtained_c (EBook */*book*/,
			 EBookStatus status,
			 EBookView *view,
			 gpointer data)
{
  ((Wrapper *)data)->book.on_book_view_obtained (status, view);

  delete ((Wrapper *)data);
}

void
Evolution::Book::on_book_view_obtained (EBookStatus status,
					EBookView *_view)
{
  if (status == E_BOOK_ERROR_OK) {

    view = _view;

    /* no need for wrappers here : the view will only die with us */
    g_object_ref (view);

    g_signal_connect (view, "contacts-added",
		      G_CALLBACK (on_view_contacts_added_c), this);

    g_signal_connect (view, "contacts-removed",
		      G_CALLBACK (on_view_contacts_removed_c), this);

    g_signal_connect (view, "contacts-changed",
		      G_CALLBACK (on_view_contacts_changed_c), this);

    e_book_view_start (view);
  } else
    removed.emit ();
}

static void
on_book_opened_c (EBook */*book*/,
		  EBookStatus status,
		  gpointer data)
{
  ((Wrapper *)data)->book.on_book_opened (status);

  delete ((Wrapper *)data);
}

void
Evolution::Book::on_book_opened (EBookStatus status)
{
  Wrapper *self = new Wrapper (*this);

  if (status == E_BOOK_ERROR_OK) {

    (void)e_book_async_get_book_view (book, query, NULL, 100,
				      on_book_view_obtained_c, self);
  } else {

    book = NULL;
    removed.emit ();
  }
}

Evolution::Book::Book (Ekiga::ServiceCore &_services,
		       EBook *_book,
		       EBookQuery *_query)
  : services(_services), book(_book), query(_query)
{
  Wrapper *self = new Wrapper (*this);

  g_object_ref (book);

  if (e_book_is_opened (book))
    on_book_opened_c (book, E_BOOK_ERROR_OK, self);
  else
    e_book_async_open (book, TRUE,
		       on_book_opened_c, self);
}

Evolution::Book::~Book ()
{
  if (book != NULL)
    g_object_unref (book);
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

const std::string
Evolution::Book::get_name () const
{
  ESource *source = NULL;
  std::string result;

  source = e_book_get_source (book);
  result = e_source_peek_name (source);
  g_object_unref (source);

  return result;
}

EBook *
Evolution::Book::get_ebook () const
{
  return book;
}

bool
Evolution::Book::populate_menu (Ekiga::MenuBuilder &builder)
{
  bool populated = false;
  Ekiga::UI *ui = dynamic_cast<Ekiga::UI *>(services.get ("ui"));

  if (ui != NULL) {
    builder.add_action ("new",
                        _("New contact"),
			sigc::mem_fun (this,
				       &Evolution::Book::new_contact_action));
    populated = true;
  }

  return populated;
}

void
Evolution::Book::new_contact_action ()
{
  Ekiga::UI *ui = dynamic_cast<Ekiga::UI *>(services.get ("ui"));
  Ekiga::FormRequestSimple request;

  request.title ("New contact");

  request.instructions ("Please edit the following fields\n(groups are comma-separated)");

  request.text ("name",
		e_contact_pretty_name (E_CONTACT_FULL_NAME), "");

  request.text ("home",
		e_contact_pretty_name (E_CONTACT_PHONE_HOME), "");
  request.text ("cell phone",
		e_contact_pretty_name (E_CONTACT_PHONE_MOBILE), "");
  request.text ("work",
		e_contact_pretty_name (E_CONTACT_PHONE_BUSINESS), "");
  request.text ("pager",
		e_contact_pretty_name (E_CONTACT_PHONE_PAGER), "");
  request.text ("video",
		e_contact_pretty_name (E_CONTACT_VIDEO_URL), "");

  request.text ("groups", "Groups", "");

  request.submitted.connect (sigc::mem_fun (this,
					    &Evolution::Book::on_new_contact_form_submitted));

  ui->run_form_request (request);

}

void
Evolution::Book::on_new_contact_form_submitted (Ekiga::Form &result)
{
  try {

    EContact *econtact = NULL;
    std::map<EContactField, std::string> data;

    /* first check we have everything before using */
    data[E_CONTACT_FULL_NAME] = result.text ("name");
    data[E_CONTACT_PHONE_HOME] = result.text ("home");
    data[E_CONTACT_PHONE_MOBILE] = result.text ("cell phone");
    data[E_CONTACT_PHONE_BUSINESS] = result.text ("work");
    data[E_CONTACT_PHONE_PAGER] = result.text ("pager");
    data[E_CONTACT_VIDEO_URL] = result.text ("video");
    data[E_CONTACT_CATEGORIES] = result.text ("groups");

    econtact = e_contact_new ();
    for (std::map<EContactField, std::string>::const_iterator iter
	   = data.begin ();
	 iter != data.end ();
	 iter++)
      if (!iter->second.empty ())
	e_contact_set (econtact, iter->first,
		       (void *)iter->second.c_str ()); // why is this cast there?
    e_book_add_contact (book, econtact, NULL);
    g_object_unref (econtact);

  } catch (Ekiga::Form::not_found) {
#ifdef __GNUC__
    std::cerr << "Invalid result form submitted to "
	      << __PRETTY_FUNCTION__
	      << std::endl;
#endif
  }
}


void
Evolution::Book::on_remove_me (Evolution::Contact *contact)
{
  gchar *id = g_strdup (contact->get_id ().c_str ());

  e_book_remove_contact (book, id, NULL);

  g_free (id);
}

void
Evolution::Book::on_commit_me (const std::map<EContactField, std::string> data,
			       Evolution::Contact *contact)
{
  gchar *id = g_strdup (contact->get_id ().c_str ());
  EContact *econtact = NULL;

  if (e_book_get_contact (book, id, &econtact, NULL)) {

    for (std::map<EContactField, std::string>::const_iterator iter
	   = data.begin ();
	 iter != data.end ();
	 iter++)
      e_contact_set (econtact, iter->first,
		     (void *)iter->second.c_str ()); // why is this cast there?
    e_book_commit_contact (book, econtact, NULL);
  }

  g_free (id);
}
