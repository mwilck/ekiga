
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
 *                         book-impl.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of a partial implementation
 *                          of an addressbook
 *
 */

#ifndef __BOOK_IMPL_H__
#define __BOOK_IMPL_H__

#include "lister.h"
#include "book.h"



namespace Ekiga
{

/**
 * @addtogroup contacts
 * @{
 */

  /** Generic implementation for the Ekiga::Book abstract class.
   *
   * This class is there to make it easy to implement a new type of contact
   * addressbook: it will take care of implementing the external api, you
   * just have to decide when to add and remove contacts.
   *
   * It also provides basic memory management for Contacts, with the second
   * (optional) template argument:
   *  - either no management (the default);
   *  - or the contact is considered bound to one Ekiga::Book, which will
   *    trigger its destruction (using delete) when removed from it.
   *
   * You can remove a Contact from an Ekiga::Book in two ways:
   *  - either by calling the remove_contact method,
   *  - or by emission of the contact's removed signal.
   *
   * Notice that this class won't take care of removing the Contact from a
   * backend -- only from the Ekiga::Book.
   * If you want the Contact <b>deleted</b> from the real backend, then you
   * probably should have an organization like:
   *  - the contact has a 'deleted' signal ;
   *  - the book listens to this signal ;
   *  - when the signal is received, then do a remove_contact followed by
   *    calling the appropriate api function to delete the contact in your
   *    backend.
   */
  template<typename ContactType = Contact,
	   typename ObjectManagementTrait = no_object_management<ContactType> >
  class BookImpl:
    public Book,
    protected Lister<ContactType, ObjectManagementTrait>
  {

  public:

    typedef typename Lister<ContactType, ObjectManagementTrait>::iterator iterator;
    typedef typename Lister<ContactType, ObjectManagementTrait>::const_iterator const_iterator;

    /** The constructor
     */
    BookImpl ();

    /** The destructor.
     */
    ~BookImpl ();


    /** Visit all contacts of the book and trigger the given callback.
     * @param The callback.
     */
    void visit_contacts (sigc::slot<void, Contact &> visitor);

  protected:

    /** More STL-like ways to access the contacts within this Ekiga::BookImpl
     *
     */
    iterator begin ();
    iterator end ();
    const_iterator begin () const;
    const_iterator end () const;

    /** Adds a contact to the Ekiga::Book.
     * @param: The contact to be added.
     * @return: The Ekiga::Book 'contact_added' signal is emitted when the contact
     * has been added. The Ekiga::Book 'contact_updated' signal will be emitted
     * when the contact has been updated and the Ekiga::Book 'contact_removed' signal
     * will be emitted when the contact has been removed from the Ekiga::Book.
     */
    void add_contact (ContactType &contact);


    /** Removes a contact from the Ekiga::Book.
     * @param: The contact to be removed.
     * @return: The Ekiga::Book 'contact_removed' signal is emitted when the contact
     * has been removed. The ContactManagementTrait associated with the Ekiga::Book
     * will determine the memory management policy for that contact.
     */
    void remove_contact (ContactType &contact);


    /** Get the current status.
     * This function is purely virtual and should be implemented by
     * the descendant of the Ekiga::Book, ie BookImpl or one
     * of its descendant.
     */
    std::string get_status ();

    std::string status;

  };

/**
 * @}
 */

};


/* here begins the code from the template functions */

template<typename ContactType, typename ContactManagementTrait>
Ekiga::BookImpl<ContactType, ContactManagementTrait>::BookImpl ()
{
  /* this is signal forwarding */
  Lister<ContactType,ContactManagementTrait>::object_added.connect (contact_added.make_slot ());
  Lister<ContactType,ContactManagementTrait>::object_removed.connect (contact_removed.make_slot ());
  Lister<ContactType,ContactManagementTrait>::object_updated.connect (contact_updated.make_slot ());
}


template<typename ContactType, typename ContactManagementTrait>
Ekiga::BookImpl<ContactType, ContactManagementTrait>::~BookImpl ()
{
}


template<typename ContactType, typename ContactManagementTrait>
void
Ekiga::BookImpl<ContactType, ContactManagementTrait>::visit_contacts (sigc::slot<void, Contact &> visitor)
{
  for (iterator iter = begin (); iter != end (); iter++)
    visitor (*iter);
}


template<typename ContactType, typename ContactManagementTrait>
typename Ekiga::BookImpl<ContactType, ContactManagementTrait>::iterator
Ekiga::BookImpl<ContactType, ContactManagementTrait>::begin ()
{
  return Lister<ContactType, ContactManagementTrait>::begin ();
}


template<typename ContactType, typename ContactManagementTrait>
typename Ekiga::BookImpl<ContactType, ContactManagementTrait>::iterator
Ekiga::BookImpl<ContactType, ContactManagementTrait>::end ()
{
  return Lister<ContactType, ContactManagementTrait>::end ();
}


template<typename ContactType, typename ContactManagementTrait>
typename Ekiga::BookImpl<ContactType, ContactManagementTrait>::const_iterator
Ekiga::BookImpl<ContactType, ContactManagementTrait>::begin () const
{
  return Lister<ContactType, ContactManagementTrait>::begin ();
}


template<typename ContactType, typename ContactManagementTrait>
typename Ekiga::BookImpl<ContactType, ContactManagementTrait>::const_iterator
Ekiga::BookImpl<ContactType, ContactManagementTrait>::end () const
{
  return Lister<ContactType, ContactManagementTrait>::end ();
}


template<typename ContactType, typename ContactManagementTrait>
void
Ekiga::BookImpl<ContactType, ContactManagementTrait>::add_contact (ContactType &contact)
{
  contact.questions.add_handler (questions.make_slot ());
  add_object (contact);
}


template<typename ContactType, typename ContactManagementTrait>
void
Ekiga::BookImpl<ContactType, ContactManagementTrait>::remove_contact (ContactType &contact)
{
  remove_object (contact);
}


template<typename ContactType, typename ContactManagementTrait>
std::string
Ekiga::BookImpl<ContactType, ContactManagementTrait>::get_status ()
{
  return status;
}

#endif
