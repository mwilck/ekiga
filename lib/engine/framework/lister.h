
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
 *                         lister.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of an object able to list others
 *
 */

#ifndef __LIST_H__
#define __LIST_H__

#include <sigc++/sigc++.h>

#include "map-key-reference-iterator.h"
#include "map-key-const-reference-iterator.h"


/** Ekiga::List
 *
 * This class is there to help write a dynamic object lister, that is an object
 * which will more or less 'own' objects which will emit "updated" and
 * "removed" signals.
 *
 * The more or less means that it supports different memory management traits
 * using the second (optional) template argument :
 * - either no management (the default) ;
 * - or the object is considered fully owned, which will trigger its
 * destruction (using delete) when removed.
 *
 * You can remove an object from an Ekiga::Lister in two ways :
 * - either by calling the remove_object method,
 * - or by emission of the object's removed signal.
 *
 * Notice that this class won't take care of removing the object from a
 * backend -- only from the Ekiga::Lister.
 * If you want the object *deleted* from the real backend, then you
 * probably should have an organization like :
 * - the object has a 'deleted' signal ;
 * - the lister child-class listens to this signal ;
 * - when the signal is received, then do a remove_object followed by calling
 * the appropriate api function to delete the object in your backend.
 */

#define connection_pair std::pair<sigc::connection, sigc::connection>


namespace Ekiga {

  template<typename ObjectType>
  struct no_object_management
  {
    static void announced_release (ObjectType &);

    static void release (ObjectType &);
  };

  template<typename ObjectType>
  struct delete_object_management
  {
    static void announced_release (ObjectType &object);

    static void release (ObjectType &object);

  };

  template<typename ObjectType,
	   typename ObjectManagementTrait = no_object_management<ObjectType> >
  class Lister
  {

  public:

    typedef MapKeyReferenceIterator<ObjectType, connection_pair> iterator;
    typedef MapKeyConstReferenceIterator<ObjectType, connection_pair> const_iterator;


    /** The destructor.
     */
    ~Lister ();


    /** Returns a const iterator to the first object of the collection.
     */
    const_iterator begin () const;


    /** Returns an iterator to the first object of the collection.
     */
    iterator begin ();


    /** Returns a const iterator to the first object of the collection.
     */
    const_iterator end () const;


    /** Returns an iterator to the last object of the collection.
     */
    iterator end ();

  protected:

    /** Adds an object to the Ekiga::Lister.
     * @param: The object to be added.
     * @return: The Ekiga::Lister 'object_added' signal is emitted when
     *          the object has been added. The
     *          Ekiga::Lister 'object_updated' signal will be emitted
     *          when the object has been updated and the
     *          Ekiga::Lister 'object_removed' signal will be emitted when
     *          the object has been removed from the Ekiga::Lister.
     */
    void add_object (ObjectType &object);


    /** Removes an object from the Ekiga::Lister.
     * @param: The object to be removed.
     * @return: The Ekiga::Lister 'object_removed' signal is emitted when
     * the object has been removed. The ObjectManagementTrait associated
     * with the Ekiga::Lister will determine the memory management policy
     * for that object.
     */
    void remove_object (ObjectType &object);


    /** Signals emitted by this object
     *
     */
    sigc::signal<void, ObjectType &> object_added;
    sigc::signal<void, ObjectType &> object_removed;
    sigc::signal<void, ObjectType &> object_updated;

  private:

    /** Disconnects the signals for the object, emits the 'object_removed'
     * signal on the Ekiga::Lister and takes care of the release of that
     * object following the policy of the ObjectManagementTrait associated
     * with the Ekiga::Lister.
     * @param: The object to remove.
     */
    void common_removal_steps (ObjectType &object);


    /** This callback is triggered when the 'updated' signal is emitted on
     * an object.
     * Emits the Ekiga::Lister 'object_updated' signal for that object.
     * @param: The updated object.
     */
    void on_object_updated (ObjectType *object);


    /** This callback is triggered when the 'removed' signal is emitted on
     * an object.
     * Emits the Ekiga::Lister 'object_removed' signal for that object and
     * takes care of the deletion of the object or not following the
     * ObjectManagementTrait associated with the Ekiga::Lister.
     * @param: The removed object.
     */
    void on_object_removed (ObjectType *object);

    /** Map of objects and signals.
     */
    std::map<ObjectType *, connection_pair> connections;
  };
};


/* here begins the code from the template functions */
template<typename ObjectType>
void
Ekiga::no_object_management<ObjectType>::announced_release (ObjectType &object)
{
  // nothing
}


template<typename ObjectType>
void
Ekiga::no_object_management<ObjectType>::release (ObjectType &object)
{
  // nothing
}


template<typename ObjectType>
void
Ekiga::delete_object_management<ObjectType>::announced_release (ObjectType &object)
{
  object.removed.emit ();
  release (object);
}


template<typename ObjectType>
void
Ekiga::delete_object_management<ObjectType>::release (ObjectType &object)
{
  delete &object;
}


template<typename ObjectType, typename ObjectManagementTrait>
Ekiga::Lister<ObjectType, ObjectManagementTrait>::~Lister ()
{
  iterator iter = begin ();

  while (iter != end ()) {

    remove_object (*iter); // here iter becomes invalid
    iter = begin ();
  }
}


template<typename ObjectType, typename ObjectManagementTrait>
typename Ekiga::Lister<ObjectType, ObjectManagementTrait>::const_iterator
Ekiga::Lister<ObjectType, ObjectManagementTrait>::begin () const
{
  return const_iterator (connections.begin ());
}


template<typename ObjectType, typename ObjectManagementTrait>
typename Ekiga::Lister<ObjectType, ObjectManagementTrait>::iterator
Ekiga::Lister<ObjectType, ObjectManagementTrait>::begin ()
{
  return iterator (connections.begin ());
}


template<typename ObjectType, typename ObjectManagementTrait>
typename Ekiga::Lister<ObjectType, ObjectManagementTrait>::const_iterator
Ekiga::Lister<ObjectType, ObjectManagementTrait>::end () const
{
  return const_iterator (connections.end ());
}


template<typename ObjectType, typename ObjectManagementTrait>
typename Ekiga::Lister<ObjectType, ObjectManagementTrait>::iterator
Ekiga::Lister<ObjectType, ObjectManagementTrait>::end ()
{
  return iterator (connections.end ());
}


template<typename ObjectType, typename ObjectManagementTrait>
void
Ekiga::Lister<ObjectType, ObjectManagementTrait>::add_object (ObjectType &object)
{
  sigc::connection rem_conn = object.removed.connect (sigc::bind (sigc::mem_fun (this, &Lister::on_object_removed), &object));
  sigc::connection upd_conn = object.updated.connect (sigc::bind (sigc::mem_fun (this, &Lister::on_object_updated), &object));
  connections[&object] = connection_pair (rem_conn, upd_conn);
  object_added.emit (object);
}


template<typename ObjectType, typename ObjectManagementTrait>
void
Ekiga::Lister<ObjectType, ObjectManagementTrait>::remove_object (ObjectType &object)
{
  common_removal_steps (object);
  ObjectManagementTrait::announced_release (object);
}


template<typename ObjectType, typename ObjectManagementTrait>
void
Ekiga::Lister<ObjectType, ObjectManagementTrait>::common_removal_steps (ObjectType &object)
{
  connection_pair conns = connections[&object];
  conns.first.disconnect ();
  conns.second.disconnect ();
  connections.erase (&object);
  object_removed.emit (object);
}


template<typename ObjectType, typename ObjectManagementTrait>
void
Ekiga::Lister<ObjectType, ObjectManagementTrait>::on_object_updated (ObjectType *object)
{
  object_updated.emit (*object);
}


template<typename ObjectType, typename ObjectManagementTrait>
void
Ekiga::Lister<ObjectType, ObjectManagementTrait>::on_object_removed (ObjectType *object)
{
  common_removal_steps (*object);
  ObjectManagementTrait::release (*object);
}

#endif
