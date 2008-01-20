
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

#include "lister.h"
#include "heap.h"

namespace Ekiga
{

/**
 * @addtogroup presence
 * @{
 */

  /** Generic implementation for the Heap pure virtual class.
   *
   * This class is there to make it easy to implement a new type of presentity
   * heap: it will take care of implementing the external api, you
   * just have to decide when to add and remove presentities.
   *
   * It also provides basic memory management for presentities, with the second
   * (optional) template argument:
   *  - either no management (the default);
   *  - or the presentity is considered bound to one heap, which will trigger
   *    its destruction (using delete) when removed from it, which can happen
   *    in two ways: either by calling the remove_presentity method, or by
   *    emission of the presentity's removed signal.
   *
   * Notice that this class won't take care of removing the presentity from a
   * backend -- only from the heap. If you want the presentity <b>deleted</b>
   * then you probably should have an organization like:
   *  - the presentity has a 'deleted' signal;
   *  - the heap listens for this signal;
   *  - when the signal is received, then do a remove_presentity followed by
   *    calling the appropriate api function to delete the presentity in your
   *    backend.
   */
  template<typename PresentityType = Presentity,
	   typename ObjectManagementTrait = delete_object_management<PresentityType> >
  class HeapImpl:
    public Heap,
    protected Lister<PresentityType, ObjectManagementTrait>
  {

  public:

    typedef typename Lister<PresentityType, ObjectManagementTrait>::iterator iterator;
    typedef typename Lister<PresentityType, ObjectManagementTrait>::const_iterator const_iterator;

    HeapImpl ();

    ~HeapImpl ();

    void visit_presentities (sigc::slot<void, Presentity &> visitor);

    const_iterator begin () const;

    iterator begin ();

    const_iterator end () const;

    iterator end ();

  protected:

    void add_presentity (PresentityType &presentity);

    void remove_presentity (PresentityType &presentity);
  };

/**
 * @}
 */

};

/* here are the implementations of the template methods */
template<typename PresentityType, typename PresentityManagementTrait>
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::HeapImpl ()
{
  /* this is signal forwarding */
  Lister<PresentityType,PresentityManagementTrait>::object_added.connect (presentity_added.make_slot ());
  Lister<PresentityType,PresentityManagementTrait>::object_removed.connect (presentity_removed.make_slot ());
  Lister<PresentityType,PresentityManagementTrait>::object_updated.connect (presentity_updated.make_slot ());
}


template<typename PresentityType, typename PresentityManagementTrait>
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::~HeapImpl ()
{
}

template<typename PresentityType, typename PresentityManagementTrait>
void
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::visit_presentities (sigc::slot<void, Presentity &> visitor)
{
  for (iterator iter = begin (); iter != end (); iter++)
    visitor (*iter);
}

template<typename PresentityType, typename PresentityManagementTrait>
typename Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::iterator
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::begin ()
{
  return Lister<PresentityType, PresentityManagementTrait>::begin ();
}

template<typename PresentityType, typename PresentityManagementTrait>
typename Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::iterator
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::end ()
{
  return Lister<PresentityType, PresentityManagementTrait>::end ();
}

template<typename PresentityType, typename PresentityManagementTrait>
typename Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::const_iterator
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::begin () const
{
  return Lister<PresentityType, PresentityManagementTrait>::begin ();
}

template<typename PresentityType, typename PresentityManagementTrait>
typename Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::const_iterator
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::end () const
{
  return Lister<PresentityType, PresentityManagementTrait>::end ();
}

template<typename PresentityType, typename PresentityManagementTrait>
void
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::add_presentity (PresentityType &presentity)
{
  presentity.questions.add_handler (questions.make_slot ());
  add_object (presentity);
}

template<typename PresentityType, typename PresentityManagementTrait>
void
Ekiga::HeapImpl<PresentityType, PresentityManagementTrait>::remove_presentity (PresentityType &presentity)
{
  remove_object (presentity);
}

#endif
