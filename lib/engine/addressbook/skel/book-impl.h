
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

#include "book.h"
#include "map-key-reference-iterator.h"
#include "map-key-const-reference-iterator.h"


/** Ekiga::Book
 *
 * This class is there to make it easy to implement a new type of contact
 * addressbook : it will take care of implementing the external api, you
 * just have to decide when to add and remove contacts.
 *
 * It also provides basic memory management for contacts, with the second
 * (optional) template argument :
 * - either no management (the default) ;
 * - or the contact is considered bound to one Ekiga::Book, which will trigger its
 * destruction (using delete) when removed from it.
 *
 * You can remove a contact from an Ekiga::Book in two ways : 
 * - either by calling the remove_contact method, 
 * - or by emission of the contact's removed signal.
 *
 * Notice that this class won't take care of removing the contact from a
 * backend -- only from the Ekiga::Book. 
 * If you want the contact *deleted* from the real backend, then you
 * probably should have an organization like :
 * - the contact has a 'deleted' signal ;
 * - the book listens to this signal ;
 * - when the signal is received, then do a remove_contact followed by calling
 * the appropriate api function to delete the contact in your backend.
 */

#define connection_pair std::pair<sigc::connection, sigc::connection>


namespace Ekiga {

  template<typename ContactType>
    struct no_contact_management
      {
        static void announced_release (ContactType &);

        static void release (ContactType &);
      };

  template<typename ContactType>
    struct delete_contact_management
      {
        static void announced_release (ContactType &contact);

        static void release (ContactType &contact);

      };

  template<typename ContactType = Contact,
    typename ContactManagementTrait = no_contact_management<ContactType> >
      class BookImpl: public Book
      {

    public:

        typedef MapKeyReferenceIterator<ContactType, connection_pair> iterator;
        typedef MapKeyConstReferenceIterator<ContactType, connection_pair> const_iterator;


        /** The destructor. 
        */
        ~BookImpl ();


        /** Visit all contacts of the book and trigger the given callback.
         * @param The callback.
         */
        void visit_contacts (sigc::slot<void, Contact &> visitor);


        /** Returns a const iterator to the first Contact of the collection.
        */
        const_iterator begin () const;


        /** Returns an iterator to the first Contact of the collection.
        */
        iterator begin ();


        /** Returns a const iterator to the first Contact of the collection.
        */
        const_iterator end () const;


        /** Returns an iterator to the last Contact of the collection.
        */
        iterator end ();


    protected:

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


    private:

        /** Disconnects the signals for the contact, emits the 'contact_removed' signal on the 
         * Ekiga::Book and takes care of the release of that contact following the policy of
         * the ContactManagementTrait associated with the Ekiga::Book.
         * @param: The contact to remove.
         */
        void common_removal_steps (ContactType &contact);


        /** This callback is triggered when the 'updated' signal is emitted on a contact.
         * Emits the Ekiga::Book 'contact_updated' signal for that contact.
         * @param: The updated contact.
         */
        void on_contact_updated (ContactType *contact);


        /** This callback is triggered when the 'removed' signal is emitted on a contact.
         * Emits the Ekiga::Book 'contact_removed' signal for that contact and takes
         * care of the deletion of the contact or not following the ContactManagementTrait
         * associated with the Ekiga::Book.
         * @param: The updated contact.
         */
        void on_contact_removed (ContactType *contact);

        /** Map of contacts and signals.
        */
        std::map<ContactType *, connection_pair> connections;
      };
};


/* here begins the code from the template functions */
template<typename ContactType>
void
Ekiga::no_contact_management<ContactType>::announced_release (ContactType &contact)
{
  // nothing
}


template<typename ContactType>
void
Ekiga::no_contact_management<ContactType>::release (ContactType &contact)
{
  // nothing
}


template<typename ContactType>
void
Ekiga::delete_contact_management<ContactType>::announced_release (ContactType &contact)
{
  contact.removed.emit ();
  release (contact);
}


template<typename ContactType>
void
Ekiga::delete_contact_management<ContactType>::release (ContactType &contact)
{
  delete &contact;
}


template<typename ContactType, typename ContactManagementTrait>
Ekiga::BookImpl<ContactType, ContactManagementTrait>::~BookImpl ()
{
  iterator iter = begin ();

  while (iter != end ()) {

    remove_contact (*iter); // here iter becomes invalid
    iter = begin ();
  }
}


template<typename ContactType, typename ContactManagementTrait>
void
Ekiga::BookImpl<ContactType, ContactManagementTrait>::visit_contacts (sigc::slot<void, Contact &> visitor)
{
  for (iterator iter = begin (); iter != end (); iter++)
    visitor (*iter);
}


template<typename ContactType, typename ContactManagementTrait>
typename Ekiga::BookImpl<ContactType, ContactManagementTrait>::const_iterator
Ekiga::BookImpl<ContactType, ContactManagementTrait>::begin () const
{
  return const_iterator (connections.begin ());
}


template<typename ContactType, typename ContactManagementTrait>
typename Ekiga::BookImpl<ContactType, ContactManagementTrait>::iterator
Ekiga::BookImpl<ContactType, ContactManagementTrait>::begin ()
{
  return iterator (connections.begin ());
}


template<typename ContactType, typename ContactManagementTrait>
typename Ekiga::BookImpl<ContactType, ContactManagementTrait>::const_iterator
Ekiga::BookImpl<ContactType, ContactManagementTrait>::end () const
{
  return const_iterator (connections.end ());
}


template<typename ContactType, typename ContactManagementTrait>
typename Ekiga::BookImpl<ContactType, ContactManagementTrait>::iterator
Ekiga::BookImpl<ContactType, ContactManagementTrait>::end ()
{
  return iterator (connections.end ());
}


template<typename ContactType, typename ContactManagementTrait>
void
Ekiga::BookImpl<ContactType, ContactManagementTrait>::add_contact (ContactType &contact)
{
  sigc::connection rem_conn = contact.removed.connect (sigc::bind (sigc::mem_fun (this, &BookImpl::on_contact_removed), &contact));
  sigc::connection upd_conn = contact.updated.connect (sigc::bind (sigc::mem_fun (this, &BookImpl::on_contact_updated), &contact));
  connections[&contact] = connection_pair (rem_conn, upd_conn);
  contact_added (contact);
}


template<typename ContactType, typename ContactManagementTrait>
void
Ekiga::BookImpl<ContactType, ContactManagementTrait>::remove_contact (ContactType &contact)
{
  common_removal_steps (contact);
  ContactManagementTrait::announced_release (contact);
}


template<typename ContactType, typename ContactManagementTrait>
void
Ekiga::BookImpl<ContactType, ContactManagementTrait>::common_removal_steps (ContactType &contact)
{
  connection_pair conns = connections[&contact];
  conns.first.disconnect ();
  conns.second.disconnect ();
  connections.erase (&contact);
  contact_removed.emit (contact);
}


template<typename ContactType, typename ContactManagementTrait>
void
Ekiga::BookImpl<ContactType, ContactManagementTrait>::on_contact_updated (ContactType *contact)
{
  contact_updated.emit (*contact);
}


template<typename ContactType, typename ContactManagementTrait>
void
Ekiga::BookImpl<ContactType, ContactManagementTrait>::on_contact_removed (ContactType *contact)
{
  common_removal_steps (*contact);
  ContactManagementTrait::release (*contact);
}

#endif
