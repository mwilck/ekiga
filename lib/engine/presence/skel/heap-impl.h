
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
 *                         heap-impl.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of a partial implementation
 *                          of a heap
 *
 */

#ifndef __HEAP_IMPL_H__
#define __HEAP_IMPL_H__

#include "heap.h"
#include "map-key-reference-iterator.h"
#include "map-key-const-reference-iterator.h"

/* This class is there to make it easy to implement a new type of presentity
 * heap : it will take care of implementing the external api, you
 * just have to decide when to add and remove presentities.
 *
 * It also provides basic memory management for presentities, with the second
 * (optional) template argument :
 * - either no management (the default) ;
 * - or the presentity is considered bound to one heap, which will trigger its
 * destruction (using delete) when removed from it, which can happen in two
 * ways : either by calling the remove_presentity method, or by emission of the
 * presentity's removed signal.
 *
 * Notice that this class won't take care of removing the presentity from a
 * backend -- only from the heap. If you want the presentity *deleted* then you
 * probably should have an organization like :
 * - the presentity has a 'deleted' signal ;
 * - the heap listens for this signal ;
 * - when the signal is received, then do a remove_presentity followed by
 * calling the appropriate api function to delete the presentity in your
 * backend.
 */

#define connection_pair std::pair<sigc::connection, sigc::connection>


namespace Ekiga {

  template<typename PresentityType>
  struct no_presentity_management
  {
    static void announced_release (PresentityType &);

    static void release (PresentityType &);
  };

  template<typename PresentityType>
  struct delete_presentity_management
  {
    static void announced_release (PresentityType &presentity);

    static void release (PresentityType &presentity);
  };

  template<typename PresentityType = Presentity,
	   typename PresentityManagementTrait = no_presentity_management<PresentityType> >
  class HeapImpl: public Heap
    {

      public:

      typedef MapKeyReferenceIterator<PresentityType, connection_pair> iterator;
      typedef MapKeyConstReferenceIterator<PresentityType, connection_pair> const_iterator;

      ~HeapImpl ();

      void visit_presentities (sigc::slot<void, Presentity &> visitor);

      const_iterator begin () const;

      iterator begin ();

      const_iterator end () const;

      iterator end ();

      protected:

      void add_presentity (PresentityType &presentity);

      void remove_presentity (PresentityType &presentity);

      private:

      void common_removal_steps (PresentityType &presentity);

      void on_presentity_updated (PresentityType *presentity);

      void on_presentity_removed (PresentityType *presentity);

      std::map<PresentityType *, connection_pair> connections;
    };

};

/* here are the implementations of the template methods */
template<typename PresentityType>
void
Ekiga::no_presentity_management<PresentityType>::announced_release (PresentityType &)
{
  // nothing
}

template<typename PresentityType>
void
Ekiga::no_presentity_management<PresentityType>::release (PresentityType &)
{
  // nothing
}

template<typename PresentityType>
void
Ekiga::delete_presentity_management<PresentityType>::announced_release (PresentityType &presentity)
{
  presentity.removed.emit ();
  release (presentity);
}

template<typename PresentityType>
void
Ekiga::delete_presentity_management<PresentityType>::release (PresentityType &presentity)
{
  delete &presentity;
}

template<typename PresentityType, typename PresentityManagementTrait>
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::~HeapImpl ()
{
  iterator iter = begin ();

  while (iter != end ()) {

    remove_presentity (*iter); // here iter becomes invalid
    iter = begin ();
  }
}

template<typename PresentityType, typename PresentityManagementTrait>
void
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::visit_presentities (sigc::slot<void, Presentity &> visitor)
{
  for (iterator iter = begin (); iter != end (); iter++)
    visitor (*iter);
}

template<typename PresentityType, typename PresentityManagementTrait>
typename Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::const_iterator
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::begin () const
{
  return const_iterator (connections.begin ());
}

template<typename PresentityType, typename PresentityManagementTrait>
typename Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::iterator
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::begin ()
{
  return iterator (connections.begin ());
}

template<typename PresentityType, typename PresentityManagementTrait>
typename Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::const_iterator
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::end () const
{
  return const_iterator (connections.end ());
}

template<typename PresentityType, typename PresentityManagementTrait>
typename Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::iterator
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::end ()
{
  return iterator (connections.end ());
}

template<typename PresentityType, typename PresentityManagementTrait>
void
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::add_presentity (PresentityType &presentity)
{
  sigc::connection rem_conn = presentity.removed.connect (sigc::bind (sigc::mem_fun (this, &HeapImpl::on_presentity_removed), &presentity));
  sigc::connection upd_conn = presentity.updated.connect (sigc::bind (sigc::mem_fun (this, &HeapImpl::on_presentity_updated), &presentity));
  connections[&presentity] = connection_pair (rem_conn, upd_conn);
  presentity_added.emit (presentity);
}

template<typename PresentityType, typename PresentityManagementTrait>
void
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::remove_presentity (PresentityType &presentity)
{
  common_removal_steps (presentity);
  PresentityManagementTrait::announced_release (presentity);
}


template<typename PresentityType, typename PresentityManagementTrait>
void
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::common_removal_steps (PresentityType &presentity)
{
  connection_pair conns = connections[&presentity];
  conns.first.disconnect ();
  conns.second.disconnect ();
  connections.erase (&presentity);
  presentity_removed.emit (presentity);
}

template<typename PresentityType, typename PresentityManagementTrait>
void
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::on_presentity_updated (PresentityType *presentity)
{
  presentity_updated.emit (*presentity);
}

template<typename PresentityType, typename PresentityManagementTrait>
void
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::on_presentity_removed (PresentityType *presentity)
{
  common_removal_steps (*presentity);
  PresentityManagementTrait::release (*presentity);
}

#endif
