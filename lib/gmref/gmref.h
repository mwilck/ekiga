/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras
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
 *                         gmref.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : Reference-counted memory management helpers
 *
 */

#ifndef __GMREF_H__
#define __GMREF_H__

/* base class */
struct GmRefCounted
{
  virtual ~GmRefCounted ()
  {}
};


/* base api */

void gmref_init ();
void gmref_inc (GmRefCounted* obj);
void gmref_dec (GmRefCounted* obj);

/* reference-counted pointer class */

template<typename T>
class gmref_ptr
{
public:

  gmref_ptr (T* obj_);

  gmref_ptr (const gmref_ptr<T>& ptr);

  template<typename Tprim> gmref_ptr (const gmref_ptr<Tprim>& ptr);

  ~gmref_ptr ();

  gmref_ptr<T>& operator= (const gmref_ptr<T>& other);

  T* operator-> () const;

  T& operator* () const;

  operator bool () const;

  bool operator==(const gmref_ptr<T>& other) const;

  bool operator!=(const gmref_ptr<T>& other) const;

private:

  template<typename Tprim> friend class gmref_ptr;
  template<typename Tprim> friend void gmref_inc (gmref_ptr<Tprim> ptr);
  template<typename Tprim> friend void gmref_dec (gmref_ptr<Tprim> ptr);

  void reset ();
  void inc ();

  T* obj;
};

/* extended api */

template<typename T>
void gmref_inc (gmref_ptr<T> ptr)
{
  gmref_inc (ptr.obj);
}

template<typename T>
void gmref_dec (gmref_ptr<T> ptr)
{
  gmref_dec (ptr.obj);
}


/* implementation of the templates */

template<typename T>
gmref_ptr<T>::gmref_ptr (T* obj_ = 0)
{
  obj = obj_;
  inc ();
}

template<typename T>
gmref_ptr<T>::gmref_ptr (const gmref_ptr<T>& ptr)
{
  obj = ptr.obj;
  inc ();
}

template<typename T>
template<typename Tprim>
gmref_ptr<T>::gmref_ptr (const gmref_ptr<Tprim>& ptr)
{
  obj = dynamic_cast<T*>(ptr.obj);
  inc ();
}

template<typename T>
gmref_ptr<T>::~gmref_ptr ()
{
  reset ();
}

template<typename T>
gmref_ptr<T>&
gmref_ptr<T>::operator= (const gmref_ptr<T>& other)
{
  if (this != &other) {

    reset ();
    obj = other.obj;
    inc ();
  }

  return *this;
}

template<typename T>
T*
gmref_ptr<T>::operator-> () const
{
  return obj;
}

template<typename T>
T&
gmref_ptr<T>::operator* () const
{
  return *obj;
}

template<typename T>
gmref_ptr<T>::operator bool () const
{
  return obj != 0;
}

template<typename T>
bool
gmref_ptr<T>::operator==(const gmref_ptr<T>& other) const
{
  return obj == other.obj;
}

template<typename T>
bool
gmref_ptr<T>::operator!=(const gmref_ptr<T>& other) const
{
  return !operator==(other);
}

template<typename T>
void
gmref_ptr<T>::reset ()
{
  if (obj != 0)
    gmref_dec (obj);
  obj = 0;
}

template<typename T>
void
gmref_ptr<T>::inc ()
{
  if (obj != 0)
    gmref_inc (obj);
}

#endif
