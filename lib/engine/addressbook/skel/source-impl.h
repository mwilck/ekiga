
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
 *                         source-impl.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of the interface of a partial
 *                          implementation of an addressbook
 *                          implementation backend
 *
 */

#ifndef __SOURCE_IMPL_H__
#define __SOURCE_IMPL_H__

#include <vector>

#include "map-key-reference-iterator.h"
#include "map-key-const-reference-iterator.h"
#include "source.h"


/** Ekiga::Source
 *
 * This class is there to make it easy to implement a new type of
 * addressbook source : it will take care of implementing the external api, you
 * just have to decide when to add and remove books.
 *
 * It also provides basic memory management for books, with the second
 * (optional) template argument :
 * - either no management (the default) ;
 * - or the book is considered bound to one Ekiga::Source, which will trigger
 * its destruction (using delete) when removed from it.
 *
 * You ca remove a book from an Ekiga::Source in two ways :
 * - either by calling the remove_book method,
 * - or by emission of the book's removed signal.
 *
 * Notice that this class won't take care of removing the book from a
 * backend -- only from the Ekiga::Source.
 * If you want the book *deleted* from the real backend, then you
 * probably should have an organization like :
 * - the book has a 'deleted' signal ;
 * - the source listens for this signal ;
 * - when the signal is received, then do a remove_book followed by calling
 * the appropriate api function to delete the book in your backend.
 */

namespace Ekiga {

  template<typename BookType>
  struct no_book_management
  {
    static void announced_release (BookType &);

    static void release (BookType &);

  };

  template<typename BookType>
  struct delete_book_management
  {
    static void announced_release (BookType &book);

    static void release (BookType &book);
  };

  template<typename BookType = Book,
	   typename BookManagementTrait = no_book_management <BookType> >
  class SourceImpl: public Source
  {

  public:

    typedef MapKeyReferenceIterator<BookType,
				    std::vector<sigc::connection> > iterator;
    typedef MapKeyConstReferenceIterator<BookType,
					 std::vector<sigc::connection> > const_iterator;

    /** The destructor.
     */
    virtual ~SourceImpl ();


    /** Visit all books of the source and trigger the given callback.
     * @param The callback.
     */
    void visit_books (sigc::slot<void, Book &> visitor);


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

    /** Adds a book to the Ekiga::Source.
     * @param: The Ekiga::Book to be added.
     * @return: The Ekiga::Source 'book_added' signal is emitted when the
     * Ekiga::Book has been added. The Ekiga::Source 'book_updated' signal will
     * be emitted when the Ekiga::Book has been updated and the Ekiga::Source
     * 'book_removed' signal will be emitted when the Ekiga::Book has been
     * removed from the Ekiga::Source.
     */
    void add_book (BookType &book);


    /** Removes a book from the Ekiga::Source.
     * @param: The Ekiga::Book to be removed.
     * @return: The Ekiga::Source 'book_removed' signal is emitted when the
     * Ekiga::Book has been removed. The BookManagementTrait associated with
     * the Ekiga::Source will determine the memory management policy for that
     * Ekiga::Book.
     */
    void remove_book (BookType &book);


  private:

    /** Disconnects the signals for the Ekiga::Book, emits the 'book_removed'
     * signal on the Ekiga::Source and takes care of the release of that
     * Ekiga::Book following the policy of the BookManagementTrait associated
     * with the Ekiga::Source.
     * @param: The Book to remove.
     */
    void common_removal_steps (BookType &book);


    /** This callback is triggered when the 'updated' signal is emitted on an
     * Ekiga::Book. Emits the Ekiga::Source 'book_updated' signal for that
     * Ekiga::Book.
     * @param: The updated book.
     */
    void on_book_updated (BookType *book);


    /** This callback is triggered when the 'removed' signal is emitted on an
     * Ekiga::Book. Emits the Ekiga::Source 'book_removed' signal for that book
     * and takes care of the deletion of the book or not following the
     * BookManagementTrait associated with the Ekiga::Source.
     * @param: The removed book.
     */
    void on_book_removed (BookType *book);


    /** This callback is triggered when the 'contact_added' signal is emitted
     * on an Ekiga::Book in this source. Emits the Ekiga::Source
     * 'contact_added' signal.
     * @param: The contact and the book
     */
    void on_contact_added (Contact &contact,
			   BookType *book);


    /** This callback is triggered when the 'contact_removed' signal is emitted
     * on an Ekiga::Book in this source. Emits the Ekiga::Source
     * 'contact_removed' signal.
     * @param: The contact and the book
     */
    void on_contact_removed (Contact &contact,
			     BookType *book);


    /** This callback is triggered when the 'contact_updated' signal is emitted
     * on an Ekiga::Book in this source. Emits the Ekiga::Source
     * 'contact_updated' signal.
     * @param: The contact and the book
     */
    void on_contact_updated (Contact &contact,
			     BookType *book);


    /** Map of books and signals.
     */
    std::map<BookType *, std::vector<sigc::connection> > connections;
  };
};


/* here comes the implementation of the template functions */
template<typename BookType>
void
Ekiga::no_book_management<BookType>::announced_release (BookType &)
{
  // nothing
}


template<typename BookType>
void
Ekiga::no_book_management<BookType>::release (BookType &)
{
  // nothing
}


template<typename BookType>
void
Ekiga::delete_book_management<BookType>::announced_release (BookType &book)
{
  book.removed.emit ();
  release (book);
}


template<typename BookType>
void
Ekiga::delete_book_management<BookType>::release (BookType &book)
{
  delete &book;
}


template<typename BookType,
	 typename BookManagementTrait>
Ekiga::SourceImpl<BookType, BookManagementTrait>::~SourceImpl ()
{
  iterator iter = begin ();

  while (iter != end ()) {

    remove_book (*iter); // here iter becomes invalid
    iter = begin ();
  }
}


template<typename BookType,
	 typename BookManagementTrait>
void
Ekiga::SourceImpl<BookType,
		  BookManagementTrait>::visit_books (sigc::slot<void, Book &> visitor)
{
  for (iterator iter = begin (); iter != end (); iter++)
    visitor (*iter);
}


template<typename BookType,
	 typename BookManagementTrait>
typename Ekiga::SourceImpl<BookType, BookManagementTrait>::const_iterator
Ekiga::SourceImpl<BookType, BookManagementTrait>::begin () const
{
  return const_iterator (connections.begin ());
}


template<typename BookType,
	 typename BookManagementTrait>
typename Ekiga::SourceImpl<BookType, BookManagementTrait>::const_iterator
Ekiga::SourceImpl<BookType, BookManagementTrait>::end () const
{
  return const_iterator (connections.end ());
}


template<typename BookType,
	 typename BookManagementTrait>
typename Ekiga::SourceImpl<BookType, BookManagementTrait>::iterator
Ekiga::SourceImpl<BookType, BookManagementTrait>::begin ()
{
  return iterator (connections.begin ());
}


template<typename BookType,
	 typename BookManagementTrait>
typename Ekiga::SourceImpl<BookType, BookManagementTrait>::iterator
Ekiga::SourceImpl<BookType, BookManagementTrait>::end ()
{
  return iterator (connections.end ());
}


template<typename BookType,
	 typename BookManagementTrait>
void
Ekiga::SourceImpl<BookType, BookManagementTrait>::add_book (BookType &book)
{
  std::vector<sigc::connection> conns;
  sigc::connection conn;

  conn = book.removed.connect (sigc::bind (sigc::mem_fun (this, &SourceImpl::on_book_removed), &book));
  conns.push_back (conn);

  conn = book.updated.connect (sigc::bind (sigc::mem_fun (this, &SourceImpl::on_book_updated), &book));
  conns.push_back (conn);

  conn = book.contact_added.connect (sigc::bind (sigc::mem_fun (this, &SourceImpl::on_contact_added), &book));
  conns.push_back (conn);

  conn = book.contact_removed.connect (sigc::bind (sigc::mem_fun (this, &SourceImpl::on_contact_removed), &book));
  conns.push_back (conn);

  conn = book.contact_updated.connect (sigc::bind (sigc::mem_fun (this, &SourceImpl::on_contact_updated), &book));
  conns.push_back (conn);

  connections[&book] = conns;
  book_added.emit (book);
}


template<typename BookType,
	 typename BookManagementTrait>
void
Ekiga::SourceImpl<BookType, BookManagementTrait>::remove_book (BookType &book)
{
  common_removal_steps (book);
  BookManagementTrait::announced_release (book);
}


template<typename BookType,
	 typename BookManagementTrait>
void
Ekiga::SourceImpl<BookType,
		  BookManagementTrait>::common_removal_steps (BookType &book)
{
  std::vector<sigc::connection> conns = connections[&book];

  for (std::vector<sigc::connection>::iterator iter = conns.begin ();
       iter != conns.end ();
       iter++)
    iter->disconnect ();

  connections.erase (&book);
  book_removed.emit (book);
}


template<typename BookType,
	 typename BookManagementTrait>
void
Ekiga::SourceImpl<BookType,
		  BookManagementTrait>::on_book_updated (BookType *book)
{
  book_updated.emit (*book);
}


template<typename BookType,
	 typename BookManagementTrait>
void
Ekiga::SourceImpl<BookType,
		  BookManagementTrait>::on_book_removed (BookType *book)
{
  common_removal_steps (*book);
  BookManagementTrait::release (*book);
}


template<typename BookType,
	 typename BookManagementTrait>
void
Ekiga::SourceImpl<BookType,
		  BookManagementTrait>::on_contact_added (Contact &contact,
							  BookType *book)
{
  contact_added.emit (*book, contact);
}


template<typename BookType,
	 typename BookManagementTrait>
void
Ekiga::SourceImpl<BookType,
		  BookManagementTrait>::on_contact_removed (Contact &contact,
							    BookType *book)
{
  contact_removed.emit (*book, contact);
}


template<typename BookType,
	 typename BookManagementTrait>
void
Ekiga::SourceImpl<BookType,
		  BookManagementTrait>::on_contact_updated (Contact &contact,
							    BookType *book)
{
  contact_updated.emit (*book, contact);
}

#endif
