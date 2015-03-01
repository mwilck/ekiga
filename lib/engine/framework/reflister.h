/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>

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
 *                         reflister.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : declaration of an object able to list gmref_ptr
 *                          objects
 *
 */

#ifndef __REFLISTER_H__
#define __REFLISTER_H__

#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <list>

#include <boost/smart_ptr.hpp>

#include "live-object.h"
#include "map-key-iterator.h"
#include "map-key-const-iterator.h"
#include "scoped-connections.h"

namespace Ekiga
{
  template<typename ObjectType>
  class RefLister:
    public virtual LiveObject
  {
  public:

    typedef std::map<boost::shared_ptr<ObjectType>, boost::shared_ptr<scoped_connections> > container_type;
    typedef Ekiga::map_key_iterator<container_type> iterator;
    typedef Ekiga::map_key_const_iterator<container_type> const_iterator;

    ~RefLister ();

    void visit_objects (boost::function1<bool, boost::shared_ptr<ObjectType> > visitor) const;

    void add_object (boost::shared_ptr<ObjectType> obj);

    void add_connection (boost::shared_ptr<ObjectType> obj,
			 boost::signals2::connection connection);

    void remove_object (boost::shared_ptr<ObjectType> obj);

    void remove_all_objects ();

    int size () const;

    iterator begin ();
    iterator end ();

    const_iterator begin () const;
    const_iterator end () const;

    boost::signals2::signal<void(boost::shared_ptr<ObjectType>)> object_added;
    boost::signals2::signal<void(boost::shared_ptr<ObjectType>)> object_removed;
    boost::signals2::signal<void(boost::shared_ptr<ObjectType>)> object_updated;

  private:
    container_type objects;
  };

};


template<typename ObjectType>
Ekiga::RefLister<ObjectType>::~RefLister ()
{
  remove_all_objects ();
}


template<typename ObjectType>
void
Ekiga::RefLister<ObjectType>::visit_objects (boost::function1<bool, boost::shared_ptr<ObjectType> > visitor) const
{
  bool go_on = true;
  for (typename container_type::const_iterator iter = objects.begin ();
       go_on && iter != objects.end ();
       ++iter)
    go_on = visitor (iter->first);
}

template<typename ObjectType>
void
Ekiga::RefLister<ObjectType>::add_object (boost::shared_ptr<ObjectType> obj)
{
  typename container_type::iterator iter = objects.find (obj);
  if (iter == objects.end ())
    objects[obj] = boost::shared_ptr<scoped_connections> (new scoped_connections);
  objects[obj]->add (obj->updated.connect (boost::bind (boost::ref (object_updated), obj)));
  objects[obj]->add (obj->updated.connect (boost::ref (updated)));
  objects[obj]->add (obj->removed.connect (boost::bind (&Ekiga::RefLister<ObjectType>::remove_object, this, obj)));

  object_added (obj);
  updated ();
}

template<typename ObjectType>
void
Ekiga::RefLister<ObjectType>::add_connection (boost::shared_ptr<ObjectType> obj,
					      boost::signals2::connection connection)
{
  typename container_type::iterator iter = objects.find (obj);
  if (iter == objects.end ())
    objects[obj] = boost::shared_ptr<scoped_connections> (new scoped_connections);
  objects[obj]->add (connection);
}

template<typename ObjectType>
void
Ekiga::RefLister<ObjectType>::remove_object (boost::shared_ptr<ObjectType> obj)
{
  objects.erase (objects.find (obj));
  object_removed (obj);
  updated ();
}

template<typename ObjectType>
void
Ekiga::RefLister<ObjectType>::remove_all_objects ()
{
  /* iterators get invalidated as we go, hence the strange loop */
  while ( !objects.empty ())
    remove_object (objects.begin ()->first);
}

template<typename ObjectType>
int
Ekiga::RefLister<ObjectType>::size () const
{
  return objects.size ();
}

template<typename ObjectType>
typename Ekiga::RefLister<ObjectType>::iterator
Ekiga::RefLister<ObjectType>::begin ()
{
  return iterator (objects.begin ());
}

template<typename ObjectType>
typename Ekiga::RefLister<ObjectType>::iterator
Ekiga::RefLister<ObjectType>::end ()
{
  return iterator (objects.end ());
}

template<typename ObjectType>
typename Ekiga::RefLister<ObjectType>::const_iterator
Ekiga::RefLister<ObjectType>::begin () const
{
  return const_iterator (objects.begin ());
}

template<typename ObjectType>
typename Ekiga::RefLister<ObjectType>::const_iterator
Ekiga::RefLister<ObjectType>::end () const
{
  return const_iterator (objects.end ());
}

#endif
