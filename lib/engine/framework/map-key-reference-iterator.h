
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
 *                         map-key-reference-iterator.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of an iterator on keys of a std::map
 *
 */

#ifndef __MAP_KEY_REFERENCE_ITERATOR_H__
#define __MAP_KEY_REFERENCE_ITERATOR_H__

#include <map>

/* You have a std::map<RealKeyType *, DataType>, but would like to provide
 * a way to iterate over (1) the keys but (2) not as pointers but as references
 *
 * This class is for you!
 *
 * Notice that if you would like the references to be of an ancestor type of
 * RealKeyType, it's possible with the third template argument.
 */

namespace Ekiga
{

  template<typename RealKeyType,
	   typename DataType,
	   typename BaseKeyType = RealKeyType>
  class MapKeyReferenceIterator: public std::forward_iterator_tag
  {

  public:

    MapKeyReferenceIterator (typename std::map<RealKeyType *, DataType>::iterator _it) : it(_it) {}

    ~MapKeyReferenceIterator () {}

    MapKeyReferenceIterator &operator=(const MapKeyReferenceIterator &other);

    bool operator== (const MapKeyReferenceIterator &other);

    bool operator!= (const MapKeyReferenceIterator &other);

    MapKeyReferenceIterator &operator++ ();

    MapKeyReferenceIterator operator++(int);

    BaseKeyType &operator*();

    BaseKeyType *operator->();

  private:

    typename std::map<RealKeyType *, DataType>::iterator it;

  };

};

/* here come the implementations of the template methods */

template<typename RealKeyType, typename DataType, typename BaseKeyType>
Ekiga::MapKeyReferenceIterator<RealKeyType, DataType, BaseKeyType> &
Ekiga::MapKeyReferenceIterator<RealKeyType, DataType, BaseKeyType>::operator=(const Ekiga::MapKeyReferenceIterator<RealKeyType, DataType, BaseKeyType> &other)
{
  it = other.it;
  return *this;
}

template<typename RealKeyType, typename DataType, typename BaseKeyType>
bool
Ekiga::MapKeyReferenceIterator<RealKeyType, DataType, BaseKeyType>::operator==(const Ekiga::MapKeyReferenceIterator<RealKeyType, DataType, BaseKeyType> &other)
{
  return it == other.it;
}

template<typename RealKeyType, typename DataType, typename BaseKeyType>
bool
Ekiga::MapKeyReferenceIterator<RealKeyType, DataType, BaseKeyType>::operator!=(const Ekiga::MapKeyReferenceIterator<RealKeyType, DataType, BaseKeyType> &other)
{
  return it != other.it;
}

template<typename RealKeyType, typename DataType, typename BaseKeyType>
Ekiga::MapKeyReferenceIterator<RealKeyType, DataType, BaseKeyType> &
Ekiga::MapKeyReferenceIterator<RealKeyType, DataType, BaseKeyType>::operator++()
{
  it++;
  return *this;
}

template<typename RealKeyType, typename DataType, typename BaseKeyType>
Ekiga::MapKeyReferenceIterator<RealKeyType, DataType, BaseKeyType>
Ekiga::MapKeyReferenceIterator<RealKeyType, DataType, BaseKeyType>::operator++(int)
{
  Ekiga::MapKeyReferenceIterator<RealKeyType, DataType, BaseKeyType> tmp = *this;
  it++;
  return tmp;
}

template<typename RealKeyType, typename DataType, typename BaseKeyType>
BaseKeyType &
Ekiga::MapKeyReferenceIterator<RealKeyType, DataType, BaseKeyType>::operator*()
{
  return *static_cast<BaseKeyType *>((*it).first);
}

template<typename RealKeyType, typename DataType, typename BaseKeyType>
BaseKeyType *
Ekiga::MapKeyReferenceIterator<RealKeyType, DataType, BaseKeyType>::operator->()
{
  return static_cast<BaseKeyType *>((*it).first);
}

#endif
