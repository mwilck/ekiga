
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
 *                         cluster-impl.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : partial implementation of a cluster
 *
 */

#ifndef __CLUSTER_IMPL_H__
#define __CLUSTER_IMPL_H__

#include "map-key-reference-iterator.h"
#include "map-key-const-reference-iterator.h"
#include "cluster.h"

/* This class is there to make it easy to implement a new type of
 * cluster : it will take care of implementing the external api, you
 * just have to decide when to add and remove heaps.
 *
 * It also provides basic memory management for heaps, with the second
 * (optional) template argument :
 * - either no management (the default) ;
 * - or the heap is considered bound to one cluster, which will trigger its
 * destruction (using delete) when removed from it, which can happen in two
 * ways : either by calling the remove_heap method, or by emission of the
 * heap's removed signal.
 *
 * Notice that this class won't take care of removing the heap from a
 * backend -- only from the cluster. If you want the heap *deleted* then you
 * probably should have an organization like :
 * - the heap has a 'deleted' signal ;
 * - the cluster listens for this signal ;
 * - when the signal is received, then do a remove_heap followed by calling
 * the appropriate api function to delete the heap in your backend.
 */

#define connection_pair std::pair<sigc::connection, sigc::connection>

namespace Ekiga {

  template<typename HeapType>
  struct no_heap_management
  {
    static void announced_release (HeapType &);

    static void release (HeapType &);
  };

  template<typename HeapType>
  struct delete_heap_management
  {
    static void announced_release (HeapType &heap);

    static void release (HeapType &heap);

  };

  template<typename HeapType = Heap,
	   typename HeapManagementTrait = no_heap_management <HeapType> >
  class ClusterImpl: public Cluster
  {

  public:

    typedef MapKeyReferenceIterator<HeapType, connection_pair> iterator;
    typedef MapKeyConstReferenceIterator<HeapType, connection_pair> const_iterator;

    virtual ~ClusterImpl ();

    void visit_heaps (sigc::slot<void, Heap &> visitor);

    const_iterator begin () const;

    iterator begin ();

    const_iterator end () const;

    iterator end ();

  protected:

    void add_heap (HeapType &heap);

    void remove_heap (HeapType &heap);

  private:

    void common_removal_steps (HeapType &heap);

    void on_heap_updated (HeapType *heap);

    void on_heap_removed (HeapType *heap);

    std::map<HeapType *, connection_pair> connections;
  };

};

/* here are the implementations of the template methods */

template<typename HeapType>
void
Ekiga::no_heap_management<HeapType>::announced_release (HeapType &)
{
  // nothing
}

template<typename HeapType>
void
Ekiga::no_heap_management<HeapType>::release (HeapType &)
{
  // nothing
}

template<typename HeapType>
void
Ekiga::delete_heap_management<HeapType>::announced_release (HeapType &heap)
{
  heap.removed.emit ();
  release (heap);
}

template<typename HeapType>
void
Ekiga::delete_heap_management<HeapType>::release (HeapType &heap)
{
  delete &heap;
}

template<typename HeapType, typename HeapManagementTrait>
Ekiga::ClusterImpl<HeapType, HeapManagementTrait>::~ClusterImpl ()
{
  iterator iter = begin ();

  while (iter != end ()) {

    remove_heap (*iter); // here iter becomes invalid
    iter = begin ();
  }
}

template<typename HeapType, typename HeapManagementTrait>
void
Ekiga::ClusterImpl<HeapType, HeapManagementTrait>::visit_heaps (sigc::slot<void, Heap &> visitor)
{
  for (iterator iter = begin (); iter != end (); iter++)
    visitor (*iter);
}

template<typename HeapType, typename HeapManagementTrait>
typename Ekiga::ClusterImpl<HeapType, HeapManagementTrait>::const_iterator
Ekiga::ClusterImpl<HeapType, HeapManagementTrait>::begin () const
{
  return const_iterator (connections.begin ());
}

template<typename HeapType, typename HeapManagementTrait>
typename Ekiga::ClusterImpl<HeapType, HeapManagementTrait>::iterator
Ekiga::ClusterImpl<HeapType, HeapManagementTrait>::begin ()
{
  return iterator (connections.begin ());
}

template<typename HeapType, typename HeapManagementTrait>
typename Ekiga::ClusterImpl<HeapType, HeapManagementTrait>::const_iterator
Ekiga::ClusterImpl<HeapType, HeapManagementTrait>::end () const
{
  return const_iterator (connections.end ());
}

template<typename HeapType, typename HeapManagementTrait>
typename Ekiga::ClusterImpl<HeapType, HeapManagementTrait>::iterator
Ekiga::ClusterImpl<HeapType, HeapManagementTrait>::end ()
{
  return iterator (connections.end ());
}

template<typename HeapType, typename HeapManagementTrait>
void
Ekiga::ClusterImpl<HeapType, HeapManagementTrait>::add_heap (HeapType &heap)
{
  sigc::connection rem_conn = heap.removed.connect (sigc::bind (sigc::mem_fun (this, &ClusterImpl::on_heap_removed), &heap));
  sigc::connection upd_conn = heap.updated.connect (sigc::bind (sigc::mem_fun (this, &ClusterImpl::on_heap_updated), &heap));

  connections[&heap] = connection_pair (rem_conn, upd_conn);
  heap_added.emit (heap);
}

template<typename HeapType, typename HeapManagementTrait>
void
Ekiga::ClusterImpl<HeapType, HeapManagementTrait>::remove_heap (HeapType &heap)
{
  common_removal_steps (heap);
  HeapManagementTrait::announced_release (heap);
}

template<typename HeapType, typename HeapManagementTrait>
void
Ekiga::ClusterImpl<HeapType, HeapManagementTrait>::common_removal_steps (HeapType &heap)
{
  connection_pair conns = connections[&heap];
  conns.first.disconnect ();
  conns.second.disconnect ();
  connections.erase (&heap);
  heap_removed.emit (heap);
}

template<typename HeapType, typename HeapManagementTrait>
void
Ekiga::ClusterImpl<HeapType, HeapManagementTrait>::on_heap_updated (HeapType *heap)
{
  heap_updated.emit (*heap);
}

template<typename HeapType, typename HeapManagementTrait>
void
Ekiga::ClusterImpl<HeapType, HeapManagementTrait>::on_heap_removed (HeapType *heap)
{
  common_removal_steps (*heap);
  HeapManagementTrait::release (*heap);
}

#endif
